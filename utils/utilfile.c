#include "utilfile.h"

#ifdef __cplusplus
extern "C" {
#endif

char *get_file_buffer(const char *file, size_t *r)
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

const char *file_size(const char *file)
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

char *get_tmpdir(char *name, int mode, const char *prefix)
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

int get_tmpfile_ex(char *name, int suflen)
{
    int fd;
    if ((fd = mkstemps(name, suflen)) >= 0)
        chmod(name, 0666);

    return fd;
}

int get_tmpfile(char *name, int mode, const char *dir, const char *suffix)
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


#ifdef _TEST
int main(int argc, char *argv[])
{
    char buf[1024];
    int fd = get_tmpfile(buf, 0666, argc>1?argv[1]:NULL, argc>2?argv[2]:NULL);


    printf("tmpfile %s fd:%d\n", buf, fd);
    printf("tmpdir %s\n", get_tmpdir(buf, 0755, "data."));

    return 0;
}
#endif
