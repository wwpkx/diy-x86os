/**
 * 与x86的体系结构相关的接口及参数
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef CPU_H
#define CPU_H

#include "comm/types.h"

#define CR0_PG					(1 << 31)		// 分页开启位

#define GDT_SEG_G				(1 << 15)		// ���ȵ�λΪ4KB
#define GDT_SEG_D				(1 << 14)		// ���ݺ�ָ��Ϊ32λ
#define GDT_SET_PRESENT			(1 << 7)		// ���Ƿ����

#define GDT_SEG_DPL0			(0 << 5)		// DPL��Ȩ��0
#define GDT_SEG_DPL3			(3 << 5)		// DPL��Ȩ��3

#define GDT_SEG_S_SYS_GATE		(0 << 4)		// ϵͳ����������
#define GDT_SEG_S_CODE_DATA		(1 << 4)		// ���ݺʹ���������

#define GDT_SEG_TYPE_CODE		(1 << 3)		// �����
#define GDT_SEG_TYPE_DATA		(0 << 3)		// ���ݶ�
#define GDT_SEG_TYPE_A			(1 << 0)		// �Ѿ������ʹ�
#define GDT_SEG_TYPE_RW			(1 << 1)		// ���ݶοɶ�д������οɶ���ִ�С�����ֻ����ִֻ��
#define GDT_SEG_TYPE_C_E		(1 << 2)		// ����Σ�һ�´���Ρ����ݶΣ����λ��չ

#define GDB_TSS_BUSY            (1 << 1)        // TSS忙
#define GDB_TSS_TYPE            (0x9)           // TSS类型

#define GDT_TYPE_LDT            (2 << 0)
#define GDT_TYPE_TSS            (9 << 0)
#define GDT_TYPE_INT_GATE       (14 << 0)

#define GDT_SELECTOR_TI         (1 << 2)
#define GDT_RPL0                (0 << 0)
#define GDT_RPL1                (1 << 0)
#define GDT_RPL2                (2 << 0)
#define GDT_RPL3                (3 << 0)

#define GDT_GATE_PRESENT		(1 << 15)		// ���Ƿ����
#define GDT_GATE_DPL0			(0 << 13)		// DPL��Ȩ��0
#define GDT_GATE_DPL1			(1 << 13)		// DPL��Ȩ��1
#define GDT_GATE_DPL2			(2 << 13)		// DPL��Ȩ��2
#define GDT_GATE_DPL3			(3 << 13)		// DPL��Ȩ��3
#define GDT_GATE_TYPE_IDT		(0xE << 8)		// ����:IDT, 32λ
#define GDT_GATE_TYPE_CALL      (0xC << 8)      // 调用门

#define EFLAGS_IF           (1 << 9)
#define EFLAGS_DEFAULT      (1 << 1)

#pragma pack(1)

/**
 * GDT描述符
 */
typedef struct _gdt_descriptor_t {
	uint16_t limit15_0;		
	uint16_t base15_0;
	uint8_t base23_16;
	uint16_t attr;
	uint8_t base31_24;
}gdt_descriptor_t;

/*
 * 调用门描述符
 */
typedef struct _gate_descriptor_t {
	uint16_t offset15_0;
	uint16_t selector;
	uint16_t attr;
	uint16_t offset31_16;
}gate_descriptor_t;

/**
 * tss描述符
 */
typedef struct _tss_t {
    uint32_t pre_link;
    uint32_t esp0, ss0, esp1, ss1, esp2, ss2;
    uint32_t cr3;
    uint32_t eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt;
    uint32_t iomap;
}tss_t;
#pragma pack()

void cpu_init (void);
void gdt_reload (void);
gdt_descriptor_t * gdt_alloc_desc (void);
void set_gate_desc(gate_descriptor_t *desc, uint16_t selector, uint32_t offset, uint16_t attr);
void set_segment_desc(gdt_descriptor_t *desc, uint32_t base, uint32_t limit, uint16_t attr);
int gdt_alloc_segment (uint32_t base, uint32_t limit, uint16_t attr);

void switch_to_tss (uint32_t tss_selector);

#endif

