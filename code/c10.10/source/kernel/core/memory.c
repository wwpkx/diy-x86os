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

#define PADDR_ALLOC_START           (1024*1024)
#define SIZE_4GB_BITS               (4*1024*1024*1024/PAGE_SIZE/8)

static bitmap_t paddr_bitmap;           // 物理地址分配位图
static uint8_t paddr_bits[128 * 1024];  // 
static addr_alloc_t paddr_alloc;        // 物理地址分配结构

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
    int page_index = bitmap_alloc_nbits(&alloc->bitmap, 1, page_count);
    if (page_index < 0) {
        return 0;
    }

    // 转换为实际地址后返回
    return alloc->start + page_index * alloc->page_size;
}

/**
 * @brief 释放多页内存
 */
void addr_alloc_free (addr_alloc_t * alloc, uint32_t addr, int page_count) {
    uint32_t page_addr = (addr - alloc->start) / alloc->page_size;
    bitmap_set_bit(&alloc->bitmap, page_addr, page_count, 0);
}

uint32_t paddr_alloc_page (int page_count) {
    return addr_alloc_page(&paddr_alloc, page_count);
}

void paddr_alloc_free (uint32_t addr, int page_count) {
    addr_alloc_free(&paddr_alloc, addr, page_count);
}

/**
 * @brief 初始化内存管理系统
 */
void memory_init (boot_info_t * boot_info) { 
    // 计算1MB以上可用的RAM量，然后建立地址分配器
    int up1M_size = 0;
    for (int i = 0; i < boot_info->ram_region_count; i++) {
        if (boot_info->ram_region_cfg[i].start >= PADDR_ALLOC_START) {
            up1M_size += boot_info->ram_region_cfg[i].size;
        }
    }

    // 考虑中间有间隔的？
    //addr_alloc_init(&paddr_alloc, paddr_bits, PADDR_ALLOC_START, up1M_size, PAGE_SIZE);
    
    page_init();
    // page_add_remap(0, 1024*1024, 0);
}

