/**
 * 终端显示部件
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 * 
 * 只支持VGA模式
 */
#ifndef CONSOLE_H
#define CONSOLE_H

// https://wiki.osdev.org/Printing_To_Screen
#define CONSOLE_DISP_ADDR           0xb8000
#define CONSOLE_DISP_END			(0xb8000 + 32*1024)	// 显存的结束地址
#define CONSOLE_ROW_MAX				25			// 行数
#define CONSOLE_COL_MAX				80			// 最大列数

/**
 * @brief 显示字符
 */
typedef struct _disp_char_t {
	uint16_t v;
}disp_char_t;

/**
 * 终端显示部件
 */
typedef struct _console_t {
	disp_char_t * disp_base;	// 显示基地址
    int display_rows, display_cols;	// 显示界面的行数和列数
}console_t;

int console_init (void);
int console_write (int dev, char * data, int size);
void console_close (int dev);

#endif /* SRC_UI_TTY_WIDGET_H_ */
