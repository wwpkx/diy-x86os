/**
 * 基础显示部件
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <ui/ui_color.h>
#include <ui/ui_device.h>
#include <ui/ui_core.h>
#include <core/klib.h>
#include <ui/widget.h>

// 缺省的颜色
#define WIDGET_BACKGROUND       		COLOR_Gainsboro
#define WIDGET_FOREGROUND       		COLOR_Red
#define WIDGET_DEACTIVE_BOARDER      	COLOR_Black
#define WIDGET_ACTIVE_BOARDER			COLOR_LightBlue

/**
 * 缺省绘制函数
 */
void painter_widget_default(struct _widget_t *widget) {
	ui_color_t boarder_color = widget->active ? WIDGET_ACTIVE_BOARDER : WIDGET_DEACTIVE_BOARDER;

    // 上边框
    widget_draw_rect(widget,
                 0,
                 0,
                 widget_width(widget),
                 widget->border_size,
                 boarder_color);
    // 下边框
    widget_draw_rect(widget,
                 0,
				 widget_height(widget) - widget->border_size,
				 widget_width(widget),
                 widget->border_size,
                 boarder_color);
    // 左边框
    widget_draw_rect(widget,
                 0,
                 0,
                 widget->border_size,
				 widget_height(widget),
                 boarder_color);
    // 右边框
    widget_draw_rect(widget,
    			 widget_width(widget) - widget->border_size,
                 0,
                 widget->border_size,
				 widget_height(widget),
                 boarder_color);

    // 然后是内容区，即实际内容区，加上padding区
    widget_draw_rect(widget,
                 widget->border_size,
                 widget->border_size,
				 widget_width(widget) - 2 * widget->border_size,
				 widget_height(widget) - 2 * widget->border_size,
                 widget->backgroud_color);
}

/**
 * 鼠标事件处理
 */
int widget_mevent_handler (struct _widget_t * widget, mouse_event_t * event) {
    switch (event->type) {
        case MOUSE_EVENT_DRAG:
        	// 拖动时，移动部件
        	if (!event->dx && !event->dy) {
        		return 0;
        	}

            widget_move_to(widget, widget_x(widget) + event->dx, widget_y(widget) + event->dy);
            break;
        case MOUSE_EVENT_PRESS:
        	// 有点击，直接选中
            widget_set_active(widget, 1);
            break;
        default:
        	break;
    }

    return 1;
}

/**
 * 初始化部件
 */
void widget_init(widget_t *widget, widget_t *parent, int width, int height, task_t * owner) {
    list_init(&widget->child_list);

    widget->type = WIDGET_TYPE_WIDGET;
    widget->visiable = parent ? UI_VISIABLE : UI_HIDDEN;		// 缺省不显示
    widget->active = 0;
    widget->device = ui_screen_device();  	// 缺省值

    // 默认居中显示, 屏幕中间或者父窗口中间
    if (parent == (widget_t *) 0) {
    	rect_init(&widget->rect,
    			(ui_device_width(widget->device) - 1 - width) / 2,
    			(ui_device_height(widget->device) - 1 - height) / 2,
				width, height);
    } else {
    	rect_init(&widget->rect,
    			(widget_width(parent) - 1 - width) / 2,
				(widget_height(parent) - 1 - height) / 2,
				width, height);
    }

    // 颜色与边框
    widget->backgroud_color = WIDGET_BACKGROUND;
    widget->foreground_color = WIDGET_FOREGROUND;
    widget->boarder_color = WIDGET_DEACTIVE_BOARDER;
    widget->draw_text_background = 0;

    widget->padding_size = 0;
    widget->border_size = 0;

    widget->task = owner;

    // 相关回调函数
    widget->paint = painter_widget_default;
    widget->mevent_hander = widget_mevent_handler;
    widget->kevent_handler = (key_event_handler_t)0;

    // 顶层widget没有父对像, 插入最顶层队列
    if (widget != ui_screen_widget()) {
        // 有父窗口列表，则插入其中。没有，插入顶层窗口中
        if (parent == (widget_t *) 0){
            parent = ui_root_widget();
        }

        widget->parent = parent;

        // 后插入的先显示，同时指示脏，需要刷新
        list_insert_last(&widget->parent->child_list, &widget->node);

        rect_t dirty_rect;
        widget_content_rect(widget, &dirty_rect);
        device_add_dirty(widget->device, widget,  &dirty_rect);
    }
}

/**
 * 移动部件
 */
void widget_move_to(widget_t *widget, int x, int y) {

    // 这里的x, y并非真正相对父起始，而是相对内容区而言
    int margin = widget->parent->border_size + widget->parent->padding_size;
    x += margin;
    y += margin;

    // 相同位置，不调整
    if ((x == widget_x(widget)) && (y == widget_y(widget))) {
        return;
    }

    rect_t dirty_rect;
    widget_content_rect(widget, &dirty_rect);
    device_add_dirty(widget->parent->device, widget, &dirty_rect);

    // 不超过父区域
    widget->rect.x = k_min(x, widget->parent->rect.width);
    widget->rect.y = k_min(y, widget->parent->rect.height);

    // 这里要设置一下脏区域
    widget_content_rect(widget, &dirty_rect);
    device_add_dirty(widget->parent->device, widget, &dirty_rect);
}

/**
 * 添加子部件
 */
void widget_add_child(widget_t *parent, widget_t *child) {
	// 已经有父窗口了，则脱离原来的子链表
	if (child->parent) {
	    rect_t dirty_rect;
	    widget_content_rect(child, &dirty_rect);
	    device_add_dirty(child->parent->device, child, &dirty_rect);

	    list_remove(&child->parent->child_list, &child->node);
	}

    list_insert_first(&parent->child_list, &child->node);
    child->parent = parent;
    child->visiable = 1;

    // 在将新的父窗口中的区域置脏，刷新显示
    rect_setx(&child->rect, 0);
    rect_sety(&child->rect, 0);
    device_add_dirty(parent->device, child, &child->rect);
}

/**
 * 设置部件为激活状态
 */
void widget_set_active (widget_t * widget, int active) {
	// 状态不变，直接不处理
	if (widget->active == active) {
		return;
	}

	rect_t dirty_rect;
    widget_content_rect(widget, &dirty_rect);

    widget_t * parent = widget->parent;
    if (active) {
        device_add_dirty(ui_screen_device(), widget, &dirty_rect);

    	// 激活状态：将结点移动队列尾部
        list_remove(&parent->child_list, &widget->node);
        list_insert_last(&parent->child_list, &widget->node);
    } else {
    	// 非激活状态: 不需要移动。其它某结点会自动移动
    	// 可能在同一队列，也可能是其它队列
        device_add_dirty(ui_screen_device(), widget, &dirty_rect);
    }
    widget->active = active;
}

/**
 * 设置为可见状态
 */
void widget_set_visiable(widget_t * widget, int visiable)  {
	// 显示不变
	if (visiable == widget->visiable) {
		return;
	}

	// 先添加脏区域，因为此次可能是将widget设置隐藏状态，这时device_add_dirty无法添加
	rect_t dirty_rect;
    widget_content_rect(widget, &dirty_rect);
    device_add_dirty(ui_screen_device(), widget, &dirty_rect);

    widget->visiable = visiable;
}

