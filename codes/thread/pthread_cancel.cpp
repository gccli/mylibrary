#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <sys/syscall.h>
#define gettid() syscall(__NR_gettid)

pthread_mutex_t       mutex;



void readline_destructor() 
{
  printf ("call readline_destructor\n");
}


void *thread_fun(void *p)
{
    return NULL;
}

int main(int argc, char* argv[])
{
  pthread_mutex_init(&mutex, NULL);

  pthread_t th1, th2;
  pthread_create(&th1, NULL, thread_fun, NULL);
  pthread_create(&th2, NULL, thread_fun, NULL);

  pthread_join(th1, NULL); 
  pthread_join(th2, NULL); 

  return 0;
}
