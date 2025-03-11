#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>

#define MAXHOSTNAMELEN    256
#define AST_MAX_CONTEXT   256
#define AST_MAX_EXTENSION 256
#define MAX_LANGUAGE      256

#define _A_ __FILE__, __LINE__, __PRETTY_FUNCTION__
#ifdef AST_LOG_WARNING
#undef AST_LOG_WARNING
#endif
#define __LOG_WARNING  3
#define AST_LOG_WARNING    __LOG_WARNING, _A_
void ast_log(int level, const char *file, int line, const char *function, const char *fmt, ...){}



//static char charset[32] = "ISO-8859-1";
static char charset[32] = "utf-8";
//static char emaildateformat[32] = "%A, %B %d, %Y at %r";


#define BASELINELEN 72
#define BASEMAXINLINE 256
#ifdef IMAP_STORAGE
#define ENDL "\r\n"
#else
#define ENDL "\n"
#endif

#if 0
static void ast_copy_string(char *dst, const char *src, size_t size)
{
	while (*src && size) {
		*dst++ = *src++;
		size--;
	}
	if (__builtin_expect(!size, 0))
		dst--;
	*dst = '\0';
}

static int ast_strlen_zero(const char *s)
{
	return (!s || (*s == '\0'));
}

static long int ast_random(void)
{
	long int res;
	res = random();
	return res;
}
#endif

#if !defined(ast_strdupa) && defined(__GNUC__)
#define ast_strdupa(s)                                                    \
	(__extension__                                                    \
	({                                                                \
		const char *__old = (s);                                  \
		size_t __len = strlen(__old) + 1;                         \
		char *__new = __builtin_alloca(__len);                    \
		memcpy (__new, __old, __len);                             \
		__new;                                                    \
	}))
#endif




struct baseio {
	int iocp;
	int iolen;
	int linelength;
	int ateof;
	unsigned char iobuf[BASEMAXINLINE];
};

