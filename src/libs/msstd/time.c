#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "msstd.h"

#define ONE_MIN_SECONDS 			(60)
#define ONE_HOUR_SECONDS 			(3600)
#define ONE_DAY_SECONDS 			(86400)
#define HALF_ADAY_SECONDS 			(43200)

void time_to_string(time_t ntime, char stime[32])
{
	struct tm temp;

	localtime_r((long*)&ntime, &temp);
	snprintf(stime, 32, "%04d-%02d-%02d %02d:%02d:%02d", 
		temp.tm_year + 1900, temp.tm_mon + 1, temp.tm_mday, temp.tm_hour, temp.tm_min, temp.tm_sec);
}

void time_to_string_ms(char stime[32])
{
	struct tm temp;
	struct timeval tv;
	gettimeofday(&tv, NULL);
	localtime_r(&tv.tv_sec, &temp);
	snprintf(stime, 32, "%4d-%02d-%02d %02d:%02d:%02d.%03d", 
		temp.tm_year + 1900, temp.tm_mon + 1, temp.tm_mday, temp.tm_hour, temp.tm_min, temp.tm_sec, (int)(tv.tv_usec/1000));
}

void get_current_time_to_compact_string(char stime[32])
{
    struct tm temp;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &temp);
    snprintf(stime, 32, "%4d%02d%02d%02d%02d%02d",
        temp.tm_year + 1900, temp.tm_mon + 1, temp.tm_mday, temp.tm_hour, temp.tm_min, temp.tm_sec);
}

MS_U64 ms_get_current_time_to_msec(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return ((MS_U64)tv.tv_sec*1000 + tv.tv_usec/1000);
}

MS_U64 ms_get_current_time_to_usec(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return ((MS_U64)tv.tv_sec*1000000 + tv.tv_usec);
}

int ms_get_current_date_cnt(void)
{
    int curSecondCnt = (int)time(0);
	
    return curSecondCnt/ONE_DAY_SECONDS;
}

int ms_get_current_hours_cnt(void)
{
    int curSecondCnt = (int)time(0);
	int curhours = curSecondCnt%ONE_DAY_SECONDS;
	
    return curhours/ONE_HOUR_SECONDS;
}

int ms_get_current_date_cnt_ex(char *localTime)
{
	struct tm m_tm = {0};
	m_tm.tm_isdst = -1;
	int curSecondCnt = 0;
	
	strptime(localTime, "%Y-%m-%d %H:%M:%S", &m_tm);
	curSecondCnt = mktime(&m_tm);
    return curSecondCnt/ONE_DAY_SECONDS;
}

int ms_get_current_hours_cnt_ex(char *localTime)
{
	struct tm m_tm = {0};
	m_tm.tm_isdst = -1;
	int curSecondCnt = 0;
	int curhours = 0;
	
	strptime(localTime, "%Y-%m-%d %H:%M:%S", &m_tm);
	curSecondCnt = mktime(&m_tm);
	curhours = curSecondCnt%ONE_DAY_SECONDS;
    return curhours/ONE_HOUR_SECONDS;
}

time_t ms_get_time_from_string(char *localTime)
{
    struct tm m_tm = {0};
    m_tm.tm_isdst = -1;
    time_t curSecondCnt = 0;

    do {
        if (!localTime) {
            break;
        }
        if (strlen(localTime) == 0) {
            break;
        }
        strptime(localTime, "%Y-%m-%d %H:%M:%S", &m_tm);
        curSecondCnt = mktime(&m_tm);
    } while (0);

    return curSecondCnt;
}
