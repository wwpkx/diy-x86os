/**
 * UI事件处理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <core/task.h>
#include <ipc/queue.h>
#include <core/syscall.h>
#include <core/irq.h>
#include <core/klib.h>
#include <core/os_cfg.h>
#include <ui/button.h>
#include <ui/pb_widget.h>
#include <ui/ui_core.h>
#include <ui/ui_event.h>
#include <ui/tty_widget.h>
#include <ui/label.h>
#include <ui/window.h>
#include <ui/mouse.h>

static task_t ui_task;
static uint32_t ui_task_stack[UI_TASK_SIZE];
static ui_msg_t ui_msg_buf[UI_MSG_COUNT];

/**
 * 移动主窗口
 */
static void do_widget_move_to(ui_msg_t * msg) {
	ui_widget_t win = (ui_widget_t)msg->api_msg.param[0];
	int x = msg->api_msg.param[1];
	int y = msg->api_msg.param[2];

	if (win) {
		widget_move_to((widget_t *) win, x, y);
	    msg->api_msg.ret = 0;
	} else {
		msg->api_msg.ret = -1;
	}
}

/**
 * 设置widget为显示状态
 */
static void do_widget_set_visiable(ui_msg_t * msg) {
	widget_t * widget = (widget_t *)msg->api_msg.param[0];
	int visiable = msg->api_msg.param[1];

	if (widget) {
		widget_set_visiable(widget, visiable ? UI_VISIABLE : UI_HIDDEN);

		// 如果启用了显示，将其设置成激活状态
		widget_set_active(widget, visiable);
		msg->api_msg.ret = 0;
	} else {
		msg->api_msg.ret = -1;
	}
}

/**
 * 获取窗口的宽度
 */
static void do_widget_width(ui_msg_t * msg) {
    widget_t * widget = (widget_t *)msg->api_msg.param[0];
    if (widget) {
    	msg->api_msg.ret = (int)widget_width(widget);
    } else {
    	msg->api_msg.ret = -1;
    }
}

/**
 * 获取窗口的高度
 */
static void do_widget_height(ui_msg_t * msg) {
    widget_t * widget = (widget_t *)msg->api_msg.param[0];
    if (widget) {
    	msg->api_msg.ret = (int)widget_height(widget);
    } else {
    	msg->api_msg.ret = -1;
    }
}

/**
 * 获取窗口内容区的宽度
 */
static void do_widget_content_width(ui_msg_t * msg) {
    widget_t * widget = (widget_t *)msg->api_msg.param[0];
    if (widget) {
    	msg->api_msg.ret = (int)widget_content_width(widget);
    } else {
    	msg->api_msg.ret = -1;
    }
}

/**
 * 获取窗口的内容区高度
 */
static void do_widget_content_height(ui_msg_t * msg) {
    widget_t * widget = (widget_t *)msg->api_msg.param[0];
    if (widget) {
    	msg->api_msg.ret = (int)widget_content_height(widget);
    } else {
    	msg->api_msg.ret = -1;
    }
}

/**
 * 创建主窗口调用
 */
static void do_window_create(ui_msg_t *msg) {
    window_t *win;

    const char * title = (const char * )msg->api_msg.param[0];
    int width = msg->api_msg.param[1];
    int height = msg->api_msg.param[2];
    ui_widget_t parent = (ui_widget_t)msg->api_msg.param[3];

    win = window_create(title, width, height, (widget_t *) parent, msg->from);
    msg->api_msg.ret = (int) win;
}

/**
 * 给主窗口添加子部分
 */
static void do_window_add(ui_msg_t * msg) {
	ui_widget_t win = (ui_widget_t)msg->api_msg.param[0];
	ui_widget_t sub = (ui_widget_t)msg->api_msg.param[1];

	if (win && sub) {
		window_add_widget((window_t *) win, (widget_t *) sub);
		msg->api_msg.ret = 0;
	} else {
		msg->api_msg.ret = -1;
	}
}

/**
 * 获取主窗口的容器窗口
 */
static void do_windown_container(ui_msg_t * msg) {
    window_t * win = (window_t *)msg->api_msg.param[0];

    if (win) {
    	msg->api_msg.ret = (int)&win->content_widget;
    } else {
    	msg->api_msg.ret = 0;
    }
}

/**
 * 创建标签
 */
static void do_label_create(ui_msg_t * msg) {
    const char *content = (const char *)msg->api_msg.param[0];
    int width = msg->api_msg.param[1];
    int height = msg->api_msg.param[2];
    ui_widget_t parent = (ui_widget_t)msg->api_msg.param[3];

	label_t * label = label_create(content, width, height, (widget_t *) parent, msg->from);
	msg->api_msg.ret = (int)label;
}

