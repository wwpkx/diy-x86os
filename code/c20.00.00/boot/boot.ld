
SECTIONS {
    .text 0x7c00 : {
        *boot_start.o(.text)
        *boot_c.o(.text)
		. = 0x1fe;
  		*boot_end.o (.text)
     }
}

