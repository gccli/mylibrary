#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#define GREEN "\033[32;1m"
#define END   "\033[0m"

void printi(int& i)
{
  static int i0=0;
  unsigned char* p = (unsigned char *) &i;
  printf(GREEN"\n%d INTEGET INFO\n"END, ++i0);
  printf("address   :0x%0x\n", p);
  printf("hex value :%x\n", p, i);
  printf("v[0]:%-8x  v[1]:%-8x  v[2]:%-8x  v[3]:%-8x\n", p[0], p[1], p[2], p[3]);
  printf("a[0]:%08x  a[1]:%08x  a[2]:%08x  a[3]:%08x\n", p, p+1, p+2, p+3);
}

void printa(unsigned int addr)
{
  double dx1=addr;
  double dx2=(unsigned int)-1;
  dx2 += 1; // 4*1024*1024*1024
  printf(GREEN"address position:0x%x, precent of total address apace:%lf\n"END, addr, dx1/dx2);
}

int main(int argc, char* argv[])
{
  char c0, c1=1, c2=2;
  int i1=3, i2=4;
  double d1;
  printa((unsigned int)&c0);
  printa((unsigned int)&c1);
  printa((unsigned int)&c2);
  printi(i1);
  printi(i2);

  printf ("\n----------------\n");
  printf ("c1:%x, c2:%x\n", &c1, &c2);
  printf ("i1:%x, i2:%x, i1%4=%d \n", &i1, &i2, (int)&i1%4);
  printf ("d1:%x, d1%8=%d \n", &d1, (int)&d1%8);

  for (int i=0; i<100; ++i) {
    char* p = (char *)malloc(0x100000);
    printa((unsigned int)p);
    printa((unsigned int)sbrk(0));
  }

  return 0;
}
