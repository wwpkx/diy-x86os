/**
 * UI核心配置
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef UI_H
#define UI_H

#include <ui/widget.h>
#include <ui/ui_device.h>

widget_t *ui_screen_widget(void);
widget_t *ui_root_widget(void);

void ui_init (boot_info_t * boot_info);
void ui_refresh (void);
int ui_screen_width (void);
int ui_screen_height (void);
ui_device_t * ui_screen_device(void);
void ui_device_flush(ui_device_t *device, widget_t *owner);
void device_add_dirty(ui_device_t *device, widget_t * widget, rect_t *rect);

void widget_draw_rect(widget_t *widget, int x, int y, int width, int height, ui_color_t color);
void widget_draw_line(widget_t * widget, int start_x, int start_y, int end_x, int end_y, ui_color_t color);
void widget_draw_point(widget_t *widget, int x, int y, ui_color_t color);
void widget_draw_text(widget_t *widget, int x, int y, const char *string, int size);

queue_t * ui_touch_queue (void);
void ui_send_mouse_event (mouse_event_t * event);
void ui_send_key_event (key_event_t * event);

#endif //OS_UI_H
