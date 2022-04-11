/**
 * 系统调用接口
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef LIB_SYSCALL_H
#define LIB_SYSCALL_H

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdarg.h>

// 调用门相关配置
#define SELECTOR_SYSCALL     		(3 << 3)	// 调用门的选择子
#define SYSCALL_PARAM_COUNT         5       	// 系统调用最大支持的参数

#define SYS_getpid              0
#define SYS_exit                1           
#define SYS_execve              2
#define SYS_fork                3
#define SYS_kill                4
#define SYS_sbrk                5
#define SYS_wait                6
#define SYS_sched_yield         7
#define SYS_msleep              8

#define SYS_open                100
#define SYS_read                101
#define SYS_write               102
#define SYS_lseek               103
#define SYS_close               104
#define SYS_unlink              105
#define SYS_fstat               106
#define SYS_stat                107
#define SYS_isatty              108

/**
 * 执行系统调用
 */
static int sys_call (int syscall_num, int arg0, int arg1, int arg2, int arg3) {
	const uint32_t sys_gate_addr[] = {0, SELECTOR_SYSCALL | 0};  // 使用特权级0
    uint32_t ret;

    // 采用调用门, 最多支持5个参数
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

// 系统调用的简化编写
#define sys_call0(syscall_num) sys_call(syscall_num, 0, 0, 0, 0)
#define sys_call1(syscall_num, arg0) sys_call(syscall_num, arg0, 0, 0, 0)
#define sys_call2(syscall_num, arg0, arg1) sys_call(syscall_num, arg0, arg1, 0, 0)
#define sys_call3(syscall_num, arg0, arg1, arg2) sys_call(syscall_num, arg0, arg1, arg2, 0)
#define sys_call4(syscall_num, arg0, arg1, arg2, arg3) sys_call(syscall_num, arg0, arg1, arg2, arg3)

// newlib需要的系统调用
// 文件相关
int open(const char *name, int flags, ...);
int read(int file, char *ptr, int len);
int write(int file, char *ptr, int len);
int lseek(int file, int ptr, int dir);
int close(int file);
int unlink(char *name);
int link(char *old, char *new);
int fstat(int file, struct stat *st);
int stat(const char *file, struct stat *st);
int isatty(int file);

// 进程相关
void _exit(int status);
int execve(char *name, char **argv, char **env);

static inline int fork() {
    return sys_call0(SYS_fork);
}

int getpid();
int kill(int pid, int sig);
caddr_t sbrk(int incr);
int wait(int *status);
int sched_yield (void);

// 时间相关
clock_t times(struct tms *buf);
int gettimeofday(struct timeval *p, void *z);
/**
 * @brief 毫秒延时
 */
static inline void msleep (unsigned int ms) {
    sys_call1(SYS_msleep, ms);
}

#endif //LIB_SYSCALL_H
