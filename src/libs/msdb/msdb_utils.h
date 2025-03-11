#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include "ccsqlite.h"
#include "msdefs.h"
#include "msdb.h"
#include "msstd.h"
#include <stddef.h>
#include <sqlite3.h>
#include <errno.h>

#define FILE_LOCK_NAME 		"/tmp/msdb.fl"
#define FILE_TZ_NAME 		"/tmp/tzdb.fl"
#define FILE_ANPR_NAME  	"/tmp/anpr.fl"
#define FILE_PCNT_NAME  	"/tmp/peoplecnt.fl"
#define FILE_PUSHMSG_NAME   "/tmp/pushmsg.fl"

#define FILE_MODE_RD		('r')
#define FILE_MODE_WR		('w')

#define FileLock(p, m, l)   MsFileLock(p, m, l, __func__, __LINE__)
#define FileUnlock(p, m, l)    MsFileUnlock(p, m, l, __func__, __LINE__)

int MsFileLock(const char *pFilePath, int mode, pthread_rwlock_t *lock,  const char* file, int lineno);
void MsFileUnlock(int nFd, int mode, pthread_rwlock_t *lock,  const char* file, int lineno);

extern pthread_rwlock_t global_rwlock;
extern pthread_rwlock_t global_pcnt_rwlock;
extern pthread_rwlock_t g_pushmsgRwlock;

extern int globla_msdb_debug;

