
    .text
    .extern kernel_init
    .global _start
    # void start (boot_info_t * boot_info)
_start:
<<<<<<< HEAD

    # 第一种方法
    # push %ebp
    # mov %esp, %ebp
    # mov 0x8(%ebp), %eax
    # push %eax

    # 第二种方法
    # mov 4(%esp), %eax
    # push %eax

    # 第三种方法
    push 4(%esp)

    # kernel_init(boot_info)
=======
    # push %ebp
    # mov %esp, %ebp
    # mov 0x8(%ebp), %eax

    mov 4(%esp), %eax

    # kernel_init(boot_info)
    push %eax
>>>>>>> e976b19bba3b541389c97d9804ec66f4bea3f2b4
    call kernel_init
    jmp .
    