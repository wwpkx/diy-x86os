/**
 * os配置
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef OS_OS_CFG_H
#define OS_OS_CFG_H

#define OS_VERSION      		100			// 操作系统版本号
#define	BOOT_INFO_ADDR			0x6000		// boot启动信息的地址
#define	BOOT_START_ADDR			0x7c00		// boot运行地址
#define LOADER_START_ADDR       0x8000      // loader的起始地址，位于BOOT的后边

#define KERNEL_FILENAME			"KERNEL"	// FAT32中内核文件的名称，必须大写

// 系统启动后所用的画面模式
#define INIT_SCREEN_WIDTH       800
#define INIT_SCREEN_HEIGHT      600
#define INIT_SCREEN_BPP         32

#define GDT_TABLE_SIZE      	256		// GDT表项数量
#define KERNEL_SELECTOR_CS		(1 * 8)		// 内核代码段描述符
#define KERNEL_SELECTOR_DS		(2 * 8)		// 内核数据段描述符

#define OS_TICK_MS              10       	// 每毫秒的时钟数
#define TASK_SLICE_DEFAULT_MS   20

#define USE_DOUBLE_BUFFER		1			// 屏幕刷新是否使用双缓存

// 任务管理
#define TASK_IDLE_STACK_SIZE     		40 * 1024	// 空闲任务栈

// 标签配置项
#define LABEL_CONTENT_MAX_LEN      256		// label标签最大容量

// 鼠标配置
#define MOUSE_EDGE_MARGIN		5			// 鼠标距离边沿的最小距离，避免移出看不见

// 主窗口配置
#define WINDOW_DEFAULT_BOARDER_SIZE		1	// 边框宽度
#define	WINDOW_DEFAULT_PADDING_SIZE		3	// 填充宽度
#define WINDOW_CONTENT_BOARDER_SIZE		5	// 内容区边框宽度

#define TITLE_BAR_ACTIVE_COLOR      COLOR_MidnightBlue  // 标题栏激活颜色
#define TITLE_BAR_DEACTIVE_COLOR    COLOR_DimGray		// 标题栏不激活时颜色
#define WINDOW_TITLE_BAR_HEIGHT     40					// 标题栏默认高度

#define WINDOW_TITLE_SIZE       128		// 标题栏字符长度

// 终端显示部件
#define	TTY_WIDGET_CURSOR_WIDTH			1		// 光标宽度
#define	TTY_CURSOR_TIMER_INTERVAL		500		// 光标闪烁定时器
#define	ESC_PARAM_MAX					10		// 最多支持的ESC [ 参数数量


// UI进程配置
#define UI_TASK_SIZE        	40 * 1024	// 堆栈
#define UI_TASK_PRIORITY    	0			// 优先级
#define UI_MSG_COUNT			50			// UI线程的消息总量

#define UI_KBD_OCDE_COUNT          10		// 最大允许的键盘参数
#define UI_API_PARAM_COUNT          10		// 最大允许的参数数量

// tty配置
#define TTY_IN_MSG_MAX_COUNT		10			// 输入最大消息数量，可设大一些
#define TTY_OUT_MSG_MAX_COUNT		10			// 输出最大消息数量，可设大一些
#define TTY_MAX_COUNT				10			// 最大支持的TTY数量

#define	FILE_TABLE_MAX				100			// 最大支持打开的文件数量

// 文件系统
#define	ROOT_DEV						0x001		// 根文件系统设备
#define	XDISK_MAX_COUNT					2		// 最多支持的磁盘数量
#define	DISK_PART_PER_DISK				4		// 每磁盘的最大分区数
#define	XDISK_PART_MAX_COUNT			4  	// 最多支持的总分区数量

#define	DISK_SIZE_PER_SECTOR			512	// 每扇区字节数
#define	ATA_REQUEST_NR					128		// ATA请求数量

#define	FILE_TABLE_MAX_SIZE				1024	// 最大支持打开的文件数量
#define	FILE_NODE_MAX_SIZE				1024	// 文件结点表最大数量
#define	TASK_FILE_MAX_SIZE				20		// 任务最多打开的文件数量

#define	MOUNT_LIST_MAX_SIZE				4		// 最多挂载的分区
#define	MOUT_NAME_MAX_SIZE				32		// 挂载名称长度

#endif //OS_OS_CFG_H
