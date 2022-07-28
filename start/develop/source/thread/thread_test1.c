#include <stdio.h>
#include "thread.h"

void * thread_a_entry (void * arg) {
    for (;;) {
        printf("hello, %s\n", (char *)arg);
    }
    return (void *)0;
}

void thread_test1 (void) {
    thread_init();

    thread_t * thread_a = thread_create("Thread A", NULL, thread_a_entry, "Thread A");
    thread_start(thread_a);

    for (;;) {
        thread_yield();
    }
}
