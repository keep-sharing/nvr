#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <getopt.h>
#include "msstd.h"
#include "msop.h"

#define MSOP_PID_PATH "/tmp/msop"
unsigned int global_debug_mask = DEBUG_LEVEL_DEFAULT;
hook_print global_debug_hook = 0;

struct option msopOpts[] = {
    { "live", no_argument, NULL, 'l' },
    { "log", no_argument, NULL, 'L' },
    { "debug", required_argument, NULL, 'd' },
    { "help", no_argument, NULL, 'h' },
    { 0, 0, 0, 0},
};

void msop_usage()
{
    fprintf(stderr,
            "Usage: msop [options]\n"
            "Options:\n"
            "    --live/-l      running print live mode, msg will print.example:msop --live\n"
            "    --log/-L       display log content. example:msop --log\n"
            "    --debug/-d     show level(INFO DEBUG WARN ERROR FATAL default: WARN). example:msop --debug ERROR\n"
            "    --help/-h      help\n");
    exit(0);
}

static void msop_signal_handler(int signal)
{
    switch (signal) {
        case SIGINT:
        case SIGTERM:
        case SIGKILL:
        case SIGPIPE:
            msop_stop();
            break;
        default:
            break;
    }   
}

static void set_msop_signal()
{
    signal(SIGINT,  msop_signal_handler);
    signal(SIGTERM, msop_signal_handler);
    signal(SIGKILL, msop_signal_handler);
    signal(SIGPIPE, msop_signal_handler);
}

static TSAR_DEBUG_LEVEL msop_get_debug_level(const char *data)
{
    TSAR_DEBUG_LEVEL level = TSAR_WARN;

    if (!data) {
        return level;
    }

    if (!strcasecmp(data, "INFO")) {
        level = TSAR_INFO;
    } else if (!strcasecmp(data, "WARN")) {
        level = TSAR_WARN;
    } else if (!strcasecmp(data, "DEBUG")) {
        level = TSAR_DEBUG;
    } else if (!strcasecmp(data, "ERROR")) {
        level = TSAR_ERR;
    } else if (!strcasecmp(data, "FATAL")) {
        level = TSAR_FATAL;
    } else {
        level = TSAR_WARN;
    }

    return level;
}

static void msop_parse_argc(int argc, char **argv, struct MsopCfgStr *cfg)
{
    int opt;
    int level = 0;
    
    while ((opt = getopt_long(argc, argv, ":d:Lfh", msopOpts, NULL)) != -1) {
        switch (opt) {
            case 'l':
                cfg->mode = MSOP_LIVE;
                break;
            case 'L':
                cfg->mode = MSOP_LOG;
                break;
            case 'd':
                level = 1;
                cfg->level = msop_get_debug_level(optarg);
                break;
            case ':':
                printf("must have parameter\n");
                msop_usage();
                break;
            case 'h':
            case '?':
                msop_usage();
                break;
        }
    }

    if (cfg->mode == MSOP_IDEL && cfg->logStdout) {
        cfg->mode = MSOP_LIVE;
    }

    if (!level) {
        cfg->level = msop_get_debug_level(cfg->debugStr);
    }
}

static int msop_parse_get_int(char *token, int *value)
{
    if (!token) {
        return -1;
    }

    *value = atoi(token);
    return 0;
}

static int msop_parse_get_string(char *token, char *value, int len)
{
    if (!token) {
        return -1;
    }

    snprintf(value, len, "%s", token);
    return 0;
}

//on/enable = 1, others = 0;
static int msop_parse_get_switch(char *token, int *value)
{
    if (!token) {
        return -1;
    }

    if (token && (!strcasecmp(token, "on") || !strcasecmp(token, "enable"))) {
        *value = 1;
    } else {
        *value = 0;
    }
    
    return 0;
}

