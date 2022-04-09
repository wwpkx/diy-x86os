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

#define SYS_open                100
#define SYS_read                101
#define SYS_write               102
#define SYS_lseek               103
#define SYS_close               104
#define SYS_unlink              105
#define SYS_fstat               106
#define SYS_stat                107
#define SYS_isatty              108

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
int fork();
int getpid();
int kill(int pid, int sig);
caddr_t sbrk(int incr);
int wait(int *status);

// 时间相关
clock_t times(struct tms *buf);
int gettimeofday(struct timeval *p, void *z);

#endif //LIB_SYSCALL_H
