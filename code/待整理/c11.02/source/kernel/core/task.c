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
#include "cpu/irq.h"
#include "cpu/mmu.h"
#include "os_cfg.h"
#include "core/memory.h"

static task_manager_t task_manager;     // 任务管理器
static uint32_t idle_task_stack[IDLE_STACK_SIZE];	// 空闲任务堆栈

/**
 * @brief 初始化任务
 */
int task_init (task_t *task, const char * name, uint32_t entry, uint32_t esp) {
    // 为TSS分配GDT
    int tss_sel = gdt_alloc_segment((uint32_t)&task->tss,
                                sizeof(tss_t), SEG_P_PRESENT | SEG_DPL0 | GDB_TSS_TYPE);
    if (tss_sel < 0) {
        return -1;
    }

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
    task->tss_sel = tss_sel;

    // 页表初始化
    uint32_t page_dir = memory_create_uvm();
    if (page_dir == 0) {
        goto task_init_failed;
    }
    task->tss.cr3 = page_dir;

    // 任务字段初始化
    kernel_strncpy(task->name, name, TASK_NAME_SIZE);
    task->state = TASK_CREATED;
    task->sleep_ticks = 0;
    task->time_slice = TASK_TIME_SLICE_DEFAULT;
    task->slice_ticks = task->time_slice;
    list_node_init(&task->all_node);
    list_node_init(&task->run_node);
    list_node_init(&task->wait_node);

    // 插入就绪队列中'
    irq_state_t state = irq_enter_protection();
    task_set_ready(task);
    irq_leave_protection(state);
    return 0;

task_init_failed:
    gdt_free_sel(tss_sel);
    return -1;
}

/**
 * @brief 切换至指定任务
 */
void task_switch_to (task_t * task) {
    switch_to_tss(task->tss_sel);
}

/**
 * @brief 空闲任务
 */
static void idle_task_entry (void) {
    for (;;) {
        hlt();
    }
}

static void first_task_init (void) {
    extern uint8_t * init_load_addr;
    extern uint8_t * init_load_size;

    uint32_t init_size = (uint32_t)&init_load_size;
    uint32_t total_size = 10 * MEM_PAGE_SIZE;        // 可以设置的大一些, 如40KB
    ASSERT(init_size < MEM_PAGE_SIZE);

    // 第一个任务代码量小一些，好和栈放在1个页面呢
    // 这样就不要立即考虑还要给栈分配空间的问题
    task_init(&task_manager.init_task, "kernel task", 0, 0);     // 里面的值不必要写
    task_manager.curr_task = (task_t *)&task_manager.init_task;

    // 更新页表地址为自己的
    mmu_set_page_dir(task_manager.init_task.tss.cr3);

    // 分配一页内存供代码存放使用，然后将代码复制过去
    memory_alloc_page_for(MEMORY_TASK_BASE,  total_size, PTE_P | PTE_W);
    kernel_memcpy((void *)MEMORY_TASK_BASE, (void *)&init_load_addr, init_size);
}

/**
 * @brief 任务管理器初始化
 */
void task_manager_init (void) {
    task_manager.curr_task = (task_t *)0;

    // 各队列初始化
    list_init(&task_manager.ready_list);
    list_init(&task_manager.task_list);
    list_init(&task_manager.sleep_list);

    first_task_init();
    task_init(&task_manager.idle_task,
                "idle task", 
                (uint32_t)idle_task_entry, 
                (uint32_t)(idle_task_stack + IDLE_STACK_SIZE));     // 里面的值不必要写
    task_manager.curr_task = &task_manager.init_task;

    // 写TR寄存器，指示当前运行的第一个任务
    write_tr(task_manager.init_task.tss_sel);
}

/**
 * @brief 将任务插入就绪队列
 */
void task_set_ready(task_t *task) {
    if (task != &task_manager.idle_task) {
        list_insert_last(&task_manager.ready_list, &task->run_node);
        task->state = TASK_READY;
    }
}

/**
 * @brief 将任务从就绪队列移除
 */
void task_set_block (task_t *task) {
    if (task != &task_manager.idle_task) {
        list_remove(&task_manager.ready_list, &task->run_node);
    }
}

/**
 * @brief 获取下一将要运行的任务
 */
task_t * task_next_run (void) {
    // 如果没有任务，则运行空闲任务
    if (list_count(&task_manager.ready_list) == 0) {
        return &task_manager.idle_task;
    }
    
    // 普通任务
    list_node_t * task_node = list_first(&task_manager.ready_list);
    return list_node_parent(task_node, task_t, run_node);
}

/**
 * @brief 将任务加入睡眠状态
 */
void task_set_sleep(task_t *task, uint32_t ticks) {
    if (ticks <= 0) {
        return;
    }

    task->sleep_ticks = ticks;
    task->state = TASK_SLEEP;
    list_insert_last(&task_manager.sleep_list, &task->run_node);
}

/**
 * @brief 将任务从延时队列移除
 * 
 * @param task 
 */
void task_set_wakeup (task_t *task) {
    list_remove(&task_manager.sleep_list, &task->run_node);
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

    irq_state_t state = irq_enter_protection();

    // 如果队列中还有其它任务，则将当前任务移入到队列尾部
    task_set_block(curr_task);
    task_set_ready(curr_task);

    // 切换至下一个任务，在切换完成前要保护，不然可能下一任务
    // 由于某些原因运行后阻塞或删除，再回到这里切换将发生问题
    task_dispatch();

    irq_leave_protection(state);
    return 0;
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
    irq_state_t state = irq_enter_protection();
    if (--curr_task->slice_ticks == 0) {
        // 时间片用完，重新加载时间片
        // 对于空闲任务，此处减未用
        curr_task->slice_ticks = curr_task->time_slice;

        // 调整队列的位置到尾部，不用直接操作队列
        task_set_block(curr_task);
        task_set_ready(curr_task);
    }
    
    // 睡眠处理
    list_node_t * curr = list_first(&task_manager.sleep_list);
    while (curr) {
        list_node_t * next = list_node_next(curr);

        task_t * task = list_node_parent(curr, task_t, run_node);
        if (--task->sleep_ticks == 0) {
            // 延时时间到达，从睡眠队列中移除，送至就绪队列
            task_set_wakeup(task);
            task_set_ready(task);
        }
        curr = next;
    }

    task_dispatch();
    irq_leave_protection(state);
}

/**
 * @brief 任务进入睡眠状态
 * 
 * @param ms 
 */
void sys_msleep (uint32_t ms) {
    // 至少延时1个tick
    if (ms < OS_TICK_MS) {
        ms = OS_TICK_MS;
    }

    irq_state_t state = irq_enter_protection();

    // 从就绪队列移除，加入睡眠队列
    task_set_block(task_manager.curr_task);
    task_set_sleep(task_manager.curr_task, (ms + (OS_TICK_MS - 1))/ OS_TICK_MS);
    
    // 进行一次调度
    task_dispatch();

    irq_leave_protection(state);
}
