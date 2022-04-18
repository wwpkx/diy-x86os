/**
 * 命令行实现
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <lib_ui.h>
#include "cli.h"

/**
 * 显示命令行提示符
 */
static void show_promot(cli_t * cli) {
    printf("%s", cli->promot);
}

/**
 * 删除前一字符
 */
static void do_bs_key (cli_t * cli) {
	// 左侧无，退出
	if (cli->curr_cursor == 0) {
		return;
	}

	if (cli->curr_cursor == cli->curr_count) {
		// 内部删除一个字符
		cli->curr_cursor--;
		cli->curr_count--;
		cli->curr_input[cli->curr_cursor] = '\0';

		// tty回退
		printf("\b"ESC_SAVE_CURSOR" "ESC_RESTORE_CURSOR);
	} else {
		// 将currsor位置以及其后面的字符全部给前一过来
		int move_count = cli->curr_count - cli->curr_cursor;
		cli->curr_cursor--;
		memmove(cli->curr_input + cli->curr_cursor,
				cli->curr_input + cli->curr_cursor + 1,
				move_count);

		// 虽然长度缩短，但是还是保留一个空格，这样才能清空之前所写
		cli->curr_input[cli->curr_count] = '\0';
		cli->curr_count--;
		cli->curr_input[cli->curr_count] = ' ';

		// 控制台上也要同样的处理，将输入给处理一下b
		printf("\b"ESC_SAVE_CURSOR"%s"ESC_RESTORE_CURSOR, cli->curr_input + cli->curr_cursor);
	}
}

/**
 * tab键处理
 */
static void do_tab_key (cli_t * cli) {
	int tab_count;

	 // 自己要保证前面总是提示符开头，否则就不能用cli->promot_len，必须知道当前位置
	// 要考虑右侧是否有字符，如果有则需要移动
	int col_in_disp = (cli->curr_cursor + cli->promot_len) % cli->cols_count;  // 屏幕中的列
	int next_col = (col_in_disp + 7) / 8 * 8;	// 下一在屏幕中的列

	// 已经在8边界对齐，则直接往前挪一个8位
	if (next_col == col_in_disp) {
		next_col += 8;
	}

	// 计算移动量，不管最后是否越过边界，移动量都是这么多
	if (next_col < cli->cols_count) {
		tab_count = next_col - col_in_disp;  // 填充的空白字符量或者移动的量
	} else {
		tab_count = cli->cols_count - col_in_disp;
	}

	// 先将所有右侧的字符移动，如果有越界，则不移动。从末端开始移动，直到当前光标处
	int move_count = cli->curr_count - cli->curr_cursor;
	if (move_count + cli->curr_count >= CLI_CURRENT_INPUT_MAX_SIZE) {
		move_count = CLI_CURRENT_INPUT_MAX_SIZE - cli->curr_count;
	}
	if (move_count) {
		memmove(cli->curr_input + cli->curr_cursor + tab_count,
				cli->curr_input + cli->curr_cursor,
				move_count);
	}

	// 填入tab键所需要的空白字符
	int old_cursor = cli->curr_cursor;
	for (int i = 0; i < tab_count; i++) {
//		cli->curr_input[cli->curr_cursor++] = '1' + i;		// 调试用
		cli->curr_input[cli->curr_cursor++] = ' ';
	}

	cli->curr_count += tab_count;
	if (cli->curr_count > CLI_CURRENT_INPUT_MAX_SIZE) {
		cli->curr_count = CLI_CURRENT_INPUT_MAX_SIZE;
	}

	if (cli->curr_cursor > CLI_CURRENT_INPUT_MAX_SIZE) {
		cli->curr_cursor = CLI_CURRENT_INPUT_MAX_SIZE;
	}

	// 添加末尾的0值，保险起见
	cli->curr_input[cli->curr_count] = '\0';

	// 从原光标位置处重新显示整行
	printf(ESC_SAVE_CURSOR"%s"ESC_RESTORE_CURSOR, cli->curr_input + old_cursor);

	// 光标需要前移
    printf("\033[%dC", tab_count);
}

/**
 * 显示普通字符
 */
static void show_char(cli_t * cli, char key) {
	if (cli->curr_count >= CLI_CURRENT_INPUT_MAX_SIZE) {
		return;
	}

	// 以下是插入模式，如果想做成覆盖模式，直接写入即可，不用加这么多判断
	// 普通可打印字符，直接显示
	if (cli->curr_count == cli->curr_cursor) {
		// 追加写入，可简单处理
		cli->curr_count++;
		cli->curr_input[cli->curr_cursor++] = key;
		putchar(key);
	} else {
		// 先将所有右侧的字符移动，如果有越界，则不移动。从末端开始移动，直到当前光标处
		int move_count = cli->curr_count - cli->curr_cursor;
		if (move_count + cli->curr_count >= CLI_CURRENT_INPUT_MAX_SIZE) {
			move_count = CLI_CURRENT_INPUT_MAX_SIZE - cli->curr_count;
		}

		// 右移一个单元，用于插入新字符
		memmove(cli->curr_input + cli->curr_cursor + 1,
				cli->curr_input + cli->curr_cursor,
				move_count);
		int old_cursor = cli->curr_cursor;
		cli->curr_input[cli->curr_cursor++] = key;		// 调试用
		cli->curr_count ++;

		// 从原光标位置处重新显示整行
		cli->curr_input[cli->curr_count] = '\0';
		printf(ESC_SAVE_CURSOR"%s"ESC_RESTORE_CURSOR, cli->curr_input + old_cursor);

		// 光标需要前移
	    printf(ESC_CURSOR_MOV_RIGHT);
	}
}

