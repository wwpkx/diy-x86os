/**
 * 键盘设备处理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "comm/cpu_instr.h"
#include "cpu/irq.h"
#include "dev/kbd.h"
#include "dev/tty.h"

static key_data_t key_data;		// 当前键值
static kbd_state_t kbd_state;	// 键盘状态

/**
 * 更新键盘上状态指示灯
 */
static void update_led_status (void) {
	int data = 0;

	// qemu不支持0xed命令，所以这里的设置看起来没用
	data |= kbd_state.num_lock << 1;
	data |= kbd_state.caps_lock << 0;
    kbd_write(KBD_PORT_CMD, KBD_CMD_RW_LEDS);
    kbd_write(KBD_PORT_DATA, data);
}

/**
 * 键盘映射表，分3类
 * normal是没有shift键按下，或者没有numlock按下时默认的键值
 * func是按下shift或者numlock按下时的键值
 * esc是以esc开头的的键值
 */
static const key_map_t map_table[] = {
		[0x1] = {ASCII_ESC, ASCII_ESC},
		[0x2] = {'1', '!'},
		[0x3] = {'2', '@'},
		[0x4] = {'3', '#'},
		[0x5] = {'4', '$'},
		[0x6] = {'5', '%'},
		[0x7] = {'6', '^'},
		[0x08] = {'7', '&'},
		[0x09] = {'8', '*' },
		[0x0A] = {'9', '('},
		[0x0B] = {'0', ')'},
		[0x0C] = {'-', '_'},
		[0x0D] = {'=', '+'},
		[0x0E] = {'\b', '\b'},
		[0x0F] = {'\t', '\t'},
		[0x10] = {'q', 'Q'},
		[0x11] = {'w', 'W'},
		[0x12] = {'e', 'E'},
		[0x13] = {'r', 'R'},
		[0x14] = {'t', 'T'},
		[0x15] = {'y', 'Y'},
		[0x16] = {'u', 'U'},
		[0x17] = {'i', 'I'},
		[0x18] = {'o', 'O'},
		[0x19] = {'p', 'P'},
		[0x1A] = {'[', '{'},
		[0x1B] = {']', '}'},
		[0x1C] = {'\n', '\n'},
		[0x1D] = {},
		[0x1E] = {'a', 'A'},
		[0x1F] = {'s', 'B'},
		[0x20] = {'d',  'D'},
		[0x21] = {'f', 'F'},
		[0x22] = {'g', 'G'},
		[0x23] = {'h', 'H'},
		[0x24] = {'j', 'J'},
		[0x25] = {'k', 'K'},
		[0x26] = {'l', 'L'},
		[0x27] = {';', ':'},
		[0x28] = {'\'', '|'},
		[0x29] = {'`', '~'},
		[0x2A] = {},
		[0x2B] = {'\\', '|'},
		[0x2C] = {'z', 'Z'},
		[0x2D] = {'x', 'X'},
		[0x2E] = {'c', 'C'},
		[0x2F] = {'v', 'V'},
		[0x30] = {'b', 'B'},
		[0x31] = {'n', 'N'},
		[0x32] = {'m', 'M'},
		[0x33] = {',', '<'},
		[0x34] = {'.', '>'},
		[0x35] = {'/', '?'},
		[0x36] = {},
		[0x37] = {'*', '*'},		// keypad
		[0x38] = {},		// 只有一个ATL，没有右侧的
		[0x39] = {' ', ' '},
		[0x3A] = {},		// KEY_CAPS
		[0x3B] = {KEY_F1,},
		[0x3C] = {KEY_F2,},
		[0x3D] = {KEY_F3,},
		[0x3E] = {KEY_F4,},
		[0x3F] = {KEY_F5,},
		[0x40] = {KEY_F6,},
		[0x41] = {KEY_F7,},
		[0x42] = {KEY_F8,},
		[0x43] = {KEY_F9,},
		[0x44] = {KEY_F10,},
		[0x45] = {},		// KEY_NUMLOCK
		[0x46] = {KEY_SCROLL_LOCK},
		[0x47] = {'7', KEY_HOME,},
		[0x48] = {'8', KEY_CURSOR_UP},
		[0x49] = {'9', KEY_PAGE_UP},
		[0x4A] = {'-', '-'},
		[0x4B] = {'4', KEY_CURSOR_LEFT},
		[0x4C] = {'5', '+'},
		[0x4D] = {'6', KEY_CURSOR_RIGHT},
		[0x4E] = {'+', '+'},
		[0x4F] = {'1', KEY_END},
		[0x50] = {'2', KEY_CURSOR_DOWN},
		[0x51] = {'3', KEY_PAGE_DOWN},
		[0x52] = {'0', KEY_INSERT},
		[0x53] = {'.', KEY_DELETE},
		[0x57] = {KEY_F11,},
		[0x58] = {KEY_F12,},
};

