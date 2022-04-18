/**
 * 基础绘图设备
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef OS_SCREEN_H
#define OS_SCREEN_H

#include <core/boot_info.h>
#include <ui/rect.h>
#include <ui/ui_color.h>

/**
 * 绘图驱动
 */
struct _ui_device_t;
typedef struct _ui_device_driver_t {
	void (*init) (struct _ui_device_t * device);
	void (*draw_point) (struct _ui_device_t * device, uint16_t x, uint16_t y, ui_color_t color);
	void (*fill_rect) (struct _ui_device_t * device, rect_t * rect, ui_color_t color);
	void (*draw_hline)(struct _ui_device_t * device, int start_x, int y, int end_x, ui_color_t color);
	void (*draw_vline) (struct _ui_device_t * device, int x, int start_y, int end_y, ui_color_t color);
	void (*flush) (struct _ui_device_t * device, int device_x, int device_y);
}ui_device_driver_t;

/**
 * 基础显示设备
 * 用于屏幕、UI绘图设置，自行管理大小、脏区域，以及是否开启双缓存等显示
 */
typedef struct _ui_device_t {
	int width, height;
	int pixel_width;				// 下移一个像素时需要增加的字节数
	int pitch;						// 下移一行时需要增加的字节数
	int bpp;						// 每个像素颜色的位宽
	int view_x, view_y;				// 起始显示的x和y
	rect_t dirty_rect;				// 脏区域
    uint8_t * front_buffer;			// 前端写缓存
	uint8_t * backend_buffer;		// 后端写显存
    ui_device_driver_t driver;		// 设备驱动
    struct _ui_device_t * low_device;		// 该设备下的底层设备
}ui_device_t;

static inline int ui_device_width(ui_device_t * device) {
	return device->width;
}

static inline int ui_device_height(ui_device_t * device) {
	return device->height;
}

static inline void* ui_device_buffer(struct _ui_device_t *device, uint16_t x, uint16_t y) {
	return (void *)(device->front_buffer + y * device->pitch + x * device->pixel_width);
}

void ui_device_scroll_up(ui_device_t * device, int dy);

#endif //OS_SCREEN_H
