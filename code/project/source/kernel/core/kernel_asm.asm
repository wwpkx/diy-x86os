	// 内核数据段，要与C中设置的一致
	.equ KERNEL_SELECTOR_DS,	(2 * 8)

// 中断发生时，会自动切换到特权级0对应的栈中去执行
// 并且只保存ss,esp,cs,eip,flags寄存器
// 所以需要在中断中自行保存其它寄存器

    .text
    .extern irq_enter_handler, irq_leave_handler
// 使用宏来生成相应的代码，这样简单
// 而且可将处理流程简化成相同的处理
.macro excetpion_handler name num with_error_code is_int=0
	.global handler_\name
handler_\name:
	// 如果没有错误码，压入一个缺省值
	// 这样堆栈就和有错误码的情形一样了
	.if \with_error_code == 0
		push $0
	.endif

	// 压入异常号
	push $\num

	// 寄存器调用：https://blog.csdn.net/w55100/article/details/89739484
	// 压入可能被C处理程序改写的寄存器
	// 没有必要将所有的寄存器都进行保存，太过于浪费时间了
	push %eax
	push %ecx
	push %edx

	// 这里还要切换至内核的数据段寄存器
	push %ds
	push %es
	push %fs
	push %gs

	mov $(KERNEL_SELECTOR_DS), %eax
	mov %eax, %ds
	mov %eax, %es
	mov %eax, %fs
	mov %eax, %gs

	// 调整esp指向异常号的位置，eax, ecx, edx对中断处理函数没什么用
	// 跳转到统一的处理程序
	push %esp

	.if \is_int
	call irq_enter_handler
	.endif

	call do_handler_\name

	.if \is_int
	call irq_leave_handler
	.endif

	pop %eax		// 丢掉刚重设的esp

	pop %gs
	pop %fs
	pop %es
	pop %ds

	// 恢复保存的寄存器
	pop %edx
	pop %ecx
	pop %eax

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

// 中断处理函数
excetpion_handler keyboard, 0x21, 0, 1
excetpion_handler mouse, 0x2c, 0, 1
excetpion_handler timer, 0x20, 0, 1
excetpion_handler primary_ata, 0x2e, 0, 1

     .global handler_syscall
    .extern do_handler_syscall
handler_syscall:
    // 保存可能被C程序修改的寄存器
    // %eax不需要保存，因为do_handler_syscall的返回值保存在eax中
    // 且会给调用者使用，所以下面对eax做了修改也没关系，只要保证在调用do_handler_syscall
    // 之后，不更改eax即可
	//push %eax
	push %ecx
	push %edx

	// 这里还要切换至内核的数据段寄存器
	pushl %ds
	pushl %es
	pushl %fs
	pushl %gs

	mov $(KERNEL_SELECTOR_DS), %eax
	mov %eax, %ds
	mov %eax, %es
	mov %eax, %fs
	mov %eax, %gs

    // 调整到syscall的参数传入位置, 6个此处压的寄存器
    mov %esp, %eax
    add $(4*6), %eax
    push %eax
	call do_handler_syscall
	add $4, %esp

    // 再切换回来
    // 返回值保存在%eax中
	pop %gs
	pop %fs
	pop %es
	pop %ds

	// 恢复保存的寄存器
	pop %edx
	pop %ecx
	//pop %eax

	// 5个参数，加上5*4，不加会导致返回时ss取不出来，最后返回出现问题
    retf $(5*4)
