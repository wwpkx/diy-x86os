/**
 * FAT文件系统结构
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef FAT_H
#define FAT_H

#include "ipc/mutex.h"

#pragma pack(1)    // 千万记得加这个

/**
 * 完整的DBR类型
 */
typedef struct _dbr_t {
    uint8_t BS_jmpBoot[3];                 // 跳转代码
    uint8_t BS_OEMName[8];                 // OEM名称
    uint16_t BPB_BytsPerSec;               // 每扇区字节数
    uint8_t BPB_SecPerClus;                // 每簇扇区数
    uint16_t BPB_RsvdSecCnt;               // 保留区扇区数
    uint8_t BPB_NumFATs;                   // FAT表项数
    uint16_t BPB_RootEntCnt;               // 根目录项目数
    uint16_t BPB_TotSec16;                 // 总的扇区数
    uint8_t BPB_Media;                     // 媒体类型
    uint16_t BPB_FATSz16;                  // FAT表项大小
    uint16_t BPB_SecPerTrk;                // 每磁道扇区数
    uint16_t BPB_NumHeads;                 // 磁头数
    uint32_t BPB_HiddSec;                  // 隐藏扇区数
    uint32_t BPB_TotSec32;                 // 总的扇区数

	uint8_t BS_DrvNum;                     // 磁盘驱动器参数
	uint8_t BS_Reserved1;				   // 保留字节
	uint8_t BS_BootSig;                    // 扩展引导标记
	uint32_t BS_VolID;                     // 卷标序号
	uint8_t BS_VolLab[11];                 // 磁盘卷标
	uint8_t BS_FileSysType[8];             // 文件类型名称
} dbr_t;
#pragma pack()

/**
 * fat结构
 */
typedef struct _fat_t {
    // fat文件系统本身信息
    uint32_t tbl_start;                     // FAT表起始扇区号
    uint32_t tbl_cnt;                       // FAT表数量
    uint32_t tbl_sectors;                   // 每个FAT表的扇区数
    uint32_t bytes_per_sec;                 // 每扇区大小
    uint32_t sec_per_cluster;               // 每簇的扇区数
    uint32_t root_ent_cnt;                  // 根目录的项数
    uint32_t root_start;                    // 根目录起始扇区号
    uint32_t data_start;                    // 数据区起始扇区号
    uint32_t cluster_byte_size;             // 每簇字节数

    // 与文件系统读写相关信息
    uint8_t * fat_buffer;             		// FAT表项缓冲

    struct _fs_t * fs;                      // 所在的文件系统
} fat_t;

#endif // FAT_H
