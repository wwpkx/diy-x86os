/**
 * 简单的命令行解释器
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <stdio.h>
#include "lib_syscall.h"

char cmd_buf[256];
int main (int argc, char **argv) {
#if 0
    sbrk(0);
    sbrk(100);
    sbrk(200);
    sbrk(4096*2 + 200);
    sbrk(4096*5 + 1234);

    printf("abef\b\b\b\bcd\n");    // \b: 输出cdef
    printf("abcd\x7f;fg\n");   // 7f: 输出 abc;fg
    printf("\0337Hello,word!\0338123\n");  // ESC 7,8 输出123lo,word!
    printf("\033[31;42mHello,word!\033[39;49m123\n");  // ESC [pn m, Hello,world红色，其余绿色
    printf("123\033[2DHello,word!\n");  // 光标左移2，1Hello,word!
    printf("123\033[2CHello,word!\n");  // 光标右移2，123  Hello,word!

    printf("\033[31m");  // ESC [pn m, Hello,world红色，其余绿色
    printf("\033[10;10H test!\n");  // 定位到10, 10，test!
    printf("\033[20;20H test!\n");  // 定位到20, 20，test!
    printf("\033[32;25;39m123\n");  // ESC [pn m, Hello,world红色，其余绿色  

    printf("\033[2J\n");   // clear screen
#endif

	open("tty:0", 0);

    puts("hello from x86 os");
    printf("os version: %s\n", OS_VERSION);
    puts("author: lishutong");
    puts("create data: 2022-5-31");

    puts("sh >>");

    for (;;) {
        gets(cmd_buf);
        puts(cmd_buf);

        // print_msg("pid=%d", getpid());
        // msleep(1000);
    }
}