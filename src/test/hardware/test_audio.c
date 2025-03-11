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

#include "hardware.h"
#include "vapi.h"
#include "gpio.h"

static int g_exit = 0;

static void test_audio_init()
{
    VAPI_ATTR_S stVapiAttr;
    
    memset(&stVapiAttr, 0, sizeof(VAPI_ATTR_S));
    
	stVapiAttr.enRes[SCREEN_MAIN] = SCREEN_1920X1080_60;//display_info.main_resolution;
	stVapiAttr.enRes[SCREEN_SUB] = SCREEN_1920X1080_60;
	stVapiAttr.stCallback.start_stream_cb   = NULL;
	stVapiAttr.stCallback.stop_stream_cb    = NULL;
	stVapiAttr.stCallback.update_chn_cb     = NULL;
	stVapiAttr.stCallback.update_screen_res_cb = NULL;
	stVapiAttr.isHotplug = 0;
	stVapiAttr.isogeny = 1;
	vapi_init(&stVapiAttr);	
}

static void test_audio_uninit()
{
    vapi_uninit();
}

static int test_audio_run(void)
{
	AIO_FRAME_S stFrame;
    AIO_CTRL_S stCtrl;
#if defined (_NT98323_) || defined(_NT98633_)
    if(vapi_audio_read(0 , ARATE_8000, AENC_PCM_ALAW, &stFrame) == 0)
#else
    if(vapi_audio_read(0 , ARATE_32000, AENC_PCM_ALAW, &stFrame) == 0)
#endif
    {
        vapi_audio_write(AO_MAIN_HDMI, &stFrame);
        vapi_audio_write(AO_SPK_MIC, &stFrame);
#if defined (_HI3536_)
        vapi_audio_write(AO_SUB_HDMI, &stFrame);
#endif
    }    

    stCtrl.devID = AO_SPK_MIC;
    stCtrl.enCmd = ACMD_VO_SET_VOLUME;
    stCtrl.value = 100;
    vapi_audio_ctrl(&stCtrl); 
    return 0;
}

static int test_start()
{
    int tmp = 1;
    int key;
    int mute = MUTE_OFF;
    int res = -1;
    unsigned char ver[64] = {0};
	AIO_CTRL_S stCtrl;
//    hdinfo("\n\n\n**********audio_test_start**********\n\n\n");

    res = get_hardware_version(ver);
    if (res != -1)
    {
#if defined (_HI3798_)
		//hdinfo("**********1004 1009 and 5016poe do not test!!**********\n");
		// return -1;
#endif
    }
    else
    {
        hdinfo("**********encrypt chip is failed !!**********\n");
        return -1;
    }

    test_audio_init();
    gpio_write(GIO_MUTE_CTL_EN, mute);
    tmp = gpio_read(GIO_RESET_FACTORY);
  
    g_exit = 0;
    while(!g_exit)
    {
        test_audio_run();
        key = gpio_read(GIO_RESET_FACTORY);
        if (tmp == 1 && key == 0)
        {
            mute = !mute;
			
			if(strstr((char *)ver, "MSN5008")
			|| strstr((char *)ver, "MSN5016"))
			{
				stCtrl.devID = AO_SPK_MIC;
			    stCtrl.enCmd = ACMD_VO_SET_MUTE;
			    stCtrl.value = !mute;
			    vapi_audio_ctrl(&stCtrl); 
			}

			gpio_write(GIO_MUTE_CTL_EN, mute);
        }
        tmp = key;
    }

    test_audio_uninit();
//    hdinfo("**********audio_test_stop**********\n");
    return 0;
}

static int test_stop()
{
    g_exit = 1;
    return 0;
}

HD_MODULE_INFO(TEST_NAME_AUDIO, test_start, test_stop);

