/**
 * MMU与分布处理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */

#include "cpu/mmu.h"
#include "comm/cpu_instr.h"

#define PDE_ENTRY_SIZE              1024        // PDE表项数量
#define PTE_ENTRY_SIZE              1024        // PTE表项数量

/**
 * @brief 重新加载整个页表
 * @param vaddr 页表的虚拟地址
 */
void mmu_set_page_dir (uint32_t paddr) {
    // 将虚拟地址转换为物理地址
    write_cr3(paddr);
}

/**
 * @brief 获取页表地址
 */
uint32_t mmu_get_page_dir (void) {
    uint32_t cr3 = read_cr3();
    return cr3;
}
