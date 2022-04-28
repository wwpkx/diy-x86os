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
#include "fs/fat/fat.h"
#include "ipc/mutex.h"
#include "tools/klib.h"
#include "tools/log.h"
#include "dev/dev.h"
#include "dev/tty.h"
#include "dev/disk.h"
#include "os_cfg.h"

static fs_t root_fs;		// 根文件系统

// 文件系统操作接口
static fs_op_t fs_ops[] = {
	[FS_FAT16_0] = {
		.mount = fat_mount,
		.unmount = fat_unmount,
		.open = fat_open,
		.read = fat_read,
		.seek = fat_seek,
		.stat = fat_stat,
	},
};

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
 * @brief 检查路径是否正常
 */
int is_path_valid (const char * path) {
	if ((path == (const char *)0) || (path[0] == '\0')) {
		return 0;
	}

    return 1;
}

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
 * @brief 加载根文件系统
 */
int fs_load_root (int root_device) {
	log_printf("loading root file system...");

	// 根据文件系统类型模块进行特定的处理
	partinfo_t * part_info = device_to_part(root_device);
	if (!part_info) {
		log_printf("Get part info failed. device= %d", root_device);
		return -1;
	}

	// 检查分区的支持情况
	if (part_info->type >= sizeof(fs_ops)) {
		log_printf("Unsupport file system. deivce = %d, type = %d", root_device, part_info->type);
		return -1;
	}

	root_fs.part_info = part_info;
	root_fs.op = fs_ops + part_info->type;

	// 调用实际的加载机制
	if (!root_fs.op->mount) {
		log_printf("Unsupport file system. deivce = %d, type = %d", root_device, part_info->type);
		return -1;
	}

	int err = root_fs.op->mount(&root_fs, part_info);
	if (err < 0) {
		log_printf("mount fs %s failed", part_info->name);
		return -1;
	}

	return 0;
}

/**
 * @brief 文件系统初始化
 */
void fs_init (void) {
	// 文件描述符表初始化
	kernel_memset(&file_table, 0, sizeof(file_table));
	mutex_init(&fs_mutex);

	// 磁盘初始化
	disk_init();
}

/**
 * 打开文件
 */
int sys_open(const char *path, int flags, ...) {
	int fd = -1;

	// 必要的参数检查
	if (!is_path_valid(path)) {
		return -1;
	}

	// 分配文件描述符链接。这个过程中可能会被释放
	file_t * file = file_alloc();
	if (file) {
		fd = task_alloc_fd(file);
		if (fd < 0) {
			goto sys_open_failed;
		}
	}

	if (kernel_strncmp(path, "tty0", sizeof("tty0")) == 0) {
		int dev = make_device_num(DEV_TTY, 0);	// 暂时只支持tty0

		// 打开tty设备，检查是否重复打开？
		int tty = tty_open(dev);
		if (tty < 0) {
			goto sys_open_failed;
		}
		
		file->dev = dev;
		file->mode = O_RDWR;
		file->pos = 0;
		file->ref = 1;
		file->type = FILE_TTY;
	} else {
		int err = root_fs.op->open(&root_fs, path, file);
		if (err < 0) {
			goto sys_open_failed;
		}

		// 记下读写模式等, 暂时只支持打开已有的文件
		file->mode = flags;
	}

	return fd;
sys_open_failed:
	if (file) {
		file_free(file);
	}

	if (fd >= 0) {
		task_remove_fd(fd);
	}
	return -1;
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
 * @brief 获取文件的状态
 */
int sys_stat(const char *file, struct stat *st) {
	// 必要的参数检查
	if (!is_path_valid(file)) {
		return -1;
	}

	if (st == (struct stat *)0) {
		return -1;
	}

	return root_fs.op->stat(&root_fs, file, st);
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
		case FILE_NORMAL: {
			if (pfile->pos >= pfile->size) {
				return 0;
			}

			return root_fs.op->read(ptr, len, pfile);
		}
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
		default: {
			// 获取最终的定位位置
			uint32_t final_pos;
			switch (dir) {
			case FILE_SEEK_SET:
				final_pos = ptr;
				break;
			case FILE_SEEK_CUR:
				final_pos = pfile->pos + ptr;
				break;
			case FILE_SEEK_END:
				final_pos = pfile->size + ptr;
				break;
			default:
				final_pos = -1;
				break;
			}

			if ((pos < 0) || (final_pos > pfile->size)) {
				return -1;
			}

			return root_fs.op->seek(pfile, final_pos);
		}
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
