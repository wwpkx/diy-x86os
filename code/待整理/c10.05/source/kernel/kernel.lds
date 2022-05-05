ENTRY(_start)
SECTIONS
{
	. = 0x00010000;

	PROVIDE(text_start = .);	
	.text : {
		*(.text)
	}
	.rodata : {
		*(.rodata .rodata.*)
	}
	PROVIDE(text_end = .);	

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
