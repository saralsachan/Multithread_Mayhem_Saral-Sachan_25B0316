
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

/* ---- settings you can change ---- */
#define CAPACITY      5      /* how many items fit in the buffer at once  */
#define NUM_PRODUCERS 3      /* the problem needs at least 3              */
#define NUM_CONSUMERS 2      /* the problem needs at least 2              */
#define PER_PRODUCER  1000   /* how many items each producer makes        */
#define STOP          (-1)   /* a fake "item" meaning: no more items left */

/* ---- the shared buffer (one copy, shared by all threads) ---- */
int *buffer;        /* the array that holds the items                    */
int  capacity;      /* its size                                          */
int  count = 0;     /* how many items are in the buffer right now         */
int  in    = 0;     /* index where the NEXT put will write                */
int  out   = 0;     /* index where the NEXT get will read                 */

pthread_mutex_t mutex;     /* the lock that protects everything above     */
pthread_cond_t  can_put;   /* producers wait here when the buffer is full  */
pthread_cond_t  can_get;   /* consumers wait here when the buffer is empty */

/* ---- the buffer operations ---- */

void buffer_init(int cap) {
    capacity = cap;
    buffer   = malloc(sizeof(int) * cap);   /* make room for `cap` ints  */
    count = 0;
    in    = 0;
    out   = 0;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&can_put, NULL);
    pthread_cond_init(&can_get, NULL);
}

void buffer_put(int item) {
    pthread_mutex_lock(&mutex);                 /* take the lock          */
    while (count == capacity)                   /* full? then wait        */
        pthread_cond_wait(&can_put, &mutex);

    buffer[in] = item;                          /* store the item         */
    in = (in + 1) % capacity;                   /* move to the next slot  */
    count++;                                    /* one more item inside   */

    pthread_cond_signal(&can_get);              /* wake one waiting reader */
    pthread_mutex_unlock(&mutex);               /* release the lock        */
}

int buffer_get(void) {
    pthread_mutex_lock(&mutex);                 /* take the lock          */
    while (count == 0)                          /* empty? then wait       */
        pthread_cond_wait(&can_get, &mutex);

    int item = buffer[out];                     /* read the item          */
    out = (out + 1) % capacity;                 /* move to the next slot  */
    count--;                                    /* one fewer item inside  */

    pthread_cond_signal(&can_put);              /* wake one waiting writer */
    pthread_mutex_unlock(&mutex);               /* release the lock        */
    return item;
}

void buffer_destroy(void) {
    free(buffer);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&can_put);
    pthread_cond_destroy(&can_get);
}

/*  the test  */

/* each consumer writes its results into its own slot (no clashing) */
long      consumed_count[NUM_CONSUMERS];
long long consumed_sum[NUM_CONSUMERS];

int prod_id[NUM_PRODUCERS];   /* holds the number we pass to each producer */
int cons_id[NUM_CONSUMERS];   /* holds the index we pass to each consumer  */

void *producer(void *arg) {
    int n     = *(int *)arg;     /* this producer's number: 1, 2, or 3     */
    int start = n * 1000;        /* producer 1 -> 1000, producer 2 -> 2000 */
    for (int i = 0; i < PER_PRODUCER; i++)
        buffer_put(start + i);   /* produces start .. start+999            */
    return NULL;
}

void *consumer(void *arg) {
    int id = *(int *)arg;        /* this consumer's slot: 0 or 1           */
    consumed_count[id] = 0;
    consumed_sum[id]   = 0;
    while (1) {
        int item = buffer_get();
        if (item == STOP)        /* our signal to stop                     */
            break;
        consumed_count[id]++;
        consumed_sum[id] += item;
    }
    return NULL;
}

int main(void) {
    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];

    buffer_init(CAPACITY);

    /* start the consumers (they just wait quietly until items appear) */
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        cons_id[i] = i;
        pthread_create(&consumers[i], NULL, consumer, &cons_id[i]);
    }

    /* start the producers (numbered 1, 2, 3) */
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        prod_id[i] = i + 1;
        pthread_create(&producers[i], NULL, producer, &prod_id[i]);
    }

    /* wait until all producers have finished making items */
    for (int i = 0; i < NUM_PRODUCERS; i++)
        pthread_join(producers[i], NULL);

    /* now put one STOP per consumer so each one can quit */
    for (int i = 0; i < NUM_CONSUMERS; i++)
        buffer_put(STOP);

    /* wait until all consumers have finished */
    for (int i = 0; i < NUM_CONSUMERS; i++)
        pthread_join(consumers[i], NULL);

    /* ---- check the results ---- */

    /* what SHOULD have been produced (re-derive it the same way) */
    long      produced_count = 0;
    long long produced_sum   = 0;
    for (int n = 1; n <= NUM_PRODUCERS; n++)
        for (int i = 0; i < PER_PRODUCER; i++) {
            produced_count++;
            produced_sum += n * 1000 + i;
        }

    /* what the consumers actually took */
    long      total_count = 0;
    long long total_sum   = 0;
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        printf("consumer %d took %ld items, sum %lld\n",
               i, consumed_count[i], consumed_sum[i]);
        total_count += consumed_count[i];
        total_sum   += consumed_sum[i];
    }

    printf("\nproduced: %ld items, sum %lld\n", produced_count, produced_sum);
    printf("consumed: %ld items, sum %lld\n", total_count, total_sum);

    if (total_count == produced_count && total_sum == produced_sum)
        printf("\nPASS: everything matches.\n");
    else
        printf("\nFAIL: something is wrong.\n");

    buffer_destroy();
    return 0;
}
