/**
 * 简单的命令行解释器
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "lib_syscall.h"

int main (int argc, char **argv) {
    for (int i = 0; i < argc; i++) {
        print_msg("arg: %s", (int)argv[i]);
    }
    
    for (;;) {
        msleep(1000);
    }
}