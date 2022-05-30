/**
 * 终端tty
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "dev/tty.h"
#include "dev/kbd.h"
#include "dev/console.h"
#include "dev/dev.h"
#include "tools/list.h"
#include "tools/klib.h"
#include "ipc/mutex.h"
#include <stdarg.h>

static tty_t tty_list[TTY_MAX_COUNT];
static mutex_t tty_list_mutex;
static int curr_tty;					// 当前的tty设备

/**
 * @brief 当前的tty控制
 * @return int 
 */
int tty_current(void) {
	return curr_tty;
}

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
static int to_index (tty_t * tty) {
	return (tty - tty_list);
}

/**
 * @brief 分配一个tty设备
 */
static tty_t * alloc_tty (int dev) {
	tty_t * tty = (tty_t *)0;

	// 找一个空间的tty结构
	mutex_lock(&tty_list_mutex);
	for (int i = 0; i < sizeof(tty_list) / sizeof(tty_t); i++) {
		tty_t * curr = tty_list + i;
		if (curr->dev == (tty_dev_t *)0) {
			tty = curr;
			break;
		}
	}
	mutex_unlock(&tty_list_mutex);

	if (tty) {
		// 根据设备号找到对应的设备属性
		int minor_dev = device_minor(dev);
		if (minor_dev >= sizeof(tty_dev_list) / sizeof(tty_dev_t)) {
			return (tty_t *)0;
		}
		
		tty->dev = tty_dev_list + minor_dev;
		mutex_init(&tty->mutex);
		bfifo_init(&tty->in_fifo, tty->in_buf, TTY_IN_SIZE);
		bfifo_init(&tty->out_fifo, tty->out_buf, TTY_OUT_SIZE);
	}

	return tty;
}

/**
 * @brief 释放一个tty设备
 */
static void free_tty (tty_t * tty) {
	mutex_lock(&tty_list_mutex);
	tty->dev = (tty_dev_t *)0;
	mutex_unlock(&tty_list_mutex);
}

/**
 * 初始化tty列表
 */
void tty_init (void) {
	kernel_memset(tty_list, 0, sizeof(tty_list));
	mutex_init(&tty_list_mutex);
}

/**
 * 打开一个tty设备
 */
int tty_open (int dev) {
	// 分配tty结构
	tty_t * tty = alloc_tty(dev);
	if (!tty) {
		return -1;
	}

	tty_dev_t * tty_deivce = tty->dev;
	tty->device_num = dev;
	tty->echo = 1;			// 默认开启回显

	// 初始化设备
	if (tty_deivce->init) {
		int err = tty_deivce->init(tty);
		if (err < 0) {
			return -1;
		}
	}

	return to_index(tty);
}

/**
 * 从tty中读取数据
 * 主要是从缓存中读取数据, 缓存中有多少数据就读多少数据，如果没有数据则等待
 */
int tty_read (int tty, char * buffer, int size) {
	tty_t * p_tty = to_tty(tty);

	mutex_lock(&p_tty->mutex);
	// 试着读一下，能读多少就读多少。如果没有，则等待
	int read_size = bfifo_get(&p_tty->in_fifo, buffer, size);
	if (read_size == 0) {
		// 即使是等待，也不要等全部，也是有多少先读多少.
		// 先等1个字节，然后下次循环时再重复读取
		read_size = bfifo_read(&p_tty->in_fifo, buffer, 1);
	}

	mutex_unlock(&p_tty->mutex);
	return read_size;
}

/**
 * 向tty中写入数据
 */
int tty_write (int tty, char * buffer, int size) {
	tty_t * p_tty = to_tty(tty);
	int total_size = size;
	char * curr = buffer;

	// size可能比out_fifo的容量大，所以直接用size去写
	// 所以下面每次尽可能多写，能写多少是多少。写不了说明缓存满，等一下
	mutex_lock(&p_tty->mutex);
	while (size > 0) {
		// 先尝试写入，看看实际能写多少。不能用write,如果size比fifo大，会卡死
		int write_size = bfifo_put(&p_tty->out_fifo, curr, size);
		if (write_size <= 0) {
			// 写入不了，可能是缓冲区满，此时应当等有任意可用的空间
			write_size = bfifo_write(&p_tty->out_fifo, curr, 1);
		}

		// 至这里，无论前面是否等了，缓存里面都是有数据的，在这里启动发送
		size -= write_size;
		curr += write_size;

		// 写入缓存中后，再由底层设备从设备中取出数据，完成数据的最终写入
		p_tty->dev->write(p_tty);
	}
	mutex_unlock(&p_tty->mutex);
	return total_size;
}

/**
 * 关闭tty设备
 */
void tty_close (int tty) {
	tty_t * p_tty = to_tty(tty);
	if (p_tty && p_tty->dev->close) {
		p_tty->dev->close(p_tty);
	}
}

/**
 * @brief tty设备接收到数据时的处理
 * 将从底层硬件接受到的数据写入tty的输入队列
 */
void tty_in_data(int tty, char * data, int size) {
	tty_t * p_tty = to_tty(tty);

	// 该函数大概率是由中断调用，所以能写多少就写多少
	// 而不能因缓存满而挂起，这将导致中断被挂起
	mutex_lock(&p_tty->mutex);

	// 在这里要处理一些控制字符的问题
	bfifo_put(&p_tty->in_fifo, data, size);
	if (p_tty->echo) {
		// 并非所有字符都需要回显
		tty_write(tty, data, size);
	}
	
	mutex_unlock(&p_tty->mutex);
}


