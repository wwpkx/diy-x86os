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
static addr_alloc_t kvaddr_alloc;       // 物理地址分配结构
static pde_t kernel_pde_table[PDE_CNT]; // 内核页目录表

extern uint8_t data[];                  // 内核数据起始
extern uint8_t text[];                  // 内核代码起始

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

#define PDE_START_ADDR      0xFFC00000

/**
 * @brief 将指定的地址空间进行一页的映射
 */
void memory_create_map (pde_t * page_dir, uint32_t vaddr, uint32_t paddr, uint32_t perm) {    
    // 获取当前页表的地址, 在pde和pte中进行映射
    pde_t * pde = get_pde(page_dir, vaddr);
    if (pde->present) {
        // PTE表存在，则定位到PTE表项进行添加
        pte_t * pte = get_pte(page_dir, vaddr);
        pte->v = paddr | perm;
    } else {
        // 不存在，则分配一个Page Table，然后进行更新
        uint32_t pg_paddr = addr_alloc_page(&paddr_alloc, 1);
        pde->v = pg_paddr | perm;      // 更新pde表项

        // 新分配的表，需要在memory中建立映射关系
        uint32_t pg_vaddr = (uint32_t)get_page_table(page_dir, vaddr);
        memory_create_map((pde_t *)PDE_START_ADDR, pg_vaddr, pg_paddr, perm);

        // 清空，避免无效数据影响
        pte_t * pte_start = get_page_table(page_dir, vaddr);
        kernel_memset(pte_start, 0, PAGE_SIZE);

        // 最后写入新值
        pte_t * pte = pte_start + pte_index(vaddr);
        pte->v = paddr | perm;
    }
}

/**
 * @brief 分配一页内核页面，并返回虚拟地址
 */
static uint32_t alloc_kernel_page (uint32_t perm, int clear) {
    uint32_t vaddr = addr_alloc_page(&kvaddr_alloc, 1);
    if (vaddr == 0) {
        return vaddr;
    } 

    uint32_t paddr = addr_alloc_page(&paddr_alloc, 1);
    if (paddr == 0) {
        goto failed;
    }

    memory_create_map(PDE_START_ADDR, vaddr, paddr, perm);
    return vaddr;

    if (clear) {
        kernel_memset((void *)vaddr, 0, PAGE_SIZE);
    }
failed:
    return 0;
}

/**
 * @brief 根据内存映射表，建立pde页表
 */
uint32_t create_kernel_table (void) {
    // 地址映射表, 用于建立内核级的地址映射
    static memory_map_t kernel_map[] = {
        {SYS_KERNEL_BASE_ADDR,      0,   text - SYS_KERNEL_BASE_ADDR,   PTE_W},       // 内核栈区
        {text, text-SYS_KERNEL_BASE_ADDR, MEMORY_EBDA_START-SYS_KERNEL_BASE_ADDR, 0},      // 代码和只读数据区
    };

    // 清空其它表项，设置页表自己定位到PDE_START_ADDR开始处
    // kernel_memset(kernel_pde_table, 0, sizeof(kernel_pde_table));

    // // 清空后，然后依次根据映射关系创建映射表
    // for (int i = 0; i < sizeof(kernel_map) / sizeof(memory_map_t); i++) {
    //     memory_map_t * map = kernel_map + i;

    //     // 可能有多个页，建立多个页的配置
    //     int page_count = map->size / PAGE_SIZE;
    //     for (int i = 0; i < page_count; i++) {
    //         memory_create_map(page_dir, map->vaddr, map->paddr, map->perm);
    //     }
    // }

    return 0;
}

/**
 * @brief 初始化内存管理系统
 * 该函数的主要任务：
 * 1、初始化物理内存分配器：将所有物理内存管理起来
 * 2、重新创建内核页表：原loader中创建的页表已经不再合适
 */
void memory_init (boot_info_t * boot_info) { 
    // 空闲物理块的分配：将1M以上的空间用来分配
    uint8_t * mem_free =(uint8_t *) mem_free_start;
    uint32_t total_mem = 128 * 1024 * 1024; // total_mem_size(boot_info);

    // 4GB大小需要总共4*1024*1024*1024/4096/8=128KB的位图, 使用低1MB的RAM空间中足够
    total_mem = DOWN_BOUND(total_mem, PAGE_SIZE);   // 对齐到4KB页
    addr_alloc_init(&paddr_alloc, mem_free, PADDR_ALLOC_START, total_mem, PAGE_SIZE);

    // 再分配一些空间用于内核虚拟地址池，也放在低端1MB内存区域。1GB只需要32KB
    mem_free += bitmap_byte_count(paddr_alloc.size / PAGE_SIZE);
    addr_alloc_init(&kvaddr_alloc, mem_free, SYS_KERNEL_BASE_ADDR, SIZE_1GB, PAGE_SIZE);

    // 最后mem不能超过ebda区域
    mem_free += bitmap_byte_count(kvaddr_alloc.size / PAGE_SIZE);
    ASSERT(mem_free < (uint8_t *)(SYS_KERNEL_BASE_ADDR + MEMORY_EBDA_START));

    uint32_t addr = create_kernel_table();
    mmu_set_page_dir(addr);
}

