/**
 * 系统调用实现
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef OS_SYSCALL_H
#define OS_SYSCALL_H

#define SYSCALL_PARAM_COUNT     5       	// 系统调用最大支持的参数

#define SYS_msleep              0
#define SYS_getpid              1

/**
 * 系统调用的栈信息
 */
typedef struct _syscall_frame_t {
    int eflags;
	int edi, esi, ebx, ebp;
    int gs, fs, es, ds;
	int eip, cs;
	int func_id, arg0, arg1, arg2, arg3;
	int esp3, ss3;
}syscall_frame_t;

void exception_handler_syscall (void);		// syscall处理

#endif //OS_SYSCALL_H
