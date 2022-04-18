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
#include "dev/tty.h"

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
static mutex_t fs_mutex;                // 访问file_table的互斥信号量

/**
 * @brief 分配一个文件描述符
 */
static file_t * file_alloc (void) {
    file_t * file = (file_t *)0;

    mutex_lock(&fs_mutex);
    for (int i = 0; i < FILE_TABLE_SIZE; i++) {
        file_t * pfile = file_table + i;
        if (pfile->ref == 0) {
			kernel_memset(pfile, 0, sizeof(file_t));
            pfile->ref = 1;
			file = pfile;
            break;
        }
    }
    mutex_unlock(&fs_mutex);
    return file;
}

/**
 * @brief 释放文件描述符
 */
static void file_free (file_t * file) {
    mutex_lock(&fs_mutex);
    if (file->ref) {
        file->ref--;
    }
    mutex_unlock(&fs_mutex);
}

/**
 * @brief 增加file的引用计数
 */
void fs_add_ref (file_t * file) {
    mutex_lock(&fs_mutex);
	file->ref++;
    mutex_unlock(&fs_mutex);
}

/**
 * @brief 文件系统初始化
 */
void fs_init (void) {
	// 文件描述符表初始化
	kernel_memset(&file_table, 0, sizeof(file_table));
	mutex_init(&fs_mutex);
}

/**
 * 打开文件
 */
int sys_open(const char *path, int flags, ...) {
	int fd = -1;

	// 必要的参数检查
	if ((path == (const char *)0) || (path[0] == '\0')) {
		return -1;
	}

	int dev;
	if (kernel_strncmp(path, "tty0", sizeof("tty0")) == 0) {
		dev = make_device_num(DEV_TTY, 0);	// 暂时只支持tty0

		// 打开tty设备，检查是否重复打开？
		int tty = tty_open(dev);
		if (tty < 0) {
			return -1;
		}
		
		// 分配文件描述符链接。这个过程中可能会被释放
		mutex_lock(&fs_mutex);
		file_t * file = file_alloc();
		if (file) {
			fd = task_alloc_fd(file);
			if (fd < 0) {
				file_free(file);
				mutex_unlock(&fs_mutex);
				return -1;
			}
		}
		mutex_unlock(&fs_mutex);

		file->dev = dev;
		file->mode = O_RDWR;
		file->pos = 0;
		file->ref = 1;
		file->type = FILE_TTY;
	} else {
		// 分配文件描述符链接。这个过程中可能会被释放
		mutex_lock(&fs_mutex);
		file_t * file = file_alloc();
		if (file) {
			fd = task_alloc_fd(file);
			if (fd < 0) {
				file_free(file);
				mutex_unlock(&fs_mutex);
				return -1;
			}
		}
		mutex_unlock(&fs_mutex);

		file->dev = dev;
		file->mode = O_RDWR;
		file->pos = 0;
		file->ref = 1;
		file->type = FILE_NORMAL;

        // 以下临时使用，后续将替换掉
		// 暂时直接从扇区5000上读取, 读取大概400KB，足够了
		read_disk(5000, 800, (uint8_t *)ADDR);
		pos = (uint8_t *)ADDR;
	}

	return fd;
}

/**
 * 复制一个文件描述符
 */
int sys_dup (int file) {
	// 超出进程所能打开的全部，退出
	if ((file < 0) && (file >= TASK_OFILE_NR)) {
		return -1;
	}

	file_t * pfile = task_file(file);
	if (pfile) {
		int fd = task_alloc_fd(pfile);	// 新fd指向同一描述符
		if (fd >= 0) {
			pfile->ref++;		// 增加引用
			return fd;
		}
	}

	return -1;
}

/**
 * 读取文件api
 */
int sys_read(int file, char *ptr, int len) {
	// 必要的参数检查
	if ((ptr == (char *)0) || (len < 0) || (file < 0) || (file >= TASK_OFILE_NR)) {
		return -1;
	}

	// 无需读取数据
	if (len == 0) {
		return 0;
	}

	// 获取文件描述符，可能为空，如file不合法等
	file_t * pfile = task_file(file);
	if (pfile == (file_t *)0) {
		return -1;
	}

	// 不能写
	if (pfile->mode == O_WRONLY) {
		return -1;
	}

	// 根据文件类型做不同的处理
	switch (pfile->type) {
	case FILE_TTY: {
		int tty = device_minor(pfile->dev);
		return tty_read(tty, ptr, len);
	}	
	default:			// 普通文件
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
	// 必要的参数检查
	if ((ptr == (char *)0) || (len < 0) || (file < 0) || (file >= TASK_OFILE_NR)) {
		return -1;
	}

	// 无需读取数据
	if (len == 0) {
		return 0;
	}

	// 获取文件描述符，可能为空，如file不合法等
	file_t * pfile = task_file(file);
	if (pfile == (file_t *)0) {
		return -1;
	}

	// 不能写
	if (pfile->mode == O_RDONLY) {
		return -1;
	}

	// 根据文件类型做不同的处理
	switch (pfile->type) {
	case FILE_TTY: {
		int tty = device_minor(pfile->dev);
		return tty_write(tty, ptr, len);
	}
	default:			// 普通文件
		kernel_memcpy(ptr, pos, len);
		pos += len;
		return len;
	}

	return -1;
}

/**
 * 文件访问位置定位
 */
int sys_lseek(int file, int ptr, int dir) {
	// 必要的参数检查
	if ((file < 0) && (file >= TASK_OFILE_NR)) {
		return -1;
	}

	// 获取文件描述符，可能为空，如file不合法等
	file_t * pfile = task_file(file);
	if (pfile == (file_t *)0) {
		return -1;
	}

	// 根据文件类型做不同的处理
	switch (pfile->type) {
	case FILE_TTY:		// tty文件
		return -1;		// 不支持
	default:			// 普通文件
		pos = (uint8_t *)(ptr + ADDR);
		return 0;
	}

    return -1;
}

/**
 * 关闭文件
 */
int sys_close(int file) {
	if ((file < 0) && (file >= TASK_OFILE_NR)) {
		return -1;
	}

	file_t * pfile = task_file(file);
	if (pfile == (file_t *)0) {
		return -1;
	}

	ASSERT(pfile->ref > 0);

	mutex_lock(&fs_mutex);
	if (--pfile->ref == 0) {
		// 仅当引用计数到0时才释放
		switch (pfile->type) {
		case FILE_TTY: {
			int tty = device_minor(pfile->dev);
			tty_close(tty);
			break;
		}
		default:
			break;
		}
		file_free(pfile);
		task_remove_fd(file);
	}
	mutex_unlock(&fs_mutex);
	return 0;
}

/**
 * 判断文件描述符与tty关联
 */
int sys_isatty(int file) {
	if ((file < 0) && (file >= TASK_OFILE_NR)) {
		return 0;
	}

	file_t * pfile = task_file(file);
	if (pfile == (file_t *)0) {
		return 0;
	}

	return pfile->type == FILE_TTY;
}
