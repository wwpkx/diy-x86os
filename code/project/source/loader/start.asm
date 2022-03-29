/**
 * 自己动手写操作系统
 *
 * 二级引导，负责进行硬件检测，进入保护模式，然后加载内核，并跳转至内核运行
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
  	// 16位代码，务必加上
  	.code16
 	.text
 	.extern loader_entry
_start:
	// 沿用之前的设置
	jmp loader_entry

	// 32位保护模式下的代码
	.code32
	.text
	.global protect_mode_entry
	.extern load_kernel
protect_mode_entry:
	// 重新加载所有的数据段描述符
	mov $16, %ax		// 16为数据段选择子
	mov %ax, %ds
    mov %ax, %ss
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    // 长跳转进入到32位内核加载模式中
    jmp $8, $load_kernel


