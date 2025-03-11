#ifndef _app_log_h_
#define _app_log_h_

#include <stdio.h>
// from sys/syslog.h
#define LOG_EMERG       0       /* system is unusable */
#define LOG_ALERT       1       /* action must be taken immediately */
#define LOG_CRIT        2       /* critical conditions */
#define LOG_ERROR       3       /* error conditions */
#define LOG_WARNING     4       /* warning conditions */
#define LOG_NOTICE      5       /* normal but significant condition */
#define LOG_INFO        6       /* informational */
#define LOG_DEBUG       7       /* debug-level messages */

/* Word color define */
#define YELLOW			"\033[0;33m"	// yellow
#define GRAY 			"\033[2;37m"
#define DGREEN 			"\033[1;32m"
#define GREEN 			"\033[0;32m"
#define DARKGRAY 		"\033[0;30m"
//#define BLACK 		"\033[0;39m"
#define BLACK 			"\033[2;30m" 	// not faint black
#define NOCOLOR 		"\033[0;39m \n"
#define CLRCOLOR		"\033[0;39m "
//#define NOCOLOR 		"\033[0m \n"
#define BLUE 			"\033[1;34m"
#define DBLUE 			"\033[2;34m"	
#define RED 			"\033[0;31m"
#define BOLD 			"\033[1m"
#define UNDERLINE 		"\033[4m"
#define CLS 			"\014"
#define NEWLINE 		"\r\n"


#ifdef __cplusplus
extern "C" {
#endif

void app_log(int level, const char* format, ...);

void app_log_setlevel(int level);

int app_log_getlevel();

void app_print(const char* msg);

#define app_debug(level, prefix, format, ...) \
	do{ \
		if(app_log_getlevel() >= level) \
		{ \
			char __smsg[1024] = {0}; \
			snprintf(__smsg, sizeof(__smsg), "%s|" format "==func:%s file:%s line:%d", \
				prefix, ##__VA_ARGS__, __func__, __FILE__, __LINE__); \
			app_print(__smsg); \
		} \
    }while(0)

#define app_log_printf(level, format, ...) \
	do{ \
		switch(level) \
		{ \
		case LOG_ERROR: \
			app_debug(level, "ERR:", RED format CLRCOLOR, ##__VA_ARGS__); \
			break; \
		case LOG_WARNING: \
			app_debug(level, "WRN:", YELLOW format CLRCOLOR, ##__VA_ARGS__); \
			break; \
		default: \
			app_debug(level, "INF:", format, ##__VA_ARGS__); \
			break; \
		} \
	}while(0)



#ifdef __cplusplus
}
#endif
#endif /* !_app_log_h_ */
