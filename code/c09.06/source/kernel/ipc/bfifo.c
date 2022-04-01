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
 * 向bfifo中写入数据
 */
int bfifo_write (bfifo_t * bfifo, void * buf, int size) {
	uint8_t * p_buf = (uint8_t *)buf;

	// 逐个字节写入
	for (int i = 0; i < size; i++) {
		// 等待可写的空间
		sem_wait(&bfifo->write_sem);

		mutex_lock(&bfifo->mutex);
		write_byte(bfifo, *p_buf++);
		mutex_unlock(&bfifo->mutex);

		// 通知有新数据可用
		sem_notify(&bfifo->read_sem);
	}

	return size;
}

/**
 * 尝试向fifo写入数据,只写一次，能写多少就写多少
 */
int bfifo_put (bfifo_t * bfifo, void * buf, int size) {
	uint8_t * p_buf = (uint8_t *)buf;

	mutex_lock(&bfifo->mutex);

	// 计算最大能写入的量
	int free_count = sem_count(&bfifo->write_sem);
	if (size > free_count) {
		size = free_count;
	}

	for (int i = 0; i < size; i++) {
		sem_wait(&bfifo->write_sem);
		write_byte(bfifo, *p_buf++);
		sem_notify(&bfifo->read_sem);
	}

	mutex_unlock(&bfifo->mutex);
	return size;
}

/**
 * 获取fifo中已有的数据量
 */
int bfifo_count (bfifo_t * bfifo) {
	return sem_count(&bfifo->read_sem);
}
