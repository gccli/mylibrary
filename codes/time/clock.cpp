#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>

#define errexit(msg) do { perror(msg); exit(1); } while (0)

static void handler(int sig, siginfo_t *si, void *uc)
{
	/* Note: calling printf() from a signal handler is not
	   strictly correct, since printf() is not async-signal-safe;
	   see signal(7) */

	printf("Caught signal %d\n", sig);

	timer_t *tidp = (timer_t *)si->si_value.sival_ptr;
	printf("	timer = %p\n ", *tidp);
	printf("	overrun count = %d\n", timer_getoverrun(*tidp));

//	signal(sig, SIG_IGN);
}

int main(int argc, char *argv[])
{
	timer_t timerid;
	struct sigevent sev;
	struct itimerspec its;
	unsigned long freq_nanosecs;
	sigset_t mask;
	struct sigaction sa;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <sleep-secs> <freq-nanosecs>\n",	argv[0]);
		exit(1);
	}

	/* Establish handler for timer signal */

	printf("Establishing handler for signal %d\n", SIGHUP);
	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = handler;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGHUP, &sa, NULL) == -1)
		errexit("sigaction");

	/* Block timer signal temporarily */
	printf("Blocking signal %d\n", SIGHUP);
	sigemptyset(&mask);
	sigaddset(&mask, SIGHUP);
	if (sigprocmask(SIG_SETMASK, &mask, NULL) == -1)
		errexit("sigprocmask");

	/* Create the timer */
	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIGHUP;
	sev.sigev_value.sival_ptr = &timerid;
	if (timer_create(CLOCK_REALTIME, &sev, &timerid) == -1)
		errexit("timer_create");

	printf("timer %p created\n",  timerid);

	/* Start the timer */
	freq_nanosecs = atoll(argv[2]);
	its.it_value.tv_sec = freq_nanosecs / 1000000000;
	its.it_value.tv_nsec = freq_nanosecs % 1000000000;
	its.it_interval.tv_sec = its.it_value.tv_sec;
	its.it_interval.tv_nsec = its.it_value.tv_nsec;

	if (timer_settime(timerid, 0, &its, NULL) == -1)
		 errexit("timer_settime");

	/* Sleep for a while; meanwhile, the timer may expire
	   multiple times */

	printf("Sleeping for %d seconds\n", atoi(argv[1]));
	sleep(atoi(argv[1]));

	/* Unlock the timer signal, so that timer notification
	   can be delivered */

	printf("Unblocking signal %d\n", SIGHUP);
	if (sigprocmask(SIG_UNBLOCK, &mask, NULL) == -1)
		errexit("sigprocmask");

	char c;
	while((c = getchar()))
	{
		putchar(c);
	}

	return 0;
}

