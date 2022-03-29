/**
 * 本源码配套的课程为 - 从0到1动手写FAT32文件系统。每个例程对应一个课时，尽可能注释。
 * 作者：李述铜
 * 课程网址：http://01ketang.cc
 * 版权声明：本源码非开源，二次开发，或其它商用前请联系作者。
 */
#include <core/os_cfg.h>
#include <dev/disk.h>
#include <core/task.h>
#include <core/klib.h>
#include <fs/dir.h>
#include <fs/fat.h>

#define	XFAT_MAX_SIZE		4


/**
 * 解析diritem，获取文件类型
 * @param diritem 需解析的diritem
 * @return
 */
static xfile_type_t get_file_type(const diritem_t *diritem) {
    xfile_type_t type;

    if (diritem->DIR_Attr & DIRITEM_ATTR_VOLUME_ID) {
        type = FILE_VOL;
    } else if (diritem->DIR_Attr & DIRITEM_ATTR_DIRECTORY) {
        type = FILE_DIR;
    } else {
        type = FILE_FILE;
    }

    return type;
}

//
///**
// * 将文件路径分割成目录和文件名部分
// * /a/b/c/d 分隔成/a/b/c   d
// * 可能的值：a/b/c    /   a/    /a
// */
//static void splite_path (const char * path, char ** dir_path, char ** file_name) {
//	const char * f_pos = path;
//
//	do {
//		f_pos = skip_sep(f_pos);
//
//		// 定位到下一个/，如果有则说明后面还有文件名，则继续搜索
//		// 如果没有/，则留在上一个位置
//		const char * next = jmp_next_sep(f_pos);
//		if (!next) {
//			break;
//		}
//
//		f_pos = next;
//	}while (1);
//
//	*file_name = f_pos;
//	*dir_path = (f_pos == path) ? (char *)0 : path;
//}
//
///**
// * 解析路径，获取下一个目录名
// */
//static const char * next_path_name(const char * path) {
//	path = skip_sep(path);			// 开头可能有/
//	path = jmp_next_sep(path);		// 跳过当前的名字
//	path = skip_sep(path);			// 跳过开头的/
//
//	return path;
//}