/**
 * 设置标签内容
 */
static void do_label_set_text(ui_msg_t * msg) {
	ui_widget_t win = (ui_widget_t)msg->api_msg.param[0];
	const char *content = (const char *)msg->api_msg.param[1];

	if (win && content) {
		label_set_text((label_t *) win, content);
		msg->api_msg.ret = 0;
	} else {
		msg->api_msg.ret = -1;
	}
}

/**
 * 创建按键
 */
static void do_button_create(ui_msg_t * msg) {
    const char *content = (const char *)msg->api_msg.param[0];
    int width = msg->api_msg.param[1];
    int height = msg->api_msg.param[2];
    ui_widget_t parent = (ui_widget_t)msg->api_msg.param[3];

	button_t * button = button_create(content, width, height, (widget_t *) parent, msg->from);
	msg->api_msg.ret = (int)button;
}

/**
 * 设置按键的文字
 */
static void do_button_set_text(ui_msg_t * msg) {
	ui_widget_t win = (ui_widget_t)msg->api_msg.param[0];
	const char *content = (const char *)msg->api_msg.param[1];

	if (win && content) {
		button_set_text((button_t *) win, content);
		msg->api_msg.ret = 0;
	} else {
		msg->api_msg.ret = -1;
	}
}

/**
 * 创建绘图窗口
 */
static void do_pbwidget_create(ui_msg_t * msg) {
	int width = msg->api_msg.param[0];
	int height = msg->api_msg.param[1];
	ui_widget_t parent = (ui_widget_t)msg->api_msg.param[2];

	pb_widget_t * win = pb_wdiget_create(width, height, parent, msg->from);
	msg->api_msg.ret = (int)win;
}

/**
 * 在绘图窗口上画图
 */
static void do_pbwidget_draw_point(ui_msg_t * msg) {
	pb_widget_t * win = (pb_widget_t *)msg->api_msg.param[0];
	int x = msg->api_msg.param[1];
	int y = msg->api_msg.param[2];
	ui_color_t color = msg->api_msg.param[3];

	if (win && (win->widget.type == WIDGET_TYPE_PIXEL_WIN)) {
		pb_widget_draw_point(win, x, y, color);
		msg->api_msg.ret = 0;
	} else {
		msg->api_msg.ret = -1;
	}
}

/**
 * 在绘图窗口上画矩形
 */
static void do_pwidget_draw_rect(ui_msg_t * msg) {
	pb_widget_t * win = (pb_widget_t *)msg->api_msg.param[0];
	int x = msg->api_msg.param[1];
	int y = msg->api_msg.param[2];
	int width = msg->api_msg.param[3];
	int height = msg->api_msg.param[4];
	ui_color_t color = msg->api_msg.param[5];

	if (win && (win->widget.type == WIDGET_TYPE_PIXEL_WIN)) {
		pb_widget_draw_rect(win, x, y, width, height, color);
		msg->api_msg.ret = 0;
	} else {
		msg->api_msg.ret = -1;
	}
}

/**
 * 在绘图窗口上写字符串
 */
static void do_pbwidget_draw_string(ui_msg_t * msg) {
	pb_widget_t * win = (pb_widget_t *)msg->api_msg.param[0];
	int x = msg->api_msg.param[1];
	int y = msg->api_msg.param[2];
	ui_color_t color = (ui_color_t)msg->api_msg.param[3];
	const char * string = (const char * )msg->api_msg.param[4];

	if (win && (win->widget.type == WIDGET_TYPE_PIXEL_WIN)) {
		pb_widget_draw_text(win, x, y,color, string, 0);
		msg->api_msg.ret = 0;
	} else {
		msg->api_msg.ret = -1;
	}
}

/**
 * 在绘图窗口上画直线
 */
static void do_pidget_draw_line(ui_msg_t * msg) {
	pb_widget_t * win = (pb_widget_t *)msg->api_msg.param[0];
	int start_x = msg->api_msg.param[1];
	int start_y = msg->api_msg.param[2];
	int end_x = msg->api_msg.param[3];
	int end_y = msg->api_msg.param[4];
	ui_color_t color = (ui_color_t)msg->api_msg.param[5];

	if (win && (win->widget.type == WIDGET_TYPE_PIXEL_WIN)) {
		pb_widget_draw_line(win, start_x, start_y, end_x, end_y, color);
		msg->api_msg.ret = 0;
	} else {
		msg->api_msg.ret = -1;
	}
}

/**
 * 在绘图窗口上绘制矩形
 */
