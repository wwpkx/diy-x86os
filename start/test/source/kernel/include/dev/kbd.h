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
#define KBD_PORT_CONFIG			0x61
#define KBD_PORT_STAT			0x64
#define KBD_PORT_CMD			0x64

#define KBD_STAT_RECV_READY		(1 << 0)
#define KBD_STAT_SEND_FULL		(1 << 1)

#define KBD_CMD_CONTROLLER		0x60
#define KBD_CONTROLLER_IBMPC	(1 << 6)
#define KBD_CONTROLLER_PCMODE	(1 << 5)

#define KBD_CMD_READ_CONFIG		0x20
#define KBD_CMD_WRITE_CONFIG	0x60
#define KBD_CMD_SEND_MOUSE		0xd4
#define KBD_CMD_ENABLE_MOUSE	0xA8
#define KBD_CMD_RW_LEDS			0xED

/**
 * 键盘扫描码表单元类型
 * 每个按键至多有两个功能键值
 * code1：无shift按下或numlock灯亮的值，即缺省的值
 * code2：shift按下或者number灯灭的值，即附加功能值
 */
typedef struct _key_map_t {
	uint8_t normal;				// 普通功能
	uint8_t func;				// 第二功能
}key_map_t;

#define KEY_E0			0xE0	// E0编码
#define KEY_E1			0xE1	// E1编码
#define	ASCII_ESC		0x1b
#define	ASCII_DEL		0x7F

/**
 * 特殊功能键
 */
#define KEY_CTRL 		0x1D		// E0, 1D或1d
#define KEY_RSHIFT		0x36
#define KEY_LSHIFT 		0x2A
#define KEY_ALT 		0x38		// E0, 38或38

#define	KEY_FUNC		 0x8000
#define KEY_F1			(0x3B)
#define KEY_F2			(0x3C)
#define KEY_F3			(0x3D)
#define KEY_F4			(0x3E)
#define KEY_F5			(0x3F)
#define KEY_F6			(0x40)
#define KEY_F7			(0x41)
#define KEY_F8			(0x42)
#define KEY_F9			(0x43)
#define KEY_F10			(0x44)
#define KEY_F11			(0x57)
#define KEY_F12			(0x58)

#define KEY_NUMLOCK			0x45
#define KEY_CAPS			(0x3A)
#define	KEY_SCROLL_LOCK		(0x46)
#define KEY_HOME			(0x47)
#define KEY_END				(0x4F)
#define KEY_PAGE_UP			(0x49)
#define KEY_PAGE_DOWN		(0x51)
#define KEY_CURSOR_UP		(0x48)
#define KEY_CURSOR_DOWN		(0x50)
#define KEY_CURSOR_RIGHT	(0x4D)
#define KEY_CURSOR_LEFT		(0x4B)
#define KEY_INSERT			(0x52)
#define KEY_DELETE			(0x53)

#define KEY_BACKSPACE		0xE

/**
 * 转换后的按键键码值， 32位键值
 */
typedef union _key_data_t{
	struct {
		int key_code : 8;
		int : 16;
		int : 1;
		int l_alt : 1;
		int r_alt : 1;
		int l_ctrl : 1;
		int r_ctrl : 1;
		int l_shift : 1;
		int r_shift : 1;
		int func : 1;
	}__attribute__((packed));
	int data;
}key_data_t;

/**
 * 状态指示灯
 */
typedef struct _kbd_state_t {
	int caps_lock : 1;			// 大写状态
	int num_lock : 1;			// 数字键盘启用
}kbd_state_t;

void kbd_init(void);
void kbd_wait_send_ready(void);
void kbd_wait_recv_ready(void);
void kbd_write(uint8_t port, uint8_t data);
uint8_t kbd_read(void);

void handler_keyboard (void);

#endif
