/*
 * diritem.h
 *
 *  Created on: 2021年8月26日
 *      Author: mac
 */

#ifndef DIR_H
#define DIR_H

#include "comm/types.h"
#include "fs/fat/cluster.h"
#include "fs/fat/fat.h"

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

#pragma pack(1)

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

#pragma pack()

/**
 * @brief 基于簇的位置信息
 */
typedef struct _cluster_pos_t {
    uint32_t cluster;
    uint32_t offset;
}cluster_pos_t;

uint32_t diritem_get_cluster (diritem_t * item);
void diritem_set_cluster (diritem_t * item, cluster_t cluster);
file_type_t diritem_get_type (diritem_t *diritem);
diritem_t * diritem_from_path (fat_t * fat, cluster_t parent, const char * path);

void diritem_init (diritem_t * item);

#endif /* DIR_H */
