/**
 * 中断处理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "cpu/irq.h"
#include "cpu/cpu.h"
#include "comm/cpu_instr.h"
#include "os_cfg.h"

#define IDT_TABLE_NR			128				// IDT表项数量

static gate_descriptor_t idt_table[IDT_TABLE_NR];	// 中断描述表

static void do_default_handler (const char * message) {
    for (;;) {hlt();}
}

void do_handler_divider(void) {
	do_default_handler("Device Error.");
}

void irq_init(void) {	
	for (uint32_t i = 0; i < sizeof(idt_table) / sizeof(gate_descriptor_t); i++) {
        set_gate_desc(idt_table + i, 0, 0, 0);
	}

	// 设置异常处理接口
	irq_install(IRQ0_DE, (irq_handler_t)handler_divider);

	lidt((uint32_t)idt_table, sizeof(idt_table));
}

int irq_install(int irq_num, irq_handler_t handler) {
	if (irq_num >= IDT_TABLE_NR) {
		return -1;
	}

    set_gate_desc(idt_table + irq_num, KERNEL_SELECTOR_CS | GDT_RPL3, (uint32_t) handler,
                  GDT_GATE_PRESENT | GDT_GATE_DPL0 | GDT_GATE_TYPE_IDT);
	return 0;
}