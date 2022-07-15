/**
 * 键盘设备处理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef KBD_H
#define KBD_H

#include "comm/types.h"

// https://wiki.osdev.org/%228042%22_PS/2_Controller
#define KBD_PORT_DATA			0x60
#define KBD_PORT_STAT			0x64
#define KBD_PORT_CMD			0x64

#define KBD_STAT_RECV_READY		(1 << 0)
#define KBD_STAT_SEND_FULL		(1 << 1)

void kbd_init(void);

void exception_handler_kbd (void);

#endif
