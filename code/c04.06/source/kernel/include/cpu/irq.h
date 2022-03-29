/**
 * 中断处理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef IRQ_H
#define IRQ_H

#include "comm/types.h"

// 中断号码
#define IRQ0_DE             0
#define IRQ1_DB             1
#define IRQ2_NMI            2
#define IRQ3_BP             3
#define IRQ4_OF             4
#define IRQ5_BR             5
#define IRQ6_UD             6
#define IRQ7_NM             7
#define IRQ8_DF             8
#define IRQ10_TS            10
#define IRQ11_NP            11
#define IRQ12_SS            12
#define IRQ13_GP            13
#define IRQ14_PF            14
#define IRQ16_MF            16
#define IRQ17_AC            17
#define IRQ18_MC            18
#define IRQ19_XM            19
#define IRQ20_VE            20

#define	IRQ0_TIMER			0x20				// 100Hz定时器中断
#define IRQ1_KEYBOARD		0x21				// 按键中断
#define IRQ12_MOUSE			0x2C				// PS/2鼠标中断
#define IRQ14_HARDDISK		0x2E				// 主总线上的ATA磁盘中断

/**
 * 中断发生时相应的栈结构
 */
typedef struct _exception_frame_t {
	int gs, fs, es, ds, edx, ecx, eax;
	int num;
	int error_code;
	int eip, cs, eflags;
	int esp, ss;
}exception_frame_t;

typedef void(*irq_handler_t)(void);

void irq_init (void);
int irq_install(int irq_num, irq_handler_t handler);
void irq_enable(int irq_num);
void irq_disable(int irq_num);
void irq_disable_global(void);
void irq_enable_global(void);

typedef uint32_t irq_state_t;
irq_state_t irq_enter_protection (void);
void irq_leave_protection (irq_state_t state);

void irq_enter_handler (void);
void irq_leave_handler (void);
int irq_is_in (void);

// PIC0��PIC1���ƼĴ���
#define PIC0_COMMAND		0x20
#define PIC0_DATA			0x21
#define PIC1_COMMAND		0xA0
#define PIC1_DATA			0xA1

#define PIC0_ICW1			0x20
#define PIC0_ICW2			0x21
#define PIC0_ICW3			0x21
#define PIC0_ICW4			0x21
#define PIC0_OCW2			0x20
#define PIC0_IMR			0x21

#define PIC1_ICW1			0xa0
#define PIC1_ICW2			0xa1
#define PIC1_ICW3			0xa1
#define PIC1_ICW4			0xa1
#define PIC1_OCW2			0xa0
#define PIC1_IMR			0xa1

#define PIC_ICW1_ICW4		(1 << 0)		// ��ʼ��ʱ��1- ��ҪICW4, 0-����Ҫ
#define PIC_ICW1_SNGL		(1 << 1)		// 1- ��Ƭ8259, 0 - ��Ƭ����
#define PIC_ICW1_LTIM		(1 << 3)		// 1- �ߵ�ƽ����, 0 - �����ش���
#define PIC_ICW_ALWAYS_1	(1 << 4)		// ����Ϊ1

#define PIC_ICW4_ALWAYS_1	(1 << 0)		// ����Ϊ1
#define PIC_OCW2_EOI		(1 << 5)		// 1 - ʹ��ǰISR�Ĵ�������Ӧλ��0

void pic_send_eoi(int irq);
void pic_clear_all(void);

// 异常处理函数
void handler_divider (void);
void handler_Debug (void);
void handler_NMI (void);
void handler_breakpoint (void);
void handler_overflow (void);
void handler_bound_range (void);
void handler_invalid_opcode (void);
void handler_device_unavailable (void);
void handler_double_fault (void);
void handler_invalid_tss (void);
void handler_segment_not_present (void);
void handler_stack_segment_fault (void);
void handler_general_protection (void);
void handler_page_fault (void);
void handler_fpu_error (void);
void handler_alignment_check (void);
void handler_machine_check (void);
void handler_smd_exception (void);
void handler_virtual_exception (void);

#endif
