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
#include "dev/dev.h"
#include "file.h"
#include "tools/list.h"

struct _fs_t;

/**
 * @brief 文件系统操作接口
 */
typedef struct _fs_op_t {
	int (*mount) (struct _fs_t * fs, int dev_id);
    void (*unmount) (struct _fs_t * fs);
    int (*open) (struct _fs_t * fs, const char * path, file_t * file);
    int (*read) (char * buf, int size, struct _file_t * file);
    int (*write) (char * buf, int size, struct _file_t * file);
    int (*close) (struct _file_t * file);
    int (*seek) (file_t * file, uint32_t offset, int dir);
    int (*stat)(struct _fs_t * fs, const char *file, struct stat *st);
}fs_op_t;

#define FS_MOUNTP_SIZE      512

// 文件系统类型
typedef enum _fs_type_t {
    FS_FAT16,
    FS_DEVFS,
}fs_type_t;

typedef struct _fs_t {
    char mount_point[FS_MOUNTP_SIZE];       // 挂载点路径长
    fs_type_t type;              // 文件系统类型

    fs_op_t * op;              // 文件系统操作接口
    void * data;                // 文件系统的操作数据
    int dev_id;                 // 所属的设备

    list_node_t node;           // 下一结点
}fs_t;

void fs_init (void);
void fs_inc_ref (file_t * file);

int path_is_valid (const char * path);
int path_is_relative (const char * path);
int path_to_num (const char * path, int * num);

int sys_open(const char *name, int flags, ...);
int sys_read(int file, char *ptr, int len);
int sys_write(int file, char *ptr, int len);
int sys_lseek(int file, int ptr, int dir);
int sys_close(int file);

int sys_isatty(int file);
int sys_fstat(int file, struct stat *st);

int sys_dup (int file);

#endif // FILE_H

