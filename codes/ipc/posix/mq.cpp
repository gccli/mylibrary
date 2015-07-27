#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <mqueue.h>
#include <pthread.h>

struct exclient {
  char* p;
};

// # g++ mq.cpp -rt
void* thread_mq(void* param);
int main(int argc, char* argv[])
{
  struct mq_attr mqattr;
  memset (&mqattr, 0, sizeof(mqattr));
  mqattr.mq_msgsize = 1024;
  mqattr.mq_maxmsg  = 256;

  mq_unlink("/mqueue_0");
  mqd_t q = mq_open("/mqueue_0", O_RDWR|O_CREAT, 0700, &mqattr);
  if (q == (mqd_t )-1) {
    fprintf (stderr, "mq_open error %d %s\n", errno, strerror(errno));
    return 1;
  }
  
  // get attribute
  memset (&mqattr, 0, sizeof(mqattr));
  if (mq_getattr(q, &mqattr) < 0) {
    fprintf (stderr, "mq_getattr error %d %s\n", errno, strerror(errno));
    return 1;
  }
  printf ("max #msgs = %ld, max #bytes/msg = %ld, currently on queue %ld\n",
	  mqattr.mq_maxmsg, mqattr.mq_msgsize, mqattr.mq_curmsgs);

  pthread_t th;
  pthread_attr_t thattr;
  pthread_attr_init(&thattr);
  pthread_create(&th, &thattr, thread_mq, &q);

  sleep(10);
  for (int i=0; i<1000; ++i) {
    struct exclient e;
    e.p = (char*) calloc(1, 128);
    sprintf (e.p, "hello %-6d", i);
    if (mq_send(q, (char*)&e, 4, 0) != 0) {
      fprintf (stderr, "mq_send error %d %s\n", errno, strerror(errno));
    }
  }

  pthread_join(th, NULL);

  mq_close(q);
  return 0;
}

void* thread_mq(void* param)
{
  mqd_t q = *(mqd_t* )param;
  mqd_t qrecv;
  struct mq_attr mqattr;
  memset (&mqattr, 0, sizeof(mqattr));

  if (mq_getattr(q, &mqattr) < 0) {
    fprintf (stderr, "mq_getattr error %d %s\n", errno, strerror(errno));
    return NULL;
  }
  printf ("max #msgs = %ld, max #bytes/msg = %ld, currently on queue %ld\n",
	  mqattr.mq_maxmsg, mqattr.mq_msgsize, mqattr.mq_curmsgs);

  unsigned int prio;
  struct timespec ts;
  while (true) {
    struct exclient e;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 1;
    qrecv = mq_timedreceive(q, (char*)&e, mqattr.mq_msgsize, &prio, &ts);
    if (qrecv < 0) {
      if (errno == ETIMEDOUT) {
	fprintf (stderr, "timeout\n");
	continue;
      }
      else
	fprintf (stderr, "mq_receive error %d %s\n", errno, strerror(errno));
      break;
    }
    else {
      printf ("get message %s %d\n", e.p, prio);
    }
    usleep(1000);
  }

  return NULL;
}
