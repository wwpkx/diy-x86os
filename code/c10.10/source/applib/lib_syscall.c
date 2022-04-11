/**
 * 系统调用接口
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "lib_syscall.h"

/**
 * 打开文件
 */
int open(const char *name, int flags, ...) {
    return -1;
}

/**
 * 读取文件api
 */
int read(int file, char *ptr, int len) {
    return -1;
}

/**
 * 写文件
 */
int write(int file, char *ptr, int len) {
    return -1;
}

/**
 * 文件访问位置定位
 */
int lseek(int file, int ptr, int dir) {
    return -1;
}

/**
 * 关闭文件
 */
int close(int file) {
    return -1;
}

/**
 * 删除文件
 */
int unlink(char *name) {
    return -1;
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

 __attribute__((noreturn)) void _exit(int status) {
     for (;;) {}
}

char **environ; /* pointer to array of char * strings that define the current environment variables */
int execve(char *name, char **argv, char **env) {
	return -1;
}

int getpid() {
	return sys_call0(SYS_getpid);
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

/**
 * @brief 切换至下一优先级相同或更高的进程
 */
int sched_yield (void) {
    return sys_call0(SYS_sched_yield);
}

clock_t times(struct tms *buf) {
	return -1;
}

int gettimeofday(struct timeval *p, void *z) {
	return -1;
}


