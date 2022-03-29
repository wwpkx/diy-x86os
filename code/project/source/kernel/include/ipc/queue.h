/**
 * 消息队列
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef OS_QUEUE_H
#define OS_QUEUE_H

#include <core/list.h>
#include <ipc/sem.h>

/**
 * 基础消息类型
 */
typedef struct _queue_msg_t {
    list_node_t node;			// 消息的链表结点
}queue_msg_t;

/**
 * 进程间通信的消息队列
 */
typedef struct _queue_t {
    list_t msg_list;            // 已经写入的消息列表
    list_t msg_wait_list;       // 等待读取消息的进程队列

    list_t free_list;            // 空闲消息列表
    list_t free_wait_list;       // 等待空闲消息的进程队列
}queue_t;

static inline int queue_msg_count (queue_t * queue) {
	return list_count(&queue->msg_list);
}

void queue_init (queue_t * queue, void * msg_buf, int msg_size, int count);
queue_msg_t * queue_alloc_free(queue_t * queue);
queue_msg_t * queue_get_free(queue_t * queue);
void queue_free(queue_t * queue, queue_msg_t * msg);
queue_msg_t * queue_get_msg(queue_t * queue);
queue_msg_t * queue_check_msg(queue_t * queue);
queue_msg_t * queue_wait_msg(queue_t * queue);
void queue_send_msg(queue_t * queue, queue_msg_t * msg);

#endif //OS_QUEUE_H
