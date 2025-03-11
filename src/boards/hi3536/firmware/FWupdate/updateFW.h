#ifndef __UPDATE_FW_H__
#define __UPDATE_FW_H__

#include "md5.h"
#include "nandwrite.h"
#include "baseinfo.h"
#include "fwinfo.h"

#ifndef	O_BINARY		/* should be define'd on __WIN32__ */
#define O_BINARY	0
#endif /* O_BINARY */

#define MKIMAGE_DEBUG

#ifdef MKIMAGE_DEBUG
#define debug(fmt,args...)	printf (fmt ,##args)
#else
#define debug(fmt,args...)
#endif /* MKIMAGE_DEBUG */


#define DEFAULT_PAGE_SIZE   2048
//#define BLD_UPDATE_ALWAYS


int UPFW_creat_img(void);

int UPFW_list_intfo(char* datafile);

int UPFW_verify(char * datafile,mtd_table_t *mtdtable);

int UPFW_analysis_img( char* datafile , int b_analysis_all);

#ifdef BUILD_NANDWRITE
int UPFW_version_check(char* datafile);

int UPFW_run_update( char* datafile );
#endif

int UPFW_log(int err_not, char errstr[]);


#endif /* __UPDATE_FW_H__ */
