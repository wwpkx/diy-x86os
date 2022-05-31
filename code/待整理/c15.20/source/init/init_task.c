/**
 * 内核初始化以及测试代码
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "applib/lib_syscall.h"

int main (int argc, char **argv) {
 	// 打开标准输入输出设备，后续所有子进程将使用这些文件
	open("tty0", 0);
    dup(0);     // 标准输出
    dup(0);     // 标准错误输出

    puts("############################################################");
    puts("#");
    puts("#                      diyx86os");
    puts("# Author: lishutong");
    puts("# Time:"__DATE__ __TIME__);
    puts("#");
    puts("############################################################");

    for(;;){        
        // 创建进程
        int pid = fork();
        if(pid < 0){
            msleep(1000);
        } else if(pid == 0){
            // 子进程，用sh替代
            char *argv[] = {"arg0", "arg1", "arg2", "arg3"};

            execve("/shell.elf", argv, (char **)0);
            printf("create shell failed.\n");

            int cnt = 0;
            for (;;) {
                printf("this is child task: %d\n", cnt++);
                msleep(1000);
            }
        } 
        // else {
        //     int cnt = 0;
        //     for (;;) {
        //         //printf("this is parent task: %d\n", cnt++);
        //         msleep(2000);
        //     }
        // }

        // 等待子进程退出
        int status;
        while((pid = wait(&status)) >= 0) {
            printf("child %d exist: %d\n", pid, status);
        }
    }

    return 0;
} 