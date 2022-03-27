/**
 * 自己动手写操作系统
 *
 * 16位引导代码
 * 二级引导，负责进行硬件检测，进入保护模式，然后加载内核，并跳转至内核运行
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */

// 16位代码，必须加上放在开头，以便有些io指令生成为32位
__asm__(".code16gcc");

#include "comm/boot_info.h"
#include "loader.h"

static boot_info_t boot_info;			// 启动参数信息

/**
 * BIOS下显示字符串
 * @param msg
 */
static void show_msg (const char * msg) {
	char c;

	// 使用bios写显存，持续往下写
	// 参考资料：https://blog.csdn.net/qq_40169767/article/details/101511805
	while ((c = *msg++) != '\0') {
		__asm__ __volatile__(
				"mov $0xe, %%ah\n\t"
				"mov %0, %%al\n\t"
				"mov $0x3, %%bx\n\t"
				"int $0x10"::"r"(c));
	}
}

// 参考：https://wiki.osdev.org/Memory_Map_(x86)
// 1MB以下比较标准, 在1M以上会有差别
static void  detect_memory(void) {
	uint32_t contID = 0;
	smap_entry_t smap_entry;
	int signature, bytes;

	// 初次：EDX=0x534D4150,EAX=0xE820,ECX=24,INT 0x15, EBX=0（初次）
	// 后续：EAX=0xE820,ECX=24,
	// 结束判断：EBX=0
	boot_info.ram_region_count = 0;
	for (int i = 0; i < BOOT_RAM_REGION_MAX; i++) {
		smap_entry_t * entry = &smap_entry;

		__asm__ __volatile__("int  $0x15"
			: "=a"(signature), "=c"(bytes), "=b"(contID)
			: "a"(0xE820), "b"(contID), "c"(sizeof(smap_entry_t)), "d"(SMAP_MAGIC_NUMBER), "D"(entry));
		if (signature != SMAP_MAGIC_NUMBER) {
			return;
		}

		// todo: 20字节
		if (bytes > 20 && !(entry->acpi & SMAP_ACPI_NO_IGNORE)){
			continue;
		}

		if (contID == 0) {
			break;
		}

		// 保存RAM信息，只取32位，空间有限无需考虑更大容量的情况
		if (entry->type == SMAP_TYPE_USABLE_RAM) {
			boot_info.ram_region_cfg[boot_info.ram_region_count].start = entry->base31_0;
			boot_info.ram_region_cfg[boot_info.ram_region_count].size = entry->length31_0;
			boot_info.ram_region_count++;
		}
	}
}

void loader_entry(void) {
    show_msg("....loading.....");
	detect_memory();
    for(;;) {}
}


