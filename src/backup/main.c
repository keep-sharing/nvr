#include "backup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/signal.h>
#include <stdint.h>
#include "msdefs.h"
#include "cel_public.h"
//#include "disk_op.h"
#include "msstd.h"
#include "msdb.h"

#define MAX_BACKUP_HANDLE 64
enum media_type{
    MEDIA_USB = 0,
    MEDIA_CD_DVD,
    MEDIA_ESATA,
};

static void *bkhandles[MAX_BACKUP_HANDLE] = {0};
static unsigned int pastresolution[MAX_BACKUP_HANDLE] = {0};
static int chnframecnt[MAX_BACKUP_HANDLE] = {0};
// unsigned int global_debug_mask = DEBUG_LEVEL_DEFAULT;
// hook_print global_debug_hook = 0;

uint64_t cache_size_byte = (1<<20);

static void manually_sync_free_cache(void)
{
	ms_system("sync");
	usleep(100*1000);
	ms_system("echo 3 > /proc/sys/vm/drop_caches");
}

static void tmstr2tm(const char *tmstr, struct tm *dsttm)
{
	int year, mon, day, hour, min, sec;
	
	sscanf(tmstr, "%d %d %d %d %d %d", &year, &mon, &day, &hour, &min, &sec);
	dsttm->tm_year = year - 1900;
	dsttm->tm_mon = mon - 1;
	dsttm->tm_mday = day;
	dsttm->tm_hour = hour;
	dsttm->tm_min = min;
	dsttm->tm_sec = sec;
}

static void finish_all_backup(void **bkhandles)
{
	int ch;
	
	for (ch = 0; ch < MAX_BACKUP_HANDLE; ch++)
	{
		backup_finish(bkhandles[ch], ch);
	}
	
	celbk_deinit();
	disk_deinit();
	cel_deinit();
	manually_sync_free_cache();
}

static void bk_stop(int signo)
{
	finish_all_backup(bkhandles);
	exit(0);
}
extern int g_debug;

static int cmd_output(const char* cmd, char* buffer, int len)
{
	if (!cmd || !buffer || len<=0)
		return -1;
	FILE* fp = popen(cmd , "r");
	if(!fp) 
		return -1;
	
	int result = fread(buffer, 1, len-1, fp);
	if (result == -1) 
		buffer[0] = 0;
	buffer[result] = 0;
	pclose(fp);	
	return 0;	
}

