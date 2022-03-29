/**
 * ascii字符及其显示
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef OS_UI_ASCII_FONT_H
#define OS_UI_ASCII_FONT_H

#include "core/types.h"

/**
 * 字体描述
 */
typedef struct {
    int width, height;
    uint8_t * font_data;
}ui_font_t;

ui_font_t * ui_get_font(void);
void ui_font_width_height(int max_width, ui_font_t * font, const char * string,
                           int * width, int * height);

#endif //OS_UI_ASCII_FONT_H
