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
#include "comm/boot_info.h"
#include <sys/stat.h>
#include "dev/console.h"
#include "fs/file.h"
#include "tools/log.h"
#include "dev/dev.h"

#define FS_TABLE_SIZE		10		// 文件系统表数量

static list_t mounted_list;			// 已挂载的文件系统
static list_t free_list;				// 空闲fs列表
static fs_t fs_tbl[FS_TABLE_SIZE];		// 空闲文件系统列表大小

extern fs_op_t devfs_op;

#define TEMP_FILE_ID		100
#define TEMP_ADDR        	(8*1024*1024)      // 在0x800000处缓存原始

static uint8_t * temp_pos;       // 当前位置

/**
* 使用LBA48位模式读取磁盘
*/
static void read_disk(int sector, int sector_count, uint8_t * buf) {
    outb(0x1F6, (uint8_t) (0xE0));

	outb(0x1F2, (uint8_t) (sector_count >> 8));
    outb(0x1F3, (uint8_t) (sector >> 24));		// LBA参数的24~31位
    outb(0x1F4, (uint8_t) (0));					// LBA参数的32~39位
    outb(0x1F5, (uint8_t) (0));					// LBA参数的40~47位

    outb(0x1F2, (uint8_t) (sector_count));
	outb(0x1F3, (uint8_t) (sector));			// LBA参数的0~7位
	outb(0x1F4, (uint8_t) (sector >> 8));		// LBA参数的8~15位
	outb(0x1F5, (uint8_t) (sector >> 16));		// LBA参数的16~23位

	outb(0x1F7, (uint8_t) 0x24);

	// 读取数据
	uint16_t *data_buf = (uint16_t*) buf;
	while (sector_count-- > 0) {
		// 每次扇区读之前都要检查，等待数据就绪
		while ((inb(0x1F7) & 0x88) != 0x8) {}

		// 读取并将数据写入到缓存中
		for (int i = 0; i < SECTOR_SIZE / 2; i++) {
			*data_buf++ = inw(0x1F0);
		}
	}
}

/**
 * @brief 获取指定文件系统的操作接口
 */
static fs_op_t * get_fs_op (fs_type_t type, int major) {
	switch (type) {
	case FS_DEVFS:
		return &devfs_op;
	default:
		return (fs_op_t *)0;
	}
}

/**
 * @brief 挂载文件系统
 */
static fs_t * mount (fs_type_t type, char * mount_point, int dev_major, int dev_minor) {
	fs_t * fs = (fs_t *)0;

	log_printf("mount file system, name: %s, dev: %x", mount_point, dev_major);

	// 遍历，查找是否已经有挂载
 	list_node_t * curr = list_first(&mounted_list);
	while (curr) {
		fs_t * fs = list_node_parent(curr, fs_t, node);
		if (kernel_strncmp(fs->mount_point, mount_point, FS_MOUNTP_SIZE) == 0) {
			log_printf("fs alreay mounted.");
			goto mount_failed;
		}
		curr = list_node_next(curr);
	}

	// 分配新的fs结构
	list_node_t * free_node = list_remove_first(&free_list);
	if (!free_node) {
		log_printf("no free fs, mount failed.");
		goto mount_failed;
	}
	fs = list_node_parent(free_node, fs_t, node);

	// 检查挂载的文件系统类型：不检查实际
	fs_op_t * op = get_fs_op(type, dev_major);
	if (!op) {
		log_printf("unsupported fs type: %d", type);
		goto mount_failed;
	}

	// 给定数据一些缺省的值
	kernel_memset(fs, 0, sizeof(fs_t));
	kernel_strncpy(fs->mount_point, mount_point, FS_MOUNTP_SIZE);
	fs->op = op;
	fs->mutex = (mutex_t *)0;

	// 挂载文件系统
	if (op->mount(fs, dev_major, dev_minor) < 0) {
		log_printf("mount fs %s failed", mount_point);
		goto mount_failed;
	}
	list_insert_last(&mounted_list, &fs->node);
	return fs;
mount_failed:
	if (fs) {
		// 回收fs
		list_insert_first(&free_list, &fs->node);
	}
	return (fs_t *)0;
}

