/**
 * UI部分的系统调用头文件
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef LIB_UI_H
#define LIB_UI_H

#include <ui/ui_color.h>
#include <dev/keyboard.h>

typedef void * ui_widget_t;

int ui_widget_width(ui_widget_t win);
int ui_widget_height(ui_widget_t win);
int ui_widget_content_width(ui_widget_t win);
int ui_widget_content_height(ui_widget_t win);
void ui_widget_move_to(ui_widget_t win, int x, int y);
void ui_widget_set_visiable(ui_widget_t win, int visiable);

ui_widget_t ui_window_create(const char *title, int width, int height, ui_widget_t parent);
ui_widget_t ui_window_container(ui_widget_t win);
void ui_window_add(ui_widget_t window, ui_widget_t sub);

ui_widget_t ui_label_create (const char *content, int width, int height, ui_widget_t parent);
void ui_label_set (ui_widget_t handler, const char *content);

ui_widget_t ui_button_create (const char *content, int width, int height, ui_widget_t parent);
void ui_button_set (ui_widget_t button, const char *content);

ui_widget_t ui_pixel_win_create(int width, int height, ui_widget_t paren);
void ui_pixel_win_draw_point(ui_widget_t win, int x, int y, ui_color_t color);
void ui_pixel_win_draw_rect(ui_widget_t win, int x, int y, int width, int height, ui_color_t color);
void ui_pixel_win_draw_line(ui_widget_t win, int start_x, int start_y, int end_x, int end_y, ui_color_t color);
void ui_pixel_win_draw_string(ui_widget_t win, int x, int y, ui_color_t color, const char * string);

ui_widget_t ui_tty_widget_create(int width, int height, ui_widget_t parent);
int ui_tty_widget_cols(ui_widget_t widget);

typedef void * ui_timer_t;
typedef void (*ui_timer_handler_t)(ui_timer_t timer, void * param);
ui_timer_t ui_timer_create(int interval_ms, int once, ui_timer_handler_t * handler, void * param);
void ui_timer_stop(ui_timer_t timer);

typedef int (*timer_callback_t)(int timer, int data);
typedef int (*key_callback_t)(ui_widget_t win, key_data_t key);

void ui_set_timer_callback (timer_callback_t cb);
void ui_set_key_callback (key_callback_t cb);
void ui_loop_event (void);

#endif //LIB_UI_H
