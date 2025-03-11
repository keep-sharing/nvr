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
#include "msdefs.h"

#define SST_VAPI_SUPPORTED		(SST_IPC_STREAM|SST_LOCAL_PB)
#define SST_AUDIO_SUPPORT	    (SST_LOCAL_PB | SST_IPC_STREAM)
#if defined(_NT98323_)
#define IPC_NUM 1
#else
#define IPC_NUM 2
#endif

extern int ms_rtsp_create_client(int chan_id, int tcp_udp, int rtsp_port, char *ip_addr, char *username, char *password);
extern int ms_rtsp_create_server(int chan_id);

extern int ms_rtsp_init(int rtsp_server_port);
extern int ms_rtsp_unit();

typedef struct rtsp_info_s{
	int run_decode[MAX_CHN_NUM];
	MUTEX_OBJECT vapi_mutex[MAX_CHN_NUM];
}RTSP_INFO_S;

static RTSP_INFO_S g_rtsp_info;

static int g_exit = 0;
static int g_mode = 0;

static int test_rtsp_create(char *pIPaddr, char *pUsername, char *pPassword)
{
    int i;
    int res;
    
    for(i = 0; i < IPC_NUM; i++)
    {
        ms_rtsp_create_client(i, 0, 554, pIPaddr, pUsername, pPassword);
        res = ms_rtsp_create_server(i);         
    }
    
    printf("\n\nip:%s admin:%s password:%s port:%d\n\n", pIPaddr, pUsername, pPassword, 554); 
    return res;
}

static inline int vapi_check_frame(struct reco_frame* frame)
{
	if (frame->strm_type != ST_VIDEO
#if defined (_HI3798_)
	    && frame->strm_type != ST_AUDIO
#endif
	    ) 
		return -1;
	if (!(frame->stream_from & SST_VAPI_SUPPORTED)
#if defined (_HI3798_)
	    && !(frame->stream_from & SST_AUDIO_SUPPORT)
#endif
	    ) 
		return -1;

	return 0;
}

int rtsp_write_data(struct reco_frame *frame, int capability)
{
	int i = 0, chan_id = -1, run_decode;
	AIO_FRAME_S audio_frame;
	memset(&audio_frame, 0, sizeof(AIO_FRAME_S));
	if(!capability)
		return -1;
	
	for(i = 0; i < capability; i++)
	{
		chan_id = frame[i].ch;
		if(vapi_check_frame(&frame[i])) 
			continue;
        if (frame[i].strm_type == ST_VIDEO)
        {
            ms_mutex_lock(&g_rtsp_info.vapi_mutex[chan_id]);
            run_decode = g_rtsp_info.run_decode[chan_id];
            ms_mutex_unlock(&g_rtsp_info.vapi_mutex[chan_id]);
            //printf("vapi_write chan_id:%d\n", chan_id);
            if(run_decode)
                vapi_send_frame(chan_id, frame[i].data, frame[i].size, frame[i].time_usec, 1); 
        }
#if defined (_HI3798_)
        else if (frame[i].strm_type == ST_AUDIO)
        {
    		audio_frame.enArate = (ARATE_E)frame[i].sample;
    		audio_frame.enAenc = (AENC_E)AENC_PCM_ULAW;//global_audio_conf.audio.codec;//notyet
    		audio_frame.enAbit = (ABIT_E)ABIT_8_BIT;//global_audio_conf.audio.bit;//notyet
    		audio_frame.pts = frame[i].time_usec;    		
			audio_frame.date = (unsigned char *)frame[i].data;
            audio_frame.len = frame[i].size;  
            vapi_audio_write(AO_SPK_MIC, &audio_frame);
        }
#endif
	}
	
	return 0;
}


static int on_run_decode(int chan_id, int enable)
{
	ms_mutex_lock(&g_rtsp_info.vapi_mutex[chan_id]);
	g_rtsp_info.run_decode[chan_id] = enable;
	ms_mutex_unlock(&g_rtsp_info.vapi_mutex[chan_id]);
	return 0;
}

static int callback_start_stream(int chan_id)
{
	on_run_decode(chan_id, 1);//debug

    return 0;
}

static int callback_stop_stream(int chan_id)
{
	on_run_decode(chan_id, 0);

	return 0;
}


static int test_rtsp_init()
{
    int i;
    int res = 0;
    char aIP[32] = "192.168.63.161";    
    char aUsr[32] = "admin";
    char aPsw[32] = "yu111111";

    memset(&g_rtsp_info, 0, sizeof(RTSP_INFO_S));    
	for(i = 0; i < MAX_CHN_NUM; i++)
	{
		ms_mutex_init(&g_rtsp_info.vapi_mutex[i]);		
	}
	
    ms_rtsp_init(554);
    if(!g_isAuto)
        getchar();
    if (g_mode == 1)
    {
        hdinfo("ip:\n\t");
        gets(aIP);
        hdinfo("username:\n\t");
        gets(aUsr);
        hdinfo("password:\n\t");
        gets(aPsw);
    }
    res = test_rtsp_create(aIP, aUsr, aPsw);

    return res;
}

