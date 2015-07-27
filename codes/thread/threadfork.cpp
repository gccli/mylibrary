#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <sys/syscall.h>
#define gettid() syscall(__NR_gettid)

void signal_fun(int signo)
{
  printf ("catch a signal %d\n", signo);
  if (signo == SIGINT) {
  }
}


void printFun(const char* fmt, ...)
{
  char buffer[1024] = "\0";
  struct tm Tm, *pTm;
  struct timeval tv;
  gettimeofday (&tv, NULL);
  pTm = localtime_r(&tv.tv_sec, &Tm);

  sprintf(buffer, "thread[%d] %04d/%02d/%02d %02d:%02d:%02d.%03d ", gettid(), 
	  pTm->tm_year+1900, pTm->tm_mon+1, pTm->tm_mday,
	  pTm->tm_hour, pTm->tm_min, pTm->tm_sec, tv.tv_usec/1000);
  int buflen = strlen(buffer);

  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer+buflen, 1024-buflen-2, fmt, args);
  va_end(args);

  printf("%s\n", buffer);
  fflush(stdout);
}

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
void sendtoParent(int pipefd, const char* buffer)
{
  pthread_mutex_lock(&mutex);

  int wlen = write(pipefd, buffer, strlen(buffer));
  printf("child write len = %d\n", wlen);

  pthread_mutex_unlock(&mutex);
}

void* thread_fun(void* param)
{
  int pipefd[2];
  pipe(pipefd);

  pid_t child;
  if ((child = fork()) < 0) {
    perror("fork");
    return NULL;
  }
  else if (child == 0) {
    usleep(1000);
    printFun("\033[31;1mRun in Child Process [1]...\033[0m"); 
    sendtoParent(pipefd[1], "1. Child Send to parent ...");
    usleep(1000);
    printFun("\033[31;1mRun in Child Process [2]...\033[0m"); 
    sendtoParent(pipefd[1], "2. Child Send to parent ...");
    usleep(1000);
    exit(0);
  }
  usleep(10);
  close(pipefd[1]);

  fd_set rfds;
  struct timeval tv;
  while (true) {
    tv.tv_sec = 1;  tv.tv_usec = 0;
    FD_ZERO(&rfds);
    FD_SET(pipefd[0], &rfds);
    int count = select(pipefd[0]+1, &rfds, NULL, NULL, &tv);
    if (count == 0) {
      continue;
    }
    char buffer[1024];
    int retval = read(pipefd[0], buffer, sizeof(buffer));
    printf ("Child Write to Me: %s\n", buffer);
    if (*buffer == '2') {
      printFun("Child Quiting.. ");
      break;
    }
  }
  close(pipefd[0]); 

  int status;
  int w = waitpid(child, &status, 0);
  if (w == child) {
    printf ("child %d quit.\n", w);
  }
  return NULL;
}

int main(int argc, char* argv[])
{
  // signal(SIGINT, signal_fun);

  int thread_size = 20;
  if (argc > 1)
    thread_size = atoi(argv[1]);

  pthread_attr_t thattr;
  pthread_attr_init(&thattr);
  pthread_attr_setdetachstate(&thattr, PTHREAD_CREATE_DETACHED); 

  for (int i=0; i<thread_size; ++i) {
    pthread_t th;
    pthread_create(&th, &thattr, thread_fun, NULL);
  }

  pause();
  return 0;
}
