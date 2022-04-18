SECTIONS {
    .text 0x8000 : {
   		*asm.o(.text)
        * (.text)
    }

    .data : {
         * (.data)
    }
    .bss : {
         * (.bss)
    }
}