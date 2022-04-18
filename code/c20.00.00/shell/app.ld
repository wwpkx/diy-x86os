ENTRY(_start)
SECTIONS {
    .text 0x6800000 : {
   		*crt_start.o(.text)
        * (.text)
    }

    .data : {
         * (.data)
    }
    .bss : {
         * (.bss)
    }
}