/**
 * os配置
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef OS_OS_CFG_H
#define OS_OS_CFG_H

#define	LOADER_START_ADDR	    0x8000		// loader加载的地址

#define GDT_TABLE_SIZE      	256		// GDT表项数量
#define KERNEL_SELECTOR_CS		(1 * 8)		// 内核代码段描述符
#define KERNEL_SELECTOR_DS		(2 * 8)		// 内核数据段描述符
#define SELECTOR_SYSCALL     	(3 * 8)	// 调用门的选择子

#define OS_TICK_MS              10       	// 每毫秒的时钟数

#define LOG_ENABLE              1           // 是否开启log输出

#define OS_VERSION              "0.0.1"     // OS版本号

#define TASK_NR                 128         // 进程的数量
#define ROOT_DEV                0xc1        // 根文件系统所在的设备

#endif //OS_OS_CFG_H