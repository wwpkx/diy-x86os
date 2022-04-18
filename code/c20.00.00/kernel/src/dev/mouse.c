/**
 * 鼠标设备处理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <ui/ui_core.h>
#include <ui/ui_event.h>
#include <core/cpu.h>
#include <core/cpu_instr.h>
#include <core/irq.h>
#include <ipc/queue.h>
#include <dev/keyboard.h>
#include <dev/mouse.h>

/**
 * 鼠标状态
 */
enum {
    MOUSE_RECV_STATE = 0,
    MOUSE_RECV_X,
    MOUSE_RECV_Y,
    MOUSE_RECV_DROP_X,
    MOUSE_RECV_DROP_Y,
};

static int recv_state;			// 接收状态

/**
 * 写鼠标命令
 */
static void mouse_write(uint8_t data) {
    kbd_write(KBD_PORT_CMD, KBD_CMD_SEND_MOUSE);
    kbd_write(KBD_PORT_DATA, data);
}

/**
 * 读鼠标数据
 */
static uint8_t mouse_read(void) {
    uint8_t data;

    kbd_wait_recv_ready();
    data = inb(KBD_PORT_DATA);
    return data;
}

/**
 * 鼠标中断处理
 */
void do_handler_mouse(exception_frame_t *frame) {
    static uint8_t mouse_state;
    static ui_msg_t *msg;

    uint8_t data = inb(KBD_PORT_DATA);
    switch (recv_state) {
        case MOUSE_RECV_STATE: {
            // 检查数据的合理性
            if ((data & 0xC8) != 0x8) {
            	recv_state = MOUSE_RECV_STATE;
            } else {
                msg = ui_alloc_msg(UI_MSG_MOUSE, 0);
                if (!msg) {
                    // 当UI刷新较慢时，鼠标的数据可能来不及处理，导致无msg可用
                    // 所用这里直接丢弃掉
                    recv_state = MOUSE_RECV_DROP_X;        // 抛弃后面的两个数据
                } else {
                    msg->left_pressed = data & MOUSE_STAT_LEFT_PRESS ? 1 : 0;
                    msg->center_pressed = data & MOUSE_STAT_CENTER_PRESS ? 1 : 0;
                    msg->right_pressed = data & MOUSE_STAT_RIGHT_PRESS ? 1 : 0;
                    mouse_state = data;

                    recv_state = MOUSE_RECV_X;
                }
            }
            break;
        }
        case MOUSE_RECV_X: {
            if (mouse_state & MOUSE_X_SIGN) {
                msg->delta_x = (0xFFFFFF00 | data);
            } else {
                msg->delta_x = data;
            }
            recv_state = MOUSE_RECV_Y;
            break;
        }
        case MOUSE_RECV_Y: {
        	// 方向与UI的座标相反
            if (mouse_state & MOUSE_Y_SIGN) {
                msg->delta_y = -(0xFFFFFF00 | data);
            } else {
                msg->delta_y = -data;
            }
            recv_state = MOUSE_RECV_STATE;

            // 消息发出去
            ui_send_msg(msg, (task_t *)0, 0);
            break;
        }
        case MOUSE_RECV_DROP_X:
            recv_state = MOUSE_RECV_DROP_Y;
            break;
        case MOUSE_RECV_DROP_Y:
            recv_state = MOUSE_RECV_STATE;
            break;
    }

    // 先发EOI，再离开中断。如果反过来，就可能leave中进行切换切换，导致EOI延迟产生，或者永远不能产生
    // 这样中断就永远不会产生
    pic_send_eoi(IRQ12_MOUSE);
}

/**
 * 鼠标硬件初始化
 */
void mouse_init(void) {
    uint8_t ack;

    recv_state = MOUSE_RECV_STATE;

    // 启用鼠标
    kbd_write(KBD_PORT_CMD, KBD_CMD_ENABLE_MOUSE);

    // 启用注机制，鼠标移动时，会自动中断产生数据
    mouse_write(MOUSE_CMD_ENABLE_STREAM);
    mouse_read();	// 消耗掉ack

    // https://wiki.osdev.org/%228042%22_PS/2_Controller#Translation
    kbd_write(KBD_PORT_CMD, KBD_CMD_READ_CONFIG);
    ack = kbd_read() | (1 << 1);  // 启用2号端口
    kbd_write(KBD_PORT_CMD, KBD_CMD_WRITE_CONFIG);
    kbd_write(KBD_PORT_DATA, ack);

    // 开启并配置中断
    irq_install(IRQ12_MOUSE, (irq_handler_t) handler_mouse);
    irq_enable(IRQ12_MOUSE);
}
