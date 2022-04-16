/**
 * help命令处理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <stdio.h>
#include "cli.h"
#include "do_help.h"

/**
 * help命令
 */
int do_help(int argc, char **argv) {
	const cli_cmd_t * start = cli.cmd_start;
	while (start < cli.cmd_end) {
		printf("%s\t\t%s\n",  start->name, start->useage);
		start++;
	}
	return 0;
}
