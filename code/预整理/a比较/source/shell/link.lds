ENTRY(_start)
SECTIONS
{
	/* 这个值可以随意的，只要在用户空间就行 */
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
		__bss_start__ = .;
		*(*.bss)
    	__bss_end__ = . ;
	}
}
