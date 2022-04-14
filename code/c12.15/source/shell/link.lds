ENTRY(_start)
SECTIONS
{
	/* 这个值可以随意的，只要在用户空间就行 */
	. = 0x81000000;
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
