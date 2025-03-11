#include "backup.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <time.h>
//#include "disk_op.h"
#include "msavi.h"
#include "msdefs.h"

#define SAFE_FREE_SPACE	20
#define CACHE_SIZE	40
#define MAX_BACKUP_HANDLE 64
#define MAX_FRAME_BUFF 1024
#define MAX_SIZE_PRINT (10<<20)
	
struct bk_handle
{
	void *avihdl;
	char filename[128];
	int vdts;
	int adts;
	int curfid;
	uint64_t size_limit;
	uint64_t fsize;
	uint64_t free_space;
	uint64_t wsize;
	struct bk_file_info fileinfo;
};

static struct reco_frame frame_buff[MAX_BACKUP_HANDLE][MAX_FRAME_BUFF];
static int frame_buff_cnt[MAX_BACKUP_HANDLE];
static char *buf_frame_data[MAX_BACKUP_HANDLE] = {0};

//static uint64_t cache_size_byte = CACHE_SIZE << 20;
extern uint64_t cache_size_byte;

static uint64_t get_file_size_b(const char *fpath)
{
	struct stat st;
	
	if (stat(fpath, &st) != 0) 
	{
		perror("stat failed:");
		return -1;
	}
	
	return (uint64_t)st.st_size;
}

#if 0
static long get_file_size_m(const char *fpath)
{
	return (long)(get_file_size_b(fpath) >> 20);
}
#endif

static uint64_t get_free_space(const char *fspath)
{
	struct statfs st;
	
	if (statfs(fspath, &st) != 0) {
		perror("statfs failed:");
		return -1;
	}
	
	return (uint64_t)st.f_bsize * st.f_bfree;
}

static long get_free_space_m(const char *fspath)
{
	return (long)(get_free_space(fspath) >> 20);
}

static int avi_init_fileinfo(struct avi_format *avifmt, struct bk_file_info *fileinfo)
{
	memset(avifmt, 0, sizeof(struct avi_format));
	avifmt->video_width = fileinfo->video_width;
	avifmt->video_height = fileinfo->video_height;
	avifmt->video_codec_id = fileinfo->video_codec;
	avifmt->video_fps = fileinfo->video_fps;
	avifmt->audio_samplerate = fileinfo->audio_samplerate;
	avifmt->audio_bitrate = fileinfo->audio_bitrate;
	avifmt->audio_codec_id = 0x10006;
	avifmt->audio_channels = 1;		
	return 0;
}

static int avfile_open(struct bk_handle *bkhdl, struct bk_file_info *fileinfo)
{
    int i = 0;
	struct avi_format avifmt;
	if (bkhdl == NULL || fileinfo == NULL) 
		return -1;
	
	snprintf(bkhdl->filename, sizeof(bkhdl->filename), "%s/%s_%02d.avi", fileinfo->backpath, fileinfo->filename, bkhdl->curfid);
	avi_init_fileinfo(&avifmt, fileinfo);
	while(ms_file_existed(bkhdl->filename) && i < 20)
	{
		snprintf(bkhdl->filename, sizeof(bkhdl->filename), "%s/%s_%02d(%d).avi", fileinfo->backpath, 
			fileinfo->filename, bkhdl->curfid, ++i);
	}
	bkhdl->avihdl = ms_avi_open(bkhdl->filename, &avifmt);
	if(bkhdl->avihdl == NULL)
	{
		printf("open new file error.\n");
		return -1;
	}

	bkhdl->vdts = bkhdl->adts = 0;
	bkhdl->fsize = get_file_size_b(bkhdl->filename);
	//printf("start write %s.\n", bkhdl->filename);
	
	return 0;
}

static int avfile_close(struct bk_handle *bkhdl)
{
	return ms_avi_close(bkhdl->avihdl);
}

