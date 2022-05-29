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
#define FILE_NAME_SIZE          32          // 文件名称大小

struct _fs_t;

/**
 * 文件类型
 */
typedef enum _file_type_t {
    FILE_UNKNOWN = 0,
    FILE_NORMAL,
    FILE_DIR,
	FILE_TTY,
} file_type_t;

/**
 * @brief 文件信息
 */
typedef struct _file_info_t {
    int cstart;            	// 数据区起始簇号
	int cparent;            // 所在的父目录簇
	int poffset;            // 在父目录中的偏移量
}file_info_t;

/**
 * 文件描述符
 */
typedef struct _file_t {
    char file_name[FILE_NAME_SIZE];	// 文件名
    file_type_t type;           // 文件类型
    uint32_t size;              // 文件大小，只支持到4G，对这个系统足够了
    int dev;                    // 文件所属的设备号

    int pos;                   	// 当前位置
    int mode;					// 读写模式
    int ref;                    // 被引用的次数
    struct _fs_t * fs;          // 所在的文件系统

    uint32_t start_cluster;     // 数据的起始簇号
    uint32_t curr_cluster;      // 当前簇号
} file_t;

typedef struct _dir_t {
	int	idx;		            // 目录表遍历的写索引
    uint32_t start_cluster;     // 数据的起始簇号
    uint32_t curr_cluster;      // 当前簇号
} dir_t;

#endif // FILE_H
