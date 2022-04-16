/**
 * 终端显示部件
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "dev/console.h"
#include "dev/kbd.h"
#include "tools/klib.h"

static console_t console;			// 控制台结构，整个计算机只有一个

/**
 * 设置光标的可见性
 */
static void set_cursor_visiable (int visiable) {
	// ui_color_t color;
	// if (visiable) {
	// 	color = console.cursor_color;
	// } else {
	// 	color = widget_background((widget_t *)console);
	// }

	// pbwidget_request_draw_rect((pb_widget_t *)console,
	// 		console.cursor_col * console.text_width,
	// 		console.cursor_row * console.text_height,
	// 		console_CURSOR_WIDTH,
	// 		console.text_height,
	// 		color);
	// console.cursor_visiable = visiable;
}


/**
 * 整体屏幕上移若干行
 */
static void scroll_up(int lines) {
	if (console.cursor_row <= 0) {
		return;
	}

	// int dy = lines * console.text_height;
	// pbwidget_request_scroll_up((pb_widget_t *)console, dy);

	console.cursor_row--;
}

static void move_to_col0 (void) {
	console.cursor_col = 0;
}

/**
 * 换至下一行
 */
static void move_next_line (void) {
	console.cursor_row++;

	// 超出当前屏幕显示的所有行，上移一行
	if (console.cursor_row >= console.display_rows) {
		scroll_up(1);
	}
}

/**
 * 将光标往前移一个字符
 */
static void move_forward (int n) {
	for (int i = 0; i < n; i++) {
		if (++console.cursor_col >= console.display_cols) {
			console.cursor_col = 0;
			if (++console.cursor_row >= console.display_rows) {
				// 超出末端，上移
				scroll_up(1);
			}
		}
	}
}

/**
 * 在当前位置显示一个字符
 */
static void show_char(char c) {
	// pb_widget_t *pb_widget = &console.pb_widget;
	// int x = console.cursor_col * console.text_width;
	// int y = console.cursor_row * console.text_height;
	// ui_color_t forground = widget_foreground((widget_t*)pb_widget);
	// pbwidget_request_draw_text(pb_widget, x, y, forground, &c, 1);

	move_forward(1);
}

/**
 * 只支持保存光标
 */
void save_cursor(void) {
	console.old_cursor_col = console.cursor_col;
	console.old_cursor_row = console.cursor_row;
}

void restore_cursor(void) {
	console.cursor_col = console.old_cursor_col;
	console.cursor_row = console.old_cursor_row;
}

/**
 * 光标左移
 */
static int move_backword (int n) {
	int status = -1;

	for (int i = 0; i < n; i++) {
		if (console.cursor_col > 0) {
			// 非列超始处,可回退
			console.cursor_col--;
			status = 0;
		} else if (console.cursor_row > 0) {
			// 列起始处，但非首行，可回腿
			console.cursor_row--;
			console.cursor_col = console.display_cols - 1;
			status = 0;
		}
	}

	return status;
}

/**
 * 擦除前一字符
 * @param console
 */
static void erase_backword (void) {
	if (move_backword(1) == 0) {
		show_char(' ');
		move_backword(1);
	}
}

/**
 * 将光标对齐到8的倍数位置上
 */
static void move_next_tab(void) {
	int col = console.cursor_col;
	col = (col + 7) / 8 * 8;		// 下一显示位置
	if (col >= console.display_cols) {
		col = 0;
	}
	console.cursor_col = col;
}

/**
 * 设置字符属性
 */
static void set_font_style (void) {
	// static const ui_color_t color_table[] = {
	// 		COLOR_Black, COLOR_Red, COLOR_Green, COLOR_Yellow, // 0-3
	// 		COLOR_Blue, COLOR_Magenta, COLOR_Cyan, COLOR_White, // 3-7
	// 		COLOR_White, COLOR_White, // 8 - 9, 最后一项为默认色
	// };

	// for (int i = 0; i < console.curr_param_index; i++) {
	// 	int param = console.esc_param[i];
	// 	if ((param >= 30) && (param <= 37)) {
	// 		widget_set_foreground((widget_t *)console, color_table[param - 30]);
	// 	} else if ((param >= 40) && (param <= 47)) {
	// 		widget_set_background((widget_t *)console, color_table[param - 30]);
	// 	} else if (param == 39) {
	// 		widget_set_foreground((widget_t *)console, COLOR_White);
	// 	} else if (param == 49) {
	// 		widget_set_background((widget_t *)console, COLOR_Black);
	// 	}
	// }
}

/**
 * 擦除字符操作
 */
static void erase_in_display(void) {
	if (console.curr_param_index <= 0) {
		return;
	}

	int param = console.esc_param[0];
	if (param == 2) {
		// 擦除整屏
		// pbwidget_request_draw_rect((pb_widget_t *)console, 0, 0,
		// 		widget_width((widget_t *)console),
		// 		widget_height((widget_t *)console),
		// 		widget_background((widget_t *)console));
	}
}

/**
 * 移动光标
 */
static void move_cursor(void) {
	int row = 0, col = 0;

	if (console.curr_param_index >= 1) {
		row = console.esc_param[0];
	}

	if (console.curr_param_index >= 2) {
		col = console.esc_param[1];
	}

	console.cursor_col = col;
	console.cursor_row = row;
}

