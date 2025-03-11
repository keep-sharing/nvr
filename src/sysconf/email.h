#ifndef _MS_EMAIL_H_
#define _MS_EMAIL_H_
#include "msdb.h"

#define SMTP_LOG_PATH "/tmp/smtp.log"

typedef enum 
{
	ATT_IMG_JPG = 0,
	ATT_IMG_BMP,
}ATT_TYPE;

struct recp 
{
	char name[64];
	char address[128];
};

struct mailattach
{
	int type;
	char name[64];
	char path[128];
}; 

struct mailcontent 
{
	char sendername[64];
	char senderaddr[128];
	int recp_num;
	struct recp *recp_list;
	int cc_num;
	struct recp *cc_list;
	int bcc_num;
	struct recp *bcc_list;
	char subject[128];
	char datetime[32];
	char timediff[8];
	char body[1024];
	int attach_num;
	struct mailattach *attach_list;
};

int create_email_file(const char *dstfile, struct mailcontent *mailcont);
const char *search_timediff(const char *tzname);

int create_ssmtp_conf(const char * filename, struct email *mailconf, struct mailcontent*mailcontent);
int get_smtp_error_code(const char *path);

#endif //_MS_EMAIL_H_