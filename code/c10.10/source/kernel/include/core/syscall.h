/**
 * 系统调用实现
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef OS_SYSCALL_H
#define OS_SYSCALL_H

#include "comm/types.h"


#define SYSCALL_PARAM_COUNT     5       	// 系统调用最大支持的参数
#define SYSCALL_GET_VERSION   	0				// 获取OS版本

/**
 * 系统调用的栈信息
 */
typedef struct _syscall_frame_t {
	int eflags;
	int gs, fs, es, ds;
	int edi, esi, ebp, dummy, ebx, edx, ecx, eax;
	int eip, cs;
	int func_id, arg0, arg1, arg2, arg3;
	int esp, ss;
}syscall_frame_t;

void excetpion_handler_syscall (void);		// syscall异常处理
void init_syscall (void);

#endif //OS_SYSCALL_H
