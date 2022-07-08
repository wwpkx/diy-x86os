#include "comm/types.h"
#include "tools/klib.h"
#include "tools/log.h"
#include "comm/cpu_instr.h"

int strings_count (char ** start) {
    int count = 0;

    if (start) {
        while (*start++) {
            count++;
        }
    }

    return count;
}

// /a/c/c/d
char * get_file_name (const char * name) {
    char * s = name;

    while (*s != '\0') {
        s++;
    }

    while ((*s != '/') && (*s != '\\') && (s >= name)) {
        s--;
    }

    return s+1;
}

void kernel_strcpy(char * dest, const char * src) {
    if (!dest || !src) {
        return;
    }

    while (*dest && *src) {
        *dest++ = *src++;
    }

    *dest = '\0';
}

void kernel_strncpy(char * dest, const char * src, int size) {
    if (!dest || !src || !size) {
        return;
    }

    char * d = dest;
    const char * s = src;
    while ((size-- > 0) && (*s)) {
        *d++ = *s++;
    }

    if (size == 0) {
        *(d - 1) = '\0';
    } else {
        *d = '\0';
    }
}

int kernel_strncmp(const char * s1, const char * s2, int size) {
    if (!s1 || !s2) {
        return -1;
    }

    while (*s1 && *s2 && (*s1 == *s2) && size) {
        s1++;
        s2++;
    }
    // s2 = "abcdefg"
    // s1 = "abcdefg"

    return !((*s1 == '\0') || (*s2 == '\0') || (*s1 == *s2));
}
int kernel_strlen(const char * str) {
    if (!str) {
        return 0;
    }

    const char * c = str;
    int len = 0;
    while (*c++) {
        len++;
    }
    return len;
}

void kernel_memcpy(void * dest, void * src, int size) {
    if (!dest || !src || !size) {
        return;
    }

    uint8_t * s = (uint8_t *)src;
    uint8_t * d = (uint8_t *)dest;
    while (size--) {
        *d++ = *s++;
    }
}

void kernel_memset(void * dest, uint8_t v, int size) {
    if (!dest || !size) {
        return;
    }

    uint8_t * d = (uint8_t *)dest;
    while (size--) {
        *d++ = v;
    }
}

int kernel_memcmp(void * d1, void * d2, int size) {
    if (!d1 || !d2 || !size) {
        return 1;
    }

    uint8_t * p_d1 = (uint8_t *)d1;
    uint8_t * p_d2 = (uint8_t *)d2;
    while (size--) {
        if (*p_d1++ != *p_d2++) {
            return 1;
        }
    }

    return 0;
}

// num = 12345 -> 10
// 1、12345 % 10 = 5   12345 / 10 = 1234
// 2. 1234 % 10 = 4   1234 / 10 = 123
// 3. 123 % 10 = 3    123 / 10 = 12
// 4. 12 % 10 = 2     12 / 10 = 1
// 5. 1 / % = 1       1 / 10 = 0
void kernel_itoa (char * buf, int num, int base) {
    static const char * num2ch = {"FEDCBA9876543210123456789ABCDEF"};
    char * p = buf;
    int old_num = num;

    if ((base != 2) && (base != 8) && (base != 10) && (base != 16)) {
        *p = '\0';
        return;
    }

    if ((num < 0) && (base == 10)) {
        *p++ = '-';
    }

    do {
        char ch = num2ch[num % base + 15]; 
        *p++ = ch;
        num /= base;
    }while (num);
    *p-- = '\0';

    // a b c d e 
    char * start = (old_num > 0) ? buf : buf + 1;
    while (start < p) {
        char ch = *start;
        *start = *p;
        *p = ch;

        p--;
        start++;
    }
}

void kernel_sprintf(char * buf, const char * fmt, ...) {
    va_list args;

    va_start(args, fmt);
    kernel_vsprintf(buf, fmt, args);
    va_end(args);
}

// "Version: %s", "1.0.10"
void kernel_vsprintf(char * buf, const char * fmt, va_list args) {
    enum {NORMAL, READ_FMT} state = NORMAL;
    char * curr = buf;
    char ch;
    while ((ch = *fmt++)) {
        switch (state) {
            case NORMAL:
                if (ch == '%') {
                    state = READ_FMT;
                } else {
                    *curr++ = ch;
                }
                break;
            case READ_FMT:
                if (ch == 'd') {
                    int num = va_arg(args, int);
                    kernel_itoa(curr, num, 10);
                    curr += kernel_strlen(curr);
                } else if (ch == 'x') {
                    int num = va_arg(args, int);
                    kernel_itoa(curr, num, 16);
                    curr += kernel_strlen(curr);
                } else if (ch == 'c') {
                    char c = va_arg(args, int);
                    *curr++ = c;
                } else if (ch == 's') {
                    const char * str = va_arg(args, char *);
                    int len = kernel_strlen(str);
                    while (len--) {
                        *curr++ = *str++;
                    }
                } 

                state = NORMAL;
                break;
        }
    }
}

void pannic(const char * file, int line, const char * func, const char * cond) {
    log_printf("assert failed! %s", cond);
    log_printf("file:%s\nline %d\nfunc:%s\n", file, line, func);
    for (;;) {
        hlt();
    }
}
