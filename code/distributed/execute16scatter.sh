#!/bin/bash
# change the number of cpus as needed, more threads should require more cpus
#PBS -l select=16:ncpus=4:mem=160gb -l place=scatter
# set max execution time
#PBS -l walltime=03:00:00
# imposta la coda di esecuzione
#PBS -q shortCPUQ
module load OpenMPI/4.1.6-GCC-13.2.0
echo "ARG2: $ARG2"
mpirun -n 16 ./HPC/code/distributed/executables/distributed "$ARG1" "$ARG2" "$ARG3"