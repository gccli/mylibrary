#include <stdio.h>

void func_test(int x, int y)
{
  // The XCHG (exchange) instruction swaps the contents of two operands.
    int res = -1;
    __asm__("test %%eax, %%ebx;"
	    "je label_1;"
	    "jne label_2;"
	    "label_1: mov $0, %%ecx;"
	    "label_2: mov $1, %%ecx;"
	    : "=c" (res)
	    : "a" (x), "b" (y)
	);
    printf("result: %d\n", res);
}

int main(int argc, char *argv[])
{
    func_test(0x00, 0xff);
    func_test(0xf0, 0x0f);
    func_test(0xff, 0x0f);
    return 0;
}
