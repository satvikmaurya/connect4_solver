#!/usr/bin/env bash
#SBATCH -p wacc
#SBATCH -J connect4cuda_omptiming
#SBATCH -c 11
#SBATCH -o omp-timing.out -e omp-timing.err

build/app --time-omp --num-threads 11

