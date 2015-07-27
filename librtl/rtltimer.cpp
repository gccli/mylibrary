#include <semaphore.h>

#include "rtltimer.h"
#define gettid() syscall(__NR_gettid)
#define RTL_SIG  SIGUSR1

static pid_t rtl_tid;
static sem_t rtl_sem;

void rtl_timer_post()
{
	sem_post(&rtl_sem);
}

static const char *rtl_timestamp(char *strtime, int sz)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	struct tm tmt;
	localtime_r(&tv.tv_sec, &tmt);
	size_t len = strftime(strtime, sz, "%D %T", &tmt);
	sprintf(strtime+len, ".%03ld thread[%ld]", tv.tv_usec/1000, gettid());
	return strtime;
}


int rtl_timer_start(timer_t timerid, int value, int interval)
{
	char prefix[64];

	struct itimerspec its;
	its.it_value.tv_sec = value;
	its.it_value.tv_nsec = 0;
	its.it_interval.tv_sec = interval;
	its.it_interval.tv_nsec = 0;
	if (timer_settime(timerid, 0, &its, NULL) < 0)
	{
		fprintf(stderr, "%s failed to start timer<%p> %s\n",
			rtl_timestamp(prefix, sizeof(prefix)), timerid, strerror(errno));
		return -1;
	}

	return 0;
}

int rtl_timer_stop(timer_t timerid)
{
	char prefix[64];

	struct itimerspec its;
	memset(&its, 0, sizeof(&its));
	if (timer_settime(timerid, 0, &its, NULL) < 0)
	{
		fprintf(stderr, "%s failed to stop timer<%p> %s\n",
			rtl_timestamp(prefix, sizeof(prefix)), timerid, strerror(errno));
		return -1;
	}

	return 0;
}


int rtl_timer_create(timer_t *timerp)
{
	struct sigevent sev;
	memset(&sev, 0, sizeof(sev));
	sev.sigev_value.sival_ptr = timerp;

#ifndef sigev_notify_thread_id
#define	sigev_notify_thread_id _sigev_un._tid
#endif
	
	sev.sigev_signo = SIGHUP;
	sev.sigev_notify = SIGEV_THREAD_ID;
	sev.sigev_notify_thread_id = rtl_tid;
	if (timer_create(CLOCK_REALTIME, &sev, timerp) < 0)
	{
		fprintf(stderr, "failed to create timer %s\n", strerror(errno));
		return 1;
	}

	printf("create timer %p successfully\n", *timerp);

	return 0;
}



static void *rtl_timer_threadfunc(void *param)
{
	int ret = 0;
	rtl_tid = gettid();

	while(1)
	{
		while((ret = sem_wait(&rtl_sem)) == -1 && errno == EINTR)
			continue ;
		printf("timer expired\n");
	}

	return NULL;
}

int rtl_timer_init(sighandler_t handler)
{
	sem_init(&rtl_sem, 0, 0);
	pthread_t th;
	pthread_attr_t thattr;
	pthread_attr_init(&thattr);
	pthread_create(&th, &thattr, rtl_timer_threadfunc, NULL);


	signal(RTL_SIG, handler);

	return 0;	
}



