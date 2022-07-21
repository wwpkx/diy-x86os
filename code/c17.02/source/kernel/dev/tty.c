/**
 * 终端tty
 * 目前只考虑处理cooked模式的处理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "dev/tty.h"
#include "dev/console.h"
#include "dev/kbd.h"
#include "dev/dev.h"

/**
 * @brief 打开tty设备
 */
int tty_open (device_t * dev)  {
	return 0;
}

/**
 * @brief 从tty读取数据
 */
int tty_read (device_t * dev, int addr, char * buf, int size) {
	return size;
}

/**
 * @brief 向tty写入数据
 */
int tty_write (device_t * dev, int addr, char * buf, int size) {
	return size;
}

/**
 * @brief 向tty设备发送命令
 */
int tty_control (device_t * dev, int cmd, int arg0, int arg1) {
	
}

/**
 * @brief 关闭tty设备
 */
void tty_close (device_t * dev) {

}

// 设备描述表: 描述一个设备所具备的特性
dev_desc_t dev_tty_desc = {
	.name = "tty",
	.major = DEV_TTY,
	.open = tty_open,
	.read = tty_read,
	.write = tty_write,
	.control = tty_control,
	.close = tty_close,
};