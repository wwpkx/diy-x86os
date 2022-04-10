
SECTIONS
{
	PROVIDE(kernel_base_addr = 0x0);

	/* 代码的起始地址为64KB地址处 */
	. = 0x00010000;
	PROVIDE(text_start = .);	
	.text : AT(0x10000) {
		*(.text)
	}

    /* 只读数据段 */
	.rodata : {
		*(.rodata .rodata.* .gnu.linkonce.r.*)
	}
	PROVIDE(text_end = .);	

	. = ALIGN(4096);

    /* 数据段 */
	PROVIDE(data_start = .);
	.data : {
		*(.data)
	}

    /* bss段 */
	.bss : {
		*(.bss)
	}
	PROVIDE(data_end = .);

	PROVIDE(mem_free_start = .);
}
