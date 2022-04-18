/**
 * 计数信号量
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <core/cpu.h>
#include <core/irq.h>
#include <core/task.h>
#include <ipc/sem.h>

/**
 * 信号量初始化
 */
void sem_init (sem_t * sem, int init_count) {
    sem->count = init_count;
    list_init(&sem->wait_list);
}

/**
 * 申请信号量
 */
void sem_wait (sem_t * sem) {
    irq_state_t  irq_state = irq_enter_protection();

    if (sem->count > 0) {
        sem->count--;
    } else {
        // 无可用信号，则等待
        task_t * curr = task_current();
        task_remove_ready(curr);
        list_insert_last(&sem->wait_list, &curr->node);
        task_dispatch();
    }

    irq_leave_protection(irq_state);
}

/**
 * 释放信号量
 */
void sem_notify (sem_t * sem) {
    irq_state_t  irq_state = irq_enter_protection();

    if (list_count(&sem->wait_list)) {
        // 有进程等待，则唤醒
        list_node_t * node = list_remove_first(&sem->wait_list);
        task_t * task = node_to_parent(node, task_t, node);
        task_add_ready(task);
        task_dispatch();
    } else {
        sem->count++;
    }

    irq_leave_protection(irq_state);
}

/**
 * 获取信号量的当前值
 */
int sem_count (sem_t * sem) {
    irq_state_t  irq_state = irq_enter_protection();
    int count = sem->count;
    irq_leave_protection(irq_state);
    return count;
}

