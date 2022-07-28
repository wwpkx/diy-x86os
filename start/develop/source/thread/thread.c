/**
 * 系统调用接口
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
//#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include<sys/time.h>
#include "thread.h"

static scheduler_t scheduler;

/**
 * @brief 清理用户线程
 */
void thread_exit (void * ret) {
    thread_t * self = thread_self();
    if (self == scheduler.main_thread) {
        // 主进程退出，直接退出进程
        // exit(ret);
        return;
    }

    // 插入待删除队列，等待空闲线程释放自己
    self->state = THREAD_TERMINATED;
    self->ret = ret;
    if (!self->joinable) {
        list_insert_last(&scheduler.recyle_list, &self->ready_node);
    } else if (self->join_thread) {
        thread_unblock(self->join_thread);
    }

    // 获取下一结点
    list_node_t * node = list_remove_first(&scheduler.ready_list);
    thread_t * next = list_node_parent(node, thread_t, ready_node);
    next->state = THREAD_RUNNING;
    scheduler.running_thread = next;

    // 切换至下一结点运行，无需恢复当前结构
    context_set(next->context);
}

/**
 * @brief 进行线程调试
 */
void thread_scheduler (void) {
    thread_t * self = scheduler.running_thread;
    if (self->state == THREAD_RUNNING) {
        // 当前正在运行，移入就绪队列中
        list_insert_last(&scheduler.ready_list, &self->ready_node);
        self->ticks = self->priority;
        self->state = THREAD_RUNNING;
    }

    // 取出新进程
    list_node_t * node = list_remove_first(&scheduler.ready_list);
    thread_t * next = list_node_parent(node, thread_t, ready_node);
    //assert(next->state == THREAD_READY);
    if (next != self) {
        next->state = THREAD_RUNNING;
        scheduler.running_thread = next;

        // 进程切换
        context_swap(self->context, next->context);
    }
}

/**
 * @brief 当前线程主动放弃CPU
 */
void thread_yield (void) {
    thread_t * self = scheduler.running_thread;

    // 没有其它线程，退出
    if (list_count(&scheduler.ready_list) == 0) {
        return;
    }

    // 当前进程进入队列等待
    list_insert_last(&scheduler.ready_list, &self->ready_node);
    self->ticks = self->priority;
    self->state = THREAD_READY;
    
    thread_scheduler();
}

/**
 * @brief 释放线程
 */
static void thread_free(thread_t * thread) {
    context_free(thread->context);
    free(thread);
}

/**
 * @brief 空闲线程函数
 */
static void thread_idle_entry (void * arg) {
    for (;;) {
        // 遍历回收队列，释放所有资源
        while (list_count(&scheduler.recyle_list)) {
            list_node_t * node = list_remove_first(&scheduler.recyle_list);
            thread_t * thread = list_node_parent(node, thread_t, ready_node);
            thread_free(thread);
        }

        // 如果有用户线程，则运行；否则当前进程放弃运行机会
        if (list_count(&scheduler.ready_list)) {
            thread_yield();
        } else {
            // 进程切换放弃
        }
    }
}

void sig_alarm_handler (int signum) {
#ifdef DEBUG
    printf("signal alarm");
#endif

    thread_t * current = thread_self();
    if (--current->ticks == 0) {
        thread_scheduler();
        current->ticks = current->priority;
    }
}

/**
 * @brief 初始化线程库
 */
void thread_init (void) {
    scheduler.total_ticks = 0;
    list_init(&scheduler.ready_list);
    list_init(&scheduler.recyle_list);

    // 创建主线程，无需再启动，因为当前已经启动；而且不应当设置栈？？？
    // todo:主进程的栈设置有问题？
    scheduler.main_thread = thread_create(
            "Main Thread", 
            NULL, 
            (thread_func_t)0,
            (void *)0);
    scheduler.running_thread = scheduler.main_thread;
    //assert(scheduler.running_thread != (thread_t *)0);

    // 创建空闲线程并启动
    scheduler.idle_thread = thread_create(
            "Idle Thread",
            NULL,
            (thread_func_t)thread_idle_entry,
            (void *)0
    );
    //assert(scheduler.idle_thread != (thread_t *)0);
    thread_start(scheduler.idle_thread);

    struct itimerval timer; 
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 10;
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 10;
    if (setitimer(ITIMER_PROF, &timer, NULL) < 0) {
        fprintf(stderr, "set timer failed.");
        return;
    }
    signal(SIGALRM, sig_alarm_handler);
}

/**
 * @brief 所有线程的入口代码
 */
void thread_entry (void) {
    thread_t * self = thread_self();

    void * ret = self->entry(self->arg);
    thread_exit(ret);
}

/**
 * @brief 创建一个线程但不启动运行
 */
thread_t * thread_create (char *name, thread_attr_t * attr, thread_func_t entry, void * arg) {
    // 分配空间
    thread_t * thread = (thread_t *)calloc(1, sizeof(thread_t));
    if (!thread) {
        fprintf(stderr, "Failed allocate memory for thread_t\n");
        return (thread_t *)0;
    }

    thread->context = context_create(entry ? THREAD_STACK_DEF_SIZE : 0);
    if (thread->context == (context_t *)0) {
        free(thread);
        fprintf(stderr, "Failed allocate memory for context\n");
        return (thread_t *)0;
    }

    strncpy(thread->name, name, THREAD_NAME_SIZE);
    thread->entry = entry;
    thread->arg = arg;
    thread->state = THREAD_CREATED;

    if (attr) {
        thread->joinable = attr->joinable;
        thread->priority = attr->priority;
    } else {
        thread->joinable = 1;
        thread->priority = THREAD_DEF_PRIO;
    }

    thread->ticks = thread->priority;
    thread->thread_ticks = 0;
    list_node_init(&thread->ready_node);
    return thread;
}

/**
 * @brief 设置线程为分离状态
 */
void thread_deatch(thread_t * thread) {
    thread->joinable = 0;
    thread->join_thread = (thread_t *)0;
}

/**
 * @brief 启动线程的运行
 */
void thread_start(thread_t * thread) {
    list_insert_last(&scheduler.ready_list, &thread->ready_node);
}

/**
 * @brief 获取当前正在运行的线程
 */
thread_t * thread_self (void) {
    return scheduler.running_thread;
}

/**
 * @brief 挂起当前线程
 */
void thread_block (void) {
    thread_t * self = thread_self();
    self->state = THREAD_BLOCKED;
}

/**
 * @brief 唤醒指定的线程
 */
void thread_unblock (thread_t * thread) {
    //assert(thread->state == THREAD_BLOCKED);

    thread->state = THREAD_READY;
    list_insert_last(&scheduler.ready_list, &thread->ready_node);
}

/**
 * @brief 等待指定进程退出或返回
 */
int thread_join (thread_t * thread, void ** ret) {
    thread_t * self = thread_self();

    // 检查线程是否可连接
    if (!thread->joinable) {
        return -1;
    }

    // 如果已经退出，则取返回值后立即退出
    if (thread->state == THREAD_TERMINATED) {
        *ret = thread->ret;
        thread_free(thread);
        return 0;
    }

    // 如果已经有线程在等待，则直接退出
    if (!thread->join_thread) {
        thread->join_thread = self;
        thread_block();
        thread_scheduler();

        // 线程结束，返回
        *ret = thread->ret;
        thread_free(thread);
        return 0;
    }

    return -1;
}
