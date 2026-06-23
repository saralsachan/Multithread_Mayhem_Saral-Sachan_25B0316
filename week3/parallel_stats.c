#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 4
#define ARRAY_SIZE 1000000

// Shared dataset
int data[ARRAY_SIZE];

// The shared structure holding inputs and the single partial result
typedef struct {
    int start;
    int end;
    long sum;
} worker_result_t;

// The thread routine
void *worker(void *arg) {
    // Cast the argument back to our struct type
    worker_result_t *r = (worker_result_t *)arg;
    
    // Initialize partial sum
    r->sum = 0;
    
    // Compute partial sum for this thread's specific slice
    for (int i = r->start; i < r->end; i++) {
        r->sum += data[i];
    }
    
    // Using the shared structure strategy, we safely return NULL
    return NULL;
}

int main() {
    // 1. Initialize the dataset with dummy values
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (i % 100) + 1; // Fills array with values from 1 to 100
    }

    pthread_t threads[NUM_THREADS];
    worker_result_t results[NUM_THREADS]; // Array allocated on main thread's stack

    int chunk_size = ARRAY_SIZE / NUM_THREADS;

    // 2. Create threads and assign them their slices
    for (int i = 0; i < NUM_THREADS; i++) {
        results[i].start = i * chunk_size;
        
        // Ensure the last thread covers any remaining elements
        results[i].end = (i == NUM_THREADS - 1) ? ARRAY_SIZE : (i + 1) * chunk_size;
        
        pthread_create(&threads[i], NULL, worker, &results[i]);
    }

    // 3. Join threads and combine the results
    long total_sum = 0;

    for (int i = 0; i < NUM_THREADS; i++) {
        // Wait for the worker to finish
        pthread_join(threads[i], NULL); 
        
        // Aggregate the partial sum
        total_sum += results[i].sum;
    }

    // 4. Print final statistics
    printf("--- Parallel Statistics (Threads: %d) ---\n", NUM_THREADS);
    printf("Total Elements: %d\n", ARRAY_SIZE);
    printf("Sum: %ld\n", total_sum);

    return 0;
}