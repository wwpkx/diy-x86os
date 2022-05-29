﻿/**
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
#include "tools/list.h"
#include "ipc/sem.h"
#include "ipc/bfifo.h"

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

    task_manager_init();
}

static uint32_t init_task_stack[1024];	// 空闲任务堆栈
static task_t init_task;
static sem_t sem;
static bfifo_t bfifo;
static uint8_t buffer[32];

/**
 * 初始任务函数
 * 目前暂时用函数表示，以后将会作为加载为进程
 */
void init_task_entry(void *param) {
    int count = 0;

    for (;;) {
        sem_wait(&sem);
        
        log_printf("init task: %d", count++);
        //sys_msleep(2000);
    }
}

void init_main(void) {
    log_printf("Kernel is running....");
    log_printf("Version: %s, name: %s", OS_VERSION, "tiny x86 os");
    log_printf("%d %x %c", 'a', 'a', 'a');

    // 初始化任务，调整下前后位置，让first_task在前，因为其是先运行的
    // 如果不调换，则当开启中断时，会触发定时，最终调用task_dipatch，立即切换到test_task中运行
    // 然而此时信号量还未初始化
    task_first_init();
    task_init(&init_task, "test task", (uint32_t)init_task_entry, (uint32_t)&init_task_stack[1024]);

    irq_enable_global();

    sem_init(&sem, 0);
    bfifo_init(&bfifo, buffer, sizeof(buffer));

    //int a = 3 / 0;
    int count = 0;
    for (;;) {
        log_printf("first task: %d", count++);
        // bfifo_write(&bfifo, &count, sizeof(count));
        bfifo_put(&bfifo, &count, sizeof(count));

        // 发消息给init task，可以打印了
        // sem_notify(&sem);

        sys_msleep(1000);
    }
}
