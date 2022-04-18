/**
 * 贪吃蛇游戏
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef SRC_APP_SNAKE_H_
#define SRC_APP_SNAKE_H_

#define PLAYER1_KEY_UP			'w'
#define PLAYER1_KEY_DOWN		's'
#define PLAYER1_KEY_LEFT		'a'
#define PLAYER1_KEY_RIGHT		'd'
#define PLAYER1_KEY_QUITE		'q'

#define WALL_THICK				5		// 围墙的厚度
#define BODY_PART_MAX			500		// 蛇身最长数量
#define BODY_PART_SIZE			10		// 蛇身部位单元显示的大小
#define SNAKE_EMPTY_COLOR		COLOR_Black		// 空白颜色
#define SNAKE_BODY_COLOR		COLOR_White		// 蛇身颜色
#define SNAKE_HEAD_COLOR		COLOR_Yellow	// 蛇头颜色
#define SNAKE_FOOD_COLOR		COLOR_Blue		// 食物颜色
#define SNAKE_MOVE_PERIOD		500		// 蛇自动往前移的时间间隔，以毫秒计

typedef enum {
	NONE, LEFT, RIGHT, UP, DOWN,		// 蛇的移动方向
}move_dir_t;

/**
 * 蛇身的一个节点
 */
typedef struct _body_part_t {
	int row;
	int col;
	struct _body_part_t *next;
}body_part_t;

/**
 * 蛇的当前状态
 */
typedef enum {
	SNAKE_BIT_NONE,
	SNAKE_BIT_ITSELF,
	SNAKE_BIT_WALL,
	SNAKE_BIT_FOOD,
}snake_status_t;

/*
 * 蛇结构
 */
typedef struct _snake_t {
	body_part_t * head;
	snake_status_t status;
	move_dir_t dir;
}snake_t;

#endif /* SRC_APP_SNAKE_H_ */
