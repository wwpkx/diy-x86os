#include <stdlib.h>
#include "semaphore.h"
#include "thread.h"

/**
 * @brief 初始化信号量
 */
semaphore_t * semaphore_create (int init_count) {
	semaphore_t * sem = (semaphore_t *)malloc(sizeof(semaphore_t));
	if (!sem) {
        return (semaphore_t *)0;
    }

    sem->count = init_count;
    list_init(&sem->wait_list);
	return sem;
}

/**
 * @brief 销毁信号量
 */
void semaphore_destroy(semaphore_t * sem) {
    if (!sem) {
        return;
    }

    // 移除所有等待的结点
    while (list_count(&sem->wait_list)) {
        list_node_t * node = list_remove_first(&sem->wait_list);
        thread_t * thread = list_node_parent(node, thread_t, ready_node);
        thread_unblock(thread);
    }
    free(sem);

    thread_scheduler();
}

/**
 * @brief 消耗掉一个信号量计数
 */
void semaphore_down (semaphore_t * sem) {
    if (!sem) {
        return;
    }

    if (sem->count > 0) {
        sem->count--;
    } else {
        thread_t * curr = thread_self();
        list_insert_last(&sem->wait_list, &curr->ready_node);
        thread_block();
        thread_scheduler();
    }
}

/**
 * @brief 释放一个信号量
 */
void semaphore_up (semaphore_t * sem) {
    if (!sem) {
        return;
    }

    if (list_count(&sem->wait_list) == 0) {
        sem->count++;
    } else {
        list_node_t * node = list_remove_first(&sem->wait_list);
        thread_t * thread = list_node_parent(node, thread_t, ready_node);

        // 唤醒当前进程，但不需要立即去运行
        thread_unblock(thread);
    }
}
