/*
 * diritem.h
 *
 *  Created on: 2021年8月26日
 *      Author: mac
 */
#include "fs/fs.h"
#include "fs/fat/dir.h"
#include "tools/klib.h"


/**
 * @brief 转换文件名为diritem中的短文件名，如a.txt 转换成a      txt
 */
static void to_sfn(char* dest, const char* src) {
    kernel_memset(dest, ' ', SFN_LEN);

    // 不断生成直到遇到分隔符和写完缓存
    char * curr = dest;
    char * end = dest + SFN_LEN;
    while (is_path_valid(src) && (curr < end)) {
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
int diritem_is_match_name (diritem_t * item, const char * path) {
    char buf[SFN_LEN];

    // FAT文件名的比较检测等，全部转换成大写比较
    // 根据目录的大小写配置，将其转换成8+3名称，再进行逐字节比较
    to_sfn(buf, path);
    return kernel_memcmp(buf, item->DIR_Name, SFN_LEN) == 0;
}

/**
 * 缺省初始化driitem
 */
int diritem_init(diritem_t * item, uint8_t attr,
		const char * name, cluster_t cluster, uint32_t size) {
    to_sfn((char *)item->DIR_Name, name);
    diritem_set_cluster(item, cluster);
    item->DIR_FileSize = size;
    item->DIR_Attr = attr;
    item->DIR_NTRes = 0;
    item->DIR_CrtTime.hour = 12;
    item->DIR_CrtTime.minute = 0;
    item->DIR_CrtTime.second_2 = 0;
    item->DIR_CrtTimeTeenth = 0;
    item->DIR_CrtDate.year_from_1980 = 2008 - 1980;
    item->DIR_CrtDate.month = 12;
    item->DIR_CrtDate.day = 1;
    item->DIR_WrtTime = item->DIR_CrtTime;
    item->DIR_WrtDate = item->DIR_CrtDate;
    item->DIR_LastAccDate = item->DIR_CrtDate;
    return 0;
}

/**
 * 获取diritem的文件起始簇号
 */
uint32_t diritem_get_cluster (diritem_t * item) {
    return (item->DIR_FstClusHI << 16) | item->DIR_FstClusL0;
}

/**
 * 设置diritem的cluster
 */
void diritem_set_cluster (diritem_t * item, cluster_t cluster) {
    item->DIR_FstClusHI = (uint16_t )(cluster >> 16);
    item->DIR_FstClusL0 = (uint16_t )(cluster & 0xFFFF);
}

/**
 * @brief 获取文件类型
 */
file_type_t diritem_get_type (diritem_t *diritem) {
    file_type_t type;

    if (diritem->DIR_Attr & DIRITEM_ATTR_VOLUME_ID) {
        type = FILE_UNKNOWN;
    } else if (diritem->DIR_Attr & DIRITEM_ATTR_DIRECTORY) {
        type = FILE_DIR;
    } else {
        type = FILE_NORMAL;
    }

    return type;
}
