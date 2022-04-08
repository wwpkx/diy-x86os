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

#define CR3_PWT             (1 << 3)            // Page-level write-through
#define CR3_PCD             (1 << 4)            // Page-level cache disable

#define PAGE_SIZE           (4*1024)            // 页大小
#define ADDR_1MB            (1024*1024)

#pragma pack(1)

// Page table/directory entry flags.
#define PTE_P           0x001   // Present
#define PTE_W           0x002   // Writeable
#define PTE_U           0x004   // User
#define PTE_PWT         0x008   // Write-Through
#define PTE_PCD         0x010   // Cache-Disable
#define PTE_A           0x020   // Accessed
#define PTE_D           0x040   // Dirty
#define PTE_PS          0x080   // Page Size
#define PTE_MBZ         0x180   // Bits must be zero

/**
 * @brief Page-Table Entry
 */
typedef union _pde_t {
    uint32_t v;
    struct {
        uint32_t present : 1;                   // 0 (P) Present; must be 1 to map a 4-KByte page
        uint32_t write_disable : 1;             // 1 (R/W) Read/write, if 0, writes may not be allowe
        uint32_t user_mode_acc : 1;             // 2 (U/S) if 0, user-mode accesses are not allowed t
        uint32_t write_through : 1;             // 3 (PWT) Page-level write-through
        uint32_t cache_disable : 1;             // 4 (PCD) Page-level cache disable
        uint32_t accessed : 1;                  // 5 (A) Accessed
        uint32_t : 1;                           // 6 Ignored;
        uint32_t ps : 1;                        // 7 (PS)
        uint32_t : 4;                           // 11:8 Ignored
        uint32_t phy_pt_addr : 20;              // 高20位page table物理地址
    };
}pde_t;

/**
 * @brief Page-Table Entry
 */
typedef union _pte_t {
    uint32_t v;
    struct {
        uint32_t present : 1;                   // 0 (P) Present; must be 1 to map a 4-KByte page
        uint32_t write_disable : 1;             // 1 (R/W) Read/write, if 0, writes may not be allowe
        uint32_t user_mode_acc : 1;             // 2 (U/S) if 0, user-mode accesses are not allowed t
        uint32_t write_through : 1;             // 3 (PWT) Page-level write-through
        uint32_t cache_disable : 1;             // 4 (PCD) Page-level cache disable
        uint32_t accessed : 1;                  // 5 (A) Accessed;
        uint32_t dirty : 1;                     // 6 (D) Dirty
        uint32_t pat : 1;                       // 7 PAT
        uint32_t global : 1;                    // 8 (G) Global
        uint32_t : 3;                           // Ignored
        uint32_t phy_page_addr : 20;            // 高20位物理地址
    };
}pte_t;

#pragma pack()

/**
 * @brief 返回vaddr在页目录中的索引
 */
static inline int pde_index (uint32_t vaddr) {
    return (vaddr >> 22) & 0x3FF;   // 取高10位
}

/**
 * @brief 获取pde中地址
 */
static inline uint32_t pde_paddr (pde_t * pde) {
    return pde->phy_pt_addr << 22;
}

/**
 * @brief 返回vaddr在页表中的索引
 */
static inline int pte_index (uint32_t vaddr) {
    return (vaddr >> 12) & 0x3FF;   // 取中间10位
}

/**
 * @brief 获取pte中的物理地址
 */
static inline uint32_t pte_paddr (pte_t * pte) {
    return pte->phy_page_addr << 12;
}

/**
 * @brief 获取指定虚拟地址对应的pde表项
 */
static inline pde_t * get_pde(uint32_t page_dir, uint32_t vaddr) {
    uint32_t offset = pde_index(vaddr) * sizeof(pde_t);
    return (pde_t *)(page_dir + offset);
}

/**
 * @brief 获取虚拟地址所在的页表起始
 */
static inline pte_t * get_page_table (uint32_t page_dir, uint32_t vaddr) {
    // 跳过最开始的页目录表
    uint32_t offset = pde_index(vaddr) * sizeof(pde_t);
    return (pte_t *)(page_dir + PAGE_SIZE + offset);
}

/**
 * @brief 获取指定虚拟地址对应的pte表项
 */
static inline pte_t * get_pte (uint32_t page_dir, uint32_t vaddr) {
    pte_t * pte_table = get_page_table(page_dir, vaddr);
    return  pte_table + pte_index(vaddr);
}

void mmu_set_page_dir (uint32_t paddr);

#endif // MMU_H