///**
// * 在指定目录下创建子文件
// */
//static int create_sub_file (xfat_t * xfat, int is_dir, int parent_cluster,
//                const char* child_name, int * file_cluster) {
//    int err;
//    xdisk_t * disk = xfat_get_disk(xfat);
//    diritem_t * target_item = (diritem_t *)0;
//    int curr_cluster = parent_cluster, curr_offset = 0;
//    int free_item_cluster = CLUSTER_INVALID, free_item_offset = 0;
//    int file_diritem_sector;
//    int found_cluster, found_offset;
//    int next_cluster, next_offset;
//	int file_first_cluster = FILE_DEFAULT_CLUSTER;
//
//    // 遍历找到空闲项，在目录末尾添加新项
//	diritem_t* diritem;
//    do {
//
//        diritem_t* diritem = diritem_next(xfat, curr_cluster, curr_offset, &next_cluster, &next_offset);
//        if (diritem == (diritem_t *)0) {
//        	return -1;
//        }
//
//        if (diritem->DIR_Name[0] == DIRITEM_NAME_END) {        // 有效结束标记
//            target_item = diritem;
//            break;
//        } else if (diritem->DIR_Name[0] == DIRITEM_NAME_FREE) {
//            // 空闲项, 还要继续检查，看是否有同名项
//            // 记录空闲项的位置
//            if (!is_cluster_valid(free_item_cluster)) {
//                free_item_cluster = curr_cluster;
//                free_item_offset = curr_offset;
//            }
//        } else if (is_filename_match((const char*)diritem->DIR_Name, child_name)) {
//            // 仅名称相同，还要检查是否是同名的文件或目录
//            int item_is_dir = diritem->DIR_Attr & DIRITEM_ATTR_DIRECTORY;
//            if ((is_dir && item_is_dir) || (!is_dir && !item_is_dir)) { // 同类型且同名
//                *file_cluster = get_diritem_cluster(diritem);  // 返回
//                return FS_ERR_EXISTED;
//            } else {    // 不同类型，即目录-文件同名，直接报错
//                return FS_ERR_NAME_USED;
//            }
//        }
//
//        curr_cluster = next_cluster;
//        curr_offset = next_offset;
//    } while (1);
//
//    // 如果是目录且不为dot file， 预先分配目录项空间
//    if (is_dir && strncmp(".", child_name, 1) && strncmp("..", child_name, 2)) {
//        int cluster_count;
//
//        err = allocate_free_cluster(xfat, CLUSTER_INVALID, 1, &file_first_cluster, &cluster_count, 1, 0);
//        if (err < 0) return err;
//
//        if (cluster_count < 1) {
//            return FS_ERR_DISK_FULL;
//        }
//	} else {
//		file_first_cluster = *file_cluster;
//	}
//
//    // 未找到空闲的项，需要为父目录申请新簇，以放置新文件/目录
//    if ((target_item == (diritem_t *)0) && !is_cluster_valid(free_item_cluster)) {
//        int parent_diritem_cluster;
//        int cluster_count;
//
//        int err = allocate_free_cluster(xfat, found_cluster, 1, &parent_diritem_cluster, &cluster_count, 1, 0);
//        if (err < 0)  return err;
//
//        if (cluster_count < 1) {
//            return FS_ERR_DISK_FULL;
//        }
//
//        // 读取新建簇中的第一个扇区，获取target_item
//        file_diritem_sector = cluster_fist_sector(xfat, parent_diritem_cluster);
//        err = xdisk_read_sector(disk, temp_buffer, file_diritem_sector, 1);
//        if (err < 0) {
//            return err;
//        }
//        target_item = (diritem_t *)temp_buffer;     // 获取新簇项
//    } else {    // 找到空闲或末尾
//        int diritem_offset;
//        if (is_cluster_valid(free_item_cluster)) {
//            file_diritem_sector = cluster_fist_sector(xfat, free_item_cluster) + to_sector(disk, free_item_offset);
//            diritem_offset = free_item_offset;
//        } else {
//            file_diritem_sector = cluster_fist_sector(xfat, found_cluster) + to_sector(disk, found_offset);
//            diritem_offset = found_offset;
//        }
//        err = xdisk_read_sector(disk, temp_buffer, file_diritem_sector, 1);
//        if (err < 0) {
//            return err;
//        }
//        target_item = (diritem_t*)(temp_buffer + to_sector_offset(disk, diritem_offset));     // 获取新簇项
//    }
//
//    // 获取目录项之后，根据文件或目录，创建item
//    err = diritem_init_default(target_item, disk, is_dir, child_name, file_first_cluster);
//    if (err < 0) {
//        return err;
//    }
//
//    // 写入所在目录项
//    err = xdisk_write_sector(disk, temp_buffer, file_diritem_sector, 1);
//    if (err < 0) {
//        return err;
//    }
//
//    *file_cluster = file_first_cluster;
//    return err;
//}
//
///**
// * 创建一个目录
// */
//xfile_node_t * xfat_mkdir (const char * path) {
//	char * dir_path, * dir_name;
//
//	splite_path(path, &dir_path, &dir_name);
//
//	xfile_node_t * dir_node;
//	if (dir_path) {
//		// 绝对路径
//		dir_node = xfat_open(dir_path, flags);
//	} else {
//		// 相对路径
//		dir_node = task_current()->curr_dir;
//	}
//
//	xfile_node_t * file_node = create_sub_file(dir_node, 1, dir_name);
//	return file_node;
//}
//
///**
// * 创建一个文件
// */
//xfile_node_t * xfat_mkfile (const char * path, int flags) {
//	char * dir_path, * file_name;
//
//	splite_path(path, &dir_path, &file_name);
//
//	xfile_node_t * dir_node;
//	if (dir_path) {
//		// 绝对路径
//		dir_node = xfat_open(dir_path, flags);
//	} else {
//		// 相对路径
//		dir_node = task_current()->curr_dir;
//	}
//
//	xfile_node_t * file_node = create_sub_file(dir_node, 0, file_name);
//	return file_node;
//}

/**
 * 扫描树形的结构目录，找到对应的目录或目录，返回相应的xfile_node_t结构
 */
