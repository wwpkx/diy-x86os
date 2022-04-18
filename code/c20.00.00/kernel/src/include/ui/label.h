/**
 * 标签实现
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef UI_LABLE_H
#define UI_LABLE_H

#include <core/os_cfg.h>
#include <ui/widget.h>

/**
 * 标签结构
 */
typedef struct _label_t {
    widget_t widget;
    char content[LABEL_CONTENT_MAX_LEN];	// 文字显示区
    int last_width, last_height;
}label_t;

label_t * label_create (const char * content, int width, int height,
		widget_t * parent, task_t * owner);
void label_set_text(label_t * label, const char * content);

#endif // UI_LABLE_H
