/**
 * 消息队列
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <ipc/queue.h>
#include <core/irq.h>
#include <core/task.h>

/**
 * 初始化消息队列
 */
void queue_init (queue_t * queue, void * msg_buf, int msg_size, int count) {
	// 依次将msg_buf中的消息数组，作为结点插入到空闲队列
    list_init(&queue->free_list);
	if (msg_buf && msg_size && count) {
	    for (int i = 0; i < count; i++) {
	        queue_msg_t * msg = (queue_msg_t *)((uint8_t *)msg_buf + msg_size * i);
	        list_insert_first(&queue->free_list, &msg->node);
	    }
	}

    list_init(&queue->free_wait_list);

    list_init(&queue->msg_list);
    list_init(&queue->msg_wait_list);
}

/**
 * 获取空闲的消息，如果没有，就等待
 */
queue_msg_t * queue_alloc_free(queue_t * queue) {
    queue_msg_t * msg;

    irq_state_t state = irq_enter_protection();
    if ((list_count(&queue->free_list) <= 0)) {
    	// 如果在中断中，没有就直接放弃，不用等待
    	if (irq_is_in()) {
            irq_leave_protection(state);
            return (queue_msg_t *)0;
    	}

    	// 没有空闲的消息，则在该队列中等等
        task_t * curr_task = task_current();

        // 从就绪队列中移除，插入等待队列，并且释放cpu
        task_remove_ready(curr_task);
        list_insert_last(&queue->free_wait_list, &task_current()->node);
        task_dispatch();

        // 再次回来时，应当已经获取了消息，从中取一个
        msg = (queue_msg_t *)list_remove_first(&queue->free_list);
    } else {
    	// 如果有消息，则取一个
        msg = (queue_msg_t *)list_remove_first(&queue->free_list);
    }

    // 初始设置，标记为从QUEUE中分配得到的消息
    irq_leave_protection(state);
    return msg;
}

/**
 * 获取空闲消息，如果没有不等，直接退出
 */
queue_msg_t * queue_get_free(queue_t * queue) {
    queue_msg_t * msg = (queue_msg_t *)0;

    irq_state_t state = irq_enter_protection();

    if (list_count(&queue->free_list) > 0) {
        msg = (queue_msg_t *)list_remove_first(&queue->free_list);
    }

    irq_leave_protection(state);
    return msg;
}

/**
 * 释放消息
 */
void queue_free(queue_t * queue, queue_msg_t * msg) {
    irq_state_t state = irq_enter_protection();

    list_insert_last(&queue->free_list, &msg->node);
    if (list_count(&queue->free_wait_list) <= 0) {
        irq_leave_protection(state);
    } else {
    	// 如果有任务等待，则应当尝试唤醒任务并调度
        list_node_t * node = list_remove_first(&queue->free_wait_list);
        task_t * task = node_to_parent(node, task_t, node);
        task_add_ready(task);
        task_dispatch();
        irq_leave_protection(state);
    }
}

/**
 * 等待消息，如果没有则直接退出
 */
queue_msg_t * queue_get_msg(queue_t * queue) {
    queue_msg_t * msg = (queue_msg_t *)0;

    if (list_count(&queue->msg_list) > 0) {
        irq_state_t state = irq_enter_protection();
        msg = (queue_msg_t *)list_remove_first(&queue->msg_list);
        irq_leave_protection(state);
    }

    return msg;
}

/**
 * 检查是否有消息，如果有，则返回但不取出
 */
queue_msg_t * queue_check_msg(queue_t * queue) {
    queue_msg_t * msg = (queue_msg_t *)0;

    if (list_count(&queue->msg_list) > 0) {
        irq_state_t state = irq_enter_protection();
        msg = (queue_msg_t *)list_first(&queue->msg_list);
        irq_leave_protection(state);
    }

    return msg;
}

/**
 * 等待可用的消息，如果没有，就等待
 */
queue_msg_t * queue_wait_msg(queue_t * queue) {
    queue_msg_t * msg;

    if (list_count(&queue->msg_list) <= 0) {
    	// 中断中，没有就直接退出
    	if (irq_is_in()) {
    		return (queue_msg_t *)0;
    	}

    	// 否则，就是任务在等，将任务置为阻塞态，然后调试
        task_t * curr_task = task_current();
        irq_state_t state = irq_enter_protection();
        task_remove_ready(curr_task);
        list_insert_last(&queue->msg_wait_list, &task_current()->node);

        task_dispatch();

        // 取获得的消息
        msg = (queue_msg_t *)list_remove_first(&queue->msg_list);
        irq_leave_protection(state);
    } else {
        irq_state_t state = irq_enter_protection();
        msg = (queue_msg_t *)list_remove_first(&queue->msg_list);
        irq_leave_protection(state);
    }

    return msg;
}

/**
 * 发送消息到队列中
 */
void queue_send_msg(queue_t * queue, queue_msg_t * msg) {
    irq_state_t state = irq_enter_protection();

    list_insert_last(&queue->msg_list, &msg->node);
    if (list_count(&queue->msg_wait_list) <= 0) {
        irq_leave_protection(state);
    } else {
    	// 如果有任务在等，唤醒它
        list_node_t * node = list_remove_first(&queue->msg_wait_list);
        task_t * task = node_to_parent(node, task_t, node);

        task_add_ready(task);
        task_dispatch();
        irq_leave_protection(state);
    }
}


