#ifndef DAC_TIME_H__
#define DAC_TIME_H__

#include <sys/time.h>
#include <time.h>

/* globals for accessing time */
extern struct timeval current_time;
extern double current_dtime;
extern time_t curtime;

time_t GetCurrentTime(void);

class TimeService
{
public:
    virtual ~TimeService();

    /** tick the clock - update from the OS or other time source, */
    virtual void tick();
};

#endif

