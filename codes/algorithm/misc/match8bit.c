#include <stdio.h>

int match8bit(const unsigned char *buffer, unsigned char c)
{
    unsigned const char *p = buffer;
    do
    {
	unsigned short x = *(unsigned short *) p;
	if ((x & 0xff00) == 0) // x < 256  =>  *(p+1) == 0
	    break;
	printf("x = 0x%04x\n", x);
	for(; x; x >>= 1)
	{
	    printf("  x = 0x%04x\n", x);
	    unsigned char cc = x & 0x00ff;
	    if (c == (x & 0xff))
	    {
		printf("matched x = 0x%04x\n", x);
		return 1;
	    }
	    if (c > x) break;
	}
    }while(*(++p));

    return 0;	
}

char *mystrcpy(char *dst, char *src)
{
    char *p = src;
    char *d = dst;
    unsigned int i = 0;
    while(*p)
    {
	if (((int )p % 4) == 0) 
	{
	    i = *(unsigned int *) p;
	    if ((*d = (i & 0xff)) == 0)
		break;
	    if ((*(d+1) = ((i>>8)&0xff)) == 0)
		break;
	    if ((*(d+2) = ((i>>16)&0xff)) == 0)
		break;
	    if ((*(d+3) = ((i>>24)&0xff)) == 0)
		break;
	    p += 4;
	    d += 4;
	}
	else
	{
	    *d++ = *p++;
	}
    }

    return dst;
}

char *mystrcpy2(char *dst, char *src)
{
    char *p = src;
    char *d = dst;
    while((*d++ = *p++))
	;
    return dst;
}


int main(int argc, char *argv[])
{
    char buffer[1024] = {0};
    int i=0;
    for (i=0; i<10000000; ++i)
	strcpy(buffer, argv[1]);

    return 0;
}

/*
int main(int argc, char *argv[])
{
    unsigned char str[] = "abcd";
    match8bit(str, 0x8d);
    printf("\n");
    match8bit(str, 0x19);

    char buffer[128] = {0};
    printf("%s\n", mystrcpy(buffer, argv[1]));

    return 0;
}
*/