static void do_tty_draw_rect(ui_msg_t * msg) {
	pb_widget_t * win = (pb_widget_t *)msg->api_msg.param[0];
	int x = msg->api_msg.param[1];
	int y = msg->api_msg.param[2];
	int width = msg->api_msg.param[3];
	int height = msg->api_msg.param[4];
	ui_color_t color = msg->api_msg.param[5];

	pb_widget_draw_rect(win, x, y, width, height, color);
	msg->api_msg.ret = 0;
}

/**
 * 在绘图窗口上绘制直线
 */
static void do_tty_draw_text(ui_msg_t * msg) {
	pb_widget_t * win = (pb_widget_t *)msg->api_msg.param[0];
	int x = msg->api_msg.param[1];
	int y = msg->api_msg.param[2];
	ui_color_t color = msg->api_msg.param[3];
	const char * string = (const char *)msg->api_msg.param[4];
	int size = msg->api_msg.param[5];

	pb_widget_draw_text(win, x, y, color, string, size);
	msg->api_msg.ret = 0;
}

/**
 * 屏幕上滚
 */
static void do_tty_scroll_up (ui_msg_t * msg) {
	pb_widget_t * win = (pb_widget_t *)msg->api_msg.param[0];
	int dy = msg->api_msg.param[1];
	pb_widget_scroll_up(win, dy);
	msg->api_msg.ret = 0;
}

/**
 * 创建tty窗口
 */
static void do_tty_widget_create (ui_msg_t * msg) {
	int width = msg->api_msg.param[0];
	int height = msg->api_msg.param[1];
	ui_widget_t parent = (ui_widget_t)msg->api_msg.param[2];

	tty_widget_t * win = tty_widget_create(width, height, parent, msg->from);
	msg->api_msg.ret = (int)win;
}

/**
 * 获取窗口大小等信息
 */
static void do_tty_widget_cols (ui_msg_t * msg) {
	tty_widget_t * win = (tty_widget_t *)msg->api_msg.param[0];
	msg->api_msg.ret = tty_widget_cols(win);
}

/**
 * 通知刷新UI
 */
static void do_ui_refresh (ui_msg_t * msg) {
	ui_refresh();
}


/**
 * 分配一个UI消息
 */
ui_msg_t *ui_alloc_msg(ui_msg_type_t type, int wait) {
	ui_msg_t * msg;

	if (wait) {
		msg = (ui_msg_t *) queue_alloc_free(&ui_task.queue);
	} else {
		msg = (ui_msg_t *) queue_get_free(&ui_task.queue);
	}
	if (msg) {
		msg->type = type;
	}

	return msg;
}

/**
 * 释放UI消息
 */
void ui_free_msg (ui_msg_t * msg) {
	queue_free(&ui_task.queue, (queue_msg_t *) msg);
}

/**
 * 发送UI消息给进程处理，由中断或者应用进程调用
 * 如果是任务，则简单点等待处理是否完成，仅当处理完成才继续做自己原来的事情
 * @param msg
 * @param task_wait
 */
void ui_send_msg(ui_msg_t *msg, task_t * from, int wait_ret) {
	msg->from = from;

	if (wait_ret) {
		// 创建信号量去等结果
		sem_t sem;
		sem_init(&sem, 0);
		msg->sem = &sem;
	    queue_send_msg(&ui_task.queue, (queue_msg_t *) msg);
		sem_wait(&sem);
	} else {
		msg->sem = (sem_t *)0;
	    queue_send_msg(&ui_task.queue, (queue_msg_t *) msg);
	}
}

/**
 * 通知pbwidget绘制矩形
 */
void pbwidget_request_draw_rect(pb_widget_t * win, int x, int y, int width, int height, ui_color_t color) {
    ui_msg_t * ui_msg = (ui_msg_t *)queue_alloc_free(&ui_task.queue);
    ui_api_msg_t * api_msg = &ui_msg->api_msg;

    api_msg->func = UI_API_TTY_DRAW_RECT;
    api_msg->param[0] = (int) win;
    api_msg->param[1] = x;
    api_msg->param[2] = y;
    api_msg->param[3] = width;
    api_msg->param[4] = height;
    api_msg->param[5] = color;

    ui_msg->type = UI_MSG_API;
	ui_send_msg(ui_msg, task_current(), 1);
	ui_free_msg(ui_msg);
}

/**
 * 通知pbwidget写文字
 */
