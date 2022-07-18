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
#include "dev/disk.h"


#define TEMP_FILE_ID		100
#define TEMP_ADDR        	(8*1024*1024)      // 在0x800000处缓存原始

static uint8_t * temp_pos;       // 当前位置

/**
 * @brief 文件系统初始化
 */
void fs_init (void) {
    file_table_init();
	disk_init();
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
			int dev_id = dev_open(DEV_DISK, 0xa0, (void *)0);
			dev_read(dev_id, 5000, (uint8_t *)TEMP_ADDR, 80);
            temp_pos = (uint8_t *)TEMP_ADDR;
			return TEMP_FILE_ID;

            // // 暂时直接从扇区1000上读取, 读取大概40KB，足够了
            // read_disk(5000, 80, (uint8_t *)TEMP_ADDR);
            // temp_pos = (uint8_t *)TEMP_ADDR;
            // return TEMP_FILE_ID;
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
