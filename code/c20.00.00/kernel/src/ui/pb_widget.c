/**
 * 像素绘图部件
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <core/klib.h>
#include <ui/pb_widget.h>
#include <ui/ui_core.h>
#include <ui/widget.h>
#include <ui/ui_font.h>

static pb_widget_t win_buffer[2];
static list_t win_list;

/**
 * 绘图函数
 */
static void painter_pwidget_default(struct _widget_t *widget) {
	pb_widget_t * pwidget = (pb_widget_t *)widget;

	// 缺省绘制
	//painter_widget_default(widget);

#if 0
	uint32_t * buf = (uint32_t *)pwidget->frame_buffer;
	for (int i = 0; i < 30; i++) {
		for (int j = 0; j < 30; j++) {
			buf[i*pwidget->wdevice.base.pitch / 4 + j] = COLOR_Red;
		}
	}
#endif


	// 将脏的数据区字节内容，全部拷贝到screen中
	ui_device_flush((ui_device_t *)&pwidget->wdevice, widget);
}

/**
 * 画点
 */
void pb_widget_draw_point(pb_widget_t * widget, int x, int y, ui_color_t color) {
	rect_t rect;

	widget_draw_point((widget_t *)widget, x, y, color);

	// 先添加区域为脏
	rect_init(&rect, x, y, 1, 1);
	device_add_dirty(widget_device((widget_t *)widget), (widget_t *)widget, &rect);
	device_add_dirty(ui_screen_device(), (widget_t *)widget, &rect);
}

/**
 * 矩形填充
 */
void pb_widget_draw_rect(pb_widget_t * widget, int x, int y, int width, int height, ui_color_t color) {
	rect_t rect;

	widget_draw_rect((widget_t *)widget, x, y, width, height, color);

	// todo: 不加这个不行，不知道为什么
	// 最后通知屏幕device区域为脏
	rect_init(&rect, x, y, width, height);
	device_add_dirty(widget_device((widget_t *)widget), (widget_t *)widget, &rect);
	device_add_dirty(ui_screen_device(), (widget_t *)widget,  &rect);
}

/**
 * 写文字
 */
void pb_widget_draw_text(pb_widget_t * widget, int x, int y, ui_color_t color, const char * string, int size) {
	ui_color_t forge_ground = widget->widget.foreground_color;
	widget->widget.foreground_color = color;

    widget_draw_text((widget_t *)widget, x, y, string, size);

    // 简单起见，只显示一行，多行暂不考虑
    ui_font_t *font = ui_get_font();
    int len = k_strlen(string);

    rect_t dirty_rect;
    rect_init(&dirty_rect, x, y, len * font->width, font->height);
	device_add_dirty(widget_device((widget_t *)widget), (widget_t *)widget, &dirty_rect);
	device_add_dirty(ui_screen_device(), (widget_t *)widget, &dirty_rect);


    // 恢复原来的设置
    widget->widget.foreground_color = forge_ground;
}

/**
 * 画直线
 */
void pb_widget_draw_line(pb_widget_t * widget, int start_x, int start_y, int end_x, int end_y, ui_color_t color) {
	rect_t rect;
	int width = k_abs(start_x - end_x) + 1;
	int height = k_abs(start_y - end_y) + 1;
	int x = k_min(start_x, end_x);
	int y = k_min(start_y, end_y);

	widget_draw_line((widget_t *)widget, start_x, start_y, end_x, end_y, color);

	// 先添加区域为脏
	rect_init(&rect, x, y, width, height);
	device_add_dirty(widget_device((widget_t *)widget), (widget_t *)widget, &rect);
	device_add_dirty(ui_screen_device(), (widget_t *)widget, &rect);
}

/**
 * 设置屏幕上滚
 */
void pb_widget_scroll_up(pb_widget_t * widget, int dy) {
	rect_t rect;

	// 怎样实现屏幕上移？
	pb_device_scroll_up(widget_device((widget_t *)widget), dy);

	// 整个屏幕发生了变动
	rect_copy(&rect, &((widget_t *)widget)->rect);
	rect.x = rect.y = 0;
	device_add_dirty(widget_device((widget_t *)widget), (widget_t *)widget, &rect);
	device_add_dirty(ui_screen_device(), (widget_t *)widget, &rect);
}

/**
 * 初始化
 */
void pb_widget_init (pb_widget_t * widget, int width, int height, widget_t * parent, task_t * owner) {
    widget_init((widget_t *) widget, parent, width, height, owner);

    // 初始化绘图设备及缓存
    k_memset(widget->frame_buffer, COLOR_Black, sizeof(FRAME_BUFFER_SIZE));
    pb_device_init(&widget->wdevice, ui_screen_device(), widget->frame_buffer, width, height, 4);
    widget->widget.device = (ui_device_t *)&widget->wdevice;
    widget->widget.backgroud_color = COLOR_Black;

    // 重设相关数据
    widget->widget.paint = painter_pwidget_default;
    widget->widget.type = WIDGET_TYPE_PIXEL_WIN;
}

/**
 * 创建部件
 */
pb_widget_t * pb_wdiget_create (int width, int height, widget_t * parent, task_t * owner) {
	// 临时空间分配使用
	if (list_count(&win_list) == 0) {
		list_init(&win_list);

		for (int i = 0; i < sizeof(pb_widget_t) / sizeof(pb_widget_t); i++) {
			list_insert_first(&win_list, (list_node_t *)(win_buffer + i));
		}
	}

	// 取结点，初始化各项数据
	pb_widget_t * widget = (pb_widget_t *)list_remove_first(&win_list);
	pb_widget_init(widget, width, height, parent, owner);
    return widget;
}

