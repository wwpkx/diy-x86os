/* 参考文档： https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_chapter/ld_3.html */
ENTRY(_start)
OUTPUT_FORMAT("elf32-i386")  /* 输出格式, 由于链接参数中指定了binary，需要加上，不然生成的是bin文件 */
SECTIONS
{
	PROVIDE(kernel_base_addr = 0x0);

	/* 起始位置64KB，这样不会覆盖loader */
	. = 0x00010000;

	/* 代码区 */
	PROVIDE(text_start = .);
	.text : {
		*(EXCLUDE_FILE(*first_task* *lib_syscall*) .text)
	} 

	.rodata : {
		*(EXCLUDE_FILE(*first_task* *lib_syscall*) .rodata)
	}
	PROVIDE(text_end = .);	

	/* 数据区 */
	. = ALIGN(4096);
	PROVIDE(data_start = .);
	.data : {
		*(EXCLUDE_FILE(*first_task* *lib_syscall*) .data)
	}
	.bss : {
		*(EXCLUDE_FILE(*first_task* *lib_syscall*) .bss)
	}
	PROVIDE(data_end = .);

	/* 第一个任务，与内核一起编译链接，但是会被复制到用户空间中运行 */
	PROVIDE(first_task_start = .);
	PROVIDE(first_task_end = . + SIZEOF(.first_task));
	.first_task 0x80000000: AT(first_task_start) {
		/*不能将lib_syscall和init放一行，这样调试时将导致init中设置不了断点*/
		*first_task*(.text)
		*lib_syscall*(.text)
	}

	.first_task.data : {
		*first_task*(.data .bss)
		*lib_syscall*(.data .bss)
	}

	/* 空闲区 */
	. = ALIGN(4096);
	PROVIDE(mem_free_start = . - 0x80000000);
}
