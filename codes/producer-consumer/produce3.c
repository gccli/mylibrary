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
int nitems;
struct {
  int buffer[MAXITEMS];
  sem_t mutex, nempty, nstored;
} shared;

void *producer(void *param)
{
  int i;
  for(i=0; i<nitems; ++i) {
    sem_wait(&shared.nempty);
    sem_wait(&shared.mutex);
    shared.buffer[i%MAXITEMS] = i;
    sem_post(&shared.mutex);
    sem_post(&shared.nstored);
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

  pthread_t th_producer, th_consumer;
  pthread_attr_t thattr;
  pthread_attr_init(&thattr);
  pthread_attr_setdetachstate(&thattr, PTHREAD_CREATE_DETACHED);
  pthread_create(&th_producer, &thattr, producer, NULL);
  pthread_create(&th_consumer, NULL, consumer, NULL);

  pthread_join(th_consumer, NULL);

  return 0;
}
