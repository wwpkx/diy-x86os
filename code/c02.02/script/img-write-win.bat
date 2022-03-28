rem 写boot区，定位到磁盘开头，写1个块：512字节
dd if=boot.bin of=os.vhd bs=512 conv=notrunc count=1 


