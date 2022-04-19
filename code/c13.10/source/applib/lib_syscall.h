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

// 系统调用号
#define SYS_msleep              0
#define SYS_getpid              1
#define SYS_sched_yield         3
#define SYS_fork                4
#define SYS_execve              5
#define SYS_wait                6
#define SYS_exit                7

#define SYS_open                100
#define SYS_read                101
#define SYS_write               102
#define SYS_close               103
#define SYS_lseek               104
#define SYS_unlink              105
#define SYS_link                106
#define SYS_fstat               107
#define SYS_stat                108
#define SYS_isatty              109
#define SYS_sbrk                110

int msleep (int ms);
int fork(void);
int getpid(void);
int sched_yield (void);
int execve(const char *name, char * const *argv, char * const *env);
int wait(int* status);
void exit(int status);

int open(const char *name, int flags, ...);
int read(int file, void *ptr, size_t len);
int write(int file, const void *ptr, size_t len);
int close(int file);
off_t lseek(int file, off_t ptr, int dir);
int unlink(const char *name);
int link(const char *old, const char *new);
int fstat(int file, struct stat *st);
int stat(const char *file, struct stat *st);
int isatty(int file);

void * sbrk(ptrdiff_t incr);

#endif //LIB_SYSCALL_H