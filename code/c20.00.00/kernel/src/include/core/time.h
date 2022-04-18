//
// Created by lishutong on 2021-07-17.
//

#ifndef TIMER_H
#define TIMER_H

#include "core/list.h"
#include "core/types.h"

#define PIT_CHANNLE0_DATA_PORT       0x40
#define PIT_COMMAND_MODE_PORT       0x43

#define PIT_CHANNLE0                (0 << 6)

#define PIT_LATCH_COUNT             (0 << 4)
#define PIT_ACCESS_LOBYTE_ONLY       (1 << 4)
#define PIT_ACCESS_HIBYTE_ONLY       (2 << 4)
#define PIT_ACCESS_LOHIBYTE_ONLY     (3 << 4)

#define PIT_MODE3                   (3 << 1)
#define PIT_BINARY_MODE             (0 << 0)

typedef void (timer_fun_t)(void * arg);

struct _task_t;
typedef struct _rtimer_t {
    list_node_t node;
    uint32_t reload_count;
    uint32_t curr_tmo;

	struct {
		int system : 1;		// 系统用定时器
	};

    struct _task_t * task;
    void (*time_out)(struct _rtimer_t * timer);
    void * data;
}rtimer_t;

void rtimer_init (void);
rtimer_t * rtimer_alloc (uint32_t ms, void (*time_out)(rtimer_t *), void * data, int system, struct _task_t * owner);
void rtimer_free(rtimer_t * timer);
void rtimer_set (rtimer_t * timer, uint32_t ms);

uint32_t time_get_tick (void);
void handler_timer (void);

/**
 * RTC时间值
 */
typedef struct _date_time_t {
	unsigned char second;
	unsigned char minute;
	unsigned char hour;
	unsigned char day;
	unsigned char month;
	unsigned int year;
}date_time_t;

#define	CMOS_RTC_ADDR			0x70
#define	CMOS_RTC_DATA			0x71

void read_time_date(date_time_t * date_time);

#endif //OS_TIMER_H
