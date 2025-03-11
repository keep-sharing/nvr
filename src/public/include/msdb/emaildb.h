#ifndef _MS_EMAILDB_H_
#define _MS_EMAILDB_H_

#include "msdefs.h"
#define EMAIL_INIT_FILE_PATH "/opt/app/email.db"
#define EMAIL_DB_PATH		"/mnt/nand/app/email.db"

struct smtpconf
{
	char emailaddr[128];
	char username[128];
	char password[128];
	char smtpserver[128];
	int  smtpport;
	int  usessl;
	int  auth;
	char sender[128];
};

int read_smtpconf(const char* db, struct smtpconf* conf);
int write_smtpconf(const char* db, struct smtpconf* conf);

struct mailtosend
{
	int  mailid;
	char mailto[512];
	char mailcc[16];
	char mailsub[128];
	char mailbody[1024];
	char mailattach[1024];
	char tasktime[32];
	char sendtime[32];
	int  status;
	int  retry;
};

int read_mailtosend(const char* db, struct mailtosend mails[], int capacity, int* readcount);
int update_mailtosend(const char* db, int mailid, const char* sendtime, int status, int retry);
int add_mailtosend(const char* db, struct mailtosend* mail);
int remove_mailtosend(const char*  db, int nDeleType, int nCmpType, void* pCondit);
int count_mailtosend(const char*  db, int *pCnt);
#endif//_MS_EMAILDB_H_
