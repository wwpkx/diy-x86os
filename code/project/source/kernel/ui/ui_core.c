/**
 * UI核心配置
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <ipc/queue.h>
#include <core/klib.h>
#include <ui/ui_core.h>
#include <ui/ui_screen.h>
#include <ui/ui_event.h>
#include <ui/ui_font.h>
#include "ui/ui_event.h"
#include <ui/mouse.h>

static ui_screen_t screen;				// 屏幕设备，只支持一个屏幕
static widget_t root_widget;			// 不包含mouse的widget容器
static widget_t screen_widget;			// 屏幕对应的widget，作为最顶层

ui_device_t* ui_screen_device(void) {
	return (ui_device_t*) &screen;
}

int ui_screen_width(void) {
	return ui_device_width((ui_device_t*) &screen);
}

int ui_screen_height(void) {
	return ui_device_height((ui_device_t*) &screen);
}

widget_t* ui_screen_widget(void) {
	return &screen_widget;
}

widget_t* ui_root_widget(void) {
	return &root_widget;
}

/**
 * 将位于widget中的x, y座标转换为device中的绝对坐标
 * device可能是widget的直接绘图设备，也可能是更底层的绘图设备
 * device如果是屏幕设备，则转换为屏幕中的绝对坐标。如果是虚拟设备，则转换为
 * 虚拟设备中的坐标，此坐标并不一定是屏幕中的绝对坐标。
 * 如果没有widget，即指定device中的绝对x, y坐标
 * 另外考虑到widget不一定直接在device中，例如widget可能是在虚拟设备上，但是
 * device传入的是屏幕设备，此时要不断映射，直到找到屏幕中的坐标
 */
static void map_to_device(ui_device_t *device, widget_t *widget, int x, int y, int *gx, int *gy) {
	// 直接映射，直接返回结果
	if (widget == (widget_t *)0) {
		*gx = x;
		*gy = y;
		return;
	}

	int t_gx = x, t_gy = y;

	// device不一定是widget所在的直接device
	// 可能是父-祖辈widget的device，所以先向上累加坐标
	// 先不断向上映射，直到碰到相同的device
	// 注意：widget自身的x, y坐标是相对于其父widget中的
	widget_t * parent = widget->parent;
	while (parent && (parent->device != device) && (widget->device != device)) {
		t_gx += widget_x(widget);
		t_gy += widget_y(widget);

		widget = parent;
		parent = widget->parent;
	}

	// 在device中不断向上累加父-祖辈的坐标
	while (parent && (parent->device == device)) {
		// 如果映射的是非屏幕设备，那么只需要往上映射到不同的设备即可
		// 如果是屏幕设备，则一直需要映射到最顶层
		t_gx += widget_x(widget);
		t_gy += widget_y(widget);

		widget = parent;
		parent = widget->parent;
	}

	*gx = t_gx;
	*gy = t_gy;
}

/*
 * 将device中的x, y坐标转换为widget中的坐标
 * widget为使用device显示的设备, 如果不指定widget，则为device内的绝对坐标
 */
static void map_to_widget(ui_device_t *device, int x, int y, widget_t *widget, int *gx, int *gy) {
	// 先获取widget在device的起始绝对坐标
	int start_x, start_y;
	map_to_device(device, widget, 0, 0, &start_x, &start_y);

	// 然后减一下即可得到相对device的绝对坐标
	*gx = x - start_x;
	*gy = y - start_y;
}

/**
 * 将位于widget的rect区域在device设备上标记为脏
 * device可能是widget的直接绘图设备，也可能是更底层的绘图设备
 */
void device_add_dirty(ui_device_t *device, widget_t * widget, rect_t *rect) {
	rect_t global_rect;

	// 如果widget不可显示，则这里可以不添加
	if (widget && !widget->visiable) {
		return;
	}

	// 转换得到所在绘图设备的绝对坐标
	map_to_device(device, widget, rect->x, rect->y, &global_rect.x, &global_rect.y);
	global_rect.width = rect->width;
	global_rect.height = rect->height;

	// 调整绘制区域，设定在device中的可见区域？
	rect_t device_rect, visible_rect;
	rect_init(&device_rect, 0, 0, device->width, device->height);
	rect_get_overlap(&device_rect, &global_rect, &visible_rect);

	// 和已有的脏区域进行合并
	rect_t *dirty_rect = &device->dirty_rect;
	if ((device->dirty_rect.width == 0) || (device->dirty_rect.height == 0)) {
		// 之前为干净，现在重新设置
		rect_copy(dirty_rect, &visible_rect);
	} else {
		int overlap = rect_is_overlap(dirty_rect, &visible_rect);
		if (overlap) {
			// 有交叉就合并成一个
			rect_t temp_rect;
			rect_merge(dirty_rect, &visible_rect, &temp_rect);
			rect_copy(dirty_rect, &temp_rect);
		} else {
			// 无交叉，先将之前的脏数据写到显存里
			// 如果相临，是否可能考虑合并?
			ui_refresh();	// todo: 怎样简化处理？

			// 然后再写入新的脏区域
			rect_copy(dirty_rect, &visible_rect);
		}
	}
}

