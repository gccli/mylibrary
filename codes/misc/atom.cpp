#include <stdio.h>
#include <pthread.h>
//#include <atomic>
#include <cstdatomic>
static int count;
static int iadd;
static int isub = 40000;

std::atomic_int ai;

void *threadfunc(void *args)
{
    for(int i=0; i<10000; ++i) {
	count+=2;
	ai++;
	__sync_fetch_and_add(&iadd, 2);
	__sync_fetch_and_sub(&isub, 1);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    int i,n=4;
    pthread_t t[8];
    printf("%d %d %d %d\n", count, iadd, isub, ai);
    for (i=0; i<4; ++i) {
	pthread_create(&t[i], NULL, threadfunc, NULL);
    }

    for (i=0; i<4; ++i) {
	pthread_join(t[i], NULL);
    }
    printf("%d %d %d\n", count, iadd, isub);

    return 0;
}
