/**
 * 简单的命令行解释器
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <stdio.h>
#include <string.h>
#include "lib_syscall.h"
#include "main.h"

static cli_t cli;
static const char * promot = "sh >>";       // 命令行提示符

/**
 * 显示命令行提示符
 */
static void show_promot(void) {
    printf("%s", cli.promot);
    fflush(stdout);
}

/**
 * help命令
 */
static int do_help(int argc, char **argv) {
    const cli_cmd_t * start = cli.cmd_start;

    // 循环打印名称及用法
    while (start < cli.cmd_end) {
        printf("%s %s\n",  start->name, start->useage);
        start++;
    }
    return 0;
}

// 命令列表
static const cli_cmd_t cmd_list[] = {
    {
        .name = "help",
		.useage = "help -- list support command",
		.do_func = do_help,
    },
};

/**
 * 命令行初始化
 */
static void cli_init(const char * promot, const cli_cmd_t * cmd_list, int cnt) {
    cli.promot = promot;
    
    memset(cli.curr_input, 0, CLI_INPUT_SIZE);
    
    cli.cmd_start = cmd_list;
    cli.cmd_end = cmd_list + cnt;
}

/**
 * 在内部命令中搜索
 */
static const cli_cmd_t * find_builtin (const char * name) {
    for (const cli_cmd_t * cmd = cli.cmd_start; cmd < cli.cmd_end; cmd++) {
        if (strcmp(cmd->name, name) != 0) {
            continue;
        }

        return cmd;
    }

    return (const cli_cmd_t *)0;
}

/**
 * 运行内部命令
 */
static void run_builtin (const cli_cmd_t * cmd, int argc, char ** argv) {
    int ret = cmd->do_func(argc, argv);
    if (ret < 0) {
        fprintf(stderr,"error: %d\n", ret);
    }
}

int main (int argc, char **argv) {
	open(argv[0], 0);
    dup(0);     // 标准输出
    dup(0);     // 标准错误输出

   	cli_init(promot, cmd_list, sizeof(cmd_list) / sizeof(cli_cmd_t));
	for (;;) {
        // 显示提示符，开始工作
        show_promot();

        // 获取输入的字符串，然后进行处理.
        // 注意，读取到的字符串结尾中会包含换行符和0
        char * str = fgets(cli.curr_input, CLI_INPUT_SIZE, stdin);
        if (str == (char *)0) {
            // 读不到错误，或f发生错误，则退出
            break;
        }

        // 读取的字符串中结尾可能有换行符，去掉之
        char * cr = strchr(cli.curr_input, '\n');
        if (cr) {
            *cr = '\0';
        }
        cr = strchr(cli.curr_input, '\r');
        if (cr) {
            *cr = '\0';
        }

        int argc = 0;
        char * argv[CLI_MAX_ARG_COUNT];
        memset(argv, 0, sizeof(argv));

        // 提取出命令，找命令表
        const char * space = " ";  // 字符分割器
        char *token = strtok(cli.curr_input, space);
        while (token) {
            // 记录参数
            argv[argc++] = token;

            // 先获取下一位置
            token = strtok(NULL, space);
        }

        // 没有任何输入，则x继续循环
        if (argc == 0) {
            continue;
        }

        // 试图作为内部命令加载执行
        const cli_cmd_t * cmd = find_builtin(argv[0]);
        if (cmd) {
            run_builtin(cmd, argc, argv);
            continue;
        }

        // 测试程序，运行虚拟的程序
        //run_exec_file("", argc, argv);

        // 试图作为外部命令执行。只检查文件是否存在，不考虑是否可执行
        // const char * path = find_exec_path(argv[0]);
        // if (path) {
        //     run_exec_file(path, argc, argv);
        //     continue;
        // }

        // 找不到命令，提示错误
        fprintf(stderr, "Unknown command: %s\n", cli.curr_input);
    }

    return 0;
}