/**
 * 将widget中的区域转换为所在的显示设备上的区域值
 * 即转换得到在所在直接绘制设备上的x, y的绝对坐标值
 * width和height可能由于其显示宽度超出了显示设备的区域被裁剪
 */
static int relocate_rect(rect_t *dest_rect, widget_t *widget, int x, int y, int width, int height) {
	// 调整得到实际在widget的有效显示区域, 放在show_rect中
	rect_t widget_rect, content_rect, show_rect;
	rect_init(&widget_rect, 0, 0, widget_width(widget), widget_height(widget)); // 整个widget区域
	rect_init(&content_rect, x, y, width, height);      // 期望的内容区域
	if (rect_get_overlap(&widget_rect, &content_rect, &show_rect) < 0) { // rect2实际显示的内容区域
		return -1;
	}

	// 转换成所在设备绘图区域的绝对位置
	ui_device_t * device = widget->device;
	map_to_device(device, widget, show_rect.x, show_rect.y, &show_rect.x, &show_rect.y);

	// 对于非屏幕设备，需要检查脏区域
	// 转换获得在device上的有效区域，并且与脏区域合并，这样方便显示，就不必要在不会刷新的区域进行绘制
	// 由于dirty_rect已经为屏幕上显示的区域内，所以不必再对dest_rect进行调整
	if (device != ui_screen_device()) {
		rect_copy(dest_rect, &show_rect);
	} else {
		rect_t *device_dirty = &device->dirty_rect;
		if (rect_get_overlap(device_dirty, &show_rect, dest_rect) < 0) {
			return -1;
		}
	}

	// 这里得到的结果应当是在device中可见区域并且需要刷新的区域
	// 方便后面调用widget_draw_xxx进行各种绘制
	return 0;
}

/**
 * 刷新整个device的显示到屏幕上
 * 刷新需要知道device的位置信息（用于非screen设备），所以这里要传一个owner
 */
void ui_device_flush(ui_device_t *device, widget_t *owner) {
	// 需要先知道device在frame_buffer对应的起始位置，然后才能拷贝写入
	// 所以先将device所在的位置转换为在frame_buffer中的绝对位置
	int gx, gy;
	map_to_device(ui_screen_device(), owner, 0, 0, &gx, &gy);

	// 然后再刷新写入到frame_buffer中
	device->driver.flush(device, gx, gy);
	rect_init(&device->dirty_rect, 0, 0, 0, 0);
}

/**
 * 在widget指定区域内绘制点
 */
void widget_draw_point(widget_t *widget, int x, int y, ui_color_t color) {
	rect_t dest_rect;

	if (relocate_rect(&dest_rect, widget, x, y, 1, 1) == 0) {
		ui_device_t *device = widget->device;
		device->driver.draw_point(device, dest_rect.x, dest_rect.y, color);
	}
}

/**
 * 在widget内绘制直线
 */
void widget_draw_line(widget_t *widget, int start_x, int start_y, int end_x, int end_y, ui_color_t color) {
	// 较正绘制坐标，x, y设置为低的起点
	int width = k_abs(start_x - end_x) + 1;
	int height = k_abs(start_y - end_y) + 1;
	int x = k_min(start_x, end_x);
	int y = k_min(start_y, end_y);

	// 高度或宽度为0，无需绘制
	if (!width || !height) {
		return;
	}

	// 直线绘制的区域可能超出当前范围，进行调整
	// 一是不能超出widget自身，二是不能超出屏幕范围
	rect_t draw_rect;
	int err = relocate_rect(&draw_rect, widget, x, y, width, height);
	if (err < 0) {
		return;
	}

	ui_device_t *device = widget->device;
	if (height == 1) {
		device->driver.draw_hline(device, x, y, x + draw_rect.width - 1, color);
	} else if (width == 1) {
		device->driver.draw_vline(device, x, y, y + draw_rect.height - 1, color);
	} else {
		// todo: 暂不支持
	}
}

/**
 * 在widget进行矩形填充
 */
