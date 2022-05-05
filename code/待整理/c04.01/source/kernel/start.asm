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
	.global _start
_start:
	// 栈和段等沿用之前的设置
	jmp kernel_entry



