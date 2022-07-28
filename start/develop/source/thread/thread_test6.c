#include <stdio.h>
#include "thread.h"

void * thread_a_entry (void * arg) {
    for (int i = 0; i < 1; i++) {
        printf("hello, %s\n", (char *)arg);
    }

    return (void *)1;
}

void *  thread_b_entry (void * arg) {
    for (int i = 0; i < 2; i++) {
        printf("hello, %s\n", (char *)arg);
    }

    return (void *)2;
}

void *  thread_c_entry (void * arg) {
    for (int i = 0; i < 3; i++) {
        printf("hello, %s\n", (char *)arg);
    }

    return (void *)3;
}

void thread_test6 (void) {
    thread_init();

    thread_t * thread_a = thread_create("Thread A", NULL, thread_a_entry, "Thread A");
    thread_t * thread_b = thread_create("Thread B", NULL, thread_b_entry, "Thread B");
    thread_t * thread_c = thread_create("Thread C", NULL, thread_c_entry, "Thread C");
    thread_start(thread_a);
    thread_start(thread_b);
    thread_start(thread_c);

    void * message;
    thread_join(thread_a, &message);
    printf("thread a exit: %d\n", (int)message);
    thread_join(thread_b, &message);
    printf("thread a exit: %d\n", (int)message);
    thread_join(thread_c, &message);
    printf("thread a exit: %d\n", (int)message);

    for (;;) {
        thread_yield();
    }
}
