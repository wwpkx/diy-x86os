/**
 * os配置
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef OS_OS_CFG_H
#define OS_OS_CFG_H

#define GDT_TABLE_SIZE      	256		// GDT表项数量
#define KERNEL_SELECTOR_CS		(1 * 8)		// 内核代码段描述符
#define KERNEL_SELECTOR_DS		(2 * 8)		// 内核数据段描述符
#define KERNEL_STACK_SIZE       (8*1024)    // 内核栈

#endif //OS_OS_CFG_H
