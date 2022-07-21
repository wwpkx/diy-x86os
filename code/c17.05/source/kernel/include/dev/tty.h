/**
 * 终端tty
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef TTY_H
#define TTY_H

#include "ipc/sem.h"

#define TTY_NR						8		// 最大支持的tty设备数量
#define TTY_IBUF_SIZE				512		// tty输入缓存大小
#define TTY_OBUF_SIZE				512		// tty输出缓存大小

typedef struct _tty_fifo_t {
	char * buf;
	int size;				// 最大字节数
	int read, write;		// 当前读写位置
	int count;				// 当前已有的数据量
}tty_fifo_t;

/**
 * tty设备
 */
typedef struct _tty_t {
	char obuf[TTY_OBUF_SIZE];
	tty_fifo_t ofifo;				// 输出队列
	char ibuf[TTY_IBUF_SIZE];
	tty_fifo_t ififo;				// 输入处理后的队列
	int console_idx;				// 控制台索引号
}tty_t;

#endif /* TTY_H */
