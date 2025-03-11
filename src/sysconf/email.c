#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include "email.h"
#include "msdefs.h"

#define BASELINELEN 72
#define BASEMAXINLINE 256
#define ENDL "\n"

struct tz_map 
{
	const char *tzname;
	const char *timediff;
};

struct att_type
{
	ATT_TYPE id;
	const char *name;
};

static struct att_type att_type_map[] = 
{
	{ATT_IMG_JPG, "image/jpg"},
	{ATT_IMG_BMP, "image/bmp"},
};

static struct tz_map tzmap[] = 
{
	{"UTC11", 		"-1100"},
	{"UTC10", 		"-1000"},
	{"UTC9", 		"-0900"},
	{"UTC8", 		"-0800"},
	{"UTC7", 		"-0700"},
	{"UTC6", 		"-0600"},
	{"UTC5", 		"-0500"},
	{"UTC4:30", 	"-0430"},
	{"UTC4", 		"-0400"},
	{"UTC3:30", 	"-0330"},
	{"UTC3", 		"-0300"},
	{"UTC2", 		"-0200"},
	{"UTC1", 		"-0100"},
	{"UTC0", 		"+0000"},
	{"UTC-1", 		"+0100"},
	{"UTC-2", 		"+0200"},
	{"UTC-3", 		"+0300"},
	{"UTC-3:30", 	"+0330"},
	{"UTC-4", 		"+0400"},
	{"UTC-5", 		"+0500"},
	{"UTC-5:30", 	"+0530"},
	{"UTC-6", 		"+0600"},
	{"UTC-7", 		"+0700"},
	{"UTC-8", 		"+0800"},
	{"UTC-9", 		"+0900"},
	{"UTC-9:30", 	"+0930"},
	{"UTC-10", 		"+1000"},
	{"UTC-10:30", 	"+1030"},
	{"UTC-11", 		"+1100"},
	{"UTC-12", 		"+1200"},
	{"UTC-12:45", 	"+1245"},
	{"UTC-13", 		"+1300"},
};

static int tzmap_size = sizeof(tzmap) / sizeof(tzmap[0]);

struct baseio
{
	int iocp;
	int iolen;
	int linelength;
	int ateof;
	unsigned char iobuf[BASEMAXINLINE];
};

#define HEADER_FMT_PART1 "\
Date: %s %s\n\
From: %s <%s>\n"

#define HEADER_FMT_PART2 "\
Subject: %s\n\
MIME-Version: 1.0\n\
Content-type: multipart/mixed; boundary=\"#HDVR_BOUNDARY#\"\n\n"

#define BODY_FMT "\
--#HDVR_BOUNDARY#\n\
Content-Type: text/plain; charset=utf-8\n\
Content-Transfer-Encoding: quoted-printable\n\n\
%s\n\n"

#define ATTACH_HEADER_FMT "\
\n--#HDVR_BOUNDARY#\n\
Content-Type: %s; name=%s\n\
Content-Disposition: attachment; filename=%s\n\
Content-Transfer-Encoding: base64\n\n"

#define ENDBOUNDARY "\n--#HDVR_BOUNDARY#--\n"

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

int create_ssmtp_conf(const char * filename, struct email *mailconf, struct mailcontent*mailcontent)
{
	FILE* fp = NULL;
	char info[1024] = {0};
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
		mailconf->username, mailconf->password, mailconf->sender_addr, sender, mailcontent->subject, receiver, mailcontent->body);
	fp = fopen(filename, "w");
	if (fp){
		fwrite(info, strlen(info), 1, fp);
		fclose(fp);
		fp = NULL;
	}
	else
		return -1;
	return 0;
}

int get_smtp_error_code(const char *path)
{
	char buf[32] = {0};
	FILE *fp = fopen(path, "rb");
	if(!fp)
		return 0;
	fgets(buf, sizeof(buf), fp);
	fclose(fp);

	return atoi(buf);
}

const char *search_timediff(const char *tzname)
{
	int i;

	for (i = 0; i < tzmap_size; i++) {
		if (strcmp(tzname, tzmap[i].tzname) == 0) {
			return tzmap[i].timediff;
		}
	}

	return "+0000";//找不到，返回默认值
}

static int inbuf(struct baseio *bio, FILE *fi)
{
	int l;

	if (bio->ateof)
		return 0;

	if ((l = fread(bio->iobuf,1,BASEMAXINLINE,fi)) <= 0) 
	{
		if (ferror(fi))
			return -1;

		bio->ateof = 1;
		return 0;
	}

	bio->iolen= l;
	bio->iocp= 0;

	return 1;
}

static int inchar(struct baseio *bio, FILE *fi)
{
	if (bio->iocp>=bio->iolen) 
	{
		if (!inbuf(bio, fi))
			return EOF;
	}

	return bio->iobuf[bio->iocp++];
}

