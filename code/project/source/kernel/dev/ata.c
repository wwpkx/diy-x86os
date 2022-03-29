/**
 * 简单的ATA磁盘驱动，只支持两块块硬盘
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <core/cpu_instr.h>
#include <core/irq.h>
#include <dev/ata.h>
#include <dev/disk.h>
#include <dev/dev.h>

static ata_request_t ata_request_buf[ATA_REQUEST_NR]; // 空闲请求缓存
static list_t free_request_list;			// 空闲请求列表
static sem_t mutex;					// 空闲访问的互斥信号量

static list_t ata_request_list;				// 请求列表

/**
 * 初始化请求列表
 */
static void init_request_list (void) {
	// 建立空闲列表
	list_init(&free_request_list);
	for (int i = 0; i < ATA_REQUEST_NR; i++) {
		list_insert_first(&free_request_list, &ata_request_buf[i].node);
	}
	sem_init(&mutex, 1);

	list_init(&ata_request_list);
}

/**
 * 分配一个空闲请求
 */
static ata_request_t * alloc_request (void) {
	ata_request_t * request = (ata_request_t *)0;

	sem_wait(&mutex);
	list_node_t * node = list_remove_first(&free_request_list);
	if (node) {
		request = node_to_parent(node, ata_request_t, node);
	}
	sem_notify(&mutex);

	return request;
}

/**
 * 释放请求项
 */
static void free_request (ata_request_t * request) {
	sem_wait(&mutex);
	list_insert_first(&free_request_list, &request->node);
	sem_notify(&mutex);
}

/**
 * 发送ata命令
 */
static void ata_send_cmd (int drive, int start_sector, int sector_count, int cmd) {
	outb(ATA_PRIMARY_DRIVE, drive);		// 使用LBA寻址，并设置驱动器

	// 需要写吗?
	outb(ATA_PRIMARY_ERROR, (uint8_t) 0);
	outb(ATA_PRIMARY_ERROR, (uint8_t) 0);

	// 必须先写高字节
	outb(ATA_PRIMARY_SECTOR_COUNT, (uint8_t) (sector_count >> 8));	// 扇区数高8位

	outb(ATA_PRIMARY_LBA_LO, (uint8_t) (start_sector >> 24));		// LBA参数的24~31位
	outb(ATA_PRIMARY_LBA_MID, 0);									// 高于32位不支持
	outb(ATA_PRIMARY_LBA_HI, 0);										// 高于32位不支持
	outb(ATA_PRIMARY_SECTOR_COUNT, (uint8_t) (sector_count));		// 扇区数量低8位
	outb(ATA_PRIMARY_LBA_LO, (uint8_t) (start_sector >> 0));			// LBA参数的0-7
	outb(ATA_PRIMARY_LBA_MID, (uint8_t) (start_sector >> 8));		// LBA参数的8-15位
	outb(ATA_PRIMARY_LBA_HI, (uint8_t) (start_sector >> 16));		// LBA参数的16-23位

	// 选择对应的主-从磁盘
	outb(ATA_PRIMARY_CMD, (uint8_t)cmd);
}

/**
 * 等待数据就绪
 */
static void ata_wait_ready (int PRIMARY_base) {
	while ((inb(ATA_PRIMARY_STATUS) & 0x88) != 0x8) {}
}

/**
 * 读取ATA数据端口
 */
static void ata_read_data (uint16_t * buf, int count) {
	for (int i = 0; i < count; i++) {
		*buf++ = inw(ATA_PRIMARY_DATA);
	}
}

/**
 * 读取ATA数据端口
 */
static void ata_write_data (uint16_t * buf, int count) {
	for (int i = 0; i < count; i++) {
		outw(ATA_PRIMARY_DATA, *buf++);
	}
}

/**
 * 检查是否有错误
 */
static int check_error (void) {
	uint8_t status = inb(ATA_PRIMARY_STATUS);
	if (status & 0x1) {
		return -1;
	}

	return 0;
}

/**
 * 向磁盘驱动器发送新请求
 */
static void send_new_request (void) {
	list_node_t * first_node = list_first(&ata_request_list);
	if (first_node) {
		ata_request_t * request = node_to_parent(first_node, ata_request_t, node);
		switch (request->cmd) {
		case ATA_CMD_READ:  // 逐个扇区读取
			ata_send_cmd(request->dev, request->start_sector, 1, ATA_CMD_READ);
			break;
		case ATA_CMD_WRITE: // 逐个扇区写入
			ata_send_cmd(request->dev, request->start_sector, 1, ATA_CMD_WRITE);
			break;
		}
	}
}

/**
 * 添加读写请求项
 */
static void add_request (ata_request_t * request) {
	irq_state_t state = irq_enter_protection();

	list_insert_last(&ata_request_list, &request->node);
	if (list_count(&ata_request_list) == 1) {
		// 这里要开中断，这样键盘中断才能够响应
		irq_leave_protection(state);
		send_new_request();
	} else {
		irq_leave_protection(state);
	}
}

/**
 * 移除请求项
 */
