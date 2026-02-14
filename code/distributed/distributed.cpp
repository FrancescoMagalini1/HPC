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
    int matrixSize = argc > 1 ? stoi(argv[1]) : 100;
    int threads = argc > 2 ? stoi(argv[2]) : 4;
    int maxIterations = argc > 3 ? stoi(argv[3]) : 1;
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
    if (matrixSize % nProcesses != 0)
    {
        if (processRank == 0)
        {
            cerr << "Error: Matrix size n = " << matrixSize
                 << " is not divisible by number of processes p = "
                 << nProcesses << endl;
        }
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        return EXIT_FAILURE; // (not strictly necessary after MPI_Abort)
    }
    int rowsPerProcess = matrixSize / nProcesses;
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
        for (int i = 0; i < matrixSize; i++)
        {
            for (int j = 0; j < matrixSize; j++)
            {
                flatBuffer[i * matrixSize + j] = matrix[i][j];
            }
        }
    }

    MPI_Scatter(
        flatBuffer.data(),           // send buffer (root only)
        rowsPerProcess * matrixSize, // elements per process
        MPI_DOUBLE,
        localBuffer.data(), // receive buffer
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
        MPI_Allreduce(
            localColSums.data(),  // send buffer
            globalColSums.data(), // receive buffer
            matrixSize,           // number of elements
            MPI_DOUBLE,
            MPI_SUM, // reduction operation
            MPI_COMM_WORLD);

        localD2 = vectorMultiply(localD2, globalColSums);
        localBuffer = normalizeDistributed(localBuffer, globalColSums, rowsPerProcess, matrixSize, 0);
    }
    // Gather the normalized submatrices back to the root process
    MPI_Gather(
        localBuffer.data(),          // send buffer
        rowsPerProcess * matrixSize, // elements to send
        MPI_DOUBLE,
        flatBuffer.data(),           // receive buffer (root only)
        rowsPerProcess * matrixSize, // elements to receive per process
        MPI_DOUBLE,
        0, // root process
        MPI_COMM_WORLD);
    // Gather the localD1 and localD2 vectors back to the root process
    DVector globalD1;
    DVector globalD2;

    if (processRank == 0)
    {
        globalD1.resize(matrixSize);
        globalD2.resize(matrixSize);
    }

    MPI_Gather(
        localD1.data(), // send buffer
        rowsPerProcess, // elements to send
        MPI_DOUBLE,
        globalD1.data(), // receive buffer (root only)
        rowsPerProcess,  // elements to receive per process
        MPI_DOUBLE,
        0, // root process
        MPI_COMM_WORLD);

    // Since all processes have the same localD2, we can just copy it on root
    // Or gather and verify consistency
    if (processRank == 0)
    {
        globalD2 = localD2; // All processes have the same D2
        // write to json globald1, globald2, initial matrix, final matrix
        json j;
        j["D1"] = globalD1;
        j["D2"] = globalD2;
        j["initialMatrix"] = initialMatrix;
        j["finalMatrix"] = flatBuffer;
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