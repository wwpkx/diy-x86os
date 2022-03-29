/**
 * 简单的链表
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef SRC_INCLUDE_CORE_LIST_H_
#define SRC_INCLUDE_CORE_LIST_H_

/**
 * 链表结点类型
 */
typedef struct _list_node_t {
    struct _list_node_t* next;         // 后继结点
}list_node_t;

/**
 * 头结点的初始化
 * @param node 待初始化的结果
 */
static inline void list_node_init(list_node_t *node) {
    node->next = (list_node_t *)0;
}

/**
 * 获取结点的后继结点
 * @param node 查询的结点
 * @return 后继结点
 */
static inline list_node_t * list_node_next(list_node_t *node) {
    return node->next;
}

/**
 * 设置结点的后续结点
 * @param pre 前一结点
 * @param after 后一结点
 */
static inline void list_node_set_next(list_node_t *pre, list_node_t *after) {
    pre->next = after;
}


// 已知结构体中的某个字段的指针，求所在结构体的指针
// 例如：
// struct aa{
//  .....
//  int node;
//  .....
// };
// struct aa a;
// 1.求结点在所在结构中的偏移:定义一个指向0的指针，用(struct aa *)&0->node，所得即为node字段在整个结构体的偏移
#define offset_in_parent(parent_type, node_name)    \
    ((char *)&(((parent_type*)0)->node_name))

// 2.求node所在的结构体首址：node的地址 - node的偏移
// 即已知a->node的地址，求a的地址
#define offset_to_parent(node, parent_type, node_name)   \
    ((char *)node - offset_in_parent(parent_type, node_name))

// 3. 进行转换: (struct aa *)addr
// 使用方式：node_to_parent(node_addr, struct aa, node)
#define node_to_parent(node, parent_type, node_name)   \
        ((parent_type *)(node ? offset_to_parent((node), parent_type, node_name) : 0))

/**
 * 带头结点和尾结点的单链表
 * 每个结点只需要一个指针，用于减少内存使用量
 */
typedef struct _list_t {
    struct _list_node_t* first;        // 头结点
    struct _list_node_t* last;         // 尾结点
    int count;                              // 结点数量
}list_t;

/**
 * 初始化链表
 * @param list 待初始化的链表
 */
static inline void list_init(list_t *list) {
    list->first = list->last = (list_node_t*)0;
    list->count = 0;
}

void list_init_with_nodes(list_t * list, void * elem, int elem_size, int count);

/**
 * 判断链表是否为空
 * @param list 判断的链表
 * @return 1 - 空，0 - 非空
 */
static inline int list_is_empty(list_t *list) {
    return list->first == (list_node_t*)0;
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

/**
 * 获取链表的结点数量
 * @param list 查询的链表
 * @return 结果的数据
 */
static inline int list_count(list_t *list) {
    return list->count;
}

void list_insert_first(list_t *list, list_node_t *node);
list_node_t* list_remove_first(list_t *list);

void list_insert_last(list_t *list, list_node_t *node);
list_node_t* list_remove(list_t *list, list_node_t *node);

/**
 * 移除指定链表的中最后一个
 * @param list 操作的链表
 * @return 移除的表项
 */
static inline list_node_t* list_remove_last(list_t *list) {
    if (list->count == 0) {
        return (list_node_t*)0;
    }
    return list_remove(list, list->last);
}

void list_insert_after(list_t *list, list_node_t *pre, list_node_t *node);
void list_remove_after(list_t *list, list_node_t *pre, list_node_t *node);

/**
 * 遍历函数，当返回值为1时，终止后续的遍历
 */
typedef int (*trav_func_t)(list_t * list, list_node_t * node);
void list_trav(list_t *list, trav_func_t trav_func);

/**
 * 查找函数，当返回值为1时，表明找到结点，立即返回，终止后续遍历。为0时，找不到继续查找后续结点
 */
typedef int (*find_func_t)(list_t * list, list_node_t * node, void * param);
list_node_t * list_find(list_t *list, find_func_t find_fuc, void *param);

/**
 * 插入判断函数，返回1表示立即插入到pre和next之间后，终止遍历，为0表示继续遍历
 */
typedef int (*insert_func_t)(list_t * list, void * param,
        list_node_t * pre, list_node_t * my, list_node_t * next);
int list_insert_cond(list_t *list, list_node_t *node, insert_func_t insert_func, void *param);

/**
 * 删除判断判断，返回1表示立即删除当前结点，终止遍历。为0表示继续遍历
 */
typedef enum _xnet_list_mv_t {
    XNET_LIST_MV_FRONT,
    XNET_LIST_MV_TAIL,
}xnet_list_mv_t;

typedef int (*move_cond_func_t)(list_t * list, list_node_t * node, void * param);
int list_move_cond(list_t *from_list, list_t *to_list,
                   move_cond_func_t remove_func, void *param, xnet_list_mv_t mv);

void list_move_to_front(list_t *list, list_node_t *node);

#endif /* SRC_INCLUDE_CORE_LIST_H_ */