static void remove_request (ata_request_t * request) {
	irq_state_t state = irq_enter_protection();
	list_remove(&ata_request_list, &request->node);
	irq_leave_protection(state);
}

/**
 * primary ata disk中断处理
 */
void do_handler_primary_ata (exception_frame_t *frame) {
	list_node_t * first_node = list_first(&ata_request_list);
	if (first_node) {
		ata_request_t * request = node_to_parent(first_node, ata_request_t, node);

		int err = check_error();
		if (err < 0) {
			// 出错，唤醒
			remove_request(request);
			sem_notify(&request->complete_sem);

			// 不处理错误，继续下一请求
			send_new_request();
		} else {
			switch (request->cmd) {
			case ATA_CMD_READ:
				// 到这里，中断通知有数据，这里读取
				ata_read_data((uint16_t *)request->buf, DISK_SIZE_PER_SECTOR / 2);
				request->buf += DISK_SIZE_PER_SECTOR;

				// 读取完毕，移除当前请求
				if (--request->sector_count == 0) {
					remove_request(request);
					sem_notify(&request->complete_sem);

					// 继续新请求
					send_new_request();
				}
				break;
			case ATA_CMD_WRITE:
				// 写完比，看看还有没有数据要写，没有则释放
				if (--request->sector_count > 0) {
					ata_write_data((uint16_t *)request->buf, DISK_SIZE_PER_SECTOR / 2);
					request->buf += DISK_SIZE_PER_SECTOR;
				} else {
					// 没有数据
					remove_request(request);
					sem_notify(&request->complete_sem);

					// 继续新请求
					send_new_request();
				}
				break;
			}
		}
	}

	// 发送eoi
	pic_send_eoi(IRQ14_HARDDISK);
}

/**
 * ATA读磁盘
 */
int ata_read_sector (struct _disk_info_t  * disk_info, uint8_t * buf, int sector, int count) {
	ata_request_t * request = alloc_request();
	request->cmd = ATA_CMD_READ;
	request->dev = ATA_DRIVE_BASE + dev_sub(disk_info->device);
	request->buf = buf;
	request->sector_count = count;
	request->start_sector = sector;
	sem_init(&request->complete_sem, 0);
	add_request(request);

	// 等待完成
	sem_wait(&request->complete_sem);
	int read_count = count - request->sector_count;
	free_request(request);
	return read_count;
}

/**
 * ATA磁盘写
 */
int ata_write_sector (struct _disk_info_t  * disk_info, uint8_t * buf, int sector, int count) {
	ata_request_t * request = alloc_request();
	request->cmd = ATA_CMD_WRITE;
	request->dev = ATA_DRIVE_BASE + dev_sub(disk_info->device);
	request->buf = buf;
	request->sector_count = count;
	request->start_sector = sector;
	sem_init(&request->complete_sem, 0);
	add_request(request);

	// 等待完成
	sem_wait(&request->complete_sem);
	int read_count = count - request->sector_count;
	free_request(request);
	return read_count;
}

/**
 * 初始化ATA磁盘
 */
int ata_open (struct _disk_info_t * disk_info) {
	return 0;
}

/**
 * 关闭ATA磁盘
 */
void ata_close (struct _disk_info_t * disk_info) {
}

/**
 * ATA读写驱动
 */
static xdisk_driver_t ata_driver = {
		.open = ata_open,
		.close = ata_close,
		.read = ata_read_sector,
		.write = ata_write_sector,
};

/**
 * 检测ATA主总线上的磁盘信息
 */
void ata_device_init (void) {
	int count = 0;

	// 检测主从总线及上面的各个硬盘是否存在
	int dev[] = {0xA0, 0xB0};
	for (int i = 0; i < 2; i++) {
		// 发送命令
		ata_send_cmd(dev[i], 0, 0, ATA_CMD_IDENTIFY);

		// 检测状态，如果为0，则控制器不存在
		// 如果0x1F4 and 0x1F5不为0，则非ATA控制器
		int status =  inb(ATA_PRIMARY_STATUS);
		if (status == 0) {
			continue;
		}

		// 等待数据就绪
		ata_wait_ready(ATA_PRIMARY_STATUS);

		// 读取返回的数据，特别是uint16_t 100 through 103
		// 测试用的盘： 总共102400 = 0x19000， 实测会多一个扇区，为vhd磁盘格式增加的一个扇区
		int sector_count = 0;
		for (int i = 0; i < 256; i++) {
			int data = inw(ATA_PRIMARY_DATA);
			if ((i >= 100) && (i <= 103)) {  // 共4个16位，即8字节
				sector_count |= data << ((i - 100) * 16);		// 小端模式?
			}
		}
		if (sector_count == 0) {
			continue;
		}

		// 注册一个设备
		int dev = make_device_num(DEV_DISK, i, 0);
		disk_register(dev, sector_count, DISK_SIZE_PER_SECTOR, &ata_driver);
		count++;
	}

	if (count) {
		init_request_list();

		// 开启中断
		irq_install(IRQ14_HARDDISK, handler_primary_ata);
		irq_enable(IRQ14_HARDDISK);
	}
}
