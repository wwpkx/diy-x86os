/**
 * 文件管理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef FILE_H
#define FILE_H

#define FILE_TABLE_SIZE         2048        // 可打开的文件数量

/**
 * 文件类型
 */
typedef enum _file_type_t {
    FILE_NONE = 0,
    FILE_NORMAL,
	FILE_TTY,
} file_type_t;

/**
 * 文件描述符
 */
typedef struct _xfile_t {
    file_type_t type;           // 文件类型
    int dev;                    // 文件所属的设备号

    int pos;                   	// 当前位置
    int mode;					// 读写模式
    int ref;                    // 被引用的次数
} file_t;

#endif // FILE_H
