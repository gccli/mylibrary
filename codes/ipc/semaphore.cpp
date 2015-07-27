#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <errno.h>

#include <sys/syscall.h>
#include <sys/time.h>
#include <time.h>
#define gettid() syscall(__NR_gettid)

sem_t sem;
void signal_fun(int signo)
{
  printf ("catch a signal %d\n", signo);
  if (signo == SIGINT) {
    sem_post(&sem);
    int val;
    sem_getvalue(&sem, &val);
    printf ("sem_post() from handler, sem_getvalue() return %d\n", val);
  }
}

void* thread_fun(void* param)
{
  int lwpid = getpid();
  

  return NULL;
}

// g++ -g semaphore.cpp -lpthread -lrt

int main(int argc, char* argv[])
{
  signal(SIGINT, signal_fun);
  sem_init(&sem, 0, 0);

  /**
     long sys_clock_gettime (clockid_t which_clock, struct timespec *tp);
     CLOCK_REALTIME
     Systemwide realtime clock.

     CLOCK_MONOTONIC
     Represents monotonic time. Cannot be set.

     CLOCK_PROCESS_CPUTIME_ID
     High resolution per-process timer.

     CLOCK_THREAD_CPUTIME_ID
     Thread-specific timer.

     CLOCK_REALTIME_HR
     High resolution version of CLOCK_REALTIME.

     CLOCK_MONOTONIC_HR
     High resolution version of CLOCK_MONOTONIC
   */

  struct timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  ts.tv_sec += 2;

  printf ("timespec tv_sec:%ld, tv_nsec:%09ld\n", ts.tv_sec, ts.tv_nsec);
  int ret;
  while ((ret = sem_timedwait(&sem, &ts)) == -1 && errno == EINTR)
    perror ("sem_wait");
  
  printf ("ret = %d\n", ret);

  pthread_t th1, th2;
  pthread_create(&th1, NULL, thread_fun, NULL);
  pthread_create(&th2, NULL, thread_fun, NULL);

  pthread_join(th1, NULL); 
  pthread_join(th2, NULL); 

  return 0;
}
