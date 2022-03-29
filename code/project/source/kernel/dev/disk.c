/**
 * 磁盘操作接口，与具体设备无关
 *
 * 源码主要来源于 从0到1动手写FAT32文件系统 课程，并进行了调整
 * 在这一层涉及磁盘的原始读取，可考虑在读写之前加点缓存处理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <dev/disk.h>
#include <dev/ata.h>
#include <dev/dev.h>
#include <core/klib.h>
#include <core/cpu_instr.h>

static disk_info_t disk_info_list[XDISK_MAX_COUNT];			// 磁盘列表
static int disk_count;										// 总的磁盘数量

static list_t disk_free_list;								// 磁盘分配列表
static xdisk_t disk_free_buf[XDISK_PART_MAX_COUNT];

static uint8_t sector_buf[DISK_SIZE_PER_SECTOR];				// 读写缓存

/**
 * 注册一个disk设备
 */
void disk_register (int device, int sector_num, int sector_size, xdisk_driver_t * driver) {
	if (disk_count < XDISK_MAX_COUNT) {
		disk_info_t * disk_info = disk_info_list + disk_count;
		disk_info->total_sectors = sector_num;
		disk_info->sector_size = sector_size;
		disk_info->device = device;
		disk_info->driver = driver;

		k_memset(disk_info->part, 0, sizeof(disk_info->part));
		disk_count++;
	}
}

/**
 * 获取指定序号的分区信息
 * 注意，该操作依赖物理分区分配，如果设备的分区结构有变化，则序号也会改变，得到的结果不同
 */
void detect_part_info(disk_info_t * disk_info) {
	disk_info->driver->open(disk_info);

	// 读取mbr
    disk_info->driver->read(disk_info, sector_buf, 0, 1);

	// 遍历4个主分区描述，不考虑支持扩展分区
	mbr_part_t * mbr_part = ((mbr_t *)sector_buf)->part_info;
    part_info_t * part_info = disk_info->part;
	for (int i = 0; i < MBR_PRIMARY_PART_NR; i++, mbr_part++) {
		if (mbr_part->system_id == FS_NOT_VALID) {
			continue;
        }

		// 在主分区中找到，复制信息
		part_info->type = mbr_part->system_id;
		part_info->start_sector = mbr_part->relative_sectors;
		part_info->total_sector = mbr_part->total_sectors;
		part_info++;
	}

	disk_info->driver->close(disk_info);
}

/**
 * 磁盘列表初始化
 */
void xdisk_list_init (void) {
	// 先清空所有
	k_memset(disk_info_list, 0, sizeof(disk_info_list));
	list_init(&disk_free_list);
	for (int i = 0; i < sizeof(disk_free_buf) / sizeof(disk_free_buf[0]); i++) {
		list_insert_first(&disk_free_list, &disk_free_buf[i].node);
	}

	disk_count = 0;
}

/**
 * 初始化磁盘设备
 * 根据指定的device号，找到相应的磁盘信息，然后生成disk文件
 */
xdisk_t * xdisk_open(int device) {
	// 如果之前没有加载过，先加载一遍磁盘
	if (disk_count == 0) {
		// 检测出磁盘数量, 在此可加上其它类型的设备检测
		ata_device_init();

		// 检测所有磁盘设备上的分区信息，此处与具体设备无关
		for (int i = 0; i < disk_count; i++) {
			detect_part_info(disk_info_list + i);
		}
	}

    // 次设备号不能超过磁盘数量，子设备号不超过分区数
    int minor = dev_minor(device);
    int sub = dev_sub(device);
    if ((minor >= disk_count) || (sub + 1) >= MBR_PRIMARY_PART_NR) {
    	return (xdisk_t *)0;
    }

    // 先找磁盘，再找分区
    for (int i = 0; i < disk_count; i++) {
		disk_info_t * disk_info = disk_info_list + i;

    	// 先找磁盘
    	if (minor != dev_minor(disk_info->device)) {
    		continue;
    	}

    	// 再找分区
    	int total_sector, start_sector;
    	if (sub > 0) {
        	part_info_t * part = disk_info->part + sub - 1;

        	// 分区不存在，退出
        	if (part->type == FS_NOT_VALID) {
            	return (xdisk_t *)0;
        	}
        	total_sector = part->total_sector;
        	start_sector = part->start_sector;
    	} else {
    		total_sector = disk_info->total_sectors;
    		start_sector = 0;
    	}

    	// 分配结点，写入
    	list_node_t * node = list_remove_first(&disk_free_list);
		if (node) {
			xdisk_t * disk = node_to_parent(node, xdisk_t, node);

			// 初始化磁盘结构
			disk->device = device;
			disk->total_sector = total_sector;
			disk->start_sector = start_sector;
			disk->disk_info = disk_info;

			// 打开设备
			disk_info->driver->open(disk_info);
			return disk;
    	}
    }

    return (xdisk_t *)0;
}

/**
 * 关闭存储设备
 */
void xdisk_close(xdisk_t * disk) {
    xdisk_driver_t * driver = disk->disk_info->driver;
	driver->close(disk->disk_info);
}

/**
 * 从设备中读取指定扇区数量的数据
 * 只是作了简单的封装
 */
int xdisk_read_sector(xdisk_t *disk, uint8_t *buffer, int start_sector, int count) {
    if (start_sector + count >= disk->total_sector) {
        return -1;
    }

    xdisk_driver_t * driver = disk->disk_info->driver;
    int err = driver->read(disk->disk_info, buffer, start_sector, count);
	return err;
}

/**
 * 向设备中写指定的扇区数量的数据
 * 只是作了简单的封装
 */
int xdisk_write_sector(xdisk_t *disk, uint8_t *buffer, int start_sector, int count) {
    if (start_sector + count >= disk->total_sector) {
        return -1;
    }

    xdisk_driver_t * driver = disk->disk_info->driver;
    int err = driver->write(disk->disk_info, buffer, start_sector, count);
	return err;
}



