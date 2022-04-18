/**
 * 打印文本文件内容命令
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "do_cat.h"
#include "cli.h"

static char read_buf[1024];			// 读取缓冲区

/**
 * 打印文本文件内容
 */
int do_cat(int argc, char **argv) {

	// 参数判断
	if (argc < 2) {
		fprintf(stderr, "wrong argument\n");
		return 1;
	}

	// 打开文件
	argv[1] = "mount.c";
	FILE * file = fopen(argv[1], "r");
	if(file==NULL) {
		fprintf(stderr, "open file failed: %s\n", strerror(errno));
		return -1;
	}

	// 循环读取，打印文件内容
	int size;
	while((size = fread(read_buf, 1, sizeof(read_buf) - 1,file))>0) {
		read_buf[sizeof(read_buf) - 1] = '\0';
		printf("%s",read_buf);
	}
	return 0;
}
