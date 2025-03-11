#include "msdb_utils.h"
#include "msdefs.h"

hook_print global_debug_hook = 0;
pthread_rwlock_t global_rwlock = PTHREAD_RWLOCK_INITIALIZER;
pthread_rwlock_t global_pcnt_rwlock = PTHREAD_RWLOCK_INITIALIZER;
pthread_rwlock_t g_pushmsgRwlock = PTHREAD_RWLOCK_INITIALIZER;

int globla_msdb_debug = 0;
unsigned int global_debug_mask = 0;

int MsFileLock(const char *pFilePath, int mode, pthread_rwlock_t *lock,  const char *file, int lineno)
{
    if (globla_msdb_debug) {
        msprintf("db file lock %s start", file);
    }

    if (!pFilePath) {
        system("echo \"===FileLock Faild===00===\" >> /mnt/nand/err_log.txt");
        return -1;
    }

    int nFd = open(pFilePath, O_CREAT | O_RDWR | O_CLOEXEC | O_TRUNC);
    if (-1 == nFd) {
        system("echo \"===FileLock Faild===11===\" >> /mnt/nand/err_log.txt");
        return -1;
    }

    if (mode == 'r') {
        pthread_rwlock_rdlock(lock);
    } else {
        pthread_rwlock_wrlock(lock);
    }
    lockf(nFd, F_LOCK, 0);

    if (globla_msdb_debug) {
        msprintf("db file lock %s %d end", file, nFd);
    }

    return nFd;
}

void MsFileUnlock(int nFd, int mode, pthread_rwlock_t *lock,  const char *file, int lineno)
{
    if (globla_msdb_debug) {
        msprintf("db file unlock %s %d start", file, nFd);
    }

    if (nFd <= -1) {
        return ;
    }
    lockf(nFd, F_ULOCK, 0);
    pthread_rwlock_unlock(lock);

    close(nFd);

    if (globla_msdb_debug) {
        msprintf("db file unlock %s %d end", file, nFd);
    }
}

int ms_get_executable_name(char *pNanme, int size)
{
    int len = 0;
    char *ptr = NULL;
    char pBuff[MAX_LEN_1024] = {0};

    if (!pNanme || size <= 0) {
        msprintf("[david debug] get executable params is err!");
        return -1;
    }

    len = readlink("/proc/self/exe", pBuff, sizeof(pBuff) - 1);
    if (len <= 0) {
        msprintf("[david debug] get executable name failed!");
        return -1;
    }
    ptr = strrchr(pBuff,  '/');
    if (ptr == NULL) {
        msprintf("[david debug] get executable buff NULL!");
        return -1;
    }
    ++ptr;
    snprintf(pNanme, size, "%s", ptr);

    //msprintf("processname:%s success.", pNanme);
    return 0;
}

void get_event_type(char *event_type, int event)
{
    int length = 32;
    switch (event) {
        case PUSH_MOTION:
            snprintf(event_type, length, "Motion Detection");
            break;
        case PUSH_VIDEOLOSS:
            snprintf(event_type, length, "Video Loss");
            break;
        case PUSH_REGIONIN:
            snprintf(event_type, length, "Region Entrance");
            break;
        case PUSH_REGIONOUT:
            snprintf(event_type, length, "Region Exiting");
            break;
        case PUSH_ADVANCED_MOTION:
            snprintf(event_type, length, "Advanced Motion Detection");
            break;
        case PUSH_TAMPER:
            snprintf(event_type, length, "Tamper Detection");
            break;
        case PUSH_LINECROSS:
            snprintf(event_type, length, "Line Crossing");
            break;
        case PUSH_LOITERING:
            snprintf(event_type, length, "Loitering");
            break;
        case PUSH_HUMAN:
            snprintf(event_type, length, "Human Detection");
            break;
        case PUSH_OBJECT_LEFTREMOVE:
            snprintf(event_type, length, "Object Left/Removed");
            break;
        case PUSH_IPC_ALARMIN1:
        case PUSH_IPC_ALARMIN2:
        case PUSH_IPC_ALARMIN3:
        case PUSH_IPC_ALARMIN4:
            snprintf(event_type, length, "Camera Alarm Input");
            break;
        case PUSH_ANPR_BLACK:
            snprintf(event_type, length, "ANPR Black List");
            break;
        case PUSH_ANPR_WHITE:
            snprintf(event_type, length, "ANPR White List");
            break;
        case PUSH_ANPR_VISITOR:
            snprintf(event_type, length, "ANPR Visitor List");
            break;
        case PUSH_NVR_ALARMIN:
            snprintf(event_type, length, "Alarm Input");
            break;
        case PUSH_NVR_POS:
            snprintf(event_type, length, "POS");
            break;
        case PUSH_FACE:
            snprintf(event_type, length, "Face Detection");
            break;
        case PUSH_AUDIO_ALARM:
            snprintf(event_type, length, "Audio Alarm");
            break;
        case PUSH_PCNT:
            snprintf(event_type, length, "People Counting");
            break;
        case PUSH_REGION_PCNT:
            snprintf(event_type, length, "Regional People Counting");
            break;
        default :
            snprintf(event_type, length, "Unknown Event");
            break;
    }
}

EVENT_IN_TYPE_E converse_chn_alarmid_to_event_in(int alarmId)
{
    if (alarmId == 0) {
        return ALARM_CHN_IN0_EVT;
    } else if (alarmId == 1) {
        return ALARM_CHN_IN1_EVT;
    } else if (alarmId == 2) {
        return ALARM_CHN_IN2_EVT;
    } else if (alarmId == 3) {
        return ALARM_CHN_IN3_EVT;
    }

    return -1;
}

int converse_event_in_to_chn_alarmid(EVENT_IN_TYPE_E event)
{
    if (event == ALARM_CHN_IN0_EVT) {
        return 0;
    } else if (event == ALARM_CHN_IN1_EVT) {
        return 1;
    } else if (event == ALARM_CHN_IN2_EVT) {
        return 2;
    } else if (event == ALARM_CHN_IN3_EVT) {
        return 3;
    }

    return -1;
}

int get_exception_name(int exceptId, char *exceptName, int len)
{
    if (exceptId < 0 || exceptId >= EXCEPT_COUNT || !exceptName || len <= 0) {
        msdebug(DEBUG_INF, "get exception name err, exceptId = %d, exceptName = %p, len = %d", exceptId, exceptName, len);
        return -1;
    }

    switch (exceptId) {
        case EXCEPT_NETWORK_DISCONN: {
            snprintf(exceptName, len, "networkDisconn");
            break;
        }
        case EXCEPT_DISK_FULL: {
            snprintf(exceptName, len, "diskFull");
            break;
        }
        case EXCEPT_RECORD_FAIL: {
            snprintf(exceptName, len, "recordFail");
            break;
        }
        case EXCEPT_DISK_FAIL: {
            snprintf(exceptName, len, "diskErr");
            break;
        }
        case EXCEPT_DISK_NO_FORMAT: {
            snprintf(exceptName, len, "diskUninit");
            break;
        }
        case EXCEPT_NO_DISK: {
            snprintf(exceptName, len, "noDisk");
            break;
        }
        case EXCEPT_IP_CONFLICT: {
            snprintf(exceptName, len, "ipConfilct");
            break;
        }
        case EXCEPT_DISK_OFFLINE: {
            snprintf(exceptName, len, "diskOffline");
            break;
        }
        case EXCEPT_DISK_HEAT: {
            snprintf(exceptName, len, "diskHeat");
            break;
        }
        case EXCEPT_DISK_MICROTHERM: {
            snprintf(exceptName, len, "diskMicrotherm");
            break;
        }
        case EXCEPT_DISK_CONNECTION_EXCEPTION: {
            snprintf(exceptName, len, "diskConnectionException");
            break;
        }
        case EXCEPT_DISK_STRIKE: {
            snprintf(exceptName, len, "diskStrike");
            break;
        }
        default: {
            exceptName[0] = '\0';
            break;
        }
    }

    return 0;
}

Uint64 get_channel_mask(char *copyChn)
{
    int i = 0;
    Uint64 ret = 0;

    if (copyChn) {
        for (i = 0; i < MAX_REAL_CAMERA; i++) {
            if (copyChn[i] == '1') {
                ret |= (Uint64)1 << i;
            }
        }
    }

    return ret;
}

SMART_EVENT_TYPE converse_vca_to_smart_event(VCA_ALARM_EVENT type)
{
    switch (type) {
        case VCA_REGIONIN:
            return REGIONIN;
        case VCA_REGIONOUT:
            return REGIONOUT;
        case VCA_ADVANCED_MOTION:
            return ADVANCED_MOTION;
        case VCA_TAMPER:
            return TAMPER;
        case VCA_LINECROSS:
            return LINECROSS;
        case VCA_LOITERING:
            return LOITERING;
        case VCA_HUMAN:
            return HUMAN;
        case VCA_PEOPLECNT:
            return PEOPLE_CNT;
        case VCA_OBJECT_LEFT:
        case VCA_OBJECT_REMOVE:
            return OBJECT_LEFTREMOVE;
        default:
            msdebug(DEBUG_INF, "invalid type:%d", type);
            return MAX_SMART_EVENT;
    }
}


