/**
 * 应用入口
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <stdlib.h>

extern int main ();
 
void _start() {
    int err = main();
    exit(err);
}
