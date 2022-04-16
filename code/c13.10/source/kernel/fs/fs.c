/**
 * 文件系统相关接口的实现
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "core/task.h"
#include "comm/cpu_instr.h"
#include "tools/klib.h"
#include "fs/fs.h"
#include "fs/file.h"
#include "ipc/mutex.h"
#include "tools/klib.h"
#include "dev/dev.h"

#define ADDR                    (8*1024*1024)      // 在0x800000处缓存原始
#define SYS_DISK_SECTOR_SIZE    512

static uint8_t * pos;       // 当前位置

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
		for (int i = 0; i < SYS_DISK_SECTOR_SIZE / 2; i++) {
			*data_buf++ = inw(0x1F0);
		}
	}
}

static file_t file_table[FILE_TABLE_SIZE];      // 系统中可打开的文件表
static mutex_t file_table_mutex;                // 访问file_table的互斥信号量

/**
 * @brief 分配一个文件描述符
 */
file_t * file_alloc (void) {
    file_t * file = (file_t *)0;

    mutex_lock(&file_table_mutex);
    for (int i = 0; i < FILE_TABLE_SIZE; i++) {
        file_t * file = file_table + i;
        if (file->ref == 0) {
			kernel_memset(file, 0, sizeof(file_t));
            file->ref = 1;
            break;
        }
    }
    mutex_unlock(&file_table_mutex);

    return file;
}

/**
 * @brief 释放文件描述符
 */
void file_free (file_t * file) {
    mutex_lock(&file_table_mutex);
    if (file->ref) {
        file->ref--;
    }
    mutex_unlock(&file_table_mutex);
}

/**
 * @brief 文件系统初始化
 */
void fs_init (void) {
	// 文件描述符表初始化
	kernel_memset(&file_table, 0, sizeof(file_table));
	mutex_init(&file_table_mutex);
}

/**
 * 跳过文件分隔符
 */
const char * skip_sep (const char * path) {
	while (path && *path == '/') {
		path++;
	}
	return path;
}

/**
 * 跳至下一有效名称
 */
const char * dir_path_next(const char * path) {
	// 空字符串
	if (path == (const char *)0) {
		return (const char *)0;
	}

	// 定位到下一个分隔符的后边。可能是有效的名称或者字符串结束
	while (*path && *path++ != '/') {}
	return path;
}

/**
 * 打开文件
 */
int sys_open(const char *name, int flags, ...) {
	// 以下临时使用，后续将替换掉
    if (name[0] == '/') {
        // 暂时直接从扇区1000上读取, 读取大概40KB，足够了
        read_disk(1000, 80, (uint8_t *)ADDR);
        pos = (uint8_t *)ADDR;
        return 100;
    }


	int err = tty_open();
	
	// // 必要的参数检查
	// if ((name == (const char *)name) || (name[0] == '\0')) {
	// 	return -1;
	// }

	// int file_type = FILE_TTY;
	// switch (file_type) {
	// 	case FILE_TTY: {
	// 		path = jmp_next_sep(path);
	// 		if (!path || (*path == '\0')) {
	// 			// 路径错误
	// 			return -1;
	// 		}

	// 		// 看看是否对应的设备已经打开，有则退出
	// 		int tty_num = kernel_itoa(path);   // 取tty设备号
	// 		if ((fd = find_fd(FILE_TTY, tty_num, pid)) >= 0) {
	// 			return fd;
	// 		}

	// 		// 否则，新创建一个
	// 		int tty = tty_open(&tty_driver, (void *)0);
	// 		if (tty < 0) {
	// 			return -1;
	// 		}

	// 		int dev = make_device_num(DEV_TTY, tty, 0);
	// 		fd = alloc_fd(dev, tty_num);
	// 		if (fd < 0) {
	// 			tty_close(tty);
	// 			return -1;
	// 		}

	// 		k_strcpy(file_node->file_name, path);
	// 		break;
	// 	}
	// 	default:
	// 		return -1;
	// }

	return 0;
}

/**
 * 读取文件api
 */
int sys_read(int file, char *ptr, int len) {
    if (file == 100) {
        kernel_memcpy(ptr, pos, len);
        pos += len;
        return len;
    }
    return -1;
}

/**
 * 写文件
 */
int sys_write(int file, char *ptr, int len) {
    return -1;
}

/**
 * 文件访问位置定位
 */
int sys_lseek(int file, int ptr, int dir) {
    if (file == 100) {
        pos = (uint8_t *)(ptr + ADDR);
        return 0;
    }
    return -1;
}

/**
 * 关闭文件
 */
int sys_close(int file) {
}

/**
 * 判断文件描述符与tty关联
 */
int sys_isatty(int file) {
    return -1;
}
