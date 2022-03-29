/**
 * 终端显示部件
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <core/klib.h>
#include <core/time.h>
#include <ui/ui_core.h>
#include <ui/tty_widget.h>
#include <ui/pb_widget.h>
#include <ui/ui_font.h>

static tty_widget_t win_buffer[3];
static list_t win_list;

/**
 * 设置光标的可见性
 */
static void set_cursor_visiable (tty_widget_t *tty_widget, int visiable) {
	ui_color_t color;
	if (visiable) {
		color = tty_widget->cursor_color;
	} else {
		color = widget_background((widget_t *)tty_widget);
	}

	pbwidget_request_draw_rect((pb_widget_t *)tty_widget,
			tty_widget->cursor_col * tty_widget->text_width,
			tty_widget->cursor_row * tty_widget->text_height,
			TTY_WIDGET_CURSOR_WIDTH,
			tty_widget->text_height,
			color);
	tty_widget->cursor_visiable = visiable;
}


/**
 * 整体屏幕上移若干行
 */
static void scroll_up(tty_widget_t *tty_widget, int lines) {
	if (tty_widget->cursor_row <= 0) {
		return;
	}

	int dy = lines * tty_widget->text_height;
	pbwidget_request_scroll_up((pb_widget_t *)tty_widget, dy);

	tty_widget->cursor_row--;
}

static void move_to_col0 (tty_widget_t * tty_widget) {
	tty_widget->cursor_col = 0;
}

/**
 * 换至下一行
 */
static void move_next_line (tty_widget_t * tty_widget) {
	tty_widget->cursor_row++;

	// 超出当前屏幕显示的所有行，上移一行
	if (tty_widget->cursor_row >= tty_widget->display_rows) {
		scroll_up(tty_widget, 1);
	}
}

/**
 * 将光标往前移一个字符
 */
static void move_forward (tty_widget_t *tty_widget, int n) {
	for (int i = 0; i < n; i++) {
		if (++tty_widget->cursor_col >= tty_widget->display_cols) {
			tty_widget->cursor_col = 0;
			if (++tty_widget->cursor_row >= tty_widget->display_rows) {
				// 超出末端，上移
				scroll_up(tty_widget, 1);
			}
		}
	}
}

/**
 * 在当前位置显示一个字符
 */
static void show_char(tty_widget_t *tty_widget, char c) {
	pb_widget_t *pb_widget = &tty_widget->pb_widget;
	int x = tty_widget->cursor_col * tty_widget->text_width;
	int y = tty_widget->cursor_row * tty_widget->text_height;
	ui_color_t forground = widget_foreground((widget_t*)pb_widget);
	pbwidget_request_draw_text(pb_widget, x, y, forground, &c, 1);

	move_forward(tty_widget, 1);
}

/**
 * 只支持保存光标
 */
void save_cursor(tty_widget_t * tty_widget) {
	tty_widget->old_cursor_col = tty_widget->cursor_col;
	tty_widget->old_cursor_row = tty_widget->cursor_row;
}

void restore_cursor(tty_widget_t * tty_widget) {
	tty_widget->cursor_col = tty_widget->old_cursor_col;
	tty_widget->cursor_row = tty_widget->old_cursor_row;
}


/**
 * 光标左移
 */
static int move_backword (tty_widget_t *tty_widget, int n) {
	int status = -1;

	for (int i = 0; i < n; i++) {
		if (tty_widget->cursor_col > 0) {
			// 非列超始处,可回退
			tty_widget->cursor_col--;
			status = 0;
		} else if (tty_widget->cursor_row > 0) {
			// 列起始处，但非首行，可回腿
			tty_widget->cursor_row--;
			tty_widget->cursor_col = tty_widget->display_cols - 1;
			status = 0;
		}
	}

	return status;
}

/**
 * 擦除前一字符
 * @param tty_widget
 */
static void erase_backword (tty_widget_t *tty_widget) {
	if (move_backword(tty_widget, 1) == 0) {
		show_char(tty_widget, ' ');
		move_backword(tty_widget, 1);
	}
}

/**
 * 将光标对齐到8的倍数位置上
 */
