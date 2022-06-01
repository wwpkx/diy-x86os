/**
 * 键盘设备处理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "comm/cpu_instr.h"
#include "cpu/irq.h"
#include "dev/kbd.h"
#include "tools/log.h"

/**
 * @brief 按键中断处理程序
 */
void do_handler_kbd(exception_frame_t *frame) {
	// 检查是否有数据，无数据则退出
	uint8_t status = inb(KBD_PORT_STAT);
	if (!(status & KBD_STAT_RECV_READY)) {
        pic_send_eoi(IRQ1_KEYBOARD);
		return;
	}

	// 读取键值
    uint8_t raw_code = inb(KBD_PORT_DATA);
    log_printf("key: %d", raw_code);
    
    pic_send_eoi(IRQ1_KEYBOARD);
}

/**
 * 键盘硬件初始化
 */
void kbd_init(void) {
    irq_install(IRQ1_KEYBOARD, (irq_handler_t)exception_handler_kbd);
    irq_enable(IRQ1_KEYBOARD);
}
