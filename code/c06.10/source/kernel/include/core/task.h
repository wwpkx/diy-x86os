/**
 * 任务结构实现
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef TASK_H
#define TASK_H

#include "comm/types.h"
#include "cpu/cpu.h"

#define TASK_NAME_SIZE          128         // 任务的名称大小
#define TASK_LDT_TABLE_SIZE     4           // 任务LDT描述符

/**
 * @brief 任务结构 
 */
typedef struct _task_t {
    // 任务状态
    enum {
        TASK_RUNNING, 
        TASK_READY,    
    }state;

    char name[TASK_NAME_SIZE];          // 任务的名称
    int priority;                       // 优先级
    int ticks;                          // 时间片计数

    int ldt_selector;
    gdt_descriptor_t ldt_table[TASK_LDT_TABLE_SIZE]; // 任务LDT描述符
    int tss_selector;                   // tss选择符
    tss_t tss;                          // 任务的tss段
}task_t;

typedef void task_param_t;          // 任务参数
typedef void (task_entry_t)(task_param_t * param);      // 任务入口类型

int task_init (task_t * task, const char * name, task_entry_t * entry,
        task_param_t * param, void * stack_top, uint32_t priority);
void task_run_first (task_t * task);
void task_switch_to (task_t * task);

#endif //TASK_H
