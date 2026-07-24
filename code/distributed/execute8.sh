#!/bin/bash
# change the number of cpus as needed, more threads should require more cpus
#PBS -l select=8:ncpus=4:mem=80gb -l place=pack
# set max execution time
#PBS -l walltime=03:00:00
# imposta la coda di esecuzione
#PBS -q shortCPUQ
module load OpenMPI/4.1.6-GCC-13.2.0
echo "ARG2: $ARG2"
mpirun -n 8 ./HPC/code/distributed/executables/distributed "$ARG1" "$ARG2" "$ARG3"