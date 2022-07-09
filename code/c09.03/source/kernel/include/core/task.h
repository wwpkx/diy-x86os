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

#define TASK_NAME_SIZE				32			// 任务名字长度


/**
 * @brief 任务控制块结构
 */
typedef struct _task_t {
    enum {
		TASK_CREATED,
		TASK_RUNNING,
		TASK_SLEEP,
		TASK_READY,
		TASK_WAITING,
	}state;

    char name[TASK_NAME_SIZE];		// 任务名字

	tss_t tss;				// 任务的TSS段
	uint16_t tss_sel;		// tss选择子
	
	list_node_t run_node;		// 运行相关结点
	list_node_t all_node;		// 所有队列结点
}task_t;

int task_init (task_t *task, const char * name, uint32_t entry, uint32_t esp);
void task_switch_from_to (task_t * from, task_t * to);
void task_set_ready(task_t *task);
void task_set_block (task_t *task);
int sys_yield (void);
void task_dispatch (void);
task_t * task_current (void);

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

