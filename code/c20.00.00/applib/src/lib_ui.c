/**
 * UI部分的系统调用
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <core/syscall.h>
#include <ui/ui_event.h>
#include "lib_ui.h"
#include "lib_syscall.h"

static timer_callback_t timer_cb = (timer_callback_t)0;	// 定时时间到回调函数
static key_callback_t key_cb = (key_callback_t)0;		// 按键回调函数

// 是否强制等待UI的API调用，调试用
// 有时可能希望立即看到api调用的结果，则可以将该值置为1
// 有时可能不需要，可置为0，这样应用可做其它的事情，不需要等待
#define	UI_API_FORCE_WAIT			1

/**
 * 创建主窗口的API
 */
ui_widget_t ui_window_create(const char *title, int width, int height, ui_widget_t parent) {
    ui_api_msg_t msg;

    msg.func = UI_API_WIN_CREATE;
    msg.param[0] = (int) title;
    msg.param[1] = width;
    msg.param[2] = height;
    msg.param[3] = (int) parent;

    if (sys_send_msg(SYSCALL_MSG_DEST_UI, &msg, 1) == 0) {
    	return (ui_widget_t)msg.ret;
    }

    return (ui_widget_t ) 0;
}

/**
 * 添加子窗口
 */
void ui_window_add(ui_widget_t window, ui_widget_t sub) {
    ui_api_msg_t msg;

    msg.func = UI_API_WIN_ADD_SUB;
    msg.param[0] = (int) window;
    msg.param[1] = (int) sub;

    sys_send_msg(SYSCALL_MSG_DEST_UI, &msg, UI_API_FORCE_WAIT);
}


/**
 * 宽度主窗口容器
 */
ui_widget_t ui_window_container(ui_widget_t win) {
    ui_api_msg_t msg;

    msg.func = UI_API_WIN_CONTAINER;
    msg.param[0] = (int) win;
    if (sys_send_msg(SYSCALL_MSG_DEST_UI, &msg, 1) == 0) {
    	return (ui_widget_t)msg.ret;
    }

    return 0;
}

/**
 * 创建标签
 */
ui_widget_t ui_label_create(const char *content, int width, int height, ui_widget_t parent) {
    ui_api_msg_t msg;

    msg.func = UI_API_LABEL_CREATE;
    msg.param[0] = (int) content;
    msg.param[1] = width;
    msg.param[2] = height;
    msg.param[3] = (int) parent;

    if (sys_send_msg(SYSCALL_MSG_DEST_UI, &msg, 1) == 0) {
    	return (ui_widget_t)msg.ret;
    }

    return (ui_widget_t) 0;
}

/**
 * 设置标签
 */
void ui_label_set(ui_widget_t win, const char *content) {
    ui_api_msg_t msg;

    msg.func = UI_API_LABEL_SET_TEXT;
    msg.param[0] = (int) win;
    msg.param[1] = (int) content;

    sys_send_msg(SYSCALL_MSG_DEST_UI, &msg, UI_API_FORCE_WAIT);
}

/**
 * 创建按钮
 */
ui_widget_t ui_button_create (const char *content, int width, int height, ui_widget_t parent) {
    ui_api_msg_t msg;

    msg.func = UI_API_BUTTON_CREATE;
    msg.param[0] = (int) content;
    msg.param[1] = width;
    msg.param[2] = height;
    msg.param[3] = (int) parent;

    if (sys_send_msg(SYSCALL_MSG_DEST_UI, &msg, 1) == 0) {
    	return (ui_widget_t)msg.ret;
    }

    return (ui_widget_t ) 0;
}

/**
 * 设置按钮
 */
void ui_button_set (ui_widget_t button, const char *content) {
    ui_api_msg_t msg;

    msg.func = UI_API_BUTTON_SET_TEXT;
    msg.param[0] = (int) button;
    msg.param[1] = (int) content;

    sys_send_msg(SYSCALL_MSG_DEST_UI, &msg, UI_API_FORCE_WAIT);
}


