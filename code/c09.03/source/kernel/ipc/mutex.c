/**
 * 互斥锁
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "cpu/irq.h"
#include "ipc/mutex.h"

/**
 * 锁初始化
 */
void mutex_init (mutex_t * mutex) {
    mutex->locked_count = 0;
    mutex->owner = (task_t *)0;
    list_init(&mutex->wait_list);
}

/**
 * 申请锁
 */
void mutex_lock (mutex_t * mutex) {
    irq_state_t  irq_state = irq_enter_protection();

    task_t * curr = task_current();
    if (mutex->locked_count == 0) {
        // 没有任务占用，占用之
        mutex->locked_count = 1;
        mutex->owner = curr;
    } else if (mutex->owner == curr) {
        // 已经为当前任务所有，只增加计数
        mutex->locked_count++;
    } else {
        // 有其它任务占用，则进入队列等待
        task_t * curr = task_current();
        task_set_block(curr);
        list_insert_last(&mutex->wait_list, &curr->wait_node);
        task_dispatch();
    }

    irq_leave_protection(irq_state);
}

