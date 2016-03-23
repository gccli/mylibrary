#include <stdio.h>
#include <string.h>

#ifndef u_char
#define u_char unsigned char
#endif

static const char *memsearch(u_char *mem, u_char *end, u_char *s, size_t n)
{
    size_t i, len;
    u_char *addr = mem;

    do {
        len = end - addr;
        if ((addr = memchr(addr, *s, len)) == NULL)
            break;
        if (len < n)
            break;

        for(i=1; i<n; ++i)
            if (*(s+i) != *(addr+i)) break;
        if (i == n)
            return (char *)addr;
        addr++;
    } while(1);

    return NULL;
}


int main(int argc, char *argv[])
{
    size_t n, len;
    const char *str;
    if (argc == 3) {
        len = strlen(argv[1]);
        n = strlen(argv[2]);
        printf("search '%s' in string '%s'\n", argv[2], argv[1]);
        str = memsearch((u_char *)argv[1], (u_char *)argv[1]+len, (u_char *)argv[2], n);
        if (str) {
            printf("%s %ld\n", str, str-argv[1]);
        }
    }
    return 0;
}
