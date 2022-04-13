/**
 * 系统调用接口
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdarg.h>
#include "lib_syscall.h"
#include "kernel/include/core/syscall.h"
#include "kernel/include/os_cfg.h"
#include "kernel/include/cpu/cpu.h"

/**
 * 执行系统调用
 */
static inline int sys_call (int syscall_num, int arg0, int arg1, int arg2, int arg3) {
	static const uint32_t sys_gate_addr[] = {0, SELECTOR_SYSCALL | GDT_RPL0};
    uint32_t ret;

    // 采用调用门
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

// newlib需要的系统调用
/**
 * 打开文件
 */
int open(const char *name, int flags, ...) {
    return -1;
}

/**
 * 读取文件api
 */
int read(int file, char *ptr, int len) {
    return -1;
}

/**
 * 写文件
 */
int write(int file, char *ptr, int len) {
    return -1;
}

/**
 * 文件访问位置定位
 */
int lseek(int file, int ptr, int dir) {
    return -1;
}

/**
 * 关闭文件
 */
int close(int file) {
    return -1;
}

/**
 * 删除文件
 */
int unlink(char *name) {
    return -1;
}

/**
 * 建立硬连接
 */
int link(char *old, char *new) {
	return -ECANCELED;
}

/**
 * 获取文件的状态
 */
int fstat(int file, struct stat *st) {
    return -1;
}

/**
 * 获取文件的状态
 */
int stat(const char *file, struct stat *st) {
    return -1;
}

/**
 * 判断文件描述符与tty关联
 */
int isatty(int file) {
    return -1;
}

void _exit() {
}

char **environ; /* pointer to array of char * strings that define the current environment variables */
int execve(char *name, char **argv, char **env) {
	return -1;
}

int fork() {
    return sys_call(SYS_fork, 0, 0, 0, 0);
}

int getpid() {
	return sys_call(SYS_getpid, 0, 0, 0, 0);
}

int kill(int pid, int sig) {
	return -1;
}

caddr_t sbrk(int incr) {
	return (caddr_t)0;
}

int wait(int *status) {
	return -1;
}

clock_t times(struct tms *buf) {
	return -1;
}

int gettimeofday(struct timeval *p, void *z) {
	return -1;
}

int msleep (int ms) {
	return sys_call(SYS_msleep, ms, 0, 0, 0);
}

int sched_yield (void) {
    return sys_call(SYS_sched_yield, 0, 0, 0, 0);
}
