#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <time.h>
#include <sys/time.h>
#include <sys/types.h>

void zonetest1()
{
	tzset();
	printf("timezone   : %d\n"
		   "tzname     : %s %s\n"
		   "daylight   : %d\n", timezone, tzname[0], tzname[1], daylight);

	char   strTime[64], rfc822_time[64];
	time_t curTime = time(NULL);

	struct tm *pLOC = localtime(&curTime);
	strftime(strTime, sizeof(strTime), "%D %T", pLOC);
	strftime(rfc822_time, sizeof(rfc822_time), "%a, %d %b %Y %H:%M:%S %z", pLOC);
	printf("LOC time   : %s\n"
		   "    zone   : %s\n"
	       "    rfc822 : %s\n",  strTime, pLOC->tm_zone, rfc822_time);


	struct tm *pGMT = gmtime(&curTime);
	strftime(strTime, sizeof(strTime), "%D %T", pGMT);
	strftime(rfc822_time, sizeof(rfc822_time), "%a, %d %b %Y %H:%M:%S %z", pGMT);
	printf("GMT time   : %s\n"
		   "    zone   : %s\n"
	       "    rfc822 : %s\n",  strTime, pGMT->tm_zone, rfc822_time);


	struct tm tm0;
	strptime(strTime, "%D %T", &tm0);
	time_t stamp = mktime(&tm0);
	stamp -= timezone;
	localtime_r(&stamp, &tm0);
	strftime(rfc822_time, sizeof(rfc822_time), "%a, %d %b %Y %H:%M:%S %z", &tm0);
	printf("GMT to LOC : %04d-%02d-%0d %02d:%02d:%02d\n"
	       "    rfc822 : %s\n",  tm0.tm_year+1900,tm0.tm_mon+1,tm0.tm_mday,tm0.tm_hour,tm0.tm_min,tm0.tm_sec, rfc822_time);
}

int main(int argc, char *argv[])
{
	zonetest1();

	return 0;
}

