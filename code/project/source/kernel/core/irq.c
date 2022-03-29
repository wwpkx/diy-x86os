/**
 * 中断处理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <core/task.h>
#include "core/irq.h"
#include "core/cpu.h"
#include "core/cpu_instr.h"
#include <core/syscall.h>
#include <core/os_cfg.h>
#include <core/klib.h>
#include <ui/ui_core.h>

#define IDT_TABLE_NR			128				// IDT表项数量
#define IRQ_PIC_START			0x20			// PIC中断起始号
#define IRQ_PIC_NR				8				// PIC的中断数量

static int irq_counter;						// 计数发生了多少次中断
static gate_descriptor_t idt_table[IDT_TABLE_NR];	// 中断描述表

static void init_pic(void) {
    outb(PIC0_ICW1, PIC_ICW_ALWAYS_1 | PIC_ICW1_ICW4);
    outb(PIC0_ICW2, IRQ_PIC_START);
    outb(PIC0_ICW3, 1 << 2);
    outb(PIC0_ICW4, PIC_ICW4_ALWAYS_1);

    outb(PIC1_ICW1, PIC_ICW1_ICW4 | PIC_ICW_ALWAYS_1);
    outb(PIC1_ICW2, IRQ_PIC_START + 8);
    outb(PIC1_ICW3, 2);
    outb(PIC1_ICW4, PIC_ICW4_ALWAYS_1);

    // 禁止所有中断
    outb(PIC0_IMR, 0xFF & ~(1 << 2));
    outb(PIC1_IMR, 0xFF);
}

void pic_send_eoi(int irq_num) {
	irq_num -= IRQ_PIC_START;

	// 从片也可能需要发送EOI
	if (irq_num >= IRQ_PIC_NR) {
		outb(PIC1_COMMAND, PIC_OCW2_EOI);
	}

	outb(PIC0_COMMAND, PIC_OCW2_EOI);
}

void pic_clear_all(void) {
	outb(PIC1_COMMAND, PIC_OCW2_EOI);
	outb(PIC0_COMMAND, PIC_OCW2_EOI);
}

static void do_default_handler (exception_frame_t * frame, const char * message) {
//	// 打印CPU寄存器相关内容
//    kernel_consle_printf(
//            "IRQ/Exception happend: %s."
//            "num: %d, "
//            "error code: %d. \n"
//            "cs: 0x%x, eip: 0x%x, eflags: 0x%x \n\n",
//            message, frame->num, frame->error_code,
//			frame->cs, frame->eip, frame->eflags);

    // 打印任务相关内容，可再详细点的信息打印
    task_t * task = task_current();
//    kernel_consle_printf("task name: %s", task->name);

//    kernel_consle_printf("\n\r");

    // 系统任务，只好死机。非系统进程，杀死
    if (task->flags & TASK_FLAG_SYSTEM_TASK) {
    	for (;;) {
        	hlt();
        }
    } else {
    	// todo: 杀死应用进程
    	for (;;) {hlt();}
    }
}

void do_handler_divider(exception_frame_t * frame) {
	do_default_handler(frame, "Device Error.");
}

void do_handler_Debug(exception_frame_t * frame) {
	do_default_handler(frame, "Debug Exception");
}

void do_handler_NMI(exception_frame_t * frame) {
	do_default_handler(frame, "NMI Interrupt.");
}

void do_handler_breakpoint(exception_frame_t * frame) {
	do_default_handler(frame, "Breakpoint.");
}

void do_handler_overflow(exception_frame_t * frame) {
	do_default_handler(frame, "Overflow.");
}

void do_handler_bound_range(exception_frame_t * frame) {
	do_default_handler(frame, "BOUND Range Exceeded.");
}

void do_handler_invalid_opcode(exception_frame_t * frame) {
	do_default_handler(frame, "Invalid Opcode.");
}

void do_handler_device_unavailable(exception_frame_t * frame) {
	do_default_handler(frame, "Device Not Available.");
}

void do_handler_double_fault(exception_frame_t * frame) {
	do_default_handler(frame, "Double Fault.");
}

void do_handler_invalid_tss(exception_frame_t * frame) {
	do_default_handler(frame, "Invalid TSS");
}

void do_handler_segment_not_present(exception_frame_t * frame) {
	do_default_handler(frame, "Segment Not Present.");
}

void do_handler_stack_segment_fault(exception_frame_t * frame) {
	do_default_handler(frame, "Stack-Segment Fault.");
}

void do_handler_general_protection(exception_frame_t * frame) {
	do_default_handler(frame, "General Protection.");
}

void do_handler_page_fault(exception_frame_t * frame) {
	do_default_handler(frame, "Page Fault.");
}

void do_handler_fpu_error(exception_frame_t * frame) {
	do_default_handler(frame, "X87 FPU Floating Point Error.");
}

void do_handler_alignment_check(exception_frame_t * frame) {
	do_default_handler(frame, "Alignment Check.");
}

void do_handler_machine_check(exception_frame_t * frame) {
	do_default_handler(frame, "Machine Check.");
}

void do_handler_smd_exception(exception_frame_t * frame) {
	do_default_handler(frame, "SIMD Floating Point Exception.");
}

void do_handler_virtual_exception(exception_frame_t * frame) {
	do_default_handler(frame, "Virtualization Exception.");
}

void irq_init(void) {
    irq_counter = 0;

	// 设置异常处理接口
	irq_install(IRQ0_DE, (irq_handler_t)handler_divider);
	irq_install(IRQ1_DB, (irq_handler_t)handler_Debug);
	irq_install(IRQ2_NMI, (irq_handler_t)handler_NMI);
	irq_install(IRQ3_BP, (irq_handler_t)handler_breakpoint);
	irq_install(IRQ4_OF, (irq_handler_t)handler_overflow);
	irq_install(IRQ5_BR, (irq_handler_t)handler_bound_range);
	irq_install(IRQ6_UD, (irq_handler_t)handler_invalid_opcode);
	irq_install(IRQ7_NM, (irq_handler_t)handler_device_unavailable);
	irq_install(IRQ8_DF, (irq_handler_t)handler_double_fault);
	irq_install(IRQ10_TS, (irq_handler_t)handler_invalid_tss);
	irq_install(IRQ11_NP, (irq_handler_t)handler_segment_not_present);
	irq_install(IRQ12_SS, (irq_handler_t)handler_stack_segment_fault);
	irq_install(IRQ13_GP, (irq_handler_t)handler_general_protection);
	irq_install(IRQ14_PF, (irq_handler_t)handler_page_fault);
	irq_install(IRQ16_MF, (irq_handler_t)handler_fpu_error);
	irq_install(IRQ17_AC, (irq_handler_t)handler_alignment_check);
	irq_install(IRQ18_MC, (irq_handler_t)handler_machine_check);
	irq_install(IRQ19_XM, (irq_handler_t)handler_smd_exception);
	irq_install(IRQ20_VE, (irq_handler_t)handler_virtual_exception);

	// 其余设置无效
	for (uint32_t i = IRQ20_VE + 1; i < sizeof(idt_table) / sizeof(gate_descriptor_t); i++) {
        set_gate_desc(idt_table + i, 0, 0, 0);
	}
	lidt((uint32_t)idt_table, sizeof(idt_table));

	init_pic();
}

int irq_install(int irq_num, irq_handler_t handler) {
	if (irq_num >= IDT_TABLE_NR) {
		return -1;
	}

    set_gate_desc(idt_table + irq_num, KERNEL_SELECTOR_CS | GDT_RPL3, (uint32_t) handler,
                  GDT_GATE_PRESENT | GDT_GATE_DPL0 | GDT_GATE_TYPE_IDT);
	return 0;
}

void irq_enable(int irq_num) {
    if (irq_num < IRQ_PIC_START) {
        return;
    }

	irq_num -= IRQ_PIC_START;
	if (irq_num < IRQ_PIC_NR) {
		uint8_t mask = inb(PIC0_IMR) & ~(1 << irq_num);
		outb(PIC0_IMR, mask);
	} else {
		irq_num -= IRQ_PIC_NR;
		uint8_t mask = inb(PIC1_IMR) & ~(1 << irq_num);
		outb(PIC1_IMR, mask);
	}
}

void irq_disable(int irq_num) {
    if (irq_num < IRQ_PIC_START) {
        return;
    }

	irq_num -= IRQ_PIC_START;
	if (irq_num < IRQ_PIC_NR) {
		uint8_t mask = inb(PIC0_IMR) | (1 << irq_num);
		outb(PIC0_IMR, mask);
	} else {
		irq_num -= IRQ_PIC_NR;
		uint8_t mask = inb(PIC1_IMR) | (1 << irq_num);
		outb(PIC1_IMR, mask);
	}
}

void irq_disable_global(void) {
	cli();
}

void irq_enable_global(void) {
	sti();
}

irq_state_t irq_enter_protection (void) {
    irq_state_t state;

    state = read_eflags();
    irq_disable_global();
    return state;
}

void irq_leave_protection (irq_state_t state) {
    write_eflags(state);
}

void irq_enter_handler (void) {
    irq_state_t  state = irq_enter_protection();
    irq_counter++;
    irq_leave_protection(state);
}

void irq_leave_handler (void) {
    irq_state_t  state = irq_enter_protection();

    if (irq_counter && (--irq_counter == 0)) {
        task_dispatch();

        irq_leave_protection(state);
    } else {
        irq_leave_protection(state);
    }
}

int irq_is_in (void) {
    return irq_counter;
}


