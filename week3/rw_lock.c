#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// --- Readers-Writers Lock State ---
// The coordination state belongs to the shared resource.
pthread_mutex_t rw_mutex;
pthread_cond_t read_cv;
pthread_cond_t write_cv;

int active_readers = 0;
int active_writers = 0;
int waiting_writers = 0;

void rw_init(void) {
    pthread_mutex_init(&rw_mutex, NULL);
    pthread_cond_init(&read_cv, NULL);
    pthread_cond_init(&write_cv, NULL);
    active_readers = 0;
    active_writers = 0;
    waiting_writers = 0;
}

void reader_lock(void) {
    pthread_mutex_lock(&rw_mutex);
    
    // Wait if there is an active writer OR if there are writers waiting.
   
    while (active_writers > 0 || waiting_writers > 0) {
        pthread_cond_wait(&read_cv, &rw_mutex);
    }
    
    active_readers++;
    pthread_mutex_unlock(&rw_mutex);
}

void reader_unlock(void) {
    pthread_mutex_lock(&rw_mutex);
    active_readers--;
    
    // If this was the last reader, wake up a waiting writer
    if (active_readers == 0) {
        pthread_cond_signal(&write_cv);
    }
    
    pthread_mutex_unlock(&rw_mutex);
}

void writer_lock(void) {
    pthread_mutex_lock(&rw_mutex);
    waiting_writers++;
    
    // Wait if there are ANY active readers or an active writer
    while (active_readers > 0 || active_writers > 0) {
        pthread_cond_wait(&write_cv, &rw_mutex);
    }
    
    waiting_writers--;
    active_writers++;
    pthread_mutex_unlock(&rw_mutex);
}

void writer_unlock(void) {
    pthread_mutex_lock(&rw_mutex);
    active_writers--;
    
    // Wake up other waiting threads. 
    // Signal writers first (preference), otherwise broadcast to all readers.
    if (waiting_writers > 0) {
        pthread_cond_signal(&write_cv);
    } else {
        pthread_cond_broadcast(&read_cv);
    }
    
    pthread_mutex_unlock(&rw_mutex);
}

void rw_destroy(void) {
    pthread_mutex_destroy(&rw_mutex);
    pthread_cond_destroy(&read_cv);
    pthread_cond_destroy(&write_cv);
}

// --- Stress Test Implementation ---

// Shared resource for testing
int shared_data = 0;

void *reader_thread(void *arg) {
    int id = *((int *)arg);
    for (int i = 0; i < 3; i++) {
        reader_lock();
        printf("[Reader %d] enters. Shared data = %d. Active readers = %d\n", id, shared_data, active_readers);
        usleep(50000); // Simulate reading time
        printf("[Reader %d] leaves.\n", id);
        reader_unlock();
        usleep(10000); // Time between reads
    }
    return NULL;
}

void *writer_thread(void *arg) {
    int id = *((int *)arg);
    for (int i = 0; i < 2; i++) {
        writer_lock();
        printf(">>> [Writer %d] enters. Modifying data...\n", id);
        shared_data += 10;
        usleep(80000); // Simulate writing time
        printf(">>> [Writer %d] leaves. New data = %d\n", id, shared_data);
        writer_unlock();
        usleep(20000); // Time between writes
    }
    return NULL;
}
int main() {
    printf("--- Testing Readers-Writers Lock ---\n");
    rw_init();

    pthread_t readers[4], writers[2];
    int r_ids[4] = {1, 2, 3, 4};
    int w_ids[2] = {1, 2};

    // Spawn readers and writers
    for (int i = 0; i < 4; i++) pthread_create(&readers[i], NULL, reader_thread, &r_ids[i]);
    for (int i = 0; i < 2; i++) pthread_create(&writers[i], NULL, writer_thread, &w_ids[i]);

    // Join threads
    for (int i = 0; i < 4; i++) pthread_join(readers[i], NULL);
    for (int i = 0; i < 2; i++) pthread_join(writers[i], NULL);

    rw_destroy();
    printf("--- Execution Complete ---\n");
    return 0;

}    