#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <string>
#include <stack>

void dec2radix(int a, int radix)
{
    std::stack<int> s;
    int n = a;
    for(; n > 0; n = n/radix)
	s.push(n%radix);
    while(!s.empty()) {
	printf("%c", (s.top()<10) ? ('0' + s.top()) : ('A' + s.top() - 10) );
	s.pop();
    }
    printf("\n");
}

void dec2hex(int a)
{
    dec2radix(a, 16);
}

void dec2bin(int a)
{
    dec2radix(a, 2);
}

int main(int argc, char *argv[])
{
  if (argc < 2) {
    printf("Usage: %s <number>\n", argv[0]);
    return 1;
  }
  int a = atoi(argv[1]);
  printf("DEC: %d\n", a);
  printf("HEX: ");
  dec2hex(a);
  printf("BIN: ");
  dec2bin(a);
  printf("OCT: ");
  dec2radix(a, 8);
  printf("26: ");
  dec2radix(a, 26);

  return 0;
}
