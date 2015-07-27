#include "shm.h"
#include "globaldefines.h"

#include <sys/types.h>
#include <time.h>

const int timeout = 1000; // ms
const int nloop   = 1000;

static inline void mktimeout(struct timespec *ts, int sec, int usec)
{
	clock_gettime(CLOCK_REALTIME, ts);
	ts->tv_sec  += sec;
	ts->tv_nsec += usec*1000;
	if(ts->tv_nsec > NSEC_PER_SECOND) 
	{
		ts->tv_nsec -= NSEC_PER_SECOND;
		ts->tv_sec  += 1;
	}
}

int main(int argc, char *argv[])
{
	int fd = shm_open(SHMNAME, O_RDWR, 0700);
	if (fd < 0)
	{
		fprintf(stderr, "shm_open failure %s\n", strerror(errno));
		return 1;
	}

	struct shm_shared *ptr = (struct shm_shared *) mmap(NULL, sizeof(struct shm_shared), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if (ptr == NULL)
	{
		fprintf(stderr, "mmap %s\n", strerror(errno));
		return 1;
	}
	close(fd);

	char buffer[MESGSIZE];
	int  buflen, offset;
	pid_t pid = getpid();
	struct timespec ts;
	for (int i=0; i<nloop; ++i)
	{
		usleep(5000);
		mktimeout(&ts, 0, timeout);
		if (sem_timedwait(&ptr->nempty, &ts) == -1)
		{
			fprintf(stderr, "sem_timedwait failure %s\n", strerror(errno));
			if (errno == ETIMEDOUT) {
				shm_shared_incoverflow(ptr);
				continue;
			}
			else {
				break;
			}
		}

		sem_wait(&ptr->mutex);
		offset = ptr->msgoff[ptr->nput];
		buflen = sprintf(buffer, "PID:%d put into slot:%2d, offset:%05d, loop:%03d", pid, ptr->nput, offset, i);
		if (++(ptr->nput) >= MESGNUM) ptr->nput = 0;
		sem_post(&ptr->mutex);

		strcpy(&ptr->msgdata[offset], buffer);
		sem_post(&ptr->nstored);		
	}
	
	return 0;
}

