/**
 * os配置
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef OS_OS_CFG_H
#define OS_OS_CFG_H

#define GDT_TABLE_SIZE      	8192		// GDT表项数量
#define KERNEL_SELECTOR_CS		(1 * 8)		// 内核代码段描述符
#define KERNEL_SELECTOR_DS		(2 * 8)		// 内核数据段描述符

#define OS_TICK_MS              10       	// 每毫秒的时钟数

#define LOG_ENABLE              1           // 是否开启log输出

#define OS_VERSION              "0.0.1"     // OS版本号

#define IDLE_STACK_SIZE       1024          // 空闲任务栈

#define TASK_LDT_TABLE_SIZE   	10		    // 任务的LDT表大小

#define SELECTOR_SYSCALL     	(3 << 3)	// 调用门的选择子
#define OS_VERSION_NUM          0x1001      // OS版本号

#endif //OS_OS_CFG_H
