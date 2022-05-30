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
    int count = 0;

    for (;;) {
        printf("arg[0]: hello %d\n", count++);
        msleep(1000);
    }
}