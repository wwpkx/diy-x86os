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
#include "os_cfg.h"

static boot_info_t * init_boot_info;        // 启动信息

#define TASK_DEFAULT_STACK_SIZE         1024     // 任务默认的栈大小
#define TASK_DEFAULT_PRIO               32       // 任务默认的优先级大小

static task_t init_task;				// 空闲任务结构
static uint32_t init_task_stack[TASK_DEFAULT_STACK_SIZE];	// 空闲任务堆栈
static task_t idle_task;				// 空闲任务结构
static uint32_t idle_task_stack[TASK_DEFAULT_STACK_SIZE];	// 空闲任务堆栈

/**
 * 空闲任务代码
 */
void init_task_entry(task_param_t *param) {
    int count = 0;

    for (;;) {
        log_printf("init task: %d", count++);
        task_switch_to(&idle_task);
    }
}

/**
 * 空闲任务代码
 */
void idle_task_entry(task_param_t *param) {
    int count = 0;

    for (;;) {
        log_printf("idle task: %d", count++);
        task_switch_to(&init_task);
    }
}

/**
 * 创建第一个任务
 */
void create_init_task (void) {
    // 创建空闲任务
    task_init(&idle_task,
              "idle task",
			  idle_task_entry,
			  (void *) 0x12345678,
              (uint32_t *) ((uint8_t *) idle_task_stack + sizeof(idle_task_stack)),
			  TASK_DEFAULT_PRIO);

    // 创建初始任务
    task_init(&init_task,
              "init task",
			  init_task_entry,
			  (void *) 0x12345678,
              (uint32_t *) ((uint8_t *) init_task_stack + sizeof(init_task_stack)),
			  TASK_DEFAULT_PRIO);
}

/**
 * 内核入口
 */
void _start (boot_info_t *boot_info) {
    init_boot_info = boot_info;

    // 初始化CPU，再重新加载
    cpu_init();
    far_jump(KERNEL_SELECTOR_CS, (uint32_t)gdt_reload);
}

void kernel_entry(boot_info_t *boot_info) {
    irq_init();

    timer_init();
    irq_enable_global();
    log_init();

    log_printf("Kernel is running....");
    log_printf("Version: %s", OS_VERSION);
    log_printf("print int: %d, %x", 1234, 0x1234);

    task_run_first(&init_task);
    //int a = 3 / 0;
    for (;;) {}
}
