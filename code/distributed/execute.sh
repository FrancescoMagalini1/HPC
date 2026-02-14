#!/bin/bash
# change the number of cpus as needed, more threads should require more cpus
#PBS -l select=2:ncpus=8:mem=4gb
# set max execution time
#PBS -l walltime=00:03:00
# imposta la coda di esecuzione
#PBS -q shortCPUQ
module load OpenMPI/4.1.6-GCC-13.2.0
mpirun -n 5 ./HPC/code/distributed/executables/test