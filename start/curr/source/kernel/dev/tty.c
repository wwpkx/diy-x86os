#include "dev/tty.h"
#include "dev/dev.h"
#include "tools/log.h"
#include "dev/kbd.h"
#include "dev/console.h"

static tty_t    tty_devs[TTY_NR];

static tty_t * get_tty (device_t * dev) {
    int tty = dev->minor;
    if ((tty < 0) || (tty >= TTY_NR) || (!dev->open_count)) {
        log_printf("tty is not opened. tty=%d", tty);
        return (tty_t *)0;
    }

    return tty_devs + tty;
}

void tty_fifo_init (tty_fifo_t * fifo, char * buf, int size) {
    fifo->buf = buf;
    fifo->count = 0;
    fifo->size = size;
    fifo->read = fifo->write = 0;
}

int tty_fifo_put (tty_fifo_t * fifo, char c) {
    if (fifo->count >= fifo->size) {
        return -1;
    }

    fifo->buf[fifo->write++] = c;
    if (fifo->write >= fifo->size) {
        fifo->write = 0;
    }
    fifo->count++;
    return 0;
}

int tty_fifo_get (tty_fifo_t * fifo, char *c) {
    if (fifo->count <= 0) {
        return -1;
    }

    *c = fifo->buf[fifo->read++];
    if (fifo->read >= fifo->size) {
        fifo->read = 0;
    }
    fifo->count--;
    return 0;
}

int tty_open (device_t * dev) {
    int idx = dev->minor;
    if ((idx < 0) || (idx >= TTY_NR)) {
        log_printf("open tty failed. incorrect tty num = %d", idx);
        return -1;
    }

    tty_t * tty = tty_devs + idx;
    tty_fifo_init (&tty->ofifo, tty->obuf, TTY_OBUF_SIZE);
    sem_init(&tty->osem, TTY_OBUF_SIZE);
    tty_fifo_init (&tty->ififo, tty->ibuf, TTY_IBUF_SIZE);
    sem_init(&tty->isem, 0);
    tty->iflags = TTY_INCLR | TTY_IECHO;
    tty->oflags = TTY_OCRLF;
    tty->console_idx = idx;

    kbd_init();
    console_init(idx);

    return 0;
}


int tty_write (device_t * dev, int addr, char * buf, int size) {
    if (size < 0) {
        return -1;
    }

    tty_t * tty = get_tty(dev);
    if (!tty) {
        return -1;
    }

    int len = 0;
    while (size) {
        char c = *buf++;

        if ((c == '\n') && (tty->oflags & TTY_OCRLF)) {
            sem_wait(&tty->osem);
            int err = tty_fifo_put(&tty->ofifo, '\r');
            if (err < 0) {
                break;
            }
        }

        sem_wait(&tty->osem);
        int err = tty_fifo_put(&tty->ofifo, c);
        if (err < 0) {
            break;
        }

        len++;
        size--;

        // if (显示器是不是正在忙) {
        //     continue;
        // } else {
        //     // 启动硬件发送显示
        // }
        console_write(tty);
    }

    return size;
}

int tty_read (device_t * dev, int addr, char * buf, int size) {
    if (size < 0) {
        return -1;
    }

    tty_t * tty = get_tty(dev);
    char * pbuf = buf;
    int len = 0;
    while (len < size) {
        sem_wait(&tty->isem);

        char ch;
        tty_fifo_get(&tty->ififo, &ch);
        switch (ch) {
        case '\n':
            if ((tty->iflags & TTY_INCLR) && (len < size - 1)) {
                *pbuf++ = '\r';
                len++;
            }
            *pbuf++ = '\n';
            len++;
            break;
        default:
            *pbuf++ = ch;
            len++;
            break;
        }

        if (tty->iflags & TTY_IECHO) {
            tty_write(dev, 0, &ch, 1);
        }

        if ((ch == '\n') || (ch == '\r')) {
            break;
        }
    }
    return len;
}

int tty_control(device_t * dev, int cmd, int arg0, int arg1) {
    return 0;
}

void tty_close (device_t * dev) {

}

void tty_in(int idx, char ch) {
    tty_t * tty = tty_devs + idx;

    if (sem_count(&tty->isem) >= TTY_IBUF_SIZE) {
        return;
    }

    tty_fifo_put(&tty->ififo, ch);
    sem_notify(&tty->isem);
}


dev_desc_t dev_tty_desc = {
    .name = "tty",
    .major = DEV_TTY,
    .open = tty_open,
    .read = tty_read,
    .write = tty_write,
    .control = tty_control,
    .close = tty_close,
};