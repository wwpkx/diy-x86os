/**
 * 终端tty
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef TTY_H
#define TTY_H

#include "ipc/bfifo.h"

#define TTY_MAX_COUNT				10		// 最大支持的tty设备数量
#define TTY_IN_SIZE					512		// tty输入缓存大小
#define TTY_OUT_SIZE				512		// tty输出缓存大小

struct _tty_t;
typedef struct _tty_dev_t {
	int (*init) (struct _tty_t * tty);
	int (*write) (struct _tty_t * tty);	// 写函数
	void (*close) (struct _tty_t * tty);	// 关闭函数
}tty_dev_t;

/**
 * tty设备
 */
typedef struct _tty_t {
	int device_num;					// 所在的设备号
	tty_dev_t * dev;				// 实际设备
	void * dev_data;				// 设备参数

	mutex_t mutex;					// 进程访问的互斥锁
	char in_buf[TTY_IN_SIZE];
	bfifo_t in_fifo;				// 输入队列
	char out_buf[TTY_OUT_SIZE];
	bfifo_t out_fifo;				// 输出队列

	struct {
		int echo : 1;				// 是否回显
	};
}tty_t;

void tty_init (void);
int tty_open (int device);
int tty_read (int tty, char * buffer, int size);
int tty_write (int tty, char * buffer, int size);
void tty_close (int tty);
int tty_current(void);
void tty_in_data(int tty, char * data, int size);

#endif /* TTY_H */
