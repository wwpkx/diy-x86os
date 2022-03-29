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
 	.extern kernel_entry
	.global gdt_reload
gdt_reload:
	mov $16, %ax		// 16为数据段选择子
	mov %ax, %ds
    mov %ax, %ss
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

	// 栈设置，32KB足够
	mov $0x10000, %esp

	// 栈和段等沿用之前的设置
	jmp kernel_entry


	// 内核数据段，要与C中设置的一致
	.equ KERNEL_SELECTOR_DS,	(2 * 8)

// 中断发生时，会自动切换到特权级0对应的栈中去执行
// 并且只保存ss,esp,cs,eip,flags寄存器
// 所以需要在中断中自行保存其它寄存器

    .text
// 使用宏来生成相应的代码，这样简单
// 而且可将处理流程简化成相同的处理
.macro excetpion_handler name num with_error_code
	.global handler_\name
handler_\name:
	// 如果没有错误码，压入一个缺省值
	// 这样堆栈就和有错误码的情形一样了
	.if \with_error_code == 0
		push $0
	.endif

	// 压入异常号
	push $\num

	// 保存所有寄存器
	pusha
	push %ds
	push %es
	push %fs
	push %gs

	// 调用中断处理函数
	call do_handler_\name

	// 恢复保存的寄存器
	pop %gs
	pop %fs
	pop %es
	pop %ds
	popa

	// 跳过压入的异常号和错误码
	add $(2*4), %esp

	iret
.endm

excetpion_handler divider, 0, 0
