/**
 * 文件系统相关接口的实现
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef FS_H
#define FS_H

#include "dev/disk.h"

struct _fs_t;

/**
 * @brief 文件系统操作接口
 */
typedef struct _fs_op_t {
	int (*mount) (struct _fs_t * fs, partinfo_t * part_info);
    void (*unmount) (struct _fs_t * fs);
    int (*open) (struct _fs_t * fs, const char * path, file_t * file);
    int (*read) (char * buf, int size, struct _file_t * file);
    int (*write) (char * buf, int size, struct _file_t * file);
    int (*close) (struct _file_t * file);
    int (*seek) (file_t * file, uint32_t pos);
    int (*stat)(struct _fs_t * fs, const char *file, struct stat *st);
}fs_op_t;

/**
 * @brief 文件系统类型
 */
typedef struct _fs_t {
    fs_op_t * op;              // 文件系统操作接口
    void * op_data;            // 文件系统的操作数据
    partinfo_t * part_info;     // 分区信息
}fs_t;

/**
 * 文件seek的定位类型
 */ 
enum {
    FILE_SEEK_SET = 0,                    // 文件开头
    FILE_SEEK_CUR = 1,                    // 当前位置
    FILE_SEEK_END = 2,                    // 文件结尾
};


int fs_load_root (int root_device);
void fs_init (void);
void fs_add_ref (file_t * file);

int sys_open(const char *name, int flags, ...);
int sys_read(int file, char *ptr, int len);
int sys_write(int file, char *ptr, int len);
int sys_lseek(int file, int ptr, int dir);
int sys_close(int file);
int sys_isatty(int file);
int sys_dup (int file);
int sys_stat(const char *file, struct stat *st);
int sys_fstat(int file, struct stat *st);

int is_path_valid (const char * path);

#endif // FS_H

