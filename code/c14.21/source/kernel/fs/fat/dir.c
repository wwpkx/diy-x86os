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
