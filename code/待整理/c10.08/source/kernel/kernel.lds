ENTRY(_start)
SECTIONS
{
	PROVIDE(kernel_base_addr = 0x0);

	. = 0x00010000;

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
