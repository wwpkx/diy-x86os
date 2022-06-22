/**
 * 位图数据结构
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "tools/bitmap.h"
#include "tools/klib.h"

/**
 * @brief 获取所需要的字节数量
 */
int bitmap_byte_count (int bit_count) {
    return (bit_count + 8 - 1) / 8;         // 向上取整
}

/**
 * @brief 位图初始化
 */
void bitmap_init (bitmap_t * bitmap, uint8_t * bits, int count, int init_bit) {
    bitmap->bit_count = count;
    bitmap->bits = bits;

    int bytes = bitmap_byte_count(bitmap->bit_count);
    kernel_memset(bitmap->bits, init_bit ? 0xFF: 0, bytes);
}
