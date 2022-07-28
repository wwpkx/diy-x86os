
// eax, ecx, edx由调用者自动保存
// ebx, esi, edi, ebp需要由被调用者保存和恢复
// cs/ds/es/fs/gs/ss不用保存，因为都是相同的
// esp不用保存，只需要让esp保存在调用之前的状态
// eflags ?
	.text
	.global swap_context
swap_context:
	movl 4(%esp), %eax   // 取from的stack存储地址
	movl 8(%esp), %edx   // 取to的stack

	// 保存前一任务的状态
	push %ebp
	push %ebx
	push %esi
	push %edi

	// 切换栈
	mov %esp, (%eax)    // 保存esp
  	mov %edx, %esp      // 恢复esp

	// 加载下一任务的栈
	pop %edi
	pop %esi
	pop %ebx
	pop %ebp
  	ret

	.global restore_context
restore_context:
	movl 4(%esp), %eax   // 取栈顶指针
  	mov %eax, %esp 
	pop %edi
	pop %esi
	pop %ebx
	pop %ebp
	ret