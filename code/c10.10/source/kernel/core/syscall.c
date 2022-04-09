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

// 系统调用处理函数类型
typedef int (*syscall_handler_t)(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);


// 系统调用表
static syscall_handler_t sys_call_table[] = {
    [SYS_getpid] =(syscall_handler_t)sys_getpid,
};

/**
 * 处理系统调用。该函数由系统调用函数调用
 */
int do_handler_syscall (syscall_frame_t * frame) {
	// 超出边界，返回错误
    if (frame->func_id < ARRAY_COUNT(sys_call_table)) {
		// 查表取得处理函数，然后调用处理
		syscall_handler_t handler = sys_call_table[frame->func_id];
		if (handler) {
			int ret = handler(frame->arg0, frame->arg1, frame->arg2, frame->arg3);
			return ret;
		}
	}

	// 不支持的系统调用，打印出错信息
	task_t * task = task_current();
	log_printf("pid: %d, %s, Unknown syscall: %d", task->pid, task->name,  frame->func_id);
    return -1;
}

/**
 * 初始化系统调用
 */
void init_syscall (void) {

}
