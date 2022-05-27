#include "tools/list.h"

/**
 * 初始化链表
 * @param list 待初始化的链表
 */
void list_init(list_t *list) {
    list->first = list->last = (list_node_t *)0;
    list->count = 0;
}
