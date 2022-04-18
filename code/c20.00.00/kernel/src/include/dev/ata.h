/**
 * 简单的ATA磁盘驱动
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef SRC_INCLUDE_DEV_ATA_H_
#define SRC_INCLUDE_DEV_ATA_H_

#include <core/types.h>
#include <core/task.h>
#include <dev/disk.h>

typedef struct _ata_request_t {
	int cmd;
	int dev;
	uint8_t * buf;
	int sector_count;
	int start_sector;
	list_node_t node;
	sem_t complete_sem;
}ata_request_t;

// https://wiki.osdev.org/ATA_PIO_Mode#IDENTIFY_command
// 只考虑支持主总结primary bus
// https://wiki.osdev.org/ATA_PIO_Mode#IDENTIFY_command
// 只考虑支持主总结primary bus
#define	ATA_PRIMARY_DATA				0x1F0		// 数据寄存器
#define	ATA_PRIMARY_ERROR				0x1F1		// 错误寄存器
#define	ATA_PRIMARY_SECTOR_COUNT		0x1F2		// 扇区数量寄存器
#define	ATA_PRIMARY_LBA_LO				0x1F3		// LBA寄存器
#define	ATA_PRIMARY_LBA_MID				0x1F4		// LBA寄存器
#define	ATA_PRIMARY_LBA_HI				0x1F5		// LBA寄存器
#define	ATA_PRIMARY_DRIVE				0x1F6		// 磁盘或磁头？
#define	ATA_PRIMARY_STATUS				0x1F7		// 状态寄存器
#define	ATA_PRIMARY_CMD					0x1F7		// 命令寄存器

// ATA命令
#define	ATA_CMD_IDENTIFY				0xEC	// IDENTIFY命令
#define	ATA_CMD_READ					0x24	// 读命令
#define	ATA_CMD_WRITE					0x34	// 写命令

#define	ATA_BUS_MAX_COUNT				2			// ATA总线最大数量

#define	ATA_DRIVER_MASTER				0				// 主驱动器
#define	ATA_DRIVER_SLAVE				1				// 从驱动器

#define	ATA_DRIVE_BASE					0xE0		// 驱动器号基础值

void ata_device_init (void);
int ata_init (struct _disk_info_t * disk_info);
int ata_read_sector (struct _disk_info_t * disk_info, uint8_t * buf, int sector, int count);
int ata_write_sector (struct _disk_info_t * disk_info, uint8_t * buf, int sector, int count);
void ata_close (struct _disk_info_t * disk_info);

void handler_primary_ata (void);
void handler_secondary_ata (void);

#endif /* SRC_INCLUDE_DEV_ATA_H_ */
