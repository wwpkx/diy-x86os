set DISK1_NAME=disk1.vhd

@REM дboot������λ�����̿�ͷ��д1���飺512�ֽ�
dd if=boot.bin of=%DISK1_NAME% bs=512 conv=notrunc count=1

@REM дloader������λ�����̵�2���飬д1���飺512�ֽ�
dd if=loader.bin of=%DISK1_NAME% bs=512 conv=notrunc seek=1

@REM дkernel������λ�����̵�100����
dd if=kernel.elf of=%DISK1_NAME% bs=512 conv=notrunc seek=100

@REM  дӦ�ó���init����ʱʹ��
@REM dd if=init.elf of=%DISK1_NAME% bs=512 conv=notrunc seek=5000
@dd if=shell.elf of=%DISK1_NAME% bs=512 conv=notrunc seek=5000

@REM дӦ�ó���ʹ��ϵͳ�Ĺ�������
set DISK2_NAME=disk2.vhd
set TARGET_PATH=k
echo select vdisk file="%cd%\%DISK2_NAME%" >a.txt
echo attach vdisk >>a.txt
echo select partition 1 >> a.txt
echo assign letter=%TARGET_PATH% >> a.txt
diskpart /s a.txt
del a.txt

@REM ����Ӧ�ó���
@copy /Y init.elf %TARGET_PATH%:\init
copy /Y shell.elf %TARGET_PATH%:\shell.elf
@copy /Y loop %TARGET_PATH%:\loop

echo select vdisk file="%cd%\%DISK2_NAME%" >a.txt
echo detach vdisk >>a.txt
diskpart /s a.txt
del a.txt
