/**
 * 内核初始化以及测试代码
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "comm/boot_info.h"

int test (int a, int b) {
    return a + b;
}

/**
 * 内核入口
 */
void kernel_init (boot_info_t * boot_info) {
    int a = 1, b = 2;
    test(a , b);

    for (;;) {}
}