/**
 * 移动窗口
 */
void ui_widget_move_to(ui_widget_t win, int x, int y) {
    ui_api_msg_t msg;

    msg.func = UI_API_WIDGET_MOVE_TO;
    msg.param[0] = (int) win;
    msg.param[1] = x;
    msg.param[2] = y;

    sys_send_msg(SYSCALL_MSG_DEST_UI, &msg, UI_API_FORCE_WAIT);
}

/**
 * 设置可见性
 */
void ui_widget_set_visiable(ui_widget_t win, int visiable) {
    ui_api_msg_t msg;

    msg.func = UI_API_WIDGET_SET_VISIABLE;
    msg.param[0] = (int) win;
    msg.param[1] = (int) visiable;

    sys_send_msg(SYSCALL_MSG_DEST_UI, &msg, UI_API_FORCE_WAIT);
}

/**
 * 获取宽度
 */
int ui_widget_width(ui_widget_t win) {
    ui_api_msg_t msg;

    msg.func = UI_API_WIDGET_WIDTH;
    msg.param[0] = (int) win;
    if (sys_send_msg(SYSCALL_MSG_DEST_UI, &msg, 1) == 0) {
    	return (int)msg.ret;
    }

    return 0;
}

/**
 * 获取高度
 */
int ui_widget_height(ui_widget_t win) {
    ui_api_msg_t msg;

    msg.func = UI_API_WIDGET_HEIGHT;
    msg.param[0] = (int) win;
    if (sys_send_msg(SYSCALL_MSG_DEST_UI, &msg, 1) == 0) {
    	return (int)msg.ret;
    }

    return 0;
}

/**
 * 获取宽度
 */
int ui_widget_content_width(ui_widget_t win) {
    ui_api_msg_t msg;

    msg.func = UI_API_WIDGET_CONTENT_WIDTH;
    msg.param[0] = (int) win;
    if (sys_send_msg(SYSCALL_MSG_DEST_UI, &msg, 1) == 0) {
    	return (int)msg.ret;
    }

    return 0;
}

/**
 * 获取高度
 */
int ui_widget_content_height(ui_widget_t win) {
    ui_api_msg_t msg;

    msg.func = UI_API_WIDGET_CONTENT_HEIGHT;
    msg.param[0] = (int) win;
    if (sys_send_msg(SYSCALL_MSG_DEST_UI, &msg, 1) == 0) {
    	return (int)msg.ret;
    }

    return 0;
}

/**
 * 创建绘图窗口
 */
ui_widget_t ui_pixel_win_create(int width, int height, ui_widget_t parent) {
    ui_api_msg_t msg;

    msg.func = UI_API_PBWIDGET_CREATE;
    msg.param[0] = (int) width;
    msg.param[1] = (int) height;
    msg.param[2] = (int) parent;

    if (sys_send_msg(SYSCALL_MSG_DEST_UI, &msg, 1) == 0) {
        return (ui_widget_t)msg.ret;
    }

    return (ui_widget_t ) 0;
}

/**
 * 绘图窗口中绘制点
 */
void ui_pixel_win_draw_point(ui_widget_t win, int x, int y, ui_color_t color) {
    ui_api_msg_t msg;

    msg.func = UI_API_PBWIDGET_DRAW_POINT;
    msg.param[0] = (int) win;
    msg.param[1] = x;
    msg.param[2] = y;
    msg.param[3] = (int)color;
    sys_send_msg(SYSCALL_MSG_DEST_UI, &msg, UI_API_FORCE_WAIT);
}


/**
 * 绘图窗口中画矩形
 */
