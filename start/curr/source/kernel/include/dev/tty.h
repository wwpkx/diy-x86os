#ifndef TTY_H
#define TTY_H

#include "ipc/sem.h"
#define TTY_NR              8

#define TTY_OBUF_SIZE       512
#define TTY_IBUF_SIZE       512

typedef struct _tty_fifo_t {
    char * buf;
    int size;
    int read, write;
    int count;
}tty_fifo_t;

#define TTY_OCRLF       (1 << 0)

#define TTY_INCLR       (1 << 0)
#define TTY_IECHO       (1 << 1)

typedef struct _tty_t {
    char obuf[TTY_OBUF_SIZE];
    tty_fifo_t ofifo;
    sem_t osem;
    char ibuf[TTY_IBUF_SIZE];
    tty_fifo_t ififo;
    sem_t isem;

    int iflags;
    int oflags;
    int console_idx;
}tty_t;

int tty_fifo_put (tty_fifo_t * fifo, char c);
int tty_fifo_get (tty_fifo_t * fifo, char *c);

void tty_in(int idx, char ch);

#endif
