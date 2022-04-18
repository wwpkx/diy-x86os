//
// Created by lishutong on 2021-07-16.
//

#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include <ui/widget.h>

#define LABEL_CONTENT_MAX_LEN      256

typedef struct _button_t {
    widget_t widget;
    char content[LABEL_CONTENT_MAX_LEN];
    rect_t text_rect;
}button_t;

button_t * button_create (const char * content, int width, int height, widget_t * parent, task_t * owner);
void button_set_text(button_t * label, const char * content);

#endif //UI_LABLE_H
