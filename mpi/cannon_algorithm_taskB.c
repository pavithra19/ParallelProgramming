#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

/* cannon_algorithm_taskB.c
This code is written by
Pavithra Purushothaman (fd0001571)
Matrikel Nummer: 1535949
Code executed in machine UV 21 - F13 at the linux lab
*/

#define N 800  // We can adjust this value as needed (800 to 8000)
typedef int MatrixType;

// Function prototypes
void fillMatrix(MatrixType *matrix, int size);
void distributeMatrices(MatrixType *A, MatrixType *B, MatrixType *localA, MatrixType *localB, int sub_n, MPI_Comm comm);
void cannon_algorithm(MatrixType *localA, MatrixType *localB, MatrixType *localC, int sub_n, int np, MPI_Comm comm);
void printMatrix(const char* name, MatrixType *matrix, int size);

int main(int argc, char **argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int np = (int)sqrt(size);
    if (np * np != size || N % np != 0) {
        if (rank == 0) {
            fprintf(stderr, "Error: Number of processes must be a perfect square, and N must be divisible by grid size.\n");
        }
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    // Allocating memory for local matrices
    int sub_n = N / np;
    MatrixType *localA = (MatrixType *)malloc(sub_n * sub_n * sizeof(MatrixType));
    MatrixType *localB = (MatrixType *)malloc(sub_n * sub_n * sizeof(MatrixType));
    MatrixType *localC = (MatrixType *)calloc(sub_n * sub_n, sizeof(MatrixType));

    // Creating 2D Cartesian communicator
    MPI_Comm gridComm;
    int dims[2] = {np, np};
    int periods[2] = {1, 1};  
    MPI_Cart_create(MPI_COMM_WORLD, 2, dims, periods, 1, &gridComm);

    MatrixType *A = NULL, *B = NULL, *C = NULL;
    if (rank == 0) {
        A = (MatrixType *)malloc(N * N * sizeof(MatrixType));
        B = (MatrixType *)malloc(N * N * sizeof(MatrixType));
        C = (MatrixType *)calloc(N * N, sizeof(MatrixType));

        fillMatrix(A, N);
        fillMatrix(B, N);

        // Printing small portion of input matrices for verification
        //printMatrix("Matrix A (top-left corner)", A, (N < 10) ? N : 10);
        //printMatrix("Matrix B (top-left corner)", B, (N < 10) ? N : 10);
    }

    double start_time = MPI_Wtime();
    
    distributeMatrices(A, B, localA, localB, sub_n, gridComm);
    cannon_algorithm(localA, localB, localC, sub_n, np, gridComm);
    
    // Gathering final multiplication results from all the ranks
    MPI_Gather(localC, sub_n * sub_n, MPI_INT, C, sub_n * sub_n, MPI_INT, 0, gridComm);

    double end_time = MPI_Wtime();

    if (rank == 0) {
        printf("Execution Time: %f seconds\n", end_time - start_time);
        
        // Printing small portion of output matrix for verification
        //printMatrix("Result Matrix C (top-left corner)", C, (N < 10) ? N : 10);
        printf("Result Matrix C (top-left corner): %d\n", C[0]);

        free(A);
        free(B);
        free(C);
    }

    free(localA);
    free(localB);
    free(localC);

    MPI_Finalize();
    return 0;
}

void fillMatrix(MatrixType *matrix, int size) {
    for (int i = 0; i < size * size; i++) {
        matrix[i] = rand() % 10;
    }
}

// This method distributes the matix blocks to all the ranks
void distributeMatrices(MatrixType *A, MatrixType *B, MatrixType *localA, MatrixType *localB, int sub_n, MPI_Comm comm) {
    int rank, np;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &np);
    np = sqrt(np);

    int coords[2];
    MPI_Cart_coords(comm, rank, 2, coords);

    // Creating MPI datatypes for scattering the input matrices as blocks
    MPI_Datatype block, blocktype;
    MPI_Type_vector(sub_n, sub_n, N, MPI_INT, &block);
    MPI_Type_create_resized(block, 0, sizeof(int), &blocktype);
    MPI_Type_commit(&blocktype);

    int *sendcounts = NULL, *displs = NULL;
    if (rank == 0) {
        sendcounts = (int*)malloc(np * np * sizeof(int));
        displs = (int*)malloc(np * np * sizeof(int));
        for (int i = 0; i < np; i++) {
            for (int j = 0; j < np; j++) {
                sendcounts[i * np + j] = 1;
                displs[i * np + j] = i * N * sub_n + j * sub_n;
            }
        }
    }

    // Scatter A and B matrices to all the ranks
    MPI_Scatterv(A, sendcounts, displs, blocktype, localA, sub_n * sub_n, MPI_INT, 0, comm);
    MPI_Scatterv(B, sendcounts, displs, blocktype, localB, sub_n * sub_n, MPI_INT, 0, comm);

    if (rank == 0) {
        free(sendcounts);
        free(displs);
    }

    MPI_Type_free(&blocktype);

    // Initial alignment for Cannon's algorithm
    int shift_src, shift_dest;
    MPI_Cart_shift(comm, 1, -coords[0], &shift_src, &shift_dest);
    MPI_Sendrecv_replace(localA, sub_n * sub_n, MPI_INT, shift_dest, 0, shift_src, 0, comm, MPI_STATUS_IGNORE);

    MPI_Cart_shift(comm, 0, -coords[1], &shift_src, &shift_dest);
    MPI_Sendrecv_replace(localB, sub_n * sub_n, MPI_INT, shift_dest, 0, shift_src, 0, comm, MPI_STATUS_IGNORE);
}

// This method perform cannon's multiplication by shifting the elements
void cannon_algorithm(MatrixType *localA, MatrixType *localB, MatrixType *localC, int sub_n, int np, MPI_Comm comm) {
    int left, right, up, down;
    MPI_Cart_shift(comm, 1, -1, &left, &right); // Shifting matrix to left by one
    MPI_Cart_shift(comm, 0, -1, &up, &down); // Shifting matrix to up by one

    for (int step = 0; step < np; step++) {
        // Multiplying local matrices
        for (int i = 0; i < sub_n; i++) {
            for (int k = 0; k < sub_n; k++) {
                for (int j = 0; j < sub_n; j++) {
                    localC[i * sub_n + j] += localA[i * sub_n + k] * localB[k * sub_n + j];
                }
            }
        }

        // Shifting elements A left and B up
        MPI_Sendrecv_replace(localA, sub_n * sub_n, MPI_INT, left, 0, right, 0, comm, MPI_STATUS_IGNORE);
        MPI_Sendrecv_replace(localB, sub_n * sub_n, MPI_INT, up, 0, down, 0, comm, MPI_STATUS_IGNORE);
    }
}

void printMatrix(const char* name, MatrixType *matrix, int size) {
    printf("%s:\n", name);
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            printf("%d ", matrix[i * size + j]);
        }
        printf("\n");
    }
    printf("\n");
}

