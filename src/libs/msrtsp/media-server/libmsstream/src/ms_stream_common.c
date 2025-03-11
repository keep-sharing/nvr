#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

#include "ms_stream_common.h"
#include "app-log.h"

RTSP_STREAM_CONF g_rtspComonConf = {
    .client = {0},
    .server = {0},
    .print  = NULL,
};

void ms_time_to_string_ex(char *stime, int nLen)
{
    if (!stime) {
        return;
    }
    
    struct tm temp;
    struct timeval tv;
    gettimeofday(&tv, NULL);
    localtime_r(&tv.tv_sec, &temp);
    snprintf(stime, nLen, "%4d-%02d-%02d %02d:%02d:%02d.%03d",
             temp.tm_year + 1900, temp.tm_mon + 1, temp.tm_mday, temp.tm_hour, temp.tm_min, temp.tm_sec, (int)(tv.tv_usec / 1000));
    return ;
}

void ms_rtsp_log(int level, const char *file, const char *func, int line, const char* format, ...)
{
    char buf[1024] = {0};
    va_list ap;

    va_start(ap, format);
    vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);

    g_rtspComonConf.print(level, file, func, line, buf);
}

void set_rtsp_stream_client_debug(int enable)
{
    g_rtspComonConf.client.enable = enable;
    if (g_rtspComonConf.client.enable) {
        g_rtspComonConf.client.level = RTSP_INFO;
    } else {
        g_rtspComonConf.client.level = RTSP_ERR;
    }
}

void set_rtsp_stream_server_debug(int enable)
{
    g_rtspComonConf.server.enable = enable;
    if (g_rtspComonConf.server.enable) {
        g_rtspComonConf.server.level = RTSP_INFO;
    } else {
        g_rtspComonConf.server.level = RTSP_ERR;
    }
}

void set_rtsp_stream_print_func(void *print)
{
    g_rtspComonConf.print = print;
}

void set_rtsp_stream_client_app_log(int enable)
{
    if (enable) {
        app_log_setlevel(LOG_INFO);
    } else {
        app_log_setlevel(LOG_ERROR);
    }
}

void parse_codec_type(const char *encoding, MS_StreamCodecType *codec_type)
{
    if (0 == strcasecmp("H264", encoding)) {
        *codec_type = MS_StreamCodecType_H264;
    } else if (0 == strcasecmp("H265", encoding)) {
        *codec_type = MS_StreamCodecType_H265;
    } else if (0 == strcasecmp("PCMU", encoding)) {
        *codec_type = MS_StreamCodecType_G711_ULAW;
    } else if (0 == strcasecmp("PCMA", encoding)) {
        *codec_type = MS_StreamCodecType_G711_ALAW;
    } else if (0 == strcasecmp(encoding, "MPEG4-GENERIC")) {
        *codec_type = MS_StreamCodecType_AAC;
    } else if (0 == strcasecmp(encoding, "G722")) {
        *codec_type = MS_StreamCodecType_G722;
    } else if (0 == strcasecmp(encoding, "G726")) {
        *codec_type = MS_StreamCodecType_G726;
    } else if (strstr(encoding, "G726-16")) { //maybe AAL2-G726-16 or G726-16
        *codec_type = MS_StreamCodecType_G726;
    } else if (strstr(encoding, "metadata")) {
        *codec_type = MS_StreamCodecType_ONVIF_METADATA;
    } else {
        //【Elaine-伊朗-Kankash: NVR使用RTSP流添加一个第三方设备，播放liveview没有声音】https://www.tapd.cn/33530584/bugtrace/bugs/view?bug_id=1133530584001075132
        *codec_type = MS_StreamCodecType_G711_ULAW;
    }
}

int ms_get_websocket_header_opcode(char *dst, int *dstlen, uint64_t size, int type)
{
    if (size <= 0) {
        return -1;
    }
    
    int Opcode;
    Opcode = WEBSOCKET_BINARY_OPCODE;
	if (type) {
		Opcode = WEBSOCKET_TEXT_OPCODE;
	}
		
	if (size < WEBSOCKET_MIN_OPCODE_LEN) {       
		dst[0] = Opcode;  
		dst[1] = size;  
		*dstlen = 2;  
	} else if (size < WEBSOCKET_MEDIUM_LEN) {
		dst[0] = Opcode;  
		dst[1] = WEBSOCKET_MIN_OPCODE_LEN;  
		dst[2] = (size>>8 & 0xFF);  
		dst[3] = (size & 0xFF);  
		*dstlen = 4;  
	} else {
		dst[0] = Opcode;  
		dst[1] = WEBSOCKET_MEDIUM_OPCODE_LEN;  
		dst[2] = ((size>>56) & 0xFF);
        dst[3] = ((size>>48) & 0xFF);
        dst[4] = ((size>>40) & 0xFF);
        dst[5] = ((size>>32) & 0xFF);
        dst[6] = ((size>>24) & 0xFF);
        dst[7] = ((size>>16) & 0xFF);
        dst[8] = ((size>>8) & 0xFF);
        dst[9] = (size & 0xFF);
		*dstlen = 10; 
	} 
	
    return 0;
}

//mutex
int ms_com_mutex_init(MS_MUTEX_OBJECT *mutex)
{
    int ret = -1;
#ifdef THREAD_MUTEX_RECURSIVE
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
    ret = pthread_mutex_init(mutex, &attr);
    pthread_mutexattr_destroy(&attr);
#else
    ret = pthread_mutex_init(mutex, NULL);
#endif
    if (ret) {
        printf("%s Failed: %d\n", __FUNCTION__, ret);
        return MS_TASK_ERROR;
    }

    return MS_TASK_OK;
}

int ms_com_mutex_trylock(MS_MUTEX_OBJECT *mutex)
{
    return pthread_mutex_trylock(mutex);
}

int ms_com_mutex_timelock(MS_MUTEX_OBJECT *mutex, int us)
{
    struct timespec tout;
    struct timeval now;

    gettimeofday(&now, NULL);
    now.tv_usec += us;
    if (now.tv_usec >= 1000000) {
        now.tv_sec += now.tv_usec / 1000000;
        now.tv_usec %= 1000000;
    }
    tout.tv_nsec = now.tv_usec * 1000;
    tout.tv_sec = now.tv_sec;
    return pthread_mutex_timedlock(mutex, &tout);
}

void ms_com_mutex_lock(MS_MUTEX_OBJECT *mutex)
{
    pthread_mutex_lock(mutex);
}

void ms_com_mutex_unlock(MS_MUTEX_OBJECT *mutex)
{
    pthread_mutex_unlock(mutex);
}

void ms_com_mutex_uninit(MS_MUTEX_OBJECT *mutex)
{
    pthread_mutex_destroy(mutex);
}
