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

int msleep (int ms);
int fork(void);
int getpid(void);
int yield (void);
int execve(const char *name, char * const *argv, char * const *env);
int wait(int* status);
void _exit(int status);

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
int dup (int file);

void * sbrk(ptrdiff_t incr);

#endif //LIB_SYSCALL_H