void pbwidget_request_draw_text(pb_widget_t * widget, int x, int y,
		ui_color_t color, const char * string, int size) {
    ui_msg_t * ui_msg = (ui_msg_t *)queue_alloc_free(&ui_task.queue);
    ui_api_msg_t * api_msg = &ui_msg->api_msg;

    api_msg->func = UI_API_TTY_DRAW_TEXT;
    api_msg->param[0] = (int) widget;
    api_msg->param[1] = x;
    api_msg->param[2] = y;
    api_msg->param[3] = color;
    api_msg->param[4] = (int)string;
    api_msg->param[5] = size;

    ui_msg->type = UI_MSG_API;
	ui_send_msg(ui_msg, task_current(), 1);
	ui_free_msg(ui_msg);
}

/**
 * 请求上下滚动屏幕
 */
void pbwidget_request_scroll_up (pb_widget_t * widget, int dy) {
    ui_msg_t * ui_msg = (ui_msg_t *)queue_alloc_free(&ui_task.queue);
    ui_api_msg_t * api_msg = &ui_msg->api_msg;

    api_msg->func = UI_API_TTY_SCROLL_UP;
    api_msg->param[0] = (int) widget;
    api_msg->param[1] = dy;

    ui_msg->type = UI_MSG_API;
	ui_send_msg(ui_msg, task_current(), 1);
	ui_free_msg(ui_msg);
}

/**
 * 通知UI刷新
 */
void ui_notify_refresh (void) {
    ui_msg_t * ui_msg = (ui_msg_t *)queue_alloc_free(&ui_task.queue);
    ui_api_msg_t * api_msg = &ui_msg->api_msg;

    api_msg->func = UI_API_REFRESH;
    ui_msg->type = UI_MSG_API;
	ui_send_msg(ui_msg, task_current(), 1);
	ui_free_msg(ui_msg);
}

/**
 * UI的系统调用处理
 */
typedef void (*ui_event_handler_t) (ui_msg_t *msg);
static void do_api_msg(ui_msg_t *msg) {
	static ui_event_handler_t handler[] = {
			[UI_API_WIN_CREATE] = do_window_create,
			[UI_API_WIN_ADD_SUB] = do_window_add,
			[UI_API_WIN_CONTAINER] = do_windown_container,

			[UI_API_WIDGET_MOVE_TO] = do_widget_move_to,
			[UI_API_WIDGET_WIDTH] = do_widget_width,
			[UI_API_WIDGET_HEIGHT] = do_widget_height,
			[UI_API_WIDGET_CONTENT_WIDTH] = do_widget_content_width,
			[UI_API_WIDGET_CONTENT_HEIGHT] = do_widget_content_height,
			[UI_API_WIDGET_SET_VISIABLE] = do_widget_set_visiable,

			[UI_API_LABEL_CREATE] = do_label_create,
			[UI_API_LABEL_SET_TEXT] = do_label_set_text,

			[UI_API_BUTTON_CREATE] = do_button_create,
			[UI_API_BUTTON_SET_TEXT] = do_button_set_text,

			[UI_API_PBWIDGET_CREATE] = do_pbwidget_create,
			[UI_API_PBWIDGET_DRAW_POINT] = do_pbwidget_draw_point,
			[UI_API_PBWIDGET_DRAW_RECT]  = do_pwidget_draw_rect,
			[UI_API_PBWIDGET_DRAW_STRING] = do_pbwidget_draw_string,
			[UI_API_PBWIDGET_DRAW_LINE] = do_pidget_draw_line,

			[UI_API_TTY_WIDGET_CREATE] = do_tty_widget_create,
			[UI_API_TTY_WIDGET_COLS] = do_tty_widget_cols,

			[UI_API_TTY_DRAW_RECT] = do_tty_draw_rect,
			[UI_API_TTY_DRAW_TEXT] = do_tty_draw_text,
			[UI_API_TTY_SCROLL_UP] = do_tty_scroll_up,

			[UI_API_REFRESH] = do_ui_refresh,
	};

	// 超出返回
	if ((msg->api_msg.func < 0) ||
			(msg->api_msg.func >= sizeof(handler) / sizeof(ui_event_handler_t))) {
		return;
	}

	// 执行回调
    ui_event_handler_t func = handler[msg->api_msg.func];
    if (func) {
    	func(msg);
    }
}

/**
 * 鼠标消息的处理
 */
