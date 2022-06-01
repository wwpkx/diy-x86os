/**
 * 终端显示部件
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 * 参考资料：https://wiki.osdev.org/Printing_To_Screen
 */
#include "dev/console.h"

#define CONSOLE_NR          1           // 控制台的数量

static console_t console_buf[CONSOLE_NR];

/**
 * 将光标往前移一个字符
 */
static void move_forward (console_t * console, int n) {
	for (int i = 0; i < n; i++) {
		if (++console->cursor_col >= console->display_cols) {
			console->cursor_col = 0;
            console->cursor_row++;
		}
	}
}

/**
 * 在当前位置显示一个字符
 */
static void show_char(console_t * console, char c) {
    // 每显示一个字符，都进行计算，效率有点低。不过这样直观简单
    int offset = console->cursor_col + console->cursor_row * console->display_cols;

    disp_char_t * p = console->disp_base + offset;
    p->c = c;
    p->foreground = console->foreground;
    p->background = console->background;
    move_forward(console, 1);
}

/**
 * 初始化控制台及键盘
 */
int console_init (void) {
    for (int i = 0; i < CONSOLE_NR; i++) {
        console_t *console = console_buf + i;

        console->disp_base = (disp_char_t *) CONSOLE_DISP_ADDR;
        console->display_cols = CONSOLE_COL_MAX;
        console->display_rows = CONSOLE_ROW_MAX;
        console->cursor_row = 0;
        console->cursor_col = 0;
        console->foreground = COLOR_White;
        console->background = COLOR_Black;
    }

	return 0;
}

/**
 * 实现pwdget作为tty的输出
 * 可能有多个进程在写，注意保护
 */
int console_write (int dev, char * data, int size) {
	console_t * console = console_buf + dev;

    int len;
	for (len = 0; len < size; len++){
        char c = *data++;
        show_char(console, c);
    }
	return len;
}

/**
 * @brief 关闭控制台及键盘
 */
void console_close (int dev) {
	// 似乎不太需要做点什么
}
