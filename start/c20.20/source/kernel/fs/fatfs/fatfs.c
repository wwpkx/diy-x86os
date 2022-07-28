// /**
//  * 简单的FAT文件系统结构
//  * 仅支持打开和读取，不支持写入和目录遍历
//  * 
//  * 创建时间：2021年8月5日
//  * 作者：李述铜
//  * 联系邮箱: 527676163@qq.com
//  */
#include "fs/fs.h"
#include "fs/fatfs/fatfs.h"
#include "dev/disk.h"
#include "tools/klib.h"
#include "tools/log.h"
#include "core/memory.h"
#include "core/task.h"

// /**
//  * @brief 将位置转换为簇号的扇区号偏移
//  */
// static inline uint32_t to_cluster_sector_offset (fat_t * fat, uint32_t pos) {
//     return pos % fat->cluster_byte_size / SECTOR_SIZE;
// }

// /**
//  * @brief 将偏移量转换为相对于扇区内的偏移
//  */
// static inline uint32_t to_sector_offset (uint32_t pos) {
//     return pos % SECTOR_SIZE;
// }

/**
 * @brief 获取pos所在的扇区号，从0开始计数
 */
static inline uint32_t n_in_blk (uint32_t pos, int blk_size) {
    return pos / blk_size;
}

/**
 * @brief 获取指定位置对应块偏移量
 */
static inline uint32_t off_in_blk (uint32_t pos, int blk_size) {
    return pos % blk_size;
}

/**
 * 检查指定簇是否可用，非占用或坏簇
 */
int cluster_is_valid (uint16_t cluster) {
    cluster &= 0x0FFF;
    return (cluster < 0x0FFF) && (cluster >= 0x2);     // 值是否正确
}

// /**
//  * @brief 获取簇号在fat表中的所在的扇区号
//  */
// static int to_fat_tbl_sector (fat_t * fat, uint32_t sector_size, uint32_t cluster) {
//     int sector = fat->tbl_start + cluster * sizeof(cluster_t) / sector_size;
//     return sector;
// }

// /**
//  * @brief 获取簇号在fat表中的所在的扇区号
//  */
// static int to_fat_tbl_offset (uint32_t sector_size, uint32_t cluster) {
//     int offset = cluster * sizeof(cluster_t) % sector_size;
//     return offset;
// }

/**
 * 获取指定簇的下一个簇
 */
int cluster_next (fat_t * fat, uint32_t curr) {
    if (!cluster_is_valid(curr)) {
        return FAT_CLUSTER_INVALID;
    }

    int offset = curr * sizeof(cluster_t);
    int sector = n_in_blk(offset, fat->bytes_per_sec);
    if (sector >= fat->tbl_start + fat->tbl_sectors) {
        log_printf("cluster too big. %d", curr);
        return FAT_CLUSTER_INVALID;
    }

    int err = dev_read(fat->fs->dev_id, fat->tbl_start + sector, fat->fat_buffer, 1);
    if (err < 0) {
        return FAT_CLUSTER_INVALID;
    }

    offset = off_in_blk(offset, fat->bytes_per_sec);
    return *(cluster_t*)(fat->fat_buffer + offset);
}

/**
 * @brief 转换文件名为diritem中的短文件名，如a.txt 转换成a      txt
 */
static void to_sfn(char* dest, const char* src) {
    kernel_memset(dest, ' ', SFN_LEN);

    // 不断生成直到遇到分隔符和写完缓存
    char * curr = dest;
    char * end = dest + SFN_LEN;
    while (path_is_valid(src) && (curr < end)) {
        char c = *src++;

        switch (c) {
        case '.':       // 隔附，跳到扩展名区，不写字符
            curr = dest + 8;
            break;
        default:
            if ((c >= 'a') || (c <= 'z')) {
                c = c - 'a' + 'A';
            }
            *curr++ = c;
            break;
        }
    }
}

/**
 * @brief 判断item项是否与指定的名称相匹配
 */
int diritem_name_match (diritem_t * item, const char * path) {
    char buf[SFN_LEN];
    to_sfn(buf, path);
    return kernel_memcmp(buf, item->DIR_Name, SFN_LEN) == 0;
}

