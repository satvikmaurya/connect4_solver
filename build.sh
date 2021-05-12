#!/usr/bin/env bash
#SBATCH -p wacc
#SBATCH -J connect4build
#SBATCH -c 1
#SBATCH -o build.out -e build.err
#SBATCH --time=0-0:5:0

module load cuda

mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../src
cmake --build .

module unload cuda
