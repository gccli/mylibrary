#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <sys/syscall.h>
#define gettid() syscall(__NR_gettid)
#define DEFAULT_THREAD_POOLSIZE 10
typedef void* (*thread_func) (void*);

pthread_mutex_t mutex;
pthread_cond_t  cond;
int  count = 1;
const int count_limit = 10;
bool terminate = false;
void* thread_poolfunc(void* param);
void* thread_wakeup(void* param);

struct thread_param 
{
  unsigned int id;
  char         *buf; 
};

int main(int argc, char* argv[])
{
  int i=0, thread_poolsize = DEFAULT_THREAD_POOLSIZE;
  if (argc > 1) 
    thread_poolsize = atoi (argv[1]);

  pthread_mutex_init(&mutex, 0);
  pthread_cond_init (&cond, 0);
  pthread_attr_t thattr;
  pthread_attr_init(&thattr);

  pthread_t thmain, th[thread_poolsize];
  thread_param* param = new thread_param[thread_poolsize];
  memset (param, 0, thread_poolsize*sizeof(thread_param));
  for (i=0; i<thread_poolsize; ++i) {
    pthread_create(&th[i], &thattr, thread_poolfunc, (void*) &param[i]);
  }
  thmain = pthread_create (&thmain, &thattr, thread_wakeup, NULL);
  sleep (1);


  void *mem = NULL;
  int fd = open ("/tmp/mem", O_RDWR|O_CREAT, 0600);
  int len;
  if ((len=write (fd, param, thread_poolsize*sizeof(thread_param))) < 0)
    perror ("write");
  off_t offset = lseek (fd, SEEK_SET, 0);

  if ((mem = mmap (0, len, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0)) == MAP_FAILED)
    return 1;
  printf ("memory address:   %x\n", mem);
  thread_param* p = (thread_param* )mem;
  for (i=0; i<thread_poolsize; i++, p += 1) 
    printf ("read from memory: %d\n", p->id);

  close (fd);
  //  munmap (mem, 10240);

  for (i=0; i<thread_poolsize; ++i)
    pthread_join(th[i], NULL);
  pthread_join(thmain, NULL);

  return 0;
}

void* thread_poolfunc(void* param)
{
  thread_param* p = (thread_param* )param;
  p->id = gettid();

  while (!terminate) {
    pthread_mutex_lock (&mutex);       // lock
    if (count%count_limit != 0) 
      pthread_cond_wait (&cond, &mutex); // wait

    printf ("thread id<%d> wakeup \n", gettid());

    pthread_mutex_unlock(&mutex);      // unlock
  }
}

void* thread_wakeup(void* param)
{
  printf ("main control thread created\n");

  while (!terminate) {
    pthread_mutex_lock (&mutex);       // lock
    if (count%count_limit == 0) {
      pthread_cond_signal (&cond); // wake one of thread in thread pool
    }
    pthread_mutex_unlock(&mutex);      // unlock

    count++;
    usleep (100000);
  }
}
