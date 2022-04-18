ENTRY(_start)
SECTIONS
{
	/* init起始地址要求固定，因为kernel会跳到这里运行 */
	. = 0x80000000;
	.text : {
		*crt0*.o(*.text)
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
