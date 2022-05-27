#include "fs/fat/fat.h"
#include "fs/fs.h"
#include "fs/fat/cluster.h"
#include "tools/log.h"

/**
 * 检查指定簇是否可用，非占用或坏簇
 * @param cluster 待检查的簇
 * @return
 */
int cluster_is_valid (uint16_t cluster) {
    cluster &= 0x0FFF;
    return (cluster < 0x0FFF) && (cluster >= 0x2);     // 值是否正确
}

/**
 * @brief 获取簇号在fat表中的所在的扇区号
 */
static int to_fat_tbl_sector (fat_t * fat, uint32_t sector_size, uint32_t cluster) {
    int sector = fat->fat_start + cluster * sizeof(cluster_t) / sector_size;
    return sector;
}

/**
 * @brief 获取簇号在fat表中的所在的扇区号
 */
static int to_fat_tbl_offset (uint32_t sector_size, uint32_t cluster) {
    int offset = cluster * sizeof(cluster_t) % sector_size;
    return offset;
}

/**
 * 获取指定簇的下一个簇
 */
int cluster_next (fat_t * fat, uint32_t curr, cluster_t * next) {
    if (!cluster_is_valid(curr)) {
        *next = FAT_CLUSTER_INVALID;
        return 0;
    }

    // 获取所在的扇区号
    partinfo_t * part = fat->fs->part_info;
    uint32_t sector_size = part->disk->sector_size;

    int sector = to_fat_tbl_sector(fat, sector_size, curr);
    if (sector >= fat->fat_start + fat->fat_tbl_sectors) {
        log_printf("cluster too large. %d", curr);
        return -1;
    }

    // 读入缓存
    int err = disk_read_sector(part->device, fat->fat_buffer, sector, 1);
    if (err < 0) {
        return err;
    }

    // 取数据
    *next = *(cluster_t*)(fat->fat_buffer + to_fat_tbl_offset(sector_size, curr));
    return 0;
}

/**
 * @brief 加载文件系统中的簇信息
 */
int cluster_load_free_info (fat_t * fat) {
    partinfo_t * part = fat->fs->part_info;

    int curr_idx = 0;
    for (int i = 0; i < fat->fat_tbl_sectors; i++) {
        int err = disk_read_sector(part->device, fat->fat_buffer, fat->fat_start + i, 1);
        if (err < 0) {
            return err;
        }

        for (int j = 0; j < part->disk->sector_size; j += sizeof(cluster_t)) {
            cluster_t cluster = *(cluster_t*)(fat->fat_buffer + j);

            if (cluster == FAT_CLUSTER_FREE) {
                fat->cluster_total_free++;       // 累记空闲的簇号

                // 记录第一个空闲的簇号
                if (fat->cluster_next_free == 0) {
                    fat->cluster_next_free = curr_idx;
                }
            }

            curr_idx++;
        }
    }

    return 0;
}
