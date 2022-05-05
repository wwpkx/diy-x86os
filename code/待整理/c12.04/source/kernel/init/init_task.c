/**
 * 内核初始化以及测试代码
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "applib/lib_syscall.h"

int init_task_main (void) {
    int pid = getpid();
    
    sched_yield();
    for (;;) {
        // hlt();
        msleep(1000);
    }

    return 0;
} 