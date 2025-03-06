#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

// Pavithra Purushothaman - [FD Number fd0001571]
// Used machine UV 21 - F9 in lab
// Executed using EduMPI cluster

#define RANDOM_ARRAY_SIZE 100L // Defining the array size globally here from 100 to 1000000000

void calculateSumParallel(long *randomArray);
void calculateSumSections(long *randomArray);
void calculateSumTask(long *randomArray);

int main()
{
    long *randomArray = (long *)malloc(RANDOM_ARRAY_SIZE * sizeof(long));
    
    srand(time(NULL)); // Seeding to generate random numbers

    for (long i = 0; i < RANDOM_ARRAY_SIZE; i++)
    {
        randomArray[i] = rand() % 10; // Storing random numbers between 0 and 9 for sum calculation
    }
    
    calculateSumParallel(randomArray);
    calculateSumSections(randomArray);
    calculateSumTask(randomArray);
    
    free(randomArray); // Freeing the allocated memory to avoid overhead

    return 0; // Program executed successfully.
}

void calculateSumParallel(long *randomArray)
{
  long sum = 0;
  double startTime = omp_get_wtime(); // Start time of the program

    // Calculating the sum in parallel by sharing sum and array over all the threads
    #pragma omp parallel shared(sum, randomArray)
    {
        // All the threads stores the total in sum variable without interferring other threads
        #pragma omp for reduction(+:sum)
        for (long i = 0; i < RANDOM_ARRAY_SIZE; i++)
        {
            sum = sum + randomArray[i];
        }
    }

    double endTime = omp_get_wtime(); // End time of the program

    // Final result will be printed from the sum variable along with array size and execution time
     printf("Parallel Approach-> Threads: %d, Array Size: %ld, Execution Time: %f seconds, Sum: %ld\n", omp_get_max_threads(), RANDOM_ARRAY_SIZE, endTime - startTime, sum);
}

void calculateSumSections(long *randomArray)
{
  long sum = 0, section1 = 0, section2 = 0;
  double startTime = omp_get_wtime(); // Start time of the program

// Calculating the sum in two sections by sharing sum and array over all the threads
  #pragma omp parallel sections shared(randomArray, section1, section2)
    {
        #pragma omp section
        {
            for (long i = 0; i < RANDOM_ARRAY_SIZE / 2; i++)
            {
                section1 += randomArray[i];
            }
        }

        #pragma omp section
        {
            for (long i = RANDOM_ARRAY_SIZE / 2; i < RANDOM_ARRAY_SIZE; i++)
            {
                section2 += randomArray[i];
            }
        }
    }

    sum = section1 + section2;
    
    double endTime = omp_get_wtime(); // End time of the program

    // Final result will be printed from the sum variable along with array size and execution time
    printf("Sections Approach-> Threads: %d, Array Size: %ld, Execution Time: %f seconds, Sum: %ld\n", omp_get_max_threads(), RANDOM_ARRAY_SIZE, endTime - startTime, sum);
}

void calculateSumTask(long *randomArray)
{
  long sum = 0;
  double startTime = omp_get_wtime(); // Start time of the program
  
    // Calculating the sum as two separate tasks by sharing sum and array over all the threads
         #pragma omp task shared(sum, randomArray)
            for (long i = 0; i < RANDOM_ARRAY_SIZE / 2; i++)
            {
                sum += randomArray[i];
            }
        
        #pragma omp task shared(sum, randomArray)
            for (long i = RANDOM_ARRAY_SIZE / 2; i < RANDOM_ARRAY_SIZE; i++)
            {
                sum += randomArray[i];
            }
        
        #pragma omp taskwait
    
    double endTime = omp_get_wtime(); // End time of the program

    // Final result will be printed from the sum variable along with array size and execution time
    printf("Task Approach-> Threads: %d, Array Size: %ld, Execution Time: %f seconds, Sum: %ld\n", omp_get_max_threads(), RANDOM_ARRAY_SIZE, endTime - startTime, sum);
}
