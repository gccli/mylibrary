#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <signal.h>
#include <setjmp.h>

static sigjmp_buf jmpbuf;
void sigfunc(int signo)
{
	printf("alrm 0\n");	
	siglongjmp(jmpbuf, 1);
	printf("alrm 1\n");
} 


#include <syscall.h>
#define gettid() syscall(__NR_gettid)

static const char *timestamp(char *strtime, int sz)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	struct tm tmt;
	localtime_r(&tv.tv_sec, &tmt);
	size_t len = strftime(strtime, sz, "%D %T", &tmt);
	sprintf(strtime+len, ".%03ld thread[%ld]", tv.tv_usec/1000, gettid());
	return strtime;
}


int main(int argc, char *argv[])
{
	int i=0;
	char strf[64] = {0};
	signal(SIGALRM, sigfunc);
	
again:
	alarm(2);
	if (sigsetjmp(jmpbuf, 1) != 0) {
		i++;
		printf("%s setjmp sizeof(sigjmp_buf) = %d \n", timestamp(strf, sizeof(strf)), sizeof(sigjmp_buf));
		if (i > 3)
			goto giveup;
		goto again;
	}

giveup:
	while(1) {
		putchar(getchar());
	}
	return 0;
}

