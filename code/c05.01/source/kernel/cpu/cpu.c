/**
 * CPU设置
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "cpu/cpu.h"
#include "os_cfg.h"

static gdt_descriptor_t gdt_table[GDT_TABLE_SIZE];

/**
 * 设置段描述符
 */
void gdt_segment_desc_set(gdt_descriptor_t *desc, uint32_t base, uint32_t limit, uint16_t attr) {
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
 * 初始化GDT
 */
void init_gdt(void) {
	// 全部清空
    for (int i = 0; i < GDT_TABLE_SIZE; i++) {
        gdt_segment_desc_set(gdt_table + i, 0, 0, 0);
    }
}

/**
 * CPU初始化
 */
void cpu_init (void) {
    init_gdt();
}
