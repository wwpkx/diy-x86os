/**
 * 应用实例：显示一个不断计数的窗口，测试系统调用的可行性
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <os.h>
#include <stdio.h>
#include "app.h"

int main (void) {
	char string_buffer[128];

	// 系统调用，获取版本号
	int version = sys_get_version();

	// UI界面也可以printf，只不过需要关联控制台才可显示
	sprintf(string_buffer, "system version: %d", version);

	// 显示在窗口中的一个标签上
	ui_widget_t * win = ui_window_create(string_buffer, 300, 300, (ui_widget_t *) 0);
	ui_widget_t * label = ui_label_create("label", 150, 20, (ui_widget_t *) 0);
	ui_window_add(win, label);
	ui_widget_set_visiable(win, 1);

	// 计数显示
	int counter = 0;
	for (;;) {
		sprintf(string_buffer, "counter:%d", counter++);
		ui_label_set(label, string_buffer);
		sys_sleep(100);
	}

	return 0;
}

