start qemu-system-i386  -m 128M -serial stdio -s -S -hda disk1.vhd -hdc disk2.vhd -d pcall,cpu_reset,guest_errors,trace:ps2_keyboard_set_translation
