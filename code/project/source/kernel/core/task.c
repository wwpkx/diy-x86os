//
// Created by lishutong on 2021-07-18.
//

#include <core/irq.h>
#include "core/task.h"
#include "core/list.h"
#include "core/os_cfg.h"
#include "core/cpu.h"
#include "core/cpu_instr.h"
#include "core/klib.h"

#define TASK_LDT_CODE_INDEX             0
#define TASK_LDT_DATA_INDEX             1
#define TASK_LDT_SS0_INDEX              2

#define TASK_SLICE_RELOAD_CNT   (TASK_SLICE_DEFAULT_MS + OS_TICK_MS - 1) / OS_TICK_MS

task_manager_t task_manager;

#define TASK_SS0_SIZE       20*1024
static uint32_t task_ss0_stack[10][TASK_SS0_SIZE];
static uint32_t task_ss9_index;

void task_manager_init (void) {
    task_ss9_index = 0;

    task_manager.flags = 0;
    task_manager.curr_task = (task_t *)0;
    task_manager.idle_task = (task_t *)0;

    // 优先级位图
    task_manager.ready_group = 0;
    for (int i = 0; i < sizeof(task_manager.ready_bitmap); i++) {
        task_manager.ready_bitmap[i] = 0;
    }

    // 就绪、挂起、睡眠队列
    for (int i = 0; i < TASK_PRIORITY_NR; i++) {
        list_init(task_manager.ready_list + i);
   }
    list_init(&task_manager.sleep_list);
    list_init(&task_manager.suspend_list);

}

void task_add_ready(task_t *task) {
    list_insert_last(task_manager.ready_list + task->priority, &task->node);

    uint8_t group = task->priority / 8;
    task_manager.ready_group |= (1 << group);
    task_manager.ready_bitmap[group] |= 1 << (task->priority % 8);

    task->state = TASK_STATE_READY;
}

void task_remove_ready(task_t *task) {
    list_t * ready_list = task_manager.ready_list + task->priority;
    list_remove(ready_list, &task->node);

    uint8_t group = task->priority / 8;
    if (list_count(ready_list) == 0) {
        task_manager.ready_bitmap[group] &= ~(1 << (task->priority % 8));

        if (task_manager.ready_bitmap[group] == 0) {
            task_manager.ready_group &= ~(1 << group);
        }
    }

    task->state = TASK_STATE_WAIT;
}

static void task_add_sleep(task_t *task, uint32_t ticks) {
    if (ticks <= 0) {
        return;
    }

    task->delay_ticks = ticks;

    // todo: 这里，延时队列的插入算法可以进行调整和优化
    list_insert_last(&task_manager.sleep_list, &task->node);

    task->state = TASK_STATE_SLEEP;
}

static void task_remove_sleep(task_t *task) {
    list_remove(&task_manager.sleep_list, &task->node);
}

static task_t * task_next_to_run(void) {
    static const uint8_t priority_find_table[] = {
            /* 00 */ 0xff, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* 10 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* 20 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* 30 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* 40 */ 6,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* 50 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* 60 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* 70 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* 80 */ 7,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* 90 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* A0 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* B0 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* C0 */ 6,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* D0 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* E0 */ 5,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
            /* F0 */ 4,    0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
    };

    uint8_t group = priority_find_table[task_manager.ready_group];
    uint32_t priority = (group * 8) + priority_find_table[task_manager.ready_bitmap[group]];

    list_node_t * node = list_first(task_manager.ready_list + priority);
    return node_to_parent(node, task_t, node);
}

void task_time_tick (void) {
    task_t * curr_task = task_manager.curr_task;

    if (curr_task) {
        curr_task = task_manager.curr_task;
        if (--curr_task->slice_counter == 0) {
            curr_task->slice_counter = TASK_SLICE_RELOAD_CNT;

            list_t * list = &task_manager.ready_list[curr_task->priority];
            list_remove(list, &curr_task->node);
            list_insert_last(list, &curr_task->node);
        }

        list_node_t * curr, * next;
        for (curr = list_first(&task_manager.sleep_list); curr; curr = next) {
            next = list_node_next(curr);

            task_t * task = node_to_parent(curr, task_t, node);
            if (--task->delay_ticks == 0) {
                task_remove_sleep(task);
                task_add_ready(task);
            }
        }
    }
}

