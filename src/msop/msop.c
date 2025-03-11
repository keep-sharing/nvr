#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "msop.h"
#include "msstd.h"
#include "list.h"
#include "msdefs.h"
#include "logrotate.h"


#define MSOP_NODE_MAX_SIZE  100*1024

/*
|time|module|level|file|function|line|info|\n|
2022-06-23 15:24:20 BOA  FATAL request.c uri_dispatch_req:1775 dispatch sdk get.record.format_info 
2022-06-23 15:24:20 CORE DEBUG mscore/sockserver.c thread_reqslist_task:1734 ====== IND_NO mscore recv begin
2022-06-23 15:24:20 CORE DEBUG mscore/sockserver.c thread_reqslist_task:1738 ====== IND_NO mscore recv end
*/
#define MSOP_PRINT_FORMAT   "%s %s %s %s %s:%d %s\n"

//顺序不能改，否则前后兼容有问题
enum {
    MSLOG_TIME,
    MSLOG_MODULE,
    MSLOG_LEVEL,
    MSLOG_FILENAME,
    MSLOG_FUNCNAME,
    MSLOG_FILELINE,
};

struct MsopHandle {
    struct MsopCfgStr conf;
    void *tsarSock;
    void *cacheList;
    int cacheFull;
    int fileLimit;
};

static int g_msopStop = 0;
static struct MsopCfgStr *g_msopConf = NULL;

static inline void msop_debug(TSAR_DEBUG_LEVEL level,
                              const char *timestamp,
                              const char *mod,
                              const char *debug,
                              const char *fileName,
                              const char *funcName,
                              int fileLine,
                              const char *data)
{
    if (level == TSAR_WARN) {
        printf("%s%s %s %s %s %s:%d %s%s\n", YELLOW, timestamp, mod, debug, fileName, funcName, fileLine, data, CLRCOLOR);
    } else if (level == TSAR_ERR || level == TSAR_FATAL) {
        printf("%s%s %s %s %s %s:%d %s%s\n", RED, timestamp, mod, debug, fileName, funcName, fileLine, data, CLRCOLOR);
    } else {
        printf("%s %s %s %s %s:%d %s\n", timestamp, mod, debug, fileName, funcName, fileLine, data);
    }
}


static void msop_node_free(COMM_LIST_NODE *node)
{
    if (node) {
        if (node->data) {
            ms_free(node->data);
        }
        ms_free(node);
    }
    
    return ;
}

static void msop_get_mod_str(int mod, char *str, int len)
{
    switch (mod) {
        case TSAR_SOCK_MSOP:
            snprintf(str, len, "%s", "MSOP");
            break;
        
        case TSAR_SOCK_MSCORE:
            snprintf(str, len, "%s", "CORE");
            break;
        
        case TSAR_SOCK_BOA:
            snprintf(str, len, "%s", "BOA ");
            break;
        
        default:
            snprintf(str, len, "%d", mod);
            break;
    }
}

static void msop_get_level_str(TSAR_DEBUG_LEVEL level, char *str, int len)
{
    switch (level) {
        case TSAR_INFO:
            snprintf(str, len, "%s", "INFO ");
            break;
        
        case TSAR_DEBUG:
            snprintf(str, len, "%s", "DEBUG");
            break;
        
        case TSAR_WARN:
            snprintf(str, len, "%s", "WARN ");
            break;
        
        case TSAR_ERR:
            snprintf(str, len, "%s", "ERROR");
            break;
        
        case TSAR_FATAL:
            snprintf(str, len, "%s", "FATAL");
            break;
        
        default:
            snprintf(str, len, "%d", level);
            break;
    }
}

static void msop_output_print(TSAR_SOCKET_HEADER *header, const char *data, int size)
{
    char mod[32] = {0};
    char level[32] = {0};
    char timestamp[32] = {0};

    time_to_string(header->timestamp, timestamp);
    msop_get_mod_str(header->from, mod, sizeof(mod));
    msop_get_level_str(header->level, level, sizeof(level));
    msop_debug(header->level, timestamp, mod, level, header->fileName, header->funcName, header->fileLine, data);
}

