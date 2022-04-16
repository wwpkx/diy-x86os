/**
 * 内存管理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "lib_syscall.h"
#include <stdio.h>

int main (int argc, char **argv) {
    open("dev/tty/0", 0);

	// 设置c标准输入输出为实时写入，不要采用缓存的方式
	// 这样按下的键能够立即显示在屏幕上。在printf时会逐个字符立即输出
    // 不过，这样的显示速率很慢
	setvbuf(stdout, (char *)0, _IONBF, 0);

    printf("sh >>");
    while (1) {
        char c = getchar();
        if (c == EOF) {
        } else {
            putchar(c);
        }
    }

    // int count = 0;
    // for (;;) {
    //     // printf("arg[0]: hello %d\n", count++);
    //     msleep(1000);
    // }
}