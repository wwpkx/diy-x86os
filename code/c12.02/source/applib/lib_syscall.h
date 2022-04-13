/**
 * 系统调用接口
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef LIB_SYSCALL_H
#define LIB_SYSCALL_H

#include "core/syscall.h"
#include "os_cfg.h"

// 系统调用号
#define SYS_msleep              0

/**
 * 执行系统调用
 */
static inline int sys_call (int syscall_num, int arg0, int arg1, int arg2, int arg3) {
	const unsigned long sys_gate_addr[] = {0, SELECTOR_SYSCALL | 0};  // 使用特权级0
    int ret;

    // 采用调用门, 这里只支持5个参数
    // 用调用门的好处是会自动将参数复制到内核栈中，这样内核代码很好取参数
    // 而如果采用寄存器传递，取参比较困难，需要先压栈再取
    __asm__ __volatile__(
            "push %1\n\t"
            "push %2\n\t"
            "push %3\n\t"
            "push %4\n\t"
            "push %5\n\t"
            "lcalll *(%6)\n\n"
    		:"=a"(ret)
			 :"r"(arg3), "r"(arg2), "r"(arg1), "r"(arg0), "r"(syscall_num), "r"(sys_gate_addr));
    return ret;
}

static inline int msleep (int ms) {
	return sys_call(SYS_msleep, ms, 0, 0, 0);
}

#endif //LIB_SYSCALL_H
