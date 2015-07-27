#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

int main(int argc, char* argv[])
{
  for (int i=0; ; ++i) {
    printf ("%d", i);
  }

  return 0;
}
