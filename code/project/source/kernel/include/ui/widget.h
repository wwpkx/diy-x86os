/**
 * 基础显示部件
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <core/boot_info.h>
#include <core/list.h>
#include <core/types.h>
#include <core/task.h>
#include <ui/rect.h>
#include <ui/ui_color.h>
#include <ui/ui_device.h>
#include <ui/ui_event.h>

/**
 * 部件类型
 */
typedef enum _widget_type_t {
    WIDGET_TYPE_WIDGET,/**< WIDGET_TYPE_WIDGET */
    WIDGET_TYPE_LABEL, /**< WIDGET_TYPE_LABEL */
	WIDGET_TYPE_BUTTON,   /**< WIDGET_TYPE_BUTTON */
    WIDGET_TYPE_MOUSE, /**< WIDGET_TYPE_MOUSE */
    WIDGET_TYPE_WINDOW,/**< WIDGET_TYPE_WINDOW */
    WIDGET_TYPE_EDITOR,/**< WIDGET_TYPE_EDITOR */
	WIDGET_TYPE_PIXEL_WIN,/**< WIDGET_TYPE_PIXEL_WIN */
}widget_type_t;

/**
 * 可见性
 */
typedef enum _ui_visiable_t {
	UI_HIDDEN = 0,  /**< UI_HIDDEN */
	UI_VISIABLE = 1,/**< UI_VISIABLE */
}ui_visiable_t;

/**
 * 显示部件
 */
typedef struct _widget_t {
    list_node_t node;

    widget_type_t type;

    ui_visiable_t visiable;
    int active;
    
    rect_t rect;
    ui_color_t backgroud_color;
    ui_color_t foreground_color;
    ui_color_t boarder_color;
    int draw_text_background;

    mouse_event_handler_t mevent_hander;
    key_event_handler_t  kevent_handler;

    // 所在的绘图设备
    ui_device_t * device;
    void (*paint)(struct _widget_t * widget);

    struct _widget_t * parent;
    list_t child_list;

    int padding_size, border_size;
    task_t * task;
}widget_t;

static inline int widget_x(widget_t * widget) {
    return rect_x(&widget->rect);
}

static inline int widget_y(widget_t * widget) {
    return rect_y(&widget->rect);
}

static inline int widget_width(widget_t * widget) {
    return rect_width(&widget->rect);
}

static inline int widget_height(widget_t * widget) {
    return rect_height(&widget->rect);
}

static inline int widget_content_width(widget_t * widget) {
	return widget_width(widget) - 2 * (widget->padding_size + widget->border_size);
}

static inline int widget_content_height(widget_t * widget) {
	return widget_height(widget) - 2 * (widget->padding_size + widget->border_size);
}

static inline ui_color_t widget_foreground (widget_t *widget) {
	return widget->foreground_color;
}

static inline ui_color_t widget_background (widget_t *widget) {
	return widget->backgroud_color;
}

static inline void widget_set_foreground (widget_t *widget, ui_color_t color) {
	widget->foreground_color = color;
}

static inline void widget_set_background (widget_t *widget, ui_color_t color) {
	widget->backgroud_color = color;
}

static inline void widget_content_rect(widget_t * widget, rect_t * rect) {
	rect_init(rect, 0, 0, widget_width(widget), widget_height(widget));
}

static inline ui_device_t * widget_device (widget_t * widget) {
	return widget->device;
}

static inline widget_t * widget_parent (widget_t * widget) {
	return widget->parent;
}

static inline void widget_set_use_text_background (widget_t * widget, int use) {
	widget->draw_text_background = use;
}

void painter_widget_default(struct _widget_t *widget);
int widget_mevent_handler (struct _widget_t * widget, mouse_event_t * event);

void widget_init (widget_t * widget, widget_t * parent, int width, int height, task_t * owner);
void widget_move_to (widget_t * widget, int x, int y);
void widget_add_child(widget_t *parent, widget_t *child);
void widget_set_active (widget_t * widget, int active);
void widget_set_visiable(widget_t * widget, int visiable);

#endif //UI_WIDGET_H
