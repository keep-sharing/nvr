#include <stdarg.h>
#include <limits.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <glob.h>
#include <locale.h>
#include <sys/types.h>
#include <utime.h>
#include <stdint.h>
#include <libgen.h>
#include <string.h>
#include <dirent.h>
#include "logrotate.h"
#include "msstd.h"

struct LogrotateLogState {
    int flag;
    char name[64];
    struct stat st;
};

struct LogrotateInfoStr {
    int debug;
    
    char *pattern;
    char dirPath[128];
    char fileName[64];

    int rotateCount;
    int minSize;

    /*current file*/
    struct LogrotateLogState targetState;

    /*bak file*/
    int bakFileCount;
    struct LogrotateLogState *bakFileState;

    time_t secNow;
    struct tm tmNow;
};


static struct LogrotateInfoStr g_logrotateConf;

static void logrotate_debug(const char *format, ...)
{
    va_list args;

    if (!g_logrotateConf.debug) {
        return;
    }
    
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}

static int rotate_filter(const struct dirent *pDir)
{
    char tmp[32] = {0};
    
    snprintf(tmp, sizeof(tmp), "%s.", g_logrotateConf.fileName);
    if (strstr(pDir->d_name, tmp)) {
        return 1;
    }

    return 0;
}

static int rotate_compar(const struct dirent **a, const struct dirent **b)
{
    return alphasort(a, b);
}

int get_file_state(struct LogrotateInfoStr *log)
{
    int offset = 0;
    int sum = 0;
    struct dirent **ents;
    char tmp[32] = {0};
    struct LogrotateLogState *logState;

    logState = &log->targetState;
    if (!ms_file_existed(logState->name)) {
        return -1;
    }
    if (lstat(logState->name, &logState->st)) {
        return -1;
    }
    time_to_string(logState->st.st_mtime, tmp);
    sum = scandir(log->dirPath, &ents, rotate_filter, rotate_compar);
    if (sum < 0) {
        return 0;
    }
    log->bakFileCount = sum;
    if (log->bakFileCount > 0) {
        log->bakFileState = ms_malloc(sizeof(struct LogrotateLogState) * log->bakFileCount);
    }
    if (sum != -1) {
        while (offset < sum) {
            if (log->bakFileState) {
                logState = &log->bakFileState[offset];
                snprintf(logState->name, sizeof(logState->name), "%s%s", log->dirPath, ents[offset]->d_name);
                if (lstat(logState->name, &logState->st)) {
                    logState->flag = 0;
                } else {
                    logState->flag = 1;
                }
            }
            free(ents[offset]);
            offset++;
        }
        free(ents);
    }

    return 0;
}

int parse_config_paths(char *pattern, char *dirPath, int dirLen, char *fileName, int fileLen)
{
    char *p, *tmp;
    int ret = -1;
    int cnt = 0;
    
    tmp = pattern;
    do {
        p = strchr(tmp, '/');
        if (p) {
            tmp = p+1;
        } else {
            if (tmp == pattern) {
                logrotate_debug("file path invalid. %s\n", pattern);
            } else {
                ret = 0;
                snprintf(dirPath, (int)(tmp-pattern)+1, "%s", pattern);
                snprintf(fileName, fileLen, "%s", tmp);
            }
            break;
        }

        if(cnt++ >= 10) {
            ret = -1;
            break;
        }
    
    } while (1);

    //logrotate_debug("patten:%s ## dir:%s ## filename:%s\n", pattern, dirPath, fileName);
    return ret;
}

