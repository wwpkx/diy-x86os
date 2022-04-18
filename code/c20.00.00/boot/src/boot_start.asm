/**
 * 自己动手写操作系统
 *
 * 系统引导部分，启动时由硬件加载运行，然后完成对二级引导程序loader的加载
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
  	// 16位代码，务必加上
  	.code16

  	// FAT的DBR区及最初始的代码区
 	.text
	.extern boot_entry
	.global fat_dbr
	.global loading
 fat_dbr: .space 62
_code:
	// 重新设置各个数据段段及栈空间
	// 根据https://wiki.osdev.org/Memory_Map_(x86)
	// 大约有30KB的RAM用作栈空间，足够boot和loader
	mov $0, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %ss
	mov $fat_dbr, %esp		// 将dbr之前也就boot之前的空间用作栈

	call loading
	call boot_entry

loading:
	pusha
	mov $0xe, %ah
	mov $'B', %al
	mov $0x3, %bx
	int $0x10
	popa
	ret
