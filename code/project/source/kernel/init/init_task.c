/**
 * 创建初始任务
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <core/os_cfg.h>
#include <core/cpu_instr.h>
#include <core/task.h>
#include <init/init.h>
#include <ui/tty_widget.h>
#include <dev/dev.h>
#include <fs/mount.h>

// 以下为测试代码，
#if 1
static task_t counter_task;
static uint32_t counter_stack[200 * 1024];
static task_msg_t counter_msg_buffer[32];

static void counter_task_entry(task_param_t *param) {
	((void (*)(void))0x6400000)();
}


task_t snake_task;
static uint32_t snake_task_stack[32 * 1024];
static task_msg_t snake_msg_buffer[32];

void snake_task_entry(task_param_t *param) {
	((void (*)(void))0x6700000)();
}

static task_t shell_task;
static uint32_t shell_task_stack[200 * 1024];
static task_msg_t shell_msg_buffer[32];

static void shell_task_entry(task_param_t *param) {
	date_time_t date_time;

//	read_time_date(&date_time);

	int fd_stdin = xfile_open("/tty/0", 0);		// 打开stdin
	xfile_dup(fd_stdin);			// 打开stdout，使用同一设备
	xfile_dup(fd_stdin);			// 打开stderr，使用同一设备

//	int tty = tty_open(&tty_driver, 0);
//	task_current()->tty = tty;

	((void (*)(void))0x6800000)();
}

/**
 * 调用任务初始化：调试用
 */
static void app_task_init (void) {
	// 设置第一个任务, 满递减的栈，所以写末端。kernel所用的栈仍然是来自loader中设置的栈
	// 在这里没有必要设置，不起作用，因为切换到其它任务时，esp会指向之前的设置
	task_init(&counter_task,
		  "window task",
		  0,
		  counter_task_entry,
		  (void *) 0,
		  (uint32_t *) ((uint8_t *) counter_stack + sizeof(counter_stack)),
		  1,
		  counter_msg_buffer, sizeof(task_msg_t),  sizeof(counter_msg_buffer) / sizeof(task_msg_t));

//	task_init(&snake_task,
//		"Snake Game",
//		  0,
//		  snake_task_entry,
//		  (void *) 0x22334455,
//		  (uint32_t *) ((uint8_t *) snake_task_stack + sizeof(snake_task_stack)),
//		  1,
//		  snake_msg_buffer, sizeof(task_msg_t), sizeof(snake_msg_buffer) / sizeof(task_msg_t));

	task_init(&shell_task,
			"Shell",
			  0,
			  shell_task_entry,
			  (void *) 0x22334455,
			  (uint32_t *) ((uint8_t *) shell_task_stack + sizeof(shell_task_stack)),
			  1,
			  shell_msg_buffer, sizeof(task_msg_t), sizeof(shell_msg_buffer) / sizeof(task_msg_t));
}
#else
#define	app_task_init()
#endif

static task_t init_task;				// 空闲任务结构
static uint32_t init_task_stack[TASK_IDLE_STACK_SIZE];	// 空闲任务堆栈

/**
 * 空闲任务代码
 */
void init_task_entry(task_param_t *param) {
	// 安装根文件系统，装到c盘上
	fs_mount("c:", ROOT_DEV);
	fs_mount("tty", make_device_num(DEV_TTY, 0, 0));

	// 创建应用任务，
	app_task_init();
    for (;;) {
    	sys_msleep(100);
    }
}

static task_t idle_task;				// 空闲任务结构
static uint32_t idle_task_stack[TASK_IDLE_STACK_SIZE];	// 空闲任务堆栈

/**
 * 空闲任务代码
 */
void idle_task_entry(task_param_t *param) {
    for (;;) {
        hlt();
    }
}


/**
 * 创建第一个任务
 */
void create_init_task (void) {
    // 创建空闲任务
    task_init(&idle_task,
              "idle task",
              TASK_FLAG_SYSTEM_TASK,
			  idle_task_entry,
			  (void *) 0x12345678,
              (uint32_t *) ((uint8_t *) idle_task_stack + sizeof(idle_task_stack)),
			  TASK_PRIORITY_NR - 1,
			  0, 0, 0);

    // 创建初始任务
    task_init(&init_task,
              "init task",
              TASK_FLAG_SYSTEM_TASK,
			  init_task_entry,
			  (void *) 0x12345678,
              (uint32_t *) ((uint8_t *) init_task_stack + sizeof(init_task_stack)),
			  0,
			  0, 0, 0);
}
