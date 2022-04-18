/**
 * 贪吃蛇游戏
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#include <lib_ui.h>
#include <lib_syscall.h>
#include "snake.h"

// 显示相关
static ui_widget_t * content_win;
static int wall_width, wall_height;
static int move_area_startx, move_area_starty;
static int row_max, col_max;
static int move_timer;

// 蛇相关
static body_part_t body_part_buf[BODY_PART_MAX];
static body_part_t * alloc_from;
static body_part_t * food;
static snake_t * play1_snake;		// 目前只支持一条蛇
static int game_quit;

/**
 * 判断蛇是否咬到自己
 */
static int is_bit_itself (snake_t * snake) {
	for (body_part_t * body = snake->head->next; body; body = body->next) {
		if ((body->row == snake->head->row) && (body->col == snake->head->col)) {
			return 1;
		}
	}
	return 0;
}

/**
 * 判断是否碰到墙
 */
static int is_bit_wall (snake_t * snake) {
	return ((snake->head->row < 0) || (snake->head->col < 0))
			|| (snake->head->row >= row_max) || (snake->head->col >= col_max);
}

/**
 * 判断是否吃到食物
 */
static int is_bit_food (snake_t * snake) {
	return ((snake->head->row == food->row) && (snake->head->col == food->col));
}

/**
 * 初始化分配链表
 */
static void init_part_list (void) {
	alloc_from = (body_part_t *)0;
	for (int i = 0; i < BODY_PART_MAX; i++) {
		body_part_t * part = body_part_buf + i;
		part->next = alloc_from;
		alloc_from = part;
	}
}

/**
 * 分配一个身体结点或食物
 */
static body_part_t * alloc_part(void) {
	body_part_t * part = (body_part_t *)0;

	if (alloc_from) {
		part = alloc_from;
		alloc_from = alloc_from->next;

		part->next = (body_part_t *)0;
	}
	return part;
}

/**
 * 释放一个结点或结点链
 */
static void free_part (body_part_t * part) {
	part->next = alloc_from;
	alloc_from = part;
}

/**
 * 清空整个地图
 */
void clear_map (void) {
	ui_pixel_win_draw_rect(content_win,
			0, 0,
			ui_widget_width(content_win),
			ui_widget_height(content_win),
			SNAKE_EMPTY_COLOR);
}

/**
 * 显示欢迎信息
 */
void show_welcome(void) {
	clear_map();

	ui_pixel_win_draw_string(content_win,
			move_area_startx, move_area_starty,
			COLOR_White, "Welcome to sname game");
	ui_pixel_win_draw_string(content_win,
			move_area_startx, move_area_starty + 20,
			COLOR_White, "Use a.w.s.d to move, J acc, K dec");
	ui_pixel_win_draw_string(content_win,
			move_area_startx, move_area_starty + 40,
			COLOR_White, "Press any key to start game");

	// 读取键盘任意数值，然后启动应用
	key_data_t key;
	sys_get_key(&key);
}

/**
 * 创建地图
 */
void create_map(void) {
	clear_map();

	// 四边的围墙
	// 顶部
	ui_pixel_win_draw_rect(content_win,
			move_area_startx - WALL_THICK, move_area_starty - WALL_THICK,
			wall_width, WALL_THICK,
			COLOR_Red);
	// 底部
	ui_pixel_win_draw_rect(content_win,
			move_area_startx - WALL_THICK, move_area_starty + wall_height - 2 * WALL_THICK,
			wall_width, WALL_THICK,
			COLOR_Red);
	// 左边
	ui_pixel_win_draw_rect(content_win,
			move_area_startx - WALL_THICK, move_area_starty,
			WALL_THICK, wall_height - 2 * WALL_THICK,
			COLOR_Red);
	// 右边
	ui_pixel_win_draw_rect(content_win,
			move_area_startx + wall_width - 2 * WALL_THICK, move_area_starty,
			WALL_THICK, wall_height - 2 * WALL_THICK,
			COLOR_Red);
}

/**
 * 创建自动前移定时器
 */
