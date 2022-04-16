/**
 * 内存管理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "core/memory.h"
#include "tools/klib.h"
#include "cpu/mmu.h"
#include "dev/console.h"

extern uint8_t mem_free_start[];        // 内核末端空闲ram

static addr_alloc_t paddr_alloc;        // 物理地址分配结构

static pde_t kernel_page_dir[PDE_CNT] __attribute__((aligned(MEM_PAGE_SIZE))); // 内核页目录表

/**
 * @brief 获取当前页表地址
 */
static pde_t * current_page_dir (void) {
    return (pde_t *)task_current()->tss.cr3;
}

/**
 * @brief 初始化地址分配结构
 * 以下不检查页边界，由上层调用者检查
 */
void addr_alloc_init (addr_alloc_t * alloc, uint8_t * bitmap_bis, uint32_t start, uint32_t size, uint32_t page_size) {
    mutex_init(&paddr_alloc.mutex);

    alloc->start = start;
    alloc->size = size / page_size * page_size;     // 不够一页的丢掉
    alloc->page_size = page_size;
    bitmap_init(&alloc->bitmap, bitmap_bis, alloc->size / page_size, 0);
}

/**
 * @brief 分配多页内存
 */
uint32_t addr_alloc_page (addr_alloc_t * alloc, int page_count) {
    uint32_t addr = 0;

    mutex_lock(&alloc->mutex);

    int page_index = bitmap_alloc_nbits(&alloc->bitmap, 0, page_count);
    if (page_index >= 0) {
        addr = alloc->start + page_index * alloc->page_size;
    }

    mutex_unlock(&alloc->mutex);
    return addr;
}

/**
 * @brief 释放多页内存
 */
void addr_free_page (addr_alloc_t * alloc, uint32_t addr, int page_count) {
    mutex_lock(&alloc->mutex);

    uint32_t page_addr = (addr - alloc->start) / alloc->page_size;
    bitmap_set_bit(&alloc->bitmap, page_addr, page_count, 0);

    mutex_unlock(&alloc->mutex);
}

/**
 * @brief 获取可用的物理内存大小
 */
static uint32_t total_mem_size(boot_info_t * boot_info) {
    int mem_size = 0;
    for (int i = 0; i < boot_info->ram_region_count; i++) {
        mem_size += boot_info->ram_region_cfg[i].size;
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
        kernel_memset(page_table, 0, MEM_PAGE_SIZE);
    }

    return page_table + pte_index(vaddr);
}

/**
 * @brief 将指定的地址空间进行一页的映射
 */
int memory_create_map (pde_t * page_dir, uint32_t vaddr, uint32_t paddr, int count, uint32_t perm) {
    for (int i = 0; i < count; i++) {
        pte_t * pte = find_pte(page_dir, vaddr, 1);
        if (pte == (pte_t *)0) {
            return -1;
        }

        // 创建映射的时候，这条pte应当是不存在的。
        // 如果存在，说明可能有问题
        ASSERT(pte->present == 0);

        pte->v = paddr | perm | PTE_P;

        vaddr += MEM_PAGE_SIZE;
        paddr += MEM_PAGE_SIZE;
    }

    return 0;
}

/**
 * @brief 根据内存映射表，构造内核页表
 */
