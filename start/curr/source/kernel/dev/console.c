#include "dev/console.h"
#include "tools/klib.h"
#include "comm/cpu_instr.h"
#include "dev/tty.h"
#include "cpu/irq.h"

#define CONSOLE_NR      8

static console_t console_buf[CONSOLE_NR];
static int curr_console_idx = 0;

static int read_cursor_pos (void) {
    int pos;

    irq_state_t state = irq_enter_protection();
    outb(0x3D4, 0xF);
    pos = inb(0x3d5);
    outb(0x3D4, 0xE);
    pos |= inb(0x3d5) << 8;
    irq_leave_protection(state);
    return pos;
}

static int update_cursor_pos (console_t * conole) {
    uint16_t pos = (conole - console_buf) * conole->display_rows * conole->display_cols;

    pos += conole->cursor_row * conole->display_cols + conole->cursor_col;
    
    irq_state_t state = irq_enter_protection();
    outb(0x3D4, 0xF);
    outb(0x3d5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0xE);
    outb(0x3d5, (uint8_t)((pos >> 8) & 0xFF));
    irq_leave_protection(state);
    return pos;
}

static void erase_rows (console_t * console, int start, int end) {
    disp_char_t * disp_start = console->disp_base + console->display_cols * start;
    disp_char_t * disp_end = console->disp_base + console->display_cols * (end + 1);
    while (disp_start < disp_end) {
        disp_start->c = ' ';
        disp_start->foreground = console->foreground;
        disp_start->background = console->background;

        disp_start++;
    }
}

static void scroll_up (console_t * console, int lines) {
    disp_char_t * dest = console->disp_base;
    disp_char_t * src = console->disp_base + console->display_cols * lines;
    uint32_t size = (console->display_rows - lines) * console->display_cols * sizeof(disp_char_t);
    kernel_memcpy(dest, src, size);

    erase_rows(console, console->display_rows - lines, console->display_rows - 1);
    console->cursor_row -= lines;
}

static void move_to_col0 (console_t * console) {
    console->cursor_col = 0;
}

static void move_next_line (console_t * console) {
    console->cursor_row++;
    if (console->cursor_row >= console->display_rows) {
        scroll_up(console, 1);
    }
}

static void move_forward (console_t * console, int n) {
    for (int i = 0; i < n; i++) {
        if (++console->cursor_col >= console->display_cols) {
            console->cursor_row++;
            console->cursor_col = 0;

            if (console->cursor_row >= console->display_rows) {
                scroll_up(console, 1);
            }
         }
    }
}

static void show_char (console_t * console, char c) {
    int offset = console->cursor_col + console->cursor_row * console->display_cols;
    disp_char_t * p = console->disp_base + offset;
    p->c = c;
    p->foreground = console->foreground;
    p->background = console->background;
    move_forward(console, 1);
}

static void clear_display (console_t * console) {
    int size = console->display_cols * console->display_rows;

    disp_char_t * start = console->disp_base;
    for (int i = 0; i < size; i++, start++) {
        start->c = ' ';
        start->foreground = console->foreground;
        start->background = console->background;
    }
}


static int move_backword (console_t * console, int n) {
    int status = -1;

    for (int i = 0; i < n; i++) {
        if (console->cursor_col > 0) {
            console->cursor_col--;
            status = 0;
        } else if (console->cursor_row > 0) {
            console->cursor_row--;
            console->cursor_col = console->display_cols - 1;
            status = 0;
        }
    }

    return status;
}

static void erase_backword (console_t * console) {
    if (move_backword(console, 1) == 0) {
        show_char(console, ' ');
        move_backword(console, 1);
    }
}

int console_init (int idx) {
    console_t * console = console_buf + idx;

    console->display_cols = CONSOLE_COL_MAX;
    console->display_rows = CONSOLE_ROW_MAX;

    console->disp_base = (disp_char_t *)CONSOLE_DISP_ADDR + idx *(CONSOLE_COL_MAX * CONSOLE_ROW_MAX);

    console->foreground = COLOR_White;
    console->background = COLOR_Black;

    if (idx == 0) {
        int cursor_pos = read_cursor_pos();
        console->cursor_row = cursor_pos / console->display_cols;
        console->cursor_col = cursor_pos % console->display_cols;
    } else {
        console->cursor_row = 0;
        console->cursor_col = 0;
        clear_display(console);
        //update_cursor_pos(console);
    }
    console->old_cursor_col = console->cursor_col;
    console->old_cursor_row = console->cursor_col;
    console->write_state = CONSOLE_WRITE_NORMAL;
        // clear_display(console);

    mutex_init(&console->mutex);
    return 0;
}

static void write_normal(console_t * console, char c) {
    switch (c) {
        case ASCII_ESC:
            console->write_state = CONSOLE_WRITE_ESC;
            break;
        case 0x7F:
            erase_backword(console);
            break;
        case '\b':
            move_backword(console, 1);
            break;
        case '\r':
            move_to_col0(console);
            break;
        case '\n':
            move_next_line(console);
            break;
        default:
            if ((c >= ' ') && (c <= '~')) {
                show_char(console, c);
            }
            break;
    }
}

