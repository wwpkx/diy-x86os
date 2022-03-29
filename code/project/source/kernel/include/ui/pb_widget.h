/**
 * 像素绘图部件
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef UI_PIXEL_WIN_H_
#define UI_PIXEL_WIN_H_

#include <ui/pb_device.h>
#include <ui/ui_color.h>
#include <ui/widget.h>

typedef struct _pb_widget_t {
	widget_t widget;

	// 临时使用
	pb_device_t wdevice;
#define FRAME_BUFFER_SIZE		(4*800*600)
	uint8_t frame_buffer[FRAME_BUFFER_SIZE];
}pb_widget_t;

void pb_widget_draw_rect(pb_widget_t * win, int x, int y, int width, int height, ui_color_t color);
void pb_widget_draw_text(pb_widget_t * widget, int x, int y, ui_color_t color, const char * string, int size);
void pb_widget_draw_point(pb_widget_t * win, int x, int y, ui_color_t color);
void pb_widget_draw_line(pb_widget_t * win, int start_x, int start_y, int end_x, int end_y, ui_color_t color);
void pb_widget_init (pb_widget_t * widget, int width, int height, widget_t * parent, task_t * owner);
pb_widget_t * pb_wdiget_create (int width, int height, widget_t * parent, task_t * owner);
void pb_widget_scroll_up(pb_widget_t * widget, int dy);

void pbwidget_request_draw_rect(pb_widget_t * win, int x, int y, int width, int height, ui_color_t color);
void pbwidget_request_draw_text(pb_widget_t * widget, int x, int y,
		ui_color_t color, const char * string, int size);
void pbwidget_request_scroll_up (pb_widget_t * widget, int dy);

#endif /* SRC_INCLUDE_UI_UI_PIXEL_WIN_H_ */
