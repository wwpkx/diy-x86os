/**
 * 第一个用户任务，功能比较简单，直接加载init进程
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "applib/lib_syscall.h"

int main (void) {
    execve("/bin/sh", 0, 0);
    while (1) {}
}