void save_cursor (console_t * console) {
    console->old_cursor_col = console->cursor_col;
    console->old_cursor_row = console->cursor_row;
}

void restore_cursor (console_t * console) {
    console->cursor_col = console->old_cursor_col;
    console->cursor_row = console->old_cursor_row;
}

static void clear_esc_param (console_t * console) {
    kernel_memset(console->esc_param, 0, sizeof(console->esc_param));
    console->curr_param_index = 0;
}

// ESC 7/8
// ESC [pn;pn1
static void write_esc (console_t * console, char c) {
    switch (c) {
    case '7':
        save_cursor(console);
        console->write_state = CONSOLE_WRITE_NORMAL;
        break;
    case '8':
        restore_cursor(console);
        console->write_state = CONSOLE_WRITE_NORMAL;
        break;
    case '[':
        clear_esc_param(console);
        console->write_state = CONSOLE_WRITE_SQUARE;
        break;
    default:
        console->write_state = CONSOLE_WRITE_NORMAL;
        break;
    }
}

static void set_font_style (console_t * console) {
    static const color_t color_table[] = {
        COLOR_Black, COLOR_Red, COLOR_Green, COLOR_Yellow,
        COLOR_Blue, COLOR_Magenta, COLOR_Cyan, COLOR_White
    };

    for (int i = 0; i <= console->curr_param_index; i++) {
        int param = console->esc_param[i];
        if ((param >= 30) && (param <= 37)) {
            console->foreground = color_table[param - 30];
        } else if ((param >= 40) && (param <= 47)) {
            console->background = color_table[param - 40];
        } else if (param == 39) {
            console->foreground = COLOR_White;
        } else if (param == 49) {
            console->background = COLOR_Black;
        }
    }
}

static void move_left (console_t * console, int n) {
    if (n == 0) {
        n = 1;
    }

    int col = console->cursor_col - n;
    console->cursor_col = (col >= 0) ? col : 0;
}

static void move_right (console_t * console, int n) {
    if (n == 0) {
        n = 1;
    }

    int col = console->cursor_col + n;
    if (col >= console->display_cols) {
        console->cursor_col = console->display_cols - 1;
    } else {
        console->cursor_col = col;
    }
}

static void move_cursor (console_t * console) {
    console->cursor_row = console->esc_param[0];
    console->cursor_col = console->esc_param[1];
}

static void erase_in_display (console_t * console) {
    if (console->curr_param_index < 0) {
        return;
    }

    int param = console->esc_param[0];
    if (param == 2) {
        erase_rows(console, 0, console->display_cols - 1);
        console->cursor_col = console->cursor_row = 0;
    }
}

// ESC [p0;p1  m 31,39
static void write_esc_square(console_t * console, char c) {
    if ((c >= '0') && (c <='9')) {
        int *param = &console->esc_param[console->curr_param_index];
        *param = *param * 10 + c - '0';
    } else if ((c == ';') && (console->curr_param_index < ESC_PARAM_MAX)) {
        console->curr_param_index++;
    } else {
        switch (c) {
        case 'm':
            set_font_style(console);
            break;
        case 'D':
            move_left(console, console->esc_param[0]);
            break;
        case 'C':
            move_right(console, console->esc_param[0]);
            break;
        case 'H':
        case 'f':
            move_cursor(console);
            break;
        case 'J':
            erase_in_display(console);
            break;
        default:
            break;
        }

        console->write_state = CONSOLE_WRITE_NORMAL;
    }
}


// hello, world
// ESC 7/8
int console_write (tty_t * tty) {
    console_t * c = console_buf + tty->console_idx;
    int len = 0;

    mutex_lock(&c->mutex);
    do {
        char ch;
        int err = tty_fifo_get(&tty->ofifo, &ch);
        if (err < 0) {
            break;
        }
        sem_notify(&tty->osem);

        switch (c->write_state) {
        case CONSOLE_WRITE_NORMAL:
            write_normal(c, ch);
            break;
        case CONSOLE_WRITE_ESC:
            write_esc(c, ch);
            break;
        case CONSOLE_WRITE_SQUARE:
            write_esc_square(c, ch);
            break;
        default:
            break;
        }
        len++;
     }while (1);

    mutex_unlock(&c->mutex);
    
    if (tty->console_idx == curr_console_idx) {
        update_cursor_pos(c);
    } 
    return len;
}

void console_close (int console) {

}

void console_select (int idx) {
    console_t * console = console_buf + idx;
    if (console->disp_base == 0) {
        console_init(idx);
    }

    uint16_t pos = idx * console->display_rows * console->display_cols;
    outb(0x3D4, 0xC);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
    outb(0x3D4, 0xD);
    outb(0x3D5, (uint8_t)(pos & 0xFF));

    curr_console_idx = idx;
    update_cursor_pos(console);
}
