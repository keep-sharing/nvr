#ifndef _BACKUP_H_
#define _BACKUP_H_

#include "msdefs.h"

#if 0
#ifndef _BACKUP_ERR_H_
#define _BACKUP_ERR_H_
#define BACKUP_ERR_NONE 							0
#define BACKUP_ERR_ARGS 							1
#define BACKUP_ERR_CHANNEL 							2
#define BACKUP_ERR_PATH 							3
#define BACKUP_ERR_TIME								4
#define BACKUP_ERR_CREATE							5
#define BACKUP_ERR_INIT								6
#define BACKUP_ERR_WRITE							7
#define BACKUP_ERR_NOSPACE							8
#define BACKUP_ERR_NOMEM							9
#endif
#endif

struct bk_file_info
{
	char backpath[128];
	char filename[128];
	int filesize; 		
	int hasaudio;		
	int video_width;	
	int video_height;	
	int video_fps;		
	int video_bitrate;	
	int video_codec;
	int audio_samplerate;
	int audio_bitrate;
};

void *backup_create(struct bk_file_info *fileinfo);
int backup_write(void *fhndl, void *data);
int backup_finish(void *fhndl, int chan_id);

#endif
