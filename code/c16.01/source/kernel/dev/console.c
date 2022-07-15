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
 * 初始化控制台及键盘
 */
int console_init (void) {
    for (int i = 0; i < CONSOLE_NR; i++) {
        console_t *console = console_buf + i;

        console->display_cols = CONSOLE_COL_MAX;
        console->display_rows = CONSOLE_ROW_MAX;
        console->disp_base = (disp_char_t *)CONSOLE_DISP_ADDR + i * (CONSOLE_COL_MAX * CONSOLE_ROW_MAX);
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
    }
	return len;
}

/**
 * @brief 关闭控制台及键盘
 */
void console_close (int dev) {
	// 似乎不太需要做点什么
}
