#include <stdio.h>

int main(int argc, char *argv[])
{
    char ss[] = {
        [1] = '0'+1,
        [7] = '0'+7,
        [35] = '0'+35
    };

    printf("sizeof(ss) = %zu\n", sizeof(ss));
    printf("ss[0]  = '%c' value:%d\n", ss[0], ss[0]);
    printf("ss[7]  = '%c' value:%d\n", ss[7], ss[7]);
    printf("ss[8]  = '%c' value:%d\n", ss[8], ss[8]);
    printf("ss[35] = '%c' value:%d\n", ss[35], ss[35]);

    return 0;
}
