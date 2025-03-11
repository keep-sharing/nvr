#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cel_rec.h"
#include "cel_public.h"

#include "cel_muldec.h"
#include "cel_playback.h"

static int g_chn_max;
extern struct rec_ctrl g_rec_ctrl;
extern T_CELREC_CTRL g_ctrl;
static pthread_mutex_t global_mutex;
extern int g_cel_rec_status;

int cel_get_status()
{
    return g_cel_rec_status;
}

int cel_start_record(int chn)
{
	return cel_rec_start(chn);
}

int cel_stop_record(int chn)
{
	return cel_rec_stop(chn);
}

int cel_init(int chn_cnt, CALLBACK_EXCEPTION fn, void *arg, int maxpb)
{
	int rv = 0;

	pthread_mutex_init(&global_mutex, NULL);
	g_chn_max = chn_cnt;
	if ((rv = cel_init_rec_ctrl(chn_cnt, fn, arg)))
		return rv;
	celpb_init(chn_cnt, fn, arg, maxpb);

	return rv;
}

int cel_deinit(void)
{
	pthread_mutex_destroy(&global_mutex);
	if (cel_deinit_rec_ctrl())
		return -1;
	celpb_deinit();
	return 0;
}

int cel_create_fs(const char *mnt_path)
{
	int rv;
	if (!mnt_path)
	{
		CEL_TRACE("mnt_path is null\n");
		return -1;
	}
	pthread_mutex_lock(&global_mutex);
	rv = cel_creat(mnt_path) == CEL_OK ? 0: -1;
	pthread_mutex_unlock(&global_mutex);
	return rv;
}

int cel_check_fs(const char *mnt_path)
{
	int fd;
	char path[48] = {0};
	T_CEL_INFO ver_info = {{0}};
	snprintf(path, sizeof(path), "%s/%s", mnt_path, FMT_DONE_FLAG);
	if (!OPEN_RDONLY(fd, path)) {
		CEL_TRACE("open %s error\n", path);
		return -1;
	}
	if (!READ_ST(fd, ver_info)) {
		CEL_TRACE("read %s error\n", path);
		CLOSE_CEL(fd);
		return -1;
	}
	if (strcmp(ver_info.ver, CEL_VERSION)) {
		CLOSE_CEL(fd);
		return -1;
	}
	CLOSE_CEL(fd);
	return 0;
}

int cel_get_cur_port(void)
{
	int port = -1;
	T_CEL_MNT_INFO hdd_info;
	pthread_mutex_lock(&global_mutex);
	cel_get_hdd_info(&hdd_info);
	if (cel_is_opened())
		port = hdd_info.hdd_info[hdd_info.cur_idx].port_num;
	pthread_mutex_unlock(&global_mutex);
	return port;
}

int cel_get_rec_path(char *path)
{
	pthread_mutex_lock(&global_mutex);
	int ret = cel_get_cur_disk_path(path);
	pthread_mutex_unlock(&global_mutex);
	return ret;
}
#ifdef MSFS_DAVID
int cel_update_disk_info(struct hdd_info_list *hdd_list)
{
	pthread_mutex_lock(&global_mutex);
	cel_update_mnt_info(hdd_list);
	pthread_mutex_unlock(&global_mutex);
	return 0;
}
#endif
int cel_update_disk_health_status(int smart_res, int port)
{
	pthread_mutex_lock(&global_mutex);
	int ret = cel_update_disk_health(smart_res, port);
	pthread_mutex_unlock(&global_mutex);
	return ret;
}

void cel_set_recycle(int port, int mode)
{
	pthread_mutex_lock(&global_mutex);
	cel_set_recycle_mode(port, mode);
	pthread_mutex_unlock(&global_mutex);
}

void cel_debug(int enable)
{
	pthread_mutex_lock(&global_mutex);
	cel_set_debug(enable);
	pthread_mutex_unlock(&global_mutex);
}

