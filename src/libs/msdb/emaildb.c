#include "emaildb.h"
#include "ccsqlite.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "msdb.h"

#define EMAIL_FILE_LOCK_NAME "/tmp/emaildb.fl"

static int file_lock(const char* flock)
{
	if(!flock) return -1;
	int fd = open(flock, O_CREAT|O_RDWR, 7777);
	if(-1 == fd) return -1;
	lockf(fd, F_LOCK, 0);
	return fd;
}

static void file_unlock(int fd)
{
	if(fd <= -1) return ;
	lockf(fd, F_ULOCK, 0);
	close(fd);
}

int read_smtpconf(const char* db, struct smtpconf* conf)
{
	if (!db || !conf) return -1;
	int fd = file_lock(EMAIL_FILE_LOCK_NAME);
	HSQLITE hconn = 0;
	if (sqlite_conn(db, &hconn) != 0){file_unlock(fd);return -1;}
	HSQLSTMT hstmt  = 0;
	int  column = 0;
	int  result = 0;
	char query[256] = {0};
	snprintf(query, sizeof(query), "select emailaddr,username,password,smtpserver,smtpport,usessl,auth,sender from smtpconf");
	result = sqlite_query_record(hconn, query, &hstmt, &column);
	if (result != 0){
		if (hstmt) sqlite_clear_stmt(hstmt);
		sqlite_disconn(hconn);
		file_unlock(fd);
		return -1;
	}
	sqlite_query_column_text(hstmt, 0, conf->emailaddr, sizeof(conf->emailaddr));
	sqlite_query_column_text(hstmt, 1, conf->username,  sizeof(conf->username));
	sqlite_query_column_text(hstmt, 2, conf->password,  sizeof(conf->password));
	sqlite_query_column_text(hstmt, 3, conf->smtpserver,  sizeof(conf->smtpserver));
	sqlite_query_column(hstmt, 4, &conf->smtpport);
	sqlite_query_column(hstmt, 5, &conf->usessl);
	sqlite_query_column(hstmt, 6, &conf->auth);
	sqlite_query_column_text(hstmt, 7, conf->sender,  sizeof(conf->sender));
	sqlite_clear_stmt(hstmt);
	sqlite_disconn(hconn);
	file_unlock(fd);
	return 0;
}

int write_smtpconf(const char* db, struct smtpconf* conf)
{
	if (!db || !conf) return -1;
	int fd = file_lock(EMAIL_FILE_LOCK_NAME);
	HSQLITE hconn = 0;
	if (sqlite_conn(db, &hconn) != 0){file_unlock(fd);return -1;}
	char exec[1024] = {0};
	snprintf(exec, sizeof(exec), "update smtpconf set emailaddr='%s',username='%s',password='%s',smtpserver='%s',smtpport=%d,usessl=%d,auth=%d,sender='%s'",
	conf->emailaddr, conf->username, conf->password, conf->smtpserver, conf->smtpport, conf->usessl, conf->auth, conf->sender);
	sqlite_execute(hconn, 'w', exec);
	sqlite_disconn(hconn);
	file_unlock(fd);
	return 0;
}

int read_mailtosend(const char* db, struct mailtosend mails[], int capacity, int* readcount)
{
	if (!db || !mails || capacity<=0 || !readcount) return -1;
	int fd = file_lock(EMAIL_FILE_LOCK_NAME);
	HSQLITE hconn = 0;
	if (sqlite_conn(db, &hconn) != 0){file_unlock(fd);return -1;}
	HSQLSTMT hstmt  = 0;
	int  i = 0;
	int  column = 0;
	int  result = 0;
	char query[256] = {0};
	snprintf(query, sizeof(query)-1, "select mailid,mailto,mailcc,mailsub,mailbody,mailattach,tasktime,sendtime,status,retry from mailtosend where status==0 limit %d", capacity);
	result = sqlite_query_record(hconn, query, &hstmt, &column);
	if (result != 0){
		if (hstmt) sqlite_clear_stmt(hstmt);
		sqlite_disconn(hconn);
		file_unlock(fd);
		return -1;
	}
	for ( i = 0; i < capacity; i++)
	{
		sqlite_query_column(hstmt, 0, &mails[i].mailid);
		sqlite_query_column_text(hstmt, 1, mails[i].mailto, sizeof(mails[i].mailto));
		sqlite_query_column_text(hstmt, 2, mails[i].mailcc, sizeof(mails[i].mailcc));
		sqlite_query_column_text(hstmt, 3, mails[i].mailsub, sizeof(mails[i].mailsub));
		sqlite_query_column_text(hstmt, 4, mails[i].mailbody, sizeof(mails[i].mailbody));
		sqlite_query_column_text(hstmt, 5, mails[i].mailattach, sizeof(mails[i].mailattach));
		sqlite_query_column_text(hstmt, 6, mails[i].tasktime, sizeof(mails[i].tasktime));
		sqlite_query_column_text(hstmt, 7, mails[i].sendtime, sizeof(mails[i].sendtime));
		sqlite_query_column(hstmt, 8, &mails[i].status);
		sqlite_query_column(hstmt, 9, &mails[i].retry);
		*readcount = i+1;
		if (sqlite_query_next(hstmt) != 0) break;
	}
	sqlite_clear_stmt(hstmt);
	sqlite_disconn(hconn);
	file_unlock(fd);
	return 0;
}

