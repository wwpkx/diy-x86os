/**
 * 系统调用接口
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdarg.h>
#include <core/syscall.h>
#include <core/task.h>
#include "lib_syscall.h"

/**
 * 执行系统调用
 */
static inline int exec_sys_call (int syscall_num, int arg0, int arg1, int arg2, int arg3) {
	static const uint32_t sys_gate_addr[] = {0, SELECTOR_SYSCALL | GDT_RPL0};
    uint32_t ret;

    // 采用调用门
    __asm__ __volatile__(
            "push %1\n\t"
            "push %2\n\t"
            "push %3\n\t"
            "push %4\n\t"
            "push %5\n\t"
            "lcalll *(%6)\n\n"
    		:"=a"(ret)
			 :"r"(arg3), "r"(arg2), "r"(arg1), "r"(arg0), "r"(syscall_num), "r"(sys_gate_addr));
    return ret;
}

/**
 * 获取版本号
 */
int sys_get_version (void) {
    int version = exec_sys_call(SYSCALL_GET_OS_VERSION_NUM, 0, 0, 0, 0);
    return version;
}

/**
 * 获取时钟节拍计数
 */
int sys_get_ticks (void) {
    int ticks = exec_sys_call(SYSCALL_GET_TICKS, 0, 0, 0, 0);
    return ticks;
}

/**
 * 延时指定的毫秒
 */
void sys_sleep (int ms) {
    exec_sys_call(SYSCALL_SLEEP, ms, 0, 0, 0);
}

/**
 * 获取按键
 */
int sys_get_key(key_data_t * key) {
	app_msg_t msg;
	do {
		int err = sys_get_event(&msg);
		if (err < 0) {
			return -1;
		}

		if (msg.type == APP_MSG_TYPE_KEYBOARD) {
			*key = msg.key;
			return 0;
		}
	} while (1);
}

/**
 * 获取事件消息
 */
int sys_get_event(app_msg_t * msg) {
    int err = exec_sys_call(SYSCALL_GET_EVENT, (int)msg, 0, 0, 0);
    return err;
}

/**
 * 创建定时器
 */
int sys_create_timer (int period, int data) {
	int timer = exec_sys_call(SYSCALL_CREATE_TIMER, period, data, 0, 0);
	return timer;
}

/**
 * 删除定时器
 */
int sys_free_timer (int timer) {
	exec_sys_call(SYSCALL_FREE_TIMER, timer, 0, 0, 0);
	return 0;
}

/**
 * 设置定时器的定时秒数
 */
int sys_set_timer (int timer_t, int ms) {
	exec_sys_call(SYSCALL_SET_TIMER, 0, 0, 0, 0);
	return 0;
}


/**
 *设定文件参数，只支持两个
 */
int sys_ioctl(int file, int cmd, ...) {
    va_list args;

    va_start(args, cmd);

    int arg0 = va_arg(args, int);
    int arg1 = va_arg(args, int);

    int ret = exec_sys_call(SYSCALL_FILE_IOCTL, file, cmd, arg0, arg1);
    return ret;
}

/**
 * 给进程发送消息
 * 用于实现向服务器请求执行某些操作
 */
int sys_send_msg(int dest, void *msg, int wait_ret) {
    int ret = exec_sys_call(SYSCALL_SEND_MSG, dest, (int)msg, wait_ret, 0);
    return ret;
}


// newlib需要的系统调用
/**
 * 打开文件
 */
int open(const char *name, int flags, ...) {
    return exec_sys_call(SYSCALL_FILE_OPEN, (int)name, flags, 0, 0);
}

/**
 * 读取文件api
 */
int read(int file, char *ptr, int len) {
    return exec_sys_call(SYSCALL_FILE_READ, file, (int)ptr, len, 0);
}

/**
 * 写文件
 */
int write(int file, char *ptr, int len) {
    return exec_sys_call(SYSCALL_FILE_WRITE, file, (int)ptr, len, 0);
}

/**
 * 文件访问位置定位
 */
int lseek(int file, int ptr, int dir) {
    return exec_sys_call(SYSCALL_FILE_LSEEK, file, ptr, dir, 0);
}

/**
 * 关闭文件
 */
int close(int file) {
    return exec_sys_call(SYSCALL_FILE_CLOSE, file, 0, 0, 0);
}

/**
 * 删除文件
 */
int unlink(char *name) {
    return exec_sys_call(SYSCALL_FILE_UNLINK, (int)name, 0, 0, 0);
}

/**
 * 建立硬连接
 */
int link(char *old, char *new) {
	return -ECANCELED;
}

/**
 * 获取文件的状态
 */
int fstat(int file, struct stat *st) {
    return -1;
}

/**
 * 获取文件的状态
 */
int stat(const char *file, struct stat *st) {
    return -1;
}

/**
 * 判断文件描述符与tty关联
 */
int isatty(int file) {
    return -1;
}

void _exit() {
}

char **environ; /* pointer to array of char * strings that define the current environment variables */
int execve(char *name, char **argv, char **env) {
	return -1;
}

int fork() {
	return -1;
}

int getpid() {
	return -1;
}

int kill(int pid, int sig) {
	return -1;
}

caddr_t sbrk(int incr) {
	return (caddr_t)0;
}

int wait(int *status) {
	return -1;
}

clock_t times(struct tms *buf) {
	return -1;
}

int gettimeofday(struct timeval *p, void *z) {
	return -1;
}