static int inbuf(struct baseio *bio, FILE *fi)
{
	int l;

	if (bio->ateof)
		return 0;

	if ((l = fread(bio->iobuf,1,BASEMAXINLINE,fi)) <= 0) {
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
	if (bio->iocp>=bio->iolen) {
		if (!inbuf(bio, fi))
			return EOF;
	}

	return bio->iobuf[bio->iocp++];
}

/*!
 * \brief utility used by base_encode()
 */
static int ochar(struct baseio *bio, int c, FILE *so)
{
	if (bio->linelength >= BASELINELEN) {
		if (fputs(ENDL, so) == EOF) {
			return -1;
		}

		bio->linelength= 0;
	}

	if (putc(((unsigned char) c), so) == EOF) {
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
		ast_log(AST_LOG_WARNING, "Failed to open file: %s: %s\n", filename, strerror(errno));
		return -1;
	}

	while (!hiteof){
		unsigned char igroup[3], ogroup[4];
		int c,n;

		igroup[0]= igroup[1]= igroup[2]= 0;

		for (n= 0;n<3;n++) {
			if ((c = inchar(&bio, fi)) == EOF) {
				hiteof= 1;
				break;
			}

			igroup[n]= (unsigned char)c;
		}

		if (n> 0) {
			ogroup[0]= dtable[igroup[0]>>2];
			ogroup[1]= dtable[((igroup[0]&3)<<4) | (igroup[1]>>4)];
			ogroup[2]= dtable[((igroup[1]&0xF)<<2) | (igroup[2]>>6)];
			ogroup[3]= dtable[igroup[2]&0x3F];

			if (n<3) {
				ogroup[3]= '=';

				if (n<2)
					ogroup[2]= '=';
			}

			for (i= 0;i<4;i++)
				ochar(&bio, ogroup[i], so);
		}
	}

	fclose(fi);

	if (fputs(ENDL, so) == EOF) {
		return 0;
	}

	return 1;
}

//逐个获取多个以';'分隔的邮件
static int split_multi_mail(const char* cc, int* pos, char mailinfo[128])
{
	if (!cc || !pos) return -1;
	int npos = *pos;
	if (npos < 0) return -1;
	//char* start = (char*)(cc) + (*pos);
	int i = 0;
	for (; i < 128; i++, npos++)
	{
		if (cc[npos]==';')
		{
			mailinfo[i] = 0;
			*pos = npos + 1;
			return 0;
		}
		else if (cc[npos]==0) 
		{
			mailinfo[i] = 0;
			*pos = -1;
			if (npos==0) return -1;
			return 0;
		}
		mailinfo[i] = cc[npos];
	}
	return -1;
}
//multi to
//From: "penzchan@hotmail.com" <penzchan@hotmail.com>
//To: penzchan <penzchan@sina.com>, 
//	penzchan <penzchan@sohu.com>
//Subject: 111111111
static int add_email_to(FILE* p, const char* to)
{
	if (!p || to[0]==0) return 0;
	int i = 0;
	int pos = 0;
	int addheader = 0;
	for (; i < 512; i++)
	{
		char mailto[128] = {0};
		if (split_multi_mail(to, &pos, mailto) == -1) break;
		if (mailto[0] == 0) break;
		char toname[128] = {0};
		snprintf(toname, sizeof(toname), "%s", mailto);
		char* find = strchr(toname, '@');
		if (!find) break;
		*find = 0;
		if (addheader == 0){
			fprintf(p, "To: %s <%s>", toname, mailto);
			addheader = 1;
		}else{
			fprintf(p, "," ENDL " %s <%s>", toname, mailto);			
		}
	}
	fprintf(p, ENDL);
	return 0;	
}
#if 0
static int add_email_cc(FILE* p, const char* cc)
{
	if (!p || cc[0]==0) return 0;
	int i = 0;
	int pos = 0;
	int addheader = 0;
	for (; i < 512; i++)
	{
		char mailcc[128] = {0};
		if (split_multi_mail(cc, &pos, mailcc) == -1) break;
		if (mailcc[0] == 0) break;
		char ccname[128] = {0};
		snprintf(ccname, sizeof(ccname), "%s", mailcc);
		char* find = strchr(ccname, '@');
		if (!find) break;
		*find = 0;
		if (addheader == 0){
			fprintf(p, "Cc: %s <%s>", ccname, mailcc);
			addheader = 1;
		}else{
			fprintf(p, "," ENDL " %s <%s>", ccname, mailcc);			
		}
	}
	fprintf(p, ENDL);
	return 0;
}
#endif
//参数:
//format  :附件后缀格式wav/txt/...
//attach  :附件名字(不包括后缀名)
//filename:文件名字(不包括路径)
//bound   :mime的分隔字符串
//last    :是否为最后附件
static int add_email_attachment(FILE* p, char* bound, char* attachs)
{
	if (!p || attachs[0]==0) return 0;
	int i = 0;
	int pos = 0;
	//int addheader = 0;
	for (; i < 512; i++)
	{
		char path[128] = {0};
		if (split_multi_mail(attachs, &pos, path) == -1) break;
		if (path[0] == 0) break;
		//解析附件名
		char name[128]  = {0};
		//char format[32] = {0};
		char* find = NULL;
		find = strrchr(path, '/');
		if (find) snprintf(name, sizeof(name), "%s", find+1);
		else snprintf(name, sizeof(name), "%s", path);	
		//find = strrchr(file, '.');
		//if (find) snprintf(format, sizeof(format), "%s", find+1);
		//printf("######### filename:%s\n", name); sleep(2);
		char* ctype = "application/octet-stream";
		fprintf(p, "--%s" ENDL, bound);
		fprintf(p, "Content-Type: %s; name=\"%s\"" ENDL, ctype, name);
		fprintf(p, "Content-Transfer-Encoding: base64" ENDL);
		fprintf(p, "Content-Disposition: attachment; filename=\"%s\"" ENDL ENDL, name);
		base_encode(path, p);
	}	
	return 0;
}
/*
static int add_email_attachment(FILE *p, char *format, char *attach, char *bound, char *filename)
{
	printf("format:%s attach:%s bound:%s filename:%s\n", format, attach, bound, filename);
	char fname[256];
	//char *ctype = (!strcasecmp(format, "ogg")) ? "application/" : "audio/x-";
	char* ctype = "application/octet-stream";
	fprintf(p, "--%s" ENDL, bound);
	fprintf(p, "Content-Type: %s; name=\"%s.%s\"" ENDL, ctype, filename, format);
	fprintf(p, "Content-Transfer-Encoding: base64" ENDL);
	fprintf(p, "Content-Disposition: attachment; filename=\"%s.%s\"" ENDL ENDL, filename, format);
	//组合附件的实际路径名
	snprintf(fname, sizeof(fname), "%s.%s", attach, format);
	base_encode(fname, p);
	return 0;
}
*/

static char global_sender[128] = {0};

static void make_email_file(FILE *p, char *src, const char* mailto, const char* cc, const char* subject, const char* body, char* attachs)
{
	char date[256] = {0};
	char bound[256]= {0};
	//char filename[256] = {0};
	printf("src:%s mailto:%s cc:%s subject:%s body:%s attach:%s\n", src, mailto, cc, subject, body, attachs);
	time_t seconds;
	time(&seconds);
	strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S +0800", localtime(&seconds));  // +0800 maybe hard code
	snprintf(bound, sizeof(bound), "----recovison_email_%d_%d_19841120", (int)time(0), (int)getpid());	
	fprintf(p, "Date: %s" ENDL, date);
	fprintf(p, "From: \"%s\" <%s>" ENDL, global_sender, src);
	char toname[128] = {0};
	snprintf(toname, sizeof(toname), "%s", mailto);
	char* findpos = strrchr(toname, '@');
	if (findpos) *findpos = 0;
	//add to
	add_email_to(p, mailto);
	//add cc
	//add_email_cc(p, cc);
	fprintf(p, "Subject: %s" ENDL, subject);
	fprintf(p, "Message-ID: <rvemail.%d.%d.19841120@recovsion>" ENDL, (unsigned int)random(), (int)getpid());
	fprintf(p, "MIME-Version: 1.0" ENDL);
	fprintf(p, "Content-Type: multipart/mixed; boundary=\"%s\"" ENDL, bound);
	fprintf(p, ENDL ENDL "This is a multi-part message in MIME format." ENDL ENDL);
	fprintf(p, "--%s" ENDL, bound);
	fprintf(p, "Content-Type: text/plain; charset=%s" ENDL "Content-Transfer-Encoding: 8bit" ENDL ENDL, charset);
	//add body
	fprintf(p, "%s" ENDL ENDL, body);
	//add attachment
	add_email_attachment(p, bound, attachs);
	//添加结束符
	//if (hasattach) fprintf(p, ENDL ENDL "--%s--" ENDL "." ENDL, bound);
	//else fprintf(p,  "--%s--" ENDL ENDL ENDL, bound);
	fprintf(p, ENDL ENDL "--%s--" ENDL "." ENDL, bound);
}
#undef ENDL


int make_mail_mime_file(char* sender, char *src, const char* mailto, const char* mailcc, const char* subject, const char* body, const char* attachfile, char* mimefile)
{
	if (!src || !mailto || !mailcc || !subject || !body || !mimefile) return -1;
	if (sender) snprintf(global_sender, sizeof(global_sender), "%s", sender);
	else snprintf(global_sender, sizeof(global_sender), "%s", "Recovison");
	FILE* fp = fopen(mimefile, "w");
	if (!fp) return -1;
	make_email_file(fp, src, mailto, mailcc, subject, body, (char*)(attachfile));
	fclose(fp);
	return 0;
}
