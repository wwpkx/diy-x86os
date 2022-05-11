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

/**
 * @brief 初始化任务
 */
int task_init (task_t *task, uint32_t entry, uint32_t esp) {
    // 为TSS分配GDT
    gdt_descriptor_t * tss_desc = gdt_alloc_desc();
    if (tss_desc == (gdt_descriptor_t *)0) {
        return -1;
    }
    gdt_segment_desc_set(tss_desc, (uint32_t)&task->tss, sizeof(tss_t), GDT_SET_PRESENT | GDT_SEG_DPL0 | GDB_TSS_TYPE);

    // tss段初始化
    kernel_memset(&task->tss, 0, sizeof(tss_t));
    task->tss.eip = entry;
    task->tss.esp = task->tss.esp0 = esp;
    task->tss.ss0 = 16;
    task->tss.eip = entry;
    task->tss.eflags = EFLAGS_DEFAULT | EFLAGS_IF;
    task->tss.es = task->tss.ss = task->tss.ds = task->tss.fs = task->tss.gs = 16;   // 暂时写死
    task->tss.cs = 8;    // 暂时写死
    task->tss.iomap = 0x40000000;

    task->tss_sel = desc_2_gdt_selector(tss_desc);
    return 0;
}

/**
 * @brief 切换至指定任务
 */
void task_switch_to (task_t * task) {
    switch_to_tss(task->tss_sel);
}
