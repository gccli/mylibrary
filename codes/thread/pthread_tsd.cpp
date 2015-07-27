#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <sys/syscall.h>
#define gettid() syscall(__NR_gettid)

#define MAXLINE 1024
struct RLineTSD {
  int   rl_cnt;
  char* rl_bufptr;
  char  rl_buf[MAXLINE];
};

pthread_key_t  rl_key;
pthread_once_t rl_once = PTHREAD_ONCE_INIT;
void readline_destructor(void* ptr) 
{
  printf ("call readline_destructor\n");
  free(ptr);
}
void readline_once()
{
  printf ("call readline_once\n");
  pthread_key_create(&rl_key, readline_destructor);
}

static int read_internal(struct RLineTSD* tsd, int fd, char* ptr)
{
  if (tsd->rl_cnt <= 0) {
    if ((tsd->rl_cnt = read(fd, tsd->rl_buf, MAXLINE)) < 0) {
      perror ("read");
      return -1;
    }
    else if (tsd->rl_cnt == 0) return 0;
    tsd->rl_bufptr = tsd->rl_buf;
  }
  
  tsd->rl_cnt--;
  *ptr = *tsd->rl_bufptr++;
  return 1;
}

size_t readline(int fd, char *line, int len)
{
  int n, rc;
  char c, *ptr;
  RLineTSD* tsd = NULL;

  pthread_once(&rl_once, readline_once);
  if ((tsd = (RLineTSD*) pthread_getspecific(rl_key)) == NULL) {
    tsd = (RLineTSD* ) calloc(1, sizeof(RLineTSD));
    pthread_setspecific(rl_key, tsd);
  }

  ptr = line;
  for (n=1; n<len;++n) {
    if ((rc = read_internal(tsd, fd, &c)) == 1) {
      *ptr++ = c;
      if (c == '\n') break;
    }
    else if (rc == 0) {
      *ptr = 0;
      return (n-1);
    }
    else return -1;
  }

  *ptr = 0;
  return n;
}

void* thread_fun(void* param)
{
  //  pthread_detach(pthread_self());
  char* buffer = new char[1024];
  int maxlength = 1024;
  size_t rlen; 
  while ((rlen = readline(0, buffer, maxlength)) > 0) {
    buffer[rlen-1] = 0;
    if (strcmp (buffer, "quit") == 0) break;
    printf ("thread<%d> read buffer: %s, length: %d\n", gettid(), buffer, rlen);
  }

  delete [] buffer;

  return NULL;
}

int main(int argc, char* argv[])
{
  pthread_t th1, th2;
  pthread_create(&th1, NULL, thread_fun, NULL);
  pthread_create(&th2, NULL, thread_fun, NULL);

  pthread_join(th1, NULL); 
  pthread_join(th2, NULL); 

  return 0;
}
