# ParallelProgramming

This repository contains tasks and comparisons of basic mathematical computations using OpenMP and MPI parallel programming interfaces. The tasks are computed and analyzed based on computational power, threads, and performance. Used EduMPI, Tau and Cube tools to perform the performance analysis of the different algorithms.

## Directory Structure

### MPI
The `mpi` directory contains the following files:
- `Cannon_Algorithm_TaskA.pdf`: Documentation and analysis of Task A using Cannon's algorithm.
- `Cannon_Algorithm_TaskB.pdf`: Documentation and analysis of Task B using Cannon's algorithm.
- `Parallel Programming MPI Presentation.pdf`: Presentation slides explaining MPI and its applications.
- `cannon_algorithm_taskA.c`: Source code for Task A using Cannon's algorithm.
- `cannon_algorithm_taskB.c`: Source code for Task B using Cannon's algorithm.
- `cannon_input_generator_taskA.c`: Input generator code for Task A.

### OpenMP
The `openmp` directory contains the following files:
- `Screenshot 2024-11-28 at 22.08.04.png`: Screenshot related to OpenMP tasks.
- `array_sum_parallel.c`: Source code for parallel array summation using OpenMP.
- `array_sum_serial.c`: Source code for serial array summation for comparison with the parallel version.
- `openMP_TaskB_explanation.pdf`: Explanation and analysis of Task B using OpenMP.
- `sorting_algorithms_taskB.c`: Source code for sorting algorithms implemented with OpenMP.
- `taskA_sum_of_random_array.pdf`: Documentation and analysis of Task A, which involves summing a random array.

## Usage
To compile and run the MPI programs, use the following commands:
```sh
mpicc cannon_algorithm_taskA.c -o taskA
mpirun -np 4 ./taskA
```

To compile and run the OpenMP programs, use the following commands:
```sh
gcc -fopenmp array_sum_parallel.c -o parallel_sum
./parallel_sum
```
