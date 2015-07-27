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


typedef struct 
{
	union {
		void *data;
		long  size;
	} u;

	char  c;
	int   i;
} x64;

void unsignedlong()
{
	unsigned long l1 = ~0;
	unsigned long l2 = (unsigned long)-1U;
	unsigned long l3 = (unsigned long)-1L;
	printf("~0 = %lu  \n(unsigned long)-1U = %lu  \n(unsigned long)-1L = %lu\n", l1, l2, l3);
}

int main(int argc, char *argv[])
{
	printf("On %d-bit platform\n", __WORDSIZE);
	printf(" sizeof(x64) = %ld\n", sizeof(x64));
	
	unsignedlong();

	return 0;
}

