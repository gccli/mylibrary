#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <mqueue.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>


#define SEM_NAME "mysem"
#define SEM_FILE "/tmp/shm"

static int   *shm_ptr;
void do_loop(const char *tag, sem_t *mutex)
{
	for (int i=0; i<10000; ++i)
	{
		sem_wait(mutex);
		printf("%s: %d\n", tag, (*shm_ptr)++);
		sem_post(mutex);
	}
}

// named file based semaphore
void prog1()
{
	sem_t *mutex;
	int zero = 0;
	int fd = open(SEM_FILE, O_RDWR|O_CREAT);
	write(fd, &zero, sizeof(zero));
	shm_ptr = (int *)mmap(0, sizeof(int), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	printf("PTR:%p\n", shm_ptr);

	
	sem_unlink(SEM_NAME);
	if ((mutex = sem_open(SEM_NAME, O_CREAT|O_EXCL, 0700, 1)) == NULL)
		perror("sem_open");
	if (fork() == 0)
	{
		do_loop("child ", mutex);
		exit(0);
	}

	do_loop("parent", mutex);
}


struct shm_shared {
	sem_t  mutex;
	int    count;
} shared;

// memory based semaphore
void prog2()
{
	int fd = open(SEM_FILE, O_RDWR|O_CREAT);
	write(fd, &shared, sizeof(shared));
	shm_ptr = (int *)mmap(0, sizeof(struct shm_shared), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	printf("PTR:%p\n", shm_ptr);

	if (sem_init(&shared.mutex, 1, 1) < 0)
		perror("sem_init");

	perror("sem_open");
	if (fork() == 0)
	{
		do_loop("child ", &shared.mutex);
		exit(0);
	}

	do_loop("parent", &shared.mutex);
}

int main(int argc, char *argv[])
{
	prog1();
	sleep(1);
	prog2();
	
	return 0;
}

