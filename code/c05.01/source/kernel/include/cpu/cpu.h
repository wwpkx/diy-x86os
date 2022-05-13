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

#pragma pack(1)

/**
 * GDT描述符
 */
typedef struct _segment_desc_t {
	uint16_t limit15_0;
	uint16_t base15_0;
	uint8_t base23_16;
	uint16_t attr;
	uint8_t base31_24;
}segment_desc_t;

#pragma pack()

void cpu_init (void);
void segment_desc_set(int selector, uint32_t base, uint32_t limit, uint16_t attr);

#endif

