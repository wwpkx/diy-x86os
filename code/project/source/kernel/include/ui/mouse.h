/**
 * 鼠标实现
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef UI_MOUSE_H
#define UI_MOUSE_H

#include <ui/widget.h>

/**
 * 鼠标部件
 */
typedef struct _mouse_t {
    widget_t widget;
}mouse_t;

void ui_mouse_init(void);
void ui_mouse_move_by(int dx, int dy);
int ui_mouse_getx(void);
int ui_mouse_gety(void);
widget_t * ui_mouse_widget (void);

#endif //UI_MOUSE_H
