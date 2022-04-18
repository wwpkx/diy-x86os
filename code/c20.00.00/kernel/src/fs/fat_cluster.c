/*
 * fat_cluster.c
 *
 *  Created on: 2021年8月26日
 *      Author: mac
 */
#include <core/klib.h>
#include <fs/fat_cluster.h>
#include <dev/disk.h>
#include <fs/fat.h>

/**
 * 检查指定簇是否可用，非占用或坏簇
 * @param cluster 待检查的簇
 * @return
 */
int is_cluster_valid(cluster_t cluster) {
	return (cluster < 0xFFF8) && (cluster >= FAT_START_CLUSTER);     // 值是否正确
}

/**
 * 获取指定簇号的第一个扇区编号
 * @param xfat xfat结构
 * @param cluster_no  簇号
 * @return 扇区号
 */
uint32_t cluster_first_sector(xfat_t *xfat, cluster_t cluster) {
    uint32_t data_start_sector = xfat->fat_start_sector + xfat->fat_tbl_sectors * xfat->fat_tbl_nr;
    return data_start_sector + (cluster - FAT_START_CLUSTER) * xfat->sec_per_cluster;    // 前两个簇号保留
}

/**
 * 获取指定簇的下一个簇
 * @param xfat xfat结构
 * @param curr_cluster_no
 * @param next_cluster
 * @return
 */
int get_next_cluster(xfat_t * xfat, cluster_t curr) {
    if (is_cluster_valid(curr)) {
    	cluster_t * cluster = (cluster_t *)xfat->fat_buffer;
        return cluster[curr];
    }

    return CLUSTER_INVALID;
}

/**
 * 清除指定簇的所有数据内容，内容填0
 * @param xfat
 * @param cluster
 * @param erase_state 擦除状态的字节值
 * @return
 */
int erase_cluster(xfat_t * xfat, cluster_t cluster, uint8_t erase_state) {
	char disk_buffer[DISK_SIZE_PER_SECTOR];
    uint32_t sector = cluster_first_sector(xfat, cluster);
    xdisk_t * xdisk = xfat->disk;

    k_memset(disk_buffer, erase_state, xdisk->disk_info->sector_size);
    for (int i = 0; i < xfat->sec_per_cluster; i++) {
        int err = xdisk_write_sector(xdisk, disk_buffer, sector + i, 1);
        if (err < 0) {
            return err;
        }
    }
    return 0;
}

/**
 * 回写fat表到磁盘中
 */
static void sync_fat_to_disk (xfat_t * xfat) {
	xdisk_t * disk = xfat->disk;

	// FAT表项可能有多个，同时更新
	for (int i = 0; i < xfat->fat_tbl_nr; i++) {
		uint32_t start_sector = xfat->fat_start_sector + xfat->fat_tbl_sectors * i;
		xdisk_write_sector(disk, (uint8_t *)xfat->fat_buffer, start_sector, xfat->fat_tbl_sectors);
	}
}
/**
 * 分配空闲簇
 */
int allocate_cluster(xfat_t * xfat, cluster_t pre_cluster, uint8_t en_erase, uint8_t erase_data) {
    xdisk_t * disk = xfat->disk;
    uint32_t total = xfat->fat_tbl_sectors * disk->disk_info->sector_size / sizeof(cluster_t);
    cluster_t * clusters = (cluster_t *)xfat->fat_buffer;

    // 从头到尾去遍历
    for (int i = FAT_START_CLUSTER; i < total; i++) {
    	// 空闲状态，分配出去
        if (clusters[i] == 0) {
            if (is_cluster_valid(pre_cluster)) {
            	// 建立链接
            	clusters[pre_cluster] = i;
            }

            // 标记为已经占用
            clusters[pre_cluster] = CLUSTER_INVALID;
            if (en_erase) {
            	// 擦除簇，用于目录
			   erase_cluster(xfat, i, erase_data);
		   }

            // 回写刚才修改的fat表
            sync_fat_to_disk(xfat);
            return i;
       }
    }

    return CLUSTER_INVALID;
}

/**
 * 解除簇的链接关系
 * @param xfat xfat结构
 * @param cluster 将该簇之后的所有链接依次解除, 并将该簇之后标记为解囊
 * @return
 */
int free_cluster_chain(xfat_t *xfat, cluster_t cluster) {
	int write_back;
    xdisk_t * disk = xfat->disk;

    // 先在缓冲区中解除链接关系
    cluster_t curr = cluster;
    while (is_cluster_valid(curr)) {
        // 先获取一下簇
        cluster_t next = get_next_cluster(xfat, curr);

        // 标记该簇为空闲状态
        cluster_t * clusters = (cluster_t *)xfat->fat_buffer;
        clusters[curr] = CLUSTER_FREE;

        curr = next;
        write_back = 1;
    }

    // 回写
    sync_fat_to_disk(xfat);
    return 0;
}
