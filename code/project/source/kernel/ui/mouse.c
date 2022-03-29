/**
 * 鼠标实现
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <ui/mouse.h>
#include <ui/ui_core.h>

// 只支持一个鼠标
static mouse_t mouse_widget;

/**
 * 鼠标图标
 */
void painter_mouse_default (struct _widget_t * widget) {
#define MOUSE_WIDTH     16
#define MOUSE_HIGHT     16
    static char cursor[MOUSE_HIGHT][MOUSE_WIDTH] = {
            "**************..",
            "*OOOOOOOOOOO*...",
            "*OOOOOOOOOO*....",
            "*OOOOOOOOO*.....",
            "*OOOOOOOO*......",
            "*OOOOOOO*.......",
            "*OOOOOOO*.......",
            "*OOOOOOOO*......",
            "*OOOO**OOO*.....",
            "*OOO*..*OOO*....",
            "*OO*....*OOO*...",
            "*O*......*OOO*..",
            "**........*OOO*.",
            "*..........*OOO*",
            "............*OO*",
            ".............***"
    };

    // 先调用父类的绘图
    painter_widget_default(widget);

    // 逐行画点的形式写入
    for (int y = 0; y < MOUSE_HIGHT; y++) {
        for (int x = 0; x < MOUSE_WIDTH; x++) {
            if (cursor[y][x] == '*') {
                widget_draw_point(widget, x, y, COLOR_Gray);
            } else if (cursor[y][x] == 'O') {
                widget_draw_point(widget, x, y, COLOR_White);
            }
        }
    }
}

/**
 * 鼠标部件初始化
 */
void ui_mouse_init(void) {
    widget_init((widget_t *)&mouse_widget, ui_screen_widget(),
    		MOUSE_WIDTH, MOUSE_HIGHT, (task_t *)0);

    mouse_widget.widget.paint = painter_mouse_default;
    mouse_widget.widget.type = WIDGET_TYPE_MOUSE;
    mouse_widget.widget.visiable = 1;
}

/**
 * 移动鼠标
 */
void ui_mouse_move_by(int dx, int dy) {
	// 没有移动
    if ((dx == 0) && (dy == 0)) {
        return;
    }

    // 调整x, y不能超出屏幕范围
    // 鼠标需要始终显示在屏幕上边
    int x = widget_x((widget_t *)&mouse_widget) + dx;
    if (x < 0) {
        x = 0;
    } else if (x >= ui_screen_width() - MOUSE_EDGE_MARGIN) {
        x = ui_screen_width() - MOUSE_EDGE_MARGIN;
    }

    int y = widget_y((widget_t *)&mouse_widget) + dy;
    if (y < 0) {
        y = 0;
    } else if (y >= ui_screen_height() - MOUSE_EDGE_MARGIN) {
        y = ui_screen_height() - MOUSE_EDGE_MARGIN;
    }

    widget_move_to(&mouse_widget.widget, x, y);
}

/**
 * 获取鼠标的x坐标
 */
int ui_mouse_getx(void) {
    return mouse_widget.widget.rect.x;
}

/**
 * 获取鼠标的y坐标
 */
int ui_mouse_gety(void) {
    return mouse_widget.widget.rect.y;
}

/**
 * 获取鼠标部件
 */
widget_t * ui_mouse_widget (void) {
    return (widget_t *)&mouse_widget;
}
