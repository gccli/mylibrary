#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/time.h>

#include <pthread.h>
#include <semaphore.h>


#define MAXITEMS   1000000
#define MAXTHREADS 5

int nitems;
struct {
  pthread_mutex_t mutex;
  int             buffer[MAXITEMS];
  int             nput;
  int             nval;
} shared = { PTHREAD_MUTEX_INITIALIZER };

struct {
  pthread_mutex_t mutex;
  pthread_cond_t  cond;
  int             nready;
} ready = { PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER };


void *producer(void *param)
{
  for(;;) {
    pthread_mutex_lock(&shared.mutex);
    if (shared.nput >= nitems) {
      pthread_mutex_unlock(&shared.mutex);
      return NULL;
    }
    shared.buffer[shared.nput] = shared.nval++;
    shared.nput++;
    pthread_mutex_unlock(&shared.mutex);

    pthread_mutex_lock(&ready.mutex);
    if (ready.nready == 0) 
      pthread_cond_signal(&ready.cond);
    ready.nready++;
    pthread_mutex_unlock(&ready.mutex);

    *((int *) param) += 1;
  }

  return NULL;
}

void *consumer(void *param)
{
  int i;
  for (i=0; i<nitems; ++i) {
    pthread_mutex_lock(&ready.mutex);
    while (ready.nready == 0)
      pthread_cond_wait(&ready.cond, &ready.mutex);
    ready.nready--;
    pthread_mutex_unlock(&ready.mutex);

    if(shared.buffer[i] != i) {
      printf("buffer[%d] = %d\n", i, shared.buffer[i]);
    } else {
    }
  }
  return NULL;
}

int main(int argc, char* argv[])
{
  int i, nthreads = MAXTHREADS;
  nitems = MAXITEMS;
  
  int total = 0, count[MAXTHREADS];
  pthread_t th[MAXTHREADS];

  pthread_attr_t thattr;
  pthread_attr_setdetachstate(&thattr, PTHREAD_CREATE_DETACHED);
  for(i=0; i<nthreads; ++i) {
    count[i] = 0;
    pthread_create(&th[i], 0, producer, &(count[i]));
  }
  pthread_t consume;
  pthread_create(&consume, NULL, consumer, NULL);

  pthread_join(consume, NULL);
  printf("\n");
  for(i=0; i<nthreads; ++i) {
    pthread_join(th[i], NULL);
    printf("thread[%d]  produce %d\n", i, count[i]);
    total += count[i];
  }
  printf("Total: %d\n", total);

  return 0;
}
