/**
 * 内核初始化以及测试代码
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "applib/lib_syscall.h"

int first_task_main (void) {
    // 可将task_manager添加到观察窗口中，找到curr_task.pid比较
    int pid = getpid();

    for (;;) {
        msleep(1000);
    }

    return 0;
} 