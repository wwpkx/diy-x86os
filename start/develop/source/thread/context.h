#ifndef CONTEXT_H
#define CONTEXT_H

#include "comm/types.h"

#define THREAD_STACK_DEF_SIZE       8192        // 默认栈的大小

// 线程入口函数类型
typedef void *  (*thread_func_t)(void * arg);

typedef uint32_t thread_stack_t;       // 栈类型

/**
 * @brief 线程运行状态结构
 */
typedef struct _thread_state_t {
    // 线程切换时保存的状态信息
    thread_stack_t edi;
    thread_stack_t esi;
    thread_stack_t ebx;
    thread_stack_t ebp;
    thread_stack_t entry;
}thread_state_t;

/**
 * @brief 线程运行上下文
 */
typedef struct _context_t {
    thread_stack_t * stack_top;                // 当前栈顶指针
    thread_stack_t * stack_start;              // 栈起始地址
    int stack_size;                     // 栈大小，以字节计
}context_t;

context_t * context_create (int stack_size);
int context_swap (context_t * from, context_t * to);
int context_set (context_t * to);
void context_free (context_t * context);

void swap_context (thread_stack_t ** from, thread_stack_t * to);
void restore_context (thread_stack_t * to);

#endif
