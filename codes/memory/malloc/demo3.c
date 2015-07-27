#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 102400
char *p[SIZE];

int main(int argc, char *argv[])
{
  int i;
  for(i=0; ;i++) {
    p[i] = malloc(1024*1024);
    if(p[i] == NULL) {
      perror("malloc");
      break;
    }
    printf("demo3 got %d MB memory\n", i);
  }

  for(i=0; p[i] != NULL; ++i) {
    memset(p[i], 0, 1024*1024-1);
    printf("demo3 used %d MB memory\n", i);
  }

  return 0;
}
 
