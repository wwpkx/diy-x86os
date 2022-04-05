/**
 * 系统启动信息
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef BOOT_INFO_H
#define BOOT_INFO_H

#include "types.h"

#define BOOT_RAM_REGION_MAX			10		// RAM区最大数量

/**
 * 启动信息参数
 */
typedef struct _boot_info_t {
	// RAM区信息
	struct {
		uint32_t start;
		uint32_t size;
	}ram_region_cfg[BOOT_RAM_REGION_MAX];
	int ram_region_count;

	// 图形显示
	int screen_width;
	int screen_height;
	uint32_t screen_vram;
 	int screen_vmode;
}boot_info_t;

// 系统启动后所用的画面模式
#define SYS_CFG_SCREEN_WIDTH       800			// 图形像素的宽度
#define SYS_CFG_SCREEN_HEIGHT      600			// 图形像素的高度
#define SYS_CFG_SCREEN_BPP         32			// 每像素的表达位数

#define SYS_DISK_SECTOR_SIZE		512			// 磁盘扇区大小

#define SYS_KERNEL_LOAD_ADDR		(1024*1024)		// 内核加载的起始地址

#define SYS_KERNEL_BASE_ADDR		0xc0000000		// 内核空间起始地址

#endif // BOOT_INFO_H
