/**
 * clear清屏命令处理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <stdio.h>
#include "do_clear.h"
#include "cli.h"

/**
 * 清屏命令
 */
int do_clear (int argc, char ** argv) {
	printf(ESC_CLEAR_SCREEN);
	printf(ESC_MOVE_CURSOR(0, 0));

	// 回到屏幕原点
	return 0;
}
