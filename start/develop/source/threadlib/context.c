#include <stdlib.h>
#include <stdio.h>
#include "context.h"
#include "thread.h"

/**
 * @brief 线程运行上下文初始化
 */
context_t * context_create (int stack_size) {
    context_t * context = (context_t *)malloc(sizeof(context_t));
    if (!context) {
        return (context_t *)0;
    }

    // 栈空间初始化。主线程无需设置，直接使用当前栈
    if (stack_size) {
        char * stack_start = (char *)calloc(1, stack_size);
        if (!stack_start) {
            free(context);
            return (context_t *)0;
        }

        context->stack_start = (stack_t *)stack_start;
        context->stack_top = (stack_t *)(stack_start + THREAD_STACK_DEF_SIZE - sizeof(thread_state_t));
        
        // 配置运行状态
        thread_state_t * stack_state = (thread_state_t *)context->stack_top;
        stack_state->edi = 0x1;     // 测试用
        stack_state->esi = 0x2;
        stack_state->ebx = 0x3;
        stack_state->ebp = 0x4;
        stack_state->entry = (stack_t)thread_entry; 
    } else {
        context->stack_start = (stack_t *)0;
        context->stack_top = (stack_t *)0;
        context->stack_size = 0;
    }
 
    return context;
}

/**
 * @brief 释放上下文结构
 */
void context_free (context_t * context) {
    free(context->stack_start);
    free(context);
}


/**
 * @brief 切换上下文
 */
int context_swap (context_t * from, context_t * to) {
    swap_context(&from->stack_top, to->stack_top);
}

/**
 * @brief 恢复下文
 */
int context_set (context_t * to) {
    restore_context(to->stack_top);
}