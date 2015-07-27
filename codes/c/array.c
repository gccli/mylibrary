#include <stdio.h>

void func_array(char array[])
{
  putchar(array[0]);
}

void func_pointer(char *array)
{
  putchar(array[0]);
}


int main(int argc, char *argv[])
{

  char buffer[16] = "hello, world";
  func_array(buffer);
  func_pointer(buffer);

  return 0;
}
