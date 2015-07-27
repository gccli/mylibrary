#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <pthread.h>
#include <semaphore.h>

#include <errno.h>

#define  BUFFER_NUM  4
#define  BUFFER_SIZE 32768

struct {
  struct {
    char *data;
    int   datalen;
  } buffer[BUFFER_NUM];
  sem_t mutex, nempty, nstored;
} shared;


void *producer(void *param)
{
  int i, fd;
  if ((fd = open((char *) param, O_RDONLY)) < 0) {
    perror("open");
    return NULL;
  }

  struct stat st;
  if (fstat (fd, &st) < 0) {
    fprintf (stderr, "stat() error: %d %s\n", errno, strerror(errno));
    return NULL;
  }
  int length = st.st_size;
  int fsize = 0, left = length;

  void *mem;
  if ((mem = mmap (0, length, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
    fprintf (stderr, "mmap() error: %d %s\n", errno, strerror(errno));
    return NULL;
  }

  for(i=0; ; ) {
    sem_wait(&shared.nempty);
    sem_wait(&shared.mutex);
    // critical region
    sem_post(&shared.mutex);

    shared.buffer[i].datalen = BUFFER_SIZE > left ? left : BUFFER_SIZE;

    if (shared.buffer[i].datalen <= 0) {
      sem_post(&shared.nstored);
      break;
    }
    else {
      shared.buffer[i].data = mem + fsize;
      left  -= shared.buffer[i].datalen;
      fsize += shared.buffer[i].datalen;
      //memcpy(shared.buffer[i].data, mem, shared.buffer[i].datalen);
    }
    if (++i >= BUFFER_NUM) i = 0;

    sem_post(&shared.nstored);
  }

  printf("producer done, write %d bytes\n", fsize);
  close(fd);
  //  munmap(mem, length);

  return NULL;
}

void *consumer(void *param)
{
  int fsize = 0;
  int i=0, fd;

  if ((fd = open((char *)param, O_RDWR|O_CREAT|O_TRUNC, 0600)) < 0) {
    perror("open");
    return NULL;
  }

  for(; ; ) {
    sem_wait(&shared.nstored);
    sem_wait(&shared.mutex);
    // critical region
    sem_post(&shared.mutex);
    if(shared.buffer[i].datalen <= 0) 
      break;

    int wlen = write(fd, shared.buffer[i].data, shared.buffer[i].datalen);
    if (wlen <= 0) { 
      printf("write %d error %d, %s\n", fd, errno, strerror(errno));
    }
    else 
      fsize += wlen;
    if (++i >= BUFFER_NUM)
      i=0;

    sem_post(&shared.nempty);
  }
  printf("consumer done, write %d bytes\n", fsize);
  close(fd);
  
  return NULL;
}

int main(int argc, char* argv[])
{
  if(argc < 3) {
    printf("usage: %s <filename> <target>\n", argv[0]);
    return 0;
  }
  
  sem_init(&shared.mutex ,0, 1);
  sem_init(&shared.nempty ,0, BUFFER_NUM);
  sem_init(&shared.nstored ,0, 0);

  pthread_t th_producer, th_consumer;
  pthread_attr_t thattr;
  pthread_attr_init(&thattr);
  pthread_attr_setdetachstate(&thattr, PTHREAD_CREATE_DETACHED);
  pthread_create(&th_producer, &thattr, producer, argv[1]);
  pthread_create(&th_consumer, NULL, consumer, argv[2]);

  pthread_join(th_consumer, NULL);

  exit(0);
}
