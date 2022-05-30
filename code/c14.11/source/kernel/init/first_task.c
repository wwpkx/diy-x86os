/**
 * 内核初始化以及测试代码
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "applib/lib_syscall.h"

int parent = 0, child = 0;

int first_task_main (void) {
    int pid = getpid();
    print_msg("first task id=%d", pid);
    
    pid = fork();
    if (pid < 0) {
        print_msg("create child proc failed.", 0);
        for (;;) {
            msleep(1000);
        }
    } else if (pid == 0) {
        for (;;) {
            // 子进程
            char * argv[] = {"arg0", "arg1", "arg2", "arg3"};
            execve("shell", argv, (char **)0);
        }
    } else {
        print_msg("child task id=%d", pid);
        for (;;) {
            // 父进程
            print_msg("parent=%d", parent++);
            msleep(2000);
        }
    }

    return 0;
} 