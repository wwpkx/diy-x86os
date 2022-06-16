/**
 * 计数信号量
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "cpu/irq.h"
#include "core/task.h"
#include "ipc/sem.h"

/**
 * 信号量初始化
 */
void sem_init (sem_t * sem, int init_count) {
    sem->count = init_count;
    list_init(&sem->wait_list);
}
