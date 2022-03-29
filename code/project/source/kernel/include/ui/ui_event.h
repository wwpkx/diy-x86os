/**
 * UI事件处理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef OS_MUI_H
#define OS_MUI_H

#include <ui/ui_color.h>
#include <core/os_cfg.h>
#include <lib_ui.h>
#include "core/types.h"
#include <ipc/sem.h>
#include <ipc/queue.h>
#include <dev/keyboard.h>
#include <core/task.h>

/**
 * 鼠标事件类型
 */
typedef struct _mouse_event_t {
    enum {
    	MOUSE_EVENT_MOVE, /**< MOUSE_EVENT_MOVE */
		MOUSE_EVENT_DRAG, /**< MOUSE_EVENT_DRAG */
		MOUSE_EVENT_PRESS,/**< MOUSE_EVENT_PRESS */
		MOUSE_EVENT_CLICK /**< MOUSE_EVENT_CLICK */

    }type;
    int dx, dy;
    int x, y;
}mouse_event_t;

/**
 * 键盘事件类型
 */
typedef struct _key_event_t {
    enum {
    	KEY_EVENT_PRESS,
		KEY_EVENT_RELEASE,
		KEY_EVENT_ENTERED,
    }type;
    key_data_t code;
}key_event_t;

struct _widget_t;
typedef int (*mouse_event_handler_t) (struct _widget_t * widget, mouse_event_t * event);
typedef int (*key_event_handler_t) (struct _widget_t * widget, key_event_t * event);

// UI的API消息类型
// 一旦写好后，顺序不能改，否则会影响到应用的调用
typedef enum _ui_api_func_t {
    UI_API_WIN_CREATE = 0x0001,
    UI_API_WIN_ADD_SUB = 0x0002,
	UI_API_WIN_CONTAINER = 0x0003,

    UI_API_WIDGET_MOVE_TO = 0x1000,
	UI_API_WIDGET_WIDTH = 0x1001,
	UI_API_WIDGET_HEIGHT = 0x1002,
	UI_API_WIDGET_CONTENT_WIDTH = 0x1003,
	UI_API_WIDGET_CONTENT_HEIGHT = 0x1004,
	UI_API_WIDGET_SET_VISIABLE = 0x1005,

    UI_API_LABEL_CREATE = 0x3000,
    UI_API_LABEL_SET_TEXT = 0x3001,

    UI_API_BUTTON_CREATE = 0x4000,
    UI_API_BUTTON_SET_TEXT = 0x4001,

	UI_API_PBWIDGET_CREATE = 0x5000,
	UI_API_PBWIDGET_DRAW_POINT = 0x5001,
	UI_API_PBWIDGET_DRAW_RECT = 0x5002,
	UI_API_PBWIDGET_DRAW_STRING = 0x5003,
	UI_API_PBWIDGET_DRAW_LINE = 0x5004,

	UI_API_TTY_WIDGET_CREATE = 0x6000,
	UI_API_TTY_WIDGET_COLS = 0x6001,

	UI_API_TTY_DRAW_RECT = 0x10000,
	UI_API_TTY_DRAW_TEXT = 0x10001,
	UI_API_TTY_SCROLL_UP = 0x10002,

	UI_API_REFRESH = 0x20003,
}ui_api_func_t ;

/**
 * UI的API消息结构
 */
typedef struct _ui_api_msg_t {
	ui_api_func_t func;				// 申请调用的功能
	int ret;						// 调用结果
	int param[UI_API_PARAM_COUNT];	// 参数，放在最后，方便扩充
}ui_api_msg_t;

/**
 * 消息类型
 */
typedef enum _ui_msg_type_t {
    UI_MSG_MOUSE = 0x0,
    UI_MSG_KEYBOARD,
    UI_MSG_API,

	UI_MSG_TIMER,
	UI_MSG_TTY,
	UI_MSG_MASK = 0xF,
}ui_msg_type_t;

/**
 * UI相关的消息：三种消息的全体
 */
typedef struct _ui_msg_t {
    queue_msg_t base;

    ui_msg_type_t type;

    union {
    	// 鼠标消息
        struct  {
            int left_pressed;
            int right_pressed;
            int center_pressed;
            int delta_x, delta_y;
        };

        // 按键消息
        struct  {
        	key_data_t code;
        };

        // API调用消息
        struct {
        	ui_api_msg_t api_msg;
        	task_t * from;
            sem_t * sem;					// 等待结果的信号量
        };
    };
}ui_msg_t;

void ui_server_init (void);
ui_msg_t *ui_alloc_msg(ui_msg_type_t type, int wait);
void ui_free_msg (ui_msg_t * msg);
void ui_send_msg(ui_msg_t *msg, task_t * from, int wait_ret);
void ui_notify_refresh (void);

#endif //OS_MUI_H
