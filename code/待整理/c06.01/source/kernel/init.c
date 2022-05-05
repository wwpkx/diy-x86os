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
#include "os_cfg.h"

static boot_info_t * init_boot_info;        // 启动信息

/**
 * 内核入口
 */
void kernel_init (boot_info_t * boot_info) {
    init_boot_info = boot_info;

    // 初始化CPU，再重新加载
    cpu_init();
    far_jump(KERNEL_SELECTOR_CS, (uint32_t)gdt_reload);
}

/**
 * 空闲任务代码
 */
void init_task_entry(void *param) {
    int count = 0;

    for (;;) {
        log_printf("init task: %d", count++);
        //task_switch_to(&init_task);
    }
} 

void init_main(void) {
    irq_init();

    timer_init();
    irq_enable_global();
    log_init();

    log_printf("Kernel is running....");
    log_printf("Version: %s", OS_VERSION);
    log_printf("print int: %d, %x", 1234, 0x1234);

    //int a = 3 / 0;
    int count = 0;
    for (;;) {
        log_printf("kernel task: %d", count++);
        //task_switch_to(&init_task);
    }
}
