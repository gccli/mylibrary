#ifndef UTIL_FILE_H__
#define UTIL_FILE_H__

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef __cplusplus
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

char *get_file_buffer(const char *file, size_t *r);

/**
 * Return the human-readable file size (e.g., 1.24 K 24.52 M 2.0 G)
 */
const char *file_size(const char *file);

int get_tmpfile(char *);

#ifdef __cplusplus
}
#endif

#endif
