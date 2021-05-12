#!/usr/bin/env bash
#SBATCH -p wacc
#SBATCH -J connect4cuda
#SBATCH -c 1
#SBATCH -o cuda-timing.out -e cuda-timing.err
#SBATCH --gres=gpu:1

module load cuda

for n in 2 4 6  #8 10 12 
do
    echo "Depth: $n"
    build/app --time-cuda --search-depth $n
done

module unload cuda
