start qemu-system-i386 -k en-us -m 128M -monitor stdio -s -S -drive format=raw,file=os.vhd -d pcall,cpu_reset,guest_errors,trace:ps2_keyboard_set_translation
