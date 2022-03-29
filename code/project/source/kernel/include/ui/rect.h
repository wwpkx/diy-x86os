/**
 * 矩形区域实现
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef UI_RECT_H
#define UI_RECT_H

#include "core/types.h"

typedef struct _rect_t {
    int x, y, width, height;
}rect_t;

static inline void rect_init (rect_t * rect, int x, int y, int width, int height) {
    rect->x = x;
    rect->y = y;
    rect->height = height;
    rect->width = width;
}

static inline void rect_copy (rect_t * dest, rect_t * src) {
    dest->x = src->x;
    dest->y = src->y;
    dest->height = src->height;
    dest->width = src->width;
}

static inline void rect_setx(rect_t * rect, int x) {
	rect->x = x;
}

static inline void rect_sety(rect_t * rect, int y) {
	rect->y = y;
}

static inline int rect_x (rect_t * rect) {
    return rect->x;
}

static inline int rect_y (rect_t * rect) {
    return rect->y;
}

static inline void rect_set_width(rect_t * rect, int width) {
	rect->width = width;
}

static inline void rect_set_height(rect_t * rect, int height) {
	rect->height = height;
}

static inline int rect_width (rect_t * rect) {
    return rect->width;
}

static inline int rect_height (rect_t * rect) {
    return rect->height;
}

static inline int rect_endx (rect_t * rect) {
    return rect->x + rect->width - 1;
}

static inline int rect_endy (rect_t * rect) {
    return rect->y + rect->height - 1;
}

static inline int rect_contain_xy (rect_t * rect, int x, int y) {
    return ((x >= rect->x) && (x < rect->x + rect->width)
            && ((y >= rect->y) && (y < rect->y + rect->height )));
}

int rect_get_overlap(rect_t * rect1, rect_t * rect2, rect_t * dest_rect);
void rect_merge (rect_t * rect1, rect_t * rect2, rect_t * dest_rect);
int rect_is_overlap (rect_t * rect1, rect_t * rect2);
int rect_contain_rect (rect_t * container, rect_t * rect);

#endif //UI_RECT_H
