#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <math.h>
#include <stdbool.h>

// Pavithra Purushothaman - Merge Sort [FD Number fd0001571]
// Logeshwari Purushothaman - Bubble Sort [FD fd0001575]
// Spandan Basu - Quick sort [FD fd1452130]
// Used machine 115948 in lab
// Executed using EduMPI cluster

#define RANDOM_ARRAY_SIZE 100 // Array size (can be adjusted) 100 to 100000000 (Use smaller size for bubble sort)
#define MIN_THRESHOLD 10  // Define your minimum threshold
#define MAX_THRESHOLD 20 // Define your maximum threshold

void fillValuesInRandomArray(int *source);
void duplicateArrayValuesForSorting(int* source, int* destination);
void print_array_for_verification(int *arr); 
int partition(int *arr, int low, int high);
int computeThreshold(int size);

// Spandan Basu - Quick Sort [FD fd1452130]
void quicksort_serial(int *arr, int low, int high);
void quicksort_parallel(int *arr, int low, int high);
void executeQuickSortSerial(int *arr);
void executeQuickSortParallel(int *arr);

// Logeshwari Purushothaman - Bubble Sort [FD fd0001575]
void SerialBubbleSort(int* arr);
void executeSerialBubbleSort(int *arr);
void ParallelBubbleSortEvenOdd(int* arr);
void executeParallelBubbleSortEvenOdd(int *arr);

// Pavithra Purushothaman - Merge Sort [FD fd0001571]
void executeMergeSortSerial(int *arr);
void executeMergeSortParallel(int *arr);
void divideAndSortSerial(int *arr, int left, int right);
void divideAndSortParallel(int *arr, int left, int right);
void mergeSortedResult(int *arr, int left, int mid, int right);

int main()
{
    int* source_array = (int*)malloc(RANDOM_ARRAY_SIZE * sizeof(int));
    int* copy_array_instance = (int*)malloc(RANDOM_ARRAY_SIZE * sizeof(int));

    fillValuesInRandomArray(source_array);// Fill the array with random numbers

    printf("Array Size: %d\n", RANDOM_ARRAY_SIZE);
    printf("Number of Threads: %d\n", omp_get_max_threads());

    duplicateArrayValuesForSorting(source_array, copy_array_instance);
    executeQuickSortSerial(copy_array_instance);  // Serial approach for smaller arrays

    duplicateArrayValuesForSorting(source_array, copy_array_instance);
    executeQuickSortParallel(copy_array_instance); // Parallel approach for larger arrays
 
    duplicateArrayValuesForSorting(source_array, copy_array_instance);
    executeSerialBubbleSort(copy_array_instance);  // Serial approach for smaller arrays

    duplicateArrayValuesForSorting(source_array, copy_array_instance);
    executeParallelBubbleSortEvenOdd(copy_array_instance);  // Parallel approach for larger arrays

    duplicateArrayValuesForSorting(source_array, copy_array_instance);
    executeMergeSortSerial(copy_array_instance);  // Serial approach for smaller arrays
    
    duplicateArrayValuesForSorting(source_array, copy_array_instance);
    executeMergeSortParallel(copy_array_instance); // Parallel approach for larger arrays

    //print_array_for_verification(copy_array_instance);
 
    free(source_array);
    free(copy_array_instance);
    
    return 0;
}

// Function to fill the array with random numbers between 0 and 99
void fillValuesInRandomArray(int *source)
{
    srand(time(NULL));

    for (int i = 0; i < RANDOM_ARRAY_SIZE; i++) {
        int val = rand() % 100; // Values between 0 and 100, with duplication entries
        source[i] = val;
       // copy[i] = val;
    }
}

void duplicateArrayValuesForSorting(int* source, int* destination)
{
    for (int i = 0; i < RANDOM_ARRAY_SIZE; i++) {
        destination[i] = source[i];
    }
}

