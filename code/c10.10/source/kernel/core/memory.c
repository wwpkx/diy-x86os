/**
 * 内存管理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "core/memory.h"
#include "cpu/mmu.h"
#include "tools/bitmap.h"
#include "tools/klib.h"
#include "os_cfg.h"

#define PADDR_ALLOC_START           (1024*1024)

extern uint8_t mem_free_start[];        // 内核末端空闲ram
static addr_alloc_t paddr_alloc;        // 物理地址分配结构
static addr_alloc_t kvaddr_alloc;        // 物理地址分配结构

/**
 * @brief 初始化地址分配结构
 */
void addr_alloc_init (addr_alloc_t * alloc, uint8_t * bitmap_bis, uint32_t start, uint32_t size, uint32_t page_size) {
    alloc->start = start;
    alloc->size = size / page_size * page_size;     // 不够一页的丢掉
    alloc->page_size = page_size;
    bitmap_init(&alloc->bitmap, bitmap_bis, alloc->size / page_size, 0);
}

/**
 * @brief 分配多页内存
 */
uint32_t addr_alloc_page (addr_alloc_t * alloc, int page_count) {
    int page_index = bitmap_alloc_nbits(&alloc->bitmap, 0, page_count);
    if (page_index < 0) {
        return 0;
    }

    // 转换为实际地址后返回
    return alloc->start + page_index * alloc->page_size;
}

/**
 * @brief 释放多页内存
 */
void addr_free_page (addr_alloc_t * alloc, uint32_t addr, int page_count) {
    uint32_t page_addr = (addr - alloc->start) / alloc->page_size;
    bitmap_set_bit(&alloc->bitmap, page_addr, page_count, 0);
}

/**
 * @brief 获取空闲的物理内存大小
 */
static uint32_t total_mem_size(boot_info_t * boot_info) {
    // 计算1MB以上可用的RAM量，然后建立地址分配器
    int mem_size = 0;
    for (int i = 0; i < boot_info->ram_region_count; i++) {
        if (boot_info->ram_region_cfg[i].start >= PADDR_ALLOC_START) {
            mem_size += boot_info->ram_region_cfg[i].size;
        }
    }

    return mem_size;
}

#define PDE_TEMP_ADDR       0xFF800000
#define PDE_START_ADDR      0xFFC00000

// /**
//  * @brief 将指定的地址空间进行一页的映射
//  */
// void memory_create_map (pde_t * page_dir, uint32_t vaddr, uint32_t paddr, uint32_t perm) {    
//     // 获取当前页表的地址, 在pde和pte中进行映射
//     pde_t * pde = get_pde(page_dir, vaddr);
//     if (pde->present) {
//         // PTE表存在，则定位到PTE表项进行添加
//         pte_t * pte = get_pte(page_dir, vaddr);
//         pte->v = paddr | perm;
//     } else {
//         // 不存在，则分配一个Page Table，然后进行更新
//         uint32_t pg_paddr = addr_alloc_page(&paddr_alloc, 1);
//         pde->v = pg_paddr | perm;      // 更新pde表项

//         // 新分配的表，需要在memory中建立映射关系
//         uint32_t pg_vaddr = (uint32_t)get_page_table(page_dir, vaddr);
//         memory_create_map((pde_t *)PDE_START_ADDR, pg_vaddr, pg_paddr, perm);

//         // 清空，避免无效数据影响
//         pte_t * pte_start = get_page_table(page_dir, vaddr);
//         kernel_memset(pte_start, 0, PAGE_SIZE);

//         // 最后写入新值
//         pte_t * pte = pte_start + pte_index(vaddr);
//         pte->v = paddr | perm;
//     }
// }

// /**
//  * @brief 根据内存映射表，建立pde页表
//  */
// uint32_t memory_create_kvm (void) {
//     // 地址映射表, 用于建立内核级的地址映射
//     static memory_map_t kernel_map[] = {
//         {SYS_KERNEL_BASE_ADDR,      0,   SYS_BIOS_END_ADDR,   0},       // 低1MB，包含了内核区域
//     };

