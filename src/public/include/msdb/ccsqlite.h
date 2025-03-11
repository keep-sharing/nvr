#ifndef _CC_SQL_LITE_H_
#define _CC_SQL_LITE_H_

typedef void* HSQLITE;
typedef void* HSQLSTMT;
typedef unsigned long long Uint64;

// 功能：打开指定的sqlite.
// pSqlFile - in. 指定sqlite文件的路径.
// phConn   - in/out. 打开成功返回操作句柄.
int  sqlite_conn(const char* pSqlFile, HSQLITE* phConn);

// 功能：关闭已经打开的sqlite.
// hConn - in. 由sqlite_conn返回的操作句柄.
int  sqlite_disconn(HSQLITE hConn);

// 功能：执行sql操作.
// 备注：begin_commit,end_commit对应事务的应用
int  sqlite_execute(HSQLITE hSqlite, int mode, const char* pSql);
// 功能：执行查询操作.
// -1:error  0:has record  1:no record
const char* sqlite_query_column_name(HSQLITE hSqlite, HSQLSTMT hStmt, int index);
int  sqlite_query_record(HSQLITE hSql, const char* pQuery, HSQLSTMT* pStmt, int* pClmCnt);
// -1:error  0:ok for int type data
int  sqlite_query_column(HSQLSTMT hStmt, int pClmIndex, int* sDataInfo); 
// -1:error  0:has record  1:no record
int  sqlite_query_next(HSQLSTMT hStmt);
// after query data, clear the stmt
int  sqlite_clear_stmt(HSQLSTMT hStmt); 
//  -1:error  0:ok for text type data
int  sqlite_query_column_text(HSQLSTMT hStmt, int pClmIndex, char* sDataInfo, int nBufLen);
//  -1:error  0:ok for double type data
int  sqlite_query_column_double(HSQLSTMT hStmt, int pClmIndex, double* fDataInfo);
int sqlite_query_column_byte(HSQLSTMT hStmt, int pClmIndex, unsigned char* sDataInfo, int size);

int sqlite_exec_blob(HSQLSTMT pStmt, int pCol, void *pBuf, int size);

int  sqlite_prepare_blob(HSQLITE hSqlite, const char* pCmd, HSQLSTMT* pStmt, int query);

int sqlite_query_column_blob(HSQLSTMT hStmt, int pClmIndex, void *pData, int size);
int  sqlite_query_record_fast(HSQLITE hSqlite, const char* pQuery, HSQLSTMT* pStmt, int* pClmCnt);

int sqlite_get_conn_by_proname(char *proName, HSQLITE* phConn);

// wcm add
int sqlite_query_column_int64(HSQLSTMT hStmt, int pClmIndex, Uint64* sDataInfo); 
#endif
