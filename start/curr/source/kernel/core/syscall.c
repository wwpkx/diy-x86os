#include "core/syscall.h"
#include "core/task.h"
#include "tools/log.h"
#include "fs/fs.h"

typedef int (*syscall_hanler_t)(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);

void sys_print_msg (char * fmt, int arg) {
    log_printf(fmt, arg);
}

static const syscall_hanler_t sys_table[] = {
    [SYS_sleep] = (syscall_hanler_t)sys_sleep,
    [SYS_getpid] = (syscall_hanler_t)sys_getpid,
    [SYS_fork] = (syscall_hanler_t)sys_fork,
    [SYS_printmsg] = (syscall_hanler_t)sys_print_msg,
    [SYS_execve] = (syscall_hanler_t)sys_execve,
    [SYS_yield] = (syscall_hanler_t)sys_yield,
    [SYS_exit] = (syscall_hanler_t)sys_exit,
    [SYS_wait] = (syscall_hanler_t)sys_wait,

    [SYS_open] = (syscall_hanler_t)sys_open,
    [SYS_read] = (syscall_hanler_t)sys_read,
    [SYS_write] = (syscall_hanler_t)sys_write,
    [SYS_close] = (syscall_hanler_t)sys_close,
    [SYS_lseek] = (syscall_hanler_t)sys_lseek,

    [SYS_isatty] = (syscall_hanler_t)sys_isatty,
    [SYS_sbrk] = (syscall_hanler_t)sys_sbrk,
    [SYS_fstat] = (syscall_hanler_t)sys_fstat,
    [SYS_dup] = (syscall_hanler_t)sys_dup,
    [SYS_ioctl] = (syscall_hanler_t)sys_ioctl,

    [SYS_opendir] = (syscall_hanler_t)sys_opendir,
    [SYS_readdir] = (syscall_hanler_t)sys_readdir,
    [SYS_closedir] = (syscall_hanler_t)sys_closedir,
};

void do_handler_syscall (syscall_frame_t * frame) {
    if (frame->func_id < sizeof(sys_table) / sizeof(sys_table[0])) {
        syscall_hanler_t handler = sys_table[frame->func_id];
        if (handler) {
            int ret = handler(frame->arg0, frame->arg1, frame->arg2, frame->arg3);
            frame->eax = ret;
            return;
        }
    }

    task_t * task = task_current();
    log_printf("task: %s, Unknown syscall: %d", task->name, frame->func_id);
    frame->eax = -1;
}