#include <stdio.h>

void func(int a, int i)
{
  printf("%d %d\n", a, i);
}

int main(int argc, char *argv[])
{
  int a[6] = {0,1,2,3,4,5};
  int i=0;
  func(a[i++], i++);
  printf("%d\n", i);

  return 0;
}