static void move_next_tab(tty_widget_t *tty_widget) {
	int col = tty_widget->cursor_col;
	col = (col + 7) / 8 * 8;		// 下一显示位置
	if (col >= tty_widget->display_cols) {
		col = 0;
	}
	tty_widget->cursor_col = col;
}

/**
 * 设置字符属性
 */
static void set_font_style (tty_widget_t *tty_widget) {
	static const ui_color_t color_table[] = {
			COLOR_Black, COLOR_Red, COLOR_Green, COLOR_Yellow, // 0-3
			COLOR_Blue, COLOR_Magenta, COLOR_Cyan, COLOR_White, // 3-7
			COLOR_White, COLOR_White, // 8 - 9, 最后一项为默认色
	};

	for (int i = 0; i < tty_widget->curr_param_index; i++) {
		int param = tty_widget->esc_param[i];
		if ((param >= 30) && (param <= 37)) {
			widget_set_foreground((widget_t *)tty_widget, color_table[param - 30]);
		} else if ((param >= 40) && (param <= 47)) {
			widget_set_background((widget_t *)tty_widget, color_table[param - 30]);
		} else if (param == 39) {
			widget_set_foreground((widget_t *)tty_widget, COLOR_White);
		} else if (param == 49) {
			widget_set_background((widget_t *)tty_widget, COLOR_Black);
		}
	}
}

/**
 * 擦除字符操作
 */
static void erase_in_display(tty_widget_t * tty_widget) {
	if (tty_widget->curr_param_index <= 0) {
		return;
	}

	int param = tty_widget->esc_param[0];
	if (param == 2) {
		// 擦除整屏
		pbwidget_request_draw_rect((pb_widget_t *)tty_widget, 0, 0,
				widget_width((widget_t *)tty_widget),
				widget_height((widget_t *)tty_widget),
				widget_background((widget_t *)tty_widget));
	}
}

/**
 * 移动光标
 */
static void move_cursor(tty_widget_t * tty_widget) {
	int row = 0, col = 0;

	if (tty_widget->curr_param_index >= 1) {
		row = tty_widget->esc_param[0];
	}

	if (tty_widget->curr_param_index >= 2) {
		col = tty_widget->esc_param[1];
	}

	tty_widget->cursor_col = col;
	tty_widget->cursor_row = row;
}

/**
 * 光标定时处理
 */
static void cursor_timer_timeout(rtimer_t * timer) {
	tty_widget_t * tty_widget = (tty_widget_t *)timer->data;
	set_cursor_visiable(tty_widget, ~tty_widget->cursor_visiable);
}

/**
 * 部件初始化
 */
void tty_widget_init(tty_widget_t *widget, int width, int height,
		widget_t *parent, task_t * owner) {
	pb_widget_init((pb_widget_t*) widget, width, height, parent, owner);

	// 根据模式计算相应的属性
	// 起始从0开始显示
	widget->cursor_row = widget->cursor_col = 0;

	// 计算屏幕的高度和宽度
	ui_font_t *font = ui_get_font();
	widget->text_width = font->width;
	widget->text_height = font->height;
	widget->display_cols = width / widget->text_width;
	widget->display_rows = height / widget->text_height;
	widget->max_rows = ui_device_height(widget_device((widget_t*) widget));
	widget->cursor_color = COLOR_White;
	widget->pb_widget.widget.backgroud_color = COLOR_Black;
	widget->write_state = TTY_RECV_NORMAL;
	widget->curr_param_index = 0;

	widget->old_cursor_col = widget->old_cursor_row = 0;

	widget_set_use_text_background((widget_t *)widget, 1);
	widget_set_foreground((widget_t *)widget, COLOR_White);
	widget_set_background((widget_t *)widget, COLOR_Black);

	// 光标定时处理？
	widget->cursor_timer = rtimer_alloc(TTY_CURSOR_TIMER_INTERVAL, cursor_timer_timeout, widget, 1, owner);
}

/**
 * 创建一个tty部件
 */
