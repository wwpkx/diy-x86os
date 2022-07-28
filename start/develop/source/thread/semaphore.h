#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include "list.h"

/**
 * @brief 计数信号量
 */
typedef struct _semaphore_t {
	int count;					// 计数器
	list_t wait_list;			// 等待队列
}semaphore_t;

semaphore_t * semaphore_create(int init_count);
void semaphore_destroy(semaphore_t * sem);
void semaphore_down (semaphore_t * sem);
void semaphore_up (semaphore_t * sem);


#endif // SEMAPHORE_H