/**
 * 字符流FIFO
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <ipc/bfifo.h>
#include <core/klib.h>

/**
 * fifo初始化
 */
void bfifo_init (bfifo_t * bfifo, char * buf, int size) {
	bfifo->buf_start = buf;
	bfifo->max_size = size;
	bfifo->read = bfifo->write = 0;

	// 初始可写，可写量为size。但不能读。可访问
	sem_init(&bfifo->acc_sem, 1);
	sem_init(&bfifo->write_sem, size);
	sem_init(&bfifo->read_sem, 0);
}

/**
 * 向bfifo中写入数据
 */
int bfifo_write (bfifo_t * bfifo, char * buf, int size) {
	// 申请访问
	for (int i = 0; i < size; i++) {
		// 等待可写的空间
		sem_wait(&bfifo->write_sem);

		// 先锁定访问
		sem_wait(&bfifo->acc_sem);
		bfifo->buf_start[bfifo->write++] = *buf++;
		if (bfifo->write >= bfifo->max_size) {
			bfifo->write = 0;
		}
		sem_notify(&bfifo->acc_sem);

		// 通知有新数据
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

		// 先锁定访问
		sem_wait(&bfifo->acc_sem);
		*buf++ = bfifo->buf_start[bfifo->read++];
		if (bfifo->read >= bfifo->max_size) {
			bfifo->read = 0;
		}
		sem_notify(&bfifo->acc_sem);

		// 通知有新的空闲空间
		sem_notify(&bfifo->write_sem);
	}

	return size;
}

/**
 * 尝试向fifo写入数据
 */
int bfifo_put (bfifo_t * bfifo, char * buf, int size) {
	int write_size;

	// 先锁定访问
	sem_wait(&bfifo->acc_sem);

	int free_count = sem_count(&bfifo->write_sem);
	int total_size = k_min(free_count, size);
	for (write_size = 0; write_size < total_size; write_size++) {
		bfifo->buf_start[bfifo->write++] = *buf++;
		if (bfifo->write >= bfifo->max_size) {
			bfifo->write = 0;
		}

		sem_notify(&bfifo->read_sem);
		sem_wait(&bfifo->write_sem);
	}

	sem_notify(&bfifo->acc_sem);
	return write_size;
}

/**
 * 从fifo中读数据
 */
int bfifo_get (bfifo_t * bfifo, char * buf, int size) {
	int read_size;

	// 先锁定访问
	sem_wait(&bfifo->acc_sem);

	int data_count = sem_count(&bfifo->read_sem);
	for (read_size = 0; read_size < data_count; read_size++) {
		*buf++ = bfifo->buf_start[bfifo->read++];
		if (bfifo->read >= bfifo->max_size) {
			bfifo->read = 0;
		}

		sem_notify(&bfifo->write_sem);
		sem_wait(&bfifo->read_sem);
	}

	sem_notify(&bfifo->acc_sem);
	return read_size;
}

/**
 * 获取fifo中已有的数据量
 */
int bfifo_count (bfifo_t * bfifo) {
	return sem_count(&bfifo->read_sem);
}
