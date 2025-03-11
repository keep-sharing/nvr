#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/stat.h>

#include "hardware.h"
#include "hisi_dm2016_eeprom.h"
#include "msoem.h"
#include "chipinfo.h"


int g_isAuto = 1;
int g_isTestAio = 0;	//special test Aio for hi3798

static struct hd_module_info *g_test_info = NULL;
static int g_module_num = 0;

static const char *short_options = "?hGAVIREF";
static struct option long_options[] = {
	{0, 0, 0, 0},
};

int get_hardware_version(unsigned char *device_info)
{
   	ms_env_init();
	char chipinfo[CHIPINFO_LEN + 1] = {0};
	if(mschip_read(chipinfo))
	{
		return -1;
	}
	strncpy((char *)device_info, chipinfo, DM2016_VAL_LEN);
	printf("device_info:%s\n", device_info);
	ms_env_deinit();
	return 0;

}

static void signal_handler(int signal)
{	
    int i;

    for (i=0; i<g_module_num; i++)
    {
        g_test_info[i].unload();
    }

}

static void set_test_signal()
{
	signal(SIGINT,  signal_handler);
	signal(SIGTERM, signal_handler);
	signal(SIGKILL, signal_handler);
}

static void hd_test_usage()
{
    printf("\nUsage : hardware [option]\n");   
    printf("option : \n");
    printf("\t-G ----------- gpio  test\n");
    printf("\t-V ----------- video test\n");
    printf("\t-A ----------- audio test\n");   
    printf("\t-I ----------- IR    test\n");
    printf("\t-R ----------- RS485 test\n");
    printf("\t-E --------- encrypt test\n");
    printf("\t-F ----------- flash test\n");
}

int main(int argc, char **argv)
{	
    int res = -1;
    int ch;
	int option_index = 0;
	int i;
    
	if (argc > 2)
	{
        hd_test_usage();
        return -1;
	}

	if (argc == 1)
	{
        g_isAuto = 1;
        //ms_system("killall -SIGURG daemon");
		//ms_system("killall -9 gui");
        sleep(1);
	}
	else
	{
        g_isAuto = 0;
	}
	
	set_test_signal();
    //ms_system("killall -9 mscore");

	if (g_isAuto == 0)
	{
        while ((ch = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1)
        {
            switch(ch)
            {
                case 'G' : 
                {
                    res = hd_get_module_by_name(&g_test_info, TEST_NAME_GPIO);
                    break;
                }
                case 'A' : 
                {
				#if defined (_HI3798_)
					g_isTestAio = 1;
					ms_system("killall -SIGURG daemon");
				#endif
					res = hd_get_module_by_name(&g_test_info, TEST_NAME_AUDIO);	 
                    break;
                }
                case 'V' : 
                {
				#if defined (_HI3798_)
					g_isTestAio = 1;
					ms_system("killall -SIGURG daemon");
				#endif
                    res = hd_get_module_by_name(&g_test_info, TEST_NAME_VIDEO);
                    break;
                }
                case 'I' : 
                {
                    res = hd_get_module_by_name(&g_test_info, TEST_NAME_IR);
                    break;
                }
                case 'R' : 
                {
                    res = hd_get_module_by_name(&g_test_info, TEST_NAME_RS485);
                    break;
                }
                case 'E' : 
                {
                    res = hd_get_module_by_name(&g_test_info, TEST_NAME_ENCRYPT);
                    break;
                }
                case 'F' : 
                {
                    res = hd_get_module_by_name(&g_test_info, TEST_NAME_FLASH);
                    break;
                }
                case 'h':
                default :
                {
                    hd_test_usage(); 
                    return -1;
                }
            }
        }
        g_module_num = 1;
	}
    else
    {
#if !defined (_NT98323_)
        ms_system("killall -SIGURG daemon");
#endif
        res = hd_get_modules(&g_test_info, &g_module_num);
    }

    if (res == 0)
    {
        for (i=0; i<g_module_num; i++)
        {
            hdinfo("\n\n\nTEST:  %s\n", g_test_info[i].name);
            g_test_info[i].load();
        }
    }
    
	if (g_isAuto == 1)
	{
        usleep(1000);
        ms_system("killall -SIGURG daemon");
	}
	else
	{
#if defined (_HI3798_)
		if(g_isTestAio)
		{
			g_isTestAio = 0;
			ms_system("killall -SIGURG daemon");
		}
#endif

	}
	return 0;	
}


