/**
 * 鼠标设备处理
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef SRC_INCLUDE_DEV_MOUSE_H_
#define SRC_INCLUDE_DEV_MOUSE_H_

#define MOUSE_CMD_SET_DEFAULT			0xF6		// 禁用输出流且设置成缺省模式
#define MOUSE_CMD_ENABLE_STREAM			0xF4		// 启动自动包输出
#define MOUSE_CMD_ACK					0xFA		// 响应码

#define MOUSE_Y_SIGN					(1 << 5)	// Y坐标为负数
#define MOUSE_X_SIGN					(1 << 4)	// X坐标为负数
#define MOUSE_STAT_RIGHT_PRESS			(1 << 2)	// 左键按下
#define MOUSE_STAT_CENTER_PRESS			(1 << 1)	// 中间按下
#define MOUSE_STAT_LEFT_PRESS			(1 << 0)	// 右键按下

void mouse_init(void);
void handler_mouse(void);

#endif /* SRC_INCLUDE_DEV_MOUSE_H_ */
