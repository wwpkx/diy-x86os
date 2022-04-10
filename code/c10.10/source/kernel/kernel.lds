
SECTIONS
{
	PROVIDE(kernel_base_addr = 0x0);

	/* 启动代码，放在最开头，加载地址为0x80000000 */
	.text.init 0x80000000: AT(0x10000) {
		*init_task*(.text .data .bss)
	}
	PROVIDE(init_load_addr = LOADADDR(.text.init));
	PROVIDE(init_load_size = SIZEOF(.text.init));

	/* 重设LMA，并对齐的4KB */
	. = 0x00010000 + SIZEOF(.text.init);
	. = ALIGN(4096);

	/* 代码的起始地址为64KB地址处 */
	PROVIDE(text_start = .);	
	.text : {
		*(.text)
	}
	.rodata : {
		*(.rodata .rodata.* .gnu.linkonce.r.*)
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
	PROVIDE(mem_free_start = .);
}
