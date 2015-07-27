#include <stdio.h>

// REF: http://en.wikipedia.org/wiki/Operators_in_C_and_C%2B%2B

int main(int argc, char *argv[])
{
  int a[4][3];

  // ap is a array of pointer-to int (*(ap[4])) NOT a pointer to array of ints (int (*ap)[4])
  int *ap[4] = {a[0], a[1], a[2], a[3]};
  

  printf("0x%08x 0x%08x 0x%08x 0x%08x\n", ap[0], ap[1], ap[2], ap[3]);

  return 0;
}
