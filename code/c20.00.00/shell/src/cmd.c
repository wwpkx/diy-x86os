/**
 * 命令行实现
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <stdio.h>
#include <stdlib.h>
#include "cli.h"
#include "do_help.h"
#include "do_clear.h"
#include "do_echo.h"
#include "do_cat.h"
#include "do_dir.h"

#define	SHELL_WIDTH			500		// 终端宽度
#define	SHELL_HEIGHT		600		// 终端高度

cli_t cli;

/**
 * 命令列表
 */
const cli_cmd_t cmd_list[] = {
    {
        .name = "help",
		.useage = "help -- list support command",
		.do_func = do_help,
    },
    {
        .name = "clear",
		.useage = "clear -- clear the screen",
		.do_func = do_clear,
    },
	{
		.name = "echo",
		.useage = "echo [-n count] msg  -- echo something",
		.do_func = do_echo,
	},
	{
		.name = "cat",
		.useage = "cat file  -- display file content",
		.do_func = do_cat,
	},
	{
		.name = "dir",
		.useage = "dir -- list current directory",
		.do_func = do_dir,
	},
	{
        .name = 0,		// 结束标记
    }
};

int main (void) {
	// 创建一个窗口，中间包含一个终端输出窗口
	ui_widget_t * win = ui_window_create("CMD",
			SHELL_WIDTH, SHELL_HEIGHT, (ui_widget_t *) 0);
	ui_widget_t * container = ui_window_container(win);
	ui_widget_t tty_widget = ui_tty_widget_create(
			ui_widget_content_width(container),
			ui_widget_content_height(container),
			(ui_widget_t)0);
	ui_window_add(win, tty_widget);
	ui_widget_set_visiable(win, 1);

	// 关联tty窗口和进程自己的stdout
	sys_ioctl(1, TTY_SET_DRIVER_DATA, tty_widget);

	// 设置c标准输入输出为实时写入，不要采用缓存的方式
	// 这样按下的键能够立即显示在屏幕上
	setvbuf(stdout, (char *)0, _IONBF, 0);

	// 转义字符测试
#if 0
    // insert code here...
    printf("Hello, World!Hello, World!Hello, World!\n");  // 普通：输出Hello, World!
    // 输出1    12    123    1234    12345    123456    1234567
    printf("1\t12\t123\t1234\t12345\t123456\t1234567\t\n");

    printf("a23\b45\n");    // \b 输出a245
    printf("\0337Hello,word!\0338123\n");  // ESC 7,8 输出123lo,word!
    printf("\033[sHello,word!\033[u123\n");  // 输出123lo,word!, 没用！！！
    printf("\033[31mHello,word!\033[32m123\n");  // ESC [pn m, Hello,world红色，其余绿色
    printf("123\033[2DHello,word!\n");  // 光标左移，1Hello,word!
    printf("123\033[2CHello,word!\n");  // 光标右移，123  Hello,word!
    printf("232\033[2J");  // 清屏
    printf("Hello, World!Hello, World!Hello, World!\n");  // 普通：输出:有空格   Hello, World!
#endif
	// 提示符，先写死循环吧
    cli_init(&cli, ">>", cmd_list, tty_widget);
	for (;;) {
		app_msg_t msg;

		sys_get_event(&msg);
		if (msg.type == APP_MSG_TYPE_KEYBOARD) {
			// 按键消息，命令行处理
		    cli_in(&cli, msg.key);
		}
	}

    cli_end(&cli);
	return 0;
}

