#include <stdio.h>
#include "thread.h"

void *  thread_a_entry (void * arg) {
    for (;;) {
        printf("hello, %s\n", (char *)arg);
        thread_yield();
    }

    return (void *)0;
}

void *  thread_b_entry (void * arg) {
    for (;;) {
        printf("hello, %s\n", (char *)arg);
        thread_yield();
    }

    return (void *)0;
}

void *  thread_c_entry (void * arg) {
    for (;;) {
        printf("hello, %s\n", (char *)arg);
        thread_yield();
    }

    return (void *)0;
}

void thread_test3 (void) {
    thread_init();

    thread_t * thread_a = thread_create("Thread A", NULL, thread_a_entry, "Thread A");
    thread_t * thread_b = thread_create("Thread B", NULL, thread_b_entry, "Thread B");
    thread_t * thread_c = thread_create("Thread C", NULL, thread_c_entry, "Thread C");
    thread_start(thread_a);
    thread_start(thread_b);
    thread_start(thread_c);

    for (;;) {
        printf("hello, %s\n", thread_self()->name);
        thread_yield();
    }
}
