/**
 * 终端tty
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "dev/tty.h"
#include "dev/console.h"
#include "tools/log.h"
#include <setjmp.h>

static tty_t tty_list[TTY_MAX_COUNT];	// tty缓存
static mutex_t mutex;					// 互斥信号量

// tty设备列表
static tty_dev_t tty_dev_list[] = {
	{
		.init = console_init,
		.write = console_write,
	}
};

/**
 * 将tty索引转换为指针
 */
static tty_t * to_tty(int tty) {
	if ((tty >= 0) && (tty < TTY_MAX_COUNT)) {
		return tty_list + tty;
	}

	return (tty_t *)0;
}

/**
 * 转换指针为索引
 */
static inline int to_index (tty_t * tty) {
	return (tty - tty_list);
}

/**
 * @brief 分配一个tty设备
 */
static tty_t * alloc_tty (int dev) {
	tty_t * tty = (tty_t *)0;

	// 找一个空间的tty结构
	mutex_lock(&mutex);
	for (int i = 0; i < sizeof(tty_list) / sizeof(tty_t); i++) {
		tty_t * curr = tty_list + i;
		if (curr->dev == (tty_dev_t *)0) {
			tty = curr;
			break;
		}
	}
	mutex_unlock(&mutex);

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
	mutex_lock(&mutex);
	tty->dev = (tty_dev_t *)0;
	mutex_unlock(&mutex);
}

/**
 * 初始化tty列表
 */
void tty_init (void) {
	kernel_memset(tty_list, 0, sizeof(tty_list));
	mutex_init(&mutex);
}

/**
 * 打开一个tty设备
 */
tty_t * tty_open (const char * name, file_t * file) {
	tty_t * tty = alloc_tty(name);
	if (!tty) {
		log_printf("no tty avaliable");
		return (tty_t *)0;
	}

	if (!tty_deivce->init) {
		log_printf("no tty init func");
		goto open_failed;
	}

	int err = tty_deivce->init(tty);
	if (err < 0) {
		goto open_failed;
	}

	return tty;
open_failed:
	free_tty(tty);
	return (tty_t *)0;
}

/**
 * @brief 向tty写入一定数量的数据，由进程使用
 */
int tty_write (tty_t * tty, char * buf, int size) {

}

/**
 * @brief 从tty读取一定数量的数据，由进程使用
 */
int tty_read (tty_t * tty, char * buf, int size) {

}