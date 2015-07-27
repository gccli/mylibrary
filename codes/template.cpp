#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <pthread.h>
#include <signal.h>

#define gettid() syscall(__NR_gettid)


void signal_fun(int signo)
{
    printf ("catch a signal %d\n", signo);
    if (signo == SIGINT)
    {
    }
}

void* thread_fun(void* param)
{
  int lwpid = gettid();

  return NULL;
}

int main(int argc, char* argv[])
{
    // signal(SIGINT, signal_fun);


    int thread_size = 2;
    if (argc > 1)
	thread_size = atoi(argv[1]);

    pthread_t th[thread_size];
    for (int i=0; i<thread_size; ++i)
	pthread_create(&th[i], NULL, thread_fun, NULL);

    for (int i=0; i<thread_size; ++i)
	pthread_join(th[i], NULL); 

  return 0;
}