void ui_pixel_win_draw_rect(ui_widget_t win, int x, int y, int width, int height, ui_color_t color) {
    ui_api_msg_t msg;

    msg.func = UI_API_PBWIDGET_DRAW_RECT;
    msg.param[0] = (int) win;
    msg.param[1] = x;
    msg.param[2] = y;
    msg.param[3] = width;
    msg.param[4] = height;
    msg.param[5] = (int)color;
    sys_send_msg(SYSCALL_MSG_DEST_UI, &msg, UI_API_FORCE_WAIT);
}

/**
 * 绘图窗口中画字
 */
void ui_pixel_win_draw_string(ui_widget_t win, int x, int y, ui_color_t color, const char * string) {
    ui_api_msg_t msg;

    msg.func = UI_API_PBWIDGET_DRAW_STRING;
    msg.param[0] = (int) win;
    msg.param[1] = x;
    msg.param[2] = y;
    msg.param[3] = (int)color;
    msg.param[4] = (int)string;
    sys_send_msg(SYSCALL_MSG_DEST_UI, &msg, UI_API_FORCE_WAIT);
}

/**
 * 绘图窗口中画线
 */
void ui_pixel_win_draw_line(ui_widget_t win, int start_x, int start_y,
		int end_x, int end_y, ui_color_t color) {
    ui_api_msg_t msg;

    msg.func = UI_API_PBWIDGET_DRAW_LINE;
    msg.param[0] = (int) win;
    msg.param[1] = start_x;
    msg.param[2] = start_y;
    msg.param[3] = end_x;
    msg.param[4] = end_y;
    msg.param[5] = (int)color;
    sys_send_msg(SYSCALL_MSG_DEST_UI, &msg, UI_API_FORCE_WAIT);
}

/**
 * 创建tty终端输出窗口
 */
ui_widget_t ui_tty_widget_create(int width, int height, ui_widget_t parent) {
    ui_api_msg_t msg;

    msg.func = UI_API_TTY_WIDGET_CREATE;
    msg.param[0] = (int) width;
    msg.param[1] = (int) height;
    msg.param[2] = (int) parent;

    if (sys_send_msg(SYSCALL_MSG_DEST_UI, &msg, 1) == 0) {
        return (ui_widget_t)msg.ret;
    }

    return (ui_widget_t ) 0;
}

/**
 * 获取tty终端窗口的宽度
 */
int ui_tty_widget_cols(ui_widget_t widget) {
    ui_api_msg_t msg;

    msg.func = UI_API_TTY_WIDGET_COLS;
    msg.param[0] = (int) widget;
    if (sys_send_msg(SYSCALL_MSG_DEST_UI, &msg, 1) == 0) {
        return (int)msg.ret;
    }

    return  0;
}

/**
 * 设置定时器回调函数
 */
void ui_set_timer_callback (timer_callback_t cb) {
	timer_cb = cb;
}

/**
 * 设置按键回调函数
 */
void ui_set_key_callback (key_callback_t cb) {
	key_cb = cb;
}

/**
 * UI事件循环
 * 仅当进入该函数时，才开始消息循环，否则不接收消息
 * 不然，有可能之前收到的消息也会被处理
 */
void ui_loop_event (void) {
	app_msg_t msg;

	// todo: 允许任务接收UI消息，同时清空所有的消息

	// 循环等待消息事件，直至收到退出消息或者出错
	do {
		// 取UI消息
		int err = sys_get_event(&msg);
		if (err < 0) {
			break;
		}

		// 处理各种消息
		switch (msg.type) {
		case APP_MSG_TYPE_TIMER:
			if (timer_cb) {
				err = timer_cb((int)msg.timer, msg.data);
			}
			break;
		case APP_MSG_TYPE_KEYBOARD:
			if (key_cb) {
				err = key_cb((ui_widget_t)0, msg.key);
			}
			break;
		case APP_MSG_TYPE_QUIT:
			return;
		default:
			break;
		}

		// 有错误就退出
		if (err < 0) {
			break;
		}
	}while (1);

	// todo: 关闭消息接收
}
