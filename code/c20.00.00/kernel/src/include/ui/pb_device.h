/**
 * 虚拟绘图设备
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef SRC_INCLUDE_UI_PB_DEVICE_H_
#define SRC_INCLUDE_UI_PB_DEVICE_H_

#include <ui/ui_device.h>
#include <ui/ui_screen.h>

/**
 * 绘图设备
 */
typedef struct _pb_device_t {
	ui_device_t base;
}pb_device_t;

void pb_device_init (pb_device_t * wdevice, ui_device_t * low_device,
		uint8_t * buffer, int width, int height, int bpp);
void pb_device_scroll_up(ui_device_t * device, int dy);

#endif /* SRC_INCLUDE_UI_PB_DEVICE_H_ */
