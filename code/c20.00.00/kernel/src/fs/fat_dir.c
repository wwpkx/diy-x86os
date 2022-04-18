/*
 * fat_dir.c
 *
 *  Created on: 2021年8月26日
 *      Author: mac
 */
#include <core/klib.h>
#include <fs/fat_dir.h>
#include <fs/fs.h>

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
const char * jmp_next_sep(const char * path) {
	while (path && *path != '/') {
		path++;
	}
	return path;
}

/**
 * 将指定的name按FAT 8+3命名转换
 * @param dest_name
 * @param my_name
 * @return
 */
int to_sfn(char* dest_name, const char* my_name) {
    k_memset(dest_name, ' ', SFN_LEN);

    // 遍历名称，逐个复制字符, 算上.分隔符，最长12字节，如果分离符，则只应有
    my_name = skip_sep(my_name);
    const char * src = my_name;
    char * dest = dest_name;
    while (*src && (*src != '/')) {
    	if (*src == '.') {
            dest = dest_name + 8;
    	} else {
    		char ch = *src;
    		if ((ch >= 'a') && (ch <= 'z')) {
    			ch = ch - 'a' + 'A';
    		}

            *dest++ = ch;
    	}
    }
    return 0;
}

/**
 * 检查是否小写字母
 */
static char is_lower (char c) {
	return (c >= 'a') && (c <= 'z');
}

/**
 * 检查sfn字符串中是否是大写。如果中间有任意小写，都认为是小写
 * @param name
 * @return
 */
static int get_sfn_case_cfg(const char * sfn_name) {
	int case_cfg = 0;

    int name_len;
    const char * src_name = sfn_name;
    const char * ext_dot;
    const char * p;
    int ext_existed;

    // 跳过开头的分隔符
    while (*src_name == '/') {
        src_name++;
    }

    // 找到第一个斜杠之前的字符串，将ext_dot定位到那里，且记录有效长度
    ext_dot = src_name;
    p = src_name;
    name_len = 0;
    while ((*p != '\0') && (*p != '/')) {
        if (*p == '.') {
            ext_dot = p;
        }
        p++;
        name_len++;
    }

    // 如果文件名以.结尾，意思就是没有扩展名？
    // todo: 长文件名处理?
    ext_existed = (ext_dot > src_name) && (ext_dot < (src_name + name_len - 1));
    for (p = src_name; p < src_name + name_len; p++) {
        if (ext_existed) {
            if (p < ext_dot) { // 文件名主体部分大小写判断
                case_cfg |= is_lower(*p) ? DIRITEM_NTRES_BODY_LOWER : 0;
            } else if (p > ext_dot) {
                case_cfg |= is_lower(*p) ? DIRITEM_NTRES_EXT_LOWER : 0;
            }
        } else {
            case_cfg |= is_lower(*p) ? DIRITEM_NTRES_BODY_LOWER : 0;
        }
    }

    return case_cfg;
}

/**
 * 判断两个文件名是否匹配
 * @param name_in_item fatdir中的文件名格式
 * @param my_name 应用可读的文件名格式
 * @return
 */
int is_filename_match(const char *name_in_dir, const char *to_find_name) {
    char temp_name[SFN_LEN];

    // FAT文件名的比较检测等，全部转换成大写比较
    // 根据目录的大小写配置，将其转换成8+3名称，再进行逐字节比较
    // 但实际显示时，会根据diritem->NTRes进行大小写转换
    to_sfn(temp_name, to_find_name);
    return k_memcmp((void *)temp_name, (void *)name_in_dir, SFN_LEN) == 0;
}

/**
 * 缺省初始化driitem
 */
int diritem_init(diritem_t * dir_item, uint8_t attr,
		const char * name, cluster_t cluster, uint32_t size) {
    xfile_time_t timeinfo;

    // todo:获取时间
//    int err = xdisk_curr_time(disk, &timeinfo);
//    if (err < 0) {
//        return err;
//    }

    to_sfn((char *)dir_item->DIR_Name, name);
    set_diritem_cluster(dir_item, cluster);
    dir_item->DIR_FileSize = size;
    dir_item->DIR_Attr = attr;
    dir_item->DIR_NTRes = get_sfn_case_cfg(name);

    dir_item->DIR_CrtTime.hour = timeinfo.hour;
    dir_item->DIR_CrtTime.minute = timeinfo.minute;
    dir_item->DIR_CrtTime.second_2 = (uint16_t)(timeinfo.second / 2);
    dir_item->DIR_CrtTimeTeenth = (uint8_t)((timeinfo.second & 1) * 1000);

    dir_item->DIR_CrtDate.year_from_1980 = (uint16_t)(timeinfo.year - 1980);
    dir_item->DIR_CrtDate.month = timeinfo.month;
    dir_item->DIR_CrtDate.day = timeinfo.day;

    dir_item->DIR_WrtTime = dir_item->DIR_CrtTime;
    dir_item->DIR_WrtDate = dir_item->DIR_CrtDate;
    dir_item->DIR_LastAccDate = dir_item->DIR_CrtDate;
    return 0;
}

/**
 * 获取下一个有效的目录项
 * @return
 */
diritem_t * diritem_next(xfat_t* xfat, cluster_t curr_cluster, uint32_t curr_offset,
		cluster_t* next_cluster, uint32_t* next_offset) {
	uint8_t disk_buffer[DISK_SIZE_PER_SECTOR];
	if (!is_cluster_valid(curr_cluster)) {
		return (diritem_t *)0;
	}

	int dir_offset = to_sector_offset(xfat->disk, curr_offset);
	int dir_sector = to_phy_sector(xfat, curr_cluster, curr_offset);
	if (dir_offset == 0) {
		// 没有item表，则读取表
		int err = xdisk_read_sector(xfat->disk, disk_buffer, dir_sector, 1);
		if (err < 0) {
			return (diritem_t *)0;
		}
	}

	// 前移位置
	if ((sizeof(diritem_t) + curr_offset) >= xfat->cluster_byte_size) {
		*next_cluster = get_next_cluster(xfat, curr_cluster);
		*next_offset = 0;
	} else {
		*next_cluster = curr_cluster;
		*next_offset = sizeof(diritem_t) + curr_offset;
	}
	return (diritem_t*)(disk_buffer + dir_offset);
}


