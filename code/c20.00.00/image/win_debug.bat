
@ 写boot区，定位到dbr区，写只写代码区
@ fat16分区位于第1扇区（从前面的fdisk中获得），fat16的头占62字节，所以写起始偏移为512+62=574
@ 读取起始偏移为：62字节，用于跳过boot.bin中的fat16头

rem 写boot区，定位到dbr区，写只写代码区
rem fat16分区位于第2扇区（从前面的fdisk中获得），fat16的头占62字节，所以写起始偏移为1024+62=1086
rem 读取起始偏移为：62字节，用于跳过boot.bin中的fat16头
rem 总拷贝数量为：count=512-62=450

dd if=boot.bin of=os.vhd bs=1 conv=notrunc skip=62 seek=1086 count=450

rem diskpart脚本，用于挂载os.vhd， 如果k盘被占用，请换个其它的名字
set driver_num=k
echo select vdisk file="%cd%\os.vhd" >a.txt
echo attach vdisk >>a.txt
echo select partition 1 >> a.txt
echo assign letter=%driver_num% >> a.txt
diskpart /s a.txt
del a.txt

rem 开始复制
if exist loader.bin (
	copy /Y loader.bin %driver_num%:\loader.bin
) else (
	echo "loader.bin not exist"
)

if exist kernel.elf (
	copy /Y kernel.elf %driver_num%:\kernel.elf
) else (
	echo "kernel.elf not exist"
)

if exist snake.elf (
	copy /Y snake.elf %driver_num%:\snake.elf
) else (
	echo "snake.elf not exist"
)

if exist counter.elf (
	copy /Y counter.elf %driver_num%:\counter.elf
) else (
	echo "counter.elf not exist"
)

if exist shell.elf (
	copy /Y shell.elf %driver_num%:\shell.elf
) else (
	echo "shell.elf not exist"
)

@ 结束复制，取消os.vhd挂载
echo select vdisk file="%cd%\os.vhd" >a.txt
echo detach vdisk >>a.txt
diskpart /s a.txt
del a.txt

qemu-system-i386  -m 128M  -S -gdb tcp::1234,ipv4 -drive format=raw,file=os.vhd -d pcall,cpu_reset,guest_errors,trace:ps2_keyboard_set_translation