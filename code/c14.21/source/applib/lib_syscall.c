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
int execve(const char *name, char * const *argv, char * const *env) {
    return sys_call(SYS_execve, (int)name, (int)argv, (int)env, 0);
}

int wait(int* status) {
    return -1;
}

void exit(int status) {
}

int open(const char *name, int flags, ...) {
    // 简单点，不支持那么多的参数
    return sys_call(SYS_open, (int)name, (int)flags, 0, 0);
}

int read(int file, void *ptr, size_t len) {
    return sys_call(SYS_read, (int)file, (int)ptr, (int)len, 0);
}

int write(int file, const void *ptr, size_t len) {
    return sys_call(SYS_write, (int)file, (int)ptr, (int)len, 0);
}

int close(int file) {
    return sys_call(SYS_close, file, 0, 0, 0);
}

off_t lseek(int file, off_t ptr, int dir) {
    return sys_call(SYS_lseek, (int)file, (int)ptr, (int)dir, 0);
}

/**
 * 删除文件
 */
int unlink(const char *name) {
    return sys_call(SYS_unlink, (int)name, 0, 0, 0);
}

/**
 * 建立硬连接
 */
int link(const char *old, const char *new) {
    return sys_call(SYS_link, (int)old, (int)new, 0, 0);
}

/**
 * 获取文件的状态
 */
int fstat(int file, struct stat *st) {
    return sys_call(SYS_fstat, (int)file, (int)st, 0, 0);
}

/**
 * 获取文件的状态
 */
int stat(const char *file, struct stat *st) {
    return sys_call(SYS_stat, (int)file, (int)st, 0, 0);
}

/**
 * 判断文件描述符与tty关联
 */
int isatty(int file) {
    return sys_call(SYS_isatty, file, 0, 0, 0);
}

void * sbrk(ptrdiff_t incr) {
    return (caddr_t)sys_call(SYS_sbrk, incr, 0, 0, 0);
}

int dup (int file) {
    return sys_call(SYS_dup, file, 0, 0, 0);
}