int cel_write_strm(struct reco_frame *frm, int evt)
{
	int flag = 0;
	unsigned long long ts = 0;

	if (unlikely(!frm)) 
	{
	    printf("frm is null\n");
	    return -1;
	}
	if (unlikely(cel_is_closed()))
	{
	    CEL_TRACE("error: cellular is closed, ch: %d\n", frm->ch);
	    return -1;
	}

#if 0 
	if (frm->ch < 0 || frm->ch >= g_chn_max || !(((unsigned long long)1 << frm->ch) &(g_rec_ctrl.ch_mask)))
	{
	    printf("error: invalid channel number: %d\n", frm->ch);
	    return -1;
	}
#endif
	//david.milesight
	if (frm->ch < 0 || frm->ch >= g_chn_max || !cel_get_chmask_status(frm->ch))
	{
	    CEL_TRACE("error: invalid channel number: %d, g_chn_max:%d\n", frm->ch, g_chn_max);
	    return -1;
	}
	
	ts = frm->time_usec;
	if (likely(frm->strm_type == ST_VIDEO)) {
		T_VIDEO_REC_PARAM pv = {0};
		pv.ch = frm->ch;
		pv.codec_type = frm->codec_type;
		pv.frm_rate = frm->frame_rate;
		pv.frm_size = frm->size;
		pv.frm_type = frm->frame_type;
		pv.height = frm->height;
		pv.width = frm->width;
		pv.buff = (char *)(frm->data);
		pv.ts.sec = ts / 1000000;
		pv.ts.usec = ts % 1000000;
		pv.time_lower = frm->time_lower;
		pv.time_upper = frm->time_upper;
		pv.evt = evt;
		pv.strm_fmt = frm->stream_format;

		time_t time0 = 0;
		time0 = time(NULL);
		flag = cel_write_video_strm(&pv);
        if(time(NULL) - time0 > 3)
        {
            printf("======cel_write_strm::flush_reclist_data timeout:%d===========\n", (int)(time(NULL)-time0));
        }	
		
		if (likely(flag == CEL_OK))
			return 0;
		else if (unlikely(flag == CEL_ERR)) 
		{
			CEL_TRACE("record failed exception");
			T_CELREC_CTRL *celrec_ctrl = &g_ctrl;
			struct ms_exception except = {0};
			CEL_TRACE("recctrl: %p", celrec_ctrl);
			except.type = ET_RECORD_FAIL;
			cel_get_cur_path(except.disk_path, sizeof(except.disk_path));
			if (celrec_ctrl->cb_exc) 
			{
				celrec_ctrl->cb_exc(celrec_ctrl->exc_arg, &except);
			}
			if (cel_is_opened())
				cel_exit(SF_STOP, CEL_REC_CLOSE_BUF_FLUSH);
			return -1;
		} 
		else if (unlikely(flag == CEL_FULL))
		{
			T_CELREC_CTRL *celrec_ctrl = &g_ctrl;
			struct ms_exception except = {0};
			except.type = ET_HDD_FULL;
			cel_get_cur_path(except.disk_path, sizeof(except.disk_path));
			if (celrec_ctrl->cb_exc) {
				celrec_ctrl->cb_exc(celrec_ctrl->exc_arg, &except);
			}
			if (cel_is_opened())
				cel_exit(SF_STOP, CEL_REC_CLOSE_BUF_FLUSH);
			return -1;
		}
	}
	else if (unlikely(frm->strm_type == ST_AUDIO))
	{
		T_AUDIO_REC_PARAM pa = {0};
		pa.achn = frm->achn;
		pa.bps = frm->bps;
		pa.buff = (char *)(frm->data);
		pa.codec_type = frm->codec_type;
		pa.frm_size = frm->size;
		pa.smp_rate = frm->sample;
		pa.ts.sec = ts / 1000000;
		pa.ts.usec = ts % 1000000;
		pa.ch = frm->ch;
		flag = cel_write_audio_strm(&pa);
		if (likely(flag == CEL_OK))
			return 0;
		else if (unlikely(flag == CEL_ERR)) 
		{
			T_CELREC_CTRL *celrec_ctrl = &g_ctrl;
			struct ms_exception except = {0};
			except.type = ET_RECORD_FAIL;
			cel_get_cur_path(except.disk_path, sizeof(except.disk_path));
			if (celrec_ctrl->cb_exc) 
			{
				celrec_ctrl->cb_exc(celrec_ctrl->exc_arg, &except);
			}
			if (cel_is_opened())
				cel_exit(SF_STOP, CEL_REC_CLOSE_BUF_FLUSH);
			return -1;
		}
		else if (unlikely(flag == CEL_FULL))
		{
			T_CELREC_CTRL *celrec_ctrl = &g_ctrl;
			struct ms_exception except = {0};
			except.type = ET_HDD_FULL;
			cel_get_cur_path(except.disk_path, sizeof(except.disk_path));
			if (celrec_ctrl->cb_exc) {
				celrec_ctrl->cb_exc(celrec_ctrl->exc_arg, &except);
			}
			if (cel_is_opened())
				cel_exit(SF_STOP, CEL_REC_CLOSE_BUF_FLUSH);
			return -1;
		}
	}
	
	return flag;
}

int cellular_chn_to_masks(CH_MASK* ch_mask, int channel)
{
	if(channel<0 || channel >= MAX_CH_MASK*MASK_LEN)
		return -1;
	int modulus = channel/MASK_LEN;
	int remainder = channel%MASK_LEN;

	//CEL_TRACE("===david test==channel:%d, modulus:%d, remainder:%d===\n", channel, modulus, remainder);
	ch_mask->mask[modulus] |= ((unsigned int)1 << remainder);
	
	return 0;
}

int cellular_chns_to_masks(CH_MASK* ch_mask, long long channels)
{
	int i = 0;
	int modulus = 0, remainder = 0;
	
	for(i = 0; i < 64; i++){
		if(channels & ((long long)1<<i)){
			modulus = i/MASK_LEN;
			remainder = i%MASK_LEN;
			ch_mask->mask[modulus] |= ((unsigned int)1 << remainder);
		}
	}
	
	return 0;
}

int cellular_masks_to_masks(CH_MASK* des, CH_MASK* src)
{
	memcpy(des, src, sizeof(CH_MASK));
	
	return 0;
}

int cellular_chn_is_being(CH_MASK* ch_mask, int channel)
{
	if(channel<0 || channel >= MAX_CH_MASK*MASK_LEN)
		return -1;
	int modulus = channel/MASK_LEN;
	int remainder = channel%MASK_LEN;

	return (ch_mask->mask[modulus] & ((unsigned int)1 << remainder));
}

int cellular_masks_is_empty(CH_MASK* ch_mask)
{
	CH_MASK tmp_mask;
	memset(&tmp_mask, 0, sizeof(tmp_mask));
	if(!memcmp(ch_mask, &tmp_mask, sizeof(CH_MASK)))
		return -1;
	return 0;
}

int cellular_masks_clear(CH_MASK* ch_mask)
{
	memset(ch_mask, 0, sizeof(CH_MASK));
	return 0;
}


int cellular_masks_to_chn_num(CH_MASK* ch_mask)
{
	int i = 0, j = 0;
	int chnNum = 0;
	for(i = 0; i < MAX_CH_MASK; i++){
		if(ch_mask->mask[i]){
			for(j = 0; j < MASK_LEN; j++){
				if(ch_mask->mask[i] & (1<<j)) 
					chnNum = chnNum + 1;		
			}
		}
	}
	return chnNum;
}

