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

static pde_t kernel_page_dir[PDE_CNT] __attribute__((aligned(PAGE_SIZE))); // 内核页目录表

extern uint8_t text_start[], text_end[], data_start[], data_end[];
extern uint8_t kernel_base_addr[];

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

pte_t * find_pte (pde_t * page_dir, uint32_t vaddr, int alloc) {
    pte_t * page_table;

    pde_t *pde = page_dir + pde_index(vaddr);
    if (pde->present) {
        page_table = (pte_t *)pde_paddr(pde);
    } else {
        // 如果不存在，则考虑分配一个
        if (alloc == 0) {
            return (pte_t *)0;
        }

        // 分配一个物理页表
        uint32_t pg_paddr = addr_alloc_page(&paddr_alloc, 1);
        if (pg_paddr == 0) {
            return (pte_t *)0;
        }
        pde->v = pg_paddr | PTE_P | PTE_W | PTE_U;      // 暂设置为用户可读写

        // 为物理页表绑定虚拟地址的映射，这样下面就可以计算出虚拟地址了
        //kernel_pg_last[pde_index(vaddr)].v = pg_paddr | PTE_P | PTE_W;

        // 清空页表，防止出现异常
        // 这里虚拟地址和物理地址一一映射，所以直接写入
        page_table = (pte_t *)(pg_paddr);
        kernel_memset(page_table, 0, PAGE_SIZE);
    }

    return page_table + pte_index(vaddr);
}

/**
 * @brief 将指定的地址空间进行一页的映射
 */
int memory_create_map (pde_t * page_dir, uint32_t vaddr, uint32_t paddr, int count, uint32_t perm) { 
    static int j = 0; 
    for (int i = 0; i < count; i++) {
        pte_t * pte = find_pte(page_dir, vaddr, 1);
        if (pte == (pte_t *)0) {
            return -1;
        }

        // 创建映射的时候，这条pte应当是不存在的。
        // 如果存在，说明可能有问题
        ASSERT(pte->present == 0);

        pte->v = paddr | perm | PTE_P;

        vaddr += PAGE_SIZE;
        paddr += PAGE_SIZE;
    }
}  

/**
 * @brief 根据内存映射表，构造内核页表
 */
void create_kernel_table (void) {
    // 地址映射表, 用于建立内核级的地址映射
    static memory_map_t kernel_map[] = {
        {kernel_base_addr,  text_start,     0,              PTE_W},        // 内核栈区
        {text_start,        text_end,       text_start,     0},             // 代码区
        {data_start,        (void *)(MEMORY_EBDA_START - 1),   data_start,        PTE_W},      // 数据区

        // // 扩展存储空间一一映射，方便直接操作
        {(void *)MEM_EXT_START, (void *)MEMORY_EXT_MAX_END, (void *)MEM_EXT_START, PTE_W},      
    };

    // 清空页目录表
    kernel_memset(kernel_page_dir, 0, sizeof(kernel_page_dir));
    
    // 清空后，然后依次根据映射关系创建映射表
    for (int i = 0; i < sizeof(kernel_map) / sizeof(memory_map_t); i++) {
        memory_map_t * map = kernel_map + i;

        // 可能有多个页，建立多个页的配置
        // 简化起见，不考虑4M的情况
        int vstart = down_2bound((uint32_t)map->vaddr_start, PAGE_SIZE);
        int vend = up_2bound((uint32_t)map->vaddr_end, PAGE_SIZE);
        int page_count = down_2bound((uint32_t)(vend - vstart), PAGE_SIZE) / PAGE_SIZE;
        memory_create_map(kernel_page_dir, vstart, (uint32_t)map->paddr, page_count, map->perm);
    }
}

/**
 * @brief 创建进程的初始页表
 * 主要的工作创建页目录表，然后从内核页表中复制一部分
 */
uint32_t memory_create_uvm (void) {
    pde_t * page_dir = (pde_t *)addr_alloc_page(&paddr_alloc, 1);
    if (page_dir == 0) {
        return 0;
    }
    kernel_memset((void *)page_dir, 0, PAGE_SIZE);

    // 复制整个内核空间的页目录项，以便与其它进程共享内核空间
    // 用户空间的内存映射暂不处理，等加载程序时创建
    uint32_t user_pde_start = pde_index(MEMORY_PROC_BASE);
    for (int i = 0; i < user_pde_start; i++) {
        page_dir[i].v = kernel_page_dir[i].v;
    }

    return (uint32_t)page_dir;
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
    total_mem = down_2bound(total_mem, PAGE_SIZE);   // 对齐到4KB页
    addr_alloc_init(&paddr_alloc, mem_free, PADDR_ALLOC_START, total_mem, PAGE_SIZE);

    // 再分配一些空间用于内核虚拟地址池，也放在低端1MB内存区域。1GB只需要32KB
    mem_free += bitmap_byte_count(paddr_alloc.size / PAGE_SIZE);
    addr_alloc_init(&kvaddr_alloc, mem_free, (uint32_t)kernel_base_addr, SIZE_1GB, PAGE_SIZE);

    // 最后mem不能超过ebda区域
    mem_free += bitmap_byte_count(kvaddr_alloc.size / PAGE_SIZE);
    ASSERT(mem_free < (kernel_base_addr + MEMORY_EBDA_START));

    // 创建内核页表并切换过去
    create_kernel_table();
    
    // 先切换到当前页表
    mmu_set_page_dir((uint32_t)kernel_page_dir);
}