static void create_timer (void) {
	move_timer = sys_create_timer(SNAKE_MOVE_PERIOD, 0);
}

/**
 * 终止定时器
 */
static void free_timer(void) {
	sys_free_timer(move_timer);
}

/**
 * 设置蛇身显示在某个地方
 */
static void show_pos (int row, int col, ui_color_t color) {
	ui_pixel_win_draw_rect(content_win,
			col * BODY_PART_SIZE + move_area_startx,
			row * BODY_PART_SIZE + move_area_starty,
			BODY_PART_SIZE, BODY_PART_SIZE, color);
}

/**
 * 创建蛇。最开始只有一个头
 */
static void create_snake (void) {
	static snake_t snake;

	snake_t * p_snake = &snake;
	p_snake->head = alloc_part();
	p_snake->head->row = 10;
	p_snake->head->col = 20;
	p_snake->status = SNAKE_BIT_NONE;
	p_snake->dir = LEFT;
	show_pos(p_snake->head->row, p_snake->head->col, SNAKE_HEAD_COLOR);

	play1_snake = p_snake;
}

/**
 * 给蛇添加头部
 */
static void add_head (snake_t * snake, int row, int col) {
	body_part_t * part = alloc_part();
	part->row = row;
	part->col = col;
	part->next = snake->head;
	snake->head = part;
	show_pos(row, col, SNAKE_HEAD_COLOR);
}

/**
 * 移除蛇尾的最后一个结点
 */
static void remove_tail (snake_t * snake) {
	// 先定位curr到最后一个结点
	body_part_t * pre = (body_part_t *)0;
	body_part_t * curr = snake->head;
	while (curr->next) {
		pre = curr;
		curr = curr->next;
	}

	show_pos(curr->row, curr->col, SNAKE_EMPTY_COLOR);

	// 再移除
	pre->next = curr->next;
	curr->next = (body_part_t *)0;
	free_part(curr);
}

/**
 * 创建食物
 */
static void create_food(void)  {
	// 创建一个body
	food = alloc_part();

	// 在设定位置时要避免与身体重合，所以要反复调整
	body_part_t * part = play1_snake->head;
	do {
		// 设定一个随机的位置，没有随机数怎么办？
		// 只要让每次出现的位置有变化即可，不一定要真的随机
		int ticks = sys_get_ticks();
		food->row = ticks % row_max;
		food->col = (2 * ticks + 100) % col_max;

		// 食物不能在墙上
		if ((food->row < 0) || (food->row >= row_max)
				|| (food->col < 0) || (food->col >= col_max)) {
			continue;
		}

		// 食物不能在蛇身上
		// 遍历，如果有重合，则重来。没有则退出
		while (part) {
			if ((food->row != play1_snake->head->row) || (food->col != play1_snake->head->col)) {
				// 将食物显示出来
				show_pos(food->row, food->col, SNAKE_FOOD_COLOR);
				return;
			}
			part = part->next;
		}
		part = play1_snake->head;
	} while (1);
}

/**
 * 释放掉食物
 */
static void free_food (void) {
	free_part(food);
	food = (body_part_t *)0;
}

/**
 * 蛇向前移动一个位置，具体移动方向由键盘控制
 */
static void move_forward (snake_t * snake, move_dir_t dir) {
	int next_row, next_col;

	// 计算下一位置
	switch (dir) {
	case LEFT:
		next_row = snake->head->row;
		next_col = snake->head->col - 1;
		break;
	case RIGHT:
		next_row = snake->head->row;
		next_col = snake->head->col + 1;
		break;
	case UP:
		next_row = snake->head->row - 1;
		next_col = snake->head->col;
		break;
	case DOWN:
		next_row = snake->head->row + 1;
		next_col = snake->head->col;
		break;
	case NONE:
		return;
	}

	// 判断有没有往回走
	body_part_t * next_part = snake->head->next;
	if (next_part) {
		if ((next_row == next_part->row) && (next_col == next_part->col)) {
			return;
		}
	}

	// 先不管有没有食物，都生成一个头部，然后前移
	add_head(snake, next_row, next_col);

	// 然后检查相应的异常情况
	if (is_bit_itself(snake)) {
		snake->status = SNAKE_BIT_ITSELF;
		remove_tail(snake);
	} else if (is_bit_wall(snake)) {
		snake->status = SNAKE_BIT_WALL;
		remove_tail(snake);
	} else if (is_bit_food(snake)){
		// 食物被吃掉, 回收，重新生成
		free_food();
		create_food();
		snake->status = SNAKE_BIT_FOOD;
	} else {
		// 没有吃到食物，需要移除尾部
		remove_tail(snake);
		snake->status = SNAKE_BIT_NONE;
	}

	snake->dir = dir;
}

