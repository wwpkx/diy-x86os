/* 参考文档： https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_chapter/ld_3.html */
ENTRY(_start)
SECTIONS
{
	PROVIDE(kernel_base_addr = 0x0);

	/* 起始位置64KB，这样不会覆盖loader */
	. = 0x00010000;

	/* 代码区 */
	PROVIDE(text_start = .);
	.text : {
		*(EXCLUDE_FILE (*init_* *lib_syscall*) .text .rodata)
	}
	PROVIDE(text_end = .);	

	/* 数据区 */
	. = ALIGN(4096);
	PROVIDE(data_start = .);
	.data : {
		*(EXCLUDE_FILE (*init_* *lib_syscall*) .data .bss)
	}
	PROVIDE(data_end = .);

	/* 空闲区 */
	. = ALIGN(4096);
	PROVIDE(mem_free_start = .);

	/* 启动代码，放在最开头，加载地址为0x80000000 */
	.init_task 0x80000000: {
		/*不能将lib_syscall和init放一行，这样调试时将导致init中设置不了断点. 可能是识别不了 */
		*init_*(.text .data .bss)
		*lib_syscall*(.text .data .bss)
	}
	PROVIDE(init_task_addr = LOADADDR(.init_task));
	PROVIDE(init_task_size = SIZEOF(.init_task));
	PROVIDE(init_heap_top = .);
}
