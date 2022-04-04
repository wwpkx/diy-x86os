/**
 * MMU与分布处理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */

#include "cpu/mmu.h"
#include "cpu/cpu.h"
#include "comm/boot_info.h"
#include "comm/cpu_instr.h"
#include "tools/klib.h"

#define PDE_ENTRY_SIZE              1024        // PDE表项数量
#define PTE_ENTRY_SIZE              1024        // PTE表项数量

static pde_t kernel_page_dir[PDE_ENTRY_SIZE] __attribute__((aligned(PAGE_SIZE)));   // 内核页目录表
static pte_t kernel_page_table_0[PTE_ENTRY_SIZE] __attribute__((aligned(PAGE_SIZE)));

/**
 * @brief 初始化分页机制
 * 完整的页目录+页表，需要4KB + 4KB*1024 = 4KB + 4MB
 */
void page_init (void) {
    uint32_t addr = 0;

    // 设置第1个页表项，即1MB以下的空间
    kernel_memset(kernel_page_table_0, 0, sizeof(kernel_page_table_0));
    for (int i = 0; (i < PTE_ENTRY_SIZE) && (addr < ADDR_1MB); i++) {
        pte_t * pte = kernel_page_table_0 + i;

        pte->present = 1;
        pte->phy_page_addr = addr >> 12;

        addr += PAGE_SIZE;       
    }

    // 设置第0项，指向第1个页表
    kernel_memset(kernel_page_dir, 0, sizeof(kernel_page_dir));
    pde_t * pde = kernel_page_dir + 0;
    pde->present = 1;
    pde->phy_pt_addr = ((uint32_t)kernel_page_table_0) >> 12;      // 只取高20位

    // 开启MMU
    write_cr3((uint32_t)kernel_page_dir);
    uint32_t cr0 = read_cr0();
    write_cr0(cr0 | CR0_PG);
}

/**
 * @brief 将指定的物理地址映射到虚拟地址处
 * 
 * @param paddr 
 * @param size 
 * @param vaddr 
 */
void page_add_remap(uint32_t paddr, uint32_t size, uint16_t vaddr) {

}
