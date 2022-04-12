/**
 * MMU与分布处理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef MMU_H
#define MMU_H

#include "comm/types.h"

// 页目录属性值
#define PTE_P           (1 << 0)
#define PTE_W           (1 << 1)
#define PTE_U           (1 << 2)
#define PTE_PWT         (1 << 3)
#define PTE_PCD         (1 << 4)
#define PTE_A           (1 << 5)
#define PTE_D           (1 << 6)
#define PTE_PS          (1 << 7)

#define PDE_CNT             1024
#define PTE_CNT             1024



#endif // MMU_H
