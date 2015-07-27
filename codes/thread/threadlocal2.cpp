#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <sys/syscall.h>
#define gettid() syscall(__NR_gettid)
typedef int (*MThread_callback)(int);
class MThread_t {
public:
    int init(MThread_callback);
    int comp(long);
};

#define MAXLINE 64
struct ThreadLocalArea {
    int   tid;
    MThread_t* tptr;
    char  tbuf[MAXLINE];
};

static __thread struct ThreadLocalArea tls;
static void TLSInit(struct ThreadLocalArea *tlp)
{
    memset(tlp, 0, sizeof(struct ThreadLocalArea));
    tlp->tid = gettid();
    sprintf(tlp->tbuf, "Thread[%d]", tlp->tid);
}

static int callback(int id)
{
    printf("%s TLS %p this pointer %p, id:%d\n", tls.tbuf, &tls, tls.tptr, id);

    long addr = (long) tls.tptr;
    tls.tptr->comp(addr);
    return 0;
}


int MThread_t::init(MThread_callback f) 
{
    TLSInit(&tls);

    tls.tptr = this;
    callback(gettid());
    return 0;
}

int MThread_t::comp(long addr)
{
    long myaddr = (long) this;
    assert(addr == myaddr);
    return 0;
}

void* thread_fun(void* param)
{
    MThread_t t1;
    printf("1 %p\n", &t1);
    t1.init(callback);

    int i=0;
    for(; i<3; ++i) {
	MThread_t t2;
	printf("2 %p\n", &t2);
	t2.init(callback);
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

    usleep(100);
    MThread_t tm;
    printf("0 %p\n", &tm);
    tm.init(callback);
    for(int i=0; i<50; ++i) {
    }

    return 0;
}
