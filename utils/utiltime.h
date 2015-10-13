#ifndef UTIL_TIME_H__
#define UTIL_TIME_H__

#include <unistd.h>
#include <stdio.h>
#include <time.h>

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>


#define DATE_STR_SIZE 32

#ifdef __cpluplus
extern "C" {
#endif

// %Y-%m-%d, the ISO 8601 date format
static inline char *to_date(time_t t, char* datestr)
{
    struct tm tm;
    localtime_r(&t, &tm);
    strftime(datestr, DATE_STR_SIZE, "%F", &tm);

    return datestr;
}

// %m/%d
static inline char *to_date_noyear(time_t t, char* datestr)
{
    struct tm tm;
    localtime_r(&t, &tm);
    strftime(datestr, DATE_STR_SIZE, "%m/%d", &tm);

    return datestr;
}

// The time in 24-hour notation (%H:%M:%S)
static inline char *to_time(time_t t, char *timestr)
{
    struct tm tm;
    localtime_r(&t, &tm);
    strftime(timestr, DATE_STR_SIZE, "%T", &tm);
    return timestr;
}

// %Y-%m-%d %H:%M:%S
static inline char *to_datetime(time_t t, char* datestr)
{
    struct tm tm;
    localtime_r(&t, &tm);
    strftime(datestr, DATE_STR_SIZE, "%F %T", &tm);

    return datestr;
}

// %m/%d %H:%M:%S
static inline char *to_datetime_noyear(time_t t, char* datestr)
{
    struct tm tm;
    localtime_r(&t, &tm);
    strftime(datestr, DATE_STR_SIZE, "%m/%d %T", &tm);

    return datestr;
}

// %Y-%m-%dT%H:%M:%S+hhmm
static inline char *to_iso_datetime(time_t t, char* datestr)
{
    struct tm tm;
    localtime_r(&t, &tm);
    strftime(datestr, DATE_STR_SIZE, "%FT%T%z", &tm);

    return datestr;
}

static inline time_t get_0_clock(time_t time)
{
    struct tm ptm = {0};
    localtime_r(&time,&ptm);
    return (time - ptm.tm_hour*3600 - ptm.tm_min*60 - ptm.tm_sec);
}


static inline double timing_start()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double start = 1.0*tv.tv_sec + 0.000001*tv.tv_usec;

    return start;
}

static inline double timing_cost(double start)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double end = 1.0*tv.tv_sec + 0.000001*tv.tv_usec;

    return end - start;
}

#ifdef __cpluplus
}
#endif

#endif                                            //UTIL_TIME_H__
