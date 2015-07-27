#include <stdio.h>

void func1()
{
  char str[256];

  unsigned char i;
  for (i=0; i<256; ++i){
    str[i] = 0;
  }
}

void func2()
{
  long long x = 4;
  char *p = "hello";

  //  printf("%d %s\n", x, p);
  printf("%lld %s\n", x, p);
}


int main(int argc, char *argv[])
{
    func1();
    func2();

    return 0;
}
