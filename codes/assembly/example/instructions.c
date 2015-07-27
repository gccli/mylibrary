#include <stdio.h>

void func_xchg(int *x, int *y)
{
  // The XCHG (exchange) instruction swaps the contents of two operands.
  __asm__("xchg %%eax, %%ebx;\n"
	  : "=a" (*x), "=b" (*y)
	  : "a" (*x), "b" (*y)
	  );
}

void func_bswap(int *x)
{
  // BSWAP (byte swap) instruction reverses the byte order in a 32-bit register operand.
  __asm__("bswap %%eax;\n"
	  : "=a" (*x)
	  : "0" (*x)
	  );
}

void func_xadd(int *x, int *y)
{
  // The XADD (exchange and add) instruction swaps two operands and then stores the
  // sum of the two operands in the destination operand
  __asm__("xadd %%eax, %%ebx;\n"
	  : "=a" (*x), "=b" (*y)
	  : "a" (*x), "b" (*y)
	  );
}

void func_cmpxchg(int x, int y, int *z)
{
  /*
  __asm__("cmpxchg %%eax, %%ebx, %%ecx;\n"
	  : "=c" (*z)
	  : "a" (x), "b" (y)
	  );
  */
}

void func_pusha()
{
  // The PUSHA instruction saves the contents of the eight general-purpose registers
  // The registers are pushed on the stack in the following order: EAX, ECX, EDX,
  // EBX, the initial value of ESP before EAX was pushed, EBP, ESI, and EDI
  int x=0, y=0;

  __asm__("movl %%eax, %0;\n"
	  "movl %%ecx, %1;\n"
	  : "=a" (x), "=b" (y) );
  printf("eax 0x%x, ecx 0x%x\n", x, y);
  __asm__("pusha;\n");

  x=100;y=100;
  __asm__("leal (%%eax), %%eax;\n"
	  "leal (%%ecx), %%ecx;\n"
	  : :"a" (x), "b" (y) );

  __asm__("popa;\n");

  __asm__("movl %%eax, %0;\n"
	  "movl %%ecx, %1;\n"
	  : "=a" (x), "=b" (y) );
  printf("eax 0x%x, ecx 0x%x\n", x, y);
}

void func_binary_arithmetic()
{
  int x=100, y=110, z=0;
  __asm__("addl %%ebx, %%ecx;\n"
	  "addl %%eax, %%ecx;\n"
	  :"=c" (z)
	  :"a" (x), "b" (y));
  printf("x=%d, y=%d, z=%d\n", x, y, z);
  __asm__("subl %%ecx, %2;\n"
	  :"=c" (z), "=a" (x)
	  :"a" (x), "b" (y), "0" (z));
  printf("x=%d, y=%d, z=%d\n", x, y, z);

}

void func_shift()
{
  int x=0x80000001, y=0xffffff00, z=0x1000;
  __asm__("shl $1, %%eax\n"
	  "sal $16, %%ecx\n"
	  "shr $16, %%ebx\n"
	  :"=a" (x), "=b" (y), "=c" (z)
	  :"a" (x), "b" (y), "c" (z)
	  );
  printf("x=0x%x, y=0x%x, z=0x%x\n", x, y, z);

  x=0x80000001; y=0xffffff00; z=0x1000;
  __asm__("sal $1, %%eax\n"
	  "shl $16, %%ecx\n"
	  "sar $16, %%ebx\n"
	  :"=a" (x), "=b" (y), "=c" (z)
	  :"a" (x), "b" (y), "c" (z)
	  );
  printf("x=0x%x, y=0x%x, z=0x%x\n", x, y, z);
}

int main()
{
  int x=5, y=6, c=7;
  func_xchg(&x, &y);
  printf("x = 0x%x, y = 0x%x\n", x, y);
  func_bswap(&x);
  printf("x = 0x%x\n", x);
  func_xadd(&x, &y);
  printf("x = 0x%x, y = 0x%x\n", x, y);

  func_pusha();
  func_binary_arithmetic();
  func_shift();

  
  return c;
}
