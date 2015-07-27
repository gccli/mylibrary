#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

void __attribute__((stdcall))  printbinary(int n) 
{
  unsigned int i;
  i = 1<<(sizeof(n) * 8 - 1);
  while (i > 0) {
    if (n & i)
      printf("1");
    else
      printf("0");
    i >>= 1;
  }
}
/*
  Bitwise operators

  Symbol Operator
  &      bitwise AND
  |      bitwise inclusive OR
  ^      bitwise exclusive OR
  <<     left shift
  >>     right shift
  ~      one's complement (unary)
**/

const char *printbits(int n, int show)
{
  static char binary[32];
  
  unsigned offset = 0;
  unsigned int i, step;
  if (n == 0) {
    offset = sprintf(binary, "0000");
    binary[offset] = 0;
    return binary;
  }

  i = 1<<(sizeof(n) * 8 - 1);
  step = 0xffffffff;
  step >>= 4;
  while (step >= n) {
    i >>= 4;
    step >>= 4;
  }
  while (i > 0) {
    if (n & i)
      offset += sprintf(binary+offset, "1");
    else
      offset += sprintf(binary+offset, "0");
    i >>= 1;
  }
  binary[offset] = 0;
  if (show) printf("%s\n", binary);

  return binary;
}

int main(int argc, char *argv[])
{
  if (argc < 3)
  {
    printf("usage: %s x y\n", argv[0]);
    return 0;
  }

  int x = atoi(argv[1]);
  int y = atoi(argv[2]);
  printf("x = 0x%08x  %s\n", x, printbits(x, 0));
  printf("y = 0x%08x  %s\n", y, printbits(y, 0));

  int z = x&y;
  printf("x & y = 0x%x & 0x%x = %s (0x%x)\n", x, y, printbits(z,0), z);
  z = x|y;
  printf("x | y = 0x%x & 0x%x = %s (0x%x)\n", x, y, printbits(z,0), z);
  z = x^y;
  printf("x ^ y = 0x%x & 0x%x = %s (0x%x)\n", x, y, printbits(z,0), z);
  z = x^x;
  printf("x ^ x = 0x%x\n", z);

  z = ~x;
  printf("~x = 0x%x = %s\n", z, printbits(z,0));
  z = ~y;
  printf("~y = 0x%x = %s\n", z, printbits(z,0));
  x |= 0x20;
  printf("set 6'th bit for x |= 0x20 = 0x%x, %s\n", x, printbits(x,0));
  x &= ~0x20;
  printf("del 6'th bit for x &= ^0x20 = 0x%x, %s\n", x, printbits(x,0));

  return 0;
}
