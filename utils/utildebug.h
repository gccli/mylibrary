#ifndef UTIL_DEBUG_H__
#define UTIL_DEBUG_H__

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <sys/syscall.h>

#ifdef __cplusplus
extern "C" {
#endif

#define thread_id() syscall(__NR_gettid)
static inline void print_string(void *data, int length)
{
    char *str = (char*) data;

    for (int i = 0; i < length; i++) {
        if (str[i] >= 32 && str[i] <= 126)
            printf("%c", str[i]);
        else
            printf("\\x%02x", (uint8_t) str[i]);
    }
}

static inline void print_hex_string(void *data, int length, int max_size)
{
    uint8_t *pstr = (uint8_t *)data;
    if (max_size == 0) max_size = 32;
    length = length > max_size ? max_size : length;
    for (int i = 0; i < length; i++)
        printf("%02x ", pstr[i]);
    if (length > 32)
        printf("...");
}

#ifdef __cplusplus
}
#endif

#endif
