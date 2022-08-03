ENTRY(_start)
SECTIONS
{
    /* Ϊ�˷�����ԣ�û�д�0x80000000��ʼ��������first_task��ͻ */
	. = 0x83000000;
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
