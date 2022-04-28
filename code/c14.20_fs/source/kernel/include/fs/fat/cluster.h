/**
 * FAT文件系统的簇管理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef CLUSTER_H
#define CLUSTER_H

#include "comm/types.h"
#include "fs/fat/fat.h"

#define FAT_CLUSTER_INVALID 		0xF8      	// 无效的簇号
#define FAT_CLUSTER_FREE          	0x00     	// 空闲或无效的簇号

// 簇类型
typedef uint16_t cluster_t;			
int cluster_is_valid (cluster_t cluster);
uint32_t cluster_first_sect (fat_t *fat, cluster_t cluster_no);
int cluster_next (fat_t * fat, uint32_t curr, cluster_t * next);
int cluster_erase (fat_t * fat, cluster_t cluster, uint8_t erase_state);
int cluster_alloc (fat_t * fat, cluster_t pre_cluster, uint8_t erase, uint8_t erase_data);
int cluster_free_chain (fat_t *fat, cluster_t cluster);

int cluster_load_free_info (fat_t * fat);

#endif /* CLUSTER_H */
