#include <stdio.h>
#include <stdint.h>

#define IS_CF_SET(x) ((x) & (1 << 0))
#define IS_PF_SET(x) ((x) & (1 << 2))
#define IS_AF_SET(x) ((x) & (1 << 4))
#define IS_ZF_SET(x) ((x) & (1 << 6))
#define IS_SF_SET(x) ((x) & (1 << 7))
#define IS_OF_SET(x) ((x) & (1 << 11))

void dump_eflags(uint32_t flags)
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

int test(int x, int y, void (*dump)(uint32_t))
{
    uint32_t flags = 0;
    __asm__("test %%eax, %%ebx;"
	    "pushfq;"  // Push EFLAGS Register onto the Stack
	    "pop %%rax;" // Pop EFLAGS Register onto the eax
	    "mov %%al, %[f];"
	    "push $0x1;"
	    "push $0x2;"
	    "push $0x3;"
	    "push $0x4;"
	    "pop %%rax;"
	    "pop %%rbx;"
	    "pop %%rcx;"
	    "pop %%rdx;"
	    : [f] "=m" (flags)
	    : "a" (x), "b" (y)
	);

    dump(flags);
    return flags;
}

int main(int argc, char *argv[])
{
    int ret;
    ret = test(0x00, 0xff, dump_eflags); // ZF PF
    ret = test(0xf0, 0x0f, dump_eflags); // ZF PF
    ret = test(0xff, 0x0f, dump_eflags);
    ret = test(0x80000001, 0xff000001, dump_eflags);

    return 0;
}
