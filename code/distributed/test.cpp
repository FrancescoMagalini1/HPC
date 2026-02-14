#include <mpi.h>
#include <stdio.h>
#include <iostream>

using namespace std;

int main()
{
    MPI_Init(NULL, NULL);
    MPI_Barrier(MPI_COMM_WORLD);
    double start = MPI_Wtime();
    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    // Print off a hello world message
    cout << "Hello world from process rank " << world_rank << " out of " << world_size << " processors\n";
    // Finalize the MPI environment. No more MPI calls can be made after this
    MPI_Barrier(MPI_COMM_WORLD);
    double end = MPI_Wtime();
    if (world_rank == 0)
    {
        cout << "Execution time: " << end - start << " seconds\n";
    }

    MPI_Finalize();

    return 0;
}