/**
 * CPU设置
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "comm/cpu_instr.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "os_cfg.h"

static segment_desc_t gdt_table[GDT_TABLE_SIZE];

/**
 * 设置段描述符
 */
void segment_desc_set(segment_desc_t *desc, uint32_t base, uint32_t limit, uint16_t attr) {
	// 如果界限比较长，将长度单位换成4KB
	if (limit > 0xfffff) {
		attr |= 0x8000;
		limit /= 0x1000;
	}
	desc->limit15_0 = limit & 0xffff;
	desc->base15_0 = base & 0xffff;
	desc->base23_16 = (base >> 16) & 0xff;
	desc->attr = attr | (((limit >> 16) & 0xf) << 8);
	desc->base31_24 = (base >> 24) & 0xff;
}

/**
 * 设置门描述符
 */
void gate_desc_set(gate_desc_t *desc, uint16_t selector, uint32_t offset, uint16_t attr) {
	desc->offset15_0 = offset & 0xffff;
	desc->selector = selector;
	desc->attr = attr;
	desc->offset31_16 = (offset >> 16) & 0xffff;
}

/**
 * 分配一个GDT推荐表符
 */
segment_desc_t * gdt_alloc_desc (void) {
    for (int i = 1; i < GDT_TABLE_SIZE; i++) {
        segment_desc_t * desc = gdt_table + i;
        if ((desc->attr & (0xF)) == 0) {
            return desc;
        }
    }

    return (segment_desc_t *)0;
}

/**
 * 分配一个GDT推荐表符
 */
int gdt_alloc_segment (uint32_t base, uint32_t limit, uint16_t attr) {
    segment_desc_t * desc = (segment_desc_t *)0;

    irq_state_t state = irq_enter_protection();

    for (int i = 1; i < GDT_TABLE_SIZE; i++) {
        desc = gdt_table + i;
        if ((desc->attr & (0xF)) == 0) {
            // 如果界限比较长，将长度单位换成4KB
            if (limit > 0xfffff) {
                attr |= 0x8000;
                limit /= 0x1000;
            }
            desc->limit15_0 = limit & 0xffff;
            desc->base15_0 = base & 0xffff;
            desc->base23_16 = (base >> 16) & 0xff;
            desc->attr = attr | (((limit >> 16) & 0xf) << 8);
            desc->base31_24 = (base >> 24) & 0xff;
            break;
        }
    }

    irq_leave_protection(state);
    return desc;
}

/**
 * GDT描述符转换为索引
 */
uint16_t desc_2_gdt_selector(segment_desc_t * desc) {
    return (desc - gdt_table) * sizeof(segment_desc_t);
}


/**
 * 初始化GDT
 */
void init_gdt(void) {
	// 全部清空
    for (int i = 0; i < GDT_TABLE_SIZE; i++) {
        segment_desc_set(gdt_table + i, 0, 0, 0);
    }

    //数据段
    segment_desc_set(gdt_table + (KERNEL_SELECTOR_DS >> 3), 0x00000000, 0xFFFFFFFF,
                     SEG_P_PRESENT | SEG_DPL0 | SEG_S_NORMAL | SEG_TYPE_DATA | SEG_TYPE_RW |
                     SEG_D);

    // 只能用非一致代码段，以便通过调用门更改当前任务的CPL执行关键的资源访问操作
    segment_desc_set(gdt_table + (KERNEL_SELECTOR_CS >> 3), 0x00000000, 0xFFFFFFFF,
                     SEG_P_PRESENT | SEG_DPL0 | SEG_S_NORMAL | SEG_TYPE_CODE | SEG_TYPE_RW | SEG_D);


    // 加载gdt
    lgdt((uint32_t)gdt_table, sizeof(gdt_table));
}

/**
 * 切换至TSS，即跳转实现任务切换
 */
void switch_to_tss (uint32_t tss_selector) {
    far_jump(tss_selector, 0);
}

/**
 * CPU初始化
 */
void cpu_init (void) {
    init_gdt();
}
