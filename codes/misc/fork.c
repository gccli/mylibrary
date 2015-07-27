#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

int main1(int argc, char* argv[])
{
  struct timeval tvstart, tvend;
  pid_t child;

  gettimeofday(&tvstart, NULL);
  printf ("fork start\n");

  if ((child = fork()) < 0) {
    fprintf (stderr, "fork() error %d %s\n", errno, strerror(errno));
    return 1;
  }
  else if (child == 0) {
    int i;
    for (i=0; i<500000000; ++i)
      ;
    return 0;
  }
  gettimeofday(&tvend, NULL);
  
  double timecost = (tvend.tv_sec-tvstart.tv_sec) + 1.0*(tvend.tv_usec-tvstart.tv_usec)/1000000;
  printf ("create child<%d> timecost: %lf\n", child, timecost);
  
  int status;
  waitpid(child, &status, 0);
  if (WIFEXITED(status)) {
    printf("exited, status=%d\n", WEXITSTATUS(status));
  } else if (WIFSIGNALED(status)) {
    printf("killed by signal %d\n", WTERMSIG(status));
  }


  return 0;
}

int main(int argc, char* argv[])
{
    pid_t pid;
    if((pid = fork()))
	printf("%d fork %d\n", getpid(), pid);

    sleep(1);

    if((pid = fork()))
	printf("%d fork %d\n", getpid(), pid);
//    if((pid = fork()))
//	printf("%d fork %d\n", getpid(), pid);

    return 0;
}
