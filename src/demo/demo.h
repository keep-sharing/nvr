#ifndef _DEMO_H_
#define _DEMO_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "msstd.h"
#include "msdefs.h"

#define RED 			"\033[0;31m"
#define GREEN 			"\033[0;32m"
#define CLRCOLOR		"\033[0;39m "

#define hdprintf(color, format, ...) \
    do{\
		printf(color format CLRCOLOR, ##__VA_ARGS__);\
    }while(0);
    
#define hdinfo(format, ...) \
    do{\
		printf(GREEN format CLRCOLOR, ##__VA_ARGS__);\
    }while(0);

#define hderr(format, ...) \
    do{\
		printf(RED format CLRCOLOR, ##__VA_ARGS__);\
    }while(0);

int test_start();
int test_stop();

int send_frame_to_vdec(struct reco_frame *frame);

#ifdef __cplusplus
}
#endif

#endif


