qemu-system-i386  -m 128M -monitor stdio -s -S -drive format=raw,file=os.vhd -d pcall,page,mmu,cpu_reset,guest_errors,trace:ps2_keyboard_set_translation_translation
