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
    // 标准输入输出文件必须要有，所以先打开，打不开则创建一个
    if(open(":dev/tty/0", O_RDWR) < 0){
        for (;;) {
            msleep(1000);
        }
    }
    
    for (;;) {
        msleep(1000);
    }
    
    // 复制标准输入描述符，将生成描述符1，2,供输出和错误输出使用
    //dup(stdin);         // 1 - stdout
    //dup(stdin);         // 2 - stderr
    for(;;){        
        // 创建进程
        int pid = fork();
        if(pid < 0){
            msleep(1000);
        } else if(pid == 0){
            // 子进程，用sh替代
            char *argv[] = {"arg0", "arg1", "arg2", "arg3"};
            execve("/bin/sh", argv, (char **)0);
            // printf(1, "init: exec sh failed\n");
            // exit();
        } else {
            msleep(2000);
        }
    }

    return 0;
} 