ENTRY(_start)
SECTIONS
{
	/* init起始地址要求固定，因为kernel会跳到这里运行 */
	. = 0x82000000;
	.text : {
		*(*.text)
	}

	.rodata : {
		*(*.rodata)
	}

	.data : {
		*(*.data)
	}

	.bss : {
		*(*.bss)
	}
}
