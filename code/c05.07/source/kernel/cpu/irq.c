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

static gate_desc_t idt_table[IDT_TABLE_NR];	// 中断描述表

static void do_default_handler (const char * message) {
    for (;;) {}
}

void do_handler_unknown (void) {
	do_default_handler("Unknown exception.");
}

/**
 * @brief 中断和异常初始化
 */
void irq_init(void) {	
	for (uint32_t i = 0; i < IDT_TABLE_NR; i++) {
    	gate_desc_set(idt_table + i, KERNEL_SELECTOR_CS, (uint32_t) exception_handler_unknown,
                  GATE_P_PRESENT | GATE_DPL0 | GATE_TYPE_IDT);
	}
	lidt((uint32_t)idt_table, sizeof(idt_table));
}

