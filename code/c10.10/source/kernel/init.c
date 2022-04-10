/**
 * 内核初始化以及测试代码
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "comm/boot_info.h"
#include "comm/cpu_instr.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "dev/timer.h"
#include "tools/log.h"
#include "core/task.h"
#include "tools/list.h"
#include "ipc/bfifo.h"
#include "core/memory.h"
#include "cpu/mmu.h"
#include "tools/klib.h"
#include "os_cfg.h"

static boot_info_t * init_boot_info;        // 启动信息

static uint32_t init_task_stack[1024];	// 空闲任务堆栈

static bfifo_t bfifo;
static uint8_t fifo_buf[32];

/**
 * @brief 预先初始化
 */
void kernel_init (boot_info_t * boot_info) {
    init_boot_info = boot_info;

    memory_init(init_boot_info);
    cpu_init();

    irq_init();

    timer_init();
    log_init();
}

/**
 * @brief 移至第一个任务运行
 */
void move_to_first_task(uint32_t entry) {
    extern uint8_t * init_load_addr;
    extern uint8_t * init_load_size;

    // 分配一页内存供代码存放使用，然后将代码复制过去
    uint32_t size = (uint32_t)&init_load_size;
    memory_alloc_vaddr_page(MEMORY_PROC_BASE,  size, PTE_P | PTE_U);
    kernel_memcpy((void *)MEMORY_PROC_BASE, (void *)&init_load_addr, size);

    // 不能直接用Jmp far进入，因为当前特权级0，不能跳到低特权级的代码
    // 下面的iret后，还需要手动加载ds, fs, es等寄存器值，iret不会自动加载
    tss_t * tss = &(task_current()->tss);
    __asm__ __volatile__(
        // 模拟中断返回，切换入第1个可运行应用进程
        // 不过这里并不直接进入到进程的入口，而是先设置好段寄存器，再跳过去
        "push %0\n\t"			// SS
        "push %1\n\t"			// ESP
        "pushfl\n\t"			// EFLAGS
        "push %2\n\t"			// CS
        "push $first\n\t"		// ip
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
        "jmp *%3"::"r"(tss->ss), "r"(tss->esp), "r"(tss->cs), "r"(entry));
}

int init_task_entry (void);

void init_main(void) {
    task_manager_init();

    log_printf("Kernel is running....");
    log_printf("Version: %s", OS_VERSION);
    log_printf("print int: %d, %x", 1234, 0x1234);

    // 初始化任务
    move_to_first_task((uint32_t)init_task_entry);
}
