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

// 系统调用号
#define SYS_msleep              0
#define SYS_getpid              1
#define SYS_sched_yield         3
#define SYS_fork                4
#define SYS_execve              5
#define SYS_wait                6
#define SYS_exit                7

#define SYS_open                100
#define SYS_read                101
#define SYS_write               102
#define SYS_close               103
#define SYS_lseek               104
#define SYS_unlink              105
#define SYS_link                106
#define SYS_fstat               107
#define SYS_stat                108
#define SYS_isatty              109
#define SYS_sbrk                110
#define SYS_dup                 111

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

#endif //OS_SYSCALL_H