void widget_draw_rect(widget_t *widget, int x, int y, int width, int height, ui_color_t color) {
	rect_t dest_rect;

	int err = relocate_rect(&dest_rect, widget, x, y, width, height);
	if (err == 0) {
		ui_device_t *device = widget->device;
		device->driver.fill_rect(device, &dest_rect, color);
	}
}

/**
 * 在widget指定位置显示文字
 */
void widget_draw_text(widget_t *widget, int x, int y, const char *string, int size) {
	int global_x, global_y;
	ui_font_t *font = ui_get_font();

	// 获得在设备上显示的绝对坐标，用于绘制显示
	map_to_device(widget->device, widget, x, y, &global_x, &global_y);

	// 遍历所有字符显示。注：目前只支持一种字体，后面要修改.todo
	ui_device_t *device = widget->device;
	for (const char *curr_pos = string; *curr_pos; curr_pos++) {
		char ch = *curr_pos - 32;  // 转换成可显示的字符

		// 判断当前字符是否在显示区域内并计算显示区域
		rect_t dest_rect;
		int err = relocate_rect(&dest_rect, widget, x, y, font->width, font->height);
		if (err ==  0) {
			// 上面获得的dest_rect可能只包含字符的一部分，所以下面写的时候还需要逐像素判断
			for (int row = 0; row < font->height; row++) {
				uint32_t font_data = ui_get_font()->font_data[ch * font->height + row];

				// 写列
				for (int col = 0; col < font->width; col++) {
					if (!rect_contain_xy(&dest_rect, global_x + col, global_y + row)) {
						continue;
					}

					if (font_data & (1 << (font->width - col))) {
						device->driver.draw_point(device, global_x + col, global_y + row, widget->foreground_color);
					} else if (widget->draw_text_background) {
						device->driver.draw_point(device, global_x + col, global_y + row, widget->backgroud_color);
					}
				}
			}
		}

		global_x += font->width;
		x += font->width;

		// 传参size非0时，即有限定长度时，打印完毕，则结束
		if (size && (--size == 0)) {
			break;
		}
	}
}

/**
 * 判断widget是否需要重新绘制显示
 */
int widget_need_refresh(widget_t *widget) {
	rect_t rect;
	ui_device_t *screen = ui_screen_device();

	// 将widget的坐标转换成在屏幕上的，以判断是否需要刷新
	map_to_device(screen, widget, 0, 0, &rect.x, &rect.y);
	rect.width = widget_width(widget);
	rect.height = widget_height(widget);

	// 与屏幕脏区域进行重叠，检查是否需要刷新
	return rect_is_overlap(&rect, &screen->dirty_rect);
}

void ui_refresh(void) {
	widget_t *curr_widget = &screen_widget;
	list_node_t *node;

	while (curr_widget) {
		if (widget_need_refresh(curr_widget) && curr_widget->visiable) {
			// pixel win有自己的脏区域块数据。flush时只写该区域到屏幕
			// 但是其在screen上的显示可能已经被其它窗口更改，而flush又不涉及到此区域
			// 所以，在这里将pixel_win的脏区域与screen的脏区域进行合并到pixel_win
			if (curr_widget->type == WIDGET_TYPE_PIXEL_WIN) {
				ui_device_t *screen = ui_screen_device();

				// 将屏幕中的脏区域转换为相对自己的脏区域
				rect_t local_dirty;
				rect_copy(&local_dirty, &screen->dirty_rect);
				map_to_widget(screen, screen->dirty_rect.x,
						screen->dirty_rect.y, curr_widget, &local_dirty.x, &local_dirty.y);

				// 两个脏区域合并，得到在本widget中脏区域
				rect_t merge_rect;
				rect_merge(&curr_widget->device->dirty_rect, &local_dirty,
						&merge_rect);

				// 注意，合并后可能超边界，取在自己内可显示的部分，并更新脏区域
				rect_t rect_show, inner_rect;
				rect_init(&inner_rect, 0, 0, curr_widget->rect.width,
						curr_widget->rect.height);
				rect_get_overlap(&inner_rect, &merge_rect, &rect_show);
				rect_copy(&curr_widget->device->dirty_rect, &rect_show);
			}

			// 先刷当前结点
			if (curr_widget->paint) {
				curr_widget->paint(curr_widget);
			}

			// 看看是否有子结点
			node = list_first(&curr_widget->child_list);
			if (node) {
				// 如果有，则刷子结点
				curr_widget = node_to_parent(node, widget_t, node);
				continue;
			}
		}

		// 不需要刷新，或者没有子结点，找同级的后一个结点
		node = list_node_next(&curr_widget->node);
		if (node) {
			// 如果有，则刷该结点
			curr_widget = node_to_parent(node, widget_t, node);
			continue;
		}

		// 当前不需要刷新，或者没有子结点，以及没有同级后续结点时
		// 往上寻找父结点的下一结点去刷新。如果没有，继续向上
		widget_t *parent_widget = curr_widget->parent;
		node = (list_node_t*) 0;
		while (parent_widget) {
			node = list_node_next(&parent_widget->node);
			if (node) {
				break;
			} else {
				parent_widget = parent_widget->parent;
			}
		}

		// 没有可用结点，终止循环
		if (!node) {
			break;
		}
		curr_widget = node_to_parent(node, widget_t, node);
	}

	// 刷完后，清空缓存
	ui_device_flush((ui_device_t*) &screen, &screen_widget);
}

