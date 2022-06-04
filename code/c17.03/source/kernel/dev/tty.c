/**
 * 终端tty
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "dev/tty.h"
#include "tools/log.h"

static tty_t tty_list[TTY_MAX_COUNT];	// tty缓存
static mutex_t alloc_mutex;			// 分配互斥信号量
static int curr_tty;					// 当前的tty设备


/**
 * @brief 分配一个tty设备
 */
static tty_t * alloc_tty (int dev) {
	tty_t * tty = (tty_t *)0;

	// 找一个空间的tty结构
	mutex_lock(&alloc_mutex);
	for (int i = 0; i < sizeof(tty_list) / sizeof(tty_t); i++) {
		tty_t * curr = tty_list + i;
		if (curr->dev == (tty_dev_t *)0) {
			tty = curr;
			break;
		}
	}
	mutex_unlock(&alloc_mutex);

	if (tty) {
		// 根据设备号找到对应的设备属性
		int minor_dev = device_minor(dev);
		if (minor_dev >= sizeof(tty_dev_list) / sizeof(tty_dev_t)) {
			return (tty_t *)0;
		}
		
		tty->dev = tty_dev_list + minor_dev;
		mutex_init(&tty->mutex);
		tty->pre_in_size = 0;		// 预先没有数据量
		bfifo_init(&tty->in_fifo, tty->in_buf, TTY_IN_SIZE);
		bfifo_init(&tty->out_fifo, tty->out_buf, TTY_OUT_SIZE);
	}

	return tty;
}

/**
 * @brief 释放一个tty设备
 */
static void free_tty (tty_t * tty) {
	mutex_lock(&alloc_mutex);
	tty->dev = (tty_dev_t *)0;
	mutex_unlock(&alloc_mutex);
}

/**
 * 初始化tty列表
 */
void tty_init (void) {
	kernel_memset(tty_list, 0, sizeof(tty_list));
	mutex_init(&alloc_mutex);
	curr_tty = -1;
}

/**
 * 打开一个tty设备
 */
int tty_open (const char * name, file_t * file) {
	tty_t * tty = alloc_tty(dev);
	if (!tty) {
		log
		return -1;
	}

	tty_dev_t * tty_deivce = tty->dev;
	tty->device_num = dev;

	// 初始化设备
	if (tty_deivce->init) {
		int err = tty_deivce->init(tty);
		if (err < 0) {
			return -1;
		}
	}

	int idx = to_index(tty);
	if (curr_tty < 0) {
		curr_tty = idx;
	}
	return idx;
}