static void do_mouse_msg(ui_msg_t *msg) {
    static int pre_left_pressed = 0;

    int total_dx = msg->delta_x, total_dy = msg->delta_y;

    // 短时间内可能产生大量的鼠标事件，导致产生很多消息
    // 虽然UI任务中有尝试连续取多个消息，但是其内部可能会导致多次频繁UI刷新
    // 所以在这里一并取连续的鼠标事件，将x, y的坐标变化进行合并，最后一起刷新
    // 也可考虑在底层中断处理，这样能减少发送的消息数量
    ui_msg_t * new_msg;
    while ((new_msg = (ui_msg_t *)queue_check_msg(&ui_task.queue))) {
    	// 事件类型需要相同，不同则退出
    	if (new_msg->type != UI_MSG_MOUSE) {
    		break;
    	}

    	// 鼠标按键状态需要相同
    	if ((new_msg->left_pressed) != (msg->left_pressed)
    			|| (new_msg->center_pressed != msg->center_pressed)
				|| (new_msg->right_pressed != msg->right_pressed)) {
    		break;
    	}

    	new_msg = (ui_msg_t *)queue_get_msg(&ui_task.queue);

    	// 合并鼠标变化值
    	total_dx += new_msg->delta_x;
    	total_dy += new_msg->delta_y;

    	queue_free(&ui_task.queue, (queue_msg_t *)new_msg);
    }

    // 移动鼠标， 因鼠标不能超出屏幕外，导致实际移动的偏移可能比total_d?的要少
    int old_x = ui_mouse_getx(), old_y = ui_mouse_gety();
    ui_mouse_move_by(total_dx, total_dy);
    int dx = ui_mouse_getx() - old_x, dy = ui_mouse_gety() - old_y;

    // 将事件发送给其它窗口
    mouse_event_t mouse_event;
    if (msg->left_pressed) {
        // 之前没有按下，现在按下
        if (!pre_left_pressed) {
            mouse_event.type = MOUSE_EVENT_PRESS;
        } else if (dx || dy) {
            // 之前按下，现在也按下，也有位移
            mouse_event.type = MOUSE_EVENT_DRAG;
        }
    } else {
        // 现在没有按下，之前未按下
        if (!pre_left_pressed) {
            mouse_event.type = MOUSE_EVENT_MOVE;
        } else {
            mouse_event.type = MOUSE_EVENT_CLICK;
        }
    }

    mouse_event.dx = dx;
    mouse_event.dy = dy;
    mouse_event.x = old_x;		// 注意要用之前的坐标
    mouse_event.y = old_y;
    ui_send_mouse_event(&mouse_event);

    pre_left_pressed = msg->left_pressed;
}

/**
 * 按键事件的处理
 */
static void do_key_msg(ui_msg_t *msg) {
    key_event_t event;

    // 转换成send_key中的消息去处理
    event.type = KEY_EVENT_ENTERED;
    event.code = msg->code;
    ui_send_key_event(&event);
}


/**
 * UI进程
 * 负责UI的刷新和各种API请求的处理
 */
void ui_task_entry(void * arg) {
	(void)arg;

    for (;;) {
    	// 先刷新一遍UI，让该显示的都显示出来，再处理消息
        ui_refresh();

        // 一次性的将队列中有的消息全部处理完，如果没有，再刷新UI
        // 对于短时间内有大量消息的事件, 避免了过于频繁的刷新UI
        do {
            ui_msg_t *msg = (ui_msg_t *) queue_wait_msg(&ui_task.queue);
            switch (msg->type) {
                case UI_MSG_MOUSE:
                    do_mouse_msg(msg);
                    queue_free(&ui_task.queue, (queue_msg_t *) msg);
                    break;
                case UI_MSG_KEYBOARD:
                    do_key_msg(msg);
                    queue_free(&ui_task.queue, (queue_msg_t *) msg);
                    break;
                case UI_MSG_API:
                    do_api_msg(msg);

                    // 通知调用方处理完成，msg的释放由任务自己去做，因为要从msg取返回结果
                    if (msg->sem) {
                    	sem_notify(msg->sem);
                    } else {
                    	// 不需要等待，由UI进程释放
                        queue_free(&ui_task.queue, (queue_msg_t *) msg);
                    }
                 break;
                default:
                    queue_free(&ui_task.queue, (queue_msg_t *) msg);
                    break;
            }
        } while (queue_check_msg(&ui_task.queue) != (queue_msg_t *) 0);
    }
}

/**
 * UI进程
 * 处理各种窗口创建刷新等事件
 */
void ui_server_init(void) {
    task_init(&ui_task,
              "UI task",
              TASK_FLAG_SYSTEM_TASK,		// 系统任务，可访问所有数据
              (task_entry_t *) ui_task_entry,
              (void *) 0,
              (uint32_t *) ((uint8_t *) ui_task_stack + sizeof(ui_task_stack)),
              UI_TASK_PRIORITY,
			  ui_msg_buf, sizeof(ui_msg_t), UI_MSG_COUNT);
}
