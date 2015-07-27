#ifndef __RTL_TIMER_H__
#define __RTL_TIMER_H__

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

void rtl_timer_post();


int rtl_timer_start(timer_t timerid, int value, int interval);
int rtl_timer_stop(timer_t timerid);
int rtl_timer_create(timer_t *timerp);
int rtl_timer_init(sighandler_t handler);



#endif

