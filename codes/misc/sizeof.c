#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

int sizefunc(int iarray[10], char sarray[20])
{
    const int wordsize = __WORDSIZE;

    char str[] = "hello, world";
    char *ptrc[10];
    long *ptrl[10];
    printf("WORDSIZE %d  ", wordsize);

# if __WORDSIZE == 64
    printf("In 64-bit system\n");
# else
    printf("In 32-bit system\n");
# endif

    printf("sizeof(char)                   = %u\n", sizeof(char));
    printf("sizeof(short)                  = %u\n", sizeof(short));
    printf("sizeof(int)                    = %u\n", sizeof(int));
    printf("sizeof(long)                   = %u\n", sizeof(long));
    printf("sizeof(long int)               = %u\n", sizeof(long int));
    printf("sizeof(long long)              = %u\n", sizeof(long long));
    printf("sizeof(array)                  = %u\n", sizeof(str));
    printf("sizeof(pointer char array)     = %u\n", sizeof(ptrc));
    printf("sizeof(pointer long array)     = %u\n", sizeof(ptrl));
    printf("sizeof(int array parameter)    = %u\n", sizeof(iarray));
    printf("sizeof(char array parameter)   = %u\n", sizeof(sarray));
    printf("sizeof(function)               = %u\n", sizeof(sizefunc));

    return 0;
}


int main(int argc, char *argv[])
{
    int  iarr[10];
    char sarr[20];

    sizefunc(iarr, sarr);
    return 0;
}