static int msop_output_file(FILE *fp, TSAR_SOCKET_HEADER *header, const char *data, int size)
{
    char line[1024] = {0};
    char timestamp[32] = {0};

    if (!fp) {
        return -1;
    }

    time_to_string(header->timestamp, timestamp);
    snprintf(line, sizeof(line), "%s|%d|%d|%s|%s|%d|%s\n", timestamp, header->from, header->level, header->fileName, header->funcName, header->fileLine, data);
    fputs(line, fp);
    return 0;
}

static void deal_msop_recv_node(struct MsopHandle *msop)
{
    COMM_LIST_NODE *node;
    TSAR_SOCKET_HEADER *header;
    char *data;
    FILE *fp = NULL;
    struct stat fsize;
    unsigned int writeSize = 0;
    unsigned int space = msop->conf.tsarLogSize;

    do {
        node = popfront_common_list_node(msop->cacheList);
        if (!node) {
            break;
        }

        if (!msop->conf.logFileOut) {
            msop_node_free(node);
            continue;
        }
        header = (TSAR_SOCKET_HEADER *)node->data;
        data = node->data+sizeof(TSAR_SOCKET_HEADER);
        if (!fp) {
            fp = fopen(msop->conf.logPath, "a");
            if (fstat(fileno(fp), &fsize) == 0) {
                if (fsize.st_size > msop->conf.tsarLogSize) {
                    space = 0;
                } else {
                    space = msop->conf.tsarLogSize - fsize.st_size;
                }
            }
        }
        writeSize += node->size;
        msop_output_file(fp, header, data, header->size);
        msop_node_free(node);
        if (writeSize >= space) {
            msop->fileLimit = 1;
            break;
        }

    } while(1);

    if (fp) {
        fclose(fp);
    }
    return;    
}

static void msop_server_recv_callback(void *arg, TSAR_SOCKET_HEADER *header, void *data)
{
    COMM_LIST_NODE *node;
    struct MsopHandle *msop = (struct MsopHandle *)arg;

    if (!msop) {
        return;
    }

    if (header->level < msop->conf.level) {
        return;
    }

    if (msop->conf.mode == MSOP_LIVE) {
        msop_output_print(header, data, header->size);
    }

    if (!msop->conf.logFileOut) {
        return;
    }
        
    node = ms_malloc(sizeof(COMM_LIST_NODE));
    if (!node) {
        return;
    }
    node->size = sizeof(TSAR_SOCKET_HEADER) + header->size;
    node->data = ms_malloc(node->size);
    if (!node->data) {
        ms_free(node);
        return;
    }
    memcpy(node->data, header, sizeof(TSAR_SOCKET_HEADER));
    memcpy(node->data+sizeof(TSAR_SOCKET_HEADER), data, header->size);
    if (insert_common_list_node(msop->cacheList, node) != 0) {
        //full
        msop->cacheFull = 1;
        msop_node_free(node);
    }
}

static struct MsopHandle *creat_msop(struct MsopCfgStr *conf)
{
    struct MsopHandle *msop = NULL;

    msop = ms_malloc(sizeof(struct MsopHandle));
    if (!msop) {
        return NULL;
    }

    memcpy(&msop->conf, conf, sizeof(struct MsopCfgStr));

    //socket
    msop->tsarSock = creat_tsar_socket(TSAR_SOCK_MSOP,conf->level, msop_server_recv_callback, msop);
    if (!msop->tsarSock) {
        ms_free(msop);
        return NULL;
    }

    //list
    msop->cacheList = creat_common_list(MSOP_NODE_MAX_SIZE);
    if (!msop->cacheList) {
        destroy_tsar_socket(msop->tsarSock);
        ms_free(msop);
        return NULL;
    }

    return msop;
}

static void destroy_msop(struct MsopHandle *msop)
{
    if (!msop) {
        return;
    }
    if (msop->tsarSock) {
        destroy_tsar_socket(msop->tsarSock);
    }
    if (msop->cacheList) {
        destroy_common_list(msop->cacheList);
    }
    ms_free(msop);
    return;
}