xfile_info_t * xfat_open(const char * path) {
	xfat_t * xfat;
	int parent_cluster;
	if (*path == '/') {
		xfat = fs_mount_get_fat(ROOT_DEV);
		parent_cluster = FAT_START_CLUSTER;
	} else {
		xfat = fs_mount_get_fat(task_current()->curr_dir->device);
		parent_cluster = task_current()->curr_dir->start_cluster;
	}

	path = skip_sep(path);   // 开头可能以/开始

    // 不断扫描各个层级的目录名称，直至找到最终的文件或目录信息
	diritem_t * dir_item;

    while (path != (const char *)0) {
		// 依次遍历所有的diritem，找到名字匹配的项目
		int curr_cluster = parent_cluster, curr_offset = 0;
		while ((dir_item = diritem_next(xfat, curr_cluster, curr_offset, &curr_cluster, curr_offset))) {
			if (is_filename_match((const char *)dir_item->DIR_Name, path)) {
				break;;
			}
		}

		// 两种结果：找到或者未找到。未找到，则退出
		if (dir_item == (diritem_t *)0) {
			return (xfile_info_t *)0;
		}

		// 进入下一级目录
		parent_cluster = diritem_cluster(dir_item);
		path = jmp_next_sep(path);
   }

    // 到这里，dir_item应当是最后找到的匹配项目
	xfile_info_t * file_node;
	file_node->size = (int)dir_item->DIR_FileSize;
	file_node->type = get_file_type(dir_item);
	file_node->start_cluster = diritem_cluster(dir_item);
	file_node->parent = parent_cluster;
	file_node->pos_in_parent = 0;
	return file_node;
}

/**
 * 关闭文件
 */
int xfat_close(xfile_t *file) {
	xfile_info_t * file_node = file->file_node;
	xfat_t * xfat = fs_mount_get_fat(file_node->device);
	uint8_t disk_buf[DISK_SIZE_PER_SECTOR];

	xdisk_t * disk = xfat->disk;
	int sector = to_phy_sector(xfat, file_node->parent, file_node->pos_in_parent);
	int offset = to_sector_offset(disk, file_node->pos_in_parent);

	int err = xdisk_read_sector(disk, disk_buf, sector, 1);
	if (err < 0) {
		return err;
	}

	// 更新文件簇号、大小. todo: 更新文件时间
	diritem_t * dir_item = (diritem_t *)(disk_buf + offset);
	dir_item->DIR_FileSize = file_node->size;
	set_diritem_cluster(dir_item, file_node->start_cluster);

	err = xdisk_write_sector(disk, disk_buf, sector, 1);
	if (err < 0) {
		return err;
	}
	return 0;
}

/**
 * 前移文件指针
 */
static int move_file_pos(xfile_t* file, xfat_t * xfat, uint32_t move_bytes, int expand) {
	xfile_info_t * file_node = file->file_node;
	uint32_t to_move = move_bytes;

	// 不要超过文件的大小
	if (file->pos + move_bytes >= file_node->size) {
		to_move = file_node->size - file->pos;
	}

	// 簇间移动调整，需要调整簇
	int offset_in_cluster = to_cluster_offset(xfat, file->pos);
	if (offset_in_cluster + to_move >= xfat->cluster_byte_size) {
		int next = get_next_cluster(xfat, file->curr_cluster);

		// 检查一下是否有足够的空间，如果没有，则分配一个新簇
		if (expand && !is_cluster_valid(next)) {
			next = allocate_cluster(xfat, file->curr_cluster, 1, 0x0);
		}

		file->curr_cluster = next;
	}

	file->pos += to_move;
	return 0;
}

/**
 * 读取指定的文件量，支持目录的读取
 */
