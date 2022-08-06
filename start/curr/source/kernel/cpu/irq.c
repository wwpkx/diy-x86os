#include "comm/cpu_instr.h"
#include "cpu/irq.h"
#include "cpu/cpu.h"
#include "os_cfg.h"
#include "tools/log.h"
#include "core/task.h"

#define IDE_TABLE_NR        128

void exception_handler_unknown (void);

static gate_desc_t idt_table[IDE_TABLE_NR];

static void dump_core_regs (exception_frame_t * frame) {
    uint32_t ss, esp;
    if (frame->cs & 0x3) {
        ss = frame->ss3;
        esp = frame->esp3;
    } else {
        ss = frame->ds;
        esp = frame->esp;
    }
    log_printf("IRQ: %d, error code: %d", frame->num, frame->error_code);
    log_printf("CS: %d\nDS: %d\nES: %d\nSS: %d\nFS:%d\nGS:%d",
        frame->cs, frame->ds, frame->es, ss, frame->gs
    );
    log_printf("EAX: 0x%x\n" 
        "EBX:0x%x\n"
        "ECX:0x%x\n"
        "EDX:0x%x\n"
        "EDI:0x%x\n"
        "ESI:0x%x\n"
        "EBP:0x%x\n"
        "ESP:0x%x\n",
        frame->eax, frame->ebx, frame->ecx, frame->edx,
        frame->edi, frame->esi, frame->ebp, esp
    );
    log_printf("EIP: 0x%x\nEFLAGS:0x%x\n", frame->eip, frame->eflags);
}


static void do_default_handler (exception_frame_t * frame, const char * message) {
    log_printf("---------------------------");
    log_printf("IRQ/Exception happend: %s", message);
    dump_core_regs(frame);
    
    // CS - CPL  0, 3
    if (frame->cs & 0x3) {
        sys_exit(frame->error_code);
    } else {
        while (1) {
            hlt();
        }    
    }
}

void do_handler_unknown (exception_frame_t * frame) {
    do_default_handler(frame, "unknown exception");
}

void do_handler_divider (exception_frame_t * frame) {
    do_default_handler(frame, "Divider exception");
}

void do_handler_Debug (exception_frame_t * frame) {
    do_default_handler(frame, "Debug exception");
}

void do_handler_NMI (exception_frame_t * frame) {
    do_default_handler(frame, "NMI exception");
}
void do_handler_breakpoint (exception_frame_t * frame) {
    do_default_handler(frame, "breakpoint exception");
}
void do_handler_overflow (exception_frame_t * frame) {
    do_default_handler(frame, "overflow exception");
}
void do_handler_bound_range (exception_frame_t * frame) {
    do_default_handler(frame, "bound range exception");
}
void do_handler_invalid_opcode (exception_frame_t * frame) {
    do_default_handler(frame, "invalid opcode exception");
}
void do_handler_device_unavabliable (exception_frame_t * frame) {
    do_default_handler(frame, "device unavabliable exception");
}
void do_handler_double_fault (exception_frame_t * frame) {
    do_default_handler(frame, "double fault exception");
}
void do_handler_invalid_tss (exception_frame_t * frame) {
    do_default_handler(frame, "invalid tss exception");
}
void do_handler_segment_not_present (exception_frame_t * frame) {
    do_default_handler(frame, "segment not present exception");
}
void do_handler_stack_segment_fault (exception_frame_t * frame) {
    do_default_handler(frame, "stack segment fault exception");
}
void do_handler_general_protection (exception_frame_t * frame) {
    log_printf("-------");
    log_printf("GP fault.");

    if (frame->error_code & ERR_EXT) {
        log_printf("the exception occurred during delivery of an event external to the program,");
    } else {
        log_printf("the exception occurred during delivery of a software interrupt (INT n, INT3, or INTO)");
    } 

    if (frame->error_code & ERR_IDT) {
        log_printf(" the index portion of the error code refers to a gate descriptor in the IDT");
    } else {
        log_printf(" indicates that the index refers to a descriptor in the GDT or the current LDT");
    }

    log_printf("selector index: %d", frame->error_code & 0xFFF8);

    dump_core_regs(frame);
    // CS - CPL  0, 3
    if (frame->cs & 0x3) {
        sys_exit(frame->error_code);
    } else {
        while (1) {
            hlt();
        }    
    }
}
void do_handler_page_fault (exception_frame_t * frame) {
    log_printf("-------");
    log_printf("Page fault.");

    if (frame->error_code & ERR_PAGE_P) {
        log_printf("The fault was caused by a page-level protection violation.: 0x%x", read_cr2());
    } else {
        log_printf("The fault was caused by a non-present page: 0x%x", read_cr2());
    }

    if (frame->error_code & ERR_PAGE_WR) {
        log_printf("The access causing the fault was a write.: 0x%x", read_cr2());
    } else {
        log_printf("The access causing the fault was a read: 0x%x", read_cr2());
    }

    if (frame->error_code & ERR_PAGE_US) {
        log_printf("A user-mode access caused the fault.: 0x%x", read_cr2());
    } else {
        log_printf("A supervisor-mode access caused the fault.: 0x%x", read_cr2());
    }  

    dump_core_regs(frame);

    // CS - CPL  0, 3
    if (frame->cs & 0x3) {
        sys_exit(frame->error_code);
    } else {
        while (1) {
            hlt();
        }    
    }
}
void do_handler_fpu_error (exception_frame_t * frame) {
    do_default_handler(frame, "fpu error exception");
}
void do_handler_aligment_check (exception_frame_t * frame) {
    do_default_handler(frame, "aligment check exception");
}
void do_handler_machine_check (exception_frame_t * frame) {
    do_default_handler(frame, "machine check exception");
}
void do_handler_simd_exception (exception_frame_t * frame) {
    do_default_handler(frame, "simd excetpion exception");
}
void do_handler_virtual_exception (exception_frame_t * frame) {
    do_default_handler(frame, "virtual exception exception");
}
void do_handler_control_exception (exception_frame_t * frame) {
    do_default_handler(frame, "control exception exception");
}

