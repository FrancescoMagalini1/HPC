#include <mpi.h>
#include <stdio.h>
#include <iostream>
#include "../libraries/utils.h"
#include "../libraries/distributed_functions.h"
#include <stdlib.h>
#include <vector>
#include <string>
#include <omp.h>

using namespace std;
using namespace utils;
using namespace distributed_functions;
using DVector = vector<double>;

int main(int argc, char *argv[])
{
    int matrixSize = argc > 1 ? stoi(argv[1]) : 100;
    int threads = argc > 2 ? stoi(argv[2]) : 4;
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
    if (processRank == 0)
    {
        cout << "Matrix size: " << matrixSize << "x" << matrixSize
             << ", Number of processes: " << nProcesses << endl;

        DMatrix matrix = toDouble(getRand(matrixSize, 1, 100));
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

    for (int iter = 0; iter < 1; iter++)
    {
        DVector localSums = sumAlongDistributed(localBuffer, rowsPerProcess, matrixSize, 1);
        cout << "Process " << processRank << " computed local sums for iteration " << iter << ": ";
        for (double sum : localSums)
        {
            cout << sum << " ";
        }
        cout << endl;
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