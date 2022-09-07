/**
 * 功能：16位与32位的启动混合代码
 *
 *创建时间：2022年8月31日
 *作者：李述铜
 *联系邮箱: 527676163@qq.com
 *相关信息：此工程为《从0写x86 Linux操作系统》的前置课程，用于帮助预先建立对32位x86体系结构的理解。整体代码量不到200行（不算注释）
 *课程请见：https://study.163.com/course/introduction.htm?courseId=1212765805&_trace_c_p_k2_=0bdf1e7edda543a8b9a0ad73b5100990
 */
	#include "os.h"

	// 声明本地以下符号是全局的，在其它源文件中可以访问
	.global _start,timer_init, syscall_handler

	// 当需要引用外部的符号是，使用extern声明。相当于C语言中的extern
	.extern gdt_table, pg_dir, idt_table, task_sched

	// 指定以下的代码生成16位的机器指令，这样才能在启动时的实模式下运行
  	.code16

	// 以下是代码区
 	.text
_start:
	// x86使用段寄存器中的内容作为内存访问的基址，因此以下预先设置好
	mov $0, %ax				// 设置代码段
	mov %ax, %ds			// 设置数据段
	mov %ax, %es			// 设置数据段
	mov %ax, %ss			// 设置栈段
	mov $_start, %esp		// 设置栈的起始地址

	// 计算机上电启动后，只取512字节到0x7c00处，即相当于自己只有个头在内存，所以下面全部出来
	// 将自己的其余部分读取到0x7E00处，即0x7c00的后512字节
read_self_all:
	mov $_start_32, %bx	// 读取到的内存地址
	mov $0x2, %cx		// ch:磁道号，cl起始扇区号
	mov $0x0240, %ax	// ah: 0x42读磁盘命令, al=0x40 64个扇区，多读一些, 32KB
	mov $0x80, %dx		// dh: 磁头号，dl驱动器号0x80(磁盘1)
	int $0x0013
	jc read_self_all	// 读取失败，则重复

	//  加载idt和gdt表, 进入保护模式
	cli					// 关中断
	lgdt [gdt_desc]		// 加载gdt表
	lidt [idt_desc]		// 加载idt表
	mov $1, %eax
	lmsw %ax			// 设置PE位，进入保护模式
	jmp $KERNEL_CODE_SEG, $_start_32	// 进入32位保护模式代码，可以运行32位指令

	// 跳到引导标志, 由bios在上电后检查
	.org 0x1fe			// 引导标志
	.byte 0x55, 0xaa

	// 32位保护模式，位于512字节后
	.code32
	.text
_start_32:
	// 重新加载数据段寄存器
	mov $KERNEL_DATA_SEG, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %ss
	mov $_start, %esp

	// 跳转到c语言中运行
	call os_init

	// 打开分页机制
	mov $pg_dir, %eax
	mov %eax, %cr3

	mov %cr4, %eax
	orl $(1 << 4), %eax			// PSE位，支持4MB分页
	mov %eax, %cr4

	mov %cr0, %eax
	orl $(1 << 31), %eax		// 打开PG位，开启分页机制
	mov %eax, %cr0

	// 设置当前的任务0
	// 模拟中断返回，返回至任务0，特权级3模式
	mov $TASK0_TSS_SEL, %ax		// 加载任务0对应的状态段
	ltr %ax

	// 下面模拟中断返回，从而实现从特权级0到特权级3的变化
	// 中断发生时，会自动压入原SS, ESP, EFLAGS, CS, EIP到栈中
	push $APP_DATA_SEG
	push $task0_dpl3_stack + 1024	// 特权级3时的栈
	push $0x202						// EFLAGS
	push $APP_CODE_SEG				// CPL=3时的选择子
	push $task_0_entry					// eip
	iret							// 从中断返回，将切换至任务0

task_0_entry:
	// 进入任务0时，需要重设其数据段寄存器为特权级3的
	mov %ss, %ax
	mov %ax, %ds
	mov %ax, %es
	jmp task_0			// 跳转到任务0运行

// 定时器0中断函数
timer_init:
	push %ds
	pusha						// 保护现场，段寄存器不用保存

	mov $0x20, %al
	outb %al, $0x20				// 发送EOI

	// 使用内核的数据段寄存器，因为要访问内核的数据
	mov $KERNEL_DATA_SEG, %ax
	mov %ax, %ds

	call task_sched				// 调用调试函数
	popa						// 恢复现场
	pop %ds
	iret						// 中断返回

syscall_handler:
	push %ds
	pusha						// 保护现场，段寄存器不用保存
	mov $KERNEL_DATA_SEG, %ax
	mov %ax, %ds				//  Push AX, CX, DX, BX, original SP, BP, SI, and DI.

	mov %esp, %ebp				// 下面压栈时，esp会不断变化，所以使用ebp作为基址
	push 13*4(%ebp)				// 提取出原压入的各项参数，再按相同的顺序压入一遍
	push 12*4(%ebp)
	push 11*4(%ebp)
	call do_syscall				// 调用api函数，注意这时是在特级0的模式下运行，可以做很多有需要高权限才能做的事
	add $(3*4), %esp			// 取消刚才压入的值
	popa						// 恢复现场
	pop %ds
	retf $(3*4)					// 使用远跳转

// gdt描述符，由lgdt加载
gdt_desc: .word (256*8) - 1
	.long gdt_table

// idt描述符，由idt加载
idt_desc: .word (256*8) - 1
	.long idt_table
