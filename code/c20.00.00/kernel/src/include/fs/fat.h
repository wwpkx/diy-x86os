/**
 * 本源码配套的课程为 - 从0到1动手写FAT32文件系统。每个例程对应一个课时，尽可能注释。
 * 作者：李述铜
 * 课程网址：http://01ketang.cc
 * 版权声明：本源码非开源，二次开发，或其它商用前请联系作者。
 */
#ifndef XFAT_H
#define XFAT_H

#include <dev/disk.h>
#include <fs/fs.h>

#pragma pack(1)

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

#define XFAT_NAME_LEN       16
#define	FAT_START_CLUSTER				2					// 起始簇号

/**
 * xfat结构
 */
typedef struct _xfat_t {
    uint32_t fat_start_sector;             // FAT表起始扇区
    uint32_t fat_tbl_nr;                   // FAT表数量
    uint32_t fat_tbl_sectors;              // 每个FAT表的扇区数
    uint32_t sec_per_cluster;              // 每簇的扇区数
    uint32_t root_sector;                 // 根目录的扇区号
    uint32_t cluster_byte_size;            // 每簇字节数
    uint32_t total_sectors;                // 总扇区数

    uint8_t * fat_buffer;             		// FAT表项缓冲
    xdisk_t * disk;           				// 对应的磁盘信息

    int device;								// 对应的设备号
} xfat_t;

/**
 * 文件seek的定位类型
 */
typedef enum _xfile_orgin_t {
    XFAT_SEEK_SET,                    // 文件开头
    XFAT_SEEK_CUR,                    // 当前位置
    XFAT_SEEK_END,                    // 文件结尾
}xfile_orgin_t;

int xfat_load(xfat_t * xfat, int dev);

int xfat_mkdir (int parent, const char * path);
xfile_node_t * xfat_mkfile (const char * path, int flags);
int xfat_rmfile (int parent, const char * path);
int xfat_rmdir (int parent, const char * path);

xfile_node_t * xfat_open(const char *path);
int xfat_close(xfile_t *file);
int xfat_read(void * buffer, int size, xfile_t * file);
int xfat_write(void * buffer, int size, xfile_t * file);
int xfat_seek(xfile_t * file, int offset, xfile_orgin_t origin);

xfat_t * get_xfat(int dev);

#endif /* XFAT_H */
