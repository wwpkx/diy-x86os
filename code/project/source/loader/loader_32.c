/**
 * 自己动手写操作系统
 *
 * 32位引导代码
 * 二级引导，负责进行硬件检测，进入保护模式，然后加载内核，并跳转至内核运行
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "loader.h"
#include "comm/cpu_instr.h"

#define DISK_SECTOR_SIZE        512         // 磁盘扇区大小

/**
* 读磁盘
*/
static void read_disk(int sector, int sector_count, uint8_t * buf) {
	outb(0x1F1, (uint8_t) 0);
	outb(0x1F1, (uint8_t) 0);
	outb(0x1F2, (uint8_t) (sector_count >> 8));
	outb(0x1F2, (uint8_t) (sector_count));
	outb(0x1F3, (uint8_t) (sector >> 24));		// LBA参数的24~31位
	outb(0x1F3, (uint8_t) (sector));			// LBA参数的0~7位
	outb(0x1F4, (uint8_t) (0));					// LBA参数的32~39位
	outb(0x1F4, (uint8_t) (sector >> 8));		// LBA参数的8~15位
	outb(0x1F5, (uint8_t) (0));					// LBA参数的40~47位
	outb(0x1F5, (uint8_t) (sector >> 16));		// LBA参数的16~23位
	outb(0x1F6, (uint8_t) (0xE0));
	outb(0x1F7, (uint8_t) 0x24);

	// 读取数据
	uint16_t *data_buf = (uint16_t*) buf;
	while (sector_count-- > 0) {
		// 每次扇区读之前都要检查，等待数据就绪
		while ((inb(0x1F7) & 0x88) != 0x8) {}

		// 读取并将数据写入到缓存中
		for (int i = 0; i < DISK_SECTOR_SIZE / 2; i++) {
			*data_buf++ = inw(0x1F0);
		}
	}
}

/**
 * 初始化内核加载
 */
static void init_kernel_load(void) {
}

/**
 * 从磁盘上加载内核
 */
void load_kernel(void) {
    init_kernel_load();
	
	for (;;) {}
}

