/**
 * 终端显示部件
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 * 参考资料：https://wiki.osdev.org/Printing_To_Screen
 */
#include "dev/console.h"
#include "tools/klib.h"
#include "comm/cpu_instr.h"

#define CONSOLE_NR          1           // 控制台的数量

static console_t console_buf[CONSOLE_NR];

/**
 * @brief 读取当前光标的位置
 */
static int read_cursor_pos (void) {
    int pos;

 	outb(0x3D4, 0x0F);		// 写低地址
	pos = inb(0x3D5);
	outb(0x3D4, 0x0E);		// 写高地址
	pos |= inb(0x3D5) << 8;   
    return pos;
}

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
    // 整体上移
    disp_char_t * dest = console->disp_base;
    disp_char_t * src = console->disp_base + console->display_cols * lines;
    uint32_t size = (console->display_rows - lines) * console->display_cols * sizeof(disp_char_t);
    kernel_memcpy(dest, src, size);

    // 擦除最后一行
    erase_rows(console, console->display_rows - lines, console->display_rows - 1);

    console->cursor_row -= lines;
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
            console->cursor_row++;
            if (console->cursor_row >= console->display_rows) {
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
    // 每显示一个字符，都进行计算，效率有点低。不过这样直观简单
    int offset = console->cursor_col + console->cursor_row * console->display_cols;

    disp_char_t * p = console->disp_base + offset;
    p->c = c;
    p->foreground = console->foreground;
    p->background = console->background;
    move_forward(console, 1);
}

static void clear_display (console_t * console) {
    int size = console->display_cols * console->display_rows;

    disp_char_t * start = console->disp_base;
    for (int i = 0; i < size; i++, start++) {
        // 为便于理解，以下分开三步写一个字符，速度慢一些
        start->c = ' ';
        start->background = console->background;
        start->foreground = console->foreground;
    }
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

        int cursor_pos = read_cursor_pos();
        console->cursor_row = cursor_pos / console->display_cols;
        console->cursor_col = cursor_pos % console->display_cols;
        console->foreground = COLOR_White;
        console->background = COLOR_Black;

        // clear_display(console);
        // update_cursor_pos(console);
    }

	return 0;
}

/**
 * 普通状态下的字符的写入处理
 */
int console_write (int dev, char * data, int size) {
	console_t * console = console_buf + dev;

    int len;
	for (len = 0; len < size; len++){
        char c = *data++;
        switch (c) {
            case '\n':
                move_to_col0(console);
                move_next_line(console);
                break;
            default:
                show_char(console, c);
                break;
        }
    }
    update_cursor_pos(console);
    return len;
}

/**
 * @brief 关闭控制台及键盘
 */
void console_close (int dev) {
	// 似乎不太需要做点什么
}
