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
#include "os_cfg.h"
#include "core/memory.h"
#include "fs/fs.h"
#include "dev/tty.h"

static boot_info_t * init_boot_info;        // 启动信息

/**
 * 内核入口
 */
void kernel_init (boot_info_t * boot_info) {
    init_boot_info = boot_info;

    // 初始化CPU，再重新加载
    cpu_init();

    memory_init(init_boot_info);

    irq_init();

    timer_init();
    log_init();
    tty_init();
    fs_init();
}

/**
 * @brief 移至第一个任务运行
 */
void move_to_first_task(uint32_t entry) {
    // 不能直接用Jmp far进入，因为当前特权级0，不能跳到低特权级的代码
    // 下面的iret后，还需要手动加载ds, fs, es等寄存器值，iret不会自动加载
    // 注意，运行下面的代码可能会产生异常：段保护异常或页保护异常。
    // 可根据产生的异常类型和错误码，并结合手册来找到问题所在
    tss_t * tss = &(task_current()->tss);
    __asm__ __volatile__(
        // 模拟中断返回，切换入第1个可运行应用进程
        // 不过这里并不直接进入到进程的入口，而是先设置好段寄存器，再跳过去
        "push %0\n\t"			// SS
        "push %1\n\t"			// ESP
        "push %2\n\t"           // EFLAGS
        "push %3\n\t"			// CS
        "push %4\n\t"		    // ip
        "iret\n\t"::"r"(tss->ss),  "r"(tss->esp), "r"(tss->eflags),"r"(tss->cs), "r"(entry));
}

void init_task_entry (void);

void init_main(void) {
    task_manager_init();

    log_printf("Kernel is running....");
    log_printf("Version: %s", OS_VERSION);
    log_printf("print int: %d, %x", 1234, 0x1234);

    // 初始化任务
    move_to_first_task((uint32_t)init_task_entry);
}
