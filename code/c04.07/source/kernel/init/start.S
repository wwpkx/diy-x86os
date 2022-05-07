/**
 * 自己动手写操作系统
 *
 * 二级引导，负责进行硬件检测，进入保护模式，然后加载内核，并跳转至内核运行
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
  	// 不必加.code32因默认就是32位
 	.text
 	.global _start
	.extern kernel_init
_start:
	push %ebp
	mov %esp, %ebp
	movl 8(%ebp), %eax			// 取传给loader的参数

	push %eax					// 再作为参数传递给kernel_init
	call kernel_init
	add $0x4, %esp				// 恢复之前的esp值

	jmp .




