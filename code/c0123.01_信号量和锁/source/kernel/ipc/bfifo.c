/**
 * 字符流FIFO
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "ipc/bfifo.h"

/**
 * @brief 写入一个字节
 */
static void write_byte (bfifo_t * bfifo, uint8_t data) {
	bfifo->start[bfifo->write++] = data;
	if (bfifo->write >= bfifo->size) {
		bfifo->write = 0;
	}
}

/**
 * @brief 写入一个字节
 */
static uint8_t read_byte (bfifo_t * bfifo) {
	uint8_t data = bfifo->start[bfifo->read++];
	if (bfifo->read >= bfifo->size) {
		bfifo->read = 0;
	}
	return data;
}

/**
 * fifo初始化
 */
void bfifo_init (bfifo_t * bfifo, char * buf, int size) {
	bfifo->start = buf;
	bfifo->size = size;
	bfifo->read = bfifo->write = 0;

	sem_init(&bfifo->write_sem, size);
	sem_init(&bfifo->read_sem, 0);
	mutex_init(&bfifo->mutex);
}

/**
 * 向bfifo中写入数据
 */
int bfifo_write (bfifo_t * bfifo, char * buf, int size) {
	// 逐个字节写入
	for (int i = 0; i < size; i++) {
		// 等待可写的空间
		sem_wait(&bfifo->write_sem);

		mutex_lock(&bfifo->mutex);
		write_byte(bfifo, *buf++);
		mutex_unlock(&bfifo->mutex);

		// 通知有新数据可用
		sem_notify(&bfifo->read_sem);
	}

	return size;
}

/**
 * 从bfifo中读取数据
 */
int bfifo_read (bfifo_t * bfifo, char * buf, int size) {
	// 申请访问
	for (int i = 0; i < size; i++) {
		// 等待可读的数据
		sem_wait(&bfifo->read_sem);

		mutex_lock(&bfifo->mutex);
		*buf++ = read_byte(bfifo);
		mutex_unlock(&bfifo->mutex);

		// 通知有新的空闲空间
		sem_notify(&bfifo->write_sem);
	}

	return size;
}

/**
 * 尝试向fifo写入数据,只写一次，能写多少就写多少
 */
int bfifo_put (bfifo_t * bfifo, char * buf, int size) {
	mutex_lock(&bfifo->mutex);

	// 计算最大能写入的量
	int free_count = sem_count(&bfifo->write_sem);
	if (size > free_count) {
		size = free_count;
	}

	for (int i = 0; i < size; i++) {
		sem_wait(&bfifo->write_sem);
		write_byte(bfifo, *buf++);
		sem_notify(&bfifo->read_sem);
	}

	mutex_unlock(&bfifo->mutex);
	return size;
}

/**
 * 从fifo中读数据，只读一次，有多少读多少
 */
int bfifo_get (bfifo_t * bfifo, char * buf, int size) {
	mutex_lock(&bfifo->mutex);

	// 计算实际能读取的量
	int data_count = sem_count(&bfifo->read_sem);
	if (size > data_count) {
		size = data_count;
	}

	for (int i = 0; i < size; i++) {
		sem_wait(&bfifo->read_sem);
		*buf++ = read_byte(bfifo);
		sem_notify(&bfifo->write_sem);
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
