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
 * @brief 文件系统初始化
 */
void fs_init (void) {
    file_table_init();
}

/**
 * @brief 检查路径是否正常
 */
static int is_path_valid (const char * path) {
	if ((path == (const char *)0) || (path[0] == '\0')) {
		return 0;
	}

    return 1;
}

/**
 * 打开文件
 */
int sys_open(const char *name, int flags, ...) {
	if (kernel_strncmp(name, "tty", 3) == 0) {
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

		if (kernel_strlen(name) < 5) {
			goto sys_open_failed;
		}

		int num = name[4] - '0';
		int dev_id = dev_open(DEV_TTY, num, 0);
		if (dev_id < 0) {
			goto sys_open_failed;
		}

		file->dev_id = dev_id;
		file->mode = 0;
		file->pos = 0;
		file->ref = 1;
		file->type = FILE_TTY;
		kernel_strncpy(file->file_name, name, FILE_NAME_SIZE);
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
 * 读取文件api
 */
int sys_read(int file, char *ptr, int len) {
    if (file == TEMP_FILE_ID) {
        kernel_memcpy(ptr, temp_pos, len);
        temp_pos += len;
        return len;
    }
    return -1;
}

#include "tools/log.h"

/**
 * 写文件
 */
int sys_write(int file, char *ptr, int len) {
	file = 0;		// 临时使用
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
