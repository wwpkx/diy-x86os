/**
 * 系统调用实现
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef OS_SYSCALL_H
#define OS_SYSCALL_H

#define SYSCALL_PARAM_COUNT     5       	// 系统调用最大支持的参数

void exception_handler_syscall (void);		// syscall处理

#endif //OS_SYSCALL_H
