/**
 * 任务实现
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef TASK_H
#define TASK_H

#include "comm/types.h"
#include "cpu/cpu.h"
#include "tools/list.h"


/**
 * @brief 任务控制块结构
 */
typedef struct _task_t {
	//uint32_t * stack;

	tss_t tss;				// 任务的TSS段
	uint16_t tss_sel;		// tss选择子
}task_t;

int task_init (task_t *task, uint32_t entry, uint32_t esp);
void task_switch_from_to (task_t * from, task_t * to);

typedef struct _task_manager_t {
    task_t * curr_task;         // 当前运行的任务

	list_t ready_list;			// 就绪队列
	list_t task_list;			// 所有已创建任务的队列

	task_t first_task;			// 内核任务
}task_manager_t;

void task_manager_init (void);
void task_first_init (void);
task_t * task_first_task (void);

#endif

