#include <stdio.h>

void func1()
{
  int i, sum=0;
  for (i=0; i<100; ++i)
    sum += i;
}

int main()
{
  int i;
  int x=0x80000001, y=0xffffff00, z=0x1000;
  i = x&y;
  printf("0x%x\n", i);

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
  
  return z;
}