/**
 * 回车键处理
 */
static void do_enter_key (cli_t * cli) {
	putchar('\n');

	// 对命令行进行预处理，形成字符串
	if (cli->curr_cursor >= CLI_CURRENT_INPUT_MAX_SIZE) {
		cli->curr_cursor = CLI_CURRENT_INPUT_MAX_SIZE - 1;
	}
	cli->curr_input[cli->curr_cursor] = '\0';

	char * argv[CLI_MAX_ARG_COUNT];
	int argc = 0;

	// 提取出命令，找命令表
	const char * space = " ";  // 字符分割器
	char *token = strtok(cli->curr_input, space);
	while (token) {
		//		puts(token);   // 调试用
		// 记录参数
		argv[argc++] = token;

		// 先获取下一位置
        token = strtok(NULL, space);
	}

	// 有输入就检查执行
	if (argc > 0) {
		const cli_cmd_t * cmd;

		for (cmd = cli->cmd_start; cmd < cli->cmd_end; cmd++) {
			// 找匹配的命令
			if (strcmp(cmd->name, argv[0]) != 0) {
				continue;
			}

			// 找到命令行，执行命令
	//		printf("%s\n", cmd->name);		// 调试用
			int ret = cmd->do_func(argc, argv);
			if (ret < 0) {
				fprintf(stderr,ESC_COLOR_ERROR"error: %d\n"ESC_COLOR_DEFAULT, ret);
			}
			break;
		}

		// 找不到提示错误
		if (!cmd->do_func) {
			fprintf(stderr, ESC_COLOR_ERROR"Unknown command: %s\n"ESC_COLOR_DEFAULT, cli->curr_input);
		}
	}

  	// 重设显示
	cli->curr_cursor = 0;
	cli->curr_count = 0;
    show_promot(cli);
    memset(cli->curr_input, 0, CLI_CURRENT_INPUT_MAX_SIZE);
}

/**
 * 右移光标
 */
static void do_move_right(cli_t * cli) {
	// 右边有字符才能右移
	if (cli->curr_cursor < cli->curr_count) {
		cli->curr_cursor++;
	    printf(ESC_CURSOR_MOV_RIGHT);
	}
}

/**
 * 左移光标
 */
static void do_move_left(cli_t * cli) {
	// 左边有字符才能左移
	if (cli->curr_cursor > 0) {
		cli->curr_cursor--;
		printf("\b");
	}
}

/**
 * 命令行初始化
 */
void cli_init(cli_t * cli, const char * promot, const cli_cmd_t * cmd_list, ui_widget_t * widget) {
    cli->promot = promot;
    cli->promot_len = strlen(cli->promot);

    // 清空输入
    memset(cli->curr_input, 0, CLI_CURRENT_INPUT_MAX_SIZE);
    cli->curr_cursor = cli->curr_count = 0;

    // 初始化命令表
    const cli_cmd_t * cmd = cmd_list;
    cli->cmd_start = cmd_list;
    while (cmd->name != 0) {
        cmd++;
    }
    cli->cmd_end = cmd;

    // 显示设备相关
    cli->widget = widget;
    cli->cols_count = ui_tty_widget_cols(cli->widget);

    // 显示提示符，开始工作
    show_promot(cli);
}

/**
 * 输入字符处理
 */
void cli_in(cli_t * cli, key_data_t key) {
	char key_code = key.key_code;

	if (key.func) {
		// 功能键
		switch (key_code) {
		case KEY_CURSOR_RIGHT:
			do_move_right(cli);
			break;
		case KEY_CURSOR_LEFT:
			do_move_left(cli);
			break;
		}
	} else {
		// 检查是否可打印字符，是在加入并回显
		if (isprint(key_code)) {
			show_char(cli, key_code);
		} else if (iscntrl(key_code)) {
			switch (key_code) {
			case '\t':
				do_tab_key(cli);
				break;
			case '\n':
				do_enter_key(cli);
				break;
			case '\b':
				do_bs_key(cli);
				break;
			default:
				break;
			}
		}
	}
}

/**
 * 结束命令行
 */
void cli_end(cli_t * cli) {

}
