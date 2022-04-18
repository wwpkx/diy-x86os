#ifndef LOADER_H
#define LOADER_H

#include "core/types.h"
#include "core/boot_info.h"

#pragma pack(1)
// https://wiki.osdev.org/User:Omarrx024/VESA_Tutorial
// http://www.monstersoft.com/tutorial1/VESA_intro.html
struct VbeInfoBlock {
    char VbeSignature[4];             // 固定值 "VESA"
    uint16_t VbeVersion;              // 版本号 0x0300 for VBE 3.0
    uint16_t OemStringPtr[2];         // 制造商字符串位置: segment:offset
    uint8_t Capabilities[4];			 // 显卡支持的特性
    uint16_t VideoModePtr[2];         // 支持的模式列表
    uint16_t TotalMemory;             // 有多少VGA RAM，以64KB为单位
    uint16_t software_rev;			// software revision
    uint32_t vendor;					// segment:offset to card vendor string
    uint32_t product_name;			// segment:offset to card model name
    uint32_t product_rev;			// segment:offset pointer to product revision
    char reserved[222];				// reserved for future expansion
    char oem_data[256];				// OEM BIOSes store their strings in this area
};

struct ModeInfoBlock {
    uint16_t attributes;		// deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
    uint8_t window_a;			// deprecated
    uint8_t window_b;			// deprecated
    uint16_t granularity;		// deprecated; used while calculating bank numbers
    uint16_t window_size;
    uint16_t segment_a;
    uint16_t segment_b;
    uint32_t win_func_ptr;		// deprecated; used to switch banks from protected mode without returning to real mode
    uint16_t pitch;			// number of bytes per horizontal line
    uint16_t width;			// width in pixels
    uint16_t height;			// height in pixels
    uint8_t w_char;			// unused...
    uint8_t y_char;			// ...
    uint8_t planes;
    uint8_t bpp;			// bits per pixel in this mode
    uint8_t banks;			// deprecated; total number of banks in this mode
    uint8_t memory_model;
    uint8_t bank_size;		// deprecated; size of a bank, almost always 64 KB but may be 16 KB...
    uint8_t image_pages;
    uint8_t reserved0;

    uint8_t red_mask;
    uint8_t red_position;
    uint8_t green_mask;
    uint8_t green_position;
    uint8_t blue_mask;
    uint8_t blue_position;
    uint8_t reserved_mask;
    uint8_t reserved_position;
    uint8_t direct_color_attributes;

    uint32_t framebuffer;		// physical address of the linear frame buffer; write here to draw to the screen
    uint32_t off_screen_mem_off;
    uint16_t off_screen_mem_size;	// size of memory in the framebuffer but not being displayed on the screen
    uint8_t reserved1[206];
};
#pragma pack()

void protect_mode_entry (void);
void load_kernel (void);

#endif // LOADER_H
