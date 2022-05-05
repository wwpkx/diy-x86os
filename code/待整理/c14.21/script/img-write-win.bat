rem 写boot区，定位到磁盘开头，写1个块：512字节
dd if=boot.bin of=disk1.vhd bs=512 conv=notrunc count=1 

rem 写loader区，定位到磁盘第2个块，写1个块：512字节
dd if=loader.bin of=disk1.vhd bs=512 conv=notrunc seek=1

rem 写kernel区，定位到磁盘第100个块
dd if=kernel.elf of=disk1.vhd bs=512 conv=notrunc seek=100

rem 写应用程序init，临时使用
rem dd if=init.elf of=dis1.vhd bs=512 conv=notrunc seek=5000
rem dd if=shell.elf of=disk1.vhd bs=512 conv=notrunc seek=5000

rem 写应用程序，使用系统的挂载命令
rem 开始复制
set DISK_NAME=disk2.vhd
set TARGET_PATH=k
echo select vdisk file="%cd%\%DISK_NAME%" >a.txt
echo attach vdisk >>a.txt
echo select partition 1 >> a.txt
echo assign letter=%TARGET_PATH% >> a.txt
diskpart /s a.txt
del a.txt

copy /Y init.elf %TARGET_PATH%:\init.elf
copy /Y shell.elf %TARGET_PATH%:\shell.elf
copy /Y loop.elf %TARGET_PATH%:\loop.elf

echo select vdisk file="%cd%\%DISK_NAME%" >a.txt
echo detach vdisk >>a.txt
diskpart /s a.txt
del a.txt
