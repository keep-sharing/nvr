/**
 @file udbasket.c
*/

#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>

#include <sys/time.h>
#include <unistd.h>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include "udcellular.h"

//MULTI-HDD SKSUNG

///////////////////////////////////////
//#define BKT_DEBUG

#ifdef BKT_DEBUG
#define DBG_BKT(msg, args...)  printf("[BKT] - " msg, ##args)
#define DBG1_BKT(msg, args...) printf("[BKT] - %s:" msg, __FUNCTION__, ##args)
#define DBG2_BKT(msg, args...) printf("[BKT] - %s(%d):" msg, __FUNCTION__, __LINE__, ##args)
#define DBG3_BKT(msg, args...) printf("[BKT] - %s(%d):\t%s:" msg, __FILE__, __LINE__, __FUNCTION__, ##args)
#else
#define DBG_BKT(msg, args...)  ((void)0)
#define DBG1_BKT(msg, args...) ((void)0)
#define DBG2_BKT(msg, args...) ((void)0)
#define DBG3_BKT(msg, args...) ((void)0)
#endif
///////////////////////////////////////

/*int alphasort_reverse(const struct dirent **a, const struct dirent **b)
{
    return alphasort(b, a);
}*/

long cel_get_max_ts(T_TIMESTAMP *arr_t)
{
	long max_t = 0;
	int c = 0;

	for(c = 0;c < MAX_CAMERA; c++)
	{
		max_t = max((arr_t[c].sec), max_t);
	}

	return max_t;
}

long cel_get_min_ts(T_TIMESTAMP *arr_t)
{
	long min_t = 0x7fffffff;
	int c = 0;
	time_t t=0;

	for(c = 0;c < MAX_CAMERA; c++)
	{
		t = arr_t[c].sec;

		if(t != 0)
		{
			min_t = min( (arr_t[c].sec), min_t);
		}
	}

	return min_t;
}

long cel_trans_pos2num(long fpos_index)
{
	return  (fpos_index-sizeof(T_INDEX_HDR))/sizeof(T_INDEX_DATA);
}

long cel_trans_idx_num2pos(int index_number)
{
    // milesigh.bruce
	// return (index_number*sizeof(T_INDEX_DATA)+sizeof(T_INDEX_HDR));
    return ((index_number-1)*sizeof(T_INDEX_DATA)+sizeof(T_INDEX_HDR));
}

void cel_get_local_time(long sec, T_CEL_TM* ptm)
{
	struct tm t1;
#ifdef RDB_UTC	
	gmtime_r(&sec, &t1);
#else
	localtime_r(&sec, &t1);
#endif	

	ptm->year = t1.tm_year+1900;
	ptm->mon  = t1.tm_mon+1;
	ptm->day  = t1.tm_mday;
	ptm->hour = t1.tm_hour;
	ptm->min  = t1.tm_min;
	ptm->sec  = t1.tm_sec;
}

int g_debug = 0;		
void cel_set_debug(int on)
{
	g_debug = on;
}

int ms_write_data(int fd, void *data, int len)
{
	int wdataSize = 0, rewriteSize = 0, pdateLen = 0, rewriteCnt = 2;
	
	wdataSize = write(fd, data, len);
	if(wdataSize < 0 || wdataSize > len){
		msdebug(DEBUG_ERR, "ms_write_data WRITE ERROR!!!");
		return 0;
	}
	if(wdataSize == len)
		return 1;
	
	/*Data need to rewrite*/	
	do{
		pdateLen = wdataSize + pdateLen;
		rewriteSize = len - wdataSize - rewriteSize;
		if(rewriteSize > 0){
			wdataSize = write(fd, (data + pdateLen), rewriteSize);
			if(0 < wdataSize || wdataSize > rewriteSize){
				msdebug(DEBUG_ERR, "ms_write_data WRITE ERROR!!!");
				return 0;
			}
			if(wdataSize == rewriteSize)
				break;
		}
	}while(rewriteCnt--);
	
	if(rewriteCnt <=  0)
		return 0;
	return 1;
}


#ifdef HDR_ENABLE


long cel_trans_hdr_pos2num(long fpos)
{
	return ((fpos - SIZEOF_HDRFILE_HDR) / SIZEOF_HDR);
}

long cel_trans_hdr_num2pos(long num)
{
	return (num * SIZEOF_HDR + SIZEOF_HDRFILE_HDR);
}
#endif
////////////////////////////
// EOF basket.c

