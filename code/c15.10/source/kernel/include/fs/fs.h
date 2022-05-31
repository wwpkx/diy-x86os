/**
 * 文件系统相关接口的实现
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef FS_H
#define FS_H

#define	O_RDONLY	0x000       // 读模式
#define	O_WRONLY	0x001		// 写模式
#define	O_RDWR		0x002		// 读写模式
#define O_CREATE    0x200       // 创建文件

int sys_open(const char *name, int flags, ...);
int sys_read(int file, char *ptr, int len);
int sys_write(int file, char *ptr, int len);
int sys_lseek(int file, int ptr, int dir);
int sys_close(int file);
int sys_isatty(int file);

void fs_init (void);

#endif // FS_H