int main(int argc, char **argv)
{
#ifdef MSFS_DAVID
	struct bk_file_info filep;
	HDD_INFO_LIST info;
	struct tm starttm = {0};
	struct tm endtm = {0};
	struct reco_frame frame = {0};
	long start, end;
	int filesize = 0;
	int err, ch;
	int i, timer = 0;
	int chan_sleep = 0;
	int scsi_max = SCSI_N04_MAX;
	struct device_info device = {0};

	int time0 = time(0);
	if(!access(MS_CONF_DIR "/bk_debug", F_OK))
		g_debug = 1;//cel_set_debug(1);

	memset(&filep, 0, sizeof(filep));
	if (argc != 15)
	{
		fprintf(stderr, "usage: %s chnmask starttime endtime backuppath filesize\n", argv[0]);
		return BACKUP_ERR_ARGS;
	}
	
	signal(SIGINT, bk_stop);
	signal(SIGTERM, bk_stop);
	signal(SIGQUIT, bk_stop);
	
	/*mask = atoll(argv[1]);
	if (mask == 0)
	{
		printf("no channel selected.\n");
		return BACKUP_ERR_CHANNEL;
	}*/
	//david modify
	CH_MASK ch_mask;
	memset(&ch_mask, 0, sizeof(ch_mask));
	for(i = 0; i < MAX_CH_MASK; i++){
		ch_mask.mask[i] = atoi(argv[i+6]);
	}
		
	if(cellular_masks_is_empty(&ch_mask)){
		printf("no channel selected.\n");
		return BACKUP_ERR_CHANNEL;
	}
#ifdef MSFS_DAVID	
	if (!argv[3][0] || disk_get_mount_state(argv[3]) == 0)
	{
		printf("invalid path \"%s\"\n", argv[3]);
		return BACKUP_ERR_PATH;
	}
#endif	
	snprintf(filep.backpath, sizeof(filep.backpath), "%s", argv[3]);
	filesize = atoi(argv[4]);
	if (filesize > 0)
	{
		filep.filesize = filesize;
	}
	filep.hasaudio = 1;

	tmstr2tm(argv[1], &starttm);
	tmstr2tm(argv[2], &endtm);
	//david.milesight
	starttm.tm_isdst = -1;
	endtm.tm_isdst = -1;
	start = mktime(&starttm);
	end = mktime(&endtm);
	printf("===========backup=====start:%ld==end:%ld==\n", start, end);	
	if (start >= end) 
	{
		printf("endtime must be larger than starttime\n");
		return BACKUP_ERR_TIME;
	}

	db_get_device(SQLITE_FILE_NAME, &device);	
	if(device.prefix[0] == '7') 
	{
		scsi_max = SCSI_N03_MAX;
		if(atoi(argv[14]) == MEDIA_ESATA)
			cache_size_byte = (40<<20);
	}
	else if(device.prefix[0] == '8')
	{
		scsi_max = SCSI_N04_MAX;
		if(atoi(argv[14]) == MEDIA_ESATA)
			cache_size_byte = (40<<20);
	}
	else if(device.prefix[0] == '5')
	{
		if(atoi(argv[14]) == MEDIA_ESATA)
			cache_size_byte = (10<<20);
	}

	DISK_INFO pDiskInfo = {0};
	pDiskInfo.scsi_max = scsi_max;
	pDiskInfo.raid_mode = get_param_int(SQLITE_FILE_NAME, PARAM_RAID_MODE, 0);
	disk_init(NULL, NULL, (void *)&pDiskInfo);
	
	cel_init(0, NULL, NULL, 0);
	disk_get_info(&info, 0);
	cel_update_disk_info(&info);

	//david.milesight modify 20160424 
	unsigned int ichn = 0, imask = 0, ibit = 0, ihourinfo = 0;
	char phourinfo[TOTAL_MINUTES_DAY] = {0};
	for(ichn = 0; ichn < MAX_CH_MASK; ichn++)
	{
		imask = ichn/32;
		ibit = ichn%32;
		if(((unsigned int)1<<ibit) & ch_mask.mask[imask]){
			memset(phourinfo, 0, TOTAL_MINUTES_DAY);
			celpb_get_rec_mins(ichn, start, phourinfo);	
			for(ihourinfo = 0; ihourinfo < TOTAL_MINUTES_DAY; ihourinfo++)
			{
				if(phourinfo[ihourinfo] != 0)
				{		
					break;
				}
				else if(ihourinfo == TOTAL_MINUTES_DAY-1)
				{
					ch_mask.mask[imask] &= ~((unsigned int)1<<ibit);
				}
			}
		}
	}
	/////////////////////////////
	//david modify
	if (celbk_init(&ch_mask, start, end, atoi(argv[5]))) 
	{
		printf("celbk init error\n");
		disk_deinit();
		cel_deinit();
		return BACKUP_ERR_INIT;
	}

	for(i = 0; i < MAX_CAMERA; i++)
		if(cellular_chn_is_being(&ch_mask, i))
			chan_sleep++;
	if(chan_sleep <= 0)
		chan_sleep = 1;
	else if(chan_sleep > 16)
		chan_sleep = 16;
	printf("==chan_sleep:%d==\n", chan_sleep);
	unsigned int tmpreso;
	while (celbk_read_frame(&frame) == 0)
	{
		//printf("========backup=========frame time:%lld\n", frame.time_usec);
		timer++;
		if((timer % (100*chan_sleep)) == 0)
			usleep(10*1000);
		ch = frame.ch;
		chnframecnt[ch] += 1;
		tmpreso = frame.width << 16 | frame.height;
		if(frame.width <= 0 || frame.height <= 0 || frame.frame_rate <= 0)
		{
			printf("backup===widht:%d height:%d fps:%d", frame.width, frame.height, frame.frame_rate);
			continue;
		}
		if (bkhandles[ch] == NULL || (frame.strm_type == ST_VIDEO && tmpreso != pastresolution[ch]))
		{
			if(bkhandles[ch])
				backup_finish(bkhandles[ch], ch);

			pastresolution[ch] = tmpreso;
            char hostnamebuf[128] = {0};
            cmd_output("hostname", hostnamebuf, sizeof(hostnamebuf));
            hostnamebuf[strlen(hostnamebuf) - 1] = '\0';
            snprintf(filep.filename, sizeof(filep.filename), "%s-CH%02d_S%04d%02d%02d-%02d%02d_E%02d%02d-%02d%02d",
                hostnamebuf, ch+1, starttm.tm_year + 1900, starttm.tm_mon + 1, starttm.tm_mday, starttm.tm_hour, starttm.tm_min,
                endtm.tm_mon + 1, endtm.tm_mday, endtm.tm_hour, endtm.tm_min);
			printf("=backup==ch:%d=====backup_create:%s=====width:%d==height:%d=====\n", ch, filep.filename, frame.width, frame.height);
			bkhandles[ch] = backup_create(&filep);
		}

		if ((err = backup_write(bkhandles[ch], (void *)&frame))) 
		{
			perror("finish_all_backup");
			finish_all_backup(bkhandles);			
			printf("=====reco_backup_write:%d========\n", err);
			return err;
		}
	}
	
	for(i = 0;i < MAX_BACKUP_HANDLE;i ++)
	{
		if(chnframecnt[i])
			printf("channel %d total read %d frames\n", i, chnframecnt[i]);	
	}

	finish_all_backup(bkhandles);
	msprintf("==========backup finished======time:%d============", (int)(time(0)-time0));
#endif	
	return BACKUP_ERR_NONE;
}
