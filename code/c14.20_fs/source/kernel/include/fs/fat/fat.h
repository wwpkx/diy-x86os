/**
 * FAT文件系统结构
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef FAT_H
#define FAT_H

#include "comm/types.h"

/**
 * xfat结构
 */
typedef struct _fat_t {
    uint32_t fat_start_sector;             // FAT表起始扇区
    uint32_t fat_tbl_nr;                   // FAT表数量
    uint32_t fat_tbl_sectors;              // 每个FAT表的扇区数
    uint32_t sec_per_cluster;              // 每簇的扇区数
    uint32_t root_sector;                 // 根目录的扇区号
    uint32_t cluster_byte_size;            // 每簇字节数
    uint32_t total_sectors;                // 总扇区数

    uint8_t * fat_buffer;             		// FAT表项缓冲

    int device;								// 对应的设备号
} fat_t;

#endif // FAT_H