static inline char get_key(uint8_t key_code) {
	return key_code & 0x7F;
}

static inline int is_make_code(uint8_t key_code) {
	return !(key_code & 0x80);
}

/**
 * @brief 写键值
 */
static void write_key (void) {
	int tty = tty_current();
	if (tty >= 0) {
		// 仅在当前有打开tty时才使用，否则数据将简单的丢掉
		tty_in_data(tty, (char *)&key_data, sizeof(key_data));
	}

	key_data.data = 0;
}

/**
 * E0开始的键处理，只处理功能键，其它更长的序列不处理
 */
static void do_e0_key (uint8_t raw_code) {
    int key = get_key(raw_code);			// 去掉最高位
	int is_make = is_make_code(raw_code);	// 按下或释放

	// E0开头，主要是HOME、END、光标移动等功能键
	// 设置一下光标位置，然后直接写入
	switch (key) {
	case KEY_CTRL:		// 右ctrl和左ctrl都是这个值
		key_data.r_ctrl = is_make;  // 仅设置标志位
		break;
	case KEY_ALT:
		key_data.r_alt = is_make;  // 仅设置标志位
		break;
	default:
		// 其它功能键，非小键盘上的控制值
		if (is_make) {
			// 这里的功能同小键盘上的功能功能键一致，所以直接小小键盘上的值即可
			key_data.key_code = map_table[key].func;
			key_data.func = 1;
			write_key();
			break;
		}
	}
}

/**
 * 处理单字符的标准键
 */
static void do_normal_key (uint8_t raw_code) {
    char key = get_key(raw_code);		// 去掉最高位
	int is_make = is_make_code(raw_code);

	// 转换成实际的码
	switch (key) {
	// shift, alt, ctrl键，记录标志位
	case KEY_RSHIFT:
		key_data.r_shift = is_make;  // 仅设置标志位
		break;
	case KEY_LSHIFT:
		key_data.l_shift = is_make;  // 仅设置标志位
		break;
	case KEY_ALT:
		key_data.l_alt = is_make;  // 仅设置标志位
		break;
	case KEY_CTRL:
		key_data.r_ctrl = is_make;  // 仅设置标志位
		break;
	case KEY_NUMLOCK:  // 小键盘
		if (is_make) {
			kbd_state.num_lock = ~kbd_state.num_lock;
			update_led_status();
		}
		break;
	case KEY_CAPS:  // 大小写键，设置大小写状态
		if (is_make) {
			kbd_state.caps_lock = ~kbd_state.caps_lock;
			update_led_status();
		}
		break;
	// 功能键：写入键盘缓冲区，由应用自行决定如何处理
	case KEY_F1:
	case KEY_F2:
	case KEY_F3:
	case KEY_F4:
	case KEY_F5:
	case KEY_F6:
	case KEY_F7:
	case KEY_F8:
	case KEY_F9:
	case KEY_F10:
	case KEY_F11:
	case KEY_F12:
	case KEY_SCROLL_LOCK:
		key_data.key_code = map_table[key].normal;
		key_data.func = 1;
		write_key();
		break;
	// 小键盘处理：要看num_lock
	case KEY_END:
	case KEY_HOME:
	case KEY_PAGE_UP:
	case KEY_PAGE_DOWN:
	case KEY_CURSOR_UP:
	case KEY_CURSOR_DOWN:
	case KEY_CURSOR_RIGHT:
	case KEY_CURSOR_LEFT:
	case KEY_INSERT:
	case KEY_DELETE:
		// 这里要根据当前的numlock来确定是否显示字符或者是使用功能键
		if (is_make) {
			if (kbd_state.num_lock) {
				// 显示数字
				key = map_table[key].normal;
			} else {
				// 功能键
				key_data.func = 1;
				key = map_table[key].func;
			}
			// 记录功能键标志位，然后写入
			write_key();
		}
		break;
	default:
		// 其它字符，则混合shift-ctrl-alt写入
		if (is_make) {
			// 根据shift控制取相应的字符，这里有进行大小写转换或者shif转换
			if (key_data.r_shift || key_data.l_shift) {
				key = map_table[key].func;  // 第2功能
			}else {
				key = map_table[key].normal;  // 第1功能
			}

			// 根据caps再进行一次字母的大小写转换
			if (kbd_state.caps_lock) {
				if ((key >= 'A') && (key <= 'Z')) {
					// 大写转小写
					key += 0x20;
				} else if ((key >= 'a') && (key <= 'z')) {
					// 小写转大小
					key -= 0x20;
				}
			}

			// 最后，不管是否是控制字符，都会被写入
			key_data.key_code = key;
			key_data.func = 0;
			write_key();
		}
		break;
	}
}

