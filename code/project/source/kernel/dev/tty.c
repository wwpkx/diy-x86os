/**
 * 终端tty
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <dev/tty.h>
#include <core/list.h>
#include <stdarg.h>
#include <ui/tty_widget.h>

static dev_tty_t tty_buffer[TTY_MAX_COUNT];
static list_t tty_list;					// 已经打开的tty列表

/**
 * 将tty索引转换为指针
 */
static dev_tty_t * to_tty(int tty) {
	return tty_buffer + tty;
}

/**
 * 转换指针为索引
 */
static int to_index (dev_tty_t * tty) {
	return (tty - tty_buffer);
}

/**
 * 初始化tty列表
 */
void tty_manager_init (void) {
	list_init(&tty_list);
	for (int i = 0;i < sizeof(tty_buffer) / sizeof(dev_tty_t); i++) {
		list_insert_first(&tty_list, (list_node_t *)(tty_buffer + i));
	}
}


tty_driver_t tty_driver = {
	.write = tty_wdiget_write,
};

/**
 * 打开一个tty设备
 */
int tty_open (void) {
	dev_tty_t * tty = (dev_tty_t *)list_remove_first(&tty_list);
	if (tty == (dev_tty_t *)0) {
		return -1;
	}

	tty->driver = &tty_driver;
	tty->device_data = (void *)0;

	// 初始化输入输出
	bfifo_init(&tty->in_fifo, tty->in_msgs, TTY_IN_MSG_MAX_COUNT);
	bfifo_init(&tty->out_fifo, tty->out_msgs, TTY_OUT_MSG_MAX_COUNT);

	return to_index(tty);
}

/**
 * 从tty中读取数据
 */
int tty_read (int tty, char * buffer, int size) {
	dev_tty_t * p_tty = to_tty(tty);
	int total_size = size;
	char * curr = buffer;

	// size可能比fifo的大，所以要分批读
	while (size > 0) {
		int read_size = bfifo_get(&p_tty->in_fifo, curr, size);
		if (read_size <= 0) {
			read_size = bfifo_read(&p_tty->in_fifo, curr, 1);
		}

		size -= read_size;
		curr += read_size;
	}
	return total_size;
}

/**
 * 向tty中写入数据
 */
int tty_write (int tty, char * buffer, int size) {
	dev_tty_t * p_tty = to_tty(tty);
	int total_size = size;
	char * curr = buffer;

	// size可能比out_fifo的容量大，所以直接用size去写
	// 所以下面每次尽可能多写，能写多少是多少。写不了说明缓存满，等一下
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

		p_tty->driver->write(p_tty);
	}

	return total_size;
}

/**
 * 关闭tty设备
 */
void tty_close (int tty) {
	dev_tty_t * p_tty = to_tty(tty);

	// todo:注意释放挂在上面的进程？
	list_insert_first(&tty_list, (list_node_t *)p_tty);
}

/**
 * 设置tty参数
 */
int tty_ioctl(int tty, int cmd, ...) {
	dev_tty_t * p_tty = to_tty(tty);
	va_list args;

	va_start(args, cmd);
	switch (cmd) {
	case TTY_SET_DRIVER_DATA: // 设置驱动参数
		p_tty->device_data = va_arg(args, void *);
		break;
	default:
		break;
	}

	va_end(args);
	return -1;
}



