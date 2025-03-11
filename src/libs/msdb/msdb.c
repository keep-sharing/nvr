#include "msdb.h"
#include "msdb_utils.h"

#if 0
static int msdb_log(const char *msg)
{
    char buff[2048] = {0};

    snprintf(buff, sizeof(buff), "%s\n", msg);
    FILE *fp = fopen("/mnt/nand/msdb_log.txt", "a+");
    if (!fp) {
        return -1;
    }
    fwrite(buff, 1, strlen(buff) + 1, fp);
    fclose(fp);

    return 0;
}
#endif


#if 0 // for debug cgi's db operations
#define MSDBLOG __FILE__, __LINE__
int msdblog(const char *file, int line, const char *msg, ...)
{
    char buf[2048] = {0};
    char test[2048] = {0};
    char szTime[20] = {0};
    va_list  va;
    time_t  iTime = time(NULL);
    struct tm *ptm = localtime((long *)&iTime);
    snprintf(szTime, 20, "%4d-%02d-%02d %02d:%02d:%02d", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour,
             ptm->tm_min, ptm->tm_sec);

    va_start(va, msg);
    vsnprintf(buf, sizeof(buf), (char *)msg, va);
    va_end(va);

    snprintf(test, sizeof(test), "%s %s:%d : %s\n", szTime, file, line, buf);

    FILE *fp = fopen("/mnt/nand/msdb_log.txt", "a+");
    if (!fp) {
        return -1;
    }
    fwrite(test, 1, strlen(test), fp);
    fflush(fp);
    fclose(fp);

    return 0;
}
#endif

int translate_pwd(char *dst, const char *src, int len)
{
    int a = 0, b = 0;

    while (b < len) {
        if (src[b] == '\'') {
            dst[a++] = '\'';
            dst[a++] = '\'';
            b++;
        } else {
            dst[a++] = src[b++];
        }
    }
    dst[a] = '\0';
    return (a);
}

int db_run_sql(const char *db_file, const char *sql)
{
    if (!db_file || !sql) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(db_file, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    sqlite_execute(hConn, mode, sql);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int db_query_sql(const char *db_file, const char *sql, char *str_result, int size)
{
    if (!db_file || !sql || !str_result || size <= 0) {
        return -1;
    }
    int i;
    char value[256] = {0};
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(db_file, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    int nColumnCnt = 0;
    int nResult = sqlite_query_record(hConn, sql, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    snprintf(str_result + strlen(str_result), size - strlen(str_result), "\n");
    for (i = 0; i < nColumnCnt; i++) {
        const char *name = sqlite_query_column_name(hConn, hStmt, i);
        if (name) {
            snprintf(str_result + strlen(str_result), size - strlen(str_result), "%-16s", name);
        }
    }
    do {
        //msprintf("nColumnCnt:%d", nColumnCnt);
        snprintf(str_result + strlen(str_result), size - strlen(str_result), "\n");
        for (i = 0; i < nColumnCnt; i++) {
            sqlite_query_column_text(hStmt, i, value, sizeof(value));
            if (i == nColumnCnt - 1) {
                snprintf(str_result + strlen(str_result), size - strlen(str_result), "%s", value);
            } else {
                snprintf(str_result + strlen(str_result), size - strlen(str_result), "%-16s", value);
            }
        }
    } while (sqlite_query_next(hStmt) == 0);
    snprintf(str_result + strlen(str_result), size - strlen(str_result), "\n");

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
static int read_schedule(const char *pDbFile, SMART_SCHEDULE *Schedule, char *db_table, int chn_id)
{
    if (!pDbFile || !Schedule) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[256] = {0};

    HSQLSTMT hStmt = 0;
    snprintf(sQuery, sizeof(sQuery), "select week_id,plan_id,start_time,end_time,action_type from %s where chn_id=%d;",
             db_table, chn_id);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, Schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, Schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

static int write_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, char *db_table, int chn_id)
{
    if (!pDbFile || !schedule || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "delete from %s where chn_id=%d;", db_table, chn_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into %s values(%d,%d,%d, '%s', '%s', %d);", db_table, chn_id, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

static int copy_comn_sches(const char *pDbFile, SMART_SCHEDULE *schedule, char *db_table, char *squery,
                           Uint64 changeFlag)
{
    if (!pDbFile || !schedule || !db_table || db_table[0] == '\0' || !changeFlag) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[256] = {0};
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    int k = 0;
    for (k = 0; k < MAX_REAL_CAMERA; k++) {
        if (!(changeFlag >> k & 0x01)) {
            continue;
        }

        snprintf(sExec, sizeof(sExec), "delete from %s where %s=%d;", db_table, squery, k);
        sqlite_execute(hConn, mode, sExec);
        int i = 0, j = 0;
        for (i = 0; i < MAX_DAY_NUM; i++) {
            for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
                if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                               schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                    continue;
                }
                snprintf(sExec, sizeof(sExec), "insert into %s values(%d,%d,%d, '%s', '%s', %d);", db_table, k, i, j,
                         schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                         schedule->schedule_day[i].schedule_item[j].action_type);
                sqlite_execute(hConn, mode, sExec);
            }
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    
    return 0;
}

static int copy_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, char *db_table, Uint64 changeFlag)
{
    return copy_comn_sches(pDbFile, schedule, db_table, "chn_id", changeFlag);
}

static int read_event(const char *pDbFile, SMART_EVENT *smartevent, char *db_table, int id)
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
    HSQLSTMT hStmt = 0;
    char sQuery[2048] = {0};
    int nColumnCnt = 0;

    snprintf(sQuery, sizeof(sQuery), "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,email_interval,\
		tri_chnout1_alarms,tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval,\
		email_pic_enable,tri_channels_pic,tri_audio_id,http_notification_interval from %s where id=%d;", db_table, id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, &smartevent->id);
    sqlite_query_column(hStmt, 1, &smartevent->enable);
    sqlite_query_column(hStmt, 2, (int *)&smartevent->tri_alarms);
    sqlite_query_column_text(hStmt, 3, smartevent->tri_channels_ex, sizeof(smartevent->tri_channels_ex));
    sqlite_query_column(hStmt, 4, &smartevent->buzzer_interval);
    sqlite_query_column(hStmt, 5, &smartevent->email_interval);
    sqlite_query_column_text(hStmt, 6, smartevent->tri_chnout1_alarms, sizeof(smartevent->tri_chnout1_alarms));
    sqlite_query_column_text(hStmt, 7, smartevent->tri_chnout2_alarms, sizeof(smartevent->tri_chnout2_alarms));
    sqlite_query_column(hStmt, 8, &smartevent->popup_interval);
    sqlite_query_column(hStmt, 9, &smartevent->ptzaction_interval);
    sqlite_query_column(hStmt, 10, &smartevent->alarmout_interval);
    sqlite_query_column(hStmt, 11, &smartevent->whiteled_interval);
    sqlite_query_column(hStmt, 12, &smartevent->email_pic_enable);
    sqlite_query_column_text(hStmt, 13, smartevent->tri_channels_pic, sizeof(smartevent->tri_channels_pic));
    sqlite_query_column(hStmt, 14, &smartevent->tri_audio_id);
    sqlite_query_column(hStmt, 15, &smartevent->http_notification_interval);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

static int read_events(const char *pDbFile, SMART_EVENT *smartevent, char *db_table, int count)
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
    HSQLSTMT hStmt = 0;
    char sQuery[2048] = {0};
    int nColumnCnt = 0;
    int i;

    snprintf(sQuery, sizeof(sQuery), "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,email_interval,\
		tri_chnout1_alarms,tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval,\
		email_pic_enable,tri_channels_pic,tri_audio_id,http_notification_interval from %s;", db_table);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < MAX_CAMERA && i < count; i++) {
        sqlite_query_column(hStmt, 0, &smartevent[i].id);
        sqlite_query_column(hStmt, 1, &smartevent[i].enable);
        sqlite_query_column(hStmt, 2, (int *)&smartevent[i].tri_alarms);
        sqlite_query_column_text(hStmt, 3, smartevent[i].tri_channels_ex, sizeof(smartevent[i].tri_channels_ex));
        sqlite_query_column(hStmt, 4, &smartevent[i].buzzer_interval);
        sqlite_query_column(hStmt, 5, &smartevent[i].email_interval);
        sqlite_query_column_text(hStmt, 6, smartevent[i].tri_chnout1_alarms, sizeof(smartevent[i].tri_chnout1_alarms));
        sqlite_query_column_text(hStmt, 7, smartevent[i].tri_chnout2_alarms, sizeof(smartevent[i].tri_chnout2_alarms));
        sqlite_query_column(hStmt, 8, &smartevent[i].popup_interval);
        sqlite_query_column(hStmt, 9, &smartevent[i].ptzaction_interval);
        sqlite_query_column(hStmt, 10, &smartevent[i].alarmout_interval);
        sqlite_query_column(hStmt, 11, &smartevent[i].whiteled_interval);
        sqlite_query_column(hStmt, 12, &smartevent[i].email_pic_enable);
        sqlite_query_column_text(hStmt, 13, smartevent[i].tri_channels_pic, sizeof(smartevent[i].tri_channels_pic));
        sqlite_query_column(hStmt, 14, &smartevent[i].tri_audio_id);
        sqlite_query_column(hStmt, 15, &smartevent[i].http_notification_interval);
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

static int write_event(const char *pDbFile, SMART_EVENT *smartevent, char *db_table)
{
    if (!pDbFile || !smartevent) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[2048] = {0};
    snprintf(sExec, sizeof(sExec), "update %s set enable=%d, tri_alarms=%d, tri_channels_ex='%s', buzzer_interval='%d', \
		email_interval='%d',tri_chnout1_alarms='%s',tri_chnout2_alarms='%s',popup_interval=%d,ptzaction_interval=%d,\
		alarmout_interval=%d,whiteled_interval=%d,email_pic_enable=%d,tri_channels_pic='%s',tri_audio_id=%d,\
		http_notification_interval=%d where id=%d;",
             db_table, smartevent->enable, smartevent->tri_alarms, smartevent->tri_channels_ex, smartevent->buzzer_interval,
             smartevent->email_interval, smartevent->tri_chnout1_alarms, smartevent->tri_chnout2_alarms, smartevent->popup_interval,
             smartevent->ptzaction_interval, smartevent->alarmout_interval, smartevent->whiteled_interval,
             smartevent->email_pic_enable, smartevent->tri_channels_pic, smartevent->tri_audio_id, 
             smartevent->http_notification_interval, smartevent->id);

    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

static int write_events(const char *pDbFile, SMART_EVENT *smartevent, char *db_table, int count, long long changeFlag)
{
    if (!pDbFile || !smartevent) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[2048] = {0};
    int i = 0;
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (i = 0; i < count && i < MAX_CAMERA; i++) {
        if (!(changeFlag >> i & 0x01)) {
            continue;
        }

        snprintf(sExec, sizeof(sExec), "update %s set enable=%d,tri_alarms=%d,tri_channels_ex='%s',buzzer_interval='%d', \
			email_interval='%d',tri_chnout1_alarms='%s',tri_chnout2_alarms='%s',popup_interval=%d,ptzaction_interval=%d,\
			alarmout_interval=%d,whiteled_interval=%d,email_pic_enable=%d,tri_channels_pic='%s',tri_audio_id=%d,\
			http_notification_interval=%d where id=%d;",
                 db_table, smartevent[i].enable, smartevent[i].tri_alarms, smartevent[i].tri_channels_ex, smartevent[i].buzzer_interval,
                 smartevent[i].email_interval, smartevent[i].tri_chnout1_alarms, smartevent[i].tri_chnout2_alarms,
                 smartevent[i].popup_interval, smartevent[i].ptzaction_interval, smartevent[i].alarmout_interval,
                 smartevent[i].whiteled_interval, smartevent[i].email_pic_enable, smartevent[i].tri_channels_pic,
                 smartevent[i].tri_audio_id, smartevent[i].http_notification_interval, smartevent[i].id);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

//tri_channels_ex 不参与复制
static int copy_events(const char *pDbFile, SMART_EVENT *smartevent, char *db_table, Uint64 changeFlag)
{
    if (!pDbFile || !smartevent || !db_table || db_table[0] == '\0' || !changeFlag) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[2048] = {0};
    int i = 0;
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (i = 0; i < MAX_CAMERA; i++) {
        if (!(changeFlag >> i & 0x01)) {
            continue;
        }

        snprintf(sExec, sizeof(sExec), "update %s set enable=%d,tri_alarms=%d,buzzer_interval='%d',\
            email_interval='%d', tri_chnout1_alarms='%s',tri_chnout2_alarms='%s',popup_interval=%d,\
            ptzaction_interval=%d,alarmout_interval=%d,whiteled_interval=%d,email_pic_enable=%d,\
            tri_audio_id=%d,http_notification_interval=%d where id=%d;",
            db_table, smartevent->enable, smartevent->tri_alarms, smartevent->buzzer_interval, smartevent->email_interval,
            smartevent->tri_chnout1_alarms, smartevent->tri_chnout2_alarms, smartevent->popup_interval, 
            smartevent->ptzaction_interval, smartevent->alarmout_interval, smartevent->whiteled_interval,
            smartevent->email_pic_enable, smartevent->tri_audio_id, smartevent->http_notification_interval, i);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}


static int read_event_name(const char *pDbFile, struct alarm_chn_name *param, char *db_table, int id)
{
    if (!pDbFile || !param) {
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

    snprintf(sQuery, sizeof(sQuery), "select id,name from %s where id=%d;", db_table, id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, &param->chnid);
    sqlite_query_column_text(hStmt, 1, param->name, sizeof(param->name));

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

static int write_event_name(const char *pDbFile, struct alarm_chn_name *param, char *db_table)
{
    if (!pDbFile || !param) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[2048] = {0};
    snprintf(sExec, sizeof(sExec), "update %s set name='%s' where id=%d;", db_table, param->name, param->chnid);

    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

static int read_action_ptz_params(const char *pDbFile, PTZ_ACTION_PARAMS ptzActionParams[], char *db_table, int chn_id,
                                  int *cnt)
{
    if (!pDbFile || !ptzActionParams || chn_id < 0) {
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
    int i = 0;

    snprintf(sQuery, sizeof(sQuery),
             "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from %s where chn_id=%d;",
             db_table, chn_id);
    if (sQuery[0] == '\0') {
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *cnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < MAX_REAL_CAMERA; i++) {
        sqlite_query_column(hStmt, 0, &ptzActionParams[i].chn_id);
        sqlite_query_column(hStmt, 1, &ptzActionParams[i].acto_ptz_channel);
        sqlite_query_column(hStmt, 2, &ptzActionParams[i].acto_fish_channel);
        sqlite_query_column(hStmt, 3, &ptzActionParams[i].acto_ptz_type);
        sqlite_query_column(hStmt, 4, &ptzActionParams[i].acto_ptz_preset);
        sqlite_query_column(hStmt, 5, &ptzActionParams[i].acto_ptz_patrol);
        sqlite_query_column(hStmt, 6, &ptzActionParams[i].acto_ptz_pattern);

        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    *cnt  = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

static int write_action_ptz_params(const char *pDbFile, struct ptz_action_params *ptzActionParams, char *db_table)
{
    if (!pDbFile || !ptzActionParams) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    char sExec[2048] = {0};
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    snprintf(sExec, sizeof(sExec), "delete from %s where chn_id=%d and acto_ptz_channel=%d;", db_table,
             ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
    sqlite_execute(hConn, mode, sExec);
    snprintf(sExec, sizeof(sExec),
             "insert into %s(chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern) values(%d,%d,%d,%d,%d,%d,%d);",
             db_table, ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel, ptzActionParams->acto_fish_channel,
             ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset, ptzActionParams->acto_ptz_patrol,
             ptzActionParams->acto_ptz_pattern);
    if (sExec[0] == '\0') {
        sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

static int update_action_ptz_params(const char *pDbFile, struct ptz_action_params *ptzActionParams, char *db_table,
                                    int chn_id)
{
    if (!pDbFile || !ptzActionParams) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    char sExec[2048] = {0};
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    snprintf(sExec, sizeof(sExec),
             "update %s set acto_fish_channel=%d,acto_ptz_type=%d,acto_ptz_preset=%d,acto_ptz_patrol=%d,acto_ptz_pattern=%d,acto_ptz_channel=%d where chn_id=%d and acto_ptz_channel=%d;",
             db_table, ptzActionParams->acto_fish_channel, ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset,
             ptzActionParams->acto_ptz_patrol, ptzActionParams->acto_ptz_pattern, ptzActionParams->acto_ptz_channel,
             ptzActionParams->chn_id, chn_id);

    if (sExec[0] == '\0') {
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

static int delete_action_ptz_params(const char *pDbFile, struct ptz_action_params *ptzActionParams, char *db_table)
{
    if (!pDbFile || !ptzActionParams) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    char sExec[2048] = {0};
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    snprintf(sExec, sizeof(sExec), "delete from %s where chn_id=%d and acto_ptz_channel=%d;", db_table,
             ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_osd(const char *pDbFile, struct osd *osd, int chn_id)
{
    if (!pDbFile || !osd || chn_id < 0) {
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
    char sQuery[512] = {0};
    int nColumnCnt = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select id, name, show_name, show_date, show_week, date_format, time_format, date_pos_x,date_pos_y,name_pos_x,name_pos_y,alpha from osd where id=%d;",
             chn_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, &osd->id);
    sqlite_query_column_text(hStmt, 1, osd->name, sizeof(osd->name));
    sqlite_query_column(hStmt, 2, &osd->show_name);
    sqlite_query_column(hStmt, 3, &osd->show_date);
    sqlite_query_column(hStmt, 4, &osd->show_week);
    sqlite_query_column(hStmt, 5, &osd->date_format);
    sqlite_query_column(hStmt, 6, &osd->time_format);
    sqlite_query_column_double(hStmt, 7, &osd->date_pos_x);
    sqlite_query_column_double(hStmt, 8, &osd->date_pos_y);
    sqlite_query_column_double(hStmt, 9, &osd->name_pos_x);
    sqlite_query_column_double(hStmt, 10, &osd->name_pos_y);
    sqlite_query_column(hStmt, 11, &osd->alpha);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_osd_name(const char *pDbFile, char *name, int chn_id)
{
    if (!pDbFile || !name || chn_id < 0) {
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
    char sQuery[512] = {0};
    int nColumnCnt = 0;
    snprintf(sQuery, sizeof(sQuery), "select name from osd where id=%d;", chn_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column_text(hStmt, 0, name, 64);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_osd(const char *pDbFile, struct osd *osd)
{
    if (!pDbFile || !osd) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    //david.milesight
    char osdNameStr[64] = {0};
    char osdNameSqlStr[128] = {0};
    int i = 0, j = 0;
    snprintf(osdNameStr, sizeof(osdNameStr), "%s", osd->name);
    for (i = 0 ; i < strlen(osdNameStr); i++) {
        if (osdNameStr[i] == '\'') {
            osdNameSqlStr[j++] = '\'';
            osdNameSqlStr[j++] = '\'';
        } else {
            osdNameSqlStr[j++] = osdNameStr[i];
        }
    }
    osdNameSqlStr[j] = '\0';
    snprintf(sExec, sizeof(sExec),
             "update osd set name = '%s', show_name=%d, show_date=%d,show_week=%d,date_format=%d,time_format=%d,date_pos_x=%f,date_pos_y=%f,name_pos_x=%f,name_pos_y=%f,alpha=%d where id=%d;",
             osdNameSqlStr /*osd->name*/, osd->show_name, osd->show_date, osd->show_week, osd->date_format, osd->time_format,
             osd->date_pos_x, osd->date_pos_y, osd->name_pos_x, osd->name_pos_y, osd->alpha, osd->id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int insert_osd(const char *pDbFile, struct osd *osd)
{
    if (!pDbFile || !osd) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[1024] = {0};
    snprintf(sExec, sizeof(sExec),
             "insert into osd(id,name,show_name,show_date,show_week,date_format,time_format,date_pos_x,date_pos_y,name_pos_x,name_pos_y,alpha) values(%d,'%s',%d,%d,%d,%d,%d,%f,%f,%f,%f,%d);",
             osd->id, osd->name, osd->show_name, osd->show_date, osd->show_week, osd->date_format, osd->time_format, osd->date_pos_x,
             osd->date_pos_y, osd->name_pos_x, osd->name_pos_y, osd->alpha);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}


int read_osds(const char *pDbFile, struct osd osds[], int *pCount)
{
    if (!pDbFile || !osds) {
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
    int nColumnCnt = 0;
    char sQuery[512] = {0};
    snprintf(sQuery, sizeof(sQuery),
             "select id, name, show_name, show_date, show_week, date_format, time_format, date_pos_x,date_pos_y,name_pos_x,name_pos_y,alpha from osd order by(id);");
    int i = 0;
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *pCount = i;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    for (i = 0; i < MAX_CAMERA; i++) {
        sqlite_query_column(hStmt, 0, &osds[i].id);
        sqlite_query_column_text(hStmt, 1, osds[i].name, sizeof(osds[i].name));
        sqlite_query_column(hStmt, 2, &osds[i].show_name);
        sqlite_query_column(hStmt, 3, &osds[i].show_date);
        sqlite_query_column(hStmt, 4, &osds[i].show_week);
        sqlite_query_column(hStmt, 5, &osds[i].date_format);
        sqlite_query_column(hStmt, 6, &osds[i].time_format);
        sqlite_query_column_double(hStmt, 7, &osds[i].date_pos_x);
        sqlite_query_column_double(hStmt, 8, &osds[i].date_pos_y);
        sqlite_query_column_double(hStmt, 9, &osds[i].name_pos_x);
        sqlite_query_column_double(hStmt, 10, &osds[i].name_pos_y);
        sqlite_query_column(hStmt, 11, &osds[i].alpha);
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    *pCount = i;
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_motion(const char *pDbFile, struct motion *move, int id)
{
    if (!pDbFile || !move) {
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
    snprintf(sQuery, sizeof(sQuery), "select id,enable,sensitivity,tri_channels,tri_alarms,motion_table,\
		buzzer_interval,ipc_sched_enable,tri_channels_ex,email_enable,email_buzzer_interval,tri_chnout1_alarms,\
		tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval,email_pic_enable,\
		tri_channels_pic,tri_audio_id,http_notification_interval from motion where id=%d;", id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, &move->id);
    sqlite_query_column(hStmt, 1, &move->enable);
    sqlite_query_column(hStmt, 2, &move->sensitivity);
    sqlite_query_column(hStmt, 3, (int *)&move->tri_channels);
    sqlite_query_column(hStmt, 4, (int *)&move->tri_alarms);
    sqlite_query_column_text(hStmt, 5, (char *)move->motion_table, sizeof(move->motion_table));
    sqlite_query_column(hStmt, 6, &move->buzzer_interval);
    sqlite_query_column(hStmt, 7, &move->ipc_sched_enable);
    sqlite_query_column_text(hStmt, 8, move->tri_channels_ex, sizeof(move->tri_channels_ex));
    sqlite_query_column(hStmt, 9, &move->email_enable);
    sqlite_query_column(hStmt, 10, &move->email_buzzer_interval);
    sqlite_query_column_text(hStmt, 11, move->tri_chnout1_alarms, sizeof(move->tri_chnout1_alarms));
    sqlite_query_column_text(hStmt, 12, move->tri_chnout2_alarms, sizeof(move->tri_chnout2_alarms));
    sqlite_query_column(hStmt, 13, &move->popup_interval);
    sqlite_query_column(hStmt, 14, &move->ptzaction_interval);
    sqlite_query_column(hStmt, 15, &move->alarmout_interval);
    sqlite_query_column(hStmt, 16, &move->whiteled_interval);
    sqlite_query_column(hStmt, 17, &move->email_pic_enable);
    sqlite_query_column_text(hStmt, 18, move->tri_channels_pic, sizeof(move->tri_channels_pic));
    sqlite_query_column(hStmt, 19, &move->tri_audio_id);
    sqlite_query_column(hStmt, 20, &move->http_notification_interval);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_motions(const char *pDbFile, struct motion moves[], int *pCnt)
{
    if (!pDbFile || !moves) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery), "select id,enable,sensitivity,tri_channels,tri_alarms, motion_table,\
		buzzer_interval,ipc_sched_enable,tri_channels_ex,email_enable,email_buzzer_interval,tri_chnout1_alarms,\
		tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval,email_pic_enable,\
		tri_channels_pic,tri_audio_id,http_notification_interval from motion;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *pCnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    for (i = 0; i < MAX_CAMERA; i++) {
        sqlite_query_column(hStmt, 0, &moves[i].id);
        sqlite_query_column(hStmt, 1, &moves[i].enable);
        sqlite_query_column(hStmt, 2, &moves[i].sensitivity);
        sqlite_query_column(hStmt, 3, (int *)&moves[i].tri_channels);
        sqlite_query_column(hStmt, 4, (int *)&moves[i].tri_alarms);
        sqlite_query_column_text(hStmt, 5, (char *)moves[i].motion_table, sizeof(moves[i].motion_table));
        sqlite_query_column(hStmt, 6, &moves[i].buzzer_interval);
        sqlite_query_column(hStmt, 7, &moves[i].ipc_sched_enable);
        sqlite_query_column_text(hStmt, 8, moves[i].tri_channels_ex, sizeof(moves[i].tri_channels_ex));
        sqlite_query_column(hStmt, 9, &moves[i].email_enable);
        sqlite_query_column(hStmt, 10, &moves[i].email_buzzer_interval);
        sqlite_query_column_text(hStmt, 11, moves[i].tri_chnout1_alarms, sizeof(moves[i].tri_chnout1_alarms));
        sqlite_query_column_text(hStmt, 12, moves[i].tri_chnout2_alarms, sizeof(moves[i].tri_chnout2_alarms));
        sqlite_query_column(hStmt, 13, &moves[i].popup_interval);
        sqlite_query_column(hStmt, 14, &moves[i].ptzaction_interval);
        sqlite_query_column(hStmt, 15, &moves[i].alarmout_interval);
        sqlite_query_column(hStmt, 16, &moves[i].whiteled_interval);
        sqlite_query_column(hStmt, 17, &moves[i].email_pic_enable);
        sqlite_query_column_text(hStmt, 18, moves[i].tri_channels_pic, sizeof(moves[i].tri_channels_pic));
        sqlite_query_column(hStmt, 19, &moves[i].tri_audio_id);
        sqlite_query_column(hStmt, 20, &moves[i].http_notification_interval);
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    *pCnt = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int insert_motion(const char *pDbFile, struct motion *move)
{
    if (!pDbFile || !move) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[2048] = {0};
    move->motion_table[sizeof(move->motion_table) - 1] = '\0';
    snprintf(sExec, sizeof(sExec), "insert into motion(id,enable,sensitivity,tri_channels,tri_alarms,motion_table,\
		buzzer_interval,ipc_sched_enable,tri_channels_ex,email_enable,email_buzzer_interval,tri_chnout1_alarms,\
		tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval,email_pic_enable,\
		tri_channels_pic,tri_audio_id,http_notification_interval) \
		values(%d,%d,%d,%d,%d,'%s',%d,%d,'%s',%d,%d,'%s','%s',%d, %d, %d, %d, %d, '%s', %d, %d);",
             move->id, move->enable, move->sensitivity, move->tri_channels, move->tri_alarms, move->motion_table,
             move->buzzer_interval, move->ipc_sched_enable, move->tri_channels_ex, move->email_enable, move->email_buzzer_interval,
             move->tri_chnout1_alarms, move->tri_chnout1_alarms, move->popup_interval, move->ptzaction_interval,
             move->alarmout_interval, move->whiteled_interval, move->email_pic_enable, move->tri_channels_pic,
             move->tri_audio_id, move->http_notification_interval);

    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_motion(const char *pDbFile, struct motion *move)
{
    if (!pDbFile || !move) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[2048] = {0};
    move->motion_table[sizeof(move->motion_table) - 1] = '\0';
    snprintf(sExec, sizeof(sExec), "update motion set enable=%d, sensitivity=%d, tri_channels=%u, tri_alarms=%d, \
		motion_table='%s',buzzer_interval=%d,ipc_sched_enable=%d,tri_channels_ex='%s',email_enable=%d,email_buzzer_interval=%d,\
		tri_chnout1_alarms='%s',tri_chnout2_alarms='%s',popup_interval=%d,ptzaction_interval=%d,alarmout_interval=%d,\
		whiteled_interval=%d,email_pic_enable=%d,tri_channels_pic='%s',tri_audio_id=%d,http_notification_interval=%d where id=%d;",
             move->enable, move->sensitivity, move->tri_channels, move->tri_alarms, move->motion_table, move->buzzer_interval,
             move->ipc_sched_enable, move->tri_channels_ex, move->email_enable, move->email_buzzer_interval,
             move->tri_chnout1_alarms, move->tri_chnout2_alarms, move->popup_interval, move->ptzaction_interval,
             move->alarmout_interval, move->whiteled_interval, move->email_pic_enable, move->tri_channels_pic,
             move->tri_audio_id, move->http_notification_interval, move->id);

    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_motions(const char *pDbFile, struct motion *moves, int cnt, long long changeFlag)
{
    if (!pDbFile || !moves) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    struct motion *move = NULL;
    int i = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[2048] = {0};
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (i = 0; i < MAX_CAMERA && i < cnt; i++) {
        if (!(changeFlag >> i & 0x01)) {
            continue;
        }
        move = moves + i;
        move->motion_table[sizeof(move->motion_table) - 1] = '\0';
        snprintf(sExec, sizeof(sExec), "update motion set enable=%d, sensitivity=%d, tri_channels=%u, tri_alarms=%d, \
			motion_table='%s',buzzer_interval=%d,ipc_sched_enable=%d,tri_channels_ex='%s',email_enable=%d,email_buzzer_interval=%d,\
			tri_chnout1_alarms='%s',tri_chnout2_alarms='%s',popup_interval=%d,ptzaction_interval=%d,alarmout_interval=%d,\
			whiteled_interval=%d,email_pic_enable=%d,tri_channels_pic='%s',tri_audio_id=%d,http_notification_interval=%d where id=%d;",
                 move->enable, move->sensitivity, move->tri_channels, move->tri_alarms, move->motion_table, move->buzzer_interval,
                 move->ipc_sched_enable, move->tri_channels_ex, move->email_enable, move->email_buzzer_interval,
                 move->tri_chnout1_alarms, move->tri_chnout2_alarms, move->popup_interval, move->ptzaction_interval,
                 move->alarmout_interval, move->whiteled_interval, move->email_pic_enable, move->tri_channels_pic,
                 move->tri_audio_id, move->http_notification_interval, move->id);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

//tri_channels_ex 不参与复制
int copy_motions(const char *pDbFile, struct motion *move, Uint64 changeFlag)
{
    if (!pDbFile || !move) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    int i = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[2048] = {0};
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (i = 0; i < MAX_CAMERA; i++) {
        if (!(changeFlag >> i & 0x01)) {
            continue;
        }
        move->motion_table[sizeof(move->motion_table) - 1] = '\0';
        snprintf(sExec, sizeof(sExec), "update motion set enable=%d, sensitivity=%d, tri_alarms=%d, motion_table='%s',\
			buzzer_interval=%d,ipc_sched_enable=%d,email_enable=%d,email_buzzer_interval=%d,tri_chnout1_alarms='%s',\
			tri_chnout2_alarms='%s',popup_interval=%d,ptzaction_interval=%d,alarmout_interval=%d,whiteled_interval=%d,\
			email_pic_enable=%d, tri_audio_id=%d, http_notification_interval=%d where id=%d;",
                 move->enable, move->sensitivity, move->tri_alarms, move->motion_table, move->buzzer_interval, move->ipc_sched_enable,
                 move->email_enable, move->email_buzzer_interval, move->tri_chnout1_alarms, move->tri_chnout2_alarms,
                 move->popup_interval, move->ptzaction_interval, move->alarmout_interval, move->whiteled_interval,
                 move->email_pic_enable, move->tri_audio_id, move->http_notification_interval, i);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}


int read_motion_schedule(const char *pDbFile, struct motion_schedule  *motionSchedule, int chn_id)
{
    if (!pDbFile || !motionSchedule) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select week_id,plan_id,start_time,end_time,action_type from motion_schedule where chn_id=%d;", chn_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, motionSchedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(motionSchedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, motionSchedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(motionSchedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(motionSchedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int delete_motion_schedule(const char *pDbFile, int chn_id)
{
    if (!pDbFile || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "delete from motion_schedule where chn_id=%d;", chn_id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_motion_schedule(const char *pDbFile, struct motion_schedule *schedule, int chn_id)
{
    if (!pDbFile || !schedule || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "delete from motion_schedule where chn_id=%d;", chn_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into motion_schedule values(%d,%d,%d, '%s', '%s', %d);", chn_id, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_motion_audible_schedule(const char *pDbFile, struct motion_schedule  *motionSchedule, int chn_id)
{
    if (!pDbFile || !motionSchedule) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select week_id,plan_id,start_time,end_time,action_type from motion_audible_schedule where chn_id=%d;", chn_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, motionSchedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(motionSchedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, motionSchedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(motionSchedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(motionSchedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int delete_motion_audible_schedule(const char *pDbFile, int chn_id)
{
    if (!pDbFile || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "delete from motion_audible_schedule where chn_id=%d;", chn_id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_motion_audible_schedule(const char *pDbFile, struct motion_schedule *schedule, int chn_id)
{
    if (!pDbFile || !schedule || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "delete from motion_audible_schedule where chn_id=%d;", chn_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into motion_audible_schedule values(%d,%d,%d, '%s', '%s', %d);", chn_id, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_motion_audible_schedules(const char *pDbFile, struct motion_schedule *schedule, long long changeFlag)
{
    if (!pDbFile || !schedule) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[256] = {0};
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);

    int i = 0, j = 0, k = 0;

    for (k = 0; k < MAX_CAMERA; k++) {
        if (!(changeFlag >> k & 0x01)) {
            continue;
        }

        snprintf(sExec, sizeof(sExec), "delete from motion_audible_schedule where chn_id=%d;", k);
        sqlite_execute(hConn, mode, sExec);
        for (i = 0; i < MAX_DAY_NUM; i++) {
            for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
                if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                               schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                    continue;
                }
                snprintf(sExec, sizeof(sExec), "insert into motion_audible_schedule values(%d,%d,%d, '%s', '%s', %d);", k, i, j,
                         schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                         schedule->schedule_day[i].schedule_item[j].action_type);
                sqlite_execute(hConn, mode, sExec);
            }
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_alarm_out(const char *pDbFile, struct alarm_out *alarm, int alarm_id)
{
    if (!pDbFile || !alarm || alarm_id < 0) {
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
    int nColumnCnt = 0;
    char sQuery[256] = {0};
    snprintf(sQuery, sizeof(sQuery), "select id,name,type,enable,duration_time from alarm_out where id=%d;", alarm_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, &alarm->id);
    sqlite_query_column_text(hStmt, 1, alarm->name, sizeof(alarm->name));
    sqlite_query_column(hStmt, 2, &alarm->type);
    sqlite_query_column(hStmt, 3, &alarm->enable);
    sqlite_query_column(hStmt, 4, &alarm->duration_time);
//    sqlite_query_column(hStmt, 5, &alarm->buzzer_interval);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_alarm_outs(const char *pDbFile, struct alarm_out alarms[], int *pCount)
{
    if (!pDbFile || !alarms) {
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
    int nColumnCnt = 0;
    char sQuery[256] = {0};
    snprintf(sQuery, sizeof(sQuery), "select id,name,type,enable,duration_time from alarm_out order by(id);");
    int i = 0;
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *pCount = i;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    for (i = 0; i < MAX_ALARM_OUT; i++) {
        sqlite_query_column(hStmt, 0, &alarms[i].id);
        sqlite_query_column_text(hStmt, 1, alarms[i].name, sizeof(alarms[i].name));
        sqlite_query_column(hStmt, 2, &alarms[i].type);
        sqlite_query_column(hStmt, 3, &alarms[i].enable);
        sqlite_query_column(hStmt, 4, &alarms[i].duration_time);
//        sqlite_query_column(hStmt, 4, &alarms[i].buzzer_interval);
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    *pCount = i;
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_alarm_out(const char *pDbFile, struct alarm_out *alarm)
{
    if (!pDbFile || !alarm) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec), "update alarm_out set name='%s',type=%d,enable=%d,duration_time=%d where id=%d;",
             alarm->name, alarm->type, alarm->enable, alarm->duration_time, alarm->id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_alarm_outs(const char *pDbFile, struct alarm_out *alarm, int count, long long changeFlag)
{
    if (!pDbFile || !alarm) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[512] = {0};
    int i = 0;
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (i = 0; i < count && i < MAX_ALARM_OUT; i++) {
        if (!(changeFlag >> i & 0x01)) {
            continue;
        }
        snprintf(sExec, sizeof(sExec), "update alarm_out set name='%s',type=%d,enable=%d,duration_time=%d where id=%d;",
                 alarm[i].name, alarm[i].type, alarm[i].enable, alarm[i].duration_time, alarm[i].id);
        sqlite_execute(hConn, mode, sExec);

    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}


int read_alarm_out_schedule(const char *pDbFile, struct alarm_out_schedule *schedule, int alarm_id)
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select week_id,plan_id,start_time,end_time,action_type from alarm_out_schedule where alarm_id=%d;", alarm_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int nWeekId = 0;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(schedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_alarm_out_schedule(const char *pDbFile, struct alarm_out_schedule *schedule, int alarm_id)
{
    if (!pDbFile || !schedule || alarm_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "delete from alarm_out_schedule where alarm_id=%d;", alarm_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into alarm_out_schedule values(%d,%d,%d,'%s','%s',%d);", alarm_id, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_alarm_out_schedules(const char *pDbFile, struct alarm_out_schedule *schedule, long long changeFlag)
{
    if (!pDbFile || !schedule) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[512] = {0};
    char db_table[64] = {0};
    int i = 0, j = 0, k = 0;
    snprintf(db_table, sizeof(db_table), "%s", "alarm_out_schedule");
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (k = 0; k < MAX_ALARM_OUT; k++) {
        if (!(changeFlag >> k & 0x01)) {
            continue;
        }

        snprintf(sExec, sizeof(sExec), "delete from %s where alarm_id=%d;", db_table, k);
        sqlite_execute(hConn, mode, sExec);
        for (i = 0; i < MAX_DAY_NUM; i++) {
            for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
                if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                               schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                    continue;
                }
                snprintf(sExec, sizeof(sExec), "insert into %s values(%d,%d,%d, '%s', '%s', %d);", db_table, k, i, j,
                         schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                         schedule->schedule_day[i].schedule_item[j].action_type);
                sqlite_execute(hConn, mode, sExec);
            }
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_video_lost(const char *pDbFile, struct video_loss *videolost, int chn_id)
{
    if (!pDbFile || !videolost) {
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
    char sQuery[512] = {0};
    int nColumnCnt = 0;

    snprintf(sQuery, sizeof(sQuery), "select id,enable,tri_alarms,buzzer_interval,email_enable,email_buzzer_interval,\
        tri_chnout1_alarms,tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval, \
        tri_audio_id,http_notification_interval from video_loss where id=%d;", chn_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &videolost->id);
    sqlite_query_column(hStmt, 1, &videolost->enable);
    sqlite_query_column(hStmt, 2, (int *)&videolost->tri_alarms);
    sqlite_query_column(hStmt, 3, &videolost->buzzer_interval);
    sqlite_query_column(hStmt, 4, (int *)&videolost->email_enable);
    sqlite_query_column(hStmt, 5, &videolost->email_buzzer_interval);
    sqlite_query_column_text(hStmt, 6, videolost->tri_chnout1_alarms, sizeof(videolost->tri_chnout1_alarms));
    sqlite_query_column_text(hStmt, 7, videolost->tri_chnout2_alarms, sizeof(videolost->tri_chnout2_alarms));
    sqlite_query_column(hStmt, 8, &videolost->popup_interval);
    sqlite_query_column(hStmt, 9, &videolost->ptzaction_interval);
    sqlite_query_column(hStmt, 10, &videolost->alarmout_interval);
    sqlite_query_column(hStmt, 11, &videolost->whiteled_interval);
    sqlite_query_column(hStmt, 12, &videolost->tri_audio_id);
    sqlite_query_column(hStmt, 13, &videolost->http_notification_interval);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_video_losts(const char *pDbFile, struct video_loss videolosts[], int *pCnt)
{
    if (!pDbFile || !videolosts) {
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
    char sQuery[512] = {0};
    int nColumnCnt = 0;
    int i = 0;
    snprintf(sQuery, sizeof(sQuery), "select id,enable,tri_alarms,buzzer_interval,email_enable,email_buzzer_interval,\
        tri_chnout1_alarms,tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval, \
        tri_audio_id,http_notification_interval from video_loss;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *pCnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    for (i = 0; i < MAX_CAMERA; i++) {
        sqlite_query_column(hStmt, 0, &videolosts[i].id);
        sqlite_query_column(hStmt, 1, &videolosts[i].enable);
        sqlite_query_column(hStmt, 2, (int *)&videolosts[i].tri_alarms);
        sqlite_query_column(hStmt, 3, &videolosts[i].buzzer_interval);
        sqlite_query_column(hStmt, 4, (int *)&videolosts[i].email_enable);
        sqlite_query_column(hStmt, 5, &videolosts[i].email_buzzer_interval);
        sqlite_query_column_text(hStmt, 6, videolosts[i].tri_chnout1_alarms, sizeof(videolosts[i].tri_chnout1_alarms));
        sqlite_query_column_text(hStmt, 7, videolosts[i].tri_chnout2_alarms, sizeof(videolosts[i].tri_chnout2_alarms));
        sqlite_query_column(hStmt, 8, &videolosts[i].popup_interval);
        sqlite_query_column(hStmt, 9, &videolosts[i].ptzaction_interval);
        sqlite_query_column(hStmt, 10, &videolosts[i].alarmout_interval);
        sqlite_query_column(hStmt, 11, &videolosts[i].whiteled_interval);
        sqlite_query_column(hStmt, 12, &videolosts[i].tri_audio_id);
        sqlite_query_column(hStmt, 13, &videolosts[i].http_notification_interval);
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    *pCnt = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_video_lost(const char *pDbFile, struct video_loss *videolost)
{
    if (!pDbFile || !videolost) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec), "update video_loss set enable=%d,tri_alarms=%d,buzzer_interval=%d,email_enable=%d,\
		email_buzzer_interval=%d,tri_chnout1_alarms='%s',tri_chnout2_alarms='%s',popup_interval=%d,ptzaction_interval=%d,\
		alarmout_interval=%d,whiteled_interval=%d,tri_audio_id=%d, http_notification_interval=%d where id=%d;",
             videolost->enable, videolost->tri_alarms, videolost->buzzer_interval, videolost->email_enable,
             videolost->email_buzzer_interval, videolost->tri_chnout1_alarms, videolost->tri_chnout2_alarms,
             videolost->popup_interval, videolost->ptzaction_interval, videolost->alarmout_interval,
             videolost->whiteled_interval,videolost->tri_audio_id, videolost->http_notification_interval, videolost->id);

    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_video_losts(const char *pDbFile, struct video_loss *videolosts, int count, long long changeFlag)
{
    if (!pDbFile || !videolosts) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    int i = 0;
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[512] = {0};
    struct video_loss *videolost = NULL;

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (i = 0; i < MAX_CAMERA && i < count; i++) {
        if (!(changeFlag >> i & 0x01)) {
            continue;
        }

        videolost = videolosts + i;
        snprintf(sExec, sizeof(sExec), "update video_loss set enable=%d,tri_alarms=%d,buzzer_interval=%d,email_enable=%d,\
			email_buzzer_interval=%d,tri_chnout1_alarms='%s',tri_chnout2_alarms='%s',popup_interval=%d,ptzaction_interval=%d,\
			alarmout_interval=%d,whiteled_interval=%d,tri_audio_id=%d,http_notification_interval=%d where id=%d;",
                 videolost->enable, videolost->tri_alarms, videolost->buzzer_interval, videolost->email_enable,
                 videolost->email_buzzer_interval, videolost->tri_chnout1_alarms, videolost->tri_chnout2_alarms,
                 videolost->popup_interval, videolost->ptzaction_interval, videolost->alarmout_interval,
                 videolost->whiteled_interval, videolost->tri_audio_id, videolost->http_notification_interval, videolost->id);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_video_losts(const char *pDbFile, struct video_loss *videolost, long long changeFlag)
{
    if (!pDbFile || !videolost) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    int i = 0;
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[512] = {0};

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (i = 0; i < MAX_CAMERA ; i++) {
        if (!(changeFlag >> i & 0x01)) {
            continue;
        }

        snprintf(sExec, sizeof(sExec), "update video_loss set enable=%d,tri_alarms=%d,buzzer_interval=%d,email_enable=%d,\
			email_buzzer_interval=%d,tri_chnout1_alarms='%s',tri_chnout2_alarms='%s',popup_interval=%d,ptzaction_interval=%d,\
			alarmout_interval=%d,whiteled_interval=%d,tri_audio_id=%d,http_notification_interval=%d where id=%d;",
                 videolost->enable, videolost->tri_alarms, videolost->buzzer_interval, videolost->email_enable,
                 videolost->email_buzzer_interval, videolost->tri_chnout1_alarms, videolost->tri_chnout2_alarms,
                 videolost->popup_interval, videolost->ptzaction_interval, videolost->alarmout_interval,
                 videolost->whiteled_interval, videolost->tri_audio_id, videolost->http_notification_interval, i);

        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int insert_video_lost(const char *pDbFile, struct video_loss *videolost)
{
    if (!pDbFile || !videolost) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec),
             "insert into video_loss(id,enable,tri_alarms,buzzer_interval,email_enable,email_buzzer_interval,\
		tri_chnout1_alarms,tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval,tri_audio_id) \
		values(%d,%d,%d,%d,%d,%d,%s,%s,%d,%d,%d,%d,%d);",
             videolost->id, videolost->enable, videolost->tri_alarms, videolost->buzzer_interval, videolost->email_enable,
             videolost->email_buzzer_interval,
             videolost->tri_chnout1_alarms, videolost->tri_chnout2_alarms, videolost->popup_interval, videolost->ptzaction_interval,
             videolost->alarmout_interval, videolost->whiteled_interval, videolost->tri_audio_id);

    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_video_loss_schedule(const char *pDbFile, struct video_loss_schedule  *videolostSchedule, int chn_id)
{
    if (!pDbFile || !videolostSchedule) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select week_id,plan_id,start_time,end_time,action_type from video_loss_schedule where chn_id=%d;", chn_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, videolostSchedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(videolostSchedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, videolostSchedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(videolostSchedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(videolostSchedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_video_loss_schedule(const char *pDbFile, struct video_loss_schedule *schedule, int chn_id)
{
    if (!pDbFile || !schedule || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "delete from video_loss_schedule where chn_id=%d;", chn_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into video_loss_schedule values(%d,%d,%d,'%s','%s',%d);", chn_id, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_videoloss_audible_schedule(const char *pDbFile, struct video_loss_schedule  *videolostSchedule, int chn_id)
{
    if (!pDbFile || !videolostSchedule) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select week_id,plan_id,start_time,end_time,action_type from videoloss_audible_schedule where chn_id=%d;", chn_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, videolostSchedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(videolostSchedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, videolostSchedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(videolostSchedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(videolostSchedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_videoloss_audible_schedule(const char *pDbFile, struct video_loss_schedule *schedule, int chn_id)
{
    if (!pDbFile || !schedule || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "delete from videoloss_audible_schedule where chn_id=%d;", chn_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into videoloss_audible_schedule values(%d,%d,%d,'%s','%s',%d);", chn_id, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_videoloss_audible_schedules(const char *pDbFile, struct video_loss_schedule *schedule, long long changeFlag)
{
    if (!pDbFile || !schedule) {
        return -1;
    }

    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[256] = {0};
    int i = 0, j = 0, k = 0;

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (k = 0; k < MAX_CAMERA; k++) {
        if (!(changeFlag >> k & 0x01)) {
            continue;
        }

        snprintf(sExec, sizeof(sExec), "delete from videoloss_audible_schedule where chn_id=%d;", k);
        sqlite_execute(hConn, mode, sExec);

        for (i = 0; i < MAX_DAY_NUM; i++) {
            for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
                if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                               schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                    continue;
                }
                snprintf(sExec, sizeof(sExec), "insert into videoloss_audible_schedule values(%d,%d,%d,'%s','%s',%d);", k, i, j,
                         schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                         schedule->schedule_day[i].schedule_item[j].action_type);
                sqlite_execute(hConn, mode, sExec);
            }
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}


int disable_ip_camera(const char *pDbFile, int nCamId)
{
    if (!pDbFile || nCamId < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "update camera set enable=0 where id=%d;", nCamId);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int clear_cameras(const char *pDbFile)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[2048] = {0};
    snprintf(sExec, sizeof(sExec),
             "update camera set type=1,enable=0,covert_on=0,brightness=0,contrast=0,saturation=0,username='',password='',ip_addr='',main_rtsp_port=0,main_source_path='',sub_rtsp_enable=0,sub_rtsp_port=0,sub_source_path='',manage_port=0,camera_protocol=0,transmit_protocol=0,play_stream=0,record_stream=0,sync_time=0,main_rtspurl='',sub_rtspurl='',poe_channel=0,physical_port=0,mac='',anr=0,anr_start='',anr_end='',minorid=0,ddns='',https_port=0;");
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}


int read_cameras_type(const char *pDbFile, int types[])
{
    if (!pDbFile || !types) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery), "select type from camera;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    for (i = 0; i < MAX_CAMERA; i++) {
        sqlite_query_column(hStmt, 0, &types[i]);
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_camera(const char *pDbFile, struct camera *cam, int chn_id)
{
    if (!pDbFile || !cam) {
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
    snprintf(sQuery, sizeof(sQuery),
             "select id,type,enable,covert_on,brightness,contrast,saturation, ip_addr,manage_port,camera_protocol,play_stream,record_stream,username,password,transmit_protocol,main_rtsp_port,main_source_path,sub_rtsp_enable,sub_rtsp_port,sub_source_path,sync_time,codec,main_rtspurl,sub_rtspurl,poe_channel,physical_port,mac,anr,anr_start,anr_end,minorid,ddns,https_port from camera where id = %d;",
             chn_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &cam->id);
    sqlite_query_column(hStmt, 1, &cam->type);
    sqlite_query_column(hStmt, 2, &cam->enable);
    sqlite_query_column(hStmt, 3, &cam->covert_on);
    sqlite_query_column(hStmt, 4, &cam->brightness);
    sqlite_query_column(hStmt, 5, &cam->contrast);
    sqlite_query_column(hStmt, 6, &cam->saturation);

    sqlite_query_column_text(hStmt, 7, cam->ip_addr, sizeof(cam->ip_addr));
    sqlite_query_column(hStmt, 8, &cam->manage_port);
    sqlite_query_column(hStmt, 9, &cam->camera_protocol);
    sqlite_query_column(hStmt, 10, &cam->play_stream);
    sqlite_query_column(hStmt, 11, &cam->record_stream);
    sqlite_query_column_text(hStmt, 12, cam->username, sizeof(cam->username));
    sqlite_query_column_text(hStmt, 13, cam->password, sizeof(cam->password));
    sqlite_query_column(hStmt, 14, &cam->transmit_protocol);
    sqlite_query_column(hStmt, 15, &cam->main_rtsp_port);
    sqlite_query_column_text(hStmt, 16, cam->main_source_path, sizeof(cam->main_source_path));
    sqlite_query_column(hStmt, 17, &cam->sub_rtsp_enable);
    sqlite_query_column(hStmt, 18, &cam->sub_rtsp_port);
    sqlite_query_column_text(hStmt, 19, cam->sub_source_path, sizeof(cam->sub_source_path));
    sqlite_query_column(hStmt, 20, &cam->sync_time);
    sqlite_query_column(hStmt, 21, &cam->codec);
    sqlite_query_column_text(hStmt, 22, cam->main_rtspurl, sizeof(cam->main_rtspurl));
    sqlite_query_column_text(hStmt, 23, cam->sub_rtspurl, sizeof(cam->sub_rtspurl));

    sqlite_query_column(hStmt, 24, &cam->poe_channel);
    sqlite_query_column(hStmt, 25, &cam->physical_port);
    sqlite_query_column_text(hStmt, 26, cam->mac_addr, sizeof(cam->mac_addr));
    sqlite_query_column(hStmt, 27, &cam->anr);
    sqlite_query_column_text(hStmt, 28, cam->anr_start, sizeof(cam->anr_start));
    sqlite_query_column_text(hStmt, 29, cam->anr_end, sizeof(cam->anr_end));
    sqlite_query_column(hStmt, 30, &cam->minorid);
    sqlite_query_column_text(hStmt, 31, cam->ddns, sizeof(cam->ddns));
    sqlite_query_column(hStmt, 32, &cam->https_port);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_cameras(const char *pDbFile, struct camera cams[], int *cnt)
{
    if (!pDbFile || !cams) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select id,type,enable,covert_on,brightness,contrast,saturation,ip_addr,manage_port,camera_protocol,play_stream,record_stream,username,password,transmit_protocol,main_rtsp_port,main_source_path,sub_rtsp_enable,sub_rtsp_port,sub_source_path,sync_time,codec,main_rtspurl,sub_rtspurl,poe_channel,physical_port,mac,anr,anr_start,anr_end,minorid,ddns,https_port from camera;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *cnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < MAX_CAMERA; i++) {
        sqlite_query_column(hStmt, 0, &cams[i].id);
        sqlite_query_column(hStmt, 1, &cams[i].type);
        sqlite_query_column(hStmt, 2, &cams[i].enable);
        sqlite_query_column(hStmt, 3, &cams[i].covert_on);
        sqlite_query_column(hStmt, 4, &cams[i].brightness);
        sqlite_query_column(hStmt, 5, &cams[i].contrast);
        sqlite_query_column(hStmt, 6, &cams[i].saturation);

        sqlite_query_column_text(hStmt, 7, cams[i].ip_addr, sizeof(cams[i].ip_addr));
        sqlite_query_column(hStmt, 8, &cams[i].manage_port);
        sqlite_query_column(hStmt, 9, &cams[i].camera_protocol);
        sqlite_query_column(hStmt, 10, &cams[i].play_stream);
        sqlite_query_column(hStmt, 11, &cams[i].record_stream);
        sqlite_query_column_text(hStmt, 12, cams[i].username, sizeof(cams[i].username));
        sqlite_query_column_text(hStmt, 13, cams[i].password, sizeof(cams[i].password));
        sqlite_query_column(hStmt, 14, &cams[i].transmit_protocol);
        sqlite_query_column(hStmt, 15, &cams[i].main_rtsp_port);
        sqlite_query_column_text(hStmt, 16, cams[i].main_source_path, sizeof(cams[i].main_source_path));
        sqlite_query_column(hStmt, 17, &cams[i].sub_rtsp_enable);
        sqlite_query_column(hStmt, 18, &cams[i].sub_rtsp_port);
        sqlite_query_column_text(hStmt, 19, cams[i].sub_source_path, sizeof(cams[i].sub_source_path));
        sqlite_query_column(hStmt, 20, &cams[i].sync_time);
        sqlite_query_column(hStmt, 21, &cams[i].codec);
        sqlite_query_column_text(hStmt, 22, cams[i].main_rtspurl, sizeof(cams[i].main_rtspurl));
        sqlite_query_column_text(hStmt, 23, cams[i].sub_rtspurl, sizeof(cams[i].sub_rtspurl));

        sqlite_query_column(hStmt, 24, &cams[i].poe_channel);
        sqlite_query_column(hStmt, 25, &cams[i].physical_port);
        sqlite_query_column_text(hStmt, 26, cams[i].mac_addr, sizeof(cams[i].mac_addr));
        sqlite_query_column(hStmt, 27, &cams[i].anr);
        sqlite_query_column_text(hStmt, 28, cams[i].anr_start, sizeof(cams[i].anr_start));
        sqlite_query_column_text(hStmt, 29, cams[i].anr_end, sizeof(cams[i].anr_end));
        sqlite_query_column(hStmt, 30, &cams[i].minorid);
        sqlite_query_column_text(hStmt, 31, cams[i].ddns, sizeof(cams[i].ddns));
        sqlite_query_column(hStmt, 32, &cams[i].https_port);

        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    *cnt = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_poe_cameras(const char *pDbFile, struct camera cams[], int *cnt)
{
    if (!pDbFile || !cams) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select id,type,enable,covert_on,brightness,contrast,saturation,ip_addr,manage_port,camera_protocol,play_stream,record_stream,username,password,transmit_protocol,main_rtsp_port,main_source_path,sub_rtsp_enable,sub_rtsp_port,sub_source_path,sync_time,codec,main_rtspurl,sub_rtspurl,poe_channel,physical_port,mac,anr,anr_start,anr_end,minorid,ddns,https_port from camera where poe_channel = 1;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *cnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < nResult && i < MAX_CAMERA; i++) {
        sqlite_query_column(hStmt, 0, &cams[i].id);
        sqlite_query_column(hStmt, 1, &cams[i].type);
        sqlite_query_column(hStmt, 2, &cams[i].enable);
        sqlite_query_column(hStmt, 3, &cams[i].covert_on);
        sqlite_query_column(hStmt, 4, &cams[i].brightness);
        sqlite_query_column(hStmt, 5, &cams[i].contrast);
        sqlite_query_column(hStmt, 6, &cams[i].saturation);

        sqlite_query_column_text(hStmt, 7, cams[i].ip_addr, sizeof(cams[i].ip_addr));
        sqlite_query_column(hStmt, 8, &cams[i].manage_port);
        sqlite_query_column(hStmt, 9, &cams[i].camera_protocol);
        sqlite_query_column(hStmt, 10, &cams[i].play_stream);
        sqlite_query_column(hStmt, 11, &cams[i].record_stream);
        sqlite_query_column_text(hStmt, 12, cams[i].username, sizeof(cams[i].username));
        sqlite_query_column_text(hStmt, 13, cams[i].password, sizeof(cams[i].password));
        sqlite_query_column(hStmt, 14, &cams[i].transmit_protocol);
        sqlite_query_column(hStmt, 15, &cams[i].main_rtsp_port);
        sqlite_query_column_text(hStmt, 16, cams[i].main_source_path, sizeof(cams[i].main_source_path));
        sqlite_query_column(hStmt, 17, &cams[i].sub_rtsp_enable);
        sqlite_query_column(hStmt, 18, &cams[i].sub_rtsp_port);
        sqlite_query_column_text(hStmt, 19, cams[i].sub_source_path, sizeof(cams[i].sub_source_path));
        sqlite_query_column(hStmt, 20, &cams[i].sync_time);
        sqlite_query_column(hStmt, 21, &cams[i].codec);
        sqlite_query_column_text(hStmt, 22, cams[i].main_rtspurl, sizeof(cams[i].main_rtspurl));
        sqlite_query_column_text(hStmt, 23, cams[i].sub_rtspurl, sizeof(cams[i].sub_rtspurl));

        sqlite_query_column(hStmt, 24, &cams[i].poe_channel);
        sqlite_query_column(hStmt, 25, &cams[i].physical_port);
        sqlite_query_column_text(hStmt, 26, cams[i].mac_addr, sizeof(cams[i].mac_addr));
        sqlite_query_column(hStmt, 27, &cams[i].anr);
        sqlite_query_column_text(hStmt, 28, cams[i].anr_start, sizeof(cams[i].anr_start));
        sqlite_query_column_text(hStmt, 29, cams[i].anr_end, sizeof(cams[i].anr_end));
        sqlite_query_column(hStmt, 30, &cams[i].minorid);
        sqlite_query_column_text(hStmt, 31, cams[i].ddns, sizeof(cams[i].ddns));
        sqlite_query_column(hStmt, 32, &cams[i].https_port);

        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    *cnt = nResult;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int insert_camera(const char *pDbFile, struct camera *cam)
{
    if (!pDbFile || !cam) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[2048] = {0};
    snprintf(sExec, sizeof(sExec),
             "insert into camera(id,type,enable,covert_on,brightness,brightness,saturation,username,password,ip_addr,main_rtsp_port,main_source_path,sub_rtsp_enable,sub_rtsp_port,sub_source_path,manage_port,camera_protocol,transmit_protocol,play_stream,record_stream,sync_time,main_rtspurl,sub_rtspurl,poe_channel,physical_port,mac,minorid,ddns) values(%d,%d,%d,%d,%d,%d,%d,'%s','%s','%s',%d,'%s',%d,%d,'%s',%d,%d,%d,%d,%d,%d,'%s','%s',%d,%d,'%s',%d,'%s');",
             \
             cam->id, cam->type, cam->enable, cam->covert_on, cam->brightness, cam->contrast, cam->saturation, cam->username,
             cam->password, cam->ip_addr, cam->main_rtsp_port, cam->main_source_path, cam->sub_rtsp_enable, cam->sub_rtsp_port,
             cam->sub_source_path, cam->manage_port, cam->camera_protocol, cam->transmit_protocol, cam->play_stream,
             cam->record_stream, cam->sync_time, cam->main_rtspurl, cam->sub_rtspurl, cam->poe_channel, cam->physical_port,
             cam->mac_addr, cam->minorid, cam->ddns);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_camera_codec(const char *pDbFile, struct camera *cam)
{
    if (!pDbFile || !cam) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec), "update camera set codec=%d where id=%d;", cam->codec, cam->id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int update_camera(const char *pDbFile, struct camera *cam, int type)
{
    if (!pDbFile || !cam) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[2048] = {0};
    char pwd[MAX_PWD_LEN * 2 + 2] = {0};

    switch (type) {
        case CAMERA_PASSWORD:
            translate_pwd(pwd, cam->password, strlen(cam->password));
            snprintf(sExec, sizeof(sExec), "update camera set password='%s' where id=%d;", pwd, cam->id);
            break;
        case CAMERA_USERNAME:
            snprintf(sExec, sizeof(sExec), "update camera set username='%s' where id=%d;", cam->username, cam->id);
            break;
        case CAMERA_POE_CHANNEL:
            snprintf(sExec, sizeof(sExec), "update camera set poe_channel=%d where id=%d;", cam->poe_channel, cam->id);
            break;
        default:
            break;
    }

    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_camera(const char *pDbFile, struct camera *cam)
{
    if (!pDbFile || !cam) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[2048] = {0};
    char pwd[MAX_PWD_LEN * 2 + 2] = {0};

    translate_pwd(pwd, cam->password, strlen(cam->password));

    do {
        if (cam->type != CAMERA_TYPE_IPNC) {
            msdebug(DEBUG_INF, "ch:%d invalid type:%d", cam->id, cam->type);
            break;
        }
        if (cam->enable && (cam->https_port <= 0 || cam->https_port > 65535)) {
            cam->https_port = 443;
        }
        snprintf(sExec, sizeof(sExec),
                 "update camera set type=%d,enable=%d,covert_on=%d,brightness=%d,contrast=%d,saturation=%d,username='%s',password='%s',ip_addr='%s',main_rtsp_port=%d,main_source_path='%s',sub_rtsp_enable=%d,sub_rtsp_port=%d,sub_source_path='%s',manage_port=%d,camera_protocol=%d,transmit_protocol=%d,play_stream=%d,record_stream=%d,sync_time=%d,main_rtspurl='%s',sub_rtspurl='%s',poe_channel=%d,physical_port=%d,mac='%s',anr=%d,anr_start='%s',anr_end='%s',minorid=%d,ddns='%s',https_port=%d where id=%d;",
                 \
                 cam->type, cam->enable, cam->covert_on, cam->brightness, cam->contrast, cam->saturation, cam->username, pwd,
                 cam->ip_addr, cam->main_rtsp_port, cam->main_source_path, cam->sub_rtsp_enable, cam->sub_rtsp_port,
                 cam->sub_source_path, cam->manage_port, cam->camera_protocol, cam->transmit_protocol, cam->play_stream,
                 cam->record_stream, cam->sync_time, cam->main_rtspurl, cam->sub_rtspurl, cam->poe_channel, cam->physical_port,
                 cam->mac_addr, cam->anr, cam->anr_start, cam->anr_end, cam->minorid, cam->ddns,cam->https_port, cam->id);
        sqlite_execute(hConn, mode, sExec);
    } while (0);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_cameras(const char *pDbFile, struct camera cams[], Uint64 changeFlag)
{
    if (!pDbFile || !cams) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[2048] = {0};
    int i = 0;
    for (i = 0; i < MAX_REAL_CAMERA; i++) {
        if (!(changeFlag >> i & 0x01)) {
            continue;
        }
        if (cams[i].type != CAMERA_TYPE_IPNC) {
            msdebug(DEBUG_INF, "No.:%d ch:%d invalid type:%d", i, cams[i].id, cams[i].type);
            continue;
        }
        if (cams[i].enable && (cams[i].https_port <= 0 || cams[i].https_port > 65535)) {
            cams[i].https_port = 443;
        }
        memset(sExec, 0, sizeof(sExec));
        snprintf(sExec, sizeof(sExec),
                 "update camera set type=%d,enable=%d,covert_on=%d,brightness=%d,contrast=%d,saturation=%d,username='%s',password='%s',ip_addr='%s',main_rtsp_port=%d,main_source_path='%s',sub_rtsp_enable=%d,sub_rtsp_port=%d,sub_source_path='%s',manage_port=%d,camera_protocol=%d,transmit_protocol=%d,play_stream=%d,record_stream=%d,sync_time=%d,main_rtspurl='%s',sub_rtspurl='%s',poe_channel=%d,physical_port=%d,mac='%s',anr=%d,anr_start='%s',anr_end='%s',minorid=%d,ddns='%s',https_port=%d where id=%d;",
                 \
                 cams[i].type, cams[i].enable, cams[i].covert_on, cams[i].brightness, cams[i].contrast, cams[i].saturation,
                 cams[i].username, cams[i].password, cams[i].ip_addr, cams[i].main_rtsp_port, cams[i].main_source_path,
                 cams[i].sub_rtsp_enable, cams[i].sub_rtsp_port, cams[i].sub_source_path, cams[i].manage_port, cams[i].camera_protocol,
                 cams[i].transmit_protocol, cams[i].play_stream, cams[i].record_stream, cams[i].sync_time, cams[i].main_rtspurl,
                 cams[i].sub_rtspurl, cams[i].poe_channel, cams[i].physical_port, \
                 cams[i].mac_addr, cams[i].anr, cams[i].anr_start, cams[i].anr_end, cams[i].minorid, cams[i].ddns, cams[i].https_port, cams[i].id);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_camera_ip(const char *pDbFile, int id, const char *ip)
{
    if (!pDbFile || !ip || id < 0 || id >= MAX_CAMERA) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[2048] = {0};
    memset(sExec, 0, sizeof(sExec));
    snprintf(sExec, sizeof(sExec), "update camera set ip_addr='%s' where id=%d;", ip, id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}


int read_record(const char *pDbFile, struct record *record, int chn_id)
{
    if (!pDbFile || !record || chn_id < 0) {
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
    char sQuery[512] = {0};
    int nColumnCnt = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select id,codec_type,iframe_interval,bitrate_type,bitrate,fps,resolution,audio_enable,audio_codec_type,audio_samplerate,prev_record_on,prev_record_duration,post_record_on,post_record_duration,input_video_signal_type,mode,record_expiration_date,photo_expiration_date from record where id=%d;",
             chn_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &record->id);
    sqlite_query_column(hStmt, 1, &record->codec_type);
    sqlite_query_column(hStmt, 2, &record->iframe_interval);
    sqlite_query_column(hStmt, 3, &record->bitrate_type);
    sqlite_query_column(hStmt, 4, &record->kbps);
    sqlite_query_column(hStmt, 5, &record->fps);
    sqlite_query_column(hStmt, 6, &record->resolution);

    sqlite_query_column(hStmt, 7, &record->audio_enable);
    sqlite_query_column(hStmt, 8, &record->audio_codec_type);
    sqlite_query_column(hStmt, 9, &record->audio_samplerate);

    sqlite_query_column(hStmt, 10, &record->prev_record_on);
    sqlite_query_column(hStmt, 11, &record->prev_record_duration);
    sqlite_query_column(hStmt, 12, &record->post_record_on);
    sqlite_query_column(hStmt, 13, &record->post_record_duration);
    sqlite_query_column(hStmt, 14, &record->input_video_signal_type);
    sqlite_query_column(hStmt, 15, &record->mode);
    sqlite_query_column(hStmt, 16, &record->record_expiration_date);
    sqlite_query_column(hStmt, 17, &record->photo_expiration_date);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_records(const char *pDbFile, struct record records[], int *pCnt)
{
    if (!pDbFile || !records) {
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
    char sQuery[512] = {0};
    int nColumnCnt = 0;
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select id,codec_type,iframe_interval,bitrate_type,bitrate,fps,resolution,audio_enable,audio_codec_type,audio_samplerate,prev_record_on,prev_record_duration,post_record_on,post_record_duration,input_video_signal_type,mode,record_expiration_date,photo_expiration_date from record;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *pCnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < MAX_CAMERA; i++) {
        sqlite_query_column(hStmt, 0, &records[i].id);
        sqlite_query_column(hStmt, 1, &records[i].codec_type);
        sqlite_query_column(hStmt, 2, &records[i].iframe_interval);
        sqlite_query_column(hStmt, 3, &records[i].bitrate_type);
        sqlite_query_column(hStmt, 4, &records[i].kbps);
        sqlite_query_column(hStmt, 5, &records[i].fps);
        sqlite_query_column(hStmt, 6, &records[i].resolution);

        sqlite_query_column(hStmt, 7, &records[i].audio_enable);
        sqlite_query_column(hStmt, 8, &records[i].audio_codec_type);
        sqlite_query_column(hStmt, 9, &records[i].audio_samplerate);

        sqlite_query_column(hStmt, 10, &records[i].prev_record_on);
        sqlite_query_column(hStmt, 11, &records[i].prev_record_duration);
        sqlite_query_column(hStmt, 12, &records[i].post_record_on);
        sqlite_query_column(hStmt, 13, &records[i].post_record_duration);

        sqlite_query_column(hStmt, 14, &records[i].input_video_signal_type);
        sqlite_query_column(hStmt, 15, &records[i].mode);
        //david modify
        sqlite_query_column(hStmt, 16, &records[i].record_expiration_date);
        sqlite_query_column(hStmt, 17, &records[i].photo_expiration_date);
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    *pCnt = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}


int read_records_ex(const char *pDbFile, struct record records[], long long changeFlag)
{
    if (!pDbFile || !records) {
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
    char sQuery[512] = {0};
    int nColumnCnt = 0;
    int i = 0;
    for (i = 0; i < MAX_CAMERA; i++) {
        if ((changeFlag >> i) & 0x01) {
            hStmt = 0;
            memset(sQuery, 0, sizeof(sQuery));
            snprintf(sQuery, sizeof(sQuery),
                     "select id,codec_type,iframe_interval,bitrate_type,bitrate,fps,resolution,audio_enable,audio_codec_type,audio_samplerate,prev_record_on,prev_record_duration,post_record_on,post_record_duration,input_video_signal_type,mode,record_expiration_date from record where id=%d;",
                     i);
            int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
            if (nResult != 0 || nColumnCnt == 0) {
                if (hStmt) {
                    sqlite_clear_stmt(hStmt);
                }
                continue;
            }
            sqlite_query_column(hStmt, 0, &records[i].id);
            sqlite_query_column(hStmt, 1, &records[i].codec_type);
            sqlite_query_column(hStmt, 2, &records[i].iframe_interval);
            sqlite_query_column(hStmt, 3, &records[i].bitrate_type);
            sqlite_query_column(hStmt, 4, &records[i].kbps);
            sqlite_query_column(hStmt, 5, &records[i].fps);
            sqlite_query_column(hStmt, 6, &records[i].resolution);

            sqlite_query_column(hStmt, 7, &records[i].audio_enable);
            sqlite_query_column(hStmt, 8, &records[i].audio_codec_type);
            sqlite_query_column(hStmt, 9, &records[i].audio_samplerate);

            sqlite_query_column(hStmt, 10, &records[i].prev_record_on);
            sqlite_query_column(hStmt, 11, &records[i].prev_record_duration);
            sqlite_query_column(hStmt, 12, &records[i].post_record_on);
            sqlite_query_column(hStmt, 13, &records[i].post_record_duration);

            sqlite_query_column(hStmt, 14, &records[i].input_video_signal_type);
            sqlite_query_column(hStmt, 15, &records[i].mode);
            sqlite_query_column(hStmt, 16, &records[i].record_expiration_date);
            sqlite_clear_stmt(hStmt);
        }
    }
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_records(const char *pDbFile, struct record records[], long long changeFlag)
{
    if (!pDbFile || !records || !changeFlag) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    char sExec[512] = {0};
    int i = 0 ;
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (i = 0; i < MAX_CAMERA; i++) {
        if (!(changeFlag >> i & 0x01)) {
            continue;
        }

        memset(sExec, 0, sizeof(sExec));
        snprintf(sExec, sizeof(sExec),
                 "update record set codec_type=%d,iframe_interval=%d,bitrate_type=%d,bitrate=%d,fps=%d,resolution=%d,audio_enable=%d,audio_codec_type=%d,audio_samplerate=%d,prev_record_on=%d,prev_record_duration=%d,post_record_on=%d,post_record_duration=%d,input_video_signal_type=%d,mode=%d,record_expiration_date=%d where id=%d;",
                 records[i].codec_type, records[i].iframe_interval, records[i].bitrate_type, records[i].kbps, records[i].fps,
                 records[i].resolution, records[i].audio_enable, records[i].audio_codec_type, records[i].audio_samplerate,
                 records[i].prev_record_on, records[i].prev_record_duration, records[i].post_record_on, records[i].post_record_duration,
                 records[i].input_video_signal_type, records[i].mode, records[i].record_expiration_date, records[i].id);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}
//end

int write_record(const char *pDbFile, struct record *record)
{
    if (!pDbFile || !record) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec),
             "update record set codec_type=%d,iframe_interval=%d,bitrate_type=%d,bitrate=%d,fps=%d,resolution=%d,audio_enable=%d,audio_codec_type=%d,audio_samplerate=%d,prev_record_on=%d,prev_record_duration=%d,post_record_on=%d,post_record_duration=%d,input_video_signal_type=%d,mode=%d,record_expiration_date=%d,photo_expiration_date=%d where id=%d;",
             record->codec_type, record->iframe_interval, record->bitrate_type, record->kbps, record->fps, record->resolution,
             record->audio_enable, record->audio_codec_type, record->audio_samplerate, record->prev_record_on,
             record->prev_record_duration, record->post_record_on, record->post_record_duration, record->input_video_signal_type,
             record->mode, record->record_expiration_date, record->photo_expiration_date, record->id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int insert_record(const char *pDbFile, struct record *record)
{
    if (!pDbFile || !record) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[1024] = {0};
    snprintf(sExec, sizeof(sExec),
             "insert into record(id,codec_type,iframe_interval,bitrate_type,bitrate,fps,resolution,audio_enable,audio_codec_type,audio_samplerate,prev_record_on,prev_record_duration,post_record_on,post_record_duration,input_video_signal_type,mode,record_expiration_date,photo_expiration_date) values(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d);",
             record->id, record->codec_type, record->iframe_interval, record->bitrate_type, record->kbps, record->fps,
             record->resolution, record->audio_enable, record->audio_codec_type, record->audio_samplerate, record->prev_record_on,
             record->prev_record_duration, record->post_record_on, record->post_record_duration, record->input_video_signal_type,
             record->mode, record->record_expiration_date, record->photo_expiration_date);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_record_prev_post(const char *pDbFile, struct record *record)
{
    if (!pDbFile || !record) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec),
             "update record set prev_record_on=%d, prev_record_duration=%d, post_record_on=%d, post_record_duration=%d;",
             record->prev_record_on, record->prev_record_duration, record->post_record_on, record->post_record_duration);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int check_record_post_time(const char *pDbFile)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec),"update record set post_record_duration='-3' where (post_record_duration=-4 or post_record_duration=0);");
    sqlite_execute(hConn, mode, sExec);
    snprintf(sExec, sizeof(sExec),"update record set post_record_on=%d;", 1);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_record_schedule(const char *pDbFile, struct record_schedule *schedule, int chn_id)
{
    if (!pDbFile || !schedule || chn_id < 0) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select enable,wday_id,wholeday_enable,wholeday_action_type,plan_id,start_time,end_time,action_type from record_schedule where chn_id=%d;",
             chn_id);

    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM;
    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &schedule->enable);
        sqlite_query_column(hStmt, 1, &nWeekId);
        sqlite_query_column(hStmt, 2, &schedule->schedule_day[nWeekId].wholeday_enable);
        sqlite_query_column(hStmt, 3, &schedule->schedule_day[nWeekId].wholeday_action_type);
        sqlite_query_column(hStmt, 4, &nPlanId);
        sqlite_query_column_text(hStmt, 5, schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 6, schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 7, &(schedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_record_schedules(const char *pDbFile, struct record_schedule *schedules, int count, int *pCnt)
{
    if (!pDbFile || !schedules) {
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
    int i = 0, j = 0, k = 0;
    int nResult = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM;
    struct record_schedule *schedule = NULL;

    for (k = 0; k < MAX_CAMERA && k < count; k++) {
        snprintf(sQuery, sizeof(sQuery),
                 "select enable,wday_id,wholeday_enable,wholeday_action_type,plan_id,start_time,end_time,action_type from record_schedule where chn_id=%d;",
                 k);
        nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);

        if (nResult || !nColumnCnt) {
            sqlite_clear_stmt(hStmt);
            continue;
        }
        schedule = schedules + k;
        for (i = 0; i <= nLoopCnt; i++) {
            sqlite_query_column(hStmt, 0, &schedule->enable);
            sqlite_query_column(hStmt, 1, &nWeekId);
            sqlite_query_column(hStmt, 2, &schedule->schedule_day[nWeekId].wholeday_enable);
            sqlite_query_column(hStmt, 3, &schedule->schedule_day[nWeekId].wholeday_action_type);
            sqlite_query_column(hStmt, 4, &nPlanId);
            sqlite_query_column_text(hStmt, 5, schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 6, schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 7, &(schedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
            if (sqlite_query_next(hStmt) != 0) {
                break;
            }
        }
        sqlite_clear_stmt(hStmt);
        j++;
    }
    *pCnt = j;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_record_schedule(const char *pDbFile, struct record_schedule *schedule, int chn_id)
{
    if (!pDbFile || !schedule || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[512] = {0};

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    snprintf(sExec, sizeof(sExec), "delete from record_schedule where chn_id=%d;", chn_id);
    sqlite_execute(hConn, mode, sExec);

    int i = 0;
    int j = 0;
    int nNotInsNum = 0;
    int nHasWholeDayEnable = 0;
    int t;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        nNotInsNum = 0;
        nHasWholeDayEnable = 0;
        if (schedule->schedule_day[i].wholeday_enable != 0) {
            nHasWholeDayEnable = 1;
        }
        t = 0;
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM; j++) {
            if (schedule->schedule_day[i].schedule_item[j].action_type == NONE ||
                strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                nNotInsNum++;
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into record_schedule values(%d,%d,%d,%d,%d,'%s','%s',%d,%d);", chn_id, i,
                     schedule->schedule_day[i].wholeday_enable, schedule->schedule_day[i].wholeday_action_type, t,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type, schedule->enable);
            t++;
            sqlite_execute(hConn, mode, sExec);
        }

        if (nHasWholeDayEnable == 1 && nNotInsNum == MAX_PLAN_NUM_PER_DAY) {
            snprintf(sExec, sizeof(sExec), "insert into record_schedule values(%d,%d,%d,%d,%d,'%s','%s',%d,%d);", chn_id, i,
                     schedule->schedule_day[i].wholeday_enable, schedule->schedule_day[i].wholeday_action_type, 0, "00:00:00", "00:00:00", 0,
                     schedule->enable);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_record_schedules(const char *pDbFile, struct record_schedule *schedules, struct channel_batch *batch)
{
    if (!pDbFile || !schedules || !batch) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[512] = {0};
    int i = 0;
    int j = 0;
    int k = 0;
    int nNotInsNum = 0;
    int nHasWholeDayEnable = 0;
    int t = 0;
    int chanid = 0;
    struct record_schedule *schedule = NULL;

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (k = 0; k < batch->size && k < MAX_CAMERA; k++) {
        chanid = batch->chanid[k];
        if (chanid < 0 || chanid >= MAX_CAMERA) {
            continue;
        }

        schedule = schedules + chanid;
        snprintf(sExec, sizeof(sExec), "delete from record_schedule where chn_id=%d;", chanid);
        sqlite_execute(hConn, mode, sExec);
        for (i = 0; i < MAX_DAY_NUM; i++) {
            nNotInsNum = 0;
            nHasWholeDayEnable = 0;
            if (schedule->schedule_day[i].wholeday_enable != 0) {
                nHasWholeDayEnable = 1;
            }
            t = 0;
            for (j = 0; j < MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM; j++) {
                if (schedule->schedule_day[i].schedule_item[j].action_type == NONE ||
                    strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                               schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                    nNotInsNum++;
                    continue;
                }
                snprintf(sExec, sizeof(sExec), "insert into record_schedule values(%d,%d,%d,%d,%d,'%s','%s',%d,%d);", chanid, i,
                         schedule->schedule_day[i].wholeday_enable, schedule->schedule_day[i].wholeday_action_type, t,
                         schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                         schedule->schedule_day[i].schedule_item[j].action_type, schedule->enable);
                t++;
                sqlite_execute(hConn, mode, sExec);
            }

            if (nHasWholeDayEnable == 1 && nNotInsNum == MAX_PLAN_NUM_PER_DAY) {
                snprintf(sExec, sizeof(sExec), "insert into record_schedule values(%d,%d,%d,%d,%d,'%s','%s',%d,%d);", chanid, i,
                         schedule->schedule_day[i].wholeday_enable, schedule->schedule_day[i].wholeday_action_type, 0, "00:00:00", "00:00:00", 0,
                         schedule->enable);
                sqlite_execute(hConn, mode, sExec);
            }
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_record_schedules(const char *pDbFile, struct record_schedule *schedule, long long changeFlag)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[512] = {0};
    int i = 0;
    int j = 0;
    int k = 0;
    int nNotInsNum = 0;
    int nHasWholeDayEnable = 0;
    int t = 0;

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (k = 0; k < MAX_CAMERA; k++) {
        if (!(changeFlag >> k & 0x01)) {
            continue;
        }

        snprintf(sExec, sizeof(sExec), "delete from record_schedule where chn_id=%d;", k);
        sqlite_execute(hConn, mode, sExec);
        for (i = 0; i < MAX_DAY_NUM; i++) {
            nNotInsNum = 0;
            nHasWholeDayEnable = 0;
            if (schedule->schedule_day[i].wholeday_enable != 0) {
                nHasWholeDayEnable = 1;
            }
            t = 0;
            for (j = 0; j < MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM; j++) {
                if (schedule->schedule_day[i].schedule_item[j].action_type == NONE ||
                    strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                               schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                    nNotInsNum++;
                    continue;
                }
                snprintf(sExec, sizeof(sExec), "insert into record_schedule values(%d,%d,%d,%d,%d,'%s','%s',%d,%d);", k, i,
                         schedule->schedule_day[i].wholeday_enable, schedule->schedule_day[i].wholeday_action_type, t,
                         schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                         schedule->schedule_day[i].schedule_item[j].action_type, schedule->enable);
                t++;
                sqlite_execute(hConn, mode, sExec);
            }

            if (nHasWholeDayEnable == 1 && nNotInsNum == MAX_PLAN_NUM_PER_DAY) {
                snprintf(sExec, sizeof(sExec), "insert into record_schedule values(%d,%d,%d,%d,%d,'%s','%s',%d,%d);", k, i,
                         schedule->schedule_day[i].wholeday_enable, schedule->schedule_day[i].wholeday_action_type, 0, "00:00:00", "00:00:00", 0,
                         schedule->enable);
                sqlite_execute(hConn, mode, sExec);
            }
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;

}

int reset_record_schedule(const char *pDbFile, int chn_id)
{
    if (!pDbFile || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec), "delete from record_schedule where chn_id=%d;", chn_id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_holidays(const char *pDbFile, struct holiday holidays[], int *pCnt)
{
    if (!pDbFile || !holidays) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select id,enable,name,type,start_year,start_mon,start_mday,start_mweek,start_wday,end_year,end_mon,end_mday,end_mweek,end_wday from holiday;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *pCnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < MAX_CAMERA; i++) {
        sqlite_query_column(hStmt, 0, &holidays[i].id);
        sqlite_query_column(hStmt, 1, &holidays[i].enable);
        sqlite_query_column_text(hStmt, 2, holidays[i].name, sizeof(holidays[i].name));
        sqlite_query_column(hStmt, 3, &holidays[i].type);
        sqlite_query_column(hStmt, 4, &holidays[i].start_year);
        sqlite_query_column(hStmt, 5, &holidays[i].start_mon);
        sqlite_query_column(hStmt, 6, &holidays[i].start_mday);
        sqlite_query_column(hStmt, 7, &holidays[i].start_mweek);
        sqlite_query_column(hStmt, 8, &holidays[i].start_wday);
        sqlite_query_column(hStmt, 9, &holidays[i].end_year);
        sqlite_query_column(hStmt, 10, &holidays[i].end_mon);
        sqlite_query_column(hStmt, 11, &holidays[i].end_mday);
        sqlite_query_column(hStmt, 12, &holidays[i].end_mweek);
        sqlite_query_column(hStmt, 13, &holidays[i].end_wday);

        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    *pCnt = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_holiday(const char *pDbFile, struct holiday *holiday)
{
    if (!pDbFile || !holiday) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec),
             "update holiday set enable=%d,name='%s',type=%d,start_year=%d,start_mon=%d,start_mday=%d,start_mweek=%d,start_wday=%d,end_year=%d,end_mon=%d,end_mday=%d,end_mweek=%d,end_wday=%d where id=%d;",
             holiday->enable, holiday->name, holiday->type, holiday->start_year, holiday->start_mon, holiday->start_mday,
             holiday->start_mweek, holiday->start_wday, holiday->end_year, holiday->end_mon, holiday->end_mday, holiday->end_mweek,
             holiday->end_wday, holiday->id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_ptz_port(const char *pDbFile, struct ptz_port *port, int chn_id)
{
    if (!pDbFile || !port || chn_id < 0) {
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
    char sQuery[512] = {0};
    int nColumnCnt = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select id, baudrate,data_bit,stop_bit,parity_type,protocol,address,com_type,connect_type from ptz_port where id=%d;",
             chn_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &port->id);
    sqlite_query_column(hStmt, 1, &port->baudrate);
    sqlite_query_column(hStmt, 2, &port->data_bit);
    sqlite_query_column(hStmt, 3, &port->stop_bit);
    sqlite_query_column(hStmt, 4, &port->parity_type);
    sqlite_query_column(hStmt, 5, &port->protocol);
    sqlite_query_column(hStmt, 6, &port->address);
    sqlite_query_column(hStmt, 7, &port->com_type);
    sqlite_query_column(hStmt, 8, &port->connect_type);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_ptz_port(const char *pDbFile, struct ptz_port *port)
{
    if (!pDbFile || !port) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec),
             "update ptz_port set baudrate=%d,data_bit=%d,stop_bit=%d,parity_type=%d,protocol=%d,address=%d,com_type=%d,connect_type=%d where id=%d;",
             port->baudrate, port->data_bit, port->stop_bit, port->parity_type, port->protocol, port->address, port->com_type,
             port->connect_type, port->id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_ptz_ports(const char *pDbFile, struct ptz_port ports[], int *pCnt)
{
    if (!pDbFile || !ports) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select id,baudrate,data_bit,stop_bit,parity_type,protocol,address,com_type,connect_type from ptz_port;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *pCnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < MAX_CAMERA; i++) {
        sqlite_query_column(hStmt, 0, &ports[i].id);
        sqlite_query_column(hStmt, 1, &ports[i].baudrate);
        sqlite_query_column(hStmt, 2, &ports[i].data_bit);
        sqlite_query_column(hStmt, 3, &ports[i].stop_bit);
        sqlite_query_column(hStmt, 4, &ports[i].parity_type);
        sqlite_query_column(hStmt, 5, &ports[i].protocol);
        sqlite_query_column(hStmt, 6, &ports[i].address);
        sqlite_query_column(hStmt, 7, &ports[i].com_type);
        sqlite_query_column(hStmt, 8, &ports[i].connect_type);

        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    *pCnt = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_alarm_in(const char *pDbFile, struct alarm_in *alarm, int alarm_id)
{
    if (!pDbFile || !alarm || alarm_id < 0) {
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
    snprintf(sQuery, sizeof(sQuery), "select id, enable,name,type,tri_channels,tri_alarms,buzzer_interval,tri_channels_ex,\
		acto_ptz_channel,acto_ptz_preset_enable,acto_ptz_preset,acto_ptz_patrol_enable,acto_ptz_patrol,email_enable,email_buzzer_interval,\
		Names,tri_channels_snapshot,tri_channels_popup,tri_chnout1_alarms,tri_chnout2_alarms,popup_interval,ptzaction_interval,\
		alarmout_interval,whiteled_interval,email_pic_enable,tri_channels_pic,event_popup_layout,event_popup_channel_1,\
		event_popup_channel_2,event_popup_channel_3,event_popup_channel_4,tri_audio_id,http_notification_interval from alarm_in where id=%d;", alarm_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &alarm->id);
    sqlite_query_column(hStmt, 1, &alarm->enable);
    sqlite_query_column_text(hStmt, 2, alarm->name, sizeof(alarm->name));
    sqlite_query_column(hStmt, 3, &alarm->type);
    sqlite_query_column(hStmt, 4, (int *)&alarm->tri_channels);
    sqlite_query_column(hStmt, 5, (int *)&alarm->tri_alarms);
    sqlite_query_column(hStmt, 6, &alarm->buzzer_interval);
    sqlite_query_column_text(hStmt, 7, alarm->tri_channels_ex, sizeof(alarm->tri_channels_ex));
    sqlite_query_column(hStmt, 8, &alarm->acto_ptz_channel);
    sqlite_query_column(hStmt, 9, &alarm->acto_ptz_preset_enable);
    sqlite_query_column(hStmt, 10, &alarm->acto_ptz_preset);
    sqlite_query_column(hStmt, 11, &alarm->acto_ptz_patrol_enable);
    sqlite_query_column(hStmt, 12, &alarm->acto_ptz_patrol);
    sqlite_query_column(hStmt, 13, (int *)&alarm->email_enable);
    sqlite_query_column(hStmt, 14, &alarm->email_buzzer_interval);
    sqlite_query_column_text(hStmt, 15, alarm->name, sizeof(alarm->name));
    sqlite_query_column_text(hStmt, 16, alarm->tri_channels_snapshot, sizeof(alarm->tri_channels_snapshot));
    sqlite_query_column(hStmt, 17, &alarm->tri_channels_popup);
    sqlite_query_column_text(hStmt, 18, alarm->tri_chnout1_alarms, sizeof(alarm->tri_chnout1_alarms));
    sqlite_query_column_text(hStmt, 19, alarm->tri_chnout2_alarms, sizeof(alarm->tri_chnout2_alarms));
    sqlite_query_column(hStmt, 20, &alarm->popup_interval);
    sqlite_query_column(hStmt, 21, &alarm->ptzaction_interval);
    sqlite_query_column(hStmt, 22, &alarm->alarmout_interval);
    sqlite_query_column(hStmt, 23, &alarm->whiteled_interval);
    sqlite_query_column(hStmt, 24, &alarm->email_pic_enable);
    sqlite_query_column_text(hStmt, 25, alarm->tri_channels_pic, sizeof(alarm->tri_channels_pic));
    sqlite_query_column(hStmt, 26, &alarm->event_popup_layout);
    sqlite_query_column(hStmt, 27, &alarm->event_popup_channel[0]);
    sqlite_query_column(hStmt, 28, &alarm->event_popup_channel[1]);
    sqlite_query_column(hStmt, 29, &alarm->event_popup_channel[2]);
    sqlite_query_column(hStmt, 30, &alarm->event_popup_channel[3]);
    sqlite_query_column(hStmt, 31, &alarm->tri_audio_id);
    sqlite_query_column(hStmt, 32, &alarm->http_notification_interval);
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_alarm_ins(const char *pDbFile, struct alarm_in alarms[], int *pCnt)
{
    if (!pDbFile || !alarms) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery), "select id,enable,name,type,tri_channels,tri_alarms,buzzer_interval,tri_channels_ex,\
        email_enable,email_buzzer_interval,Names,tri_channels_snapshot,tri_chnout1_alarms,tri_chnout2_alarms,\
        popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval,email_pic_enable,tri_channels_pic,tri_channels_popup,event_popup_layout,\
        event_popup_channel_1,event_popup_channel_2,event_popup_channel_3,event_popup_channel_4,tri_audio_id,http_notification_interval from alarm_in;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *pCnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < MAX_ALARM_IN; i++) {
        sqlite_query_column(hStmt, 0, &alarms[i].id);
        sqlite_query_column(hStmt, 1, &alarms[i].enable);
        sqlite_query_column_text(hStmt, 2, alarms[i].name, sizeof(alarms[i].name));
        sqlite_query_column(hStmt, 3, &alarms[i].type);
        sqlite_query_column(hStmt, 4, (int *)&alarms[i].tri_channels);
        sqlite_query_column(hStmt, 5, (int *)&alarms[i].tri_alarms);
        sqlite_query_column(hStmt, 6, &alarms[i].buzzer_interval);
        sqlite_query_column_text(hStmt, 7, alarms[i].tri_channels_ex, sizeof(alarms[i].tri_channels_ex));
        sqlite_query_column(hStmt, 8, (int *)&alarms[i].email_enable);
        sqlite_query_column(hStmt, 9, &alarms[i].email_buzzer_interval);
        sqlite_query_column_text(hStmt, 10, alarms[i].name, sizeof(alarms[i].name));
        sqlite_query_column_text(hStmt, 11, alarms[i].tri_channels_snapshot, sizeof(alarms[i].tri_channels_snapshot));
        sqlite_query_column_text(hStmt, 12, alarms[i].tri_chnout1_alarms, sizeof(alarms[i].tri_chnout1_alarms));
        sqlite_query_column_text(hStmt, 13, alarms[i].tri_chnout2_alarms, sizeof(alarms[i].tri_chnout2_alarms));
        sqlite_query_column(hStmt, 14, &alarms[i].popup_interval);
        sqlite_query_column(hStmt, 15, &alarms[i].ptzaction_interval);
        sqlite_query_column(hStmt, 16, &alarms[i].alarmout_interval);
        sqlite_query_column(hStmt, 17, &alarms[i].whiteled_interval);
        sqlite_query_column(hStmt, 18, &alarms[i].email_pic_enable);
        sqlite_query_column_text(hStmt, 19, alarms[i].tri_channels_pic, sizeof(alarms[i].tri_channels_pic));
        sqlite_query_column(hStmt, 20, &alarms[i].tri_channels_popup);
        sqlite_query_column(hStmt, 21, &alarms[i].event_popup_layout);
        sqlite_query_column(hStmt, 22, &alarms[i].event_popup_channel[0]);
        sqlite_query_column(hStmt, 23, &alarms[i].event_popup_channel[1]);
        sqlite_query_column(hStmt, 24, &alarms[i].event_popup_channel[2]);
        sqlite_query_column(hStmt, 25, &alarms[i].event_popup_channel[3]);
        sqlite_query_column(hStmt, 26, &alarms[i].tri_audio_id);
        sqlite_query_column(hStmt, 27, &alarms[i].http_notification_interval);

        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    *pCnt = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_alarm_in(const char *pDbFile, struct alarm_in *alarm)
{
    if (!pDbFile || !alarm) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[2048] = {0};
    snprintf(sExec, sizeof(sExec), "update alarm_in set enable=%d,name='%s',type=%d, tri_channels=%u,tri_alarms=%u, \
		buzzer_interval =%d,tri_channels_ex='%s', acto_ptz_channel=%d, acto_ptz_preset_enable=%d ,acto_ptz_preset=%d ,\
		acto_ptz_patrol_enable=%d ,acto_ptz_patrol=%d ,email_enable=%d, email_buzzer_interval=%d,Names='%s',\
		tri_channels_snapshot='%s',tri_channels_popup=%d,tri_chnout1_alarms='%s',tri_chnout2_alarms='%s',\
		popup_interval=%d,ptzaction_interval=%d,alarmout_interval=%d,whiteled_interval=%d,email_pic_enable=%d,\
		tri_channels_pic='%s',event_popup_layout=%d,event_popup_channel_1=%d,event_popup_channel_2=%d,\
		event_popup_channel_3=%d,event_popup_channel_4=%d,tri_audio_id=%d,http_notification_interval=%d where id=%d;",
             alarm->enable, alarm->name, alarm->type, alarm->tri_channels, alarm->tri_alarms, alarm->buzzer_interval,
             alarm->tri_channels_ex, alarm->acto_ptz_channel, alarm->acto_ptz_preset_enable, alarm->acto_ptz_preset,
             alarm->acto_ptz_patrol_enable, alarm->acto_ptz_patrol,alarm->email_enable, alarm->email_buzzer_interval,
             alarm->name, alarm->tri_channels_snapshot, alarm->tri_channels_popup, alarm->tri_chnout1_alarms,
             alarm->tri_chnout2_alarms, alarm->popup_interval, alarm->ptzaction_interval, alarm->alarmout_interval,
             alarm->whiteled_interval,alarm->email_pic_enable, alarm->tri_channels_pic, alarm->event_popup_layout,
             alarm->event_popup_channel[0], alarm->event_popup_channel[1], alarm->event_popup_channel[2], 
             alarm->event_popup_channel[3], alarm->tri_audio_id, alarm->http_notification_interval, alarm->id);
    //msprintf("write_alarm_in sExec:%s len:%d\n", sExec, strlen(sExec));
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_alarm_ins(const char *pDbFile, struct alarm_in *alarms, int count, long long changeFlag)
{
    if (!pDbFile || !alarms) {
        return -1;
    }

    struct alarm_in *alarm = NULL;
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0;
    char sExec[2048] = {0};
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (i = 0; i < count && i < MAX_ALARM_IN; i++) {
        if (!(changeFlag >> i & 0x01)) {
            continue;
        }
        alarm = alarms + i;
        snprintf(sExec, sizeof(sExec), "update alarm_in set enable=%d,name='%s',type=%d, tri_channels=%u,tri_alarms=%u,\
			buzzer_interval =%d,tri_channels_ex='%s', acto_ptz_channel=%d, acto_ptz_preset_enable=%d ,acto_ptz_preset=%d ,\
			acto_ptz_patrol_enable=%d ,acto_ptz_patrol =%d ,email_enable=%d, email_buzzer_interval=%d,Names='%s',tri_channels_snapshot='%s',\
			tri_channels_popup=%d,tri_chnout1_alarms='%s',tri_chnout2_alarms='%s',popup_interval=%d,ptzaction_interval=%d,\
			alarmout_interval=%d,whiteled_interval=%d,email_pic_enable=%d,tri_channels_pic='%s',event_popup_layout=%d,event_popup_channel_1=%d,event_popup_channel_2=%d,\
			event_popup_channel_3=%d,event_popup_channel_4=%d,tri_audio_id=%d,http_notification_interval=%d where id=%d;",
                 alarm->enable, alarm->name, alarm->type, alarm->tri_channels, alarm->tri_alarms, alarm->buzzer_interval,
                 alarm->tri_channels_ex, alarm->acto_ptz_channel, alarm->acto_ptz_preset_enable, alarm->acto_ptz_preset,
                 alarm->acto_ptz_patrol_enable, alarm->acto_ptz_patrol, alarm->email_enable, alarm->email_buzzer_interval, 
                 alarm->name, alarm->tri_channels_snapshot, alarm->tri_channels_popup, alarm->tri_chnout1_alarms,
                 alarm->tri_chnout2_alarms, alarm->popup_interval, alarm->ptzaction_interval, alarm->alarmout_interval,
                 alarm->whiteled_interval, alarm->email_pic_enable, alarm->tri_channels_pic, alarm->event_popup_layout, 
                 alarm->event_popup_channel[0], alarm->event_popup_channel[1], alarm->event_popup_channel[2],
                 alarm->event_popup_channel[3], alarm->tri_audio_id, alarm->http_notification_interval, alarm->id);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_alarm_in_events(const char *pDbFile, struct alarm_in *alarms, int count, Uint64 changeFlag)
{
    if (!pDbFile || !alarms) {
        return -1;
    }

    struct alarm_in *alarm = alarms;
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0;
    char sExec[1024] = {0};
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (i = 0; i < count && i < MAX_ALARM_IN; i++) {
        if (!(changeFlag >> i & 0x01)) {
            continue;
        }
        snprintf(sExec, sizeof(sExec), "update alarm_in set enable=%d,type=%d,tri_alarms=%u,buzzer_interval =%d,\
			acto_ptz_channel=%d,acto_ptz_preset_enable=%d,acto_ptz_preset=%d,acto_ptz_patrol_enable=%d,\
			acto_ptz_patrol =%d, email_enable=%d, email_buzzer_interval=%d, tri_channels_snapshot='%s',\
			tri_channels_popup=%d,tri_chnout1_alarms='%s',tri_chnout2_alarms='%s', popup_interval=%d,\
			ptzaction_interval=%d, alarmout_interval=%d, whiteled_interval=%d,email_pic_enable=%d,event_popup_layout=%d,\
			event_popup_channel_1=%d,event_popup_channel_2=%d,event_popup_channel_3=%d,event_popup_channel_4=%d,\
			tri_audio_id=%d,http_notification_interval=%d where id=%d;",
                 alarm->enable, alarm->type, alarm->tri_alarms, alarm->buzzer_interval,
                 alarm->acto_ptz_channel, alarm->acto_ptz_preset_enable, alarm->acto_ptz_preset, alarm->acto_ptz_patrol_enable,
                 alarm->acto_ptz_patrol, alarm->email_enable, alarm->email_buzzer_interval, alarm->tri_channels_snapshot,
                 alarm->tri_channels_popup, alarm->tri_chnout1_alarms, alarm->tri_chnout2_alarms, alarm->popup_interval,
                 alarm->ptzaction_interval, alarm->alarmout_interval, alarm->whiteled_interval, alarm->email_pic_enable,
                 alarm->event_popup_layout,
                 alarm->event_popup_channel[0], alarm->event_popup_channel[1], alarm->event_popup_channel[2],
                 alarm->event_popup_channel[3], alarm->tri_audio_id, alarm->http_notification_interval, i);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_alarm_in_schedule(const char *pDbFile, struct alarm_in_schedule *schedule, int alarm_id)
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select wday_id,plan_id,start_time,end_time,action_type from alarm_in_schedule where alarm_id=%d;", alarm_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int nWeekId = 0;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(schedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));

        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_alarm_in_schedule(const char *pDbFile, struct alarm_in_schedule *schedule, int alarm_id)
{
    if (!pDbFile || !schedule || alarm_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "delete from alarm_in_schedule where alarm_id=%d;", alarm_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into alarm_in_schedule values(%d,%d,%d,'%s','%s',%d);", alarm_id, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}
int read_alarmin_audible_schedule(const char *pDbFile, struct alarm_in_schedule *schedule, int alarm_id)
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select week_id,plan_id,start_time,end_time,action_type from alarmin_audible_schedule where chn_id=%d;", alarm_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int nWeekId = 0;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(schedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));

        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_alarmin_audible_schedule(const char *pDbFile, struct alarm_in_schedule *schedule, int alarm_id)
{
    if (!pDbFile || !schedule || alarm_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "delete from alarmin_audible_schedule where chn_id=%d;", alarm_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into alarmin_audible_schedule values(%d,%d,%d,'%s','%s',%d);", alarm_id, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_alarmin_audible_schedules(const char *pDbFile, struct alarm_in_schedule *schedule, long long changeFlag)
{
    if (!pDbFile || !schedule) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[256] = {0};
    int i = 0, j = 0, k = 0;
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (k = 0; k < MAX_CAMERA; k++) {
        if (!(changeFlag >> k & 0x01)) {
            continue;
        }
        snprintf(sExec, sizeof(sExec), "delete from alarmin_audible_schedule where chn_id=%d;", k);
        sqlite_execute(hConn, mode, sExec);
        for (i = 0; i < MAX_DAY_NUM; i++) {
            for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
                if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                               schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                    continue;
                }
                snprintf(sExec, sizeof(sExec), "insert into alarmin_audible_schedule values(%d,%d,%d,'%s','%s',%d);", k, i, j,
                         schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                         schedule->schedule_day[i].schedule_item[j].action_type);
                sqlite_execute(hConn, mode, sExec);
            }
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_alarmin_ptz_schedule(const char *pDbFile, struct alarm_in_schedule *schedule, int alarm_id)
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select week_id,plan_id,start_time,end_time,action_type from alarmin_ptz_schedule where chn_id=%d;", alarm_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int nWeekId = 0;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(schedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));

        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_alarmin_ptz_schedule(const char *pDbFile, struct alarm_in_schedule *schedule, int alarm_id)
{
    if (!pDbFile || !schedule || alarm_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "delete from alarmin_ptz_schedule where chn_id=%d;", alarm_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into alarmin_ptz_schedule values(%d,%d,%d,'%s','%s',%d);", alarm_id, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_alarmin_ptz_schedules(const char *pDbFile, struct alarm_in_schedule *schedule, long long changeFlag)
{
    if (!pDbFile || !schedule) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[256] = {0};
    int i = 0, j = 0, k = 0;
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (k = 0; k < MAX_CAMERA; k++) {
        if (!(changeFlag >> k & 0x01)) {
            continue;
        }
        snprintf(sExec, sizeof(sExec), "delete from alarmin_ptz_schedule where chn_id=%d;", k);
        sqlite_execute(hConn, mode, sExec);
        for (i = 0; i < MAX_DAY_NUM; i++) {
            for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
                if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                               schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                    continue;
                }
                snprintf(sExec, sizeof(sExec), "insert into alarmin_ptz_schedule values(%d,%d,%d,'%s','%s',%d);", k, i, j,
                         schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                         schedule->schedule_day[i].schedule_item[j].action_type);
                sqlite_execute(hConn, mode, sExec);
            }
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_user(const char *pDbFile, struct db_user *user, int id)
{
    if (!pDbFile || !user || id < 0 || id > MAX_USER) {
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
    int nColumnCnt = 0;
    char sQuery[2048] = {0};
    snprintf(sQuery, sizeof(sQuery),
             "select id,enable,username,password,type,permission,remote_permission,"
             "local_live_view,local_playback,remote_live_view,remote_playback,"
             "local_live_view_ex,local_playback_ex,remote_live_view_ex,remote_playback_ex,"
             "password_ex,pattern_psw,"
             "perm_local_live,perm_local_playback,perm_local_retrieve,perm_local_smart,perm_local_event,perm_local_camera,perm_local_storage,perm_local_settings,perm_local_status,perm_local_shutdown,"
             "perm_remote_live,perm_remote_playback,perm_remote_retrieve,perm_remote_smart,perm_remote_event,perm_remote_camera,perm_remote_storage,perm_remote_settings,perm_remote_status,perm_remote_shutdown "
             "from user where id=%d;",
             id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, &(user->id));
    sqlite_query_column(hStmt, 1, &(user->enable));
    sqlite_query_column_text(hStmt, 2, user->username, sizeof(user->username));
    sqlite_query_column_text(hStmt, 3, user->password, sizeof(user->password));
    sqlite_query_column(hStmt, 4, &(user->type));
    sqlite_query_column(hStmt, 5, &(user->permission));
    sqlite_query_column(hStmt, 6, &(user->remote_permission));
    sqlite_query_column(hStmt, 7, (int *) & (user->local_live_view));
    sqlite_query_column(hStmt, 8, (int *) & (user->local_playback));
    sqlite_query_column(hStmt, 9, (int *) & (user->remote_live_view));
    sqlite_query_column(hStmt, 10, (int *) & (user->remote_playback));
    sqlite_query_column_text(hStmt, 11, user->local_live_view_ex, sizeof(user->local_live_view_ex));
    sqlite_query_column_text(hStmt, 12, user->local_playback_ex, sizeof(user->local_playback_ex));
    sqlite_query_column_text(hStmt, 13, user->remote_live_view_ex, sizeof(user->remote_live_view_ex));
    sqlite_query_column_text(hStmt, 14, user->remote_playback_ex, sizeof(user->remote_playback_ex));
    sqlite_query_column_text(hStmt, 15, user->password_ex, sizeof(user->password_ex));
    sqlite_query_column_text(hStmt, 16, user->pattern_psw, sizeof(user->pattern_psw));
    sqlite_query_column(hStmt, 17, (int*)&(user->perm_local_live));
    sqlite_query_column(hStmt, 18, (int*)&(user->perm_local_playback));
    sqlite_query_column(hStmt, 19, (int*)&(user->perm_local_retrieve));
    sqlite_query_column(hStmt, 20, (int*)&(user->perm_local_smart));
    sqlite_query_column(hStmt, 21, (int*)&(user->perm_local_event));
    sqlite_query_column(hStmt, 22, (int*)&(user->perm_local_camera));
    sqlite_query_column(hStmt, 23, (int*)&(user->perm_local_storage));
    sqlite_query_column(hStmt, 24, (int*)&(user->perm_local_settings));
    sqlite_query_column(hStmt, 25, (int*)&(user->perm_local_status));
    sqlite_query_column(hStmt, 26, (int*)&(user->perm_local_shutdown));
    sqlite_query_column(hStmt, 27, (int*)&(user->perm_remote_live));
    sqlite_query_column(hStmt, 28, (int*)&(user->perm_remote_playback));
    sqlite_query_column(hStmt, 29, (int*)&(user->perm_remote_retrieve));
    sqlite_query_column(hStmt, 30, (int*)&(user->perm_remote_smart));
    sqlite_query_column(hStmt, 31, (int*)&(user->perm_remote_event));
    sqlite_query_column(hStmt, 32, (int*)&(user->perm_remote_camera));
    sqlite_query_column(hStmt, 33, (int*)&(user->perm_remote_storage));
    sqlite_query_column(hStmt, 34, (int*)&(user->perm_remote_settings));
    sqlite_query_column(hStmt, 35, (int*)&(user->perm_remote_status));
    sqlite_query_column(hStmt, 36, (int*)&(user->perm_remote_shutdown));

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_user_by_name(const char *pDbFile, struct db_user *user, const char *username)
{
    if (!pDbFile || !user || !username) {
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
    int nColumnCnt = 0;
    char sQuery[2048] = {0};
    snprintf(sQuery, sizeof(sQuery),
             "select id,enable,username,password,type,permission,remote_permission,"
             "local_live_view,local_playback,remote_live_view,remote_playback,"
             "local_live_view_ex,local_playback_ex,remote_live_view_ex,remote_playback_ex,"
             "password_ex,pattern_psw,"
             "perm_local_live,perm_local_playback,perm_local_retrieve,perm_local_smart,perm_local_event,perm_local_camera,perm_local_storage,perm_local_settings,perm_local_status,perm_local_shutdown,"
             "perm_remote_live,perm_remote_playback,perm_remote_retrieve,perm_remote_smart,perm_remote_event,perm_remote_camera,perm_remote_storage,perm_remote_settings,perm_remote_status,perm_remote_shutdown "
             "from user where username='%s';",
             username);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, &(user->id));
    sqlite_query_column(hStmt, 1, &(user->enable));
    sqlite_query_column_text(hStmt, 2, user->username, sizeof(user->username));
    sqlite_query_column_text(hStmt, 3, user->password, sizeof(user->password));
    sqlite_query_column(hStmt, 4, &(user->type));
    sqlite_query_column(hStmt, 5, &(user->permission));
    sqlite_query_column(hStmt, 6, &(user->remote_permission));
    sqlite_query_column(hStmt, 7, (int *) & (user->local_live_view));
    sqlite_query_column(hStmt, 8, (int *) & (user->local_playback));
    sqlite_query_column(hStmt, 9, (int *) & (user->remote_live_view));
    sqlite_query_column(hStmt, 10, (int *) & (user->remote_playback));
    sqlite_query_column_text(hStmt, 11, user->local_live_view_ex, sizeof(user->local_live_view_ex));
    sqlite_query_column_text(hStmt, 12, user->local_playback_ex, sizeof(user->local_playback_ex));
    sqlite_query_column_text(hStmt, 13, user->remote_live_view_ex, sizeof(user->remote_live_view_ex));
    sqlite_query_column_text(hStmt, 14, user->remote_playback_ex, sizeof(user->remote_playback_ex));
    sqlite_query_column_text(hStmt, 15, user->password_ex, sizeof(user->password_ex));
    sqlite_query_column_text(hStmt, 16, user->pattern_psw, sizeof(user->pattern_psw));
    sqlite_query_column(hStmt, 17, (int*)&(user->perm_local_live));
    sqlite_query_column(hStmt, 18, (int*)&(user->perm_local_playback));
    sqlite_query_column(hStmt, 19, (int*)&(user->perm_local_retrieve));
    sqlite_query_column(hStmt, 20, (int*)&(user->perm_local_smart));
    sqlite_query_column(hStmt, 21, (int*)&(user->perm_local_event));
    sqlite_query_column(hStmt, 22, (int*)&(user->perm_local_camera));
    sqlite_query_column(hStmt, 23, (int*)&(user->perm_local_storage));
    sqlite_query_column(hStmt, 24, (int*)&(user->perm_local_settings));
    sqlite_query_column(hStmt, 25, (int*)&(user->perm_local_status));
    sqlite_query_column(hStmt, 26, (int*)&(user->perm_local_shutdown));
    sqlite_query_column(hStmt, 27, (int*)&(user->perm_remote_live));
    sqlite_query_column(hStmt, 28, (int*)&(user->perm_remote_playback));
    sqlite_query_column(hStmt, 29, (int*)&(user->perm_remote_retrieve));
    sqlite_query_column(hStmt, 30, (int*)&(user->perm_remote_smart));
    sqlite_query_column(hStmt, 31, (int*)&(user->perm_remote_event));
    sqlite_query_column(hStmt, 32, (int*)&(user->perm_remote_camera));
    sqlite_query_column(hStmt, 33, (int*)&(user->perm_remote_storage));
    sqlite_query_column(hStmt, 34, (int*)&(user->perm_remote_settings));
    sqlite_query_column(hStmt, 35, (int*)&(user->perm_remote_status));
    sqlite_query_column(hStmt, 36, (int*)&(user->perm_remote_shutdown));

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

/**
 * @brief read_users
 * 这个会在数据库升级前调用，所以要判断数据库新增字段
 * @param pDbFile
 * @param users
 * @param pCnt
 * @return
 */
int read_users(const char *pDbFile, struct db_user users[], int *pCnt)
{
    if (!pDbFile || !users) {
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
    int i = 0;
    //图案解锁
    int pattern_psw_exist = check_table_key_ex(pDbFile, "user", "pattern_psw");
    //权限细分，每个模块加了一个权限字段
    int perm_local_live_exist = check_table_key_ex(pDbFile, "user", "perm_local_live");
    //
    snprintf(sQuery, sizeof(sQuery),
             "select id,enable,username,password,type,permission,remote_permission,"
             "local_live_view,local_playback,remote_live_view,remote_playback,"
             "local_live_view_ex,local_playback_ex,remote_live_view_ex,remote_playback_ex,"
             "password_ex");
    if (pattern_psw_exist == 1) {
        snprintf(sQuery + strlen(sQuery), sizeof(sQuery) - strlen(sQuery),
                 ",pattern_psw");
    }
    if (perm_local_live_exist) {
        snprintf(sQuery + strlen(sQuery), sizeof(sQuery) - strlen(sQuery),
                 ",perm_local_live,perm_local_playback,perm_local_retrieve,perm_local_smart,perm_local_event,perm_local_camera,perm_local_storage,perm_local_settings,perm_local_status,perm_local_shutdown,"
                 "perm_remote_live,perm_remote_playback,perm_remote_retrieve,perm_remote_smart,perm_remote_event,perm_remote_camera,perm_remote_storage,perm_remote_settings,perm_remote_status,perm_remote_shutdown");
    }
    snprintf(sQuery + strlen(sQuery), sizeof(sQuery) - strlen(sQuery),
             " from user;");
    //
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *pCnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < MAX_USER; i++) {
        int column = 0;
        sqlite_query_column(hStmt, column++, &(users[i].id));
        sqlite_query_column(hStmt, column++, &(users[i].enable));
        sqlite_query_column_text(hStmt, column++, users[i].username, sizeof(users[i].username));
        sqlite_query_column_text(hStmt, column++, users[i].password, sizeof(users[i].password));
        sqlite_query_column(hStmt, column++, &(users[i].type));
        sqlite_query_column(hStmt, column++, &(users[i].permission));
        sqlite_query_column(hStmt, column++, &(users[i].remote_permission));
        sqlite_query_column(hStmt, column++, (int *) & (users[i].local_live_view));
        sqlite_query_column(hStmt, column++, (int *) & (users[i].local_playback));
        sqlite_query_column(hStmt, column++, (int *) & (users[i].remote_live_view));
        sqlite_query_column(hStmt, column++, (int *) & (users[i].remote_playback));
        sqlite_query_column_text(hStmt, column++, users[i].local_live_view_ex, sizeof(users[i].local_live_view_ex));
        sqlite_query_column_text(hStmt, column++, users[i].local_playback_ex, sizeof(users[i].local_playback_ex));
        sqlite_query_column_text(hStmt, column++, users[i].remote_live_view_ex, sizeof(users[i].remote_live_view_ex));
        sqlite_query_column_text(hStmt, column++, users[i].remote_playback_ex, sizeof(users[i].remote_playback_ex));
        sqlite_query_column_text(hStmt, column++, users[i].password_ex, sizeof(users[i].password_ex));
        if (pattern_psw_exist) {
            sqlite_query_column_text(hStmt, column++, users[i].pattern_psw, sizeof(users[i].pattern_psw));
        }
        if (perm_local_live_exist) {
            sqlite_query_column(hStmt, column++, (int*)&(users[i].perm_local_live));
            sqlite_query_column(hStmt, column++, (int*)&(users[i].perm_local_playback));
            sqlite_query_column(hStmt, column++, (int*)&(users[i].perm_local_retrieve));
            sqlite_query_column(hStmt, column++, (int*)&(users[i].perm_local_smart));
            sqlite_query_column(hStmt, column++, (int*)&(users[i].perm_local_event));
            sqlite_query_column(hStmt, column++, (int*)&(users[i].perm_local_camera));
            sqlite_query_column(hStmt, column++, (int*)&(users[i].perm_local_storage));
            sqlite_query_column(hStmt, column++, (int*)&(users[i].perm_local_settings));
            sqlite_query_column(hStmt, column++, (int*)&(users[i].perm_local_status));
            sqlite_query_column(hStmt, column++, (int*)&(users[i].perm_local_shutdown));
            sqlite_query_column(hStmt, column++, (int*)&(users[i].perm_remote_live));
            sqlite_query_column(hStmt, column++, (int*)&(users[i].perm_remote_playback));
            sqlite_query_column(hStmt, column++, (int*)&(users[i].perm_remote_retrieve));
            sqlite_query_column(hStmt, column++, (int*)&(users[i].perm_remote_smart));
            sqlite_query_column(hStmt, column++, (int*)&(users[i].perm_remote_event));
            sqlite_query_column(hStmt, column++, (int*)&(users[i].perm_remote_camera));
            sqlite_query_column(hStmt, column++, (int*)&(users[i].perm_remote_storage));
            sqlite_query_column(hStmt, column++, (int*)&(users[i].perm_remote_settings));
            sqlite_query_column(hStmt, column++, (int*)&(users[i].perm_remote_status));
            sqlite_query_column(hStmt, column++, (int*)&(users[i].perm_remote_shutdown));
        }

        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    *pCnt = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int add_user(const char *pDbFile, struct db_user *user)
{
    if (!pDbFile || !user) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    HSQLSTMT hStmt = 0;
    int nColumnCnt = 0;
    int nId = 0;
    char sQuery[2048] = {0};
    snprintf(sQuery, sizeof(sQuery), "select id from user order by(enable);");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &nId);
    sqlite_clear_stmt(hStmt);

    char buf1[MAX_PWD_LEN * 2 + 2] = {0};
    char buf2[MAX_PWD_LEN * 2 + 2] = {0};
    translate_pwd(buf1, user->password, strlen(user->password));
    translate_pwd(buf2, user->password_ex, strlen(user->password_ex));

    if (strlen(user->pattern_psw) < 4) {
        snprintf(user->pattern_psw, sizeof(user->pattern_psw), "0");
    }

    int pattern = atoi(user->pattern_psw);
    if (pattern < 1234 || pattern > 987654321) {
        snprintf(user->pattern_psw, sizeof(user->pattern_psw), "0");
    }

    user->id = nId;
    snprintf(sQuery, sizeof(sQuery),
             "update user set enable=%d,username='%s',password='%s',type=%d,permission=%d,remote_permission=%d"
             ",local_live_view=%ld,local_playback=%ld,remote_live_view=%ld,remote_playback=%ld"
             ",local_live_view_ex='%s',local_playback_ex='%s',remote_live_view_ex='%s',remote_playback_ex='%s'"
             ",password_ex='%s',pattern_psw='%s'"
             ",perm_local_live=%d,perm_local_playback=%d,perm_local_retrieve=%d,perm_local_smart=%d,perm_local_event=%d,perm_local_camera=%d,perm_local_storage=%d,perm_local_settings=%d,perm_local_status=%d,perm_local_shutdown=%d"
             ",perm_remote_live=%d,perm_remote_playback=%d,perm_remote_retrieve=%d,perm_remote_smart=%d,perm_remote_event=%d,perm_remote_camera=%d,perm_remote_storage=%d,perm_remote_settings=%d,perm_remote_status=%d,perm_remote_shutdown=%d"
             " where id=%d;",
             user->enable, user->username, buf1, user->type, user->permission, user->remote_permission,
             user->local_live_view,user->local_playback, user->remote_live_view, user->remote_playback,
             user->local_live_view_ex, user->local_playback_ex,user->remote_live_view_ex, user->remote_playback_ex,
             buf2, user->pattern_psw,
             user->perm_local_live,user->perm_local_playback,user->perm_local_retrieve,user->perm_local_smart,user->perm_local_event,user->perm_local_camera,user->perm_local_storage,user->perm_local_settings,user->perm_local_status,user->perm_local_shutdown,
             user->perm_remote_live,user->perm_remote_playback,user->perm_remote_retrieve,user->perm_remote_smart,user->perm_remote_event,user->perm_remote_camera,user->perm_remote_storage,user->perm_remote_settings,user->perm_remote_status,user->perm_remote_shutdown,
             nId);
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    sqlite_execute(hConn, mode, sQuery);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_user(const char *pDbFile, struct db_user *user)
{
    if (!pDbFile || !user) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[2048] = {0};
    char buf1[MAX_PWD_LEN * 2 + 2] = {0};
    char buf2[MAX_PWD_LEN * 2 + 2] = {0};
    translate_pwd(buf1, user->password, strlen(user->password));
    translate_pwd(buf2, user->password_ex, strlen(user->password_ex));

    snprintf(sExec, sizeof(sExec),
             "update user set enable=%d,username='%s',password='%s',type=%d,permission=%d,remote_permission=%d"
             ",local_live_view=%ld,local_playback=%ld,remote_live_view=%ld,remote_playback=%ld"
             ",local_live_view_ex='%s',local_playback_ex='%s',remote_live_view_ex='%s',remote_playback_ex='%s'"
             ",password_ex='%s',pattern_psw='%s'"
             ",perm_local_live=%d,perm_local_playback=%d,perm_local_retrieve=%d,perm_local_smart=%d,perm_local_event=%d,perm_local_camera=%d,perm_local_storage=%d,perm_local_settings=%d,perm_local_status=%d,perm_local_shutdown=%d"
             ",perm_remote_live=%d,perm_remote_playback=%d,perm_remote_retrieve=%d,perm_remote_smart=%d,perm_remote_event=%d,perm_remote_camera=%d,perm_remote_storage=%d,perm_remote_settings=%d,perm_remote_status=%d,perm_remote_shutdown=%d"
             " where id=%d;",
             user->enable, user->username, buf1, user->type, user->permission, user->remote_permission,
             user->local_live_view,user->local_playback, user->remote_live_view, user->remote_playback,
             user->local_live_view_ex, user->local_playback_ex,user->remote_live_view_ex, user->remote_playback_ex,
             buf2, user->pattern_psw,
             user->perm_local_live,user->perm_local_playback,user->perm_local_retrieve,user->perm_local_smart,user->perm_local_event,user->perm_local_camera,user->perm_local_storage,user->perm_local_settings,user->perm_local_status,user->perm_local_shutdown,
             user->perm_remote_live,user->perm_remote_playback,user->perm_remote_retrieve,user->perm_remote_smart,user->perm_remote_event,user->perm_remote_camera,user->perm_remote_storage,user->perm_remote_settings,user->perm_remote_status,user->perm_remote_shutdown,
             user->id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_user_count(const char *pDbFile, int *pCnt)
{
    if (!pDbFile || !pCnt) {
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
    snprintf(sQuery, sizeof(sQuery), "select count(*) from user where enable=1;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, pCnt);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

/**
 * @brief get_user_default_permission
 * 14版本重新定义了用户默认权限，根据user.type生成默认权限
 * @param user
 * @return
 */
int get_user_default_permission(struct db_user *user)
{
    int i;
    switch (user->type) {
    case USERLEVEL_OPERATOR:
        memset(user->local_live_view_ex, 0, sizeof(user->local_live_view_ex));
        memset(user->local_playback_ex, 0, sizeof(user->local_playback_ex));
        memset(user->remote_live_view_ex, 0, sizeof(user->remote_live_view_ex));
        memset(user->remote_playback_ex, 0, sizeof(user->remote_playback_ex));
        for (i = 0; i < MAX_LEN_64; ++i) {
            user->local_live_view_ex[i] = '1';
            user->local_playback_ex[i] = '1';
            user->remote_live_view_ex[i] = '1';
            user->remote_playback_ex[i] = '1';
        }
        user->perm_local_live = PERM_LIVE_ALL;
        user->perm_local_playback = PERM_PLAYBACK_ALL;
        user->perm_local_retrieve = PERM_RETRIEVE_ALL;
        user->perm_local_smart = PERM_SMART_ALL;
        user->perm_local_event = PERM_EVENT_ALL;
        user->perm_local_camera = PERM_CAMERA_ALL;
        user->perm_local_storage = PERM_STORAGE_ALL;
        user->perm_local_settings = PERM_SETTINGS_NONE;
        user->perm_local_status = PERM_STATUS_ALL;
        user->perm_local_shutdown = PERM_SHUTDOWN_NONE;

        user->perm_remote_live = PERM_LIVE_ALL;
        user->perm_remote_playback = PERM_PLAYBACK_ALL;
        user->perm_remote_retrieve = PERM_RETRIEVE_ALL;
        user->perm_remote_smart = PERM_SMART_ALL;
        user->perm_remote_event = PERM_EVENT_ALL;
        user->perm_remote_camera = PERM_CAMERA_ALL;
        user->perm_remote_storage = PERM_STORAGE_ALL;
        user->perm_remote_settings = PERM_SETTINGS_NONE;
        user->perm_remote_status = PERM_STATUS_ALL;
        user->perm_remote_shutdown = PERM_SHUTDOWN_NONE;
        break;
    case USERLEVEL_USER:
        memset(user->local_live_view_ex, 0, sizeof(user->local_live_view_ex));
        memset(user->local_playback_ex, 0, sizeof(user->local_playback_ex));
        memset(user->remote_live_view_ex, 0, sizeof(user->remote_live_view_ex));
        memset(user->remote_playback_ex, 0, sizeof(user->remote_playback_ex));
        for (i = 0; i < MAX_LEN_64; ++i) {
            user->local_live_view_ex[i] = '1';
            user->local_playback_ex[i] = '1';
            user->remote_live_view_ex[i] = '1';
            user->remote_playback_ex[i] = '1';
        }
        user->perm_local_live = PERM_LIVE_NONE;
        user->perm_local_playback = PERM_PLAYBACK_NONE;
        user->perm_local_retrieve = PERM_RETRIEVE_NONE;
        user->perm_local_smart = PERM_SMART_NONE;
        user->perm_local_event = PERM_EVENT_NONE;
        user->perm_local_camera = PERM_CAMERA_NONE;
        user->perm_local_storage = PERM_STORAGE_NONE;
        user->perm_local_settings = PERM_SETTINGS_NONE;
        user->perm_local_status = PERM_STATUS_ALL;
        user->perm_local_shutdown = PERM_SHUTDOWN_NONE;

        user->perm_remote_live = PERM_LIVE_NONE;
        user->perm_remote_playback = PERM_PLAYBACK_NONE;
        user->perm_remote_retrieve = PERM_RETRIEVE_NONE;
        user->perm_remote_smart = PERM_SMART_NONE;
        user->perm_remote_event = PERM_EVENT_NONE;
        user->perm_remote_camera = PERM_CAMERA_NONE;
        user->perm_remote_storage = PERM_STORAGE_NONE;
        user->perm_remote_settings = PERM_SETTINGS_NONE;
        user->perm_remote_status = PERM_STATUS_ALL;
        user->perm_remote_shutdown = PERM_SHUTDOWN_NONE;
        break;
    default:
        msprintf("error user.type: %d", user->type);
        break;
    }
    return 0;
}

int read_display(const char *pDbFile, struct display *display)
{
    if (!pDbFile || !display) {
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
    int nColumnCnt = 0;
    char sQuery[512] = {0};
    snprintf(sQuery, sizeof(sQuery),
             "select main_resolution,main_seq_enable,main_seq_interval,audio_out_on,hdmi_audio_on,volume,stereo,sub_resolution,sub_seq_enable,sub_seq_interval,spot_out_channel,spot_resolution,border_line_on,date_format,show_channel_name,sub_enable,camera_info,start_screen,page_info,eventPop_screen,eventPop_time,time_info,occupancy_screen,event_region,font_size from display;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, &(display->main_resolution));
    sqlite_query_column(hStmt, 1, &(display->main_seq_enable));
    sqlite_query_column(hStmt, 2, &(display->main_seq_interval));
    sqlite_query_column(hStmt, 3, &(display->audio_output_on));
    sqlite_query_column(hStmt, 4, &(display->hdmi_audio_on));
    sqlite_query_column(hStmt, 5, &(display->volume));
    sqlite_query_column(hStmt, 6, &(display->stereo));

    sqlite_query_column(hStmt, 7, &(display->sub_resolution));
    sqlite_query_column(hStmt, 8, &(display->sub_seq_enable));
    sqlite_query_column(hStmt, 9, &(display->sub_seq_interval));
    sqlite_query_column(hStmt, 10, &(display->spot_output_channel));
    sqlite_query_column(hStmt, 11, &(display->spot_resolution));

    sqlite_query_column(hStmt, 12, &(display->border_line_on));
    sqlite_query_column(hStmt, 13, &(display->date_format));
    sqlite_query_column(hStmt, 14, &(display->show_channel_name));
    sqlite_query_column(hStmt, 15, &(display->sub_enable));
    sqlite_query_column(hStmt, 16, &(display->camera_info));
    sqlite_query_column(hStmt, 17, &(display->start_screen));
    sqlite_query_column(hStmt, 18, &(display->page_info));
    sqlite_query_column(hStmt, 19, &(display->eventPop_screen));
    sqlite_query_column(hStmt, 20, &(display->eventPop_time));
    sqlite_query_column(hStmt, 21, &(display->time_info));
    sqlite_query_column(hStmt, 22, &(display->occupancy_screen));
    sqlite_query_column(hStmt, 23, &(display->event_region));
    sqlite_query_column(hStmt, 24, (int *)&(display->fontSize));

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

//8.0.2-oem/////
int read_display_oem(const char *pDbFile, struct display *display)
{
    if (!pDbFile || !display) {
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
    int nColumnCnt = 0;
    char sQuery[512] = {0};
    snprintf(sQuery, sizeof(sQuery),
             "select main_resolution,main_seq_enable,main_seq_interval,audio_out_on,hdmi_audio_on,volume,stereo,sub_resolution,sub_seq_enable,sub_seq_interval,spot_out_channel,spot_resolution,border_line_on,date_format,show_channel_name,sub_enable from display;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, &(display->main_resolution));
    sqlite_query_column(hStmt, 1, &(display->main_seq_enable));
    sqlite_query_column(hStmt, 2, &(display->main_seq_interval));
    sqlite_query_column(hStmt, 3, &(display->audio_output_on));
    sqlite_query_column(hStmt, 4, &(display->hdmi_audio_on));
    sqlite_query_column(hStmt, 5, &(display->volume));
    sqlite_query_column(hStmt, 6, &(display->stereo));

    sqlite_query_column(hStmt, 7, &(display->sub_resolution));
    sqlite_query_column(hStmt, 8, &(display->sub_seq_enable));
    sqlite_query_column(hStmt, 9, &(display->sub_seq_interval));
    sqlite_query_column(hStmt, 10, &(display->spot_output_channel));
    sqlite_query_column(hStmt, 11, &(display->spot_resolution));

    sqlite_query_column(hStmt, 12, &(display->border_line_on));
    sqlite_query_column(hStmt, 13, &(display->date_format));
    sqlite_query_column(hStmt, 14, &(display->show_channel_name));
    sqlite_query_column(hStmt, 15, &(display->sub_enable));

    //////////for db-1014.txt
    display->camera_info = 0;
    display->start_screen = 0;
    display->page_info = 0;
    display->eventPop_screen = 0;
    display->eventPop_time = 11;
    display->main_seq_interval = 0;
    display->sub_seq_interval = 0;
    //////////end

    // for db-9.0.16-1002.txt start
    display->fontSize = 1;
    // for db-9.0.16-1002.txt end

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_display(const char *pDbFile, struct display *display)
{
    if (!pDbFile || !display) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec),
             "update display set main_resolution=%d,main_seq_enable=%d,main_seq_interval=%d,audio_out_on=%d,hdmi_audio_on=%d,volume=%d,stereo=%d,sub_resolution=%d,sub_seq_enable=%d,sub_seq_interval=%d,spot_out_channel=%d,spot_resolution=%d,border_line_on=%d,date_format=%d,show_channel_name=%d,sub_enable=%d,camera_info=%d,start_screen=%d,page_info=%d,eventPop_screen=%d,eventPop_time=%d,time_info=%d,occupancy_screen=%d,event_region=%d,font_size=%d;",
             \
             display->main_resolution, display->main_seq_enable, display->main_seq_interval, display->audio_output_on,
             display->hdmi_audio_on, display->volume, display->stereo, display->sub_resolution, display->sub_seq_enable,
             display->sub_seq_interval, display->spot_output_channel, display->spot_resolution, display->border_line_on,
             display->date_format, display->show_channel_name, display->sub_enable, display->camera_info, display->start_screen, \
             display->page_info, display->eventPop_screen, display->eventPop_time, display->time_info, display->occupancy_screen,
             display->event_region, display->fontSize);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_spots(const char *pDbFile, struct spot spots[], int *pCnt)
{
    if (!pDbFile || !spots) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        *pCnt = 0;
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    int nColumnCnt = 0;
    int i = 0;
    char sQuery[512] = {0};
    snprintf(sQuery, sizeof(sQuery), "select id,seq_enable, seq_interval, input_channels from spot;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0) {
        *pCnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    for (i = 0; i < DVR_SPOT_OUT_CH; i++) {
        sqlite_query_column(hStmt, 0, &(spots[i].id));
        sqlite_query_column(hStmt, 1, &(spots[i].seq_enable));
        sqlite_query_column(hStmt, 2, &(spots[i].seq_interval));
        sqlite_query_column(hStmt, 3, &(spots[i].input_channels));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    *pCnt = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_spot(const char *pDbFile, struct spot *spot)
{
    if (!pDbFile || !spot) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec), "update spot set seq_enable=%d, seq_interval=%d,input_channels=%d where id=%d;",
             spot->seq_enable, spot->seq_interval, spot->input_channels, spot->id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_time(const char *pDbFile, struct time *times)
{
    if (!pDbFile || !times) {
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
    int nColumnCnt = 0;
    char sQuery[512] = {0};
    snprintf(sQuery, sizeof(sQuery),
             "select ntp_enable,dst_enable,time_zone,time_zone_name,ntp_server,sync_enable,sync_interval,sync_pc from time;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &(times->ntp_enable));
    sqlite_query_column(hStmt, 1, &(times->dst_enable));
    sqlite_query_column_text(hStmt, 2, times->time_zone, sizeof(times->time_zone));
    sqlite_query_column_text(hStmt, 3, times->time_zone_name, sizeof(times->time_zone_name));
    sqlite_query_column_text(hStmt, 4, times->ntp_server, sizeof(times->ntp_server));
    sqlite_query_column(hStmt, 5, &(times->sync_enable));
    sqlite_query_column(hStmt, 6, &(times->sync_interval));
    sqlite_query_column(hStmt, 7, &(times->sync_pc));

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_time(const char *pDbFile, struct time *times)
{
    if (!pDbFile || !times) {
        return -1;
    }
    
    char tmp[128] = {0};
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    // sql use "''" to indicate "'" 
    snprintf(tmp, sizeof(tmp), times->ntp_server);
    translate_pwd(tmp, times->ntp_server, strlen(times->ntp_server));
    
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec),
             "update time set ntp_enable=%d,dst_enable=%d,time_zone='%s',time_zone_name='%s',ntp_server='%s',sync_enable='%d',sync_interval='%d',sync_pc='%d';",
             times->ntp_enable, times->dst_enable, times->time_zone, times->time_zone_name, tmp, times->sync_enable,
             times->sync_interval, times->sync_pc);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_pppoe(const char *pDbFile, struct pppoe *pppoe)
{
    if (!pDbFile || !pppoe) {
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
    int nColumnCnt = 0;
    char sQuery[512] = {0};
    snprintf(sQuery, sizeof(sQuery), "select enable,auto_connect,username,password from pppoe;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &(pppoe->enable));
    sqlite_query_column(hStmt, 1, &(pppoe->auto_connect));
    sqlite_query_column_text(hStmt, 2, pppoe->username, sizeof(pppoe->username));
    sqlite_query_column_text(hStmt, 3, pppoe->password, sizeof(pppoe->password));

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_pppoe(const char *pDbFile, struct pppoe *pppoe)
{
    if (!pDbFile || !pppoe) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    char tmpUsername[128] = {0};
    char tmpPassword[128] = {0};
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    translate_pwd(tmpUsername, pppoe->username, strlen(pppoe->username));
    translate_pwd(tmpPassword, pppoe->password, strlen(pppoe->password));
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec), "update pppoe set enable=%d, auto_connect=%d, username='%s', password='%s';",
             pppoe->enable, pppoe->auto_connect, tmpUsername, tmpPassword);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_ddns(const char *pDbFile, struct ddns *ddns)
{
    if (!pDbFile || !ddns) {
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
    int nColumnCnt = 0;
    char sQuery[512] = {0};
    snprintf(sQuery, sizeof(sQuery),
             "select enable, domain, username, password, host_name, free_dns_hash, update_freq, http_port, rtsp_port, ddnsurl from ddns;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &(ddns->enable));
    sqlite_query_column_text(hStmt, 1, ddns->domain, sizeof(ddns->domain));
    sqlite_query_column_text(hStmt, 2, ddns->username, sizeof(ddns->username));
    sqlite_query_column_text(hStmt, 3, ddns->password, sizeof(ddns->password));
    sqlite_query_column_text(hStmt, 4, ddns->host_name, sizeof(ddns->host_name));
    sqlite_query_column_text(hStmt, 5, ddns->free_dns_hash, sizeof(ddns->free_dns_hash));
    sqlite_query_column(hStmt, 6, &(ddns->update_freq));
    sqlite_query_column(hStmt, 7, &(ddns->http_port));
    sqlite_query_column(hStmt, 8, &(ddns->rtsp_port));
    sqlite_query_column_text(hStmt, 9, ddns->ddnsurl, sizeof(ddns->ddnsurl));

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_ddns(const char *pDbFile, struct ddns *ddns)
{
    if (!pDbFile || !ddns) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec),
             "update ddns set enable=%d,domain='%s',username='%s', password='%s', host_name='%s', free_dns_hash='%s', update_freq=%d, http_port=%d, rtsp_port=%d, ddnsurl='%s';",
             ddns->enable, ddns->domain, ddns->username, ddns->password, ddns->host_name, ddns->free_dns_hash, ddns->update_freq,
             ddns->http_port, ddns->rtsp_port, ddns->ddnsurl);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_upnp(const char *pDbFile, struct upnp *upnp)
{
    if (!pDbFile || !upnp) {
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
    int nColumnCnt = 0;
    char sQuery[512] = {0};
    snprintf(sQuery, sizeof(sQuery),
             "select enable, type, name, extern_ip, http_port, http_status, rtsp_port, rtsp_status from upnp;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &(upnp->enable));
    sqlite_query_column(hStmt, 1, &(upnp->type));
    sqlite_query_column_text(hStmt, 2, upnp->name, sizeof(upnp->name));
    sqlite_query_column_text(hStmt, 3, upnp->extern_ip, sizeof(upnp->extern_ip));
    sqlite_query_column(hStmt, 4, &(upnp->http_port));
    sqlite_query_column(hStmt, 5, &(upnp->http_status));
    sqlite_query_column(hStmt, 6, &(upnp->rtsp_port));
    sqlite_query_column(hStmt, 7, &(upnp->rtsp_status));

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}


int write_upnp(const char *pDbFile, struct upnp *upnp)
{
    if (!pDbFile || !upnp) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec),
             "update upnp set enable=%d,type=%d,name='%s', extern_ip='%s', http_port=%d, http_status=%d, rtsp_port=%d, rtsp_status=%d;",
             upnp->enable, upnp->type, upnp->name, upnp->extern_ip, upnp->http_port, upnp->http_status, upnp->rtsp_port,
             upnp->rtsp_status);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}


int read_email(const char *pDbFile, struct email *email)
{
    if (!pDbFile || !email) {
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
    int nColumnCnt = 0;
    char sQuery[512] = {0};
    snprintf(sQuery, sizeof(sQuery),
             "select username, password, smtp_server, port, sender_addr, sender_name, enable_tls, enable_attach, capture_interval,enable from email;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column_text(hStmt, 0, email->username, sizeof(email->username));
    sqlite_query_column_text(hStmt, 1, email->password, sizeof(email->password));
    sqlite_query_column_text(hStmt, 2, email->smtp_server, sizeof(email->smtp_server));
    sqlite_query_column(hStmt, 3, &(email->port));
    sqlite_query_column_text(hStmt, 4, email->sender_addr, sizeof(email->sender_addr));
    sqlite_query_column_text(hStmt, 5, email->sender_name, sizeof(email->sender_name));
    sqlite_query_column(hStmt, 6, &(email->enable_tls));
    sqlite_query_column(hStmt, 7, &(email->enable_attach));
    sqlite_query_column(hStmt, 8, &(email->capture_interval));
    sqlite_query_column(hStmt, 9, &(email->enable));
    sqlite_clear_stmt(hStmt);

    memset(sQuery, 0, sizeof(sQuery));
    snprintf(sQuery, sizeof(sQuery), "select address, name from email_receiver;");
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int i = 0;
    for (i = 0; i < EMAIL_RECEIVER_NUM; i++) {
        sqlite_query_column_text(hStmt, 0, email->receiver[i].address, sizeof(email->receiver[i].address));
        sqlite_query_column_text(hStmt, 1, email->receiver[i].name, sizeof(email->receiver[i].name));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_email(const char *pDbFile, struct email *email)
{
    if (!pDbFile || !email) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec),
             "update email set username='%s', password='%s',smtp_server='%s',port=%d,sender_addr='%s',sender_name='%s',enable_tls=%d,enable_attach=%d,capture_interval=%d,enable=%d;",
             \
             email->username, email->password, email->smtp_server, email->port, email->sender_addr, email->sender_name,
             email->enable_tls, email->enable_attach, email->capture_interval, email->enable);
    sqlite_execute(hConn, mode, sExec);

    memset(sExec, 0, sizeof(sExec));
    int i = 0;
    for (i = 0; i < EMAIL_RECEIVER_NUM; i++) {
        snprintf(sExec, sizeof(sExec), "update email_receiver set address='%s', name='%s' where id=%d;",
                 email->receiver[i].address, email->receiver[i].name, i);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_network(const char *pDbFile, struct network *net)
{
    if (!pDbFile || !net) {
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
    int nColumnCnt = 0;
    char sQuery[2048] = {0};
    int defaultRouteExist = check_table_key_ex(pDbFile, "network", "default_route");

    if (defaultRouteExist == 1) {
        snprintf(sQuery, sizeof(sQuery), "select mode,host_name,miimon,bond0_primary_net, bond0_enable,\
                 bond0_type, bond0_ip_address, bond0_netmask, bond0_gateway, bond0_primary_dns, bond0_second_dns,\
                 bond0_mtu,lan1_enable, lan1_type, lan1_ip_address, lan1_netmask, lan1_gateway, lan1_primary_dns,\
                 lan1_second_dns, lan1_mtu, lan2_enable, lan2_type, lan2_ip_address, lan2_netmask, lan2_gateway,\
                 lan2_primary_dns, lan2_second_dns, lan2_mtu, lan1_dhcp_gateway, lan2_dhcp_gateway, lan1_ip6_address,\
                 lan1_ip6_netmask, lan1_ip6_gateway, lan2_ip6_address, lan2_ip6_netmask, lan2_ip6_gateway, lan1_ip6_dhcp,\
                 lan2_ip6_dhcp, bond0_ip6_dhcp, bond0_ip6_address, bond0_ip6_netmask, bond0_ip6_gateway, default_route from network;");
    } else {
        snprintf(sQuery, sizeof(sQuery), "select mode,host_name,miimon,bond0_primary_net, bond0_enable,\
                 bond0_type, bond0_ip_address, bond0_netmask, bond0_gateway, bond0_primary_dns, bond0_second_dns,\
                 bond0_mtu,lan1_enable, lan1_type, lan1_ip_address, lan1_netmask, lan1_gateway, lan1_primary_dns,\
                 lan1_second_dns, lan1_mtu, lan2_enable, lan2_type, lan2_ip_address, lan2_netmask, lan2_gateway,\
                 lan2_primary_dns, lan2_second_dns, lan2_mtu, lan1_dhcp_gateway, lan2_dhcp_gateway, lan1_ip6_address,\
                 lan1_ip6_netmask, lan1_ip6_gateway, lan2_ip6_address, lan2_ip6_netmask, lan2_ip6_gateway, lan1_ip6_dhcp,\
                 lan2_ip6_dhcp, bond0_ip6_dhcp, bond0_ip6_address, bond0_ip6_netmask, bond0_ip6_gateway from network;");
    }

    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &(net->mode));
    sqlite_query_column_text(hStmt, 1, net->host_name, sizeof(net->host_name));
    sqlite_query_column(hStmt, 2, &(net->miimon));
    sqlite_query_column(hStmt, 3, &(net->bond0_primary_net));
    sqlite_query_column(hStmt, 4, &(net->bond0_enable));
    sqlite_query_column(hStmt, 5, &(net->bond0_type));
    sqlite_query_column_text(hStmt, 6, net->bond0_ip_address, sizeof(net->bond0_ip_address));
    sqlite_query_column_text(hStmt, 7, net->bond0_netmask, sizeof(net->bond0_netmask));
    sqlite_query_column_text(hStmt, 8, net->bond0_gateway, sizeof(net->bond0_gateway));
    sqlite_query_column_text(hStmt, 9, net->bond0_primary_dns, sizeof(net->bond0_primary_dns));
    sqlite_query_column_text(hStmt, 10, net->bond0_second_dns, sizeof(net->bond0_second_dns));
    sqlite_query_column(hStmt, 11, &(net->bond0_mtu));
    sqlite_query_column(hStmt, 12, &(net->lan1_enable));
    sqlite_query_column(hStmt, 13, &(net->lan1_type));
    sqlite_query_column_text(hStmt, 14, net->lan1_ip_address, sizeof(net->lan1_ip_address));
    sqlite_query_column_text(hStmt, 15, net->lan1_netmask, sizeof(net->lan1_netmask));
    sqlite_query_column_text(hStmt, 16, net->lan1_gateway, sizeof(net->lan1_gateway));
    sqlite_query_column_text(hStmt, 17, net->lan1_primary_dns, sizeof(net->lan1_primary_dns));
    sqlite_query_column_text(hStmt, 18, net->lan1_second_dns, sizeof(net->lan1_second_dns));
    sqlite_query_column(hStmt, 19, &(net->lan1_mtu));
    sqlite_query_column(hStmt, 20, &(net->lan2_enable));
    sqlite_query_column(hStmt, 21, &(net->lan2_type));
    sqlite_query_column_text(hStmt, 22, net->lan2_ip_address, sizeof(net->lan2_ip_address));
    sqlite_query_column_text(hStmt, 23, net->lan2_netmask, sizeof(net->lan2_netmask));
    sqlite_query_column_text(hStmt, 24, net->lan2_gateway, sizeof(net->lan2_gateway));
    sqlite_query_column_text(hStmt, 25, net->lan2_primary_dns, sizeof(net->lan2_primary_dns));
    sqlite_query_column_text(hStmt, 26, net->lan2_second_dns, sizeof(net->lan2_second_dns));
    sqlite_query_column(hStmt, 27, &(net->lan2_mtu));
    sqlite_query_column_text(hStmt, 28, net->lan1_dhcp_gateway, sizeof(net->lan1_dhcp_gateway));
    sqlite_query_column_text(hStmt, 29, net->lan2_dhcp_gateway, sizeof(net->lan2_dhcp_gateway));
    sqlite_query_column_text(hStmt, 30, net->lan1_ip6_address, sizeof(net->lan1_ip6_address));
    sqlite_query_column_text(hStmt, 31, net->lan1_ip6_netmask, sizeof(net->lan1_ip6_netmask));
    sqlite_query_column_text(hStmt, 32, net->lan1_ip6_gateway, sizeof(net->lan1_ip6_gateway));
    sqlite_query_column_text(hStmt, 33, net->lan2_ip6_address, sizeof(net->lan2_ip6_address));
    sqlite_query_column_text(hStmt, 34, net->lan2_ip6_netmask, sizeof(net->lan2_ip6_netmask));
    sqlite_query_column_text(hStmt, 35, net->lan2_ip6_gateway, sizeof(net->lan2_ip6_gateway));
    sqlite_query_column(hStmt, 36, &(net->lan1_ip6_dhcp));
    sqlite_query_column(hStmt, 37, &(net->lan2_ip6_dhcp));
    sqlite_query_column(hStmt, 38, &(net->bond0_ip6_dhcp));
    sqlite_query_column_text(hStmt, 39, net->bond0_ip6_address, sizeof(net->bond0_ip6_address));
    sqlite_query_column_text(hStmt, 40, net->bond0_ip6_netmask, sizeof(net->bond0_ip6_netmask));
    sqlite_query_column_text(hStmt, 41, net->bond0_ip6_gateway, sizeof(net->bond0_ip6_gateway));
    if (defaultRouteExist) {
        sqlite_query_column(hStmt, 42, &(net->defaultRoute));
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

static int remove_leading_zeros(const char *srcIp, char *dstIp, int dstIpSize) 
{
    if (!srcIp || srcIp[0] == '\0' || !dstIp || dstIpSize < 16) {
        return -1;
    }

    char tmp[16] = {0};
    int octet;
    char *saveptr = NULL;
    char *segment = NULL;

    memset(dstIp, 0, dstIpSize);
    snprintf(tmp, sizeof(tmp), "%s", srcIp);
    segment= strtok_r(tmp, ".", &saveptr);
    while (segment) {
        octet = atoi(segment);
        snprintf(dstIp + strlen(dstIp), dstIpSize - strlen(dstIp), "%d", octet);
        segment = strtok_r(NULL, ".", &saveptr);
        if(segment) {
            strcat(dstIp, ".");
        }
    }

    return 0;
}

int write_network(const char *pDbFile, struct network *net)
{
    if (!pDbFile || !net) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    char tmpIp[NET_DEV_MAX][16] = {{0}};
    char tmpNetmask[NET_DEV_MAX][16] = {{0}};
    char tmpGateway[NET_DEV_MAX][16] = {{0}};
    char tmpPrimaryDns[NET_DEV_MAX][16] = {{0}};
    char tmpSecondDns[NET_DEV_MAX][16] = {{0}};

    remove_leading_zeros(net->bond0_ip_address, tmpIp[NET_DEV_BOND], sizeof(tmpIp[NET_DEV_BOND]));
    remove_leading_zeros(net->bond0_netmask, tmpNetmask[NET_DEV_BOND], sizeof(tmpNetmask[NET_DEV_BOND]));
    remove_leading_zeros(net->bond0_gateway, tmpGateway[NET_DEV_BOND], sizeof(tmpGateway[NET_DEV_BOND]));
    remove_leading_zeros(net->bond0_primary_dns, tmpPrimaryDns[NET_DEV_BOND], sizeof(tmpPrimaryDns[NET_DEV_BOND]));
    remove_leading_zeros(net->bond0_second_dns, tmpSecondDns[NET_DEV_BOND], sizeof(tmpSecondDns[NET_DEV_BOND]));
    remove_leading_zeros(net->lan1_ip_address, tmpIp[NET_DEV_ETH0], sizeof(tmpIp[NET_DEV_ETH0]));
    remove_leading_zeros(net->lan1_netmask, tmpNetmask[NET_DEV_ETH0], sizeof(tmpNetmask[NET_DEV_ETH0]));
    remove_leading_zeros(net->lan1_gateway, tmpGateway[NET_DEV_ETH0], sizeof(tmpGateway[NET_DEV_ETH0]));
    remove_leading_zeros(net->lan1_primary_dns, tmpPrimaryDns[NET_DEV_ETH0], sizeof(tmpPrimaryDns[NET_DEV_ETH0]));
    remove_leading_zeros(net->lan1_second_dns, tmpSecondDns[NET_DEV_ETH0], sizeof(tmpSecondDns[NET_DEV_ETH0]));
    remove_leading_zeros(net->lan2_ip_address, tmpIp[NET_DEV_ETH1], sizeof(tmpIp[NET_DEV_ETH1]));
    remove_leading_zeros(net->lan2_netmask, tmpNetmask[NET_DEV_ETH1], sizeof(tmpNetmask[NET_DEV_ETH1]));
    remove_leading_zeros(net->lan2_gateway, tmpGateway[NET_DEV_ETH1], sizeof(tmpGateway[NET_DEV_ETH1]));
    remove_leading_zeros(net->lan2_primary_dns, tmpPrimaryDns[NET_DEV_ETH1], sizeof(tmpPrimaryDns[NET_DEV_ETH1]));
    remove_leading_zeros(net->lan2_second_dns, tmpSecondDns[NET_DEV_ETH1], sizeof(tmpSecondDns[NET_DEV_ETH1]));

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[2048] = {0};
    snprintf(sExec, sizeof(sExec), "update network set mode=%d, host_name='%s', bond0_primary_net=%d,\
        bond0_enable=%d, bond0_type=%d, bond0_ip_address='%s', bond0_netmask='%s', bond0_gateway='%s',\
        bond0_primary_dns='%s', bond0_second_dns='%s', bond0_mtu=%d, lan1_enable=%d, lan1_type=%d,\
        lan1_ip_address='%s', lan1_netmask='%s', lan1_gateway='%s',lan1_primary_dns='%s',\
        lan1_second_dns='%s', lan1_mtu=%d, lan2_enable=%d, lan2_type=%d, lan2_ip_address='%s',\
        lan2_netmask='%s', lan2_gateway='%s',lan2_primary_dns='%s', lan2_second_dns='%s',\
        lan2_mtu=%d, lan1_dhcp_gateway='%s', lan2_dhcp_gateway='%s', lan1_ip6_address='%s',\
        lan1_ip6_netmask='%s', lan1_ip6_gateway='%s', lan2_ip6_address='%s', lan2_ip6_netmask='%s',\
        lan2_ip6_gateway='%s',lan1_ip6_dhcp=%d,lan2_ip6_dhcp=%d,bond0_ip6_dhcp=%d,bond0_ip6_address='%s',\
        bond0_ip6_netmask='%s',bond0_ip6_gateway='%s', default_route=%d;", 
        net->mode, net->host_name, net->bond0_primary_net, net->bond0_enable, net->bond0_type, tmpIp[NET_DEV_BOND],
        tmpNetmask[NET_DEV_BOND], tmpGateway[NET_DEV_BOND], tmpPrimaryDns[NET_DEV_BOND], tmpSecondDns[NET_DEV_BOND],
        net->bond0_mtu, net->lan1_enable, net->lan1_type, tmpIp[NET_DEV_ETH0], tmpNetmask[NET_DEV_ETH0],
        tmpGateway[NET_DEV_ETH0], tmpPrimaryDns[NET_DEV_ETH0], tmpSecondDns[NET_DEV_ETH0], net->lan1_mtu,
        net->lan2_enable, net->lan2_type, tmpIp[NET_DEV_ETH1], tmpNetmask[NET_DEV_ETH1], tmpGateway[NET_DEV_ETH1],
        tmpPrimaryDns[NET_DEV_ETH1], tmpSecondDns[NET_DEV_ETH1], net->lan2_mtu, net->lan1_dhcp_gateway,
        net->lan2_dhcp_gateway, net->lan1_ip6_address, net->lan1_ip6_netmask, net->lan1_ip6_gateway,
        net->lan2_ip6_address, net->lan2_ip6_netmask, net->lan2_ip6_gateway, net->lan1_ip6_dhcp, net->lan2_ip6_dhcp,
        net->bond0_ip6_dhcp, net->bond0_ip6_address, net->bond0_ip6_netmask, net->bond0_ip6_gateway, net->defaultRoute);
    if (sqlite_execute(hConn, mode, sExec) != 0) {
        snprintf(sExec, sizeof(sExec),
            "update network set mode=%d, host_name='%s', bond0_primary_net=%d, bond0_enable=%d, bond0_type=%d, \
            bond0_ip_address='%s', bond0_netmask='%s', bond0_gateway='%s', bond0_primary_dns='%s', \
            bond0_second_dns='%s', bond0_mtu=%d, lan1_enable=%d, lan1_type=%d, lan1_ip_address='%s', \
            lan1_netmask='%s', lan1_gateway='%s',lan1_primary_dns='%s', lan1_second_dns='%s', lan1_mtu=%d, \
            lan2_enable=%d, lan2_type=%d, lan2_ip_address='%s', lan2_netmask='%s', lan2_gateway='%s',\
            lan2_primary_dns='%s', lan2_second_dns='%s', lan2_mtu=%d, lan1_dhcp_gateway='%s', lan2_dhcp_gateway='%s';",
            net->mode, net->host_name, net->bond0_primary_net, net->bond0_enable, net->bond0_type,
            tmpIp[NET_DEV_BOND], tmpNetmask[NET_DEV_BOND], tmpGateway[NET_DEV_BOND], tmpPrimaryDns[NET_DEV_BOND],
            tmpSecondDns[NET_DEV_BOND], net->bond0_mtu, net->lan1_enable, net->lan1_type, tmpIp[NET_DEV_ETH0],
            tmpNetmask[NET_DEV_ETH0], tmpGateway[NET_DEV_ETH0], tmpPrimaryDns[NET_DEV_ETH0],
            tmpSecondDns[NET_DEV_ETH0], net->lan1_mtu, net->lan2_enable, net->lan2_type, tmpIp[NET_DEV_ETH1],
            tmpNetmask[NET_DEV_ETH1], tmpGateway[NET_DEV_ETH1], tmpPrimaryDns[NET_DEV_ETH1],
            tmpSecondDns[NET_DEV_ETH1], net->lan2_mtu, net->lan1_dhcp_gateway, net->lan2_dhcp_gateway);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_network_mode(const char *pDbFile, int *pMode)
{
    if (!pDbFile || !pMode) {
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
    int nColumnCnt = 0;
    char sQuery[512] = {0};
    snprintf(sQuery, sizeof(sQuery), "select mode from network;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, pMode);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_network_bond(const char *pDbFile, struct network *net)
{
    if (!pDbFile || !net) {
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
    int nColumnCnt = 0;
    char sQuery[1024] = {0};
    snprintf(sQuery, sizeof(sQuery), "select mode, host_name, bond0_primary_net, bond0_enable, bond0_type,\
            bond0_ip_address, bond0_netmask, bond0_gateway, bond0_primary_dns, bond0_second_dns, bond0_mtu,\
            bond0_ip6_dhcp, bond0_ip6_address, bond0_ip6_netmask, bond0_ip6_gateway, default_route from network;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &(net->mode));
    sqlite_query_column_text(hStmt, 1, net->host_name, sizeof(net->host_name));
    sqlite_query_column(hStmt, 2, &(net->bond0_primary_net));
    sqlite_query_column(hStmt, 3, &(net->bond0_enable));
    sqlite_query_column(hStmt, 4, &(net->bond0_type));
    sqlite_query_column_text(hStmt, 5, net->bond0_ip_address, sizeof(net->bond0_ip_address));
    sqlite_query_column_text(hStmt, 6, net->bond0_netmask, sizeof(net->bond0_netmask));
    sqlite_query_column_text(hStmt, 7, net->bond0_gateway, sizeof(net->bond0_gateway));
    sqlite_query_column_text(hStmt, 8, net->bond0_primary_dns, sizeof(net->bond0_primary_dns));
    sqlite_query_column_text(hStmt, 9, net->bond0_second_dns, sizeof(net->bond0_second_dns));
    sqlite_query_column(hStmt, 10, &(net->bond0_mtu));

    sqlite_query_column(hStmt, 11, &(net->bond0_ip6_dhcp));
    sqlite_query_column_text(hStmt, 12, net->bond0_ip6_address, sizeof(net->bond0_ip6_address));
    sqlite_query_column_text(hStmt, 13, net->bond0_ip6_netmask, sizeof(net->bond0_ip6_netmask));
    sqlite_query_column_text(hStmt, 14, net->bond0_ip6_gateway, sizeof(net->bond0_ip6_gateway));
    sqlite_query_column(hStmt, 15, &(net->defaultRoute));


    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_network_muti(const char *pDbFile, struct network *net)
{
    if (!pDbFile || !net) {
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
    int nColumnCnt = 0;
    char sQuery[1024] = {0};
    snprintf(sQuery, sizeof(sQuery), "select mode, host_name, lan1_enable, lan1_type, lan1_ip_address, lan1_netmask,\
            lan1_gateway, lan1_primary_dns, lan1_second_dns, lan1_mtu, lan2_enable, lan2_type, lan2_ip_address,\
            lan2_netmask, lan2_gateway, lan2_primary_dns, lan2_second_dns, lan2_mtu, lan1_ip6_address,\
            lan1_ip6_netmask, lan1_ip6_gateway, lan2_ip6_address, lan2_ip6_netmask, lan2_ip6_gateway,\
            lan1_ip6_dhcp, lan2_ip6_dhcp, default_route from network;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &(net->mode));
    sqlite_query_column_text(hStmt, 1, net->host_name, sizeof(net->host_name));
    sqlite_query_column(hStmt, 2, &(net->lan1_enable));
    sqlite_query_column(hStmt, 3, &(net->lan1_type));
    sqlite_query_column_text(hStmt, 4, net->lan1_ip_address, sizeof(net->lan1_ip_address));
    sqlite_query_column_text(hStmt, 5, net->lan1_netmask, sizeof(net->lan1_netmask));
    sqlite_query_column_text(hStmt, 6, net->lan1_gateway, sizeof(net->lan1_gateway));
    sqlite_query_column_text(hStmt, 7, net->lan1_primary_dns, sizeof(net->lan1_primary_dns));
    sqlite_query_column_text(hStmt, 8, net->lan1_second_dns, sizeof(net->lan1_second_dns));
    sqlite_query_column(hStmt, 9, &(net->lan1_mtu));

    sqlite_query_column(hStmt, 10, &(net->lan2_enable));
    sqlite_query_column(hStmt, 11, &(net->lan2_type));
    sqlite_query_column_text(hStmt, 12, net->lan2_ip_address, sizeof(net->lan2_ip_address));
    sqlite_query_column_text(hStmt, 13, net->lan2_netmask, sizeof(net->lan2_netmask));
    sqlite_query_column_text(hStmt, 14, net->lan2_gateway, sizeof(net->lan2_gateway));
    sqlite_query_column_text(hStmt, 15, net->lan2_primary_dns, sizeof(net->lan2_primary_dns));
    sqlite_query_column_text(hStmt, 16, net->lan2_second_dns, sizeof(net->lan2_second_dns));
    sqlite_query_column(hStmt, 17, &(net->lan2_mtu));

    sqlite_query_column_text(hStmt, 18, net->lan1_ip6_address, sizeof(net->lan1_ip6_address));
    sqlite_query_column_text(hStmt, 19, net->lan1_ip6_netmask, sizeof(net->lan1_ip6_netmask));
    sqlite_query_column_text(hStmt, 20, net->lan1_ip6_gateway, sizeof(net->lan1_ip6_gateway));
    sqlite_query_column_text(hStmt, 21, net->lan2_ip6_address, sizeof(net->lan2_ip6_address));
    sqlite_query_column_text(hStmt, 22, net->lan2_ip6_netmask, sizeof(net->lan2_ip6_netmask));
    sqlite_query_column_text(hStmt, 23, net->lan2_ip6_gateway, sizeof(net->lan2_ip6_gateway));
    sqlite_query_column(hStmt, 24, &(net->lan1_ip6_dhcp));
    sqlite_query_column(hStmt, 25, &(net->lan2_ip6_dhcp));
    sqlite_query_column(hStmt, 26, &(net->defaultRoute));


    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_network_bond(const char *pDbFile, struct network *net)
{
    if (!pDbFile || !net) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    char tmpIp[16] = {0};
    char tmpNetmask[16] = {0};
    char tmpGateway[16] = {0};
    char tmpPrimaryDns[16] = {0};
    char tmpSecondDns[16] = {0};

    remove_leading_zeros(net->bond0_ip_address, tmpIp, sizeof(tmpIp));
    remove_leading_zeros(net->bond0_netmask, tmpNetmask, sizeof(tmpNetmask));
    remove_leading_zeros(net->bond0_gateway, tmpGateway, sizeof(tmpGateway));
    remove_leading_zeros(net->bond0_primary_dns, tmpPrimaryDns, sizeof(tmpPrimaryDns));
    remove_leading_zeros(net->bond0_second_dns, tmpSecondDns, sizeof(tmpSecondDns));

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[1024] = {0};
    snprintf(sExec, sizeof(sExec), "update network set mode=%d,host_name='%s', bond0_primary_net=%d, bond0_enable=%d,\
        bond0_type=%d, bond0_ip_address='%s', bond0_netmask='%s', bond0_gateway='%s', bond0_primary_dns='%s',\
        bond0_second_dns='%s', bond0_mtu=%d, bond0_ip6_dhcp=%d, bond0_ip6_address='%s', bond0_ip6_netmask='%s',\
        bond0_ip6_gateway='%s', default_route=%d;", 
        net->mode, net->host_name, net->bond0_primary_net, net->bond0_enable, net->bond0_type, tmpIp, tmpNetmask,
        tmpGateway, tmpPrimaryDns, tmpSecondDns, net->bond0_mtu, net->bond0_ip6_dhcp, net->bond0_ip6_address, 
        net->bond0_ip6_netmask, net->bond0_ip6_gateway, net->defaultRoute);

    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_network_muti(const char *pDbFile, struct network *net)
{
    if (!pDbFile || !net) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    char tmpIp[NET_DEV_MAX][16] = {{0}};
    char tmpNetmask[NET_DEV_MAX][16] = {{0}};
    char tmpGateway[NET_DEV_MAX][16] = {{0}};
    char tmpPrimaryDns[NET_DEV_MAX][16] = {{0}};
    char tmpSecondDns[NET_DEV_MAX][16] = {{0}};

    remove_leading_zeros(net->lan1_ip_address, tmpIp[NET_DEV_ETH0], sizeof(tmpIp[NET_DEV_ETH0]));
    remove_leading_zeros(net->lan1_netmask, tmpNetmask[NET_DEV_ETH0], sizeof(tmpNetmask[NET_DEV_ETH0]));
    remove_leading_zeros(net->lan1_gateway, tmpGateway[NET_DEV_ETH0], sizeof(tmpGateway[NET_DEV_ETH0]));
    remove_leading_zeros(net->lan1_primary_dns, tmpPrimaryDns[NET_DEV_ETH0], sizeof(tmpPrimaryDns[NET_DEV_ETH0]));
    remove_leading_zeros(net->lan1_second_dns, tmpSecondDns[NET_DEV_ETH0], sizeof(tmpSecondDns[NET_DEV_ETH0]));
    remove_leading_zeros(net->lan2_ip_address, tmpIp[NET_DEV_ETH1], sizeof(tmpIp[NET_DEV_ETH1]));
    remove_leading_zeros(net->lan2_netmask, tmpNetmask[NET_DEV_ETH1], sizeof(tmpNetmask[NET_DEV_ETH1]));
    remove_leading_zeros(net->lan2_gateway, tmpGateway[NET_DEV_ETH1], sizeof(tmpGateway[NET_DEV_ETH1]));
    remove_leading_zeros(net->lan2_primary_dns, tmpPrimaryDns[NET_DEV_ETH1], sizeof(tmpPrimaryDns[NET_DEV_ETH1]));
    remove_leading_zeros(net->lan2_second_dns, tmpSecondDns[NET_DEV_ETH1], sizeof(tmpSecondDns[NET_DEV_ETH1]));

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[1024] = {0};
    snprintf(sExec, sizeof(sExec), "update network set mode=%d, host_name='%s', lan1_enable=%d, lan1_type=%d,\
        lan1_ip_address='%s', lan1_netmask='%s', lan1_gateway='%s',lan1_primary_dns='%s', lan1_second_dns='%s',\
        lan1_mtu=%d, lan2_enable=%d, lan2_type=%d, lan2_ip_address='%s', lan2_netmask='%s', lan2_gateway='%s',\
        lan2_primary_dns='%s', lan2_second_dns='%s', lan2_mtu=%d, lan1_ip6_address='%s', lan1_ip6_netmask='%s',\
        lan1_ip6_gateway='%s', lan2_ip6_address='%s', lan2_ip6_netmask='%s', lan2_ip6_gateway='%s',\
        lan1_ip6_dhcp=%d, lan2_ip6_dhcp=%d, default_route=%d;", net->mode, net->host_name, net->lan1_enable,
        net->lan1_type, tmpIp[NET_DEV_ETH0], tmpNetmask[NET_DEV_ETH0], tmpGateway[NET_DEV_ETH0], 
        tmpPrimaryDns[NET_DEV_ETH0], tmpSecondDns[NET_DEV_ETH0], net->lan1_mtu, net->lan2_enable, net->lan2_type, 
        tmpIp[NET_DEV_ETH1], tmpNetmask[NET_DEV_ETH1], tmpGateway[NET_DEV_ETH1], tmpPrimaryDns[NET_DEV_ETH1], 
        tmpSecondDns[NET_DEV_ETH1], net->lan2_mtu, net->lan1_ip6_address, net->lan1_ip6_netmask, net->lan1_ip6_gateway, 
        net->lan2_ip6_address, net->lan2_ip6_netmask, net->lan2_ip6_gateway, net->lan1_ip6_dhcp, net->lan2_ip6_dhcp, 
        net->defaultRoute);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_port(const char *pDbFile, struct network_more *more)
{
    if (!pDbFile || !more) {
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
    int nColumnCnt = 0;
    char sQuery[512] = {0};
    snprintf(sQuery, sizeof(sQuery),
             "select enable_ssh, ssh_port, http_port, rtsp_port, sdk_port, url, url_enable, https_port,pos_port from network_more;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &(more->enable_ssh));
    sqlite_query_column(hStmt, 1, &(more->ssh_port));
    sqlite_query_column(hStmt, 2, &(more->http_port));
    sqlite_query_column(hStmt, 3, &(more->rtsp_port));
    sqlite_query_column(hStmt, 4, &(more->sdk_port));
    sqlite_query_column_text(hStmt, 5, more->url, sizeof(more->url));
    sqlite_query_column(hStmt, 6, &(more->url_enable));
    sqlite_query_column(hStmt, 7, &(more->https_port));
    sqlite_query_column(hStmt, 8, &(more->posPort));
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_port(const char *pDbFile, struct network_more *more)
{
    if (!pDbFile || !more) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    char url[256] = {0};
    translate_pwd(url, more->url, strlen(more->url));
    snprintf(sExec, sizeof(sExec),
             "update network_more set enable_ssh=%d, ssh_port=%d, http_port=%d, rtsp_port=%d, sdk_port=%d, url='%s', url_enable=%d, https_port=%d, pos_port=%d;",
             more->enable_ssh, more->ssh_port, more->http_port, more->rtsp_port, more->sdk_port, url, more->url_enable,
             more->https_port, more->posPort);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_snmp(const char *pDbFile, struct snmp *snmp)
{
    if (!pDbFile || !snmp) {
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
    int nColumnCnt = 0;
    char sQuery[2048] = {0};

    snprintf(sQuery, sizeof(sQuery),
             "select v1_enable, v2c_enable, write_community, read_community, v3_enable, read_name, read_level, read_auth_type, read_auth_psw, read_pri_type, read_pri_psw, write_name, write_level, write_auth_type, write_auth_psw, write_pri_type, write_pri_psw, port from snmp;");//hrz.milesight
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, &(snmp->v1_enable));
    sqlite_query_column(hStmt, 1, &(snmp->v2c_enable));
    sqlite_query_column_text(hStmt, 2, snmp->write_community, sizeof(snmp->write_community));
    sqlite_query_column_text(hStmt, 3, snmp->read_community, sizeof(snmp->read_community));

    sqlite_query_column(hStmt, 4, &(snmp->v3_enable));
    sqlite_query_column_text(hStmt, 5, snmp->read_security_name, sizeof(snmp->read_security_name));
    sqlite_query_column(hStmt, 6, &(snmp->read_level_security));
    sqlite_query_column(hStmt, 7, &(snmp->read_auth_algorithm));
    sqlite_query_column_text(hStmt, 8, snmp->read_auth_password, sizeof(snmp->read_auth_password));
    sqlite_query_column(hStmt, 9, &(snmp->read_pri_algorithm));
    sqlite_query_column_text(hStmt, 10, snmp->read_pri_password, sizeof(snmp->read_pri_password));

    sqlite_query_column_text(hStmt, 11, snmp->write_security_name, sizeof(snmp->write_security_name));
    sqlite_query_column(hStmt, 12, &(snmp->write_level_security));
    sqlite_query_column(hStmt, 13, &(snmp->write_auth_algorithm));
    sqlite_query_column_text(hStmt, 14, snmp->write_auth_password, sizeof(snmp->write_auth_password));
    sqlite_query_column(hStmt, 15, &(snmp->write_pri_algorithm));
    sqlite_query_column_text(hStmt, 16, snmp->write_pri_password, sizeof(snmp->write_pri_password));

    sqlite_query_column(hStmt, 17, &(snmp->port));

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_snmp(const char *pDbFile, struct snmp *snmp)
{
    if (!pDbFile || !snmp) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;

    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[2048] = {0};
    snprintf(sExec, sizeof(sExec),
             "update snmp set v1_enable=%d, v2c_enable=%d, write_community='%s', read_community='%s', v3_enable=%d, read_name='%s', read_level=%d, read_auth_type=%d, read_auth_psw='%s', read_pri_type=%d, read_pri_psw='%s', write_name='%s', write_level=%d, write_auth_type=%d, write_auth_psw='%s', write_pri_type=%d, write_pri_psw='%s', port=%d;",
             snmp->v1_enable, snmp->v2c_enable, snmp->write_community, snmp->read_community, snmp->v3_enable,
             snmp->read_security_name, snmp->read_level_security, snmp->read_auth_algorithm, snmp->read_auth_password,
             snmp->read_pri_algorithm, snmp->read_pri_password, snmp->write_security_name, snmp->write_level_security,
             snmp->write_auth_algorithm, snmp->write_auth_password, snmp->write_pri_algorithm, snmp->write_pri_password,
             snmp->port);//hrz.milesight
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}


int read_audio_ins(const char *pDbFile, struct audio_in audioins[MAX_AUDIOIN], int *pCnt)
{
    if (!pDbFile || !audioins) {
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
    int nColumnCnt = 0;
    char sQuery[512] = {0};
    snprintf(sQuery, sizeof(sQuery), "select id, enable, samplerate, volume from audio_in;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0) {
        *pCnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int i = 0;
    for (i = 0; i < MAX_AUDIOIN; i++) {
        sqlite_query_column(hStmt, 0, &audioins[i].id);
        sqlite_query_column(hStmt, 1, &audioins[i].enable);
        sqlite_query_column(hStmt, 2, &audioins[i].samplerate);
        sqlite_query_column(hStmt, 3, &audioins[i].volume);
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    *pCnt = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_audio_in(const char *pDbFile, struct audio_in *audioin)
{
    if (!pDbFile || !audioin) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "update audio_in set enable=%d, samplerate=%d,volume=%d where id=%d;", audioin->enable,
             audioin->samplerate, audioin->volume, audioin->id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_layout(const char *pDbFile, struct layout *layout)
{
    if (!pDbFile || !layout) {
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
    int nColumnCnt = 0;
    char sQuery[512] = {0};
    snprintf(sQuery, sizeof(sQuery), "select main_output, sub_output, main_layout_mode, sub_layout_mode from layout;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &(layout->main_output));
    sqlite_query_column(hStmt, 1, &(layout->sub_output));
    sqlite_query_column(hStmt, 2, &(layout->main_layout_mode));
    sqlite_query_column(hStmt, 3, &(layout->sub_layout_mode));

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_layout(const char *pDbFile, struct layout *layout)
{
    if (!pDbFile || !layout) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec),
             "update layout set main_output=%d, sub_output=%d, main_layout_mode=%d, sub_layout_mode=%d;", layout->main_output,
             layout->sub_output, layout->main_layout_mode, layout->sub_layout_mode);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_layout_custom(const char *pDbFile, struct layout_custom *layout, int *count)
{
    if (!pDbFile || !layout) {
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
    int nColumnCnt = 0;
    char sQuery[512] = {0};
    snprintf(sQuery, sizeof(sQuery), "select screen, name, type, page from layout_custom;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0) {
        *count = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int i = 0;
    for (i = 0; i < 2; ++i) {
        sqlite_query_column(hStmt, 0, &layout[i].screen);
        sqlite_query_column_text(hStmt, 1, layout[i].name, sizeof(layout[i].name));
        sqlite_query_column(hStmt, 2, &layout[i].type);
        sqlite_query_column(hStmt, 3, &layout[i].page);
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    *count = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_layout_custom(const char *pDbFile, struct layout_custom *layout)
{
    if (!pDbFile || !layout) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec), "insert or replace into layout_custom values(%d, '%s', %d, %d);", layout->screen,
             layout->name, layout->type, layout->page);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_layout_custom_64Screen(const char *pDbFile, struct layout_custom *layout, int *count)
{
    if (!pDbFile || !layout) {
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
    int nColumnCnt = 0;
    char sQuery[512] = {0};
    snprintf(sQuery, sizeof(sQuery), "select screen, name, type, page from layout_custom_64Screen;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0) {
        *count = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int i = 0;
    for (i = 0; i < 2; ++i) {
        sqlite_query_column(hStmt, 0, &layout[i].screen);
        sqlite_query_column_text(hStmt, 1, layout[i].name, sizeof(layout[i].name));
        sqlite_query_column(hStmt, 2, &layout[i].type);
        sqlite_query_column(hStmt, 3, &layout[i].page);
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
#if defined(_HI3536G_)
    if (strcmp(layout[1].name, "LAYOUTMODE_64") == 0) {
        snprintf(layout[1].name, sizeof(layout[1].name),"LAYOUTMODE_32");
    }
#endif
    *count = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_layout_custom_64Screen(const char *pDbFile, struct layout_custom *layout)
{
    if (!pDbFile || !layout) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec), "insert or replace into layout_custom_64Screen values(%d, '%s', %d, %d);", layout->screen,
             layout->name, layout->type, layout->page);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_ptz_speeds(const char *pDbFile, struct ptz_speed ptzspeeds[MAX_CAMERA], int *pCnt)
{
    if (!pDbFile || !ptzspeeds) {
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
    int nColumnCnt = 0;
    char sQuery[512] = {0};
    snprintf(sQuery, sizeof(sQuery), "select id,pan,tilt,zoom,focus,timeout from ptz_speed;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0) {
        *pCnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int i = 0;
    for (i = 0; i < MAX_CAMERA; i++) {
        sqlite_query_column(hStmt, 0, &ptzspeeds[i].id);
        sqlite_query_column(hStmt, 1, &ptzspeeds[i].pan);
        sqlite_query_column(hStmt, 2, &ptzspeeds[i].tilt);
        sqlite_query_column(hStmt, 3, &ptzspeeds[i].zoom);
        sqlite_query_column(hStmt, 4, &ptzspeeds[i].focus);
        sqlite_query_column(hStmt, 5, &ptzspeeds[i].timeout);
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    *pCnt = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_ptz_speed(const char *pDbFile, struct ptz_speed *ptzspeed, int chn_id)
{
    if (!pDbFile || !ptzspeed || chn_id < 0) {
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
    int nColumnCnt = 0;
    char sQuery[512] = {0};
    snprintf(sQuery, sizeof(sQuery), "select id,pan,tilt,zoom,focus,timeout from ptz_speed where id=%d;", chn_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &ptzspeed->id);
    sqlite_query_column(hStmt, 1, &ptzspeed->pan);
    sqlite_query_column(hStmt, 2, &ptzspeed->tilt);
    sqlite_query_column(hStmt, 3, &ptzspeed->zoom);
    sqlite_query_column(hStmt, 4, &ptzspeed->focus);
    sqlite_query_column(hStmt, 5, &ptzspeed->timeout);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_ptz_speed(const char *pDbFile, struct ptz_speed *ptzspeed)
{
    if (!pDbFile || !ptzspeed) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "update ptz_speed set pan=%d,tilt=%d,zoom=%d,focus=%d,timeout=%d where id=%d;",
             ptzspeed->pan, ptzspeed->tilt, ptzspeed->zoom, ptzspeed->focus, ptzspeed->timeout, ptzspeed->id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_storages(const char *pDbFile, struct storage storages[SATA_MAX], int *pCnt)
{
    if (!pDbFile || !storages) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery), "select id, recycle_mode from storage;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *pCnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    for (i = 0; i < SATA_MAX; i++) {
        sqlite_query_column(hStmt, 0, &(storages[i].id));
        sqlite_query_column(hStmt, 1, &(storages[i].recycle_mode));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    *pCnt = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_storage(const char *pDbFile, struct storage *storage, int port_id)
{
    if (!pDbFile || !storage || port_id < 0) {
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
    snprintf(sQuery, sizeof(sQuery), "select id, recycle_mode from storage where id=%d;", port_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &(storage->id));
    sqlite_query_column(hStmt, 1, &(storage->recycle_mode));

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_storage(const char *pDbFile, struct storage *storage)
{
    if (!pDbFile || !storage) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "update storage set recycle_mode=%d where id=%d;", storage->recycle_mode, storage->id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}


int read_ptz_tour(const char *pDbFile, struct ptz_tour tour[TOUR_MAX], int ptz_id)
{
    if (!pDbFile || !tour || ptz_id < 0) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery), "select tour_id, key_id, preset_id, timeout, speed from ptz_tour where ptz_id=%d;",
             ptz_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int tour_id = 0;
    int key_id = 0;
    int loop_cnt = TOUR_MAX * MS_KEY_MAX;
    for (i = 0; i <= loop_cnt; i++) {
        sqlite_query_column(hStmt, 0, &tour_id);
        sqlite_query_column(hStmt, 1, &key_id);
        sqlite_query_column(hStmt, 2, &(tour[tour_id].keys[key_id].preset_id));
        sqlite_query_column(hStmt, 3, &(tour[tour_id].keys[key_id].timeout));
        sqlite_query_column(hStmt, 4, &(tour[tour_id].keys[key_id].speed));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_ptz_tour(const char *pDbFile, struct ptz_tour tour[TOUR_MAX], int ptz_id)
{
    if (!pDbFile || !tour || ptz_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "delete from ptz_tour where ptz_id=%d;", ptz_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < TOUR_MAX; i++) {
        for (j = 0; j < MS_KEY_MAX; j++) {
            if (tour[i].keys[j].preset_id == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into ptz_tour values(%d,%d,%d,%d,%d,%d);", ptz_id, i, j,
                     tour[i].keys[j].preset_id, tour[i].keys[j].timeout, tour[i].keys[j].speed);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int get_param_value(const char *pDbFile, const char *key, char *value, int value_len, const char *sdefault)
{
    if (!pDbFile || !key || !value) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        snprintf(value, value_len, "%s", sdefault);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[256] = {0};
    snprintf(sQuery, sizeof(sQuery), "select value from params where name='%s';", key);
    int nColumnCnt = 0;
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        snprintf(value, value_len, "%s", sdefault);
        return -1;
    }
    sqlite_query_column_text(hStmt, 0, value, value_len);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int get_param_int(const char *pDbFile, const char *key, int defvalue)
{
    char value[128] = {0};
    if (!pDbFile || !key) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);

    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return defvalue;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[256] = {0};
    snprintf(sQuery, sizeof(sQuery), "select value from params where name='%s';", key);
    int nColumnCnt = 0;
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return defvalue;
    }
    sqlite_query_column_text(hStmt, 0, value, sizeof(value));

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return atoi(value);
}

int set_param_value(const char *pDbFile, const char *key, const char *value)
{
    if (!pDbFile || !key || !value) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    char buf[128] = {0};
    if (strstr(key, PARAM_POE_PSD) || strstr(key, PARAM_DEVICE_SN)) {
        translate_pwd(buf, value, strlen(value));
        snprintf(sExec, sizeof(sExec), "update params set value ='%s' where name='%s';", buf, key);
    } else {
        snprintf(sExec, sizeof(sExec), "update params set value ='%s' where name='%s';", value, key);
    }
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int set_param_int(const char *pDbFile, const char *key, int value)
{
    if (!pDbFile || !key) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "update params set value ='%d' where name='%s';", value, key);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int add_param_value(const char *pDbFile, const char *key, const char *value)
{
    if (!pDbFile || !key || !value) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "insert into params(name,value) values('%s','%s');", key, value);
    int ret = sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return ret;
}

int add_param_int(const char *pDbFile, const char *key, int value)
{
    if (!pDbFile || !key) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "insert into params(name,value) values('%s','%d');", key, value);
    int ret = sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return ret;
}

int check_param_key(const char *pDbFile, const char *key)
{
    if (!pDbFile || !key) {
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
    snprintf(sQuery, sizeof(sQuery), "select value from params where name='%s';", key);
    int nColumnCnt = 0;
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);

    return 0;
}

//params_oem
int get_param_oem_value(const char *pDbFile, const char *key, char *value, int value_len, const char *sdefault)
{
    if (!pDbFile || !key || !value) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        snprintf(value, value_len, "%s", sdefault);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[256] = {0};
    snprintf(sQuery, sizeof(sQuery), "select value from params_oem where name='%s';", key);
    int nColumnCnt = 0;
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        snprintf(value, value_len, "%s", sdefault);
        return -1;
    }
    sqlite_query_column_text(hStmt, 0, value, value_len);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int get_param_oem_int(const char *pDbFile, const char *key, int defvalue)
{
    char value[128] = {0};
    if (!pDbFile || !key) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return defvalue;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[256] = {0};
    snprintf(sQuery, sizeof(sQuery), "select value from params_oem where name='%s';", key);
    int nColumnCnt = 0;
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return defvalue;
    }
    sqlite_query_column_text(hStmt, 0, value, sizeof(value));

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return atoi(value);
}

int set_param_oem_value(const char *pDbFile, const char *key, char *value)
{
    if (!pDbFile || !key || !value) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "update params_oem set value ='%s' where name='%s';", value, key);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int set_param_oem_int(const char *pDbFile, const char *key, int value)
{
    if (!pDbFile || !key) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "update params_oem set value ='%d' where name='%s';", value, key);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int add_param_oem_value(const char *pDbFile, const char *key, char *value)
{
    if (!pDbFile || !key || !value) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "insert into params_oem(name,value) values('%s','%s');", key, value);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int add_param_oem_int(const char *pDbFile, const char *key, int value)
{
    if (!pDbFile || !key) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "insert into params_oem(name,value) values('%s','%d');", key, value);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int check_param_oem_key(const char *pDbFile, const char *key)
{
    if (!pDbFile || !key) {
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
    snprintf(sQuery, sizeof(sQuery), "select value from params_oem where name='%s';", key);
    int nColumnCnt = 0;
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);

    return 0;
}


/* ipc protocol */
int read_ipc_protocols(const char *pDbFile, struct ipc_protocol **protocols, int *pCnt)
{
    if (!pDbFile || !pCnt || !protocols) {
        return -1;
    }
    int nFd;
    int mode = FILE_MODE_RD;
    if ((nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock)) < 0) {
        return -1;
    }
    HSQLITE hConn = NULL;
    HSQLSTMT hStmt = NULL;
    HSQLSTMT qStmt = NULL;
    int ret, cols, rows = 0, flag = -1, i = 0;
    struct ipc_protocol *ppro = NULL;
    char buf[256] = {0};

    do {
        if (sqlite_conn(pDbFile, &hConn)) {
            break;
        }
#if 1
        snprintf(buf, sizeof(buf), "select count(*) from protocol;");
        ret = sqlite_query_record(hConn, buf, &hStmt, &cols);
        if (ret != 0 || cols == 0) {
            break;
        }
        if (sqlite_query_column(hStmt, 0, &rows) || !rows) {
            break;
        }
        sqlite_clear_stmt(hStmt);
        hStmt = NULL;
#endif
        snprintf(buf, sizeof(buf), "select pro_id,pro_name,function,enable,display_model from protocol;");
        ret = sqlite_query_record(hConn, buf, &qStmt, &cols);
        if (ret != 0 || cols == 0) {
            break;
        }
        if ((ppro = ms_calloc(rows, sizeof(struct ipc_protocol))) == NULL) {
            break;
        }
        do {
            sqlite_query_column(qStmt, 0, &ppro[i].pro_id);
            sqlite_query_column_text(qStmt, 1, ppro[i].pro_name, sizeof(ppro[i].pro_name));
            sqlite_query_column(qStmt, 2, &ppro[i].function);
            sqlite_query_column(qStmt, 3, &ppro[i].enable);
            sqlite_query_column(qStmt, 4, &ppro[i].display_model);
            i++;
        } while (sqlite_query_next(qStmt) == 0 && i < rows);
        flag = 0;
        *pCnt = rows;
        *protocols = ppro;
    } while (0);
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
    }
    if (qStmt) {
        sqlite_clear_stmt(qStmt);
    }
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return flag;
}

int read_ipc_protocol(const char *pDbFile, struct ipc_protocol *protocol, int pro_id)
{
    if (!pDbFile || !protocol) {
        return -1;
    }
    int nFd;
    int mode = FILE_MODE_RD;
    if ((nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock)) < 0) {
        return -1;
    }
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[256] = { 0 };
    snprintf(sQuery, sizeof(sQuery), "select pro_id,pro_name,function,enable,display_model from protocol where pro_id=%d;",
             pro_id);
    int nColumnCnt = 0;
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &protocol->pro_id);
    sqlite_query_column_text(hStmt, 1, protocol->pro_name, sizeof(protocol->pro_name));
    sqlite_query_column(hStmt, 2, &protocol->function);
    sqlite_query_column(hStmt, 3, &protocol->enable);
    sqlite_query_column(hStmt, 4, &protocol->display_model);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_ipc_protocol(const char *pDbFile, struct ipc_protocol *protocol)
{
    if (!pDbFile || !protocol) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec),
             "update protocol set pro_name='%s', function=%d, enable=%d, display_model=%d where pro_id=%d;", protocol->pro_name,
             protocol->function, protocol->enable, protocol->display_model, protocol->pro_id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int delete_ipc_protocols(const char *pDbFile)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "delete from protocol;");
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int insert_ipc_protocol(const char *pDbFile, struct ipc_protocol *protocol)
{
    if (!pDbFile || !protocol) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec),
             "insert into protocol(pro_id,pro_name,function,enable,display_model) values(%d, '%s', %d, %d, %d);", protocol->pro_id,
             protocol->pro_name, protocol->function, protocol->enable, protocol->display_model);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

void release_ipc_protocol(struct ipc_protocol **protocols)
{
    if (!protocols || !*protocols) {
        return;
    }

    ms_free(*protocols);
    *protocols = NULL;
}


/* ipc model */
int read_ipc_models(const char *pDbFile, struct ipc_model **models, int *pCnt)
{
    if (!pDbFile || !pCnt || !models) {
        return -1;
    }
    int nFd;
    int mode = FILE_MODE_RD;
    if ((nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock)) < 0) {
        return -1;
    }
    HSQLITE hConn = NULL;
    HSQLSTMT hStmt = NULL;
    HSQLSTMT qStmt = NULL;
    int ret, cols, rows = 0, flag = -1, i = 0;
    struct ipc_model *pmod = NULL;
    char buf[256] = { 0 };

    do {
        if (sqlite_conn(pDbFile, &hConn)) {
            break;
        }
        snprintf(buf, sizeof(buf), "select count(*) from model;");
        ret = sqlite_query_record(hConn, buf, &hStmt, &cols);
        if (ret != 0 || cols == 0) {
            break;
        }
        if (sqlite_query_column(hStmt, 0, &rows) || !rows) {
            break;
        }
        snprintf(buf, sizeof(buf), "select mod_id,pro_id,mod_name,alarm_type,default_model from model;");
        ret = sqlite_query_record(hConn, buf, &qStmt, &cols);
        if (ret != 0 || cols == 0) {
            break;
        }
        if ((pmod = ms_calloc(rows, sizeof(struct ipc_model))) == NULL) {
            break;
        }
        do {
            sqlite_query_column(qStmt, 0, &pmod[i].mod_id);
            sqlite_query_column(qStmt, 1, &pmod[i].pro_id);
            sqlite_query_column_text(qStmt, 2, pmod[i].mod_name, sizeof(pmod[i].mod_name));
            sqlite_query_column(qStmt, 3, &pmod[i].alarm_type);
            sqlite_query_column(qStmt, 4, &pmod[i].default_model);
            i++;
        } while (sqlite_query_next(qStmt) == 0 && i < rows);
        flag = 0;
        *pCnt = rows;
        *models = pmod;
    } while (0);
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
    }
    if (qStmt) {
        sqlite_clear_stmt(qStmt);
    }
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return flag;
}

int read_ipc_models_by_protocol(const char *pDbFile, struct ipc_model **models, int pro_id, int *pCnt)
{
    if (!pDbFile || !pCnt || !models) {
        return -1;
    }
    int nFd;
    int mode = FILE_MODE_RD;
    if ((nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock)) < 0) {
        return -1;
    }
    HSQLITE hConn = NULL;
    HSQLSTMT hStmt = NULL;
    HSQLSTMT qStmt = NULL;
    int ret, cols, rows = 0, flag = -1, i = 0;
    struct ipc_model *pmod = NULL;
    char buf[256] = { 0 };

    do {
        if (sqlite_conn(pDbFile, &hConn)) {
            break;
        }
        snprintf(buf, sizeof(buf),
                 "select count(*) from model where pro_id=%d;", pro_id);
        ret = sqlite_query_record(hConn, buf, &hStmt, &cols);
        if (ret != 0 || cols == 0) {
            break;
        }
        if (sqlite_query_column(hStmt, 0, &rows) || !rows) {
            break;
        }
        snprintf(buf, sizeof(buf),
                 "select mod_id,pro_id,mod_name,alarm_type,default_model from model where pro_id=%d;",
                 pro_id);
        ret = sqlite_query_record(hConn, buf, &qStmt, &cols);
        if (ret != 0 || cols == 0) {
            break;
        }
        if ((pmod = ms_calloc(rows, sizeof(struct ipc_model))) == NULL) {
            break;
        }
        do {
            sqlite_query_column(qStmt, 0, &pmod[i].mod_id);
            sqlite_query_column(qStmt, 1, &pmod[i].pro_id);
            sqlite_query_column_text(qStmt, 2, pmod[i].mod_name, sizeof(pmod[i].mod_name));
            sqlite_query_column(qStmt, 3, &pmod[i].alarm_type);
            sqlite_query_column(qStmt, 4, &pmod[i].default_model);
            i++;
        } while (sqlite_query_next(qStmt) == 0 && i < rows);
        flag = 0;
        *pCnt = rows;
        *models = pmod;
    } while (0);
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
    }
    if (qStmt) {
        sqlite_clear_stmt(qStmt);
    }
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return flag;
}

int read_ipc_model(const char *pDbFile, struct ipc_model *model, int mod_id)
{
    if (!pDbFile || !model) {
        return -1;
    }
    int nFd;
    int mode = FILE_MODE_RD;
    if ((nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock)) < 0) {
        return -1;
    }
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[256] = { 0 };
    snprintf(sQuery, sizeof(sQuery),
             "select mod_id,pro_id,mod_name,alarm_type,default_model from model where mod_id=%d;",
             mod_id);
    int nColumnCnt = 0;
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &model->mod_id);
    sqlite_query_column(hStmt, 1, &model->pro_id);
    sqlite_query_column_text(hStmt, 2, model->mod_name, sizeof(model->mod_name));
    sqlite_query_column(hStmt, 3, &model->alarm_type);
    sqlite_query_column(hStmt, 4, &model->default_model);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_ipc_default_model(const char *pDbFile, struct ipc_model *model, int pro_id)
{
    if (!pDbFile || !model) {
        return -1;
    }
    int nFd;
    int mode = FILE_MODE_RD;
    if ((nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock)) < 0) {
        return -1;
    }
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[256] = { 0 };
    snprintf(sQuery, sizeof(sQuery),
             "select mod_id,pro_id,mod_name,alarm_type,default_model from model where pro_id=%d and default_model=1;",
             pro_id);
    int nColumnCnt = 0;
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &model->mod_id);
    sqlite_query_column(hStmt, 1, &model->pro_id);
    sqlite_query_column_text(hStmt, 2, model->mod_name, sizeof(model->mod_name));
    sqlite_query_column(hStmt, 3, &model->alarm_type);
    sqlite_query_column(hStmt, 4, &model->default_model);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_ipc_model(const char *pDbFile, struct ipc_model *model)
{
    if (!pDbFile || !model) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec),
             "update model set pro_id=%d, mod_name='%s', alarm_type=%d, default_model=%d where mod_id=%d;", model->pro_id,
             model->mod_name, model->alarm_type, model->default_model, model->mod_id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int insert_ipc_model(const char *pDbFile, struct ipc_model *model)
{
    if (!pDbFile || !model) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec),
             "insert into model(mod_id,pro_id,mod_name,alarm_type,default_model) values(%d, %d, '%s', %d, %d);", model->mod_id,
             model->pro_id, model->mod_name, model->alarm_type, model->default_model);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int delete_ipc_models(const char *pDbFile)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "delete from model;");
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

void ipc_destroy(void *ptr)
{
    ms_free(ptr);
}

int read_mosaic(const char *pDbFile, struct mosaic *mosaic)
{
    if (!pDbFile || !mosaic) {
        return -1;
    }
    int nFd;
    int mode = FILE_MODE_RD;
    if ((nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock)) < 0) {
//      perror("lock file");
        return -1;
    }
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char cmd[256] = {0};
    snprintf(cmd, sizeof(cmd), "select layout_mode,channels from mosaic where layout_mode=%d;", mosaic->layoutmode);
    if (sqlite_prepare_blob(hConn, cmd, &hStmt, 1)) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &mosaic->layoutmode);
    sqlite_query_column_blob(hStmt, 1, mosaic->channels, sizeof(mosaic->channels));

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_mosaics(const char *pDbFile, struct mosaic mosaic[LAYOUT_MODE_MAX], int *pCnt)
{
    if (!pDbFile || !pCnt) {
        return -1;
    }
    int nFd;
    int mode = FILE_MODE_RD;
    if ((nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock)) < 0) {
        perror("lock file");
        return -1;
    }
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        perror("conn file");
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char cmd[256] = {0};
    snprintf(cmd, sizeof(cmd), "select layout_mode,channels from mosaic;");
    if (sqlite_prepare_blob(hConn, cmd, &hStmt, 1)) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        perror("sqlite_prepare_blob");
        return -1;
    }
    int i = 0;
    for (; i < LAYOUT_MODE_MAX; i++) {
        sqlite_query_column(hStmt, 0, &mosaic[i].layoutmode);
        sqlite_query_column_blob(hStmt, 1, mosaic[i].channels, sizeof(mosaic[i].channels));
        if (sqlite_query_next(hStmt)) {
            i++;
            break;
        }
    }
    *pCnt = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_mosaic(const char *pDbFile, struct mosaic *mosaic)
{
    if (!pDbFile || !mosaic) {
        return -1;
    }
    int nFd;
    int mode = FILE_MODE_WR;
    if ((nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock)) < 0) {
        return -1;
    }
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    HSQLSTMT hStmt = 0;
    char cmd[256] = {0};
    snprintf(cmd, sizeof(cmd), "update mosaic set channels=? where layout_mode=%d;", mosaic->layoutmode);
    if (sqlite_prepare_blob(hConn, cmd, &hStmt, 0)) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int flag = 0;
    if (sqlite_exec_blob(hStmt, 1, mosaic->channels, sizeof(mosaic->channels))) {
        flag = -1;
    }
    sqlite_clear_stmt(hStmt);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return flag;
}

int update_mosaics(const char *pDbFile, struct mosaic *mosaic, int count)
{
    if (!pDbFile || !mosaic) {
        return -1;
    }
    int nFd;
    int mode = FILE_MODE_WR;
    if ((nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock)) < 0) {
        return -1;
    }
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    HSQLSTMT hStmt = 0;
    char cmd[256] = {0};
    int i = 0;
    int flag = 0;
    for (; i < count; ++i) {
        snprintf(cmd, sizeof(cmd), "update mosaic set channels=? where layout_mode=%d;", mosaic[i].layoutmode);
        if (sqlite_prepare_blob(hConn, cmd, &hStmt, 0)) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);
            sqlite_disconn(hConn);
            FileUnlock(nFd, mode, &global_rwlock);
            return -1;
        }
        if (sqlite_exec_blob(hStmt, 1, mosaic[i].channels, sizeof(mosaic[i].channels))) {
            flag = -1;
        }
    }
    sqlite_clear_stmt(hStmt);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return flag;
}

int read_custom_mosaics(const char *pDbFile, struct custom_mosaic *mosaics, int *count)
{
    if (!pDbFile || !mosaics) {
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
    char cmd[256] = {0};
    snprintf(cmd, sizeof(cmd), "select name,screen,type,row,column,positions,channels from mosaic_custom;");
    if (sqlite_prepare_blob(hConn, cmd, &hStmt, 1)) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        perror("sqlite_prepare_blob");
        return -1;
    }

    int i = 0;
    do {
        sqlite_query_column_text(hStmt, 0, mosaics[i].name, sizeof(mosaics[i].name));
        sqlite_query_column(hStmt, 1, &mosaics[i].screen);
        sqlite_query_column(hStmt, 2, &mosaics[i].type);
        sqlite_query_column(hStmt, 3, &mosaics[i].baseRow);
        sqlite_query_column(hStmt, 4, &mosaics[i].baseColumn);
        sqlite_query_column_blob(hStmt, 5, mosaics[i].positions, sizeof(mosaics[i].positions));
        sqlite_query_column_blob(hStmt, 6, mosaics[i].channels, sizeof(mosaics[i].channels));
        i++;
    } while (sqlite_query_next(hStmt) == 0);

    *count = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_custom_mosaics(const char *pDbFile, struct custom_mosaic *mosaics, int count)
{
    if (!pDbFile || !mosaics) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int i = 0;
    int ret = 0;
    char cmd[256] = {0};
    HSQLSTMT hStmt = 0;
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    //
    snprintf(cmd, sizeof(cmd), "delete from mosaic_custom;");
    sqlite_execute(hConn, mode, cmd);
    //
    for (i = 0; i < count; ++i) {
        snprintf(cmd, sizeof(cmd),
                 "insert into mosaic_custom(name,screen,type,row,column,positions,channels) values('%s',%d,%d,%d,%d,?,?);",
                 mosaics[i].name, mosaics[i].screen, mosaics[i].type, mosaics[i].baseRow, mosaics[i].baseColumn);
        if (sqlite_prepare_blob(hConn, cmd, &hStmt, 0)) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);
            sqlite_disconn(hConn);
            FileUnlock(nFd, mode, &global_rwlock);
            return -1;
        }
        ret = sqlite3_bind_blob(hStmt, 1, mosaics[i].positions, sizeof(mosaics[i].positions), NULL);
        if (ret != SQLITE_OK) {
            msprintf("sqlite3_bind_blob 1: error, %d\n", ret);
        }
        ret = sqlite3_bind_blob(hStmt, 2, mosaics[i].channels, sizeof(mosaics[i].channels), NULL);
        if (ret != SQLITE_OK) {
            msprintf("sqlite3_bind_blob 2: error, %d\n", ret);
        }
        sqlite3_step(hStmt);
    }
    sqlite_clear_stmt(hStmt);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_custom_mosaics_64Screen(const char *pDbFile, struct custom_mosaic *mosaics, int *count)
{
    if (!pDbFile || !mosaics) {
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
    char cmd[256] = {0};
    snprintf(cmd, sizeof(cmd), "select name,screen,type,row,column,positions,channels from mosaic_custom_64Screen;");
    if (sqlite_prepare_blob(hConn, cmd, &hStmt, 1)) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        perror("sqlite_prepare_blob");
        return -1;
    }

    int i = 0;
    do {
        sqlite_query_column_text(hStmt, 0, mosaics[i].name, sizeof(mosaics[i].name));
        sqlite_query_column(hStmt, 1, &mosaics[i].screen);
        sqlite_query_column(hStmt, 2, &mosaics[i].type);
        sqlite_query_column(hStmt, 3, &mosaics[i].baseRow);
        sqlite_query_column(hStmt, 4, &mosaics[i].baseColumn);
        sqlite_query_column_blob(hStmt, 5, mosaics[i].positions, sizeof(mosaics[i].positions));
        sqlite_query_column_blob(hStmt, 6, mosaics[i].channels, sizeof(mosaics[i].channels));
        i++;
    } while (sqlite_query_next(hStmt) == 0);

    *count = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_custom_mosaics_64Screen(const char *pDbFile, struct custom_mosaic *mosaics, int count)
{
    if (!pDbFile || !mosaics) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int i = 0;
    int ret = 0;
    char cmd[256] = {0};
    HSQLSTMT hStmt = 0;
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    //
    snprintf(cmd, sizeof(cmd), "delete from mosaic_custom_64Screen;");
    sqlite_execute(hConn, mode, cmd);
    //
    for (i = 0; i < count; ++i) {
        snprintf(cmd, sizeof(cmd),
                 "insert into mosaic_custom_64Screen(name,screen,type,row,column,positions,channels) values('%s',%d,%d,%d,%d,?,?);",
                 mosaics[i].name, mosaics[i].screen, mosaics[i].type, mosaics[i].baseRow, mosaics[i].baseColumn);
        if (sqlite_prepare_blob(hConn, cmd, &hStmt, 0)) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);
            sqlite_disconn(hConn);
            FileUnlock(nFd, mode, &global_rwlock);
            return -1;
        }
        ret = sqlite3_bind_blob(hStmt, 1, mosaics[i].positions, sizeof(mosaics[i].positions), NULL);
        if (ret != SQLITE_OK) {
            msprintf("sqlite3_bind_blob 1: error, %d\n", ret);
        }
        ret = sqlite3_bind_blob(hStmt, 2, mosaics[i].channels, sizeof(mosaics[i].channels), NULL);
        if (ret != SQLITE_OK) {
            msprintf("sqlite3_bind_blob 2: error, %d\n", ret);
        }
        sqlite3_step(hStmt);
    }
    sqlite_clear_stmt(hStmt);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_disk_maintain_info(const char *pDbFile, struct disk_maintain_info *info)
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
    char sQuery[256] = {0};
    int nColumnCnt = 0;
    snprintf(sQuery, sizeof(sQuery), "select log,photo from disk_maintain");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &(info->log));
    sqlite_query_column(hStmt, 1, &(info->photo));

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_disk_maintain_info(const char *pDbFile, struct disk_maintain_info *info)
{
    if (!pDbFile || !info) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "update disk_maintain set log=%d,photo=%d", info->log, info->photo);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_p2p_info(const char *pDbFile, struct p2p_info *info)
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
    char sQuery[256] = {0};
    int nColumnCnt = 0;
    snprintf(sQuery, sizeof(sQuery), "select enable,company,email,dealer,ipc,region from p2p");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &(info->enable));
    sqlite_query_column_text(hStmt, 1, info->company, sizeof(info->company));
    sqlite_query_column_text(hStmt, 2, info->email, sizeof(info->email));
    sqlite_query_column_text(hStmt, 3, info->dealer, sizeof(info->dealer));
    sqlite_query_column_text(hStmt, 4, info->ipc, sizeof(info->ipc));
    sqlite_query_column(hStmt, 5, (int *)&(info->region));

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_p2p_info(const char *pDbFile, struct p2p_info *info)
{
    if (!pDbFile || !info) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "update p2p set enable=%d,company='%s',email='%s',dealer='%s',ipc='%s'", info->enable,
             info->company, info->email, info->dealer, info->ipc);
    if (info->region != MS_INVALID_VALUE) {
        snprintf(sExec + strlen(sExec), sizeof(sExec) - strlen(sExec), ",region=%d", info->region);
    }
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_privacy_mask(const char *pDbFile, struct privacy_mask *mask, int chn_id)
{
    if (!pDbFile || !mask) {
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
    char sQuery[272] = {0};
    int nColumnCnt = 0;
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select id,area_id,enable,start_x,start_y,area_width,area_height,width,height,fill_color from privacy_mask where id = %d order by(area_id);",
             chn_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < MAX_MASK_AREA_NUM; i++) {
        sqlite_query_column(hStmt, 0, &mask->id);
        sqlite_query_column(hStmt, 1, &mask->area[i].area_id);
        sqlite_query_column(hStmt, 2, &mask->area[i].enable);
        sqlite_query_column(hStmt, 3, &mask->area[i].start_x);
        sqlite_query_column(hStmt, 4, &mask->area[i].start_y);
        sqlite_query_column(hStmt, 5, &mask->area[i].area_width);
        sqlite_query_column(hStmt, 6, &mask->area[i].area_height);
        sqlite_query_column(hStmt, 7, &mask->area[i].width);
        sqlite_query_column(hStmt, 8, &mask->area[i].height);
        sqlite_query_column(hStmt, 9, &mask->area[i].fill_color);
        if (sqlite_query_next(hStmt) != 0) {
            break;
        }
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_privacy_masks(const char *pDbFile, struct privacy_mask mask[], int *pCnt, DB_TYPE o_flag)
{
    if (!pDbFile || !mask) {
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
    int i = 0, j = 0;
    int arer_num = 1;
    snprintf(sQuery, sizeof(sQuery),
             "select id,area_id,enable,start_x,start_y,area_width,area_height,width,height,fill_color from privacy_mask order by id,area_id;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *pCnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    if (o_flag != DB_OLD) {
        arer_num = MAX_MASK_AREA_NUM;
    }
    for (i = 0; i < MAX_CAMERA; i++) {
        for (j = 0; j < arer_num; j++) {
            sqlite_query_column(hStmt, 0, &mask[i].id);
            sqlite_query_column(hStmt, 1, &mask[i].area[j].area_id);
            sqlite_query_column(hStmt, 2, &mask[i].area[j].enable);
            sqlite_query_column(hStmt, 3, &mask[i].area[j].start_x);
            sqlite_query_column(hStmt, 4, &mask[i].area[j].start_y);
            sqlite_query_column(hStmt, 5, &mask[i].area[j].area_width);
            sqlite_query_column(hStmt, 6, &mask[i].area[j].area_height);
            sqlite_query_column(hStmt, 7, &mask[i].area[j].width);
            sqlite_query_column(hStmt, 8, &mask[i].area[j].height);
            sqlite_query_column(hStmt, 9, &mask[i].area[j].fill_color);
            if (sqlite_query_next(hStmt) != 0) {
                j++;
                break;
            }
        }
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    *pCnt = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_privacy_mask(const char *pDbFile, struct privacy_mask *mask)
{
    if (!pDbFile || !mask) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int i = 0;
    char sExec[512] = {0};
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (i = 0; i < MAX_MASK_AREA_NUM; i++) {
        snprintf(sExec, sizeof(sExec),
                 "update privacy_mask set enable=%d,start_x=%d,start_y=%d,area_width=%d,area_height=%d,width=%d,height=%d,fill_color=%d where id=%d and area_id=%d;",
                 mask->area[i].enable, mask->area[i].start_x, mask->area[i].start_y, mask->area[i].area_width, mask->area[i].area_height,
                 mask->area[i].width, mask->area[i].height, mask->area[i].fill_color, mask->id, i);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_privacy_masks(const char *pDbFile, struct privacy_mask *mask, long long changeFlag)
{
    if (!pDbFile || !mask) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int i = 0, j = 0;
    char sExec[512] = {0};
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (j = 0; j < MAX_CAMERA; j++) {
        if (!(changeFlag >> j & 0x01)) {
            continue;
        }

        for (i = 0; i < MAX_MASK_AREA_NUM; i++) {
            snprintf(sExec, sizeof(sExec),
                     "update privacy_mask set enable=%d,start_x=%d,start_y=%d,area_width=%d,area_height=%d,width=%d,height=%d,fill_color=%d where id=%d and area_id=%d;",
                     mask->area[i].enable, mask->area[i].start_x, mask->area[i].start_y, mask->area[i].area_width, mask->area[i].area_height,
                     mask->area[i].width, mask->area[i].height, mask->area[i].fill_color, j, i);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int insert_privacy_mask(const char *pDbFile, struct privacy_mask *mask)
{
    if (!pDbFile || !mask) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    int i = 0;
    char sExec[1024] = {0};
    for (i = 0; i < /*MAX_MASK_AREA_NUM*/1; i++) {
        snprintf(sExec, sizeof(sExec),
                 "insert into privacy_mask(id,area_id,enable,start_x,start_y,area_width,area_height,width,height,fill_color) values(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d);",
                 mask->id, mask->area[i].area_id, mask->area[i].enable, mask->area[i].start_x, mask->area[i].start_y,
                 mask->area[i].area_width, mask->area[i].area_height, mask->area[i].width, mask->area[i].height,
                 mask->area[i].fill_color);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_alarm_pids(const char *pDbFile, struct alarm_pid **info, int *pCnt)
{
    if (!pDbFile || !info) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = NULL;
    HSQLSTMT qStmt = NULL;
    int ret, cols, rows = 0, i = 0;
    struct alarm_pid *palarm = NULL;
    char buf[256] = { 0 };

    do {
        snprintf(buf, sizeof(buf), "select count(*) from token;");
        ret = sqlite_query_record(hConn, buf, &hStmt, &cols);
        if (ret != 0 || cols == 0) {
            break;
        }
        if (sqlite_query_column(hStmt, 0, &rows) || !rows) {
            break;
        }
        snprintf(buf, sizeof(buf), "select id,sn,enable,push_from,push_interval,name,last_time,push_sum,region,user_id from token;");
        ret = sqlite_query_record(hConn, buf, &qStmt, &cols);
        if (ret != 0 || cols == 0) {
            break;
        }

        if ((palarm = ms_calloc(rows, sizeof(struct alarm_pid))) == NULL) {
            break;
        }
        do {
            sqlite_query_column_text(qStmt, 0, palarm[i].id, sizeof(palarm[i].id));
            sqlite_query_column_text(qStmt, 1, palarm[i].sn, sizeof(palarm[i].sn));
            sqlite_query_column(qStmt, 2, &palarm[i].enable);
            sqlite_query_column(qStmt, 3, &palarm[i].from);
            sqlite_query_column(qStmt, 4, &palarm[i].push_interval);
            sqlite_query_column_text(qStmt, 5, palarm[i].name, sizeof(palarm[i].name));
            sqlite_query_column(qStmt, 6, (int *)&palarm[i].last_time);
            sqlite_query_column(qStmt, 7, &palarm[i].push_sum);
            sqlite_query_column_text(qStmt, 8, palarm[i].region, sizeof(palarm[i].region));
            sqlite_query_column(qStmt, 9, &palarm[i].userId);
            i++;
        } while (sqlite_query_next(qStmt) == 0 && i < rows);
        *pCnt = rows;
        *info = palarm;
    } while (0);
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
    }
    if (qStmt) {
        sqlite_clear_stmt(qStmt);
    }
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_alarm_pid(const char *pDbFile, struct alarm_pid *info)
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
    char sQuery[512] = {0};
    int nColumnCnt = 0;
    snprintf(sQuery, sizeof(sQuery),
        "select id,sn,enable,push_from,push_interval,name,last_time,push_sum,region,user_id from token where id='%s';",
        info->id);
//      printf("read_alarm_pid: cmd: %s\n", sQuery);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column_text(hStmt, 0, info->id, sizeof(info->id));
    sqlite_query_column_text(hStmt, 1, info->sn, sizeof(info->sn));
    sqlite_query_column(hStmt, 2, &info->enable);
    sqlite_query_column(hStmt, 3, &info->from);
    sqlite_query_column(hStmt, 4, &info->push_interval);
    sqlite_query_column_text(hStmt, 5, info->name, sizeof(info->name));
    sqlite_query_column(hStmt, 6, (int *)&info->last_time);
    sqlite_query_column(hStmt, 7, &info->push_sum);
    sqlite_query_column_text(hStmt, 8, info->region, sizeof(info->region));
    sqlite_query_column(hStmt, 9, &info->userId);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_alarm_pid_by_sn(const char *pDbFile, struct alarm_pid *info)
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
    char sQuery[512] = {0};
    int nColumnCnt = 0;
    snprintf(sQuery, sizeof(sQuery),
        "select id,sn,enable,push_from,push_interval,name,last_time,push_sum,region,user_id from token where sn='%s';", 
        info->sn);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column_text(hStmt, 0, info->id, sizeof(info->id));
    sqlite_query_column_text(hStmt, 1, info->sn, sizeof(info->sn));
    sqlite_query_column(hStmt, 2, &info->enable);
    sqlite_query_column(hStmt, 3, &info->from);
    sqlite_query_column(hStmt, 4, &info->push_interval);
    sqlite_query_column_text(hStmt, 5, info->name, sizeof(info->name));
    sqlite_query_column(hStmt, 6, (int *)&info->last_time);
    sqlite_query_column(hStmt, 7, &info->push_sum);
    sqlite_query_column_text(hStmt, 8, info->region, sizeof(info->region));
    sqlite_query_column(hStmt, 9, &info->userId);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_alarm_pid(const char *pDbFile, struct alarm_pid *info)
{
    if (!pDbFile || !info) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[1024] = {0};
    snprintf(sExec, sizeof(sExec), "insert into token(id,sn,enable,push_from,push_interval,name,last_time,push_sum,"
        "region,user_id) values('%s','%s',%d,%d,%d,'%s',null,null,'%s',%d);", info->id, info->sn, info->enable, 
        info->from, info->push_interval, info->name, info->region, info->userId);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int update_alarm_pid(const char *pDbFile, struct alarm_pid *info)
{
    if (!pDbFile || !info) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char osdNameStr[128] = {0};
    char osdNameSqlStr[128] = {0};
    int i = 0, j = 0;
    snprintf(osdNameStr, sizeof(osdNameStr), "%s", info->name);
    for (i = 0; i < strlen(osdNameStr); i++) {
        if (osdNameStr[i] == '\'') {
            osdNameSqlStr[j++] = '\'';
            osdNameSqlStr[j++] = '\'';
        } else {
            osdNameSqlStr[j++] = osdNameStr[i];
        }
    }
    osdNameSqlStr[j] = '\0';
    char sExec[1024] = {0};
    snprintf(sExec, sizeof(sExec),
        "update token set enable=%d,sn='%s',push_from=%d,push_interval=%d,name='%s',last_time=%ld,push_sum=%d,"
        "region='%s',user_id=%d where id='%s';", info->enable, info->sn, info->from, info->push_interval, osdNameSqlStr, 
        info->last_time, info->push_sum, info->region, info->userId, info->id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int update_alarm_pid_by_sn(const char *pDbFile, struct alarm_pid *pid)
{
    if (!pDbFile || !pid) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char osdNameStr[128] = {0};
    char osdNameSqlStr[128] = {0};
    int i = 0, j = 0;
    snprintf(osdNameStr, sizeof(osdNameStr), "%s", pid->name);
    for (i = 0; i < strlen(osdNameStr); i++) {
        if (osdNameStr[i] == '\'') {
            osdNameSqlStr[j++] = '\'';
            osdNameSqlStr[j++] = '\'';
        } else {
            osdNameSqlStr[j++] = osdNameStr[i];
        }
    }
    osdNameSqlStr[j] = '\0';
    char sExec[1024] = {0};
    snprintf(sExec, sizeof(sExec),
        "update token set id='%s',enable=%d,push_from=%d,push_interval=%d,name='%s',last_time=%ld,push_sum=%d,"
        "region='%s',user_id=%d where sn='%s';", pid->id, pid->enable, pid->from, pid->push_interval, osdNameSqlStr, 
        pid->last_time, pid->push_sum, pid->region, pid->userId, pid->sn);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int delete_alarm_pid(const char *pDbFile, char *id)
{
    if (!pDbFile || !id) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "delete from token where id='%s';", id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int delete_alarm_pid_by_user_id(const char *pDbFile, int id)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "delete from token where user_id=%d;", id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

void release_alarm_pid(struct alarm_pid **pid)
{
    if (!pid || !*pid) {
        return;
    }

    ms_free(*pid);
    *pid = NULL;
}

int read_trigger_alarms(const char *pDbFile, struct trigger_alarms *pa)
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
    char sQuery[516] = {0};
    int nColumnCnt = 0;
    snprintf(sQuery, sizeof(sQuery), "select network_disconn,disk_full,record_fail,disk_fail,disk_unformat,\
		no_disk,record_mail_interval,ip_conflict,disk_offline,disk_heat,disk_microtherm,disk_connection_exception,\
        disk_strike from trigger_alarms;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &pa->network_disconn);
    sqlite_query_column(hStmt, 1, &pa->disk_full);
    sqlite_query_column(hStmt, 2, &pa->record_fail);
    sqlite_query_column(hStmt, 3, &pa->disk_fail);
    sqlite_query_column(hStmt, 4, &pa->disk_unformat);
    sqlite_query_column(hStmt, 5, &pa->no_disk);
    sqlite_query_column(hStmt, 6, &pa->record_mail_interval);
    sqlite_query_column(hStmt, 7, &pa->ipConflict);
    sqlite_query_column(hStmt, 8, &pa->diskOffline);
    sqlite_query_column(hStmt, 9, &pa->diskHeat);
    sqlite_query_column(hStmt, 10, &pa->diskMicrotherm);
    sqlite_query_column(hStmt, 11, &pa->diskConnectionException);
    sqlite_query_column(hStmt, 12, &pa->diskStrike);
    sqlite_clear_stmt(hStmt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_trigger_alarms(const char *pDbFile, const struct trigger_alarms *pa)
{
    if (!pDbFile || !pa) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[516] = { 0 };
    snprintf(sExec, sizeof(sExec), "update trigger_alarms set network_disconn=%d, disk_full=%d, record_fail=%d, "
        "disk_fail=%d, disk_unformat=%d, no_disk=%d",
        pa->network_disconn, pa->disk_full, pa->record_fail, pa->disk_fail, pa->disk_unformat, pa->no_disk);
    if (pa->record_mail_interval != MS_INVALID_VALUE) {
        snprintf(sExec + strlen(sExec), sizeof(sExec) - strlen(sExec), ", record_mail_interval=%d", pa->record_mail_interval);
    }
    if (pa->ipConflict != MS_INVALID_VALUE) {
        snprintf(sExec + strlen(sExec), sizeof(sExec) - strlen(sExec), ", ip_conflict=%d", pa->ipConflict);
    }
    if (pa->diskOffline != MS_INVALID_VALUE) {
        snprintf(sExec + strlen(sExec), sizeof(sExec) - strlen(sExec), ", disk_offline=%d", pa->diskOffline);
    }
    if (pa->diskHeat != MS_INVALID_VALUE) {
        snprintf(sExec + strlen(sExec), sizeof(sExec) - strlen(sExec), ", disk_heat=%d", pa->diskHeat);
    }
    if (pa->diskMicrotherm != MS_INVALID_VALUE) {
        snprintf(sExec + strlen(sExec), sizeof(sExec) - strlen(sExec), ", disk_microtherm=%d", pa->diskMicrotherm);
    }
    if (pa->diskConnectionException != MS_INVALID_VALUE) {
        snprintf(sExec + strlen(sExec), sizeof(sExec) - strlen(sExec), ", disk_connection_exception=%d", pa->diskConnectionException);
    }
    if (pa->diskStrike != MS_INVALID_VALUE) {
        snprintf(sExec + strlen(sExec), sizeof(sExec) - strlen(sExec), ", disk_strike=%d", pa->diskStrike);
    }
    snprintf(sExec + strlen(sExec), sizeof(sExec) - strlen(sExec), ";");
        
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_cameraios(const char *pDbFile, struct camera_io cio[], int *cnt)
{
    if (!pDbFile || !cio) {
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
    char sQuery[512] = {0};
    int nColumnCnt = 0;
    int i = 0;
    snprintf(sQuery, sizeof(sQuery), "select chanid, enable, tri_channels, tri_actions, tri_channels_ex from cameraio;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *cnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < MAX_CAMERA; i++) {
        sqlite_query_column(hStmt, 0, &cio[i].chanid);
        sqlite_query_column(hStmt, 1, &cio[i].enable);
        sqlite_query_column(hStmt, 2, (int *)&cio[i].tri_channels);
        sqlite_query_column(hStmt, 3, (int *)&cio[i].tri_actions);
        sqlite_query_column_text(hStmt, 4, cio[i].tri_channels_ex, sizeof(cio[i].tri_channels_ex));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    *cnt = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}


int read_cameraio(const char *pDbFile, struct camera_io *pio, int chanid)
{
    if (!pDbFile || !pio) {
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
    char sQuery[128] = {0};
    int nColumnCnt = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select chanid,enable,tri_channels,tri_actions,tri_channels_ex from cameraio where chanid=%d;", chanid);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &pio->chanid);
    sqlite_query_column(hStmt, 1, &pio->enable);
    sqlite_query_column(hStmt, 2, (int *)&pio->tri_channels);
    sqlite_query_column(hStmt, 3, (int *)&pio->tri_actions);
    sqlite_query_column_text(hStmt, 4, pio->tri_channels_ex, sizeof(pio->tri_channels_ex));
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int insert_cameraio(const char *pDbFile, const struct camera_io *pio)
{
    if (!pDbFile || !pio) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = { 0 };
    snprintf(sExec, sizeof(sExec),
             "insert into cameraio(chanid,enable,tri_channels,tri_actions,tri_channels_ex) values(%d,%d,%d,%d,'%s');",
             pio->chanid, pio->enable, pio->tri_channels, pio->tri_actions, pio->tri_channels_ex);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_cameraio(const char *pDbFile, const struct camera_io *pio)
{
    if (!pDbFile || !pio) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = { 0 };
    snprintf(sExec, sizeof(sExec),
             "update cameraio set enable=%d, tri_channels=%d, tri_actions=%d, tri_channels_ex='%s' where chanid=%d;",
             pio->enable, pio->tri_channels, pio->tri_actions, pio->tri_channels_ex, pio->chanid);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_volumedetects(const char *pDbFile, struct volume_detect vd[], int *cnt)
{
    if (!pDbFile || !vd) {
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
    char sQuery[512] = {0};
    int nColumnCnt = 0;
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select chanid, enable, tri_channels, tri_actions, tri_channels_ex from volumedetect;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *cnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < MAX_CAMERA; i++) {
        sqlite_query_column(hStmt, 0, &vd[i].chanid);
        sqlite_query_column(hStmt, 1, &vd[i].enable);
        sqlite_query_column(hStmt, 2, (int *)&vd[i].tri_channels);
        sqlite_query_column(hStmt, 3, (int *)&vd[i].tri_actions);
        sqlite_query_column_text(hStmt, 4, vd[i].tri_channels_ex, sizeof(vd[i].tri_channels_ex));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    *cnt = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_volumedetect(const char *pDbFile, struct volume_detect *pvd, int chanid)
{
    if (!pDbFile || !pvd) {
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
    char sQuery[128] = {0};
    int nColumnCnt = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select chanid,enable,tri_channels,tri_actions,tri_channels_ex from volumedetect where chanid=%d;", chanid);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, (int *)&pvd->chanid);
    sqlite_query_column(hStmt, 1, (int *)&pvd->enable);
    sqlite_query_column(hStmt, 2, (int *)&pvd->tri_channels);
    sqlite_query_column(hStmt, 3, (int *)&pvd->tri_actions);
    sqlite_query_column_text(hStmt, 4, pvd->tri_channels_ex, sizeof(pvd->tri_channels_ex));
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_volumedetect(const char *pDbFile, const struct volume_detect *pvd)
{
    if (!pDbFile || !pvd) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = { 0 };
    snprintf(sExec, sizeof(sExec),
             "update volumedetect set enable=%d, tri_channels=%d, tri_actions=%d, tri_channels_ex='%s' where chanid=%d;",
             pvd->enable, pvd->tri_channels, pvd->tri_actions, pvd->tri_channels_ex, pvd->chanid);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int insert_volume_detect(const char *pDbFile, const struct volume_detect *pvd)
{
    if (!pDbFile || !pvd) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = { 0 };
    snprintf(sExec, sizeof(sExec),
             "insert into volumedetect(chanid,enable,tri_channels,tri_actions,tri_channels_ex) values(%d,%d,%d,%d,'%s');",
             pvd->chanid, pvd->enable, pvd->tri_channels, pvd->tri_actions, pvd->tri_channels_ex);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}


//bruce.milesight add for param device information
static int ms_set_device(const char *pDbFile, struct device_info *device, const char *table)
{
    char sExec[256] = {0};
    if (!pDbFile || !device) {
        return -1;
    }
    int     mode  = FILE_MODE_WR;
    int     nFd   = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);

    snprintf(sExec, sizeof(sExec), "update %s set value='%d' where name='%s'", table, device->devid, PARAM_DEVICE_ID);
    sqlite_execute(hConn, mode, sExec);
    snprintf(sExec, sizeof(sExec), "update %s set value='%d' where name='%s'", table, device->max_cameras, PARAM_MAX_CAM);
    sqlite_execute(hConn, mode, sExec);
    snprintf(sExec, sizeof(sExec), "update %s set value='%d' where name='%s'", table, device->max_pb_num, PARAM_MAX_PB_CAM);
    sqlite_execute(hConn, mode, sExec);
    snprintf(sExec, sizeof(sExec), "update %s set value='%d' where name='%s'", table, device->max_disk, PARAM_MAX_DISK);
    sqlite_execute(hConn, mode, sExec);
    snprintf(sExec, sizeof(sExec), "update %s set value='%d' where name='%s'", table, device->max_alarm_in, PARAM_MAX_ALARM_IN);
    sqlite_execute(hConn, mode, sExec);
    snprintf(sExec, sizeof(sExec), "update %s set value='%d' where name='%s'", table, device->max_alarm_out, PARAM_MAX_ALARM_OUT);
    sqlite_execute(hConn, mode, sExec);
    snprintf(sExec, sizeof(sExec), "update %s set value='%d' where name='%s'", table, device->max_audio_in, PARAM_MAX_AUDIO_IN);
    sqlite_execute(hConn, mode, sExec);
    snprintf(sExec, sizeof(sExec), "update %s set value='%d' where name='%s'", table, device->max_audio_out, PARAM_MAX_AUDIO_OUT);
    sqlite_execute(hConn, mode, sExec);
    snprintf(sExec, sizeof(sExec), "update %s set value='%d' where name='%s'", table, device->max_screen, PARAM_MAX_SCREEN);
    sqlite_execute(hConn, mode, sExec);
    snprintf(sExec, sizeof(sExec), "update %s set value='%d' where name='%s'", table, device->max_hdmi, PARAM_MAX_HDMI);
    sqlite_execute(hConn, mode, sExec);
    snprintf(sExec, sizeof(sExec), "update %s set value='%d' where name='%s'", table, device->max_vga, PARAM_MAX_VGA);
    sqlite_execute(hConn, mode, sExec);
    snprintf(sExec, sizeof(sExec), "update %s set value='%d' where name='%s'", table, device->camera_layout, PARAM_CAMERA_LAYOUT);
    sqlite_execute(hConn, mode, sExec);
    snprintf(sExec, sizeof(sExec), "update %s set value='%d' where name='%s'", table, device->max_lan, PARAM_MAX_NETWORKS);
    sqlite_execute(hConn, mode, sExec);
    snprintf(sExec, sizeof(sExec), "update %s set value='%d' where name='%s'", table, device->max_rs485, PARAM_MAX_RS485);
    sqlite_execute(hConn, mode, sExec);
    snprintf(sExec, sizeof(sExec), "update %s set value='%d' where name='%s'", table, device->oem_type, PARAM_OEM_TYPE);
    sqlite_execute(hConn, mode, sExec);

    snprintf(sExec, sizeof(sExec), "update %s set value='%s' where name='%s'", table, device->prefix, PARAM_DEVICE_NO);
    sqlite_execute(hConn, mode, sExec);
    snprintf(sExec, sizeof(sExec), "update %s set value='%s' where name='%s'", table, device->model, PARAM_DEVICE_MODEL);
    sqlite_execute(hConn, mode, sExec);
    snprintf(sExec, sizeof(sExec), "update %s set value='%s' where name='%s'", table, device->sncode, PARAM_DEVICE_SN);
    sqlite_execute(hConn, mode, sExec);
    snprintf(sExec, sizeof(sExec), "update %s set value='%s' where name='%s'", table, device->softver, PARAM_DEVICE_SV);
    sqlite_execute(hConn, mode, sExec);
    snprintf(sExec, sizeof(sExec), "update %s set value='%s' where name='%s'", table, device->hardver, PARAM_DEVICE_HV);
    sqlite_execute(hConn, mode, sExec);
    snprintf(sExec, sizeof(sExec), "update %s set value='%s' where name='%s'", table, device->company, PARAM_DEVICE_COMPANY);
    sqlite_execute(hConn, mode, sExec);
    snprintf(sExec, sizeof(sExec), "update %s set value='%s' where name='%s'", table, device->lang, PARAM_DEVICE_LANG);
    sqlite_execute(hConn, mode, sExec);
    snprintf(sExec, sizeof(sExec), "update %s set value='%d' where name='%s'", table, device->def_lang, PARAM_DEVICE_DEF_LANG);
    sqlite_execute(hConn, mode, sExec);

    snprintf(sExec, sizeof(sExec), "update %s set value='%d' where name='%s'", table, device->power_key, PARAM_POWER_KEY);
    sqlite_execute(hConn, mode, sExec);

    snprintf(sExec, sizeof(sExec), "update %s set value='%d' where name='%s'", table, device->msddns, PARAM_MS_DDNS);
    sqlite_execute(hConn, mode, sExec);

    snprintf(sExec, sizeof(sExec), "update %s set value='%d' where name='%s'", table, device->alarm_ctl_mode, PARAM_ALARM_CTL_MODE);
    sqlite_execute(hConn, mode, sExec);
    snprintf(sExec, sizeof(sExec), "update %s set value='%d' where name='%s'", table, device->mcu_online_update, PARAM_MCU_ONLINE_UPDATE);
    sqlite_execute(hConn, mode, sExec);

    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int db_set_device(const char *pDbFile, struct device_info *device)
{
    return ms_set_device(pDbFile, device, "params");
}

int db_set_device_oem(const char *pDbFile, struct device_info *device)
{
    return ms_set_device(pDbFile, device, "params_oem");
}

static int ms_get_device(const char *pDbFile, struct device_info *device, const char *table)
{
    char sQuery[256] = {0};
    int nColumnCnt = 0, nFd, nResult;
    HSQLITE hConn = 0;
    HSQLSTMT hStmt = 0;
    int mode = FILE_MODE_RD;

    if (!pDbFile) {
        return -1;
    }
    nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -2;
    }

    //snprintf(device->devtype, sizeof(device->devtype), "N");//NVR
    snprintf(sQuery, sizeof(sQuery), "select value from %s where name='%s';", table, PARAM_DEVICE_ID);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (!nResult && nColumnCnt) {
        sqlite_query_column(hStmt, 0, (int *)&device->devid);
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }

    snprintf(sQuery, sizeof(sQuery), "select value from %s where name='%s';", table, PARAM_MAX_CAM);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (!nResult && nColumnCnt) {
        sqlite_query_column(hStmt, 0, (int *)&device->max_cameras);
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }

    snprintf(sQuery, sizeof(sQuery), "select value from %s where name='%s';", table, PARAM_MAX_PB_CAM);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (!nResult && nColumnCnt) {
        sqlite_query_column(hStmt, 0, (int *)&device->max_pb_num);
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }

    snprintf(sQuery, sizeof(sQuery), "select value from %s where name='%s';", table, PARAM_MAX_DISK);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (!nResult && nColumnCnt) {
        sqlite_query_column(hStmt, 0, (int *)&device->max_disk);
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }


    snprintf(sQuery, sizeof(sQuery), "select value from %s where name='%s';", table, PARAM_MAX_ALARM_IN);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (!nResult && nColumnCnt) {
        sqlite_query_column(hStmt, 0, (int *)&device->max_alarm_in);
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }

    snprintf(sQuery, sizeof(sQuery), "select value from %s where name='%s';", table, PARAM_MAX_ALARM_OUT);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (!nResult && nColumnCnt) {
        sqlite_query_column(hStmt, 0, (int *)&device->max_alarm_out);
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }

    snprintf(sQuery, sizeof(sQuery), "select value from %s where name='%s';", table, PARAM_MAX_AUDIO_IN);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (!nResult && nColumnCnt) {
        sqlite_query_column(hStmt, 0, (int *)&device->max_audio_in);
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }

    snprintf(sQuery, sizeof(sQuery), "select value from %s where name='%s';", table, PARAM_MAX_AUDIO_OUT);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (!nResult && nColumnCnt) {
        sqlite_query_column(hStmt, 0, (int *)&device->max_audio_out);
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }

    snprintf(sQuery, sizeof(sQuery), "select value from %s where name='%s';", table, PARAM_MAX_SCREEN);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (!nResult && nColumnCnt) {
        sqlite_query_column(hStmt, 0, (int *)&device->max_screen);
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }

    snprintf(sQuery, sizeof(sQuery), "select value from %s where name='%s';", table, PARAM_MAX_HDMI);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (!nResult && nColumnCnt) {
        sqlite_query_column(hStmt, 0, (int *)&device->max_hdmi);
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }

    snprintf(sQuery, sizeof(sQuery), "select value from %s where name='%s';", table, PARAM_MAX_VGA);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (!nResult && nColumnCnt) {
        sqlite_query_column(hStmt, 0, (int *)&device->max_vga);
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }

    snprintf(sQuery, sizeof(sQuery), "select value from %s where name='%s';", table, PARAM_MAX_RS485);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (!nResult && nColumnCnt) {
        sqlite_query_column(hStmt, 0, (int *)&device->max_rs485);
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }

    snprintf(sQuery, sizeof(sQuery), "select value from %s where name='%s';", table, PARAM_CAMERA_LAYOUT);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (!nResult && nColumnCnt) {
        sqlite_query_column(hStmt, 0, (int *)&device->camera_layout);
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }

    snprintf(sQuery, sizeof(sQuery), "select value from %s where name='%s';", table, PARAM_MAX_NETWORKS);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (!nResult && nColumnCnt) {
        sqlite_query_column(hStmt, 0, (int *)&device->max_lan);
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }

    snprintf(sQuery, sizeof(sQuery), "select value from %s where name='%s';", table, PARAM_OEM_TYPE);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (!nResult && nColumnCnt) {
        sqlite_query_column(hStmt, 0, (int *)&device->oem_type);
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }

    snprintf(sQuery, sizeof(sQuery), "select value from %s where name='%s';", table, PARAM_DEVICE_SN);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (!nResult && nColumnCnt) {
        sqlite_query_column_text(hStmt, 0, (char *)device->sncode, sizeof(device->sncode));
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }

    snprintf(sQuery, sizeof(sQuery), "select value from %s where name='%s';", table, PARAM_DEVICE_NO);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (!nResult && nColumnCnt) {
        sqlite_query_column_text(hStmt, 0, (char *)device->prefix, sizeof(device->prefix));
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }

    snprintf(sQuery, sizeof(sQuery), "select value from %s where name='%s';", table, PARAM_DEVICE_MODEL);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (!nResult && nColumnCnt) {
        sqlite_query_column_text(hStmt, 0, (char *)device->model, sizeof(device->model));
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }

    snprintf(sQuery, sizeof(sQuery), "select value from %s where name='%s';", table, PARAM_DEVICE_SV);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (!nResult && nColumnCnt) {
        sqlite_query_column_text(hStmt, 0, (char *)device->softver, sizeof(device->softver));
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }

    snprintf(sQuery, sizeof(sQuery), "select value from %s where name='%s';", table, PARAM_DEVICE_HV);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (!nResult && nColumnCnt) {
        sqlite_query_column_text(hStmt, 0, (char *)device->hardver, sizeof(device->hardver));
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }

    snprintf(sQuery, sizeof(sQuery), "select value from %s where name='%s';", table, PARAM_DEVICE_COMPANY);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (!nResult && nColumnCnt) {
        sqlite_query_column_text(hStmt, 0, (char *)device->company, sizeof(device->company));
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }

    snprintf(sQuery, sizeof(sQuery), "select value from %s where name='%s';", table, PARAM_DEVICE_DEF_LANG);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (!nResult && nColumnCnt) {
        sqlite_query_column(hStmt, 0, (int *)&device->def_lang);
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }

    snprintf(sQuery, sizeof(sQuery), "select value from %s where name='%s';", table, PARAM_DEVICE_LANG);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (!nResult && nColumnCnt) {
        sqlite_query_column_text(hStmt, 0, (char *)device->lang, sizeof(device->lang));
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }


    snprintf(sQuery, sizeof(sQuery), "select value from %s where name='%s';", table, PARAM_POWER_KEY);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (!nResult && nColumnCnt) {
        sqlite_query_column(hStmt, 0, (int *)&device->power_key);
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }


    snprintf(sQuery, sizeof(sQuery), "select value from %s where name='%s';", table, PARAM_MS_DDNS);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (!nResult && nColumnCnt) {
        sqlite_query_column(hStmt, 0, (int *)&device->msddns);
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }

    snprintf(sQuery, sizeof(sQuery), "select value from %s where name='%s';", table, PARAM_OEM_UPDATE_ONLINE);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (!nResult && nColumnCnt) {
        sqlite_query_column(hStmt, 0, (int *)&device->oemupdateonline);
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }

    snprintf(sQuery, sizeof(sQuery), "select value from %s where name='%s';", table, PARAM_OEM_APP_MSG_PUSH);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (!nResult && nColumnCnt) {
        sqlite_query_column(hStmt, 0, (int *)&device->oemappmsgpush);
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }

    snprintf(sQuery, sizeof(sQuery), "select value from %s where name='%s';", table, PARAM_ALARM_CTL_MODE);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (!nResult && nColumnCnt) {
        sqlite_query_column(hStmt, 0, (int *)&device->alarm_ctl_mode);
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }

    snprintf(sQuery, sizeof(sQuery), "select value from %s where name='%s';", table, PARAM_MCU_ONLINE_UPDATE);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (!nResult && nColumnCnt) {
        sqlite_query_column(hStmt, 0, (int *)&device->mcu_online_update);
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int db_get_device(const char *pDbFile, struct device_info *device)
{
    return ms_get_device(pDbFile, device, "params");
}

int db_get_device_oem(const char *pDbFile, struct device_info *device)
{
    return ms_get_device(pDbFile, device, "params_oem");
}

//bruce.milesight add
///////////////////////////////////////////////////////////////////////////////////////////////////////////
// timezone
int db_get_tz_filename(const char *pDbFile, const char *tz, char *tzname, int tzname_size, char *tzfname,
                       int tzfname_size)
{
    int nColumnCnt = 0;
    HSQLSTMT hStmt = 0;
    HSQLITE hConn = 0;
    char sQuery[512] = {0};
    int mode = FILE_MODE_RD;

    if (!pDbFile) {
        return -1;
    }
    int nFd = FileLock(FILE_TZ_NAME, mode, &global_rwlock);
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    if (tzname[0]) {
        snprintf(sQuery, sizeof(sQuery), "select name,zonefile from zonemap where timezone='%s' and name='%s'", tz, tzname);
    } else {
        snprintf(sQuery, sizeof(sQuery), "select name,zonefile from zonemap where timezone='%s'", tz);
    }

    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column_text(hStmt, 0, tzname, tzname_size);
    sqlite_query_column_text(hStmt, 1, tzfname, tzfname_size);
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);

    return 0;
}

int write_layout1_chn(const char *pDbFile, int chnid)
{
    if (!pDbFile) {
        return -1;
    }
    if (!check_param_key(pDbFile, PARAM_LAYOUT1_CHNID)) {
        set_param_int(pDbFile, PARAM_LAYOUT1_CHNID, chnid);
    } else {
        add_param_int(pDbFile, PARAM_LAYOUT1_CHNID, 0);
        set_param_int(pDbFile, PARAM_LAYOUT1_CHNID, chnid);
    }
    return 0;
}

int read_layout1_chn(const char *pDbFile, int *chnid)
{
    if (!pDbFile || !chnid) {
        return -1;
    }
    *chnid = get_param_int(pDbFile, PARAM_LAYOUT1_CHNID, 0);
    return 0;
}


//###################################################//
int db_read_user_by_name(const char *pDbFile, struct db_user *user, char *username)
{
    if (!pDbFile || !user || !username) {
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
    int nColumnCnt = 0;
    char sQuery[512] = {0};
    snprintf(sQuery, sizeof(sQuery),
             "select id,enable,username,password,type,permission,remote_permission,local_live_view,local_playback,remote_live_view,remote_playback from user where username='%s';",
             username);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, &(user->id));
    sqlite_query_column(hStmt, 1, &(user->enable));
    sqlite_query_column_text(hStmt, 2, user->username, sizeof(user->username));
    sqlite_query_column_text(hStmt, 3, user->password, sizeof(user->password));
    sqlite_query_column(hStmt, 4, &(user->type));
    sqlite_query_column(hStmt, 5, &(user->permission));
    sqlite_query_column(hStmt, 6, &(user->remote_permission));
    sqlite_query_column(hStmt, 7, (int *) & (user->local_live_view));
    sqlite_query_column(hStmt, 8, (int *) & (user->local_playback));
    sqlite_query_column(hStmt, 9, (int *) & (user->remote_live_view));
    sqlite_query_column(hStmt, 10, (int *) & (user->remote_playback));

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_users_oem(const char *pDbFile, struct db_user_oem users[], int *pCnt)
{
    if (!pDbFile || !users) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select id,enable,username,password,type,permission,remote_permission,local_live_view,local_playback,remote_live_view,remote_playback from user;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *pCnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < MAX_USER; i++) {
        sqlite_query_column(hStmt, 0, &(users[i].id));
        sqlite_query_column(hStmt, 1, &(users[i].enable));
        sqlite_query_column_text(hStmt, 2, users[i].username, sizeof(users[i].username));
        sqlite_query_column_text(hStmt, 3, users[i].password, sizeof(users[i].password));
        sqlite_query_column(hStmt, 4, &(users[i].type));
        sqlite_query_column(hStmt, 5, &(users[i].permission));
        sqlite_query_column(hStmt, 6, &(users[i].remote_permission));
        sqlite_query_column(hStmt, 7, (int *) & (users[i].local_live_view));
        sqlite_query_column(hStmt, 8, (int *) & (users[i].local_playback));
        sqlite_query_column(hStmt, 9, (int *) & (users[i].remote_live_view));
        sqlite_query_column(hStmt, 10, (int *) & (users[i].remote_playback));

        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    *pCnt = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}


int write_sub_layout1_chn(const char *pDbFile, int chnid)
{
    if (!pDbFile) {
        return -1;
    }
    if (!check_param_key(pDbFile, PARAM_SUB_LAYOUT1_CHNID)) {
        set_param_int(pDbFile, PARAM_SUB_LAYOUT1_CHNID, chnid);
    } else {
        add_param_int(pDbFile, PARAM_SUB_LAYOUT1_CHNID, 0);
        set_param_int(pDbFile, PARAM_SUB_LAYOUT1_CHNID, chnid);
    }
    return 0;
}

int read_sub_layout1_chn(const char *pDbFile, int *chnid)
{
    if (!pDbFile || !chnid) {
        return -1;
    }
    *chnid = get_param_int(pDbFile, PARAM_SUB_LAYOUT1_CHNID, 0);
    return 0;
}

int write_autoreboot_conf(const char *pDbFile, reboot_conf *conf)
{
    if (!pDbFile || !conf) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec),
             "update auto_reboot set enable=%d,wday=%d,hour=%d,login=%d,username='%s',reboot=%d,minutes=%d,seconds=%d where id=0;",
             conf->enable, conf->wday, conf->hour, conf->login, conf->username, conf->reboot, conf->minutes, conf->seconds);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_autoreboot_conf(const char *pDbFile, reboot_conf *conf)
{
    if (!pDbFile || !conf) {
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
    char sQuery[512] = {0};
    int nColumnCnt = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select enable,wday,hour,login,username,reboot,minutes,seconds from auto_reboot where id=0;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &conf->enable);
    sqlite_query_column(hStmt, 1, (int *)&conf->wday);
    sqlite_query_column(hStmt, 2, &conf->hour);
    sqlite_query_column(hStmt, 3, &conf->login);
    sqlite_query_column_text(hStmt, 4, conf->username, sizeof(conf->username));
    sqlite_query_column(hStmt, 5, &conf->reboot);
    sqlite_query_column(hStmt, 6, &conf->minutes);
    sqlite_query_column(hStmt, 7, &conf->seconds);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}



int read_privacy_mask_by_areaid(const char *pDbFile, struct privacy_mask *mask, int chn_id, int area_id)
{
    if (!pDbFile || !mask) {
        return -1;
    }
    if (area_id < 0 || area_id >= MAX_MASK_AREA_NUM) {
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
    char sQuery[272] = {0};
    int nColumnCnt = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select id,area_id,enable,start_x,start_y,area_width,area_height,width,height,fill_color from privacy_mask where id = %d and area_id = %d order by(area_id);",
             chn_id, area_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, &mask->id);
    sqlite_query_column(hStmt, 1, &mask->area[area_id].area_id);
    sqlite_query_column(hStmt, 2, &mask->area[area_id].enable);
    sqlite_query_column(hStmt, 3, &mask->area[area_id].start_x);
    sqlite_query_column(hStmt, 4, &mask->area[area_id].start_y);
    sqlite_query_column(hStmt, 5, &mask->area[area_id].area_width);
    sqlite_query_column(hStmt, 6, &mask->area[area_id].area_height);
    sqlite_query_column(hStmt, 7, &mask->area[area_id].width);
    sqlite_query_column(hStmt, 8, &mask->area[area_id].height);
    sqlite_query_column(hStmt, 9, &mask->area[area_id].fill_color);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_privacy_mask_by_areaid(const char *pDbFile, struct privacy_mask *mask, int area_id)
{
    if (!pDbFile || !mask) {
        return -1;
    }
    if (area_id < 0 || area_id >= MAX_MASK_AREA_NUM) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec),
             "update privacy_mask set enable=%d,start_x=%d,start_y=%d,area_width=%d,area_height=%d,width=%d,height=%d,fill_color=%d where id=%d and area_id=%d;",
             mask->area[area_id].enable, mask->area[area_id].start_x, mask->area[area_id].start_y, mask->area[area_id].area_width,
             mask->area[area_id].area_height, mask->area[area_id].width, mask->area[area_id].height, mask->area[area_id].fill_color,
             mask->id, mask->area[area_id].area_id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_motion_email_schedule(const char *pDbFile, struct motion_schedule  *motionSchedule, int chn_id)
{
    if (!pDbFile || !motionSchedule) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select week_id,plan_id,start_time,end_time,action_type from motion_email_schedule where chn_id=%d;", chn_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, motionSchedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(motionSchedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, motionSchedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(motionSchedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(motionSchedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_motion_email_schedule(const char *pDbFile, struct motion_schedule *schedule, int chn_id)
{
    if (!pDbFile || !schedule || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "delete from motion_email_schedule where chn_id=%d;", chn_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into motion_email_schedule values(%d,%d,%d, '%s', '%s', %d);", chn_id, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_motion_email_schedules(const char *pDbFile, struct motion_schedule *schedule, long long changeFlag)
{
    if (!pDbFile || !schedule) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[256] = {0};
    int i = 0, j = 0, k = 0;
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (k = 0; k < MAX_CAMERA; k++) {
        if (!(changeFlag >> k & 0x01)) {
            continue;
        }

        snprintf(sExec, sizeof(sExec), "delete from motion_email_schedule where chn_id=%d;", k);
        sqlite_execute(hConn, mode, sExec);

        for (i = 0; i < MAX_DAY_NUM; i++) {
            for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
                if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                               schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                    continue;
                }
                snprintf(sExec, sizeof(sExec), "insert into motion_email_schedule values(%d,%d,%d, '%s', '%s', %d);", k, i, j,
                         schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                         schedule->schedule_day[i].schedule_item[j].action_type);
                sqlite_execute(hConn, mode, sExec);
            }
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}


int read_motion_popup_schedule(const char *pDbFile, struct motion_schedule  *motionSchedule, int chn_id)
{
    if (!pDbFile || !motionSchedule) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select week_id,plan_id,start_time,end_time,action_type from motion_popup_schedule where chn_id=%d;", chn_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, motionSchedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(motionSchedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, motionSchedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(motionSchedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(motionSchedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_motion_popup_schedule(const char *pDbFile, struct motion_schedule *schedule, int chn_id)
{
    if (!pDbFile || !schedule || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "delete from motion_popup_schedule where chn_id=%d;", chn_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into motion_popup_schedule values(%d,%d,%d, '%s', '%s', %d);", chn_id, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_motion_ptz_schedule(const char *pDbFile, struct motion_schedule    *motionSchedule, int chn_id)
{
    if (!pDbFile || !motionSchedule) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select week_id,plan_id,start_time,end_time,action_type from motion_ptz_schedule where chn_id=%d;", chn_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, motionSchedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(motionSchedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, motionSchedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(motionSchedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(motionSchedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_motion_ptz_schedule(const char *pDbFile, struct motion_schedule *schedule, int chn_id)
{
    if (!pDbFile || !schedule || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "delete from motion_ptz_schedule where chn_id=%d;", chn_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into motion_ptz_schedule values(%d,%d,%d,'%s','%s',%d);", chn_id, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_motion_ptz_schedules(const char *pDbFile, struct motion_schedule *schedule, long long changeFlag)
{
    if (!pDbFile || !schedule) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[256] = {0};
    int i = 0, j = 0, k = 0;
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (k = 0; k < MAX_CAMERA; k++) {
        if (!(changeFlag >> k & 0x01)) {
            continue;
        }
        snprintf(sExec, sizeof(sExec), "delete from motion_ptz_schedule where chn_id=%d;", k);
        sqlite_execute(hConn, mode, sExec);
        for (i = 0; i < MAX_DAY_NUM; i++) {
            for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
                if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                               schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                    continue;
                }
                snprintf(sExec, sizeof(sExec), "insert into motion_ptz_schedule values(%d,%d,%d,'%s','%s',%d);", k, i, j,
                         schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                         schedule->schedule_day[i].schedule_item[j].action_type);
                sqlite_execute(hConn, mode, sExec);
            }
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);

    return 0;
}

int read_videoloss_email_schedule(const char *pDbFile, struct video_loss_schedule  *schedule, int chn_id)
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select week_id,plan_id,start_time,end_time,action_type from videoloss_email_schedule where chn_id=%d;", chn_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(schedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_videoloss_email_schedule(const char *pDbFile, struct video_loss_schedule  *schedule, int chn_id)
{
    if (!pDbFile || !schedule || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "delete from videoloss_email_schedule where chn_id=%d;", chn_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into videoloss_email_schedule values(%d,%d,%d, '%s', '%s', %d);", chn_id, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_videoloss_email_schedules(const char *pDbFile, struct video_loss_schedule  *schedule, long long changeFlag)
{
    if (!pDbFile || !schedule) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[256] = {0};
    int i = 0, j = 0, k = 0;

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (k = 0; k < MAX_CAMERA; k++) {
        if (!(changeFlag >> k & 0x01)) {
            continue;
        }
        snprintf(sExec, sizeof(sExec), "delete from videoloss_email_schedule where chn_id=%d;", k);
        sqlite_execute(hConn, mode, sExec);
        for (i = 0; i < MAX_DAY_NUM; i++) {
            for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
                if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                               schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                    continue;
                }
                snprintf(sExec, sizeof(sExec), "insert into videoloss_email_schedule values(%d,%d,%d, '%s', '%s', %d);", k, i, j,
                         schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                         schedule->schedule_day[i].schedule_item[j].action_type);
                sqlite_execute(hConn, mode, sExec);
            }
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}


int read_videoloss_popup_schedule(const char *pDbFile, struct video_loss_schedule  *schedule, int chn_id)
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
    char sQuery[512] = {0};
    int nColumnCnt = 0;
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select week_id,plan_id,start_time,end_time,action_type from videoloss_popup_schedule where chn_id=%d;", chn_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(schedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_videoloss_popup_schedule(const char *pDbFile, struct video_loss_schedule  *schedule, int chn_id)
{
    if (!pDbFile || !schedule || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "delete from videoloss_popup_schedule where chn_id=%d;", chn_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into videoloss_popup_schedule values(%d,%d,%d, '%s', '%s', %d);", chn_id, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_videoloss_popup_schedules(const char *pDbFile, struct video_loss_schedule  *schedule, long long changeFlag)
{
    if (!pDbFile || !schedule) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[256] = {0};
    int i = 0, j = 0, k = 0;

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (k = 0; k < MAX_CAMERA; k++) {
        if (!(changeFlag >> k & 0x01)) {
            continue;
        }
        snprintf(sExec, sizeof(sExec), "delete from videoloss_popup_schedule where chn_id=%d;", k);
        sqlite_execute(hConn, mode, sExec);
        for (i = 0; i < MAX_DAY_NUM; i++) {
            for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
                if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                               schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                    continue;
                }
                snprintf(sExec, sizeof(sExec), "insert into videoloss_popup_schedule values(%d,%d,%d, '%s', '%s', %d);", k, i, j,
                         schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                         schedule->schedule_day[i].schedule_item[j].action_type);
                sqlite_execute(hConn, mode, sExec);
            }
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_alarmin_email_schedule(const char *pDbFile, struct alarm_in_schedule *schedule, int chn_id)
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select week_id,plan_id,start_time,end_time,action_type from alarmin_email_schedule where chn_id=%d;", chn_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(schedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_alarmin_email_schedule(const char *pDbFile, struct alarm_in_schedule *schedule, int chn_id)
{
    if (!pDbFile || !schedule || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "delete from alarmin_email_schedule where chn_id=%d;", chn_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into alarmin_email_schedule values(%d,%d,%d, '%s', '%s', %d);", chn_id, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_alarmin_email_schedules(const char *pDbFile, struct alarm_in_schedule *schedule, long long changeFlag)
{
    if (!pDbFile || !schedule) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    char sExec[256] = {0};
    int i = 0, j = 0, k = 0;
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (k = 0; k < MAX_CAMERA; k++) {
        if (!(changeFlag >> k & 0x01)) {
            continue;
        }
        snprintf(sExec, sizeof(sExec), "delete from alarmin_email_schedule where chn_id=%d;", k);
        sqlite_execute(hConn, mode, sExec);
        for (i = 0; i < MAX_DAY_NUM; i++) {
            for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
                if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                               schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                    continue;
                }
                snprintf(sExec, sizeof(sExec), "insert into alarmin_email_schedule values(%d,%d,%d, '%s', '%s', %d);", k, i, j,
                         schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                         schedule->schedule_day[i].schedule_item[j].action_type);
                sqlite_execute(hConn, mode, sExec);
            }
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_alarmin_popup_schedule(const char *pDbFile, struct alarm_in_schedule *schedule, int chn_id)
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select week_id,plan_id,start_time,end_time,action_type from alarmin_popup_schedule where chn_id=%d;", chn_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(schedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_alarmin_popup_schedule(const char *pDbFile, struct alarm_in_schedule *schedule, int chn_id)
{
    if (!pDbFile || !schedule || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "delete from alarmin_popup_schedule where chn_id=%d;", chn_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into alarmin_popup_schedule values(%d,%d,%d, '%s', '%s', %d);", chn_id, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_alarmin_popup_schedules(const char *pDbFile, struct alarm_in_schedule *schedule, long long changeFlag)
{
    if (!pDbFile || !schedule) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    char sExec[256] = {0};
    int i = 0, j = 0, k = 0;
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (k = 0; k < MAX_CAMERA; k++) {
        if (!(changeFlag >> k & 0x01)) {
            continue;
        }
        snprintf(sExec, sizeof(sExec), "delete from alarmin_popup_schedule where chn_id=%d;", k);
        sqlite_execute(hConn, mode, sExec);
        for (i = 0; i < MAX_DAY_NUM; i++) {
            for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
                if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                               schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                    continue;
                }
                snprintf(sExec, sizeof(sExec), "insert into alarmin_popup_schedule values(%d,%d,%d, '%s', '%s', %d);", k, i, j,
                         schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                         schedule->schedule_day[i].schedule_item[j].action_type);
                sqlite_execute(hConn, mode, sExec);
            }
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

//modify for action PZT preset & patrol x.7.0.9
int read_alarm_in_ex(const char *pDbFile, struct alarm_in *alarm, int alarm_id)
{
    if (!pDbFile || !alarm || alarm_id < 0) {
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
    snprintf(sQuery, sizeof(sQuery), "select id, enable,name,type,tri_channels,tri_alarms,buzzer_interval,tri_channels_ex,\
		acto_ptz_channel,acto_ptz_preset_enable,acto_ptz_preset,acto_ptz_patrol_enable,acto_ptz_patrol,email_enable,\
		email_buzzer_interval,email_pic_enable,tri_channels_pic,tri_audio_id from alarm_in where id=%d;", alarm_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &alarm->id);
    sqlite_query_column(hStmt, 1, &alarm->enable);
    sqlite_query_column_text(hStmt, 2, alarm->name, sizeof(alarm->name));
    sqlite_query_column(hStmt, 3, &alarm->type);
    sqlite_query_column(hStmt, 4, (int *)&alarm->tri_channels);
    sqlite_query_column(hStmt, 5, (int *)&alarm->tri_alarms);
    sqlite_query_column(hStmt, 6, &alarm->buzzer_interval);
    sqlite_query_column_text(hStmt, 7, alarm->tri_channels_ex, sizeof(alarm->tri_channels_ex));
    sqlite_query_column(hStmt, 8, &alarm->acto_ptz_channel);
    sqlite_query_column(hStmt, 9, &alarm->acto_ptz_preset_enable);
    sqlite_query_column(hStmt, 10, &alarm->acto_ptz_preset);
    sqlite_query_column(hStmt, 11, &alarm->acto_ptz_patrol_enable);
    sqlite_query_column(hStmt, 12, &alarm->acto_ptz_patrol);
    sqlite_query_column(hStmt, 13, &alarm->email_enable);
    sqlite_query_column(hStmt, 14, &alarm->email_buzzer_interval);
    sqlite_query_column(hStmt, 15, &alarm->email_pic_enable);
    sqlite_query_column_text(hStmt, 16, alarm->tri_channels_pic, sizeof(alarm->tri_channels_pic));
    sqlite_query_column(hStmt, 17, &alarm->tri_audio_id);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_alarm_ins_ex(const char *pDbFile, struct alarm_in alarms[], int *pCnt)
{
    if (!pDbFile || !alarms) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery), "select id,enable,name,type,tri_channels,tri_alarms,buzzer_interval,\
		tri_channels_ex,acto_ptz_channel,acto_ptz_preset_enable,acto_ptz_preset,acto_ptz_patrol_enable,acto_ptz_patrol,\
		email_enable,email_buzzer_interval,email_pic_enable,tri_channels_pic,tri_audio_id from alarm_in;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *pCnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < MAX_ALARM_IN; i++) {
        sqlite_query_column(hStmt, 0, &alarms[i].id);
        sqlite_query_column(hStmt, 1, &alarms[i].enable);
        sqlite_query_column_text(hStmt, 2, alarms[i].name, sizeof(alarms[i].name));
        sqlite_query_column(hStmt, 3, &alarms[i].type);
        sqlite_query_column(hStmt, 4, (int *)&alarms[i].tri_channels);
        sqlite_query_column(hStmt, 5, (int *)&alarms[i].tri_alarms);
        sqlite_query_column(hStmt, 6, &alarms[i].buzzer_interval);
        sqlite_query_column_text(hStmt, 7, alarms[i].tri_channels_ex, sizeof(alarms[i].tri_channels_ex));
        sqlite_query_column(hStmt, 8, &alarms[i].acto_ptz_channel);
        sqlite_query_column(hStmt, 9, &alarms[i].acto_ptz_preset_enable);
        sqlite_query_column(hStmt, 10, &alarms[i].acto_ptz_preset);
        sqlite_query_column(hStmt, 11, &alarms[i].acto_ptz_patrol_enable);
        sqlite_query_column(hStmt, 12, &alarms[i].acto_ptz_patrol);
        sqlite_query_column(hStmt, 13, &alarms[i].email_enable);
        sqlite_query_column(hStmt, 14, &alarms[i].email_buzzer_interval);
        sqlite_query_column(hStmt, 15, &alarms[i].email_pic_enable);
        sqlite_query_column_text(hStmt, 16, alarms[i].tri_channels_pic, sizeof(alarms[i].tri_channels_pic));
        sqlite_query_column(hStmt, 17, &alarms[i].tri_audio_id);

        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    *pCnt = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_alarm_in_ex(const char *pDbFile, struct alarm_in *alarm)
{
    if (!pDbFile || !alarm) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[2048] = {0};
    snprintf(sExec, sizeof(sExec), "update alarm_in set enable=%d,name='%s',type=%d, tri_channels=%u,tri_alarms=%u,\
		buzzer_interval =%d,tri_channels_ex='%s',acto_ptz_channel=%d,acto_ptz_preset_enable=%d,acto_ptz_preset=%d,\
		acto_ptz_patrol_enable=%d,acto_ptz_patrol=%d,email_enable=%d,email_buzzer_interval=%d,email_pic_enable=%d,\
		tri_channels_pic='%s',tri_audio_id=%d where id=%d;",
             alarm->enable, alarm->name, alarm->type, alarm->tri_channels, alarm->tri_alarms, alarm->buzzer_interval,
             alarm->tri_channels_ex, alarm->acto_ptz_channel, alarm->acto_ptz_preset_enable, alarm->acto_ptz_preset,
             alarm->acto_ptz_patrol_enable, alarm->acto_ptz_patrol, alarm->email_enable, alarm->email_buzzer_interval,
             alarm->email_pic_enable, alarm->tri_channels_pic, alarm->tri_audio_id, alarm->id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}
int write_ssl_params(const char *pDbFile, struct cert_info *cert)
{
    if (!pDbFile || !cert) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[1024] = {0};
    char buf[256] = {0};

    translate_pwd(buf, cert->common_name, strlen(cert->common_name));
    if (cert->type == PRIVATE_CERT) {
        snprintf(sExec, sizeof(sExec), "update https set country='%s',common_name='%s',email='%s',dates='%s';",
                 cert->country, buf, cert->email, cert->dates);
    } else if (cert->type == CERT_REQ) {
        snprintf(sExec, sizeof(sExec), "update https set country_req='%s',common_name_req='%s',email_req='%s';",
                 cert->country, buf, cert->email);
    }

    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}
int read_ssl_params(const char *pDbFile, struct cert_info *cert, int type)
{
    if (!pDbFile || !cert) {
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
    char sQuery[512] = {0};
    int nColumnCnt = 0;
    if (type == PRIVATE_CERT) {
        snprintf(sQuery, sizeof(sQuery), "select country,common_name,email,dates from https;");
        int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            sqlite_disconn(hConn);
            FileUnlock(nFd, mode, &global_rwlock);
            return -1;
        }
        sqlite_query_column_text(hStmt, 0, cert->country, sizeof(cert->country));
        sqlite_query_column_text(hStmt, 1, cert->common_name, sizeof(cert->common_name));
        sqlite_query_column_text(hStmt, 2, cert->email, sizeof(cert->email));
        sqlite_query_column_text(hStmt, 3, cert->dates, sizeof(cert->dates));
    } else if (type == CERT_REQ) {
        snprintf(sQuery, sizeof(sQuery), "select country_req,common_name_req,email_req from https;");
        int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            sqlite_disconn(hConn);
            FileUnlock(nFd, mode, &global_rwlock);
            return -1;
        }
        sqlite_query_column_text(hStmt, 0, cert->country, sizeof(cert->country));
        sqlite_query_column_text(hStmt, 1, cert->common_name, sizeof(cert->common_name));
        sqlite_query_column_text(hStmt, 2, cert->email, sizeof(cert->email));
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}
int write_encrypted_list(const char *pDbFile, struct squestion *sqa, int sorder)
{
    if (!pDbFile || !sqa) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[2048] = {0};
    char buf[MAX_LEN_64 * MAX_SQA_CNT * 2] = {0};
    char answer[MAX_LEN_64 * MAX_SQA_CNT * 2] = {0};
    char question[MAX_LEN_64 * MAX_SQA_CNT * 2] = {0};

    translate_pwd(buf, sqa->answer, strlen(sqa->answer));
    memcpy(answer, buf, MAX_LEN_64 * MAX_SQA_CNT * 2);
    memset(buf, 0, MAX_LEN_64 * MAX_SQA_CNT * 2);

    if (sqa->sqtype == 12) {
        translate_pwd(buf, sqa->squestion, strlen(sqa->squestion));
        memcpy(question, buf, MAX_LEN_64 * MAX_SQA_CNT * 2);
        snprintf(sExec, sizeof(sExec),
                 "update encrypted_list set sqType='%d',squestion='%s',answer='%s',enable='1' where id='%d';", sqa->sqtype, question,
                 answer, sorder);
    } else {
        snprintf(sExec, sizeof(sExec), "update encrypted_list set sqType='%d',answer='%s',enable='1' where id='%d';",
                 sqa->sqtype, answer, sorder);
    }

    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}
int read_encrypted_list(const char *pDbFile, struct squestion sqas[])
{
    if (!pDbFile || !sqas) {
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
    char sQuery[512] = {0};
    int nColumnCnt = 0;
    int i = 0;

    snprintf(sQuery, sizeof(sQuery), "select id,sqtype,squestion,answer,enable from encrypted_list;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < MAX_SQA_CNT; i++) {
        sqlite_query_column(hStmt, 1, &sqas[i].sqtype);
        sqlite_query_column_text(hStmt, 2, sqas[i].squestion, sizeof(sqas[i].squestion));
        sqlite_query_column_text(hStmt, 3, sqas[i].answer, sizeof(sqas[i].answer));
        sqlite_query_column(hStmt, 4, &sqas[i].enable);

        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int check_table_key(const char *pDbFile, char *table, const char *key)
{
    if (!pDbFile || !key) {
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
    snprintf(sQuery, sizeof(sQuery), "select %s from %s where id=0;", key, table);
    int nColumnCnt = 0;
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);

    return 0;
}

/*
* return 0:success
*       -1:failed
*
* pCnt:查询前pCnt个字段，返回实际查询到的字段数
*/
int get_table_fields(const char *pDbFile, char *table, int *pCnt, char (*filed_name)[MAX_LEN_32])
{
    if (!pDbFile || !table || *pCnt < 1) {
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
    snprintf(sQuery, sizeof(sQuery), "PRAGMA table_info('%s')", table);
    int nColumnCnt = 0;
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *pCnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int i = 0;
    for (i = 0; i < *pCnt; i++) {
        sqlite_query_column_text(hStmt, 1, filed_name[i], MAX_LEN_32);
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    *pCnt = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);

    return 0;
}

/*
* return -1:读取表头失败
*         0:所查字段不存在
*         1:所查字段存在
*/
int check_table_key_ex(const char *pDbFile, char *table, const char *key)
{
    int cnt = 100;//获取前cnt个字段
    char (*filed_name)[MAX_LEN_32] = (char (*)[MAX_LEN_32])ms_malloc(cnt * MAX_LEN_32 * sizeof(char));
    int ret = get_table_fields(pDbFile, table, &cnt, filed_name);
    if (ret < 0) {
        ms_free(filed_name);
        return -1;
    }

    int i = 0;
    for (i = 0; i < cnt; i++) {
        if (!strcasecmp(filed_name[i], key)) {
            ms_free(filed_name);
            return 1;
        }
    }
    ms_free(filed_name);
    return 0;
}

int read_disks(const char *pDbFile, struct diskInfo disks[], int *pCnt)
{
    if (!pDbFile || !disks) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select disk_port,enable,disk_vendor,disk_address,disk_directory,disk_type,disk_group,disk_property,raid_level,user,password from disk;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *pCnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < MAX_MSDK_NUM; i++) {
        sqlite_query_column(hStmt, 0, &disks[i].disk_port);
        sqlite_query_column(hStmt, 1, &disks[i].enable);
        sqlite_query_column_text(hStmt, 2, disks[i].disk_vendor, sizeof(disks[i].disk_vendor));
        sqlite_query_column_text(hStmt, 3, disks[i].disk_address, sizeof(disks[i].disk_address));
        sqlite_query_column_text(hStmt, 4, disks[i].disk_directory, sizeof(disks[i].disk_directory));
        sqlite_query_column(hStmt, 5, &disks[i].disk_type);
        sqlite_query_column(hStmt, 6, &disks[i].disk_group);
        sqlite_query_column(hStmt, 7, &disks[i].disk_property);
        sqlite_query_column(hStmt, 8, &disks[i].raid_level);
        sqlite_query_column_text(hStmt, 9, disks[i].user, sizeof(disks[i].user));
        sqlite_query_column_text(hStmt, 10, disks[i].password, sizeof(disks[i].password));

        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    *pCnt = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_disk(const char *pDbFile, struct diskInfo *disk, int portId)
{
    if (!pDbFile || !disk) {
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
    snprintf(sQuery, sizeof(sQuery),
             "select disk_port,enable,disk_vendor,disk_address,disk_directory,disk_type,disk_group,disk_property,raid_level, user, password from disk where disk_port=%d;",
             portId);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, &disk->disk_port);
    sqlite_query_column(hStmt, 1, &disk->enable);
    sqlite_query_column_text(hStmt, 2, disk->disk_vendor, sizeof(disk->disk_vendor));
    sqlite_query_column_text(hStmt, 3, disk->disk_address, sizeof(disk->disk_address));
    sqlite_query_column_text(hStmt, 4, disk->disk_directory, sizeof(disk->disk_directory));
    sqlite_query_column(hStmt, 5, &disk->disk_type);
    sqlite_query_column(hStmt, 6, &disk->disk_group);
    sqlite_query_column(hStmt, 7, &disk->disk_property);
    sqlite_query_column(hStmt, 8, &disk->raid_level);
    sqlite_query_column_text(hStmt, 9, disk->user, sizeof(disk->user));
    sqlite_query_column_text(hStmt, 10, disk->password, sizeof(disk->password));

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_disk(const char *pDbFile, struct diskInfo *disk)
{
    if (!pDbFile || !disk) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[1024] = {0};
    char user[128] = {0};
    char pwd[128] = {0};
    char vendor[MAX_LEN_256] = {0};
    translate_pwd(user, disk->user, strlen(disk->user));
    translate_pwd(pwd, disk->password, strlen(disk->password));
    translate_pwd(vendor, disk->disk_vendor, strlen(disk->disk_vendor));
    snprintf(sExec, sizeof(sExec),
             "update disk set disk_port=%d,enable=%d,disk_vendor='%s',disk_address='%s',disk_directory='%s',disk_type=%d,disk_group=%d,disk_property=%d,raid_level=%d,user='%s',password='%s' where disk_port=%d;",
             disk->disk_port, disk->enable, vendor, disk->disk_address, disk->disk_directory, disk->disk_type,
             disk->disk_group, disk->disk_property, disk->raid_level, user, pwd, disk->disk_port);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int insert_disk(const char *pDbFile, struct diskInfo *diskInfo)
{
    char user[128] = {0};
    char pwd[128] = {0};
    char vendor[MAX_LEN_256] = {0};
    if (!pDbFile || !diskInfo) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    char sExec[1024] = {0};
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    translate_pwd(user, diskInfo->user, strlen(diskInfo->user));
    translate_pwd(pwd, diskInfo->password, strlen(diskInfo->password));
    translate_pwd(vendor, diskInfo->disk_vendor, strlen(diskInfo->disk_vendor));
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    snprintf(sExec, sizeof(sExec),
             "insert into disk(disk_port,enable,disk_vendor,disk_address,disk_directory,disk_type,disk_group,disk_property,raid_level,user,password) values(%d,%d,%s,%s,%s,%d,%d,%d,%d,%s,%s);",
             diskInfo->disk_port,
             diskInfo->enable, vendor, diskInfo->disk_address, diskInfo->disk_directory, diskInfo->disk_type,
             diskInfo->disk_group, diskInfo->disk_property, diskInfo->raid_level, user, pwd);

    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);

    return 0;
}

int read_groups(const char *pDbFile, struct groupInfo groups[], int *pCnt)
{
    if (!pDbFile || !groups) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery), "select id,chnMaskl,chnMaskh from diskGroup;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *pCnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < MAX_MSDK_GROUP_NUM; i++) {
        sqlite_query_column(hStmt, 0, &groups[i].groupid);
        sqlite_query_column_text(hStmt, 1, groups[i].chnMaskl, sizeof(groups[i].chnMaskl));
        sqlite_query_column_text(hStmt, 2, groups[i].chnMaskh, sizeof(groups[i].chnMaskh));

        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    *pCnt = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_group_st(const char *pDbFile, struct groupInfo *group)
{
    if (!pDbFile || !group) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec), "update diskGroup set id=%d,chnMaskl='%s',chnMaskh='%s' where id=%d;",
             group->groupid, group->chnMaskl, group->chnMaskh, group->groupid);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_quota(const char *pDbFile, quota_info_t quotas[], int *pCnt)
{
    if (!pDbFile || !quotas) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery), "select chnid, video, picture from quota;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *pCnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int tmp = 0;
    for (i = 0; i < *pCnt; i++) {
        sqlite_query_column(hStmt, 0, &tmp);
        quotas[i].chnId = (MF_S8)tmp;
        sqlite_query_column(hStmt, 1, (int *)&quotas[i].vidQta);
        sqlite_query_column(hStmt, 2, (int *)&quotas[i].picQta);

        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    *pCnt = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_quota(const char *pDbFile, quota_info_t *quota)
{
    if (!pDbFile || !quota) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int tmp = (int)quota->chnId;
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec), "update quota set chnid=%d, video=%d, picture=%d where chnid=%d;",
             tmp, quota->vidQta, quota->picQta, (int)quota->chnId);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

static int get_smart_effective_table_name(SMART_EVENT_TYPE type, char *table, int size)
{
    if (!table || size <= 0) {
        return -1;
    }

    int ret = 0;
    if (type == REGIONIN) {
        snprintf(table, size, "%s", "vca_regionen_effective_schedule");
    } else if (type == REGIONOUT) {
        snprintf(table, size, "%s", "vca_regionex_effective_schedule");
    } else if (type == ADVANCED_MOTION) {
        snprintf(table, size, "%s", "vca_motionadv_effective_schedule");
    } else if (type == TAMPER) {
        snprintf(table, size, "%s", "vca_tamper_effective_schedule");
    } else if (type == LINECROSS) {
        snprintf(table, size, "%s", "vca_linecross_effective_schedule");
    } else if (type == LOITERING) {
        snprintf(table, size, "%s", "vca_loiter_effective_schedule");
    } else if (type == HUMAN) {
        snprintf(table, size, "%s", "vca_human_effective_schedule");
    } else if (type == PEOPLE_CNT) {
        snprintf(table, size, "%s", "vca_peolpe_effective_schedule");
    } else if (type == OBJECT_LEFTREMOVE) {
        snprintf(table, size, "%s", "vca_object_leftremove_effective_schedule");
    } else {
        ret = -1;
    }

    return ret;
}

int read_smart_event_effective_schedule(const char *pDbFile, struct smart_event_schedule  *Schedule, int chn_id,
                                        SMART_EVENT_TYPE type)
{
    if (!pDbFile || !Schedule) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = -1;
    HSQLITE hConn = 0;

    int i = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[256] = {0};

    HSQLSTMT hStmt = 0;
    char table[MAX_LEN_64] = {0};

    if (!get_smart_effective_table_name(type, table, sizeof(table))) {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from %s where chn_id=%d;", table, chn_id);
    } else {
        return -1;
    }

    nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, Schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, Schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_smart_event_effective_schedule(const char *pDbFile, struct smart_event_schedule *schedule, int chn_id,
                                         SMART_EVENT_TYPE type)
{
    if (!pDbFile || !schedule || chn_id < 0) {
        return -1;
    }
    int nFd = -1;
    int mode = FILE_MODE_WR;
    HSQLITE hConn = 0;
    char sExec[256] = {0};
    char db_table[64] = {0};

    if (get_smart_effective_table_name(type, db_table, sizeof(db_table))) {
        return -1;
    }
    nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    snprintf(sExec, sizeof(sExec), "delete from %s where chn_id=%d;", db_table, chn_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into %s values(%d,%d,%d, '%s', '%s', %d);", db_table, chn_id, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_smart_event_effective_schedules(const char *pDbFile, SMART_SCHEDULE *schedule,
                                                    Uint64 chnMask, SMART_EVENT_TYPE type)
{
    char table[MAX_LEN_64] = {0};
    if (chnMask && !get_smart_effective_table_name(type, table, sizeof(table))) {
        return copy_schedules(pDbFile, schedule, table, chnMask);
    }

    return -1;
}

static int get_smart_audible_table_name(SMART_EVENT_TYPE type, char *table, int size)
{
    if (!table || size <= 0) {
        return -1;
    }

    int ret = 0;
    if (type == REGIONIN) {
        snprintf(table, size, "%s", "vca_regionen_audible_schedule");
    } else if (type == REGIONOUT) {
        snprintf(table, size, "%s", "vca_regionex_audible_schedule");
    } else if (type == ADVANCED_MOTION) {
        snprintf(table, size, "%s", "vca_motionadv_audible_schedule");
    } else if (type == TAMPER) {
        snprintf(table, size, "%s", "vca_tamper_audible_schedule");
    } else if (type == LINECROSS) {
        snprintf(table, size, "%s", "vca_linecross_audible_schedule");
    } else if (type == LOITERING) {
        snprintf(table, size, "%s", "vca_loiter_audible_schedule");
    } else if (type == HUMAN) {
        snprintf(table, size, "%s", "vca_human_audible_schedule");
    } else if (type == PEOPLE_CNT) {
        snprintf(table, size, "%s", "vca_peolpe_audible_schedule");
    } else if (type == OBJECT_LEFTREMOVE) {
        snprintf(table, size, "%s", "vca_object_leftremove_audible_schedule");
    } else {
        ret = -1;
    }

    return ret;
}

int read_smart_event_audible_schedule(const char *pDbFile, struct smart_event_schedule  *Schedule, int chn_id,
                                      SMART_EVENT_TYPE type)
{
    if (!pDbFile || !Schedule) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = -1;
    HSQLITE hConn = 0;

    int i = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[256] = {0};

    HSQLSTMT hStmt = 0;
    char table[MAX_LEN_64] = {0};

    if (!get_smart_audible_table_name(type, table, sizeof(table))) {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from %s where chn_id=%d;", table, chn_id);
    } else {
        return -1;
    }

    nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, Schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, Schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_smart_event_audible_schedule(const char *pDbFile, struct smart_event_schedule *schedule, int chn_id,
                                       SMART_EVENT_TYPE type)
{
    if (!pDbFile || !schedule || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = -1;
    HSQLITE hConn = 0;

    char sExec[256] = {0};
    char db_table[64] = {0};

    if (get_smart_audible_table_name(type, db_table, sizeof(db_table))) {
        return -1;
    }
    nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    snprintf(sExec, sizeof(sExec), "delete from %s where chn_id=%d;", db_table, chn_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into %s values(%d,%d,%d, '%s', '%s', %d);", db_table, chn_id, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_smart_event_audible_schedules(const char *pDbFile, SMART_SCHEDULE *schedule,
                                                    Uint64 chnMask, SMART_EVENT_TYPE type)
{
    char table[MAX_LEN_64] = {0};
    if (chnMask && !get_smart_audible_table_name(type, table, sizeof(table))) {
        return copy_schedules(pDbFile, schedule, table, chnMask);
    }

    return -1;
}

static int get_smart_mail_table_name(SMART_EVENT_TYPE type, char *table, int size)
{
    if (!table || size <= 0) {
        return -1;
    }

    int ret = 0;
    if (type == REGIONIN) {
        snprintf(table, size, "%s", "vca_regionen_mail_schedule");
    } else if (type == REGIONOUT) {
        snprintf(table, size, "%s", "vca_regionex_mail_schedule");
    } else if (type == ADVANCED_MOTION) {
        snprintf(table, size, "%s", "vca_motionadv_mail_schedule");
    } else if (type == TAMPER) {
        snprintf(table, size, "%s", "vca_tamper_mail_schedule");
    } else if (type == LINECROSS) {
        snprintf(table, size, "%s", "vca_linecross_mail_schedule");
    } else if (type == LOITERING) {
        snprintf(table, size, "%s", "vca_loiter_mail_schedule");
    } else if (type == HUMAN) {
        snprintf(table, size, "%s", "vca_human_mail_schedule");
    } else if (type == PEOPLE_CNT) {
        snprintf(table, size, "%s", "vca_peolpe_mail_schedule");
    } else if (type == OBJECT_LEFTREMOVE) {
        snprintf(table, size, "%s", "vca_object_leftremove_mail_schedule");
    } else {
        ret = -1;
    }

    return ret;
}

int read_smart_event_mail_schedule(const char *pDbFile, struct smart_event_schedule  *Schedule, int chn_id,
                                   SMART_EVENT_TYPE type)
{
    if (!pDbFile || !Schedule) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = -1;
    HSQLITE hConn = 0;

    int i = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[256] = {0};

    HSQLSTMT hStmt = 0;
    char table[MAX_LEN_64] = {0};

    if (!get_smart_mail_table_name(type, table, sizeof(table))) {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from %s where chn_id=%d;", table, chn_id);
    } else {
        return -1;
    }

    nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, Schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, Schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_smart_event_mail_schedule(const char *pDbFile, struct smart_event_schedule *schedule, int chn_id,
                                    SMART_EVENT_TYPE type)
{
    if (!pDbFile || !schedule || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = -1;
    HSQLITE hConn = 0;

    char sExec[256] = {0};
    char db_table[64] = {0};
    if (get_smart_mail_table_name(type, db_table, sizeof(db_table))) {
        return -1;
    }
    nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    snprintf(sExec, sizeof(sExec), "delete from %s where chn_id=%d;", db_table, chn_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into %s values(%d,%d,%d, '%s', '%s', %d);", db_table, chn_id, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_smart_event_mail_schedules(const char *pDbFile, SMART_SCHEDULE *schedule,
                                                    Uint64 chnMask, SMART_EVENT_TYPE type)
{
    char table[MAX_LEN_64] = {0};
    if (chnMask && !get_smart_mail_table_name(type, table, sizeof(table))) {
        return copy_schedules(pDbFile, schedule, table, chnMask);
    }

    return -1;
}

static int get_smart_popup_table_name(SMART_EVENT_TYPE type, char *table, int size)
{
    if (!table || size <= 0) {
        return -1;
    }

    int ret = 0;
    if (type == REGIONIN) {
        snprintf(table, size, "%s", "vca_regionin_popup_schedule");
    } else if (type == REGIONOUT) {
        snprintf(table, size, "%s", "vca_regionexit_popup_schedule");
    } else if (type == ADVANCED_MOTION) {
        snprintf(table, size, "%s", "vca_motion_popup_schedule");
    } else if (type == TAMPER) {
        snprintf(table, size, "%s", "vca_tamper_popup_schedule");
    } else if (type == LINECROSS) {
        snprintf(table, size, "%s", "vca_linecross_popup_schedule");
    } else if (type == LOITERING) {
        snprintf(table, size, "%s", "vca_loiter_popup_schedule");
    } else if (type == HUMAN) {
        snprintf(table, size, "%s", "vca_human_popup_schedule");
    } else if (type == PEOPLE_CNT) {
        snprintf(table, size, "%s", "vca_people_popup_schedule");
    } else if (type == OBJECT_LEFTREMOVE) {
        snprintf(table, size, "%s", "vca_object_leftremove_popup_schedule");
    } else {
        ret = -1;
    }

    return ret;
}

int read_smart_event_popup_schedule(const char *pDbFile, struct smart_event_schedule  *Schedule, int chn_id,
                                    SMART_EVENT_TYPE type)
{
    if (!pDbFile || !Schedule) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = -1;
    HSQLITE hConn = 0;

    int i = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[256] = {0};

    HSQLSTMT hStmt = 0;
    char table[MAX_LEN_64] = {0};

    if (!get_smart_popup_table_name(type, table, sizeof(table))) {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from %s where chn_id=%d;", table, chn_id);
    } else {
        return -1;
    }

    nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, Schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, Schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_smart_event_popup_schedule(const char *pDbFile, struct smart_event_schedule *schedule, int chn_id,
                                     SMART_EVENT_TYPE type)
{
    if (!pDbFile || !schedule || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = -1;
    HSQLITE hConn = 0;

    char sExec[256] = {0};
    char db_table[64] = {0};
    if (get_smart_popup_table_name(type, db_table, sizeof(db_table))) {
        return -1;
    }
    nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    snprintf(sExec, sizeof(sExec), "delete from %s where chn_id=%d;", db_table, chn_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into %s values(%d,%d,%d, '%s', '%s', %d);", db_table, chn_id, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_smart_event_popup_schedules(const char *pDbFile, SMART_SCHEDULE *schedule,
                                                    Uint64 chnMask, SMART_EVENT_TYPE type)
{
    char table[MAX_LEN_64] = {0};
    if (chnMask && !get_smart_popup_table_name(type, table, sizeof(table))) {
        return copy_schedules(pDbFile, schedule, table, chnMask);
    }

    return -1;
}

static int get_smart_event_table_name(SMART_EVENT_TYPE type, char *table, int size)
{
    if (!table || size <= 0) {
        return -1;
    }

    int ret = 0;
    if (type == REGIONIN) {
        snprintf(table, size, "%s", "vca_regionin_event");
    } else if (type == REGIONOUT) {
        snprintf(table, size, "%s", "vca_regionexit_event");
    } else if (type == ADVANCED_MOTION) {
        snprintf(table, size, "%s", "vca_motion_event");
    } else if (type == TAMPER) {
        snprintf(table, size, "%s", "vca_tamper_event");
    } else if (type == LINECROSS) {
        snprintf(table, size, "%s", "vca_linecross_event");
    } else if (type == LOITERING) {
        snprintf(table, size, "%s", "vca_loiter_event");
    } else if (type == HUMAN) {
        snprintf(table, size, "%s", "vca_human_event");
    } else if (type == PEOPLE_CNT) {
        snprintf(table, size, "%s", "vca_people_event");
    } else if (type == OBJECT_LEFTREMOVE) {
        snprintf(table, size, "%s", "vca_object_leftremove_event");
    } else {
        ret = -1;
    }

    return ret;
}

int read_smart_event(const char *pDbFile, struct smart_event *smartevent, int id, SMART_EVENT_TYPE type)
{
    if (!pDbFile || !smartevent) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = -1;
    HSQLITE hConn = 0;
    HSQLSTMT hStmt = 0;
    char sQuery[2048] = {0};
    int nColumnCnt = 0;
    char table[MAX_LEN_64] = {0};

    if (!get_smart_event_table_name(type, table, sizeof(table))) {
        snprintf(sQuery, sizeof(sQuery), "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,\
            email_interval,tri_chnout1_alarms,tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,\
            whiteled_interval,email_pic_enable,tri_channels_pic,tri_audio_id,http_notification_interval \
            from %s where id=%d;", table, id);
    } else {
        return -1;
    }

    nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, &smartevent->id);
    sqlite_query_column(hStmt, 1, &smartevent->enable);
    sqlite_query_column(hStmt, 2, (int *)&smartevent->tri_alarms);
    sqlite_query_column_text(hStmt, 3, smartevent->tri_channels_ex, sizeof(smartevent->tri_channels_ex));
    sqlite_query_column(hStmt, 4, &smartevent->buzzer_interval);
    sqlite_query_column(hStmt, 5, &smartevent->email_interval);
    sqlite_query_column_text(hStmt, 6, smartevent->tri_chnout1_alarms, sizeof(smartevent->tri_chnout1_alarms));
    sqlite_query_column_text(hStmt, 7, smartevent->tri_chnout2_alarms, sizeof(smartevent->tri_chnout2_alarms));
    sqlite_query_column(hStmt, 8, &smartevent->popup_interval);
    sqlite_query_column(hStmt, 9, &smartevent->ptzaction_interval);
    sqlite_query_column(hStmt, 10, &smartevent->alarmout_interval);
    sqlite_query_column(hStmt, 11, &smartevent->whiteled_interval);
    sqlite_query_column(hStmt, 12, &smartevent->email_pic_enable);
    sqlite_query_column_text(hStmt, 13, smartevent->tri_channels_pic, sizeof(smartevent->tri_channels_pic));
    sqlite_query_column(hStmt, 14, &smartevent->tri_audio_id);
    sqlite_query_column(hStmt, 15, &smartevent->http_notification_interval);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_smart_event(const char *pDbFile, struct smart_event *smartevent, SMART_EVENT_TYPE type)
{
    if (!pDbFile || !smartevent) {
        return -1;
    }
    int nFd = -1;
    HSQLITE hConn = 0;
    int mode = FILE_MODE_WR;
    char sExec[2048] = {0};
    char table[MAX_LEN_64] = {0};

    if (!get_smart_event_table_name(type, table, sizeof(table))) {
        snprintf(sExec, sizeof(sExec), "update %s set enable=%d, tri_alarms=%d, buzzer_interval='%d', \
            email_interval='%d', tri_chnout1_alarms='%s', tri_chnout2_alarms='%s', popup_interval=%d, \
            ptzaction_interval=%d, alarmout_interval=%d, whiteled_interval=%d, email_pic_enable=%d, \
            tri_audio_id=%d, tri_channels_ex='%s', tri_channels_pic='%s', http_notification_interval=%d where id=%d;",
            table, smartevent->enable, smartevent->tri_alarms, smartevent->buzzer_interval, smartevent->email_interval,
            smartevent->tri_chnout1_alarms, smartevent->tri_chnout2_alarms, smartevent->popup_interval,
            smartevent->ptzaction_interval, smartevent->alarmout_interval, smartevent->whiteled_interval,
            smartevent->email_pic_enable, smartevent->tri_audio_id, smartevent->tri_channels_ex,
            smartevent->tri_channels_pic, smartevent->http_notification_interval, smartevent->id);
    } else {
        return -1;
    }

    nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_smart_events(const char *pDbFile, SMART_EVENT *smartEvent, Uint64 chnMask, SMART_EVENT_TYPE type)
{
    char table[MAX_LEN_64] = {0};
    if (!get_smart_event_table_name(type, table, sizeof(table))) {
        return copy_events(pDbFile, smartEvent, table, chnMask);
    }

    return -1;
}

//0=effective, 1=params
void get_vca_white_table_name(int type, int event, char *des, int size)
{
    switch (event) {
        case REGIONIN:
            if (type == 0) {
                snprintf(des, size, "%s", VREIN_WLED_ESCHE);
            } else {
                snprintf(des, size, "%s", VREIN_WLED_PARAMS);
            }
            break;

        case REGIONOUT:
            if (type == 0) {
                snprintf(des, size, "%s", VREEX_WLED_ESCHE);
            } else {
                snprintf(des, size, "%s", VREEX_WLED_PARAMS);
            }
            break;

        case ADVANCED_MOTION:
            if (type == 0) {
                snprintf(des, size, "%s", VMOT_WLED_ESCHE);
            } else {
                snprintf(des, size, "%s", VMOT_WLED_PARAMS);
            }
            break;

        case TAMPER:
            if (type == 0) {
                snprintf(des, size, "%s", VTEP_WLED_ESCHE);
            } else {
                snprintf(des, size, "%s", VTEP_WLED_PARAMS);
            }
            break;

        case LINECROSS:
            if (type == 0) {
                snprintf(des, size, "%s", VLSS_WLED_ESCHE);
            } else {
                snprintf(des, size, "%s", VLSS_WLED_PARAMS);
            }
            break;
            
        case LOITERING:
            if (type == 0) {
                snprintf(des, size, "%s", VLER_WLED_ESCHE);
            } else {
                snprintf(des, size, "%s", VLER_WLED_PARAMS);
            }
            break;

        case HUMAN:
            if (type == 0) {
                snprintf(des, size, "%s", VHMN_WLED_ESCHE);
            } else {
                snprintf(des, size, "%s", VHMN_WLED_PARAMS);
            }
            break;

        case PEOPLE_CNT:
            if (type == 0) {
                snprintf(des, size, "%s", VPPE_WLED_ESCHE);
            } else {
                snprintf(des, size, "%s", VPPE_WLED_PARAMS);
            }
            break;

        case OBJECT_LEFTREMOVE:
            if (type == 0) {
                snprintf(des, size, "%s", VOBJ_WLED_ESCHE);
            } else {
                snprintf(des, size, "%s", VOBJ_WLED_PARAMS);
            }
            break;

        default:
            break;
    }
}

// param: type(0: effective, 1: params)
void get_vca_http_table_name(int type, int event, char *des, int size)
{
    switch (event) {
        case REGIONIN:
            if (type == 0) {
                snprintf(des, size, "%s", VREIN_HTTP_SCHE);
            } else {
                snprintf(des, size, "%s", VREIN_HTTP_PARAMS);
            }
            break;

        case REGIONOUT:
            if (type == 0) {
                snprintf(des, size, "%s", VREEX_HTTP_SCHE);
            } else {
                snprintf(des, size, "%s", VREEX_HTTP_PARAMS);
            }
            break;

        case ADVANCED_MOTION:
            if (type == 0) {
                snprintf(des, size, "%s", VMOT_HTTP_SCHE);
            } else {
                snprintf(des, size, "%s", VMOT_HTTP_PARAMS);
            }
            break;

        case TAMPER:
            if (type == 0) {
                snprintf(des, size, "%s", VTEP_HTTP_SCHE);
            } else {
                snprintf(des, size, "%s", VTEP_HTTP_PARAMS);
            }
            break;

        case LINECROSS:
            if (type == 0) {
                snprintf(des, size, "%s", VLSS_HTTP_SCHE);
            } else {
                snprintf(des, size, "%s", VLSS_HTTP_PARAMS);
            }
            break;
            
        case LOITERING:
            if (type == 0) {
                snprintf(des, size, "%s", VLER_HTTP_SCHE);
            } else {
                snprintf(des, size, "%s", VLER_HTTP_PARAMS);
            }
            break;

        case HUMAN:
            if (type == 0) {
                snprintf(des, size, "%s", VHMN_HTTP_SCHE);
            } else {
                snprintf(des, size, "%s", VHMN_HTTP_PARAMS);
            }
            break;

        case PEOPLE_CNT:
            if (type == 0) {
                snprintf(des, size, "%s", VPPE_HTTP_SCHE);
            } else {
                snprintf(des, size, "%s", VPPE_HTTP_PARAMS);
            }
            break;

        case OBJECT_LEFTREMOVE:
            if (type == 0) {
                snprintf(des, size, "%s", VOBJ_HTTP_SCHE);
            } else {
                snprintf(des, size, "%s", VOBJ_HTTP_PARAMS);
            }
            break;

        default:
            break;
    }
}

// param: type(0: effective, 1: params)
void get_anpr_http_table_name(int type, ANPR_MODE_TYPE mode, char *name, int size)
{
    if (!name || size < 1) {
        return;
    }

    if (mode == ANPR_BLACK) {
        if (type == 0) {
            snprintf(name, size, "%s", LPRB_HTTP_SCHE);
        } else {
            snprintf(name, size, "%s", LPRB_HTTP_PARAMS);
        }
    } else if (mode == ANPR_WHITE) {
        if (type == 0) {
            snprintf(name, size, "%s", LPRW_HTTP_SCHE);
        } else {
            snprintf(name, size, "%s", LPRW_HTTP_PARAMS);
        }
    } else if (mode == ANPR_VISTOR) {
        if (type == 0) {
            snprintf(name, size, "%s", LPRV_HTTP_SCHE);
        } else {
            snprintf(name, size, "%s", LPRV_HTTP_PARAMS);
        }
    } else {
        msdebug(DEBUG_INF, "anpr mode type err, mode = %d", mode);
        name[0] = '\0';
    }

    return;
}

// param: type(0: effective, 1: params)
void get_action_type_http_table_name(int type, EVENT_IN_TYPE_E event, char *des, int size)
{
    int alarmId;

    switch (event) {
    case ALARMIO:
        if (type == 0) {
            snprintf(des, size, "%s", AIN_HTTP_SCHE);
        } else {
            snprintf(des, size, "%s", AIN_HTTP_PARAMS);
        }
        break;
    case MOTION:
        if (type == 0) {
            snprintf(des, size, "%s", MOT_HTTP_SCHE);
        } else {
            snprintf(des, size, "%s", MOT_HTTP_PARAMS);
        }
        break;
    case VIDEOLOSS:
        if (type == 0) {
            snprintf(des, size, "%s", VDL_HTTP_SCHE);
        } else {
            snprintf(des, size, "%s", VDL_HTTP_PARAMS);
        }
        break;
    case ALARM_CHN_IN0_EVT:
    case ALARM_CHN_IN1_EVT:
    case ALARM_CHN_IN2_EVT:
    case ALARM_CHN_IN3_EVT:
        alarmId = converse_event_in_to_chn_alarmid(event);
        if (type == 0) {
            snprintf(des, size, AINCH_HTTP_SCHE, alarmId);
        } else {
            snprintf(des, size, AINCH_HTTP_PARAMS, alarmId);
        }
        break;
    default:
        break;
    }
}

int copy_smart_event_action(SMART_EVENT_TYPE type, int chnId, Uint64 chnMask, int popup)
{
    if (chnId < 0 || chnId >= MAX_REAL_CAMERA || !chnMask) {
        return -1;
    }

    int cnt = 0;
    char pTable[MAX_LEN_64] = {0};
    WLED_INFO white;
    SMART_EVENT event;
    SMART_SCHEDULE schedule;
    PTZ_ACTION_PARAMS ptzParams[MAX_REAL_CAMERA];
    WHITE_LED_PARAMS whiteParams[MAX_REAL_CAMERA];
    HTTP_NOTIFICATION_PARAMS_S httpParams;

    memset(&event, 0, sizeof(event));
    read_smart_event(SQLITE_FILE_NAME, &event, chnId, type);
    copy_smart_events(SQLITE_FILE_NAME, &event, chnMask, type);

    memset(&schedule, 0, sizeof(schedule));
    read_smart_event_effective_schedule(SQLITE_FILE_NAME, &schedule, chnId, type);
    copy_smart_event_effective_schedules(SQLITE_FILE_NAME, &schedule, chnMask, type);

    memset(&schedule, 0, sizeof(schedule));
    read_smart_event_audible_schedule(SQLITE_FILE_NAME, &schedule, chnId, type);
    copy_smart_event_audible_schedules(SQLITE_FILE_NAME, &schedule, chnMask, type);

    memset(&schedule, 0, sizeof(schedule));
    read_smart_event_mail_schedule(SQLITE_FILE_NAME, &schedule, chnId, type);
    copy_smart_event_mail_schedules(SQLITE_FILE_NAME, &schedule, chnMask, type);

    if (popup) {
        memset(&schedule, 0, sizeof(schedule));
        read_smart_event_popup_schedule(SQLITE_FILE_NAME, &schedule, chnId, type);
        copy_smart_event_popup_schedules(SQLITE_FILE_NAME, &schedule, chnMask, type);
    }

    memset(&schedule, 0, sizeof(schedule));
    white.chnid = chnId;
    get_vca_white_table_name(0, type, white.pDbTable, sizeof(white.pDbTable));
    read_whiteled_effective_schedule(SQLITE_FILE_NAME, &schedule, &white);
    copy_whiteled_effective_schedules(SQLITE_FILE_NAME, &schedule, &white, chnMask);

    memset(whiteParams, 0, sizeof(whiteParams));
    get_vca_white_table_name(1, type, white.pDbTable, sizeof(white.pDbTable));
    read_whiteled_params(SQLITE_FILE_NAME, whiteParams, &white, &cnt);
    copy_whiteled_params(SQLITE_FILE_NAME, white.pDbTable, whiteParams, chnMask);

    memset(&schedule, 0, sizeof(schedule));
    read_smart_event_ptz_schedule(SQLITE_FILE_NAME, &schedule, chnId, type);
    copy_smart_event_ptz_schedules(SQLITE_FILE_NAME, &schedule, chnMask, type);

    memset(ptzParams, 0, sizeof(ptzParams));
    read_ptz_params(SQLITE_FILE_NAME, ptzParams, REGION_EN + type, chnId, &cnt);
    copy_ptz_params_all(SQLITE_FILE_NAME, ptzParams, REGION_EN + type, (long long)chnMask);

    memset(&schedule, 0, sizeof(schedule));
    get_vca_http_table_name(0, type, pTable, sizeof(pTable));
    read_http_notification_schedule(SQLITE_FILE_NAME, &schedule, pTable, chnId);
    copy_http_notification_schedules(SQLITE_FILE_NAME, &schedule, pTable, chnMask);

    memset(&httpParams, 0, sizeof(httpParams));
    get_vca_http_table_name(1, type, pTable, sizeof(pTable));
    read_http_notification_params(SQLITE_FILE_NAME, &httpParams, pTable, chnId);
    copy_http_notification_params(SQLITE_FILE_NAME, pTable, &httpParams, chnMask);
    
    return 0;
}

int write_motion_effective_schedule(const char *pDbFile, struct smart_event_schedule *schedule, int chn_id)
{
    if (!pDbFile || !schedule || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    char db_table[64] = {0};
    snprintf(db_table, sizeof(db_table), "%s", "motion_effective_schedule");
    snprintf(sExec, sizeof(sExec), "delete from %s where chn_id=%d;", db_table, chn_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into %s values(%d,%d,%d, '%s', '%s', %d);", db_table, chn_id, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);
            if (schedule->schedule_day[i].schedule_item[j].action_type != 8) {
                msprintf("[gsjt debug]write to motion effective's action type error");
            }
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_motion_effective_schedules(const char *pDbFile, struct smart_event_schedule *schedule, long long changeFlag)
{
    if (!pDbFile || !schedule) {
        return -1;
    }

    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[256] = {0};
    char db_table[64] = {0};
    int i = 0, j = 0, k = 0;
    snprintf(db_table, sizeof(db_table), "%s", "motion_effective_schedule");
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (k = 0; k < MAX_CAMERA; k++) {
        if (!(changeFlag >> k & 0x01)) {
            continue;
        }

        snprintf(sExec, sizeof(sExec), "delete from %s where chn_id=%d;", db_table, k);
        sqlite_execute(hConn, mode, sExec);
        for (i = 0; i < MAX_DAY_NUM; i++) {
            for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
                if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                               schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                    continue;
                }
                snprintf(sExec, sizeof(sExec), "insert into %s values(%d,%d,%d, '%s', '%s', %d);", db_table, k, i, j,
                         schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                         schedule->schedule_day[i].schedule_item[j].action_type);
                sqlite_execute(hConn, mode, sExec);
            }
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}


int read_motion_effective_schedule(const char *pDbFile, struct smart_event_schedule  *Schedule, int chn_id)
{
    if (!pDbFile || !Schedule) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[256] = {0};

    HSQLSTMT hStmt = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select week_id,plan_id,start_time,end_time,action_type from motion_effective_schedule where chn_id=%d;", chn_id);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, Schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, Schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_smart_events(const char *pDbFile, struct smart_event *smartevent, int chnid)
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
    int i = 0;
    HSQLSTMT hStmt = 0;
    char sQuery[MAX_SMART_EVENT][2048];
    int nColumnCnt = 0;

    snprintf(sQuery[REGIONIN], sizeof(sQuery[REGIONIN]), "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,\
		email_interval,tri_chnout1_alarms,tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval,\
		email_pic_enable,tri_channels_pic,tri_audio_id,http_notification_interval from vca_regionin_event where id=%d;", chnid);

    snprintf(sQuery[REGIONOUT], sizeof(sQuery[REGIONOUT]), "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,\
		email_interval,tri_chnout1_alarms,tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval,\
		email_pic_enable,tri_channels_pic,tri_audio_id,http_notification_interval from vca_regionexit_event where id=%d;", chnid);

    snprintf(sQuery[ADVANCED_MOTION], sizeof(sQuery[ADVANCED_MOTION]),
             "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,\
		email_interval,tri_chnout1_alarms,tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval,\
		email_pic_enable,tri_channels_pic,tri_audio_id,http_notification_interval from vca_motion_event where id=%d;", chnid);

    snprintf(sQuery[TAMPER], sizeof(sQuery[TAMPER]),
             "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,email_interval,\
		tri_chnout1_alarms,tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval,\
		email_pic_enable,tri_channels_pic,tri_audio_id,http_notification_interval from vca_tamper_event where id=%d;", chnid);

    snprintf(sQuery[LINECROSS], sizeof(sQuery[LINECROSS]), "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,\
		email_interval,tri_chnout1_alarms,tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval,\
		email_pic_enable,tri_channels_pic,tri_audio_id,http_notification_interval from vca_linecross_event where id=%d;", chnid);


    snprintf(sQuery[LOITERING], sizeof(sQuery[LOITERING]), "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,\
		email_interval,tri_chnout1_alarms,tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval,\
		email_pic_enable,tri_channels_pic,tri_audio_id,http_notification_interval from vca_loiter_event where id=%d;", chnid);

    snprintf(sQuery[HUMAN], sizeof(sQuery[HUMAN]), "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,\
		email_interval,tri_chnout1_alarms,tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval,\
		email_pic_enable,tri_channels_pic,tri_audio_id,http_notification_interval from vca_human_event where id=%d;", chnid);

    snprintf(sQuery[PEOPLE_CNT], sizeof(sQuery[PEOPLE_CNT]), "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,\
		email_interval,tri_chnout1_alarms,tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval,\
		email_pic_enable,tri_channels_pic,tri_audio_id,http_notification_interval from vca_people_event where id=%d;", chnid);

    snprintf(sQuery[OBJECT_LEFTREMOVE], sizeof(sQuery[OBJECT_LEFTREMOVE]), "select id,enable,tri_alarms,tri_channels_ex,\
		buzzer_interval,email_interval,tri_chnout1_alarms,tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval,\
		email_pic_enable,tri_channels_pic,tri_audio_id,http_notification_interval from vca_object_leftremove_event where id=%d;", chnid);

    for (i = REGIONIN; i < MAX_SMART_EVENT; i++) {
        int nResult = sqlite_query_record(hConn, sQuery[i], &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
            continue;
        }

        sqlite_query_column(hStmt, 0, &smartevent[i].id);
        sqlite_query_column(hStmt, 1, &smartevent[i].enable);
        sqlite_query_column(hStmt, 2, (int *)&smartevent[i].tri_alarms);
        sqlite_query_column_text(hStmt, 3, smartevent[i].tri_channels_ex, sizeof(smartevent[i].tri_channels_ex));
        sqlite_query_column(hStmt, 4, &smartevent[i].buzzer_interval);
        sqlite_query_column(hStmt, 5, &smartevent[i].email_interval);
        sqlite_query_column_text(hStmt, 6, smartevent[i].tri_chnout1_alarms, sizeof(smartevent[i].tri_chnout2_alarms));
        sqlite_query_column_text(hStmt, 7, smartevent[i].tri_chnout2_alarms, sizeof(smartevent[i].tri_chnout2_alarms));
        sqlite_query_column(hStmt, 8, &smartevent[i].popup_interval);
        sqlite_query_column(hStmt, 9, &smartevent[i].ptzaction_interval);
        sqlite_query_column(hStmt, 10, &smartevent[i].alarmout_interval);
        sqlite_query_column(hStmt, 11, &smartevent[i].whiteled_interval);
        sqlite_query_column(hStmt, 12, &smartevent[i].email_pic_enable);
        sqlite_query_column_text(hStmt, 13, smartevent[i].tri_channels_pic, sizeof(smartevent[i].tri_channels_pic));
        sqlite_query_column(hStmt, 14, &smartevent[i].tri_audio_id);
        sqlite_query_column(hStmt, 15, &smartevent[i].http_notification_interval);

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

int read_smart_event_effective_schedules(const char *pDbFile, struct smart_event_schedule *Schedule, int chn_id)
{
    if (!pDbFile || !Schedule) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_SMART_EVENT][256];

    HSQLSTMT hStmt = 0;
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
            sqlite_query_column_text(hStmt, 2, Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
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
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_smart_event_audible_schedules(const char *pDbFile, struct smart_event_schedule *Schedule, int chn_id)
{
    if (!pDbFile || !Schedule) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_SMART_EVENT][256];

    HSQLSTMT hStmt = 0;
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
            sqlite_query_column_text(hStmt, 2, Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
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
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_smart_event_mail_schedules(const char *pDbFile, struct smart_event_schedule *Schedule, int chn_id)
{
    if (!pDbFile || !Schedule) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_SMART_EVENT][256];

    HSQLSTMT hStmt = 0;
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
            sqlite_query_column_text(hStmt, 2, Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
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
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_smart_event_popup_schedules(const char *pDbFile, struct smart_event_schedule *Schedule, int chn_id)
{
    if (!pDbFile || !Schedule) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_SMART_EVENT][256];

    HSQLSTMT hStmt = 0;
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
            sqlite_query_column_text(hStmt, 2, Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
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
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_smart_event_all_chns(const char *pDbFile, struct smart_event *smartevent[], int allChn)
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
    char sQuery[MAX_SMART_EVENT][2048];
    char cmd[512] = {0};
    int nColumnCnt = 0;

    snprintf(sQuery[REGIONIN], sizeof(sQuery[REGIONIN]),
             "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,email_interval,\
		tri_chnout1_alarms,tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval,email_pic_enable,tri_channels_pic,tri_audio_id from vca_regionin_event");
    snprintf(sQuery[REGIONOUT], sizeof(sQuery[REGIONOUT]),
             "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,email_interval,\
		tri_chnout1_alarms,tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval,email_pic_enable,tri_channels_pic,tri_audio_id from vca_regionexit_event");
    snprintf(sQuery[ADVANCED_MOTION], sizeof(sQuery[ADVANCED_MOTION]),
             "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,email_interval,\
		tri_chnout1_alarms,tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval,email_pic_enable,tri_channels_pic,tri_audio_id from vca_motion_event");
    snprintf(sQuery[TAMPER], sizeof(sQuery[TAMPER]),
             "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,email_interval,tri_chnout1_alarms,\
		tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval,email_pic_enable,tri_channels_pic,tri_audio_id from vca_tamper_event");
    snprintf(sQuery[LINECROSS], sizeof(sQuery[LINECROSS]),
             "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,email_interval,tri_chnout1_alarms,\
		tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval,email_pic_enable,tri_channels_pic,tri_audio_id from vca_linecross_event");
    snprintf(sQuery[LOITERING], sizeof(sQuery[LOITERING]),
             "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,email_interval,tri_chnout1_alarms,\
		tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval,email_pic_enable,tri_channels_pic,tri_audio_id from vca_loiter_event");
    snprintf(sQuery[HUMAN], sizeof(sQuery[HUMAN]),
             "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,email_interval,tri_chnout1_alarms,\
		tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval,email_pic_enable,tri_channels_pic,tri_audio_id from vca_human_event");
    snprintf(sQuery[PEOPLE_CNT], sizeof(sQuery[PEOPLE_CNT]),
             "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,email_interval,tri_chnout1_alarms,\
		tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval,email_pic_enable,tri_channels_pic,tri_audio_id from vca_people_event");

    for (ichn = 0; ichn < allChn; ichn++) {
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
            sqlite_query_column(hStmt, 14, &smartevent[ichn][i].tri_audio_id);

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

int read_smart_event_effective_schedules_all_chns(const char *pDbFile, struct smart_event_schedule *Schedule[],
                                                  int allChn)
{
    if (!pDbFile || !Schedule) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0, ichn = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_SMART_EVENT][256];
    char cmd[512] = {0};

    HSQLSTMT hStmt = 0;
    snprintf(sQuery[REGIONIN], sizeof(sQuery[REGIONIN]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_regionen_effective_schedule");
    snprintf(sQuery[REGIONOUT], sizeof(sQuery[REGIONOUT]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_regionex_effective_schedule");
    snprintf(sQuery[ADVANCED_MOTION], sizeof(sQuery[ADVANCED_MOTION]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_motionadv_effective_schedule");
    snprintf(sQuery[TAMPER], sizeof(sQuery[TAMPER]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_tamper_effective_schedule");
    snprintf(sQuery[LINECROSS], sizeof(sQuery[LINECROSS]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_linecross_effective_schedule");
    snprintf(sQuery[LOITERING], sizeof(sQuery[LOITERING]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_loiter_effective_schedule");
    snprintf(sQuery[HUMAN], sizeof(sQuery[HUMAN]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_human_effective_schedule");
    snprintf(sQuery[PEOPLE_CNT], sizeof(sQuery[PEOPLE_CNT]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_peolpe_effective_schedule");

    for (ichn = 0; ichn < allChn; ichn++) {
        for (iSmt = REGIONIN; iSmt < MAX_SMART_EVENT; iSmt++) {
            snprintf(cmd, sizeof(cmd), "%s where chn_id=%d;", sQuery[iSmt], ichn);
            nResult = sqlite_query_record(hConn, cmd, &hStmt, &nColumnCnt);
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
                sqlite_query_column_text(hStmt, 2, Schedule[ichn][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                         sizeof(Schedule[ichn][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
                sqlite_query_column_text(hStmt, 3, Schedule[ichn][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                         sizeof(Schedule[ichn][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
                sqlite_query_column(hStmt, 4, &Schedule[ichn][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
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
    }

    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_smart_event_audible_schedules_all_chns(const char *pDbFile, struct smart_event_schedule *Schedule[],
                                                int allChn)
{
    if (!pDbFile || !Schedule) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0, ichn = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_SMART_EVENT][256];
    char cmd[512] = {0};

    HSQLSTMT hStmt = 0;
    snprintf(sQuery[REGIONIN], sizeof(sQuery[REGIONIN]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_regionen_audible_schedule");
    snprintf(sQuery[REGIONOUT], sizeof(sQuery[REGIONOUT]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_regionex_audible_schedule");
    snprintf(sQuery[ADVANCED_MOTION], sizeof(sQuery[ADVANCED_MOTION]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_motionadv_audible_schedule");
    snprintf(sQuery[TAMPER], sizeof(sQuery[TAMPER]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_tamper_audible_schedule");
    snprintf(sQuery[LINECROSS], sizeof(sQuery[LINECROSS]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_linecross_audible_schedule");
    snprintf(sQuery[LOITERING], sizeof(sQuery[LOITERING]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_loiter_audible_schedule");
    snprintf(sQuery[HUMAN], sizeof(sQuery[HUMAN]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_human_audible_schedule");
    snprintf(sQuery[PEOPLE_CNT], sizeof(sQuery[PEOPLE_CNT]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_peolpe_audible_schedule");

    for (ichn = 0; ichn < allChn; ichn++) {
        for (iSmt = REGIONIN; iSmt < MAX_SMART_EVENT; iSmt++) {
            snprintf(cmd, sizeof(cmd), "%s where chn_id=%d;", sQuery[iSmt], ichn);
            nResult = sqlite_query_record(hConn, cmd, &hStmt, &nColumnCnt);
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
                sqlite_query_column_text(hStmt, 2, Schedule[ichn][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                         sizeof(Schedule[ichn][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
                sqlite_query_column_text(hStmt, 3, Schedule[ichn][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                         sizeof(Schedule[ichn][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
                sqlite_query_column(hStmt, 4, &Schedule[ichn][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
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
    }

    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_smart_event_mail_schedules_all_chns(const char *pDbFile, struct smart_event_schedule *Schedule[], int allChn)
{
    if (!pDbFile || !Schedule) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0, ichn = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_SMART_EVENT][256];
    char cmd[512] = {0};

    HSQLSTMT hStmt = 0;
    snprintf(sQuery[REGIONIN], sizeof(sQuery[REGIONIN]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_regionen_mail_schedule");
    snprintf(sQuery[REGIONOUT], sizeof(sQuery[REGIONOUT]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_regionex_mail_schedule");
    snprintf(sQuery[ADVANCED_MOTION], sizeof(sQuery[ADVANCED_MOTION]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_motionadv_mail_schedule");
    snprintf(sQuery[TAMPER], sizeof(sQuery[TAMPER]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_tamper_mail_schedule");
    snprintf(sQuery[LINECROSS], sizeof(sQuery[LINECROSS]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_linecross_mail_schedule");
    snprintf(sQuery[LOITERING], sizeof(sQuery[LOITERING]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_loiter_mail_schedule");
    snprintf(sQuery[HUMAN], sizeof(sQuery[HUMAN]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_human_mail_schedule");
    snprintf(sQuery[PEOPLE_CNT], sizeof(sQuery[PEOPLE_CNT]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_peolpe_mail_schedule");

    for (ichn = 0; ichn < allChn; ichn++) {
        for (iSmt = REGIONIN; iSmt < MAX_SMART_EVENT; iSmt++) {
            snprintf(cmd, sizeof(cmd), "%s where chn_id=%d;", sQuery[iSmt], ichn);
            nResult = sqlite_query_record(hConn, cmd, &hStmt, &nColumnCnt);
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
                sqlite_query_column_text(hStmt, 2, Schedule[ichn][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                         sizeof(Schedule[ichn][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
                sqlite_query_column_text(hStmt, 3, Schedule[ichn][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                         sizeof(Schedule[ichn][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
                sqlite_query_column(hStmt, 4, &Schedule[ichn][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
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
    }

    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_smart_event_popup_schedules_all_chns(const char *pDbFile, struct smart_event_schedule *Schedule[], int allChn)
{
    if (!pDbFile || !Schedule) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0, ichn = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_SMART_EVENT][256];
    char cmd[512] = {0};

    HSQLSTMT hStmt = 0;
    snprintf(sQuery[REGIONIN], sizeof(sQuery[REGIONIN]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_regionin_popup_schedule");
    snprintf(sQuery[REGIONOUT], sizeof(sQuery[REGIONOUT]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_regionexit_popup_schedule");
    snprintf(sQuery[ADVANCED_MOTION], sizeof(sQuery[ADVANCED_MOTION]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_motion_popup_schedule");
    snprintf(sQuery[TAMPER], sizeof(sQuery[TAMPER]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_tamper_popup_schedule");
    snprintf(sQuery[LINECROSS], sizeof(sQuery[LINECROSS]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_linecross_popup_schedule");
    snprintf(sQuery[LOITERING], sizeof(sQuery[LOITERING]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_loiter_popup_schedule");
    snprintf(sQuery[HUMAN], sizeof(sQuery[HUMAN]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_human_popup_schedule");
    snprintf(sQuery[PEOPLE_CNT], sizeof(sQuery[PEOPLE_CNT]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_people_popup_schedule");
    snprintf(sQuery[OBJECT_LEFTREMOVE], sizeof(sQuery[OBJECT_LEFTREMOVE]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_object_leftremove_popup_schedule");

    for (ichn = 0; ichn < allChn; ichn++) {
        for (iSmt = REGIONIN; iSmt < MAX_SMART_EVENT; iSmt++) {
            snprintf(cmd, sizeof(cmd), "%s where chn_id=%d;", sQuery[iSmt], ichn);
            nResult = sqlite_query_record(hConn, cmd, &hStmt, &nColumnCnt);
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
                sqlite_query_column_text(hStmt, 2, Schedule[ichn][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                         sizeof(Schedule[ichn][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
                sqlite_query_column_text(hStmt, 3, Schedule[ichn][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                         sizeof(Schedule[ichn][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
                sqlite_query_column(hStmt, 4, &Schedule[ichn][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
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
    }

    if (hStmt) {
        sqlite_clear_stmt(hStmt);
        hStmt = 0;
    }
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_smart_event_effective_schedule_default(const char *pDbFile, int chn_id)
{
    if (!pDbFile || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);

    char sExec[256] = {0};
    char db_table[MAX_SMART_EVENT][64];
    char db_mot_table[64] = "motion_effective_schedule";

    snprintf(db_table[REGIONIN], sizeof(db_table[REGIONIN]), "%s", "vca_regionen_effective_schedule");
    snprintf(db_table[REGIONOUT], sizeof(db_table[REGIONOUT]), "%s", "vca_regionex_effective_schedule");
    snprintf(db_table[ADVANCED_MOTION], sizeof(db_table[ADVANCED_MOTION]), "%s", "vca_motionadv_effective_schedule");
    snprintf(db_table[TAMPER], sizeof(db_table[TAMPER]), "%s", "vca_tamper_effective_schedule");
    snprintf(db_table[LINECROSS], sizeof(db_table[LINECROSS]), "%s", "vca_linecross_effective_schedule");
    snprintf(db_table[LOITERING], sizeof(db_table[LOITERING]), "%s", "vca_loiter_effective_schedule");
    snprintf(db_table[HUMAN], sizeof(db_table[HUMAN]), "%s", "vca_human_effective_schedule");
    snprintf(db_table[PEOPLE_CNT], sizeof(db_table[PEOPLE_CNT]), "%s", "vca_peolpe_effective_schedule");
    snprintf(db_table[OBJECT_LEFTREMOVE], sizeof(db_table[OBJECT_LEFTREMOVE]), "%s",
             "vca_object_leftremove_effective_schedule");

    int i = 0, iSmt = 0;
    for (iSmt = 0; iSmt < MAX_SMART_EVENT; iSmt++) {
        snprintf(sExec, sizeof(sExec), "delete from %s where chn_id=%d;", db_table[iSmt], chn_id);
        sqlite_execute(hConn, mode, sExec);
        for (i = 0; i < MAX_DAY_NUM; i++) {
            snprintf(sExec, sizeof(sExec), "insert into %s values(%d,%d,%d, '%s', '%s', %d);", db_table[iSmt], chn_id, i, 0,
                     "00:00", "24:00", SMART_EVT_RECORD);
            sqlite_execute(hConn, mode, sExec);
        }
    }

    snprintf(sExec, sizeof(sExec), "delete from %s where chn_id=%d;", db_mot_table, chn_id);
    sqlite_execute(hConn, mode, sExec);
    for (i = 0; i < MAX_DAY_NUM; i++) {
        snprintf(sExec, sizeof(sExec), "insert into %s values(%d,%d,%d, '%s', '%s', %d);", db_mot_table, chn_id, i, 0, "00:00",
                 "24:00", MOTION_ACTION);
        sqlite_execute(hConn, mode, sExec);
    }

    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}


int read_failover(const char *pDbFile, struct failover_list *failover, int id)
{
    if (!pDbFile || !failover) {
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
    snprintf(sQuery, sizeof(sQuery),
             "select id,enable,ipaddr,port,mac,model,username,password,start_time,end_time,maxCamera from failover_list where id = %d;", id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &failover->id);
    sqlite_query_column(hStmt, 1, &failover->enable);
    sqlite_query_column_text(hStmt, 2, failover->ipaddr, sizeof(failover->ipaddr));
    sqlite_query_column(hStmt, 3, &failover->port);
    sqlite_query_column_text(hStmt, 4, failover->mac, sizeof(failover->mac));
    sqlite_query_column_text(hStmt, 5, failover->model, sizeof(failover->model));
    sqlite_query_column_text(hStmt, 6, failover->username, sizeof(failover->username));
    sqlite_query_column_text(hStmt, 7, failover->password, sizeof(failover->password));
    sqlite_query_column_text(hStmt, 8, failover->start_time, sizeof(failover->start_time));
    sqlite_query_column_text(hStmt, 9, failover->end_time, sizeof(failover->end_time));
    sqlite_query_column(hStmt, 10, &failover->maxCamera);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_failovers(const char *pDbFile, struct failover_list failovers[], int *cnt)
{
    if (!pDbFile || !failovers) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select id,enable,ipaddr,port,mac,model,username,password,start_time,end_time,maxCamera from failover_list;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *cnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < MAX_FAILOVER; i++) {
        sqlite_query_column(hStmt, 0, &failovers[i].id);
        sqlite_query_column(hStmt, 1, &failovers[i].enable);
        sqlite_query_column_text(hStmt, 2, failovers[i].ipaddr, sizeof(failovers[i].ipaddr));
        sqlite_query_column(hStmt, 3, &failovers[i].port);
        sqlite_query_column_text(hStmt, 4, failovers[i].mac, sizeof(failovers[i].mac));
        sqlite_query_column_text(hStmt, 5, failovers[i].model, sizeof(failovers[i].model));
        sqlite_query_column_text(hStmt, 6, failovers[i].username, sizeof(failovers[i].username));
        sqlite_query_column_text(hStmt, 7, failovers[i].password, sizeof(failovers[i].password));
        sqlite_query_column_text(hStmt, 8, failovers[i].start_time, sizeof(failovers[i].start_time));
        sqlite_query_column_text(hStmt, 9, failovers[i].end_time, sizeof(failovers[i].end_time));
        sqlite_query_column(hStmt, 10, &failovers[i].maxCamera);

        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    *cnt = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int delete_failovers_by_ip(const char *pDbFile, char ips[][MAX_LEN_16], int cnt, Uint32 *pMask)
{
    if (!pDbFile || !ips || cnt <= 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    char sExec[2048] = {0};
    int i;
    HSQLITE hConn = NULL;
    HSQLSTMT hStmt = NULL;
    int nResult;
    int nColumnCnt = 0;
    int id = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    snprintf(sExec, sizeof(sExec), "select id from failover_list where");
    for (i = 0; i < cnt; ++i) {
        snprintf(sExec + strlen(sExec), sizeof(sExec) - strlen(sExec), " ipaddr='%s'", ips[i]);
        if (i == cnt - 1) {
            snprintf(sExec + strlen(sExec), sizeof(sExec) - strlen(sExec), ";");
        } else {
            snprintf(sExec + strlen(sExec), sizeof(sExec) - strlen(sExec), " or");
        }
    }

    nResult = sqlite_query_record(hConn, sExec, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        sqlite_clear_stmt(hStmt);
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < cnt; i++) {
        sqlite_query_column(hStmt, 0, &id);
        *pMask |= (1 << id);
    
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    sqlite_clear_stmt(hStmt);

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (i = 0; i < cnt; ++i) {
        snprintf(sExec, sizeof(sExec), "update failover_list set enable=0 where ipaddr='%s';", ips[i]);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_failover(const char *pDbFile, struct failover_list *failover)
{
    if (!pDbFile || !failover) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[2048] = {0};
    char buf1[MAX_PWD_LEN * 2 + 2] = {0};
    if (failover->id == 0) {
        translate_pwd(buf1, failover->password, strlen(failover->password));
    }

    snprintf(sExec, sizeof(sExec),
             "update failover_list set enable=%d,ipaddr='%s',port=%d,mac='%s',model='%s',username='%s',password='%s',start_time='%s',end_time='%s',maxCamera=%d where id=%d;",
             failover->enable, failover->ipaddr, failover->port, failover->mac, failover->model, failover->username, buf1,
             failover->start_time, failover->end_time, failover->maxCamera, failover->id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int reset_schedule_for_failover(const char *pDbFile)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec), "delete from motion_effective_schedule;");
    sqlite_execute(hConn, mode, sExec);

    snprintf(sExec, sizeof(sExec), "delete from vca_regionen_effective_schedule;");
    sqlite_execute(hConn, mode, sExec);

    snprintf(sExec, sizeof(sExec), "delete from vca_regionex_effective_schedule;");
    sqlite_execute(hConn, mode, sExec);

    snprintf(sExec, sizeof(sExec), "delete from vca_motionadv_effective_schedule;");
    sqlite_execute(hConn, mode, sExec);

    snprintf(sExec, sizeof(sExec), "delete from vca_tamper_effective_schedule;");
    sqlite_execute(hConn, mode, sExec);

    snprintf(sExec, sizeof(sExec), "delete from vca_linecross_effective_schedule;");
    sqlite_execute(hConn, mode, sExec);

    snprintf(sExec, sizeof(sExec), "delete from vca_loiter_effective_schedule;");
    sqlite_execute(hConn, mode, sExec);

    snprintf(sExec, sizeof(sExec), "delete from vca_human_effective_schedule;");
    sqlite_execute(hConn, mode, sExec);

    snprintf(sExec, sizeof(sExec), "delete from vca_peolpe_effective_schedule;");
    sqlite_execute(hConn, mode, sExec);

    snprintf(sExec, sizeof(sExec), "delete from snapshot_schedule;");
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

static int get_ptz_params_table_name(EVENT_IN_TYPE_E type, char *table, int size)
{
    if (!table || size <= 0) {
        return -1;
    }

    int ret = 0;
    if (type == ALARMIO) {
        snprintf(table, size, "%s", "alarmin_ptz_params");
    } else if (type == MOTION) {
        snprintf(table, size, "%s", "motion_ptz_params");
    } else if (type == VIDEOLOSS) {
        snprintf(table, size, "%s", "videoloss_ptz_params");
    } else if (type == REGION_EN) {
        snprintf(table, size, "%s", "vca_regionein_ptz_params");
    } else if (type == REGION_EXIT) {
        snprintf(table, size, "%s", "vca_regionexit_ptz_params");
    } else if (type == ADVANCED_MOT) {
        snprintf(table, size, "%s", "vca_motion_ptz_params");
    } else if (type == TAMPER_DET) {
        snprintf(table, size, "%s", "vca_tamper_ptz_params");
    } else if (type == LINE_CROSS) {
        snprintf(table, size, "%s", "vca_linecross_ptz_params");
    } else if (type == LOITER) {
        snprintf(table, size, "%s", "vca_loiter_ptz_params");
    } else if (type == HUMAN_DET) {
        snprintf(table, size, "%s", "vca_human_ptz_params");
    } else if (type == PEOPLE_COUNT) {
        snprintf(table, size, "%s", "vca_people_ptz_params");
    } else if (type == LEFTREMOVE) {
        snprintf(table, size, "%s", "vca_object_leftremove_ptz_params");
    } else if (type == ANPR_BLACK_EVT) {
        snprintf(table, size, "%s", "lpr_black_mode_ptz_params");
    } else if (type == ANPR_WHITE_EVT) {
        snprintf(table, size, "%s", "lpr_white_mode_ptz_params");
    } else if (type == ANPR_VISTOR_EVT) {
        snprintf(table, size, "%s", "lpr_vistor_mode_ptz_params");
    } else if (type == ALARM_CHN_IN0_EVT || type == ALARM_CHN_IN1_EVT 
               || type == ALARM_CHN_IN2_EVT || type == ALARM_CHN_IN3_EVT ) {
        snprintf(table, size, "alarm_chnIn%d_ptz_params", converse_event_in_to_chn_alarmid(type));
//    } else if (type == PRIVATE_PEOPLE_CNT) {
//        snprintf(table, size, "%s", "people_cnt_ptz_params");
    } else if (type == POS_EVT) {
        snprintf(table, size, "%s", "pos_ptz_params");
    } else if (type == REGIONAL_PEOPLE_CNT0) {
        snprintf(table, size, "%s", "regional_pcnt_ptz_params");
    } else if (type == REGIONAL_PEOPLE_CNT1) {
        snprintf(table, size, "%s", "regional_pcnt1_ptz_params");
    } else if (type == REGIONAL_PEOPLE_CNT2) {
        snprintf(table, size, "%s", "regional_pcnt2_ptz_params");
    } else if (type == REGIONAL_PEOPLE_CNT3) {
        snprintf(table, size, "%s", "regional_pcnt3_ptz_params");
    } else if (type == FACE_EVT) {
        snprintf(table, size, "%s", "face_ptz_params");
    } else if (type == AUDIO_ALARM) {
        snprintf(table, size, "audio_alarm_ptz_params");
    } else {
        ret = -1;
    }

    return ret;
}

static int get_ptz_params_table_id(EVENT_IN_TYPE_E type, char *id, int size)
{
    if (!id || size <= 0) {
        return -1;
    }

    int ret = 0;
    if (type <= ALARM_CHN_IN1_EVT || type >= FACE_EVT) {
        snprintf(id, size, "%s", "chn_id");
//    } else if (type == PRIVATE_PEOPLE_CNT) {
//        snprintf(id, size, "%s", "groupid");
    } else if (type >= POS_EVT && type <= REGIONAL_PEOPLE_CNT3) {
        snprintf(id, size, "%s", "id");
    } else {
        ret = -1;
    }

    return ret;
}

int read_ptz_params(const char *pDbFile, struct ptz_action_params ptzActionParams[], int type, int chn_id, int *cnt)
{
    if (!pDbFile || !ptzActionParams || type < 0 || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = -1;
    HSQLITE hConn = 0;
    HSQLSTMT hStmt = 0;
    char sQuery[2048] = {0};
    int nColumnCnt = 0;
    int i = 0;
    char table[128] = {0};
    char id[32] = {0};

    if (get_ptz_params_table_name(type, table, sizeof(table))
        || get_ptz_params_table_id(type, id, sizeof(id))) {
        return -1;
    }
    snprintf(sQuery, sizeof(sQuery), "select %s,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,\
                acto_ptz_patrol,acto_ptz_pattern from %s where %s=%d;", id, table, id, chn_id);

    nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *cnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < MAX_REAL_CAMERA; i++) {
        sqlite_query_column(hStmt, 0, &ptzActionParams[i].chn_id);
        sqlite_query_column(hStmt, 1, &ptzActionParams[i].acto_ptz_channel);
        sqlite_query_column(hStmt, 2, &ptzActionParams[i].acto_fish_channel);
        sqlite_query_column(hStmt, 3, &ptzActionParams[i].acto_ptz_type);
        sqlite_query_column(hStmt, 4, &ptzActionParams[i].acto_ptz_preset);
        sqlite_query_column(hStmt, 5, &ptzActionParams[i].acto_ptz_patrol);
        sqlite_query_column(hStmt, 6, &ptzActionParams[i].acto_ptz_pattern);

        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    *cnt  = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_ptz_params(const char *pDbFile, struct ptz_action_params *ptzActionParams, int type)
{
    if (!pDbFile || !ptzActionParams || type < 0) {
        return -1;
    }
    int alarmId;
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    char sExec[2048] = {0};
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    switch (type) {
        case MOTION:
            snprintf(sExec, sizeof(sExec), "delete from motion_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            sqlite_execute(hConn, mode, sExec);
            snprintf(sExec, sizeof(sExec),
                     "insert into motion_ptz_params(chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern) values(%d,%d,%d,%d,%d,%d,%d);",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel, ptzActionParams->acto_fish_channel,
                     ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset, ptzActionParams->acto_ptz_patrol,
                     ptzActionParams->acto_ptz_pattern);
            break;
        case VIDEOLOSS:
            snprintf(sExec, sizeof(sExec), "delete from videoloss_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            sqlite_execute(hConn, mode, sExec);
            snprintf(sExec, sizeof(sExec),
                     "insert into videoloss_ptz_params(chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern) values(%d,%d,%d,%d,%d,%d,%d);",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel, ptzActionParams->acto_fish_channel,
                     ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset, ptzActionParams->acto_ptz_patrol,
                     ptzActionParams->acto_ptz_pattern);
            break;
        case ALARMIO:
            snprintf(sExec, sizeof(sExec), "delete from alarmin_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            sqlite_execute(hConn, mode, sExec);
            snprintf(sExec, sizeof(sExec),
                     "insert into alarmin_ptz_params(chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern) values(%d,%d,%d,%d,%d,%d,%d);",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel, ptzActionParams->acto_fish_channel,
                     ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset, ptzActionParams->acto_ptz_patrol,
                     ptzActionParams->acto_ptz_pattern);
            break;
        case REGION_EN:
            snprintf(sExec, sizeof(sExec), "delete from vca_regionein_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            sqlite_execute(hConn, mode, sExec);
            snprintf(sExec, sizeof(sExec),
                     "insert into vca_regionein_ptz_params(chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern) values(%d,%d,%d,%d,%d,%d,%d);",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel, ptzActionParams->acto_fish_channel,
                     ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset, ptzActionParams->acto_ptz_patrol,
                     ptzActionParams->acto_ptz_pattern);
            break;
        case REGION_EXIT:
            snprintf(sExec, sizeof(sExec), "delete from vca_regionexit_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            sqlite_execute(hConn, mode, sExec);
            snprintf(sExec, sizeof(sExec),
                     "insert into vca_regionexit_ptz_params(chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern) values(%d,%d,%d,%d,%d,%d,%d);",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel, ptzActionParams->acto_fish_channel,
                     ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset, ptzActionParams->acto_ptz_patrol,
                     ptzActionParams->acto_ptz_pattern);
            break;
        case ADVANCED_MOT:
            snprintf(sExec, sizeof(sExec), "delete from vca_motion_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            sqlite_execute(hConn, mode, sExec);
            snprintf(sExec, sizeof(sExec),
                     "insert into vca_motion_ptz_params(chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern) values(%d,%d,%d,%d,%d,%d,%d);",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel, ptzActionParams->acto_fish_channel,
                     ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset, ptzActionParams->acto_ptz_patrol,
                     ptzActionParams->acto_ptz_pattern);
            break;
        case TAMPER_DET:
            snprintf(sExec, sizeof(sExec), "delete from vca_tamper_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            sqlite_execute(hConn, mode, sExec);
            snprintf(sExec, sizeof(sExec),
                     "insert into vca_tamper_ptz_params(chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern) values(%d,%d,%d,%d,%d,%d,%d);",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel, ptzActionParams->acto_fish_channel,
                     ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset, ptzActionParams->acto_ptz_patrol,
                     ptzActionParams->acto_ptz_pattern);
            break;
        case LINE_CROSS:
            snprintf(sExec, sizeof(sExec), "delete from vca_linecross_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            sqlite_execute(hConn, mode, sExec);
            snprintf(sExec, sizeof(sExec),
                     "insert into vca_linecross_ptz_params(chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern) values(%d,%d,%d,%d,%d,%d,%d);",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel, ptzActionParams->acto_fish_channel,
                     ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset, ptzActionParams->acto_ptz_patrol,
                     ptzActionParams->acto_ptz_pattern);
            break;
        case LOITER:
            snprintf(sExec, sizeof(sExec), "delete from vca_loiter_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            sqlite_execute(hConn, mode, sExec);
            snprintf(sExec, sizeof(sExec),
                     "insert into vca_loiter_ptz_params(chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern) values(%d,%d,%d,%d,%d,%d,%d);",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel, ptzActionParams->acto_fish_channel,
                     ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset, ptzActionParams->acto_ptz_patrol,
                     ptzActionParams->acto_ptz_pattern);
            break;
        case HUMAN_DET:
            snprintf(sExec, sizeof(sExec), "delete from vca_human_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            sqlite_execute(hConn, mode, sExec);
            snprintf(sExec, sizeof(sExec),
                     "insert into vca_human_ptz_params(chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern) values(%d,%d,%d,%d,%d,%d,%d);",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel, ptzActionParams->acto_fish_channel,
                     ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset, ptzActionParams->acto_ptz_patrol,
                     ptzActionParams->acto_ptz_pattern);
            break;
        case PEOPLE_COUNT:
            snprintf(sExec, sizeof(sExec), "delete from vca_people_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            sqlite_execute(hConn, mode, sExec);
            snprintf(sExec, sizeof(sExec),
                     "insert into vca_people_ptz_params(chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern) values(%d,%d,%d,%d,%d,%d,%d);",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel, ptzActionParams->acto_fish_channel,
                     ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset, ptzActionParams->acto_ptz_patrol,
                     ptzActionParams->acto_ptz_pattern);
            break;
        case LEFTREMOVE:
            snprintf(sExec, sizeof(sExec), "delete from vca_object_leftremove_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            sqlite_execute(hConn, mode, sExec);
            snprintf(sExec, sizeof(sExec),
                     "insert into vca_object_leftremove_ptz_params(chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern) values(%d,%d,%d,%d,%d,%d,%d);",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel, ptzActionParams->acto_fish_channel,
                     ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset, ptzActionParams->acto_ptz_patrol,
                     ptzActionParams->acto_ptz_pattern);
            break;


        case ANPR_BLACK_EVT:
            snprintf(sExec, sizeof(sExec), "delete from lpr_black_mode_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            sqlite_execute(hConn, mode, sExec);
            snprintf(sExec, sizeof(sExec),
                     "insert into lpr_black_mode_ptz_params(chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern) values(%d,%d,%d,%d,%d,%d,%d);",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel, ptzActionParams->acto_fish_channel,
                     ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset, ptzActionParams->acto_ptz_patrol,
                     ptzActionParams->acto_ptz_pattern);
            break;
        case ANPR_WHITE_EVT:
            snprintf(sExec, sizeof(sExec), "delete from lpr_white_mode_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            sqlite_execute(hConn, mode, sExec);
            snprintf(sExec, sizeof(sExec),
                     "insert into lpr_white_mode_ptz_params(chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern) values(%d,%d,%d,%d,%d,%d,%d);",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel, ptzActionParams->acto_fish_channel,
                     ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset, ptzActionParams->acto_ptz_patrol,
                     ptzActionParams->acto_ptz_pattern);
            break;
        case ANPR_VISTOR_EVT:
            snprintf(sExec, sizeof(sExec), "delete from lpr_vistor_mode_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            sqlite_execute(hConn, mode, sExec);
            snprintf(sExec, sizeof(sExec),
                     "insert into lpr_vistor_mode_ptz_params(chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern) values(%d,%d,%d,%d,%d,%d,%d);",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel, ptzActionParams->acto_fish_channel,
                     ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset, ptzActionParams->acto_ptz_patrol,
                     ptzActionParams->acto_ptz_pattern);
            break;
        case ALARM_CHN_IN0_EVT:
        case ALARM_CHN_IN1_EVT:
        case ALARM_CHN_IN2_EVT:
        case ALARM_CHN_IN3_EVT:
            alarmId = converse_event_in_to_chn_alarmid(type);
            snprintf(sExec, sizeof(sExec), "delete from alarm_chnIn%d_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     alarmId, ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            sqlite_execute(hConn, mode, sExec);
            snprintf(sExec, sizeof(sExec),
                     "insert into alarm_chnIn%d_ptz_params(chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern) values(%d,%d,%d,%d,%d,%d,%d);",
                     alarmId, ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel, ptzActionParams->acto_fish_channel,
                     ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset, ptzActionParams->acto_ptz_patrol,
                     ptzActionParams->acto_ptz_pattern);
            break;
        case PRIVATE_PEOPLE_CNT:
            snprintf(sExec, sizeof(sExec), "delete from people_cnt_ptz_params where groupid=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            sqlite_execute(hConn, mode, sExec);
            snprintf(sExec, sizeof(sExec),
                     "insert into people_cnt_ptz_params(groupid,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern) values(%d,%d,%d,%d,%d,%d,%d);",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel, ptzActionParams->acto_fish_channel,
                     ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset, ptzActionParams->acto_ptz_patrol,
                     ptzActionParams->acto_ptz_pattern);
            break;
        default:
            break;
    }
    if (sExec[0] == '\0') {
        sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_ptz_params_all(const char *pDbFile, struct ptz_action_params ptzActionParams[], int type, int chan_id)
{
    if (!pDbFile || !ptzActionParams || type < 0) {
        return -1;
    }
    int i = 0;
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    char sExec[2048] = {0}, sqliteName[MAX_LEN_64] = {0};
    char tmpKey[16] = "chn_id";
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    switch (type) {
        case MOTION:
            snprintf(sqliteName, sizeof(sqliteName), "motion_ptz_params");
            break;
        case VIDEOLOSS:
            snprintf(sqliteName, sizeof(sqliteName), "videoloss_ptz_params");
            break;
        case ALARMIO:
            snprintf(sqliteName, sizeof(sqliteName), "alarmin_ptz_params");
            break;
        case REGION_EN:
            snprintf(sqliteName, sizeof(sqliteName), "vca_regionein_ptz_params");
            break;
        case REGION_EXIT:
            snprintf(sqliteName, sizeof(sqliteName), "vca_regionexit_ptz_params");
            break;
        case ADVANCED_MOT:
            snprintf(sqliteName, sizeof(sqliteName), "vca_motion_ptz_params");
            break;
        case TAMPER_DET:
            snprintf(sqliteName, sizeof(sqliteName), "vca_tamper_ptz_params");
            break;
        case LINE_CROSS:
            snprintf(sqliteName, sizeof(sqliteName), "vca_linecross_ptz_params");
            break;
        case LOITER:
            snprintf(sqliteName, sizeof(sqliteName), "vca_loiter_ptz_params");
            break;
        case HUMAN_DET:
            snprintf(sqliteName, sizeof(sqliteName), "vca_human_ptz_params");
            break;
        case PEOPLE_COUNT:
            snprintf(sqliteName, sizeof(sqliteName), "vca_people_ptz_params");
            break;
        case LEFTREMOVE:
            snprintf(sqliteName, sizeof(sqliteName), "vca_object_leftremove_ptz_params");
            break;
        case ANPR_BLACK_EVT:
            snprintf(sqliteName, sizeof(sqliteName), "lpr_black_mode_ptz_params");
            break;
        case ANPR_WHITE_EVT:
            snprintf(sqliteName, sizeof(sqliteName), "lpr_white_mode_ptz_params");
            break;
        case ANPR_VISTOR_EVT:
            snprintf(sqliteName, sizeof(sqliteName), "lpr_vistor_mode_ptz_params");
            break;
        case ALARM_CHN_IN0_EVT:
        case ALARM_CHN_IN1_EVT:
        case ALARM_CHN_IN2_EVT:
        case ALARM_CHN_IN3_EVT:
            snprintf(sqliteName, sizeof(sqliteName), "alarm_chnIn%d_ptz_params", converse_event_in_to_chn_alarmid(type));
            break;
        case PRIVATE_PEOPLE_CNT:
            snprintf(tmpKey, sizeof(tmpKey), "%s", "groupid");
            snprintf(sqliteName, sizeof(sqliteName), "people_cnt_ptz_params");
            break;
        case POS_EVT:
            snprintf(tmpKey, sizeof(tmpKey), "%s", "id");
            snprintf(sqliteName, sizeof(sqliteName), "pos_ptz_params");
            break;
        case REGIONAL_PEOPLE_CNT0:
            snprintf(tmpKey, sizeof(tmpKey), "%s", "id");
            snprintf(sqliteName, sizeof(sqliteName), "regional_pcnt_ptz_params");
            break;
        case REGIONAL_PEOPLE_CNT1:
            snprintf(tmpKey, sizeof(tmpKey), "%s", "id");
            snprintf(sqliteName, sizeof(sqliteName), "regional_pcnt1_ptz_params");
            break;
        case REGIONAL_PEOPLE_CNT2:
            snprintf(tmpKey, sizeof(tmpKey), "%s", "id");
            snprintf(sqliteName, sizeof(sqliteName), "regional_pcnt2_ptz_params");
            break;
        case REGIONAL_PEOPLE_CNT3:
            snprintf(tmpKey, sizeof(tmpKey), "%s", "id");
            snprintf(sqliteName, sizeof(sqliteName), "regional_pcnt3_ptz_params");
            break;
        case FACE_EVT:
            snprintf(sqliteName, sizeof(sqliteName), "face_ptz_params");
            break;
        case AUDIO_ALARM:
            snprintf(sqliteName, sizeof(sqliteName), "audio_alarm_ptz_params");
            break;
        default:
            break;
    }
    if (sqliteName[0] == '\0') {
        sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    snprintf(sExec, sizeof(sExec), "delete from %s where %s=%d", sqliteName, tmpKey, chan_id);
    sqlite_execute(hConn, mode, sExec);
    for (i = 0; i < MAX_REAL_CAMERA; i++) {
        if (ptzActionParams[i].acto_ptz_channel > 0) {
            snprintf(sExec, sizeof(sExec),
                     "insert into %s(%s,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern) values(%d,%d,%d,%d,%d,%d,%d);",
                     sqliteName, tmpKey, ptzActionParams[i].chn_id, ptzActionParams[i].acto_ptz_channel, ptzActionParams[i].acto_fish_channel,
                     ptzActionParams[i].acto_ptz_type, ptzActionParams[i].acto_ptz_preset, ptzActionParams[i].acto_ptz_patrol,
                     ptzActionParams[i].acto_ptz_pattern);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_ptz_params_all(const char *pDbFile, struct ptz_action_params ptzActionParams[], int type, long long changeFlag)
{
    if (!pDbFile || !ptzActionParams || type < 0) {
        return -1;
    }
    int i = 0, j = 0;
    int mode = FILE_MODE_WR;
    int nFd = -1;
    char sExec[2048] = {0}, sqliteName[MAX_LEN_64] = {0};
    HSQLITE hConn = 0;
    char id[32] = {0};

    if (get_ptz_params_table_name(type, sqliteName, sizeof(sqliteName))
        || get_ptz_params_table_id(type, id, sizeof(id))) {
        return -1;
    }
    nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (j = 0; j < MAX_REAL_CAMERA; j++) {
        if (!(changeFlag >> j & 0x01)) {
            continue;
        }
        snprintf(sExec, sizeof(sExec), "delete from %s where %s=%d", sqliteName, id, j);
        sqlite_execute(hConn, mode, sExec);
        for (i = 0; i < MAX_REAL_CAMERA; i++) {
            if (ptzActionParams[i].acto_ptz_channel > 0) {
                snprintf(sExec, sizeof(sExec),
                         "insert into %s(%s,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern) values(%d,%d,%d,%d,%d,%d,%d);",
                         sqliteName, id, j, ptzActionParams[i].acto_ptz_channel, ptzActionParams[i].acto_fish_channel,
                         ptzActionParams[i].acto_ptz_type, ptzActionParams[i].acto_ptz_preset, ptzActionParams[i].acto_ptz_patrol,
                         ptzActionParams[i].acto_ptz_pattern);
                sqlite_execute(hConn, mode, sExec);
            }
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}


int delete_ptz_params(const char *pDbFile, struct ptz_action_params *ptzActionParams, int type)
{
    if (!pDbFile || !ptzActionParams || type < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    char sExec[2048] = {0};
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    switch (type) {
        case MOTION:
            snprintf(sExec, sizeof(sExec), "delete from motion_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            break;
        case VIDEOLOSS:
            snprintf(sExec, sizeof(sExec), "delete from videoloss_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            break;
        case ALARMIO:
            snprintf(sExec, sizeof(sExec), "delete from alarmin_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            break;
        case REGION_EN:
            snprintf(sExec, sizeof(sExec), "delete from vca_regionein_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            break;
        case REGION_EXIT:
            snprintf(sExec, sizeof(sExec), "delete from vca_regionexit_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            break;
        case ADVANCED_MOT:
            snprintf(sExec, sizeof(sExec), "delete from vca_motion_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            break;
        case TAMPER_DET:
            snprintf(sExec, sizeof(sExec), "delete from vca_tamper_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            break;
        case LINE_CROSS:
            snprintf(sExec, sizeof(sExec), "delete from vca_linecross_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            break;
        case LOITER:
            snprintf(sExec, sizeof(sExec), "delete from vca_loiter_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            break;
        case HUMAN_DET:
            snprintf(sExec, sizeof(sExec), "delete from vca_human_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            break;
        case PEOPLE_COUNT:
            snprintf(sExec, sizeof(sExec), "delete from vca_people_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            break;
        case LEFTREMOVE:
            snprintf(sExec, sizeof(sExec), "delete from vca_object_leftremove_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            break;

        case ANPR_BLACK_EVT:
            snprintf(sExec, sizeof(sExec), "delete from lpr_black_mode_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            break;
        case ANPR_WHITE_EVT:
            snprintf(sExec, sizeof(sExec), "delete from lpr_white_mode_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            break;
        case ANPR_VISTOR_EVT:
            snprintf(sExec, sizeof(sExec), "delete from lpr_vistor_mode_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            break;
        case ALARM_CHN_IN0_EVT:
        case ALARM_CHN_IN1_EVT:
        case ALARM_CHN_IN2_EVT:
        case ALARM_CHN_IN3_EVT:
            snprintf(sExec, sizeof(sExec), "delete from alarm_chnIn%d_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     converse_event_in_to_chn_alarmid(type), ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            break;
        case PRIVATE_PEOPLE_CNT:
            snprintf(sExec, sizeof(sExec), "delete from people_cnt_ptz_params where groupid=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            break;
        default:
            break;
    }
    if (sExec[0] == '\0') {
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int update_ptz_params(const char *pDbFile, struct ptz_action_params *ptzActionParams, int type, int oldChan)
{
    if (!pDbFile || !ptzActionParams || type < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    char sExec[2048] = {0};
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    switch (type) {
        case MOTION:
            snprintf(sExec, sizeof(sExec),
                     "update motion_ptz_params set acto_fish_channel=%d,acto_ptz_type=%d,acto_ptz_preset=%d,acto_ptz_patrol=%d,acto_ptz_pattern=%d,acto_ptz_channel=%d where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->acto_fish_channel, ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset,
                     ptzActionParams->acto_ptz_patrol, ptzActionParams->acto_ptz_pattern, ptzActionParams->acto_ptz_channel,
                     ptzActionParams->chn_id, oldChan);
            break;
        case VIDEOLOSS:
            snprintf(sExec, sizeof(sExec),
                     "update videoloss_ptz_params set acto_fish_channel=%d,acto_ptz_type=%d,acto_ptz_preset=%d,acto_ptz_patrol=%d,acto_ptz_pattern=%d,acto_ptz_channel=%d where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->acto_fish_channel, ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset,
                     ptzActionParams->acto_ptz_patrol, ptzActionParams->acto_ptz_pattern, ptzActionParams->acto_ptz_channel,
                     ptzActionParams->chn_id, oldChan);
            break;
        case ALARMIO:
            snprintf(sExec, sizeof(sExec),
                     "update alarmin_ptz_params set acto_fish_channel=%d,acto_ptz_type=%d,acto_ptz_preset=%d,acto_ptz_patrol=%d,acto_ptz_pattern=%d,acto_ptz_channel=%d where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->acto_fish_channel, ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset,
                     ptzActionParams->acto_ptz_patrol, ptzActionParams->acto_ptz_pattern, ptzActionParams->acto_ptz_channel,
                     ptzActionParams->chn_id, oldChan);
            break;
        case REGION_EN:
            snprintf(sExec, sizeof(sExec),
                     "update vca_regionein_ptz_params set acto_fish_channel=%d,acto_ptz_type=%d,acto_ptz_preset=%d,acto_ptz_patrol=%d,acto_ptz_pattern=%d,acto_ptz_channel=%d where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->acto_fish_channel, ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset,
                     ptzActionParams->acto_ptz_patrol, ptzActionParams->acto_ptz_pattern, ptzActionParams->acto_ptz_channel,
                     ptzActionParams->chn_id, oldChan);
            break;
        case REGION_EXIT:
            snprintf(sExec, sizeof(sExec),
                     "update vca_regionexit_ptz_params set acto_fish_channel=%d,acto_ptz_type=%d,acto_ptz_preset=%d,acto_ptz_patrol=%d,acto_ptz_pattern=%d,acto_ptz_channel=%d where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->acto_fish_channel, ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset,
                     ptzActionParams->acto_ptz_patrol, ptzActionParams->acto_ptz_pattern, ptzActionParams->acto_ptz_channel,
                     ptzActionParams->chn_id, oldChan);
            break;
        case ADVANCED_MOT:
            snprintf(sExec, sizeof(sExec),
                     "update vca_motion_ptz_params set acto_fish_channel=%d,acto_ptz_type=%d,acto_ptz_preset=%d,acto_ptz_patrol=%d,acto_ptz_pattern=%d,acto_ptz_channel=%d where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->acto_fish_channel, ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset,
                     ptzActionParams->acto_ptz_patrol, ptzActionParams->acto_ptz_pattern, ptzActionParams->acto_ptz_channel,
                     ptzActionParams->chn_id, oldChan);
            break;
        case TAMPER_DET:
            snprintf(sExec, sizeof(sExec),
                     "update vca_tamper_ptz_params set acto_fish_channel=%d,acto_ptz_type=%d,acto_ptz_preset=%d,acto_ptz_patrol=%d,acto_ptz_pattern=%d,acto_ptz_channel=%d where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->acto_fish_channel, ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset,
                     ptzActionParams->acto_ptz_patrol, ptzActionParams->acto_ptz_pattern, ptzActionParams->acto_ptz_channel,
                     ptzActionParams->chn_id, oldChan);
            break;
        case LINE_CROSS:
            snprintf(sExec, sizeof(sExec),
                     "update vca_linecross_ptz_params set acto_fish_channel=%d,acto_ptz_type=%d,acto_ptz_preset=%d,acto_ptz_patrol=%d,acto_ptz_pattern=%d,acto_ptz_channel=%d where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->acto_fish_channel, ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset,
                     ptzActionParams->acto_ptz_patrol, ptzActionParams->acto_ptz_pattern, ptzActionParams->acto_ptz_channel,
                     ptzActionParams->chn_id, oldChan);
            break;
        case LOITER:
            snprintf(sExec, sizeof(sExec),
                     "update vca_loiter_ptz_params set acto_fish_channel=%d,acto_ptz_type=%d,acto_ptz_preset=%d,acto_ptz_patrol=%d,acto_ptz_pattern=%d,acto_ptz_channel=%d where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->acto_fish_channel, ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset,
                     ptzActionParams->acto_ptz_patrol, ptzActionParams->acto_ptz_pattern, ptzActionParams->acto_ptz_channel,
                     ptzActionParams->chn_id, oldChan);
            break;
        case HUMAN_DET:
            snprintf(sExec, sizeof(sExec),
                     "update vca_human_ptz_params set acto_fish_channel=%d,acto_ptz_type=%d,acto_ptz_preset=%d,acto_ptz_patrol=%d,acto_ptz_pattern=%d,acto_ptz_channel=%d where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->acto_fish_channel, ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset,
                     ptzActionParams->acto_ptz_patrol, ptzActionParams->acto_ptz_pattern, ptzActionParams->acto_ptz_channel,
                     ptzActionParams->chn_id, oldChan);
            break;
        case PEOPLE_COUNT:
            snprintf(sExec, sizeof(sExec),
                     "update vca_people_ptz_params set acto_fish_channel=%d,acto_ptz_type=%d,acto_ptz_preset=%d,acto_ptz_patrol=%d,acto_ptz_pattern=%d,acto_ptz_channel=%d where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->acto_fish_channel, ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset,
                     ptzActionParams->acto_ptz_patrol, ptzActionParams->acto_ptz_pattern, ptzActionParams->acto_ptz_channel,
                     ptzActionParams->chn_id, oldChan);
            break;
        case LEFTREMOVE:
            snprintf(sExec, sizeof(sExec),
                     "update vca_object_leftremove_ptz_params set acto_fish_channel=%d,acto_ptz_type=%d,acto_ptz_preset=%d,acto_ptz_patrol=%d,acto_ptz_pattern=%d,acto_ptz_channel=%d where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->acto_fish_channel, ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset,
                     ptzActionParams->acto_ptz_patrol, ptzActionParams->acto_ptz_pattern, ptzActionParams->acto_ptz_channel,
                     ptzActionParams->chn_id, oldChan);
            break;

        case ANPR_BLACK_EVT:
            snprintf(sExec, sizeof(sExec),
                     "update lpr_black_mode_ptz_params set acto_fish_channel=%d,acto_ptz_type=%d,acto_ptz_preset=%d,acto_ptz_patrol=%d,acto_ptz_pattern=%d,acto_ptz_channel=%d where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->acto_fish_channel, ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset,
                     ptzActionParams->acto_ptz_patrol, ptzActionParams->acto_ptz_pattern, ptzActionParams->acto_ptz_channel,
                     ptzActionParams->chn_id, oldChan);
            break;
        case ANPR_WHITE_EVT:
            snprintf(sExec, sizeof(sExec),
                     "update lpr_white_mode_ptz_params set acto_fish_channel=%d,acto_ptz_type=%d,acto_ptz_preset=%d,acto_ptz_patrol=%d,acto_ptz_pattern=%d,acto_ptz_channel=%d where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->acto_fish_channel, ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset,
                     ptzActionParams->acto_ptz_patrol, ptzActionParams->acto_ptz_pattern, ptzActionParams->acto_ptz_channel,
                     ptzActionParams->chn_id, oldChan);
            break;
        case ANPR_VISTOR_EVT:
            snprintf(sExec, sizeof(sExec),
                     "update lpr_vistor_mode_ptz_params set acto_fish_channel=%d,acto_ptz_type=%d,acto_ptz_preset=%d,acto_ptz_patrol=%d,acto_ptz_pattern=%d,acto_ptz_channel=%d where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->acto_fish_channel, ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset,
                     ptzActionParams->acto_ptz_patrol, ptzActionParams->acto_ptz_pattern, ptzActionParams->acto_ptz_channel,
                     ptzActionParams->chn_id, oldChan);
            break;
        case ALARM_CHN_IN0_EVT:
            snprintf(sExec, sizeof(sExec),
                     "update alarm_chnIn0_ptz_params set acto_fish_channel=%d,acto_ptz_type=%d,acto_ptz_preset=%d,acto_ptz_patrol=%d,acto_ptz_pattern=%d,acto_ptz_channel=%d where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->acto_fish_channel, ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset,
                     ptzActionParams->acto_ptz_patrol, ptzActionParams->acto_ptz_pattern, ptzActionParams->acto_ptz_channel,
                     ptzActionParams->chn_id, oldChan);
            break;
        case ALARM_CHN_IN1_EVT:
            snprintf(sExec, sizeof(sExec),
                     "update alarm_chnIn1_ptz_params set acto_fish_channel=%d,acto_ptz_type=%d,acto_ptz_preset=%d,acto_ptz_patrol=%d,acto_ptz_pattern=%d,acto_ptz_channel=%d where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->acto_fish_channel, ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset,
                     ptzActionParams->acto_ptz_patrol, ptzActionParams->acto_ptz_pattern, ptzActionParams->acto_ptz_channel,
                     ptzActionParams->chn_id, oldChan);
            break;

        default:
            break;
    }
    if (sExec[0] == '\0') {
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_videoloss_ptz_schedule(const char *pDbFile, struct video_loss_schedule *videolostSchedule, int chn_id)
{
    if (!pDbFile || !videolostSchedule) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select week_id,plan_id,start_time,end_time,action_type from videoloss_ptz_schedule where chn_id=%d;", chn_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, videolostSchedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(videolostSchedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, videolostSchedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(videolostSchedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(videolostSchedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_videoloss_ptz_schedule(const char *pDbFile, struct video_loss_schedule *videolostSchedule, int chn_id)
{
    if (!pDbFile || !videolostSchedule || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "delete from videoloss_ptz_schedule where chn_id=%d;", chn_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(videolostSchedule->schedule_day[i].schedule_item[j].start_time,
                           videolostSchedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into videoloss_ptz_schedule values(%d,%d,%d,'%s','%s',%d);", chn_id, i, j,
                     videolostSchedule->schedule_day[i].schedule_item[j].start_time,
                     videolostSchedule->schedule_day[i].schedule_item[j].end_time,
                     videolostSchedule->schedule_day[i].schedule_item[j].action_type);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_videoloss_ptz_schedules(const char *pDbFile, struct video_loss_schedule *videolostSchedule,
                                 long long changeFlag)
{
    if (!pDbFile || !videolostSchedule) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[256] = {0};
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);

    int i = 0, j = 0, k = 0;
    for (k = 0; k < MAX_CAMERA; k++) {
        if (!(changeFlag >> k & 0x01)) {
            continue;
        }

        snprintf(sExec, sizeof(sExec), "delete from videoloss_ptz_schedule where chn_id=%d;", k);
        sqlite_execute(hConn, mode, sExec);
        for (i = 0; i < MAX_DAY_NUM; i++) {
            for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
                if (strcasecmp(videolostSchedule->schedule_day[i].schedule_item[j].start_time,
                               videolostSchedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                    continue;
                }
                snprintf(sExec, sizeof(sExec), "insert into videoloss_ptz_schedule values(%d,%d,%d,'%s','%s',%d);", k, i, j,
                         videolostSchedule->schedule_day[i].schedule_item[j].start_time,
                         videolostSchedule->schedule_day[i].schedule_item[j].end_time,
                         videolostSchedule->schedule_day[i].schedule_item[j].action_type);
                sqlite_execute(hConn, mode, sExec);
            }
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_smart_event_ptz_schedules(const char *pDbFile, struct smart_event_schedule *Schedule, int chn_id)
{
    if (!pDbFile || !Schedule) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_SMART_EVENT][256];

    HSQLSTMT hStmt = 0;
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
            sqlite_query_column_text(hStmt, 2, Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
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
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

static int get_smart_ptz_table_name(SMART_EVENT_TYPE type, char *table, int size)
{
    if (!table || size <= 0) {
        return -1;
    }

    int ret = 0;
    if (type == REGIONIN) {
        snprintf(table, size, "%s", "vca_regionein_ptz_schedule");
    } else if (type == REGIONOUT) {
        snprintf(table, size, "%s", "vca_regioneexit_ptz_schedule");
    } else if (type == ADVANCED_MOTION) {
        snprintf(table, size, "%s", "vca_motion_ptz_schedule");
    } else if (type == TAMPER) {
        snprintf(table, size, "%s", "vca_tamper_ptz_schedule");
    } else if (type == LINECROSS) {
        snprintf(table, size, "%s", "vca_linecross_ptz_schedule");
    } else if (type == LOITERING) {
        snprintf(table, size, "%s", "vca_loiter_ptz_schedule");
    } else if (type == HUMAN) {
        snprintf(table, size, "%s", "vca_human_ptz_schedule");
    } else if (type == PEOPLE_CNT) {
        snprintf(table, size, "%s", "vca_people_ptz_schedule");
    } else if (type == OBJECT_LEFTREMOVE) {
        snprintf(table, size, "%s", "vca_object_leftremove_ptz_schedule");
    } else {
        ret = -1;
    }

    return ret;
}

int read_smart_event_ptz_schedule(const char *pDbFile, struct smart_event_schedule  *Schedule, int chn_id,
                                  SMART_EVENT_TYPE type)
{
    if (!pDbFile || !Schedule) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = -1;
    HSQLITE hConn = 0;

    int i = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[256] = {0};

    HSQLSTMT hStmt = 0;
    char table[MAX_LEN_64] = {0};

    if (!get_smart_ptz_table_name(type, table, sizeof(table))) {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from %s where chn_id=%d;", table, chn_id);
    } else {
        return -1;
    }

    nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, Schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, Schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_smart_event_ptz_schedule(const char *pDbFile, struct smart_event_schedule *schedule, int chn_id,
                                   SMART_EVENT_TYPE type)
{
    if (!pDbFile || !schedule || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = -1;
    HSQLITE hConn = 0;

    char sExec[256] = {0};
    char db_table[64] = {0};
    if (get_smart_ptz_table_name(type, db_table, sizeof(db_table))) {
        return -1;
    }
    nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    snprintf(sExec, sizeof(sExec), "delete from %s where chn_id=%d;", db_table, chn_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into %s values(%d,%d,%d, '%s', '%s', %d);", db_table, chn_id, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_smart_event_ptz_schedules(const char *pDbFile, SMART_SCHEDULE *schedule,
                                                    Uint64 chnMask, SMART_EVENT_TYPE type)
{
    char table[MAX_LEN_64] = {0};
    if (chnMask && !get_smart_ptz_table_name(type, table, sizeof(table))) {
        return copy_schedules(pDbFile, schedule, table, chnMask);
    }

    return -1;
}

int write_alarmin_effective_schedule(const char *pDbFile, struct smart_event_schedule *schedule, int chn_id)
{
    if (!pDbFile || !schedule || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    char db_table[64] = {0};
    snprintf(db_table, sizeof(db_table), "%s", "alarmin_effective_schedule");
    snprintf(sExec, sizeof(sExec), "delete from %s where chn_id=%d;", db_table, chn_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into %s values(%d,%d,%d, '%s', '%s', %d);", db_table, chn_id, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_alarmin_effective_schedules(const char *pDbFile, struct smart_event_schedule *schedule, long long changeFlag)
{
    if (!pDbFile || !schedule) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[256] = {0};
    char db_table[64] = {0};
    int i = 0, j = 0, k = 0;
    snprintf(db_table, sizeof(db_table), "%s", "alarmin_effective_schedule");
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (k = 0; k < MAX_CAMERA; k++) {
        if (!(changeFlag >> k & 0x01)) {
            continue;
        }

        snprintf(sExec, sizeof(sExec), "delete from %s where chn_id=%d;", db_table, k);
        sqlite_execute(hConn, mode, sExec);
        for (i = 0; i < MAX_DAY_NUM; i++) {
            for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
                if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                               schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                    continue;
                }
                snprintf(sExec, sizeof(sExec), "insert into %s values(%d,%d,%d, '%s', '%s', %d);", db_table, k, i, j,
                         schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                         schedule->schedule_day[i].schedule_item[j].action_type);
                sqlite_execute(hConn, mode, sExec);
            }
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}


int read_alarmin_effective_schedule(const char *pDbFile, struct smart_event_schedule  *Schedule, int chn_id)
{
    if (!pDbFile || !Schedule) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[256] = {0};

    HSQLSTMT hStmt = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select week_id,plan_id,start_time,end_time,action_type from alarmin_effective_schedule where chn_id=%d;", chn_id);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, Schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, Schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_exposure(const char *pDbFile, struct exposure exposures[], int chnid, int cnt)
{
    if (!pDbFile || !exposures) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    int i = 0;
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "delete from exposure where chnid=%d;", chnid);
    sqlite_execute(hConn, mode, sExec);

    for (i = 0; i < cnt; i++) {
        snprintf(sExec, sizeof(sExec), "insert into exposure(chnid,exposure_time,gain_level) values(%d,%d,%d);",
                 chnid, exposures[i].exposureTime, exposures[i].gainLevel);

        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_exposure(const char *pDbFile, struct exposure exposures[], int chnid, int *cnt)
{
    if (!pDbFile || !exposures) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery), "select exposure_time,gain_level from exposure where chnid=%d;", chnid);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *cnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < MAX_EXPOSURE_NUM; i++) {
        sqlite_query_column(hStmt, 0, &exposures[i].exposureTime);
        sqlite_query_column(hStmt, 1, &exposures[i].gainLevel);

        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    *cnt = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_snapshots(const char *pDbFile, struct snapshot snapshots[], int count, int *pCnt)
{
    if (!pDbFile || !snapshots) {
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
    char sQuery[512] = {0};
    int nColumnCnt = 0;
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select chn_id,stream_type,quality,interval,interval_unit,expiration_date,width,height,resolution from snapshot;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *pCnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < MAX_CAMERA && i < count; i++) {
        sqlite_query_column(hStmt, 0, &snapshots[i].id);
        sqlite_query_column(hStmt, 1, &snapshots[i].stream_type);
        sqlite_query_column(hStmt, 2, &snapshots[i].quality);
        sqlite_query_column(hStmt, 3, &snapshots[i].interval);
        sqlite_query_column(hStmt, 4, &snapshots[i].interval_unit);
        sqlite_query_column(hStmt, 5, &snapshots[i].expiration_date);
        sqlite_query_column(hStmt, 6, &snapshots[i].width);
        sqlite_query_column(hStmt, 7, &snapshots[i].height);
        sqlite_query_column(hStmt, 8, &snapshots[i].resolution);
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    *pCnt = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_snapshot(const char *pDbFile, struct snapshot *snapshot, int chn_id)
{
    if (!pDbFile || !snapshot || chn_id < 0) {
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
    char sQuery[512] = {0};
    int nColumnCnt = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select chn_id,stream_type,quality,interval,interval_unit,expiration_date,width,height,resolution from snapshot where chn_id=%d;",
             chn_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, &snapshot->id);
    sqlite_query_column(hStmt, 1, &snapshot->stream_type);
    sqlite_query_column(hStmt, 2, &snapshot->quality);
    sqlite_query_column(hStmt, 3, &snapshot->interval);
    sqlite_query_column(hStmt, 4, &snapshot->interval_unit);
    sqlite_query_column(hStmt, 5, &snapshot->expiration_date);
    sqlite_query_column(hStmt, 6, &snapshot->width);
    sqlite_query_column(hStmt, 7, &snapshot->height);
    sqlite_query_column(hStmt, 8, &snapshot->resolution);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_snapshot(const char *pDbFile, struct snapshot *snapshot)
{
    if (!pDbFile || !snapshot || snapshot->id < 0 || snapshot->id >= MAX_CAMERA) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    memset(sExec, 0, sizeof(sExec));
    snprintf(sExec, sizeof(sExec),
             "update snapshot set stream_type=%d,quality=%d,interval=%d,interval_unit=%d,expiration_date=%d,width=%d,height=%d,resolution=%d where chn_id=%d;",
             snapshot->stream_type, snapshot->quality, snapshot->interval, snapshot->interval_unit, snapshot->expiration_date,
             snapshot->width, snapshot->height, snapshot->resolution, snapshot->id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_snapshots(const char *pDbFile, struct snapshot *snapshots, long long changeFlag)
{
    if (!pDbFile || !snapshots) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int i = 0;
    char sExec[512] = {0};
    struct snapshot *snapshot = NULL;
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (i = 0; i < MAX_CAMERA; i++) {
        if (!(changeFlag >> i & 0x01)) {
            continue;
        }
        snapshot = snapshots + i;
        snprintf(sExec, sizeof(sExec),
                 "update snapshot set stream_type=%d,quality=%d,interval=%d,interval_unit=%d,expiration_date=%d,width=%d,height=%d,resolution=%d where chn_id=%d;",
                 snapshot->stream_type, snapshot->quality, snapshot->interval, snapshot->interval_unit, snapshot->expiration_date,
                 snapshot->width, snapshot->height, snapshot->resolution, snapshot->id);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_snapshots(const char *pDbFile, struct snapshot *snapshot, long long changeFlag)
{
    if (!pDbFile || !snapshot) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int i = 0;
    char sExec[512] = {0};

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (i = 0; i < MAX_CAMERA; i++) {
        if (!(changeFlag >> i & 0x01)) {
            continue;
        }

        snprintf(sExec, sizeof(sExec),
                 "update snapshot set stream_type=%d,quality=%d,interval=%d,interval_unit=%d,expiration_date=%d,width=%d,height=%d,resolution=%d where chn_id=%d;",
                 snapshot->stream_type, snapshot->quality, snapshot->interval, snapshot->interval_unit, snapshot->expiration_date,
                 snapshot->width, snapshot->height, snapshot->resolution, i);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}


int read_snapshot_schedule(const char *pDbFile, struct snapshot_schedule *schedule, int chn_id)
{
    if (!pDbFile || !schedule || chn_id < 0 || chn_id >= MAX_CAMERA) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select enable,wday_id,wholeday_enable,wholeday_action_type,plan_id,start_time,end_time,action_type from snapshot_schedule where chn_id=%d;",
             chn_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM;
    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &schedule->enable);
        sqlite_query_column(hStmt, 1, &nWeekId);
        sqlite_query_column(hStmt, 2, &schedule->schedule_day[nWeekId].wholeday_enable);
        sqlite_query_column(hStmt, 3, &schedule->schedule_day[nWeekId].wholeday_action_type);
        sqlite_query_column(hStmt, 4, &nPlanId);
        sqlite_query_column_text(hStmt, 5, schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 6, schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 7, &(schedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_snapshot_schedules(const char *pDbFile, struct snapshot_schedule *schedules, int count, int *pCnt)
{
    if (!pDbFile || !schedules) {
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
    int i = 0, j = 0, k = 0;
    int nResult = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM;
    struct snapshot_schedule *schedule = NULL;

    for (k = 0; k < MAX_CAMERA && k < count; k++) {
        snprintf(sQuery, sizeof(sQuery),
                 "select enable,wday_id,wholeday_enable,wholeday_action_type,plan_id,start_time,end_time,action_type from snapshot_schedule where chn_id=%d;",
                 k);
        nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);

        if (nResult || !nColumnCnt) {
            sqlite_clear_stmt(hStmt);
			hStmt = NULL;
            continue;
        }
        schedule = schedules + k;
        for (i = 0; i <= nLoopCnt; i++) {
            sqlite_query_column(hStmt, 0, &schedule->enable);
            sqlite_query_column(hStmt, 1, &nWeekId);
            sqlite_query_column(hStmt, 2, &schedule->schedule_day[nWeekId].wholeday_enable);
            sqlite_query_column(hStmt, 3, &schedule->schedule_day[nWeekId].wholeday_action_type);
            sqlite_query_column(hStmt, 4, &nPlanId);
            sqlite_query_column_text(hStmt, 5, schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 6, schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 7, &(schedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
            if (sqlite_query_next(hStmt) != 0) {
                break;
            }
        }
        sqlite_clear_stmt(hStmt);
		hStmt = NULL;
        j++;
    }
    *pCnt = j;
    if(hStmt){
        sqlite_clear_stmt(hStmt);
    }
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_snapshot_schedule(const char *pDbFile, struct snapshot_schedule *schedule, int chn_id)
{
    if (!pDbFile || !schedule || chn_id < 0 || chn_id >= MAX_CAMERA) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec), "delete from snapshot_schedule where chn_id=%d;", chn_id);
    sqlite_execute(hConn, mode, sExec);

    int i = 0;
    int j = 0;
    int nNotInsNum = 0;
    int nHasWholeDayEnable = 0;
    int t;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        nNotInsNum = 0;
        nHasWholeDayEnable = 0;
        if (schedule->schedule_day[i].wholeday_enable != 0) {
            nHasWholeDayEnable = 1;
        }
        t = 0;
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM; j++) {
            if (schedule->schedule_day[i].schedule_item[j].action_type == NONE ||
                strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                nNotInsNum++;
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into snapshot_schedule values(%d,%d,%d,%d,%d,%d,'%s','%s',%d);", chn_id,
                     schedule->enable, i, schedule->schedule_day[i].wholeday_enable, schedule->schedule_day[i].wholeday_action_type, t,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);

            t++;
            sqlite_execute(hConn, mode, sExec);
        }

        if (nHasWholeDayEnable == 1 && nNotInsNum == MAX_PLAN_NUM_PER_DAY) {
            snprintf(sExec, sizeof(sExec), "insert into snapshot_schedule values(%d,%d,%d,%d,%d,%d,'%s','%s',%d);", chn_id,
                     schedule->enable, i, schedule->schedule_day[i].wholeday_enable, schedule->schedule_day[i].wholeday_action_type, 0,
                     "00:00:00", "00:00:00", 0);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_snapshot_schedules(const char *pDbFile, struct snapshot_schedule *schedules, struct channel_batch *batch)
{
    if (!pDbFile || !schedules || !batch) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[512] = {0};
    int i = 0;
    int j = 0;
    int k = 0;
    int nNotInsNum = 0;
    int nHasWholeDayEnable = 0;
    int t = 0;
    int chanid = 0;
    struct snapshot_schedule *schedule = NULL;

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (k = 0; k < batch->size && k < MAX_CAMERA; k++) {
        chanid = batch->chanid[k];
        if (chanid < 0 || chanid >= MAX_CAMERA) {
            continue;
        }

        schedule = schedules + chanid;
        snprintf(sExec, sizeof(sExec), "delete from snapshot_schedule where chn_id=%d;", chanid);
        sqlite_execute(hConn, mode, sExec);
        for (i = 0; i < MAX_DAY_NUM; i++) {
            nNotInsNum = 0;
            nHasWholeDayEnable = 0;
            if (schedule->schedule_day[i].wholeday_enable != 0) {
                nHasWholeDayEnable = 1;
            }
            t = 0;
            for (j = 0; j < MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM; j++) {
                if (schedule->schedule_day[i].schedule_item[j].action_type == NONE ||
                    strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                               schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                    nNotInsNum++;
                    continue;
                }
                snprintf(sExec, sizeof(sExec), "insert into snapshot_schedule values(%d,%d,%d,%d,%d,%d,'%s','%s',%d);", chanid,
                         schedule->enable, i, schedule->schedule_day[i].wholeday_enable, schedule->schedule_day[i].wholeday_action_type, t,
                         schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                         schedule->schedule_day[i].schedule_item[j].action_type);

                t++;
                sqlite_execute(hConn, mode, sExec);
            }

            if (nHasWholeDayEnable == 1 && nNotInsNum == MAX_PLAN_NUM_PER_DAY) {
                snprintf(sExec, sizeof(sExec), "insert into snapshot_schedule values(%d,%d,%d,%d,%d,%d,'%s','%s',%d);", chanid,
                         schedule->enable, i, schedule->schedule_day[i].wholeday_enable, schedule->schedule_day[i].wholeday_action_type, 0,
                         "00:00:00", "00:00:00", 0);
                sqlite_execute(hConn, mode, sExec);
            }
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_snapshot_schedules(const char *pDbFile, struct snapshot_schedule *schedule, long long changeFlag)
{
    if (!pDbFile || !schedule) {
        return -1;
    }

    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[512] = {0};
    int i = 0;
    int j = 0;
    int k = 0;
    int nNotInsNum = 0;
    int nHasWholeDayEnable = 0;
    int t = 0;

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (k = 0; k < MAX_CAMERA; k++) {
        if (!(changeFlag >> k & 0x01)) {
            continue;
        }

        snprintf(sExec, sizeof(sExec), "delete from snapshot_schedule where chn_id=%d;", k);
        sqlite_execute(hConn, mode, sExec);
        for (i = 0; i < MAX_DAY_NUM; i++) {
            nNotInsNum = 0;
            nHasWholeDayEnable = 0;
            if (schedule->schedule_day[i].wholeday_enable != 0) {
                nHasWholeDayEnable = 1;
            }
            t = 0;
            for (j = 0; j < MAX_PLAN_NUM_PER_DAY * MAX_WND_NUM; j++) {
                if (schedule->schedule_day[i].schedule_item[j].action_type == NONE ||
                    strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                               schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                    nNotInsNum++;
                    continue;
                }
                snprintf(sExec, sizeof(sExec), "insert into snapshot_schedule values(%d,%d,%d,%d,%d,%d,'%s','%s',%d);", k,
                         schedule->enable, i, schedule->schedule_day[i].wholeday_enable, schedule->schedule_day[i].wholeday_action_type, t,
                         schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                         schedule->schedule_day[i].schedule_item[j].action_type);

                t++;
                sqlite_execute(hConn, mode, sExec);
            }

            if (nHasWholeDayEnable == 1 && nNotInsNum == MAX_PLAN_NUM_PER_DAY) {
                snprintf(sExec, sizeof(sExec), "insert into snapshot_schedule values(%d,%d,%d,%d,%d,%d,'%s','%s',%d);", k,
                         schedule->enable, i, schedule->schedule_day[i].wholeday_enable, schedule->schedule_day[i].wholeday_action_type, 0,
                         "00:00:00", "00:00:00", 0);
                sqlite_execute(hConn, mode, sExec);
            }
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int reset_snapshot_schedule(const char *pDbFile, int chn_id)
{
    if (!pDbFile || chn_id < 0 || chn_id >= MAX_CAMERA) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec), "delete from snapshot_schedule where chn_id=%d;", chn_id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int db_version_update(const char *pDbFile, int old_ver, int new_ver, const char *pTxtFile)
{
    int i = 0;
    int fd = -1;
    FILE *fp = NULL;
    char sql[1024] = {0};
    char path[128] = {0};
    HSQLITE hConn = 0;
    int mode = FILE_MODE_WR;

    if (!pDbFile || !pTxtFile || old_ver > new_ver) {
        return -1;
    }

    fd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);

    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(fd, mode, &global_rwlock);
        return -1;
    }

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (i = old_ver; i <= new_ver; i++) {
        snprintf(path, sizeof(path), "%s/db-%d.txt", pTxtFile, i);
        fp = fopen(path, "rb");
        if (fp) {
            while (fgets(sql, sizeof(sql), fp)) {
                sqlite_execute(hConn, mode, sql);
            }
            fclose(fp);
            fp = NULL;
        } else {
            msprintf("###### file %s is not exist! ########", path);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(fd, mode, &global_rwlock);

    return 0;
}

static int update_pushmsg_db(DB_VER_INFO_S *infos[], int cnt)
{
    if (!infos || cnt <= 0) {
        return -1;
    }

    int i;
    int fd = -1;
    FILE *fp = NULL;
    char sql[1024] = {0};
    char path[128] = {0};
    HSQLITE hConn = NULL;
    int mode = FILE_MODE_WR;

    fd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    if (sqlite_conn(SQLITE_PUSHMSG_NAME, &hConn) != 0) {
        FileUnlock(fd, mode, &global_rwlock);
        return -1;
    }
    
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (i = 0; i < cnt; ++i) {
        if (!infos[i]) {
            continue;
        }
        
        snprintf(path, sizeof(path), "%s/%s.txt", SQLITE_INIT_SQL_PATH, infos[i]->file);
        fp = fopen(path, "rb");
        if (!fp) {
            msdebug(DEBUG_ERR, "db update file %s is not exist.", path);
            continue;
        }

        while (fgets(sql, sizeof(sql), fp)) {
            if(sql[0] == '#'){ // read not sql but note
                continue;
            }
            sqlite_execute(hConn, mode, sql);
        }

        infos[i]->execute = 1;

        fclose(fp);
        fp = NULL;
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(fd, mode, &global_rwlock);

    return 0;
}

int db_version_update_by_info(const char *pDbFile, int updateCnt, DB_VER_INFO_S updateArr[], const char *pTxtFile)
{
    int i = 0;
    int fd = -1;
    FILE *fp = NULL;
    char sql[1024] = {0};
    char path[128] = {0};
    HSQLITE hConn = 0;
    int mode = FILE_MODE_WR;
    DB_VER_INFO_S *pushmsgInfos[4];
    int pushmsgCnt= 0;
    memset(pushmsgInfos, 0, sizeof(pushmsgInfos));

    if (!pDbFile || !pTxtFile) {
        return -1;
    }
    fd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(fd, mode, &global_rwlock);
        return -1;
    }

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);

    for (i = 0; i < updateCnt; ++i) {
        // need update pushmsg.db
        if (!strcmp(updateArr[i].file, "db-9.0.16-1006")) {
            pushmsgInfos[pushmsgCnt] = &updateArr[i];
            ++pushmsgCnt;
            continue;
        }
    
        snprintf(path, sizeof(path), "%s/%s.txt", pTxtFile, updateArr[i].file);

        fp = fopen(path, "rb");
        if (fp) {
            while (fgets(sql, sizeof(sql), fp)) {
                if(sql[0] == '#'){ // read not sql but note
                    continue;
                }
                sqlite_execute(hConn, mode, sql);
            }

            updateArr[i].execute = 1;

            fclose(fp);
            fp = NULL;
        } else {
            msdebug(DEBUG_ERR, "db update file %s is not exist.", path);
        }
    }

    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(fd, mode, &global_rwlock);

    update_pushmsg_db(pushmsgInfos, pushmsgCnt);

    return 0;
}
//copy_table_column_int(SQLITE_FILE_NAME, "camera", "record_stream", 1, 0xFF, "id");
int copy_table_column_int(const char *pDbFile, const char *table, const char *key, int value, long long  changeFlag,
                          const char *idKey)
{
    int i = 0;
    char sExec[512] = {0};
    int nFd = -1;
    HSQLITE hConn = 0;
    int mode = FILE_MODE_WR;

    if (!pDbFile) {
        return -1;
    }

    nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (i = 0; i < MAX_CAMERA; i++) {
        if (!(changeFlag >> i & 0x01)) {
            continue;
        }
        snprintf(sExec, sizeof(sExec), "update %s set %s=%d where %s=%d;",  table, key, value, idKey, i);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_table_column_string(const char *pDbFile, const char *table, const char *key, const char  *value,
                             long long  changeFlag, const char *idKey)
{
    int i = 0;
    char sExec[512] = {0};
    int nFd = -1;
    HSQLITE hConn = 0;
    int mode = FILE_MODE_WR;

    if (!pDbFile) {
        return -1;
    }

    nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (i = 0; i < MAX_CAMERA; i++) {
        if (!(changeFlag >> i & 0x01)) {
            continue;
        }
        snprintf(sExec, sizeof(sExec), "update %s set %s=%s where %s=%d;",  table, key, value, idKey, i);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int check_msdb_database(const char *pDbFile)
{
    if (!pDbFile) {
        return -1;
    }
    if (access(pDbFile, F_OK) == -1) {
        return 1;
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
    char value[128] = {0};

    snprintf(sQuery, sizeof(sQuery), "select value from params where name='%s';", PARAM_DEVICE_SV);
    int nColumnCnt = 0;
    int nResult = sqlite_query_record_fast(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        //SQLITE_ERROR & SQLITE_CORRUPT
        if (nResult == 1 || nResult == 11) {
            return 1;
        }
        return -1;
    }
    sqlite_query_column_text(hStmt, 0, value, sizeof(value));
    sqlite_clear_stmt(hStmt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int backup_msdb_database()
{
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    if (nFd <= 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    if (access(SQLITE_FILE_NAME, F_OK) != 0) {
        msdebug(DEBUG_ERR, "backup msdb database failed.");
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char cmd[128] = {0};
    snprintf(cmd, sizeof(cmd), "cp -f %s %s", SQLITE_FILE_NAME, SQLITE_BAK_NAME);
    ms_system(cmd);
    msdebug(DEBUG_ERR, "backup msdb database success.");

    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int update_msdb_database()
{
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    if (nFd <= 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    if (access(SQLITE_FILE_NAME, F_OK) != 0
        || access(SQLITE_BAK_NAME, F_OK) != 0) {
        msdebug(DEBUG_ERR, "update msdb database failed.");
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char cmd[128] = {0};
    snprintf(cmd, sizeof(cmd), "cp -f %s %s", SQLITE_BAK_NAME, SQLITE_FILE_NAME);
    ms_system(cmd);
    msdebug(DEBUG_ERR, "update msdb database success.");

    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

//anpr db
int write_anpr_list(const char *pDbFile, const struct anpr_list *info)
{
    if (!pDbFile || !info) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_ANPR_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char stime[32] = {0};
    time_to_string_ms(stime);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "insert into lpr_list values('%s','%s', '%s');",
             stime, info->type, info->plate);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_anpr_list_replace(const char *pDbFile, const struct anpr_list *info)
{
    if (!pDbFile || !info) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_ANPR_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char stime[32] = {0};
    time_to_string_ms(stime);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "insert or replace into lpr_list values('%s','%s', '%s');",
             stime, info->type, info->plate);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_anpr_list_replace_batch(const char *pDbFile, const struct anpr_list *info, int count)
{
    if (!pDbFile || !info) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_ANPR_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char stime[32] = {0};
    time_to_string_ms(stime);
    char sExec[256] = {0};
    int i = 0;
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (i = 0; i < count; i++) {
        snprintf(sExec, sizeof(sExec), "insert or replace into lpr_list values('%s','%s', '%s');",
                 stime, info[i].type, info[i].plate);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}


int write_anpr_list_ignore(const char *pDbFile, const struct anpr_list *info)
{
    if (!pDbFile || !info) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_ANPR_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char stime[32] = {0};
    time_to_string_ms(stime);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "insert or ignore into lpr_list values('%s','%s', '%s');",
             stime, info->type, info->plate);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_anpr_lists(const char *pDbFile, const struct anpr_list *info, int count)
{
    if (!pDbFile || !info) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_ANPR_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int i = 0;
    char stime[32] = {0};
    time_to_string_ms(stime);
    char sExec[256] = {0};
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (i = 0; i < count; i++) {
        snprintf(sExec, sizeof(sExec), "insert or replace into lpr_list values('%s','%s', '%s');",
                 stime, info[i].type, info[i].plate);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

//need free *info (malloc)
int read_anpr_lists(const char *pDbFile, struct anpr_list **info, int *pCnt)
{
    if (!pDbFile || !info) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_ANPR_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = NULL;
    HSQLSTMT qStmt = NULL;
    int ret, cols, rows = 0, i = 0;
    struct anpr_list *palarm = NULL;
    char buf[256] = { 0 };

    do {
        snprintf(buf, sizeof(buf), "select count(*) from lpr_list;");
        ret = sqlite_query_record(hConn, buf, &hStmt, &cols);
        if (ret != 0 || cols == 0) {
            break;
        }
        if (sqlite_query_column(hStmt, 0, &rows) || !rows) {
            break;
        }

        snprintf(buf, sizeof(buf), "select timeId,type,plate from lpr_list;");
        ret = sqlite_query_record(hConn, buf, &qStmt, &cols);
        if (ret != 0 || cols == 0) {
            break;
        }
        if ((palarm = ms_calloc(rows, sizeof(struct anpr_list))) == NULL) {
            break;
        }
        do {
            //sqlite_query_column_text(qStmt, 0, palarm[i].timeId, sizeof(palarm[i].timeId));
            sqlite_query_column_text(qStmt, 1, palarm[i].type, sizeof(palarm[i].type));
            sqlite_query_column_text(qStmt, 2, palarm[i].plate, sizeof(palarm[i].plate));
            i++;
        } while (sqlite_query_next(qStmt) == 0 && i < rows);
        *pCnt = rows;
        *info = palarm;
    } while (0);
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
    }
    if (qStmt) {
        sqlite_clear_stmt(qStmt);
    }
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

void release_anpr_lists(struct anpr_list **info)
{
    if (!info || !*info) {
        return;
    }

    ms_free(*info);
    *info = NULL;
}


int read_anpr_list_plate(const char *pDbFile, struct anpr_list *info, const char *plate)
{
    if (!pDbFile || plate[0] == '\0') {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_ANPR_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[512] = {0};
    int nColumnCnt = 0;
    snprintf(sQuery, sizeof(sQuery), "select timeId,type,plate from lpr_list where plate='%s';", plate);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    //sqlite_query_column_text(hStmt, 0, info->timeId, sizeof(info->timeId));
    sqlite_query_column_text(hStmt, 1, info->type, sizeof(info->type));
    sqlite_query_column_text(hStmt, 2, info->plate, sizeof(info->plate));
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_anpr_list_cnt(const char *pDbFile)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_ANPR_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return 0;
    }
    HSQLSTMT hStmt = NULL;
    int ret = 0, cols = 0, cnt = 0;
    char buf[256] = { 0 };

    snprintf(buf, sizeof(buf), "select count(*) from lpr_list;");
    ret = sqlite_query_record_fast(hConn, buf, &hStmt, &cols);
    if (ret != 0 || cols == 0) {
        cnt = 0;
    } else {
        sqlite_query_column(hStmt, 0, &cnt);
    }
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
    }
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return cnt;
}

int update_anpr_list(const char *pDbFile, const struct anpr_list *info)
{
    if (!pDbFile || !info) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_ANPR_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};
    char stime[32] = {0};
    time_to_string_ms(stime);

    snprintf(sExec, sizeof(sExec), "delete from lpr_list where plate='%s';", info->plate);
    sqlite_execute(hConn, mode, sExec);

    snprintf(sExec, sizeof(sExec), "insert into lpr_list values('%s','%s','%s');",
             stime, info->type, info->plate);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int delete_anpr_list(const char *pDbFile, const struct anpr_list *info)
{
    if (!pDbFile || !info) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_ANPR_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char stime[32] = {0};
    time_to_string_ms(stime);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "delete from lpr_list where plate='%s';", info->plate);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int delete_anpr_lists(const char *pDbFile, const struct anpr_list *info, int count)
{
    if (!pDbFile || !info) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_ANPR_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    int i = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    for (i = 0; i < count; i++) {
        snprintf(sExec, sizeof(sExec), "delete from lpr_list where plate='%s';", info[i].plate);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_anpr_effective_schedule(const char *pDbFile, struct smart_event_schedule  *Schedule, int chn_id,
                                 ANPR_MODE_TYPE type)
{
    if (!pDbFile || !Schedule) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[256] = {0};

    HSQLSTMT hStmt = 0;

    if (type == ANPR_ENABLE) {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_enable_effective_schedule where chn_id=%d;", chn_id);
    } else if (type == ANPR_BLACK) {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_black_mode_effective_schedule where chn_id=%d;",
                 chn_id);
    } else if (type == ANPR_WHITE) {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_white_mode_effective_schedule where chn_id=%d;",
                 chn_id);
    } else if (type == ANPR_VISTOR) {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_vistor_mode_effective_schedule where chn_id=%d;",
                 chn_id);
    } else {
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, Schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, Schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_anpr_effective_schedules(const char *pDbFile, struct smart_event_schedule *Schedule, int chn_id)
{
    if (!pDbFile || !Schedule) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_ANPR_MODE][256];

    HSQLSTMT hStmt = 0;
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
            sqlite_query_column_text(hStmt, 2, Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
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
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_anpr_effective_schedule(const char *pDbFile, struct smart_event_schedule *schedule, int chn_id,
                                  ANPR_MODE_TYPE type)
{
    if (!pDbFile || !schedule || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    char sExec[256] = {0};
    char db_table[64] = {0};
    if (type == ANPR_ENABLE) {
        snprintf(db_table, sizeof(db_table), "%s", "lpr_enable_effective_schedule");
    } else if (type == ANPR_BLACK) {
        snprintf(db_table, sizeof(db_table), "%s", "lpr_black_mode_effective_schedule");
    } else if (type == ANPR_WHITE) {
        snprintf(db_table, sizeof(db_table), "%s", "lpr_white_mode_effective_schedule");
    } else if (type == ANPR_VISTOR) {
        snprintf(db_table, sizeof(db_table), "%s", "lpr_vistor_mode_effective_schedule");
    } else {
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    snprintf(sExec, sizeof(sExec), "delete from %s where chn_id=%d;", db_table, chn_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into %s values(%d,%d,%d, '%s', '%s', %d);", db_table, chn_id, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_anpr_audible_schedule(const char *pDbFile, struct smart_event_schedule  *Schedule, int chn_id,
                               ANPR_MODE_TYPE type)
{
    if (!pDbFile || !Schedule) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[256] = {0};

    HSQLSTMT hStmt = 0;

    if (type == ANPR_BLACK) {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_black_mode_audible_schedule where chn_id=%d;", chn_id);
    } else if (type == ANPR_WHITE) {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_white_mode_audible_schedule where chn_id=%d;", chn_id);
    } else if (type == ANPR_VISTOR) {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_vistor_mode_audible_schedule where chn_id=%d;",
                 chn_id);
    } else {
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, Schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, Schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_anpr_audible_schedules(const char *pDbFile, struct smart_event_schedule *Schedule, int chn_id)
{
    if (!pDbFile || !Schedule) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_ANPR_MODE][256];

    HSQLSTMT hStmt = 0;
    snprintf(sQuery[ANPR_BLACK], sizeof(sQuery[ANPR_BLACK]),
             "select week_id,plan_id,start_time,end_time,action_type from lpr_black_mode_audible_schedule where chn_id=%d;", chn_id);
    snprintf(sQuery[ANPR_WHITE], sizeof(sQuery[ANPR_WHITE]),
             "select week_id,plan_id,start_time,end_time,action_type from lpr_write_mode_audible_schedule where chn_id=%d;", chn_id);
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
            sqlite_query_column_text(hStmt, 2, Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
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
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_anpr_audible_schedule(const char *pDbFile, struct smart_event_schedule *schedule, int chn_id,
                                ANPR_MODE_TYPE type)
{
    if (!pDbFile || !schedule || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    char sExec[256] = {0};
    char db_table[64] = {0};
    if (type == ANPR_BLACK) {
        snprintf(db_table, sizeof(db_table), "%s", "lpr_black_mode_audible_schedule");
    } else if (type == ANPR_WHITE) {
        snprintf(db_table, sizeof(db_table), "%s", "lpr_white_mode_audible_schedule");
    } else if (type == ANPR_VISTOR) {
        snprintf(db_table, sizeof(db_table), "%s", "lpr_vistor_mode_audible_schedule");
    } else {
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    snprintf(sExec, sizeof(sExec), "delete from %s where chn_id=%d;", db_table, chn_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into %s values(%d,%d,%d, '%s', '%s', %d);", db_table, chn_id, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_anpr_mail_schedule(const char *pDbFile, struct smart_event_schedule  *Schedule, int chn_id,
                            ANPR_MODE_TYPE type)
{
    if (!pDbFile || !Schedule) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[256] = {0};

    HSQLSTMT hStmt = 0;

    if (type == ANPR_BLACK) {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_black_mode_mail_schedule where chn_id=%d;", chn_id);
    } else if (type == ANPR_WHITE) {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_white_mode_mail_schedule where chn_id=%d;", chn_id);
    } else if (type == ANPR_VISTOR) {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_vistor_mode_mail_schedule where chn_id=%d;", chn_id);
    } else {
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, Schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, Schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_anpr_mail_schedules(const char *pDbFile, struct smart_event_schedule *Schedule, int chn_id)
{
    if (!pDbFile || !Schedule) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_ANPR_MODE][256];

    HSQLSTMT hStmt = 0;
    snprintf(sQuery[ANPR_BLACK], sizeof(sQuery[ANPR_BLACK]),
             "select week_id,plan_id,start_time,end_time,action_type from lpr_black_mode_mail_schedule where chn_id=%d;", chn_id);
    snprintf(sQuery[ANPR_WHITE], sizeof(sQuery[ANPR_WHITE]),
             "select week_id,plan_id,start_time,end_time,action_type from lpr_write_mode_mail_schedule where chn_id=%d;", chn_id);
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
            sqlite_query_column_text(hStmt, 2, Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
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
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_anpr_mail_schedule(const char *pDbFile, struct smart_event_schedule *schedule, int chn_id,
                             ANPR_MODE_TYPE type)
{
    if (!pDbFile || !schedule || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    char sExec[256] = {0};
    char db_table[64] = {0};
    if (type == ANPR_BLACK) {
        snprintf(db_table, sizeof(db_table), "%s", "lpr_black_mode_mail_schedule");
    } else if (type == ANPR_WHITE) {
        snprintf(db_table, sizeof(db_table), "%s", "lpr_white_mode_mail_schedule");
    } else if (type == ANPR_VISTOR) {
        snprintf(db_table, sizeof(db_table), "%s", "lpr_vistor_mode_mail_schedule");
    } else {
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    snprintf(sExec, sizeof(sExec), "delete from %s where chn_id=%d;", db_table, chn_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into %s values(%d,%d,%d, '%s', '%s', %d);", db_table, chn_id, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_anpr_popup_schedule(const char *pDbFile, struct smart_event_schedule  *Schedule, int chn_id,
                             ANPR_MODE_TYPE type)
{
    if (!pDbFile || !Schedule) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[256] = {0};

    HSQLSTMT hStmt = 0;

    if (type == ANPR_BLACK) {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_black_mode_popup_schedule where chn_id=%d;", chn_id);
    } else if (type == ANPR_WHITE) {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_white_mode_popup_schedule where chn_id=%d;", chn_id);
    } else if (type == ANPR_VISTOR) {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_vistor_mode_popup_schedule where chn_id=%d;", chn_id);
    } else {
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, Schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, Schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_anpr_popup_schedules(const char *pDbFile, struct smart_event_schedule *Schedule, int chn_id)
{
    if (!pDbFile || !Schedule) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_ANPR_MODE][256];

    HSQLSTMT hStmt = 0;
    snprintf(sQuery[ANPR_BLACK], sizeof(sQuery[ANPR_BLACK]),
             "select week_id,plan_id,start_time,end_time,action_type from lpr_black_mode_popup_schedule where chn_id=%d;", chn_id);
    snprintf(sQuery[ANPR_WHITE], sizeof(sQuery[ANPR_WHITE]),
             "select week_id,plan_id,start_time,end_time,action_type from lpr_write_mode_popup_schedule where chn_id=%d;", chn_id);
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
            sqlite_query_column_text(hStmt, 2, Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
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
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_anpr_popup_schedule(const char *pDbFile, struct smart_event_schedule *schedule, int chn_id,
                              ANPR_MODE_TYPE type)
{
    if (!pDbFile || !schedule || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    char sExec[256] = {0};
    char db_table[64] = {0};
    if (type == ANPR_BLACK) {
        snprintf(db_table, sizeof(db_table), "%s", "lpr_black_mode_popup_schedule");
    } else if (type == ANPR_WHITE) {
        snprintf(db_table, sizeof(db_table), "%s", "lpr_white_mode_popup_schedule");
    } else if (type == ANPR_VISTOR) {
        snprintf(db_table, sizeof(db_table), "%s", "lpr_vistor_mode_popup_schedule");
    } else {
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    snprintf(sExec, sizeof(sExec), "delete from %s where chn_id=%d;", db_table, chn_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into %s values(%d,%d,%d, '%s', '%s', %d);", db_table, chn_id, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_anpr_ptz_schedule(const char *pDbFile, struct smart_event_schedule  *Schedule, int chn_id, ANPR_MODE_TYPE type)
{
    if (!pDbFile || !Schedule) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[256] = {0};

    HSQLSTMT hStmt = 0;

    if (type == ANPR_BLACK) {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_black_mode_ptz_schedule where chn_id=%d;", chn_id);
    } else if (type == ANPR_WHITE) {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_white_mode_ptz_schedule where chn_id=%d;", chn_id);
    } else if (type == ANPR_VISTOR) {
        snprintf(sQuery, sizeof(sQuery),
                 "select week_id,plan_id,start_time,end_time,action_type from lpr_vistor_mode_ptz_schedule where chn_id=%d;", chn_id);
    } else {
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, Schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, Schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_anpr_ptz_schedules(const char *pDbFile, struct smart_event_schedule *Schedule, int chn_id)
{
    if (!pDbFile || !Schedule) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_ANPR_MODE][256];

    HSQLSTMT hStmt = 0;
    snprintf(sQuery[ANPR_BLACK], sizeof(sQuery[ANPR_BLACK]),
             "select week_id,plan_id,start_time,end_time,action_type from lpr_black_mode_ptz_schedule where chn_id=%d;", chn_id);
    snprintf(sQuery[ANPR_WHITE], sizeof(sQuery[ANPR_WHITE]),
             "select week_id,plan_id,start_time,end_time,action_type from lpr_write_mode_ptz_schedule where chn_id=%d;", chn_id);
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
            sqlite_query_column_text(hStmt, 2, Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
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
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_anpr_ptz_schedule(const char *pDbFile, struct smart_event_schedule *schedule, int chn_id, ANPR_MODE_TYPE type)
{
    if (!pDbFile || !schedule || chn_id < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    char sExec[256] = {0};
    char db_table[64] = {0};
    if (type == ANPR_BLACK) {
        snprintf(db_table, sizeof(db_table), "%s", "lpr_black_mode_ptz_schedule");
    } else if (type == ANPR_WHITE) {
        snprintf(db_table, sizeof(db_table), "%s", "lpr_white_mode_ptz_schedule");
    } else if (type == ANPR_VISTOR) {
        snprintf(db_table, sizeof(db_table), "%s", "lpr_vistor_mode_ptz_schedule");
    } else {
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    snprintf(sExec, sizeof(sExec), "delete from %s where chn_id=%d;", db_table, chn_id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into %s values(%d,%d,%d, '%s', '%s', %d);", db_table, chn_id, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_anpr_event(const char *pDbFile, struct smart_event *smartevent, int id, ANPR_MODE_TYPE type)
{
    char pDbtable[64] = {0};
    if (type == ANPR_BLACK) {
        snprintf(pDbtable, sizeof(pDbtable), "%s", "lpr_black_mode_event");
    } else if (type == ANPR_WHITE) {
        snprintf(pDbtable, sizeof(pDbtable), "%s", "lpr_white_mode_event");
    } else if (type == ANPR_VISTOR) {
        snprintf(pDbtable, sizeof(pDbtable), "%s", "lpr_vistor_mode_event");
    } else {
        return -1;
    }

    return read_event(pDbFile, smartevent, pDbtable, id);
}

int read_anpr_events(const char *pDbFile, struct smart_event *smartevent, int chnid)
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
    int i = 0;
    HSQLSTMT hStmt = 0;
    char sQuery[MAX_ANPR_MODE][2048];
    int nColumnCnt = 0;

    snprintf(sQuery[ANPR_BLACK], sizeof(sQuery[ANPR_BLACK]),
             "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,email_interval,\
		tri_chnout1_alarms,tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval,email_pic_enable,\
		tri_channels_pic,tri_audio_id,http_notification_interval from lpr_black_mode_event where id=%d;", chnid);
    snprintf(sQuery[ANPR_WHITE], sizeof(sQuery[ANPR_WHITE]),
             "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,email_interval,\
		tri_chnout1_alarms,tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval,email_pic_enable,\
		tri_channels_pic,tri_audio_id,http_notification_interval from lpr_white_mode_event where id=%d;", chnid);
    snprintf(sQuery[ANPR_VISTOR], sizeof(sQuery[ANPR_VISTOR]),
             "select id,enable,tri_alarms,tri_channels_ex,buzzer_interval,email_interval,\
		tri_chnout1_alarms,tri_chnout2_alarms,popup_interval,ptzaction_interval,alarmout_interval,whiteled_interval,email_pic_enable,\
		tri_channels_pic,tri_audio_id,http_notification_interval from lpr_vistor_mode_event where id=%d;", chnid);

    for (i = 0; i < MAX_ANPR_MODE; i++) {
        int nResult = sqlite_query_record(hConn, sQuery[i], &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
            continue;
        }

        sqlite_query_column(hStmt, 0, &smartevent[i].id);
        sqlite_query_column(hStmt, 1, &smartevent[i].enable);
        sqlite_query_column(hStmt, 2, (int *)&smartevent[i].tri_alarms);
        sqlite_query_column_text(hStmt, 3, smartevent[i].tri_channels_ex, sizeof(smartevent[i].tri_channels_ex));
        sqlite_query_column(hStmt, 4, &smartevent[i].buzzer_interval);
        sqlite_query_column(hStmt, 5, &smartevent[i].email_interval);
        sqlite_query_column_text(hStmt, 6, smartevent[i].tri_chnout1_alarms, sizeof(smartevent[i].tri_chnout1_alarms));
        sqlite_query_column_text(hStmt, 7, smartevent[i].tri_chnout2_alarms, sizeof(smartevent[i].tri_chnout2_alarms));
        sqlite_query_column(hStmt, 8, &smartevent[i].popup_interval);
        sqlite_query_column(hStmt, 9, &smartevent[i].ptzaction_interval);
        sqlite_query_column(hStmt, 10, &smartevent[i].alarmout_interval);
        sqlite_query_column(hStmt, 11, &smartevent[i].whiteled_interval);
        sqlite_query_column(hStmt, 12, &smartevent[i].email_pic_enable);
        sqlite_query_column_text(hStmt, 13, smartevent[i].tri_channels_pic, sizeof(smartevent[i].tri_channels_pic));
        sqlite_query_column(hStmt, 14, &smartevent[i].tri_audio_id);
        sqlite_query_column(hStmt, 15, &smartevent[i].http_notification_interval);

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

int write_anpr_event(const char *pDbFile, struct smart_event *smartevent, ANPR_MODE_TYPE type)
{
    if (!pDbFile || !smartevent) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[2048] = {0};
    if (type == ANPR_BLACK) {
        snprintf(sExec, sizeof(sExec), "update lpr_black_mode_event set enable=%d, tri_alarms=%d, tri_channels_ex='%s', \
			buzzer_interval='%d', email_interval='%d',tri_chnout1_alarms='%s',tri_chnout2_alarms='%s',popup_interval=%d,\
			ptzaction_interval=%d,alarmout_interval=%d,whiteled_interval=%d,email_pic_enable=%d,tri_channels_pic='%s',\
			tri_audio_id=%d,http_notification_interval=%d where id=%d;",
                 smartevent->enable, smartevent->tri_alarms, smartevent->tri_channels_ex, smartevent->buzzer_interval,
                 smartevent->email_interval, smartevent->tri_chnout1_alarms, smartevent->tri_chnout2_alarms,
                 smartevent->popup_interval, smartevent->ptzaction_interval, smartevent->alarmout_interval,
                 smartevent->whiteled_interval, smartevent->email_pic_enable, smartevent->tri_channels_pic,
                 smartevent->tri_audio_id, smartevent->http_notification_interval, smartevent->id);
    } else if (type == ANPR_WHITE) {
        snprintf(sExec, sizeof(sExec), "update lpr_white_mode_event set enable=%d, tri_alarms=%d, tri_channels_ex='%s', \
			buzzer_interval='%d', email_interval='%d',tri_chnout1_alarms='%s',tri_chnout2_alarms='%s',popup_interval=%d,\
			ptzaction_interval=%d,alarmout_interval=%d,whiteled_interval=%d,email_pic_enable=%d,tri_channels_pic='%s',\
			tri_audio_id=%d,http_notification_interval=%d where id=%d;",
                 smartevent->enable, smartevent->tri_alarms, smartevent->tri_channels_ex, smartevent->buzzer_interval,
                 smartevent->email_interval, smartevent->tri_chnout1_alarms, smartevent->tri_chnout2_alarms,
                 smartevent->popup_interval, smartevent->ptzaction_interval, smartevent->alarmout_interval,
                 smartevent->whiteled_interval, smartevent->email_pic_enable, smartevent->tri_channels_pic,
                 smartevent->tri_audio_id, smartevent->http_notification_interval, smartevent->id);
    } else if (type == ANPR_VISTOR) {
        snprintf(sExec, sizeof(sExec), "update lpr_vistor_mode_event set enable=%d, tri_alarms=%d, tri_channels_ex='%s', \
			buzzer_interval='%d', email_interval='%d',tri_chnout1_alarms='%s',tri_chnout2_alarms='%s',popup_interval=%d,\
			ptzaction_interval=%d,alarmout_interval=%d,whiteled_interval=%d,email_pic_enable=%d,tri_channels_pic='%s',\
			tri_audio_id=%d,http_notification_interval=%d where id=%d;",
                 smartevent->enable, smartevent->tri_alarms, smartevent->tri_channels_ex, smartevent->buzzer_interval,
                 smartevent->email_interval, smartevent->tri_chnout1_alarms, smartevent->tri_chnout2_alarms,
                 smartevent->popup_interval, smartevent->ptzaction_interval, smartevent->alarmout_interval,
                 smartevent->whiteled_interval, smartevent->email_pic_enable, smartevent->tri_channels_pic,
                 smartevent->tri_audio_id, smartevent->http_notification_interval, smartevent->id);
    } else {
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_anpr_ptz_params(const char *pDbFile, struct ptz_action_params ptzActionParams[], ANPR_MODE_TYPE type,
                         int chn_id, int *cnt)
{
    if (!pDbFile || !ptzActionParams || type < 0 || chn_id < 0) {
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
    int i = 0;
    switch (type) {
        case ANPR_BLACK:
            snprintf(sQuery, sizeof(sQuery),
                     "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from lpr_black_mode_ptz_params where chn_id=%d;",
                     chn_id);
            break;
        case ANPR_WHITE:
            snprintf(sQuery, sizeof(sQuery),
                     "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from lpr_white_mode_ptz_params where chn_id=%d;",
                     chn_id);
            break;
        case ANPR_VISTOR:
            snprintf(sQuery, sizeof(sQuery),
                     "select chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from lpr_vistor_mode_ptz_params where chn_id=%d;",
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
        *cnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < MAX_REAL_CAMERA; i++) {
        sqlite_query_column(hStmt, 0, &ptzActionParams[i].chn_id);
        sqlite_query_column(hStmt, 1, &ptzActionParams[i].acto_ptz_channel);
        sqlite_query_column(hStmt, 2, &ptzActionParams[i].acto_fish_channel);
        sqlite_query_column(hStmt, 3, &ptzActionParams[i].acto_ptz_type);
        sqlite_query_column(hStmt, 4, &ptzActionParams[i].acto_ptz_preset);
        sqlite_query_column(hStmt, 5, &ptzActionParams[i].acto_ptz_patrol);
        sqlite_query_column(hStmt, 6, &ptzActionParams[i].acto_ptz_pattern);

        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    *cnt  = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_anpr_ptz_params(const char *pDbFile, struct ptz_action_params *ptzActionParams, ANPR_MODE_TYPE type)
{
    if (!pDbFile || !ptzActionParams || type < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    char sExec[2048] = {0};
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    switch (type) {
        case ANPR_BLACK:
            snprintf(sExec, sizeof(sExec), "delete from lpr_black_mode_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            sqlite_execute(hConn, mode, sExec);
            snprintf(sExec, sizeof(sExec),
                     "insert into lpr_black_mode_ptz_params(chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern) values(%d,%d,%d,%d,%d,%d,%d);",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel, ptzActionParams->acto_fish_channel,
                     ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset, ptzActionParams->acto_ptz_patrol,
                     ptzActionParams->acto_ptz_pattern);
            break;
        case ANPR_WHITE:
            snprintf(sExec, sizeof(sExec), "delete from lpr_white_mode_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            sqlite_execute(hConn, mode, sExec);
            snprintf(sExec, sizeof(sExec),
                     "insert into lpr_white_mode_ptz_params(chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern) values(%d,%d,%d,%d,%d,%d,%d);",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel, ptzActionParams->acto_fish_channel,
                     ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset, ptzActionParams->acto_ptz_patrol,
                     ptzActionParams->acto_ptz_pattern);
            break;
        case ANPR_VISTOR:
            snprintf(sExec, sizeof(sExec), "delete from lpr_vistor_mode_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            sqlite_execute(hConn, mode, sExec);
            snprintf(sExec, sizeof(sExec),
                     "insert into lpr_vistor_mode_ptz_params(chn_id,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern) values(%d,%d,%d,%d,%d,%d,%d);",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel, ptzActionParams->acto_fish_channel,
                     ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset, ptzActionParams->acto_ptz_patrol,
                     ptzActionParams->acto_ptz_pattern);
            break;
        default:
            break;
    }
    if (sExec[0] == '\0') {
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int update_anpr_ptz_params(const char *pDbFile, struct ptz_action_params *ptzActionParams, ANPR_MODE_TYPE type,
                           int oldChan)
{
    if (!pDbFile || !ptzActionParams || type < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    char sExec[2048] = {0};
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    switch (type) {
        case ANPR_BLACK:
            snprintf(sExec, sizeof(sExec),
                     "update lpr_black_mode_ptz_params set acto_fish_channel=%d,acto_ptz_type=%d,acto_ptz_preset=%d,acto_ptz_patrol=%d,acto_ptz_pattern=%d,acto_ptz_channel=%d where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->acto_fish_channel, ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset,
                     ptzActionParams->acto_ptz_patrol, ptzActionParams->acto_ptz_pattern, ptzActionParams->acto_ptz_channel,
                     ptzActionParams->chn_id, oldChan);
            break;
        case ANPR_WHITE:
            snprintf(sExec, sizeof(sExec),
                     "update lpr_white_mode_ptz_params set acto_fish_channel=%d,acto_ptz_type=%d,acto_ptz_preset=%d,acto_ptz_patrol=%d,acto_ptz_pattern=%d,acto_ptz_channel=%d where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->acto_fish_channel, ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset,
                     ptzActionParams->acto_ptz_patrol, ptzActionParams->acto_ptz_pattern, ptzActionParams->acto_ptz_channel,
                     ptzActionParams->chn_id, oldChan);
            break;
        case ANPR_VISTOR:
            snprintf(sExec, sizeof(sExec),
                     "update lpr_vistor_mode_ptz_params set acto_fish_channel=%d,acto_ptz_type=%d,acto_ptz_preset=%d,acto_ptz_patrol=%d,acto_ptz_pattern=%d,acto_ptz_channel=%d where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->acto_fish_channel, ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset,
                     ptzActionParams->acto_ptz_patrol, ptzActionParams->acto_ptz_pattern, ptzActionParams->acto_ptz_channel,
                     ptzActionParams->chn_id, oldChan);
            break;
        default:
            break;
    }
    if (sExec[0] == '\0') {
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int delete_anpr_ptz_params(const char *pDbFile, struct ptz_action_params *ptzActionParams, ANPR_MODE_TYPE type)
{
    if (!pDbFile || !ptzActionParams || type < 0) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    char sExec[2048] = {0};
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    switch (type) {
        case ANPR_BLACK:
            snprintf(sExec, sizeof(sExec), "delete from lpr_black_mode_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            break;
        case ANPR_WHITE:
            snprintf(sExec, sizeof(sExec), "delete from lpr_white_mode_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            break;
        case ANPR_VISTOR:
            snprintf(sExec, sizeof(sExec), "delete from lpr_vistor_mode_ptz_params where chn_id=%d and acto_ptz_channel=%d;",
                     ptzActionParams->chn_id, ptzActionParams->acto_ptz_channel);
            break;
        default:
            break;
    }
    if (sExec[0] == '\0') {
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_alarm_chnIn_effective_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, struct alarm_chn *info)
{
    char pDbtable[64] = {0};
    snprintf(pDbtable, sizeof(pDbtable), "alarm_chnIn%d_effective_schedule", info->alarmid);

    return read_schedule(pDbFile, schedule, pDbtable, info->chnid);
}

int write_alarm_chnIn_effective_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, struct alarm_chn *info)
{
    char pDbtable[64] = {0};
    snprintf(pDbtable, sizeof(pDbtable), "alarm_chnIn%d_effective_schedule", info->alarmid);

    return write_schedule(pDbFile, schedule, pDbtable, info->chnid);
}

int copy_alarm_chnIn_effective_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int alarmid,
                                         long long changeFlag)
{
    char pDbtable[64] = {0};
    snprintf(pDbtable, sizeof(pDbtable), "alarm_chnIn%d_effective_schedule", alarmid);

    return copy_schedules(pDbFile, schedule, pDbtable, changeFlag);
}


int read_alarm_chnIn_audible_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, struct alarm_chn *info)
{
    char pDbtable[64] = {0};
    snprintf(pDbtable, sizeof(pDbtable), "alarm_chnIn%d_audible_schedule", info->alarmid);

    return read_schedule(pDbFile, schedule, pDbtable, info->chnid);
}

int write_alarm_chnIn_audible_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, struct alarm_chn *info)
{
    char pDbtable[64] = {0};
    snprintf(pDbtable, sizeof(pDbtable), "alarm_chnIn%d_audible_schedule", info->alarmid);

    return write_schedule(pDbFile, schedule, pDbtable, info->chnid);
}

int copy_alarm_chnIn_audible_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int alarmid, long long changeFlag)
{
    char pDbtable[64] = {0};
    snprintf(pDbtable, sizeof(pDbtable), "alarm_chnIn%d_audible_schedule", alarmid);

    return copy_schedules(pDbFile, schedule, pDbtable, changeFlag);
}


int read_alarm_chnIn_mail_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, struct alarm_chn *info)
{
    char pDbtable[64] = {0};
    snprintf(pDbtable, sizeof(pDbtable), "alarm_chnIn%d_mail_schedule", info->alarmid);

    return read_schedule(pDbFile, schedule, pDbtable, info->chnid);
}

int write_alarm_chnIn_mail_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, struct alarm_chn *info)
{
    char pDbtable[64] = {0};
    snprintf(pDbtable, sizeof(pDbtable), "alarm_chnIn%d_mail_schedule", info->alarmid);

    return write_schedule(pDbFile, schedule, pDbtable, info->chnid);
}

int copy_alarm_chnIn_mail_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int alarmid, long long changeFlag)
{
    char pDbtable[64] = {0};
    snprintf(pDbtable, sizeof(pDbtable), "alarm_chnIn%d_mail_schedule", alarmid);

    return copy_schedules(pDbFile, schedule, pDbtable, changeFlag);
}


int read_alarm_chnIn_popup_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, struct alarm_chn *info)
{
    char pDbtable[64] = {0};
    snprintf(pDbtable, sizeof(pDbtable), "alarm_chnIn%d_popup_schedule", info->alarmid);

    return read_schedule(pDbFile, schedule, pDbtable, info->chnid);
}

int write_alarm_chnIn_popup_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, struct alarm_chn *info)
{
    char pDbtable[64] = {0};
    snprintf(pDbtable, sizeof(pDbtable), "alarm_chnIn%d_popup_schedule", info->alarmid);

    return write_schedule(pDbFile, schedule, pDbtable, info->chnid);
}

int copy_alarm_chnIn_popup_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int alarmid, Uint64 changeFlag)
{
    char pDbtable[64] = {0};
    snprintf(pDbtable, sizeof(pDbtable), "alarm_chnIn%d_popup_schedule", alarmid);

    return copy_schedules(pDbFile, schedule, pDbtable, changeFlag);
}

int read_alarm_chnIn_ptz_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, struct alarm_chn *info)
{
    char pDbtable[64] = {0};
    snprintf(pDbtable, sizeof(pDbtable), "alarm_chnIn%d_ptz_schedule", info->alarmid);

    return read_schedule(pDbFile, schedule, pDbtable, info->chnid);
}

int write_alarm_chnIn_ptz_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, struct alarm_chn *info)
{
    char pDbtable[64] = {0};
    snprintf(pDbtable, sizeof(pDbtable), "alarm_chnIn%d_ptz_schedule", info->alarmid);

    return write_schedule(pDbFile, schedule, pDbtable, info->chnid);
}

int copy_alarm_chnIn_ptz_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int alarmid, long long changeFlag)
{
    char pDbtable[64] = {0};
    snprintf(pDbtable, sizeof(pDbtable), "alarm_chnIn%d_ptz_schedule", alarmid);

    return copy_schedules(pDbFile, schedule, pDbtable, changeFlag);
}


int read_alarm_chnIn_event(const char *pDbFile, SMART_EVENT *smartevent, struct alarm_chn *info)
{
    char pDbtable[64] = {0};
    snprintf(pDbtable, sizeof(pDbtable), "alarm_chnIn%d_event", info->alarmid);

    return read_event(pDbFile, smartevent, pDbtable, info->chnid);
}

int read_alarm_chnIn_events(const char *pDbFile, SMART_EVENT *smartevent, int count, struct alarm_chn *info)
{
    char pDbtable[64] = {0};
    snprintf(pDbtable, sizeof(pDbtable), "alarm_chnIn%d_event", info->alarmid);

    return read_events(pDbFile, smartevent, pDbtable, count);
}


int write_alarm_chnIn_event(const char *pDbFile, SMART_EVENT *smartevent, struct alarm_chn *info)
{
    char pDbtable[64] = {0};
    snprintf(pDbtable, sizeof(pDbtable), "alarm_chnIn%d_event", info->alarmid);

    return write_event(pDbFile, smartevent, pDbtable);
}

int write_alarm_chnIn_events(const char *pDbFile, SMART_EVENT *smartevent, struct alarm_chn *info, int count,
                             long long changeFlag)
{
    char pDbtable[64] = {0};
    snprintf(pDbtable, sizeof(pDbtable), "alarm_chnIn%d_event", info->alarmid);

    return write_events(pDbFile, smartevent, pDbtable, count, changeFlag);
}

int copy_alarm_chnIn_events(const char *pDbFile, SMART_EVENT *smartevent, int alarmid, long long changeFlag)
{
    char pDbtable[64] = {0};
    snprintf(pDbtable, sizeof(pDbtable), "alarm_chnIn%d_event", alarmid);

    return copy_events(pDbFile, smartevent, pDbtable, (Uint64)changeFlag);
}

int read_alarm_chnIn_event_name(const char *pDbFile, struct alarm_chn_name *param, struct alarm_chn *info)
{
    int ret = -1;
    char pDbtable[64] = {0};
    
    snprintf(pDbtable, sizeof(pDbtable), "alarm_chnIn%d_event", info->alarmid);
    
    ret = read_event_name(pDbFile, param, pDbtable, info->chnid);
    param->alarmid = info->alarmid;

    return ret;
}

int write_alarm_chnIn_event_name(const char *pDbFile, struct alarm_chn_name *param)
{
    int ret = -1;
    char pDbtable[64] = {0};

    snprintf(pDbtable, sizeof(pDbtable), "alarm_chnIn%d_event", param->alarmid);
    
    ret = write_event_name(pDbFile, param, pDbtable);

    return ret;
}

int clear_alarm_chnIn_event_name(const char *pDbFile, int chnid)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[2048] = {0};
    int i;
    for (i = 0; i < MAX_IPC_ALARM_IN; ++i) {
        snprintf(sExec, sizeof(sExec), "update alarm_chnIn%d_event set name='' where id=%d;", i, chnid);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);

    return 0;
}

int read_alarm_chnIn_ptz_params(const char *pDbFile, struct ptz_action_params ptzActionParams[], struct alarm_chn *info,
                                int *cnt)
{
    char pDbtable[64] = {0};
    snprintf(pDbtable, sizeof(pDbtable), "alarm_chnIn%d_ptz_params", info->alarmid);

    return read_action_ptz_params(pDbFile, ptzActionParams, pDbtable, info->chnid, cnt);
}

int write_alarm_chnIn_ptz_params(const char *pDbFile, struct ptz_action_params *ptzActionParams, struct alarm_chn *info)
{
    char pDbtable[64] = {0};
    snprintf(pDbtable, sizeof(pDbtable), "alarm_chnIn%d_ptz_params", info->alarmid);

    return write_action_ptz_params(pDbFile, ptzActionParams, pDbtable);
}

int update_alarm_chnIn_ptz_params(const char *pDbFile, struct ptz_action_params *ptzActionParams,
                                  struct alarm_chn *info)
{
    char pDbtable[64] = {0};
    snprintf(pDbtable, sizeof(pDbtable), "alarm_chnIn%d_ptz_params", info->alarmid);

    return update_action_ptz_params(pDbFile, ptzActionParams, pDbtable, info->chnid);
}

int delete_alarm_chnIn_ptz_params(const char *pDbFile, struct ptz_action_params *ptzActionParams,
                                  struct alarm_chn *info)
{
    char pDbtable[64] = {0};
    snprintf(pDbtable, sizeof(pDbtable), "alarm_chnIn%d_ptz_params", info->alarmid);

    return delete_action_ptz_params(pDbFile, ptzActionParams, pDbtable);
}

int read_alarm_chnOut_effective_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, struct alarm_chn *info)
{
    char pDbtable[64] = {0};
    if (info->alarmid == 0) {
        snprintf(pDbtable, sizeof(pDbtable), "%s", "alarm_chnOut0_effective_schedule");
    } else {
        snprintf(pDbtable, sizeof(pDbtable), "%s", "alarm_chnOut1_effective_schedule");
    }

    return read_schedule(pDbFile, schedule, pDbtable, info->chnid);
}

int write_alarm_chnOut_effective_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, struct alarm_chn *info)
{
    char pDbtable[64] = {0};
    if (info->alarmid == 0) {
        snprintf(pDbtable, sizeof(pDbtable), "%s", "alarm_chnOut0_effective_schedule");
    } else {
        snprintf(pDbtable, sizeof(pDbtable), "%s", "alarm_chnOut1_effective_schedule");
    }

    return write_schedule(pDbFile, schedule, pDbtable, info->chnid);
}

int copy_alarm_chnOut_effective_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, struct alarm_chn *info,
                                          long long changeFlag)
{
    char pDbtable[64] = {0};
    if (info->alarmid == 0) {
        snprintf(pDbtable, sizeof(pDbtable), "%s", "alarm_chnOut0_effective_schedule");
    } else {
        snprintf(pDbtable, sizeof(pDbtable), "%s", "alarm_chnOut1_effective_schedule");
    }

    return copy_schedules(pDbFile, schedule, pDbtable, changeFlag);
}


int read_alarm_chnOut_event_name(const char *pDbFile, struct alarm_chn_out_name *param, struct alarm_chn *info)
{
    char pDbtable[64] = {0};
    if (info->alarmid == 0) {
        snprintf(pDbtable, sizeof(pDbtable), "%s", "alarm_chnOut0_event");
    } else {
        snprintf(pDbtable, sizeof(pDbtable), "%s", "alarm_chnOut1_event");
    }

    if (!pDbFile || !param) {
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

    snprintf(sQuery, sizeof(sQuery), "select id,name,time from %s where id=%d;", pDbtable, info->chnid);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, &param->chnid);
    sqlite_query_column_text(hStmt, 1, param->name, sizeof(param->name));
    sqlite_query_column(hStmt, 2, &param->delay_time);
    param->alarmid = info->alarmid;

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);

    return 0;
}

int read_alarm_chnOut_event_names(const char *pDbFile, struct alarm_chn_out_name *param, int count, int alarmid)
{
    char pDbtable[64] = {0};
    if (alarmid == 0) {
        snprintf(pDbtable, sizeof(pDbtable), "%s", "alarm_chnOut0_event");
    } else {
        snprintf(pDbtable, sizeof(pDbtable), "%s", "alarm_chnOut1_event");
    }

    if (!pDbFile || !param || count <= 0) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery), "select id,name,time from %s;", pDbtable);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < MAX_CAMERA && i < count; i++) {
        sqlite_query_column(hStmt, 0, &param[i].chnid);
        sqlite_query_column_text(hStmt, 1, param[i].name, sizeof(param[i].name));
        sqlite_query_column(hStmt, 2, &param[i].delay_time);
        param[i].alarmid = alarmid;
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_alarm_chnOut_event_name(const char *pDbFile, struct alarm_chn_out_name *param)
{
    if (!pDbFile || !param) {
        return -1;
    }
    char pDbtable[64] = {0};
    if (param->alarmid == 0) {
        snprintf(pDbtable, sizeof(pDbtable), "%s", "alarm_chnOut0_event");
    } else {
        snprintf(pDbtable, sizeof(pDbtable), "%s", "alarm_chnOut1_event");
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[2048] = {0};
    snprintf(sExec, sizeof(sExec), "update %s set name='%s', time=%d where id=%d;", pDbtable, param->name,
             param->delay_time, param->chnid);

    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);

    return 0;
}

int write_alarm_chnOut_event_names(const char *pDbFile, struct alarm_chn_out_name *param, int count,
                                   long long changeFlag)
{
    if (!pDbFile || !param) {
        return -1;
    }
    char pDbtable[64] = {0};
    if (param->alarmid == 0) {
        snprintf(pDbtable, sizeof(pDbtable), "%s", "alarm_chnOut0_event");
    } else {
        snprintf(pDbtable, sizeof(pDbtable), "%s", "alarm_chnOut1_event");
    }

    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[2048] = {0};
    int i = 0;
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);

    for (i = 0; i < MAX_CAMERA && i < count; i++) {
        if (!(changeFlag >> i & 0x01)) {
            continue;
        }

        snprintf(sExec, sizeof(sExec), "update %s set name='%s', time=%d where id=%d;", pDbtable, param[i].name,
                 param[i].delay_time, param[i].chnid);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);

    return 0;
}


int clear_alarm_chnOut_event_name(const char *pDbFile, int chnid)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[2048] = {0};
    snprintf(sExec, sizeof(sExec), "update alarm_chnOut0_event set name='%s' where id=%d;", "", chnid);
    sqlite_execute(hConn, mode, sExec);

    snprintf(sExec, sizeof(sExec), "update alarm_chnOut1_event set name='%s' where id=%d;", "", chnid);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);

    return 0;
}

int update_database_by_file(const char *pDbFile, char *db_file)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    FILE *fp = NULL;
    char buff[1024] = {0};
    fp = fopen(db_file, "rb");
    if (!fp) {
        FileUnlock(nFd, mode, &global_rwlock);
        sqlite_disconn(hConn);
        return -1;
    }

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    while (fgets(buff, sizeof(buff), fp)) {
        sqlite_execute(hConn, mode, buff);
    }

    fclose(fp);

    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);

    return 0;
}

int read_alarm_chnIn_event_names(const char *pDbFile, struct alarm_chn_name *param, int count, int alarmid)
{
    if (!pDbFile || !param || count <= 0) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery), "select id,name from alarm_chnIn%d_event;", alarmid);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < MAX_CAMERA && i < count; i++) {
        sqlite_query_column(hStmt, 0, &param[i].chnid);
        sqlite_query_column_text(hStmt, 1, param[i].name, sizeof(param[i].name));
        param[i].alarmid = alarmid;
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_alarm_out_trigger(const char *pDbFile, int dbtype, struct alarm_out_trigger *inParam,
                           struct alarm_out_trigger_chns *outParam)
{
    if (!pDbFile || !inParam) {
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
    char pDbtable[64] = {0}, pDbSubtable[64] = {0};
    int nColumnCnt = 0;
    switch (dbtype) {
        case ALARMIO:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "alarm_in");
            break;
        case MOTION:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "motion");
            break;
        case VIDEOLOSS:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "video_loss");
            break;
        case REGION_EN:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "vca_regionin_event");
            break;
        case REGION_EXIT:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "vca_regionexit_event");
            break;
        case ADVANCED_MOT:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "vca_motion_event");
            break;
        case TAMPER_DET:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "vca_tamper_event");
            break;
        case LINE_CROSS:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "vca_linecross_event");
            break;
        case LOITER:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "vca_loiter_event");
            break;
        case HUMAN_DET:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "vca_human_event");
            break;
        case PEOPLE_COUNT:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "vca_people_event");
            break;
        case LEFTREMOVE:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "vca_object_leftremove_event");
            break;
        case ANPR_BLACK_EVT:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "lpr_black_mode_event");
            break;
        case ANPR_WHITE_EVT:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "lpr_white_mode_event");
            break;
        case ANPR_VISTOR_EVT:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "lpr_vistor_mode_event");
            break;
        case ALARM_CHN_IN0_EVT:
        case ALARM_CHN_IN1_EVT:
        case ALARM_CHN_IN2_EVT:
        case ALARM_CHN_IN3_EVT:
            snprintf(pDbtable, sizeof(pDbtable), "alarm_chnIn%d_event", converse_event_in_to_chn_alarmid(dbtype));
            break;
        default:
            break;
    }
    if (inParam->alarmid == 0) {
        snprintf(pDbSubtable, sizeof(pDbSubtable), "%s", "tri_chnout1_alarms");
    } else {
        snprintf(pDbSubtable, sizeof(pDbSubtable), "%s", "tri_chnout2_alarms");
    }
    snprintf(sQuery, sizeof(sQuery), "select %s from %s where id=%d", pDbSubtable, pDbtable, inParam->chnid);
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
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    outParam->chnid = inParam->chnid;
    outParam->alarmid = inParam->alarmid;
    sqlite_query_column_text(hStmt, 0, outParam->tri_chn_alarms, sizeof(outParam->tri_chn_alarms));

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);

    return 0;
}

int write_alarm_out_trigger(const char *pDbFile, int dbtype, struct alarm_out_trigger_chns *param)
{
    if (!pDbFile || !param) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sQuery[2048] = {0};
    char pDbtable[64] = {0}, pDbSubtable[64] = {0};
    switch (dbtype) {
        case ALARMIO:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "alarm_in");
            break;
        case MOTION:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "motion");
            break;
        case VIDEOLOSS:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "video_loss");
            break;
        case REGION_EN:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "vca_regionin_event");
            break;
        case REGION_EXIT:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "vca_regionexit_event");
            break;
        case ADVANCED_MOT:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "vca_motion_event");
            break;
        case TAMPER_DET:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "vca_tamper_event");
            break;
        case LINE_CROSS:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "vca_linecross_event");
            break;
        case LOITER:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "vca_loiter_event");
            break;
        case HUMAN_DET:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "vca_human_event");
            break;
        case PEOPLE_COUNT:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "vca_people_event");
            break;
        case LEFTREMOVE:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "vca_object_leftremove_event");
            break;
        case ANPR_BLACK_EVT:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "lpr_black_mode_event");
            break;
        case ANPR_WHITE_EVT:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "lpr_white_mode_event");
            break;
        case ANPR_VISTOR_EVT:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "lpr_vistor_mode_event");
            break;
        case ALARM_CHN_IN0_EVT:
        case ALARM_CHN_IN1_EVT:
        case ALARM_CHN_IN2_EVT:
        case ALARM_CHN_IN3_EVT:
            snprintf(pDbtable, sizeof(pDbtable), "alarm_chnIn%d_event", converse_event_in_to_chn_alarmid(dbtype));
            break;
        default:
            break;
    }
    if (param->alarmid == 0) {
        snprintf(pDbSubtable, sizeof(pDbSubtable), "%s", "tri_chnout1_alarms");
    } else {
        snprintf(pDbSubtable, sizeof(pDbSubtable), "%s", "tri_chnout2_alarms");
    }
    snprintf(sQuery, sizeof(sQuery), "update %s set %s='%s' where id=%d", pDbtable, pDbSubtable, param->tri_chn_alarms,
             param->chnid);
    sqlite_execute(hConn, mode, sQuery);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);

    return 0;
}

int copy_alarm_out_triggers(const char *pDbFile, int dbtype, struct alarm_out_trigger_chns *param, long long changeFlag)
{
    if (!pDbFile || !param) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int i = 0;
    char sQuery[2048] = {0};
    char pDbtable[64] = {0}, pDbSubtable[64] = {0};
    switch (dbtype) {
        case ALARMIO:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "alarm_in");
            break;
        case MOTION:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "motion");
            break;
        case VIDEOLOSS:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "video_loss");
            break;
        case REGION_EN:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "vca_regionin_event");
            break;
        case REGION_EXIT:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "vca_regionexit_event");
            break;
        case ADVANCED_MOT:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "vca_motion_event");
            break;
        case TAMPER_DET:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "vca_tamper_event");
            break;
        case LINE_CROSS:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "vca_linecross_event");
            break;
        case LOITER:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "vca_loiter_event");
            break;
        case HUMAN_DET:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "vca_human_event");
            break;
        case PEOPLE_COUNT:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "vca_people_event");
            break;
        case LEFTREMOVE:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "vca_object_leftremove_event");
            break;
        case ANPR_BLACK_EVT:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "lpr_black_mode_event");
            break;
        case ANPR_WHITE_EVT:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "lpr_white_mode_event");
            break;
        case ANPR_VISTOR_EVT:
            snprintf(pDbtable, sizeof(pDbtable), "%s", "lpr_vistor_mode_event");
            break;
        case ALARM_CHN_IN0_EVT:
        case ALARM_CHN_IN1_EVT:
        case ALARM_CHN_IN2_EVT:
        case ALARM_CHN_IN3_EVT:
            snprintf(pDbtable, sizeof(pDbtable), "alarm_chnIn%d_event", converse_event_in_to_chn_alarmid(dbtype));
            break;
        default:
            sqlite_disconn(hConn);
            FileUnlock(nFd, mode, &global_rwlock);
            return -1;
            break;
    }
    if (param->alarmid == 0) {
        snprintf(pDbSubtable, sizeof(pDbSubtable), "%s", "tri_chnout1_alarms");
    } else {
        snprintf(pDbSubtable, sizeof(pDbSubtable), "%s", "tri_chnout2_alarms");
    }

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (i = 0; i < MAX_CAMERA; i++) {
        if (!(changeFlag >> i & 0x01)) {
            continue;
        }
        snprintf(sQuery, sizeof(sQuery), "update %s set %s='%s' where id=%d", pDbtable, pDbSubtable, param->tri_chn_alarms, i);
        sqlite_execute(hConn, mode, sQuery);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_push_msg_event(const char *pDbFile, struct push_msg_event *param, int chn_id)
{
    if (!pDbFile || !param || chn_id < 0) {
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
    char sQuery[512] = {0};
    int nColumnCnt = 0;
    snprintf(sQuery, sizeof(sQuery), "select chn_id,push_type from push_msg_event where chn_id=%d;", chn_id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, &param->chnid);
    sqlite_query_column(hStmt, 1, &param->push_type);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_push_msg_event(const char *pDbFile, struct push_msg_event *param)
{
    if (!pDbFile || !param) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[512] = {0};

    snprintf(sExec, sizeof(sExec), "update push_msg_event set push_type=%d where chn_id=%d;", param->push_type,
             param->chnid);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_push_msg_events(const char *pDbFile, struct push_msg_event param[], int chnCnt)
{
    if (!pDbFile || !param) {
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
    char sQuery[512] = {0};
    int i = 0, nColumnCnt = 0, nResult = 0;
    do {
        snprintf(sQuery, sizeof(sQuery), "select chn_id,push_type from push_msg_event where chn_id=%d;", i);
        nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            i++;
            continue;
        }

        sqlite_query_column(hStmt, 0, &param[i].chnid);
        sqlite_query_column(hStmt, 1, &param[i].push_type);
        sqlite_clear_stmt(hStmt);
        i++;
    } while (i < chnCnt && i < MAX_CAMERA);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_push_msg_events(const char *pDbFile, struct push_msg_event param[], int chnCnt)
{
    if (!pDbFile || !param) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int i = 0;
    char sExec[512] = {0};

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (i = 0; i < chnCnt && i < MAX_CAMERA; i++) {
        snprintf(sExec, sizeof(sExec), "update push_msg_event set push_type=%d where chn_id=%d;", param[i].push_type,
                 param[i].chnid);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

static int exec_version_callback(void *arg, int argc, char **argv, char **cols)
{
    int *version = (int *)arg;

    *version = atoi(argv[0]);

    return 0;
}

int read_db_version(const char *pDbFile)
{
    HSQLITE hsql;
    int result = -1, version = 0;

    if (sqlite_conn(pDbFile, &hsql)) {
        msdebug(DEBUG_ERR, "read_db_version to %s error.", pDbFile);
        return -1;
    }
    result = sqlite3_exec(hsql, "PRAGMA user_version;", exec_version_callback, (void *)&version, NULL);
    if (result != 0) {
        msdebug(DEBUG_ERR, "read_db_version is Err!");
        sqlite_disconn(hsql);
        return -1;
    }
    sqlite_disconn(hsql);

    return version;
}

int read_whiteled_effective_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, WLED_INFO *info)
{
    return read_schedule(pDbFile, schedule, info->pDbTable, info->chnid);
}

int write_whiteled_effective_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, WLED_INFO *info)
{
    return write_schedule(pDbFile, schedule, info->pDbTable, info->chnid);
}

int copy_whiteled_effective_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, WLED_INFO *info, Uint64 chnMasks)
{
    return copy_schedules(pDbFile, schedule, info->pDbTable, chnMasks);
}

int read_whiteled_params(const char *pDbFile, WHITE_LED_PARAMS *ledParams, WLED_INFO *info, int *cnt)
{
    if (!pDbFile || !ledParams) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0;
    int nColumnCnt = 0;
    HSQLSTMT hStmt = 0;
    char sQuery[2048] = {0};

    snprintf(sQuery, sizeof(sQuery), "select chnid,flash_mode,flash_time,acto_chn_id from %s where chnid=%d;",
             info->pDbTable, info->chnid);
    if (sQuery[0] == '\0') {
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *cnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < MAX_REAL_CAMERA; i++) {
        sqlite_query_column(hStmt, 0, &ledParams[i].chnid);
        sqlite_query_column(hStmt, 1, &ledParams[i].flash_mode);
        sqlite_query_column(hStmt, 2, &ledParams[i].flash_time);
        sqlite_query_column(hStmt, 3, &ledParams[i].acto_chn_id);

        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    *cnt  = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_whiteled_params(const char *pDbFile, WHITE_LED_PARAMS *ledParams, const char *pDbTable)
{
    if (!pDbFile || !ledParams || pDbTable[0] == '\0') {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    char sExec[2048] = {0};
    HSQLITE hConn = 0;

    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    snprintf(sExec, sizeof(sExec), "delete from %s where chnid=%d and acto_chn_id=%d;", pDbTable, ledParams->chnid,
             ledParams->acto_chn_id);
    sqlite_execute(hConn, mode, sExec);

    snprintf(sExec, sizeof(sExec), "insert into %s(chnid,flash_mode,flash_time,acto_chn_id) values(%d,%d,%d,%d);",
             pDbTable, ledParams->chnid, ledParams->flash_mode, ledParams->flash_time, ledParams->acto_chn_id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_whiteled_params_all(const char *pDbFile, WHITE_LED_PARAMS *ledParams, const char *pDbTable, int chn_id)
{
    if (!pDbFile || !ledParams) {
        return -1;
    }

    int i = 0;
    char sExec[2048] = {0};
    HSQLITE hConn = 0;

    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    snprintf(sExec, sizeof(sExec), "delete from %s where chnid=%d", pDbTable, chn_id);
    sqlite_execute(hConn, mode, sExec);
    for (i = 0; i < MAX_REAL_CAMERA; i++) {
        if (ledParams[i].flash_time > 0) {
            snprintf(sExec, sizeof(sExec), "insert into %s(chnid,flash_mode,flash_time,acto_chn_id) values(%d,%d,%d,%d);",
                     pDbTable, chn_id, ledParams[i].flash_mode, ledParams[i].flash_time, ledParams[i].acto_chn_id);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}


int delete_whiteled_params(const char *pDbFile, WHITE_LED_PARAMS *ledParams, const char *pDbTable)
{
    if (!pDbFile || !ledParams || pDbTable[0] == '\0') {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    char sExec[2048] = {0};
    HSQLITE hConn = 0;

    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    snprintf(sExec, sizeof(sExec), "delete from %s where chnid=%d and acto_chn_id=%d;", pDbTable, ledParams->chnid,
             ledParams->acto_chn_id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_whiteled_params(const char *pDbFile, const char *pDbTable, WHITE_LED_PARAMS ledParams[], Uint64 chnMasks)
{
    if (!pDbFile || !ledParams || !pDbTable || !chnMasks) {
        return -1;
    }

    int i = 0, j = 0;
    char sExec[2048] = {0};
    HSQLITE hConn = 0;

    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (j = 0; j < MAX_REAL_CAMERA; j++) {
        if (!(chnMasks >> j & 0x01)) {
            continue;
        }
        snprintf(sExec, sizeof(sExec), "delete from %s where chnid=%d", pDbTable, j);
        sqlite_execute(hConn, mode, sExec);
        for (i = 0; i < MAX_REAL_CAMERA; i++) {
            if (ledParams[i].flash_time > 0) {
                snprintf(sExec, sizeof(sExec), "insert into %s(chnid,flash_mode,flash_time,acto_chn_id) values(%d,%d,%d,%d);",
                         pDbTable, j, ledParams[i].flash_mode, ledParams[i].flash_time, ledParams[i].acto_chn_id);
                sqlite_execute(hConn, mode, sExec);
            }
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_smart_event_led_schedules(const char *pDbFile, struct smart_event_schedule *Schedule, int chn_id)
{
    if (!pDbFile || !Schedule) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_SMART_EVENT][256];

    HSQLSTMT hStmt = 0;
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
            sqlite_query_column_text(hStmt, 2, Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
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
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_motion_popup_schedules(const char *pDbFile, struct motion_schedule *schedule, Uint64 changeFlag)
{
    if (!pDbFile || !schedule) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    char sExec[256] = {0};
    int i = 0, j = 0, k = 0;
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (k = 0; k < MAX_CAMERA; k++) {
        if (!(changeFlag >> k & 0x01)) {
            continue;
        }
        snprintf(sExec, sizeof(sExec), "delete from motion_popup_schedule where chn_id=%d;", k);
        sqlite_execute(hConn, mode, sExec);
        for (i = 0; i < MAX_DAY_NUM; i++) {
            for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
                if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                               schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                    continue;
                }
                snprintf(sExec, sizeof(sExec), "insert into motion_popup_schedule values(%d,%d,%d, '%s', '%s', %d);", k, i, j,
                         schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                         schedule->schedule_day[i].schedule_item[j].action_type);
                sqlite_execute(hConn, mode, sExec);
            }
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_access_filter(const char *pDbFile, struct access_list limit_list[], int *pCnt)
{
    if (!pDbFile || !limit_list) {
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
    char sQuery[128] = {0};
    int nColumnCnt = 0;
    int i = 0;
    snprintf(sQuery, sizeof(sQuery), "select type,address from access_filter;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *pCnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int type;
    for (i = 0; i < MAX_LIMIT_ADDR; i++) {
        sqlite_query_column(hStmt, 0, &type);
        limit_list[i].type = (ADDRESS_TYPE)type;
        sqlite_query_column_text(hStmt, 1, limit_list[i].address, sizeof(limit_list[i].address));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }
    *pCnt = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_access_filter(const char *pDbFile, struct access_list limit_list[], int count)
{
    if (!pDbFile || !limit_list) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[512] = {0};
    int i = 0;
    for (i = 0; i < count && i < MAX_LIMIT_ADDR; i++) {
        if (limit_list[i].type < ADDRESS_TYPE_MAC || limit_list[i].type >= ADDRESS_TYPE_MAX) {
            sqlite_disconn(hConn);
            FileUnlock(nFd, mode, &global_rwlock);
            return -2;
        }
    }

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    sqlite_execute(hConn, mode, "delete from access_filter;");
    for (i = 0; i < count && i < MAX_LIMIT_ADDR; i++) {
        snprintf(sExec, sizeof(sExec), "insert into access_filter values(%d,'%s');",
                 limit_list[i].type, limit_list[i].address);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);

    return 0;
}

static int read_comn_sche(const char *pDbFile, SMART_SCHEDULE *Schedule, char *db_table, char *squery, int id)
{
    if (!pDbFile || !Schedule || !db_table || db_table[0] == '\0' || id < 0 || id >= MAX_REAL_CAMERA) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[256] = {0};

    HSQLSTMT hStmt = 0;
    snprintf(sQuery, sizeof(sQuery), "select week_id,plan_id,start_time,end_time,action_type from %s where %s=%d;",
             db_table, squery, id);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i <= nLoopCnt; i++) {
        sqlite_query_column(hStmt, 0, &nWeekId);
        sqlite_query_column(hStmt, 1, &nPlanId);
        sqlite_query_column_text(hStmt, 2, Schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                 sizeof(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].start_time));
        sqlite_query_column_text(hStmt, 3, Schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                 sizeof(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].end_time));
        sqlite_query_column(hStmt, 4, &(Schedule->schedule_day[nWeekId].schedule_item[nPlanId].action_type));
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

static int read_comn_sches(const char *pDbFile, SMART_SCHEDULE *Schedule, char *db_table, char *squery, int cnt)
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

    int i = 0;
    int j = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[256] = {0};

    HSQLSTMT hStmt = 0;
    do {
        snprintf(sQuery, sizeof(sQuery), "select week_id,plan_id,start_time,end_time,action_type from %s where %s=%d;",
                 db_table, squery, i);
        nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
                hStmt = 0;
            }
            i++;
            continue;
        }

        for (j = 0; j <= nLoopCnt; j++) {
            sqlite_query_column(hStmt, 0, &nWeekId);
            sqlite_query_column(hStmt, 1, &nPlanId);
            sqlite_query_column_text(hStmt, 2, Schedule[i].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(Schedule[i].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, Schedule[i].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(Schedule[i].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &Schedule[i].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
            if (sqlite_query_next(hStmt) != 0) {
                j++;
                break;
            }
        }
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
            hStmt = 0;
        }

        i++;
    } while (i < cnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

static int write_comn_sche(const char *pDbFile, SMART_SCHEDULE *schedule, char *db_table, char *squery, int id)
{
    if (!pDbFile || !schedule || !db_table || db_table[0] == '\0' || id < 0 || id >= MAX_REAL_CAMERA) {
        return -1;
    }
    
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "delete from %s where %s=%d;", db_table, squery, id);
    sqlite_execute(hConn, mode, sExec);
    int i = 0, j = 0;
    for (i = 0; i < MAX_DAY_NUM; i++) {
        for (j = 0; j < MAX_PLAN_NUM_PER_DAY; j++) {
            if (strcasecmp(schedule->schedule_day[i].schedule_item[j].start_time,
                           schedule->schedule_day[i].schedule_item[j].end_time) == 0) {
                continue;
            }
            snprintf(sExec, sizeof(sExec), "insert into %s values(%d,%d,%d, '%s', '%s', %d);", db_table, id, i, j,
                     schedule->schedule_day[i].schedule_item[j].start_time, schedule->schedule_day[i].schedule_item[j].end_time,
                     schedule->schedule_day[i].schedule_item[j].action_type);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_peoplecnt_audible_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int groupid)
{
    return read_comn_sche(pDbFile, schedule, "people_cnt_audible_sche", "groupid", groupid);
}

int write_peoplecnt_audible_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int groupid)
{
    return write_comn_sche(pDbFile, schedule, "people_cnt_audible_sche", "groupid", groupid);
}

int copy_peoplecnt_audible_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int alarmid, Uint64 changeFlag)
{
    return copy_comn_sches(pDbFile, schedule, "people_cnt_audible_sche", "groupid", changeFlag);
}

int read_peoplecnt_email_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int groupid)
{
    return read_comn_sche(pDbFile, schedule, "people_cnt_email_sche", "groupid", groupid);
}

int write_peoplecnt_email_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int groupid)
{
    return write_comn_sche(pDbFile, schedule, "people_cnt_email_sche", "groupid", groupid);
}

int copy_peoplecnt_email_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int alarmid, Uint64 changeFlag)
{
    return copy_comn_sches(pDbFile, schedule, "people_cnt_email_sche", "groupid", changeFlag);
}

int read_peoplecnt_ptz_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int groupid)
{
    return read_comn_sche(pDbFile, schedule, "people_cnt_ptz_sche", "groupid", groupid);
}

int write_peoplecnt_ptz_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int groupid)
{
    return write_comn_sche(pDbFile, schedule, "people_cnt_ptz_sche", "groupid", groupid);
}

int copy_peoplecnt_ptz_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int alarmid, Uint64 changeFlag)
{
    return copy_comn_sches(pDbFile, schedule, "people_cnt_ptz_sche", "groupid", changeFlag);
}

int read_peoplecnt_wled_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int groupid)
{
    return read_comn_sche(pDbFile, schedule, "people_cnt_wled_sche", "groupid", groupid);
}

int write_peoplecnt_wled_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int groupid)
{
    return write_comn_sche(pDbFile, schedule, "people_cnt_wled_sche", "groupid", groupid);
}

int copy_peoplecnt_wled_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int alarmid, Uint64 changeFlag)
{
    return copy_comn_sches(pDbFile, schedule, "people_cnt_wled_sche", "groupid", changeFlag);
}

int read_peoplecnt_wled_params(const char *pDbFile, PEOPLECNT_WLED_PARAMS *ledParams, int groupid, int *cnt)
{
    if (!pDbFile || !ledParams) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0;
    int nColumnCnt = 0;
    HSQLSTMT hStmt = 0;
    char sQuery[2048] = {0};

    snprintf(sQuery, sizeof(sQuery),
             "select groupid,flash_mode,flash_time,acto_chn_id from people_cnt_wled_params where groupid=%d;", groupid);
    if (sQuery[0] == '\0') {
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *cnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < MAX_REAL_CAMERA; i++) {
        sqlite_query_column(hStmt, 0, &ledParams[i].groupid);
        sqlite_query_column(hStmt, 1, &ledParams[i].flash_mode);
        sqlite_query_column(hStmt, 2, &ledParams[i].flash_time);
        sqlite_query_column(hStmt, 3, &ledParams[i].acto_chn_id);

        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    *cnt  = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_peoplecnt_wled_params(const char *pDbFile, PEOPLECNT_WLED_PARAMS *ledParams)
{
    if (!pDbFile || !ledParams) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    char sExec[2048] = {0};
    HSQLITE hConn = 0;

    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    snprintf(sExec, sizeof(sExec), "delete from people_cnt_wled_params where groupid=%d and acto_chn_id=%d;",
             ledParams->groupid, ledParams->acto_chn_id);
    sqlite_execute(hConn, mode, sExec);

    snprintf(sExec, sizeof(sExec),
             "insert into people_cnt_wled_params(groupid,flash_mode,flash_time,acto_chn_id) values(%d,%d,%d,%d);",
             ledParams->groupid, ledParams->flash_mode, ledParams->flash_time, ledParams->acto_chn_id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_peoplecnt_wled_params_all(const char *pDbFile, PEOPLECNT_WLED_PARAMS *ledParams, int chn_id)
{
    if (!pDbFile || !ledParams) {
        return -1;
    }

    int i = 0;
    char sExec[2048] = {0};
    HSQLITE hConn = 0;

    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    snprintf(sExec, sizeof(sExec), "delete from people_cnt_wled_params where groupid=%d", chn_id);
    sqlite_execute(hConn, mode, sExec);
    for (i = 0; i < MAX_REAL_CAMERA; i++) {
        if (ledParams[i].flash_time > 0) {
            snprintf(sExec, sizeof(sExec),
                     "insert into people_cnt_wled_params(groupid,flash_mode,flash_time,acto_chn_id) values(%d,%d,%d,%d);",
                     chn_id, ledParams[i].flash_mode, ledParams[i].flash_time, ledParams[i].acto_chn_id);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int delete_peoplecnt_wled_params(const char *pDbFile, PEOPLECNT_WLED_PARAMS *ledParams)
{
    if (!pDbFile || !ledParams) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    char sExec[2048] = {0};
    HSQLITE hConn = 0;

    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    snprintf(sExec, sizeof(sExec), "delete from people_cnt_wled_params where groupid=%d and acto_chn_id=%d;",
             ledParams->groupid, ledParams->acto_chn_id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_peoplecnt_wled_params(const char *pDbFile, PEOPLECNT_WLED_PARAMS ledParams[], Uint64 chnMasks)
{
    if (!pDbFile || !ledParams) {
        return -1;
    }

    int i = 0, j = 0;
    char sExec[2048] = {0};
    HSQLITE hConn = 0;

    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (j = 0; j < MAX_REAL_CAMERA; j++) {
        if (!(chnMasks >> j & 0x01)) {
            continue;
        }
        snprintf(sExec, sizeof(sExec), "delete from people_cnt_wled_params where groupid=%d", j);
        sqlite_execute(hConn, mode, sExec);
        for (i = 0; i < MAX_REAL_CAMERA; i++) {
            if (ledParams[i].flash_time > 0) {
                snprintf(sExec, sizeof(sExec),
                         "insert into people_cnt_wled_params(groupid,flash_mode,flash_time,acto_chn_id) values(%d,%d,%d,%d);",
                         j, ledParams[i].flash_mode, ledParams[i].flash_time, ledParams[i].acto_chn_id);
                sqlite_execute(hConn, mode, sExec);
            }
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_peoplecnt_ptz_params(const char *pDbFile, PEOPLECNT_PTZ_PARAMS ptzActionParams[], int groupid, int *cnt)
{
    if (!pDbFile || !ptzActionParams || groupid < 0) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select groupid,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern from people_cnt_ptz_params where groupid=%d;",
             groupid);

    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *cnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < MAX_REAL_CAMERA; i++) {
        sqlite_query_column(hStmt, 0, &ptzActionParams[i].groupid);
        sqlite_query_column(hStmt, 1, &ptzActionParams[i].acto_ptz_channel);
        sqlite_query_column(hStmt, 2, &ptzActionParams[i].acto_fish_channel);
        sqlite_query_column(hStmt, 3, &ptzActionParams[i].acto_ptz_type);
        sqlite_query_column(hStmt, 4, &ptzActionParams[i].acto_ptz_preset);
        sqlite_query_column(hStmt, 5, &ptzActionParams[i].acto_ptz_patrol);
        sqlite_query_column(hStmt, 6, &ptzActionParams[i].acto_ptz_pattern);

        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    *cnt  = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_peoplecnt_ptz_params_ex(const char *pDbFile, PEOPLECNT_PTZ_PARAMS ptzActionParams[], int groupid)
{
    if (!pDbFile || !ptzActionParams) {
        return -1;
    }
    int i = 0;
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    char sExec[256] = {0};
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    snprintf(sExec, sizeof(sExec), "delete from people_cnt_ptz_params where groupid=%d;", ptzActionParams->groupid);
    sqlite_execute(hConn, mode, sExec);
    for (i = 0; i < MAX_REAL_CAMERA; i++) {
        if (ptzActionParams[i].acto_ptz_channel > 0) {
            snprintf(sExec, sizeof(sExec),
                     "insert into people_cnt_ptz_params(groupid,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern) values(%d,%d,%d,%d,%d,%d,%d);",
                     ptzActionParams[i].groupid, ptzActionParams[i].acto_ptz_channel, ptzActionParams[i].acto_fish_channel,
                     ptzActionParams[i].acto_ptz_type, ptzActionParams[i].acto_ptz_preset, ptzActionParams[i].acto_ptz_patrol,
                     ptzActionParams[i].acto_ptz_pattern);
            sqlite_execute(hConn, mode, sExec);
        }
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_peoplecnt_ptz_params(const char *pDbFile, PEOPLECNT_PTZ_PARAMS *ptzActionParams)
{
    if (!pDbFile || !ptzActionParams) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    char sExec[256] = {0};
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    snprintf(sExec, sizeof(sExec), "delete from people_cnt_ptz_params where groupid=%d and acto_ptz_channel=%d;",
             ptzActionParams->groupid, ptzActionParams->acto_ptz_channel);
    sqlite_execute(hConn, mode, sExec);
    snprintf(sExec, sizeof(sExec),
             "insert into people_cnt_ptz_params(groupid,acto_ptz_channel,acto_fish_channel,acto_ptz_type,acto_ptz_preset,acto_ptz_patrol,acto_ptz_pattern) values(%d,%d,%d,%d,%d,%d,%d);",
             ptzActionParams->groupid, ptzActionParams->acto_ptz_channel, ptzActionParams->acto_fish_channel,
             ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset, ptzActionParams->acto_ptz_patrol,
             ptzActionParams->acto_ptz_pattern);

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int update_peoplecnt_ptz_params(const char *pDbFile, PEOPLECNT_PTZ_PARAMS *ptzActionParams, int oldGroupid)
{
    if (!pDbFile || !ptzActionParams) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    char sExec[512] = {0};
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    snprintf(sExec, sizeof(sExec),
             "update people_cnt_ptz_params set acto_fish_channel=%d,acto_ptz_type=%d,acto_ptz_preset=%d,acto_ptz_patrol=%d,acto_ptz_pattern=%d,acto_ptz_channel=%d where groupid=%d and acto_ptz_channel=%d;",
             ptzActionParams->acto_fish_channel, ptzActionParams->acto_ptz_type, ptzActionParams->acto_ptz_preset,
             ptzActionParams->acto_ptz_patrol, ptzActionParams->acto_ptz_pattern, ptzActionParams->acto_ptz_channel,
             ptzActionParams->groupid, oldGroupid);

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int delete_peoplecnt_ptz_params(const char *pDbFile, PEOPLECNT_PTZ_PARAMS *ptzActionParams)
{
    if (!pDbFile || !ptzActionParams) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    char sExec[256] = {0};
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    snprintf(sExec, sizeof(sExec), "delete from people_cnt_ptz_params where groupid=%d and acto_ptz_channel=%d;",
             ptzActionParams->groupid, ptzActionParams->acto_ptz_channel);

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_peoplecnt_event(const char *pDbFile, PEOPLECNT_EVENT *smartevent, int groupid)
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
    HSQLSTMT hStmt = 0;
    char sQuery[2048] = {0};
    int nColumnCnt = 0;

    snprintf(sQuery, sizeof(sQuery),
             "select groupid,tri_alarms,tri_chnout1_alarms,tri_chnout2_alarms,buzzer_interval,email_interval,\
		ptzaction_interval,alarmout_interval,whiteled_interval,email_pic_enable,tri_channels_pic,tri_audio_id,\
		http_notification_interval from people_cnt_event where groupid=%d;", groupid);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, &smartevent->groupid);
    sqlite_query_column(hStmt, 1, (int *)&smartevent->tri_alarms);
    sqlite_query_column_text(hStmt, 2, smartevent->tri_chnout1_alarms, sizeof(smartevent->tri_chnout1_alarms));
    sqlite_query_column_text(hStmt, 3, smartevent->tri_chnout2_alarms, sizeof(smartevent->tri_chnout2_alarms));
    sqlite_query_column(hStmt, 4, &smartevent->buzzer_interval);
    sqlite_query_column(hStmt, 5, &smartevent->email_interval);
    sqlite_query_column(hStmt, 6, &smartevent->ptzaction_interval);
    sqlite_query_column(hStmt, 7, &smartevent->alarmout_interval);
    sqlite_query_column(hStmt, 8, &smartevent->whiteled_interval);
    sqlite_query_column(hStmt, 9, &smartevent->email_pic_enable);
    sqlite_query_column_text(hStmt, 10, smartevent->tri_channels_pic, sizeof(smartevent->tri_channels_pic));
    sqlite_query_column(hStmt, 11, &smartevent->tri_audio_id);
    sqlite_query_column(hStmt, 12, &smartevent->http_notification_interval);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_peoplecnt_event(const char *pDbFile, PEOPLECNT_EVENT *smartevent)
{
    if (!pDbFile || !smartevent) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[2048] = {0};
    snprintf(sExec, sizeof(sExec),
             "update people_cnt_event set tri_alarms=%d,tri_chnout1_alarms='%s',tri_chnout2_alarms='%s',\
			buzzer_interval='%d', email_interval='%d',ptzaction_interval=%d,alarmout_interval=%d,whiteled_interval=%d,\
			email_pic_enable=%d,tri_channels_pic='%s',tri_audio_id=%d,http_notification_interval=%d where groupid=%d;",
             smartevent->tri_alarms, smartevent->tri_chnout1_alarms, smartevent->tri_chnout2_alarms,
             smartevent->buzzer_interval, smartevent->email_interval, smartevent->ptzaction_interval,
             smartevent->alarmout_interval, smartevent->whiteled_interval, smartevent->email_pic_enable,
             smartevent->tri_channels_pic, smartevent->tri_audio_id, smartevent->http_notification_interval,
             smartevent->groupid);

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_peoplecnt_setting(const char *pDbFile, PEOPLECNT_SETTING *pSettings, int groupid)
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

    snprintf(sQuery, sizeof(sQuery),
             "select groupid,name,enable,stays,tri_channels,liveview_auto_reset,auto_day,auto_day_time,liveview_font_size,liveview_green_tips,liveview_red_tips,liveview_display from people_cnt_setting where groupid=%d;",
             groupid);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, &pSettings->groupid);
    sqlite_query_column_text(hStmt, 1, pSettings->name, sizeof(pSettings->name));
    sqlite_query_column(hStmt, 2, &pSettings->enable);
    sqlite_query_column(hStmt, 3, &pSettings->stays);
    sqlite_query_column_text(hStmt, 4, pSettings->tri_channels, sizeof(pSettings->tri_channels));
    sqlite_query_column(hStmt, 5, &pSettings->liveview_auto_reset);
    sqlite_query_column(hStmt, 6, &pSettings->auto_day);
    sqlite_query_column_text(hStmt, 7, pSettings->auto_day_time, sizeof(pSettings->auto_day_time));
    sqlite_query_column(hStmt, 8, &pSettings->liveview_font_size);
    sqlite_query_column_text(hStmt, 9, pSettings->liveview_green_tips, sizeof(pSettings->liveview_green_tips));
    sqlite_query_column_text(hStmt, 10, pSettings->liveview_red_tips, sizeof(pSettings->liveview_red_tips));
    sqlite_query_column(hStmt, 11, &pSettings->liveview_display);


    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_peoplecnt_settings(const char *pDbFile, PEOPLECNT_SETTING pSet[])
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
    int i, nColumnCnt = 0;

    snprintf(sQuery, sizeof(sQuery),
             "select groupid,name,enable,stays,tri_channels,liveview_auto_reset,auto_day,auto_day_time,liveview_font_size,liveview_green_tips,liveview_red_tips,liveview_display from people_cnt_setting;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < MAX_PEOPLECNT_GROUP; i++) {
        sqlite_query_column(hStmt, 0, &pSet[i].groupid);
        sqlite_query_column_text(hStmt, 1, (char *) & (pSet[i].name), sizeof(pSet[i].name));
        sqlite_query_column(hStmt, 2, &pSet[i].enable);
        sqlite_query_column(hStmt, 3, &pSet[i].stays);
        sqlite_query_column_text(hStmt, 4, (char *) & (pSet[i].tri_channels), sizeof(pSet[i].tri_channels));
        sqlite_query_column(hStmt, 5, &pSet[i].liveview_auto_reset);
        sqlite_query_column(hStmt, 6, &pSet[i].auto_day);
        sqlite_query_column_text(hStmt, 7, (char *) & (pSet[i].auto_day_time), sizeof(pSet[i].auto_day_time));
        sqlite_query_column(hStmt, 8, &pSet[i].liveview_font_size);
        sqlite_query_column_text(hStmt, 9, (char *) & (pSet[i].liveview_green_tips), sizeof(pSet[i].liveview_green_tips));
        sqlite_query_column_text(hStmt, 10, (char *) & (pSet[i].liveview_red_tips), sizeof(pSet[i].liveview_red_tips));
        sqlite_query_column(hStmt, 11, &pSet[i].liveview_display);

        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_peoplecnt_setting(const char *pDbFile, PEOPLECNT_SETTING *pSettings)
{
    if (!pDbFile || !pSettings) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char tmpName[MAX_LEN_128];
    char tmpGreenTips[MAX_LEN_256];
    char tmpRedTips[MAX_LEN_256];
    translate_pwd(tmpName, pSettings->name, strlen(pSettings->name));
    translate_pwd(tmpGreenTips, pSettings->liveview_green_tips, strlen(pSettings->liveview_green_tips));
    translate_pwd(tmpRedTips, pSettings->liveview_red_tips, strlen(pSettings->liveview_red_tips));
    char sExec[2048] = {0};
    snprintf(sExec, sizeof(sExec),
             "update people_cnt_setting set groupid=%d,name='%s',enable=%d,stays=%d,tri_channels='%s',\
            liveview_auto_reset=%d,auto_day=%d,auto_day_time='%s',liveview_font_size=%d,liveview_green_tips='%s',liveview_red_tips='%s',liveview_display=%d where groupid=%d;",
             pSettings->groupid, tmpName, pSettings->enable, pSettings->stays,
             pSettings->tri_channels, pSettings->liveview_auto_reset,
             pSettings->auto_day, pSettings->auto_day_time,
             pSettings->liveview_font_size, tmpGreenTips, tmpRedTips, pSettings->liveview_display, pSettings->groupid);

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_peoplecnt_settings(const char *pDbFile, PEOPLECNT_SETTING *pSettings, int count)
{
    if (!pDbFile || !pSettings) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[2048] = {0};
    char tmpName[MAX_LEN_128];
    char tmpGreenTips[MAX_LEN_256];
    char tmpRedTips[MAX_LEN_256];
    int i = 0;
    PEOPLECNT_SETTING *setting = NULL;
    for (i = 0; i < count; ++i) {
        setting = &pSettings[i];
        translate_pwd(tmpName, setting->name, strlen(setting->name));
        translate_pwd(tmpGreenTips, setting->liveview_green_tips, strlen(setting->liveview_green_tips));
        translate_pwd(tmpRedTips, setting->liveview_red_tips, strlen(setting->liveview_red_tips));
        snprintf(sExec, sizeof(sExec),
                 "update people_cnt_setting set groupid=%d,name='%s',enable=%d,stays=%d,tri_channels='%s',\
                 liveview_auto_reset=%d,auto_day=%d,auto_day_time='%s',liveview_font_size=%d,liveview_green_tips='%s',liveview_red_tips='%s',liveview_display=%d where groupid=%d;",
                 setting->groupid, tmpName, setting->enable, setting->stays,
                 setting->tri_channels, setting->liveview_auto_reset,
                 setting->auto_day, setting->auto_day_time,
                 setting->liveview_font_size, tmpGreenTips, tmpRedTips, setting->liveview_display, setting->groupid);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_peoplecnt_auto_backup(const char *pDbFile, REPORT_AUTO_BACKUP_S *backup)
{
    if (!pDbFile || !backup) {
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
    char sQuery[1024] = {0};
    int i, nColumnCnt = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select report_type, enable,tri_channels,tri_groups,stay_length_type,stay_length_value,backup_day,backup_time,file_format,time_range,backup_to,line_mask from people_cnt_auto_backup;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    int type;
    for (i = 0; i < MAX_REPORT_AUTO_BACKUP_TYPE_COUNT; ++i)
    {
        sqlite_query_column(hStmt, 0, &type);
        REPORT_AUTO_BACKUP_SETTING_S *setting = &backup->settings[type];
        setting->reportType = type;
        sqlite_query_column(hStmt, 1, &setting->enable);
        sqlite_query_column_text(hStmt, 2, setting->triChannels, sizeof(setting->triChannels));
        sqlite_query_column_text(hStmt, 3, setting->triGroups, sizeof(setting->triGroups));
        sqlite_query_column(hStmt, 4, &setting->stayLengthType);
        sqlite_query_column(hStmt, 5, &setting->stayLengthValue);
        sqlite_query_column(hStmt, 6, &setting->backupDay);
        sqlite_query_column_text(hStmt, 7, setting->backupTime, sizeof(setting->backupTime));
        sqlite_query_column(hStmt, 8, &setting->fileFormat);
        sqlite_query_column(hStmt, 9, &setting->timeRange);
        sqlite_query_column(hStmt, 10, &setting->backupTo);
        sqlite_query_column(hStmt, 11, &setting->lineMask);

        if (sqlite_query_next(hStmt) != 0) {
            break;
        }
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_peoplecnt_auto_backup(const char *pDbFile, REPORT_AUTO_BACKUP_S *backup)
{
    if (!pDbFile || !backup) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    int i = 0;
    char sExec[1024] = {0};
    for (i = 0; i < MAX_REPORT_AUTO_BACKUP_TYPE_COUNT; ++i) {
        REPORT_AUTO_BACKUP_SETTING_S *setting = &backup->settings[i];
        snprintf(sExec, sizeof(sExec),
                 "update people_cnt_auto_backup set enable=%d, tri_channels='%s', tri_groups='%s', stay_length_type=%d, stay_length_value=%d, "
                 "backup_day=%d, backup_time='%s', file_format=%d, time_range=%d, backup_to=%d, line_mask=%d where report_type=%d;",
                 setting->enable, setting->triChannels, setting->triGroups, setting->stayLengthType, setting->stayLengthValue,
                 setting->backupDay, setting->backupTime, setting->fileFormat, setting->timeRange, setting->backupTo, setting->lineMask, i);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_auto_backup(const char *pDbFile, ESATA_AUTO_BACKUP *autoBackup, const char *sqlQuery)
{
    if (!pDbFile || !autoBackup) {
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
    int nColumnCnt = 0;
    int nResult = sqlite_query_record(hConn, sqlQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &autoBackup->enable);
    sqlite_query_column(hStmt, 1, (int *)&autoBackup->backup_day);
    sqlite_query_column_text(hStmt, 2, autoBackup->backup_time, sizeof(autoBackup->backup_time));
    sqlite_query_column(hStmt, 3, (int *)&autoBackup->stream_type);
    sqlite_query_column(hStmt, 4, (int *)&autoBackup->file_type);
    sqlite_query_column(hStmt, 5, (int *)&autoBackup->recycle);
    sqlite_query_column_text(hStmt, 6, autoBackup->tri_channels, sizeof(autoBackup->tri_channels));
    sqlite_query_column(hStmt, 7, (int *)&autoBackup->device);
    sqlite_query_column(hStmt, 8, (int *)&autoBackup->port);


    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_auto_backup(const char *pDbFile, ESATA_AUTO_BACKUP *autoBackup, const char *sqlQuery)
{
    if (!pDbFile || !autoBackup) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[1024] = {0};
    snprintf(sExec, sizeof(sExec),
             sqlQuery,
             autoBackup->enable, autoBackup->backup_day, autoBackup->backup_time, autoBackup->stream_type, autoBackup->file_type,
             autoBackup->recycle, autoBackup->tri_channels, autoBackup->device, autoBackup->port);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_esata_auto_backup(const char *pDbFile, ESATA_AUTO_BACKUP *autoBackup)
{
    return read_auto_backup(pDbFile, autoBackup, "select enable,backup_day,backup_time,stream_type,file_type,recycle,tri_channels, device, port from esata_auto_backup;");
}

int write_esata_auto_backup(const char *pDbFile, ESATA_AUTO_BACKUP *autoBackup)
{
    return write_auto_backup(pDbFile, autoBackup, "update esata_auto_backup set enable=%d, backup_day=%d, backup_time='%s', stream_type=%d, file_type=%d, recycle=%d, tri_channels='%s', device='%d', port='%d';");
}

int read_anpr_auto_backup(const char *pDbFile, ESATA_AUTO_BACKUP *autoBackup)
{
    return read_auto_backup(pDbFile, autoBackup, "select enable,backup_day,backup_time,stream_type,file_type,recycle,tri_channels, device, port from anpr_auto_backup;");
}

int write_anpr_auto_backup(const char *pDbFile, ESATA_AUTO_BACKUP *autoBackup)
{
    return write_auto_backup(pDbFile, autoBackup, "update anpr_auto_backup set enable=%d, backup_day=%d, backup_time='%s', stream_type=%d, file_type=%d, recycle=%d, tri_channels='%s', device='%d', port='%d';");
}
int read_pos_setting(const char *pDbFile, Db_POS_CONFIG *pos, int id)
{
    if (!pDbFile || !pos) {
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
    snprintf(sQuery, sizeof(sQuery), "select id,enable,name,protocol,connection_mode,ip,liveview_display,\
        display_channel,start_x,start_y,area_width,area_height,character_encodeing,font_size,font_color,\
        overlay_mode,display_time,timeout,privacy1,privacy2,privacy3,port,clear_time,nvr_port from pos_setting where id=%d;", id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, &pos->id);
    sqlite_query_column(hStmt, 1, &pos->enable);
    sqlite_query_column_text(hStmt, 2, pos->name, sizeof(pos->name));
    sqlite_query_column(hStmt, 3, &pos->protocol);
    sqlite_query_column(hStmt, 4, &pos->connectionMode);
    sqlite_query_column_text(hStmt, 5, pos->ip, sizeof(pos->ip));
    sqlite_query_column(hStmt, 6, &pos->liveviewDisplay);
    sqlite_query_column(hStmt, 7, &pos->displayChannel);
    sqlite_query_column(hStmt, 8, &pos->startX);
    sqlite_query_column(hStmt, 9, &pos->startY);
    sqlite_query_column(hStmt, 10, &pos->areaWidth);
    sqlite_query_column(hStmt, 11, &pos->areaHeight);
    sqlite_query_column(hStmt, 12, &pos->characterEncodeing);
    sqlite_query_column(hStmt, 13, &pos->fontSize);
    sqlite_query_column(hStmt, 14, &pos->fontColor);
    sqlite_query_column(hStmt, 15, &pos->overlayMode);
    sqlite_query_column(hStmt, 16, &pos->displayTime);
    sqlite_query_column(hStmt, 17, &pos->timeout);
    sqlite_query_column_text(hStmt, 18, pos->privacy1, sizeof(pos->privacy1));
    sqlite_query_column_text(hStmt, 19, pos->privacy2, sizeof(pos->privacy2));
    sqlite_query_column_text(hStmt, 20, pos->privacy3, sizeof(pos->privacy3));
    sqlite_query_column(hStmt, 21, &pos->port);
    sqlite_query_column(hStmt, 22, &pos->clearTime);
    sqlite_query_column(hStmt, 23, &pos->nvrPort);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_pos_settings(const char *pDbFile, Db_POS_CONFIG pos[])
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
    int i, nColumnCnt = 0;

    snprintf(sQuery, sizeof(sQuery), "select id,enable,name,protocol,connection_mode,ip,liveview_display,\
        display_channel,start_x,start_y,area_width,area_height,character_encodeing,font_size,font_color,\
        overlay_mode,display_time,timeout,privacy1,privacy2,privacy3,port,clear_time,nvr_port from pos_setting;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < MAX_POS_CLIENT; i++) {
        sqlite_query_column(hStmt, 0, &pos[i].id);
        sqlite_query_column(hStmt, 1, &pos[i].enable);
        sqlite_query_column_text(hStmt, 2, pos[i].name, sizeof(pos[i].name));
        sqlite_query_column(hStmt, 3, &pos[i].protocol);
        sqlite_query_column(hStmt, 4, &pos[i].connectionMode);
        sqlite_query_column_text(hStmt, 5, pos[i].ip, sizeof(pos[i].ip));
        sqlite_query_column(hStmt, 6, &pos[i].liveviewDisplay);
        sqlite_query_column(hStmt, 7, &pos[i].displayChannel);
        sqlite_query_column(hStmt, 8, &pos[i].startX);
        sqlite_query_column(hStmt, 9, &pos[i].startY);
        sqlite_query_column(hStmt, 10, &pos[i].areaWidth);
        sqlite_query_column(hStmt, 11, &pos[i].areaHeight);
        sqlite_query_column(hStmt, 12, &pos[i].characterEncodeing);
        sqlite_query_column(hStmt, 13, &pos[i].fontSize);
        sqlite_query_column(hStmt, 14, &pos[i].fontColor);
        sqlite_query_column(hStmt, 15, &pos[i].overlayMode);
        sqlite_query_column(hStmt, 16, &pos[i].displayTime);
        sqlite_query_column(hStmt, 17, &pos[i].timeout);
        sqlite_query_column_text(hStmt, 18, pos[i].privacy1, sizeof(pos[i].privacy1));
        sqlite_query_column_text(hStmt, 19, pos[i].privacy2, sizeof(pos[i].privacy2));
        sqlite_query_column_text(hStmt, 20, pos[i].privacy3, sizeof(pos[i].privacy3));
        sqlite_query_column(hStmt, 21, &pos[i].port);
        sqlite_query_column(hStmt, 22, &pos[i].clearTime);
        sqlite_query_column(hStmt, 23, &pos[i].nvrPort);
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_pos_setting(const char *pDbFile, Db_POS_CONFIG *pos)
{
    if (!pDbFile || !pos) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[2048] = {0};
    char tmp1[128] = {0};
    char tmp2[256] = {0};
    char tmp3[256] = {0};
    char tmp4[256] = {0};

    translate_pwd(tmp1, pos->name, strlen(pos->name));
    translate_pwd(tmp2, pos->privacy1, strlen(pos->privacy1));
    translate_pwd(tmp3, pos->privacy2, strlen(pos->privacy2));
    translate_pwd(tmp4, pos->privacy3, strlen(pos->privacy3));
    
    snprintf(sExec, sizeof(sExec), "update pos_setting set enable=%d,name='%s',protocol=%d,connection_mode=%d,\
        ip='%s',liveview_display=%d,display_channel=%d,start_x=%d,start_y=%d,area_width=%d,area_height=%d,\
        character_encodeing=%d,font_size=%d,font_color=%d,overlay_mode=%d,display_time=%d,timeout=%d,\
        privacy1='%s',privacy2='%s',privacy3='%s',port=%d,clear_time=%d,nvr_port=%d where id=%d;", pos->enable, tmp1, pos->protocol,
             pos->connectionMode, pos->ip, pos->liveviewDisplay, pos->displayChannel, pos->startX, pos->startY,
             pos->areaWidth, pos->areaHeight, pos->characterEncodeing, pos->fontSize, pos->fontColor, pos->overlayMode,
             pos->displayTime, pos->timeout, tmp2, tmp3, tmp4, pos->port, pos->clearTime, pos->nvrPort, pos->id);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_pos_schedule(const char *pDbFile, SCHEDULE_TYPE type, SMART_SCHEDULE *schedule, int id)
{
    char table[128] = {0};
    char *idName = "id";

    if (type < SCHE_EFFECTIVE || type >= SCHE_MAX) {
        return -1;
    }

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
            idName = "posId";
            break;

        default:
            break;
    }

    return read_comn_sche(pDbFile, schedule, table, idName, id);
}

int write_pos_schedule(const char *pDbFile, SCHEDULE_TYPE type, SMART_SCHEDULE *schedule, int id)
{
    char table[128] = {0};

    if (type < SCHE_EFFECTIVE || type >= SCHE_MAX) {
        return -1;
    }

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

    return write_comn_sche(pDbFile, schedule, table, "id", id);
}

int read_pos_event(const char *pDbFile, SMART_EVENT *smartevent, int id)
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
    HSQLSTMT hStmt = 0;
    char sQuery[2048] = {0};
    int nColumnCnt = 0;

    snprintf(sQuery, sizeof(sQuery), "select id,tri_alarms,tri_chnout1_alarms,tri_chnout2_alarms,buzzer_interval,\
        email_interval,ptzaction_interval,alarmout_interval,whiteled_interval,email_pic_enable,tri_channels_pic,\
        popup_interval,tri_channels_ex,tri_audio_id,http_notification_interval from pos_event where id=%d;", id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, &smartevent->id);
    sqlite_query_column(hStmt, 1, (int *)&smartevent->tri_alarms);
    sqlite_query_column_text(hStmt, 2, smartevent->tri_chnout1_alarms, sizeof(smartevent->tri_chnout1_alarms));
    sqlite_query_column_text(hStmt, 3, smartevent->tri_chnout2_alarms, sizeof(smartevent->tri_chnout2_alarms));
    sqlite_query_column(hStmt, 4, &smartevent->buzzer_interval);
    sqlite_query_column(hStmt, 5, &smartevent->email_interval);
    sqlite_query_column(hStmt, 6, &smartevent->ptzaction_interval);
    sqlite_query_column(hStmt, 7, &smartevent->alarmout_interval);
    sqlite_query_column(hStmt, 8, &smartevent->whiteled_interval);
    sqlite_query_column(hStmt, 9, &smartevent->email_pic_enable);
    sqlite_query_column_text(hStmt, 10, smartevent->tri_channels_pic, sizeof(smartevent->tri_channels_pic));
    sqlite_query_column(hStmt, 11, &smartevent->popup_interval);
    sqlite_query_column_text(hStmt, 12, smartevent->tri_channels_ex, sizeof(smartevent->tri_channels_ex));
    sqlite_query_column(hStmt, 13, &smartevent->tri_audio_id);
    sqlite_query_column(hStmt, 14, &smartevent->http_notification_interval);
    
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_pos_event(const char *pDbFile, SMART_EVENT *smartevent)
{
    if (!pDbFile || !smartevent) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[2048] = {0};
    snprintf(sExec, sizeof(sExec), "update pos_event set tri_alarms=%d,tri_chnout1_alarms='%s',tri_chnout2_alarms='%s',\
            buzzer_interval=%d, email_interval=%d,ptzaction_interval=%d,alarmout_interval=%d,whiteled_interval=%d,\
            email_pic_enable=%d,tri_channels_pic='%s',popup_interval=%d,tri_channels_ex='%s',\
            tri_audio_id=%d,http_notification_interval=%d where id=%d;",
             smartevent->tri_alarms, smartevent->tri_chnout1_alarms, smartevent->tri_chnout2_alarms,
             smartevent->buzzer_interval, smartevent->email_interval, smartevent->ptzaction_interval,
             smartevent->alarmout_interval, smartevent->whiteled_interval, smartevent->email_pic_enable,
             smartevent->tri_channels_pic, smartevent->popup_interval, smartevent->tri_channels_ex,
             smartevent->tri_audio_id, smartevent->http_notification_interval, smartevent->id);

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

static int get_regional_pcnt_table_name(SCHEDULE_TYPE type, int regionNo, char *table, int size)
{
    if (regionNo < 0 || regionNo >= MAX_IPC_PCNT_REGION || !table || size <= 0) {
        return -1;
    }

    int ret = 0;
    if (type == SCHE_EFFECTIVE) {
        if (regionNo == 0) {
            snprintf(table, size, "%s", "regional_pcnt_effective_sche");
        } else {
            snprintf(table, size, "regional_pcnt%d_effective_sche", regionNo);
        }
    } else if (type == SCHE_AUDIO) {
        if (regionNo == 0) {
            snprintf(table, size, "%s", "regional_pcnt_audio_sche");
        } else {
            snprintf(table, size, "regional_pcnt%d_audio_sche", regionNo);
        }
    } else if (type == SCHE_EMAIL) {
        if (regionNo == 0) {
            snprintf(table, size, "%s", "regional_pcnt_email_sche");
        } else {
            snprintf(table, size, "regional_pcnt%d_email_sche", regionNo);
        }
    } else if (type == SCHE_PTZ) {
        if (regionNo == 0) {
            snprintf(table, size, "%s", "regional_pcnt_ptz_sche");
        } else {
            snprintf(table, size, "regional_pcnt%d_ptz_sche", regionNo);
        }
    } else if (type == SCHE_WHITELED) {
        if (regionNo == 0) {
            snprintf(table, size, "%s", "regional_pcnt_whiteled_sche");
        } else {
            snprintf(table, size, "regional_pcnt%d_whiteled_sche", regionNo);
        }
    } else if (type == SCHE_POPUP) {
        if (regionNo == 0) {
            snprintf(table, size, "%s", "regional_pcnt_popup_sche");
        } else {
            snprintf(table, size, "regional_pcnt%d_popup_sche", regionNo);
        }
    } else if (type == SCHE_HTTP) {
        if (regionNo == 0) {
            snprintf(table, size, "%s", "regional_pcnt_http_notification_sche");
        } else {
            snprintf(table, size, "regional_pcnt%d_http_notification_sche", regionNo);
        }
    } else {
        ret = -1;
    }

    return ret;
}

int read_regional_pcnt_schedule(const char *pDbFile, SCHEDULE_TYPE type, SMART_SCHEDULE *schedule, int id, int regionNo)
{
    char table[128] = {0};
    if (!get_regional_pcnt_table_name(type, regionNo, table, sizeof(table))) {
        if (type == SCHE_HTTP) {
            return read_comn_sche(pDbFile, schedule, table, "chnid", id);
        } else {
            return read_comn_sche(pDbFile, schedule, table, "id", id);
        }
    }

    return -1;
}

int write_regional_pcnt_schedule(const char *pDbFile, SCHEDULE_TYPE type, SMART_SCHEDULE *schedule, int id, int regionNo)
{
    char table[128] = {0};
    if (!get_regional_pcnt_table_name(type, regionNo, table, sizeof(table))) {
        if (type == SCHE_HTTP) {
            return write_comn_sche(pDbFile, schedule, table, "chnid", id);
        } else {
            return write_comn_sche(pDbFile, schedule, table, "id", id);
        }
    }

    return -1;
}

int copy_regional_pcnt_schedule(const char *pDbFile, SCHEDULE_TYPE type, SMART_SCHEDULE *schedule, Uint64 chnMask, int regionNo)
{
    char table[MAX_LEN_64] = {0};
    if (!get_regional_pcnt_table_name(type, regionNo, table, sizeof(table))) {
        if (type == SCHE_HTTP) {
            return copy_comn_sches(pDbFile, schedule, table, "chnid", chnMask);
        } else {
            return copy_comn_sches(pDbFile, schedule, table, "id", chnMask);
        }
    }

    return -1;
}

int read_regional_pcnt_event(const char *pDbFile, SMART_EVENT *smartevent, int id, int regionNo)
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
    HSQLSTMT hStmt = 0;
    char sQuery[2048] = {0};
    int nColumnCnt = 0;
    char table[32] = {0};

    if (regionNo == 0) {
        snprintf(table, sizeof(table), "%s", "regional_pcnt_event");
    } else {
        snprintf(table, sizeof(table), "regional_pcnt%d_event", regionNo);
    }

    snprintf(sQuery, sizeof(sQuery), "select id,tri_alarms,tri_chnout1_alarms,tri_chnout2_alarms,buzzer_interval,\
        email_interval,ptzaction_interval,alarmout_interval,whiteled_interval,email_pic_enable,tri_channels_pic,\
        popup_interval,tri_channels_ex,tri_audio_id,http_notification_interval from %s where id=%d;", table, id);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, &smartevent->id);
    sqlite_query_column(hStmt, 1, (int *)&smartevent->tri_alarms);
    sqlite_query_column_text(hStmt, 2, smartevent->tri_chnout1_alarms, sizeof(smartevent->tri_chnout1_alarms));
    sqlite_query_column_text(hStmt, 3, smartevent->tri_chnout2_alarms, sizeof(smartevent->tri_chnout2_alarms));
    sqlite_query_column(hStmt, 4, &smartevent->buzzer_interval);
    sqlite_query_column(hStmt, 5, &smartevent->email_interval);
    sqlite_query_column(hStmt, 6, &smartevent->ptzaction_interval);
    sqlite_query_column(hStmt, 7, &smartevent->alarmout_interval);
    sqlite_query_column(hStmt, 8, &smartevent->whiteled_interval);
    sqlite_query_column(hStmt, 9, &smartevent->email_pic_enable);
    sqlite_query_column_text(hStmt, 10, smartevent->tri_channels_pic, sizeof(smartevent->tri_channels_pic));
    sqlite_query_column(hStmt, 11, &smartevent->popup_interval);
    sqlite_query_column_text(hStmt, 12, smartevent->tri_channels_ex, sizeof(smartevent->tri_channels_ex));
    sqlite_query_column(hStmt, 13, &smartevent->tri_audio_id);
    sqlite_query_column(hStmt, 14, &smartevent->http_notification_interval);
    
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_regional_pcnt_event(const char *pDbFile, SMART_EVENT *smartevent, int regionNo)
{
    if (!pDbFile || !smartevent) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[2048] = {0};
    char table[32] = {0};
    
    if (regionNo == 0) {
        snprintf(table, sizeof(table), "%s", "regional_pcnt_event");
    } else {
        snprintf(table, sizeof(table), "regional_pcnt%d_event", regionNo);
    }
    
    snprintf(sExec, sizeof(sExec), "update %s set tri_alarms=%d,tri_chnout1_alarms='%s',tri_chnout2_alarms='%s',\
            buzzer_interval=%d, email_interval=%d,ptzaction_interval=%d,alarmout_interval=%d,whiteled_interval=%d,\
            email_pic_enable=%d,tri_channels_pic='%s',popup_interval=%d,tri_channels_ex='%s',tri_audio_id=%d,\
            http_notification_interval=%d where id=%d;",
             table, smartevent->tri_alarms, smartevent->tri_chnout1_alarms, smartevent->tri_chnout2_alarms,
             smartevent->buzzer_interval, smartevent->email_interval, smartevent->ptzaction_interval,
             smartevent->alarmout_interval, smartevent->whiteled_interval, smartevent->email_pic_enable,
             smartevent->tri_channels_pic, smartevent->popup_interval, smartevent->tri_channels_ex,
             smartevent->tri_audio_id, smartevent->http_notification_interval, smartevent->id);

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_regional_pcnt_event(const char *pDbFile, SMART_EVENT *smartEvent, int regionNo, Uint64 chnMask)
{
    char table[MAX_LEN_64] = {0};

    if (regionNo == 0) {
        snprintf(table, sizeof(table), "%s", "regional_pcnt_event");
    } else if (regionNo > 0 && regionNo < MAX_IPC_PCNT_REGION) {
        snprintf(table, sizeof(table), "regional_pcnt%d_event", regionNo);
    } else {
        return -1;
    }

    return copy_events(pDbFile, smartEvent, table, chnMask);
}


/////////// people cnt db //////////////
int write_peoplecnt_logs(const char *pDbFile, struct peoplecnt_logs *log)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_PCNT_NAME, mode, &global_pcnt_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_pcnt_rwlock);
        return -1;
    }

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[1024] = {0};
    snprintf(sExec, sizeof(sExec),
             "insert or replace into people_cnt_logs values(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d);",
             log->groupid, log->chnid, log->date,
             log->pcntin[0], log->pcntin[1], log->pcntin[2], log->pcntin[3], log->pcntin[4], log->pcntin[5],
             log->pcntin[6], log->pcntin[7], log->pcntin[8], log->pcntin[9], log->pcntin[10], log->pcntin[11],
             log->pcntin[12], log->pcntin[13], log->pcntin[14], log->pcntin[15], log->pcntin[16], log->pcntin[17],
             log->pcntin[18], log->pcntin[19], log->pcntin[20], log->pcntin[21], log->pcntin[22], log->pcntin[23],
             log->pcntout[0], log->pcntout[1], log->pcntout[2], log->pcntout[3], log->pcntout[4], log->pcntout[5],
             log->pcntout[6], log->pcntout[7], log->pcntout[8], log->pcntout[9], log->pcntout[10], log->pcntout[11],
             log->pcntout[12], log->pcntout[13], log->pcntout[14], log->pcntout[15], log->pcntin[16], log->pcntout[17],
             log->pcntout[18], log->pcntout[19], log->pcntout[20], log->pcntout[21], log->pcntout[22], log->pcntout[23]);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_pcnt_rwlock);
    return 0;
}

int write_peoplecnt_log_by_hours(const char *pDbFile, struct peoplecnt_log_hours *log)
{
    if (!pDbFile || !log) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_PCNT_NAME, mode, &global_pcnt_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_pcnt_rwlock);
        return -1;
    }

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    int ret = -1, cols = 0, rows = 0;
    int allcols = 0, allrows = 0;
    int old_groupid = 0, old_chnid = 0, old_date = 0;
    char sExec[1024] = {0};
    char sQuery[MAX_LEN_512] = {0};
    HSQLSTMT hStmt = NULL;

    do {
        snprintf(sQuery, sizeof(sQuery), "select count(*) from people_cnt_logs where (date=%d and groupid=%d and chnid=%d);",
                 log->date, log->groupid, log->chnid);
        ret = sqlite_query_record(hConn, sQuery, &hStmt, &cols);
        if (ret != 0 || cols == 0) {
            msprintf("quert record ret:%d cols:%d failed.", ret, cols);
            break;
        }
        if (sqlite_query_column(hStmt, 0, &rows) || !rows) {
            msprintf("quert record ret:%d cols:%d rows:%d failed.", ret, cols, rows);
            break;
        }
    } while (0);
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
    }

    if (!allrows) {
        do {
            snprintf(sQuery, sizeof(sQuery), "select count(*) from people_cnt_logs;");
            ret = sqlite_query_record(hConn, sQuery, &hStmt, &allcols);
            if (ret != 0 || allcols == 0) {
                break;
            }
            if (sqlite_query_column(hStmt, 0, &allrows) || !allrows) {
                break;
            }
        } while (0);
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
    }
    if ((allrows >= MAX_PEOPLECNT_LOG_LIMIT) && (!rows)) {
        do {
            snprintf(sQuery, sizeof(sQuery), "select groupid,chnid,MIN(date) from people_cnt_logs;");
            ret = sqlite_query_record_fast(hConn, sQuery, &hStmt, &cols);
            if (ret != 0 || cols == 0) {
                msprintf("read people cnt ret:%d cols:%d.", ret, cols);
                break;
            }

            sqlite_query_column(hStmt, 0, &old_groupid);
            sqlite_query_column(hStmt, 1, &old_chnid);
            sqlite_query_column(hStmt, 2, &old_date);

            snprintf(sExec, sizeof(sExec), "delete from people_cnt_logs where (groupid=%d and chnid=%d and date=%d)", old_groupid,
                     old_chnid, old_date);
            sqlite_execute(hConn, mode, sExec);
        } while (0);
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
    }

    if (!rows) {
        snprintf(sExec, sizeof(sExec),
                 "insert or replace into people_cnt_logs values(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d);",
                 log->groupid, log->chnid, log->date,
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
        sqlite_execute(hConn, mode, sExec);
    }
    snprintf(sExec, sizeof(sExec),
             "update people_cnt_logs set pcntin%d=%d,pcntout%d=%d where (groupid=%d and chnid=%d and date=%d);",
             log->hour, log->pcntin, log->hour, log->pcntout, log->groupid, log->chnid, log->date);
    sqlite_execute(hConn, mode, sExec);

    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_pcnt_rwlock);
    return 0;
}

int read_peoplecnt_logs(const char *pDbFile, struct peoplecnt_logs **log, int *pCnt, SEARCH_PEOPLECNT_LOGS *info)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_PCNT_NAME, mode, &global_pcnt_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_pcnt_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = NULL;
    HSQLSTMT qStmt = NULL;
    int ret, cols, rows = 0, i = 0, n = 0, sflag = 0, val = 0;
    struct peoplecnt_logs *palarm = NULL;
    char buf[MAX_LEN_1024] = { 0 };
    char sQuery[MAX_LEN_512] = {0};
    do {
        snprintf(sQuery, sizeof(sQuery), "%s", "where ");
        if (info->type == PEOPLECNT_SEARCH_BY_GROUP) {
            for (i = 0; i < MAX_PEOPLECNT_GROUP; i++) {
                if (info->id & ((Uint64)1 << i)) {
                    if (!sflag) {
                        snprintf(sQuery + strlen(sQuery), sizeof(sQuery) - strlen(sQuery), "(groupid=%d", i);
                        sflag = 1;
                    } else {
                        snprintf(sQuery + strlen(sQuery), sizeof(sQuery) - strlen(sQuery), " or groupid=%d", i);
                    }
                }
            }
        } else {
            for (i = 0; i < MAX_REAL_CAMERA; i++) {
                if (info->id & ((Uint64)1 << i)) {
                    if (!sflag) {
                        snprintf(sQuery + strlen(sQuery), sizeof(sQuery) - strlen(sQuery), "(chnid=%d", i);
                        sflag = 1;
                    } else {
                        snprintf(sQuery + strlen(sQuery), sizeof(sQuery) - strlen(sQuery), " or chnid=%d", i);
                    }
                }
            }
        }
        snprintf(sQuery + strlen(sQuery), sizeof(sQuery) - strlen(sQuery), ") and (date>=%d and date<=%d)", info->pdstart,
                 info->pdend);
        snprintf(buf, sizeof(buf), "select count(*) from people_cnt_logs %s;", sQuery);
        //msprintf("[david debug] db cmd:%s", buf);
        ret = sqlite_query_record_fast(hConn, buf, &hStmt, &cols);
        if (ret != 0 || cols == 0) {
            msprintf("[david debug] read people cnt ret:%d cols:%d.", ret, cols);
            break;
        }
        if (sqlite_query_column(hStmt, 0, &rows) || !rows) {
            msprintf("[david debug] read people cnt rows:%d.", rows);
            break;
        }

        snprintf(buf, sizeof(buf),
                 "select groupid,chnid,date,pcntin0,pcntin1,pcntin2,pcntin3,pcntin4,pcntin5,pcntin6,pcntin7,pcntin8,pcntin9,pcntin10,pcntin11,pcntin12,pcntin13,pcntin14,pcntin15,pcntin16,pcntin17,pcntin18,pcntin19,pcntin20,pcntin21,pcntin22,pcntin23,pcntout0,pcntout1,pcntout2,pcntout3,pcntout4,pcntout5,pcntout6,pcntout7,pcntout8,pcntout9,pcntout10,pcntout11,pcntout12,pcntout13,pcntout14,pcntout15,pcntout16,pcntout17,pcntout18,pcntout19,pcntout20,pcntout21,pcntout22,pcntout23 from people_cnt_logs %s;",
                 sQuery);
        ret = sqlite_query_record_fast(hConn, buf, &qStmt, &cols);
        if (ret != 0 || cols == 0) {
            msprintf("[david debug] read people cnt ret:%d cols:%d.", ret, cols);
            break;
        }
        if ((palarm = ms_calloc(rows, sizeof(struct peoplecnt_logs))) == NULL) {
            msprintf("[david debug] read people cnt rows:%d.", rows);
            break;
        }

        i = 0;
        do {
            sqlite_query_column(qStmt, 0, &palarm[i].groupid);
            sqlite_query_column(qStmt, 1, &palarm[i].chnid);
            sqlite_query_column(qStmt, 2, &palarm[i].date);
            for (n = 0; n < MAX_LEN_24; n++) {
                sqlite_query_column(qStmt, n + 3, &val);
                palarm[i].pcntin[n] = (Int16)val;
            }
            for (n = 0; n < MAX_LEN_24; n++) {
                sqlite_query_column(qStmt, n + 27, &val);
                palarm[i].pcntout[n] = (Int16)val;
            }
            if ((++i) >= (rows)) {
                break;
            }
        } while (sqlite_query_next(qStmt) == 0);

        *pCnt = rows;
        *log = palarm;
    } while (0);
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
    }
    if (qStmt) {
        sqlite_clear_stmt(qStmt);
    }
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_pcnt_rwlock);
    return 0;
}

void release_peoplecnt_log(struct peoplecnt_logs **log)
{
    if (!log || !*log) {
        return;
    }

    ms_free(*log);
    *log = NULL;
}

int read_peoplecnt_logs_by_date(const char *pDbFile, PEOPLECNT_INFO *peopleCnt, int ndate)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_PCNT_NAME, mode, &global_pcnt_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_pcnt_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = NULL;
    HSQLSTMT qStmt = NULL;
    int ret, cols, rows = 0, i = 0, n = 0, val = 0;
    struct peoplecnt_logs palarm = {0};
    char sQuery[MAX_LEN_512] = {0};
    do {
        snprintf(sQuery, sizeof(sQuery), "select count(*) from people_cnt_logs where date=%d;", ndate);

        ret = sqlite_query_record(hConn, sQuery, &hStmt, &cols);
        if (ret != 0 || !cols) {
            msprintf("[david debug] quert record ret:%d cols:%d failed.", ret, cols);
            break;
        }
        if (sqlite_query_column(hStmt, 0, &rows) || !rows) {
            msprintf("[david debug] quert record ret:%d cols:%d rows:%d failed.", ret, cols, rows);
            break;
        }
        msprintf("[david debug] ret:%d cols:%d rows:%d", ret, cols, rows);

        snprintf(sQuery, sizeof(sQuery),
                 "select groupid,chnid,date,pcntin0,pcntin1,pcntin2,pcntin3,pcntin4,pcntin5,pcntin6,pcntin7,pcntin8,pcntin9,pcntin10,pcntin11,pcntin12,pcntin13,pcntin14,pcntin15,pcntin16,pcntin17,pcntin18,\
			pcntin19,pcntin20,pcntin21,pcntin22,pcntin23,pcntout0,pcntout1,pcntout2,pcntout3,pcntout4,pcntout5,pcntout6,pcntout7,pcntout8,pcntout9,pcntout10,pcntout11,pcntout12,pcntout13,pcntout14,pcntout15,pcntout16,pcntout17,\
			pcntout18,pcntout19,pcntout20,pcntout21,pcntout22,pcntout23 from people_cnt_logs where date=%d;", ndate);
        ret = sqlite_query_record(hConn, sQuery, &qStmt, &cols);
        if (ret != 0 || cols == 0) {
            msprintf("[david debug] quert record ret:%d cols:%d failed.", ret, cols);
            break;
        }

        do {
            memset(&palarm, 0, sizeof(struct peoplecnt_logs));
            sqlite_query_column(qStmt, 0, &palarm.groupid);
            sqlite_query_column(qStmt, 1, &palarm.chnid);
            sqlite_query_column(qStmt, 2, &palarm.date);
            for (n = 0; n < MAX_LEN_24; n++) {
                sqlite_query_column(qStmt, n + 3, &val);
                palarm.pcntin[n] = (Int16)val;
            }
            for (n = 0; n < MAX_LEN_24; n++) {
                sqlite_query_column(qStmt, n + 27, &val);
                palarm.pcntout[n] = (Int16)val;
            }
            i++;
            if (palarm.groupid >= 0 && palarm.groupid < MAX_PEOPLECNT_GROUP
                && palarm.chnid >= 0 && palarm.chnid < MAX_REAL_CAMERA) {
                memcpy(&peopleCnt->logs[palarm.groupid][palarm.chnid], &palarm, sizeof(struct peoplecnt_logs));
            }
        } while (sqlite_query_next(qStmt) == 0 && i < rows);
    } while (0);
    if (hStmt) {
        sqlite_clear_stmt(hStmt);
    }
    if (qStmt) {
        sqlite_clear_stmt(qStmt);
    }
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_pcnt_rwlock);
    return 0;
}

int delete_peoplecnt_logs_by_grouid(const char *pDbFile, int groupid)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_PCNT_NAME, mode, &global_pcnt_rwlock);
    char sExec[256] = {0};
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_pcnt_rwlock);
        return -1;
    }

    snprintf(sExec, sizeof(sExec), "delete from people_cnt_logs where groupid=%d;", groupid);

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_pcnt_rwlock);
    return 0;
}

int delete_peoplecnt_logs_by_date(const char *pDbFile, int date)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_PCNT_NAME, mode, &global_pcnt_rwlock);
    char sExec[256] = {0};
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_pcnt_rwlock);
        return -1;
    }

    snprintf(sExec, sizeof(sExec), "delete from people_cnt_logs where date<=%d;", date);

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_pcnt_rwlock);
    return 0;
}

int read_peoplecnt_current(const char *pDbFile, PEOPLECNT_CURRENT *pCur, int groupid)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_PCNT_NAME, mode, &global_pcnt_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_pcnt_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[2048] = {0};
    int nColumnCnt = 0;
    int tmpIn = 0, tmpOut = 0;

    snprintf(sQuery, sizeof(sQuery), "select groupid,pcntin,pcntout from people_cnt_current where groupid=%d;", groupid);
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_pcnt_rwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, &pCur->groupid);
    sqlite_query_column(hStmt, 1, &tmpIn);
    sqlite_query_column(hStmt, 2, &tmpOut);
    pCur->pcntin = (short)tmpIn;
    pCur->pcntin = (short)tmpOut;

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_pcnt_rwlock);
    return 0;
}

int read_peoplecnt_currents(const char *pDbFile, PEOPLECNT_CURRENT pCur[])
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_PCNT_NAME, mode, &global_pcnt_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_pcnt_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[2048] = {0};
    int i, nColumnCnt = 0;
    int tmpIn = 0, tmpOut = 0;

    snprintf(sQuery, sizeof(sQuery), "select groupid,pcntin,pcntout from people_cnt_current;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_pcnt_rwlock);
        return -1;
    }
    for (i = 0; i < MAX_PEOPLECNT_GROUP; i++) {
        sqlite_query_column(hStmt, 0, &pCur[i].groupid);
        sqlite_query_column(hStmt, 1, &tmpIn);
        sqlite_query_column(hStmt, 2, &tmpOut);
        pCur[i].pcntin = (short)tmpIn;
        pCur[i].pcntout = (short)tmpOut;
        if (sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_pcnt_rwlock);
    return 0;
}

int write_peoplecnt_current(const char *pDbFile, PEOPLECNT_CURRENT *pCur)
{
    if (!pDbFile || !pCur) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_PCNT_NAME, mode, &global_pcnt_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_pcnt_rwlock);
        return -1;
    }
    char sExec[2048] = {0};
    snprintf(sExec, sizeof(sExec), "update people_cnt_current set groupid=%d,pcntin=%d,pcntout=%d where groupid=%d;",
             pCur->groupid, pCur->pcntin, pCur->pcntout, pCur->groupid);

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_pcnt_rwlock);
    return 0;
}

int read_push_msgs_by_times(const char *pDbFile, int startTime, int endTime, MSG_CACHE_S *msgCache)
{
    if (!pDbFile || !msgCache) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_PUSHMSG_NAME, mode, &g_pushmsgRwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &g_pushmsgRwlock);
        return -1;
    }

    HSQLSTMT hStmt = 0;
    char sQuery[2048] = {0};
    int i, nColumnCnt = 0;
    int nResult = -1;
    double tmp;

    snprintf(sQuery, sizeof(sQuery), "select chnId,alarmTime,eventType,recordChn,UTCTime,recordType,port,object,"
        "hasSnapshot from msg where UTCTime >=%d and UTCTime<=%d order by UTCTime desc;", startTime, endTime);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0  || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &g_pushmsgRwlock);
        return -1;
    }

    for (i = 0; i < MAX_MSG_NUM; ++i) {
        sqlite_query_column_double(hStmt, 0, &tmp);
        msgCache->pushMsgArr[i].chnId = (Uint64)tmp;
        sqlite_query_column_text(hStmt, 1, msgCache->pushMsgArr[i].alarmTime, sizeof(msgCache->pushMsgArr[i].alarmTime));
        sqlite_query_column(hStmt, 2, (int *)&msgCache->pushMsgArr[i].eventType);
        sqlite_query_column_double(hStmt, 3, &tmp);
        msgCache->pushMsgArr[i].recordChn = (Uint64)tmp;
        sqlite_query_column(hStmt, 4, (int *)&msgCache->pushMsgArr[i].UTCTime);
        sqlite_query_column_text(hStmt, 5, msgCache->pushMsgArr[i].recordType, sizeof(msgCache->pushMsgArr[i].recordType));
        sqlite_query_column(hStmt, 6, (int *)&msgCache->pushMsgArr[i].port);
        sqlite_query_column_text(hStmt, 7, msgCache->pushMsgArr[i].objStr, sizeof(msgCache->pushMsgArr[i].objStr));
        sqlite_query_column(hStmt, 8, (int *)&msgCache->pushMsgArr[i].hasSnapshot);
        ++msgCache->cnt;

        if (sqlite_query_next(hStmt) != 0) {
            ++i;
            break;
        }
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &g_pushmsgRwlock);

    return 0;
}

int get_push_msg_cnt(const char *pDbFile, int *cnt)
{
    if (!pDbFile || !cnt) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_PUSHMSG_NAME, mode, &g_pushmsgRwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &g_pushmsgRwlock);
        return -1;
    }

    HSQLSTMT hStmt = 0;
    char sQuery[1024] = {0};
    int nColumnCnt = 0;

    snprintf(sQuery, sizeof(sQuery), "select count(*) from msg;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &g_pushmsgRwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, cnt);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);

    FileUnlock(nFd, mode, &g_pushmsgRwlock);

    return 0;
}

int write_push_msgs_from_cache(const char *pDbFile, int cnt, MSG_CACHE_S *msgCache, int *id)
{
    if (!pDbFile || !msgCache) {
        return -1;
    }

    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_PUSHMSG_NAME, mode, &g_pushmsgRwlock);
    char sExec[512] = {0};
    HSQLITE hConn = 0;
    int i, index;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &g_pushmsgRwlock);
        return -1;
    }

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (i = 0; i < msgCache->cnt; ++i) {
        if (i == MAX_MSG_NUM - cnt) {
            break;
        }
        index = (msgCache->offset + i) % MAX_MSG_NUM;
        snprintf(sExec, sizeof(sExec),
            "insert into msg(chnId,alarmTime,eventType,recordChn,UTCTime,recordType,port,object,hasSnapshot) "
            "values(%llu,'%s',%d,%llu,%d,'%s',%d,'%s',%d);",
            msgCache->pushMsgArr[index].chnId, msgCache->pushMsgArr[index].alarmTime, msgCache->pushMsgArr[index].eventType,
            msgCache->pushMsgArr[index].recordChn, msgCache->pushMsgArr[index].UTCTime, msgCache->pushMsgArr[index].recordType,
            msgCache->pushMsgArr[index].port, msgCache->pushMsgArr[index].objStr, msgCache->pushMsgArr[index].hasSnapshot);
        sqlite_execute(hConn, mode, sExec);
    }
    for (; i < msgCache->cnt; ++i) {
        index = (msgCache->offset + i) % MAX_MSG_NUM;
        snprintf(sExec, sizeof(sExec),
            "update msg set chnId=%llu,alarmTime='%s',eventType=%d,recordChn=%llu,UTCTime=%d,recordType='%s',"
            "port=%d,object='%s',hasSnapshot=%d where id=%d;",
            msgCache->pushMsgArr[index].chnId, msgCache->pushMsgArr[index].alarmTime, msgCache->pushMsgArr[index].eventType,
            msgCache->pushMsgArr[index].recordChn, msgCache->pushMsgArr[index].UTCTime, msgCache->pushMsgArr[index].recordType,
            msgCache->pushMsgArr[index].port, msgCache->pushMsgArr[index].objStr, msgCache->pushMsgArr[index].hasSnapshot, 
            (*id)++);
        sqlite_execute(hConn, mode, sExec);

        if ((*id) > 1000) {
            (*id) = 1;
        }
    }

    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &g_pushmsgRwlock);

    return 0;
}

int read_push_msg_index(const char *pDbFile, int *index)
{
    if (!pDbFile) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_PUSHMSG_NAME, mode, &g_pushmsgRwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &g_pushmsgRwlock);
        return -1;
    }

    HSQLSTMT hStmt = 0;
    char sQuery[2048] = {0};
    int nColumnCnt = 0;
    int nResult = -1;

    snprintf(sQuery, sizeof(sQuery), "select id from msgIndex;");
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0  || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &g_pushmsgRwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, index);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &g_pushmsgRwlock);

    return 0;
}

int update_push_msg_index(const char *pDbFile, int index)
{
    if (!pDbFile) {
        return -1;
    }

    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_PUSHMSG_NAME, mode, &g_pushmsgRwlock);
    char sExec[512] = {0};
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &g_pushmsgRwlock);
        return -1;
    }

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    snprintf(sExec, sizeof(sExec), "update msgIndex set id=%d;", index);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &g_pushmsgRwlock);

    return 0;
}


int read_alarm_audio_file(const char *pDbFile, AUDIO_FILE audioFile[])
{
    if (!pDbFile || !audioFile) {
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
    int i, nColumnCnt = 0;

    snprintf(sQuery, sizeof(sQuery), "select id, enable, name from alarm_audio_file;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < MAX_AUDIO_FILE_NUM; i++) {
        sqlite_query_column(hStmt, 0, &audioFile[i].id);
        sqlite_query_column(hStmt, 1, &audioFile[i].enable);
        sqlite_query_column_text(hStmt, 2, audioFile[i].name, sizeof(audioFile[i].name));

        if(sqlite_query_next(hStmt) != 0) {
            i++;
            break;
        }
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_alarm_audio_file(const char *pDbFile, AUDIO_FILE *audioFile)
{
    if (!pDbFile || !audioFile) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "update alarm_audio_file set enable=%d, name='%s' where id=%d;",
            audioFile->enable, audioFile->name, audioFile->id);

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int clear_alarm_audio_file(const char *pDbFile)
{
    if (!pDbFile) {
        return -1;
    }
    HSQLITE hConn = 0;
    char sExec[256] = {0};
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    snprintf(sExec, sizeof(sExec), "update alarm_audio_file set enable=0, name='';");
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_push_msg_nvr_event(const char *pDbFile, struct PushMsgNvrEvent *db)
{
    if (!pDbFile || !db) {
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
    int nColumnCnt = 0;
    char sQuery[512] = {0};
    snprintf(sQuery, sizeof(sQuery),
             "select pos from push_msg_nvr_event;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column_text(hStmt, 0, db->pos, sizeof(db->pos));
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_push_msg_nvr_event(const char *pDbFile, struct PushMsgNvrEvent *db)
{
    if (!pDbFile || !db) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[512] = {0};
    snprintf(sExec, sizeof(sExec), "update push_msg_nvr_event set pos='%s';", db->pos);

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_db_ver_info(const char *pDbFile, DB_VER_INFO_S updateArr[], int updateCnt)
{
    if (!pDbFile || !updateArr) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;

    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    char sExec[2048] = {0};
    int i;
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);

    for (i = 0; i < updateCnt; ++i) {
        snprintf(sExec, sizeof(sExec), "insert into db_version values('%s',%d);", updateArr[i].file, updateArr[i].execute);
        sqlite_execute(hConn, mode, sExec);
    }

    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);

    return 0;

}

int check_table_exist(const char *pDbFile, const char *tblName, int *state)
{
    if (!pDbFile || !tblName || !state) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    HSQLSTMT hStmt = NULL;
    int ret = 0, cols = 0;
    char sql[256] = { 0 };

    snprintf(sql, sizeof(sql), "select count(*) from sqlite_master where type='table' and name='%s'", tblName);
    ret = sqlite_query_record_fast(hConn, sql, &hStmt, &cols);
    if (ret != 0 || cols == 0) {
        *state = 0;
    } else {
        sqlite_query_column(hStmt, 0, state);
    }

    if (hStmt) {
        sqlite_clear_stmt(hStmt);
    }
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);

    return ret;
}

int create_tbl_db_version(const char *pDbFile, int msdbVer)
{
    if (!pDbFile) {
        return -1;
    }

    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    int i;

    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[2048] = {0};

    snprintf(sExec, sizeof(sExec), "CREATE TABLE db_version ('file' TEXT NOT NULL,'execute' INTEGER NOT NULL);");
    sqlite_execute(hConn, mode, sExec);
    for (i = 1001; i <= msdbVer; ++i) {
        snprintf(sExec, sizeof(sExec), "insert into db_version values('db-%d',1)", i);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);

    return 0;
}

int read_db_ver_info(const char *pDbFile, DB_VER_INFO_S dbVerInfo[], int *cnt)
{
    if (!pDbFile) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = NULL;
    if (sqlite_conn(pDbFile, &hConn)) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    HSQLSTMT hStmt = NULL;
    char sQuery[2048] = {0};
    int i, nColumnCnt = 0;
    int nResult = 0;
    snprintf(sQuery, sizeof(sQuery), "select count(*) from db_version;");
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, cnt);

    sqlite_clear_stmt(hStmt);

    snprintf(sQuery, sizeof(sQuery), "select file,execute from db_version;");
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < *cnt; ++i) {
        sqlite_query_column_text(hStmt, 0, dbVerInfo[i].file, sizeof(dbVerInfo[i].file));
        sqlite_query_column(hStmt, 1, (int *)&dbVerInfo[i].execute);

        if (sqlite_query_next(hStmt) != 0) {
            ++i;
            break;
        }
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}


int read_face_audible_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int id)
{
    return read_comn_sche(pDbFile, schedule, "face_audible_schedule", "chn_id", id);
}

int read_face_audible_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int cnt)
{
    return read_comn_sches(pDbFile, schedule, "face_audible_schedule", "chn_id", cnt);
}

int write_face_audible_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int id)
{
    return write_comn_sche(pDbFile, schedule, "face_audible_schedule", "chn_id", id);
}

int copy_face_audible_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, Uint64 changeFlag)
{
    return copy_comn_sches(pDbFile, schedule, "face_audible_schedule", "chn_id", changeFlag);
}

int read_face_effective_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int id)
{
    return read_comn_sche(pDbFile, schedule, "face_effective_schedule", "chn_id", id);
}

int read_face_effective_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int cnt)
{
    return read_comn_sches(pDbFile, schedule, "face_effective_schedule", "chn_id", cnt);
}

int write_face_effective_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int id)
{
    return write_comn_sche(pDbFile, schedule, "face_effective_schedule", "chn_id", id);
}

int copy_face_effective_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, Uint64 changeFlag)
{
    return copy_comn_sches(pDbFile, schedule, "face_effective_schedule", "chn_id", changeFlag);
}

int read_face_mail_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int id)
{
    return read_comn_sche(pDbFile, schedule, "face_mail_schedule", "chn_id", id);
}

int read_face_mail_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int cnt)
{
    return read_comn_sches(pDbFile, schedule, "face_mail_schedule", "chn_id", cnt);
}

int write_face_mail_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int id)
{
    return write_comn_sche(pDbFile, schedule, "face_mail_schedule", "chn_id", id);
}

int copy_face_mail_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, Uint64 changeFlag)
{
    return copy_comn_sches(pDbFile, schedule, "face_mail_schedule", "chn_id", changeFlag);
}

int read_face_popup_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int id)
{
    return read_comn_sche(pDbFile, schedule, "face_popup_schedule", "chn_id", id);
}

int read_face_popup_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int cnt)
{
    return read_comn_sches(pDbFile, schedule, "face_popup_schedule", "chn_id", cnt);
}

int write_face_popup_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int id)
{
    return write_comn_sche(pDbFile, schedule, "face_popup_schedule", "chn_id", id);
}

int copy_face_popup_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, Uint64 changeFlag)
{
    return copy_comn_sches(pDbFile, schedule, "face_popup_schedule", "chn_id", changeFlag);
}

int read_face_ptz_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int id)
{
    return read_comn_sche(pDbFile, schedule, "face_ptz_schedule", "chn_id", id);
}

int read_face_ptz_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int cnt)
{
    return read_comn_sches(pDbFile, schedule, "face_ptz_schedule", "chn_id", cnt);
}

int write_face_ptz_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int id)
{
    return write_comn_sche(pDbFile, schedule, "face_ptz_schedule", "chn_id", id);
}

int copy_face_ptz_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, Uint64 changeFlag)
{
    return copy_comn_sches(pDbFile, schedule, "face_ptz_schedule", "chn_id", changeFlag);
}

int read_face_whiteled_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int id)
{
    return read_comn_sche(pDbFile, schedule, "face_whiteled_schedule", "chn_id", id);
}

int read_face_whiteled_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, int cnt)
{
    return read_comn_sches(pDbFile, schedule, "face_whiteled_schedule", "chn_id", cnt);
}

int write_face_whiteled_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, int id)
{
    return write_comn_sche(pDbFile, schedule, "face_whiteled_schedule", "chn_id", id);
}

int copy_face_whiteled_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, Uint64 changeFlag)
{
    return copy_comn_sches(pDbFile, schedule, "face_whiteled_schedule", "chn_id", changeFlag);
}

int read_face_event(const char *pDbFile, struct smart_event *smartevent, int id)
{
    return read_event(pDbFile, smartevent, "face_event", id);
}

int read_face_events(const char *pDbFile, SMART_EVENT *smartevent, int count)
{
    return read_events(pDbFile, smartevent, "face_event", count);
}

int write_face_event(const char *pDbFile, SMART_EVENT *smartevent)
{
    return write_event(pDbFile, smartevent, "face_event");
}

int write_face_events(const char *pDbFile, SMART_EVENT *smartevent, int count, long long changeFlag)
{
    return write_events(pDbFile, smartevent, "face_event", count, changeFlag);
}

int copy_face_events(const char *pDbFile, SMART_EVENT *smartevent, long long changeFlag)
{
    return copy_events(pDbFile, smartevent, "face_event", (Uint64)changeFlag);
}

int read_multicast(const char *pDbFile, struct DbMulticast *multicast)
{
    if (!pDbFile || !multicast) {
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
    int nColumnCnt = 0;
    char sQuery[512] = {0};
    snprintf(sQuery, sizeof(sQuery), "select enable,ip from multicast;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column(hStmt, 0, &multicast->enable);
    sqlite_query_column_text(hStmt, 1, multicast->ip, sizeof(multicast->ip));
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_multicast(const char *pDbFile, struct DbMulticast *multicast)
{
    if (!pDbFile || !multicast) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    char sExec[512] = {0};
    char tmp[128] = {0};
    translate_pwd(tmp, multicast->ip, strlen(multicast->ip));
    snprintf(sExec, sizeof(sExec), "update multicast set enable=%d,ip='%s';", multicast->enable, tmp);

    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

static int get_table_id_name(const char *pDbTable, char *idName, int size)
{
    if (!pDbTable || !idName || size <= 0) {
        return -1;
    }
    
    if (!strcmp(pDbTable, PCNT_HTTP_PARAMS) || !strcmp(pDbTable, PCNT_HTTP_SCHE)) {
        snprintf(idName, size, "groupid");
    } else if (!strcmp(pDbTable, AIN_HTTP_PARAMS) || !strcmp(pDbTable, AIN_HTTP_SCHE)) {
        snprintf(idName, size, "alarmid");
    } else if (!strcmp(pDbTable, POS_HTTP_PARAMS) || !strcmp(pDbTable, POS_HTTP_SCHE)) {
        snprintf(idName, size, "posid");
    } else if (!strcmp(pDbTable, EXCEPT_HTTP_PARAMS)) {
        snprintf(idName, size, "exceptid");
    } else {
        snprintf(idName, size, "chnid");
    }

    return 0;
}

int read_http_notification_schedule_init(const char *pDbFile, const char *pDbTable, void *params, int maxCnt)
{
    char idName[16] = {0};
    get_table_id_name(pDbTable, idName, sizeof(idName));
    
    return read_comn_sches(pDbFile, (SMART_SCHEDULE *)params, (char *)pDbTable, idName, maxCnt);
}

int read_http_notification_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, char *pDbTable, int id)
{    

    char idName[16] = {0};
    get_table_id_name(pDbTable, idName, sizeof(idName));
    
    return read_comn_sche(pDbFile, schedule, pDbTable, idName, id);
}

int write_http_notification_schedule(const char *pDbFile, SMART_SCHEDULE *schedule, char *pDbTable, int id)
{
    char squery[16] = {0};
    get_table_id_name(pDbTable, squery, sizeof(squery));
    
    return write_comn_sche(pDbFile, schedule, pDbTable, squery, id);
}

int copy_http_notification_schedules(const char *pDbFile, SMART_SCHEDULE *schedule, char *pDbTable, Uint64 masks)
{
    char squery[16] = {0};
    get_table_id_name(pDbTable, squery, sizeof(squery));
    
    return copy_comn_sches(pDbFile, schedule, pDbTable, squery, masks);
}

int read_http_notification_params_init(const char *pDbFile, const char *pDbTable, HTTP_NOTIFICATION_PARAMS_S *pHttpPrms, int maxCnt)
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
    int nResult= 0;
    int i = 0, id = 0;
    char idName[16] = {0};

    get_table_id_name(pDbTable, idName, sizeof(idName));

    snprintf(sQuery, sizeof(sQuery), "select %s,url,username,password from %s;", idName, pDbTable);
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    for (i = 0; i < maxCnt; ++i) {
        sqlite_query_column(hStmt, 0, &id);
        pHttpPrms[id].id = id;
        sqlite_query_column_text(hStmt, 1, pHttpPrms[id].url, sizeof(pHttpPrms[id].url));
        sqlite_query_column_text(hStmt, 2, pHttpPrms[id].username, sizeof(pHttpPrms[id].username));
        sqlite_query_column_text(hStmt, 3, pHttpPrms[id].password, sizeof(pHttpPrms[id].password));
        
        if (sqlite_query_next(hStmt) != 0) {
            break;
        }
    }

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_http_notification_params(const char *pDbFile, HTTP_NOTIFICATION_PARAMS_S *pHttpPrms, const char *pDbTable, int id)
{
    if (!pDbFile || !pHttpPrms) {
        return -1;
    }
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int nColumnCnt = 0;
    HSQLSTMT hStmt = 0;
    char sQuery[2048] = {0};
    char idName[16] = {0};

    get_table_id_name(pDbTable, idName, sizeof(idName));
    snprintf(sQuery, sizeof(sQuery), "select %s,url,username,password from %s where %s=%d;", idName, pDbTable, idName, id);
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
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, &pHttpPrms->id);
    sqlite_query_column_text(hStmt, 1, pHttpPrms->url, sizeof(pHttpPrms->url));
    sqlite_query_column_text(hStmt, 2, pHttpPrms->username, sizeof(pHttpPrms->username));
    sqlite_query_column_text(hStmt, 3, pHttpPrms->password, sizeof(pHttpPrms->password));

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    
    return 0;
}

int write_http_notification_params(const char *pDbFile, HTTP_NOTIFICATION_PARAMS_S *pHttpPrms, const char *pDbTable)
{
    if (!pDbFile || !pHttpPrms || pDbTable[0] == '\0') {
        return -1;
    }
    
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    char sExec[2048] = {0};
    HSQLITE hConn = 0;
    char idName[16] = {0};
    char tmpUrl[MAX_LEN_1024 + 1] = {0};
    char tmpUser[192 + 1] = {0};
    char tmpPwd[MAX_LEN_128 + 1] = {0};

    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    get_table_id_name(pDbTable, idName, sizeof(idName));
    
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    snprintf(sExec, sizeof(sExec), "delete from %s where %s=%d", pDbTable, idName, pHttpPrms->id);
    sqlite_execute(hConn, mode, sExec);
    
    translate_pwd(tmpUrl, pHttpPrms->url, strlen(pHttpPrms->url));
    translate_pwd(tmpUser, pHttpPrms->username, strlen(pHttpPrms->username));
    translate_pwd(tmpPwd, pHttpPrms->password, strlen(pHttpPrms->password));
    snprintf(sExec, sizeof(sExec), "insert into %s(%s,url,username,password) values(%d,'%s','%s','%s');",
             pDbTable, idName, pHttpPrms->id, tmpUrl, tmpUser, tmpPwd);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);
    
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_http_notification_params_batch(const char *pDbFile, HTTP_NOTIFICATION_PARAMS_S *pHttpPrms, const char *pDbTable, int cnt)
{
    if (!pDbFile || !pHttpPrms || pDbTable[0] == '\0' || cnt <= 0) {
        return -1;
    }
    
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    char sExec[2048] = {0};
    HSQLITE hConn = 0;
    int i;
    char idName[16] = {0};
    char tmpUrl[MAX_LEN_1024 + 1] = {0};
    char tmpUser[192 + 1] = {0};
    char tmpPwd[MAX_LEN_128 + 1] = {0};

    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    
    get_table_id_name(pDbTable, idName, sizeof(idName));
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (i = 0; i < cnt; ++i) {
        if (pHttpPrms[i].id == -1) {
            continue;
        }

        snprintf(sExec, sizeof(sExec), "delete from %s where %s=%d", pDbTable, idName, pHttpPrms[i].id);
        sqlite_execute(hConn, mode, sExec);

        translate_pwd(tmpUrl, pHttpPrms[i].url, strlen(pHttpPrms[i].url));
        translate_pwd(tmpUser, pHttpPrms[i].username, strlen(pHttpPrms[i].username));
        translate_pwd(tmpPwd, pHttpPrms[i].password, strlen(pHttpPrms[i].password));
        snprintf(sExec, sizeof(sExec), "insert into %s(%s,url,username,password) values(%d,'%s','%s','%s');",
                 pDbTable, idName, pHttpPrms[i].id, tmpUrl, tmpUser, tmpPwd);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_http_notification_params(const char *pDbFile, const char *pDbTable, HTTP_NOTIFICATION_PARAMS_S *pHttpPrms, Uint64 chnMasks)
{
    if (!pDbFile || !pHttpPrms || !pDbTable || !chnMasks) {
        return -1;
    }

    int i = 0;
    char sExec[2048] = {0};
    HSQLITE hConn = 0;
    char idName[16] = {0};
    char tmpUrl[MAX_LEN_1024 + 1] = {0};
    char tmpUser[192 + 1] = {0};
    char tmpPwd[MAX_LEN_128 + 1] = {0};

    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    get_table_id_name(pDbTable, idName, sizeof(idName));
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (i = 0; i < MAX_REAL_CAMERA; i++) {
        if (!(chnMasks >> i & 0x01)) {
            continue;
        }
        snprintf(sExec, sizeof(sExec), "delete from %s where %s=%d", pDbTable, idName, i);
        sqlite_execute(hConn, mode, sExec);
        translate_pwd(tmpUrl, pHttpPrms->url, strlen(pHttpPrms->url));
        translate_pwd(tmpUser, pHttpPrms->username, strlen(pHttpPrms->username));
        translate_pwd(tmpPwd, pHttpPrms->password, strlen(pHttpPrms->password));
        snprintf(sExec, sizeof(sExec), "insert into %s(%s,url,username,password) values(%d,'%s','%s','%s');",
                 pDbTable, idName, i, tmpUrl, tmpUser, tmpPwd);
        sqlite_execute(hConn, mode, sExec);
    }
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_event_http_notification_params_init(const char *pDbFile, int type, HTTP_NOTIFICATION_PARAMS_S *params, int maxChnCnt)
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
    int id = 0;
    char table[64] = {0};
    char idName[16] = {0};

    get_table_id_name(pDbFile, idName, sizeof(idName));

    do {
        switch (type) {
            case ALARMIO:
                snprintf(table, sizeof(table), "%s", AIN_HTTP_PARAMS);
                break;
            case MOTION:
                snprintf(table, sizeof(table), "%s", MOT_HTTP_PARAMS);
                break;
            case VIDEOLOSS:
                snprintf(table, sizeof(table), "%s", VDL_HTTP_PARAMS);
                break;
            case REGION_EN:
                snprintf(table, sizeof(table), "%s", VREIN_HTTP_PARAMS);
                break;
            case REGION_EXIT:
                snprintf(table, sizeof(table), "%s", VREEX_HTTP_PARAMS);
                break;
            case ADVANCED_MOT:
                snprintf(table, sizeof(table), "%s", VMOT_HTTP_PARAMS);
                break;
            case TAMPER_DET:
                snprintf(table, sizeof(table), "%s", VTEP_HTTP_PARAMS);
                break;
            case LINE_CROSS:
                snprintf(table, sizeof(table), "%s", VLSS_HTTP_PARAMS);
                break;
            case LOITER:
                snprintf(table, sizeof(table), "%s", VLER_HTTP_PARAMS);
                break;
            case HUMAN_DET:
                snprintf(table, sizeof(table), "%s", VHMN_HTTP_PARAMS);
                break;
            case PEOPLE_COUNT:
                snprintf(table, sizeof(table), "%s", VPPE_HTTP_PARAMS);
                break;
            case LEFTREMOVE:
                snprintf(table, sizeof(table), "%s", VOBJ_HTTP_PARAMS);
                break;
            case POS_EVT:
                snprintf(table, sizeof(table), "%s", POS_HTTP_PARAMS);
                break;
            case REGIONAL_PEOPLE_CNT0:
                snprintf(table, sizeof(table), "%s", REGIONAL_PCNT_HTTP_PARAMS);
                break;
            case REGIONAL_PEOPLE_CNT1:
                snprintf(table, sizeof(table), "%s", REGIONAL_PCNT1_HTTP_PARAMS);
                break;
            case REGIONAL_PEOPLE_CNT2:
                snprintf(table, sizeof(table), "%s", REGIONAL_PCNT2_HTTP_PARAMS);
                break;
            case REGIONAL_PEOPLE_CNT3:
                snprintf(table, sizeof(table), "%s", REGIONAL_PCNT3_HTTP_PARAMS);
                break;
            default:
                break;
        }

        snprintf(sQuery, sizeof(sQuery),"select url,username,password from %s where %s=%d;", table, idName, id);
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

        params[id].id = id;
        sqlite_query_column_text(hStmt, 0, params[id].url, sizeof(params[id].url));
        sqlite_query_column_text(hStmt, 1, params[id].username, sizeof(params[id].username));
        sqlite_query_column_text(hStmt, 2, params[id].password, sizeof(params[id].password));

        sqlite_clear_stmt(hStmt);
    } while (++id < maxChnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_smart_event_http_schedules(const char *pDbFile, struct smart_event_schedule *Schedule, int chnId)
{
    if (!pDbFile || !Schedule) {
        return -1;
    }

    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    int i = 0, iSmt = 0;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_SMART_EVENT][256];

    HSQLSTMT hStmt = 0;
    snprintf(sQuery[REGIONIN], sizeof(sQuery[REGIONIN]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_regionin_http_notification_sche where chnid=%d;", chnId);
    snprintf(sQuery[REGIONOUT], sizeof(sQuery[REGIONOUT]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_regionexit_http_notification_sche where chnid=%d;", chnId);
    snprintf(sQuery[ADVANCED_MOTION], sizeof(sQuery[ADVANCED_MOTION]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_motion_http_notification_sche where chnid=%d;", chnId);
    snprintf(sQuery[TAMPER], sizeof(sQuery[TAMPER]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_tamper_http_notification_sche where chnid=%d;", chnId);
    snprintf(sQuery[LINECROSS], sizeof(sQuery[LINECROSS]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_linecross_http_notification_sche where chnid=%d;", chnId);
    snprintf(sQuery[LOITERING], sizeof(sQuery[LOITERING]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_loiter_http_notification_sche where chnid=%d;", chnId);
    snprintf(sQuery[HUMAN], sizeof(sQuery[HUMAN]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_human_http_notification_sche where chnid=%d;", chnId);
    snprintf(sQuery[PEOPLE_CNT], sizeof(sQuery[PEOPLE_CNT]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_people_http_notification_sche where chnid=%d;", chnId);
    snprintf(sQuery[OBJECT_LEFTREMOVE], sizeof(sQuery[OBJECT_LEFTREMOVE]),
             "select week_id,plan_id,start_time,end_time,action_type from vca_object_leftremove_http_notification_sche where chnid=%d;", chnId);

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
            sqlite_query_column_text(hStmt, 2, Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                     sizeof(Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
            sqlite_query_column_text(hStmt, 3, Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                     sizeof(Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
            sqlite_query_column(hStmt, 4, &Schedule[iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
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
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_smart_event_http_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int chnCnt)
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

    int i = 0, iSmt = 0, chnId = 0;;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_SMART_EVENT][256];

    HSQLSTMT hStmt = 0;
    do {
        snprintf(sQuery[REGIONIN], sizeof(sQuery[REGIONIN]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_regionin_http_notification_sche where chnid=%d;", chnId);
        snprintf(sQuery[REGIONOUT], sizeof(sQuery[REGIONOUT]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_regionexit_http_notification_sche where chnid=%d;", chnId);
        snprintf(sQuery[ADVANCED_MOTION], sizeof(sQuery[ADVANCED_MOTION]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_motion_http_notification_sche where chnid=%d;", chnId);
        snprintf(sQuery[TAMPER], sizeof(sQuery[TAMPER]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_tamper_http_notification_sche where chnid=%d;", chnId);
        snprintf(sQuery[LINECROSS], sizeof(sQuery[LINECROSS]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_linecross_http_notification_sche where chnid=%d;", chnId);
        snprintf(sQuery[LOITERING], sizeof(sQuery[LOITERING]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_loiter_http_notification_sche where chnid=%d;", chnId);
        snprintf(sQuery[HUMAN], sizeof(sQuery[HUMAN]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_human_http_notification_sche where chnid=%d;", chnId);
        snprintf(sQuery[PEOPLE_CNT], sizeof(sQuery[PEOPLE_CNT]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_people_http_notification_sche where chnid=%d;", chnId);
        snprintf(sQuery[OBJECT_LEFTREMOVE], sizeof(sQuery[OBJECT_LEFTREMOVE]),
                 "select week_id,plan_id,start_time,end_time,action_type from vca_object_leftremove_http_notification_sche where chnid=%d;",
                 chnId);
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
                sqlite_query_column_text(hStmt, 2, Schedule[chnId][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                         sizeof(Schedule[chnId][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
                sqlite_query_column_text(hStmt, 3, Schedule[chnId][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                         sizeof(Schedule[chnId][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
                sqlite_query_column(hStmt, 4, &Schedule[chnId][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
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
    } while (++chnId < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_smart_event_http_params(const char *pDbFile, HTTP_NOTIFICATION_PARAMS_S **pHttpPrms, int chnId)
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
    char sQuery[256] = {0};
    char pTable[64] = {0};
    int nColumnCnt = 0;
    int i, nResult;

    for (i = 0; i< MAX_SMART_EVENT; ++i) {
        get_vca_http_table_name(1, i, pTable, sizeof(pTable));
        snprintf(sQuery, sizeof(sQuery), "select chnid,url,username,password from %s where chnid=%d;", pTable, chnId);
        if (pTable[0] == '\0' || sQuery[0] == '\0') {
            continue;
        }

        
        nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
        if (nResult != 0 || nColumnCnt == 0) {
            memset(&pHttpPrms[i][chnId], 0, sizeof(HTTP_NOTIFICATION_PARAMS_S));
            if (hStmt) {
                sqlite_clear_stmt(hStmt);
            }
            continue;
        }

        sqlite_query_column(hStmt, 0, &pHttpPrms[i][chnId].id);
        sqlite_query_column_text(hStmt, 1, pHttpPrms[i][chnId].url, sizeof(pHttpPrms[i][chnId].url));
        sqlite_query_column_text(hStmt, 2, pHttpPrms[i][chnId].username, sizeof(pHttpPrms[i][chnId].username));
        sqlite_query_column_text(hStmt, 3, pHttpPrms[i][chnId].password, sizeof(pHttpPrms[i][chnId].password));

        sqlite_clear_stmt(hStmt);
    };

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_smart_event_http_params_init(const char *pDbFile, HTTP_NOTIFICATION_PARAMS_S *httpPrms, int type, int chnCnt)
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
    int chnId = 0;

    do {
        switch (type) {
            case REGION_EN:
                snprintf(sQuery, sizeof(sQuery), "select chnid,url,username,password from %s where chnid=%d;",
                         VREIN_HTTP_PARAMS, chnId);
                break;
            case REGION_EXIT:
                snprintf(sQuery, sizeof(sQuery), "select chnid,url,username,password from %s where chnid=%d;",
                         VREEX_HTTP_PARAMS, chnId);
                break;
            case ADVANCED_MOT:
                snprintf(sQuery, sizeof(sQuery), "select chnid,url,username,password from %s where chnid=%d;",
                         VMOT_HTTP_PARAMS, chnId);
                break;
            case TAMPER_DET:
                snprintf(sQuery, sizeof(sQuery), "select chnid,url,username,password from %s where chnid=%d;",
                         VTEP_HTTP_PARAMS, chnId);
                break;
            case LINE_CROSS:
                snprintf(sQuery, sizeof(sQuery), "select chnid,url,username,password from %s where chnid=%d;",
                         VLSS_HTTP_PARAMS, chnId);
                break;
            case LOITER:
                snprintf(sQuery, sizeof(sQuery), "select chnid,url,username,password from %s where chnid=%d;",
                         VLER_HTTP_PARAMS, chnId);
                break;
            case HUMAN_DET:
                snprintf(sQuery, sizeof(sQuery), "select chnid,url,username,password from %s where chnid=%d;",
                         VHMN_HTTP_PARAMS, chnId);
                break;
            case PEOPLE_COUNT:
                snprintf(sQuery, sizeof(sQuery), "select chnid,url,username,password from %s where chnid=%d;",
                         VPPE_HTTP_PARAMS, chnId);
                break;
            case LEFTREMOVE:
                snprintf(sQuery, sizeof(sQuery), "select chnid,url,username,password from %s where chnid=%d;",
                         VOBJ_HTTP_PARAMS, chnId);
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

        sqlite_query_column(hStmt, 0, &httpPrms[chnId].id);
        sqlite_query_column_text(hStmt, 1, httpPrms[chnId].url, sizeof(httpPrms[chnId].url));
        sqlite_query_column_text(hStmt, 2, httpPrms[chnId].username, sizeof(httpPrms[chnId].username));
        sqlite_query_column_text(hStmt, 3, httpPrms[chnId].password, sizeof(httpPrms[chnId].password));

        sqlite_clear_stmt(hStmt);
    } while (++chnId < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_anpr_http_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int chnCnt)
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

    int i = 0, iSmt = 0, chnId = 0;;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[MAX_ANPR_MODE][256];

    HSQLSTMT hStmt = 0;
    do {
        snprintf(sQuery[ANPR_BLACK], sizeof(sQuery[ANPR_BLACK]),
                 "select week_id,plan_id,start_time,end_time,action_type from %s where chnid=%d;", LPRB_HTTP_SCHE, chnId);
        snprintf(sQuery[ANPR_WHITE], sizeof(sQuery[ANPR_WHITE]),
                 "select week_id,plan_id,start_time,end_time,action_type from %s where chnid=%d;", LPRW_HTTP_SCHE, chnId);
        snprintf(sQuery[ANPR_VISTOR], sizeof(sQuery[ANPR_VISTOR]),
                 "select week_id,plan_id,start_time,end_time,action_type from %s where chnid=%d;", LPRV_HTTP_SCHE, chnId);
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
                sqlite_query_column_text(hStmt, 2, Schedule[chnId][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                         sizeof(Schedule[chnId][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
                sqlite_query_column_text(hStmt, 3, Schedule[chnId][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                         sizeof(Schedule[chnId][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
                sqlite_query_column(hStmt, 4, &Schedule[chnId][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
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
    } while (++chnId < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_anpr_http_params_init(const char *pDbFile, HTTP_NOTIFICATION_PARAMS_S *httpPrms, ANPR_MODE_TYPE type, int chnCnt)
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
    int chnId = 0;

    do {
        switch (type) {
            case ANPR_BLACK:
                snprintf(sQuery, sizeof(sQuery), "select chnid,url,username,password from %s where chnid=%d;",
                         LPRB_HTTP_PARAMS, chnId);
                break;
            case ANPR_WHITE:
                snprintf(sQuery, sizeof(sQuery), "select chnid,url,username,password from %s where chnid=%d;",
                         LPRW_HTTP_PARAMS, chnId);
                break;
            case ANPR_VISTOR:
                snprintf(sQuery, sizeof(sQuery), "select chnid,url,username,password from %s where chnid=%d;",
                         LPRV_HTTP_PARAMS, chnId);
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

        sqlite_query_column(hStmt, 0, &httpPrms[chnId].id);
        sqlite_query_column_text(hStmt, 1, httpPrms[chnId].url, sizeof(httpPrms[chnId].url));
        sqlite_query_column_text(hStmt, 2, httpPrms[chnId].username, sizeof(httpPrms[chnId].username));
        sqlite_query_column_text(hStmt, 3, httpPrms[chnId].password, sizeof(httpPrms[chnId].password));

        sqlite_clear_stmt(hStmt);
    } while (++chnId < chnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_ipc_chnIn_http_schedules_init(const char *pDbFile, struct smart_event_schedule *Schedule[], int maxChnCnt)
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

    int i = 0, iSmt = 0, chnId = 0;;
    int nResult = 0;
    int nColumnCnt = 0;
    int nWeekId = 1;
    int nPlanId = 0;
    int nLoopCnt = MAX_DAY_NUM * MAX_PLAN_NUM_PER_DAY;
    char sQuery[256] = {0};

    HSQLSTMT hStmt = 0;
    do {
        for (iSmt = 0; iSmt < MAX_IPC_ALARM_IN; iSmt++) {
            snprintf(sQuery, sizeof(sQuery), "select week_id,plan_id,start_time,end_time,action_type from"
                " alarm_chnIn%d_http_notification_sche where chnid=%d;", iSmt, chnId);
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
                sqlite_query_column_text(hStmt, 2, Schedule[chnId][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time,
                                         sizeof(Schedule[chnId][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].start_time));
                sqlite_query_column_text(hStmt, 3, Schedule[chnId][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time,
                                         sizeof(Schedule[chnId][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].end_time));
                sqlite_query_column(hStmt, 4, &Schedule[chnId][iSmt].schedule_day[nWeekId].schedule_item[nPlanId].action_type);
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
    } while ((++chnId) < maxChnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_ipc_chnIn_http_params_init(const char *pDbFile, HTTP_NOTIFICATION_PARAMS_S *httpPrms, int alarmid, int maxChnCnt)
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
    int chnId = 0;

    do {
        snprintf(sQuery, sizeof(sQuery), "select chnid,url,username,password from" 
            " alarm_chnIn%d_http_notification_params where chnid=%d;", alarmid, chnId);

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

        sqlite_query_column(hStmt, 0, &httpPrms[chnId].id);
        sqlite_query_column_text(hStmt, 1, httpPrms[chnId].url, sizeof(httpPrms[chnId].url));
        sqlite_query_column_text(hStmt, 2, httpPrms[chnId].username, sizeof(httpPrms[chnId].username));
        sqlite_query_column_text(hStmt, 3, httpPrms[chnId].password, sizeof(httpPrms[chnId].password));
        
        sqlite_clear_stmt(hStmt);
    } while (++chnId < maxChnCnt);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int copy_http_notification_action(EVENT_IN_TYPE_E type, int chnId, Uint64 chnMask)
{
    if (chnId < 0 || chnId >= MAX_REAL_CAMERA || !chnMask) {
        return -1;
    }

    char pTable[MAX_LEN_64] = {0};
    SMART_SCHEDULE schedule;
    HTTP_NOTIFICATION_PARAMS_S httpParams;

    memset(&schedule, 0, sizeof(schedule));
    get_action_type_http_table_name(0, type, pTable, sizeof(pTable));
    read_http_notification_schedule(SQLITE_FILE_NAME, &schedule, pTable, chnId);
    copy_http_notification_schedules(SQLITE_FILE_NAME, &schedule, pTable, chnMask);

    memset(&httpParams, 0, sizeof(httpParams));
    get_action_type_http_table_name(1, type, pTable, sizeof(pTable));
    read_http_notification_params(SQLITE_FILE_NAME, &httpParams, pTable, chnId);
    copy_http_notification_params(SQLITE_FILE_NAME, pTable, &httpParams, chnMask);

    return 0;
}

int read_audio_alarm(const char * pDbFile, SMART_EVENT * audioAlarm, int id)
{
    return read_event(SQLITE_FILE_NAME, audioAlarm, "audio_alarm", id);
}

int read_audio_alarms(const char *pDbFile, SMART_EVENT *audioAlarm, int cnt)
{
    return read_events(SQLITE_FILE_NAME, audioAlarm, "audio_alarm", cnt);
}

int write_audio_alarm(const char * pDbFile, SMART_EVENT *audioAlarm)
{
    return write_event(SQLITE_FILE_NAME, audioAlarm, "audio_alarm");
}

int copy_audio_alarms(const char * pDbFile, SMART_EVENT * audioAlarm, Uint64 chnMask)
{
    return copy_events(SQLITE_FILE_NAME, audioAlarm, "audio_alarm", chnMask);
}

int read_audio_alarm_sche(const char *pDbFile, SMART_SCHEDULE *schedule, char *pDbTable, int chnId)
{
    char idName[16] = {0};
    
    if (!strcmp(pDbTable, AUD_HTTP_SCHE)) {
        snprintf(idName, sizeof(idName), "chnid");
    } else {
        snprintf(idName, sizeof(idName), "chn_id");
    }
    
    return read_comn_sche(pDbFile, schedule, pDbTable, idName, chnId);
}

int read_audio_alarm_sches(const char *pDbFile, SMART_SCHEDULE *schedule, char *pDbTable, int cnt)
{
    char idName[16] = {0};
    
    if (!strcmp(pDbTable, AUD_HTTP_SCHE)) {
        snprintf(idName, sizeof(idName), "chnid");
    } else {
        snprintf(idName, sizeof(idName), "chn_id");
    }

    return read_comn_sches(pDbFile, schedule, pDbTable, idName, cnt);
}

int copy_audio_alarm_sche(const char *pDbFile, SMART_SCHEDULE *schedule, char *pDbTable, Uint64 chnMask)
{
    char idName[16] = {0};
    
    if (!strcmp(pDbTable, AUD_HTTP_SCHE)) {
        snprintf(idName, sizeof(idName), "chnid");
    } else {
        snprintf(idName, sizeof(idName), "chn_id");
    }

    return copy_comn_sches(pDbFile, schedule, pDbTable, idName, chnMask);
}

int read_web_param(const char *pDbFile, WEB_PARAM_S *pParam)
{
    if (!pDbFile || !pParam) {
        return -1;
    }
    
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLSTMT hStmt = 0;
    char sQuery[2048] = {0};
    int nColumnCnt = 0;
    int nResult;
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    snprintf(sQuery, sizeof(sQuery), "select event_region from web_param;");
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        sqlite_clear_stmt(hStmt);
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, (int *)&pParam->eventRegion);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}


int update_web_param(const char *pDbFile, WEB_PARAM_S *pParam)
{
    if (!pDbFile || !pParam) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    char sExec[2048] = {0};
    const char *prefix = "update web_param set ";
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    snprintf(sExec, sizeof(sExec), prefix);
    if (pParam->eventRegion != MS_INVALID_VALUE) {
        snprintf(sExec + strlen(sExec), sizeof(sExec) - strlen(sExec), "event_region=%d;", pParam->eventRegion);
    }

    if (strlen(sExec) <= strlen(prefix)) {
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_network_keyboard(const char *pDbFile, NETWORK_KEYBOARD_S pParam[], int *cnt)
{
    if (!pDbFile || !pParam) {
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
    int i = 0;
    snprintf(sQuery, sizeof(sQuery),
             "select ip_addr,password from network_keyboard;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        *cnt = 0;
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    while (i < MAX_LEN_128) {
        sqlite_query_column_text(hStmt, 0, pParam[i].ip_addr, sizeof(pParam[i].ip_addr));
        sqlite_query_column_text(hStmt, 1, pParam[i].password, sizeof(pParam[i].password));
        i++;
        if (sqlite_query_next(hStmt) != 0) {
            break;
        }
    }

    *cnt = i;
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int insert_network_keyboard(const char *pDbFile, NETWORK_KEYBOARD_S *pParam)
{
    if (!pDbFile || !pParam) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[1024] = {0};
    char pwd[MAX_PWD_LEN * 2 + 2] = {0};

    translate_pwd(pwd, pParam->password, strlen(pParam->password));
    snprintf(sExec, sizeof(sExec),
             "insert into network_keyboard(ip_addr,password) values('%s','%s');",
             pParam->ip_addr, pwd);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int delete_network_keyboard(const char *pDbFile, NETWORK_KEYBOARD_S *pParam)
{
    if (!pDbFile || !pParam) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[2048] = {0};

    snprintf(sExec, sizeof(sExec), "delete from network_keyboard where ip_addr = '%s';", pParam->ip_addr);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int read_disarming(const char * pDbFile, DISARMING_S *pDisarming)
{
    if (!pDbFile || !pDisarming) {
        return -1;
    }
    
    int mode = FILE_MODE_RD;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLSTMT hStmt = 0;
    char sQuery[2048] = {0};
    int nColumnCnt = 0;
    int nResult;
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    
    snprintf(sQuery, sizeof(sQuery), "select id,linkage_action,disarming_action,status from disarming where id=0;");
    nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        sqlite_clear_stmt(hStmt);
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }

    sqlite_query_column(hStmt, 0, &pDisarming->id);
    sqlite_query_column(hStmt, 1, (int *)&pDisarming->linkageAction);
    sqlite_query_column(hStmt, 2, (int *)&pDisarming->disarmActionMask);
    sqlite_query_column(hStmt, 3, &pDisarming->status);
    
    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_disarming(const char * pDbFile, DISARMING_S *pDisarming)
{
    if (!pDbFile || !pDisarming) {
        return -1;
    }
    
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    char sExec[2048] = {0};
    HSQLITE hConn = 0;

    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
        
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    snprintf(sExec, sizeof(sExec), "update disarming set linkage_action=%d,disarming_action=%u,status=%d where id=0;",
        pDisarming->linkageAction, pDisarming->disarmActionMask, pDisarming->status);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);
    
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_db_update(const char * pDbFile, DB_VER_INFO_S oldArr[], int oldCnt, DB_VER_INFO_S updateArr[], int updateCnt,
    const char *pTxtFile)
{
    if (!pDbFile || !pTxtFile) {
        return -1;
    }

    int i = 0;
    int fd = -1, pushmsgFd = -1;
    FILE *fp = NULL;
    char sql[1024] = {0};
    char path[128] = {0};
    HSQLITE hConn = NULL, msdbConn = NULL, pushmsgConn = NULL;
    int mode = FILE_MODE_WR;

    fd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    if (sqlite_conn(pDbFile, &msdbConn) != 0) {
        FileUnlock(fd, mode, &global_rwlock);
        return -1;
    }
    pushmsgFd = FileLock(FILE_PUSHMSG_NAME, mode, &g_pushmsgRwlock);
    if (sqlite_conn(SQLITE_PUSHMSG_NAME, &pushmsgConn) != 0) {
        FileUnlock(pushmsgFd, mode, &g_pushmsgRwlock);
        sqlite_disconn(msdbConn);
        FileUnlock(fd, mode, &global_rwlock);
        return -1;
    }

    sqlite3_exec(msdbConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    sqlite3_exec(pushmsgConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    for (i = 0; i < updateCnt; ++i) {
        snprintf(path, sizeof(path), "%s/%s.txt", pTxtFile, updateArr[i].file);
        fp = fopen(path, "rb");
        if (!fp) {
            msdebug(DEBUG_ERR, "db update file %s is not exist.", path);
            continue;
        }

        if (!strcmp(updateArr[i].file, "db-9.0.16-1006")) { // need update pushmsg.db
            hConn = pushmsgConn;
        } else {
            hConn = msdbConn;
        }

        while (fgets(sql, sizeof(sql), fp)) {
            if(sql[0] == '#'){ // read not sql but note
                continue;
            }
            sqlite_execute(hConn, mode, sql);
        }
        updateArr[i].execute = 1;

        snprintf(sql, sizeof(sql), "insert into db_version values('%s',%d);", updateArr[i].file, updateArr[i].execute);
        sqlite_execute(msdbConn, mode, sql);

        fclose(fp);
        fp = NULL;
    }

    snprintf(sql, sizeof(sql), "update params set value ='%d' where name='%s';", 1000 + oldCnt + updateCnt, PARAM_DB_VERSION);
    sqlite_execute(msdbConn, mode, sql);

    sqlite3_exec(msdbConn, "COMMIT;", 0, 0, NULL);
    sqlite3_exec(pushmsgConn, "COMMIT;", 0, 0, NULL);
    sqlite_disconn(pushmsgConn);
    sqlite_disconn(msdbConn);
    FileUnlock(pushmsgFd, mode, &g_pushmsgRwlock);
    FileUnlock(fd, mode, &global_rwlock);

    return 0;
}

int read_disk_health(const char *pDbFile, Uint64 *enableMask)
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
    char sQuery[256] = {0};
    int nColumnCnt = 0;
    snprintf(sQuery, sizeof(sQuery), "select disk_health_enable from disk_health");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if (nResult != 0 || nColumnCnt == 0) {
        if (hStmt) {
            sqlite_clear_stmt(hStmt);
        }
        sqlite_disconn(hConn);
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite_query_column_int64(hStmt, 0, enableMask);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int write_disk_health(const char *pDbFile, Uint64 *enableMask)
{
    if (!pDbFile) {
        return -1;
    }
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_LOCK_NAME, mode, &global_rwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(pDbFile, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[256] = {0};
    snprintf(sExec, sizeof(sExec), "update disk_health set disk_health_enable = %llu", *enableMask);
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &global_rwlock);
    return 0;
}

int update_pushmsg()
{
    int mode = FILE_MODE_WR;
    int nFd = FileLock(FILE_PUSHMSG_NAME, mode, &g_pushmsgRwlock);
    HSQLITE hConn = 0;
    if (sqlite_conn(SQLITE_PUSHMSG_NAME, &hConn) != 0) {
        FileUnlock(nFd, mode, &global_rwlock);
        return -1;
    }
    sqlite3_exec(hConn, "BEGIN IMMEDIATE;", 0, 0, NULL);
    char sExec[1024] = {0};

    snprintf(sExec, sizeof(sExec), "alter table msg add column object TEXT default '';");
    sqlite_execute(hConn, mode, sExec);
    snprintf(sExec, sizeof(sExec), "alter table msg add column hasSnapshot INTEGER NOT NULL default 0;");
    sqlite_execute(hConn, mode, sExec);
    sqlite3_exec(hConn, "COMMIT;", 0, 0, NULL);

    sqlite_disconn(hConn);
    FileUnlock(nFd, mode, &g_pushmsgRwlock);
    return 0;
}