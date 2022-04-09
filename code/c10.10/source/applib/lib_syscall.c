/**
 * 系统调用接口
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "lib_syscall.h"

/**
 * 执行系统调用
 */
static int sys_call (int syscall_num, int arg0, int arg1, int arg2, int arg3) {
	static const uint32_t sys_gate_addr[] = {0, SELECTOR_SYSCALL | 0};  // 使用特权级0
    uint32_t ret;

    // 采用调用门, 最多支持5个参数
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

// 系统调用的简化编写
#define sys_call0(syscall_num) sys_call(syscall_num, 0, 0, 0, 0)
#define sys_call1(syscall_num, arg0) sys_call(syscall_num, arg0, 0, 0, 0)
#define sys_call2(syscall_num, arg0, arg1) sys_call(syscall_num, arg0, arg1, 0, 0)
#define sys_call3(syscall_num, arg0, arg1, arg2) sys_call(syscall_num, arg0, arg1, arg2, 0)
#define sys_call4(syscall_num, arg0, arg1, arg2, arg3) sys_call(syscall_num, arg0, arg1, arg2, arg3)

// newlib需要的系统调用
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

int fork() {
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

/**
 * @brief 毫秒延时
 */
void msleep (unsigned int seconds) {
    sys_call1(SYS_msleep, seconds);
}