// 发送鼠标事件消息：应当先由上层响应
void ui_send_mouse_event(mouse_event_t *event) {
	widget_t *event_widget = (widget_t*) 0;
	widget_t *active_widget = (widget_t*) 0;

	// 找到处于最顶层的，且鼠标操作的该区域内的widget
	list_node_t *node = list_first(&root_widget.child_list);
	if (!node) {
		return;
	}

	widget_t *curr = node_to_parent(node, widget_t, node);
	while (curr) {
		int x = event->x, y = event->y;

		// 包含鼠标的坐标范围，纪录一下
		// 但还是要要寻找相邻下一个，因为可能被其它遮挡了
		if (rect_contain_xy(&curr->rect, x, y)) {
			event_widget = node_to_parent(node, widget_t, node);
		} else {
			// 未点击中该widget，而是点击中了其它widget，需要设置非激活。先纪录一下
			if (curr->active) {
				active_widget = curr;
			}
		}

		// 取下一相依的窗口
		node = list_node_next(&curr->node);
		curr = node_to_parent(node, widget_t, node);
	}

	if (event_widget) {
		// 先交由回调处理
		if (event_widget->mevent_hander) {
			event_widget->mevent_hander(event_widget, event);
		}

		// 设置为激活状态
		widget_set_active(event_widget, 1);

		// 点击，并且当前激活窗口非当前点中的窗口，取消窗口的激活状态
		if ((event->type == MOUSE_EVENT_PRESS)
				&& active_widget
				&& (event_widget != active_widget)) {
			widget_set_active(active_widget, 0);
		}
	}
}

/**
 * 将按键事件传递给相应的应用进程处理或者是widget的回调函数处理
 */
void ui_send_key_event(key_event_t *event) {
	widget_t *top_widget = &root_widget;

	// 定位到最顶的widget
	while (list_count(&top_widget->child_list)) {
		list_node_t *node = list_last(&top_widget->child_list);
		top_widget = node_to_parent(node, widget_t, node);
	}

	// 再往上依次处理消息
	while (top_widget != &screen_widget) {
		// 先交给界面的回调处理
		if (top_widget->kevent_handler) {
			int stop = top_widget->kevent_handler(top_widget, event);
			if (stop) {
				// 已经处理完毕，不需要再处理，立即返回
				return;
			}
		}

		// 到达最顶层的应用，看看应用是否能处理
		if (top_widget->parent == &root_widget) {
		    // 回调函数处理不了，直接给任务的消息队列，让任务自行处理
			// 任务可能将自有用途，或者写入终端
			task_t * curr_task = top_widget->task;
			task_msg_t * task_msg = (task_msg_t *)queue_get_free(&curr_task->queue);
			if (task_msg) {
				task_msg->msg.type = APP_MSG_TYPE_KEYBOARD;
				task_msg->msg.key = event->code;
				queue_send_msg(&curr_task->queue, (queue_msg_t *)task_msg);
			}
		}
		top_widget = top_widget->parent;
	}
}

/**
 * UI模块初始化
 */
void ui_init(boot_info_t *boot_info) {
	// 唯一的一个屏幕设备
	screen_init(&screen, boot_info);

	// 初始化顶层widget，不需要绘制
	widget_init(&screen_widget, (widget_t*) 0, screen.base.width, screen.base.height, (task_t *)0);
	screen_widget.paint = (void (*)(struct _widget_t *widget)) 0;
	screen_widget.visiable = 1;

	// 顶层容器widget
	widget_init(&root_widget, &screen_widget, screen.base.width, screen.base.height, (task_t *)0);
	root_widget.visiable = 1;

	ui_mouse_init();
	ui_server_init();
}

