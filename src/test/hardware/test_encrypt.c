#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>
#include <sys/time.h>

#include "hardware.h"

#define MS_NEW_KEY	0
#define YS_OLD_KEY	1
#define MS_OLD_KEY	2

extern unsigned char key_verify(unsigned char *pDataBuffer, int type);
extern int dm2016_open();
extern int dm2016_close(int fd);
extern int dm2016_eeprom_read(int fd, unsigned int offset, int len, unsigned char * data);
extern int dm2016_eeprom_write(int fd, unsigned int offset, int len, char *data);

static void encrypt_get_random_bytes(void* buf, int nbytes)
{
	int i;
	int fd = -1;
	int lose_counter = 0;
	char *cp = (char*)buf;
	struct timeval tv;
	static unsigned seed = 0;
	if (fd > -1)
	{
		while (nbytes > 0)
		{
			i = read(fd, cp, nbytes);
			if ((i < 0) && ((errno == EINTR) || (errno == EAGAIN)))
				continue;
			if (i <= 0)
			{
				if (lose_counter++ == 8)
					break;
				continue;
			}
			nbytes -= i;
			cp += i;
			lose_counter = 0;
		}
		close(fd);
		if(lose_counter < 8)
			return;
	}
	for (i = 0; i < nbytes; i++)
	{
		if (seed == 0){
			gettimeofday(&tv, 0);
			seed = (getpid() << 13) ^ tv.tv_usec ^ getuid() ^ tv.tv_sec;
		}
		*cp++ = rand_r(&seed) & 0xFF;
	}
}

static int encrypt_verify(char *device_info, int len, int key_type)
{
#if defined(_NT98323_) || defined(_HI3536A_) || defined(_NT98633_)
    return 0;
#endif
	int res, i, fd;
	unsigned char buf[128] = {0};
	char data[8];
	char decdata[8];
	unsigned char product[256] = {0};

	encrypt_get_random_bytes(data, 8);
	for(i = 0; i < 8; i ++)
		decdata[i] = data[i];

	res = key_verify((unsigned char*)decdata, key_type);  
	if(res) 
		return -1;

	fd = dm2016_open();
	if(fd < 0)
		return -2;

	res = dm2016_eeprom_read(fd, 0, 32, product);
	snprintf(device_info, len, "%s", product);
	printf("chip context:%s\n", device_info);

	res = dm2016_eeprom_write(fd, 0x90, 8, (char*)decdata);//write the decdate to DM2016
	if(res < 0)
	{
		dm2016_close(fd);
		return -3;
	}

	res = dm2016_eeprom_read(fd, 0x90, 8, buf);//read back
	if(res < 0)
	{
		dm2016_close(fd);
		return -4;
	}

	for(i = 0; i < 8; i ++)
	{
		if(data[i] != buf[i])
		{
			dm2016_close(fd);
			return -5;// compare the result to the random number
		}
	}
	
	res = dm2016_eeprom_read(fd, 0, 32, product);
	snprintf(device_info, len, "%s", product);
	dm2016_close(fd);
	if(res < 0)
		return -6;

	return res;
}    

int ms_sys_start(void)
{
	char device_info[256] = {0};
	int verify_res = -1;

	verify_res = encrypt_verify(device_info, sizeof(device_info), MS_NEW_KEY);
	if(verify_res)
	{
		verify_res = encrypt_verify(device_info, sizeof(device_info), MS_OLD_KEY);
		if(verify_res)
		{
			SHOW_NO;
			return -1;	
		}
		
	}
	
    SHOW_YES;
	return 0;	
}
#if 0
static int test_start()
{
    
//    system("clear");
//    hdinfo("\n\n\n**********encrypt_test_start**********\n\n\n");

    ms_sys_start();
    
//    hdinfo("**********encrypt_test_stop**********\n");
    return 0;
}

static int test_stop()
{
    return 0;
}
#endif
//HD_MODULE_INFO(TEST_NAME_ENCRYPT, test_start, test_stop);

