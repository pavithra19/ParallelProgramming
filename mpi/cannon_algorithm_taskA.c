// cannon_algorithm.c
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* This code is written by
Pavithra Purushothaman (fd0001571)
Matrikel Nummer: 1535949
*/

#define IDX(i, j, N) ((i) * (N) + (j)) // Macro for 2D coordinates (i, j) to 1D index in a matrix of width N

// Reading input matrices from the files
void read_matrix(const char* filename, int** matrix, int* N) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Error opening input file %s\n", filename);
        exit(1);
    }

    fscanf(file, "%d", N); // Getting matrix size N from the matrices file
    *matrix = (int*)malloc((*N) * (*N) * sizeof(int)); // Allocating memory for the matrix size

    // Reading matrix elements from the generated files
    for (int i = 0; i < (*N) * (*N); i++) {
        fscanf(file, "%d", &(*matrix)[i]);
    }

    fclose(file);
}

// Function to print matrix in the console (up to 8x8 is printed in UI and ellipses is added for further larger matrices)
void print_matrix(const char* name, int *matrix, int N) {
    printf("%s:\n", name);
    int limit = (N > 8) ? 8 : N;

    for (int i = 0; i < limit; i++) {
      for (int j = 0; j < limit; j++) {
        printf("%d ", matrix[IDX(i, j, N)]);
      }
    
    if (N > 8) {
      printf(".. ..");
      }
      printf("\n");
    }

    if (N > 8) {
      for (int i = 0; i < 2; i++) {
        printf(".. .. .. .. .. .. .. .. .. ..\n");
        }
      printf("\nDue to large matrix size (%dx%d), only a portion of %s is displayed.\n", N, N, name);
      }
    printf("\n");
}

int main(int argc, char **argv) {
    int rank, size, N, block_size;
    int *A = NULL, *B = NULL, *C = NULL, *local_A = NULL, *local_B = NULL, *local_C = NULL;
    MPI_Comm cart_comm;
    MPI_Status status;

    // Initializing MPI environment
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Reading input matrices on rank 0
    if (rank == 0) {
        read_matrix("matrix_A.txt", &A, &N);
        read_matrix("matrix_B.txt", &B, &N);
    }

    // Broadcasting matrix size N to all processes
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Checking if the number of processes is a perfect square since cannon algorithm is distributed as a grid.
    int sq_size = sqrt(size);
    if (sq_size * sq_size != size || N % sq_size != 0) {
        if (rank == 0) printf("Number of processes must be a perfect square and N must be divisible by sqrt(p) to execute and distribute work for cannon algorithm.\n");
        MPI_Finalize();
        return 1;
    }
    
    // Checking if the matrix size and number of processes given are correct, N * N = p. To get correct multiplication result at the end.
    if (N * N != size)
    {
      if (rank == 0) 
      {
        printf("Error: The number of processes must be equal to matrix size (N*N = P) to get correct multiplication result at the end.\n");
        printf("Current values: N = %d, P = %d.\n", N, size);
      }
      
      MPI_Finalize();
      return 1;
    }
    
    // Printing the given input matrices in console
    if (rank == 0)
    {
      print_matrix("Matrix A", A, N);
      print_matrix("Matrix B", B, N);
    }

    block_size = N / sq_size;

    // Allocating memory for local matrices
    local_A = (int*)malloc(block_size * block_size * sizeof(int));
    local_B = (int*)malloc(block_size * block_size * sizeof(int));
    local_C = (int*)calloc(block_size * block_size, sizeof(int));

    // Creating 2D Cartesian communicator
    int dims[2] = {sq_size, sq_size};
    int periods[2] = {1, 1};
    int coords[2];
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 0, &cart_comm);
    MPI_Cart_coords(cart_comm, rank, 2, coords);

    // Creating MPI datatype for matrix block
    MPI_Datatype block_type;
    MPI_Type_vector(block_size, block_size, N, MPI_INT, &block_type);
    MPI_Type_commit(&block_type);

    // Distributing initial matrices
    if (rank == 0) {
        for (int i = 0; i < sq_size; i++) {
            for (int j = 0; j < sq_size; j++) {
                MPI_Send(&A[IDX(i*block_size, j*block_size, N)], 1, block_type, i*sq_size+j, 0, cart_comm);
                MPI_Send(&B[IDX(i*block_size, j*block_size, N)], 1, block_type, i*sq_size+j, 1, cart_comm);
            }
        }
    }
    MPI_Recv(local_A, block_size * block_size, MPI_INT, 0, 0, cart_comm, MPI_STATUS_IGNORE);
    MPI_Recv(local_B, block_size * block_size, MPI_INT, 0, 1, cart_comm, MPI_STATUS_IGNORE);

    // Initial alignment of matrices
    int shift_src, shift_dst;
    MPI_Cart_shift(cart_comm, 1, -coords[0], &shift_src, &shift_dst);
    MPI_Sendrecv_replace(local_A, block_size * block_size, MPI_INT, shift_dst, 0, shift_src, 0, cart_comm, &status);

    MPI_Cart_shift(cart_comm, 0, -coords[1], &shift_src, &shift_dst);
    MPI_Sendrecv_replace(local_B, block_size * block_size, MPI_INT, shift_dst, 0, shift_src, 0, cart_comm, &status);

    for (int i = 0; i < sq_size; i++) {
        // Multiplying local blocks
        for (int j = 0; j < block_size; j++) {
            for (int k = 0; k < block_size; k++) {
                for (int l = 0; l < block_size; l++) {
                    local_C[IDX(j, k, block_size)] += local_A[IDX(j, l, block_size)] * local_B[IDX(l, k, block_size)];
                }
            }
        }

        // Shifting matrix A left by one
        MPI_Cart_shift(cart_comm, 1, -1, &shift_src, &shift_dst);
        MPI_Sendrecv_replace(local_A, block_size * block_size, MPI_INT, shift_dst, 0, shift_src, 0, cart_comm, &status);

        // Shifting matrix B up by one
        MPI_Cart_shift(cart_comm, 0, -1, &shift_src, &shift_dst);
        MPI_Sendrecv_replace(local_B, block_size * block_size, MPI_INT, shift_dst, 0, shift_src, 0, cart_comm, &status);
    }

    // Gathering final multiplied results
    if (rank == 0) {
        C = (int*)malloc(N * N * sizeof(int));
    }
    MPI_Gather(local_C, block_size * block_size, MPI_INT, C, block_size * block_size, MPI_INT, 0, cart_comm);

    // Printing multiplied final result
    if (rank == 0) {
        printf("Result Matrix C:\n");
        print_matrix("Matrix C", C, N);
        free(A);
        free(B);
        free(C);
    }

    // Cleaning up space
    free(local_A);
    free(local_B);
    free(local_C);
    MPI_Type_free(&block_type);
    MPI_Finalize();
    return 0;
}