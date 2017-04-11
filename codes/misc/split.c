#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char **split(const char *str, char c)
{
    int          n, len;
    char        *p, **dest;
    const char  *start = str;
    const char  *end = str+strlen(str);

    dest = NULL;
    n = 0;
    p = strchr(start, c);
    while(p) {
        len = p - start;
        if (len <= 0) {
            start = p+1;
            p = strchr(start, c);
            continue;
        }

        dest = realloc(dest, (n+2)*sizeof(char *));
        dest[n] = calloc(1, len+1);
        memcpy(dest[n], start, len);
        dest[++n] = NULL;

        start = p+1;
        p = strchr(start, c);
    }

    len = end-start;
    if (len > 0) {
        dest = realloc(dest, (n+2)*sizeof(char *));
        dest[n] = calloc(1, len+1);
        memcpy(dest[n], start, len);
        dest[++n] = NULL;
    }

    printf("%d\n", n);
    return dest;
}

int main(int argc, char *argv[])
{
    int i;
    char *p;
    char **list =  split(argv[1], ',');

    printf("%s -> ", argv[1]);
    for(i=0, p=list[i]; p; p = list[++i]) {
        printf(" [%s]", p);
        free(p);
    }
    printf("\n");
    free(list);

    return 0;
}
