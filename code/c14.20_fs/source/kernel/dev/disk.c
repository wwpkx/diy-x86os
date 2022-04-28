/**
 * 磁盘驱动
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "dev/dev.h"
#include "dev/disk.h"
#include "tools/klib.h"
#include "tools/log.h"
#include "comm/cpu_instr.h"
#include "cpu/irq.h"
#include "core/memory.h"
#include "core/task.h"

static disk_t disk_buf[DISK_CNT];  // 通道结构
static mutex_t channel_mutex[DISK_CHANNEL_CNT];     // 通道信号量
static sem_t channel_op_sem[DISK_CHANNEL_CNT];      // 通道操作的信号量

/**
 * 发送ata命令，支持多达16位的扇区，对我们目前的程序来书够用了。
 */
static void ata_send_cmd (disk_t * disk, uint32_t start_sector, uint32_t sector_count, int cmd) {
    outb(ATA_DRIVE(disk), ATA_DRIVE_BASE | disk->drive);		// 使用LBA寻址，并设置驱动器

	// 必须先写高字节
	outb(ATA_SECTOR_COUNT(disk), (uint8_t) (sector_count >> 8));	// 扇区数高8位
	outb(ATA_LBA_LO(disk), (uint8_t) (start_sector >> 24));		// LBA参数的24~31位
	outb(ATA_LBA_MID(disk), 0);									// 高于32位不支持
	outb(ATA_LBA_HI(disk), 0);										// 高于32位不支持
	outb(ATA_SECTOR_COUNT(disk), (uint8_t) (sector_count));		// 扇区数量低8位
	outb(ATA_LBA_LO(disk), (uint8_t) (start_sector >> 0));			// LBA参数的0-7
	outb(ATA_LBA_MID(disk), (uint8_t) (start_sector >> 8));		// LBA参数的8-15位
	outb(ATA_LBA_HI(disk), (uint8_t) (start_sector >> 16));		// LBA参数的16-23位

	// 选择对应的主-从磁盘
	outb(ATA_CMD(disk), (uint8_t)cmd);
}

/**
 * 读取ATA数据端口
 */
static inline void ata_read_data (disk_t * disk, void * buf, int size) {
	insw(ATA_DATA(disk), buf, size / 2);
}

/**
 * 读取ATA数据端口
 */
static inline void ata_write_data (disk_t * disk, void * buf, int size) {
	outsw(ATA_DATA(disk), buf, size / 2);
}

/**
 * @brief 等待磁盘有数据到达
 */
static inline int ata_wait_data (disk_t * disk) {
    uint8_t status;
	do {
        // 等待数据或者有错误
        status = inb(ATA_STATUS(disk));
        if ((status & (ATA_STATUS_BUSY | ATA_STATUS_DRQ | ATA_STATUS_ERR))
                        != ATA_STATUS_BUSY) {
            break;
        }
    }while (1);

    // 检查是否有错误
    return (status & ATA_STATUS_ERR) ? -1 : 0;
}

/**
 * @brief 打印磁盘信息
 */
static void print_disk_info (disk_t * disk) {
    log_printf("%s:", disk->name);
    log_printf("\tchannel:%s", disk->channel == ATA_CHANNEL_PRIMARY? "Primary" : "Secondary");
    log_printf("\tdrive: %s", disk->drive == ATA_DISK_MASTER ? "Master" : "Slave");
    log_printf("\tport_base: %x", disk->port_base);
    log_printf("\ttotal_size: %d m", disk->sector_cnt * disk->sector_size / 1024 /1024);

    // 显示分区信息
    log_printf("\tPart info:");
    for (int i = 0; i < DISK_PRIMARY_PART_CNT; i++) {
        partinfo_t * part_info = disk->partinfo + i;
        if (part_info->type != FS_INVALID) {
            log_printf("\t\t%s: type: %x, start sector: %d, count %d", 
                    part_info->name, part_info->type, 
                    part_info->start_sector, part_info->total_sector);
        }
    }
}

/**
 * @brief 获取指定分区的设备号
 */
int part_to_device (disk_t * disk, int part_no) {
    int minor = ((disk - disk_buf + 0xa) << 4) | part_no;        // 每个磁盘最多支持256个分区
    return make_device_num(DEV_DISK, minor);
}

/**
 * @brief 根据设备号，获取分区信息
 */
partinfo_t * device_to_part (int device) {
    int minor = device_minor(device);

    // 不能超过磁盘数量
    int disk_idx = ((minor >> 4) & 0xF) - 0xa;       // 每块硬盘从'a'开始
    if (disk_idx >= DISK_CNT) {
        return (partinfo_t *)0;
    }

    // 不能超过分区数量
    int part_no = minor & 0xF;
    if (part_no >= DISK_PRIMARY_PART_CNT) {
        return (partinfo_t *)0;
    }

    return disk_buf[disk_idx].partinfo + part_no - 1;       // 从1开始算起
}

/**
 * 获取指定序号的分区信息
 * 注意，该操作依赖物理分区分配，如果设备的分区结构有变化，则序号也会改变，得到的结果不同
 */
