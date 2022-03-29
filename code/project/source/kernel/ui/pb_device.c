/**
 * 虚拟绘图设备
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <ui/pb_device.h>

/**
 * 初始化
 */
static void init (struct _ui_device_t * device) {

}

/**
 * 画点
 */
static void draw_point(struct _ui_device_t * device, uint16_t x, uint16_t y, ui_color_t color) {
	uint32_t *to = (uint32_t*)ui_device_buffer(device, x, y);
    *to = color;
}

/**
 * 矩形填充
 */
static void fill_rect(struct _ui_device_t * device, rect_t *rect, ui_color_t color) {
    uint8_t *write_start = ui_device_buffer(device, rect->x, rect->y);
    for (int i = 0; i < rect->height; i++) {
        uint32_t * to = (uint32_t *)write_start;

        for (int j = 0; j < rect->width; j++) {
    		*to++ = color;
    	}
        write_start += device->pitch;
    }
}

/**
 * 画横线
 */
static void draw_hline (struct _ui_device_t * device, int start_x, int y, int end_x, ui_color_t color) {
	uint32_t *to = (uint32_t *) ui_device_buffer(device, start_x, y);

	for (int x = start_x; x < end_x; x++) {
	    *to++ = color;
	}
}

/**
 * 画竖线
 */
static void draw_vline (struct _ui_device_t * device, int x, int start_y, int end_y, ui_color_t color) {
	uint32_t *to = (uint32_t *) ui_device_buffer(device, x, start_y);;

	for (int y = start_y; y < end_y; y++) {
	    *to = color;
	    to = (uint32_t *)((uint8_t *)to + device->pitch);
	}
}

/**
 * 将缓存中的像互数据写入至下层的设备
 */
static void flush(struct _ui_device_t * device, int device_x, int device_y) {
	ui_device_t * low_device = device->low_device;

	rect_t * dirty_rect = &device->dirty_rect;
	uint32_t *read_start = ui_device_buffer(device, dirty_rect->x,dirty_rect->y);

	// 要将rect转换为屏中的绝对位置区域
	uint32_t *write_start = (uint32_t *)(low_device->front_buffer
			+ (device_y + dirty_rect->y) * low_device->pitch
			+ (device_x + dirty_rect->x) * low_device->pixel_width);

    for (int y = 0; y < dirty_rect->height; y++) {
	   uint32_t * src = read_start, * dest = (uint32_t *)write_start;
		for (int x = 0; x < dirty_rect->width; x++) {
			*dest++ = *src++;
		}

	   read_start += device->pitch / 4;
	   write_start += low_device->pitch / 4;
    }
}

/**
 * 虚拟设备驱动
 */
ui_device_driver_t wdevice_drvier = {
		.init = init,
		.draw_point = draw_point,
		.fill_rect = fill_rect,
		.draw_hline = draw_hline,
		.draw_vline = draw_vline,
		.flush = flush,
};

/**
 * 虚拟设备初始化
 */
void pb_device_init (pb_device_t * pb_device, ui_device_t * low_device,
		uint8_t * buffer, int width, int height, int bpp) {
	ui_device_t * device = (ui_device_t *)pb_device;

	device->width = width;
    device->height = height;
    device->bpp = bpp;
    device->pixel_width = bpp;
    device->pitch =device->width *device->pixel_width;
    rect_init(&pb_device->base.dirty_rect, 0, 0,device->width,device->height);

    // 显存与缓存设置
	device->front_buffer = buffer;
	device->driver = wdevice_drvier;
	device->low_device = low_device;
	device->view_x = device->view_y = 0;
}

/**
 * 整体屏幕上移
 */
void pb_device_scroll_up(ui_device_t * device, int dy) {
	uint32_t * dest = (uint32_t *) ui_device_buffer(device, 0, 0);
	uint32_t * src = (uint32_t *) ui_device_buffer(device, 0, dy);
	for (int i = 0; i < device->width; i++) {
		for (int j = 0; j < device->height; j++) {
			*dest++ = *src++;
		}
	}
}

