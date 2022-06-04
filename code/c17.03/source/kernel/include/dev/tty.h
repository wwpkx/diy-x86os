/**
 * 终端tty
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef TTY_H
#define TTY_H

#include "fs/file.h"

#define TTY_MAX_COUNT				10		// 最大支持的tty设备数量
#define TTY_BUF_SIZE				512		// tty输入缓存大小

typedef struct _tty_queue_t {
	int write;			// 写入索引
	int read;			// 读取索引
	int count;			// 有效数据量
	char buf[TTY_BUF_SIZE];	// 数据缓存
}tty_queue_t;

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

	tty_queue_t in_queue;			// 输入队列
	tty_queue_t out_queue;			// 输出队列
}tty_t;

void tty_init (void);
int tty_open (const char * name, file_t * file);

#endif /* TTY_H */
