#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

char* p = NULL;

int main(int argc, char* argv[])
{
  
  int i=0;
  for (; i < 1024; ++i) {
    p = malloc(10240);
    sleep (1); 
  }

  pause();
  return 0;
}
