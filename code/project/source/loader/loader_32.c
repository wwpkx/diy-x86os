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
#include <core/boot_info.h>
#include <core/elf.h>
#include <core/cpu_instr.h>
#include <core/os_cfg.h>
#include <fs/dir.h>
#include <fs/fat.h>
#include "loader.h"

/**
 * 调试用错误变量，用颜色表示不同的错误类型
 */
typedef enum _err_t {
	ERROR_NO_KERNEL_FILE = 0xFF0000,  // 红色
	ERROR_NOT_ELF_FILE = 0x00FF00,		// 绿色
	ERROR_NO_APP_FILE = 0x0000FF,		// 蓝色
	ERROR_JMP_TO_KERNEL = 0xFFFFFF,		// 白色
	ERROR_BOOT_SECTOR_READ = 0xFFFF00,	//
}err_t;

static boot_info_t * boot_info;						// 引导信息

#define	DISK_SECTOR_SIZE		512
static uint8_t disk_buffer[DISK_SECTOR_SIZE];		// FAT32文件加载所用缓存
static uint8_t boot_sector[DISK_SECTOR_SIZE];		// 引导扇区缓存
static dbr_t * dbr;

// ELF文件存储的位置, 100KB应当足够
static uint8_t file_buffer[1024*100];

/**
 * 显示错误，其实就是画一条不同颜色的细线
 */
static void show_error_code (err_t code) {
	uint32_t * v = (uint32_t *)boot_info->screen_vram;

	// 调整，从下方一点位置开始显示
	v += boot_info->screen_width * 10;
	for (int i = 0; i < boot_info->screen_width; i++) {
		*v++ = code;
	}
}

/**
 * 死机
 */
static void die (err_t code) {
	show_error_code(code);

    for (;;) {
    	hlt();
    }
}

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
	boot_info = (boot_info_t *)BOOT_INFO_ADDR;

	// 在0x7c00处已经有了，其实不必再加载一次的
	// 不过考虑到栈可能改写的的问题，所以还是自己加载一个
	read_disk(boot_info->start_sector, 1, boot_sector);
    dbr = (dbr_t *)boot_sector;

    if ((boot_sector[510] != 0x55) || (boot_sector[511] != 0xaa)) {
        die(ERROR_BOOT_SECTOR_READ);
    }
}

/**
 * 将簇号转换为扇区号
 */
static uint32_t cluster_2_sector(uint32_t cluster) {
	return dbr->BPB_HiddSec + dbr->BPB_RsvdSecCnt
			+ dbr->BPB_FATSz16 * dbr->BPB_NumFATs
			+ dbr->BPB_RootEntCnt * sizeof(diritem_t) / DISK_SECTOR_SIZE
			+ (cluster - FAT_START_CLUSTER) * dbr->BPB_SecPerClus;
}

/**
 * 获取当前簇的下一簇
 */
static uint32_t get_next_cluster_no (uint32_t curr_cluster) {
    uint32_t fat_table_sector = boot_info->start_sector + dbr->BPB_RsvdSecCnt;

    read_disk(fat_table_sector + curr_cluster / (dbr->BPB_BytsPerSec / 2), 1, disk_buffer);
    curr_cluster = ((uint16_t *)disk_buffer)[curr_cluster % (dbr->BPB_BytsPerSec / 2)];
    return curr_cluster;
}

/**
 * 判断目录项中的名字是否是要找的文件名
 */
static int is_filename_equal (const char * name1, const char * name2) {
	const char * c1 = name1, *c2 = name2;

	// 以下不做严格的比较检查，没必要
	for (int i = 0; i < sizeof(XFAT_NAME_LEN); i++) {
		if ((*c1 == '\0') || (*c2 == '\0')) {
			return 1;
		}

		if (*c1++ != *c2++) {
			return 0;
		}
	}

	return 1;
}

/**
 * 加载ELF文件到指定存储空间
 */
