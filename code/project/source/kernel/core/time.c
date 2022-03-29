//
// https://wiki.osdev.org/Programmable_Interval_Timer
//

#include <core/time.h>
#include <core/klib.h>
#include "core/cpu_instr.h"
#include "core/irq.h"
#include "core/task.h"
#include "core/os_cfg.h"

#define PIT_OSC_FREQ        1193182				// 定时器时钟

#define TIMER_MAX_COUNT     20					// 最大支持的定时器数量
static rtimer_t timer[TIMER_MAX_COUNT];			// 定时器
static list_t free_list, started_list;			// 空闲和已经创建的定时
static uint32_t sys_tick;						// 系统启动后的tick数量

/**
 * 软定时处理
 */
static void timer_time_tick (void) {
	// 检查定时器链表，
	for (list_node_t * node = list_first(&started_list); node; node = list_node_next(node)) {
		rtimer_t * timer = node_to_parent(node, rtimer_t, node);

		if (--timer->curr_tmo == 0) {
			timer->curr_tmo = timer->reload_count;

			// 向定时器所在的进程的消息队列发送消息
			task_t * task = timer->task;
			task_msg_t * task_msg = (task_msg_t *)queue_get_free(&task->queue);
			if (task_msg) {
				task_msg->msg.type = APP_MSG_TYPE_TIMER;
				task_msg->msg.timer = timer;
				task_msg->msg.data = (int)timer->data;
				queue_send_msg(&task->queue, (queue_msg_t *)task_msg);
			}
		}
	}
}

/**
 * 读RTC寄存器
*/
static uint8_t read_rtc(int reg_addr) {
      outb(CMOS_RTC_ADDR, (0 << 7)| reg_addr);		// 禁用NMI，不处理
      return inb(CMOS_RTC_DATA);
}

/**
 * 等待RTC更新完毕
 */
static void wait_update_complete (void) {
	// 等待正在进行更新清0
	while ((read_rtc(0x0A) & (1 << 7)) == 0) {}
}

/**
 * bcd转二进制
 */
static uint8_t bcd_to_bin (uint8_t bcd) {
	return (bcd & 0x0f) + (bcd >> 4) * 10;
}

/**
 * 从rtc中读取时间和日期
 */
void read_time_date(date_time_t * date_time) {
	date_time_t last;

	// 为了保证读取正确，要读取再次，避免在第一次读时，时间值发生变化
	// 导致读取的结果有问题
	do {
		// 先读一次
		wait_update_complete();
		last.second = read_rtc(0x00);
		last.minute = read_rtc(0x02);
		last.hour = read_rtc(0x04);
		last.day = read_rtc(0x07);
		last.month = read_rtc(0x08);
		last.year = read_rtc(0x09);

		// 再读一次
		wait_update_complete();
		date_time->second = read_rtc(0x00);
		date_time->second = read_rtc(0x00);
		date_time->minute = read_rtc(0x02);
		date_time->hour = read_rtc(0x04);
		date_time->day = read_rtc(0x07);
		date_time->month = read_rtc(0x08);
		date_time->year = read_rtc(0x09);
	}while (k_memcmp(date_time, &last, sizeof(date_time_t)));

	// 状态寄存器2，了解时间格式
	uint8_t status = read_rtc(0x0B);
	if (!(status & 0x04)) {
		// bcd模式，转成二进制模式
		// 例如： 1:59:48 hours = 0x1, minutes = 0x59 = 89, seconds = 0x48
		// 即每个数位用8-4-2-1取值计算
		date_time->second = bcd_to_bin(date_time->second);
		date_time->minute = bcd_to_bin(date_time->minute);
		date_time->hour = bcd_to_bin(date_time->hour);
		date_time->day = bcd_to_bin(date_time->day);
		date_time->month = bcd_to_bin(date_time->month);
		date_time->year = bcd_to_bin(date_time->year);
	}

	// 是否12小时模式，且是下午，是在调整hour的值
	if (!(status & 0x02) && (date_time->hour & 0x80)) {
		date_time->hour = (date_time->hour & 0x7F) + 12;
	}
}

/**
 * 初始化硬件定时器
 */
static void init_pit_for_timer (void) {
    uint32_t reload_count = PIT_OSC_FREQ / (1000.0 / OS_TICK_MS);

    outb(PIT_COMMAND_MODE_PORT, PIT_CHANNLE0 | PIT_ACCESS_LOHIBYTE_ONLY | PIT_MODE3);
    outb(PIT_CHANNLE0_DATA_PORT, reload_count & 0xFF);
    outb(PIT_CHANNLE0_DATA_PORT, (reload_count >> 8) & 0xFF);
}

/**
 * 定时器中断处理函数
 */
void do_handler_timer (exception_frame_t *frame) {

    irq_state_t state = irq_enter_protection();

    sys_tick++;
    timer_time_tick();
    task_time_tick();

    irq_leave_protection(state);
    pic_send_eoi(IRQ0_TIMER);
}

/**
 * 获取系统启动后的时钟节拍
 */
uint32_t time_get_tick (void) {
	return sys_tick;
}

/**
 * 定时器初始化
 */
void rtimer_init (void) {
    sys_tick = 0;

    list_init(&started_list);
    list_init(&free_list);
    for (int i = 0; i < TIMER_MAX_COUNT; i++) {
        list_insert_last(&free_list, &timer[i].node);
    }

    init_pit_for_timer();

    irq_install(IRQ0_TIMER, (irq_handler_t)handler_timer);
    irq_enable(IRQ0_TIMER);
}

/**
 * 分配一个定时器
 * time_out 定时器回调函数，在应用空间中调用
 */
rtimer_t * rtimer_alloc (uint32_t ms, void (*time_out)(rtimer_t *), void * data, int system, task_t * owner) {
	rtimer_t * timer = (rtimer_t *)0;

	if (list_count(&free_list)) {
		irq_state_t state = irq_enter_protection();

		list_node_t * timer_node = list_remove_first(&free_list);
		timer = node_to_parent(timer_node, rtimer_t, node);
		timer->task = owner ? owner : task_current();
		timer->time_out = time_out;
		timer->data = data;
		timer->system = system;
		timer->reload_count = (ms + (OS_TICK_MS - 1))/ OS_TICK_MS;
		timer->curr_tmo = timer->reload_count;
		list_insert_last(&started_list, timer_node);

		irq_leave_protection(state);
	}
    return timer;
}

/**
 * 设置定时器的超时值
 */
void rtimer_set (rtimer_t * timer, uint32_t ms) {
    timer->reload_count = (ms + (OS_TICK_MS - 1))/ OS_TICK_MS;
    timer->curr_tmo = timer->reload_count;
}

/**
 * 释放定时器
 */
void rtimer_free(rtimer_t * timer) {
    irq_state_t state = irq_enter_protection();

    list_remove(&started_list, &timer->node);
    list_insert_last(&free_list, &timer->node);

    irq_leave_protection(state);
}
