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
#define P_THREADS  10
#define C_THREADS  3

int nitems;
struct {
  int buffer[MAXITEMS];
  int nput, nputval;
  int nget, ngetval;
  sem_t mutex, nempty, nstored;
} shared;

void *producer(void *param)
{
  for( ; ; ) {
    sem_wait(&shared.nempty);
    sem_wait(&shared.mutex);

    if(shared.nput >= nitems) {
      sem_post(&shared.nstored); // let consumer thread terminate
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
  for( ; ; ) {
    sem_wait(&shared.nstored);
    sem_wait(&shared.mutex);

    if(shared.nget >= nitems) {
      sem_post(&shared.nstored);
      sem_post(&shared.mutex);
      return NULL;
    }
    i = shared.nget % MAXITEMS;
    if(shared.buffer[i] != shared.ngetval) 
      printf("buffer[%d] = %d\n", i, shared.buffer[i%MAXITEMS]);

    shared.nget++;
    shared.ngetval++;

    sem_post(&shared.mutex);
    sem_post(&shared.nempty);

    *((int *) param) += 1;    
  }
  printf("consumer done.\n");

  return NULL;
}

int main(int argc, char* argv[])
{
  int i, p_count[P_THREADS], c_count[C_THREADS];
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

  pthread_t th_producer[P_THREADS], th_consumer[C_THREADS];
  pthread_attr_t thattr;
  pthread_attr_init(&thattr);
  pthread_attr_setdetachstate(&thattr, PTHREAD_CREATE_DETACHED);

  for(i=0; i<P_THREADS; ++i) {
    p_count[i] = 0;
    pthread_create(&th_producer[i], &thattr, producer, &p_count[i]);
  }

  for(i=0; i<C_THREADS; ++i) {
    c_count[i] = 0;
    pthread_create(&th_consumer[i], NULL, consumer, &c_count[i]);
  }

  int total = 0;
  for(i=0; i<C_THREADS; ++i) {
    pthread_join(th_consumer[i], NULL);
    printf("consumer thread[%d]  consume %d\n", i, c_count[i]);
    total += c_count[i];
  }
  printf("Total consume: %d\n", total);

  total = 0;
  for(i=0; i<P_THREADS; ++i) {
    total += p_count[i];
    printf("producer thread[%d]  produce %d\n", i, p_count[i]);
  }
  printf("Total produce: %d\n", total);

  exit(0);
}
