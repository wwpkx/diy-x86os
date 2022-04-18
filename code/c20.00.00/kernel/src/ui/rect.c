/**
 * 矩形区域实现
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <core/klib.h>
#include <ui/rect.h>

/*
 * 将rect1和rect2合并成一个
 * 注意：如果rect1和rect2不重叠，则可能会合并成一个非常大的矩形
 */
void rect_merge (rect_t * rect1, rect_t * rect2, rect_t * dest_rect) {
	if (!rect1->width || !rect1->height) {
		// rect1为空，取rect2
		rect_copy(dest_rect, rect2);
	} else if (!rect2->width || !rect2->height) {
		// rect2为空，取rect1
		rect_copy(dest_rect, rect1);
	} else {
		// 获取合并后的起始x, y，结束x, y
	    // 起始x, y总是最小值
	    int start_x = k_min(rect1->x, rect2->x);
	    int start_y = k_min(rect1->y, rect2->y);

	    // 线束的x, y总是最大值
	    int rect1_endx = rect_endx(rect1);
	    int rect2_endx = rect_endx(rect2);
	    int end_x = k_max(rect1_endx, rect2_endx);

	    int rect1_endy = rect_endy(rect1);
	    int rect2_endy = rect_endy(rect2);
	    int end_y = k_max(rect1_endy, rect2_endy);

	    // 转换得到合并后的rect
	    rect_init(dest_rect, start_x, start_y, end_x - start_x + 1, end_y - start_y + 1);
	}
}

/**
 * 检测rect1和rect2是否交叉。包含重叠
 */
int rect_is_overlap (rect_t * rect1, rect_t * rect2) {
    rect_t * low_rect, * high_rect;

    // 宽度或高度为0，必然无重叠
    if (!rect1->width || !rect1->height || !rect2->width || !rect2->height) {
        return 0;
    }

    // 为方便计算，获得低位置较低的rect和较高的rect
    // 所谓的位置，即坐标低越靠近起点则越低，而非只人看到的屏幕

    // 先处理x方向上的
    if (rect1->x < rect2->x) {
        low_rect = rect1;
        high_rect = rect2;
    } else {
        low_rect = rect2;
        high_rect = rect1;
    }
    // 如果要交叉，低的rect的末坐标必然要比高的rect的起点x相同或更大。
    // 否则，即更小，则肯定是不交叉的
    if (rect_endx(low_rect) < high_rect->x) {
        return 0;
    }

    // 同样的方式处理y方向的
    if (rect1->y < rect2->y) {
        low_rect = rect1;
        high_rect = rect2;
    } else {
        low_rect = rect2;
        high_rect = rect1;
    }

    if (rect_endy(low_rect)  < high_rect->y) {
        return 0;
    }

    return 1;
}

/**
 * 获取rect1和rect2的交叉区域
 */
int rect_get_overlap(rect_t * rect1, rect_t * rect2, rect_t * dest_rect) {
    rect_t * low_rect, * high_rect;

    // 宽度或高度为0，必然无交叉区域
    if (!rect1->width || !rect1->height || !rect2->width || !rect2->height) {
        return -1;
    }

    // 处理x方向上的交叉宽度
    // 为方便计算，先调整获得x小的low_rect和大的hight_rect
    if (rect1->x < rect2->x) {
        low_rect = rect1;
        high_rect = rect2;
    } else {
        low_rect = rect2;
        high_rect = rect1;
    }

    // 交叉区域的x肯定是heigh_rect的起点x
    dest_rect->x = high_rect->x;

    // 宽度要分两种情况：
    // 第一种是high_rect在low_rect内，此时宽度由high_rect决定
    // 第二种是low_rect和high_rect部分重叠，宽度由low_rect决定
    dest_rect->width = low_rect->x + low_rect->width - high_rect->x;
    if (dest_rect->width < 0) {
    	// low_rect的末端x比high_rect起点小，x方向上无交叉
        return -1;
    } else if (dest_rect->width > high_rect->width) {
    	// 第一种情况
        dest_rect->width = high_rect->width;
    }

    // 处理y方向上的交叉宽度，方便同上
    if (rect1->y < rect2->y) {
        low_rect = rect1;
        high_rect = rect2;
    } else {
        low_rect = rect2;
        high_rect = rect1;
    }

    dest_rect->y = high_rect->y;
    dest_rect->height = low_rect->y + low_rect->height - high_rect->y;
    if (dest_rect->height < 0) {
        return -1;
    } else if (dest_rect->height > high_rect->height) {
        dest_rect->height = high_rect->height;
    }

    return 0;
}

/**
 * 判断container是否包含rect
 */
int rect_contain_rect (rect_t * container, rect_t * rect) {
    // 宽度或高度为0，必然无重叠
    if (!container->width || !container->height || !rect->width || !rect->height) {
        return 0;
    }

    // 宽度或高度比较小，必然无重叠
    if ((container->width < rect->width) || (container->height < rect->height)) {
    	return 0;
    }

    if ((container->x > rect->x) || (rect_endx(container) < rect_endx(rect))) {
    	return 0;
    }

    if ((container->y > rect->y) || (rect_endy(container) < rect_endy(rect))) {
    	return 0;
    }

    return 1;
}


