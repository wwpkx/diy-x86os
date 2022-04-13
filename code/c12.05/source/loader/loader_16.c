/**
 * 自己动手写操作系统
 *
 * 16位引导代码
 * 二级引导，负责进行硬件检测，进入保护模式，然后加载内核，并跳转至内核运行
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */

// 16位代码，必须加上放在开头，以便有些io指令生成为32位
__asm__(".code16gcc");

#include "comm/cpu_instr.h"
#include "comm/boot_info.h"
#include "loader.h"

boot_info_t boot_info;			// 启动参数信息

/**
 * BIOS下显示字符串
 * @param msg
 */
static void show_msg (const char * msg) {
	char c;

	// 使用bios写显存，持续往下写
	// 参考资料：https://blog.csdn.net/qq_40169767/article/details/101511805
	while ((c = *msg++) != '\0') {
		__asm__ __volatile__(
				"mov $0xe, %%ah\n\t"
				"mov %0, %%al\n\t"
				"mov $0x3, %%bx\n\t"
				"int $0x10"::"r"(c));
	}
}

// 参考：https://wiki.osdev.org/Memory_Map_(x86)
// 1MB以下比较标准, 在1M以上会有差别
static void  detect_memory(void) {
	uint32_t contID = 0;
	smap_entry_t smap_entry;
	int signature, bytes;

	// 初次：EDX=0x534D4150,EAX=0xE820,ECX=24,INT 0x15, EBX=0（初次）
	// 后续：EAX=0xE820,ECX=24,
	// 结束判断：EBX=0
	boot_info.ram_region_count = 0;
	for (int i = 0; i < BOOT_RAM_REGION_MAX; i++) {
		smap_entry_t * entry = &smap_entry;

		__asm__ __volatile__("int  $0x15"
			: "=a"(signature), "=c"(bytes), "=b"(contID)
			: "a"(0xE820), "b"(contID), "c"(sizeof(smap_entry_t)), "d"(SMAP_MAGIC_NUMBER), "D"(entry));
		if (signature != SMAP_MAGIC_NUMBER) {
			return;
		}

		// todo: 20字节
		if (bytes > 20 && !(entry->acpi & SMAP_ACPI_NO_IGNORE)){
			continue;
		}

		if (contID == 0) {
			break;
		}

		// 保存RAM信息，只取32位，空间有限无需考虑更大容量的情况
		if (entry->type == SMAP_TYPE_USABLE_RAM) {
			boot_info.ram_region_cfg[boot_info.ram_region_count].start = entry->base31_0;
			boot_info.ram_region_cfg[boot_info.ram_region_count].size = entry->length31_0;
			boot_info.ram_region_count++;
		}
	}
}

/**
 * 检查指定的显示模式是否被支持
 */
static int check_vga_model(int width, int height, int bpp, uint16_t *model) {
    static struct VbeInfoBlock vbe;
    static struct ModeInfoBlock inf;

    vbe.VbeSignature[0] = 'V';
    vbe.VbeSignature[1] = 'B';
    vbe.VbeSignature[2] = 'E';
    vbe.VbeSignature[3] = '2';

    uint16_t result;
    // 获取显卡的VESA的相关信息，如制造商、版本号、支持的模式信息
    __asm__ __volatile__(
    		"int $0x10"
    		: "=a"(result)
			  : "a"(0x4F00), "D"(&vbe));
    if ((uint16_t) result != 0x004F) return -1;

    // modes为16位的，以0xFFFF结尾
    uint16_t *modes = (uint16_t *) ((vbe.VideoModePtr[1] << 1) + vbe.VideoModePtr[0]);
    for (int i = 0; modes[i] != 0xFFFF; i++) {
        // 获取模式信息
        __asm__ __volatile__("int $0x10"
        : "=a"(result)
        : "a"(0x4F01), "D"(&inf), "c"(modes[i]));
        if ((uint16_t) result != 0x004F) continue;

        // Check if this is a graphics mode width linear frame buffer support
        if ((inf.attributes & 0x80) != 0x80) continue;

        // 调色板模式：4，直接写颜色模式？6
        if (inf.memory_model != 4 && inf.memory_model != 6) continue;

        // 目前这里只能找一模一样的
        if (width == inf.width && height == inf.height && bpp == inf.bpp) {
            boot_info.screen_width = inf.width;
            boot_info.screen_height = inf.height;
            boot_info.screen_vram = inf.framebuffer;
            boot_info.screen_vmode = inf.bpp;

            *model = modes[i];
            return 0;
        }
    }

  return -1;
}

// 切换至VESA模式，以支持更高的分辨率
// 如果没有，则切换至低分辨率。 VGA最高只支持到 640x480x16
static void switch_vga_mode (uint16_t width, uint16_t height, uint8_t bpp) {
    uint16_t model;

    // 检测vga，最好是打印一个列表出来
    int result = check_vga_model(width, height, bpp, &model);
    if (result >= 0) {
    	show_msg("switch to vesa.");

    	// 切换过去该模型
        __asm__ __volatile__ ("int $0x10" ::"a"(0x4F02), "b"(0x400 | model));
    } else {
        // 如果不支持该模型，则打印出错信息，然后死机
    	show_msg("VESA is not supported.");
    	for (;;) {
    	}
    }
}

// GDT表。临时用，后面内容会替换成自己的
#define CODE_SELECTOR           8       // 缺省第1个
#define DS_SELECTOR             16      // 缺省第2个
uint16_t gdt_table[][4] __attribute__((aligned(2)))= {
    {0, 0, 0, 0},
    {0xFFFF, 0x0000, 0x9A00, 0x00CF},
    {0xFFFF, 0x0000, 0x9200, 0x00CF},
};

// 保护模式入口函数，在start.asm中定义
void protect_mode_entry (void);

/**
 * 进入保护模式
 */
static void  enter_protect_mode() {
    // 关中断
    cli();

    // 开启A20地址线，使得可访问1M以上空间
    // 使用的是Fast A20 Gate方式，见https://wiki.osdev.org/A20#Fast_A20_Gate
    uint8_t v = inb(0x92);
    outb(0x92, v | 0x2);

    // 打开CR0的保护模式位，进入保持模式
    v = read_cr0();
    write_cr0(v | (1 << 0) | (1 << 1));

    // 加载GDT。由于中断已经关掉，IDT不需要加载
    lgdt((uint32_t)gdt_table, sizeof(gdt_table));

    // 长跳转进入到保护模式
    // 使用长跳转，以便清空流水线，将里面的16位代码给清空
    far_jump(CODE_SELECTOR, (uint32_t)protect_mode_entry);
}

void loader_entry(void) {
    show_msg("....loading.....");
	detect_memory();
	switch_vga_mode(SYS_CFG_SCREEN_WIDTH, SYS_CFG_SCREEN_HEIGHT, SYS_CFG_SCREEN_BPP);
    enter_protect_mode();
    for(;;) {}
}


