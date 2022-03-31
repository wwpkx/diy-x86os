/**
 * 简单的链表
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef LIST_H
#define LIST_H

/**
 * 链表结点类型
 */
typedef struct _list_node_t {
    struct _list_node_t* pre;           // 链表的前一结点
    struct _list_node_t* next;         // 后继结点
}list_node_t;

/**
 * 头结点的初始化
 * @param node 待初始化的结果
 */
static inline void list_node_init(list_node_t *node) {
    node->pre = node->next = (list_node_t *)0;
}

/**
 * 获取结点的前一结点
 * @param node 查询的结点
 * @return 后继结点
 */
static inline list_node_t * list_node_pre(list_node_t *node) {
    return node->pre;
}

/**
 * 获取结点的前一结点
 * @param node 查询的结点
 * @return 后继结点
 */
static inline list_node_t * list_node_next(list_node_t *node) {
    return node->next;
}

/**
 * 带头结点和尾结点的单链表
 * 每个结点只需要一个指针，用于减少内存使用量
 */
typedef struct _list_t {
    list_node_t * first;            // 头结点
    list_node_t * last;             // 尾结点
    int count;                        // 结点数量
}list_t;

void list_init(list_t *list);

/**
 * 判断链表是否为空
 * @param list 判断的链表
 * @return 1 - 空，0 - 非空
 */
static inline int list_is_empty(list_t *list) {
    return list->count == 0;
}

/**
 * 获取链表的结点数量
 * @param list 查询的链表
 * @return 结果的数据
 */
static inline int list_count(list_t *list) {
    return list->count;
}

/**
 * 获取指定链表的第一个表项
 * @param list 查询的链表
 * @return 第一个表项
 */
static inline list_node_t* list_first(list_t *list) {
    return list->first;
}

/**
 * 获取指定链接的最后一个表项
 * @param list 查询的链表
 * @return 最后一个表项
 */
static inline list_node_t* list_last(list_t *list) {
    return list->last;
}

void list_insert_first(list_t *list, list_node_t *node);
void list_insert_last(list_t *list, list_node_t *node);
list_node_t* list_remove_first(list_t *list);
list_node_t* list_remove(list_t *list, list_node_t *node);

#endif /* LIST_H */
