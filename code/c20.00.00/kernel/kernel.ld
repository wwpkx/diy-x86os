ENTRY(kernel_entry)
SECTIONS {
	/* 1M以上的空间给内核使用 */
    .text 0x100000 : {
     	*(.text)
    }
    .data : { *(.data) }
    .bss : { *(.bss) }
}