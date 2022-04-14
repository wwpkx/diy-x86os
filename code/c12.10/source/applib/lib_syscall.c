/**
 * 系统调用接口
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "core/syscall.h"
#include "os_cfg.h"
#include "lib_syscall.h"

/**
 * 执行系统调用
 */
int sys_call (int syscall_num, int arg0, int arg1, int arg2, int arg3) {
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

int msleep (int ms) {
	return sys_call(SYS_msleep, ms, 0, 0, 0);
}

int fork() {
    return sys_call(SYS_fork, 0, 0, 0, 0);
}

int getpid() {
	return sys_call(SYS_getpid, 0, 0, 0, 0);
}

int sched_yield (void) {
    return sys_call(SYS_sched_yield, 0, 0, 0, 0);
}

char **environ;     // 当前的环境变量
int execve(char *name, char **argv, char **env) {
    return sys_call(SYS_execve, (int)name, (int)argv, (int)env, 0);
}
