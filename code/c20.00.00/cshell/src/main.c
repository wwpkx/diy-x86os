/**
 * 内存管理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "cli.h"
#include "do_help.h"
#include "do_clear.h"
#include "do_echo.h"

cli_t cli;

// 命令列表
static const cli_cmd_t cmd_list[] = {
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
        .name = 0,		// 结束标记
    }
};

int main (int argc, char **argv) {
    open("dev/tty/0", 0);

	// 设置c标准输入输出为实时写入，不要采用缓存的方式
	// 这样按下的键能够立即显示在屏幕上。在printf时会逐个字符立即输出
    // 不过，这样的显示速率很慢
	// setvbuf(stdout, (char *)0, _IONBF, 0);

	// 初始化命令行
   	cli_init(&cli, "sh >>", cmd_list);
	for (;;) {
		// 读取字符，然后处理？
		int c = getchar();
		if (c != EOF) {
			cli_in(&cli, c);
		}
	}
}
