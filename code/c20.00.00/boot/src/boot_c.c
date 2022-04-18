/**
 * 自己动手写操作系统
 *
 * 系统引导部分，启动时由硬件加载运行，然后完成对二级引导程序loader的加载
 * boot扇区容量较小，仅512字节。由于dbr占用了不少字节，导致其没有多少空间放代码，
 * 所以功能只能最简化,并且要开启最大的优化-os
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <core/code16.h>
#include <core/cpu_instr.h>
#include <core/os_cfg.h>
#include <fs/dir.h>
#include <fs/fat.h>

// 不要用dbr.BPB_BytsPerSec，这样会占用较多代码空间，用常数会更简单
#define DISK_SECTOR_SIZE		512			// 磁盘每扇区的大小

extern dbr_t fat_dbr;						// dbr区域

extern void loading (void);					// 加载进度显示函数

/**
 * 读取磁盘
 * 采用BIOS中断去读写磁盘，更加节省空间
 * 以下采用lba48，支持更大容量的磁盘，不过只用了32位的扇区号，支持的磁盘
*  大小足够使用
 */
static void read_disk(int sector, int sector_count) {
	outb(0x1F2, (uint8_t)(sector_count >> 8));
	outb(0x1F2, (uint8_t)(sector_count));
	outb(0x1F3, (uint8_t)(sector >> 24));		// LBA参数的24~31位
	outb(0x1F3, (uint8_t)(sector));				// LBA参数的0~7位
	outb(0x1F4, (uint8_t)(0));					// LBA参数的32~39位
	outb(0x1F4, (uint8_t)(sector >> 8));		// LBA参数的8~15位
	outb(0x1F5, (uint8_t)(0));					// LBA参数的40~47位
	outb(0x1F5, (uint8_t)(sector >> 16));		// LBA参数的16~23位
	outb(0x1F6, (uint8_t)(0xE0));
	outb(0x1F7, (uint8_t)0x24);

	// 读取数据
	uint16_t *data_buf = (uint16_t*) LOADER_START_ADDR;
	while (sector_count-- > 0) {
		// 每次扇区读之前都要检查，等待数据就绪
		while ((inb(0x1F7) & 0x88) != 0x8) {}

		// 读取并将数据写入到缓存中
		for (int i = 0; i < DISK_SECTOR_SIZE / 2; i++) {
			*data_buf++ = inw(0x1F0);
		}
	}

	// 在屏幕上显示加载进度，方便观察
	loading();
}

/**
 * Boot的C入口函数
 * 只完成一项功能，即从磁盘找到loader文件然后加载到内容中，并跳转过去
 */
void boot_entry(void) {
	// 定位到根目录区
	int start_secotr = fat_dbr.BPB_HiddSec + fat_dbr.BPB_RsvdSecCnt
			+ fat_dbr.BPB_FATSz16 * fat_dbr.BPB_NumFATs;

	// 加载根目录区，大小为多少呢？3个吧，肯定包含loader的信息
	read_disk(start_secotr, 3);

	// 解析diritem，找到loader文件
	diritem_t *loader_item = (diritem_t*) LOADER_START_ADDR;
	for (int index = DISK_SECTOR_SIZE / sizeof(diritem_t); index; index--, loader_item++) {
		// 找到根目录的第一个以L开头的文件，肯定是loader了
		// 因为要求安装系统时，loader作为第一个文件最先写入
		if (loader_item->DIR_Name[0] != 'L') {
			continue;
		}

		// 根据第一个簇，找到起始的扇区号, 然后连续读出来
		// loader.bin应当在格式化时写入，所以文件是连续的
		int cluster = (loader_item->DIR_FstClusHI << 16) | loader_item->DIR_FstClusL0;

		start_secotr = start_secotr + fat_dbr.BPB_RootEntCnt * sizeof(diritem_t) / DISK_SECTOR_SIZE
				+ (cluster - FAT_START_CLUSTER) * fat_dbr.BPB_SecPerClus;

		// 文件大小正好是512整数倍时，会多读 一扇区，不影响功能，只能多不能少
		// 但比(size + 511) / 512 更节省代码空间
		read_disk(start_secotr, loader_item->DIR_FileSize / DISK_SECTOR_SIZE + 1);

		// 跳转至loader运行，相比函数调用，更加节省空间
		__asm__ __volatile__("jmp $0, $0x8000");
		//((void (*)()(LOADER_START_ADDR))();
	}
	for (;;) {}
}