/**
 * 检查游戏是否结束
 */
static int check_game_over (snake_t * snake) {
	if ((snake->status == SNAKE_BIT_ITSELF) || (snake->status == SNAKE_BIT_WALL)) {
		ui_pixel_win_draw_string(content_win, move_area_startx, move_area_starty, COLOR_Red, "GAME OVER");
		ui_pixel_win_draw_string(content_win,
				move_area_startx, move_area_starty + 40, COLOR_Red, "Press Any key to continue");
		return 1;
	}

	return 0;
}

/**
 * 检查按键是否按下
 */
int key_pressed (ui_widget_t win, key_data_t key) {
	switch (key.data) {
	case PLAYER1_KEY_LEFT:
		move_forward(play1_snake, LEFT);
		break;
	case PLAYER1_KEY_DOWN:
		move_forward(play1_snake, DOWN);
		break;
	case PLAYER1_KEY_UP:
		move_forward(play1_snake, UP);
		break;
	case PLAYER1_KEY_RIGHT:
		move_forward(play1_snake, RIGHT);
		break;
	case PLAYER1_KEY_QUITE:
		return -1;
	}

	int game_over = check_game_over(play1_snake);
	return game_over ? -1 : 0;
}

/**
 * 超时处理
 */
int timer_expired (int timer, int data) {
	// 往前移动后检查game over
	move_forward(play1_snake, play1_snake->dir);

	int game_over = check_game_over(play1_snake);
	return game_over ? -1 : 0;
}

/**
 * 初始化游戏
 */
static void init_game(void) {
	ui_widget_t * main_win = ui_window_create("Snake Game", 500, 500, (ui_widget_t *)0);
	ui_widget_t * container = ui_window_container(main_win);

	int width = ui_widget_content_width(container);
	int height = ui_widget_content_width(container);
	content_win = ui_pixel_win_create(width, height, (ui_widget_t *)0);
	ui_window_add(main_win, content_win);
	ui_widget_set_visiable(main_win, 1);

	// 记录一些重要参数
	// 调整高度和宽度，以及起始位置，使得围墙区域内的宽度和高度都为蛇身的整数倍
	int total_width = ui_widget_width(content_win);
	int total_height = ui_widget_height(content_win);	// 预留标题高度

	// 计算可实际显示的列数和行数
	row_max = (total_height - 2 * WALL_THICK) / BODY_PART_SIZE;
	col_max = (total_width - 2 * WALL_THICK) / BODY_PART_SIZE;

	// 游戏区居中显示?
	wall_width = col_max * BODY_PART_SIZE + 2 * WALL_THICK;
	wall_height = row_max * BODY_PART_SIZE + 2 * WALL_THICK;
	move_area_startx = (total_width - wall_width) / 2 + WALL_THICK;
	move_area_starty = (total_height - wall_height) / 2 + WALL_THICK;

	// 初始化回调函数
	ui_set_key_callback(key_pressed);
	ui_set_timer_callback(timer_expired);

	game_quit = 0;
}

#include <stdlib.h>
int main (void) {
	init_game();
	while (!game_quit) {
		// 游戏初始化
 		show_welcome();
		create_map();
		init_part_list();
		create_snake();
		create_food();
		create_timer();

		// 运行游戏
		ui_loop_event();

		// 结束游戏
		free_timer();
		sys_sleep(3000);
	}
}
