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

/**
 * @brief 任务控制块结构
 */
typedef struct _task_t {
	tss_t tss;				// 任务的TSS段
	uint16_t tss_sel;		// tss选择子
}task_t;

int task_init (task_t *task, uint32_t entry, uint32_t esp);
void task_switch_from_to (task_t * from, task_t * to);

#endif