void do_handler_keyboard(exception_frame_t *frame) {
    static enum {
    	NORMAL,				// 普通，无e0或e1
		BEGIN_E0,			// 收到e0字符
		BEGIN_E1,			// 收到e1字符
    }recv_state = NORMAL;

	// 检查是否有数据，无数据则退出
	uint8_t status = inb(KBD_PORT_STAT);
	if (!(status & KBD_STAT_RECV_READY)) {
		return;
	}

	// 读取键值
    uint8_t raw_code = inb(KBD_PORT_DATA);

	// 读取完成之后，就可以发EOI，方便后续继续响应键盘中断
	// 否则,键值的处理过程可能略长，将导致中断响应延迟
    pic_send_eoi(IRQ1_KEYBOARD);

    // 实测qemu下收不到E0和E1，估计是没有发出去
    // 方向键、HOME/END等键码和小键盘上发出来的完全一样。不清楚原因
    // 也许是键盘布局的问题？所以，这里就忽略小键盘？
	if (raw_code == KEY_E0) {
		// E0字符
		recv_state = BEGIN_E0;
	} else if (raw_code == KEY_E1) {
		// E1字符，不处理
		recv_state = BEGIN_E1;
	} else {
		switch (recv_state) {
		case NORMAL:
			do_normal_key(raw_code);
			break;
		case BEGIN_E0: // 不处理print scr
			do_e0_key(raw_code);
			recv_state = NORMAL;
			break;
		case BEGIN_E1:  // 不处理pause
			recv_state = NORMAL;
			break;
		}
	}
}

/**
 * 等待可写数据
 */
void kbd_wait_send_ready(void) {
    uint32_t time_out = 100000; 
    while (time_out--) {
        if ((inb(KBD_PORT_STAT) & KBD_STAT_SEND_FULL) == 0) {
            return;
        }
    }
}

/**
 * 等待可用的键盘数据
 */
void kbd_wait_recv_ready(void) {
    uint32_t time_out = 100000;
    while (time_out--) {
        if (inb(KBD_PORT_STAT) & KBD_STAT_RECV_READY) {
            return;
        }
    }
}

/**
 * 向键盘端口写数据
 */
void kbd_write(uint8_t port, uint8_t data) {
    kbd_wait_send_ready();
    outb(port, data);
}

/**
 * 读键盘数据
 */
uint8_t kbd_read(void) {
    kbd_wait_recv_ready();
    return inb(KBD_PORT_DATA);
}

/**
 * 键盘硬件初始化
 */
void kbd_init(void) {
	kbd_write(KBD_PORT_CMD, KBD_CMD_READ_CONFIG);
    uint8_t ack = kbd_read() | (1 << 0);
    kbd_write(KBD_PORT_CMD, KBD_CMD_WRITE_CONFIG);
    kbd_wait_send_ready();
    kbd_write(KBD_PORT_DATA, ack);
    kbd_read();  // 读取ack

    // 设置numlock和caps_lock，全部关闭状态
	key_data.data = 0;
	kbd_state.caps_lock = 0;
	kbd_state.num_lock = 0;
	update_led_status();

    irq_install(IRQ1_KEYBOARD, (irq_handler_t) handler_keyboard);
    irq_enable(IRQ1_KEYBOARD);
}
