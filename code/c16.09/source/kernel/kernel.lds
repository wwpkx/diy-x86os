/* 参考文档： https://ftp.gnu.org/old-gnu/Manuals/ld-2.9.1/html_chapter/ld_3.html */
SECTIONS
{
	PROVIDE(kernel_base = 0x0);

	. = 0x00010000;

	PROVIDE(s_text = .);
	.text : {
		*(EXCLUDE_FILE(*first_task* *lib_syscall*) .text)
	}
	.rodata : {
		*(EXCLUDE_FILE(*first_task* *lib_syscall*) .rodata)
	}
	PROVIDE(e_text = .);

	. = ALIGN(4096);
	PROVIDE(s_data = .);
	.data : {
		*(EXCLUDE_FILE(*first_task* *lib_syscall*) .data)
	}
	.bss : {
		*(EXCLUDE_FILE(*first_task* *lib_syscall*) .bss)
	}

	/* 初始进程的配置：接紧着在低端1MB内存开始存储，但是运行时搬运到0x80000000处 */
	. = ALIGN(4096);
	PROVIDE(s_first_task = .);
	PROVIDE(e_first_task = . + SIZEOF(.first_task));
	.first_task 0x80000000: AT(s_first_task) {
		*first_task_entry*(.text .data .bss)
		*first_task*(.text .data .bss)
	    *lib_syscall*(.text .data .bss)
	}

	/* 重设LMA，并对齐的4KB */
	. = ALIGN(4096);
	PROVIDE(mem_free_start = . - 0x80000000);
}