// https://www.linuxprobe.com/linux-process.html
// https://blog.csdn.net/qq_35358036/article/details/79157874?utm_term=linux查看进程调度策略&utm_medium=distribute.pc_aggpage_search_result.none-task-blog-2~all~sobaiduweb~default-2-79157874&spm=3001.4430
// https://blog.csdn.net/gatieme/article/details/51701149
void task_dispatch(void) {
    if (irq_is_in()) {
        return;
    }

    irq_state_t irq_state = irq_enter_protection();

    task_t * next_task = task_next_to_run();
    if (next_task == (task_t *)0) {
    	for (;;) {}
    }
    if (next_task != task_manager.curr_task) {
        task_manager.curr_task = next_task;
        task_switch_to(task_manager.curr_task);
    }

    irq_leave_protection(irq_state);
}

static int cpu_context_init (task_t * task, int flags, task_entry_t * entry, task_param_t * param, uint32_t * stack_top) {
    // 初始化任务的LDT表
    for (int i = 0; i < TASK_LDT_TABLE_SIZE; i++) {
        set_segment_desc(task->cpu_context.ldt_table + i, 0, 0, 0);
    }

    // 核心任务，特权级为0。应用任务，特权级最低
    int dpl = GDT_SEG_DPL3, rpl = GDT_RPL3;
    if (flags & TASK_FLAG_SYSTEM_TASK) {
        dpl = GDT_GATE_DPL0;
        rpl = GDT_RPL0;
    }

    // 初始化LDT数据和代码段描述符。只有任务自己和特权级程序能访问
    set_segment_desc(task->cpu_context.ldt_table + TASK_LDT_CODE_INDEX, 0, 0xFFFFFFFF,
                     GDT_SET_PRESENT | dpl | GDT_SEG_S_CODE_DATA | GDT_SEG_TYPE_CODE | GDT_SEG_TYPE_RW |
                     GDT_SEG_D);
    set_segment_desc(task->cpu_context.ldt_table + TASK_LDT_DATA_INDEX, 0, 0xFFFFFFFF,
                     GDT_SET_PRESENT | dpl | GDT_SEG_S_CODE_DATA | GDT_SEG_TYPE_DATA | GDT_SEG_TYPE_RW |
                     GDT_SEG_D);
    set_segment_desc(task->cpu_context.ldt_table + TASK_LDT_SS0_INDEX, 0, 0xFFFFFFFF,
                     GDT_SET_PRESENT | GDT_SEG_DPL0 | GDT_SEG_S_CODE_DATA | GDT_SEG_TYPE_DATA | GDT_SEG_TYPE_RW |
                     GDT_SEG_D);

    // 在GDT中建立LDT的描述符, 只有特权级程序能访问
    gdt_descriptor_t * desc = gdt_alloc_desc();
    set_segment_desc(desc, (uint32_t) &task->cpu_context.ldt_table, sizeof(task->cpu_context.ldt_table) - 1,
                     GDT_SET_PRESENT | GDT_SEG_DPL0 | GDT_TYPE_LDT);

    // 保存自己的LDT描述索引
    task->cpu_context.ldt_selector = desc_2_gdt_selector(desc);

    // 分配gdt描述符给tss
    gdt_descriptor_t * tss_desc = gdt_alloc_desc();
    if (!tss_desc) {
        return -1;
    }

    // https://blog.csdn.net/kafmws/article/details/103513824
    if (stack_top != (uint32_t *)0) {
        *(--stack_top) = (uint32_t)task;				// task_remove的传入的参数
        *(--stack_top) = (uint32_t)task_remove;		    // task_remove的返回地址
        *(--stack_top) = (uint32_t)param;				// 任务的传入参数
        *(--stack_top) = (uint32_t)task_remove;		    // 任务的返回地址
    }

    tss_t * tss = &task->cpu_context.tss;
    tss->pre_link = 0;
    tss->ss0 = (TASK_LDT_SS0_INDEX << 3) | GDT_SELECTOR_TI;     // ss0的dpl为0
    tss->esp0 = (uint32_t)task_ss0_stack[++task_ss9_index];
    tss->ss1 = tss->esp1 = tss->esp2 = tss->ss2 = 0;     // 1/2不用
    tss->cr3 = 0;
    tss->eip = (uint32_t)entry;
    tss->eflags = EFLAGS_DEFAULT | EFLAGS_IF;
    tss->eax = tss->ecx = tss->edx = tss->ebx;
    tss->ebp = 0;
    tss->esp = (uint32_t)stack_top;
    tss->ss = (TASK_LDT_DATA_INDEX << 3) | GDT_SELECTOR_TI | rpl;
    tss->esi = tss->edi = 0;
    tss->es = tss->ds = tss->fs = tss->gs = tss->ss;
    tss->cs = (TASK_LDT_CODE_INDEX << 3) | GDT_SELECTOR_TI | rpl;
    tss->ldt = task->cpu_context.ldt_selector; // 指向不在乎中的描述符
    tss->iomap = 0x40000000;

    // 特权级设置成0，这样只有特权级程序才能进行任务切换
    set_segment_desc(tss_desc, (uint32_t) tss, sizeof(tss_t), GDT_SET_PRESENT | GDT_SEG_DPL0 | GDB_TSS_TYPE);

    // 保存tss，注意RPL=0，用于操作系统访问
    task->cpu_context.tss_selector = desc_2_gdt_selector(tss_desc);
    return 0;
}

