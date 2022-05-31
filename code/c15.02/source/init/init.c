/**
 * 内核初始化以及测试代码
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "applib/lib_syscall.h"

int main (int argc, char **argv) {
    int pid = getpid();
    print_msg("init task is running, pid=%d", pid);

    for (;;) {
        msleep(1000);
    }

    return 0;
} 