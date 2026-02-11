#!/bin/bash
#PBS -l select=1:ncpus=1:mem=2gb -l place=pack:excl
# set max execution time
#PBS -l walltime=00:01:00
# imposta la coda di esecuzione
#PBS -q shortCPUQ
./HPC/code/serial/executables/linux/serial "$ARG1"