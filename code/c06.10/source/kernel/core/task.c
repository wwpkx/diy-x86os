/**
 * 任务结构实现
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "core/task.h"
#include "cpu/cpu.h"
#include "comm/cpu_instr.h"
#include "tools/klib.h"

// 任务的LDT选择子
#define TASK_LDT_CODE_INDEX         0
#define TASK_LDT_DATA_INDEX         1
#define TASK_LDT_SS0_INDEX          2

static int cpu_context_init (task_t * task, task_entry_t * entry, task_param_t * param, uint32_t * stack_top) {
    gdt_alloc_desc();

    // 初始化LDT数据和代码段描述符。只有任务自己和特权级程序能访问
    set_segment_desc(task->ldt_table + TASK_LDT_CODE_INDEX, 0, 0xFFFFFFFF,
                     GDT_SET_PRESENT | GDT_GATE_DPL0 | GDT_SEG_S_CODE_DATA | GDT_SEG_TYPE_CODE | GDT_SEG_TYPE_RW |
                     GDT_SEG_D);
    set_segment_desc(task->ldt_table + TASK_LDT_DATA_INDEX, 0, 0xFFFFFFFF,
                     GDT_SET_PRESENT | GDT_GATE_DPL0 | GDT_SEG_S_CODE_DATA | GDT_SEG_TYPE_DATA | GDT_SEG_TYPE_RW |
                     GDT_SEG_D);
    set_segment_desc(task->ldt_table + TASK_LDT_SS0_INDEX, 0, 0xFFFFFFFF,
                     GDT_SET_PRESENT | GDT_SEG_DPL0 | GDT_SEG_S_CODE_DATA | GDT_SEG_TYPE_DATA | GDT_SEG_TYPE_RW |
                     GDT_SEG_D);

    // 在GDT中建立LDT的描述符, 只有特权级程序能访问
    gdt_descriptor_t * desc = gdt_alloc_desc();
    set_segment_desc(desc, (uint32_t) &task->ldt_table, sizeof(task->ldt_table) - 1,
                     GDT_SET_PRESENT | GDT_SEG_DPL0 | GDT_TYPE_LDT);

    // 保存自己的LDT描述索引
    task->ldt_selector = desc_2_gdt_selector(desc);

    // 分配gdt描述符给tss
    gdt_descriptor_t * tss_desc = gdt_alloc_desc();
    if (!tss_desc) {
        return -1;
    }

    tss_t * tss = &task->tss;
    tss->pre_link = 0;
    tss->ss0 = (TASK_LDT_SS0_INDEX << 3) | GDT_SELECTOR_TI;     // ss0的dpl为0
    tss->esp0 = (uint32_t)stack_top;
    tss->ss1 = tss->esp1 = tss->esp2 = tss->ss2 = 0;     // 1/2不用
    tss->cr3 = 0;
    tss->eip = (uint32_t)entry;
    tss->eflags = EFLAGS_DEFAULT | EFLAGS_IF;
    tss->eax = tss->ecx = tss->edx = tss->ebx;
    tss->ebp = 0;
    tss->esp = (uint32_t)stack_top;
    tss->ss = (TASK_LDT_DATA_INDEX << 3) | GDT_SELECTOR_TI | GDT_RPL0;
    tss->esi = tss->edi = 0;
    tss->es = tss->ds = tss->fs = tss->gs = tss->ss;
    tss->cs = (TASK_LDT_CODE_INDEX << 3) | GDT_SELECTOR_TI | GDT_RPL0;
    tss->ldt = task->ldt_selector;   // 指向任务的LDT
    tss->iomap = 0x40000000;

    // 特权级设置成0，这样只有特权级程序才能进行任务切换
    set_segment_desc(tss_desc, (uint32_t) tss, sizeof(tss_t), GDT_SET_PRESENT | GDT_SEG_DPL0 | GDB_TSS_TYPE);

    // 保存tss，注意RPL=0，用于操作系统访问
    task->tss_selector = desc_2_gdt_selector(tss_desc);
    return 0;
}

/**
 * @brief 完成任务的初始化，使共处于可运行状态
 */
int task_init (task_t * task, const char * name, task_entry_t * entry,
        task_param_t * param, void * stack_top, uint32_t priority) {
    uint32_t err = cpu_context_init(task, entry, param, stack_top);
    if (err < 0) {
        return err;
    }

    kernel_strncpy(task->name, name, TASK_NAME_SIZE);
    task->state = TASK_READY;
    task->priority = priority;
    task->ticks = 0;
    return 0;
}

/**
 * @brief 运行第一个任务，系统启动后只需要调用一次
 */
void task_run_first (task_t * task) {
    lldt(task->ldt_selector);
    write_tr(task->tss_selector);

    // 不能直接用Jmp far进入，因为当前特权级0，不能跳到低特权级的代码
    // 下面的iret后，还需要手动加载ds, fs, es等寄存器值，iret不会自动加载
    tss_t * tss = &task->tss;
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

/**
 * @brief 切换至指令任务 
 */
void task_switch_to (task_t * task) {
    // 跳转至TSS时，会触发任务切换，即当前上下文的保存和后续任务上下文的恢复
    // 实际会将当前的寄存器内容，自动保存到TSS中，不需要再写指令去做这件事情。
    // 然后再去加载新任务的TSS，从中读取寄存器，无需再写指令去做此事
    // 整个过程中，会自动设置TR寄存器
    switch_to_tss(task->tss_selector);
}