# 写boot区，定位到磁盘开头，写1个块：512字节
dd if=boot.bin of=disk1.vhd bs=512 conv=notrunc count=1 

# 写loader区，定位到磁盘第2个块，写1个块：512字节
dd if=loader.bin of=disk1.vhd bs=512 conv=notrunc seek=1

# 写kernel区，定位到磁盘第100个块
dd if=kernel.elf of=disk1.vhd bs=512 conv=notrunc seek=100

# 写应用程序init，临时使用
# dd if=init.elf of=disk1.vhd bs=512 conv=notrunc seek=5000
# dd if=shell.elf of=disk1.vhd bs=512 conv=notrunc seek=5000

# 写应用程序，使用系统的挂载命令
export DISK_NAME=disk2.vhd
export TARGET_PATH=$HOME/k
rm $TARGET_PATH
hdiutil attach $DISK_NAME -mountpoint $TARGET_PATH -verbose
cp -v init.elf $TARGET_PATH
cp -v shell.elf $TARGET_PATH
cp -v loop $TARGET_PATH
hdiutil unmount $TARGET_PATH -verbose
