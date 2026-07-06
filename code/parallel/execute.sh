#!/bin/bash
# change the number of cpus as needed, more threads should require more cpus
#PBS -l select=1:ncpus=8:mem=20gb
# set max execution time
#PBS -l walltime=01:30:00
# imposta la coda di esecuzione
#PBS -q shortCPUQ
./HPC/code/parallel/executables/linux/parallel "$ARG1" "$ARG2" "$ARG3"