static int ochar(struct baseio *bio, int c, FILE *so)
{
	if (bio->linelength >= BASELINELEN) 
	{
		if (fputs(ENDL, so) == EOF)
		{
			return -1;
		}

		bio->linelength= 0;
	}

	if (putc(((unsigned char) c), so) == EOF)
	{
		return -1;
	}

	bio->linelength++;

	return 1;
}

/*!
 * \brief Performs a base 64 encode algorithm on the contents of a File
 * \param filename The path to the file to be encoded. Must be readable, file is opened in read mode.
 * \param so A FILE handle to the output file to receive the base 64 encoded contents of the input file, identified by filename.
 *
 * TODO: shouldn't this (and the above 3 support functions) be put into some kind of external utility location, such as funcs/func_base64.c ?
 *
 * \return zero on success, -1 on error.
 */
static int base_encode(char *filename, FILE *so)
{
	static const unsigned char dtable[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
		'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
		'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0',
		'1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};
	int i,hiteof= 0;
	FILE *fi;
	struct baseio bio;

	memset(&bio, 0, sizeof(bio));
	bio.iocp = BASEMAXINLINE;

	if (!(fi = fopen(filename, "rb"))) {
		printf("Failed to open file: %s: %s\n", filename, strerror(errno));
		return -1;
	}

	while (!hiteof) {
		unsigned char igroup[3], ogroup[4];
		int c,n;

		igroup[0]= igroup[1]= igroup[2]= 0;

		for (n = 0;n < 3; n++) {
			if ((c = inchar(&bio, fi)) == EOF) {
				hiteof= 1;
				break;
			}

			igroup[n]= (unsigned char)c;
		}

		if (n > 0) {
			ogroup[0]= dtable[igroup[0]>>2];
			ogroup[1]= dtable[((igroup[0]&3)<<4) | (igroup[1]>>4)];
			ogroup[2]= dtable[((igroup[1]&0xF)<<2) | (igroup[2]>>6)];
			ogroup[3]= dtable[igroup[2]&0x3F];

			if (n < 3) {
				ogroup[3]= '=';

				if (n < 2)
					ogroup[2]= '=';
			}

			for (i = 0;i < 4;i++)
				ochar(&bio, ogroup[i], so);
		}
	}

	fclose(fi);

	if (fputs(ENDL, so) == EOF) {
		return 0;
	}

	return 1;
}

// 添加附件
static void add_attachment(FILE *fp, struct mailattach *attach)
{
	char buf[2048] = {0};

	snprintf(buf, sizeof(buf), ATTACH_HEADER_FMT, att_type_map[attach->type].name, attach->name, attach->name);
	fwrite(buf, strlen(buf), 1, fp);
	base_encode(attach->path, fp);
}

// 添加收件人/抄送人/隐藏抄送人
static void add_recp(FILE *fp, const char *header, struct recp *recp)
{
	char buf[1024] = {0};

	snprintf(buf, sizeof(buf), "%s: %s <%s>\n", header, recp->name, recp->address);
	fwrite(buf, strlen(buf), 1, fp);
}

int create_email_file(const char *dstfile, struct mailcontent *mailcont)
{
	FILE *fp;
	char buf[2048] = {0};
	int i;
	
	fp = fopen(dstfile, "wb");
	
	// 写邮件头第一部分
	snprintf(buf, sizeof(buf), HEADER_FMT_PART1, mailcont->datetime, mailcont->timediff, mailcont->sendername, mailcont->senderaddr);
	fwrite(buf, strlen(buf), 1, fp);
	
	// 写收件人，抄送人，隐藏抄送人等
	if (mailcont->recp_num > 0 && mailcont->recp_list != NULL) {
		for (i = 0; i < mailcont->recp_num; i++) {
			add_recp(fp, "To", &mailcont->recp_list[i]);
		}
	}
	
	if (mailcont->cc_num > 0 && mailcont->cc_list != NULL) {
		for (i = 0; i < mailcont->cc_num; i++) {
			add_recp(fp, "CC", &mailcont->cc_list[i]);
		}
	}
	
	if (mailcont->bcc_num > 0 && mailcont->bcc_list != NULL) {
		for (i = 0; i < mailcont->bcc_num; i++) {
			add_recp(fp, "Bcc", &mailcont->bcc_list[i]);
		}
	}
	
	// 写邮件头第二部分
	snprintf(buf, sizeof(buf), HEADER_FMT_PART2, mailcont->subject);
	fwrite(buf, strlen(buf), 1, fp);
	
	// 写正文
	if (mailcont->body[0]) {
		snprintf(buf, sizeof(buf), BODY_FMT, mailcont->body);
		fwrite(buf, strlen(buf), 1, fp);
	}
	
	// 写附件
	if (mailcont->attach_num > 0 && mailcont->attach_list != NULL) {
		for (i = 0; i < mailcont->attach_num; i++) {
			add_attachment(fp, &mailcont->attach_list[i]);
		}
	}
	
	// 写结尾
	fwrite(ENDBOUNDARY, strlen(ENDBOUNDARY), 1, fp);
	
	fclose(fp);
	
	return 0;
}
