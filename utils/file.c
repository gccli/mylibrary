#include "utils/file.h"

#ifdef _TEST
int main(int argc, char *argv[])
{
    size_t len;
    char name[1024], *buf;
    int fd = get_tmpfile(name, 0666, argc>1?argv[1]:NULL, argc>2?argv[2]:NULL);
    printf("tmpfile %s fd:%d\n", name, fd);
    write(fd, name, sizeof(name));
    close(fd);

    buf = get_file_buffer(name, &len);
    if (buf) {
        printf("length %zu - %s\n", len, buf);
    }

    printf("tmpdir %s\n", get_tmpdir(name, 0755, "data."));

    return 0;
}
#endif