static void test_rtsp_uninit()
{
    int i;
    
    ms_rtsp_unit();
    
	for(i = 0; i < MAX_CHN_NUM; i++)
	{
		ms_mutex_uninit(&g_rtsp_info.vapi_mutex[i]);		
	}
}
static int test_video_init()
{    
    VAPI_ATTR_S stVapiAttr;
    int res;
    
    if (test_rtsp_init() == -1)
    {
        hderr("\nrtsp init failed \n");
        return -1;
    }
    
    memset(&stVapiAttr, 0, sizeof(VAPI_ATTR_S));    
	stVapiAttr.enRes[SCREEN_MAIN] = SCREEN_1920X1080_60;//display_info.main_resolution;
	stVapiAttr.enRes[SCREEN_SUB] = SCREEN_1920X1080_60;
	stVapiAttr.stCallback.start_stream_cb   = callback_start_stream;
	stVapiAttr.stCallback.stop_stream_cb    = callback_stop_stream;
	stVapiAttr.stCallback.update_chn_cb     = NULL;
	stVapiAttr.stCallback.update_screen_res_cb = NULL;
	stVapiAttr.isHotplug = 0;
	stVapiAttr.isogeny = 1;
	stVapiAttr.isDualScreen = 1;
	res = vapi_init(&stVapiAttr);	

    return res;
}

static void test_video_uninit()
{
    vapi_uninit();
    test_rtsp_uninit();
}

static void test_vapi_start()
{   
    LAYOUT_S stLayout;
    int i;
    char ch;
    FORMAT_S    stFormat;    
    ZONE_S      stZone;

    if (g_mode == 1)
    {
//        system("clear");
        hdinfo("\n\nstream type(1:h264 2:h265 3:Mjpeg):\n\t");
        ch = getchar();
        if (ch == '1')
        {
            stFormat.enType = ENC_TYPE_H264;
        }
        else if(ch == '2')
        {
            stFormat.enType = ENC_TYPE_H265;
        }
        else if (ch == '3')
        {
            stFormat.enType = ENC_TYPE_MJPEG;
        }
    }
    else
    {
        stFormat.enType = ENC_TYPE_H264;
    }
    
    stFormat.width  = 3840;
    stFormat.height = 2160;
    stZone.enMode   = ZONE_MODE_USER;
    stZone.x        = 0;
    stZone.y        = 0;
    for (i=0; i<IPC_NUM; i++)
    {
        stLayout.stWinds[0].stDevinfo.id               = i;
        stLayout.stWinds[0].stDevinfo.stBind.voutChn   = 0;
        stLayout.stWinds[0].stDevinfo.stBind.vdecChn   = i;
        stLayout.stWinds[0].stDevinfo.stFormat         = stFormat;
        stLayout.stWinds[0].enRatio                    = 0;
        stLayout.winds_num = 1;
        stLayout.enRefresh  = REFRESH_CLEAR_VDEC;
        stLayout.enMode     = DSP_MODE_LIVE;
        stLayout.enScreen   = (i == 0? SCREEN_MAIN : SCREEN_SUB);
        
        vapi_get_screen_res(stLayout.enScreen, &stZone.w, &stZone.h);        
        stLayout.stWinds[0].stZone = stZone;        
        vapi_update_layout(&stLayout);
    }
    
}
static int test_start()
{
    char ch = 0;
    
//    system("clear");
//    hdinfo("\n\n\n**********viddo_test_start**********\n\n\n");
    if (g_isAuto)
    {
        g_mode = 0;
    }
    else
    {
        printf("0: auto mode 1: user mode\n");
        printf("choose : ");
        ch =  getchar();
        
        if (ch == '1')
        {
            g_mode = 1;
        }
        else
        {
            g_mode = 0;
        }
    }
    
    if (test_video_init() == -1)
    {
        return -1;
    }

    sleep(3);
    test_vapi_start();

    hdinfo("\nstart playing ...\n\n")
    g_exit = 0;
    while(!g_exit)
    {
        usleep(100000);
    }
    
    test_video_uninit();   
//    hdinfo("**********viddo_test_stop**********\n");
    return 0;
}

static int test_stop()
{
    g_exit = 1;
    g_mode = 0;
//    hdinfo("\nstop playing ...\n\n")
    return 0;
}


HD_MODULE_INFO(TEST_NAME_VIDEO, test_start, test_stop);

