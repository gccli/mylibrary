#include <stdio.h>
#include <stdint.h>

#define IS_CF_SET(x) ((x) & (1 << 0))
#define IS_PF_SET(x) ((x) & (1 << 2))
#define IS_AF_SET(x) ((x) & (1 << 4))
#define IS_ZF_SET(x) ((x) & (1 << 6))
#define IS_SF_SET(x) ((x) & (1 << 7))
#define IS_OF_SET(x) ((x) & (1 << 11))

int global_flag = 1;

void dump_flags(uint32_t flags)
{
    printf("flags=0x%04x (", flags);
    if(IS_CF_SET(flags)) printf(" CF");
    if(IS_PF_SET(flags)) printf(" PF");
    if(IS_AF_SET(flags)) printf(" AF");
    if(IS_ZF_SET(flags)) printf(" ZF");
    if(IS_SF_SET(flags)) printf(" SF");
    if(IS_OF_SET(flags)) printf(" OF");
    printf(" )\n");
}

int logical_branch(int x, int y, void (*dump)(uint32_t))
{
    int ret = 0;
    if (x > y) {
	ret = x-y;
	printf("x greater than y\n");
    } else if (x < y) {
	ret = y-x;
	printf("y greater than x\n");
    } else  {
	ret = 0;
    }

    return ret;
}

int logical_and(int x, int y)
{
    int ret = 0;
    if (x > 0 && y > 0 && x > y) {
	printf("x greater than y\n");
    }

    return ret;
}

int logical_or(int x, int y)
{
    int ret = 0;
    if (x > 0 || y > 0 || x > y) {
	printf("ok\n");
	return x;
    }

    return ret;
}

int logical_and_or(int x, int y, int z)
{
    int ret = 0;
    if ((y > 0 && x > 0) || z > y) {
	printf("ok\n");
	return x;
    }

    return ret;
}

int logical_switch(int x)
{
    int ret;
    switch (x) {
	case 'A':
	    ret = 0x10;
	    break;
	case 'B':
	    ret = 0x20;
	    break;
	case 'C':
	    ret = 0x30;
	    break;
	case 'D':
	case 'E':
	case 'F':
	    ret = 0xff;
	    break;
	default:
	    printf("unreachable\n");
	    ret = 0;
    }
    return ret;
}

int logical_switch2(int x)
{
    int ret;
    switch (x) {
	case 'A':
	    ret = 0x10;
	    break;
	case 'C':
	    ret = 0x30;
	    break;
	case 'D':
	case 'E':
	    ret = 0x50;
	    break;
	case '0':
	    ret = 0x100;
	    break;
	case 'z':
	    ret = 0x200;
	    break;
	default:
	    printf("unreachable\n");
	    ret = 0;
    }
    return ret;
}

int logical_loop(int x)
{
    int ret = 0;
    int i;
    for(i=0; i<x; ++i) {
	if (i%3 == 0) continue;
	ret += (i+1);
	if (!global_flag)
	    break;
    }

    return ret;
}


int main(int argc, char *argv[])
{
    int ret;
    ret = logical_branch(0x00, 0xff, dump_flags);
    ret = logical_branch(0xf0, 0x0f, dump_flags);
    ret = logical_branch(0xff, 0x0f, dump_flags);
    ret = logical_branch(0x80000001, 0xff000001, dump_flags);

    logical_and(0xff, 0xf1);
    logical_or(0xff, 0xf1);
    
    logical_switch('z');
    logical_switch('C');

    logical_switch2('z');
    logical_switch2('C');

    logical_loop(100);

    return 0;
}
