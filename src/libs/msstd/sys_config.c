/* 
 * ***************************************************************
 * Filename:      	sys_config.c
 * Created at:    	2018.05.17
 * Description:   	sys confgure API
 * Author:        	solin
 * Copyright (C)  	milesight
 * ***************************************************************
 */

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <errno.h>
#include <paths.h>
#include <sys/syscall.h>
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include "msstd.h"
#include "msdefs.h"

int ms_sys_get_chipinfo(char* chip_info, int size)
{
	char *chipinfo_path = "/tmp/chipinfo.txt";
	char cmd[MAX_LEN_64] = {0};
	char info_buf[MAX_LEN_128] = {0};
	FILE *fp = NULL;
	BYTE ret = -1;

	snprintf(cmd, sizeof(cmd), "mschip_update -r 32 > %s", chipinfo_path);
	ms_system(cmd);
	
	fp = fopen(chipinfo_path, "rb");
	if(!fp)
	{
		ret = -1;
	}
	else 
	{
		while(fgets(info_buf, sizeof(info_buf), fp))
		{
			//msprintf("info_buf = %s", info_buf);
			if(strstr(info_buf, "Read old:"))
			{
				snprintf(chip_info, size, "%s", info_buf+9);
				ret = 0;
				break;
			}
		}
		fclose(fp);
	}
	snprintf(cmd, sizeof(cmd), "rm -f %s", chipinfo_path);
	ms_system(cmd);
	return ret;
}

int ms_sys_get_cpu_temperature(float *T)
{
    FILE *pfd = NULL;
    char buff[512];
	
    pfd = ms_vpopen("temperature 0 1", "r");
    if (pfd != NULL)
    {
        if (fgets(buff, sizeof(buff), pfd) != NULL)
        {
            if (sscanf(buff, "Current T: %f 'C", T) != 1)
            {
                msprintf("sscanf failed \n");
				ms_vpclose(pfd);
                return -1;
            }
			ms_vpclose(pfd);
            return 0;
        }
		ms_vpclose(pfd);
    }
    return -1;
}


