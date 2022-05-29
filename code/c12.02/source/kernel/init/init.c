/**
 * 内核初始化以及测试代码
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include "comm/boot_info.h"
#include "comm/cpu_instr.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "dev/time.h"
#include "tools/log.h"
#include "core/task.h"
#include "os_cfg.h"
#include "tools/klib.h"
#include "tools/list.h"
#include "ipc/sem.h"
#include "ipc/bfifo.h"
#include "core/memory.h"

/**
 * 内核入口
 */
void kernel_init (boot_info_t * boot_info) {
    // 初始化CPU，再重新加载
    cpu_init();
    log_init();

    // 内存初始化要放前面一点，因为后面的代码可能需要内存分配
    memory_init(boot_info);

    irq_init();
    time_init();

    task_manager_init();
}

void init_main(void) {
    log_printf("Kernel is running....");
    log_printf("Version: %s, name: %s", OS_VERSION, "tiny x86 os");
    log_printf("%d %d %x %c", -123, 123456, 0x12345, 'a');

    // 初始化任务
    task_first_init();

    for (;;) {}
}
