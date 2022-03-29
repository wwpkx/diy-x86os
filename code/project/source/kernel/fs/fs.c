/**
 * 文件系统的系统调用部分实现
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <core/os_cfg.h>
#include <core/list.h>
#include <core/klib.h>
#include <core/task.h>
#include <dev/dev.h>
#include <fs/fat.h>
#include <fs/fs.h>
#include <fs/dir.h>
#include <fs/mount.h>

static xfile_t file_table[FILE_TABLE_MAX_SIZE];		// 所有任务打开的文件列表
static xfile_info_t file_node_table[FILE_NODE_MAX_SIZE];  // 文件信息表表

/**
 * 分配文件结点
 */
static xfile_t * alloc_file (xfile_info_t * file_info) {
	for (int i = 0; i < FILE_TABLE_MAX_SIZE; i++) {
		xfile_t * file = file_table + i;
		if (!file->file_node) {
			k_memset(file, 0, sizeof(xfile_t));
			file->file_node = file_info;
			file_info->ref_count++;
			return file;
		}
	}

	return (xfile_t *)0;
}

/**
 * 释放文件结点
 */
static void free_file (xfile_t * file) {
	file->file_node = (xfile_info_t *)0;
}

/**
 * 分配文件结点
 */
static xfile_info_t * alloc_file_info (void) {
	for (int i = 0; i < FILE_NODE_MAX_SIZE; i++) {
		xfile_info_t * node = file_node_table + i;
		if (node->ref_count == 0) {
			k_memset(node, 0, sizeof(xfile_info_t));
			node->ref_count = 1;
			return node;
		}
	}

	return (xfile_info_t *)0;
}

/**
 * 释放文件结点
 */
static void free_file_info (xfile_info_t * file_node) {
	file_node->ref_count--;
}

/**
 * 在任务文件表中找到一个空闲项
 */
static int alloc_fd (xfile_t * file) {
	task_t * curr = task_current();
	// 分配file结构，找空闲file_table项
	for (int i = 0; i < TASK_FILE_MAX_SIZE; i++) {
		if (curr->file_table[i] == (xfile_t *)0) {
			curr->file_table[i] = file;
			return i;
		}
	}

	return -1;
}

/**
 * 在任务打开文件列表中释放
 * @param fd
 */
static void free_fd (int fd) {
	task_t * curr = task_current();
	curr->file_table[fd] = (xfile_t *)0;
}

/**
 * 打开文件
 */
int xfile_open(const char *path, int flags) {
	int fd = -1;
	xfile_info_t * file_info = (xfile_info_t *)0;
	xfile_t * file = (xfile_t *)0;

	// 先查挂载点
	mount_point_t * mp = fs_mount_find_name(path);
	if (!mp) {
		return -1;
	}

	// 进入下一路径，打开特定文件
	path = jmp_next_sep(path);
	if (!path) {
		return -1;
	}

	// 根据挂载点的不同，选择不同类型的方式打开
	switch (mp->type) {
		case DEV_TTY: {
			// 对于tty设备，每次打开都创建一个新的设备
			int tty = tty_open();
			if (tty < 0) {
				return -1;
			}
			int dev = make_device_num(DEV_TTY, tty, 0);

			// 分配file_info结构，用于创建新文件
			file_info = alloc_file_info();
			if (file_info == (xfile_info_t *)0) {
				tty_close(tty);
				goto error;
			}
			file_info->type = FILE_TTY;
			file_info->size = -1;
			file_info->device = dev;
			k_strncpy(file_info->file_name, path, FILE_NAME_SIZE);

			// 再分配xfile_t结构
			file = alloc_file(file_info);
			if (file == (xfile_t *)0) {
				tty_close(tty);
				goto error;
			}

			// 为任务分配打开文件表
			fd = alloc_fd(file);
			if (fd < 0) {
				tty_close(tty);
				goto error;
			}
			break;
		}
		case DEV_DISK: {
			xfat_open(path);
			break;
		}
		default:
			goto error;
	}
error:
	if (file_info) {
		free_file_info(file_info);
	}

	if (file) {
		free_file(file);
	}

	if (fd >= 0) {
		free_fd(fd);
	}
	return -1;
}

/**
 * 复制一个文件描述符
 */
