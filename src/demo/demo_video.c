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
#include <sys/time.h>

#include "demo.h"
#include "vapi.h"
#include "msdefs.h"
#include "msfs_rec.h"

#define DATA_ALIGN_UP(x, a)              ((x+a-1)&(~(a-1)))
#define DATA_ALIGN_BACK(x, a)            ((a) * (((x) / (a))))


#define SST_VAPI_SUPPORTED		(SST_IPC_STREAM|SST_LOCAL_PB)
#define SST_AUDIO_SUPPORT	    (SST_LOCAL_PB | SST_IPC_STREAM)
#define IPC_NUM                 (1)

int ms_rtsp_create_client(int chan_id, int tcp_udp, int rtsp_port, char *ip_addr, char *username, char *password, enum stream_type enType);
extern int ms_rtsp_create_server(int chan_id);

extern int ms_rtsp_init();
extern int ms_rtsp_unit();
extern void send_frame_to_opengl();

typedef struct rtsp_info_s{
	int run_decode[MAX_CHN_NUM];
	MUTEX_OBJECT vapi_mutex[MAX_CHN_NUM];
	int     veChn;
}RTSP_INFO_S;

static RTSP_INFO_S g_rtsp_info;

static int g_exit = 0;

static int test_rtsp_create(char *pIPaddr, char *pUsername, char *pPassword, int type)
{
    int i;
    int res;
    
    for(i = 0; i < IPC_NUM; i++)
    {
        ms_rtsp_create_client(i, 0, 554, pIPaddr, pUsername, pPassword, type);
        res = ms_rtsp_create_server(i);         
    }
    
    printf("\n\nip:%s admin:%s password:%s port:%d\n\n", pIPaddr, pUsername, pPassword, 554); 
    return res;
}

static inline int vapi_check_frame(struct reco_frame* frame)
{
	if (frame->strm_type != ST_VIDEO ) 
		return -1;
	if (!(frame->stream_from & SST_VAPI_SUPPORTED)) 
		return -1;

	return 0;
}

static MF_U64
demo_get_time_us()
{
    struct timeval stTime;
    
    gettimeofday(&stTime, NULL);
    
    return  (stTime.tv_sec * 1000000LLU) + stTime.tv_usec;
}
#if 0
static void frame_reco_to_msfs(struct mf_frame *dst, struct reco_frame *src)
{
	dst->ch                = src->ch;
	dst->strm_type         = src->strm_type;
	dst->codec_type        = src->codec_type;
	dst->stream_from       = src->stream_from;
	dst->stream_format     = src->stream_format;
	dst->time_usec         = src->time_usec;
	dst->time_lower        = src->time_lower;
	dst->time_upper        = src->time_upper;
	dst->size              = src->size;
	dst->frame_type        = src->frame_type;
	dst->frame_rate        = src->frame_rate;
	dst->width             = src->width;
	dst->height            = src->height;
	dst->sample            = src->sample;
	dst->bps               = src->bps;
	dst->achn              = src->achn;
	dst->data              = src->data;
	dst->decch_changed     = src->decch_changed;
	dst->sid               = src->sid;
	dst->sdk_alarm         = src->sdk_alarm;
}
#endif
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

static int callback_venc_stream(int Vechn, VENC_FRAME_S *f)
{
    //printf("========venc id[%d] Iframe[%d] size[%d] pts[%ull]\n", Vechn, f->isIframe, f->size, f->pts);
    vapi_send_frame(1, f->data, f->size, demo_get_time_us(), 1);
    return 0;
}

static int test_rtsp_init()
{
    int i;
    int res = 0;
    
    memset(&g_rtsp_info, 0, sizeof(RTSP_INFO_S));    
	for(i = 0; i < MAX_CHN_NUM; i++)
	{
		ms_mutex_init(&g_rtsp_info.vapi_mutex[i]);		
	}
	
    res = ms_rtsp_init();
    
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
	stVapiAttr.stCallback.venc_stream_cb    = callback_venc_stream;
	stVapiAttr.stCallback.update_chn_cb     = NULL;
	stVapiAttr.stCallback.update_screen_res_cb = NULL;
	stVapiAttr.isHotplug = 0;
	stVapiAttr.isogeny = 1;
	res = vapi_init(&stVapiAttr);	

    return res;
}

static void test_video_uninit()
{
//    vapi_transfer_destory(g_rtsp_info.veChn);
    vapi_uninit();
    test_rtsp_uninit();
}

