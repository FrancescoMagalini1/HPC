#include <mpi.h>
#include <stdio.h>
#include <iostream>
#include "../libraries/utils.h"
#include "../libraries/distributed_functions.h"
#include <stdlib.h>
#include <vector>
#include <string>
#include <omp.h>
#include <fstream>
#include "../libraries/json.hpp"

using namespace std;
using namespace utils;
using namespace distributed_functions;
using DVector = vector<double>;
using json = nlohmann::json;

int main(int argc, char *argv[])
{
    unsigned long int matrixSize = argc > 1 ? stoi(argv[1]) : 100;
    int threads = argc > 2 ? stoi(argv[2]) : 4; // number of threads for OpenMP
    int maxIterations = argc > 3 ? stoi(argv[3]) : 1;
    bool saveFile = argc > 4 ? stoi(argv[4]) : 0;
    omp_set_num_threads(threads);
    MPI_Init(NULL, NULL);
    MPI_Barrier(MPI_COMM_WORLD);
    double startTime = MPI_Wtime();
    // Get the number of processes
    int nProcesses;
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);
    // Get the rank of the process
    int processRank;
    MPI_Comm_rank(MPI_COMM_WORLD, &processRank);
    // The matrix must be divisible by the number of processes for an even distribution
    if (matrixSize % nProcesses != 0)
    {
        if (processRank == 0)
        {
            cerr << "Error: Matrix size n = " << matrixSize
                 << " is not divisible by number of processes p = "
                 << nProcesses << endl;
        }
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        return EXIT_FAILURE;
    }
    unsigned long int rowsPerProcess = matrixSize / nProcesses;
    DVector flatBuffer; // only meaningful on root
    DVector localBuffer(rowsPerProcess * matrixSize);
    DMatrix initialMatrix; // only meaningful on root
    if (processRank == 0)
    {
        cout << "Matrix size: " << matrixSize << "x" << matrixSize
             << ", Number of processes: " << nProcesses << ", Number of iterations: " << maxIterations << endl;

        DMatrix matrix = toDouble(getRand(matrixSize, 1, 100));
        initialMatrix = matrix;
        flatBuffer.resize(matrixSize * matrixSize);
        for (unsigned long int i = 0; i < matrixSize; i++)
        {
            for (unsigned long int j = 0; j < matrixSize; j++)
            {
                flatBuffer[i * matrixSize + j] = matrix[i][j];
            }
        }
    }
    // Equally distribute the matrix rows to all processes. Each process receives a contiguous block of rows.
    MPI_Scatter(
        flatBuffer.data(),
        rowsPerProcess * matrixSize,
        MPI_DOUBLE,
        localBuffer.data(),
        rowsPerProcess * matrixSize,
        MPI_DOUBLE,
        0,
        MPI_COMM_WORLD);

    DVector localD1(rowsPerProcess, 1.0);
    DVector localD2(matrixSize, 1.0);
    for (int iter = 0; iter < maxIterations; iter++)
    {
        DVector localRowSums = sumAlongDistributed(localBuffer, rowsPerProcess, matrixSize, 1);
        localD1 = vectorMultiply(localD1, localRowSums);
        localBuffer = normalizeDistributed(localBuffer, localRowSums, rowsPerProcess, matrixSize, 1);
        DVector localColSums = sumAlongDistributed(localBuffer, rowsPerProcess, matrixSize, 0);
        DVector globalColSums(matrixSize, 0.0);
        // Combine the local column sums from all processes to get the global column sums using an MPI reduction operation
        MPI_Allreduce(
            localColSums.data(),
            globalColSums.data(),
            matrixSize,
            MPI_DOUBLE,
            MPI_SUM,
            MPI_COMM_WORLD);

        localD2 = vectorMultiply(localD2, globalColSums);
        localBuffer = normalizeDistributed(localBuffer, globalColSums, rowsPerProcess, matrixSize, 0);
    }
    // Gather the normalized submatrices back to the root process
    MPI_Gather(
        localBuffer.data(),
        rowsPerProcess * matrixSize,
        MPI_DOUBLE,
        flatBuffer.data(),
        rowsPerProcess * matrixSize,
        MPI_DOUBLE,
        0, // root process
        MPI_COMM_WORLD);
    // Gather the localD1 and localD2 vectors back to the root process and combine them to get the global D1 and D2 vectors.
    DVector globalD1;
    DVector globalD2;

    if (processRank == 0)
    {
        globalD1.resize(matrixSize);
        globalD2.resize(matrixSize);
    }
    // Gather localD1 from all processes to globalD1 on root
    MPI_Gather(
        localD1.data(),
        rowsPerProcess,
        MPI_DOUBLE,
        globalD1.data(),
        rowsPerProcess,
        MPI_DOUBLE,
        0, // root process
        MPI_COMM_WORLD);

    // Since all processes have the same localD2, we can just copy it on root
    if (processRank == 0)
    {
        globalD2 = localD2;
        // Save the results to a JSON file
        if (saveFile)
        {
            DMatrix finalMatrix(matrixSize, vector<double>(matrixSize));
            for (unsigned long int i = 0; i < matrixSize; i++)
            {
                for (unsigned long int j = 0; j < matrixSize; j++)
                {
                    finalMatrix[i][j] = flatBuffer[i * matrixSize + j];
                }
            }
            json j;
            j["D1"] = globalD1;
            j["D2"] = globalD2;
            j["initialMatrix"] = initialMatrix;
            j["finalMatrix"] = finalMatrix;
            ofstream file("HPC/code/distributed/result.json");
            if (!file.is_open())
            {
                cerr << "Cannot open file for writing\n";
                return 1;
            }
            file << j.dump(4) << endl;
            file.close();
            cout << "Saved to result.json\n";
        }
    }
    // Finalize the MPI environment. No more MPI calls can be made after this
    MPI_Barrier(MPI_COMM_WORLD);
    double endTime = MPI_Wtime();
    if (processRank == 0)
    {
        cout << "Execution time: " << endTime - startTime << " seconds\n";
    }

    MPI_Finalize();

    return 0;
}