#include <stdio.h>

struct si_t {
    char   magic[4];
    int    size;
    double val;
    short  len;
    char  *data;
};

#define get_entry(ptr, type, member) \
    ((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

int main(int argc, char *argv[])
{
    struct si_t a;
    typeof(a) b, c; // how to use typeof externsion

    printf("&a = 0x%x get_entry(ptr,type,member) = 0x%x\n", &a, get_entry(&a.data, typeof(a), data));
    printf("&b = 0x%x get_entry(ptr,type,member) = 0x%x\n", &b, get_entry(&b.len, typeof(b), len));
    printf("&b = 0x%x get_entry(ptr,type,member) = 0x%x\n", &c, get_entry(&a.val, typeof(c), val));

    return 0;
}
