/**
 * 进程启动C部分代码
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "lib_syscall.h"

/**
 * @brief 应用的初始化，C部分
 */
void cstart (int argc, char ** argv) {
    // 可以在此做点什么进一步的初始化
    exit(main(argc, argv));
}