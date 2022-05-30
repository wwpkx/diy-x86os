/**
 * 进程启动C部分代码
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "lib_syscall.h"

int main (int argc, char ** argv);

/**
 * @brief 应用的初始化，C部分
 */
void cstart (int argc, char ** argv) {
    main(argc, argv);
}