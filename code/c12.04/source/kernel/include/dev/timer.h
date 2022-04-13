//
// Created by lishutong on 2021-07-17.
//

#ifndef TIMER_H
#define TIMER_H

#include "comm/types.h"

#define PIT_OSC_FREQ        1193182				// 定时器时钟

// 定时器的寄存器和各项位配置
#define PIT_CHANNLE0_DATA_PORT       0x40
#define PIT_COMMAND_MODE_PORT        0x43

#define PIT_CHANNLE0                 (0 << 6)

#define PIT_LATCH_COUNT              (0 << 4)
#define PIT_ACCESS_LOBYTE_ONLY       (1 << 4)
#define PIT_ACCESS_HIBYTE_ONLY       (2 << 4)
#define PIT_ACCESS_LOHIBYTE_ONLY     (3 << 4)

#define PIT_MODE3                   (3 << 1)
#define PIT_BINARY_MODE             (0 << 0)

void timer_init (void);
uint32_t timer_get_sys_tick (void);

void handler_timer (void);

#endif //OS_TIMER_H
