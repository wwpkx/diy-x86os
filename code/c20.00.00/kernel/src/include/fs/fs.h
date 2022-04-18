/*
 * fs.h
 *
 *  Created on: 2021年8月14日
 *      Author: mac
 */

#ifndef FILE_H
#define FILE_H

#include <core/types.h>

#define	FILE_NAME_MAX			128

/**
 * 时间描述结构
 */
typedef struct _xfile_time_t {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
}xfile_time_t;

/**
 * 文件类型
 */
typedef enum _xfile_type_t {
    FILE_NONE = 0,
	FILE_VOL,
    FILE_DIR,
    FILE_FILE,
	FILE_TTY,
} xfile_type_t;

/**
 * 文件基础信息结点，可能多个进程共享
 */
#define FILE_NAME_SIZE        32
typedef struct _xfile_node_t {
    char file_name[FILE_NAME_SIZE];	// 文件名
    xfile_type_t type;             	// 文件类型

    int size;                 		// 文件字节大小
    int ref_count;					// 进程引用计数

    int start_cluster;            	// 数据区起始簇号
	int parent;              		// 所在的父目录簇
	int pos_in_parent;       		// 在父目录中的偏移量

	int device;						// 所在的设备
}xfile_node_t;

/**
 * 文件类型，每个进程一个
 */
typedef struct _xfile_t {
    int pos;                   	// 当前位置
    int mode;					// 读写模式
    int curr_cluster;			// 当前簇号

    xfile_node_t * file_node;	// 文件基础信息结点
} xfile_t;

void fs_init (void);

int xfile_mkdir (const char * path);
int xfile_mkfile (const char * path);
int xfile_rmfile (const char * path);
int xfile_rmdir (const char * path);

int xfile_open(const char *path, int flags);
int xfile_close(int file);
int xfile_read(int file, char * buf, int len);
int xfile_write(int file, char * buf, int len);
int xfile_ioctl(int file, int cmd, int data);
int xfile_dup(int file);

//int xdir_open (const char *path);
//int xdir_read (xdir_t * dir, xdir_entry_t * dir_entry);
//int xdir_close (xdir_t * dir);

int xfile_unlink(char *name);
int xfile_isatty(int file);

#endif /* FILE_H */
