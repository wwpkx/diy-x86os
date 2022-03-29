#include "core/types.h"
#include "core/list.h"

/**
 * 初始化列表
 */
void list_init_with_nodes(list_t * list, void * elem, int elem_size, int count) {
	uint8_t * p_elem = (uint8_t *)elem;

	list_init(list);
	for (int i = 0; i < count; i++) {
		list_node_t * node = (list_node_t *)p_elem;
		list_insert_last(list, node);

		p_elem += elem_size;
	}
}

/**
 * 将指定表项插入到指定链表的头部
 * @param list 待插入的链表
 * @param node 待插入的结点
 */
void list_insert_first(list_t *list, list_node_t *node) {
    node->next = list->first;

    if (list->first == (list_node_t *)0) {
        list->last = list->first = node;
    } else {
        list->first = node;
    }

    list->count++;
}

/**
 * 移除指定链表的头部
 * @param list 操作的链表
 * @return 链表的第一个结点
 */
list_node_t* list_remove_first(list_t *list) {
    list_node_t* node;

    // 表项为空
    if (list_is_empty(list)) {
        return (list_node_t*)0;
    }

    // 取第一个结点
    node = list->first;

    // 将first往表尾移1个，跳过刚才移过的那个，如果没有后继，则first=0
    list->first = node->next;

    // 如果原链表只有一个结点，则此时移除后，表应该为空了
    if (list->last == node) {
        list->last = (list_node_t*)0;
    }

    // 调整node自己，置0，因为没有后继结点
    node->next = (list_node_t*)0;

    // 同时调整计数值
    list->count--;

    return node;
}

/**
 * 将指定表项插入到指定链表的尾部
 * @param list 操作的链表
 * @param node 待插入的结点
 */
void list_insert_last(list_t *list, list_node_t *node) {
    node->next = (list_node_t*)0;

    // 表空，则first/last都指向唯一的node
    if (list_is_empty(list)) {
        list->first = list->last = node;
    } else {
        // 否则，调整last结点的向一指向为node
        list->last->next = node;

        // node变成了新的后继结点
        list->last = node;
    }

    list->count++;
}

/**
 * 移除指定链表的中的表项
 * @param list 操作的链表
 * @param node 待移除的结点
 * @param 待移除的结点
 * @param 移除的结点，如果结点不在其中，返回0
 */
list_node_t * list_remove(list_t *list, list_node_t *node) {
    list_node_t* curr_node, * pre_node;

    pre_node = (list_node_t*)0;
    for (curr_node = list->first; curr_node != (list_node_t*)0; curr_node = curr_node->next) {
        if (curr_node == node) {
        	if (pre_node != (list_node_t*)0) {    // 头结点，去掉当前结点
                pre_node->next = curr_node->next;
            }

            if (list->first == node) {
                list->first = node->next;
            }

            if (list->last == node) {
                list->last = pre_node;
            }

            node->next = (list_node_t*)0;
            --list->count;
            return node;
        }

        pre_node = curr_node;
    }

    // 不应该运行到这里，但是如果确实运行到，调试状态下死机，方便查找问题
    return (list_node_t*)0;
}

/**
 * 将Node插入指定结点之后
 * @param 操作的链表
 * @param pre 前一结点
 * @param node 待插入的结点
 */
void list_insert_after(list_t *list, list_node_t *pre, list_node_t *node) {
    // node的下一结点，应当为pre的下一结点
    node->next = pre->next;

    // 调整Pre的下一结点为node
    pre->next = node;

    // 如果pre恰好位于表尾，则新的表尾就需要更新成node
    if (list->last == pre) {
        list->last = node;
    }

    list->count++;
}

/**
 * 将pre之后的node移除
 * @param 操作的链表
 * @param pre 前一结点
 * @param node 待移除的结点
 */
void list_remove_after(list_t *list, list_node_t *pre, list_node_t *node) {
    if (pre) {
        pre->next = node->next;
        list->count--;
    } else {
        // 没有前一结点，则node应当为首结点
        list_remove_first(list);
    }

    node->next = (list_node_t*)0;
}

/**
 * 实现对整个链表中所有结点的遍历，遍历时对于每一个结点调用trav_func
 * 在遍历过程中，不允许对链表进行修改操作
 *
 * @param list 待遍历的链表
 * @param trav_func 访问函数
 */
void list_trav(list_t *list, trav_func_t trav_func) {
    if (list->count == 0) {
    } else {
        list_node_t * curr;

        for (curr = list->first; curr; curr = curr->next) {
            if (trav_func(list, curr)) {
                return;
            }
        }
    }
}

/**
 * 在整个链表中查找第一个遍历指定条件的结点返回
 * @param list 待查找的结点
 * @param find_fuc 比较函数
 * @return 找到的结点
 */
list_node_t * list_find(list_t *list, find_func_t find_fuc, void *param) {
    if (list->count == 0) {
    } else {
        list_node_t * curr;

        for (curr = list->first; curr; curr = curr->next) {
            if (find_fuc(list, curr, param)) {
                return curr;
            }
        }
    }

    return (list_node_t *)0;
}

/**
 * 实现条件插入
 *
 * 遍历链表中的所有结点，判断条件是否满足，如果满足则将结点插入到指定位置
 *
 * @param list 待插入的链表
 * @param node 待插入的结点
 * @param cond 判断条件
 * @return 0 插入成功，非0 插入失败
 */
int list_insert_cond(list_t *list, list_node_t *node, insert_func_t insert_func, void *param) {
    int space_count = list->count + 1;  // 空位的数量，总是比实际结点数+1
    list_node_t * pre = (list_node_t*)0;
    list_node_t * next = list->first;

    // 从第1个结点开始进行比较
    while (space_count-- > 0) {
        if (insert_func(list, param, pre, node, next)) {
            if (pre) {
                list_insert_after(list, pre, node);
            } else {
                list_insert_first(list, node);
            }
            return 0;
        }

        pre = next;
        next = next->next;
    }

    return -1;
}

/**
 * 实现条件删除
 *
 * 遍历链表中的所有结点，判断条件是否满足，如果满足则将结点从链表中移除
 *
 * @param list 待删除的链表
 * @param acc_fun 遍历函数
 * @return 移除的结点数量
 */
int list_move_cond(list_t *from_list, list_t *to_list,
                   move_cond_func_t remove_func, void *param, xnet_list_mv_t mv) {
    uint32_t removed_count = 0;

    if (from_list->count == 0) {
        return 0;
    } else {
        list_node_t *pre = (list_node_t *) 0;
        list_node_t *curr, *next;

        for (curr = from_list->first; curr; pre = curr, curr = next) {
            next = curr->next;

            if (!remove_func(from_list, curr, param)) {
                continue;
            }

            list_remove_after(from_list, pre, curr);

            if (to_list) {
                if (mv == XNET_LIST_MV_FRONT) {
                    list_insert_first(to_list, curr);
                } else {
                    list_insert_last(to_list, curr);
                }
            }}
    }

    return removed_count;

}

/**
 * 将结点移到表头
 * @param list 结点所在的链表
 * @param node 移动的结点
 */
void list_move_to_front(list_t *list, list_node_t *node) {
    if (list->first == node) {
        return;
    }

    list_remove(list, node);
    list_insert_first(list, node);
}
