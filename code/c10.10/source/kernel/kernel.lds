
SECTIONS
{
	/* 代码的起始地址为64KB地址处 */
	. = 0xC0010000;
	.text : AT(0x10000) {
		*(.text)
	}

	PROVIDE(text_end = .);	

    /* 只读数据段 */
	.rodata : {
		*(.rodata .rodata.* .gnu.linkonce.r.*)
	}

	. = ALIGN(0x1000);

    /* 数据段 */
	PROVIDE(data = .);
	.data : {
		*(.data)
	}
	PROVIDE(data_end = .);

    /* bss段 */
	.bss : {
		*(.bss)
	}

	PROVIDE(mem_free_start = .);
}
