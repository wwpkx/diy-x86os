/**
 * 简单的FAT文件系统结构
 * 仅支持打开和读取，不支持写入和目录遍历
 * 
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "fs/fs.h"
#include "fs/fat/fat.h"
#include "fs/fat/cluster.h"
#include "fs/fat/dir.h"
#include "dev/disk.h"
#include "tools/klib.h"
#include "tools/log.h"
#include "core/memory.h"
#include "core/task.h"

#define SECTOR_SIZE         512

static fat_t fat;           // 暂只支持一个fat分区的加载

/**
 * @brief 将位置转换为簇号的扇区号偏移
 */
static inline uint32_t to_cluster_sector_offset (fat_t * fat, uint32_t pos) {
    return pos % fat->cluster_byte_size / SECTOR_SIZE;
}

/**
 * @brief 将偏移量转换为相对于扇区内的偏移
 */
static inline uint32_t to_sector_offset (uint32_t pos) {
    return pos % SECTOR_SIZE;
}

/**
 * @brief 获取pos所在的扇区号，从0开始计数
 */
static inline uint32_t to_sector (fat_t * fat, uint32_t pos) {
    return fat->data_start + pos / SECTOR_SIZE;
}

/**
 * @brief 获取指定位置对应的簇内的偏移量
 */
static inline uint32_t to_cluster_offset (fat_t * fat, uint32_t pos) {
    return pos % fat->cluster_byte_size;
}

/**
 * @brief 加载fat16文件系统类型
 */
int fat_mount (struct _fs_t * fs, partinfo_t * part) {
    dbr_t * dbr = (dbr_t *)memory_alloc_page();
    if (!dbr) {
        log_printf("mount fat failed: can't alloc buf.");
        return -1;
    }
    fat.fat_buffer = (uint8_t *)dbr;
    
    // 这里需要使用查询的方式来读取，因为此时多进程还没有跑起来，只在初始化阶段？
    int cnt = disk_read_sector(part->device, (uint8_t*)dbr, part->start_sector, 1);
    if (cnt <= 0) {
        goto mount_failed;
    }

    // 简单检查是否是fat16文件系统, 可以在下边做进一步的更多检查。此处只检查做一点点检查
	if (dbr->BPB_NumFATs != 2) {
        log_printf("%s: fat table num error: %d", part->name, fat.fat_tbl_nr);
		goto mount_failed;
	}

    if (kernel_memcmp(dbr->BS_FileSysType, "FAT16", 5) != 0) {
        log_printf("%s: not a fat16 file system.", part->name);
        goto mount_failed;
    }

    // 解析DBR参数，解析出有用的参数
    fat.fat_start = dbr->BPB_RsvdSecCnt + part->start_sector;
    fat.fat_tbl_sectors = dbr->BPB_FATSz16;
    fat.fat_tbl_nr = dbr->BPB_NumFATs;
    fat.root_ent_cnt = dbr->BPB_RootEntCnt;
    fat.sec_per_cluster = dbr->BPB_SecPerClus;
    fat.total_sectors = dbr->BPB_TotSec16;
    fat.cluster_byte_size = fat.sec_per_cluster * dbr->BPB_BytsPerSec;
	fat.root_start = fat.fat_start + fat.fat_tbl_sectors * fat.fat_tbl_nr;
    fat.data_start = fat.root_start + fat.root_ent_cnt * sizeof(diritem_t) / SECTOR_SIZE;
    fat.fs = fs;
    
    // 加载簇管理的配置信息
    int err = cluster_load_free_info(&fat);
    if (err < 0) {
        log_printf("%s: load cluster free info failed.", part->name);
        return err;
    }

    fs->op_data = &fat;
    return 0;

mount_failed:
    if (dbr) {
        memory_free_page((uint32_t)dbr);
    }
    return -1;
}

/**
 * @brief 取消加载fat16文件系统类型
 * 
 * @param fs 
 */
void fat_unmount (struct _fs_t * fs) {
    fat_t * fat = (fat_t *)fs->op_data;
    if (fat->fat_buffer) {
        memory_free_page((uint32_t)fat->fat_buffer);
    }
}

/**
 * @brief 扫描，获取文件的状态
 */
int fat_stat(struct _fs_t * fs, const char *file, struct stat *st) {
    fat_t * fat = (fat_t *)fs->op_data;

    // 遍历根目录的数据区，找到已经存在的匹配项
    int sector_cnt = fat->root_ent_cnt * sizeof(cluster_t) / SECTOR_SIZE;
    for (int i = 0; i < sector_cnt; i++) {
        int cnt = disk_read_sector(fs->part_info->device, fat->fat_buffer, fat->root_start + i, 1);
        if (cnt < 0) {
            return -1;
        }
    
        diritem_t * item = (diritem_t *)fat->fat_buffer;
        for (int j = 0; j < SECTOR_SIZE / sizeof(cluster_t); j++, item++) {
            if (diritem_is_match_name(item, file)) {
                file_type_t type = diritem_get_type(item);
                
                kernel_memset(st, 0, sizeof(struct stat));
                st->st_size = item->DIR_FileSize;
                return 0;
            }
        }
    }

    // 如果找不到，根据实际情况创建一个
	return -1;
}

/**
 * 扫描树形的结构目录，找到对应的目录或目录，返回相应的xfile_node_t结构
 */
