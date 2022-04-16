/**
 * 系统调用实现
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "core/syscall.h"
#include "tools/klib.h"
#include "applib/lib_syscall.h"
#include "core/task.h"
#include "tools/log.h"
#include "fs/fs.h"
#include "core/memory.h"

// 系统调用处理函数类型
typedef int (*syscall_handler_t)(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);

// 系统调用表
static syscall_handler_t sys_table[] = {
	[SYS_msleep] = (syscall_handler_t)sys_msleep,
    [SYS_getpid] =(syscall_handler_t)sys_getpid,
    [SYS_sched_yield] = (syscall_handler_t)sys_sched_yield,
	[SYS_fork] = (syscall_handler_t)sys_fork,
	[SYS_execve] = (syscall_handler_t)sys_execve,

	[SYS_open] = (syscall_handler_t)sys_open,
	[SYS_read] = (syscall_handler_t)sys_read,
	[SYS_write] = (syscall_handler_t)sys_write,
	[SYS_close] = (syscall_handler_t)sys_close,
	[SYS_lseek] = (syscall_handler_t)sys_lseek,
	[SYS_isatty] = (syscall_handler_t)sys_isatty,
	[SYS_sbrk] = (syscall_handler_t)sys_sbrk,
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
			frame->eax = ret;		// 设置系统调用的返回值，由eax传递
			return;
		}
	}

	// 不支持的系统调用，打印出错信息
	task_t * task = task_current();
	log_printf("pid: %d, %s, Unknown syscall: %d", task->pid, task->name,  frame->func_id);
}

/**
 * 初始化系统调用
 */
void init_syscall (void) {

}
