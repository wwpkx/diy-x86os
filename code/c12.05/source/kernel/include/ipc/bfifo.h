/**
 * 字符流FIFO
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef BFIFO_H
#define BFIFO_H

#include "ipc/mutex.h"
#include "ipc/sem.h"

/**
 * 字节流fifo
 */
typedef struct _bfifo_t {
	char * start;			// 整体缓冲区起始地址
	int size;				// 整体缓存最大字节量
	int read, write;		// 当前读写位置

	sem_t read_sem;			// 读信号量
	sem_t write_sem;		// 写信号量
	mutex_t mutex;			// 锁
}bfifo_t;

void bfifo_init (bfifo_t * bfifo, void * buf, int size);
int bfifo_write (bfifo_t * bfifo, void * buf, int size);
int bfifo_put (bfifo_t * bfifo, void * buf, int size);
int bfifo_read (bfifo_t * bfifo, void * buf, int size);
int bfifo_get (bfifo_t * bfifo, void * buf, int size);
int bfifo_data_count (bfifo_t * bfifo);
int bfifo_free_count (bfifo_t * bfifo);

#endif /* BFIFO_H */
