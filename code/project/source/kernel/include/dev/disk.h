/**
 * 磁盘操作接口，与具体设备无关
 *
 * 源码主要来源于 从0到1动手写FAT32文件系统 课程，并进行了调整
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef DISK_H
#define	DISK_H

#include <core/types.h>
#include <ipc/sem.h>
#include <core/os_cfg.h>

#pragma pack(1)

/**
 * MBR的分区表项类型
 */
typedef struct _mbr_part_t {
    uint8_t boot_active;               // 分区是否活动
	uint8_t start_header;              // 起始header
	uint16_t start_sector : 6;         // 起始扇区
	uint16_t start_cylinder : 10;	    // 起始磁道
	uint8_t system_id;	                // 文件系统类型
	uint8_t end_header;                // 结束header
	uint16_t end_sector : 6;           // 结束扇区
	uint16_t end_cylinder : 10;        // 结束磁道
	uint32_t relative_sectors;	        // 相对于该驱动器开始的相对扇区数
	uint32_t total_sectors;            // 总的扇区数
}mbr_part_t;

#define MBR_PRIMARY_PART_NR	    4   // 4个分区表

/**
 * MBR区域描述结构
 */
typedef  struct _mbr_t {
	uint8_t code[446];                 // 引导代码区
    mbr_part_t part_info[MBR_PRIMARY_PART_NR];
	uint8_t boot_sig[2];               // 引导标志
}mbr_t;

#pragma pack()

/**
 * 磁盘驱动接口
 */
struct _disk_info_t ;
typedef struct _xdisk_driver_t {
    int (*open) (struct _disk_info_t * disk);
    void (*close) (struct _disk_info_t * disk);
    int (*read) (struct _disk_info_t *disk, uint8_t *buffer, int start_sector, int count);
    int (*write) (struct _disk_info_t *disk, uint8_t *buffer, int start_sector, int count);
}xdisk_driver_t;

/**
 * 文件系统类型
 */
typedef enum {
	FS_NOT_VALID = 0x00,            // 无效类型
}xfs_type_t;

typedef struct _part_info_t {
	xfs_type_t type;			// 文件系统类型
	int start_sector;
	int total_sector;
}part_info_t;

typedef struct _disk_info_t {
	int device;
	int total_sectors;				// 总的扇区数量
    uint32_t sector_size;              	// 块大小
	xdisk_driver_t * driver;		// 设备驱动
	part_info_t part[XDISK_PART_MAX_COUNT];	// 分区表
}disk_info_t;

/**
 * 存储设备类型
 */
typedef struct _xdisk_t {
	list_node_t node;
	int device;							// 设备号
	uint32_t total_sector;             	// 总的块数量
	int start_sector;					// 起始扇区号
	disk_info_t * disk_info;			// 所属磁盘信息
}xdisk_t;

void xdisk_list_init (void);
void disk_register (int device, int sector_num, int sector_size, xdisk_driver_t * driver);
xdisk_t * xdisk_open(int device);
void xdisk_close(xdisk_t * disk);
int xdisk_read_sector(xdisk_t *disk, uint8_t *buffer, int start_sector, int count);
int xdisk_write_sector(xdisk_t *disk, uint8_t *buffer, int start_sector, int count);

#endif

