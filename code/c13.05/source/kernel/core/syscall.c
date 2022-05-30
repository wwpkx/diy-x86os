/**
 * 系统调用实现
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "core/syscall.h"
#include "tools/klib.h"
#include "core/task.h"
#include "tools/log.h"
#include "core/memory.h"

// 系统调用处理函数类型
typedef int (*syscall_handler_t)(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);

int sys_print_msg (char * fmt, int arg) {
	log_printf(fmt, arg);
}

// 系统调用表
static const syscall_handler_t sys_table[] = {
	[SYS_msleep] = (syscall_handler_t)sys_msleep,
    [SYS_getpid] =(syscall_handler_t)sys_getpid,

	[SYS_printmsg] = (syscall_handler_t)sys_print_msg,
};

/**
 * 处理系统调用。该函数由系统调用函数调用
 */
void do_handler_syscall (syscall_frame_t * frame) {
	// 超出边界，返回错误
    if (frame->func_id < sizeof(sys_table) / sizeof(sys_table[0])) {
		// 查表取得处理函数，然后调用处理
		syscall_handler_t handler = sys_table[frame->func_id];
		if (handler) {
			int ret = handler(frame->arg0, frame->arg1, frame->arg2, frame->arg3);
			frame->eax = ret;  // 设置系统调用的返回值，由eax传递
            return;
		}
	}

	// 不支持的系统调用，打印出错信息
	task_t * task = task_current();
	log_printf("task: %s, Unknown syscall: %d", task->name,  frame->func_id);
    frame->eax = -1;  // 设置系统调用的返回值，由eax传递
}