int update_mailtosend(const char* db, int mailid, const char* sendtime, int status, int retry)
{
	if (!db) return -1;
	int fd = file_lock(EMAIL_FILE_LOCK_NAME);
	HSQLITE hconn = 0;
	if (sqlite_conn(db, &hconn) != 0){file_unlock(fd);return -1;}
	char exec[256] = {0};
	snprintf(exec, sizeof(exec), "update mailtosend set sendtime='%s',status=%d,retry=%d where mailid=%d", sendtime, status, retry, mailid);
	sqlite_execute(hconn, 'w', exec);
	sqlite_disconn(hconn);
	file_unlock(fd);
	return 0;
}

int add_mailtosend(const char* db, struct mailtosend* mail)
{
	if (!db || !mail) return -1;
	int fd = file_lock(EMAIL_FILE_LOCK_NAME);
	HSQLITE hconn = 0;
	if (sqlite_conn(db, &hconn) != 0){file_unlock(fd);return -1;}
	char exec[2048] = {0};
    char mailsub[256] = {0};
    char mailbody[2048] = {0};
    translate_pwd(mailsub, mail->mailsub, strlen(mail->mailsub));
    translate_pwd(mailbody, mail->mailbody, strlen(mail->mailbody));
	snprintf(exec, sizeof(exec), "insert into mailtosend(mailid,mailto,mailcc,mailsub,mailbody,mailattach,tasktime,sendtime,status,retry) values(NULL, '%s','%s','%s','%s','%s','%s','%s',%d,%d)",
	    mail->mailto, mail->mailcc, mailsub, mailbody, mail->mailattach, mail->tasktime, mail->sendtime, mail->status, mail->retry);
    sqlite_execute(hconn, 'w', exec);
	sqlite_disconn(hconn);
	file_unlock(fd);
	return 0;
}

int add_test(const char* db, struct mailtosend* mail)
{
	return 0;
}

//nDeleType(1:by mailid. 2:by sendtime. 3:by status 4:by retry -1:delete all)
//nCmpType(1: '>'. -1: '<'. 0: '=')
int remove_mailtosend(const char* db, int nDeleType, int nCmpType, void* pCondit)
{
	if (! db || (!pCondit&&(nDeleType!=-1))) return -1;
	char c = '=';
	if (nCmpType == 1)  c ='>';
	else if (nCmpType == -1) c = '<';
	char sExec[256] = {0};
	if (nDeleType == 1){
		snprintf(sExec, sizeof(sExec), "delete from mailtosend where mailid%c%d", c, *((int*)(pCondit)));
	}else if (nDeleType == 2){
		//snprintf(sExec, sizeof(sExec), "delete from mailtosend where sendtime%c%d", c, *((int*)(pCondit)));
	}else if (nDeleType == 3){
		snprintf(sExec, sizeof(sExec), "delete from mailtosend where status%c%d", c, *((int*)(pCondit)));
	}else if (nDeleType == 4){
		snprintf(sExec, sizeof(sExec), "delete from mailtosend where retry%c%d", c, *((int*)(pCondit)));
    }else if (nDeleType == 5){
            snprintf(sExec, sizeof(sExec), "delete from mailtosend where mailattach%c'%s'", c, (char*)(pCondit));
	}else if (nDeleType == -1){
		snprintf(sExec, sizeof(sExec), "delete from mailtosend");
	}

	int fd = file_lock(EMAIL_FILE_LOCK_NAME);
	HSQLITE hConn = 0;
	if (sqlite_conn( db, &hConn) != 0){file_unlock(fd);return -1;}
	sqlite_execute(hConn, 'w', sExec);
	sqlite_disconn(hConn);
	file_unlock(fd);
	return 0;
}

int count_mailtosend(const char*  db, int *pCnt)
{
	if(!db || !pCnt) return -1;
	int nFd = file_lock(EMAIL_FILE_LOCK_NAME);
    HSQLITE hConn = 0;
    if(sqlite_conn(db, &hConn) != 0)
    {
        file_unlock(nFd);
        return -1;
    }
    HSQLSTMT hStmt = 0;
    char sQuery[256] = {0};
    int nColumnCnt = 0;
    snprintf(sQuery, sizeof(sQuery), "select count(*) from mailtosend;");
    int nResult = sqlite_query_record(hConn, sQuery, &hStmt, &nColumnCnt);
    if(nResult !=0 || nColumnCnt == 0)
    {
        if(hStmt) sqlite_clear_stmt(hStmt);
        sqlite_disconn(hConn);
        file_unlock(nFd);
        return -1;
    }

    sqlite_query_column(hStmt, 0, pCnt);

    sqlite_clear_stmt(hStmt);
    sqlite_disconn(hConn);
    file_unlock(nFd);
    return 0;
}