void msop_run(struct MsopCfgStr *conf)
{
    int cnt = 0;
    struct MsopHandle *msop;

    msop = creat_msop(conf);
    if (!msop) {
        printf("creat msop failed.\n");
        return;    
    }

    while (1) {
        if (g_msopStop == 1) {
            break;
        }

        if (msop->conf.tsarFileOut && cnt % 5 == 0) {
            if (mu_proc_exist(MS_CORE_PID)) {
                system("tsar --cron");
            }
        }

        if (cnt % 5 == 0 || msop->cacheFull) {
            //per 5 sec
            deal_msop_recv_node(msop);
            msop->cacheFull = 0;
        }

        if (cnt % 120 == 0 || msop->fileLimit) {
            //logrotate per hour
            logrotate_file(conf->logPath, conf->tsarLogCnt, conf->tsarLogSize, conf->logrotateDebug);
            logrotate_file(conf->dataPath, conf->tsarDataCnt, conf->tsarDataSize, conf->logrotateDebug);
            msop->fileLimit = 0;
        }
        cnt++;
        usleep(1000*1000);
    }

    deal_msop_recv_node(msop);
    destroy_msop(msop);
    return;
}

void msop_stop()
{
    g_msopStop = 1;
}

static int msop_filter(const struct dirent *pDir)
{
    if (strstr(pDir->d_name, g_msopConf->filterName)) {
        return 1;
    }

    return 0;
}

static int msop_compar(const struct dirent **a, const struct dirent **b)
{
    return alphasort(a, b);
}

static int msop_show_msg(struct MsopCfgStr *conf, const char *path)
{
    FILE *fp;
    char line[2048] = {0};
    char *sToken;
    int item;
    char timestamp[32] = {0};
    int modValue;
    char mod[32] = {0};
    int levelValue;
    char level[32] = {0};
    char fileName[64] = {0};
    char funcName[64] = {0};
    int fileLine;
    char tmp[1024] = {0};
    char *savePtr = NULL;
    char *innerPtr;

    printf("msop_show_msg:%s\n", path);
    fp = fopen(path, "r");
    if (!fp) {
        return -1;
    }

    while (1) {
        if (!fgets(line, sizeof(line), fp)) {
            break;
        }
        //printf("%s", line);
        line[strlen(line) - 1] = '\0';
        item = 0;
        innerPtr = line;
        levelValue = 0;
        memset(tmp, 0, sizeof(tmp));
        while((sToken = strtok_r(innerPtr, "|", &savePtr)) != NULL) {
            innerPtr = NULL;
            if (item == MSLOG_TIME) {
                snprintf(timestamp, sizeof(timestamp), "%s", sToken);
            } else if (item == MSLOG_MODULE) {
                modValue = atoi(sToken);
            } else if (item == MSLOG_LEVEL) {
                levelValue = atoi(sToken);
                if (levelValue < conf->level) {
                    break;
                }
            } else if (item == MSLOG_FILENAME) {
                snprintf(fileName, sizeof(fileName), "%s", sToken);
            } else if (item == MSLOG_FUNCNAME) {
                snprintf(funcName, sizeof(funcName), "%s", sToken);
            } else if (item == MSLOG_FILELINE) {
                fileLine = atoi(sToken);
            } else {
                snprintf(tmp + strlen(tmp), sizeof(tmp) - strlen(tmp), "%s", sToken);
                msop_get_mod_str(modValue, mod, sizeof(mod));
                msop_get_level_str(levelValue, level, sizeof(level));
                msop_debug(levelValue, timestamp, mod, level, fileName, funcName, fileLine, sToken);
                break;
            }
            item++;
        }

        if (item <= MSLOG_FILELINE) {
            printf("%s\n", line);
        }
    }

    fclose(fp);
    return 0;
}

void msop_log(struct MsopCfgStr *conf)
{
    int offset = 0;
    int sum = 0;
    struct dirent **ents;
    char fileName[128] = {0};

    g_msopConf = conf;
    if (parse_config_paths(conf->logPath, conf->dirPath, sizeof(conf->dirPath), conf->fileName, sizeof(conf->fileName))) {
        printf("path invalid. %s", conf->logPath);
        return;
    }

    snprintf(conf->filterName, sizeof(conf->filterName), "%s%s", conf->fileName, ".");
    sum = scandir(conf->dirPath, &ents, msop_filter, msop_compar);
    if (sum < 0) {
        printf("not file exist.\n");
        return;
    }

    if (sum != -1) {
        while (offset < sum) {
            snprintf(fileName, sizeof(fileName), "%s%s", conf->dirPath, ents[offset]->d_name);
            msop_show_msg(conf, fileName);
            ms_free(ents[offset]);
            offset++;
        }
        ms_free(ents);
    }

    msop_show_msg(conf, conf->logPath);
    return;
}