static int ms_is_dir(const char *sDir)
{
	struct stat filestat;
	if(stat(sDir, &filestat) != 0)
	{
		perror(sDir);
		return 0;
	}
	
	if(S_ISDIR(filestat.st_mode))
	{
		return 1;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////
//
void *backup_create(struct bk_file_info *fileinfo)
{
	struct tm t1 = {0};
	time_t now_sec = time(NULL);
	localtime_r(&now_sec, &t1);
	if (fileinfo == NULL) 
		return NULL;

	struct bk_handle *bkhdl = (struct bk_handle *)ms_calloc(1, sizeof(struct bk_handle));
	if (bkhdl == NULL) 
		return NULL;
 	
	memcpy(&bkhdl->fileinfo, fileinfo, sizeof(struct bk_file_info));
	if (bkhdl->fileinfo.filesize <= 0) 
	{
		bkhdl->fileinfo.filesize = 1024;
	}
	bkhdl->size_limit = (bkhdl->fileinfo.filesize << 20) - (100 << 10);
	snprintf(bkhdl->fileinfo.backpath, sizeof(bkhdl->fileinfo.backpath), "%s/%04d.%02d.%02d", fileinfo->backpath, t1.tm_year+1900, t1.tm_mon+1, t1.tm_mday);
	if(!ms_is_dir(bkhdl->fileinfo.backpath))
	{
		mkdir(bkhdl->fileinfo.backpath, 0755);
	}
	
	if (fileinfo->video_width == 0 || fileinfo->video_height == 0 || fileinfo->video_fps == 0)
	{
		//printf("video_width:%d video_height:%d fps:%d backup_create failed.\n", fileinfo->video_width, fileinfo->video_height,
		//	fileinfo->video_fps);
		return bkhdl;
	}

	if (avfile_open(bkhdl, &bkhdl->fileinfo) != 0) 
	{
		printf("create file %s/%s failed\n", bkhdl->fileinfo.backpath, bkhdl->fileinfo.filename);
		ms_free(bkhdl);
		return NULL;
	}
	
	bkhdl->free_space = get_free_space_m(bkhdl->fileinfo.backpath);
	return bkhdl;
}

int backup_write(void *fhndl, void *data)
{	
	int i;
	int extra_size = 1 << 17;
	struct bk_handle *bkhdl = (struct bk_handle *)fhndl;
	struct reco_frame *frame = (struct reco_frame *)data;
	if (fhndl == NULL || frame == NULL)
		return -1;
	
	if (bkhdl->fileinfo.video_width == 0 || bkhdl->fileinfo.video_height == 0 || bkhdl->fileinfo.video_fps == 0)
	{
		if(frame->strm_type != ST_VIDEO)
			return BACKUP_ERR_NONE;
		bkhdl->fileinfo.video_width = frame->width;
		bkhdl->fileinfo.video_height = frame->height;
		bkhdl->fileinfo.video_fps = frame->frame_rate;
		bkhdl->fileinfo.video_codec = frame->codec_type;

		if (!bkhdl->avihdl && avfile_open(bkhdl, &bkhdl->fileinfo) != 0) 
		{
			printf("create file %s/%s failed\n", bkhdl->fileinfo.backpath, bkhdl->fileinfo.filename);
			return BACKUP_ERR_CREATE;
		}
		
		bkhdl->free_space = get_free_space_m(bkhdl->fileinfo.backpath);
	}

	if (frame->strm_type == ST_VIDEO)//video
	{
		bkhdl->vdts++;//(((int64_t)frame->time_upper) << 32) + frame->time_lower;
	} 
	else if (frame->strm_type == ST_AUDIO)//audio
	{
		bkhdl->adts++;//(((int64_t)frame->time_upper) << 32) + frame->time_lower;
	} 
	else 
	{
		printf("unknown data, ignore.\n");
		return BACKUP_ERR_NONE;
	}
	
	memcpy(&frame_buff[frame->ch][frame_buff_cnt[frame->ch]], frame, sizeof(struct reco_frame));
	if(buf_frame_data[frame->ch] == NULL)
	{
		buf_frame_data[frame->ch] = (char *)ms_malloc(sizeof(char) * (cache_size_byte + extra_size));
		if(buf_frame_data[frame->ch] == NULL) 
			return BACKUP_ERR_NOMEM;
	}
	else if((bkhdl->wsize + frame->size) > (cache_size_byte + extra_size))
	{
		buf_frame_data[frame->ch] = (char *)realloc(buf_frame_data[frame->ch], sizeof(char) * (bkhdl->wsize + frame->size + extra_size));
		if(buf_frame_data[frame->ch] == NULL)
			return BACKUP_ERR_NOMEM;
	}
	memcpy(buf_frame_data[frame->ch] + bkhdl->wsize, frame->data, frame->size);
	frame_buff_cnt[frame->ch] += 1;
	bkhdl->wsize += frame->size;
	
	if(bkhdl->wsize >= cache_size_byte || bkhdl->wsize + bkhdl->fsize >= bkhdl->size_limit 
		|| frame_buff_cnt[frame->ch] == MAX_FRAME_BUFF - 1)//memory pools
	{
#if 0 	
		if(cache_size_byte >= MAX_SIZE_PRINT)
			msprintf("=======disk_get_mount_state==============0000===cnt:%d====size:%llu==%d==", frame_buff_cnt[frame->ch], cache_size_byte, bkhdl->wsize >= cache_size_byte);
		if (disk_get_mount_state(bkhdl->fileinfo.backpath) == 0)
		{
			ms_free(buf_frame_data[frame->ch]); 
			buf_frame_data[frame->ch] = NULL;		
			return BACKUP_ERR_PATH;
		}
		if(cache_size_byte >= MAX_SIZE_PRINT)
			msprintf("=======disk_get_mount_state==============1111===chan_id:%d====limit:%d====", frame->ch, bkhdl->wsize + bkhdl->fsize >= bkhdl->size_limit);
#endif
		bkhdl->free_space = get_free_space(bkhdl->fileinfo.backpath);
		if (bkhdl->free_space < bkhdl->wsize)
		{
			ms_free(buf_frame_data[frame->ch]); 
			buf_frame_data[frame->ch] = NULL;		
			return BACKUP_ERR_NOSPACE;
		}

		int frmdata_sizecnt = 0;
		for(i = 0; i < frame_buff_cnt[frame->ch]; i++)
		{
			frame_buff[frame->ch][i].data = buf_frame_data[frame->ch] + frmdata_sizecnt;
			ms_avi_write(bkhdl->avihdl, &frame_buff[frame->ch][i]);
			frmdata_sizecnt += frame_buff[frame->ch][i].size;
		}
		ms_free(buf_frame_data[frame->ch]); 
		buf_frame_data[frame->ch] = NULL;
		frame_buff_cnt[frame->ch] = 0;

		if (bkhdl->wsize + bkhdl->fsize >= bkhdl->size_limit)
		{
			avfile_close(bkhdl);
			bkhdl->avihdl = 0;
			bkhdl->curfid++;
			avfile_open(bkhdl, &bkhdl->fileinfo);
		}
		else if (bkhdl->wsize >= cache_size_byte)
		{
			bkhdl->fsize = get_file_size_b(bkhdl->filename);
			usleep(30*1000);
		}
		bkhdl->wsize = 0;
	}
	
	return BACKUP_ERR_NONE;
}

int write_data_last(struct bk_handle *bkhdl, int chan_id)
{
	int i = chan_id,j, frmdata_sizecnt = 0;

	if(frame_buff_cnt[i] <= 0 || buf_frame_data[i] == 0)
		return -1;

	printf("=======write_data_last====chan:%d=======frame count:%d=========\n", chan_id, frame_buff_cnt[i]);
	for(j = 0; j < frame_buff_cnt[i]; j++)
	{
		frame_buff[i][j].data = buf_frame_data[i] + frmdata_sizecnt;
		ms_avi_write(bkhdl->avihdl, &frame_buff[i][j]);
		frmdata_sizecnt += frame_buff[i][j].size;
	}
		
	ms_free(buf_frame_data[i]); 
	buf_frame_data[i] = NULL;
	frame_buff_cnt[i] = 0;

	return 0;
}

int backup_finish(void *fhndl, int chan_id)
{
	if(fhndl == NULL)
		return -1;	
	struct bk_handle *bkhdl = (struct bk_handle *)fhndl;
	write_data_last(bkhdl, chan_id);
	avfile_close(bkhdl);
	ms_free(bkhdl);

	return 0;
}
