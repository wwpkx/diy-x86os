#include "init.h"
#include "comm/boot_info.h"
#include "comm/cpu_instr.h"
#include "cpu/cpu.h"
#include "cpu/irq.h"
#include "dev/time.h"
#include "tools/log.h"
#include "os_cfg.h"
#include "tools/klib.h"
#include "core/task.h"

void kernel_init (boot_info_t * boot_info) {
   cpu_init();

    log_init();
    irq_init();
    time_init();
}

static task_t first_task;
static uint32_t init_task_stack[1024];
static task_t init_task;

void init_task_entry (void) {
    int count = 0;
    for (;;) {
        log_printf("init task: %d", count++);
        task_switch_from_to(&init_task, &first_task);
    }
}

void init_main (void) {   
    log_printf("Kernel is running....");
    log_printf("Version: %s %s", OS_VERSION, "diyx86 os");
    log_printf("%d %d %x %c", 123456,  -123, 0x12345, 'a');

    task_init(&init_task, (uint32_t)init_task_entry, (uint32_t)&init_task_stack[1024]);
    task_init(&first_task, 0, 0);
    write_tr(first_task.tss_sel);

    int count = 0;
    for (;;) {
        log_printf("int main: %d", count++);
        task_switch_from_to(&first_task, &init_task);
    }
}