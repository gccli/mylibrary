#include "shm.h"
#include "globaldefines.h"

int main(int argc, char *argv[])
{
	shm_unlink(SHMNAME);
	int fd = shm_open(SHMNAME, O_RDWR|O_CREAT|O_EXCL, 0700);
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
	ftruncate(fd, sizeof(struct shm_shared));
	close(fd);

	shm_shared_init(ptr);

	int temp, offset, index = 0, noverflow = 0;
	for (;;)
	{
		sem_wait(&ptr->nstored); // decrease store counter

		sem_wait(&ptr->mutex);
		offset = ptr->msgoff[index];
		printf("msgdata %2d is -> %s\n", index, &ptr->msgdata[offset]);
		if (++index >= MESGNUM) index = 0;
		sem_post(&ptr->mutex);
	
		sem_post(&ptr->nempty);  // increase empty message slot counter

		temp = shm_shared_getoverflow(ptr);
		if (temp != noverflow) {
			printf(FMTRED"noverflow = %d\n"FMTEND, temp);
			noverflow = temp;
		}
	}

	return 0;
}

