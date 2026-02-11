#!/bin/bash
# change the number of cpus as needed, more threads should require more cpus
#PBS -l select=1:ncpus=8:mem=2gb
# set max execution time
#PBS -l walltime=00:03:00
# imposta la coda di esecuzione
#PBS -q shortCPUQ
./HPC/code/parallel/executables/linux/parallel "$ARG1" "$ARG2" "$ARG3"