#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Pavithra Purushothaman - [FD Number fd0001571]
// Used machine 115949 in lab
// Executed using EduMPI cluster

#define RANDOM_ARRAY_SIZE 100L // Defining the array size globally here from 100 to 1000000000

int main()
{
    long *randomArray = (long *)malloc(RANDOM_ARRAY_SIZE * sizeof(long));
    long sum = 0;

    srand(time(NULL)); // Seeding to generate random numbers

    for (long i = 0; i < RANDOM_ARRAY_SIZE; i++)
    {
        randomArray[i] = rand() % 10; // Storing random numbers between 0 and 9 for sum calculation
    }
    
    clock_t start_time = clock(); // Start time of the program

    for (long i = 0; i < RANDOM_ARRAY_SIZE; i++)
    {
        sum = sum + randomArray[i]; // Calculating the sum serially
    }

    clock_t end_time = clock(); // End time of the program

    double execution_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC; // Calculating execution time in seconds for printing

    // Final result will be printed from the sum variable along with array size and execution time
    printf("Serial Approach-> Array Size: %ld, Execution Time: %f seconds, Sum: %ld\n", RANDOM_ARRAY_SIZE, execution_time, sum);

    free(randomArray); // Freeing the allocated memory to avoid overhead

    return 0; // Program executed successfully.
}