// Function to print the array (useful for debugging or small arrays)
void print_array_for_verification(int *arr) {
    for (int i = 0; i < RANDOM_ARRAY_SIZE; i++) {
        printf("%d \t", arr[i]);
    }
    printf("\n");
}

// Compute the threshold dynamically based on array size and available threads
int computeThreshold(int size)
{
    int num_threads = omp_get_max_threads();  // Get the number of threads available on the system
    
    // Logarithmic scaling based on size, using log() to avoid overflow for large arrays
    double log_size = log((double)size);  // cast 'size' to double for logarithmic calculation
    
    // Base threshold is inversely proportional to log(size)
    int threshold = (int)(size / log_size);  // Cast the result to int
    
    // Dynamically adjust the maximum threshold based on the number of threads
    int dynamic_max_threshold = MAX_THRESHOLD / num_threads;  // Cap threshold based on available threads

    // Clamp the threshold between the minimum and maximum allowable thresholds
    threshold = (threshold < MIN_THRESHOLD) ? MIN_THRESHOLD : threshold;
    threshold = (threshold > dynamic_max_threshold) ? dynamic_max_threshold : threshold;

    return threshold;
}


int partition(int *arr, int low, int high) {
    int pivot = arr[high];  // Last element is pivot
    int i = low - 1;

    // Reorder elements so that smaller elements are on the left and larger on the right
    for (int j = low; j < high; j++) {
        if (arr[j] <= pivot) {
            i++;
            int temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }

    // Swap pivot element with the element at i+1 to place it in the correct position
    int temp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = temp;

    return i + 1;
}

// Serial version of quicksort
void quicksort_serial(int *arr, int low, int high) {
    if (low < high) {
        int pi = partition(arr, low, high);

        quicksort_serial(arr, low, pi - 1);  // Recurse on the left partition
        quicksort_serial(arr, pi + 1, high); // Recurse on the right partition
    }
}

// Optimized Parallel version of quicksort using OpenMP with a threshold for small arrays
void quicksort_parallel(int *arr, int low, int high) {
    if (low < high) {
        // If the subarray size is small enough, use the serial quicksort
        int threshold = computeThreshold(high - low);
        
        // If the size is above the threshold, we parallelize; otherwise, use serial merge sort
        if (high - low < threshold)
        {
            quicksort_serial(arr, low, high);
            return;
        }

        int pi = partition(arr, low, high);

        // Parallelize the two recursive calls for the left and right partitions
        #pragma omp task shared(arr)
        quicksort_parallel(arr, low, pi - 1);  // Left partition
        #pragma omp task shared(arr)
        quicksort_parallel(arr, pi + 1, high); // Right partition

        #pragma omp taskwait // Ensure both tasks are completed before returning
    }
}

void executeQuickSortSerial(int *arr)
{
    double start_time = omp_get_wtime();
    quicksort_serial(arr, 0, RANDOM_ARRAY_SIZE - 1);
    double end_time = omp_get_wtime();
    printf("Serial Quicksort completed in %f seconds.\n", end_time - start_time);
}


void executeQuickSortParallel(int *arr)
{
    double start_time = omp_get_wtime();
    
    // Parallel region for quicksort
    #pragma omp parallel
    {
        #pragma omp single
        quicksort_parallel(arr, 0, RANDOM_ARRAY_SIZE - 1);
    }

    double end_time = omp_get_wtime();
    printf("Parallel Quicksort completed in %f seconds.\n", end_time - start_time);
}


// Serial Bubble Sort function
void SerialBubbleSort(int* arr) {
    for (int i = 0; i < RANDOM_ARRAY_SIZE - 1; i++) {
        bool swapped = false;
        for (int j = 0; j < RANDOM_ARRAY_SIZE - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
                swapped = true;
            }
        }
        if (!swapped) break; // Terminate early if no swaps occurred
    }
}

