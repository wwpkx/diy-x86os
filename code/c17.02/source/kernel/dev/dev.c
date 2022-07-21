#include "dev/dev.h"
#include "dev/tty.h"

extern dev_desc_t dev_tty_desc;

/**
 * @brief 打开指定的设备
 */
int dev_open (int major, int minor, void * data) {
    return -1;
}

/**
 * @brief 读取指定字节的数据
 */
int dev_read (int dev_id, int addr, char * buf, int size) {
    return size;
}

/**
 * @brief 写指定字节的数据
 */
int dev_write (int dev_id, int addr, char * buf, int size) {
    return size;
}

/**
 * @brief 发送控制命令
 */

int dev_control (int dev_id, int cmd, int arg0, int arg1) {
    return 0;
}

/**
 * @brief 关闭设备
 */
void dev_close (int dev_id) {
}


