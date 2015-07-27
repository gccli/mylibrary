#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <sys/syscall.h>
#define gettid() syscall(__NR_gettid)

#define MAXLINE 1024
struct ThreadLocalArea {
  int   tid;
  char* tptr;
  char  tbuf[MAXLINE];
};


static __thread struct ThreadLocalArea tls;

void TLSInit(struct ThreadLocalArea *tlp)
{
    memset(tlp, 0, sizeof(struct ThreadLocalArea));
    tlp->tid = gettid();
    sprintf(tlp->tbuf, "Hello Thread[%d]\n", tlp->tid);
}

void* thread_fun(void* param)
{
    TLSInit(&tls);

    int i=0;
    for(; i<30; ++i) {
	printf("%s", tls.tbuf);
	sprintf(tls.tbuf, "Thread[%d]'s has its own storage, address:%p\n", tls.tid, &tls);
    }
    return NULL;
}

int main(int argc, char* argv[])
{
    pthread_t th1, th2;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_create(&th1, &attr, thread_fun, NULL);
    pthread_create(&th2, &attr, thread_fun, NULL);

    usleep(10);
    TLSInit(&tls);
    for(int i=0; i<50; ++i) {
	printf("%s", tls.tbuf);
	sprintf(tls.tbuf, "Main Thread[%d]'s has its own storage, address:%p\n", tls.tid, &tls);
    }

    return 0;
}
