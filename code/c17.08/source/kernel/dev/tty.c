/**
 * 终端tty
 * 目前只考虑处理cooked模式的处理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "dev/tty.h"
#include "dev/console.h"
#include "dev/kbd.h"
#include "dev/dev.h"
#include "tools/log.h"

static tty_t tty_devs[TTY_NR];

/**
 * @brief FIFO初始化
 */
void tty_fifo_init (tty_fifo_t * fifo, char * buf, int size) {
	fifo->buf = buf;
	fifo->count = 0;
	fifo->size = size;
	fifo->read = fifo->write = 0;
}

/**
 * @brief 取一字节数据
 */
int tty_fifo_get (tty_fifo_t * fifo, char * c) {
	if (fifo->count <= 0) {
		return -1;
	}

	*c = fifo->buf[fifo->read++];
	if (fifo->read >= fifo->size) {
		fifo->read = 0;
	}
	fifo->count--;
	return 0;
}

/**
 * @brief 写一字节数据
 */
int tty_fifo_put (tty_fifo_t * fifo, char c) {
	if (fifo->count >= fifo->size) {
		return -1;
	}

	fifo->buf[fifo->write++] = c;
	if (fifo->write >= fifo->size) {
		fifo->write = 0;
	}
	fifo->count++;

	return 0;
}

/**
 * @brief 判断tty是否有效
 */
static inline tty_t * get_tty (device_t * dev) {
	int tty = dev->minor;
	if ((tty < 0) || (tty >= TTY_NR) || (!dev->open_count)) {
		log_printf("tty is not opened. tty = %d", tty);
		return (tty_t *)0;
	}

	return tty_devs + tty;
}

/**
 * @brief 打开tty设备
 */
int tty_open (device_t * dev)  {
	int idx = dev->minor;
	if ((idx < 0) || (idx >= TTY_NR)) {
		log_printf("open tty failed. incorrect tty num = %d", idx);
		return -1;
	}

	tty_t * tty = tty_devs + idx;
	tty_fifo_init(&tty->ofifo, tty->obuf, TTY_OBUF_SIZE);
	sem_init(&tty->osem, TTY_OBUF_SIZE);
	tty_fifo_init(&tty->ififo, tty->ibuf, TTY_IBUF_SIZE);
	tty->oflags = TTY_OCRLF;
	tty->console_idx = 0;

	kbd_init();
	console_init(idx);
	return 0;
}

/**
 * @brief 从tty读取数据
 */
int tty_read (device_t * dev, int addr, char * buf, int size) {
	return size;
}

/**
 * @brief 向tty写入数据
 */
int tty_write (device_t * dev, int addr, char * buf, int size) {
	if (size < 0) {
		return -1;
	}

	tty_t * tty = get_tty(dev);
	int len = 0;

	// 先将所有数据写入缓存中
	while (size) {
		char c = *buf++;

		// 如果遇到\n，根据配置决定是否转换成\r\n
		if (c == '\n' && (tty->oflags & TTY_OCRLF)) {
			sem_wait(&tty->osem);
			int err = tty_fifo_put(&tty->ofifo, '\r');
			if (err < 0) {
				break;
			}
		}

		// 写入当前字符
		sem_wait(&tty->osem);
		int err = tty_fifo_put(&tty->ofifo, c);
		if (err < 0) {
			break;
		}

		len++;
		size--;

		// 启动输出, 这里是直接由console直接输出，无需中断
		console_write(tty);
	}

	return len;
}

/**
 * @brief 向tty设备发送命令
 */
int tty_control (device_t * dev, int cmd, int arg0, int arg1) {
	
}

/**
 * @brief 关闭tty设备
 */
void tty_close (device_t * dev) {

}

// 设备描述表: 描述一个设备所具备的特性
dev_desc_t dev_tty_desc = {
	.name = "tty",
	.major = DEV_TTY,
	.open = tty_open,
	.read = tty_read,
	.write = tty_write,
	.control = tty_control,
	.close = tty_close,
};