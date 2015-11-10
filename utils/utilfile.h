#ifndef UTIL_FILE_H__
#define UTIL_FILE_H__

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __cpluplus
extern "C" {
#endif

/**
 * Get file length in bytes
 */
static inline size_t get_file_size(const char *file)
{
    struct stat st;
    if (stat(file, &st) != 0) {
        fprintf(stderr, "stat error - %s\n", strerror(errno));
        return 0;
    }

    return st.st_size;
}

/**
 * Return the human-readable file size (e.g., 1.24 K 24.52 M 2.0 G)
 */
const char *file_size(const char *file);

static inline char *get_file_buffer(const char *file, size_t *r)
{
    FILE *fp;
    char *buff;
    size_t len;
    if ((len = get_file_size(file)) == 0) {
        return NULL;
    }
    if ((fp = fopen(file, "rb")) == NULL) {
        fprintf(stderr, "open file error - %s\n", strerror(errno));
        return NULL;
    }

    buff = (char *)calloc(len, 1);
    if (buff == NULL) {
        fprintf(stderr, "failed allocate memory\n");
        return NULL;
    }
    if ((*r = fread(buff, 1, len, fp)) != len) {
        fprintf(stderr, "read file %zu bytes\n", *r);
        free(buff);
        return NULL;
    }

    return buff;
}

#ifdef __cpluplus
}
#endif

#endif
