#include <ui/ui_core.h>
#include <ui/ui_font.h>
#include <core/klib.h>
#include <ui/button.h>

static button_t button_buffer[20];
static list_t button_list;

static void painter_label_default(struct _widget_t *widget) {
	button_t *button = (button_t *) widget;

    // 先调用父类的绘图
    painter_widget_default(widget);

    // 写文字，不使用背景色，即透明的方式写
    widget_draw_text(widget, button->text_rect.x, button->text_rect.y, button->content, 0);
}

/**
 * 创建按钮
 */
button_t * button_create (const char * content, int width, int height, widget_t * parent, task_t * owner) {
    if (list_count(&button_list) == 0) {
    	list_init(&button_list);
		for (int i = 0; i < sizeof(button_buffer) / sizeof(button_t); i++) {
			list_insert_first(&button_list, (list_node_t *)(button_buffer + i));
		}
    }

    button_t * button = (button_t *)list_remove_first(&button_list);

    widget_init((widget_t *) button, parent, width, height, owner);
    k_strncpy(button->content, content, LABEL_CONTENT_MAX_LEN);
    button->widget.paint = painter_label_default;
    button->widget.type = WIDGET_TYPE_BUTTON;

    button_set_text(button, content);
    return button;
}

/**
 * 设置按钮的显示文字
 */
void button_set_text(button_t *button, const char *content) {
    k_strncpy(button->content, content, LABEL_CONTENT_MAX_LEN);

    // 计算此次需要显示的宽度和高度
    int content_width, content_height;
    ui_font_width_height(widget_width((widget_t *)button), ui_get_font(),
    		content, &content_width, &content_height);

    int margin_size = button->widget.border_size + button->widget.padding_size;
    button->text_rect.x = (widget_width((widget_t *)button) - content_width - margin_size) / 2;
    button->text_rect.y = (widget_height((widget_t *)button) - content_height - margin_size) / 2;
    button->text_rect.width = content_width;
    button->text_rect.height = content_height;

    // 取之前最长和现在的宽度，取最长的，以便将之前的清空并显示新的
    rect_t dirty_rect;
    widget_content_rect((widget_t *)button, &dirty_rect);
    device_add_dirty(ui_screen_device(), (widget_t *)button,  &dirty_rect);
}


