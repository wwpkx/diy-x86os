/**
 * 系统调用接口
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "core/syscall.h"
#include "os_cfg.h"
#include "lib_syscall.h"

/**
 * 执行系统调用
 */
static inline int sys_call (syscall_args_t * args) {
    const unsigned long sys_gate_addr[] = {0, SELECTOR_SYSCALL | 0};  // 使用特权级0
    int ret;

    // 采用调用门, 这里只支持5个参数
    // 用调用门的好处是会自动将参数复制到内核栈中，这样内核代码很好取参数
    // 而如果采用寄存器传递，取参比较困难，需要先压栈再取
    __asm__ __volatile__(
            "push %[arg3]\n\t"
            "push %[arg2]\n\t"
            "push %[arg1]\n\t"
            "push %[arg0]\n\t"
            "push %[id]\n\t"
            "lcalll *(%[gate])\n\n"
            :"=a"(ret)
            :[arg3]"r"(args->arg3), [arg2]"r"(args->arg2), [arg1]"r"(args->arg1),
    [arg0]"r"(args->arg0), [id]"r"(args->id),
    [gate]"r"(sys_gate_addr));
    return ret;
}

int msleep (int ms) {
    if (ms <= 0) {
        return 0;
    }

    syscall_args_t args;
    args.id = SYS_msleep;
    args.arg0 = ms;
    return sys_call(&args);
}

int getpid() {
	return sys_call(SYS_getpid, 0, 0, 0, 0);
}

int sched_yield (void) {
    return sys_call(SYS_sched_yield, 0, 0, 0, 0);
}

char **environ;     // 当前的环境变量
int execve(const char *name, char * const *argv, char * const *env) {
    return sys_call(SYS_execve, (int)name, (int)argv, (int)env, 0);
}

int wait(int* status) {
    return sys_call(SYS_wait, (int)status, 0, 0, 0);
}

void _exit(int status) {
    return sys_call(SYS_exit, (int)status, 0, 0, 0);
}

int open(const char *name, int flags, ...) {
    // 不考虑支持太多参数
    syscall_args_t args;
    args.id = SYS_open;
    args.arg0 = (int)name;
    args.arg1 = (int)flags;
    return sys_call(&args);
}

int read(int file, char *ptr, int len) {
    syscall_args_t args;
    args.id = SYS_read;
    args.arg0 = (int)file;
    args.arg1 = (int)ptr;
    args.arg2 = len;
    return sys_call(&args);
}

int write(int file, char *ptr, int len) {
    syscall_args_t args;
    args.id = SYS_write;
    args.arg0 = (int)file;
    args.arg1 = (int)ptr;
    args.arg2 = len;
    return sys_call(&args);
}

int close(int file) {
    return sys_call(SYS_close, file, 0, 0, 0);
}

off_t lseek(int file, off_t ptr, int dir) {
    return sys_call(SYS_lseek, (int)file, (int)ptr, (int)dir, 0);
}

/**
 * 删除文件
 */
int unlink(const char *name) {
    return sys_call(SYS_unlink, (int)name, 0, 0, 0);
}

/**
 * 建立硬连接
 */
int link(const char *old, const char *new) {
    return sys_call(SYS_link, (int)old, (int)new, 0, 0);
}

/**
 * 获取文件的状态
 */
int fstat(int file, struct stat *st) {
    return sys_call(SYS_fstat, (int)file, (int)st, 0, 0);
}

/**
 * 获取文件的状态
 */
int stat(const char *file, struct stat *st) {
    return sys_call(SYS_stat, (int)file, (int)st, 0, 0);
}

/**
 * 判断文件描述符与tty关联
 */
int isatty(int file) {
    syscall_args_t args;
    args.id = SYS_isatty;
    args.arg0 = (int)file;
    return sys_call(&args);
}

void * sbrk(ptrdiff_t incr) {
    syscall_args_t args;
    args.id = SYS_sbrk;
    args.arg0 = (int)incr;
    return (void *)sys_call(&args);
}

int dup (int file) {
    return sys_call(SYS_dup, file, 0, 0, 0);
}


