/**
 * 终端显示部件
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 * 参考资料：https://wiki.osdev.org/Printing_To_Screen
 */
#include "dev/dev.h"
#include "dev/console.h"
#include "dev/kbd.h"
#include "tools/klib.h"
#include "comm/cpu_instr.h"

static console_t console_buf[1];			// 控制台结构，整个计算机只有一个

/**
 * @brief 更新鼠标的位置
 */
static void update_cursor_pos (console_t * console) {
	uint16_t pos = console->cursor_row *  console->display_cols + console->cursor_col;
 
	outb(0x3D4, 0x0F);		// 写低地址
	outb(0x3D5, (uint8_t) (pos & 0xFF));
	outb(0x3D4, 0x0E);		// 写高地址
	outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

/**
 * @brief 擦除从start到end的行
 */
static void erase_rows (console_t * console, int start, int end) {
	volatile disp_char_t * disp_start = console->disp_base + console->display_cols * start;
	volatile disp_char_t * disp_end = console->disp_base + console->display_cols * (end + 1);

	while (disp_start < disp_end) {
		disp_start->c = ' ';
		disp_start->foreground = console->foreground;
		disp_start->background = console->background;

		disp_start++;
	}
}

/**
 * 整体屏幕上移若干行
 */
static void scroll_up(console_t * console, int lines) {
	if (console->cursor_row <= 0) {
		return;
	}

	// 整体上移
	disp_char_t * dest = console->disp_base;
	disp_char_t * src = dest + console->display_cols;
	uint32_t size = (console->display_rows - 1) * console->display_cols * sizeof(disp_char_t);
	kernel_memcpy(dest, src, size);

	// 擦除最后一行
	erase_rows(console, console->display_rows - 1, console->display_rows - 1);
	
	console->cursor_row--;
}

static void move_to_col0 (console_t * console) {
	console->cursor_col = 0;
}

/**
 * 换至下一行
 */
static void move_next_line (console_t * console) {
	console->cursor_row++;

	// 超出当前屏幕显示的所有行，上移一行
	if (console->cursor_row >= console->display_rows) {
		scroll_up(console, 1);
	}
}

/**
 * 将光标往前移一个字符
 */
static void move_forward (console_t * console, int n) {
	for (int i = 0; i < n; i++) {
		if (++console->cursor_col >= console->display_cols) {
			console->cursor_col = 0;
			if (++console->cursor_row >= console->display_rows) {
				// 超出末端，上移
				scroll_up(console, 1);
			}
		}
	}
}

/**
 * 在当前位置显示一个字符
 */
static void show_char(console_t * console, char c) {
	int offset = console->cursor_col + console->cursor_row * console->display_cols;
	
	disp_char_t * p = console->disp_base + offset;
	p->c = c;
	p->foreground = console->foreground;
	p->background = console->background;
	move_forward(console, 1);
}

/**
 * 只支持保存光标
 */
void save_cursor(console_t * console) {
	console->old_cursor_col = console->cursor_col;
	console->old_cursor_row = console->cursor_row;
}

void restore_cursor(console_t * console) {
	console->cursor_col = console->old_cursor_col;
	console->cursor_row = console->old_cursor_row;
}

/**
 * 光标左移
 */
static int move_backword (console_t * console, int n) {
	int status = -1;

	for (int i = 0; i < n; i++) {
		if (console->cursor_col > 0) {
			// 非列超始处,可回退
			console->cursor_col--;
			status = 0;
		} else if (console->cursor_row > 0) {
			// 列起始处，但非首行，可回腿
			console->cursor_row--;
			console->cursor_col = console->display_cols - 1;
			status = 0;
		}
	}

	return status;
}

/**
 * 擦除前一字符
 * @param console
 */
static void erase_backword (console_t * console) {
	if (move_backword(console, 1) == 0) {
		show_char(console, ' ');
		move_backword(console, 1);
	}
}

/**
 * 将光标对齐到8的倍数位置上
 */
static void move_next_tab(console_t * console) {
	int col = console->cursor_col;
	col = (col + 7) / 8 * 8;		// 下一显示位置
	if (col >= console->display_cols) {
		col = 0;
	}
	console->cursor_col = col;
}

/**
 * 设置字符属性
 */
static void set_font_style (console_t * console) {
	static const cclor_t color_table[] = {
			COLOR_Black, COLOR_Red, COLOR_Green, COLOR_Yellow, // 0-3
			COLOR_Blue, 0, COLOR_Cyan, COLOR_White, // 3-7
			COLOR_White, COLOR_White, // 8 - 9, 最后一项为默认色
	};

	for (int i = 0; i < console->curr_param_index; i++) {
		int param = console->esc_param[i];
		if ((param >= 30) && (param <= 37)) {
			console->foreground = color_table[param - 30];
		} else if ((param >= 40) && (param <= 47)) {
			console->foreground = color_table[param - 30];
		} else if (param == 39) {
			console->foreground = COLOR_White;
		} else if (param == 49) {
			console->background = COLOR_Black;
		}
	}
}

static void clear_display (console_t * console) {
	int size = console->display_cols * console->display_rows * sizeof(disp_char_t);
	
	disp_char_t * start = console->disp_base;
	for (int i = 0; i < console->display_cols * console->display_rows; i++, start++) {
		// 为便于理解，以下分开三步写一个字符，速度慢一些
		start->c = ' ';
		start->background = CONSOLE_BACKGROUND;
		start->foreground = CONSOLE_FORGROUND;
	}
}

/**
 * 擦除字符操作
 */
static void erase_in_display(console_t * console) {
	if (console->curr_param_index <= 0) {
		return;
	}

	int param = console->esc_param[0];
	if (param == 2) {
		// 擦除整个屏幕
		erase_rows(console, 0, console->display_rows);
	}
}

/**
 * 移动光标
 */
static void move_cursor(console_t * console) {
	int row = 0, col = 0;

	if (console->curr_param_index >= 1) {
		row = console->esc_param[0];
	}

	if (console->curr_param_index >= 2) {
		col = console->esc_param[1];
	}

	console->cursor_col = col;
	console->cursor_row = row;
}

/**
 * 初始化控制台及键盘
 */
int console_init (struct _tty_t * tty) {
	static int need_init = 1;

	// 键盘和屏幕只要初始化一次
	if (need_init) {
		kbd_init();
		need_init = 0;
	}

	// 目前只支持一个console的初始化
	console_t * console = console_buf + 0;
	console->disp_base = (disp_char_t *)CONSOLE_VIDEO_BASE;
	console->cursor_row = console->cursor_col = 0;
	console->display_cols = CONSOLE_COL_MAX;
	console->display_rows = CONSOLE_ROW_MAX;
	console->foreground = CONSOLE_FORGROUND;
	console->background = CONSOLE_BACKGROUND;
	console->write_state = TTY_RECV_NORMAL;
	console->curr_param_index = 0;
	
	// 清空显示
	clear_display(console);
	update_cursor_pos(console);
	
	tty->dev_data = (void *)console;
	return 0;
}

/**
 * 清空参数表
 */
static void clear_esc_param (console_t * console) {
	kernel_memset(console->esc_param, 0, sizeof(console->esc_param));
	console->curr_param_index = 0;
}

/**
 * 实现pwdget作为tty的输出
 * 可能有多个进程在写，注意保护
 * @param tty 输出TTY
 * @return
 */
int console_write (struct _tty_t  * tty) {
	int size = bfifo_count(&tty->out_fifo);
	console_t * console = (console_t *)tty->dev_data;

	while (size--) {
		// 获取一个字符，然后显示在设备上
		char c;

		bfifo_read(&tty->out_fifo, &c, 1);
		if (console->write_state == TTY_RECV_NORMAL) {
			switch (c) {
			case ASCII_ESC:
				// 进入转义序列模式 ESC 0x20-0x27(多个) 0x30-0x7e
				console->write_state = TTY_RECV_ESC;
				break;
			case ASCII_DEL:  // 删除左侧
				erase_backword(console);
				break;
			case '\b':		// 删除字符
				erase_backword(console);
				break;
				// 换行处理
			case '\r':
				move_to_col0(console);
				break;
			case '\n':
				move_to_col0(console);
				move_next_line(console);
				break;
			case '\t':		// 对齐的下一制表符
				move_next_tab(console);
				break;
				// 普通字符显示
			default: {
				// 这里一个一个字符显示，效率是比较低的。可以有考虑将数据先累积起来，然后写出
				// 不过目前不用优化，就这样吧
				show_char(console, c);
				break;
			}
			}
		} else if (console->write_state == TTY_RECV_ESC) {
			// https://blog.csdn.net/ScilogyHunter/article/details/106874395
			// ESC状态处理, 转义序列模式 ESC 0x20-0x27(0或多个) 0x30-0x7e
			switch (c) {
			case '7':		// 保存光标
				save_cursor(console);
				console->write_state = TTY_RECV_NORMAL;
				break;
			case '8':		// 恢复光标
				restore_cursor(console);
				console->write_state = TTY_RECV_NORMAL;
				break;
			case '[':
				console->write_state = TTY_RECV_LEFT_BRACE;
				clear_esc_param(console);
				break;
			default:
				console->write_state = TTY_RECV_NORMAL;
				break;
			}
		} else if (console->write_state == TTY_RECV_LEFT_BRACE) {
			// 接收参数
			if ((c >= '0') && (c <= '9')) {
				int * param = &console->esc_param[console->curr_param_index];
				*param = *param * 10 + c - '0';
			} else if ((c == ';') && console->curr_param_index < ESC_PARAM_MAX) {
				console->curr_param_index++;
			} else {
				// 结束处理，前移
				console->curr_param_index++;

				// 已经接收到所有的字符，继续处理
				switch (c) {
				case 'D':	// 光标左移n个
					move_backword(console, console->esc_param[0]);
					break;
				case 'a':
				case 'C':
					move_forward(console, console->esc_param[0]);
					break;
				case 'm': // 设置字符属性
					set_font_style(console);
					break;
				case 'J':
					erase_in_display(console);
					break;
				case 'H':
				case 'f':
					move_cursor(console);
					break;
				default:
					break;

				}
				console->write_state = TTY_RECV_NORMAL;
			}
		}
	}
	update_cursor_pos(console);
	return 0;
}

/**
 * @brief 关闭控制台及键盘
 */
void console_close (struct _tty_t * tty) {
	// 似乎不太需要做点什么
}
