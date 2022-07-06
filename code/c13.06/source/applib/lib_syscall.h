/**
 * 系统调用接口
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef LIB_SYSCALL_H
#define LIB_SYSCALL_H

#include "core/syscall.h"
#include "os_cfg.h"

typedef struct _syscall_args_t {
    int id;
    int arg0;
    int arg1;
    int arg2;
    int arg3;
}syscall_args_t;

/**
 * 执行系统调用
 */
static inline int sys_call (syscall_args_t * args) {
    int ret;

    // 采用调用门, 这里只支持5个参数
    // 用调用门的好处是会自动将参数复制到内核栈中，这样内核代码很好取参数
    // 而如果采用寄存器传递，取参比较困难，需要先压栈再取
    __asm__ __volatile__(
            "int $0x80\n\n"
    		:"=a"(ret)
            :"S"(args->arg3), "d"(args->arg2), "c"(args->arg1),
            "b"(args->arg0), "a"(args->id));
    return ret;
}

static inline int msleep (int ms) {
    if (ms <= 0) {
        return 0;
    }

    syscall_args_t args;
    args.id = SYS_msleep;
    args.arg0 = ms;
	return sys_call(&args);
}

static inline int getpid() {
    syscall_args_t args;
    args.id = SYS_getpid;
	return sys_call(&args);
}

static inline int print_msg(char * fmt, int arg) {
    syscall_args_t args;
    args.id = SYS_printmsg;
    args.arg0 = (int)fmt;
    args.arg1 = arg;
    return sys_call(&args);
}

#endif //LIB_SYSCALL_H
