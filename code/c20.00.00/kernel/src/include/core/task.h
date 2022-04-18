//
// Created by lishutong on 2021-07-18.
//

#ifndef OS_TASK_H
#define OS_TASK_H

#include <core/time.h>
#include "core/cpu.h"
#include "core/list.h"
#include <ipc/queue.h>
#include <dev/keyboard.h>
#include <dev/tty.h>
#include <fs/fs.h>

#define TASK_LDT_TABLE_SIZE   	10		// 任务的LDT表大小
#define TASK_PRIORITY_NR    	64		// 任务优先级层次，不超过64
#define TASK_NAME_SIZE       	16		// 任务名字最大长度
#define TASK_MSG_NR				10		// 任务消息缓冲区长度

#define TASK_ANY_CRATED     	1

#define TASK_STATE_INITED       0
#define TASK_STATE_READY        1
#define TASK_STATE_SLEEP        2
#define TASK_STATE_SUSPEND      3
#define TASK_STATE_WAIT         4

#define TASK_FLAG_SYSTEM_TASK       (1 << 0)

typedef struct _task_cpu_t {
    uint16_t ldt_selector;
    gdt_descriptor_t ldt_table[TASK_LDT_TABLE_SIZE];
    uint32_t tss_selector;
    tss_t tss;
}task_cpu_t;

typedef enum _task_msg_type_t {
	APP_MSG_TYPE_KEYBOARD,
	APP_MSG_TYPE_TIMER,

	APP_MSG_TYPE_QUIT,
}task_msg_type_t;

/**
 * 发给任务的消息
 * 不得随意改
 */
typedef struct _app_msg_t {
	task_msg_type_t type;

	union {
		key_data_t key;			// 键盘消息的键值
		struct {			// 定时器消息
			rtimer_t * timer;		// 定时器
			int data;		// 定时器数据
		};
	};
}app_msg_t;

/**
 * 发送给任务的消息
 */
typedef struct _task_msg_t {
	queue_msg_t base;
	app_msg_t msg;
}task_msg_t;

typedef struct _task_t {
    struct {
        uint32_t state : 8;     // 状态标志̬
    };

    char name[TASK_NAME_SIZE];
    int flags;
    uint32_t priority;          // 优先级
    uint32_t delay_ticks;       // 延时计数
    uint32_t slice_counter;     // 时间片计数
    list_node_t node;           // 链接结点

//    int tty;					// 任务当前使用的tty
    task_cpu_t cpu_context;     // CPU下下文̬
    queue_t queue;				// 发给任务的消息队列

    xfile_node_t* curr_dir;				// 当前目录
    xfile_t * file_table[TASK_FILE_MAX_SIZE];	// 任务最多打开的文件数量
}task_t;

typedef void task_param_t;

typedef void (task_entry_t)(task_param_t * param);
int task_init (task_t * task, const char * name, int flags, task_entry_t * entry,
        task_param_t * param, uint32_t * stack_top, uint32_t priority,  void * msg_buf, int msg_size,  int msg_count);
int task_suspend (task_t * task);
int task_wakeup (task_t * task);
void task_sleep (uint32_t ms);
void task_switch_to (task_t * task);

void task_add_ready(task_t *task);
void task_remove_ready(task_t *task);

void task_remove(task_t *task);
void task_time_tick (void);
void task_dispatch(void);
void move_to_first_task(void);
task_t * task_current (void);

typedef struct _task_manager_t {
    uint32_t flags;                 // 标准位
    task_t * curr_task;             // 当前运行的任务
    task_t * idle_task;             // 空闲任务

    uint8_t ready_group;            // 就绪的任务表组位图
    uint8_t ready_bitmap[(TASK_PRIORITY_NR  + 7)/ 8];   // 就绪表位图
    list_t ready_list[TASK_PRIORITY_NR]; 	// 各就绪的任务队列
    list_t suspend_list;        // 挂起队列
    list_t sleep_list;          // 睡眠，延时队列
}task_manager_t;

void task_manager_init (void);

#endif //OS_TASK_H
