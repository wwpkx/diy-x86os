SECTIONS {
    .text 0x6400000 : {
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