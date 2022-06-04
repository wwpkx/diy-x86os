/**
 * 文件系统相关接口的实现
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef FILE_H
#define FILE_H

#include <sys/stat.h>

#define O_RDONLY        (0 << 0)            // 只读
#define O_WRONLY        (1 << 0)            // 只写
#define O_RDWR          (2 << 0)            // 读写

void fs_init (void);

int sys_open(const char *name, int flags, ...);
int sys_read(int file, char *ptr, int len);
int sys_write(int file, char *ptr, int len);
int sys_lseek(int file, int ptr, int dir);
int sys_close(int file);

int sys_isatty(int file);
int sys_fstat(int file, struct stat *st);

#endif // FILE_H

