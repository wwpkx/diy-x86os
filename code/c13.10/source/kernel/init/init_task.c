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
    for(;;){        
        // 创建进程
        int pid = fork();
        if(pid < 0){
            msleep(1000);
        } else if(pid == 0){
            // 子进程，用sh替代
            char *argv[] = {"arg0", "arg1", "arg2", "arg3"};
            execve("/bin/sh", argv, (char **)0);
            for (;;) {
                msleep(1000);
            }
        } else {
            for (;;) {
                msleep(2000);
            }
        }
    }

    return 0;
} 