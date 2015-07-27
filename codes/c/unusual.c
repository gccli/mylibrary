#include <stdio.h>

int func()
{
  int x = 5;
  printf("++x %d\n", x);
  return x;
}

//int x = func(); // initializer element is not constant


char *mystrcpy(char *dst, const char *src)
{
  char *p = dst;
  while((*p++ = *src++))
    ;
}

int main(int argc, char *argv[])
{
    printf("sizeof(\"\") = %d, sizeof(\"@\") = %d\n", sizeof(""), sizeof("@"));
  int x;
  int y=5;
  //  int x = (++y)++; // 
  //  x = (++y)++; // lvalue required as increment operand

  printf("x = %d\n", x);

  char str[256];
  mystrcpy(str, "hello, world");
  printf("%s\n", str);

  return 0;
}
