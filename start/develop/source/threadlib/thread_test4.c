#include <stdio.h>
#include "thread.h"
#include "semaphore.h"

static semaphore_t * sem_a, * sem_b;
static int count = 0;

void * thread_b_entry (void * arg) {
    for (;;) {
        printf("hello, %s: count = %d\n", (char *)arg, count++);
        semaphore_up(sem_a);

        semaphore_down(sem_b);
    }

    return (void *)0;
}

void * thread_a_entry (void * arg) {
    sem_a = semaphore_create(0);
    sem_b = semaphore_create(0);
    
    thread_t * thread_b = thread_create("Thread B", NULL, thread_b_entry, "Thread B");
    thread_start(thread_b);
    for (;;) {
        semaphore_down(sem_a);
        printf("hello, %s: count = %d\n", (char *)arg, count++);
        semaphore_up(sem_b);
    }

    return (void *)0;
}

void thread_test4 (void) {
    thread_init();

    thread_t * thread_a = thread_create("Thread A", NULL, thread_a_entry, "Thread A");
    thread_start(thread_a);
    for (;;) {
        thread_yield();
    }
}
