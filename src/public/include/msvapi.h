#ifndef _H_MS_VAPI_
#define _H_MS_VAPI_

#ifdef __cplusplus
extern "C" {
#endif

#define VAPI2_0

#include "vapi/vapi.h"
#define VAPI_STREAM_ID(chn, from, type, sid) ((int)(((chn) << 24) | ((from) << 16) | ((type) << 8) | (sid)))
#define VAPI_WIN_ID(srceen, sid) ((int)(((srceen) << 16) | ((0xA5) << 8) | (sid)))

#define STREAM_CHN(id)  (((id) >> 24) & 0xff)
#define STREAM_FROM(id) (((id) >> 16) & 0xff)
#define STREAM_TYPE(id) (((id) >> 8) & 0xff)
#define STREAM_SID(id)  ((id) & 0xff)
#define STREAM_NULL()   (VAPI_STREAM_ID(0, 0, 0, 0))  

#define WIN_SID(id)  ((id) & 0xff)
#define WIN_SCREEN(id)  ((id >> 16) & 0xff)

#define MAX_WINS_NUM  (MAX_REAL_CAMERA + 1)

typedef enum{
    AV_FROM_UNKNOWN = 0,
    AV_FROM_LIVE,//live view
    AV_FROM_PB,  //playback
//    AV_FROM_LPB, //retrieve
//    AV_FROM_LPR, //anpr
}AVFROM_E;

typedef enum{
    AV_TYPE_UNKNOWN = 0,
    AV_TYPE_NORMAL,
    AV_TYPE_MAIN,
    AV_TYPE_SUB,
    AV_TYPE_NUN,
}AVTYPE_E;

typedef struct chnwin_s{
    int bshow; //if display or not
    int chnid;
    int appointid; //Specifies that one of the streams in IPC is displayed , 0 is not care
    AVFROM_E enFrom;
    int sid; //where to show in screen
    RATIO_E enRatio;
    FISH_E enFish;
    ZONE_S stZone;
}CHNWIN_S;

int ms_vapi_init();
int ms_vapi_uninit();
int ms_vapi_get_chn_stream_type(int chnId, int screen);
int ms_vapi_win_stream_type(int winid);
int ms_vapi_create(int streamid, FORMAT_S * pstFormat, int winid,  FISH_E enFish, RATIO_E enRatio, ZONE_S *pstZone);
void ms_vapi_destroy(int winid, int streamid);
void ms_vapi_frame(int streamid, void *frame);
void ms_vapi_clear_screen(SCREEN_E enScreen, int clrMap);

#ifdef __cplusplus
}
#endif

#endif //_H_MS_VAPI_
