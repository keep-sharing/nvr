#include "ccsqlite.h"
//#include "sqlite3.h"
#include <sqlite3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> //memcpy
#include "msstd.h"
#include "msdb.h"
#include "msdb_utils.h"
#define SQLITE_OPT_CNT 200

HSQLITE gMscorehConn = NULL;
HSQLITE gBoahConn = NULL;

static void sqlite_debug(const char* pDebug, ...)
{
	#define STARTUP_DEBUG_MODE//bruce.milesight
	#ifndef STARTUP_DEBUG_MODE
	return;
	#endif
	char sDebugAll[2048] = {0};
    va_list  va;
    va_start (va, pDebug);
    vsprintf(sDebugAll, pDebug, va);
    va_end(va);
    mslogerr(DEBUG_ERR, "[sqlite_debug]%s", sDebugAll);
}

int sqlite_get_conn_by_proname(char *proName, HSQLITE* phConn)
{
	if(proName[0] == '\0') return -1;
	if(!strcmp(proName, PROCESS_EXECUTABLE_MSCORE))
	{
        // BUG: LiuHuanyu 2022-04-22, 这里赋值没用，不过这个函数目前没有用到
		phConn = &gMscorehConn;
		return 0;
	}
		
	if(!strcmp(proName, PROCESS_EXECUTABLE_WEBSEVER))
	{
        // BUG: LiuHuanyu 2022-04-22, 这里赋值没用，不过这个函数目前没有用到
		phConn = &gBoahConn;
		return 0;
	}
	
	return -1;
}

int sqlite_get_conn_by_proid(MS_EXE exeId, HSQLITE** phConn)
{
	if(exeId == MS_MSCORE)
	{
		*phConn = &gMscorehConn;
		return 0;
	}

	if(exeId == MS_BOA)
	{
		*phConn = &gBoahConn;
		return 0;
	}
	
	return -1;
}

int sqlite_open(const char* pSqlFile, MS_EXE exeId)
{
	if(!pSqlFile)
	{
		sqlite_debug("param is invalid, SqlFile:%s\n", pSqlFile);
		return -1;
	}
	int ret = -1;
	HSQLITE* phConn = NULL;
	ret = sqlite_get_conn_by_proid(exeId, &phConn);
	if(ret)
	{
		sqlite_debug("sqlite conn by proname is invalid.");
		return -1;
	}	

	ret = sqlite3_open(pSqlFile, (struct sqlite3 **)phConn);
	if (ret != SQLITE_OK)
	{
		sqlite_debug("open sqlite:%s fail with:%d", pSqlFile, ret);
		return -1;
	}
	sqlite3_busy_handler(*phConn, NULL, NULL);
	
	return 0;
}

int sqlite_close(const char* pSqlFile, MS_EXE exeId)
{
	int ret = -1;
	HSQLITE* phConn = NULL;
	
	ret = sqlite_get_conn_by_proid(exeId, &phConn);
	if(ret)
	{
		sqlite_debug("sqlite conn by proname is invalid.");
		return -1;
	}

	if (sqlite3_close((struct sqlite3 *)*phConn) != SQLITE_OK)
	{
		sqlite_debug("sqlite close fail, error:%s\n", sqlite3_errmsg(*phConn));
		return -1;
	}
	
	return 0;
}

int sqlite_conn(const char* pSqlFile, HSQLITE* phConn) 
{
	if (!pSqlFile || !phConn)
	{
		sqlite_debug("param is invalid, SqlFile:%x  Handle:%x\n", pSqlFile, phConn);
		return -1;
	}
	int ret = -1;
	char proName[MAX_LEN_32] = {0};
	
	ret = ms_get_executable_name(proName, sizeof(proName));
	if(!ret && !strcmp(SQLITE_FILE_NAME, pSqlFile))
	{
		if(!strcmp(proName, PROCESS_EXECUTABLE_MSCORE))
		{
			*phConn = gMscorehConn;
			return 0;
		}

		if(!strcmp(proName, PROCESS_EXECUTABLE_WEBSEVER))
		{
			*phConn = gBoahConn;
			return 0;
		}
	}	
	
	int nOpenRet = sqlite3_open(pSqlFile, (struct sqlite3 **)phConn);
	if (nOpenRet != SQLITE_OK)
	{
		sqlite_debug("open sqlite:%s fail with:%d", pSqlFile, nOpenRet);
		return -1;
	}
	sqlite3_busy_handler(*phConn, NULL, NULL);
	
	return 0;
}

int  sqlite_disconn(HSQLITE hConn)
{
	if (!hConn)
		return -1;

	if(hConn == gMscorehConn)
	{
		return 0;
	}
	if(hConn == gBoahConn)
	{
		return 0;
	}
	if (sqlite3_close((struct sqlite3 *)hConn) != SQLITE_OK)
	{
		sqlite_debug("sqlite close fail, error:%s\n", sqlite3_errmsg(hConn));
		return -1;
	}
	return 0;
}

