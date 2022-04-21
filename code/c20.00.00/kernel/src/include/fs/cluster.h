/**
 * FAT16文件系统的簇管理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef SRC_INCLUDE_FS_CLUSTER_H_
#define SRC_INCLUDE_FS_CLUSTER_H_

#include <core/types.h>
#include <fs/fat.h>

#define CLUSTER_INVALID          0xF8          // 无效的簇号
#define CLUSTER_FREE             0x00           // 空闲的cluster
#define CLUSTER_EMPTY            0x00                // 文件的缺省簇号

typedef uint16_t cluster_t;			// 簇类型
int is_cluster_valid(cluster_t cluster);
uint32_t cluster_first_sector(xfat_t *xfat, cluster_t cluster_no);
int read_cluster(xfat_t *xfat, uint8_t *buffer, cluster_t cluster, uint32_t count);
int get_next_cluster(xfat_t * xfat, cluster_t curr);
int erase_cluster(xfat_t * xfat, cluster_t cluster, uint8_t erase_state);
int allocate_cluster(xfat_t * xfat, cluster_t pre_cluster, uint8_t en_erase, uint8_t erase_data);
int free_cluster_chain(xfat_t *xfat, cluster_t cluster);

/**
 * 绝对字节偏移转换为扇区号
 */
static inline int to_sector (xdisk_t * disk, int offset) {
	return offset / disk->disk_info->sector_size;
}

/**
 * 绝对字节偏移转换为扇区中的偏移
 */
static inline int to_sector_offset (xdisk_t * disk, int offset) {
	return offset % disk->disk_info->sector_size;
}

/**
 * 绝对字节偏移转换为簇号
 */
static inline int to_cluster (xfat_t * xfat, int offset) {
	return offset / xfat->cluster_byte_size;
}

/**
 * 绝对字节偏移转换为簇内偏移
 */
static inline int to_cluster_offset (xfat_t * xfat, int offset) {
	return offset % xfat->cluster_byte_size;
}

/**
 * 将簇号和簇偏移转换为扇区号
 */
static inline int to_phy_sector(xfat_t* xfat, int cluster, int cluster_offset) {
    xdisk_t* disk = xfat->disk;
    return cluster_first_sector(xfat, cluster) + to_sector(disk, cluster_offset);
}

#endif /* SRC_INCLUDE_FS_CLUSTER_H_ */