int xfile_dup(int file) {
	int fd = -1;

	// 超出进程所能打开的全部，退出
	if ((file < 0) && (file >= TASK_FILE_MAX_SIZE)) {
		return -1;
	}

	xfile_t * p_file = task_current()->file_table[file];
	if (p_file) {
		// file结构要重新复制一份，以便两个文件描述符可各自独立使用
		// 如果仅在打开文件表中增加一项，则两个实际指向同一个
		// 分别独写会相互干扰
		xfile_t * new_file = alloc_file(p_file->file_node);
		if (new_file == (xfile_t *)0) {
			return -1;
		}
		k_memcpy(new_file, p_file, sizeof(xfile_t));

		// 在进程表中增加一项
		fd = alloc_fd(new_file);
		if (fd < 0) {
			free_file(new_file);
			return -1;
		}
	}

	return fd;
}

/**
 * 关闭文件
 */
int xfile_close(int file) {
	// 超出进程所能打开的全部，退出
	if ((file < 0) && (file >= TASK_FILE_MAX_SIZE)) {
		return -1;
	}

	// 先关闭底层信息
	xfile_t * p_file = task_current()->file_table[file];
	switch (p_file->file_node->type) {
	case FILE_FILE:
		xfat_close(p_file);
		break;
	case FILE_TTY: {
		tty_close(dev_minor(p_file->file_node->device));
		break;
	}
	default:
		break;
	}

	// 注意释放此结构
	free_file_info(p_file->file_node);
	free_file(p_file);
	free_fd(file);
	return 0;
}

/**
 * 读取文件
 */
int xfile_read(int file, char *buf, int len) {
	// 超出进程所能打开的全部，退出
	if ((file < 0) && (file >= TASK_FILE_MAX_SIZE)) {
		return 0;
	}

	// 取文件描述结构
	xfile_t * p_file = task_current()->file_table[file];
	if (p_file) {
		switch (p_file->file_node->type) {
		case FILE_FILE:
			return xfat_read(buf, len, p_file);
		case FILE_TTY: {
			int tty = dev_minor(p_file->file_node->device);
			return tty_write(tty, buf, len);
		}
		default:
			return -1;
		}
	}

	return -1;
}

/**
 * 文件写操作
 */
int xfile_write(int file, char *buf, int len) {
	// 超出进程所能打开的全部，退出
	if ((file < 0) && (file >= TASK_FILE_MAX_SIZE)) {
		return 0;
	}

	// 取文件描述结构
	xfile_t * p_file = task_current()->file_table[file];
	if (p_file) {
		switch (p_file->file_node->type) {
		case FILE_FILE:
			return xfat_write(buf, len, p_file);
		case FILE_TTY: {
			int tty = dev_minor(p_file->file_node->device);
			return tty_write(tty, buf, len);
		}
		default:
			break;
		}
	}

	return -1;
}

/**
 * 文件定位
 */
int xfile_seek(int file, int offset, int origin) {
	// 超出进程所能打开的全部，退出
	if ((file < 0) && (file >= TASK_FILE_MAX_SIZE)) {
		return 0;
	}

	// 取文件描述结构
	xfile_t * p_file = task_current()->file_table[file];
	if (p_file) {
		switch (p_file->file_node->type) {
		case FILE_FILE:
			return xfat_seek(p_file, offset, origin);
		case FILE_TTY:
			return -1;
		default:
			break;
		}
	}

	return -1;
}


/**
 * 设置设备参数
 */
int xfile_ioctl(int file, int cmd, int data) {
	// 超出进程所能打开的全部，退出
	if ((file < 0) && (file >= TASK_FILE_MAX_SIZE)) {
		return 0;
	}

	// 取文件描述结构
	xfile_t * p_file = task_current()->file_table[file];
	if (p_file) {
		switch (p_file->file_node->type) {
		case FILE_FILE:
			break;
		case FILE_TTY: {
			int tty = dev_minor(p_file->file_node->device);
			return tty_ioctl(tty, cmd, data);
		}
		default:
			break;
		}
	}

	return 0;
}

/**
 * 创建目录
 */
int xfile_mkdir (const char * path) {
	return 0;
}

/**
 * 创建文件
 */
int xfile_mkfile (const char * path) {
	return 0;
}

/**
 * 删除文件
 */
int xfile_rmfile (const char * path) {
	return 0;
}

/**
 * 删除目录
 */
int xfile_rmdir (const char * path) {
	return 0;
}


/**
 * 初始化文件系统
 */
void fs_init (void) {
	// 建立空闲列表
	k_memset(file_table, 0, sizeof(file_table));
	k_memset(file_node_table, 0, sizeof(file_node_table));
	fs_mount_init();
}
