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
		*(.text)
		*(.rodata)
	}
	PROVIDE(text_end = .);	

	/* 数据区 */
	. = ALIGN(4096);
	PROVIDE(data_start = .);
	.data : {
		*(.data)
		*(.bss)
	}
	PROVIDE(data_end = .);

	/* 空闲区 */
	. = ALIGN(4096);
	PROVIDE(mem_free_start = .);
}
