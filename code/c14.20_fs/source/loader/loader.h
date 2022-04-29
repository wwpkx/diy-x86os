/**
 * 自己动手写操作系统
 *
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef LOADER_H
#define LOADER_H

#include "comm/types.h"

// 保护模式入口函数，在start.asm中定义
void protect_mode_entry (void);

// 内存检测信息结构
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

#define SMAP_MAGIC_NUMBER			0x534D4150
#define SMAP_ACPI_NO_IGNORE			(1 << 0)	// 不应当忽略该条目
#define SMAP_TYPE_USABLE_RAM		1			// 正常可用的RAM

extern boot_info_t boot_info;

#endif // LOADER_H