/**
 * 内核初始化以及测试代码
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <core/task.h>
#include "core/boot_info.h"
#include "core/irq.h"
#include "dev/keyboard.h"
#include <core/syscall.h>
#include <core/time.h>
#include <ui/ui_core.h>
#include <lib_ui.h>
#include <dev/tty.h>
#include <dev/disk.h>
#include <init/init.h>
#include <dev/mouse.h>

/**
 * 内核入口
 */
void init_main(void) {
    cpu_init();

    task_manager_init();
    tty_manager_init();

    init_syscall();
    ui_init(boot_info);

    kbd_init();
    mouse_init();
    rtimer_init();

    xdisk_list_init();
    fs_init();

    create_init_task();
    move_to_first_task();
}
