/**
 * 内核初始化以及测试代码
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "core/task.h"
#include "tools/log.h"

int first_task_main (void) {
    for (;;) {
        log_printf("first task.");
        sys_msleep(1000);
    }

    return 0;
} 