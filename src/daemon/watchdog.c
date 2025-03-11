#include "watchdog.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <linux/ioctl.h>
#include <linux/watchdog.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "msdefs.h"

#define WATCHDOG_CONF_ITEM "watchdog"
static int global_fd = -1;

#if defined(_HI3798_)
typedef struct _tagWDG_TIMEOUT_S
{
	int	u32WdgIndex;
	int	s32Timeout;
}WDG_TIMEOUT_S;

typedef struct _tagWDG_OPTION_S
{
	int	u32WdgIndex;
	int	s32Option;
}WDG_OPTION_S;

#define WATCHDOG_IOCTL_BASE 'W'

#define WDIOC_SET_OPTIONS _IOR(WATCHDOG_IOCTL_BASE, 4, WDG_OPTION_S)
#define WDIOC_KEEP_ALIVE _IOR(WATCHDOG_IOCTL_BASE, 5, int)
#define WDIOC_SET_TIMEOUT _IOWR(WATCHDOG_IOCTL_BASE, 6, WDG_TIMEOUT_S)
#define WDIOC_GET_TIMEOUT _IOR(WATCHDOG_IOCTL_BASE, 7, WDG_TIMEOUT_S)
#define WDIOS_DISABLECARD 0x0001  /* Turn off the watchdog timer */
#define WDIOS_ENABLECARD 0x0002  /* Turn on the watchdog timer */
#define WDIOS_RESET_BOARD 0x0008  /* reset the board */

#endif

static int read_watchdog_conf()
{
	int startup = 1;
	char conf[256] = {0};
	snprintf(conf, sizeof(conf), "%s/%s", MS_CONF_DIR, WATCHDOG_CONF_NAME);
	FILE* fp = fopen(conf, "r");
	if (!fp) 
		return 1;
#if 0	
	while(1)
	{
		char data[64] = {0};
		if (!fgets(data, sizeof(data), fp)) 
			break;
		ms_string_strip(data, ' ');
		//watchdog=yes|no
		if (strncmp(data, WATCHDOG_CONF_ITEM, 8)) 
			continue;
		if (strncasecmp(data+8+1, "no", 2) == 0)
		{
			startup = 0;
			msprintf("======== watchdog=no watch dog closed! ==========");
			break;
		}
	}
#else
	startup = 0;
	msprintf("========watch dog closed==========");
#endif	
	fclose(fp);
	return startup;
}

static int watchdog_open()
{
	//system("modprobe wdt nodeamon=1");
	int ret = 0;
	int fd = -1;
	
#if defined(_HI3536C_) || defined(_HI3536_)
	fd = open("/dev/watchdog", O_WRONLY);
	if (fd < 0) 
	{
		printf("watchdog open failed");
		return -1;
	}

	int timeout = 45; //unit 2s
	ret = ioctl(fd, WDIOC_SETTIMEOUT, &timeout);
	if(ret < 0)
	{
		perror("ioctl WDIOC_SETTIMEOUT");
	}
	
	ret = ioctl(fd, WDIOC_GETTIMEOUT, &timeout);
	if(ret < 0)
	{
		perror("ioctl WDIOC_GETTIMEOUT");
	}
#elif defined(_NT98323_)
	fd = open("/dev/watchdog", O_WRONLY);
	if (fd < 0) 
	{
		printf("watchdog open failed");
		return -1;
	}

	int timeout = 89; //unit s
	ret = ioctl(fd, WDIOC_SETTIMEOUT, &timeout);
	if(ret < 0)
	{
		perror("ioctl WDIOC_SETTIMEOUT");
	}
	
	ret = ioctl(fd, WDIOC_GETTIMEOUT, &timeout);
	if(ret < 0)
	{
		perror("ioctl WDIOC_GETTIMEOUT");
	}
#elif defined(_HI3798_)
	WDG_TIMEOUT_S stTimeout;	
	WDG_OPTION_S stOption;
	
    fd = open("/dev/hi_wdg", O_RDWR, 0);
    if (fd < 0) 
    {
        printf("watchdog open failed\n");
        return -1;
    }
    
	stTimeout.s32Timeout = 90; //unit s
	stTimeout.u32WdgIndex = 0;
    ret = ioctl(fd, WDIOC_SET_TIMEOUT, &stTimeout);
    if (ret < 0) 
    {
        perror("WDIOC_SET_TIMEOUT failed\n");
    }	

    stOption.s32Option = WDIOS_ENABLECARD;
	stOption.u32WdgIndex = 0;

    ret = ioctl(fd, WDIOC_SET_OPTIONS, &stOption);
    if (ret < 0) 
    {
        perror("WDIOS_ENABLECARD failed\n");
    }	
#elif defined(_HI3536A_) || defined(_NT98633_)
    return -1;

#else
     #error YOU MUST DEFINE  CHIP_TYPE!
#endif
	return fd;
}
static void watchdog_close(int fd)
{
	if(fd >= 0)
	{
		close(fd);
	}
}
static int watchdog_keep_alive(int fd)
{
	if(fd < 0) 
		return -1;
#if defined(_HI3536C_) || defined(_HI3536_) || defined(_NT98323_)
	return ioctl(fd, WDIOC_KEEPALIVE, 0);
	
#elif defined(_HI3798_)
    int WdgNum = 0;
    return ioctl(fd, WDIOC_KEEP_ALIVE, &WdgNum);
    
#else
    return -1;
    
#endif
}

int watchdog_start()
{
	if (global_fd >= 0) 
		return 1;
	if (!read_watchdog_conf()) 
		return -1;
	global_fd = watchdog_open();
	if (global_fd < 0)
		return -1;
	return 0;
}
void watchdog_stop()
{
	if (global_fd < 0)
		return;
	watchdog_close(global_fd);
	global_fd = -1;
}
void watchdog_keepalive()
{
	if (global_fd < 0) 
		return;
	watchdog_keep_alive(global_fd);
}

