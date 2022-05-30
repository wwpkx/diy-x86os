/**
 * 文件系统相关接口的实现
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef FILE_H
#define FILE_H

#include "applib/lib_syscall.h"

int sys_open(const char *name, int flags, ...);
int sys_read(int file, char *ptr, int len);
int sys_write(int file, char *ptr, int len);
int sys_lseek(int file, int ptr, int dir);
int sys_close(int file);
int sys_open(const char *name, int flags, ...);
int sys_read(int file, char *ptr, int len);
int sys_write(int file, char *ptr, int len);
int sys_close(int file);
int sys_lseek(int file, int ptr, int dir);
int sys_unlink(char *name);
int sys_link(char *old, char *new);
int sys_fstat(int file, struct stat *st);
int sys_stat(const char *file, struct stat *st);
int sys_isatty(int file);

#endif // FILE_H