int xfat_read(void * buffer, int size, xfile_t * file) {
	xfile_info_t * file_node = file->file_node;
	xfat_t * xfat = fs_mount_get_fat(file_node->device);
	xdisk_t * disk = xfat->disk;
    uint8_t * read_buffer = (uint8_t *)buffer;
    uint8_t disk_buf[DISK_SIZE_PER_SECTOR];

    // 调整读取量，不要超过文件总量
    if (file->pos + size > file_node->size) {
    	size = file_node->size - file->pos;
    }

    // 循环读取，直到遇到文件末尾
    int read_size = 0;
    int sector_size = disk->disk_info->sector_size;
    while (size > 0) {
        int curr_read_bytes = 0;
        int sector_in_cluster = to_sector(disk, to_cluster_offset(xfat, file->pos));  // 簇中的扇区号
        int offset_in_sector = to_sector_offset(disk, file->pos);  // 扇区偏移位置
        int start_sector = cluster_first_sector(xfat, file->curr_cluster) + sector_in_cluster;

        // 不超过一个扇区的读取。起始非扇区开头，或者总量不超过一个扇区
        if ((offset_in_sector > 0) || (size < sector_size)) {
            curr_read_bytes = size;

            // 起始偏移非0，如果跨扇区，只读取当前扇区
            if (offset_in_sector != 0) {
                if (offset_in_sector + size > sector_size) {
                    curr_read_bytes = sector_size - offset_in_sector;
                }
            }

            int err = xdisk_read_sector(disk, disk_buf, start_sector, 1);
            if (err < 0) {
                break;
            }

            // 从缓存中拷贝需要提取的数据
            k_memcpy(read_buffer, disk_buf + offset_in_sector, curr_read_bytes);
        } else {
            // 起始为0，且读取量超过1个扇区，连续读取多扇区
            int sector_count = (uint32_t)to_sector(disk, size);

            // 如果超过一簇，则只读取当前簇
            if ((sector_in_cluster + sector_count) > xfat->sec_per_cluster) {
                sector_count = xfat->sec_per_cluster - sector_in_cluster;
            }

            int err = xdisk_read_sector(disk, read_buffer, start_sector, sector_count);
            if (err < 0) {
                break;
            }

            curr_read_bytes = sector_count * sector_size;
        }

        // 调整读写数量及前移缓存地址
        read_buffer += curr_read_bytes;
        size -= curr_read_bytes;
        read_size += curr_read_bytes;

        // 前移文件指针
		move_file_pos(file, xfat, curr_read_bytes, 0);
	}

    return read_size;
}

/**
 * 写文件指针
 */
int xfat_write(void * buffer, int size, xfile_t * file) {
	xfile_info_t * file_node = file->file_node;
	xfat_t * xfat = fs_mount_get_fat(file_node->device);
	xdisk_t * disk = xfat->disk;
    uint8_t * write_buffer = (uint8_t *)buffer;
    uint8_t disk_buf[DISK_SIZE_PER_SECTOR];

    // 字节为0，无需写，直接退出
    if (size == 0) {
        return 0;
    }

	// 检查一下是否有足够的空间，如果没有，则分配一个新簇
	if (!is_cluster_valid(file->curr_cluster)) {
		file->curr_cluster = allocate_cluster(xfat, file->curr_cluster, 1, 0x0);
	}

    int write_size = 0;
    int sector_size = disk->disk_info->sector_size;
	while (size > 0) {
		int curr_write_bytes = 0;
		int sector_in_cluster = to_sector(disk, to_cluster_offset(xfat, file->pos));  // 簇中的扇区偏移
		int offset_in_sector = to_sector_offset(disk, file->pos);  // 扇区偏移位置
		int start_sector = cluster_first_sector(xfat, file->curr_cluster) + sector_in_cluster;

        // 不超过一个扇区的写，起始偏移量非0，或者读取大小不超过一扇区
        if ((offset_in_sector > 0) || (size < sector_size)) {
            curr_write_bytes = size;

            // 起始偏移非0，如果跨扇区，只写当前扇区
            if (offset_in_sector != 0) {
                if (offset_in_sector + size > sector_size) {
                    curr_write_bytes = sector_size - offset_in_sector;
                }
            }

            // 由于只是写一部分，所以需要先读，再回写
            int err = xdisk_read_sector(disk, disk_buf, start_sector, 1);
            if (err < 0) {
                break;
            }

            k_memcpy(disk_buf + offset_in_sector, write_buffer, curr_write_bytes);
            err = xdisk_write_sector(disk, disk_buf, start_sector, 1);
            if (err < 0) {
                break;
            }
        } else {
            // 起始为0，且写量超过1个扇区，连续写多扇区
            int sector_count = to_sector(disk, size);

            // 如果超过一簇，则只写当前簇
            if ((sector_in_cluster + sector_count) > xfat->sec_per_cluster) {
                sector_count = xfat->sec_per_cluster - sector_in_cluster;
            }

            int err = xdisk_write_sector(disk, write_buffer, start_sector, sector_count);
            if (err < 0) {
                return 0;
            }

            curr_write_bytes = sector_count * sector_size;
        }

        write_buffer += curr_write_bytes;
        size -= curr_write_bytes;
        write_size += curr_write_bytes;

        // 前移文件指针
		move_file_pos(file, xfat, curr_write_bytes, 1);
    }

    return write_size;
}