//     // 分配内存，创建新页表，并映射到某个固定位置处
//     uint32_t page_dir_paddr = addr_alloc_page(&paddr_alloc, 1);
//     if (page_dir_paddr == 0) {
//         return 0;
//     }
//     memory_create_map((pde_t *)PDE_START_ADDR, PDE_TEMP_ADDR, page_dir_paddr, PTE_P | PTE_W);

//     // 清空其它表项，设置页表自己定位到PDE_START_ADDR开始处
//     pde_t * page_dir = (pde_t *)PDE_TEMP_ADDR;
//     kernel_memset(page_dir, 0, sizeof(pde_t) * 1024);
//     page_dir[1023].present = 1;
//     page_dir[1023].phy_pt_addr = page_dir_paddr >> 22;

//     // 清空后，然后依次根据映射关系创建映射表
//     for (int i = 0; i < sizeof(kernel_map) / sizeof(memory_map_t); i++) {
//         memory_map_t * map = kernel_map + i;

//         // 可能有多个页，建立多个页的配置
//         int page_count = map->size / PAGE_SIZE;
//         for (int i = 0; i < page_count; i++) {
//             memory_create_map(page_dir, map->vaddr, map->paddr, map->perm);
//         }
//     }

//     return page_dir_paddr;
// }

// 与loader不同，这里无需再处理0x0 -> 0x0的映射, 因为已经在高地址处运行
static uint32_t page_dir[1024] __attribute__((aligned(4096))) = {
    [SYS_KERNEL_BASE_ADDR >> 22] = PTE_P | PTE_PS | PTE_W,	 // 开启4MB的页
};

// 这两个仍然需要
static uint32_t pte_1023[1024] __attribute__((aligned(4096))) = {
    [0] = (uint32_t)page_dir,				// 0xFFC00000指向page_dir
    [1023] = (uint32_t)pte_1023,			// 0xFFFF000指向自己
};

/**
 * @brief 创建核心页表
 */
static void create_kvm (void) {
    // 遍历页目录表中的各项，为其分配页表
    for (int i = 769; i < 1023; i++) {
        // 分配各项页表，然后用pde指向，并映射到
        uint32_t page_table = addr_alloc_page(&paddr_alloc, 1);

        // 为页表绑定到高端虚拟地址
        uint32_t vaddr = 1024 * PAGE_SIZE * i;
        pte_t * pte = get_pte(PDE_START_ADDR, vaddr);
        pte->v = page_table | PTE_W | PTE_P;

        // 清空页表，避免数据的影响
        kernel_memset((void *)pte, 0, PAGE_SIZE);

        // 更新页表映射
        page_dir[i] = page_table | PTE_W | PTE_P;
    }
}

/**
 * @brief 初始化内存管理系统
 * 该函数的主要任务：
 * 1、初始化物理内存分配器：将所有物理内存管理起来
 * 2、重新创建内核页表：原loader中创建的页表已经不再合适
 * 3、
 */
void memory_init (boot_info_t * boot_info) { 
    // 空闲物理块的分配：将1M以上的空间用来分配
    uint32_t total_mem = 128 * 1024 * 1024; // total_mem_size(boot_info);
    addr_alloc_init(&paddr_alloc, 
                mem_free_start,         // 使用内核的空间用作位图，够用
                PADDR_ALLOC_START,      // 分配1MB以上的物理内存
                total_mem,    // 总的大小
                PAGE_SIZE);     // 以页为单位分配
    
    // 先切换至新页表，这样在调试create_kvm时，方便在qemu中查看页表映射的变化
    page_dir[1023] = PTE_P | PTE_W | (((uint32_t)pte_1023 - SYS_KERNEL_BASE_ADDR) & 0xFFFFF000);
	pte_1023[0] |= PTE_P | PTE_W;
	pte_1023[1023] |= PTE_P | PTE_W;

    mmu_set_page_dir((uint32_t)page_dir - SYS_KERNEL_BASE_ADDR);
    create_kvm();
}