// /**
//  * 缺省初始化driitem
//  */
// int diritem_init(diritem_t * item, uint8_t attr,
// 		const char * name, cluster_t cluster, uint32_t size) {
//     to_sfn((char *)item->DIR_Name, name);
//     diritem_set_cluster(item, cluster);
//     item->DIR_FileSize = size;
//     item->DIR_Attr = attr;
//     item->DIR_NTRes = 0;
//     item->DIR_CrtTime.hour = 12;
//     item->DIR_CrtTime.minute = 0;
//     item->DIR_CrtTime.second_2 = 0;
//     item->DIR_CrtTimeTeenth = 0;
//     item->DIR_CrtDate.year_from_1980 = 2008 - 1980;
//     item->DIR_CrtDate.month = 12;
//     item->DIR_CrtDate.day = 1;
//     item->DIR_WrtTime = item->DIR_CrtTime;
//     item->DIR_WrtDate = item->DIR_CrtDate;
//     item->DIR_LastAccDate = item->DIR_CrtDate;
//     return 0;
// }

/**
 * 获取diritem的文件起始簇号
 */
uint32_t diritem_get_cluster (diritem_t * item) {
    return (item->DIR_FstClusHI << 16) | item->DIR_FstClusL0;
}

// /**
//  * 设置diritem的cluster
//  */
// void diritem_set_cluster (diritem_t * item, cluster_t cluster) {
//     item->DIR_FstClusHI = (uint16_t )(cluster >> 16);
//     item->DIR_FstClusL0 = (uint16_t )(cluster & 0xFFFF);
// }

static int move_file_pos(file_t* file, fat_t * fat, uint32_t move_bytes) {
	uint32_t cluster_offset = off_in_blk(file->pos, fat->cluster_byte_size);

    // 跨簇，则调整curr_cluster。注意，如果已经是最后一个簇了，则curr_cluster不会调整
	if (cluster_offset + move_bytes >= fat->cluster_byte_size) {		
        cluster_t next = cluster_next(fat, file->cblk);
		if (next == FAT_CLUSTER_INVALID) {
			return -1;
		}

		if (cluster_is_valid(next)) {
			file->cblk = next;
		}
	}

	file->pos += move_bytes;
	return 0;
}

/**
 * @brief 获取diritem中的名称，转换成合适
 */
void diritem_get_name (diritem_t * item, char * dest) {
    char * c = dest;
    char * ext = (char *)0;

    kernel_memset(dest, 0, 11);     // 最多11个字符
    for (int i = 0; i < 11; i++) {
        if (item->DIR_Name[i] != ' ') {
            *c++ = item->DIR_Name[i];
        }

        if (i == 7) {
            ext = c;
            *c++ = '.';
        }
    }

    // 没有扩展名的情况
    if (ext[1] == '\0') {
        ext[0] = '\0';
    }
}

/**
 * @brief 获取文件类型
 */
file_type_t diritem_get_type (diritem_t * item) {
    file_type_t type = FILE_UNKNOWN;

    // 长文件名和volum id
    if (item->DIR_Attr & (DIRITEM_ATTR_VOLUME_ID | DIRITEM_ATTR_HIDDEN | DIRITEM_ATTR_SYSTEM)) {
        return FILE_UNKNOWN;
    }

    return item->DIR_Attr & DIRITEM_ATTR_DIRECTORY ? FILE_DIR : FILE_NORMAL;
}

/**
 * @brief 缓存读取磁盘数据，用于目录的遍历等
 */
static int bread_sector (fat_t * fat, int sector) {
    if (sector == fat->curr_sector) {
        return 0;
    }

    int cnt = dev_read(fat->fs->dev_id, sector, fat->fat_buffer, 1);
    return (cnt == 1) ? 0 : -1;
}

/**
 * @brief 在root目录中读取diritem
 */
static diritem_t * read_dir_entry (fat_t * fat, int index) {
    if ((index < 0) || (index >= fat->root_ent_cnt)) {
        return (diritem_t *)0;
    }

    int offset = index * sizeof(diritem_t);
    int err = bread_sector(fat, fat->root_start + offset / fat->bytes_per_sec);
    if (err < 0) {
        return (diritem_t *)0;
    }
    return (diritem_t *)(fat->fat_buffer + offset % fat->bytes_per_sec);

}

/**
 * @brief 挂载fat文件系统
 */