static int detect_part_info(disk_t * disk) {
    mbr_t mbr;

    // 读取mbr区
    ata_send_cmd(disk, 0, 1, ATA_CMD_READ);
    int err = ata_wait_data(disk);
    if (err < 0) {
        log_printf("read mbr failed");
        return err;
    }
    ata_read_data(disk, &mbr, sizeof(mbr));

	// 遍历4个主分区描述，不考虑支持扩展分区
	part_item_t * item = mbr.part_item;
    partinfo_t * part_info = disk->partinfo;
	for (int i = 0; i < MBR_PRIMARY_PART_NR; i++, item++, part_info++) {
		part_info->type = item->system_id;

        // 没有分区，清空part_info
		if (part_info->type == FS_INVALID) {
			part_info->total_sector = 0;
            part_info->start_sector = 0;
            part_info->disk = (disk_t *)0;
        } else {
            // 在主分区中找到，复制信息
            kernel_sprintf(part_info->name, "%s%d", disk->name, i + 1);
            part_info->device = part_to_device(disk, i + 1);  // 从1开始计算
            part_info->start_sector = item->relative_sectors;
            part_info->total_sector = item->total_sectors;
            part_info->disk = disk;
        }
	}
}

/**
 * @brief 检测磁盘相关的信息
 */
static int identify_disk (disk_t * disk) {
    ata_send_cmd(disk, 0, 0, ATA_CMD_IDENTIFY);

    // 检测状态，如果为0，则控制器不存在
    int err = inb(ATA_STATUS(disk));
    if (err == 0) {
        log_printf("%s doesn't exist\n", disk->name);
        return -1;
    }

    // 等待数据就绪, 此时中断还未开启，因此暂时可以使用查询模式
    err = ata_wait_data(disk);
    if (err < 0) {
        log_printf("disk[%s]: read failed!\n", disk->name);
        return err;
    }

    // 读取返回的数据，特别是uint16_t 100 through 103
    // 测试用的盘： 总共102400 = 0x19000， 实测会多一个扇区，为vhd磁盘格式增加的一个扇区
    int sector_count = 0;
    for (int i = 0; i < 256; i++) {
        uint16_t data = inw(ATA_DATA(disk));
        if ((i >= 100) && (i <= 103)) {  // 共4个16位，即8字节
            sector_count |= data << ((i - 100) * 16);		// 小端模式?
        }
    }

    disk->sector_cnt = sector_count;
    disk->sector_size = 512;            // 固定为512字节大小

    // 接下来识别硬盘上的分区信息
    detect_part_info(disk);
    return 0;
}

/**
 * @brief 磁盘模块初始化
 */
void disk_init (void) {
    
    // 通道端口地址
    static uint16_t port_base[] = {
        [ATA_CHANNEL_PRIMARY] = 0x1F0,
        [ATA_CHANNEL_SECONDARY] = 0x170,
    };    
    
    uint8_t disk_cnt = *((uint8_t*)(0x475));	// 从BIOS数据区，读取硬盘的数量
    ASSERT(disk_cnt > 0);

    log_printf("%d disk found!\n", disk_cnt) ;
    log_printf("Checking disk...");

    // 清空所有disk，以免数据错乱。不过引导程序应该有清0的，这里为安全再清一遍
    kernel_memset(disk_buf, 0, sizeof(disk_buf));

    // 初始化通道结构
    for (int i = ATA_CHANNEL_PRIMARY; i < ATA_CHANNEL_END; i++) {
        // 信号量和锁
        mutex_t * mutex = channel_mutex + i;
        sem_t * sem = channel_op_sem + i;
        mutex_init(mutex);
        sem_init(sem, 0);       // 没有操作完成

        // 检测各个硬盘, 读取硬件是否存在，有其相关信息
        int channel_disk_cnt = 0;
        for (int j = 0; j < DISK_CNT_PER_CHANNEL; j++) {
            int idx = i * DISK_CNT_PER_CHANNEL + j;
            disk_t * disk = disk_buf + idx;

            // 先初始化各字段
            kernel_sprintf(disk->name, "sd%c", idx + 'a');
            disk->channel = i;
            disk->drive = (j == 0) ? ATA_DISK_MASTER : ATA_DISK_SLAVE;
            disk->port_base = port_base[disk->channel];
            disk->irq_num = (j == 0) ? IRQ14_HARDDISK_PRIMARY : IRQ15_HARDDISK_SECOND;
            disk->mutex = mutex;
            disk->op_sem = sem;
            disk->sector_cnt = 0;
            disk->sector_size = 0;
            disk->task_on_op = 0;

            // 识别磁盘，有错不处理，直接跳过
            int err = identify_disk(disk);
            if (err == 0) {
                channel_disk_cnt++;
                print_disk_info(disk);
            }
        }

        // 如果当前通道有磁盘，则开启该通道的
        // 当前通道的中断设置，参考：https://wiki.osdev.org/IRQ
        if (channel_disk_cnt) {
            irq_install(IRQ14_HARDDISK_PRIMARY + i, i == 0 ? handler_ide_primary : handler_ide_secondary);
            irq_enable(IRQ14_HARDDISK_PRIMARY + i);
        }
    }
}

