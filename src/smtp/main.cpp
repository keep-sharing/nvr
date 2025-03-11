#include <signal.h>
#include "CSmtp.h"
//#include <iostream>

#define SMTP_DEBUG
#ifdef SMTP_DEBUG
#define smtp_debug(fmt, args...) printf(fmt, ##args)
#else
#define smtp_debug(fmt, args...)
#endif

#define MAX_STRING 128

static int parse_line(char *text, CSmtp *mail)
{
	int cnt = 0;
	char *ptr = NULL;
	char sep[] = "*", *token = 0;
	char str_temp[MAX_STRING*2] = {0};
	if(!text)
		return 0;
	snprintf(str_temp, sizeof(str_temp), "%s", text);
	token = strtok_r(str_temp, sep, &ptr);
	while(token)
	{
		mail->AddMsgLine(token);
		smtp_debug("line:%s\n", token);
		token = strtok_r(NULL, sep, &ptr);
		cnt++;
	}

	return cnt;
}

static int parse_recipient(char *receiver, CSmtp *mail)
{
	int cnt = 0;
	char *ptr = NULL;
	char sep[] = ";", *token = 0;
	char str_temp[MAX_STRING*2] = {0};
	if(!receiver)
		return 0;
	snprintf(str_temp, sizeof(str_temp), "%s", receiver);
	token = strtok_r(str_temp, sep, &ptr);
	while(token)
	{
		mail->AddRecipient(token);
		token = strtok_r(NULL, sep, &ptr);
		cnt++;
	}

	return cnt;
}

static void replace_last_lf_to_crlf(char *text)
{
    int len = strlen(text);
    if (len > 0) {
        if (text[len - 1] == '\n') {
            text[len - 1] = '\r';
            text[len] = '\n';
        }
    }
}

int parse_conf(char *conf_path, CSmtp *mail)
{
	char *temp;
	char server[MAX_STRING] = {0};
	char userid[MAX_STRING] = {0};
	char password[MAX_STRING]= {0};
	char sender_mail[MAX_STRING] = {0};
	char sender_name[MAX_STRING] = {0};
	char subject[MAX_STRING] = {0};
	char receiver[MAX_STRING*2] = {0};
	char text[MAX_STRING*8] = {0};
	int port = 25;
	int use_ssl = 0;
	int use_tls = 0;
	int pri = XPRIORITY_HIGH;
	int nTextStart = 0;
	//int attach = 5;
	
	char line[MAX_STRING*8];
	FILE *fp = fopen(conf_path, "rb");
	if(!fp)
		return -1;
	while(fgets(line, sizeof(line), fp) && line[0] != '\0')
	{
		if (line[0] == '#') 
			continue;
		temp = strchr(line, '\n');
		if (temp && temp[0] == line[0]) 
		{
			if(nTextStart)
			{
				snprintf(text+strlen(text), sizeof(text)-strlen(text), "%s", line);
				replace_last_lf_to_crlf(text);
			}
			continue;
		}
		else if(temp)
		{
			if(nTextStart)
			{
				snprintf(text+strlen(text), sizeof(text)-strlen(text), "%s", line);
				replace_last_lf_to_crlf(text);
			}			
			*temp = '\0';// Strip \n
		}
		temp = strchr(line, '=');
		if(!temp)
			continue;
		if(strstr(line, "server="))
			strcpy(server, &temp[1]);
		else if(strstr(line, "port="))
			port = atoi(&temp[1]);
		else if(strstr(line, "usessl="))
			use_ssl = atoi(&temp[1]);
		else if(strstr(line, "usetls="))
			use_tls = atoi(&temp[1]);
		else if(strstr(line, "userid="))
			strcpy(userid, &temp[1]);
		else if(strstr(line, "password="))
			strcpy(password, &temp[1]);
		else if(strstr(line, "sender="))
			strcpy(sender_mail, &temp[1]);
		else if(strstr(line, "name="))
			strcpy(sender_name, &temp[1]);
		else if(strstr(line, "subject="))
			strcpy(subject, &temp[1]);
		else if(strstr(line, "receiver="))
			strcpy(receiver, &temp[1]);
		else if(strstr(line, "text="))
		{
			strcpy(text, &temp[1]);
			snprintf(text+strlen(text), sizeof(text)-strlen(text), "\n");
        	replace_last_lf_to_crlf(text);
			nTextStart = 1;
		}
		else if(strstr(line, "priority="))
			pri = atoi(&temp[1]);
		//else if(strstr(line, "attach="))
		//	attach = atoi(&temp[1]);		
	}
	fclose(fp);

	mail->SetSMTPServer(server, port);
	if(use_ssl)
		mail->SetSecurityType(USE_SSL);
	else if(use_tls)
		mail->SetSecurityType(USE_TLS);
	else
		mail->SetSecurityType(NO_SECURITY);
	mail->SetLogin(userid);
	mail->SetPassword(password);
	mail->SetSenderName(sender_name);
	mail->SetSenderMail(sender_mail);
	mail->SetReplyTo(sender_mail);
	mail->SetSubject(subject);
	parse_recipient(receiver, mail);	
	mail->SetXPriority((CSmptXPriority)pri);
	mail->AddMsgLine(text);

	smtp_debug("server:%s port:%d ssl:%d tls:%d user:%s password:%s name:%s mail:%s subject:%s receiver:%s text:%s\n",
			server, port, use_ssl, use_tls, userid, password, sender_name, sender_mail, subject, receiver, text);
	return 0;
}