int fatfs_mount (struct _fs_t * fs, int dev_major, int dev_minor) {
    // 打开设备
    int dev_id = dev_open(dev_major, dev_minor, (void *)0);
    if (dev_id < 0) {
        log_printf("open disk failed. major: %x, minor: %x", dev_major, dev_minor);
        return -1;
    }

    // 读取dbr扇区并进行检查
    dbr_t * dbr = (dbr_t *)memory_alloc_page();
    if (!dbr) {
        log_printf("mount fat failed: can't alloc buf.");
        goto mount_failed;
    }
   
    // 这里需要使用查询的方式来读取，因为此时多进程还没有跑起来，只在初始化阶段？
    int cnt = dev_read(dev_id, 0, (char *)dbr, 1);
    if (cnt < 1) {
        log_printf("read dbr failed.");
        goto mount_failed;
    }

    // 解析DBR参数，解析出有用的参数
    fat_t * fat = &fs->fat_data;
    fat->fat_buffer = (uint8_t *)dbr;
    fat->bytes_per_sec = dbr->BPB_BytsPerSec;
    fat->tbl_start = dbr->BPB_RsvdSecCnt;
    fat->tbl_sectors = dbr->BPB_FATSz16;
    fat->tbl_cnt = dbr->BPB_NumFATs;
    fat->root_ent_cnt = dbr->BPB_RootEntCnt;
    fat->sec_per_cluster = dbr->BPB_SecPerClus;
    fat->total_sectors = dbr->BPB_TotSec16;
    fat->cluster_byte_size = fat->sec_per_cluster * dbr->BPB_BytsPerSec;
	fat->root_start = fat->tbl_start + fat->tbl_sectors * fat->tbl_cnt;
    fat->data_start = fat->root_start + fat->root_ent_cnt * sizeof(diritem_t) / SECTOR_SIZE;
    fat->curr_sector = 0;
    fat->fs = fs;

	// 简单检查是否是fat16文件系统, 可以在下边做进一步的更多检查。此处只检查做一点点检查
	if (fat->tbl_cnt != 2) {
        log_printf("%s: fat table num error, major: %x, minor: %x", dev_major, dev_minor);
		goto mount_failed;
	}

    if (kernel_memcmp(dbr->BS_FileSysType, "FAT16", 5) != 0) {
        log_printf("%s: not a fat16 file system, major: %x, minor: %x", dev_major, dev_minor);
        goto mount_failed;
    }

    // 记录相关的打开信息
    fs->type = FS_FAT16;
    fs->data = &fs->fat_data;
    fs->dev_id = dev_id;
    return 0;

mount_failed:
    if (dbr) {
        memory_free_page((uint32_t)dbr);
    }
    dev_close(dev_id);
    return -1;
}

/**
 * @brief 卸载fatfs文件系统
 */
void fatfs_unmount (struct _fs_t * fs) {
    fat_t * fat = (fat_t *)fs->data;

    dev_close(fs->dev_id);
    memory_free_page((uint32_t)fat->fat_buffer);
}

/**
 * @brief 打开指定的文件
 */
int fatfs_open (struct _fs_t * fs, const char * path, file_t * file) {
    fat_t * fat = (fat_t *)fs->data;

    if (!path_is_valid(path)) {
        log_printf("Path invalid.");
        return -1;
    }

    // 遍历根目录的数据区，找到已经存在的匹配项
    int free_index = -1;
    for (int i = 0; i < fat->root_ent_cnt; i++) {
        diritem_t * item = read_dir_entry(fat, i);
        if (item == (diritem_t *)0) {
            return -1;
        }

         // 结束项，不需要再扫描了，同时index也不能往前走
        if (item->DIR_Name[0] == DIRITEM_NAME_END) {
            break;
        }

        // 只显示普通文件和目录，其它的不显示
        if (item->DIR_Name[0] == DIRITEM_NAME_FREE) {
            free_index = i;
            continue;
        }

        // 找到要打开的目录
        if (diritem_name_match(item, path)) {
            file->type = diritem_get_type(item);;
            file->size = (int)item->DIR_FileSize;
            file->pos = 0;
            file->mode = 0;
            file->ref = 1;
            file->fs = fs;
            file->sblk = diritem_get_cluster(item);
            file->cblk = file->sblk;
            return 0;
        }       
    }

	return -1;
}

