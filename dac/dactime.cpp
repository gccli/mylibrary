#include "dactime.h"

struct timeval current_time;
double current_dtime;
time_t curtime = 0;

time_t GetCurrentTime(void)
{
    gettimeofday(&current_time, NULL);
    current_dtime = (double) current_time.tv_sec + (double) current_time.tv_usec/1000000.0;
    return curtime = current_time.tv_sec;
}

TimeService::~TimeService()
{}

void TimeService::tick()
{
    GetCurrentTime();
}
