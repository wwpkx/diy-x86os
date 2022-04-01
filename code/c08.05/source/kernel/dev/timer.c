//
// https://wiki.osdev.org/Programmable_Interval_Timer
//

#include "dev/timer.h"
#include "cpu/irq.h"
#include "comm/cpu_instr.h"
#include "core/task.h"
#include "os_cfg.h"

static uint32_t sys_tick;						// 系统启动后的tick数量

/**
 * 定时器中断处理函数
 */
void do_handler_timer (exception_frame_t *frame) {
    sys_tick++;

    // 先发EOI，而不是放在最后
    // 放最后将从任务中切换出去之后，除非任务再切换回来才能继续噢应
    pic_send_eoi(IRQ0_TIMER);


    task_time_tick();
}

/**
 * 获取系统启动后的时钟节拍
 */
uint32_t timer_get_sys_tick (void) {
	return sys_tick;
}

/**
 * 初始化硬件定时器
 */
static void init_pit (void) {
    uint32_t reload_count = PIT_OSC_FREQ / (1000.0 / OS_TICK_MS);

    outb(PIT_COMMAND_MODE_PORT, PIT_CHANNLE0 | PIT_ACCESS_LOHIBYTE_ONLY | PIT_MODE3);
    outb(PIT_CHANNLE0_DATA_PORT, reload_count & 0xFF);
    outb(PIT_CHANNLE0_DATA_PORT, (reload_count >> 8) & 0xFF);
}

/**
 * 定时器初始化
 */
void timer_init (void) {
    sys_tick = 0;

    init_pit();
    irq_install(IRQ0_TIMER, (irq_handler_t)handler_timer);
    irq_enable(IRQ0_TIMER);
}