/**
 * @brief 连续多指定磁盘设备的多个扇区
 */
int disk_read_sector(int device, uint8_t *buffer, int start_sector, int count) {
    // 取分区信息
    partinfo_t * part_info = device_to_part(device);
    if (!part_info) {
        log_printf("Get part info failed! device = %d", device);
        return -1;
    }

    disk_t * disk = part_info->disk;
    if (disk == (disk_t *)0) {
        log_printf("No disk for device %d", device);
        return -1;
    }

    int cnt;
    if (task_current() == (task_t *)0) {
        // 关中断，避免中断发送信号量通知
        irq_disable(disk->irq_num);

        ata_send_cmd(disk, start_sector, count, ATA_CMD_READ);
        for (cnt = 0; cnt < count; cnt++, buffer += disk->sector_size) {
            // 等待数据就绪
            int err = ata_wait_data(disk);
            if (err < 0) {
                log_printf("disk(%s) read error: start sect %d, count %d", disk->name, start_sector, count);
                break;
            }

            // 此处再读取数据
            void * pbuffer = (void *)memory_get_paddr(memory_kernel_page_dir(), (uint32_t)buffer);
            ata_read_data(disk, pbuffer, disk->sector_size);
        }

        irq_enable(disk->irq_num);
    } else {
        mutex_lock(disk->mutex);

        disk->task_on_op = 1;
        ata_send_cmd(disk, start_sector, count, ATA_CMD_READ);
        for (cnt = 0; cnt < count; cnt++, buffer += disk->sector_size) {
            // 利用信号量等待中断通知，然后再读取数据
            sem_wait(disk->op_sem);

            // 这里虽然有调用等待，但是由于已经是操作完毕，所以并不会等
            int err = ata_wait_data(disk);
            if (err < 0) {
                log_printf("disk(%s) read error: start sect %d, count %d", disk->name, start_sector, count);
                break;
            }

            // 此处再读取数据
            void * pbuffer = (void *)memory_get_paddr(memory_current_page_dir(), (uint32_t)buffer);
            ata_read_data(disk, pbuffer, disk->sector_size);
        }
    
        mutex_unlock(disk->mutex);
    }
    return cnt;
}

/**
 * @brief 连续读磁盘设备的多个扇区
 */
int disk_write_sector(int device, uint8_t *buffer, int start_sector, int count) {
   // 取分区信息
    partinfo_t * part_info = device_to_part(device);
    if (!part_info) {
        log_printf("Get part info failed! device = %d", device);
        return -1;
    }

    disk_t * disk = part_info->disk;
    if (disk == (disk_t *)0) {
        log_printf("No disk for device %d", device);
        return -1;
    }

    int cnt;
    if (task_current() == (task_t *)0) {
        irq_disable(disk->irq_num);

        ata_send_cmd(disk, start_sector, count, ATA_CMD_WRITE);
        for (cnt = 0; cnt < count; cnt++, buffer += disk->sector_size) {
            // 先写数据
            void * pbuffer = (void *)memory_get_paddr(memory_kernel_page_dir(), (uint32_t)buffer);
            ata_write_data(disk, pbuffer, disk->sector_size);

            // 等待写入完成
            int err = ata_wait_data(disk);
            if (err < 0) {
                log_printf("disk(%s) write error: start sect %d, count %d", disk->name, start_sector, count);
                break;
            }
        }

        irq_enable(disk->irq_num);
    } else {
        mutex_lock(disk->mutex);

        disk->task_on_op = 1;
        ata_send_cmd(disk, start_sector, count, ATA_CMD_WRITE);
        for (cnt = 0; cnt < count; cnt++, buffer += disk->sector_size) {
            // 先写数据
            void * pbuffer = (void *)memory_get_paddr(memory_current_page_dir(), (uint32_t)buffer);
            ata_write_data(disk, pbuffer, disk->sector_size);

            // 利用信号量等待中断通知，等待写完成
            sem_wait(disk->op_sem);

            // 这里虽然有调用等待，但是由于已经是操作完毕，所以并不会等
            int err = ata_wait_data(disk);
            if (err < 0) {
                log_printf("disk(%s) write error: start sect %d, count %d", disk->name, start_sector, count);
                break;
            }
        }
    
        mutex_unlock(disk->mutex);
    }
    return cnt;
}

/**
 * @brief 磁盘主通道中断处理
 */
void do_handler_ide_primary (exception_frame_t *frame)  {
    pic_send_eoi(IRQ14_HARDDISK_PRIMARY);

    // 通知主通道的信号量
    if (disk_buf[0].task_on_op || disk_buf[1].task_on_op) {
        sem_notify(channel_op_sem + 0);
    }
}

/**
 * @brief 磁盘第二通道中断处理
 */
void do_handler_ide_secondary (exception_frame_t *frame) {
    pic_send_eoi(IRQ15_HARDDISK_SECOND);

    // 通知从通道的信号量
    if (disk_buf[2].task_on_op || disk_buf[3].task_on_op) {
        sem_notify(channel_op_sem + 1);
    }
}

