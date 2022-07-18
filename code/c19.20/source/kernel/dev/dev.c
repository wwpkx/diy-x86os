#include "cpu/irq.h"
#include "dev/dev.h"
#include "dev/tty.h"

extern dev_driver_t tty_driver;

static dev_desc_t dev_desc_tbl[] = {
    {
        .name = "tty0",
        .major = DEV_TTY,
        .minor = 0,
        .mode = 0,
        .driver = &tty_driver,
        .data = 0,
    },
    {
        .name = "tty1",
        .major = DEV_TTY,
        .minor = 1,
        .mode = 0,
        .driver = &tty_driver,
        .data = 0,
    },
    {
        .name = "tty2",
        .major = DEV_TTY,
        .minor = 2,
        .mode = 0,
        .driver = &tty_driver,
        .data = 0,
    },
    {
        .name = "tty3",
        .major = DEV_TTY,
        .minor = 3,
        .mode = 0,
        .driver = &tty_driver,
        .data = 0,
    }
};

static int is_devid_bad (int dev_id) {
    if ((dev_id < 0) || (dev_id >=  sizeof(dev_desc_tbl) / sizeof(dev_desc_t))) {
        return 1;
    }

    if (dev_desc_tbl[dev_id].driver == (dev_driver_t *)0) {
        return 1;
    }

    return 0;
}

/**
 * @brief 打开指定的设备
 */
int dev_open (int major, int minor) {
    // 遍历找设备号相同的器件
    for (int i = 0; i < sizeof(dev_desc_tbl) / sizeof(dev_desc_t); i++) {
        dev_desc_t * d = dev_desc_tbl + i;
        if ((d->major == major) && (d->minor == minor)) {
            if (d->open_count) {
                return i;
            }

            int err = d->driver->open(d);
            if (err < 0) {
                return -1;
            }

            d->open_count++;
            return i;
        }
    }
}

/**
 * @brief 读取指定字节的数据
 */
int dev_read (int dev_id, char * buf, int size) {
    if (is_devid_bad(dev_id)) {
        return -1;
    }

    dev_desc_t * desc = dev_desc_tbl + dev_id;
    return desc->driver->read(desc, buf, size);
}

/**
 * @brief 写指定字节的数据
 */
int dev_write (int dev_id, char * buf, int size) {
    if (is_devid_bad(dev_id)) {
        return -1;
    }

    dev_desc_t * desc = dev_desc_tbl + dev_id;
    return desc->driver->write(desc, buf, size);
}

/**
 * @brief 发送控制命令
 */

int dev_control (int dev_id, int cmd, int arg0, int arg1) {
    if (is_devid_bad(dev_id)) {
        return -1;
    }

    dev_desc_t * desc = dev_desc_tbl + dev_id;
    return desc->driver->control(desc, cmd, arg0, arg1);
}

/**
 * @brief 关闭设备
 */
void dev_close (int dev_id) {
    if (is_devid_bad(dev_id)) {
        return;
    }

    dev_desc_t * desc = dev_desc_tbl + dev_id;
    if (--desc->open_count == 0) {
        return desc->driver->close(desc);
    }
}