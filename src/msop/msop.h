#ifndef __MSOP__H__
#define __MSOP__H__

#include "msstd.h"
#include "tsar_socket.h"

#define MSOP_CFG_PATH "/etc/tsar.conf"


typedef enum {
    MSOP_IDEL,
    MSOP_LIVE,
    MSOP_LOG,//
} MSOP_MODE_E;

struct MsopCfgStr {
    MSOP_MODE_E mode;
    TSAR_DEBUG_LEVEL level;
    int module;//bit @ TSAR_SOCK_TYPE

    //tsar path
    char dataPath[128];// @tsar.conf output_file_path

    //log path
    char logPath[128];// @tsar.conf tsar_log_file_path
    char dirPath[128];
    char fileName[64];
    char filterName[64];

    int logStdout;// @tsar.conf tsar_log_stdout
    int logFileOut;// @tsar.conf tsar_log_fileout
    int tsarFileOut;// @tsar.conf tsar_data_fileout
    char debugStr[32];// @tsar.conf debug_level

    //logrotate
    int logrotateDebug;
    int tsarLogCnt;
    int tsarLogSize;
    int tsarDataCnt;
    int tsarDataSize;
    
};

void msop_run(struct MsopCfgStr *conf);
void msop_stop();
void msop_log(struct MsopCfgStr *conf);


#endif
