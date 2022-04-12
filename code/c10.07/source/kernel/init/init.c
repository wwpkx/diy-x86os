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
}

static uint32_t init_task_stack[1024];	// 空闲任务堆栈

static task_t init_task;
static bfifo_t bfifo;
static uint8_t fifo_buf[32];

/**
 * 空闲任务代码
 */
void init_task_entry(void *param) {
    int count = 0;

    for (;;) {        
        bfifo_read(&bfifo, &count, sizeof(int));
        log_printf("init task: %d", count);
        //sys_msleep(500);
    }
}

void init_main(void) {
    irq_init();

    timer_init();
    log_init();

    task_manager_init();

    log_printf("Kernel is running....");
    log_printf("Version: %s", OS_VERSION);
    log_printf("print int: %d, %x", 1234, 0x1234);

    // 初始化任务
    task_init(&init_task, "init task", (uint32_t)init_task_entry, (uint32_t)&init_task_stack[1024]);
    //irq_enable_global();

    bfifo_init(&bfifo, fifo_buf, 32);
    
    //int a = 3 / 0;
    int count = 0;
    for (;;) {
        bfifo_write(&bfifo, &count, sizeof(int));
        count++;
        
        //log_printf("kernel task: %d", count++);
        sys_msleep(1000);
    }
}
