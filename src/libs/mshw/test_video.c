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

#define SST_VAPI_SUPPORTED      (SST_IPC_STREAM|SST_LOCAL_PB)
#define SST_AUDIO_SUPPORT       (SST_LOCAL_PB | SST_IPC_STREAM)
#if defined(_NT98323_) || defined(_NT98633_)
    #define IPC_NUM 1
#else
    #define IPC_NUM 2
#endif

extern int ms_rtsp_create_client(int chan_id, int tcp_udp, int rtsp_port, char *ip_addr, char *username,
                                 char *password);
extern void ms_rtsp_destroy_client(int chan_id);

extern int ms_rtsp_create_server(int chan_id);

extern int ms_rtsp_init(int rtsp_server_port);
extern int ms_rtsp_unit();

static int g_exit = 0;

static inline int vapi_check_frame(struct reco_frame *frame)
{
    if (frame->strm_type != ST_VIDEO
#if defined (_HI3798_)
        && frame->strm_type != ST_AUDIO
#endif
       ) {
        return -1;
    }
    if (!(frame->stream_from & SST_VAPI_SUPPORTED)
#if defined (_HI3798_)
        && !(frame->stream_from & SST_AUDIO_SUPPORT)
#endif
       ) {
        return -1;
    }

    return 0;
}

int rtsp_write_data(struct reco_frame *frame, int capability)
{
    int i = 0, chan_id = -1;
    AIO_FRAME_S audio_frame;
    memset(&audio_frame, 0, sizeof(AIO_FRAME_S));
    if (!capability) {
        return -1;
    }

    for (i = 0; i < capability; i++) {
        chan_id = frame[i].ch;
        if (vapi_check_frame(&frame[i])) {
            continue;
        }
        if (frame[i].strm_type == ST_VIDEO) {
            //printf("vapi_write chan_id:%d\n", chan_id);
            vapi_send_frame(chan_id, frame[i].data, frame[i].size, frame[i].time_usec, 1);
        }
#if defined (_HI3798_)
        else if (frame[i].strm_type == ST_AUDIO) {
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

static int test_rtsp_init(char *ip, char *usr, char *psw)
{
    int i;
    int res = 0;

//    ms_rtsp_init(554);
    for (i = 0; i < IPC_NUM; i++) {
        res |= ms_rtsp_create_client(i, 0, 554, ip, usr, psw);
    }
    return res;
}

static void test_rtsp_uninit()
{
    int i;

//    ms_rtsp_unit();

    for (i = 0; i < IPC_NUM; i++) {
        ms_rtsp_destroy_client(i);
    }
}

static void test_vapi_start(VideoFormat format)
{
    LAYOUT_S stLayout;
    int i;
    FORMAT_S    stFormat;
    ZONE_S      stZone;

    stFormat.enType = format == VIDEO_FORMAT_H264 ?
                      ENC_TYPE_H264 : ENC_TYPE_H265;
    stFormat.width  = 3840;
    stFormat.height = 2160;
    stZone.enMode   = ZONE_MODE_USER;
    stZone.x        = 0;
    stZone.y        = 0;
    for (i = 0; i < IPC_NUM; i++) {
        stLayout.stWinds[0].stDevinfo.id               = i;
        stLayout.stWinds[0].stDevinfo.stBind.voutChn   = 0;
        stLayout.stWinds[0].stDevinfo.stBind.vdecChn   = i;
        stLayout.stWinds[0].stDevinfo.stFormat         = stFormat;
        stLayout.stWinds[0].enRatio                    = 0;
        stLayout.winds_num = 1;
        stLayout.enRefresh  = REFRESH_CLEAR_VDEC;
        stLayout.enMode     = DSP_MODE_LIVE;
        stLayout.enScreen   = (i == 0 ? SCREEN_MAIN : SCREEN_SUB);

        vapi_get_screen_res(stLayout.enScreen, &stZone.w, &stZone.h);
        stLayout.stWinds[0].stZone = stZone;
        stLayout.stWinds[0].enFish = FISH_MODE_NONE;
        stLayout.stWinds[0].enRatio = RATIO_VO_FULL;
        vapi_update_layout(&stLayout);
    }

}

static void test_vapi_stop()
{
    vapi_clear_screen2(SCREEN_MAIN);
    vapi_clear_screen2(SCREEN_SUB);
}

static int test_start(void *param)
{
    TestVideo *p = (TestVideo *)param;

    if (p) {
        if (test_rtsp_init(p->ip, p->user, p->password)) {
            hderr("\nrtsp init failed \n");
            return -1;
        }
        test_vapi_start(p->format);
    } else {
        if (test_rtsp_init("192.168.5.190", "admin", "1")) {
            hderr("\nrtsp init failed \n");
            return -1;
        }
        test_vapi_start(VIDEO_FORMAT_H264);
    }
    hdinfo("\nstart playing ...\n\n");
    g_exit = 0;
    while (!g_exit) {
        usleep(100000);
    }

    test_rtsp_uninit();
    test_vapi_stop();
    hdinfo("\nstop playing ...\n\n");

    return 0;
}

static int test_stop()
{
    g_exit = 1;
    return 0;
}

static int test_find()
{
    return 1;
}

HD_MODULE_INFO(TEST_NAME_VIDEO, test_find, test_start, test_stop);