/**
 * @brief 初始化挂载列表
 */
static void mount_list_init (void) {
	list_init(&free_list);
	for (int i = 0; i < FS_TABLE_SIZE; i++) {
		list_insert_first(&free_list, &fs_tbl[i].node);
	}
	list_init(&mounted_list);
}

/**
 * @brief 文件系统初始化
 */
void fs_init (void) {
	mount_list_init();
    file_table_init();

	// 挂载设备文件系统，待后续完成。挂载点名称可随意
	fs_t * fs = mount(FS_DEVFS, "/dev", 0, 0);
	ASSERT(fs != (fs_t *)0);
}

/**
 * @brief 转换目录为数字
 */
int path_to_num (const char * path, int * num) {
	int n = 0;

	const char * c = path;
	while (*c && *c != '/') {
		n = n * 10 + *c - '0';
		c++;
	}
	*num = n;
	return 0;
}

/**
 * @brief 获取下一级子目录
 */
const char * path_next_child (const char * path) {
   const char * c = path;

    while (*c && (*c++ == '/')) {}
    while (*c && (*c++ != '/')) {}
    return *c ? c : (const char *)0;
}

/**
 * 打开文件
 */
int sys_open(const char *name, int flags, ...) {
	if (kernel_strncmp(name, "/dev", 3) == 0) {
        if (!is_path_valid(name)) {
            log_printf("path is not valid.");
            return -1;
        }

        // 分配文件描述符链接。这个过程中可能会被释放
        int fd = -1;
        file_t * file = file_alloc();
        if (file) {
            fd = task_alloc_fd(file);
            if (fd < 0) {
                goto sys_open_failed;
            }
        }

		name = path_next_child(name);
		file->dev_id = -1;
		file->mode = 0;
		file->pos = 0;
		file->ref = 1;

		kernel_strncpy(file->file_name, name, FILE_NAME_SIZE);
		int err = devfs_op.open((fs_t *)0, name, file);
		if (err < 0) {
			goto sys_open_failed;
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
	} else {
		if (name[0] == '/') {
            // 暂时直接从扇区1000上读取, 读取大概40KB，足够了
            read_disk(5000, 80, (uint8_t *)TEMP_ADDR);
            temp_pos = (uint8_t *)TEMP_ADDR;
            return TEMP_FILE_ID;
        }
	}
}

/**
 * 复制一个文件描述符
 */
int sys_dup (int file) {
	// 超出进程所能打开的全部，退出
	if ((file < 0) && (file >= TASK_OFILE_NR)) {
        log_printf("file(%d) is not valid.", file);
		return -1;
	}

	file_t * p_file = task_file(file);
	if (!p_file) {
		log_printf("file not opened");
		return -1;
	}

	int fd = task_alloc_fd(p_file);	// 新fd指向同一描述符
	if (fd >= 0) {
		p_file->ref++;		// 增加引用
		return fd;
	}

	log_printf("No task file avaliable");
    return -1;
}

/**
 * 读取文件api
 */
int sys_read(int file, char *ptr, int len) {
    if (file == TEMP_FILE_ID) {
        kernel_memcpy(ptr, temp_pos, len);
        temp_pos += len;
        return len;
    } else {
		file_t * p_file = task_file(file);
		if (!p_file) {
			log_printf("file not opened");
			return -1;
		}

		return dev_read(p_file->dev_id, 0, ptr, len);
	}
    return -1;
}

#include "tools/log.h"

/**
 * 写文件
 */
int sys_write(int file, char *ptr, int len) {
	file_t * p_file = task_file(file);
	if (!p_file) {
		log_printf("file not opened");
		return -1;
	}

	return dev_write(p_file->dev_id, 0, ptr, len);
}

/**
 * 文件访问位置定位
 */
int sys_lseek(int file, int ptr, int dir) {
    if (file == TEMP_FILE_ID) {
        temp_pos = (uint8_t *)(ptr + TEMP_ADDR);
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

/**
 * @brief 获取文件状态
 */
int sys_fstat(int file, struct stat *st) {
    kernel_memset(st, 0, sizeof(struct stat));
    st->st_size = 0;
    return 0;
}
