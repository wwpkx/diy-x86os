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
int task_init (task_t *task, const char * name, uint32_t entry, uint32_t esp) {
    // 为TSS分配GDT
    segment_desc_t * tss_desc = gdt_alloc_desc();
    if (tss_desc == (segment_desc_t *)0) {
        return -1;
    }
    segment_desc_set(tss_desc, (uint32_t)&task->tss, sizeof(tss_t), SEG_P_PRESENT | SEG_DPL0 | GDB_TSS_TYPE);

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

    // 任务字段初始化
    kernel_strncpy(task->name, name, TASK_NAME_SIZE);
    task->state = TASK_CREATED;
    task->time_slice = TASK_TIME_SLICE_DEFAULT;
    task->slice_ticks = task->time_slice;
    list_node_init(&task->all_node);
    list_node_init(&task->run_node);

    // 插入就绪队列中'
    task_set_ready(task);
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
    task_init(first_task, "kernel task", 0, 0);     // 里面的值不必要写
    task_manager.curr_task = first_task;

 
    // 写TR寄存器，指示当前运行的第一个任务
    write_tr(first_task->tss_sel);
}

/**
 * @brief 将任务插入就绪队列
 */
void task_set_ready(task_t *task) {
    list_insert_last(&task_manager.ready_list, &task->run_node);
    task->state = TASK_READY;
}

/**
 * @brief 将任务从就绪队列移除
 */
void task_set_block (task_t *task) {
    list_remove(&task_manager.ready_list, &task->run_node);
}
/**
 * @brief 获取下一将要运行的任务
 */
task_t * task_next_run (void) {
    // 普通任务
    list_node_t * task_node = list_first(&task_manager.ready_list);
    return list_node_parent(task_node, task_t, run_node);
}


/**
 * @brief 获取当前正在运行的任务
 */
task_t * task_current (void) {
    return task_manager.curr_task;
}

/**
 * @brief 当前任务主动放弃CPU
 */
int sys_sched_yield (void) {
    task_t * curr_task = task_current();

    // 如果队列中还有其它任务，则将当前任务移入到队列尾部
    task_set_block(curr_task);
    task_set_ready(curr_task);

    // 切换至下一个任务，在切换完成前要保护，不然可能下一任务
    // 由于某些原因运行后阻塞或删除，再回到这里切换将发生问题
    task_dispatch();
}

/**
 * @brief 进行一次任务调度
 */
void task_dispatch (void) {
    task_t * next_task = task_next_run();
    if (next_task != task_manager.curr_task) {
        task_manager.curr_task = next_task;
        task_switch_to(task_manager.curr_task);
    }
}

/**
 * @brief 时间处理
 * 该函数在中断处理函数中调用
 */
void task_time_tick (void) {
    task_t * curr_task = task_current();

    // 时间片的处理
    if (--curr_task->slice_ticks == 0) {
        // 时间片用完，重新加载时间片
        // 对于空闲任务，此处减未用
        curr_task->slice_ticks = curr_task->time_slice;

        // 调整队列的位置到尾部，不用直接操作队列
        task_set_block(curr_task);
        task_set_ready(curr_task);
    }

    task_dispatch();
}