int  sqlite_execute(HSQLITE hSqlite, int mode, const char* pExec)
{
	int count = 0;
	if (!hSqlite || !pExec)
	{
		sqlite_debug("sql execute param is invalid\n");
		return -1;
	}
	if(mode != FILE_MODE_WR)
	{
		msdebug(DEBUG_ERR, "sqlite_execute params is err!");
		return -1;
	}
	
	char* pErrMsg = 0;
	sqlite3_busy_timeout(hSqlite, 50*1000);
	while(sqlite3_exec(hSqlite, pExec, 0, 0, &pErrMsg) != SQLITE_OK)
	{
		count++;		
		if(count >= SQLITE_OPT_CNT)
		{
			if(pErrMsg)
			{
				if(strstr(pErrMsg, "database is locked") != NULL)
				{
					if(count == 1)
					{
						sqlite_debug("sql sqlite3_exec is busy now.");
						system("fuser -m /mnt/nand/app/msdb.db >> /mnt/nand/err_log.txt");
						sqlite_debug("sql sqlite3_exec fail:%s.", pExec);
					}
					
					sqlite3_free(pErrMsg);
					usleep(10000);
					continue;
				}
			}
			
			sqlite_debug("sql execute fail, %s=count:%d=pErrMsg:%s===\n", pExec, count, pErrMsg);
			if(pErrMsg)
				sqlite3_free(pErrMsg);
			return -1;
		}

		if(pErrMsg)
			sqlite3_free(pErrMsg);
		usleep(10000);
	}
	
	return 0;
}

const char* sqlite_query_column_name(HSQLITE hSqlite, HSQLSTMT hStmt, int index)
{
	if (!hSqlite || !hStmt)
		return 0;
	return sqlite3_column_name(hStmt, index);
}

int  sqlite_query_record(HSQLITE hSqlite, const char* pQuery, HSQLSTMT* pStmt, int* pClmCnt)
{
	int count = 0;
	int ret = 0;
	if (!hSqlite || !pQuery || !pStmt || !pClmCnt)
	{
		sqlite_debug("sql query record, param is invalid\n");
		return -1;
	}
	//sqlite_debug("sql:%s\n", pQuery);
	while((ret = sqlite3_prepare_v2( (struct sqlite3 *)hSqlite, pQuery, -1, (struct sqlite3_stmt **)pStmt, 0)) != SQLITE_OK )
	{
		count++;		
		if(count >= SQLITE_OPT_CNT)
		{
			sqlite_debug("sql query record fail,ret=%d, %s==count:%d=\n", ret, pQuery, count);
			return -1;		
		}
		usleep(10000);
	}
	//sqlite_debug("sql:%s\n", pQuery);
	count = 0;
	while ((ret=sqlite3_step(*pStmt)) != SQLITE_ROW)
	{
		if(ret == SQLITE_DONE)
			return 1;
		count++;
		if(count >= SQLITE_OPT_CNT)
		{
			sqlite_debug("sql query record sqlite3_step fail,ret=%d, %s==count:%d=\n", ret, pQuery, count);
			return ret;		
		}		
		usleep(10000);
	}
	*pClmCnt = sqlite3_column_count(*pStmt);
	
	return 0;
}

int  sqlite_query_record_fast(HSQLITE hSqlite, const char* pQuery, HSQLSTMT* pStmt, int* pClmCnt)
{
	int ret = 0, count = 0;
	if (!hSqlite || !pQuery || !pStmt || !pClmCnt)
	{
		sqlite_debug("sql query record, param is invalid\n");
		return -1;
	}
	while((ret = sqlite3_prepare_v2( (struct sqlite3 *)hSqlite, pQuery, -1, (struct sqlite3_stmt **)pStmt, 0)) != SQLITE_OK )
	{
		count++;		
		if(count >= 2)
		{
			sqlite_debug("sql query record fast fail,ret=%d, %s===\n", ret, pQuery);
			return ret;	

		}
		usleep(10000);
	}
	count = 0;
	while ((ret=sqlite3_step(*pStmt)) != SQLITE_ROW)
	{
		if(ret == SQLITE_DONE)
			return 1;
		count++;
		if(count >= SQLITE_OPT_CNT)
		{
			sqlite_debug("sql query record sqlite3_step fail,ret=%d, %s==count:%d=\n", ret, pQuery, count);
			return ret;		
		}		
		usleep(10000);
	}
	*pClmCnt = sqlite3_column_count(*pStmt);
	return 0;
}
int  sqlite_query_next(HSQLSTMT hStmt)
{
	if (!hStmt)
	{
		sqlite_debug("sql query record fail, param is invalid\n");
		return -1;		
	}
	if (sqlite3_step(hStmt) != SQLITE_ROW)
	{
		return 1;		
	}
	return 0;
}