static void msop_parse_line(char *line, int len, struct MsopCfgStr *cfg)
{
    char *token;
    char *savePtr = NULL;
    char *innerPtr;
    char *space = " \t\r\n";

    if ((token = strchr(line, '\n'))) {
        *token = '\0';
    }
    if ((token = strchr(line, '\r'))) {
        *token = '\0';
    }
    if (line[0] == '#' || line[0] == '\0') {
        return;
    }

    innerPtr = line;
    token = strtok_r(innerPtr, space, &savePtr);
    if (!token) {
        return;
    }

    innerPtr = NULL;
    if (strstr(token, "tsar_log_file_path")) {
        token = strtok_r(innerPtr, space, &savePtr);
        msop_parse_get_string(token, cfg->logPath, sizeof(cfg->logPath));
    } else if (strstr(token, "output_file_path")) {
        token = strtok_r(innerPtr, space, &savePtr);
        msop_parse_get_string(token, cfg->dataPath, sizeof(cfg->dataPath));
    } else if (strstr(token, "tsar_log_logrotate_cnt")) {
        token = strtok_r(innerPtr, space, &savePtr);
        msop_parse_get_int(token, &cfg->tsarLogCnt);
        //printf("tsar_log_logrotate_cnt %s %d\n", token, cfg->tsarLogCnt);
    } else if (strstr(token, "tsar_log_logrotate_size")) {
        token = strtok_r(innerPtr, space, &savePtr);
        msop_parse_get_int(token, &cfg->tsarLogSize);
        //printf("tsar_log_logrotate_size %s %d\n", token, cfg->tsarLogSize);
    } else if (strstr(token, "tsar_data_logrotate_cnt")) {
        token = strtok_r(innerPtr, space, &savePtr);
        msop_parse_get_int(token, &cfg->tsarDataCnt);
        //printf("tsar_data_logrotate_cnt %s %d\n", token, cfg->tsarDataCnt);
    } else if (strstr(token, "tsar_data_logrotate_size")) {
        token = strtok_r(innerPtr, space, &savePtr);
        msop_parse_get_int(token, &cfg->tsarDataSize);
        //printf("tsar_data_logrotate_size %s %d\n", token, cfg->tsarDataSize);
    } else if (strstr(token, "tsar_logrotate_debug")) {
        token = strtok_r(innerPtr, space, &savePtr);
        msop_parse_get_switch(token, &cfg->logrotateDebug);
        //printf("tsar_logrotate_debug %s %d\n", token, cfg->logrotateDebug);
    } else if (strstr(token, "tsar_log_stdout")) {
        token = strtok_r(innerPtr, space, &savePtr);
        msop_parse_get_switch(token, &cfg->logStdout);
        //printf("tsar_log_stdout %s %d\n", token, cfg->logStdout);
    } else if (strstr(token, "debug_level")) {
        token = strtok_r(innerPtr, space, &savePtr);
        msop_parse_get_string(token, cfg->debugStr, sizeof(cfg->debugStr));
        //printf("debug_level %s %s\n", token, cfg->debugStr);
    } else if (strstr(token, "tsar_log_fileout")) {
        token = strtok_r(innerPtr, space, &savePtr);
        msop_parse_get_switch(token, &cfg->logFileOut);
        //printf("tsar_log_fileout %s %d\n", token, cfg->logFileOut);
    } else if (strstr(token, "tsar_data_fileout")) {
        token = strtok_r(innerPtr, space, &savePtr);
        msop_parse_get_switch(token, &cfg->tsarFileOut);
        //printf("tsar_data_fileout %s %d\n", token, cfg->tsarFileOut);
    }
    

    if (cfg->tsarLogCnt == 0) {
        cfg->tsarLogCnt = 4;
    }
    
    if (cfg->tsarLogSize == 0) {
        cfg->tsarLogSize = 200*1024;
    }

    if (cfg->tsarDataCnt == 0) {
        cfg->tsarDataCnt = 4;
    }
    
    if (cfg->tsarDataSize == 0) {
        cfg->tsarDataSize = 200*1024;
    }
    
    return;
}

static int msop_parse_cfg(struct MsopCfgStr *cfg)
{
    FILE *fp;
    char line[1024] = {0};

    if (!(fp = fopen(MSOP_CFG_PATH, "r"))) {
        return -1;
    }

    memset(line, 0, sizeof(line));
    while (fgets(line, sizeof(line), fp)) {
        msop_parse_line(line, sizeof(line), cfg);
        memset(line, 0, sizeof(line));
    }

    fclose(fp);
    return 0;
}

static void get_msop_mode_str(MSOP_MODE_E mode, char *str, int len)
{
    if (mode == MSOP_LOG) {
        snprintf(str, len, "%s", "log");
    } else {
        snprintf(str, len, "%s", "live");
    }
}

static void get_msop_level_str(TSAR_DEBUG_LEVEL level, char *str, int len)
{
    if (level == TSAR_INFO) {
        snprintf(str, len, "%s", "INFO");
    } else if (level == TSAR_DEBUG) {
        snprintf(str, len, "%s", "DEBUG");
    } else if (level == TSAR_WARN) {
        snprintf(str, len, "%s", "WARN");
    } else if (level == TSAR_ERR) {
        snprintf(str, len, "%s", "ERROR");
    } else if (level == TSAR_FATAL) {
        snprintf(str, len, "%s", "FATAL");
    } else {
        snprintf(str, len, "%s", "WARN");
    }
}

int main(int argc, char **argv)
{
    struct MsopCfgStr cfg;
    char strMode[32] = {0};
    char strLevel[32] = {0};

    memset(&cfg, 0, sizeof(struct MsopCfgStr));
    cfg.level = TSAR_WARN;
    msop_parse_cfg(&cfg);
    msop_parse_argc(argc, argv, &cfg);
    get_msop_mode_str(cfg.mode, strMode, sizeof(strMode));
    get_msop_level_str(cfg.level, strLevel, sizeof(strLevel));
    printf("msop run mode:%s  level:%s  logfile:%s datafile:%s  stdout:%s\n", strMode, strLevel, cfg.logFileOut?"on":"off",
        cfg.tsarFileOut?"on":"off", cfg.logStdout?"on":"off");
    set_msop_signal();
    switch(cfg.mode) {
        case MSOP_IDEL:
        case MSOP_LIVE:
            if (mu_proc_exist(MSOP_PID_PATH)) {
                printf("msop already running.\n");
                break;
            }
            ms_save_proc_id(MSOP_PID_PATH);
            msop_run(&cfg);
            break;
            
        case MSOP_LOG:
            msop_log(&cfg);
            break;
    }
    
    return 0;
}