void executeSerialBubbleSort(int *arr)
{
    double start_time = omp_get_wtime();
    SerialBubbleSort(arr);
    double end_time = omp_get_wtime();
    printf("Serial BubbleSort completed in %f seconds.\n", end_time - start_time);
}

// Parallel Bubble Sort using OpenMP (Even-Odd Phases)
void ParallelBubbleSortEvenOdd(int* arr) {
    bool swapped;

    for (int i = 0; i < RANDOM_ARRAY_SIZE; i++) {
        swapped = false;

        // Even phase
        #pragma omp parallel for shared(arr) reduction(|:swapped)
        for (int j = 0; j < RANDOM_ARRAY_SIZE - 1; j += 2) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
                swapped = true;
            }
        }

        // Odd phase
        #pragma omp parallel for shared(arr) reduction(|:swapped)
        for (int j = 1; j < RANDOM_ARRAY_SIZE - 1; j += 2) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
                swapped = true;
            }
        }

        // If no elements were swapped, exit early
        if (!swapped) {
            break;
        }
    }
}


void executeParallelBubbleSortEvenOdd(int *arr)
{
    double start_time = omp_get_wtime();
    ParallelBubbleSortEvenOdd(arr);
    double end_time = omp_get_wtime();
    printf("Parallel BubbleSort completed in %f seconds.\n", end_time - start_time);
}

void executeMergeSortSerial(int *arr)
{
    double startTime = omp_get_wtime(); // Start time for serial execution

    // Perform Merge Sort (Serial)
    divideAndSortSerial(arr, 0, RANDOM_ARRAY_SIZE - 1);

    double endTime = omp_get_wtime(); // End time for serial execution
      printf("Serial MergeSort completed in %f seconds.\n", endTime - startTime);
}

void executeMergeSortParallel(int *arr)
{
    double startTime = omp_get_wtime(); // Start time for parallel execution

    // Perform Merge Sort (Parallel)
    divideAndSortParallel(arr, 0, RANDOM_ARRAY_SIZE - 1);

    double endTime = omp_get_wtime(); // End time for parallel execution
    printf("Parallel MergeSort completed in %f seconds.\n", endTime - startTime);
}

void divideAndSortSerial(int *arr, int left, int right)
{
    if (left < right) {
        int mid = left + (right - left) / 2;

        // Sort first and second halves
        divideAndSortSerial(arr, left, mid);
        divideAndSortSerial(arr, mid + 1, right);

        // Merge the sorted halves
        mergeSortedResult(arr, left, mid, right);
    }
}

void divideAndSortParallel(int *arr, int left, int right)
{
    if (left < right)
    {
        int mid = left + (right - left) / 2;
        
        // Parallelize the divide step with a parallel for loop
         #pragma omp task shared(arr)
        divideAndSortParallel(arr, left, mid); // Left section
        
        #pragma omp task shared(arr)
        divideAndSortParallel(arr, mid + 1, right); // Right section
        
        #pragma omp taskwait
        mergeSortedResult(arr, left, mid, right); // Merging the sorted halves (no parallelization needed)
    }
}

void mergeSortedResult(int *arr, int left, int mid, int right)
{
    int i, j, k;
    int n1 = mid - left + 1;  // Size of the left subarray
    int n2 = right - mid;     // Size of the right subarray

    // Create temporary arrays
    int *L = (int *)malloc(n1 * sizeof(int));
    int *R = (int *)malloc(n2 * sizeof(int));

    // Copy data to temporary arrays
    for (i = 0; i < n1; i++) {
        L[i] = arr[left + i];
    }
    for (j = 0; j < n2; j++) {
        R[j] = arr[mid + 1 + j];
    }

    // Merge the temporary arrays back into arr
    i = 0; // Initial index of the first subarray
    j = 0; // Initial index of the second subarray
    k = left; // Initial index of the merged subarray
    
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    // Copy remaining elements of L[], if any
    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }

    // Copy remaining elements of R[], if any
    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }

    free(L);
    free(R);
}