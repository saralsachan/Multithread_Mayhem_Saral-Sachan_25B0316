/*
 * race_amplified.c
 *
 * Take the same race as 02_race_counter.c and make it manifest
 * on (nearly) every iteration instead of occasionally.
 *
 * Trick: manually split counter++ into LOAD, ADD, STORE and
 * insert usleep() between LOAD and STORE. The sleep yields the
 * CPU, so the kernel reliably runs the other thread in between.
 * Both threads end up loading the same value, both add 1, both
 * store the same result — losing one increment every iteration.
 *
 * Compile:
 *   gcc -Wall -pthread race_amplified.c -o race_amplified
 */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>     /* usleep lives here */

#define ITERS 100000    /* dropped from 10M because each iter is now slow */

long counter = 0;

void *increment(void *arg) {
    (void)arg;          /* unused; pthread signature requires it */
    for (long i = 0; i < ITERS; i++) {
        long tmp = counter;     /* LOAD  — read the shared value */
        usleep(1);              /* widen the race window */
        tmp = tmp + 1;          /* ADD   — compute new value */
        counter = tmp;          /* STORE — write back */
    }
    return NULL;
}

int main(void) {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, increment, NULL);
    pthread_create(&t2, NULL, increment, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    long expected = 2L * ITERS;
    printf("Final counter:  %ld\n", counter);
    printf("Expected:       %ld\n", expected);
    printf("Difference:     %ld\n", expected - counter);

    if (counter == expected) {
        printf("Result: CORRECT (you got lucky — try again)\n");
    } else if (counter <= ITERS + ITERS/10) {
        printf("Result: HEAVILY STOMPED — counter is near ITERS, not 2*ITERS.\n");
    } else {
        printf("Result: WRONG by %ld\n", expected - counter);
    }

    return 0;
}