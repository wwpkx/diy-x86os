@REM 适用于windows
start qemu-system-i386  -m 128M -s -S -hda disk1.vhd -hdc disk2.vhd -d pcall,page,mmu,cpu_reset,guest_errors,page,trace:ps2_keyboard_set_translation