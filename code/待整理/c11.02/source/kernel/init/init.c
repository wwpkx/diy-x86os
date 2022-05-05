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

static boot_info_t * init_boot_info;        // 启动信息

/**
 * 内核入口
 */
void kernel_init (boot_info_t * boot_info) {
    init_boot_info = boot_info;

    memory_init(init_boot_info);

    // 初始化CPU，再重新加载
    cpu_init();

    irq_init();

    timer_init();
    log_init();
}

/**
 * @brief 移至第一个任务运行
 */
void move_to_first_task(uint32_t entry) {
    ((void (*)(void))entry)();
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
