ENTRY(_start)
SECTIONS
{
	PROVIDE(kernel_base_addr = 0x0);

	/* 启动代码，放在最开头，加载地址为0x80000000 */
	.init_task 0x80000000: AT(0x10000) {
		/*不能将lib_syscall和init放一行，这样调试时将导致init中设置不了断点*/
		*init_*(.text .data .bss)
		*lib_syscall*(.text .data .bss)
	}
	PROVIDE(init_load_addr = LOADADDR(.init_task));
	PROVIDE(init_load_size = SIZEOF(.init_task));
	PROVIDE(init_heap_top = .);

	/* 重设LMA，并对齐的4KB */
	. = 0x00010000 + init_load_size;
	. = ALIGN(4096);

	/* 代码的起始地址为64KB地址处 */
	PROVIDE(text_start = .);
	.text : {
		*(.text)
	}
	.rodata : {
		*(.rodata .rodata.*)
	}
	PROVIDE(text_end = .);	

	. = ALIGN(4096);
	PROVIDE(data_start = .);
	.data : {
		*(.data)
	}
	.bss : {
		*(.bss)
	}
	PROVIDE(data_end = .);

	. = ALIGN(4096);
	PROVIDE(mem_free_start = .);
}
