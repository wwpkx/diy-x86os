/*
 * diritem.h
 *
 *  Created on: 2021年8月26日
 *      Author: mac
 */

#ifndef SRC_INCLUDE_FS_DIR_H_
#define SRC_INCLUDE_FS_DIR_H_

#include <core/types.h>
#include <fs/cluster.h>

#define DIRITEM_NAME_FREE               0xE5                // 目录项空闲名标记
#define DIRITEM_NAME_END                0x00                // 目录项结束名标记

#define DIRITEM_NTRES_BODY_LOWER        0x08                // 文件名小写
#define DIRITEM_NTRES_EXT_LOWER         0x10                // 扩展名小写
#define DIRITEM_NTRES_ALL_UPPER         0x00                // 文件名全部大写
#define DIRITEM_NTRES_CASE_MASK         0x18                // 大小写掩码

#define DIRITEM_ATTR_READ_ONLY          0x01                // 目录项属性：只读
#define DIRITEM_ATTR_HIDDEN             0x02                // 目录项属性：隐藏
#define DIRITEM_ATTR_SYSTEM             0x04                // 目录项属性：系统类型
#define DIRITEM_ATTR_VOLUME_ID          0x08                // 目录项属性：卷id
#define DIRITEM_ATTR_DIRECTORY          0x10                // 目录项属性：目录
#define DIRITEM_ATTR_ARCHIVE            0x20                // 目录项属性：归档
#define DIRITEM_ATTR_LONG_NAME          0x0F                // 目录项属性：长文件名

#define SFN_LEN                    	 	11              // sfn文件名长


/**
 * FAT目录项的日期类型
 */
typedef struct _diritem_date_t {
    uint16_t day : 5;                  // 日
    uint16_t month : 4;                // 月
    uint16_t year_from_1980 : 7;       // 年
} diritem_date_t;

/**
 * FAT目录项的时间类型
 */
typedef struct _diritem_time_t {
    uint16_t second_2 : 5;             // 2秒
    uint16_t minute : 6;               // 分
    uint16_t hour : 5;                 // 时
} diritem_time_t;

/**
 * FAT目录项
 */
typedef struct _diritem_t {
    uint8_t DIR_Name[11];                   // 文件名
    uint8_t DIR_Attr;                      // 属性
    uint8_t DIR_NTRes;
    uint8_t DIR_CrtTimeTeenth;             // 创建时间的毫秒
    diritem_time_t DIR_CrtTime;         // 创建时间
    diritem_date_t DIR_CrtDate;         // 创建日期
    diritem_date_t DIR_LastAccDate;     // 最后访问日期
    uint16_t DIR_FstClusHI;                // 簇号高16位
    diritem_time_t DIR_WrtTime;         // 修改时间
    diritem_date_t DIR_WrtDate;         // 修改时期
    uint16_t DIR_FstClusL0;                // 簇号低16位
    uint32_t DIR_FileSize;                 // 文件字节大小
} diritem_t;

const char * skip_sep (const char * path);
const char * jmp_next_sep(const char * path);
int to_sfn(char* dest_name, const char* my_name);
int is_filename_match(const char *name_in_dir, const char *to_find_name);

/**
 * 获取diritem的文件起始簇号
 */
static inline uint32_t diritem_cluster (diritem_t * item) {
    return (item->DIR_FstClusHI << 16) | item->DIR_FstClusL0;
}

/**
 * 设置diritem的cluster
 * @param item 目录diritem
 * @param cluster 簇号
 */
static void inline set_diritem_cluster (diritem_t * item, cluster_t cluster) {
    item->DIR_FstClusHI = (uint16_t )(cluster >> 16);
    item->DIR_FstClusL0 = (uint16_t )(cluster & 0xFFFF);
}

xfile_type_t diritem_file_type(const diritem_t *diritem);
diritem_t * diritem_next(xfat_t* xfat, cluster_t curr_cluster, uint32_t curr_offset,
		cluster_t* next_cluster, uint32_t* next_offset);

#endif /* SRC_INCLUDE_FS_DIR_H_ */