/**
 * 调整文件读写指针
 */
int xfat_seek(xfile_t * file, int offset, xfile_orgin_t origin) {
	xfile_info_t * file_node = file->file_node;
    int final_pos;

    // 获取最终的定位位置
    switch (origin) {
    case XFAT_SEEK_SET:	// 相对文件开头
        final_pos = offset;
        break;
    case XFAT_SEEK_CUR:	// 相对当前位置
        final_pos = file->pos + offset;
        break;
    case XFAT_SEEK_END:	// 相对文件末尾
        final_pos = file_node->size + offset;
        break;
    default:
        final_pos = -1;
        break;
    }

    // 超出文件范围
    if ((final_pos < 0) || (final_pos > file_node->size)) {
        return -1;
    }

    // 相对于当前要调整的偏移量
    int curr_cluster, curr_pos, offset_to_move;
    if (final_pos >= file->pos) {
    	// 最终位置靠前，从当前位置开始调整
        curr_cluster = file->curr_cluster;
        curr_pos = file->pos;
        offset_to_move = offset;
    } else {
    	// 位置在当前位置之前，需要从头开始遍历
        curr_cluster = file_node->start_cluster;
        curr_pos = 0;
        offset_to_move = final_pos;
    }

    // 不断前移
    xfat_t * xfat = fs_mount_get_fat(file_node->device);
    while (offset_to_move > 0) {
        int offset_in_cluster = to_cluster_offset(xfat, curr_pos);

        // 不超过当前簇，移动后退出
        if (offset_in_cluster + offset_to_move < xfat->cluster_byte_size) {
            curr_pos += offset_to_move;
            break;
        }

        // 超过当前簇，只在当前簇内移动
        int curr_move_bytes = xfat->cluster_byte_size - offset_in_cluster;

        // 进入下一簇: 是否要判断后续簇是否正确？
        file->curr_cluster = get_next_cluster(xfat, curr_cluster);
        if (!is_cluster_valid(file->curr_cluster)) {
        	break;
        }

        // 前移位置
        curr_pos += curr_move_bytes;
        offset_to_move -= curr_move_bytes;
    }

    file->pos = curr_pos;
    return 0;
}

/**
 * 加载FAT文件系统
 */
int xfat_load(xfat_t * xfat, int dev) {
	// 先加载磁盘
	xdisk_t * disk = xdisk_open(dev);
	if (disk < 0) {
		return -1;
	}

	uint8_t disk_buf[DISK_SIZE_PER_SECTOR];

	// 读取dbr参数区
	int err = xdisk_read_sector(disk, disk_buf, disk->start_sector, 1);
	if (err < 0) {
		return err;
	}
	dbr_t * dbr = (dbr_t *)disk_buf;

	// 解析DBR参数，解析出有用的参数
	xfat->fat_start_sector = dbr->BPB_RsvdSecCnt + disk->start_sector;
	xfat->fat_tbl_sectors = dbr->BPB_FATSz16;
	xfat->fat_tbl_nr = dbr->BPB_NumFATs;
	xfat->sec_per_cluster = dbr->BPB_SecPerClus;
	xfat->total_sectors = dbr->BPB_TotSec16;
	xfat->cluster_byte_size = xfat->sec_per_cluster * dbr->BPB_BytsPerSec;
	xfat->root_sector = xfat->fat_start_sector + xfat->fat_tbl_sectors * xfat->fat_tbl_nr;

	// 简单检查是否是fat16文件系统
	if (xfat->fat_tbl_nr != 2) {
		return -1;
	}

	// 先一次性全部读取FAT表: todo: 优化
//	xfat->fat_buffer = (uint8_t *)malloc(xfat->fat_tbl_sectors * disk->disk_info->sector_size);
//	err = xdisk_read_sector(disk, (uint8_t *)xfat->fat_buffer, xfat->fat_start_sector, xfat->fat_tbl_sectors);
//	if (err < 0) {
//		return err;
//	}
	return 0;
}

