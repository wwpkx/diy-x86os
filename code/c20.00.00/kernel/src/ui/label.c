/**
 * 标签实现
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <core/klib.h>
#include <ui/ui_core.h>
#include <ui/ui_font.h>
#include <ui/label.h>

static label_t label_buffer[120];
static list_t label_list;

/**
 * 标签绘制函数
 */
static void painter_label_default(struct _widget_t *widget) {
    label_t *label = (label_t *) widget;

    // 先调用父类的绘图
    painter_widget_default(widget);

    // 写文字，不使用背景色，即透明的方式写
    int margin_size = label->widget.border_size + label->widget.padding_size;
    widget_draw_text(widget, margin_size, margin_size, label->content, 0);
}

/**
 * 创建一个标签
 */
label_t * label_create (const char * content, int width, int height, widget_t * parent, task_t * owner) {
    if (list_count(&label_list) == 0) {
    	list_init(&label_list);
		for (int i = 0; i < sizeof(label_buffer) / sizeof(label_t); i++) {
			list_insert_first(&label_list, (list_node_t *)(label_buffer + i));
		}
    }

    // 分配标签内存
	label_t * label = (label_t *)list_remove_first(&label_list);

	// 初始化
    widget_init((widget_t *) label, parent, width, height, owner);
    k_strncpy(label->content, content, LABEL_CONTENT_MAX_LEN);
    label->widget.paint = painter_label_default;
    label->widget.type = WIDGET_TYPE_LABEL;

    // 记录初始显示的最大区域
    ui_font_width_height(width, ui_get_font(), content, &width, &height);
    label->last_width = k_min(width, widget_width((widget_t *)label));
    label->last_height = k_min(height, widget_height((widget_t *)label));
    return label;
}

/**
 * 设置标签
 */
void label_set_text(label_t *label, const char *content) {
    k_strncpy(label->content, content, LABEL_CONTENT_MAX_LEN);

    // 计算此次需要显示的宽度和高度
    int content_width, content_height;
    ui_font_width_height(widget_width((widget_t *)label), ui_get_font(),
    		content, &content_width, &content_height);

    // 取之前最长和现在的宽度，取最长的，以便将之前的清空并显示新的
    rect_t dirty_rect;
    rect_init(&dirty_rect, 0, 0, 0, 0);
    dirty_rect.width = k_max(label->last_width, content_width);
    dirty_rect.height = k_max(label->last_height, content_height);
    device_add_dirty(ui_screen_device(), (widget_t *)label, &dirty_rect);

    label->last_height = content_height;
}


