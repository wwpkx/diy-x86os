/**
 * echo命令处理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "do_echo.h"

/**
 * 回显命令
 */
int do_echo (int argc, char ** argv) {
	// 只有一个参数，需要先手动输入，再输出
	if (argc == 1) {
		char msg_buf[128];

		// 原样的输出字符
		fgets(msg_buf, sizeof(msg_buf), stdin);
		msg_buf[sizeof(msg_buf) - 1] = '\0';
		if (msg_buf[0] != '\0') {
			puts(msg_buf);
		}
		return 0;
	}

	// https://www.cnblogs.com/yinghao-liu/p/7123622.html
	// optind是下一个要处理的元素在argv中的索引
	// 当没有选项时，变为argv第一个不是选项元素的索引。
	int count = 1;	// 缺省只打印一次
	int ch;
    while ((ch = getopt(argc, argv, "n:h")) != -1) {
    	switch (ch) {
    	case 'h':
    		puts("echo echo any message");
    		puts("Usage: echo [-n count] msg");
    		optind = 1;		// getopt需要多次调用，需要重置
    		return 0;
    	case 'n':    // 加了:，标准该参数可行
        	count = atoi(optarg);
        	break;
    	case '?':
    		if (optarg) {
    			fprintf(stderr, "Unknown option: -%s\n", optarg);
    		}
    		optind = 1;		// getopt需要多次调用，需要重置
    		return -1;
    	}
    }

    // 索引已经超过了最后一个参数的位置，意味着没有传入要发送的信息
    if (optind > argc - 1) {
        fprintf(stderr, "Message is empty \n");
    	optind = 1;		// getopt需要多次调用，需要重置
        return -1;
    }

    // 循环打印消息
    char * msg = argv[optind];
	for (int i = 0; i < count; i++) {
		puts(msg);		// 将自动加入返行符
	}

	optind = 1;		// getopt需要多次调用，需要重置
	return 0;
}
