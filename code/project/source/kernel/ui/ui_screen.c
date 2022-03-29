/**
 * 物理屏幕绘图设备
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <ui/ui_screen.h>
#include <core/os_cfg.h>

/**
 * 32位绘图初始化
 */
static void init32 (struct _ui_device_t * device) {

}

/**
 * 画点
 */
static void draw_point32(struct _ui_device_t * device, uint16_t x, uint16_t y, ui_color_t color) {
	*(uint32_t *)ui_device_buffer(device, x, y) = color;
}

/**
 * 矩形填充
 */
static void fill_rect32(struct _ui_device_t * device, rect_t *rect, ui_color_t color) {
    uint32_t *write_start = ui_device_buffer(device, rect->x, rect->y);

    int addr_inc = device->pitch / 4;
    for (int i = 0; i < rect->height; i++) {
        uint32_t * to = write_start;
        for (int j = 0; j < rect->width; j++) {
    		*to++ = color;
    	}
        write_start += addr_inc;
    }
}

/**
 * 画横线
 */
static void draw_hline32 (struct _ui_device_t * device, int start_x, int y, int end_x, ui_color_t color) {
	uint32_t *v_ram = (uint32_t *) ui_device_buffer(device, start_x, y);
	for (int x = start_x; x < end_x; x++) {
	    *v_ram++ = color;
	}
}

/**
 * 画竖线
 */
static void draw_vline32 (struct _ui_device_t * device, int x, int start_y, int end_y, ui_color_t color) {
	uint32_t *v_ram = (uint32_t *) ui_device_buffer(device, x, start_y);;

	for (int y = start_y; y < end_y; y++) {
	    *v_ram = color;
	    v_ram = (uint32_t *)((uint8_t *)v_ram + device->pitch / 4);
	}
}

/**
 * 刷新，将前端缓存值全部写到最终的显存中
 */
static void flush32(struct _ui_device_t * device, int device_x, int device_y) {
	ui_screen_t * screen = (ui_screen_t *)device;

    // 不使用双缓存，直接就不需要再刷了, 调试用
    if (device->front_buffer == device->backend_buffer) {
        return;
    }

    rect_t * rect_in_device = &device->dirty_rect;
    uint32_t *read_start = (uint32_t *)ui_device_buffer(device, rect_in_device->x,rect_in_device->y);
    uint32_t *write_start = (uint32_t *)screen_frame_buffer(screen, rect_in_device->x + device_x, rect_in_device->y + device_y);

    int add_inc = device->pitch / 4;
    for (int y = 0; y < rect_in_device->height; y++) {
    	uint32_t * src = read_start, * dest = write_start;
    	for (int x = 0; x < rect_in_device->width; x++) {
            *dest++ = *src++;
    	}

        read_start += add_inc;
        write_start += add_inc;
    }
}

// 针对颜色为32位宽的驱动
ui_device_driver_t screen_drvier_bpp32 = {
		.init = init32,
		.draw_point = draw_point32,
		.fill_rect = fill_rect32,
		.draw_hline = draw_hline32,
		.draw_vline = draw_vline32,
		.flush = flush32,
};

// 暂用1M以上的空间当屏幕缓存
static uint8_t screen_buffer[INIT_SCREEN_HEIGHT * INIT_SCREEN_WIDTH * INIT_SCREEN_BPP];

/**
 * 屏幕设备初始化
 */
void screen_init (ui_screen_t * screen, boot_info_t * boot_info) {
	ui_device_t * device = (ui_device_t *)screen;

    device->width = boot_info->screen_width;
    device->height = boot_info->screen_height;
    device->bpp = boot_info->screen_vmode;
    device->pixel_width = boot_info->screen_vmode / 8;
    device->pitch = device->width * device->pixel_width;
    rect_init(&device->dirty_rect, 0, 0, device->width, device->height);
    device->front_buffer = screen_buffer;		// 使用双缓存

    // 显存与缓存设置
    device->backend_buffer = (void *)boot_info->screen_vram;
    device->driver = screen_drvier_bpp32;
#if !USE_DOUBLE_BUFFER
    device->front_buffer = device->backend_buffer;  // 直接写显存，调试用
#endif
}
