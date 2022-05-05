/**
 * 内核初始化以及测试代码
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "applib/lib_syscall.h"

int parent = 0, child = 0;

int init_task_main (void) {
    int pid = getpid();
    
    sched_yield();

    pid = fork();
    if (pid < 0) {
        for (;;) {
            msleep(1000);
        }
    } else if (pid == 0) {
        for (;;) {
            // 子进程
            child++;
            msleep(1000);
        }
    } else {
        for (;;) {
            // 父进程
            parent++;
            msleep(2000);
        }
    }

    return 0;
} 