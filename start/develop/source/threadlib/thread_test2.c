#include <stdio.h>
#include "thread.h"

void *  thread_a_entry (void * arg) {
    for (int i = 0; i < 10; i++) {
        printf("hello, %s: %d\n", (char *)arg, i);
    }

    return (void *)0;
}

void thread_test2 (void) {
    thread_init();

    thread_t * thread_a = thread_create("Thread A", NULL, thread_a_entry, "Thread A");
    thread_start(thread_a);

    thread_yield();
    printf("test end\n");
    for (;;) {
        thread_yield();
    }
}
