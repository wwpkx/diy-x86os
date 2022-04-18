/**
 * 系统调用接口
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef LIB_SYSCALL_H
#define LIB_SYSCALL_H

#include <core/task.h>
#include <ui/ui_event.h>
#include <dev/tty.h>
#include <fs/fs.h>

int sys_get_version (void);
int sys_get_ticks (void);
void sys_sleep (int ms);

int sys_get_key(key_data_t * key);
int sys_get_event(app_msg_t * msg);

int sys_create_timer (int period, int data);
int sys_set_timer (int timer, int ms);
int sys_free_timer (int timer);

int sys_send_msg(int dest, void *msg, int wait_ret);
int sys_ioctl(int file, int cmd, ...);

#endif //LIB_SYSCALL_H
