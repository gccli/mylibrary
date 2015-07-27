#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>

#include <pthread.h>
#include <semaphore.h>

#define MAXITEMS   100
#define THREADS    10

int nitems;
struct {
  int buffer[MAXITEMS];
  int nput, nputval;
  sem_t mutex, nempty, nstored;
} shared;

void *producer(void *param)
{
  for( ; ; ) {
    sem_wait(&shared.nempty);
    sem_wait(&shared.mutex);

    if(shared.nput >= nitems) {
      sem_post(&shared.nempty);
      sem_post(&shared.mutex);
      return NULL;
    }

    shared.buffer[shared.nput%MAXITEMS] = shared.nputval;
    shared.nput++;
    shared.nputval++;
    sem_post(&shared.mutex);
    sem_post(&shared.nstored);
    *((int *) param) += 1;
  }

  printf("producer done.\n");

  return NULL;
}

void *consumer(void *param)
{
  int i=0;
  for(; i<nitems; ++i) {
    sem_wait(&shared.nstored);
    sem_wait(&shared.mutex);
    if(shared.buffer[i%MAXITEMS] != i) 
      printf("buffer[%d] = %d\n", i, shared.buffer[i%MAXITEMS]);
    sem_post(&shared.mutex);
    sem_post(&shared.nempty);
  }
  printf("consumer done.\n");

  return NULL;
}

int main(int argc, char* argv[])
{
  int i, count[THREADS];
  if(argc == 2)
    nitems = atoi(argv[1]);
  nitems = (nitems<10000) ? 10000 : nitems;
  printf("produce %d item\n", nitems);

  if(sem_init(&shared.mutex ,0, 1) == -1) {
    perror("sem_init");
    return 1;
  }
  if(sem_init(&shared.nempty ,0, MAXITEMS) == -1) {
    perror("sem_init");
    return 1;
  }
  if(sem_init(&shared.nstored ,0, 0) == -1) {
    perror("sem_init");
    return 1;
  }

  pthread_t th_producer[THREADS], th_consumer;
  pthread_attr_t thattr;
  pthread_attr_init(&thattr);
  pthread_attr_setdetachstate(&thattr, PTHREAD_CREATE_DETACHED);

  for(i=0; i<THREADS; ++i) {
    count[i] = 0;
    pthread_create(&th_producer[i], &thattr, producer, &count[i]);
  }
  pthread_create(&th_consumer, NULL, consumer, NULL);
  pthread_join(th_consumer, NULL);

  int total = 0;
  for(i=0; i<THREADS; ++i) {
    total += count[i];
    printf("thread[%d]  produce %d\n", i, count[i]);
  }
  printf("Total: %d\n", total);

  exit(0);
}
