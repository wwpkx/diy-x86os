#ifndef CPU_H
#define CPU_H

#include "comm/types.h"

#define EFLGAGS_DEFAULT         (1 << 1)
#define EFLAGS_IF               (1 << 9)

#pragma pack(1)
typedef struct _segment_desc_t {
    uint16_t limit15_0;
    uint16_t base15_0;
    uint8_t base23_16;
    uint16_t attr;
    uint8_t base31_24;
}segment_desc_t;

typedef struct _gate_desc_t {
    uint16_t offset15_0;
    uint16_t selector;
    uint16_t attr;
    uint16_t offset31_16;
}gate_desc_t;

#define GATE_TYPE_INT       (0xE << 8)
#define GATE_TYPE_SYSCALL   (0xC << 8)
#define GATE_P_PRESENT      (1 << 15)
#define GATE_DPL0           (0 << 13)
#define GATE_DPL3           (3 << 13)

typedef struct _tss_t {
    uint32_t pre_link;
    uint32_t esp0, ss0, esp1, ss1, esp2, ssp2;
    uint32_t cr3;
    uint32_t eip, eflags, eax, ecx,edx, ebx, esp, ebp, esi, edi;
    uint32_t es, cs, ss, ds, fs, gs;
    uint32_t ldt;
    uint32_t iomap;
}tss_t;

#pragma pack()

#define SEG_G   (1 << 15)
#define SEG_D   (1 << 14)
#define SEG_P_PRESENT   (1 << 7)

#define SEG_DPL0    (0 << 5)
#define SEG_DPL3    (3 << 5)

#define SEG_CPL0    (0 << 0)
#define SEG_CPL3    (3 << 0)

#define SEG_S_SYSTEM    (0 << 4)
#define SEG_S_NORMAL    (1 << 4)

#define SEG_TYPE_CODE   (1 << 3)
#define SEG_TYPE_DATA   (0 << 3)
#define SEG_TYPE_TSS    (9 << 0)

#define SEG_TYPE_RW     (1 << 1)

void cpu_init (void);
void segment_desc_set (int selector, uint32_t base, uint32_t limit, uint16_t attr);
void gate_desc_set (gate_desc_t * desc, uint16_t selector, uint32_t offset, uint16_t attr);
int gdt_alloc_desc();
void  gdt_free_sel(int sel);

void swith_to_tss (int tss_sel);

#endif
