#ifndef HTTP_PARSER_DEFS_H__
#define HTTP_PARSER_DEFS_H__

#include <stdlib.h>

// No need allocate memory
typedef struct _parser_string {
    const char *data;
    size_t      len;
} pstr_t;

static inline char *strlchr(char *p, const char *last, char c)
{
    while (p < last) {
        if (*p == c) {
            return p;
        }
        p++;
    }

    return NULL;
}

#endif
