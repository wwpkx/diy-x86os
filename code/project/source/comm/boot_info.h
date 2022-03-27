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

#define SMAP_TYPE_AVALIABLE_RAM		1
#define SMAP_TYPE_RESERVED			2

#pragma pack(1)
typedef struct _smap_entry_t {
	uint32_t base31_0;
	uint32_t base63_32;
	uint32_t length31_0;
	uint32_t length63_32;
	uint32_t type;
	uint32_t acpi;
}smap_entry_t;
#pragma pack()

/**
 * 启动信息参数
 */
typedef struct _boot_info_t {
	uint32_t start_sector;

	unsigned int screen_width;
	unsigned int screen_height;
	unsigned int screen_vram;
	unsigned int screen_vmode;

	smap_entry_t * smap_entry;
	int smap_entry_count;
}boot_info_t;

#endif // BOOT_INFO_H
