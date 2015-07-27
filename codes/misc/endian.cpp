#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <arpa/inet.h>

int main(int argc, char* argv[])
{
  const int length = sizeof(int);
  unsigned int i, x, y, xx = 0, yy = 0;
  x = 0x12345678;
  y = htonl(x);
  printf ("In this program y = htonl(x)\n\t"
	  "About endian, see http://en.wikipedia.org/wiki/Endianness\n");
  printf ("x: little-endian\n   %08u 0x%08x\n", x, x);
  printf ("y: big-endian\n   %08u 0x%08x\n", y, y);

  unsigned char strx[8], stry[8];
  memcpy (strx, &x, length);
  memcpy (stry, &y, length);

  for (i=0; i < sizeof(int); ++i) {
    xx |= (strx[i] << 8*i);
    yy |= (stry[i] << 8*(sizeof(int)-i-1));
  }
  printf ("The real value of x and y\n   "
	  "x = %u, y = %u\n", xx, yy);

  return 0;
}
