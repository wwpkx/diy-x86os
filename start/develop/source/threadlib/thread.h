/**
 * 系统调用接口
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>
#include "list.h"
#include "context.h"

#define THREAD_NAME_SIZE            32          // 线程名称大小
#define THREAD_DEF_PRIO             32          // 线程优先级大小

/**
 * @brief 线程属性结构
 */ 
typedef struct _thread_attr_t {
    int priority;               // 优先级
    int joinable;               // 是否可连接
}thread_attr_t;

/**
 * @brief 线程结构
 */
typedef struct _thread_t {
    context_t * context;

    enum { 
        THREAD_CREATED,
        THREAD_RUNNING,
        THREAD_READY,
        THREAD_BLOCKED,
        THREAD_TERMINATED,
    }state;

    thread_func_t entry;
    void * arg;

    int joinable;
    int priority;
    char name[THREAD_NAME_SIZE];
    int ticks;
    int thread_ticks;
    void * ret;
    struct _thread_t * join_thread;
    list_node_t ready_node;
}thread_t;

/**
 * @brief 调试器结构
 */
typedef struct _scheduler_t {
    int total_ticks;                // 总时间计数
    thread_t *main_thread;          // 主线程
    thread_t *running_thread;       // 当前运行的线程
    thread_t *idle_thread;          // 空闲线程
    list_t ready_list;              // 就绪队列
    list_t recyle_list;             // 回收队列
}scheduler_t;

void thread_init (void);
void thread_entry (void);

thread_t * thread_create (char *name, thread_attr_t * attr, thread_func_t entry, void * arg);
void thread_start(thread_t * thread);
void thread_deatch(thread_t * thread);
void thread_exit (void * ret);
void thread_yield (void);
void thread_block (void);
void thread_unblock (thread_t * thread);
void thread_scheduler (void);
thread_t * thread_self (void);
int thread_join (thread_t * join_thread, void ** ret);

#endif // THREAD_H
