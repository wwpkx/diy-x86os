/**
 * 功能：32位代码，完成多任务的运行
 *
 *创建时间：2022年8月31日
 *作者：李述铜
 *联系邮箱: 527676163@qq.com
 *相关信息：此工程为《从0写x86 Linux操作系统》的前置课程，用于帮助预先建立对32位x86体系结构的理解。整体代码量不到200行（不算注释）
 *课程请见：https://study.163.com/course/introduction.htm?courseId=1212765805&_trace_c_p_k2_=0bdf1e7edda543a8b9a0ad73b5100990
 */
#include "os.h"

// 类型定义
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

/**
 * @brief 系统页表
 * 下面配置中只做了一个处理，即将0x0-4MB虚拟地址映射到0-4MB的物理地址，做恒等映射。
 */
#define MAP_ADDR        (0x80000000)            // 要映射的地址
#define PDE_P			(1 << 0)
#define PDE_W			(1 << 1)
#define PDE_U			(1 << 2)
#define PDE_PS			(1 << 7)
uint8_t map_phy_buffer[4096] __attribute__((aligned(4096)));
static uint32_t pg_table[1024] __attribute__((aligned(4096))) = {PDE_U};    // 要给个值，否则其实始化值不确定
uint32_t pg_dir[1024] __attribute__((aligned(4096))) = {
    [0] = (0) | PDE_P | PDE_PS | PDE_W | PDE_U,	    // PDE_PS，开启4MB的页，恒等映射
};

struct {uint16_t offset_l, selector, attr, offset_h;} idt_table[256] __attribute__((aligned(8))) = {1};
struct {uint16_t limit_l, base_l, basehl_attr, base_limit;}gdt_table[256] __attribute__((aligned(8))) = {
    // 0x00cf9a000000ffff - 从0地址开始，P存在，DPL=0，Type=非系统段，32位代码段（非一致代码段），界限4G，
    [KERNEL_CODE_SEG / 8] = {0xffff, 0x0000, 0x9a00, 0x00cf},
    // 0x00cf93000000ffff - 从0地址开始，P存在，DPL=0，Type=非系统段，数据段，界限4G，可读写
    [KERNEL_DATA_SEG/ 8] = {0xffff, 0x0000, 0x9200, 0x00cf},
};

void outb(uint8_t data, uint16_t port) {
	__asm__ __volatile__("outb %[v], %[p]" : : [p]"d" (port), [v]"a" (data));
}

void timer_init (void);
void os_init (void) {
    // 初始化8259中断控制器，打开定时器中断
    outb(0x11, 0x20);       // 开始初始化主芯片
    outb(0x11, 0xA0);       // 初始化从芯片
    outb(0x20, 0x21);       // 写ICW2，告诉主芯片中断向量从0x20开始
    outb(0x28, 0xa1);       // 写ICW2，告诉从芯片中断向量从0x28开始
    outb((1 << 2), 0x21);   // 写ICW3，告诉主芯片IRQ2上连接有从芯片
    outb(2, 0xa1);          // 写ICW3，告诉从芯片连接g到主芯片的IRQ2上
    outb(0x1, 0x21);        // 写ICW4，告诉主芯片8086、普通EOI、非缓冲模式
    outb(0x1, 0xa1);        // 写ICW4，告诉主芯片8086、普通EOI、非缓冲模式
    outb(0xfe, 0x21);       // 开定时中断，其它屏幕
    outb(0xff, 0xa1);       // 屏幕所有中断

    // 设置定时器，每100ms中断一次
    int tmo = (1193180 / 100);      // 时钟频率为1193180
    outb(0x36, 0x43);               // 二进制计数、模式3、通道0
    outb((uint8_t)tmo, 0x40);
    outb(tmo >> 8, 0x40);

    // 添加中断
    idt_table[0x20].offset_h = (uint32_t)timer_init >> 16;
    idt_table[0x20].offset_l = (uint32_t)timer_init & 0xffff;
    idt_table[0x20].selector = KERNEL_CODE_SEG;
    idt_table[0x20].attr = 0x8E00;      // 存在，DPL=0, 中断门

    // 虚拟内存
    // 0x80000000开始的4MB区域的映射
    pg_dir[MAP_ADDR >> 22] = (uint32_t)pg_table | PDE_P | PDE_W | PDE_U;
    pg_table[(MAP_ADDR >> 12) & 0x3FF] = (uint32_t)map_phy_buffer| PDE_P | PDE_W | PDE_U;
};