static void test_vapi_start()
{   
    ACTION_S stAction;
    int i;
    FORMAT_S    stFormat;    
    ZONE_S      stZone;
    FORMAT_S    stIf;
    FORMAT_S    stOf;
	memset(&stIf, 0, sizeof(FORMAT_S));
	memset(&stOf, 0, sizeof(FORMAT_S));


    test_rtsp_create("192.168.63.160", "admin", "yu111111", STREAM_TYPE_MAINSTREAM);


    stFormat.enType = ENC_TYPE_H264;
    stFormat.width  = 4000;
    stFormat.height = 3000;
    stZone.enMode   = ZONE_MODE_USER;

    for (i=0; i<IPC_NUM; i++)
    {
        stAction.stWind.stDevinfo.id               = i;
        stAction.stWind.stDevinfo.stBind.voutChn   = i;
        stAction.stWind.stDevinfo.stBind.vdecChn   = i;
        stAction.stWind.stDevinfo.stFormat         = stFormat;
        stAction.stWind.enRatio                    = 0;
        stAction.stWind.enFish                     = FISH_MODE_1O1R1W2P1P;
        stAction.enAction   = ACTION_ADD_ONE_WIN;
        stAction.enMode     = DSP_MODE_LIVE;
        stAction.enScreen   = SCREEN_MAIN;//(i == 0? SCREEN_MAIN : SCREEN_SUB);

        vapi_get_screen_res(stAction.enScreen, &stZone.w, &stZone.h); 
        stZone.x = 0;
        stZone.y = 0;
        stAction.stWind.stZone = stZone;        
        vapi_set_action(&stAction);
        
    }
    stIf.enType = ENC_TYPE_H264;
    stIf.fps    = 30;
    stIf.width  = 3840;
    stIf.height = 2160;

    stOf.enType = ENC_TYPE_H264;
    stOf.fps    = 30;
    stOf.bitRate= 2*1024*1024;
    stOf.width  = 320;
    stOf.height = 240;
    stOf.enRc   = ENC_RC_CBR;
    
//    g_rtsp_info.veChn = vapi_transfer_create(IPC_NUM, &stIf, &stOf, NULL);
}

//use for playback
int send_frame_to_vdec(struct reco_frame *frame)
{
	int chan_id = -1, run_decode;
    int res = 0;
    chan_id = frame->ch;
    //static MF_U64 time = 0;
    if (frame->strm_type == ST_VIDEO)
    {
        ms_mutex_lock(&g_rtsp_info.vapi_mutex[chan_id]);
        run_decode = g_rtsp_info.run_decode[chan_id];
        ms_mutex_unlock(&g_rtsp_info.vapi_mutex[chan_id]);
//        printf("vapi_write chan_id:%d\n", chan_id);
//        printf("vapi_write chan_sid:%d\n", frame->sid);
//        printf("vapi_write size:%d\n", frame->size);
//        printf("vapi_write time:%lld\n", frame->time_usec);
//        printf("vapi_write type:%d\n", frame->frame_type);
//        printf("=======time = %llu=\n", demo_get_time_us() - time);
//        time = demo_get_time_us();
        if(run_decode)
        {
//            send_frame_to_opengl();
            res = vapi_send_frame(chan_id, frame->data, frame->size, demo_get_time_us(), 1); //display
//            res = vapi_send_frame(IPC_NUM, frame->data, frame->size, demo_get_time_us()); //transfer
        }
    }

    return res;
}

//use for writing disk
int rtsp_write_data(struct reco_frame *frame, int capability)
{
//    struct mf_frame stMFframe;
	int i = 0;
	if(!capability)
		return -1;
		
	
	for(i = 0; i < capability; i++)
	{
		if(vapi_check_frame(&frame[i])) 
			continue;
        if (frame[i].stream_from == SST_IPC_STREAM)
        {
            send_frame_to_vdec(&frame[i]);
//            frame_reco_to_msfs(&stMFframe, &frame[i]);
//            msfs_rec_write(&stMFframe);
        }
        if (frame[i].stream_from == SST_LOCAL_PB)
        {
            send_frame_to_vdec(&frame[i]);
        }
	}
	
	return 0;
}

int test_start()
{
    if (test_video_init() == -1)
    {
        return -1;
    }

    sleep(3);
    test_vapi_start();

    hdinfo("\nstart playing ...\n\n")
    
    g_exit = 0;
//    while(!g_exit)
//    {
//        usleep(100000);
//    }
    
//    test_video_uninit();   
    return 0;
}

int test_stop()
{
    g_exit = 1;
    test_video_uninit();   
    
    return 0;
}