static void init_pic (void) {
    outb(PIC0_ICW1, PIC_ICW1_ALWAYS_1 | PIC_ICW1_ICW4);
    outb(PIC0_ICW2, IRQ_PIC_START);
    outb(PIC0_ICW3, 1 << 2);
    outb(PIC0_ICW4, PIC_ICW4_8086);

    outb(PIC1_ICW1, PIC_ICW1_ALWAYS_1 | PIC_ICW1_ICW4);
    outb(PIC1_ICW2, IRQ_PIC_START + 8);
    outb(PIC1_ICW3, 2);
    outb(PIC1_ICW4, PIC_ICW4_8086);

    outb(PIC0_IMR, 0xFF & ~(1 << 2));
    outb(PIC1_IMR, 0xff);
}

void irq_init (void) {
    for (int i = 0; i < IDE_TABLE_NR; i++) {
        gate_desc_set(idt_table + i, KERNEL_SELECTOR_CS, (uint32_t)exception_handler_unknown, 
        GATE_P_PRESENT | GATE_DPL0 | GATE_TYPE_INT);
    }

    irq_install(IRQ0_DE, (irq_handler_t)exception_handler_divider);
    irq_install(IRQ1_DB, (irq_handler_t)exception_handler_Debug);
    irq_install(IRQ2_NMI, (irq_handler_t)exception_handler_NMI);
    irq_install(IRQ3_BP, (irq_handler_t)exception_handler_breakpoint);
    irq_install(IRQ4_OF, (irq_handler_t)exception_handler_overflow);
    irq_install(IRQ5_BR, (irq_handler_t)exception_handler_bound_range);
    irq_install(IRQ6_UD, (irq_handler_t)exception_handler_invalid_opcode);
    irq_install(IRQ7_NM, (irq_handler_t)exception_handler_device_unavabliable);
    irq_install(IRQ8_DF, (irq_handler_t)exception_handler_double_fault);
    irq_install(IRQ10_TS, (irq_handler_t)exception_handler_invalid_tss);
    irq_install(IRQ11_NP, (irq_handler_t)exception_handler_segment_not_present);
    irq_install(IRQ12_SS, (irq_handler_t)exception_handler_stack_segment_fault);
    irq_install(IRQ13_GP, (irq_handler_t)exception_handler_general_protection);
    irq_install(IRQ14_PF, (irq_handler_t)exception_handler_page_fault);
    irq_install(IRQ16_MF, (irq_handler_t)exception_handler_fpu_error);
    irq_install(IRQ18_MC, (irq_handler_t)exception_handler_machine_check);
    irq_install(IRQ19_XM, (irq_handler_t)exception_handler_simd_exception);
    irq_install(IRQ20_VE, (irq_handler_t)exception_handler_virtual_exception);
    irq_install(IRQ21_CP, (irq_handler_t)exception_handler_control_exception);

    lidt((uint32_t)idt_table, sizeof(idt_table));

    init_pic();
}

int irq_install (int irq_num, irq_handler_t handler) {
    if (irq_num >= IDE_TABLE_NR) {
        return -1;
    }

    gate_desc_set(idt_table + irq_num, KERNEL_SELECTOR_CS,
     (uint32_t)handler, GATE_P_PRESENT | GATE_DPL0 | GATE_TYPE_INT);
     return 0;
}

void irq_enable (int irq_num) {
    if (irq_num < IRQ_PIC_START) {
        return;
    }

    irq_num -= IRQ_PIC_START;
    if (irq_num < 8) {
        uint8_t mask = inb(PIC0_IMR) & ~(1 << irq_num);
        outb(PIC0_IMR, mask);
    } else {
        irq_num -= 8;
        uint8_t mask = inb(PIC1_IMR) & ~(1 << irq_num);
        outb(PIC1_IMR, mask);
    }
}

void irq_disable (int irq_num) {
    if (irq_num < IRQ_PIC_START) {
        return;
    }

    irq_num -= IRQ_PIC_START;
    if (irq_num < 8) {
        uint8_t mask = inb(PIC0_IMR) | (1 << irq_num);
        outb(PIC0_IMR, mask);
    } else {
        irq_num -= 8;
        uint8_t mask = inb(PIC1_IMR) | (1 << irq_num);
        outb(PIC1_IMR, mask);
    }
}

void irq_disable_global (void) {
    cli();
}

void irq_enable_global (void) {
    sti();
}

void pic_send_eoi (int irq_num) {
    irq_num -= IRQ_PIC_START;
    if (irq_num >= 8) {
        outb(PIC1_OCW2, PIC_OCW2_EOI);
    }

    outb(PIC0_OCW2, PIC_OCW2_EOI);
}

irq_state_t irq_enter_protection (void) {
    irq_state_t state = read_eflags();
    irq_disable_global();
    return state;
}

void irq_leave_protection (irq_state_t state) {
    write_eflags(state);
}