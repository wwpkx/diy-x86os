/**
 * 简单的命令行解释器
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <stdio.h>
#include "lib_syscall.h"

int main (int argc, char **argv) {
    sbrk(0);
    sbrk(100);
    sbrk(200);
    sbrk(4096*2 + 200);
    sbrk(4096*5 + 1234);

    printf("ab\b\bcd\n");    // \b: 输出cd
    printf("abcd\x7f;fg\n");   // 7f: 输出 abc;fg
    printf("1\t12\t123\t1234\t12345\t123456\t1234567\t1\t12\t123\t1234\t12345\t123456\t1234567\n");

    puts("hello from x86 os");
    printf("os version: %s\n", OS_VERSION);
    puts("author: lishutong");
    puts("create data: 2022-5-31");

    puts("sh >>");

    for (int i = 0; i < argc; i++) {
        print_msg("arg: %s", (int)argv[i]);
    }

    // 创建一个自己的副本
    fork();

    yield();

    for (;;) {
        print_msg("pid=%d", getpid());
        msleep(1000);
    }
}