tty_widget_t* tty_widget_create(int width, int height, widget_t *parent, task_t * owner) {
	// 临时空间分配使用
	if (list_count(&win_list) == 0) {
		list_init(&win_list);

		for (int i = 0; i < sizeof(tty_widget_t) / sizeof(tty_widget_t); i++) {
			list_insert_first(&win_list, (list_node_t*) (win_buffer + i));
		}
	}

	// 取结点，初始化各项数据
	tty_widget_t *widget = (tty_widget_t*) list_remove_first(&win_list);
	tty_widget_init(widget, width, height, parent, owner);
	return widget;
}

/**
 * 清空参数表
 */
static void clear_esc_param (tty_widget_t * tty_widget) {
	k_memset(tty_widget->esc_param, 0, sizeof(tty_widget->esc_param));
	tty_widget->curr_param_index = 0;
}

/**
 * 实现pwdget作为tty的输出
 * 可能有多个进程在写，注意保护
 * @param tty 输出TTY
 * @return
 */
int tty_wdiget_write(dev_tty_t *tty) {
	tty_widget_t *tty_widget = (tty_widget_t*) tty->device_data;
	int size = bfifo_count(&tty->out_fifo);

	set_cursor_visiable(tty_widget, 0);
	while (size--) {
		// 获取一个字符，然后显示在设备上
		char c = bfifo_read_c(&tty->out_fifo);

		if (tty_widget->write_state == TTY_RECV_NORMAL) {
			switch (c) {
			case ASCII_ESC:
				// 进入转义序列模式 ESC 0x20-0x27(多个) 0x30-0x7e
				tty_widget->write_state = TTY_RECV_ESC;
				break;
			case ASCII_DEL:  // 删除左侧
				erase_backword(tty_widget);
				break;
			case '\b':		// 左移一格
				move_backword(tty_widget, 1);
				break;
				// 换行处理
			case '\r':
				move_to_col0(tty_widget);
				break;
			case '\n':
				move_to_col0(tty_widget);
				move_next_line(tty_widget);
				break;
			case '\t':		// 对齐的下一制表符
				move_next_tab(tty_widget);
				break;
				// 普通字符显示
			default: {
				// 这里一个一个字符显示，效率是比较低的。可以有考虑将数据先累积起来，然后写出
				// 不过目前不用优化，就这样吧
				show_char(tty_widget, c);
				break;
			}
			}
		} else if (tty_widget->write_state == TTY_RECV_ESC) {
			// https://blog.csdn.net/ScilogyHunter/article/details/106874395
			// ESC状态处理, 转义序列模式 ESC 0x20-0x27(0或多个) 0x30-0x7e
			switch (c) {
			case '7':		// 保存光标
				save_cursor(tty_widget);
				tty_widget->write_state = TTY_RECV_NORMAL;
				break;
			case '8':		// 恢复光标
				restore_cursor(tty_widget);
				tty_widget->write_state = TTY_RECV_NORMAL;
				break;
			case '[':
				tty_widget->write_state = TTY_RECV_LEFT_BRACE;
				clear_esc_param(tty_widget);
				break;
			default:
				tty_widget->write_state = TTY_RECV_NORMAL;
				break;
			}
		} else if (tty_widget->write_state == TTY_RECV_LEFT_BRACE) {
			// 接收参数
			if ((c >= '0') && (c <= '9')) {
				int * param = &tty_widget->esc_param[tty_widget->curr_param_index];
				*param = *param * 10 + c - '0';
			} else if ((c == ';') && tty_widget->curr_param_index < ESC_PARAM_MAX) {
				tty_widget->curr_param_index++;
			} else {
				// 结束处理，前移
				tty_widget->curr_param_index++;

				// 已经接收到所有的字符，继续处理
				switch (c) {
				case 'D':	// 光标左移n个
					move_backword(tty_widget, tty_widget->esc_param[0]);
					break;
				case 'a':
				case 'C':
					move_forward(tty_widget, tty_widget->esc_param[0]);
					break;
				case 'm': // 设置字符属性
					set_font_style(tty_widget);
					break;
				case 'J':
					erase_in_display(tty_widget);
					break;
				case 'H':
				case 'f':
					move_cursor(tty_widget);
					break;
				default:
					break;

				}
				tty_widget->write_state = TTY_RECV_NORMAL;
			}
		}
	}
	set_cursor_visiable(tty_widget, 1);
	return 0;
}

