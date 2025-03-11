#ifndef _RECO_RTSP_COMMON_H_
#define _RECO_RTSP_COMMON_H_
#include <stdarg.h>
#include "msstd.h"

#ifndef INET6
    #define INET6
#endif
typedef long long INT64;
typedef unsigned long long UINT64;

#define MAJOR_VER        "1"
#define MINOR_VER        "0"
#define MS_RTSP_VERSION  "7349"

//打印函数
void rtsp_format_debug_info(int type, char *fmt, va_list vl);

void reco_rtsp_server_init();
void reco_rtsp_server_uninit();

int trim_char(char *src, int size, char c);
int get_xml_section(char *data, char *name, char value[64]);
int xml_true_value(char *value, int type);
int judge_motion(char *data,  char *name);
int xml_false_value(char *value, int type);

INT64 reco_rtsp_timestamp();

int reco_rtsp_cpu_setaffinity(int cpuCoreId, int pThread);

void reco_rtsp_debug(int level, const char *file, const char *func, int line, char *msg);
void reco_rtsp_log(int level, const char *file, const char *func, int line, const char* format, ...);


struct RecoRtspDebugStr {
    int client;
    int server;
    MS_FUNC_PRINT print;
};

extern struct RecoRtspDebugStr g_recoRtspDebug;



#define rtsp_log(_level, _format, ...) reco_rtsp_log(_level, __FILE__, __func__, __LINE__, _format, ## __VA_ARGS__)

#define rtsp_client_log(_level, _format, ...) \
    do{\
        if (g_recoRtspDebug.client) {\
            rtsp_log(_level, "[RTSP_CLIENT]" _format, ##__VA_ARGS__);\
        }\
    }while(0)

#define rtsp_server_log(_level, _format, ...) \
    do{\
        if (g_recoRtspDebug.server) {\
            rtsp_log(_level, "[RTSP_SERVER]" _format, ##__VA_ARGS__);\
        }\
    }while(0)




#endif
