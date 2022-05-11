/*
 * dev.h
 *
 *  Created on: 2021年8月25日
 *      Author: mac
 */

#ifndef DEV_H
#define DEV_H

/**
 * 主设备号
 */
typedef enum {
	DEV_DISK = 0,
	DEV_TTY,
}dev_major_t;

/**
 * 组合创建设备号
 */
static inline int make_device_num(dev_major_t major, int minor) {
	return (major << 16) | (minor << 0);
}

/**
 * 获取主设备号
 */
static inline dev_major_t device_major (int device) {
	return (dev_major_t)(device >> 16);
}

/**
 * 获取从设备号
 */
static inline int device_minor (int device) {
	return device & 0xFF;
}

#endif /* DEV_H */
