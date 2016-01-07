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

static inline char *get_file_buffer(const char *file, size_t *r)
{
    FILE *fp = NULL;
    char *buf = NULL;
    long len;
    do {
        if ((fp = fopen(file, "rb")) == NULL) {
            fprintf(stderr, "open file error - %s\n", strerror(errno));
            break;
        }
        fseek(fp, 0L, SEEK_END);
        len = ftell(fp);
        rewind(fp);
        if ((buf = (char *)calloc(len, 1)) == NULL) {
            fprintf(stderr, "failed allocate memory\n");
            break;
        }
        if ((*r = fread(buf, 1, len, fp)) != len) {
            fprintf(stderr, "file length %ld not equal to read size %zu\n", len, *r);
            free(buf);
            buf = NULL;
            break;
        }
    } while(0);
    if (fp) close(fp);

    return buf;
}

/**
 * Return the human-readable file size (e.g., 1.24 K 24.52 M 2.0 G)
 */
static inline const char *file_size(const char *file)
{
    static char filesz[64];
    const char *suf[] = {"Bytes", "KB", "MB", "GB", "TB"};
    int i;
    size_t sz, unit = 1024;
    sz = get_file_size(file);

    for(i=0; (sz/unit) && i<5; unit <<= 10, i++)
        ;
    if (i > 0) {
        unit >>= 10;
        snprintf(filesz, sizeof(filesz), "%.2f %s", 1.0*sz/unit, suf[i]);
    } else {
        snprintf(filesz, sizeof(filesz), "%zu Bytes", sz);
    }

    return filesz;
}

static inline char *get_tmpdir(char *name, int mode, const char *prefix)
{
    if (!prefix) {
        prefix = "/tmp/tmp.";
    }
    sprintf(name, "%sXXXXXX", prefix);
    if (mkdtemp(name)) {
        chmod(name, mode);
    }

    return name;
}

static inline int get_tmpfile_ex(char *name, int suflen)
{
    int fd;
    if ((fd = mkstemps(name, suflen)) >= 0)
        chmod(name, 0666);

    return fd;
}

static inline int get_tmpfile(char *name, int mode, const char *dir, const char *suffix)
{
    int fd, suflen;
    if (suffix) {
        suflen = strlen(suffix);
    } else {
        suffix = ".data";
        suflen = 5;
    }
    if (!dir) dir = "/tmp";
    sprintf(name, "%s/XXXXXX%s", dir, suffix);

    if ((fd = mkstemps(name, suflen)) >= 0)
        chmod(name, mode);

    return fd;
}


#ifdef __cplusplus
}
#endif

#endif
