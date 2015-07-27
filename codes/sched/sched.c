#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/shm.h>

#include <pthread.h>
#include <semaphore.h>
#include <sched.h>

int main(int argc, char *argv[])
{
    struct sched_param param;
    memset(&param, 0, sizeof(param));
    param.sched_priority = 50;

    if (sched_setscheduler(getpid(), SCHED_FIFO, &param)) {
	perror("sched_setscheduler");
	return 1;
    }

    int i;
    for(; i<500000000; ++i)
	;
    printf("process<%d>  %d\n", getpid(), i);

    return 0;
}
