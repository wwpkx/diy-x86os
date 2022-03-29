//
// Created by lishutong on 2021-07-28.
//

#ifndef OS_SYSCALL_H
#define OS_SYSCALL_H

#include <core/types.h>

#define SELECTOR_SYSCALL     		(3 << 3)	// 调用门的选择子
#define SYSCALL_PARAM_COUNT         5       	// 系统调用最大支持的参数

#define SYSCALL_GET_OS_VERSION                  0
#define SYSCALL_GET_OS_VERSION_NUM              1
#define	SYSCALL_GET_TICKS						2
#define SYSCALL_SLEEP                           3
#define SYSCALL_SEND_MSG                   		4
#define SYSCALL_GET_EVENT						6
#define SYSCALL_CREATE_TIMER					7
#define SYSCALL_FREE_TIMER						8
#define SYSCALL_SET_TIMER						9
#define SYSCALL_GET_TIME_OF_DAY					10

#define SYSCALL_FILE_OPEN						100
#define SYSCALL_FILE_READ						101
#define SYSCALL_FILE_WRITE						102
#define	SYSCALL_FILE_LSEEK						103
#define SYSCALL_FILE_CLOSE						104
#define	SYSCALL_FILE_UNLINK						105
#define SYSCALL_FILE_IOCTL						109

#define SYSCALL_MSG_DEST_UI         			0		// 消息的接收方是UI服务器

/**
 * 系统调用的栈信息
 */
typedef struct _syscall_frame_t {
	int eip, cs;
	int syscall_num, arg0, arg1, arg2, arg3;
}syscall_frame_t;

void handler_syscall (void);
void init_syscall (void);

#endif //OS_SYSCALL_H
