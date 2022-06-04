/**
 * 任务管理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "comm/cpu_instr.h"
#include "core/task.h"
#include "tools/klib.h"
#include "os_cfg.h"

static int tss_init (task_t * task, uint32_t entry, uint32_t esp) {
    // tss段初始化
    kernel_memset(&task->tss, 0, sizeof(tss_t));
    task->tss.eip = entry;
    task->tss.esp = task->tss.esp0 = esp;
    task->tss.ss0 = KERNEL_SELECTOR_DS;
    task->tss.eip = entry;
    task->tss.eflags = EFLAGS_DEFAULT | EFLAGS_IF;
    task->tss.es = task->tss.ss = task->tss.ds 
            = task->tss.fs = task->tss.gs = KERNEL_SELECTOR_DS;   // 暂时写死
    task->tss.cs = KERNEL_SELECTOR_CS;    // 暂时写死
    task->tss.iomap = 0;
    return 0;
}

/**
 * @brief 初始化任务
 */
int task_init (task_t *task, uint32_t entry, uint32_t esp) {
    ASSERT(task != (task_t *)0);

    tss_init(task, entry, esp);
    return 0;
}
