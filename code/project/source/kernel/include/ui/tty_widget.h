/**
 * 终端显示部件
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef SRC_UI_TTY_WIDGET_H_
#define SRC_UI_TTY_WIDGET_H_

#include <core/os_cfg.h>
#include <core/time.h>
#include <ui/pb_widget.h>

/**
 * 终端显示部件
 */
typedef struct _tty_widget_t {
	pb_widget_t pb_widget;

	enum {
		TTY_RECV_NORMAL,		// 普通模式
		TTY_RECV_ESC,				// 转议序列械
		TTY_RECV_LEFT_BRACE,		// 接收到[
	}write_state;

	int cursor_row, cursor_col;		// 当前编辑的行和列
	int max_rows;					// 最大行数
	int display_rows, display_cols;	// 显示界面的行数和列数
	int text_height, text_width;	// 字符的宽度和高度

	int old_cursor_col, old_cursor_row;	// 保存的光标位置

	rtimer_t * cursor_timer;			// 光标定时器
	ui_color_t cursor_color;		// 光标颜色

	struct {
		int cursor_visiable : 1;	// 光标是否可见
	};

	int esc_param[ESC_PARAM_MAX];		// ESC [ ;;参数数量
	int curr_param_index;
}tty_widget_t;

tty_widget_t * tty_widget_create (int width, int height, widget_t * parent, task_t * task);
void tty_widget_init (tty_widget_t * widget, int width, int height, widget_t * parent, task_t * task);
int tty_wdiget_write (dev_tty_t * tty);

/**
 * 获取显示的列宽，以文本字符数计
 */
static inline int tty_widget_cols (tty_widget_t * widget) {
	return widget->display_cols;
}

#endif /* SRC_UI_TTY_WIDGET_H_ */