static int read_elf_file (const char * file_name, uint8_t * buffer) {
	// 定位到根目录区
	uint32_t sector = dbr->BPB_HiddSec + dbr->BPB_RsvdSecCnt + dbr->BPB_FATSz16 * dbr->BPB_NumFATs;

	// 加载根目录的数据内容的第一个扇区，肯定包含kernel
	read_disk(sector, 1, disk_buffer);

    // 遍历diritem，找到kernel文件
    diritem_t * loader_item = (diritem_t *)disk_buffer;
    for (int index = 0; index < DISK_SECTOR_SIZE; index += sizeof(diritem_t), loader_item++) {
        // not used
        if ((loader_item->DIR_Name[0] == 0xE5) || (loader_item->DIR_Name[0] == 0x00)) {
            continue;
        }

        const char * diritem_name = (const char *)(loader_item->DIR_Name);
        if (is_filename_equal(diritem_name, file_name)) {
            uint32_t read_size = 0;
            uint32_t toatal_size = loader_item->DIR_FileSize;
            uint32_t cluster_size = dbr->BPB_BytsPerSec * dbr->BPB_SecPerClus;
            uint8_t * write_to = buffer;
            uint32_t curr_cluster = (loader_item->DIR_FstClusHI << 16) | loader_item->DIR_FstClusL0;

            // 依次加载并读取
            while (read_size < toatal_size) {
                uint32_t sector_start = cluster_2_sector(curr_cluster);
                uint32_t sector_count = dbr->BPB_SecPerClus;

                read_disk(sector_start, sector_count, write_to);

                // 这里有不指望簇连续。这样kernel就可以随时更新
                curr_cluster = get_next_cluster_no(curr_cluster);
                write_to += cluster_size;
                read_size += cluster_size;
            }

            return 0;
        }
    }

    return -1;
}

/**
 * 解析elf文件，提取内容到相应的内存中
 * @param file_buffer
 * @return
 */
static uint32_t reload_elf_file (uint8_t * file_buffer) {
    // 读取的只是ELF文件，不像BIN那样可直接运行，需要从中加载出有效数据和代码
    // 简单判断是否是合法的ELF文件
    Elf32_Ehdr * elf_hdr = (Elf32_Ehdr *)file_buffer;
    if ((elf_hdr->e_ident[0] != ELF_MAGIC) || (elf_hdr->e_ident[1] != 'E')
        || (elf_hdr->e_ident[2] != 'L') || (elf_hdr->e_ident[3] != 'F')) {
        die(ERROR_NOT_ELF_FILE);
    }

    // 然后从中加载程序头，将内容拷贝到相应的位置
    for (int i = 0; i < elf_hdr->e_phnum; i++) {
        Elf32_Phdr * phdr = (Elf32_Phdr *)(file_buffer + elf_hdr->e_phoff) + i;

        uint8_t * src = file_buffer + phdr->p_offset;
        uint8_t * dest = (uint8_t *)phdr->p_paddr;
        for (int j = 0; j < phdr->p_filesz; j++) {
            *dest++ = *src++;
        }
    }

    return elf_hdr->e_entry;
}

/**
 * 从磁盘上加载内核
 */
void load_kernel(void) {
    init_kernel_load();

#if 1
    // 调用应用程序使用，在没有文件系统前，顺带将应用程序预先加载到内存中
    // 这些名字注意一定要和工程一致，及时改啊，不然加载不了
    const char * app_name[] = {"SNAKE", "COUNTER", "SHELL"};
    for (int i = 0; i < sizeof(app_name) / sizeof(char *); i++) {
        // 读取内核ELF文件到缓存中
        int err = read_elf_file(app_name[i], file_buffer);
        if (err < 0) {
            die(ERROR_NO_APP_FILE);
        }

        reload_elf_file(file_buffer);
   }
#endif

    // 读取内核ELF文件到缓存中
    int err = read_elf_file(KERNEL_FILENAME, file_buffer);
    if (err < 0) {
        die(ERROR_NO_KERNEL_FILE);
    }

    // 解析ELF文件，并通过调用的方式，进入到内核中去执行，同时传递boot参数
    uint32_t kernel_entry = reload_elf_file(file_buffer);

    show_error_code(ERROR_JMP_TO_KERNEL);
    ((void (*)(boot_info_t *))kernel_entry)(boot_info);
}

