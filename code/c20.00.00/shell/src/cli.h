/**
 * 命令行实现
 *
 * 创建时间：2021年8月5日
 * 作者：李述铜
 * 联系邮箱: 527676163@qq.com
 */
#ifndef CMD_H
#define CMD_H

#include <os.h>

#define CLI_CURRENT_INPUT_MAX_SIZE      1024        // 输入缓存区
#define	CLI_MAX_ARG_COUNT				10			// 最大接收的参数数量

#define	ESC_CMD1(cmd)			"\033"#cmd		// ESC n命令
#define	ESC_SAVE_CURSOR			ESC_CMD1(7)		// 保存光标命令
#define	ESC_RESTORE_CURSOR		ESC_CMD1(8)		// 恢复光标命令

#define ESC_CMD2(Pn, cmd)		"\033["#Pn#cmd
#define	ESC_CURSOR_MOV_LEFT		ESC_CMD2(1, D)	// 光标左移
#define	ESC_CURSOR_MOV_RIGHT	ESC_CMD2(1, C)	// 光标右移
#define	ESC_COLOR_ERROR			ESC_CMD2(31, m)	// 红色错误
#define	ESC_COLOR_DEFAULT		ESC_CMD2(39, m)	// 默认颜色
#define ESC_CLEAR_SCREEN		ESC_CMD2(2, J)	// 擦除整屏幕
#define	ESC_MOVE_CURSOR(row, col)  "\033["#row";"#col"H"

/**
 * 命令列表
 */
typedef struct _cli_cmd_t {
    const char * name;          // 命令名称
    const char * useage;        // 使用方法
    int(*do_func)(int argc, char **argv);       // 回调函数
}cli_cmd_t;

/**
 * 命令行管理器
 */
typedef struct _cli_t {    
    char curr_input[CLI_CURRENT_INPUT_MAX_SIZE];    // 当前输入缓存
    int curr_cursor;     // 当前光标位置
    int curr_count;      // 已输入输出，或追加位置

    const cli_cmd_t * cmd_start;      // 命令起始
    const cli_cmd_t * cmd_end;        // 命令结束

    const char * promot;        	 // 提示符
    int promot_len;

    ui_widget_t * widget;			// 显示部件
    int cols_count;					// 显示的最大列数
}cli_t;

void cli_init(cli_t * cli, const char * promot, const cli_cmd_t * cmd_list, ui_widget_t * widget);
void cli_in(cli_t * cli, key_data_t key);
void cli_end(cli_t * cli);

/**
 * 设置命令提示符
 */
static inline void cli_set_promot(cli_t * cli, const char * promot) {
    cli->promot = promot;
}


#endif