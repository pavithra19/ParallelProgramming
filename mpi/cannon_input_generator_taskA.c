// input_generator.c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* This code is written by
Pavithra Purushothaman (fd0001571)
Matrikel Nummer: 1535949
*/

#define N 4

int main() {

    /* For debugging
    int N;
    printf("Enter the matrix size N: ");
    scanf("%d", &N); // Getting matrix size N for the first time
    */

    srand(time(NULL));

    // Creating files with write access to store the matrices
    FILE *fileA = fopen("matrix_A.txt", "w");
    FILE *fileB = fopen("matrix_B.txt", "w");

    if (fileA == NULL || fileB == NULL) {
        printf("Error opening files.\n");
        return 1;
    }

    // Writing N to the first line of each file so the respective matrix algorithm can read if required.
    fprintf(fileA, "%d\n", N);
    fprintf(fileB, "%d\n", N);

    // Generating random numbers between 0 to 9 and writing them as matrices in files
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            fprintf(fileA, "%d ", rand() % 10);
            fprintf(fileB, "%d ", rand() % 10);
        }
        fprintf(fileA, "\n");
        fprintf(fileB, "\n");
    }

    // Files closed once the input matrices are saved
    fclose(fileA);
    fclose(fileB);

    printf("Matrices generated and saved to files.\n");
    return 0;
}