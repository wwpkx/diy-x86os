/**
 * 字符流FIFO
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "ipc/bfifo.h"

/**
 * fifo初始化
 */
void bfifo_init (bfifo_t * bfifo, void * buf, int size) {
	uint8_t * p_buf = (uint8_t *)buf;

	bfifo->start = p_buf;
	bfifo->size = size;
	bfifo->read = bfifo->write = 0;

	sem_init(&bfifo->write_sem, size);
	sem_init(&bfifo->read_sem, 0);
	mutex_init(&bfifo->mutex);
}

/**
 * 获取fifo中已有的数据量
 */
int bfifo_count (bfifo_t * bfifo) {
	return sem_count(&bfifo->read_sem);
}
