#include <pthread.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <errno.h>
#include "msmail.h"
#include "msstd.h"
#include "emaildb.h"
#include "msdefs.h"
#include "msdb.h"

#define MAX_DURATION 5
#define MS_MAIL_DIR 			(MS_ETC_DIR "/smtp")
#define TIMER_FOR_SMTP_CLEAN    (1)
#define SSMTP_CONF_FMT "server=%s\n\
port=%d\n\
usessl=%d\n\
usetls=%d\n\
userid=%s\n\
password=%s\n\
sender=%s\n\
name=%s\n\
subject=%s\n\
receiver=%s\n\
text=%s"


static int global_sendmail = 0;
static pthread_t global_thrhdl = 0;
static int  global_dodebug  = 1;
static int  global_maxmime  = 8;
static int  global_maxretry = 2;
static struct email g_mailconf;

static void  __attribute__ ((constructor)) __load_email_debug()
{
	char data[32] = {0};
	char conf[128] = {0};
	memset(&g_mailconf, 0, sizeof(g_mailconf));
	snprintf(conf, sizeof(conf), "%s/debug.conf", MS_MAIL_DIR);
	FILE* fp = fopen(conf, "r");
	if (!fp)
		return;
	while(1)
	{
		if (!fgets(data, sizeof(data)-1, fp))
			break;
		if (data[0] == 0)
			continue;
		if (strncmp(data, "debug=", 6) == 0)
		{
			if (strncmp(data+6, "yes", 3)==0 || strncmp(data+6, "1", 1)==0) 
				global_dodebug = 1;
		}else if (strncmp(data, "maxmime=", 8) == 0)
		{
			global_maxmime = atoi(data+8);
			if (global_maxmime <= 0)
				global_maxmime = 1;
		}else if (strncmp(data, "maxretry=", 9) == 0)
		{
			global_maxretry = atoi(data+9);
			if (global_maxretry <= 0)
				global_maxretry = 0;
		}
	}

	fclose(fp);
}

static int mail_debug_info(char *format, ...)
{
	int size;
	if(global_dodebug == 0)
		return 0;
	va_list ap;
	va_start(ap, format);
	size = vprintf(format, ap);
	va_end(ap);
	//msprintf(format);
	return size;
}

static int create_ssmtp_conf(const char * filename, struct email *mailconf, struct mailtosend *mail)
{
	FILE* fp = NULL;
	char info[2048] = {0};
	char receiver[512] = {0};
	char sender[256]={0};
	char *at_marker=NULL;
	int i = 0;

	at_marker=strchr(mailconf->sender_name,'@');
	if(at_marker){
		unsigned int marker_len=(unsigned int)(at_marker-mailconf->sender_name);
		marker_len=ms_min(marker_len,256-1);
		if(at_marker>0){
			memcpy(sender,mailconf->sender_name,marker_len);
			sender[marker_len]='\0';
		}
		else{
			snprintf(sender,sizeof(sender),"%s",mailconf->sender_name);
		}
	}
	else{
		snprintf(sender,sizeof(sender),"%s",mailconf->sender_name);
	}
	//create ssmtp conf
	for(i = 0; i < EMAIL_RECEIVER_NUM; i++)
	{
		if(mailconf->receiver[i].address[0] != '\0')
		{
			if(strlen(receiver) == 0)
				snprintf(receiver, sizeof(receiver), "%s", mailconf->receiver[i].address);
			else
				snprintf(receiver + strlen(receiver), sizeof(receiver) - strlen(receiver), ";%s", mailconf->receiver[i].address);
		}
	}
	//david modify tls 
	snprintf(info, sizeof(info), SSMTP_CONF_FMT, mailconf->smtp_server, mailconf->port, (mailconf->enable_tls==2)?1:0, (mailconf->enable_tls==1)?1:0,
		mailconf->username, mailconf->password, mailconf->sender_addr, sender, mail->mailsub, receiver, mail->mailbody);
	fp = fopen(filename, "w");
	if (!fp)
		return -1;
	fwrite(info, strlen(info), 1, fp);
	fflush(fp);
	fclose(fp);
	fp = NULL;
	
	return 0;
}

static int remove_attachfile(const char *attach)
{
	if (strlen(attach))
		unlink(attach);
	return 0;
}

