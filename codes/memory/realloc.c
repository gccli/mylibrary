#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <signal.h>
#include <sys/resource.h>
#include <malloc.h>
//int getrusage(int who, struct rusage *usage);
struct st
{
  int  i;
  char c[10240];
};

int main(int argc, char *argv[])
{
  struct rusage ru;
  memset(&ru, 0, sizeof(ru));

  int i=1; 
  struct st *s = NULL;
  
  for (; i<1000; ++i) {
    s = realloc(s, i*sizeof(struct st));
    usleep(20000);
    printf("(%3d) ----- resource usage\n", i);
    malloc_stats();
  }
  free(s);
  sleep(1);
  printf("(%3d) --------------------------------------------------------------------------------\n", i);
  malloc_stats();

  getchar();
  return 0;
}

