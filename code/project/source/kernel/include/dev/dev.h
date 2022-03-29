/*
 * dev.h
 *
 *  Created on: 2021年8月25日
 *      Author: mac
 */

#ifndef SRC_INCLUDE_DEV_DEV_H_
#define SRC_INCLUDE_DEV_DEV_H_

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
static inline int make_device_num(dev_major_t major, int minor, int sub) {
	return (major << 16) | (minor << 8) | sub;
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
static inline int dev_minor (int device) {
	return (device >> 8) & 0xF;
}

/**
 * 获取子设备号
 */
static inline int dev_sub (int device) {
	return device & 0xF;
}


#endif /* SRC_INCLUDE_DEV_DEV_H_ */
