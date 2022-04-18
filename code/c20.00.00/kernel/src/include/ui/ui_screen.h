/**
 * 物理屏幕绘图设备
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef SRC_INCLUDE_UI_UI_SCREEN_H_
#define SRC_INCLUDE_UI_UI_SCREEN_H_

#include <ui/ui_device.h>

/**
 * 屏幕设备
 */
typedef struct _ui_screen_t {
	ui_device_t base;
}ui_screen_t;

static inline uint32_t* screen_frame_buffer(ui_screen_t *screen, uint16_t x, uint16_t y) {
	return (uint32_t *)(screen->base.backend_buffer + y * screen->base.pitch + x * screen->base.pixel_width);
}

void screen_init (ui_screen_t * screen, boot_info_t * boot_info);


#endif /* SRC_INCLUDE_UI_UI_SCREEN_H_ */
