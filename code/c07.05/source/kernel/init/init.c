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
#include "dev/time.h"
#include "tools/log.h"
#include "core/task.h"
#include "os_cfg.h"
#include "tools/klib.h"

static boot_info_t * init_boot_info;        // 启动信息

/**
 * 内核入口
 */
void kernel_init (boot_info_t * boot_info) {
    init_boot_info = boot_info;

    // 初始化CPU，再重新加载
    cpu_init();

    log_init();
    irq_init();
    time_init();
}

static task_t first_task;       // 第一个任务
static uint32_t init_task_stack[1024];	// 空闲任务堆栈
static task_t init_task;

/**
 * 初始任务函数
 * 目前暂时用函数表示，以后将会作为加载为进程
 */
void init_task_entry(void) {
    int count = 0;

    for (;;) {
        log_printf("init task: %d", count++);
        task_switch_from_to(&init_task, &first_task);
    }
}

void init_main(void) {
    log_printf("Kernel is running....");
    log_printf("Version: %s, name: %s", OS_VERSION, "tiny x86 os");
    log_printf("%d %d %x %c", -123, 123456, 0x12345, 'a');

    // 初始化任务
    task_init(&init_task, (uint32_t)init_task_entry, (uint32_t)&init_task_stack[1024]);
    task_init(&first_task, 0, 0);
    write_tr(first_task.tss_sel);

    //int a = 3 / 0;
    // irq_enable_global();
    int count = 0;
    for (;;) {
        log_printf("first task: %d", count++);
        task_switch_from_to(&first_task, &init_task);
    }
}