static void* thread_send_email(void* param)
{
    int nTimer = 0;
	int result  = 0;
	int stime   = MAX_DURATION;
	int mimeidx = 1;
	int mailcnt = 0;
	int i = 0, filecnt = 1;
	global_sendmail = 1;
	while(global_sendmail)
	{
	    nTimer++;
	    usleep(stime * 10000);
		int count = 0;
		struct mailtosend mail = {0};
		read_mailtosend(EMAIL_DB_PATH, &mail, 1, &count);
		if (count <= 0)
		{
			stime = (MAX_DURATION*10);
			continue;
		}
		stime = MAX_DURATION;
		mail_debug_info("read new mail\n");
		mail_debug_info("mailid  :%d\n", mail.mailid);
		mail_debug_info("mailto  :%s\n", mail.mailto);
		mail_debug_info("mailcc  :%s\n", mail.mailcc);
		mail_debug_info("mailsub :%s\n", mail.mailsub);
		mail_debug_info("mailbody:%s\n", mail.mailbody);
		mail_debug_info("attach  :%s\n", mail.mailattach);
		mail_debug_info("retry   :%d\n", mail.retry);		
		char mimefile[123] = {0};
		snprintf(mimefile, sizeof(mimefile), "%smailmime-%02d.txt", MS_MAIL_DIR, mimeidx);
		mimeidx++;
		if (mimeidx > global_maxmime)
			mimeidx = 1;
		if (mail.mailto[0] == 0) 
			result = -1;
		else 
			result = create_ssmtp_conf(mimefile, &g_mailconf, &mail);
		mail_debug_info("make mail mime. result:%d\n", result);
		filecnt = 1;
		if(mail.mailattach[0] != '\0')
		{
			for(i = 0; i < strlen(mail.mailattach) && mail.mailattach[i] != '\0'; i++)
			{
                if(mail.mailattach[i] == ' ' && mail.mailattach[i+1] == '/')
				{
					filecnt++;
				}
			}
		}
		if (result == 0)
		{
			char cmd[2560] = {0};
			if(mail.mailattach[0] == '\0')
				snprintf(cmd, sizeof(cmd), "nice -n 10 smtp -c %s;", mimefile);
			else
				snprintf(cmd, sizeof(cmd), "nice -n 10 smtp -c %s -f %d %s;", mimefile, filecnt, mail.mailattach);	
			result = ms_system(cmd);
			//msprintf("[david debug] result:%d filecnt:%d  size:%d CMD:%s\n", result, filecnt, strlen(mail.mailattach), cmd);
			mail_debug_info("send mail %s\n", !result?"ok":"fail");
		}
	
		int  status = 0;
		int icnt = 0;
		char sendtime[64] = {0};
		char file[256] = {0};
		time_to_string(time(0), sendtime);
		if (result == 0)
		{
			status = 1;
		}
		else
		{
			mail.retry++;
			if (mail.retry > global_maxretry) 
			{
				status = -1;
			}
				
			mailcnt = 0;
			count_mailtosend(EMAIL_DB_PATH, &mailcnt);
			if(mailcnt >= MAX_MAIL_RECORD_CNT)
			{
				remove_mailtosend(EMAIL_DB_PATH, 1, 0, &mail.mailid);
			}
			mail_debug_info("mailid:%d mailcnt:%d cur retry:%d do max retry:%d. drop it.\n", mail.mailid, mailcnt, mail.retry, global_maxretry);
		}
		
		if(status != 0)
		{
			if(filecnt == 1)
			{
				remove_attachfile(mail.mailattach);
				//msprintf("[david debug] result:%d status:%d file:%s 11", result, status, mail.mailattach);
			}
			else
			{
				for(i = 0; i < strlen(mail.mailattach)+1; i++)
				{
					if(mail.mailattach[i] != '\0' && mail.mailattach[i] != ' ')
					{
						file[icnt] = mail.mailattach[i];
						icnt++;
					}
					else
					{
						if(file[0] != '\0')
						{
							file[icnt] = '\0';
							remove_attachfile(file);
						}
						//msprintf("[david debug] file:%s mailattach:%s 22", file, mail.mailattach);
						file[0] = '\0';
						icnt = 0;
					}
				}
			}
		}
		
		unlink(mimefile);
		update_mailtosend(EMAIL_DB_PATH, mail.mailid, sendtime, status, mail.retry);

		nTimer = 0;
        remove_mailtosend(EMAIL_DB_PATH, 4, 1, &global_maxretry);
		int nNewStatus = 1;
		remove_mailtosend(EMAIL_DB_PATH, 3, 0, &nNewStatus);	

		//mail_debug_info("[david debug] result:%d, count:%d, global_maxretry:%d, nNewStatus:%d\n", result, count, global_maxretry, nNewStatus);
	}
	
	return NULL;
}

int ms_mail_start()
{
	msprintf("====== show msmail conf ======");
	msprintf("debug set:%d", global_dodebug);
	msprintf("max mime :%d", global_maxmime);
	msprintf("max retry:%d", global_maxretry);

	read_email(SQLITE_FILE_NAME, &g_mailconf);
	
	pthread_create(&global_thrhdl, 0, thread_send_email, 0);
	return 0;
}

void ms_mail_stop()
{
	if (global_sendmail)
	{
		global_sendmail = 0;
		pthread_join(global_thrhdl, 0);
	}
}