/////////////////////////////////////////
int read_record_schedule_init(const char *pDbFile, struct record_schedule *schedule, int chnCnt)
{
    if (!pDbFile || !schedule) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[256] = {0};
    int nColumnCnt = 0;
    int i = 0, chn_id = 0;

    do {
        snprintf(sQuery, sizeof(sQuery),
                 "select enable,wday_id,wholeday_enable,wholeday_action_type,plan_id,start_time,end_time,action_type from record_schedule where chn_id=%d;",
                 chn_id);
        int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            sqlite_clear_stmt(hStmt);
            continue;
        }
        int nWeekId = 1;
        int nPlanId = 0;
        int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM;
        for (i = 0; i <= nLoopCnt; i++) {
            sqlite_query_column(hStmt, 0, &schedule[chn_id].enable);
            sqlite_query_column(hStmt, 1, &nWeekId);
            sqlite_query_column(hStmt, 2, &schedule[chn_id].schedule_day[nWeekId].wholeday_enable);
            sqlite_query_column(hStmt, 3, &schedule[chn_id].schedule_day[nWeekId].wholeday_action_type);
            sqlite_query_column(hStmt, 4, &nPlanId);
            sqlite_query_column_text(hStmt, 5, schedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(schedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 6, schedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(schedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 7, &(schedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].action_type));
            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }
        sqlite_clear_stmt(hStmt);
    } while (++chn_id < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_ptz_params_init(const char *pDbFile, struct ptz_action_params *ptzActionParams[], int type, int chnCnt)
{
    if (!pDbFile || type < 0) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[2048] = {0};
    int nColumnCnt = 0;
    int i = 0, chn_id = 0;

    do {
        switch (type) {
            case ALARMIO:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from alarmin_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case MOTION:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from motion_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case VIDEOLOSS:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from videoloss_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case REGION_EN:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from vca_regionein_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case REGION_EXIT:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from vca_regionexit_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case ADVANCED_MOT:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from vca_motion_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case TAMPER_DET:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from vca_tamper_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case LINE_CROSS:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from vca_linecross_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case LOITER:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from vca_loiter_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case HUMAN_DET:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from vca_human_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case PEOPLE_COUNT:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from vca_people_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case LEFTREMOVE:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from vca_object_leftremove_ptz_params where chn_id=%d;",
                         chn_id);
                break;

            case POS_EVT:
                snprintf(sQuery, sizeof(sQuery),
                         "select id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from pos_ptz_params where id=%d;",
                         chn_id);
                break;

            case REGIONAL_PEOPLE_CNT0:
                snprintf(sQuery, sizeof(sQuery),
                         "select id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from regional_pcnt_ptz_params where id=%d;",
                         chn_id);
                break;

            case FACE_EVT:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from face_ptz_params where chn_id=%d;",
                         chn_id);
                break;
                
            case AUDIO_ALARM:
                snprintf(sQuery, sizeof(sQuery),
                    "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,\
                    acto_ptz_pattern from audio_alarm_ptz_params where chn_id=%d;", chn_id);
                break;
                    
            default:
                break;
        }
        if (sQuery[0] == '\0') {
            sqlite_disconn(hConn);
            FileUnlock(nFd, mode, &global_rwlock);
            return -1;
        }
        int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            continue;
        }

        for (i = 0; i < nColumnCnt && i < MAX_REAL_CAMERA; i++) {
            sqlite_query_column(hStmt, 0, &ptzActionParams[chn_id][i].chn_id);

            sqlite_query_column(hStmt, 1, &ptzActionParams[chn_id][i].acto_ptz_channel);
            sqlite_query_column(hStmt, 2, &ptzActionParams[chn_id][i].acto_fish_channel);
            sqlite_query_column(hStmt, 3, &ptzActionParams[chn_id][i].acto_ptz_type);
            sqlite_query_column(hStmt, 4, &ptzActionParams[chn_id][i].acto_ptz_preset);
            sqlite_query_column(hStmt, 5, &ptzActionParams[chn_id][i].acto_ptz_patrol);
            sqlite_query_column(hStmt, 6, &ptzActionParams[chn_id][i].acto_ptz_pattern);
            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }
        sqlite_clear_stmt(hStmt);
    } while (++chn_id < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_videoloss_audible_schedule_init(const char *pDbFile, void *params, int chnCnt)
{
    if (!pDbFile || !params) {
        return -1;
    }
    struct video_loss_schedule *vSchedule = (struct video_loss_schedule *)params;
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[256] = {0};
    int nColumnCnt = 0;
    int i = 0, chn_id = 0;

    do {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from videoloss_audible_schedule where chn_id=%d;", chn_id);
        int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            continue;
        }
        int nWeekId = 1;
        int nPlanId = 0;
        int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
        for (i = 0; i <= nLoopCnt; i++) {
            sqlite_query_column(hStmt, 0, &nWeekId);
            sqlite_query_column(hStmt, 1, &nPlanId);
            sqlite_query_column_text(hStmt, 2, vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].action_type));
            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }
        sqlite_clear_stmt(hStmt);
    } while (++chn_id < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_videoloss_email_schedule_init(const char *pDbFile, void *params, int chnCnt)
{
    if (!pDbFile || !params) {
        return -1;
    }
    struct video_loss_schedule *vSchedule = (struct video_loss_schedule *)params;
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[256] = {0};
    int nColumnCnt = 0;
    int i = 0, chn_id = 0;

    do {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from videoloss_email_schedule where chn_id=%d;", chn_id);
        int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            continue;
        }
        int nWeekId = 1;
        int nPlanId = 0;
        int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
        for (i = 0; i <= nLoopCnt; i++) {
            sqlite_query_column(hStmt, 0, &nWeekId);
            sqlite_query_column(hStmt, 1, &nPlanId);
            sqlite_query_column_text(hStmt, 2, vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].action_type));
            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }
        sqlite_clear_stmt(hStmt);
    } while (++chn_id < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_videoloss_popup_schedule_init(const char *pDbFile, void *params, int chnCnt)
{
    if (!pDbFile || !params) {
        return -1;
    }
    struct video_loss_schedule *vSchedule = (struct video_loss_schedule *)params;
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[256] = {0};
    int nColumnCnt = 0;
    int i = 0, chn_id = 0;

    do {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from videoloss_popup_schedule where chn_id=%d;", chn_id);
        int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            continue;
        }
        int nWeekId = 1;
        int nPlanId = 0;
        int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
        for (i = 0; i <= nLoopCnt; i++) {
            sqlite_query_column(hStmt, 0, &nWeekId);
            sqlite_query_column(hStmt, 1, &nPlanId);
            sqlite_query_column_text(hStmt, 2, vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].action_type));
            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }
        sqlite_clear_stmt(hStmt);
    } while (++chn_id < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_videoloss_ptz_schedule_init(const char *pDbFile, void *params, int chnCnt)
{
    if (!pDbFile || !params) {
        return -1;
    }
    struct video_loss_schedule *vSchedule = (struct video_loss_schedule *)params;
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[256] = {0};
    int nColumnCnt = 0;
    int i = 0, chn_id = 0;

    do {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from videoloss_ptz_schedule where chn_id=%d;", chn_id);
        int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            continue;
        }
        int nWeekId = 1;
        int nPlanId = 0;
        int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
        for (i = 0; i <= nLoopCnt; i++) {
            sqlite_query_column(hStmt, 0, &nWeekId);
            sqlite_query_column(hStmt, 1, &nPlanId);
            sqlite_query_column_text(hStmt, 2, vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].action_type));
            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }
        sqlite_clear_stmt(hStmt);
    } while (++chn_id < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_motion_audible_schedule_init(const char *pDbFile, void *params, int chnCnt)
{
    if (!pDbFile || !params) {
        return -1;
    }
    struct motion_schedule *vSchedule = (struct motion_schedule *)params;
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[256] = {0};
    int nColumnCnt = 0;
    int i = 0, chn_id = 0;

    do {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from motion_audible_schedule where chn_id=%d;", chn_id);
        int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            continue;
        }
        int nWeekId = 1;
        int nPlanId = 0;
        int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
        for (i = 0; i <= nLoopCnt; i++) {
            sqlite_query_column(hStmt, 0, &nWeekId);
            sqlite_query_column(hStmt, 1, &nPlanId);
            sqlite_query_column_text(hStmt, 2, vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].action_type));
            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }
        sqlite_clear_stmt(hStmt);
    } while (++chn_id < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_motion_email_schedule_init(const char *pDbFile, void *params, int chnCnt)
{
    if (!pDbFile || !params) {
        return -1;
    }
    struct motion_schedule *vSchedule = (struct motion_schedule *)params;
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[256] = {0};
    int nColumnCnt = 0;
    int i = 0, chn_id = 0;

    do {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from motion_email_schedule where chn_id=%d;", chn_id);
        int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            continue;
        }
        int nWeekId = 1;
        int nPlanId = 0;
        int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
        for (i = 0; i <= nLoopCnt; i++) {
            sqlite_query_column(hStmt, 0, &nWeekId);
            sqlite_query_column(hStmt, 1, &nPlanId);
            sqlite_query_column_text(hStmt, 2, vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].action_type));
            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }
        sqlite_clear_stmt(hStmt);
    } while (++chn_id < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_motion_popup_schedule_init(const char *pDbFile, void *params, int chnCnt)
{
    if (!pDbFile || !params) {
        return -1;
    }
    struct motion_schedule *vSchedule = (struct motion_schedule *)params;
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[256] = {0};
    int nColumnCnt = 0;
    int i = 0, chn_id = 0;

    do {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from motion_popup_schedule where chn_id=%d;", chn_id);
        int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            continue;
        }
        int nWeekId = 1;
        int nPlanId = 0;
        int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
        for (i = 0; i <= nLoopCnt; i++) {
            sqlite_query_column(hStmt, 0, &nWeekId);
            sqlite_query_column(hStmt, 1, &nPlanId);
            sqlite_query_column_text(hStmt, 2, vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].action_type));
            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }
        sqlite_clear_stmt(hStmt);
    } while (++chn_id < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_motion_ptz_schedule_init(const char *pDbFile, void *params, int chnCnt)
{
    if (!pDbFile || !params) {
        return -1;
    }
    struct motion_schedule *vSchedule = (struct motion_schedule *)params;
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[256] = {0};
    int nColumnCnt = 0;
    int i = 0, chn_id = 0;

    do {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from motion_ptz_schedule where chn_id=%d;", chn_id);
        int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            continue;
        }
        int nWeekId = 1;
        int nPlanId = 0;
        int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
        for (i = 0; i <= nLoopCnt; i++) {
            sqlite_query_column(hStmt, 0, &nWeekId);
            sqlite_query_column(hStmt, 1, &nPlanId);
            sqlite_query_column_text(hStmt, 2, vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].action_type));
            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }
        sqlite_clear_stmt(hStmt);
    } while (++chn_id < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_motion_effective_schedule_init(const char *pDbFile, void *params, int chnCnt)
{
    if (!pDbFile || !params) {
        return -1;
    }
    struct smart_event_schedule *vSchedule = (struct smart_event_schedule *)params;
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[256] = {0};
    int nColumnCnt = 0;
    int i = 0, chn_id = 0;

    do {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from motion_effective_schedule where chn_id=%d;", chn_id);
        int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            continue;
        }
        int nWeekId = 1;
        int nPlanId = 0;
        int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
        for (i = 0; i <= nLoopCnt; i++) {
            sqlite_query_column(hStmt, 0, &nWeekId);
            sqlite_query_column(hStmt, 1, &nPlanId);
            sqlite_query_column_text(hStmt, 2, vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].action_type));
            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }
        sqlite_clear_stmt(hStmt);
    } while (++chn_id < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_smart_event_effective_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int chnCnt)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0, chn_id = 0;;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_SMART_EVENT][256];

    HSQLSTMT hStmt = 0;
    do {
        snprintf(sQuery[REGIONIN], sizeof(sQuery[REGIONIN]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_regionen_effective_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[REGIONOUT], sizeof(sQuery[REGIONOUT]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_regionex_effective_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[ADVANCED_MOTION], sizeof(sQuery[ADVANCED_MOTION]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_motionadv_effective_schedule where chn_id=%d;",
                 chn_id);
        snprintf(sQuery[TAMPER], sizeof(sQuery[TAMPER]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_tamper_effective_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[LINECROSS], sizeof(sQuery[LINECROSS]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_linecross_effective_schedule where chn_id=%d;",
                 chn_id);
        snprintf(sQuery[LOITERING], sizeof(sQuery[LOITERING]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_loiter_effective_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[HUMAN], sizeof(sQuery[HUMAN]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_human_effective_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[PEOPLE_CNT], sizeof(sQuery[PEOPLE_CNT]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_peolpe_effective_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[OBJECT_LEFTREMOVE], sizeof(sQuery[OBJECT_LEFTREMOVE]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_object_leftremove_effective_schedule where chn_id=%d;",
                 chn_id);
        for (iSmt = REGIONIN; iSmt < MAX_SMART_EVENT; iSmt++) {
            nResult = sqlite_query_record(hConn, sQuery[iSmt], &hStmt, &nColumnCnt);
            if (nResult != 0 || nColumnCnt == 0) {
                if (hStmt) {
                    sqlite_clear_stmt(hStmt);
                    hStmt = 0;
                }
                continue;
            }

            for (i = 0; i <= nLoopCnt; i++) {
                sqlite_query_column(hStmt, 0, &nWeekId);
                sqlite_query_column(hStmt, 1, &nPlanId);
                sqlite_query_column_text(hStmt, 2, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
                sqlite_query_column_text(hStmt, 3, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
                sqlite_query_column(hStmt, 4, &Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
                if (sqlite_query_next(hStmt) != 0) {
                    i++;
                    break;
                }
            }
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
        }

        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }
    } while (++chn_id < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_smart_event_audible_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int chnCnt)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0, chn_id = 0;;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_SMART_EVENT][256];

    HSQLSTMT hStmt = 0;
    do {
        snprintf(sQuery[REGIONIN], sizeof(sQuery[REGIONIN]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_regionen_audible_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[REGIONOUT], sizeof(sQuery[REGIONOUT]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_regionex_audible_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[ADVANCED_MOTION], sizeof(sQuery[ADVANCED_MOTION]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_motionadv_audible_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[TAMPER], sizeof(sQuery[TAMPER]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_tamper_audible_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[LINECROSS], sizeof(sQuery[LINECROSS]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_linecross_audible_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[LOITERING], sizeof(sQuery[LOITERING]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_loiter_audible_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[HUMAN], sizeof(sQuery[HUMAN]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_human_audible_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[PEOPLE_CNT], sizeof(sQuery[PEOPLE_CNT]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_peolpe_audible_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[OBJECT_LEFTREMOVE], sizeof(sQuery[OBJECT_LEFTREMOVE]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_object_leftremove_audible_schedule where chn_id=%d;",
                 chn_id);
        for (iSmt = REGIONIN; iSmt < MAX_SMART_EVENT; iSmt++) {
            nResult = sqlite_query_record(hConn, sQuery[iSmt], &hStmt, &nColumnCnt);
            if (nResult != 0 || nColumnCnt == 0) {
                if (hStmt) {
                    sqlite_clear_stmt(hStmt);
                    hStmt = 0;
                }
                continue;
            }

            for (i = 0; i <= nLoopCnt; i++) {
                sqlite_query_column(hStmt, 0, &nWeekId);
                sqlite_query_column(hStmt, 1, &nPlanId);
                sqlite_query_column_text(hStmt, 2, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
                sqlite_query_column_text(hStmt, 3, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
                sqlite_query_column(hStmt, 4, &Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
                if (sqlite_query_next(hStmt) != 0) {
                    i++;
                    break;
                }
            }
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
        }

        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }
    } while (++chn_id < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_smart_event_email_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int chnCnt)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0, chn_id = 0;;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_SMART_EVENT][256];

    HSQLSTMT hStmt = 0;
    do {
        snprintf(sQuery[REGIONIN], sizeof(sQuery[REGIONIN]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_regionen_mail_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[REGIONOUT], sizeof(sQuery[REGIONOUT]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_regionex_mail_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[ADVANCED_MOTION], sizeof(sQuery[ADVANCED_MOTION]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_motionadv_mail_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[TAMPER], sizeof(sQuery[TAMPER]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_tamper_mail_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[LINECROSS], sizeof(sQuery[LINECROSS]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_linecross_mail_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[LOITERING], sizeof(sQuery[LOITERING]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_loiter_mail_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[HUMAN], sizeof(sQuery[HUMAN]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_human_mail_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[PEOPLE_CNT], sizeof(sQuery[PEOPLE_CNT]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_peolpe_mail_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[OBJECT_LEFTREMOVE], sizeof(sQuery[OBJECT_LEFTREMOVE]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_object_leftremove_mail_schedule where chn_id=%d;",
                 chn_id);
        for (iSmt = REGIONIN; iSmt < MAX_SMART_EVENT; iSmt++) {
            nResult = sqlite_query_record(hConn, sQuery[iSmt], &hStmt, &nColumnCnt);
            if (nResult != 0 || nColumnCnt == 0) {
                if (hStmt) {
                    sqlite_clear_stmt(hStmt);
                    hStmt = 0;
                }
                continue;
            }

            for (i = 0; i <= nLoopCnt; i++) {
                sqlite_query_column(hStmt, 0, &nWeekId);
                sqlite_query_column(hStmt, 1, &nPlanId);
                sqlite_query_column_text(hStmt, 2, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
                sqlite_query_column_text(hStmt, 3, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
                sqlite_query_column(hStmt, 4, &Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
                if (sqlite_query_next(hStmt) != 0) {
                    i++;
                    break;
                }
            }
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
        }

        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }
    } while (++chn_id < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_smart_event_popup_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int chnCnt)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0, chn_id = 0;;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_SMART_EVENT][256];

    HSQLSTMT hStmt = 0;
    do {
        snprintf(sQuery[REGIONIN], sizeof(sQuery[REGIONIN]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_regionin_popup_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[REGIONOUT], sizeof(sQuery[REGIONOUT]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_regionexit_popup_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[ADVANCED_MOTION], sizeof(sQuery[ADVANCED_MOTION]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_motion_popup_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[TAMPER], sizeof(sQuery[TAMPER]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_tamper_popup_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[LINECROSS], sizeof(sQuery[LINECROSS]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_linecross_popup_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[LOITERING], sizeof(sQuery[LOITERING]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_loiter_popup_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[HUMAN], sizeof(sQuery[HUMAN]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_human_popup_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[PEOPLE_CNT], sizeof(sQuery[PEOPLE_CNT]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_people_popup_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[OBJECT_LEFTREMOVE], sizeof(sQuery[OBJECT_LEFTREMOVE]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_object_leftremove_popup_schedule where chn_id=%d;",
                 chn_id);
        for (iSmt = REGIONIN; iSmt < MAX_SMART_EVENT; iSmt++) {
            nResult = sqlite_query_record(hConn, sQuery[iSmt], &hStmt, &nColumnCnt);
            if (nResult != 0 || nColumnCnt == 0) {
                if (hStmt) {
                    sqlite_clear_stmt(hStmt);
                    hStmt = 0;
                }
                continue;
            }

            for (i = 0; i <= nLoopCnt; i++) {
                sqlite_query_column(hStmt, 0, &nWeekId);
                sqlite_query_column(hStmt, 1, &nPlanId);
                sqlite_query_column_text(hStmt, 2, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
                sqlite_query_column_text(hStmt, 3, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
                sqlite_query_column(hStmt, 4, &Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
                if (sqlite_query_next(hStmt) != 0) {
                    i++;
                    break;
                }
            }
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
        }

        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }
    } while (++chn_id < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_smart_event_ptz_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int chnCnt)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0, chn_id = 0;;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_SMART_EVENT][256];

    HSQLSTMT hStmt = 0;
    do {
        snprintf(sQuery[REGIONIN], sizeof(sQuery[REGIONIN]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_regionein_ptz_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[REGIONOUT], sizeof(sQuery[REGIONOUT]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_regioneexit_ptz_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[ADVANCED_MOTION], sizeof(sQuery[ADVANCED_MOTION]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_motion_ptz_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[TAMPER], sizeof(sQuery[TAMPER]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_tamper_ptz_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[LINECROSS], sizeof(sQuery[LINECROSS]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_linecross_ptz_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[LOITERING], sizeof(sQuery[LOITERING]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_loiter_ptz_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[HUMAN], sizeof(sQuery[HUMAN]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_human_ptz_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[PEOPLE_CNT], sizeof(sQuery[PEOPLE_CNT]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_people_ptz_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[OBJECT_LEFTREMOVE], sizeof(sQuery[OBJECT_LEFTREMOVE]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_object_leftremove_ptz_schedule where chn_id=%d;",
                 chn_id);
        for (iSmt = REGIONIN; iSmt < MAX_SMART_EVENT; iSmt++) {
            nResult = sqlite_query_record(hConn, sQuery[iSmt], &hStmt, &nColumnCnt);
            if (nResult != 0 || nColumnCnt == 0) {
                if (hStmt) {
                    sqlite_clear_stmt(hStmt);
                    hStmt = 0;
                }
                continue;
            }

            for (i = 0; i <= nLoopCnt; i++) {
                sqlite_query_column(hStmt, 0, &nWeekId);
                sqlite_query_column(hStmt, 1, &nPlanId);
                sqlite_query_column_text(hStmt, 2, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
                sqlite_query_column_text(hStmt, 3, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
                sqlite_query_column(hStmt, 4, &Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
                if (sqlite_query_next(hStmt) != 0) {
                    i++;
                    break;
                }
            }
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
        }

        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }
    } while (++chn_id < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_smart_event_led_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int chnCnt)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0, chn_id = 0;;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_SMART_EVENT][256];

    HSQLSTMT hStmt = 0;
    do {
        snprintf(sQuery[REGIONIN], sizeof(sQuery[REGIONIN]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_regionin_whiteled_esche where chn_id=%d;", chn_id);
        snprintf(sQuery[REGIONOUT], sizeof(sQuery[REGIONOUT]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_regionexit_whiteled_esche where chn_id=%d;", chn_id);
        snprintf(sQuery[ADVANCED_MOTION], sizeof(sQuery[ADVANCED_MOTION]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_motion_whiteled_esche where chn_id=%d;", chn_id);
        snprintf(sQuery[TAMPER], sizeof(sQuery[TAMPER]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_tamper_whiteled_esche where chn_id=%d;", chn_id);
        snprintf(sQuery[LINECROSS], sizeof(sQuery[LINECROSS]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_linecross_whiteled_esche where chn_id=%d;", chn_id);
        snprintf(sQuery[LOITERING], sizeof(sQuery[LOITERING]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_loiter_whiteled_esche where chn_id=%d;", chn_id);
        snprintf(sQuery[HUMAN], sizeof(sQuery[HUMAN]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_human_whiteled_esche where chn_id=%d;", chn_id);
        snprintf(sQuery[PEOPLE_CNT], sizeof(sQuery[PEOPLE_CNT]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_people_whiteled_esche where chn_id=%d;", chn_id);
        snprintf(sQuery[OBJECT_LEFTREMOVE], sizeof(sQuery[OBJECT_LEFTREMOVE]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_object_whiteled_esche where chn_id=%d;", chn_id);
        for (iSmt = REGIONIN; iSmt < MAX_SMART_EVENT; iSmt++) {
            nResult = sqlite_query_record(hConn, sQuery[iSmt], &hStmt, &nColumnCnt);
            if (nResult != 0 || nColumnCnt == 0) {
                if (hStmt) {
                    sqlite_clear_stmt(hStmt);
                    hStmt = 0;
                }
                continue;
            }

            for (i = 0; i <= nLoopCnt; i++) {
                sqlite_query_column(hStmt, 0, &nWeekId);
                sqlite_query_column(hStmt, 1, &nPlanId);
                sqlite_query_column_text(hStmt, 2, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
                sqlite_query_column_text(hStmt, 3, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
                sqlite_query_column(hStmt, 4, &Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
                if (sqlite_query_next(hStmt) != 0) {
                    i++;
                    break;
                }
            }
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
        }

        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }
    } while (++chn_id < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_smart_event_init(const char *pDbFile, struct smart_event *smartevent[], int chnCnt)
{
    if (!pDbFile || !smartevent) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int i = 0, ichn = 0;
    HSQLSTMT hStmt = 0;
    char sQuery[MAX_SMART_EVENT][512];
    char cmd[512] = {0};
    int nColumnCnt = 0;

    snprintf(sQuery[REGIONIN], sizeof(sQuery[REGIONIN]),
             "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,email_interval,popup_interval,ptzaction_interval,\
             alarmout_interval,whiteled_interval,email_pic_enable,tri_channels_pic,http_notification_interval,tri_audio_id from vca_regionin_event");
    snprintf(sQuery[REGIONOUT], sizeof(sQuery[REGIONOUT]),
             "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,email_interval,popup_interval,ptzaction_interval,\
		alarmout_interval,whiteled_interval,email_pic_enable,tri_channels_pic,http_notification_interval,tri_audio_id from vca_regionexit_event");
    snprintf(sQuery[ADVANCED_MOTION], sizeof(sQuery[ADVANCED_MOTION]),
             "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,email_interval,popup_interval,ptzaction_interval,\
		alarmout_interval,whiteled_interval,email_pic_enable,tri_channels_pic,http_notification_interval,tri_audio_id from vca_motion_event");
    snprintf(sQuery[TAMPER], sizeof(sQuery[TAMPER]),
             "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,email_interval,popup_interval,ptzaction_interval,\
		alarmout_interval,whiteled_interval,email_pic_enable,tri_channels_pic,http_notification_interval,tri_audio_id from vca_tamper_event");
    snprintf(sQuery[LINECROSS], sizeof(sQuery[LINECROSS]),
             "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,email_interval,popup_interval,ptzaction_interval,\
		alarmout_interval,whiteled_interval,email_pic_enable,tri_channels_pic,http_notification_interval,tri_audio_id from vca_linecross_event");
    snprintf(sQuery[LOITERING], sizeof(sQuery[LOITERING]),
             "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,email_interval,popup_interval,ptzaction_interval,\
		alarmout_interval,whiteled_interval,email_pic_enable,tri_channels_pic,http_notification_interval,tri_audio_id from vca_loiter_event");
    snprintf(sQuery[HUMAN], sizeof(sQuery[HUMAN]),
             "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,email_interval,popup_interval,ptzaction_interval,\
		alarmout_interval,whiteled_interval,email_pic_enable,tri_channels_pic,http_notification_interval,tri_audio_id from vca_human_event");
    snprintf(sQuery[PEOPLE_CNT], sizeof(sQuery[PEOPLE_CNT]),
             "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,email_interval,popup_interval,ptzaction_interval,\
             alarmout_interval,whiteled_interval,email_pic_enable,tri_channels_pic,http_notification_interval,tri_audio_id from vca_people_event");
    snprintf(sQuery[OBJECT_LEFTREMOVE], sizeof(sQuery[OBJECT_LEFTREMOVE]),
             "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,email_interval,popup_interval,ptzaction_interval,\
		alarmout_interval,whiteled_interval,email_pic_enable,tri_channels_pic,http_notification_interval,tri_audio_id from vca_object_leftremove_event");

    for (ichn = 0; ichn < chnCnt; ichn++) {
        for (i = REGIONIN; i < MAX_SMART_EVENT; i++) {
            snprintf(cmd, sizeof(cmd), "%s where id=%d;", sQuery[i], ichn);
            int nResult = sqlite_query_record(hConn, cmd, &hStmt, &nColumnCnt);
            if (nResult != 0 || nColumnCnt == 0) {
                if (hStmt) {
                    sqlite_clear_stmt(hStmt);
                    hStmt = 0;
                }
                continue;
            }

            sqlite_query_column(hStmt, 0, &smartevent[ichn][i].id);
            sqlite_query_column(hStmt, 1, &smartevent[ichn][i].enable);
            sqlite_query_column(hStmt, 2, (int *)&smartevent[ichn][i].tri_alarms);
            sqlite_query_column_text(hStmt, 3, smartevent[ichn][i].tri_channels_ex, sizeof(smartevent[ichn][i].tri_channels_ex));
            sqlite_query_column(hStmt, 4, &smartevent[ichn][i].buzzer_interval);
            sqlite_query_column(hStmt, 5, &smartevent[ichn][i].email_interval);

            sqlite_query_column(hStmt, 6, &smartevent[ichn][i].popup_interval);
            sqlite_query_column(hStmt, 7, &smartevent[ichn][i].ptzaction_interval);
            sqlite_query_column(hStmt, 8, &smartevent[ichn][i].alarmout_interval);
            sqlite_query_column(hStmt, 9, &smartevent[ichn][i].whiteled_interval);
            sqlite_query_column(hStmt, 10, &smartevent[ichn][i].email_pic_enable);
            sqlite_query_column_text(hStmt, 11, smartevent[ichn][i].tri_channels_pic, sizeof(smartevent[ichn][i].tri_channels_pic));
            sqlite_query_column(hStmt, 12, &smartevent[ichn][i].http_notification_interval);
            sqlite_query_column(hStmt, 13, &smartevent[ichn][i].tri_audio_id);
            
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
        }
    }

    if (hStmt) {
        sqlite_clear_stmt(hStmt);
    }
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_smart_event_ptz_params_init(const char *pDbFile, struct vca_ptz_action_params *vca, int type, int chnCnt)
{
    if (!pDbFile || type < 0) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[2048] = {0};
    int nColumnCnt = 0;
    int i = 0, chn_id = 0;

    do {
        switch (type) {
            case REGION_EN:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from vca_regionein_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case REGION_EXIT:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from vca_regionexit_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case ADVANCED_MOT:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from vca_motion_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case TAMPER_DET:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from vca_tamper_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case LINE_CROSS:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from vca_linecross_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case LOITER:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from vca_loiter_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case HUMAN_DET:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from vca_human_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case PEOPLE_COUNT:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from vca_people_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case LEFTREMOVE:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from vca_object_leftremove_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            default:
                break;
        }
        if (sQuery[0] == '\0') {
            sqlite_disconn(hConn);
            FileUnlock(nFd, mode, &global_rwlock);
            return -1;
        }
        int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            continue;
        }

        for (i = 0; i < nColumnCnt && i < MAX_REAL_CAMERA; i++) {
            sqlite_query_column(hStmt, 0, &vca->ptzActionParams[chn_id][i].chn_id);

            sqlite_query_column(hStmt, 1, &vca->ptzActionParams[chn_id][i].acto_ptz_channel);
            sqlite_query_column(hStmt, 2, &vca->ptzActionParams[chn_id][i].acto_fish_channel);
            sqlite_query_column(hStmt, 3, &vca->ptzActionParams[chn_id][i].acto_ptz_type);
            sqlite_query_column(hStmt, 4, &vca->ptzActionParams[chn_id][i].acto_ptz_preset);
            sqlite_query_column(hStmt, 5, &vca->ptzActionParams[chn_id][i].acto_ptz_patrol);
            sqlite_query_column(hStmt, 6, &vca->ptzActionParams[chn_id][i].acto_ptz_pattern);
            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }
        sqlite_clear_stmt(hStmt);
    } while (++chn_id < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_anpr_event_init(const char *pDbFile, struct smart_event *smartevent[], int chnCnt)
{
    if (!pDbFile || !smartevent) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int i = 0, ichn = 0;
    HSQLSTMT hStmt = 0;
    char sQuery[MAX_ANPR_MODE][512];
    char cmd[512] = {0};
    int nColumnCnt = 0;

    snprintf(sQuery[ANPR_BLACK], sizeof(sQuery[ANPR_BLACK]),
             "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,email_interval,popup_interval,\
		ptzaction_interval,alarmout_interval,whiteled_interval,email_pic_enable,tri_channels_pic,tri_chnout1_alarms,\
		tri_chnout2_alarms,http_notification_interval,tri_audio_id from lpr_black_mode_event");
    snprintf(sQuery[ANPR_WHITE], sizeof(sQuery[ANPR_WHITE]),
             "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,email_interval,popup_interval,\
		ptzaction_interval,alarmout_interval,whiteled_interval,email_pic_enable,tri_channels_pic,tri_chnout1_alarms,\
		tri_chnout2_alarms,http_notification_interval,tri_audio_id from lpr_white_mode_event");
    snprintf(sQuery[ANPR_VISTOR], sizeof(sQuery[ANPR_VISTOR]),
             "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,email_interval,popup_interval,\
		ptzaction_interval,alarmout_interval,whiteled_interval,email_pic_enable,tri_channels_pic,tri_chnout1_alarms,\
		tri_chnout2_alarms,http_notification_interval,tri_audio_id from lpr_vistor_mode_event");

    for (ichn = 0; ichn < chnCnt; ichn++) {
        for (i = 0; i < MAX_ANPR_MODE; i++) {
            snprintf(cmd, sizeof(cmd), "%s where id=%d;", sQuery[i], ichn);
            int nResult = sqlite_query_record(hConn, cmd, &hStmt, &nColumnCnt);
            if (nResult != 0 || nColumnCnt == 0) {
                if (hStmt) {
                    sqlite_clear_stmt(hStmt);
                    hStmt = 0;
                }
                continue;
            }

            sqlite_query_column(hStmt, 0, &smartevent[ichn][i].id);
            sqlite_query_column(hStmt, 1, &smartevent[ichn][i].enable);
            sqlite_query_column(hStmt, 2, (int *)&smartevent[ichn][i].tri_alarms);
            sqlite_query_column_text(hStmt, 3, smartevent[ichn][i].tri_channels_ex, sizeof(smartevent[ichn][i].tri_channels_ex));
            sqlite_query_column(hStmt, 4, &smartevent[ichn][i].buzzer_interval);
            sqlite_query_column(hStmt, 5, &smartevent[ichn][i].email_interval);
            sqlite_query_column(hStmt, 6, &smartevent[ichn][i].popup_interval);
            sqlite_query_column(hStmt, 7, &smartevent[ichn][i].ptzaction_interval);
            sqlite_query_column(hStmt, 8, &smartevent[ichn][i].alarmout_interval);
            sqlite_query_column(hStmt, 9, &smartevent[ichn][i].whiteled_interval);
            sqlite_query_column(hStmt, 10, &smartevent[ichn][i].email_pic_enable);
            sqlite_query_column_text(hStmt, 11, smartevent[ichn][i].tri_channels_pic, sizeof(smartevent[ichn][i].tri_channels_pic));
            sqlite_query_column_text(hStmt, 12, smartevent[ichn][i].tri_chnout1_alarms,
                                     sizeof(smartevent[ichn][i].tri_chnout1_alarms));
            sqlite_query_column_text(hStmt, 13, smartevent[ichn][i].tri_chnout2_alarms,
                                     sizeof(smartevent[ichn][i].tri_chnout2_alarms));
            sqlite_query_column(hStmt, 14, &smartevent[ichn][i].http_notification_interval);
            sqlite_query_column(hStmt, 15, &smartevent[ichn][i].tri_audio_id);
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
        }
    }

    if (hStmt) {
        sqlite_clear_stmt(hStmt);
    }
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_anpr_effective_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int chnCnt)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0, chn_id = 0;;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_ANPR_MODE][256];

    HSQLSTMT hStmt = 0;
    do {
        snprintf(sQuery[ANPR_BLACK], sizeof(sQuery[ANPR_BLACK]),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_black_mode_effective_schedule where chn_id=%d;",
                 chn_id);
        snprintf(sQuery[ANPR_WHITE], sizeof(sQuery[ANPR_WHITE]),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_white_mode_effective_schedule where chn_id=%d;",
                 chn_id);
        snprintf(sQuery[ANPR_VISTOR], sizeof(sQuery[ANPR_VISTOR]),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_vistor_mode_effective_schedule where chn_id=%d;",
                 chn_id);
        for (iSmt = 0; iSmt < MAX_ANPR_MODE; iSmt++) {
            nResult = sqlite_query_record(hConn, sQuery[iSmt], &hStmt, &nColumnCnt);
            if (nResult != 0 || nColumnCnt == 0) {
                if (hStmt) {
                    sqlite_clear_stmt(hStmt);
                    hStmt = 0;
                }
                continue;
            }

            for (i = 0; i <= nLoopCnt; i++) {
                sqlite_query_column(hStmt, 0, &nWeekId);
                sqlite_query_column(hStmt, 1, &nPlanId);
                sqlite_query_column_text(hStmt, 2, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
                sqlite_query_column_text(hStmt, 3, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
                sqlite_query_column(hStmt, 4, &Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
                if (sqlite_query_next(hStmt) != 0) {
                    i++;
                    break;
                }
            }
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
        }

        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }
    } while (++chn_id < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_anpr_audible_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int chnCnt)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0, chn_id = 0;;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_ANPR_MODE][256];

    HSQLSTMT hStmt = 0;
    do {
        snprintf(sQuery[ANPR_BLACK], sizeof(sQuery[ANPR_BLACK]),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_black_mode_audible_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[ANPR_WHITE], sizeof(sQuery[ANPR_WHITE]),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_white_mode_audible_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[ANPR_VISTOR], sizeof(sQuery[ANPR_VISTOR]),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_vistor_mode_audible_schedule where chn_id=%d;",
                 chn_id);
        for (iSmt = 0; iSmt < MAX_ANPR_MODE; iSmt++) {
            nResult = sqlite_query_record(hConn, sQuery[iSmt], &hStmt, &nColumnCnt);
            if (nResult != 0 || nColumnCnt == 0) {
                if (hStmt) {
                    sqlite_clear_stmt(hStmt);
                    hStmt = 0;
                }
                continue;
            }

            for (i = 0; i <= nLoopCnt; i++) {
                sqlite_query_column(hStmt, 0, &nWeekId);
                sqlite_query_column(hStmt, 1, &nPlanId);
                sqlite_query_column_text(hStmt, 2, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
                sqlite_query_column_text(hStmt, 3, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
                sqlite_query_column(hStmt, 4, &Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
                if (sqlite_query_next(hStmt) != 0) {
                    i++;
                    break;
                }
            }
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
        }

        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }
    } while (++chn_id < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_anpr_email_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int chnCnt)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0, chn_id = 0;;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_ANPR_MODE][256];

    HSQLSTMT hStmt = 0;
    do {
        snprintf(sQuery[ANPR_BLACK], sizeof(sQuery[ANPR_BLACK]),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_black_mode_mail_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[ANPR_WHITE], sizeof(sQuery[ANPR_WHITE]),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_white_mode_mail_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[ANPR_VISTOR], sizeof(sQuery[ANPR_VISTOR]),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_vistor_mode_mail_schedule where chn_id=%d;", chn_id);
        for (iSmt = 0; iSmt < MAX_ANPR_MODE; iSmt++) {
            nResult = sqlite_query_record(hConn, sQuery[iSmt], &hStmt, &nColumnCnt);
            if (nResult != 0 || nColumnCnt == 0) {
                if (hStmt) {
                    sqlite_clear_stmt(hStmt);
                    hStmt = 0;
                }
                continue;
            }

            for (i = 0; i <= nLoopCnt; i++) {
                sqlite_query_column(hStmt, 0, &nWeekId);
                sqlite_query_column(hStmt, 1, &nPlanId);
                sqlite_query_column_text(hStmt, 2, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
                sqlite_query_column_text(hStmt, 3, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
                sqlite_query_column(hStmt, 4, &Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
                if (sqlite_query_next(hStmt) != 0) {
                    i++;
                    break;
                }
            }
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
        }

        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }
    } while (++chn_id < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_anpr_popup_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int chnCnt)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0, chn_id = 0;;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_ANPR_MODE][256];

    HSQLSTMT hStmt = 0;
    do {
        snprintf(sQuery[ANPR_BLACK], sizeof(sQuery[ANPR_BLACK]),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_black_mode_popup_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[ANPR_WHITE], sizeof(sQuery[ANPR_WHITE]),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_white_mode_popup_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[ANPR_VISTOR], sizeof(sQuery[ANPR_VISTOR]),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_vistor_mode_popup_schedule where chn_id=%d;", chn_id);
        for (iSmt = 0; iSmt < MAX_ANPR_MODE; iSmt++) {
            nResult = sqlite_query_record(hConn, sQuery[iSmt], &hStmt, &nColumnCnt);
            if (nResult != 0 || nColumnCnt == 0) {
                if (hStmt) {
                    sqlite_clear_stmt(hStmt);
                    hStmt = 0;
                }
                continue;
            }

            for (i = 0; i <= nLoopCnt; i++) {
                sqlite_query_column(hStmt, 0, &nWeekId);
                sqlite_query_column(hStmt, 1, &nPlanId);
                sqlite_query_column_text(hStmt, 2, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
                sqlite_query_column_text(hStmt, 3, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
                sqlite_query_column(hStmt, 4, &Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
                if (sqlite_query_next(hStmt) != 0) {
                    i++;
                    break;
                }
            }
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
        }

        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }
    } while (++chn_id < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_anpr_ptz_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int chnCnt)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0, chn_id = 0;;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_ANPR_MODE][256];

    HSQLSTMT hStmt = 0;
    do {
        snprintf(sQuery[ANPR_BLACK], sizeof(sQuery[ANPR_BLACK]),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_black_mode_ptz_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[ANPR_WHITE], sizeof(sQuery[ANPR_WHITE]),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_white_mode_ptz_schedule where chn_id=%d;", chn_id);
        snprintf(sQuery[ANPR_VISTOR], sizeof(sQuery[ANPR_VISTOR]),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_vistor_mode_ptz_schedule where chn_id=%d;", chn_id);
        for (iSmt = 0; iSmt < MAX_ANPR_MODE; iSmt++) {
            nResult = sqlite_query_record(hConn, sQuery[iSmt], &hStmt, &nColumnCnt);
            if (nResult != 0 || nColumnCnt == 0) {
                if (hStmt) {
                    sqlite_clear_stmt(hStmt);
                    hStmt = 0;
                }
                continue;
            }

            for (i = 0; i <= nLoopCnt; i++) {
                sqlite_query_column(hStmt, 0, &nWeekId);
                sqlite_query_column(hStmt, 1, &nPlanId);
                sqlite_query_column_text(hStmt, 2, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
                sqlite_query_column_text(hStmt, 3, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
                sqlite_query_column(hStmt, 4, &Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
                if (sqlite_query_next(hStmt) != 0) {
                    i++;
                    break;
                }
            }
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
        }

        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }
    } while (++chn_id < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_anpr_ptz_params_init(const char *pDbFile, struct vca_ptz_action_params *anpr, ANPR_MODE_TYPE type, int chnCnt)
{
    if (!pDbFile || type < 0) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT cStmt = 0;
    HSQLSTMT hStmt = 0;
    char buf[128] = {0}, sQuery[2048] = {0};
    int nColumnCnt = 0;
    int i = 0, chn_id = 0, ret = -1, cols = 0, rows = 0;

    do {
        switch (type) {
            case ANPR_BLACK:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from lpr_black_mode_ptz_params where chn_id=%d;",
                         chn_id);
                snprintf(buf, sizeof(buf), "select count(*) from lpr_black_mode_ptz_params where chn_id=%d;", chn_id);
                break;
            case ANPR_WHITE:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from lpr_white_mode_ptz_params where chn_id=%d;",
                         chn_id);
                snprintf(buf, sizeof(buf), "select count(*) from lpr_white_mode_ptz_params where chn_id=%d;", chn_id);
                break;
            case ANPR_VISTOR:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from lpr_vistor_mode_ptz_params where chn_id=%d;",
                         chn_id);
                snprintf(buf, sizeof(buf), "select count(*) from lpr_vistor_mode_ptz_params where chn_id=%d;", chn_id);
                break;
            default:
                break;
        }
        if (sQuery[0] == '\0') {
            sqlite_disconn(hConn);
            FileUnlock(nFd, mode, &global_rwlock);
            return -1;
        }
        ret = sqlite_query_record(hConn, buf, &cStmt, &cols);
        if (ret != 0 || cols == 0) {
            if (cStmt) {
                sqlite_clear_stmt(cStmt);
            }
            sqlite_disconn(hConn);
            FileUnlock(nFd, mode, &global_rwlock);
            return -1;
        }
        ret = sqlite_query_column(cStmt, 0, &rows);
        if (ret != 0 || rows == 0) {
            if (cStmt) {
                sqlite_clear_stmt(cStmt);
            }
            sqlite_disconn(hConn);
            FileUnlock(nFd, mode, &global_rwlock);
            return -1;
        }
        if (cStmt) {
            sqlite_clear_stmt(cStmt);
        }
        ret = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (ret != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            sqlite_disconn(hConn);
            FileUnlock(nFd, mode, &global_rwlock);
            return -1;
        }
        for (i = 0; i < rows && i < MAX_REAL_CAMERA; i++) {
            sqlite_query_column(hStmt, 0, &anpr->ptzActionParams[chn_id][i].chn_id);
            sqlite_query_column(hStmt, 1, &anpr->ptzActionParams[chn_id][i].acto_ptz_channel);
            sqlite_query_column(hStmt, 2, &anpr->ptzActionParams[chn_id][i].acto_fish_channel);
            sqlite_query_column(hStmt, 3, &anpr->ptzActionParams[chn_id][i].acto_ptz_type);
            sqlite_query_column(hStmt, 4, &anpr->ptzActionParams[chn_id][i].acto_ptz_preset);
            sqlite_query_column(hStmt, 5, &anpr->ptzActionParams[chn_id][i].acto_ptz_patrol);
            sqlite_query_column(hStmt, 6, &anpr->ptzActionParams[chn_id][i].acto_ptz_pattern);
            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }

        sqlite_clear_stmt(hStmt);
    } while (++chn_id < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_anpr_enable_effective_schedule_init(const char *pDbFile, void *params, int chnCnt)
{
    if (!pDbFile || !params) {
        return -1;
    }
    struct smart_event_schedule *vSchedule = (struct smart_event_schedule *)params;
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[256] = {0};
    int nColumnCnt = 0;
    int i = 0, chn_id = 0;

    do {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_enable_effective_schedule where chn_id=%d;", chn_id);
        int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            continue;
        }
        int nWeekId = 1;
        int nPlanId = 0;
        int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
        for (i = 0; i <= nLoopCnt; i++) {
            sqlite_query_column(hStmt, 0, &nWeekId);
            sqlite_query_column(hStmt, 1, &nPlanId);
            sqlite_query_column_text(hStmt, 2, vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].action_type));
            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }
        sqlite_clear_stmt(hStmt);
    } while (++chn_id < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_ipc_chnIn_events_init(const char *pDbFile, struct smart_event *smartevent[], int maxChnCnt)
{
    if (!pDbFile || !smartevent) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int i = 0, ichn = 0;
    HSQLSTMT hStmt = 0;
    char sQuery[512];
    char cmd[512] = {0};
    int nColumnCnt = 0;
    int nResult = -1;

    snprintf(sQuery, sizeof(sQuery), "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,email_interval,\
		tri_chnout1_alarms,tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval,\
		email_pic_enable,tri_channels_pic,http_notification_interval,tri_audio_id from");

    for (ichn = 0; ichn < maxChnCnt && i < MAX_CAMERA; ichn++) {
        for (i = 0; i < MAX_IPC_ALARM_IN; i++) {
            snprintf(cmd, sizeof(cmd), "%s alarm_chnIn%d_event where id=%d;", sQuery, i, ichn);
            nResult = sqlite_query_record(hConn, cmd, &hStmt, &nColumnCnt);
            if (nResult != 0 || nColumnCnt == 0) {
                if (hStmt) {
                    sqlite_clear_stmt(hStmt);
                    hStmt = 0;
                }
                continue;
            }

            sqlite_query_column(hStmt, 0, &smartevent[ichn][i].id);
            sqlite_query_column(hStmt, 1, &smartevent[ichn][i].enable);
            sqlite_query_column(hStmt, 2, (int *)&smartevent[ichn][i].tri_alarms);
            sqlite_query_column_text(hStmt, 3, smartevent[ichn][i].tri_channels_ex, sizeof(smartevent[ichn][i].tri_channels_ex));
            sqlite_query_column(hStmt, 4, &smartevent[ichn][i].buzzer_interval);
            sqlite_query_column(hStmt, 5, &smartevent[ichn][i].email_interval);
            sqlite_query_column_text(hStmt, 6, smartevent[ichn][i].tri_chnout1_alarms,
                                     sizeof(smartevent[ichn][i].tri_chnout1_alarms));
            sqlite_query_column_text(hStmt, 7, smartevent[ichn][i].tri_chnout2_alarms,
                                     sizeof(smartevent[ichn][i].tri_chnout2_alarms));
            sqlite_query_column(hStmt, 8, &smartevent[ichn][i].popup_interval);
            sqlite_query_column(hStmt, 9, &smartevent[ichn][i].ptzaction_interval);
            sqlite_query_column(hStmt, 10, &smartevent[ichn][i].alarmout_interval);
            sqlite_query_column(hStmt, 11, &smartevent[ichn][i].whiteled_interval);
            sqlite_query_column(hStmt, 12, &smartevent[ichn][i].email_pic_enable);
            sqlite_query_column_text(hStmt, 13, smartevent[ichn][i].tri_channels_pic, sizeof(smartevent[ichn][i].tri_channels_pic));
            sqlite_query_column(hStmt, 14, &smartevent[ichn][i].http_notification_interval);
            sqlite_query_column(hStmt, 15, &smartevent[ichn][i].tri_audio_id);

            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
        }
    }

    if (hStmt) {
        sqlite_clear_stmt(hStmt);
    }
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_ipc_chnIn_effective_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int maxChnCnt)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0, chn_id = 0;;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[256] = {0};

    HSQLSTMT hStmt = 0;
    do {
        for (iSmt = 0; iSmt < MAX_IPC_ALARM_IN; iSmt++) {
            snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from alarm_chnIn%d_effective_schedule "
                 "where chn_id=%d;", iSmt, chn_id);
            nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
            if (nResult != 0 || nColumnCnt == 0) {
                if (hStmt) {
                    sqlite_clear_stmt(hStmt);
                    hStmt = 0;
                }
                continue;
            }

            for (i = 0; i <= nLoopCnt; i++) {
                sqlite_query_column(hStmt, 0, &nWeekId);
                sqlite_query_column(hStmt, 1, &nPlanId);
                sqlite_query_column_text(hStmt, 2, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
                sqlite_query_column_text(hStmt, 3, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
                sqlite_query_column(hStmt, 4, &Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
                if (sqlite_query_next(hStmt) != 0) {
                    i++;
                    break;
                }
            }
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
        }

        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }
    } while ((++chn_id) < maxChnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_ipc_chnIn_audible_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int maxChnCnt)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0, chn_id = 0;;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[256] = {0};

    HSQLSTMT hStmt = 0;
    do {
        for (iSmt = 0; iSmt < MAX_IPC_ALARM_IN; iSmt++) {
            snprintf(sQuery, sizeof(sQuery),
                     "select week_id,plan_id,start_time,end_time,action_type from alarm_chnIn%d_audible_schedule where chn_id=%d;", iSmt, chn_id);
            nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
            if (nResult != 0 || nColumnCnt == 0) {
                if (hStmt) {
                    sqlite_clear_stmt(hStmt);
                    hStmt = 0;
                }
                continue;
            }

            for (i = 0; i <= nLoopCnt; i++) {
                sqlite_query_column(hStmt, 0, &nWeekId);
                sqlite_query_column(hStmt, 1, &nPlanId);
                sqlite_query_column_text(hStmt, 2, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
                sqlite_query_column_text(hStmt, 3, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
                sqlite_query_column(hStmt, 4, &Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
                if (sqlite_query_next(hStmt) != 0) {
                    i++;
                    break;
                }
            }
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
        }

        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }
    } while ((++chn_id) < maxChnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_ipc_chnIn_email_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int maxChnCnt)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0, chn_id = 0;;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[256] = {0};

    HSQLSTMT hStmt = 0;
    do {
        for (iSmt = 0; iSmt < MAX_IPC_ALARM_IN; iSmt++) {
            snprintf(sQuery, sizeof(sQuery),
                     "select week_id,plan_id,start_time,end_time,action_type from alarm_chnIn%d_mail_schedule where chn_id=%d;", iSmt, chn_id);
            nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
            if (nResult != 0 || nColumnCnt == 0) {
                if (hStmt) {
                    sqlite_clear_stmt(hStmt);
                    hStmt = 0;
                }
                continue;
            }

            for (i = 0; i <= nLoopCnt; i++) {
                sqlite_query_column(hStmt, 0, &nWeekId);
                sqlite_query_column(hStmt, 1, &nPlanId);
                sqlite_query_column_text(hStmt, 2, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
                sqlite_query_column_text(hStmt, 3, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
                sqlite_query_column(hStmt, 4, &Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
                if (sqlite_query_next(hStmt) != 0) {
                    i++;
                    break;
                }
            }
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
        }

        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }
    } while ((++chn_id) < maxChnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_ipc_chnIn_popup_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int maxChnCnt)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0, chn_id = 0;;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[256] = {0};

    HSQLSTMT hStmt = 0;
    do {
        for (iSmt = 0; iSmt < MAX_IPC_ALARM_IN; iSmt++) {
            snprintf(sQuery, sizeof(sQuery),
                     "select week_id,plan_id,start_time,end_time,action_type from alarm_chnIn%d_popup_schedule where chn_id=%d;", iSmt, chn_id);
            nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
            if (nResult != 0 || nColumnCnt == 0) {
                if (hStmt) {
                    sqlite_clear_stmt(hStmt);
                    hStmt = 0;
                }
                continue;
            }

            for (i = 0; i <= nLoopCnt; i++) {
                sqlite_query_column(hStmt, 0, &nWeekId);
                sqlite_query_column(hStmt, 1, &nPlanId);
                sqlite_query_column_text(hStmt, 2, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
                sqlite_query_column_text(hStmt, 3, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
                sqlite_query_column(hStmt, 4, &Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
                if (sqlite_query_next(hStmt) != 0) {
                    i++;
                    break;
                }
            }
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
        }

        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }
    } while ((++chn_id) < maxChnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_ipc_chnIn_ptz_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int maxChnCnt)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0, chn_id = 0;;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[256] = {0};

    HSQLSTMT hStmt = 0;
    do {
        for (iSmt = 0; iSmt < MAX_IPC_ALARM_IN; iSmt++) {
            snprintf(sQuery, sizeof(sQuery),
                     "select week_id,plan_id,start_time,end_time,action_type from alarm_chnIn%d_ptz_schedule where chn_id=%d;", iSmt, chn_id);
            nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
            if (nResult != 0 || nColumnCnt == 0) {
                if (hStmt) {
                    sqlite_clear_stmt(hStmt);
                    hStmt = 0;
                }
                continue;
            }

            for (i = 0; i <= nLoopCnt; i++) {
                sqlite_query_column(hStmt, 0, &nWeekId);
                sqlite_query_column(hStmt, 1, &nPlanId);
                sqlite_query_column_text(hStmt, 2, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
                sqlite_query_column_text(hStmt, 3, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
                sqlite_query_column(hStmt, 4, &Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
                if (sqlite_query_next(hStmt) != 0) {
                    i++;
                    break;
                }
            }
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
        }

        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }
    } while ((++chn_id) < maxChnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_ipc_chnIn_ptz_params_init(const char *pDbFile, struct vca_ptz_action_params *params, int alarmid,
                                   int maxChnCnt)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[2048] = {0};
    int nColumnCnt = 0;
    int i = 0, chn_id = 0;

    do {
        snprintf(sQuery, sizeof(sQuery),
                 "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from alarm_chnIn%d_ptz_params where chn_id=%d;",
                 alarmid, chn_id);
        if (sQuery[0] == '\0') {
            sqlite_disconn(hConn);
            FileUnlock(nFd, mode, &global_rwlock);
            return -1;
        }
        int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            continue;
        }
        for (i = 0; i < nColumnCnt && i < MAX_REAL_CAMERA; i++) {
            sqlite_query_column(hStmt, 0, &params->ptzActionParams[chn_id][i].chn_id);

            sqlite_query_column(hStmt, 1, &params->ptzActionParams[chn_id][i].acto_ptz_channel);
            sqlite_query_column(hStmt, 2, &params->ptzActionParams[chn_id][i].acto_fish_channel);
            sqlite_query_column(hStmt, 3, &params->ptzActionParams[chn_id][i].acto_ptz_type);
            sqlite_query_column(hStmt, 4, &params->ptzActionParams[chn_id][i].acto_ptz_preset);
            sqlite_query_column(hStmt, 5, &params->ptzActionParams[chn_id][i].acto_ptz_patrol);
            sqlite_query_column(hStmt, 6, &params->ptzActionParams[chn_id][i].acto_ptz_pattern);
            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }
        sqlite_clear_stmt(hStmt);
    } while (++chn_id < maxChnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_ipc_chnOut_effective_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[],
                                             int maxChnCnt)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0, chn_id = 0;;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[256] = {0};

    HSQLSTMT hStmt = 0;
    do {
        for (iSmt = 0; iSmt < MAX_IPC_ALARM_OUT; iSmt++) {
            if (iSmt == 0) {
                snprintf(sQuery, sizeof(sQuery),
                         "select week_id,plan_id,start_time,end_time,action_type from alarm_chnOut0_effective_schedule where chn_id=%d;",
                         chn_id);
            } else {
                snprintf(sQuery, sizeof(sQuery),
                         "select week_id,plan_id,start_time,end_time,action_type from alarm_chnOut1_effective_schedule where chn_id=%d;",
                         chn_id);
            }
            nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
            if (nResult != 0 || nColumnCnt == 0) {
                if (hStmt) {
                    sqlite_clear_stmt(hStmt);
                    hStmt = 0;
                }
                continue;
            }

            for (i = 0; i <= nLoopCnt; i++) {
                sqlite_query_column(hStmt, 0, &nWeekId);
                sqlite_query_column(hStmt, 1, &nPlanId);
                sqlite_query_column_text(hStmt, 2, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
                sqlite_query_column_text(hStmt, 3, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
                sqlite_query_column(hStmt, 4, &Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
                if (sqlite_query_next(hStmt) != 0) {
                    i++;
                    break;
                }
            }
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
        }

        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }
    } while ((++chn_id) < maxChnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_whiteled_effective_schedule_init(const char *pDbFile, const char *pDbTable, void *params, int maxChnCnt)
{
    if (!pDbFile || !params) {
        return -1;
    }
    struct smart_event_schedule *vSchedule = (struct smart_event_schedule *)params;
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[256] = {0};
    int nColumnCnt = 0;
    int i = 0, chn_id = 0;

    do {
        snprintf(sQuery, sizeof(sQuery), "select week_id,plan_id,start_time,end_time,action_type from %s where chn_id=%d;",
                 pDbTable, chn_id);
        int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            continue;
        }
        int nWeekId = 1;
        int nPlanId = 0;
        int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
        for (i = 0; i <= nLoopCnt; i++) {
            sqlite_query_column(hStmt, 0, &nWeekId);
            sqlite_query_column(hStmt, 1, &nPlanId);
            sqlite_query_column_text(hStmt, 2, vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &(vSchedule[chn_id].schedule_day[nWeekId].schedule_item[nPlanId].action_type));
            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }
        sqlite_clear_stmt(hStmt);
    } while (++chn_id < maxChnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_whiteled_params_init(const char *pDbFile, const char *pDbTable, WHITE_LED_PARAMS *ledparams[], int maxChnCnt)
{
    if (!pDbFile || !pDbTable) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[2048] = {0};
    int nColumnCnt = 0;
    int i = 0, chn_id = 0;

    do {
        snprintf(sQuery, sizeof(sQuery), "select chnid,flash_mode,flash_time,acto_chn_id from %s where chnid=%d;", pDbTable,
                 chn_id);
        int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            continue;
        }
        //msprintf("chnid:%d nColumnCnt:%d", chn_id, nColumnCnt);
        for (i = 0; i < nColumnCnt && i < MAX_REAL_CAMERA; i++) {
            sqlite_query_column(hStmt, 0, &ledparams[chn_id][i].chnid);
            sqlite_query_column(hStmt, 1, &ledparams[chn_id][i].flash_mode);
            sqlite_query_column(hStmt, 2, &ledparams[chn_id][i].flash_time);
            sqlite_query_column(hStmt, 3, &ledparams[chn_id][i].acto_chn_id);
            //msprintf("chnid:%d %d %d %d", ledparams[chn_id][i].chnid, ledparams[chn_id][i].flash_mode, ledparams[chn_id][i].flash_time, ledparams[chn_id][i].acto_chn_id);
            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }
        sqlite_clear_stmt(hStmt);
    } while (++chn_id < maxChnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_smart_event_led_params_init(const char *pDbFile, WHITE_LED_PARAMS_EVTS *vca, int type, int chnCnt)
{
    if (!pDbFile || type < 0) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[2048] = {0};
    int nColumnCnt = 0;
    int i = 0, chn_id = 0;

    do {
        switch (type) {
            case REGION_EN:
                snprintf(sQuery, sizeof(sQuery), "select chnid,flash_mode,flash_time,acto_chn_id from %s where chnid=%d;",
                         VREIN_WLED_PARAMS, chn_id);
                break;
            case REGION_EXIT:
                snprintf(sQuery, sizeof(sQuery), "select chnid,flash_mode,flash_time,acto_chn_id from %s where chnid=%d;",
                         VREEX_WLED_PARAMS, chn_id);
                break;
            case ADVANCED_MOT:
                snprintf(sQuery, sizeof(sQuery), "select chnid,flash_mode,flash_time,acto_chn_id from %s where chnid=%d;",
                         VMOT_WLED_PARAMS, chn_id);
                break;
            case TAMPER_DET:
                snprintf(sQuery, sizeof(sQuery), "select chnid,flash_mode,flash_time,acto_chn_id from %s where chnid=%d;",
                         VTEP_WLED_PARAMS, chn_id);
                break;
            case LINE_CROSS:
                snprintf(sQuery, sizeof(sQuery), "select chnid,flash_mode,flash_time,acto_chn_id from %s where chnid=%d;",
                         VLSS_WLED_PARAMS, chn_id);
                break;
            case LOITER:
                snprintf(sQuery, sizeof(sQuery), "select chnid,flash_mode,flash_time,acto_chn_id from %s where chnid=%d;",
                         VLER_WLED_PARAMS, chn_id);
                break;
            case HUMAN_DET:
                snprintf(sQuery, sizeof(sQuery), "select chnid,flash_mode,flash_time,acto_chn_id from %s where chnid=%d;",
                         VHMN_WLED_PARAMS, chn_id);
                break;
            case PEOPLE_COUNT:
                snprintf(sQuery, sizeof(sQuery), "select chnid,flash_mode,flash_time,acto_chn_id from %s where chnid=%d;",
                         VPPE_WLED_PARAMS, chn_id);
                break;
            case LEFTREMOVE:
                snprintf(sQuery, sizeof(sQuery), "select chnid,flash_mode,flash_time,acto_chn_id from %s where chnid=%d;",
                         VOBJ_WLED_PARAMS, chn_id);
                break;
            default:
                break;
        }
        if (sQuery[0] == '\0') {
            sqlite_disconn(hConn);
            FileUnlock(nFd, mode, &global_rwlock);
            return -1;
        }
        int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            continue;
        }

        for (i = 0; i < nColumnCnt && i < MAX_CAMERA; i++) {
            sqlite_query_column(hStmt, 0, &vca->ledPrms[chn_id][i].chnid);

            sqlite_query_column(hStmt, 1, &vca->ledPrms[chn_id][i].flash_mode);
            sqlite_query_column(hStmt, 2, &vca->ledPrms[chn_id][i].flash_time);
            sqlite_query_column(hStmt, 3, &vca->ledPrms[chn_id][i].acto_chn_id);

            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }
        sqlite_clear_stmt(hStmt);
    } while (++chn_id < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_anpr_led_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int chnCnt)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0, chn_id = 0;;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_ANPR_MODE][256];

    HSQLSTMT hStmt = 0;
    do {
        snprintf(sQuery[ANPR_BLACK], sizeof(sQuery[ANPR_BLACK]),
                 "select week_id,plan_id,start_time,end_time,action_type from %s where chn_id=%d;", LPRB_WLED_ESCHE, chn_id);
        snprintf(sQuery[ANPR_WHITE], sizeof(sQuery[ANPR_WHITE]),
                 "select week_id,plan_id,start_time,end_time,action_type from %s where chn_id=%d;", LPRW_WLED_ESCHE, chn_id);
        snprintf(sQuery[ANPR_VISTOR], sizeof(sQuery[ANPR_VISTOR]),
                 "select week_id,plan_id,start_time,end_time,action_type from %s where chn_id=%d;", LPRV_WLED_ESCHE, chn_id);
        for (iSmt = 0; iSmt < MAX_ANPR_MODE; iSmt++) {
            nResult = sqlite_query_record(hConn, sQuery[iSmt], &hStmt, &nColumnCnt);
            if (nResult != 0 || nColumnCnt == 0) {
                if (hStmt) {
                    sqlite_clear_stmt(hStmt);
                    hStmt = 0;
                }
                continue;
            }

            for (i = 0; i <= nLoopCnt; i++) {
                sqlite_query_column(hStmt, 0, &nWeekId);
                sqlite_query_column(hStmt, 1, &nPlanId);
                sqlite_query_column_text(hStmt, 2, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
                sqlite_query_column_text(hStmt, 3, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
                sqlite_query_column(hStmt, 4, &Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
                if (sqlite_query_next(hStmt) != 0) {
                    i++;
                    break;
                }
            }
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
        }

        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }
    } while (++chn_id < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_anpr_led_params_init(const char *pDbFile, WHITE_LED_PARAMS_EVTS *anpr, ANPR_MODE_TYPE type, int chnCnt)
{
    if (!pDbFile || type < 0) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT cStmt = 0;
    HSQLSTMT hStmt = 0;
    char buf[128] = {0}, sQuery[2048] = {0};
    int nColumnCnt = 0;
    int i = 0, chn_id = 0, ret = -1, cols = 0, rows = 0;

    do {
        switch (type) {
            case ANPR_BLACK:
                snprintf(sQuery, sizeof(sQuery), "select chnid,flash_mode,flash_time,acto_chn_id from %s where chnid=%d;",
                         LPRB_WLED_PARAMS, chn_id);
                snprintf(buf, sizeof(buf), "select count(*) from %s where chnid=%d;", LPRB_WLED_PARAMS, chn_id);
                break;
            case ANPR_WHITE:
                snprintf(sQuery, sizeof(sQuery), "select chnid,flash_mode,flash_time,acto_chn_id from %s where chnid=%d;",
                         LPRW_WLED_PARAMS, chn_id);
                snprintf(buf, sizeof(buf), "select count(*) from %s where chnid=%d;", LPRW_WLED_PARAMS, chn_id);
                break;
            case ANPR_VISTOR:
                snprintf(sQuery, sizeof(sQuery), "select chnid,flash_mode,flash_time,acto_chn_id from %s where chnid=%d;",
                         LPRV_WLED_PARAMS, chn_id);
                snprintf(buf, sizeof(buf), "select count(*) from %s where chnid=%d;", LPRV_WLED_PARAMS, chn_id);
                break;
            default:
                break;
        }
        if (sQuery[0] == '\0') {
            sqlite_disconn(hConn);
            FileUnlock(nFd, mode, &global_rwlock);
            return -1;
        }
        ret = sqlite_query_record(hConn, buf, &cStmt, &cols);
        if (ret != 0 || cols == 0) {
            if (cStmt) {
                sqlite_clear_stmt(cStmt);
            }
            sqlite_disconn(hConn);
            FileUnlock(nFd, mode, &global_rwlock);
            return -1;
        }
        ret = sqlite_query_column(cStmt, 0, &rows);
        if (ret != 0 || rows == 0) {
            if (cStmt) {
                sqlite_clear_stmt(cStmt);
            }
            sqlite_disconn(hConn);
            FileUnlock(nFd, mode, &global_rwlock);
            return -1;
        }
        if (cStmt) {
            sqlite_clear_stmt(cStmt);
        }
        ret = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (ret != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            sqlite_disconn(hConn);
            FileUnlock(nFd, mode, &global_rwlock);
            return -1;
        }
        for (i = 0; i < rows && i < MAX_CAMERA; i++) {
            sqlite_query_column(hStmt, 0, &anpr->ledPrms[chn_id][i].chnid);
            sqlite_query_column(hStmt, 1, &anpr->ledPrms[chn_id][i].flash_mode);
            sqlite_query_column(hStmt, 2, &anpr->ledPrms[chn_id][i].flash_time);
            sqlite_query_column(hStmt, 3, &anpr->ledPrms[chn_id][i].acto_chn_id);

            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }

        sqlite_clear_stmt(hStmt);
    } while (++chn_id < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_ipc_chnIn_led_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int maxChnCnt)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0, chn_id = 0;;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[256] = {0};

    HSQLSTMT hStmt = 0;
    do {
        for (iSmt = 0; iSmt < MAX_IPC_ALARM_IN; iSmt++) {
            snprintf(sQuery, sizeof(sQuery), "select week_id,plan_id,start_time,end_time,action_type from alarm_chnIn%d_whiteled_esche where chn_id=%d;",
                     iSmt, chn_id);
            nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
            if (nResult != 0 || nColumnCnt == 0) {
                if (hStmt) {
                    sqlite_clear_stmt(hStmt);
                    hStmt = 0;
                }
                continue;
            }

            for (i = 0; i <= nLoopCnt; i++) {
                sqlite_query_column(hStmt, 0, &nWeekId);
                sqlite_query_column(hStmt, 1, &nPlanId);
                sqlite_query_column_text(hStmt, 2, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
                sqlite_query_column_text(hStmt, 3, Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                         sizeof(Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
                sqlite_query_column(hStmt, 4, &Schedule[chn_id][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
                if (sqlite_query_next(hStmt) != 0) {
                    i++;
                    break;
                }
            }
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
        }

        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }
    } while ((++chn_id) < maxChnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_ipc_chnIn_led_params_init(const char *pDbFile, WHITE_LED_PARAMS_EVTS *params, int alarmid, int maxChnCnt)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[2048] = {0};
    int nColumnCnt = 0;
    int i = 0, chn_id = 0;

    do {
        snprintf(sQuery, sizeof(sQuery), "select chnid,flash_mode,flash_time,acto_chn_id from alarm_chnIn%d_white_led_params where chnid=%d;",
                 alarmid, chn_id);
        if (sQuery[0] == '\0') {
            sqlite_disconn(hConn);
            FileUnlock(nFd, mode, &global_rwlock);
            return -1;
        }
        int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            continue;
        }
        for (i = 0; i < nColumnCnt && i < MAX_CAMERA; i++) {
            sqlite_query_column(hStmt, 0, &params->ledPrms[chn_id][i].chnid);
            sqlite_query_column(hStmt, 1, &params->ledPrms[chn_id][i].flash_mode);
            sqlite_query_column(hStmt, 2, &params->ledPrms[chn_id][i].flash_time);
            sqlite_query_column(hStmt, 3, &params->ledPrms[chn_id][i].acto_chn_id);

            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }
        sqlite_clear_stmt(hStmt);
    } while (++chn_id < maxChnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_peoplecnt_event_init(const char *pDbFile, void *params, int groudCnt)
{
    if (!pDbFile || !params) {
        return -1;
    }
    PEOPLECNT_EVENT *smartevent = (PEOPLECNT_EVENT *)params;
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int groupid = 0;
    HSQLSTMT hStmt = 0;
    char sQuery[512];
    char cmd[512] = {0};
    int nColumnCnt = 0;

    snprintf(sQuery, sizeof(sQuery),
             "select groupid,tri_alarms,tri_chnout1_alarms,tri_chnout2_alarms,buzzer_interval,email_interval,\
             ptzaction_interval,alarmout_interval,whiteled_interval,email_pic_enable,tri_channels_pic,\
             http_notification_interval,tri_audio_id from people_cnt_event");
    for (groupid = 0; groupid < groudCnt; groupid++) {
        snprintf(cmd, sizeof(cmd), "%s where groupid=%d;", sQuery, groupid);
        int nResult = sqlite_query_record(hConn, cmd, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
            continue;
        }

        sqlite_query_column(hStmt, 0, &smartevent[groupid].groupid);
        sqlite_query_column(hStmt, 1, (int *)&smartevent[groupid].tri_alarms);
        sqlite_query_column_text(hStmt, 2, smartevent[groupid].tri_chnout1_alarms,
                                 sizeof(smartevent[groupid].tri_chnout1_alarms));
        sqlite_query_column_text(hStmt, 3, smartevent[groupid].tri_chnout2_alarms,
                                 sizeof(smartevent[groupid].tri_chnout2_alarms));
        sqlite_query_column(hStmt, 4, &smartevent[groupid].buzzer_interval);
        sqlite_query_column(hStmt, 5, &smartevent[groupid].email_interval);
        sqlite_query_column(hStmt, 6, &smartevent[groupid].ptzaction_interval);
        sqlite_query_column(hStmt, 7, &smartevent[groupid].alarmout_interval);
        sqlite_query_column(hStmt, 8, &smartevent[groupid].whiteled_interval);
        sqlite_query_column(hStmt, 9, &smartevent[groupid].email_pic_enable);
        sqlite_query_column_text(hStmt, 10, smartevent[groupid].tri_channels_pic, sizeof(smartevent[groupid].tri_channels_pic));
        sqlite_query_column(hStmt, 11, &smartevent[groupid].http_notification_interval);
        sqlite_query_column(hStmt, 12, &smartevent[groupid].tri_audio_id);
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }
    }

    if (hStmt) {
        sqlite_clear_stmt(hStmt);
    }
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_peoplecnt_audible_schedules_init(const char *pDbFile, void *params, int groudCnt)
{
    if (!pDbFile) {
        return -1;
    }
    struct smart_event_schedule *Schedule = (struct smart_event_schedule *)params;
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, groupid = 0;;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[256];

    HSQLSTMT hStmt = 0;
    for (groupid = 0; groupid < groudCnt; groupid++) {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from people_cnt_audible_sche where groupid=%d;", groupid);
        nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
            continue;
        }
        //msprintf("[david debug] nResult:%d nColumnCnt:%d", nResult, nColumnCnt);
        for (i = 0; i <= nLoopCnt; i++) {
            sqlite_query_column(hStmt, 0, &nWeekId);
            sqlite_query_column(hStmt, 1, &nPlanId);
            sqlite_query_column_text(hStmt, 2, Schedule[groupid].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(Schedule[groupid].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, Schedule[groupid].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(Schedule[groupid].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &Schedule[groupid].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }

        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }
    }

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_peoplecnt_email_schedules_init(const char *pDbFile, void *params, int groudCnt)
{
    if (!pDbFile) {
        return -1;
    }
    struct smart_event_schedule *Schedule = (struct smart_event_schedule *)params;
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, groupid = 0;;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[256];

    HSQLSTMT hStmt = 0;
    for (groupid = 0; groupid < groudCnt; groupid++) {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from people_cnt_email_sche where groupid=%d;", groupid);
        nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
            continue;
        }

        for (i = 0; i <= nLoopCnt; i++) {
            sqlite_query_column(hStmt, 0, &nWeekId);
            sqlite_query_column(hStmt, 1, &nPlanId);
            sqlite_query_column_text(hStmt, 2, Schedule[groupid].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(Schedule[groupid].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, Schedule[groupid].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(Schedule[groupid].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &Schedule[groupid].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }

        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }
    }

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_peoplecnt_ptz_schedules_init(const char *pDbFile, void *params, int groudCnt)
{
    if (!pDbFile) {
        return -1;
    }
    struct smart_event_schedule *Schedule = (struct smart_event_schedule *)params;
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, groupid = 0;;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[256];

    HSQLSTMT hStmt = 0;
    for (groupid = 0; groupid < groudCnt; groupid++) {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from people_cnt_ptz_sche where groupid=%d;", groupid);
        nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
            continue;
        }

        for (i = 0; i <= nLoopCnt; i++) {
            sqlite_query_column(hStmt, 0, &nWeekId);
            sqlite_query_column(hStmt, 1, &nPlanId);
            sqlite_query_column_text(hStmt, 2, Schedule[groupid].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(Schedule[groupid].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, Schedule[groupid].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(Schedule[groupid].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &Schedule[groupid].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }

        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }
    }

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_peoplecnt_wled_schedules_init(const char *pDbFile, void *params, int groudCnt)
{
    if (!pDbFile) {
        return -1;
    }
    struct smart_event_schedule *Schedule = (struct smart_event_schedule *)params;
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, groupid = 0;;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[256];

    HSQLSTMT hStmt = 0;
    for (groupid = 0; groupid < groudCnt; groupid++) {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from people_cnt_wled_sche where groupid=%d;", groupid);
        nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
            continue;
        }

        for (i = 0; i <= nLoopCnt; i++) {
            sqlite_query_column(hStmt, 0, &nWeekId);
            sqlite_query_column(hStmt, 1, &nPlanId);
            sqlite_query_column_text(hStmt, 2, Schedule[groupid].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(Schedule[groupid].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, Schedule[groupid].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(Schedule[groupid].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &Schedule[groupid].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }

        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }
    }

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_peoplecnt_wled_params_init(const char *pDbFile, PCNT_WLED_PARAMS_EVTS *params, int groudCnt)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[2048] = {0};
    int nColumnCnt = 0;
    int i = 0, groupid = 0;

    for (groupid = 0; groupid < groudCnt; groupid++) {
        snprintf(sQuery, sizeof(sQuery),
                 "select groupid,flash_mode,flash_time,acto_chn_id from people_cnt_wled_params where groupid=%d;", groupid);
        int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            continue;
        }
        //msprintf("chnid:%d nColumnCnt:%d", chn_id, nColumnCnt);
        for (i = 0; i < nColumnCnt && i < MAX_REAL_CAMERA; i++) {
            sqlite_query_column(hStmt, 0, &params->ledPrms[groupid][i].groupid);
            sqlite_query_column(hStmt, 1, &params->ledPrms[groupid][i].flash_mode);
            sqlite_query_column(hStmt, 2, &params->ledPrms[groupid][i].flash_time);
            sqlite_query_column(hStmt, 3, &params->ledPrms[groupid][i].acto_chn_id);
            //msprintf("chnid:%d %d %d %d", ledparams[chn_id][i].chnid, ledparams[chn_id][i].flash_mode, ledparams[chn_id][i].flash_time, ledparams[chn_id][i].acto_chn_id);
            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }
        sqlite_clear_stmt(hStmt);
    }

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_peoplecnt_ptz_params_init(const char *pDbFile, PEOPLECNT_ACT_PTZ_PARAMS *params, int groudCnt)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT cStmt = 0;
    HSQLSTMT hStmt = 0;
    char buf[128] = {0}, sQuery[2048] = {0};
    int nColumnCnt = 0, groupid = 0;
    int i = 0, ret = -1, cols = 0, rows = 0;

    for (groupid = 0; groupid < groudCnt; groupid++) {
        snprintf(sQuery, sizeof(sQuery),
                 "select groupid,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from people_cnt_ptz_params where groupid=%d;",
                 groupid);
        snprintf(buf, sizeof(buf), "select count(*) from people_cnt_ptz_params where groupid=%d;", groupid);

        ret = sqlite_query_record(hConn, buf, &cStmt, &cols);
        if (ret != 0 || cols == 0) {
            if (cStmt) {
                sqlite_clear_stmt(cStmt);
            }
            sqlite_disconn(hConn);
            FileUnlock(nFd, mode, &global_rwlock);
            return -1;
        }
        ret = sqlite_query_column(cStmt, 0, &rows);
        if (ret != 0 || rows == 0) {
            if (cStmt) {
                sqlite_clear_stmt(cStmt);
            }
            sqlite_disconn(hConn);
            FileUnlock(nFd, mode, &global_rwlock);
            return -1;
        }
        if (cStmt) {
            sqlite_clear_stmt(cStmt);
        }
        ret = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (ret != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            sqlite_disconn(hConn);
            FileUnlock(nFd, mode, &global_rwlock);
            return -1;
        }
        for (i = 0; i < rows && i < MAX_REAL_CAMERA; i++) {
            sqlite_query_column(hStmt, 0, &params->ptzActionParams[groupid][i].groupid);
            sqlite_query_column(hStmt, 1, &params->ptzActionParams[groupid][i].acto_ptz_channel);
            sqlite_query_column(hStmt, 2, &params->ptzActionParams[groupid][i].acto_fish_channel);
            sqlite_query_column(hStmt, 3, &params->ptzActionParams[groupid][i].acto_ptz_type);
            sqlite_query_column(hStmt, 4, &params->ptzActionParams[groupid][i].acto_ptz_preset);
            sqlite_query_column(hStmt, 5, &params->ptzActionParams[groupid][i].acto_ptz_patrol);
            sqlite_query_column(hStmt, 6, &params->ptzActionParams[groupid][i].acto_ptz_pattern);
            //msprintf("[david debug] groupid:%d channel:%d type:%d", params->ptzActionParams[groupid][i].groupid,
            //  params->ptzActionParams[groupid][i].acto_ptz_channel, params->ptzActionParams[groupid][i].acto_ptz_preset);
            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }

        sqlite_clear_stmt(hStmt);
    }

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_pos_event_init(const char *pDbFile, void *params, int cnt)
{
    if (!pDbFile || !params) {
        return -1;
    }
    SMART_EVENT *smartevent = (SMART_EVENT *)params;
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int i = 0;
    HSQLSTMT hStmt = 0;
    char sQuery[512];
    char cmd[512] = {0};
    int nColumnCnt = 0;

    snprintf(sQuery, sizeof(sQuery), "select id,tri_alarms,tri_chnout1_alarms,tri_chnout2_alarms,\
        buzzer_interval,email_interval,ptzaction_interval,alarmout_interval,whiteled_interval,\
        email_pic_enable,tri_channels_pic, tri_channels_ex,tri_audio_id,popup_interval,http_notification_interval\
        from pos_event");
    for (i = 0; i < cnt; i++) {
        snprintf(cmd, sizeof(cmd), "%s where id=%d;", sQuery, i);
        int nResult = sqlite_query_record(hConn, cmd, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
            continue;
        }
        sqlite_query_column(hStmt, 0, &smartevent[i].id);
        sqlite_query_column(hStmt, 1, (int *)&smartevent[i].tri_alarms);
        sqlite_query_column_text(hStmt, 2, smartevent[i].tri_chnout1_alarms, sizeof(smartevent[i].tri_chnout1_alarms));
        sqlite_query_column_text(hStmt, 3, smartevent[i].tri_chnout2_alarms, sizeof(smartevent[i].tri_chnout2_alarms));
        sqlite_query_column(hStmt, 4, &smartevent[i].buzzer_interval);
        sqlite_query_column(hStmt, 5, &smartevent[i].email_interval);
        sqlite_query_column(hStmt, 6, &smartevent[i].ptzaction_interval);
        sqlite_query_column(hStmt, 7, &smartevent[i].alarmout_interval);
        sqlite_query_column(hStmt, 8, &smartevent[i].whiteled_interval);
        sqlite_query_column(hStmt, 9, &smartevent[i].email_pic_enable);
        sqlite_query_column_text(hStmt, 10, smartevent[i].tri_channels_pic, sizeof(smartevent[i].tri_channels_pic));
        sqlite_query_column_text(hStmt, 11, smartevent[i].tri_channels_ex, sizeof(smartevent[i].tri_channels_ex));
        sqlite_query_column(hStmt, 12, &smartevent[i].tri_audio_id);
        sqlite_query_column(hStmt, 13, &smartevent[i].popup_interval);
        sqlite_query_column(hStmt, 14, &smartevent[i].http_notification_interval);
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }
    }

    if (hStmt) {
        sqlite_clear_stmt(hStmt);
    }
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_pos_schedules_init(const char *pDbFile, SCHEDULE_TYPE type, void *params, int cnt)
{
    if (!pDbFile) {
        return -1;
    }

    if (type < SCHE_EFFECTIVE || type >= SCHE_MAX) {
        return -1;
    }

    struct smart_event_schedule *Schedule = (struct smart_event_schedule *)params;
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, id = 0;;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[256];
    char table[64] = {0};
    char prefix[256] = {0};

    switch (type) {
        case SCHE_EFFECTIVE:
            snprintf(table, sizeof(table), "%s", "pos_effective_sche");
            break;

        case SCHE_AUDIO:
            snprintf(table, sizeof(table), "%s", "pos_audio_sche");
            break;

        case SCHE_EMAIL:
            snprintf(table, sizeof(table), "%s", "pos_email_sche");
            break;

        case SCHE_PTZ:
            snprintf(table, sizeof(table), "%s", "pos_ptz_sche");
            break;

        case SCHE_WHITELED:
            snprintf(table, sizeof(table), "%s", "pos_whiteled_sche");
            break;

        case SCHE_POPUP:
            snprintf(table, sizeof(table), "%s", "pos_popup_sche");
            break;
        case SCHE_HTTP:
            snprintf(table, sizeof(table), "%s", "pos_http_notification_sche");
            break;
        default:
            break;
    }
    snprintf(prefix, sizeof(prefix), "select week_id,plan_id,start_time,end_time,action_type from %s", table);
    HSQLSTMT hStmt = 0;
    for (id = 0; id < cnt; id++) {
        snprintf(sQuery, sizeof(sQuery), "%s where id=%d;", prefix, id);
        nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
            continue;
        }
        for (i = 0; i <= nLoopCnt; i++) {
            sqlite_query_column(hStmt, 0, &nWeekId);
            sqlite_query_column(hStmt, 1, &nPlanId);
            sqlite_query_column_text(hStmt, 2, Schedule[id].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(Schedule[id].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, Schedule[id].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(Schedule[id].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &Schedule[id].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }

        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }
    }

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_pos_setting_init(const char *pDbFile, void *params, int cnt)
{
    if (!pDbFile || !params) {
        return -1;
    }
    Db_POS_CONFIG *pos = (Db_POS_CONFIG *)params;
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int i = 0;
    HSQLSTMT hStmt = 0;
    char sQuery[512];
    char cmd[512] = {0};
    int nColumnCnt = 0;

    snprintf(sQuery, sizeof(sQuery), "select id,display_channel from pos_setting");
    for (i = 0; i < cnt; i++) {
        snprintf(cmd, sizeof(cmd), "%s where id=%d;", sQuery, i);
        int nResult = sqlite_query_record(hConn, cmd, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
            continue;
        }
        sqlite_query_column(hStmt, 0, &pos[i].id);
        sqlite_query_column(hStmt, 1, &pos[i].displayChannel);
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }
    }

    if (hStmt) {
        sqlite_clear_stmt(hStmt);
    }
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_regional_pcnt_event_init(const char *pDbFile, void *params, int cnt, int regionNo)
{
    if (!pDbFile || !params) {
        return -1;
    }

    if (regionNo < 0 || regionNo >= MAX_IPC_PCNT_REGION) {
        return -1;
    }
    
    SMART_EVENT *smartevent = (SMART_EVENT *)params;
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int i = 0;
    HSQLSTMT hStmt = 0;
    char sQuery[512];
    char cmd[512] = {0};
    int nColumnCnt = 0;
    char table[32] = {0};

    if (regionNo == 0) {
        snprintf(table, sizeof(table), "%s", "regional_pcnt_event");
    } else {
        snprintf(table, sizeof(table), "regional_pcnt%d_event", regionNo);
    }

    snprintf(sQuery, sizeof(sQuery), "select id,tri_alarms,tri_chnout1_alarms,tri_chnout2_alarms,\
        buzzer_interval,email_interval,ptzaction_interval,alarmout_interval,whiteled_interval,\
        email_pic_enable,tri_channels_pic, tri_channels_ex,tri_audio_id,popup_interval,http_notification_interval\
        from %s", table);
    for (i = 0; i < cnt; i++) {
        snprintf(cmd, sizeof(cmd), "%s where id=%d;", sQuery, i);
        int nResult = sqlite_query_record(hConn, cmd, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
            continue;
        }
        sqlite_query_column(hStmt, 0, &smartevent[i].id);
        sqlite_query_column(hStmt, 1, (int *)&smartevent[i].tri_alarms);
        sqlite_query_column_text(hStmt, 2, smartevent[i].tri_chnout1_alarms, sizeof(smartevent[i].tri_chnout1_alarms));
        sqlite_query_column_text(hStmt, 3, smartevent[i].tri_chnout2_alarms, sizeof(smartevent[i].tri_chnout2_alarms));
        sqlite_query_column(hStmt, 4, &smartevent[i].buzzer_interval);
        sqlite_query_column(hStmt, 5, &smartevent[i].email_interval);
        sqlite_query_column(hStmt, 6, &smartevent[i].ptzaction_interval);
        sqlite_query_column(hStmt, 7, &smartevent[i].alarmout_interval);
        sqlite_query_column(hStmt, 8, &smartevent[i].whiteled_interval);
        sqlite_query_column(hStmt, 9, &smartevent[i].email_pic_enable);
        sqlite_query_column_text(hStmt, 10, smartevent[i].tri_channels_pic, sizeof(smartevent[i].tri_channels_pic));
        sqlite_query_column_text(hStmt, 11, smartevent[i].tri_channels_ex, sizeof(smartevent[i].tri_channels_ex));
        sqlite_query_column(hStmt, 12, &smartevent[i].tri_audio_id);
        sqlite_query_column(hStmt, 13, &smartevent[i].popup_interval);
        sqlite_query_column(hStmt, 14, &smartevent[i].http_notification_interval);
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }
    }

    if (hStmt) {
        sqlite_clear_stmt(hStmt);
    }
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_regional_pcnt_schedules_init(const char *pDbFile, SCHEDULE_TYPE type, void *params, int cnt, int regionNo)
{
    if (!pDbFile) {
        return -1;
    }

    if (type < SCHE_EFFECTIVE || type >= SCHE_MAX) {
        return -1;
    }

    if (regionNo < 0 || regionNo >= MAX_IPC_PCNT_REGION) {
        return -1;
    }

    struct smart_event_schedule *Schedule = (struct smart_event_schedule *)params;
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, id = 0;;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[256];
    char table[64] = {0};
    char prefix[256] = {0};

    switch (type) {
        case SCHE_EFFECTIVE:
            if (regionNo == 0) {
                snprintf(table, sizeof(table), "%s", "regional_pcnt_effective_sche");
            } else {
                snprintf(table, sizeof(table), "regional_pcnt%d_effective_sche", regionNo);
            }
            break;

        case SCHE_AUDIO:
            if (regionNo == 0) {
                snprintf(table, sizeof(table), "%s", "regional_pcnt_audio_sche");
            } else {
                snprintf(table, sizeof(table), "regional_pcnt%d_audio_sche", regionNo);
            }
            break;

        case SCHE_EMAIL:
            if (regionNo == 0) {
                snprintf(table, sizeof(table), "%s", "regional_pcnt_email_sche");
            } else {
                snprintf(table, sizeof(table), "regional_pcnt%d_email_sche", regionNo);
            }
            break;

        case SCHE_PTZ:
            if (regionNo == 0) {
                snprintf(table, sizeof(table), "%s", "regional_pcnt_ptz_sche");
            } else {
                snprintf(table, sizeof(table), "regional_pcnt%d_ptz_sche", regionNo);
            }
            break;

        case SCHE_WHITELED:
            if (regionNo == 0) {
                snprintf(table, sizeof(table), "%s", "regional_pcnt_whiteled_sche");
            } else {
                snprintf(table, sizeof(table), "regional_pcnt%d_whiteled_sche", regionNo);
            }
            break;

        case SCHE_POPUP:
            if (regionNo == 0) {
                snprintf(table, sizeof(table), "%s", "regional_pcnt_popup_sche");
            } else {
                snprintf(table, sizeof(table), "regional_pcnt%d_popup_sche", regionNo);
            }
            break;
        case SCHE_HTTP:
            if (regionNo == 0) {
                snprintf(table, sizeof(table), "%s", "regional_pcnt_http_notification_sche");
            } else {
                snprintf(table, sizeof(table), "regional_pcnt%d_http_notification_sche", regionNo);
            }
            break;
        default:
            break;
    }
    snprintf(prefix, sizeof(prefix), "select week_id,plan_id,start_time,end_time,action_type from %s", table);
    HSQLSTMT hStmt = 0;
    for (id = 0; id < cnt; id++) {
        if (strstr(table, "http_notification")) {
            snprintf(sQuery, sizeof(sQuery), "%s where chnid=%d;", prefix, id);
        } else {
            snprintf(sQuery, sizeof(sQuery), "%s where id=%d;", prefix, id);
        }
        nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
            continue;
        }
        for (i = 0; i <= nLoopCnt; i++) {
            sqlite_query_column(hStmt, 0, &nWeekId);
            sqlite_query_column(hStmt, 1, &nPlanId);
            sqlite_query_column_text(hStmt, 2, Schedule[id].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(Schedule[id].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, Schedule[id].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(Schedule[id].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &Schedule[id].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }

        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }
    }

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_ipc_ptz_params_init(const char *pDbFile, int type, VCA_PTZ_ACTION_PARAMS *params, int maxChnCnt)
{
    if (!pDbFile) {
        return -1;
    }

    if (type < ALARMIO || type >= MAXEVT) {
        return -1;
    }
    
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[2048] = {0};
    int nColumnCnt = 0;
    int i = 0, chn_id = 0;

    do {
        switch (type) {
            case ALARMIO:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from alarmin_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case MOTION:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from motion_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case VIDEOLOSS:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from videoloss_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case REGION_EN:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from vca_regionein_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case REGION_EXIT:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from vca_regionexit_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case ADVANCED_MOT:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from vca_motion_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case TAMPER_DET:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from vca_tamper_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case LINE_CROSS:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from vca_linecross_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case LOITER:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from vca_loiter_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case HUMAN_DET:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from vca_human_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case PEOPLE_COUNT:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from vca_people_ptz_params where chn_id=%d;",
                         chn_id);
                break;
            case LEFTREMOVE:
                snprintf(sQuery, sizeof(sQuery),
                         "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from vca_object_leftremove_ptz_params where chn_id=%d;",
                         chn_id);
                break;

            case POS_EVT:
                snprintf(sQuery, sizeof(sQuery),
                         "select id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from pos_ptz_params where id=%d;",
                         chn_id);
                break;

            case REGIONAL_PEOPLE_CNT0:
                snprintf(sQuery, sizeof(sQuery),
                         "select id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from regional_pcnt_ptz_params where id=%d;",
                         chn_id);
                break;
            case REGIONAL_PEOPLE_CNT1:
                snprintf(sQuery, sizeof(sQuery),
                         "select id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from regional_pcnt1_ptz_params where id=%d;",
                         chn_id);
                break;
            case REGIONAL_PEOPLE_CNT2:
                snprintf(sQuery, sizeof(sQuery),
                         "select id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from regional_pcnt2_ptz_params where id=%d;",
                         chn_id);
                break;
            case REGIONAL_PEOPLE_CNT3:
                snprintf(sQuery, sizeof(sQuery),
                         "select id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from regional_pcnt3_ptz_params where id=%d;",
                         chn_id);
                break;
            default:
                break;
        }

        if (sQuery[0] == '\0') {
            sqlite_disconn(hConn);
            FileUnlock(nFd, mode, &global_rwlock);
            return -1;
        }
        int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            continue;
        }
        for (i = 0; i < nColumnCnt && i < MAX_REAL_CAMERA; i++) {
            sqlite_query_column(hStmt, 0, &params->ptzActionParams[chn_id][i].chn_id);

            sqlite_query_column(hStmt, 1, &params->ptzActionParams[chn_id][i].acto_ptz_channel);
            sqlite_query_column(hStmt, 2, &params->ptzActionParams[chn_id][i].acto_fish_channel);
            sqlite_query_column(hStmt, 3, &params->ptzActionParams[chn_id][i].acto_ptz_type);
            sqlite_query_column(hStmt, 4, &params->ptzActionParams[chn_id][i].acto_ptz_preset);
            sqlite_query_column(hStmt, 5, &params->ptzActionParams[chn_id][i].acto_ptz_patrol);
            sqlite_query_column(hStmt, 6, &params->ptzActionParams[chn_id][i].acto_ptz_pattern);
            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }
        sqlite_clear_stmt(hStmt);
    } while (++chn_id < maxChnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}


int read_ipc_led_params_init(const char *pDbFile, int type, WHITE_LED_PARAMS_EVTS *params, int maxChnCnt)
{
    if (!pDbFile) {
        return -1;
    }

    if (type < ALARMIO || type >= MAXEVT) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[2048] = {0};
    int nColumnCnt = 0;
    int i = 0, chn_id = 0;
    char table[64] = {0};

    do {
        switch (type) {
            case ALARMIO:
                snprintf(table, sizeof(table), "%s", AIN_WLED_PARAMS);
                break;
            case MOTION:
                snprintf(table, sizeof(table), "%s", MOT_WLED_PARAMS);
                break;
            case VIDEOLOSS:
                snprintf(table, sizeof(table), "%s", VDL_WLED_PARAMS);
                break;
            case REGION_EN:
                snprintf(table, sizeof(table), "%s", VREIN_WLED_PARAMS);
                break;
            case REGION_EXIT:
                snprintf(table, sizeof(table), "%s", VREEX_WLED_PARAMS);
                break;
            case ADVANCED_MOT:
                snprintf(table, sizeof(table), "%s", VMOT_WLED_PARAMS);
                break;
            case TAMPER_DET:
                snprintf(table, sizeof(table), "%s", VTEP_WLED_PARAMS);
                break;
            case LINE_CROSS:
                snprintf(table, sizeof(table), "%s", VLSS_WLED_PARAMS);
                break;
            case LOITER:
                snprintf(table, sizeof(table), "%s", VLER_WLED_PARAMS);
                break;
            case HUMAN_DET:
                snprintf(table, sizeof(table), "%s", VHMN_WLED_PARAMS);
                break;
            case PEOPLE_COUNT:
                snprintf(table, sizeof(table), "%s", VPPE_WLED_PARAMS);
                break;
            case LEFTREMOVE:
                snprintf(table, sizeof(table), "%s", VOBJ_WLED_PARAMS);
                break;
            case POS_EVT:
                snprintf(table, sizeof(table), "%s", POS_WLED_PARAMS);
                break;
            case REGIONAL_PEOPLE_CNT0:
                snprintf(table, sizeof(table), "%s", REGIONAL_PCNT_WLED_PARAMS);
                break;
            case REGIONAL_PEOPLE_CNT1:
                snprintf(table, sizeof(table), "%s", REGIONAL_PCNT1_WLED_PARAMS);
                break;
            case REGIONAL_PEOPLE_CNT2:
                snprintf(table, sizeof(table), "%s", REGIONAL_PCNT2_WLED_PARAMS);
                break;
            case REGIONAL_PEOPLE_CNT3:
                snprintf(table, sizeof(table), "%s", REGIONAL_PCNT3_WLED_PARAMS);
                break;
            default:
                break;
        }

        snprintf(sQuery, sizeof(sQuery),"select chnid,flash_mode,flash_time,acto_chn_id from %s where chnid=%d;",table, chn_id);
        if (sQuery[0] == '\0') {
            sqlite_disconn(hConn);
            FileUnlock(nFd, mode, &global_rwlock);
            return -1;
        }
        int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            continue;
        }
        for (i = 0; i < nColumnCnt && i < MAX_CAMERA; i++) {
            sqlite_query_column(hStmt, 0, &params->ledPrms[chn_id][i].chnid);
            sqlite_query_column(hStmt, 1, &params->ledPrms[chn_id][i].flash_mode);
            sqlite_query_column(hStmt, 2, &params->ledPrms[chn_id][i].flash_time);
            sqlite_query_column(hStmt, 3, &params->ledPrms[chn_id][i].acto_chn_id);

            if (sqlite_query_next(hStmt) != 0) {
                i++;
                break;
            }
        }
        sqlite_clear_stmt(hStmt);
    } while (++chn_id < maxChnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}


