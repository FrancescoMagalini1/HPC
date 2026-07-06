#!/bin/bash
#PBS -l select=1:ncpus=3:mem=10gb
# set max execution time
#PBS -l walltime=01:30:00
# imposta la coda di esecuzione
#PBS -q shortCPUQ
./HPC/code/serial/executables/linux/serial "$ARG1"