int do_rotate(struct LogrotateInfoStr *log)
{
    int delNum = -1;
    char fileName[64] = {0};
    FILE *fdOld;
    FILE *fdNew;
    char *buf = NULL;
    struct stat fsize;
    int ret;

    if (log->targetState.st.st_size == 0) {
        logrotate_debug("   log does not need rotating (log is empty).\n");
        return 0;    
    }

    if (log->targetState.st.st_size < log->minSize) {
        logrotate_debug("   log does not need rotating ('minsize' directive is used and the log size is smaller than the minsize value)\n");
        return 0;    
    }

    if (log->bakFileCount >= log->rotateCount) {
        delNum = 0;//已经升序排序,删除第一个
    }
    snprintf(fileName, sizeof(fileName), "%s%s.%04d%02d%02d%02d%02d%02d", log->dirPath, log->fileName,
             g_logrotateConf.tmNow.tm_year + 1900, g_logrotateConf.tmNow.tm_mon + 1, g_logrotateConf.tmNow.tm_mday,
             g_logrotateConf.tmNow.tm_hour, g_logrotateConf.tmNow.tm_min, g_logrotateConf.tmNow.tm_sec);
    logrotate_debug("   log needs rotating\n");
    logrotate_debug("   renaming %s to %s\n", log->targetState.name, fileName);
    if (delNum != -1) {
        logrotate_debug("   removing old log %s\n", log->bakFileState[delNum].name);
    }
    fdOld = fopen(log->targetState.name, "r");
    if (!fdOld) {
        logrotate_debug("fopen current file(r) %s failed. errno:%d\n", log->targetState.name, errno);
        return -1;
    }

    fdNew = fopen(fileName, "w+");
    if (!fdNew) {
        logrotate_debug("fopen new file(w+) %s failed. errno:%d\n", fileName, errno);
        fclose(fdOld);
        return -1;
    }

    if (fstat(fileno(fdOld), &fsize) != 0 || fsize.st_size <= 0) {
        fclose(fdOld);
        fclose(fdNew);
        return -1;
    }
    
    ret = -1;
    buf = ms_malloc(fsize.st_size);
    if (buf) {
        if (fread(buf, fsize.st_size, 1, fdOld) == 1) {
            if (fwrite(buf, fsize.st_size, 1, fdNew) == 1) {
                ret = 0;
            } else {
                logrotate_debug("fwrite failed. size:%ld errno:%d\n", fsize.st_size, errno);
            }
        } else {
            logrotate_debug("fread failed. size:%d errno:%d\n", fsize.st_size, errno);
        }
    } else {
        logrotate_debug("malloc failed. size:%d\n", fsize.st_size);
    }

    if (buf) {
        ms_free(buf);
    }

    if (fdOld) {
        fclose(fdOld);
    }

    if (fdNew) {
        fclose(fdNew);
    }

    if (!ret) {
        if (delNum != -1) {
            unlink(log->bakFileState[delNum].name);
        }

        truncate(log->targetState.name, 0);
    }

    return 0;
}

int logrotate_file(char *pattern, int rotateCount, int minSize, int debug)
{
    char sTime[32] = {0};

    if (!pattern) {
        return -1;
    }

    memset(&g_logrotateConf, 0, sizeof(struct LogrotateInfoStr));
    g_logrotateConf.debug = debug;
    g_logrotateConf.pattern = pattern;
    g_logrotateConf.rotateCount = rotateCount;
    g_logrotateConf.minSize = minSize;
    snprintf(g_logrotateConf.targetState.name, sizeof(g_logrotateConf.targetState.name), "%s", pattern);

    logrotate_debug("\nrotating pattern: %s rotate count is %d\n", g_logrotateConf.targetState.name, g_logrotateConf.rotateCount);
    if (parse_config_paths(g_logrotateConf.pattern, g_logrotateConf.dirPath, sizeof(g_logrotateConf.dirPath),
                           g_logrotateConf.fileName, sizeof(g_logrotateConf.fileName))) {
        logrotate_debug("parse_config_paths %s error.\n", pattern);
        return -1;
    }
    
    if (get_file_state(&g_logrotateConf)) {
        logrotate_debug("get_file_state failed.\n", pattern);
        if (g_logrotateConf.bakFileState) {
            ms_free(g_logrotateConf.bakFileState);
            g_logrotateConf.bakFileState = NULL;
        }
        return -1;
    }

    g_logrotateConf.secNow = time(NULL);
	localtime_r((long*)&g_logrotateConf.secNow, &g_logrotateConf.tmNow);
    logrotate_debug("only log files >= %d bytes are rotated\n", g_logrotateConf.minSize);
    logrotate_debug("considering log %s\n", g_logrotateConf.targetState.name);
    logrotate_debug("   Now: %04d-%02d-%02d %02d:%02d:%02d\n", g_logrotateConf.tmNow.tm_year + 1900,
            g_logrotateConf.tmNow.tm_mon + 1, g_logrotateConf.tmNow.tm_mday, g_logrotateConf.tmNow.tm_hour,
            g_logrotateConf.tmNow.tm_min, g_logrotateConf.tmNow.tm_sec);
    if (g_logrotateConf.bakFileCount) {
        time_to_string(g_logrotateConf.bakFileState[g_logrotateConf.bakFileCount-1].st.st_ctime, sTime);
        logrotate_debug("   Last rotated at %s\n", sTime);
    }
    
    return do_rotate(&g_logrotateConf);
}


