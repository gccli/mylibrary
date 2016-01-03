#ifndef UTIL_DEBUG_H__
#define UTIL_DEBUG_H__

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <sys/syscall.h>

#ifndef _DEBUG
#define _DEBUG 0
#endif

#if _DEBUG > 0
#include <string.h>
#define __FN__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define debug_print(msg,...)                                            \
    fprintf(stdout, "%s:%d(%s): "msg,__FN__,__LINE__,__FUNCTION__,##__VA_ARGS__)
#else
#define debug_print(msg, ...) ((void)0)
#endif

#define FMT_RED(msg)    "\x1b[31m\x1b[1m" msg "\x1b[0m"
#define FMT_GREEN(msg)  "\x1b[32m\x1b[1m" msg "\x1b[0m"
#define FMT_BLUE(msg)   "\x1b[34m\x1b[1m" msg "\x1b[0m"
#define FMT_YELLOW(msg) "\x1b[33m\x1b[1m" msg "\x1b[0m"
#define FMT_MAGE(msg)   "\x1b[35m\x1b[1m" msg "\x1b[0m"

#define FMT_R(msg)  FMT_RED(msg)"\n"
#define FMT_G(msg)  FMT_GREEN(msg)"\n"
#define FMT_B(msg)  FMT_BLUE(msg)"\n"
#define FMT_Y(msg)  FMT_YELLOW(msg)"\n"
#define FMT_M(msg)  FMT_MAGE(msg)"\n"

#ifdef __cplusplus
extern "C" {
#endif

#define thread_id() syscall(__NR_gettid)
static inline void print_string(void *data, int length)
{
    int i;
    char *str = (char*) data;
    for (i = 0; i < length; i++) {
        if (str[i] >= 32 && str[i] <= 126)
            printf("%c", str[i]);
        else
            printf("\\x%02x", (uint8_t) str[i]);
    }
}

static inline void print_hex_string(void *data, int length, int max_size)
{
    int i;
    uint8_t *pstr = (uint8_t *)data;
    if (max_size == 0) max_size = 32;
    length = length > max_size ? max_size : length;
    for (i = 0; i < length; i++)
        printf("%02x ", pstr[i]);
    if (length > 32)
        printf("...");
}

static inline const char *hexstr(void *in, size_t len, char *out)
{
    size_t i, off;
    unsigned char *p = (unsigned char *)in;
    for(i=0, off=0; i<len; ++i) {
        off += sprintf(out+off, "%02x", *p++);
    }
    out[off] = 0;

    return out;
}


#ifdef __cplusplus
}
#endif

#endif