int fatfs_read (char * buf, int size, file_t * file) {
    fat_t * fat = (fat_t *)file->fs->data;

    // 调整读取量，不要超过文件总量
    uint32_t nbytes = size;
    if (file->pos + nbytes > file->size) {
        nbytes = file->size - file->pos;
    }

    uint32_t total_read = 0;
    while (nbytes > 0) {
        uint32_t curr_read = nbytes;
		uint32_t cluster_offset = off_in_blk(file->pos, fat->cluster_byte_size);
        uint32_t start_sector = fat->data_start + (file->cblk - 2)* fat->sec_per_cluster;  // 从2开始

        // 如果是整簇, 只读一簇
        if ((cluster_offset == 0) && (nbytes == fat->cluster_byte_size)) {
            int err = dev_read(fat->fs->dev_id, start_sector, buf, fat->sec_per_cluster);
            if (err < 0) {
                return 0;
            }

            curr_read = fat->cluster_byte_size;
        } else {
            // 如果跨簇，只读第一个簇内的一部分
            if (cluster_offset + curr_read > fat->cluster_byte_size) {
                curr_read = fat->cluster_byte_size - cluster_offset;
            }

            // 读取整个簇，然后从中拷贝
            int err = dev_read(fat->fs->dev_id, start_sector, fat->fat_buffer, fat->sec_per_cluster);
            if (err < 0) {
                return 0;
            }
            kernel_memcpy(buf, fat->fat_buffer + cluster_offset, curr_read);        
        }

        buf += curr_read;
        nbytes -= curr_read;
        total_read += curr_read;

        // 前移文件指针
		int err = move_file_pos(file, fat, curr_read);
		if (err < 0) {
            return -1;
        }
	}

    return total_read;
}

int fatfs_write (char * buf, int size, file_t * file) {

}

void fatfs_close (file_t * file) {

}

int fatfs_seek (file_t * file, uint32_t offset, int dir) {
    return -1;          // 不支持，只允许应用程序连续读取
}

int fatfs_stat (file_t * file, struct stat *st) {
    st->st_size = file->size;
    return 0;
}

/**
 * @brief 打开目录。只是简单地读取位置重设为0
 */
int fatfs_opendir (struct _fs_t * fs,const char * name, DIR * dir) {
    dir->index = 0;
    return 0;
}


/**
 * @brief 读取一个目录项
 */
int fatfs_readdir (struct _fs_t * fs,DIR* dir, struct dirent * dirent) {
    fat_t * fat = (fat_t *)fs->data;

    // 做一些简单的判断，检查
    while (dir->index < fat->root_ent_cnt) {
        diritem_t * item = read_dir_entry(fat, dir->index);
        if (item == (diritem_t *)0) {
            return -1;
        }

        // 结束项，不需要再扫描了，同时index也不能往前走
        if (item->DIR_Name[0] == DIRITEM_NAME_END) {
            break;
        }

        // 只显示普通文件和目录，其它的不显示
        if (item->DIR_Name[0] != DIRITEM_NAME_FREE) {
            file_type_t type = diritem_get_type(item);
            if ((type == FILE_NORMAL) || (type == FILE_DIR)) {
                dirent->index = dir->index++;
                dirent->type = diritem_get_type(item);
                dirent->size = item->DIR_FileSize;
                diritem_get_name(item, dirent->name); 
                return 0;           
            }
        }

        dir->index++;
    }
    
    return -1;
}

/**
 * @brief 关闭文件扫描读取
 */
int fatfs_closedir (struct _fs_t * fs,DIR *dir) {
    return 0;
}

/**
 * @brief 删除文件
 */
int fatfs_unlink (struct _fs_t * fs, const char * path) {
    // fat_t * fat = (fat_t *)fs->data;

    // // 做一些简单的判断，检查
    // for (int i = 0; i < fat->root_ent_cnt; i++) {
    //     diritem_t * item = read_dir_entry(fat, i);

    //     // 结束项，不需要再扫描了，同时index也不能往前走
    //     switch (item->DIR_Name[0]) {
    //         case DIRITEM_NAME_END:
    //             break;
    //         case DIRITEM_NAME_FREE:
    //             continue;
    //         default: {
    //             if (diritem_get_type(item) != FILE_NORMAL) {
    //                 continue;
    //             }

                 
    //         }
    //         break;
    //     }
    // }
    
    // return -1;
}

fs_op_t fatfs_op = {
    .mount = fatfs_mount,
    .unmount = fatfs_unmount,
    .open = fatfs_open,
    .read = fatfs_read,
    .write = fatfs_write,
    .seek = fatfs_seek,
    .stat = fatfs_stat,
    .close = fatfs_close,

    .opendir = fatfs_opendir,
    .readdir = fatfs_readdir,
    .closedir = fatfs_closedir,
    .unlink = fatfs_unlink,
};