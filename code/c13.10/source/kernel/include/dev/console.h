/**
 * 终端显示部件
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef CONSOLE_H
#define CONSOLE_H

#include "dev/tty.h"

#define	ESC_PARAM_MAX				10		// 最多支持的ESC [ 参数数量

/**
 * 终端显示部件
 */
typedef struct _console_t {
	enum {
		TTY_RECV_NORMAL,			// 普通模式
		TTY_RECV_ESC,				// 转议序列械
		TTY_RECV_LEFT_BRACE,		// 接收到[
	}write_state;

	int cursor_row, cursor_col;		// 当前编辑的行和列
	int display_rows, display_cols;	// 显示界面的行数和列数
	int max_rows;					// 最大行数

	int old_cursor_col, old_cursor_row;	// 保存的光标位置

	struct {
		int cursor_visiable : 1;	// 光标是否可见
	};

	int esc_param[ESC_PARAM_MAX];	// ESC [ ;;参数数量
	int curr_param_index;
}console_t;

int console_init (struct _tty_t * tty);
int console_write (struct _tty_t  * tty);
void console_close (struct _tty_t * tty);

#endif /* SRC_UI_TTY_WIDGET_H_ */