int task_init (task_t * task, const char * name, int flags, task_entry_t * entry, task_param_t * param,
        uint32_t * stack_top, uint32_t priority, void * msg_buf, int msg_size, int msg_count) {
    uint32_t err = cpu_context_init(task, flags, entry, param, stack_top);
    if (err < 0) {
        return err;
    }

    k_strncpy(task->name, name, TASK_NAME_SIZE);
    task->flags = flags;
    task->state = TASK_STATE_INITED;
    task->priority = priority;
    task->delay_ticks = 0;
    task->slice_counter = TASK_SLICE_RELOAD_CNT;

    // 创建发送给任务的消息队列
    queue_init(&task->queue, msg_buf, msg_size, msg_count);

    // 插入就绪队列中
    task_add_ready(task);

    task_manager.flags |= TASK_ANY_CRATED;
    return 0;
}

void sys_msleep (uint32_t ms) {
    if (task_manager.curr_task == task_manager.idle_task) {
        return;
    }

    if (ms < OS_TICK_MS) {
        ms = OS_TICK_MS;
    }

    task_t * curr = task_manager.curr_task;

    irq_state_t state = irq_enter_protection();
    task_remove_ready(curr);
    task_add_sleep(curr, (ms + (OS_TICK_MS - 1))/ OS_TICK_MS);
    task_dispatch();
    irq_leave_protection(state);
}


void task_switch_to (task_t * task) {
    // 跳转至TSS时，会触发任务切换，即当前上下文的保存和后续任务上下文的恢复
    // 实际会将当前的寄存器内容，自动保存到TSS中，不需要再写指令去做这件事情。
    // 然后再去加载新任务的TSS，从中读取寄存器，无需再写指令去做此事
    // 整个过程中，会自动设置TR寄存器
    if (task_manager.curr_task) {
        task_manager.curr_task = task;
    }

    switch_to_tss(task->cpu_context.tss_selector);
}

void task_remove(task_t *task) {
    // 不能返回
    for (;;) {

    }
}

void move_to_first_task(void) {
    task_t * task = task_next_to_run();

    task_manager.curr_task = task;

    lldt(task->cpu_context.ldt_selector);
    write_tr(task->cpu_context.tss_selector);

    // 不能直接用Jmp far进入，因为当前特权级0，不能跳到低特权级的代码
    // 下面的iret后，还需要手动加载ds, fs, es等寄存器值，iret不会自动加载
    tss_t * tss = &task->cpu_context.tss;
    __asm__ __volatile__(
    		// 模拟中断返回，切换入第1个可运行应用进程
    		// 不过这里并不直接进入到进程的入口，而是先设置好段寄存器，再跳过去
            "push %0\n\t"			// SS
            "push %1\n\t"			// ESP
            "pushfl\n\t"			// EFLAGS
            "push %2\n\t"			// CS
            "push $first\n\t"		// 当前偏移
            "iret\n\t"
            "first:\n\t"

    		// 注意开中断，因为之前EFLAGS的IF=0，中断是关掉的
    		// IRET指令执行并不改变IF
    		"sti\n\t"

    		// 更新各个数据段寄存器为应用的自己的。SS由IRET指令自行设置
            "mov %0, %%ds\n\t"
            "mov %0, %%es\n\t"
            "mov %0, %%fs\n\t"
            "mov %0, %%gs\n\t"
            "jmp *%3"::"r"(tss->ss), "r"(tss->esp), "r"(tss->cs), "r"(tss->eip));
}

task_t * task_current (void) {
    return task_manager.curr_task;
}