/**
 * 光标定时处理
 */
// static void cursor_timer_timeout(rtimer_t * timer) {
// 	console_t * console = (console_t *)timer->data;
// 	set_cursor_visiable(console, ~console.cursor_visiable);
// }

/**
 * 部件初始化
 */
// void console_init(console_t *widget, int width, int height,
// 		widget_t *parent, task_t * owner) {
// 	pb_widget_init((pb_widget_t*) widget, width, height, parent, owner);

// 	// 根据模式计算相应的属性
// 	// 起始从0开始显示
// 	widget->cursor_row = widget->cursor_col = 0;

// 	// 计算屏幕的高度和宽度
// 	ui_font_t *font = ui_get_font();
// 	widget->text_width = font->width;
// 	widget->text_height = font->height;
// 	widget->display_cols = width / widget->text_width;
// 	widget->display_rows = height / widget->text_height;
// 	widget->max_rows = ui_device_height(widget_device((widget_t*) widget));
// 	widget->cursor_color = COLOR_White;
// 	widget->pb_widget.widget.backgroud_color = COLOR_Black;
// 	widget->write_state = TTY_RECV_NORMAL;
// 	widget->curr_param_index = 0;

// 	widget->old_cursor_col = widget->old_cursor_row = 0;

// 	widget_set_use_text_background((widget_t *)widget, 1);
// 	widget_set_foreground((widget_t *)widget, COLOR_White);
// 	widget_set_background((widget_t *)widget, COLOR_Black);

// 	// 光标定时处理？
// 	widget->cursor_timer = rtimer_alloc(TTY_CURSOR_TIMER_INTERVAL, cursor_timer_timeout, widget, 1, owner);
// }

/**
 * 初始化控制台及键盘
 */
int console_init (struct _tty_t * tty) {
	static int need_init = 1;

	if (need_init) {
		kbd_init();
		need_init = 0;
	}

	tty->echo = 1;			// 控制台显示应当回显
	return 0;
}

/**
 * 清空参数表
 */
static void clear_esc_param (void) {
	kernel_memset(console.esc_param, 0, sizeof(console.esc_param));
	console.curr_param_index = 0;
}

/**
 * 实现pwdget作为tty的输出
 * 可能有多个进程在写，注意保护
 * @param tty 输出TTY
 * @return
 */
int console_write (struct _tty_t  * tty) {
	int size = bfifo_count(&tty->out_fifo);

	set_cursor_visiable(0);
	while (size--) {
		// 获取一个字符，然后显示在设备上
		char c;

		bfifo_read(&tty->out_fifo, &c, 1);
		if (console.write_state == TTY_RECV_NORMAL) {
			switch (c) {
			case ASCII_ESC:
				// 进入转义序列模式 ESC 0x20-0x27(多个) 0x30-0x7e
				console.write_state = TTY_RECV_ESC;
				break;
			case ASCII_DEL:  // 删除左侧
				erase_backword();
				break;
			case '\b':		// 左移一格
				move_backword(1);
				break;
				// 换行处理
			case '\r':
				move_to_col0();
				break;
			case '\n':
				move_to_col0();
				move_next_line();
				break;
			case '\t':		// 对齐的下一制表符
				move_next_tab();
				break;
				// 普通字符显示
			default: {
				// 这里一个一个字符显示，效率是比较低的。可以有考虑将数据先累积起来，然后写出
				// 不过目前不用优化，就这样吧
				show_char(c);
				break;
			}
			}
		} else if (console.write_state == TTY_RECV_ESC) {
			// https://blog.csdn.net/ScilogyHunter/article/details/106874395
			// ESC状态处理, 转义序列模式 ESC 0x20-0x27(0或多个) 0x30-0x7e
			switch (c) {
			case '7':		// 保存光标
				save_cursor();
				console.write_state = TTY_RECV_NORMAL;
				break;
			case '8':		// 恢复光标
				restore_cursor();
				console.write_state = TTY_RECV_NORMAL;
				break;
			case '[':
				console.write_state = TTY_RECV_LEFT_BRACE;
				clear_esc_param();
				break;
			default:
				console.write_state = TTY_RECV_NORMAL;
				break;
			}
		} else if (console.write_state == TTY_RECV_LEFT_BRACE) {
			// 接收参数
			if ((c >= '0') && (c <= '9')) {
				int * param = &console.esc_param[console.curr_param_index];
				*param = *param * 10 + c - '0';
			} else if ((c == ';') && console.curr_param_index < ESC_PARAM_MAX) {
				console.curr_param_index++;
			} else {
				// 结束处理，前移
				console.curr_param_index++;

				// 已经接收到所有的字符，继续处理
				switch (c) {
				case 'D':	// 光标左移n个
					move_backword(console.esc_param[0]);
					break;
				case 'a':
				case 'C':
					move_forward(console.esc_param[0]);
					break;
				case 'm': // 设置字符属性
					set_font_style();
					break;
				case 'J':
					erase_in_display();
					break;
				case 'H':
				case 'f':
					move_cursor();
					break;
				default:
					break;

				}
				console.write_state = TTY_RECV_NORMAL;
			}
		}
	}
	set_cursor_visiable(1);
	return 0;
}

/**
 * @brief 关闭控制台及键盘
 */
void console_close (struct _tty_t * tty) {
	// 似乎不太需要做点什么
}
