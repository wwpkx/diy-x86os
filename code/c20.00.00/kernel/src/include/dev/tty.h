/**
 * 终端tty
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef TTY_H
#define TTY_H

#include <ipc/bfifo.h>
#include <core/types.h>
#include <core/os_cfg.h>

#define	TTY_SET_DRIVER_DATA			1		// 设置驱动参数


/**
 * tty输出接口
 */
struct _dev_tty_t ;
typedef struct _tty_driver_t {
	int (*write)(struct _dev_tty_t * tty);
}tty_driver_t;

/**
 * tty设备
 */
typedef struct _dev_tty_t {
	void * device_data;				// 设备参数
	tty_driver_t * driver;			// 设备驱动

	char in_msgs[TTY_IN_MSG_MAX_COUNT];
	bfifo_t in_fifo;				// 输入队列

	char out_msgs[TTY_OUT_MSG_MAX_COUNT];
	bfifo_t out_fifo;				// 输出队列
}dev_tty_t;

void tty_manager_init (void);
int tty_open (tty_driver_t * driver, void * device_data);
int tty_read_add (int tty, char * buffer, int size);
int tty_read (int tty, char * buffer, int size);
int tty_write (int tty, char * buffer, int size);
void tty_close (int tty);
int tty_ioctl(int tty, int cmd, ...);
int tty_current(void);

#endif /* TTY_H */
