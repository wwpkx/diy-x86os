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

#define	MOUNT_LIST_CNT				4		// 最多挂载的分区
#define	MOUNT_NAME_SIZE				32		// 挂载名称长度

// 标准输入输出文件描述符
enum {
	FILE_STDIN = 0,		
	FILE_STDOUT = 1,
	FILE_STDERR = 2,
};

// 挂载的文件系统配置数据
typedef union _fs_data_t {
    void * data;                // 配置数据
}fs_data_t;

/**
 * 挂载配置
 */
typedef struct _mount_point_t {
	char name[MOUNT_NAME_SIZE];	    // 挂载的名称
	int dev;					    // 对应的设备号
	fs_data_t fs_data;				// 文件系统配置数据

    // 挂载的类型
	enum {
		MOUNT_DISK,
		MOUNT_TTY,
	}type;
}mount_point_t;

void fs_init (void);
int fs_mount (const char * name, int dev);
void fs_unmount(int dev);
mount_point_t * mount_find_name(const char * path);
void fs_add_ref (file_t * file);

int sys_open(const char *name, int flags, ...);
int sys_read(int file, char *ptr, int len);
int sys_write(int file, char *ptr, int len);
int sys_lseek(int file, int ptr, int dir);
int sys_close(int file);
int sys_isatty(int file);
int sys_dup (int file);

#endif // FS_H