int  sqlite_query_column(HSQLSTMT hStmt, int pClmIndex, int* sDataInfo)
{
	if (!hStmt || pClmIndex<0 || !sDataInfo)
	{
		sqlite_debug("sql query data fail, param is invalid\n");
		return -1;		
	}
	*sDataInfo = sqlite3_column_int(hStmt, pClmIndex);
	return 0;
}

int sqlite_query_column_int64(HSQLSTMT hStmt, int pClmIndex, Uint64* sDataInfo)
{
	if (!hStmt || pClmIndex<0 || !sDataInfo)
	{
		sqlite_debug("sql query data fail, param is invalid\n");
		return -1;		
	}
	*sDataInfo = sqlite3_column_int64(hStmt, pClmIndex);
	return 0;
}

int  sqlite_query_column_double(HSQLSTMT hStmt, int pClmIndex, double* fDataInfo)
{
	if (!hStmt || pClmIndex<0 || !fDataInfo)
	{
		sqlite_debug("sql query data fail, param is invalid\n");
		return -1;		
	}
	*fDataInfo = sqlite3_column_double(hStmt, pClmIndex);
	return 0;
}

int  sqlite_clear_stmt(HSQLSTMT hStmt)
{
	if (!hStmt)
	{
		sqlite_debug("sql clear fail\n");
		return -1;
	}
	sqlite3_finalize(hStmt);
    return 0;
}

int sqlite_query_column_byte(HSQLSTMT hStmt, int pClmIndex, unsigned char* sDataInfo, int size)
{
	if (!hStmt || pClmIndex<0 || !sDataInfo)
	{
		sqlite_debug("sql query data fail, param is invalid\n");
		return -1;		
	}
	int value = sqlite3_column_int(hStmt, pClmIndex);
	*sDataInfo = (unsigned char)value;
	
	return 0;	
}

int  sqlite_query_column_text(HSQLSTMT hStmt, int pClmIndex, char* sDataInfo, int nBufLen)
{
	if (!hStmt || pClmIndex<0 || !sDataInfo || nBufLen<=0)
	{
		sqlite_debug("sql query data fail, param is invalid\n");
		return -1;		
	}
	const unsigned char* pData = sqlite3_column_text(hStmt, pClmIndex);
	if (pData)
	{
		snprintf(sDataInfo, nBufLen, "%s", pData);
	}
	return 0;
}

int sqlite_query_column_blob(HSQLSTMT hStmt, int pClmIndex, void *pData, int size)
{
	if (!hStmt || pClmIndex < 0 || !pData || size <= 0)
	{
		sqlite_debug("sql query data fail, param is invalid\n");
		return -1;
	}
	const void *data = sqlite3_column_blob(hStmt, pClmIndex);
	int bytes = sqlite3_column_bytes(hStmt, pClmIndex);
	if (data && bytes > 0)
	{
		memcpy(pData, data, bytes);
	}
	else
	{
		sqlite_debug("sqlite_query_column_blob failed.(bytes:%d==size:%d)\n", bytes, size);
	}	
	return 0;
}

int  sqlite_prepare_blob(HSQLITE hSqlite, const char* pQuery, HSQLSTMT* pStmt, int query)
{
	int count = 0, ret = 0;
	if (!hSqlite || !pQuery || !pStmt)
	{
		sqlite_debug("sql prepare blob, param is invalid\n");
		return -1;
	}
	while ((ret=sqlite3_prepare_v2( (struct sqlite3 *)hSqlite, pQuery, -1, (struct sqlite3_stmt **)pStmt, 0)) != SQLITE_OK )
	{
		count++;		
		if(count >= SQLITE_OPT_CNT)
		{
			sqlite_debug("sql prepare blob fail,ret=%d, %s==count:%d=\n", ret, pQuery, count);
			return -1;		
		}
		usleep(10000);
	}
	if (query) 
	{
		count = 0;
		while ((ret=sqlite3_step(*pStmt)) != SQLITE_ROW) 
		{
			if(ret == SQLITE_DONE)
				return 1;
			count++;
			if(count >= SQLITE_OPT_CNT)
			{
				sqlite_debug("sql blob sqlite3_step fail,ret=%d, %s==count:%d=\n", ret, pQuery, count);
				return 1;
			}
			usleep(10000);
		}
	}
	return 0;
}

int sqlite_exec_blob(HSQLSTMT pStmt, int pCol, void *pBuf, int size)
{
	if (!pStmt || !pBuf) {
		sqlite_debug("sql exec blob error, invalid argument\n");
		return -1;
	}
	int ret = sqlite3_bind_blob(pStmt, pCol, pBuf, size, NULL);
	if (ret != SQLITE_OK) {
        sqlite_debug("sql exec blob error: bind_blob, %d\n", ret);
		return -1;
	}
	if (sqlite3_step(pStmt) != SQLITE_DONE) {
		sqlite_debug("sql exec blob error: step\n");
		return -1;
	}
	return 0;
}
