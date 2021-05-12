#!/usr/bin/env bash
#SBATCH -p wacc
#SBATCH -J connect4cuda_seqtiming
#SBATCH -c 1
#SBATCH -o seq-timing.out -e seq-timing.err

build/app --time-seq

