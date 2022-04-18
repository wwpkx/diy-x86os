/**
 * 带关闭按钮的主窗口
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <core/os_cfg.h>
#include <core/klib.h>
#include <ui/ui_core.h>
#include <ui/ui_font.h>
#include <ui/window.h>

static window_t window_buf[20];
static list_t window_list;

/**
 * 主窗口绘制
 */
static void painter_window_default(struct _widget_t *widget) {
    window_t *window = (window_t *) widget;

    // 先调用父类的绘制
    painter_widget_default(widget);

    // 计算标题栏的宽度，高度不考虑
    int max_width = widget_width(widget) - 2 * (widget->border_size + widget->padding_size);
    int title_width, title_height;
    ui_font_width_height(max_width, ui_get_font(), window->title, &title_width, &title_height);
    title_width = k_min(title_width, max_width);		// 显示可能超出，取最小值
    title_height = k_min(title_height, WINDOW_TITLE_BAR_HEIGHT);

    // 填充标题栏背景区
    int margin = widget->border_size + widget->padding_size;
    widget_draw_rect(widget,
                 margin,
                 margin,
				 max_width,
                 WINDOW_TITLE_BAR_HEIGHT,
                 widget->active ? TITLE_BAR_ACTIVE_COLOR : TITLE_BAR_DEACTIVE_COLOR);

    // 文字居中显示, 不使用背景色写
    int title_x = (max_width - title_width) / 2;
    int title_y = margin + (WINDOW_TITLE_BAR_HEIGHT - title_height) / 2;
    widget_draw_text(widget, title_x, title_y, window->title, 0);
}

/**
 * 鼠标事件处理函数
 */
static int mevent_handler(struct _widget_t *widget, mouse_event_t *event) {
	// 先调用父类的缺省处理函数
	widget_mevent_handler(widget, event);

    switch (event->type) {
        case MOUSE_EVENT_PRESS: {
             break;
        }
        default:
			break;
    }

    return 0;
}

/**
 * 创建一个主窗口
 */
window_t * window_create(const char *title, int width, int height, widget_t *parent, task_t * owner) {
	if (list_count(&window_list) == 0) {
		list_init(&window_list);

		for (int i = 0; i < sizeof(window_buf) / sizeof(window_t); i++) {
			list_insert_first(&window_list, (list_node_t *)(window_buf + i));
		}
	}

	window_t * window = (window_t *)list_remove_first(&window_list);
    widget_init((widget_t *) window, parent, width, height, owner);
    k_strncpy(window->title, title, WINDOW_TITLE_SIZE);

    window->widget.type = WIDGET_TYPE_WINDOW;
    window->widget.foreground_color = COLOR_White;
    window->widget.paint = painter_window_default;
    window->widget.mevent_hander = mevent_handler;
    window->widget.border_size = WINDOW_DEFAULT_BOARDER_SIZE;
    window->widget.padding_size = WINDOW_DEFAULT_PADDING_SIZE;

    // 创建标题栏下方的容器
    int margin = window->widget.border_size + window->widget.padding_size;
    widget_init((widget_t *) &window->content_widget,
                (widget_t *) window,
                widget_width((widget_t *)window) - 2 * margin,
				widget_height((widget_t *)window) - WINDOW_TITLE_BAR_HEIGHT - 2 * margin,
				owner);
    window->widget.backgroud_color = COLOR_White;
    window->content_widget.boarder_color = COLOR_Orange;
    window->content_widget.border_size = WINDOW_CONTENT_BOARDER_SIZE;

    // 移动于相应的区域
    widget_move_to(&window->content_widget, 0, 0 + WINDOW_TITLE_BAR_HEIGHT);
    return window;
}

/**
 * 在内容区中添加一个部件
 */
void window_add_widget(window_t *window, widget_t *widget) {
    if (widget->parent) {
        rect_t dirty_rect;
        widget_content_rect(widget, &dirty_rect);
        device_add_dirty((ui_device_t *)ui_screen_device(), widget, &dirty_rect);

        list_remove(&widget->parent->child_list, &widget->node);
   }

    widget_add_child(&window->content_widget, widget);
    widget_move_to(widget, 0, 0);
}


