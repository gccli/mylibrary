#include <stdio.h>
#include <stdint.h>

#define IS_CF_SET(x) ((x) & (1 << 0))
#define IS_PF_SET(x) ((x) & (1 << 2))
#define IS_AF_SET(x) ((x) & (1 << 4))
#define IS_ZF_SET(x) ((x) & (1 << 6))
#define IS_SF_SET(x) ((x) & (1 << 7))
#define IS_OF_SET(x) ((x) & (1 << 11))

void dump_flags(uint32_t flags)
{
    printf("flags=0x%04x (CF:%d", flags, IS_CF_SET(flags)?1:0); 
    printf(" PF:%d", IS_PF_SET(flags)?1:0);
    printf(" AF:%d", IS_AF_SET(flags)?1:0);
    printf(" ZF:%d", IS_ZF_SET(flags)?1:0);
    printf(" SF:%d", IS_SF_SET(flags)?1:0);
    printf(" OF:%d)\n", IS_OF_SET(flags)?1:0);
}

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
//    Computes the bit-wise logical AND of first operand (source 1 operand) and the second operand (source 2 operand)
//	and sets the SF, ZF, and PF status flags according to the result.
    uint32_t flags = 0;

    __asm__("test %%eax, %%ebx;"
	    "lahf;" /* Load: AH <-EFLAGS(SF:ZF:0:AF:0:PF:1:CF)*/
	    "mov %%ah, %[f];"
	    "mov %[f], %%edi;"
	    : [f] "=m" (flags)
	    : "a" (x), "b" (y)
	);
    dump(flags);
    return flags;
}

int cmp(int x, int y, void (*dump)(uint32_t))
{
//Compares the first source operand with the second source operand and sets the status flags in the EFLAGS register
//according to the results. The comparison is performed by subtracting the second operand from the first operand
//and then setting the status flags in the same manner as the SUB instruction. When an immediate value is used as
//    an operand, it is sign-extended to the length of the first operand.
    uint32_t flags = 0;

    __asm__("cmp %%eax, %%ebx;"
	    "lahf;"
	    "mov %%ah, %[f];"
	    "mov %[f], %%dh;"
	    : [f] "=m" (flags)
	    : "a" (x), "b" (y)
	);
    dump(flags);
    return flags;
}

int main(int argc, char *argv[])
{
    int ret;
    ret = test(0x00, 0xff, dump_flags); // ZF PF
    ret = test(0xf0, 0x0f, dump_flags); // ZF PF
    ret = test(0xff, 0x0f, dump_flags);
    ret = test(0x80000001, 0xff000001, dump_flags);

    ret = cmp(0x000000ff, 0x000000ff, dump_eflags);
    ret = cmp(0x000000ff, 0x0000000f, dump_eflags);
    ret = cmp(0x000000ff, 0xff0000ff, dump_eflags);
    ret = cmp(0x700000fe, -0x0f000000, dump_eflags);

    return 0;
}
