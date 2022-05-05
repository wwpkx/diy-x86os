 	.text
 	.global init_task_entry
	.extern init_task_main
init_task_entry:
    // 需要重新加载各数据段
    mov %ss, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    jmp init_task_main
   