//use 
// /opt/app/smtp -c /opt/app/smtp.conf -f 2 filepath2 filepath2
//conf
#if 0
server=smtp.gmail.com
port=587
usessl=0
usetls=1
userid=ckt1120@gmail.com
password=1
sender=ckt1120@gmail.com
name=Bruce Lin
subject=Alarm
receiver=iamhycljc@163.com
text=Motion Detection.

//use ssl
server=smtp.163.com
port=465
usessl=1
usetls=0
userid=mstest20131@163.com
password=1
sender=mstest20131@163.com
name=mstest20131@163.com
subject=Test File
receiver=iamhycljc@163.com
text=Test Mail.

//normal
server=smtp.163.com
port=25
usessl=0
usetls=0
userid=mstest20131@163.com
password=1
sender=mstest20131@163.com
name=mstest20131@163.com
subject=Test File
receiver=iamhycljc@163.com
text=Test Mail.

#endif

void smtp_stop_handler(int signum)
{
	smtp_debug("smtp exit\n");
	exit(0);
}

void smtp_alarm_handler(int signum)
{
	printf("=====smtp_alarm_handler==========\n");
	exit(1);
}

int main(int argc, char** argv)
{
	int i = 0, j, cnt;
	int result = -1;

	signal(SIGINT, smtp_stop_handler);
	signal(SIGKILL, smtp_stop_handler);
	signal(SIGALRM, smtp_alarm_handler);
	alarm(5*60);
	try
	{
		CSmtp mail;
		mail.SetCharSet("utf-8");
#if 0	//Debug
#define test_gmail_ssl

#if defined(test_gmail_tls)
		mail.SetSMTPServer("smtp.gmail.com",587);
		mail.SetSecurityType(USE_TLS);
#elif defined(test_gmail_ssl)
		mail.SetSMTPServer("smtp.gmail.com",465);
		mail.SetSecurityType(USE_SSL);
#elif defined(test_hotmail_TLS)
		mail.SetSMTPServer("smtp.live.com",25);
		mail.SetSecurityType(USE_TLS);
#elif defined(test_aol_tls)
		mail.SetSMTPServer("smtp.aol.com",587);
		mail.SetSecurityType(USE_TLS);
#elif defined(test_yahoo_ssl)
		mail.SetSMTPServer("plus.smtp.mail.yahoo.com",465);
		mail.SetSecurityType(USE_SSL);
#endif
		mail.SetLogin("ckt1120@gmail.com");
		mail.SetPassword("c0000000");
		mail.SetSenderName("User");
		mail.SetSenderMail("ckt1120@gmail.com");
		mail.SetReplyTo("ckt1120@gmail.com");
		mail.SetSubject("The message");
		mail.AddRecipient("bruce@milesight.com");
		mail.SetXPriority(XPRIORITY_NORMAL);
		mail.SetXMailer("The Bat! (v3.02) Professional");
		mail.AddMsgLine("Hello,");
		mail.AddMsgLine("");
		mail.AddMsgLine("...");
		mail.AddMsgLine("How are you today?");
		mail.AddMsgLine("");
		mail.AddMsgLine("Regards");
		mail.ModMsgLine(5,"regards");
		mail.DelMsgLine(2);
		mail.AddMsgLine("User");
#endif
		for(i = 1; i < argc; i++)
		{
			if(strcmp(argv[i], "-c") == 0)
			{
				result = parse_conf(argv[i+1], &mail);
				smtp_debug("smtp conf:%s.parse result:%d\n", argv[i+1], result);
			}
			else if(strcmp(argv[i], "-t") == 0)//text
			{
				smtp_debug("more text:%s\n", argv[i+1]);
				if(argv[i+1])
				{
					parse_line(argv[i+1], &mail);
				}
			}
			else if(strcmp(argv[i], "-f") == 0)//filenum filepath filepath ...
			{
				cnt = atoi(argv[i+1]);
				if(cnt <= 0) continue;
				for(j = 0; j < cnt; j++)
				{
					mail.AddAttachment(argv[i+1+j+1]);
				}	
			}
		}
		if(!result)
		{
			mail.Send();
		}
	}
	catch(ECSmtp e)
	{
		smtp_debug("send smtp failed\n");
		exit(1);
	}
	catch(...)
	{
		smtp_debug("send smtp failed\n");
		exit(1);
	}

	smtp_debug("send smtp successfully\n");
	return 0;
}

