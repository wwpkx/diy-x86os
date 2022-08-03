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
#define SYS_fork				2
#define SYS_execve				3
#define SYS_yield               4
#define SYS_exit                5
#define SYS_wait                6

#define SYS_open                50
#define SYS_read                51
#define SYS_write               52
#define SYS_close               53
#define SYS_lseek				54
#define SYS_isatty              55
#define SYS_sbrk                56
#define SYS_fstat               57
#define SYS_dup              	58
#define SYS_ioctl				59

#define SYS_opendir				60
#define SYS_readdir				61
#define SYS_closedir			62
#define SYS_unlink				63


#define SYS_printmsg            100

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

void exception_handler_syscall (void);		// syscall处理

#endif //OS_SYSCALL_H
