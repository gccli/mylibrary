#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#include <syscall.h>
#define gettid() syscall(__NR_gettid)


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int shared = 0;

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


// if interval is zero, then the timer expires just once
static int timer_start(timer_t timerid, int value, int vnanosecs, int interval, int inanosecs)
{
	char prefix[64];

	struct itimerspec its;
	its.it_value.tv_sec = value;
	its.it_value.tv_nsec = vnanosecs;
	its.it_interval.tv_sec = interval;
	its.it_interval.tv_nsec = inanosecs;
	if (timer_settime(timerid, 0, &its, NULL) < 0)
	{
		fprintf(stderr, "%s failed to start timer<%p> %s\n",
			timestamp(prefix, sizeof(prefix)), timerid, strerror(errno));
		return -1;
	}

	return 0;
}

static int timer_stop(timer_t timerid)
{
	char prefix[64];

	struct itimerspec its;
	memset(&its, 0, sizeof(&its));
	if (timer_settime(timerid, 0, &its, NULL) < 0)
	{
		fprintf(stderr, "%s failed to stop timer<%p> %s\n", timestamp(prefix, sizeof(prefix)), timerid, strerror(errno));
		return -1;
	}

	return 0;
}


static void timer_kill(timer_t timerid)
{
	timer_stop(timerid);
	timer_delete(timerid);
}


void *threadfunc(void *param)
{
	timer_t *tidp = (timer_t *)param;
	sleep(1);

	int i = 0, sec = 0;
	char strt[64];
	while(1)
	{
		sec = i++%2;
		if (sec == 0) {
			if (timer_stop(*tidp) != 0)
				continue;
		}
		else {
			if (timer_start(*tidp, sec, 0, 1, 0) != 0)
				break;
		}
		printf("%s %s timer %p\n", timestamp(strt, sizeof(strt)),  sec ? "arm": "disarm", *tidp);
		sleep(6);
	}
	return NULL;
}


void handler(sigval_t sig)
{
	timer_t *tidp = (timer_t *)sig.sival_ptr;

	char strtime[64];
	printf("%s timer %p expired, overruns %d\n", timestamp(strtime, sizeof(strtime)), *tidp, timer_getoverrun(*tidp));
}


void shared_incr()
{
	char strt[64];
	pthread_mutex_lock(&mutex);
	shared++;
	printf("%s shared = %d\n", timestamp(strt, sizeof(strt)), shared);
	pthread_mutex_unlock(&mutex);
}

void *sig_threadfunc(void *param)
{
	int i=0;
	while(i<100)
	{
		i++;
		shared_incr();
	}
	return NULL;
}


void sig_handler(int sig, siginfo_t *si, void *uc)
{
	char strtime[64];
	timer_t *tidp = (timer_t *)si->si_value.sival_ptr;
	printf("%s timer %p expired, overruns %d\n", timestamp(strtime, sizeof(strtime)), *tidp, timer_getoverrun(*tidp));
	shared_incr();
}


int main_forthread(int argc, char *argv[])
{
	pthread_attr_t *thattr = (pthread_attr_t *)calloc(1, sizeof(pthread_attr_t));
	pthread_attr_init(thattr);
	pthread_attr_setdetachstate(thattr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setstacksize(thattr, 200*1024);

	timer_t timerid;
	struct sigevent sev;
	memset(&sev, 0, sizeof(sev));
	sev.sigev_value.sival_ptr = &timerid;
	sev.sigev_notify_function = handler;
	sev.sigev_notify = SIGEV_THREAD;
	sev.sigev_notify_attributes = thattr;

	if (timer_create(CLOCK_REALTIME, &sev, &timerid) < 0)
	{
		fprintf(stderr, "failed to create %s\n", strerror(errno));
		return 1;
	}

	char prefix[64];

	printf("%s timer %p created\n", timestamp(prefix, sizeof(prefix)), timerid);
	timer_start(timerid, 2, 0, 1, 0);

	sleep(6);

	pthread_t th;
	pthread_create(&th, NULL, threadfunc, &timerid);

	char c;
	while((c = getchar()))
	{
		if (c == 'k') {
			printf("kill timer\n");
			timer_kill(timerid);
		}
	}


	return 0;
}

int timer_new(timer_t *timerid)
{

}

int main(int argc, char *argv[])
{
//	sigset_t mask;
	struct sigaction sa;
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = sig_handler;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGHUP, &sa, NULL) < 0) {
		fprintf(stderr, "failed to establishing handler for signal %s\n", strerror(errno));
		return 1;
	}

	timer_t timerid = NULL;
	struct sigevent sev;
	memset(&sev, 0, sizeof(sev));
	sev.sigev_value.sival_ptr = &timerid;


#ifndef sigev_notify_thread_id
#define	sigev_notify_thread_id _sigev_un._tid
#endif

	sev.sigev_signo = SIGHUP;
	sev.sigev_notify = SIGEV_THREAD_ID;
	sev.sigev_notify_thread_id = gettid();

	if (timer_create(CLOCK_REALTIME, &sev, &timerid) < 0)
	{
		fprintf(stderr, "failed to create timer %s\n", strerror(errno));
		return 1;
	}

	char prefix[64];

	printf("%s timer %p created\n", timestamp(prefix, sizeof(prefix)), timerid);
	timer_start(timerid, 0, 2, 0, 100000000);

	pthread_t th;
	pthread_create(&th, NULL, sig_threadfunc, &timerid);


	char c;
	while((c = getchar()))
	{
		if (c == 'k') {
			printf("kill timer\n");
			timer_kill(timerid);
		}
	}

	return 0;
}


