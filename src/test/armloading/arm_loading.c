/*
 * arm_loading.c
 *
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2007
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */

/* Standard Linux headers */
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Word color define */
#define WARNING			"\033[0;33m"	// yellow
#define GRAY 			"\033[2;37m"
#define DGREEN 			"\033[1;32m"
#define GREEN 			"\033[0;32m"
#define DARKGRAY 		"\033[0;30m"
//#define BLACK 		"\033[0;39m"
#define BLACK 			"\033[2;30m" 	// not faint black
#define NOCOLOR 		"\033[0;39m \n"
//#define NOCOLOR 		"\033[0m \n"
#define BLUE 			"\033[1;34m"
#define DBLUE 			"\033[2;34m"	
#define RED 			"\033[0;31m"
#define BOLD 			"\033[1m"
#define UNDERLINE 		"\033[4m"
#define CLS 			"\014"
#define NEWLINE 		"\r\n"
#define CR 			"\r"


/* Function error codes */
#define SUCCESS         0
#define FAILURE         -1

/* True of false enumeration */
#define TRUE            1
#define FALSE           0

#ifdef __DEBUG
#define DBG(fmt, args...) fprintf(stdout, "Encode Debug: " fmt, ##args)
#else
#define DBG(fmt, args...)
#endif

#define FPRINT( fp, fmt, args...) \
	do { \
		fprintf(fp, DGREEN fmt NOCOLOR, ##args);\
	} while(0)
	
#define PRINT( fmt, args...)\
	do { \
		printf(DGREEN fmt NOCOLOR, ##args);\
	} while(0)
	
#define WARN(fmt, args...) \
	do { \
		fprintf(stderr, WARNING "Warning: " fmt NOCOLOR, ##args); \
	} while(0)
	
#define ERR(fmt, args...) \
	do { \
		fprintf(stderr, RED "Error: " fmt NOCOLOR, ##args); \
	}while(0)

#define ARMLOAD_NUM_FRAMERATE		(60)

/*****************************************************************************/

char *version = "1.0.2";

/*****************************************************************************/

typedef struct cpu_stats {
	unsigned long	user;
	unsigned long	nice;
	unsigned long	system;
	unsigned long	idle;

	unsigned long	wait;
	unsigned long	hirq;
	unsigned long	sirq;
	unsigned long	steal;
	
	unsigned long	total;
	unsigned long  proc;
}CPU_STAT_T;

typedef struct cpu_percent {
	double	user;
	double	nice;
	double	system;
	double	idle;

	double	wait;
	double	hirq;
	double	sirq;
	double	steal;
	
	double	total;
	double      proc;
}CPU_PERCET_T;


static int Quit_flag = FALSE;
static void signalHandler(int signum)
{
	WARN("Quitting!!!\n");
    	Quit_flag = TRUE;
}

void usage( FILE*fp, int rc)
{
	FPRINT( fp, "Usage: armloading [-h?] [-d interval seconds] [-c count]"  );
	FPRINT( fp, "\t-h?            printf help " );
	FPRINT( fp, "\t-v             printf version info " );
	FPRINT( fp, "\t-c count       repeat count times " );
	FPRINT( fp, "\t-d seconds     seconds between output " );
	
	exit(rc);
}

/******************************************************************************
 * getArmCpuLoad
 ******************************************************************************/
int getArmCpuLoad( CPU_PERCET_T *cpuinfo)
{
	static CPU_STAT_T preCPUinfo = {0};
	CPU_STAT_T delCPUinfo = {0};
	int                  cpuLoadFound = FALSE;
	unsigned long        user, nice, sys, idle, wait, hirq, sirq, steal, total, proc;
	unsigned long        uTime, sTime, cuTime, csTime;
	char                 textBuf[6];
	FILE                *fptr;

	/* Read the overall system information */
	fptr = fopen("/proc/stat", "r");

	if (fptr == NULL) {
		ERR("/proc/stat not found. Is the /proc filesystem mounted?\n");
		return FAILURE;
	}

	/* Scan the file line by line */
	while (fscanf(fptr, "%4s %lu %lu %lu %lu %lu %lu %lu %lu %*[^\n]", textBuf,
	          &user, &nice, &sys, &idle, &wait, &hirq, &sirq, &steal) != EOF) {
		if (strcmp(textBuf, "cpu") == 0) {
			cpuLoadFound = TRUE;
			break;
		}
	}

	if (fclose(fptr) != 0) {
		return FAILURE;
	}

	if (!cpuLoadFound) {
		return FAILURE;
	}

	/* Read the current process information */
	fptr = fopen("/proc/self/stat", "r");

	if (fptr == NULL) {
		ERR("/proc/self/stat not found. Is the /proc filesystem mounted?\n");
		return FAILURE;
	}

	if (fscanf(fptr, "%*d %*s %*s %*d %*d %*d %*d %*d %*d %*d %*d %*d %*d %lu "
	             "%lu %lu %lu", &uTime, &sTime, &cuTime, &csTime) != 4) {
		ERR("Failed to get process load information.\n");
		fclose(fptr);
		return FAILURE;
	}

	if (fclose(fptr) != 0) {
		return FAILURE;
	}

	total = user + nice + sys + idle + wait + hirq + sirq + steal;
	proc = uTime + sTime + cuTime + csTime;

	/* Check if this is the first time, if so init the prev values */
	if (preCPUinfo.total == 0 && preCPUinfo.idle == 0 && preCPUinfo.system == 0 &&
		preCPUinfo.nice ==0 && preCPUinfo.user == 0 && preCPUinfo.proc == 0 &&
		preCPUinfo.wait ==0 && preCPUinfo.hirq == 0 && preCPUinfo.sirq == 0 &&
		preCPUinfo.steal == 0) {
		preCPUinfo.idle 	= idle;
		preCPUinfo.nice 	= nice;
		preCPUinfo.system 	= sys+sirq+hirq;
		preCPUinfo.user 	= user+nice;
		preCPUinfo.wait 	= wait;
		preCPUinfo.hirq 	= hirq;
		preCPUinfo.sirq 	= sirq;
		preCPUinfo.steal 	= steal;

		preCPUinfo.total 	= total;
		preCPUinfo.proc 	= proc;
		return SUCCESS;
	}

	delCPUinfo.total 		= total - preCPUinfo.total ;
	delCPUinfo.proc 		= proc - preCPUinfo.proc ;
	
	delCPUinfo.nice 		= nice -preCPUinfo.nice ;
	delCPUinfo.system 		= sys + sirq + hirq - preCPUinfo.system ;
	delCPUinfo.user 		= user + nice - preCPUinfo.user;
	delCPUinfo.idle		= idle - preCPUinfo.idle ;
	
	delCPUinfo.wait 		= wait -preCPUinfo.wait ;
	delCPUinfo.hirq 		= hirq - preCPUinfo.hirq;
	delCPUinfo.sirq 		= sirq - preCPUinfo.sirq;
	delCPUinfo.steal		= steal - preCPUinfo.steal ;	
	
	preCPUinfo.total 		= total;
	preCPUinfo.proc 		= proc;
	
	preCPUinfo.nice 		= nice;
	preCPUinfo.system 		= sys+sirq+hirq;
	preCPUinfo.user 		= user+nice;
	preCPUinfo.idle 		= idle;
	
	preCPUinfo.wait 		= wait;
	preCPUinfo.hirq 		= hirq;
	preCPUinfo.sirq 		= sirq;
	preCPUinfo.steal 		= steal;

	cpuinfo->total 			= 100 - delCPUinfo.idle * 100.0 / delCPUinfo.total;
	cpuinfo->proc 			= delCPUinfo.proc * 100.0 / delCPUinfo.total;
	
	cpuinfo->user 			= delCPUinfo.user *100.0/ delCPUinfo.total;
	cpuinfo->nice 			= delCPUinfo.nice*100.0/ delCPUinfo.total;
	cpuinfo->system 		= delCPUinfo.system *100.0/ delCPUinfo.total;
	cpuinfo->idle 			= delCPUinfo.idle*100.0/ delCPUinfo.total;

	cpuinfo->wait 			= delCPUinfo.wait *100.0/ delCPUinfo.total;
	cpuinfo->hirq 			= delCPUinfo.hirq*100.0/ delCPUinfo.total;
	cpuinfo->sirq 			= delCPUinfo.sirq *100.0/ delCPUinfo.total;
	cpuinfo->steal 		= delCPUinfo.steal*100.0/ delCPUinfo.total;

	return SUCCESS;
}

/******************************************************************************
 * ctrlThrFxn
 ******************************************************************************/
int main(int argc, char **argv)
{
	/*  
	int         	osdTransparency      = MIN_TRANSPARENCY<<1;//OSD_TRANSPARENCY;
	OsdData 	osdData              = OSD_DATA;
	int 		uiCreated            = FALSE;
	char	 	*osdDisplays[NUM_OSD_BUFS];
	UIParams 	uiParams;
	char		TempBuff[100];
	*/
	struct sigaction 	   sigAction;
	double 		total = 0.0;
	int		c, armLoadCnt = ARMLOAD_NUM_FRAMERATE, delay = 1;
	double 		max = 0,min = 100;
	CPU_PERCET_T cpuinfo = {0};
	int 			cnt = 0, i = 0;
	
	FILE *fp = NULL;
	//int timeCount[301]={0};

	/* insure a clean shutdown if user types ctrl-c */
	sigAction.sa_handler = signalHandler;
	sigemptyset(&sigAction.sa_mask);
	sigAction.sa_flags = 0;
	sigaction(SIGINT, &sigAction, NULL);
	while ((c = getopt(argc, argv, "vh?d:c:")) > 0) {
		switch (c) {
		case 'v':
			FPRINT( stdout, "%s: version %s\n", argv[0], version);
			exit(0);
		case 'd':
			delay = atoi(optarg);
			break;
		case 'c':
			armLoadCnt = atoi(optarg);
			break;
		case 'h':
		case '?':
			usage(stdout, 0);
			break;
		default:
			ERR("unkown option '%c'\n", c);
			usage(stderr, 1);
			break;
		}
	}

	FPRINT( stdout, "ARM LOAD RUNNING FOR COUNT: %d \n", armLoadCnt);

#if 0
	if ((fp = fopen("/tmp/timelog.txt", "wb")) == NULL) {
		ERR("Can't create log file\n");
	//	Quit_flag = TRUE;
	}
#endif
	memset(&cpuinfo, 0, sizeof(CPU_STAT_T));
	
	while (!Quit_flag) {
		/*
		GetIPAddr(TempBuff);
        		uiDrawText(TempBuff, osdData.imageWidth-300, osdData.imageHeight-30, osdData.displayIdx);
		uiDrawText("test", osdData.imageWidth-600, osdData.imageHeight-30, osdData.workingIdx);

		osdData.displayIdx = (osdData.displayIdx + 1) % NUM_OSD_BUFS;
		osdData.workingIdx = (osdData.workingIdx + 1) % NUM_OSD_BUFS;
		waitForVsync(osdData.osdFd);
		*/

		getArmCpuLoad(&cpuinfo);
		if ((cnt > 0) && (cnt <= armLoadCnt)) {
			if (cpuinfo.total > max)
				max = cpuinfo.total;
			if (cpuinfo.total < min)
				min = cpuinfo.total;
			total += cpuinfo.total;
			i ++;
			if(fp) {
				fprintf(fp,"CPU:%3.0f%% (sys:%3.0f%% user:%3.0f%% wait:%3.0f%% steal:%3.0f%%)\n",
					cpuinfo.total, cpuinfo.system,cpuinfo.user, cpuinfo.wait, cpuinfo.steal);
			}
			PRINT("CPU:%3.0f%% (sys:%3.0f%% user:%3.0f%% wait:%3.0f%% steal:%3.0f%%)",
					cpuinfo.total, cpuinfo.system,cpuinfo.user, cpuinfo.wait, cpuinfo.steal);
			
		} else if (cnt > armLoadCnt){
			Quit_flag = TRUE;
		}
		cnt++;
        		sleep(delay);
	}

	total /= i;
	if(fp) {
		fprintf(fp,"Average:%4.3f%%   Max:%4.2f%%  Min:%4.2f%%\n", total, max, min);
	}
	PRINT("Average:%4.3f%%   Max:%4.2f%%  Min:%4.2f%%", total, max, min);
	if(fp)
		fclose(fp);
	
    	return 1;
}

