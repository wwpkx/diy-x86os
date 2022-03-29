//
// Created by lishutong on 2021-07-30.
//

#ifndef OS_KLIB_H
#define OS_KLIB_H

#include "core/types.h"

#define k_abs(a)		((a) < 0 ? (-(a)) : (a))
#define k_min(a, b)		((a) < (b) ? (a) : (b))
#define k_max(a, b)		((a) < (b) ? (b) : (a))


void k_strncpy(char * dest, const char * src, int size);
int k_strncmp (const char * s1, const char * s2, int size);
int k_strlen(const char * str);
void k_memcpy (void * dest, void * src, int size);
void k_memset(void * dest, uint8_t v, int size);
int k_memcmp (void * d1, void * d2, int size);
void k_sprintf(char * buffer, const char * fmt, ...);

#endif //OS_KLIB_H
