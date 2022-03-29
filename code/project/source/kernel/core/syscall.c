//
// Created by lishutong on 2021-07-28.
//
#include <core/os_cfg.h>
#include <core/syscall.h>
#include <core/cpu.h>
#include <core/task.h>
#include <ui/ui_event.h>
#include <core/klib.h>
#include <core/time.h>
#include <fs/fs.h>

/**
 * 系统调用处理函数类型
 */
typedef int (*syscall_handler_t)(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);

/**
 * 获取版本号
 * @return
 */
static int do_sys_get_version (void) {
    return OS_VERSION;
}

/**
 * 获取时间计数
 * @return
 */
static int do_sys_get_ticks (void) {
    return time_get_tick();
}

/**
 * 进程睡眠
 * @param ms
 */
static void do_sys_sleep (uint32_t ms) {
    task_sleep(ms);
}

/**
 * 应用进程选取一个事件
 * @param msg
 * @return
 */
static int do_sys_get_event(app_msg_t * msg) {
	task_t * curr = task_current();

	// 等待消息，然后复制至应用中
	task_msg_t * task_msg;
	do {
		task_msg = (task_msg_t *)queue_wait_msg(&curr->queue);

		// 让进程处理一些内部定时事件
		if (task_msg->msg.type == APP_MSG_TYPE_TIMER) {
			 rtimer_t * timer = (rtimer_t *)task_msg->msg.timer;
			 if (timer->system && timer->time_out) {
				 timer->time_out(timer);
				 queue_free(&curr->queue, (queue_msg_t *)task_msg);
				 task_msg = (task_msg_t *)0;
				 continue;
			 }
		}
	}while (!task_msg);

	// 将数据拷贝到用户空间中
	k_memcpy(msg, &task_msg->msg, sizeof(app_msg_t));
	queue_free(&curr->queue, (queue_msg_t *)task_msg);
	return 0;
}

/**
 * 创建定时器
 */
static int do_sys_create_timer(int period, void * data) {
	rtimer_t * timer = rtimer_alloc(period, 0, data, 0, task_current());
	return (int)timer;
}

/**
 * 释放定时器
 */
static int do_sys_free_timer(int timer) {
	rtimer_free((rtimer_t *)timer);
	return 0;
}


/**
 * 设置定时器
 */
static int do_sys_set_timer(int timer, int ms) {
	rtimer_set((rtimer_t *)timer, (uint32_t)ms);
	return 0;
}

/**
 * 处理发送消息的系统调用, 可用于消息发给指定的进程
 * 返回值是消息发送的结果，而不是消息处理的结果
 */
static int do_sys_send_msg(int dest, void *msg, int wait_ret) {
    if (dest == SYSCALL_MSG_DEST_UI) {
		ui_msg_t * ui_msg = (ui_msg_t *) ui_alloc_msg(UI_MSG_API, 1);
		if (ui_msg) {
			ui_api_msg_t * msg_from_app = (ui_api_msg_t *)msg;

			// 从用户空间复制所有信息
			// todo: 进一步复制
			k_memcpy(&ui_msg->api_msg, msg_from_app, sizeof(ui_api_msg_t));

			// 如果需要等等调用结果，则创建信号量用于等等
			if (wait_ret) {
				ui_send_msg(ui_msg, task_current(), 1);

				// 因为要等待调用结果，所以UI进程不释放msg，需要自己释放
				msg_from_app->ret = ui_msg->api_msg.ret;
				ui_free_msg(ui_msg);
			} else {
				// 不需要等待结果，直接发过去，由UI进程释放
				msg_from_app->ret = 0;
				ui_send_msg(ui_msg, task_current(), 0);
			}
			return 0;
		}
    }
    return -1;
}

// 系统调用表
static syscall_handler_t sys_call_table[] = {
        [SYSCALL_GET_OS_VERSION_NUM] =(syscall_handler_t)do_sys_get_version,
        [SYSCALL_GET_TICKS] = (syscall_handler_t)do_sys_get_ticks,
        [SYSCALL_SLEEP] = (syscall_handler_t)do_sys_sleep,
        [SYSCALL_SEND_MSG] = (syscall_handler_t)do_sys_send_msg,
        [SYSCALL_GET_EVENT] = (syscall_handler_t)do_sys_get_event,

		[SYSCALL_CREATE_TIMER] = (syscall_handler_t)do_sys_create_timer,
		[SYSCALL_FREE_TIMER] = (syscall_handler_t)do_sys_free_timer,
		[SYSCALL_SET_TIMER] = (syscall_handler_t)do_sys_set_timer,

		[SYSCALL_FILE_OPEN] = (syscall_handler_t)xfile_open,
		[SYSCALL_FILE_READ] = (syscall_handler_t)xfile_read,
		[SYSCALL_FILE_WRITE] = (syscall_handler_t)xfile_write,
		[SYSCALL_FILE_CLOSE] = (syscall_handler_t)xfile_close,
		[SYSCALL_FILE_IOCTL] = (syscall_handler_t)xfile_ioctl,
};

/**
 * 处理系统调用
 */
int do_handler_syscall (syscall_frame_t * frame) {
    if (frame->syscall_num >= sizeof(sys_call_table) / sizeof(syscall_handler_t)) {
    	return -1;
    }

	syscall_handler_t handler = sys_call_table[frame->syscall_num];
	if (handler) {
		return handler(frame->arg0, frame->arg1, frame->arg2, frame->arg3);
	}

    return -1;
}

/**
 * 初始化系统调用
 */
void init_syscall (void) {

}
