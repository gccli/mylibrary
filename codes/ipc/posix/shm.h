#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>

#define SHMNAME     "cliserv"
#define MESGSIZE    1024
#define MESGNUM     16

// shared memory based
struct shm_shared
{
	sem_t   mutex;
	sem_t   nempty;
	sem_t   nstored;

	int     nput;                         // index into msgoff[] for next put
	long    noverflow;                    // #overflows by senders
	sem_t   nomutex;
	long    msgoff[MESGNUM];              // offset in shared memory of each message
	char    msgdata[MESGNUM * MESGSIZE];  // the actual messages
};

static inline void shm_shared_init(struct shm_shared *shm)
{
	memset(shm, 0, sizeof(struct shm_shared));
	int i=0;
	for (; i < MESGNUM; i++)
		shm->msgoff[i] = i*MESGSIZE;
	sem_init(&shm->mutex,     1, 1);
	sem_init(&shm->nempty,    1, MESGNUM);
	sem_init(&shm->nstored,   1, 0);
	sem_init(&shm->nomutex,   1, 1);
}

static inline int shm_shared_getoverflow(struct shm_shared *shm)
{
	int noverflow = 0;
	sem_wait(&shm->nomutex);
	noverflow = shm->noverflow;
	sem_post(&shm->nomutex);

	return noverflow;
}

// increase overflow counter
static inline void shm_shared_incoverflow(struct shm_shared *shm)
{
	sem_wait(&shm->nomutex);
	shm->noverflow++;
	sem_post(&shm->nomutex);
}