int fat_open (struct _fs_t * fs, const char * path, file_t * file) {
    fat_t * fat = (fat_t *)fs->op_data;

    if (!is_path_valid(path)) {
        log_printf("Path invalid.");
        return -1;
    }

    // 遍历根目录的数据区，找到已经存在的匹配项
    int sector_cnt = fat->root_ent_cnt * sizeof(cluster_t) / SECTOR_SIZE;
    for (int i = 0; i < sector_cnt; i++) {
        int cnt = disk_read_sector(fs->part_info->device, fat->fat_buffer, fat->root_start + i, 1);
        if (cnt < 0) {
            return -1;
        }
    
        diritem_t * item = (diritem_t *)fat->fat_buffer;
        for (int j = 0; j < SECTOR_SIZE / sizeof(cluster_t); j++, item++) {
            if (diritem_is_match_name(item, path)) {
                file_type_t type = diritem_get_type(item);
                
                // 只能打开文件
                if (type != FILE_NORMAL) {
                    continue;
                }

                kernel_strcpy(file->file_name, path);
                file->type = type;
                file->size = (int)item->DIR_FileSize;
                file->dev = fs->part_info->device;
                file->pos = 0;
                file->mode = 0;
                file->ref = 1;
                file->fs = fs;
                file->start_cluster = diritem_get_cluster(item);
                file->curr_cluster = file->start_cluster;
                return 0;
            }
        }
    }

    // 如果找不到，根据实际情况创建一个
	return -1;
}

static int move_file_pos(file_t* file, fat_t * fat, uint32_t move_bytes) {
	uint32_t cluster_offset = to_cluster_offset(fat, file->pos);

    // 跨簇，则调整curr_cluster。注意，如果已经是最后一个簇了，则curr_cluster不会调整
	if (cluster_offset + move_bytes >= fat->cluster_byte_size) {
		cluster_t next;
		
        int err = cluster_next(fat, file->curr_cluster, &next);
		if (err < 0) {
			return err;
		}

		if (cluster_is_valid(next)) {
			file->curr_cluster = next;
		}
	}

	file->pos += move_bytes;
	return 0;
}

/**
 * @brief 读取指定字节数量的数据
 */
int fat_read (char * buf, int size, struct _file_t * file) {
    int device = file->fs->part_info->device;
    fat_t * fat = (fat_t *)file->fs->op_data;

    // 调整读取量，不要超过文件总量
    uint32_t total_read = 0;
    uint32_t to_read = size;
    if (file->pos + to_read > file->size) {
        to_read = file->size - file->pos;
    }

    while (to_read && cluster_is_valid(file->curr_cluster)) {
        uint32_t curr_read = 0;
		uint32_t cluster_offset = to_cluster_offset(fat, file->pos);
        uint32_t start_sector = fat->data_start + (file->curr_cluster - 2)* fat->sec_per_cluster;  // 从2开始

        // 起点不在簇的开头，或者结尾不在簇的末尾，只先只读取本簇内的，并且读取后复制其呈 小部分
        if (cluster_offset || (cluster_offset + to_read < fat->cluster_byte_size)) {
            curr_read = to_read;

            // 如果跨簇，只读第一个簇内的一部分
            if (cluster_offset + curr_read > fat->cluster_byte_size) {
                curr_read = fat->cluster_byte_size - cluster_offset;
            }

            // 读取整个簇
            int cnt = disk_read_sector(device, fat->fat_buffer, start_sector, fat->sec_per_cluster);
            if (cnt < 0) {
                return 0;
            }

            // 从中拷贝一小部分数据
            kernel_memcpy(buf, fat->fat_buffer + cluster_offset, curr_read);        
        } else {
            // 起始在簇开头以及末尾在簇末尾。此时，有可能刚好占用一个簇，也可能占用多个簇
            uint32_t cluster_count = to_read / fat->cluster_byte_size;
            if (cluster_count > 1) {
                cluster_count = 1;          // 超过一个簇，只读一个
            }
            int cnt = disk_read_sector(device, buf, start_sector, cluster_count * fat->sec_per_cluster);
            if (cnt < 0) {
                return 0;
            }

            curr_read = fat->cluster_byte_size;
        }

        buf += curr_read;
        to_read -= curr_read;
        total_read += curr_read;

        // 前移文件指针
		int err = move_file_pos(file, fat, curr_read);
		if (err < 0) {
            return -1;
        }
	}

    return total_read;
}

/**
 * @brief 写文件
 */
int fat_write (char * buf, int size, struct _file_t * file) {
    return -1;
}

/**
 * @brief 文件位置调整
 */
int fat_seek (file_t * file, uint32_t pos) {
    fat_t * fat = (fat_t *)file->fs->op_data;

    // 简单起见，从簇链的开始重新定位
    cluster_t curr_cluster = file->start_cluster;
    uint32_t curr_pos = 0;
    uint32_t offset_to_move = pos;
    while (offset_to_move > 0) {
        uint32_t cluster_offset = to_cluster_offset(fat, curr_pos);
        uint32_t curr_move = offset_to_move;

        // 不超过当前簇，直接退出设置即可
        if (cluster_offset + curr_move < fat->cluster_byte_size) {
            curr_pos += curr_move;
            break;
        }

        // 超过簇，取当前簇
        int err = cluster_next(fat, curr_cluster, &curr_cluster);
        if (err < 0) {
            return -1;
        }

        // 进入到末尾了？有问题，报错，直接接退出
        if (!cluster_is_valid(curr_cluster)) {
            return -1;  // 直接退出，
        }

        // 超过当前簇，只在当前簇内移动
        curr_move = fat->cluster_byte_size - cluster_offset;
        curr_pos += curr_move;
        offset_to_move -= curr_move;
    }

    file->pos = curr_pos;
    file->curr_cluster = curr_cluster;
    return 0;
}

/**
 * 关闭文件
 */
int fat_close (struct _file_t * file) {
	return -1;
}
