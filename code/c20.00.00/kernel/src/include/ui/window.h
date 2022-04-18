/**
 * 带关闭按钮的主窗口
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef UI_WIN_H
#define UI_WIN_H

#include <ui/widget.h>
#include <core/os_cfg.h>

/**
 * 主窗口部件
 */
typedef struct _window_t {
    widget_t widget;
    char title[WINDOW_TITLE_SIZE];			// 标题栏
    widget_t content_widget;				// 内容区显示部件
}window_t;

window_t * window_create(const char *title, int width, int height, widget_t *parent, task_t * owner);
void window_add_widget(window_t * window, widget_t * widget);

#endif //UI_WIN_H
