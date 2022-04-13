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

static task_manager_t task_manager;     // 任务管理器

/**
 * @brief 初始化任务
 */
int task_init (task_t *task, uint32_t entry, uint32_t esp) {
    // 为TSS分配GDT
    gdt_descriptor_t * tss_desc = gdt_alloc_desc();
    if (tss_desc == (gdt_descriptor_t *)0) {
        return -1;
    }
    set_segment_desc(tss_desc, (uint32_t)&task->tss, sizeof(tss_t), GDT_SET_PRESENT | GDT_SEG_DPL0 | GDB_TSS_TYPE);

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

/**
 * @brief 任务管理器初始化
 */
void task_manager_init (void) {
    task_manager.curr_task = (task_t *)0;

    // 各队列初始化
    list_init(&task_manager.ready_list);
    list_init(&task_manager.task_list);

    // 初始化内核任务
    task_t * first_task = &task_manager.init_task;
    task_init(first_task, 0, 0);     // 里面的值不必要写
 
    // 写TR寄存器，指示当前运行的第一个任务
    write_tr(first_task->tss_sel);
}