void create_kernel_table (void) {
    extern uint8_t text_start[], text_end[], data_start[], data_end[];
    extern uint8_t kernel_base_addr[];

    // 地址映射表, 用于建立内核级的地址映射
    static memory_map_t kernel_map[] = {
        {kernel_base_addr,  text_start - 1,     0,                  PTE_W},        // 内核栈区
        {text_start,        text_end - 1,       text_start,         0},             // 代码区
        {data_start,        (void *)(MEM_EBDA_START - 1),   data_start,        PTE_W},      // 数据区
        {(void *)CONSOLE_VIDEO_BASE, (void *)(CONSOLE_VIDEO_END - 1), (void *)CONSOLE_VIDEO_BASE, PTE_W},

        // // 扩展存储空间一一映射，方便直接操作
        {(void *)MEM_EXT_START, (void *)MEM_EXT_END,     (void *)MEM_EXT_START, PTE_W},
    };

    // 清空页目录表
    kernel_memset(kernel_page_dir, 0, sizeof(kernel_page_dir));

    // 清空后，然后依次根据映射关系创建映射表
    for (int i = 0; i < sizeof(kernel_map) / sizeof(memory_map_t); i++) {
        memory_map_t * map = kernel_map + i;

        // 可能有多个页，建立多个页的配置
        // 简化起见，不考虑4M的情况
        int vstart = down_2bound((uint32_t)map->vaddr_start, MEM_PAGE_SIZE);
        int vend = up_2bound((uint32_t)map->vaddr_end, MEM_PAGE_SIZE);
        int page_count = down_2bound((uint32_t)(vend - vstart), MEM_PAGE_SIZE) / MEM_PAGE_SIZE;

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
    kernel_memset((void *)page_dir, 0, MEM_PAGE_SIZE);

    // 复制整个内核空间的页目录项，以便与其它进程共享内核空间
    // 用户空间的内存映射暂不处理，等加载程序时创建
    uint32_t user_pde_start = pde_index(MEMORY_TASK_BASE);
    for (int i = 0; i < user_pde_start; i++) {
        page_dir[i].v = kernel_page_dir[i].v;
    }

    return (uint32_t)page_dir;
}

/**
 * @brief 销毁用户空间内存
 */
void memory_destroy_uvm (uint32_t page_dir) {
    uint32_t user_pde_start = pde_index(MEMORY_TASK_BASE);
    pde_t * pde = (pde_t *)page_dir + user_pde_start;

    ASSERT(page_dir != 0);

    // 释放页表中对应的各项，不包含映射的内核页面
    for (int i = user_pde_start; i < PDE_CNT; i++, pde++) {
        if (!pde->present) {
            continue;
        }

        // 释放页表对应的物理页 + 页表
        pte_t * pte = (pte_t *)pde_paddr(pde);
        for (int j = 0; j < PTE_CNT; j++, pte++) {
            if (!pte->present) {
                continue;
            }

            addr_free_page(&paddr_alloc, pte_paddr(pte), 1);
        }

        addr_free_page(&paddr_alloc, (uint32_t)pde_paddr(pde), 1);
    }

    // 页目录表
    addr_free_page(&paddr_alloc, page_dir, 1);
}

/**
 * @brief 复制页表及其所有的内存空间
 */
uint32_t memory_copy_uvm (uint32_t page_dir) {
    // 复制基础页表
    uint32_t to_page_dir = memory_create_uvm();
    if (to_page_dir == 0) {
        goto copy_uvm_failed;
    }

    // 再复制用户空间的各项
    uint32_t user_pde_start = pde_index(MEMORY_TASK_BASE);
    pde_t * pde = (pde_t *)page_dir + user_pde_start;

    // 遍历用户空间页目录项
    for (int i = user_pde_start; i < PDE_CNT; i++, pde++) {
        if (!pde->present) {
            continue;
        }

        // 遍历页表
        pte_t * pte = (pte_t *)pde_paddr(pde);
        for (int j = 0; j < PTE_CNT; j++, pte++) {
            if (!pte->present) {
                continue;
            }

            // 分配物理内存
            uint32_t page = addr_alloc_page(&paddr_alloc, 1);
            if (page == 0) {
                goto copy_uvm_failed;
            }

            // 建立映射关系
            uint32_t vaddr = (i << 22) | (j << 12);
            int err = memory_create_map((pde_t *)to_page_dir, vaddr, page, 1, get_pte_perm(pte));
            if (err < 0) {
                goto copy_uvm_failed;
            }

            // 复制内容
            kernel_memcpy((void *)page, (void *)vaddr, MEM_PAGE_SIZE);
        }
    }
    return to_page_dir;

copy_uvm_failed:
    if (to_page_dir) {
        memory_destroy_uvm(to_page_dir);
    }
    return -1;
}

/**
 * @brief 获取指定虚拟地址的物理地址
 * 如果转换失败，返回0。
 */
uint32_t memory_get_paddr (uint32_t page_dir, uint32_t vaddr) {
    pte_t * pte = find_pte((pde_t *)page_dir, vaddr, 0);
    if (pte == (pte_t *)0) {
        return 0;
    }

    return pte_paddr(pte) + (vaddr & (MEM_PAGE_SIZE - 1));
}

/**
 * @brief 从指定的页表空间中复制一些数据
 */
int memory_copy_uvm_data(uint8_t * to, uint32_t page_dir, uint32_t from, uint32_t size) {
  char *buf, *pa0;

    while(size > 0){
        // 转换得到物理地址
        uint32_t aligned_from = down_2bound(from, MEM_PAGE_SIZE);
        uint32_t paddr = memory_get_paddr(page_dir, aligned_from);  
        if (paddr == 0) {
            return -1;
        }     

        // 在当前页内可拷贝的数据量
        uint32_t poffset = from - aligned_from;
        uint32_t curr_size = MEM_PAGE_SIZE - poffset;
        if (curr_size > size) {
            curr_size = size;       // 如果比较大，超过页边界，则只拷贝此页内的
        }

        kernel_memcpy(to, (char *)paddr + poffset, curr_size);

        size -= curr_size;
        to += curr_size;
        from += curr_size;
  }

  return 0;
}

/**
 * @brief 为指定的虚拟地址空间分配多页内存
 */
int memory_alloc_page_for (uint32_t addr, uint32_t size, int perm) {
    uint32_t curr_vaddr = addr;
    int page_count = up_2bound(size, MEM_PAGE_SIZE) / MEM_PAGE_SIZE;

    for (int i = 0; i < page_count; i++) {
        uint32_t paddr = addr_alloc_page(&paddr_alloc, 1);
        if (paddr == 0) {
            return 0;
        }

        pde_t * page_dir = (pde_t *)task_current()->tss.cr3;
        int err = memory_create_map(page_dir, curr_vaddr, paddr, 1, perm);
        if (err < 0) {
            addr_free_page(&paddr_alloc, addr, i + 1);
            return -1;
        }

        curr_vaddr += MEM_PAGE_SIZE;
    }

    return 0;
}


/**
 * @brief 分配一页内存
 * 主要用于内核空间内存的分配，不用于进程内存空间
 */
uint32_t memory_alloc_page (void) {
    // 内核空间虚拟地址与物理地址相同
    return addr_alloc_page(&paddr_alloc, 1);
}

/**
 * @brief 释放一页内存
 */
void memory_free_page (uint32_t addr) {
    if (addr < MEMORY_TASK_BASE) {
        // 内核空间，直接释放
        addr_free_page(&paddr_alloc, addr, 1);
    } else {
        // 进程空间，还要释放页表
        pte_t * pte = find_pte(current_page_dir(), addr, 0);
        ASSERT((pte == (pte_t *)0) && pte->present);

        // 释放内存页
        addr_free_page(&paddr_alloc, pte_paddr(pte), 1);

        // 释放页表
        pte->v = 0;
    }
}

/**
 * @brief 初始化内存管理系统
 * 该函数的主要任务：
 * 1、初始化物理内存分配器：将所有物理内存管理起来
 * 2、重新创建内核页表：原loader中创建的页表已经不再合适
 */
void memory_init (boot_info_t * boot_info) {
    // 在内核数据后面放物理页位图
    uint8_t * mem_free =(uint8_t *)mem_free_start;
    uint32_t total_mem = total_mem_size(boot_info) - MEM_EXT_START;
    total_mem = down_2bound(total_mem, MEM_PAGE_SIZE);   // 对齐到4KB页

    // 4GB大小需要总共4*1024*1024*1024/4096/8=128KB的位图, 使用低1MB的RAM空间中足够
    addr_alloc_init(&paddr_alloc, mem_free, MEM_EXT_START, total_mem, MEM_PAGE_SIZE);
    mem_free += bitmap_byte_count(paddr_alloc.size / MEM_PAGE_SIZE);

    // 后面再放点什么
    ASSERT(mem_free < (uint8_t *)MEM_EBDA_START);

    // 创建内核页表并切换过去
    create_kernel_table();

    // 先切换到当前页表
    mmu_set_page_dir((uint32_t)kernel_page_dir);
}

/**
 * @brief 调整堆的内存分配，返回堆之前的指针
 * 
 * @param incr 
 * @return char* 
 */
char * sys_sbrk(int incr) {
    task_t * task = task_current();

    if (incr == 0) {
        return task->heap_top;
    } else if (incr > 0) {
        // 需要分配新内存，且要考虑是否跨页面
        uint32_t end = task->heap_top + incr;
        uint32_t start = task->heap_top;

        int size_in_page = start % MEM_PAGE_SIZE;
        if (size_in_page) {
            // 不足一页，调整一下bss就可以了
            if (size_in_page + incr < MEM_PAGE_SIZE) {
                task->heap_top += incr;
                return start;
            } else {
                // 超过一页，则调整本页内的, 接下来再调整后面页对齐的
                uint32_t curr_size = MEM_PAGE_SIZE - size_in_page;
                incr -= curr_size;
                start += curr_size;
            }
        }

        if (start % MEM_PAGE_SIZE == 0) {
            // 刚好一页, 则分配新页
            int err = memory_alloc_page_for(start, incr, PTE_P | PTE_U | PTE_W);
            if (err < 0) {
                return (char *)0;
            }

            // 实际分配得到的可能会比要求的多，因为页对齐
            uint32_t pre_top = task->heap_top;
            task->heap_top = start + up_2bound(incr, MEM_PAGE_SIZE);
            return pre_top;
        }
    } else {

    }
}
