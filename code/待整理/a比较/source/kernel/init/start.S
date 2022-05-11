/**
 * 自己动手写操作系统
 *
 * 二级引导，负责进行硬件检测，进入保护模式，然后加载内核，并跳转至内核运行
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
.set KERNEL_SELECTOR_CS, 8
.set KERNEL_SELECTOR_DS, 16
 .set KERNEL_STACK_SIZE, 4096

  	// 不必加.code32因默认就是32位
 	.text
 	.global _start
	.extern kernel_init
	.extern init_main
_start:
    # 第一种方法
    # push %ebp
    # mov %esp, %ebp
    # mov 0x8(%ebp), %eax
    # push %eax

    # 第二种方法
    # mov 4(%esp), %eax
    # push %eax

    # 第三种方法
    push 4(%esp)

    # kernel_init(boot_info)
    call kernel_init

	// 重新加载GDT
	jmp $KERNEL_SELECTOR_CS, $gdt_reload

gdt_reload:
	mov $KERNEL_SELECTOR_DS, %ax		// 16为数据段选择子
	mov %ax, %ds
    mov %ax, %ss
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

	// 栈设置
	mov $(stack + KERNEL_STACK_SIZE), %esp

	// 栈和段等沿用之前的设置
	jmp init_main

	.data
.comm stack, KERNEL_STACK_SIZE

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

	pushal
	push %ds
	push %es
	push %fs
	push %gs
	pushfl

	// 调整esp指向异常号的位置，eax, ecx, edx对中断处理函数没什么用
	// 跳转到统一的处理程序
	push %esp

	// 使用内核段寄存器
	mov $(KERNEL_SELECTOR_DS), %eax
	mov %eax, %ds
	mov %eax, %es
	mov %eax, %fs
	mov %eax, %gs

	// 调用中断处理函数
	call do_handler_\name

	add $(1*4), %esp		// 丢掉esp

	// 恢复保存的寄存器
	popfl
	pop %gs
	pop %fs
	pop %es
	pop %ds
	popal

	// 跳过压入的异常号和错误码
	add $(2*4), %esp

	iret
.endm

excetpion_handler divider, 0, 0
excetpion_handler Debug, 1, 0
excetpion_handler NMI, 2, 0
excetpion_handler breakpoint, 3, 0
excetpion_handler overflow, 4, 0
excetpion_handler bound_range, 5, 0
excetpion_handler invalid_opcode, 6, 0
excetpion_handler device_unavailable, 7, 0
excetpion_handler double_fault, 8, 1
excetpion_handler invalid_tss, 10, 1
excetpion_handler segment_not_present, 11, 1
excetpion_handler stack_segment_fault, 12, 1
excetpion_handler general_protection, 13, 1
excetpion_handler page_fault, 14, 1
excetpion_handler fpu_error, 16, 0
excetpion_handler alignment_check, 17, 1
excetpion_handler machine_check, 18, 0
excetpion_handler smd_exception, 19, 0
excetpion_handler virtual_exception, 20, 0

// 硬件中断
excetpion_handler timer, 0x20, 0
excetpion_handler keyboard, 0x21, 0
excetpion_handler ide_primary, 0x2E, 0
excetpion_handler ide_secondary, 0x2F, 0


     .global excetpion_handler_syscall
    .extern do_handler_syscall
excetpion_handler_syscall:
    // 保存可能被C程序修改的寄存器
 	pushal
	pushl %ds
	pushl %es
	pushl %fs
	pushl %gs
	pushfl

	// 使用内核段寄存器
	mov $(KERNEL_SELECTOR_DS), %eax
	mov %eax, %ds
	mov %eax, %es
	mov %eax, %fs
	mov %eax, %gs

    // 调用处理函数
    mov %esp, %eax
    push %eax
	call do_handler_syscall
	add $4, %esp

    // 再切换回来
	popfl
	pop %gs
	pop %fs
	pop %es
	pop %ds
	popal

	// 5个参数，加上5*4，不加会导致返回时ss取不出来，最后返回出现问题
    retf $(5*4)
