#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

void dump(unsigned int n)
{
    int i;
    unsigned char *p = (unsigned char *) &n;

    printf("0x%08x  ", n);
    for(i=0; p[i] != 0 && i<4; ++i)
	if (isprint(p[i]))
	    printf("%c", p[i]);
	else 
	    printf(".");
    printf("\n");
}

void cpuid(int eax)
{
    uint32_t ebx=0,ecx=0,edx=0;
    __asm__("cpuid\n"
	    "mov %%ebx, %[b]\n"
	    "mov %%ecx, %[c]\n"
	    "mov %%edx, %[d]\n"
	    :[b]"=m" (ebx), [c]"=m" (ecx), [d]"=m" (edx), "=a" (eax)
	    :"a" (eax)
	    :
	);


    printf("Hex         Ascii\n");
    dump(eax);
    dump(ebx);
    dump(ecx);
    dump(edx);
}

int main()
{
    cpuid(0);
    cpuid(0x1);
    cpuid(0x4);
    cpuid(0x80000002);

    return 0;
}
