/**
 * @file cel_playback.c
 * @date Created on: 2013-7-10
 * @author chimmu
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/time.h>

#include "cel_playback.h"
#include "cel_public.h"

#define INVALID_SID 	0
#define FRAME_SIZE	(51200) ///< 50k
#define MAX_PB_SID	50

#define PB_DEBUG 1

static struct pb_ctrl g_celpb_ctrl;
static int g_sid_pool[MAX_PB_SID] = {0};
static int speed_to_usec[]={1000/*1x*/, 500/*2x*/, 333/*3x*/, 250/*4x*/, 125/*8x*/, 62/*16x*/, 30/*32x*/, 15/*64x*/, 7/*128x*/};

static int speed_ratio[] = {1, 2, 3, 4, 8, 16, 32, 64, 128};
static int main_sleep = 0;
static unsigned long long get_current_time_to_msec(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return ((unsigned long long)tv.tv_sec*1000 + tv.tv_usec/1000);
}

static unsigned long long get_ts_time_to_msec(T_TS *pTS)
{
	return ((unsigned long long)(pTS->sec)*1000 + pTS->usec/1000);
}

/**
 * @detail
 * -elapsed_ref_time: 即上次解码至当前帧所花的时间
 * -elapsed_play_time: 即上次解码的帧至当前帧两帧之间的时间间隔
 * -睡眠时间计算是：若帧的时间间隔大于系统解码所花时间，则需要睡眠play - ref的时间，保持解码与给帧的同步，否则无须睡眠
 */
static int calc_time_control(struct pb_session *ss, unsigned long long play_now_msec, int ch)
{
	unsigned long long s_ref_msec = ss->start_ref_time_msec[ch];
	unsigned long long s_play_msec =ss->start_play_time_msec[ch];

	unsigned long long n_ref_msec = get_current_time_to_msec();
	unsigned long long n_play_msec = play_now_msec;

	unsigned long long elapsed_ref_time;
	unsigned long long elapsed_play_time;

	if ((s_ref_msec < 0) || (s_play_msec < 0) || (n_ref_msec < 0) || (n_play_msec < 0))
		return 0;

	elapsed_ref_time  = n_ref_msec - s_ref_msec; //　上次解码至现在所流逝的系统时间
	elapsed_play_time = n_play_msec - s_play_msec; // 上次解码至当前帧的时间间隔

//	printf("playtime: %lld, reftime: %lld\n", elapsed_play_time, elapsed_ref_time);
	return elapsed_play_time - elapsed_ref_time;
}

static int calc_time_control_low_frame(struct pb_session *ss, int ratio, unsigned long long play_now_msec, int ch)
{
	unsigned long long s_ref_msec = ss->start_ref_time_msec[ch];
	unsigned long long s_play_msec = ss->start_play_time_msec[ch];

	unsigned long long n_ref_msec = get_current_time_to_msec();
	unsigned long long n_play_msec = play_now_msec;

	unsigned long long elapsed_ref_time;
	unsigned long long elapsed_play_time;

	if ((s_ref_msec < 0) || (s_play_msec < 0) || (n_ref_msec < 0) || (n_play_msec < 0))
		return 0;

	elapsed_ref_time  = n_ref_msec - s_ref_msec;
	elapsed_play_time = (n_play_msec - s_play_msec) * ratio;

	return elapsed_play_time - elapsed_ref_time;
}

/*static int calc_time_control_high_frame(struct pb_session *ss, int ratio, unsigned long long play_now_msec, int ch)
{
	unsigned long long s_ref_msec = ss->start_ref_time_msec[ch];
	unsigned long long s_play_msec = ss->start_play_time_msec[ch];

	unsigned long long n_ref_msec = get_current_time_to_msec();
	unsigned long long n_play_msec = play_now_msec;

	unsigned long long elapsed_ref_time;
	unsigned long long elapsed_play_time;

//	CEL_TRACE("sref:%lld, splay: %lld, nref:%lld, nplay:%lld",
//			s_ref_msec, s_play_msec, n_ref_msec, n_play_msec);
	if ((s_ref_msec < 0) || (s_play_msec < 0) || (n_ref_msec < 0) || (n_play_msec < 0))
		return 0;

	elapsed_ref_time  = n_ref_msec - s_ref_msec;
	if (ss->direction == PB_DIRECT_FORWARD)
		elapsed_play_time = (n_play_msec - s_play_msec) / ratio;
	else
		elapsed_play_time = (s_play_msec - n_play_msec) / ratio;
//	CEL_TRACE("play: %lld, ref: %lld, elapsed: %lld", elapsed_play_time, elapsed_ref_time, elapsed_play_time - elapsed_ref_time);
	return (elapsed_play_time - elapsed_ref_time);
}*/


static void celpb_update_time(struct pb_session *pss, T_CELMD_PARAM *cel_frm)
{
	long ch, sec, usec;
	ch = cel_frm->ch;
	//int type = (long)(cel_frm->frm_type);
	//int size = cel_frm->frm_size;
	sec	 = cel_frm->ts.sec;
	usec = cel_frm->ts.usec;

	if (0 == pss->init_pb_time[ch]) {
		pss->start_play_time[ch] = (unsigned long long)sec; //
		pss->start_play_time_msec[ch] = (unsigned long long)sec * 1000 + usec / 1000;
		pss->start_ref_time_msec[ch] = get_current_time_to_msec(); // 参考开始时间，将帧放至解码器之前
		pss->init_pb_time[ch] = 1;
	}

	if (0 == pss->prv_time_sec[ch]) {
		pss->prv_time_sec[ch] = sec;
		pss->prv_time_usec[ch] = usec;
	}

	if (pss->direction == PB_DIRECT_FORWARD) {
		if ((sec - pss->prv_time_sec[ch]) > 2 || (sec - pss->prv_time_sec[ch]) < 0) {
			pss->start_play_time[ch] = sec;
			pss->start_play_time_msec[ch] = (unsigned long long) sec * 1000 + usec / 1000;
			pss->start_ref_time_msec[ch] = get_current_time_to_msec();
		}
	} else {
		if ((pss->prv_time_sec[ch] - sec) > 2 || (pss->prv_time_sec[ch] - sec) < 0) {
			pss->start_play_time[ch] = sec;
			pss->start_play_time_msec[ch] = (unsigned long long) sec * 1000 + usec / 1000;
			pss->start_ref_time_msec[ch] = get_current_time_to_msec();
		}
	}
	pss->prv_time_sec[ch] = sec;
	pss->prv_time_usec[ch] = usec;
	pss->frame_cnt[ch]++;
}

/**
 * @brief whether to put this iframe to decoder
 */
static int iframe_put2dec(struct pb_session *pss, T_CELMD_PARAM *cel_frm, int *interval)
{
	int ch = cel_frm->ch, flag = 0;

//	CEL_TRACE("ch: %d", ch);
	if (pss->its[ch].first == 0) {
		pss->its[ch].first = ((unsigned long long) cel_frm->ts.sec * 1000000) + cel_frm->ts.usec;
		return 1;
	}
	if (pss->its[ch].second != 0) {
		pss->its[ch].first = pss->its[ch].second;
	}
	pss->its[ch].second = ((unsigned long long) cel_frm->ts.sec * 1000000) + cel_frm->ts.usec;
	if (pss->its[ch].second == pss->its[ch].first)
		return 1;
	if (pss->direction == PB_DIRECT_FORWARD)
		*interval = 1000000 / (pss->its[ch].second - pss->its[ch].first); //iframe count in one second
	else
		*interval = 1000000 / (pss->its[ch].first - pss->its[ch].second);
	if (*interval > cel_frm->frm_rate)
		*interval = cel_frm->frm_rate;
	else if (*interval <= 0)
		*interval = 1;
	if (/*pss->frame_cnt[ch] % interval == 0 ||*/ pss->frame_cnt[ch] >= *interval) {
//		pss->frame_cnt[ch] = 0;
		flag = 1;
	};
	return flag;
}

static void *celpb_task_main(void *arg)
{
	T_CELMD_PARAM cel_frm = {0};
	struct reco_frame o_frm = {0};
	struct pb_session *pss = (struct pb_session *)arg;
	int rv = 0, low_speed_ratio, sleep_msec, chncnt = 0;
	int msleep;
	int interval = 0, unput = 0, start_flag = 1;
	short put_cnt[MAX_CAMERA] = {0};
	unsigned long long last_time = 0, sleep_time = 0, delta = 0, tmp = 0, elapse_sys_time = 0; // 快进时用到， 上一轮时间戳， 这一轮时间戳
	struct timeval start = {0}, end = {0}; //for fast forward/backward

	cel_frm.vbuffer = (char *)malloc(FRAME_SIZE * sizeof(char));
	if (!cel_frm.vbuffer) {
		CEL_TRACE("malloc vbuffer error\n");
		return NULL;
	}
	cel_frm.buf_size = FRAME_SIZE;
	o_frm.data = cel_frm.vbuffer;
	pss->except.type = ET_PLAYBACK_FAIL;
	pss->except.sid = pss->sid;
	char t_name[10];

	snprintf(t_name, sizeof(t_name), "pb_%d", pss->sid);
	prctl(PR_SET_NAME, (unsigned long)t_name);
	while(1) {
		if(main_sleep)
			usleep(1000);
		pthread_mutex_lock(&pss->lock);
		switch(pss->state) {
		case PB_INVALID:
		case PB_PAUSE:
			interval = 0;
			chncnt = 0;
			last_time = 0;
			unput = 0;
			start_flag = 1;
			memset(put_cnt, 0, sizeof(put_cnt));
			usleep(5000); break;
		case PB_START:
			//CEL_TRACE("=============PB_START=======PB_START========");
			if (celmd_read_next_frame(pss->celmd_ctrl, &cel_frm) != CEL_OK) { // exception
				celmd_exget_tar_disk(pss->celmd_ctrl, pss->except.disk_path);
				CEL_TRACE("exception occurred");
				if (pss->cb_exc) {
					pss->cb_exc(pss->exc_arg, &pss->except);
				}
				pss->state = PB_INVALID;
				CEL_TRACE("read frame error\n");
				break;
			}
			if (cel_frm.strm_type == ST_VIDEO) {
				celpb_update_time(pss, &cel_frm);
			}
			celmd_cpfrm2dst(&cel_frm, &o_frm);
			if (pss->cb_dec) {
				pss->cb_dec(pss->dec_arg, &o_frm);
			}
			if (cel_frm.strm_type == ST_AUDIO)
				break;
			sleep_msec = calc_time_control(pss, get_ts_time_to_msec(&cel_frm.ts), cel_frm.ch);
			if (sleep_msec > 0 && sleep_msec < 10000) { // less than 10s
				usleep(sleep_msec * 1000);
				//printf("sleep_msec:%d\n", sleep_msec);
			}
			break;
		case PB_SPEED:
			if (pss->speed >= PB_SPEED_0_25X) { // 慢放
				if (pss->direction == PB_DIRECT_FORWARD) {
					rv = celmd_read_next_frame(pss->celmd_ctrl, &cel_frm);
				} else {
					rv = celmd_read_prev_iframe(pss->celmd_ctrl, &cel_frm);
				}

				if (rv != CEL_OK) { // exception
					celmd_exget_tar_disk(pss->celmd_ctrl, pss->except.disk_path);
					CEL_TRACE("exception occurred");
					if (pss->cb_exc) {
						pss->cb_exc(pss->exc_arg, &pss->except);
					}
					pss->state = PB_INVALID;
					CEL_TRACE("read frame error\n");
					break;
				}

				if (cel_frm.strm_type == ST_VIDEO) {
					celpb_update_time(pss, &cel_frm);
				}
				if (PB_SPEED_0_25X == pss->speed)
					low_speed_ratio = 4;
				else if (PB_SPEED_0_5X == pss->speed)
					low_speed_ratio = 2;
				else
					low_speed_ratio = 2;
				celmd_cpfrm2dst(&cel_frm, &o_frm);

				if (pss->cb_dec)
					pss->cb_dec(pss->dec_arg, &o_frm);
				sleep_msec = calc_time_control_low_frame(pss, low_speed_ratio, get_ts_time_to_msec(&cel_frm.ts), cel_frm.ch);
				if (sleep_msec > 0 && sleep_msec < 10000) { // less than 10s
					usleep(sleep_msec * 1000);
				}
			} else { // 快放
				if (start_flag) { //本轮开始，获取系统时间
					gettimeofday(&start, NULL);
					start_flag = 0;
				}
				if (unput) {
					unput = 0;
					if (pss->cb_dec)
						pss->cb_dec(pss->dec_arg, &o_frm);
				}
				if (pss->direction == PB_DIRECT_FORWARD) {
					rv = celmd_read_next_iframe(pss->celmd_ctrl, &cel_frm);
				} else {
					rv = celmd_read_prev_iframe(pss->celmd_ctrl, &cel_frm);
				}
				if (rv != CEL_OK) {
					CEL_TRACE("exception occurred");
					celmd_exget_tar_disk(pss->celmd_ctrl, pss->except.disk_path);
					if (pss->cb_exc) {
						pss->cb_exc(pss->exc_arg, &pss->except);
					}
					pss->state = PB_INVALID;
					CEL_TRACE("read frame error\n");
					break;
				}

				msleep = speed_to_usec[pss->speed];
				celpb_update_time(pss, &cel_frm); // update frame_cnt
				if (!iframe_put2dec(pss, &cel_frm, &interval))
					break;
				chncnt++;
				celmd_cpfrm2dst(&cel_frm, &o_frm);
				put_cnt[cel_frm.ch]++;
				if (last_time == 0) {
					last_time = (unsigned long long)cel_frm.ts.sec * 1000000 + cel_frm.ts.usec;
				}
				tmp = (unsigned long long)cel_frm.ts.sec * 1000000 + cel_frm.ts.usec;
				//@todo 当前帧时间戳与本次最开始参考帧的时间戳不一致
				if (put_cnt[cel_frm.ch] > 1 || abs(cel_frm.ts.sec - last_time / 1000000) > 1) { //本次同一个通道有两帧
					unput = 1;
					chncnt = 0;
					if (pss->direction == PB_DIRECT_FORWARD)
						delta = tmp - last_time;
					else
						delta = last_time - tmp;
					gettimeofday(&end, NULL);
					elapse_sys_time = (unsigned long long)end.tv_sec * 1000000 + end.tv_usec - ((unsigned long long)start.tv_sec * 1000000 + start.tv_usec);
					if (delta <= 4000000) { //最大I帧间隔30，最少帧数8，所以最少I帧情况下大约(30／8)秒会出一个I帧
						delta /= speed_ratio[pss->speed];
						//msprintf("1111 delta:%llu, elapse_sys_time:%llu", delta, elapse_sys_time);
						if(delta > elapse_sys_time)//bruce.milesight add
							delta -= elapse_sys_time;
		//				CEL_TRACE("00000sleep_time: %lld", sleep_time);
						//msprintf("1111 delta:%llu", delta);
						if (delta > 0)
						{
							//msprintf("usleep:%llu", delta);
							usleep(delta);
						}
					} else {
						//msprintf("2222 delta:%llu", elapse_sys_time);
						if(((unsigned long long)msleep * 1000) > elapse_sys_time)//bruce.milesight add
							sleep_time = (unsigned long long)msleep * 1000 - elapse_sys_time;
		//				CEL_TRACE("11111sleep_time: %lld", sleep_time);
						if (sleep_time > 0)
						{
							//msprintf("usleep:%llu", sleep_time);
							usleep(sleep_time);
						}
					}
					last_time = tmp;
					start_flag = 1;
					memset(pss->frame_cnt, 0, sizeof(int) * g_celpb_ctrl.chn_cnt);
					memset(put_cnt, 0, sizeof(put_cnt));
					//msprintf("============9999================delta:%llu", delta);
					break;
				}
//				CEL_TRACE("ts sec: %ld, chncnt: %d, interval: %d, ch: %d", cel_frm.ts.sec, chncnt, interval, cel_frm.ch);
				if (pss->cb_dec) {
					pss->cb_dec(pss->dec_arg, &o_frm);
				}
				if (chncnt >= pss->chncnt /*|| cel_frm.ts.sec - last_time > 1*/) {//已读到所有要回放的通道
					chncnt = 0;
					last_time = tmp;
					start_flag = 1;
					gettimeofday(&end, NULL);
					elapse_sys_time = (unsigned long long)end.tv_sec * 1000000 + end.tv_usec - ((unsigned long long)start.tv_sec * 1000000 + start.tv_usec);
					if(((unsigned long long)msleep * 1000) > elapse_sys_time)//bruce.milesight add
						sleep_time = (unsigned long long)msleep * 1000 - elapse_sys_time;
//					CEL_TRACE("222222sleep_time: %lld", sleep_time);
					if (sleep_time > 0)
						usleep(sleep_time);
					memset(pss->frame_cnt, 0, sizeof(int) * g_celpb_ctrl.chn_cnt);
					memset(put_cnt, 0, sizeof(put_cnt));
				}
			}

			break;
		case PB_STEP:
			if (pss->direction == PB_DIRECT_FORWARD) {
				CEL_TRACE("read next");
				rv = celmd_read_next_frame(pss->celmd_ctrl, &cel_frm);
			}
			else {
				rv = celmd_read_prev_iframe(pss->celmd_ctrl, &cel_frm);
				CEL_TRACE("read prev");
			}
			if (rv != CEL_OK) { // exception
				CEL_TRACE("exception occurred");
				celmd_exget_tar_disk(pss->celmd_ctrl, pss->except.disk_path);
				if (pss->cb_exc) {
					pss->cb_exc(pss->exc_arg, &pss->except);
				}
				pss->state = PB_INVALID;
				CEL_TRACE("read frame error\n");
				break;
			}
			if (cel_frm.strm_type == ST_AUDIO)
				break;
			celpb_update_time(pss, &cel_frm);
			//david modify for one frame 20170517
			//if (!iframe_put2dec(pss, &cel_frm, &interval)){
			//	CEL_TRACE("[david debug] iframe_put2dec frame cnt:%d interval:%d 12333333333333\n", pss->frame_cnt[cel_frm.ch], interval);
			//	break;
			//}
			celmd_cpfrm2dst(&cel_frm, &o_frm);
			if (pss->cb_dec){ 
				CEL_TRACE("[david debug] chn:%d width:%d height:%d stream_from:%d strm_type:%d IFRAME:%d\n", o_frm.ch, o_frm.width, o_frm.height, o_frm.stream_from, o_frm.strm_type, o_frm.frame_type==FT_IFRAME?1:0);
				pss->cb_dec(pss->dec_arg, &o_frm);
			}
			++chncnt;
            // milesight.bruce
			// if (chncnt >= pss->chncnt || pss->frame_cnt[cel_frm.ch] > interval) {
			if (chncnt >= pss->chncnt) {
				CEL_TRACE("cnt: %d, total: %d, interval: %d, pss->frame_cnt[%ld]: %d", chncnt, pss->chncnt, interval, cel_frm.ch, pss->frame_cnt[cel_frm.ch]);
				interval = 0;
				memset(pss->frame_cnt, 0, g_celpb_ctrl.chn_cnt * sizeof(int));
				chncnt = 0;
				pss->state = PB_PAUSE;
			}

			break;

		case PB_SINGLE_FRAME:
			if (celmd_read_next_frame(pss->celmd_ctrl, &cel_frm) != CEL_OK) { // exception
				celmd_exget_tar_disk(pss->celmd_ctrl, pss->except.disk_path);
				CEL_TRACE("exception occurred");
				if (pss->cb_exc) {
					pss->cb_exc(pss->exc_arg, &pss->except);
				}
				pss->state = PB_INVALID;
				CEL_TRACE("read frame error\n");
				break;
			}
			if (cel_frm.strm_type == ST_AUDIO)
				break;
			if (pss->cb_dec) {
				pss->cb_dec(pss->dec_arg, &o_frm);
			}
			pss->state = PB_INVALID;
			break;
        case PB_PAUSE_SNAPSHOT://bruce.milesight add
		{
			int ch, max_try = 0, nCnt = 0, nmask = 0;
			CH_MASK ch_mask;
			memset(&ch_mask, 0, sizeof(CH_MASK));
			
			for (ch = 0; ch < CEL_MAX_CHANNEL && max_try < 10; ch++) 
			{
				/*if ((pss->ch_mask & ((unsigned long long)1 << ch)) == 0)
					continue;
				if(ch_mask == pss->ch_mask)
					break;

				int temp_mask = pss->celmd_ctrl->ch_mask;
				pss->celmd_ctrl->ch_mask = ((unsigned long long)1 << ch);
				celmd_read_prev_iframe(pss->celmd_ctrl, &cel_frm);				
				rv = celmd_read_next_iframe(pss->celmd_ctrl, &cel_frm);
				pss->celmd_ctrl->ch_mask = temp_mask;
				
				CEL_TRACE("============= mask:%lld frm_ch:%ld\n", pss->ch_mask, cel_frm.ch);
				if(!ch_mask)
					ch_mask = (1 <<cel_frm.ch);
				else if((ch_mask & (unsigned long long)(1 << cel_frm.ch)) == (unsigned long long)(1 << cel_frm.ch))
				{
					CEL_TRACE("========ch:%d ch_mask:%lld frm_ch:%ld", ch, ch_mask, cel_frm.ch);
					ch--;	
					max_try++;
					continue;
				}
				ch_mask |= (1 << cel_frm.ch);// 
				*/
				
				//david.milesight
				if(!ms_celmd_get_chmask_status(&pss->ch_mask,  ch))
					continue;
				
				for(nCnt=0, nmask = 0; nCnt < MAX_CH_MASK; nCnt++){
					if(ch_mask.mask[nCnt] == pss->ch_mask.mask[nCnt]){
						if(nCnt == MAX_CH_MASK-1)
							nmask = 1;
					}else{
						break;
					}
				}
				if(nmask)
					break;
				
				CH_MASK temp_mask;
				ms_celmd_masks_copy(&pss->celmd_ctrl->ch_mask, &temp_mask);
				ms_celmd_set_masks(&pss->celmd_ctrl->ch_mask, ch);
				celmd_read_prev_iframe(pss->celmd_ctrl, &cel_frm);				
				rv = celmd_read_next_iframe(pss->celmd_ctrl, &cel_frm);
				ms_celmd_masks_copy(&temp_mask, &pss->celmd_ctrl->ch_mask);
				if(ms_celmd_masks_is_empty(&ch_mask)){
					ms_celmd_set_masks(&ch_mask, cel_frm.ch);
				}else if(!ms_celmd_chn_cmp_masks(&ch_mask, cel_frm.ch)){
					ch--;	
					max_try++;
					continue;
				}
				
				ms_celmd_chn_to_masks(&ch_mask, cel_frm.ch);
				
				if (rv != CEL_OK) { // exception
					CEL_TRACE("exception occurred");
					celmd_exget_tar_disk(pss->celmd_ctrl, pss->except.disk_path);
					if (pss->cb_exc) {
						pss->cb_exc(pss->exc_arg, &pss->except);
					}
					pss->state = PB_INVALID;
					CEL_TRACE("read frame error\n");
					break;
				}
				celpb_update_time(pss, &cel_frm);
				if (!iframe_put2dec(pss, &cel_frm, &interval))
				{
					CEL_TRACE("iframe_put2dec error\n");
					break;
				}
				celmd_cpfrm2dst(&cel_frm, &o_frm);
				if (pss->cb_dec)
				{
					CEL_TRACE("cb_dec \n");
					pss->cb_dec(pss->dec_arg, &o_frm);
				}
			}
			pss->state = PB_PAUSE;
			//printf("=====callback_recv_pbdata pause===start_flag:%d====\n", start_flag);
			break;
        }
		}
		if (pss->stop) {
			CEL_TRACE("quit thread\n");
			ms_free(cel_frm.vbuffer);
			pthread_mutex_unlock(&pss->lock);
			return NULL;
		}
		pthread_mutex_unlock(&pss->lock);
	}
	return NULL;
}

static int get_free_sid(void)
{
	int i, sTemp = 0;
#if 1	
	for (i = 0; i < MAX_PB_SID; i++) {
		if (!g_sid_pool[i]) {
			g_sid_pool[i] = 1;
			return i+1;
		}
	}
#else	
	for (i = 0; i < MAX_PB_SID; i++) {
		if (g_sid_pool[i]) {
			if(sTemp < i)
				sTemp = i;
		}
	}
	
	for (i = sTemp; i < MAX_PB_SID; i++) {
		if (!g_sid_pool[i]) {
			g_sid_pool[i] = 1;
			return i+1;
		}
	}

	for (i = 0; i < MAX_PB_SID - sTemp; i++) {
		if (!g_sid_pool[i]) {
			g_sid_pool[i] = 1;
			return i+1;
		}
	}
#endif		
	return -1;
}

static int free_sid(int sid)
{
	if (sid <= 0 || sid > MAX_PB_SID) {
		return -1;
	}
	g_sid_pool[sid - 1] = 0;
	return 0;
}

/////////////////////////////linklist operation start//////////////////////////////////////////
static struct pb_session *search_pb_session(int sid)
{
	struct pb_ctrl *pb_ctrl = &g_celpb_ctrl;
//	struct pb_session *tmp_session = pb_ctrl->head;
//	int i;
//	pthread_mutex_lock(&pb_ctrl->mutex);
//	for (i = 0; i < pb_ctrl->ss_cnt; i++) {
//		if (tmp_session->sid != sid) {
//			tmp_session = tmp_session->next;
//			continue;
//		}
//		pthread_mutex_unlock(&pb_ctrl->mutex);
//		return tmp_session;
//	}
//	CEL_TRACE("search sid: %d", sid);
	struct pb_session *cur = NULL, *tmp = NULL;
	list_for_each_entry_safe(cur, tmp, &pb_ctrl->head, list) {
		if (cur && cur->sid == sid) {
//			CEL_TRACE("got it, cur->sid: %d, sid: %d",cur->sid, sid);
//			pthread_mutex_unlock(&pb_ctrl->mutex);
			return cur;
		}
	}
//	pthread_mutex_unlock(&pb_ctrl->mutex);
	return NULL;
}

static int celpb_add_session(struct pb_session **pps)
{
	struct pb_ctrl *pb_ctrl = &g_celpb_ctrl;
	struct pb_session *new_session = NULL;
	
	CEL_TRACE("===david test===cur sid:%d,max sid:%d===20151020===\n", pb_ctrl->ss_cnt,pb_ctrl->maxpb);
	
	pthread_mutex_lock(&g_celpb_ctrl.mutex);
	if (pb_ctrl->ss_cnt >= pb_ctrl->maxpb)
		goto EXIT;
	int sid = get_free_sid();
	if (sid == -1) {
		goto EXIT;
	}

	new_session = (struct pb_session *)ms_calloc(1, sizeof(struct pb_session));
	if (!new_session) {
		CEL_TRACE("ms_calloc session failed\n");
//		free_sid(sid);
		goto EXIT;
	}
#ifdef HDR_ENABLE
	new_session->celmd_ctrl = celmd_ctor(pb_ctrl->chn_cnt);
	if (!new_session->celmd_ctrl) {
		CEL_TRACE("ms_calloc session ctrl");
		goto EXIT;
	}
#else
	new_session->celmd_ctrl = (T_CELMD_CTRL *)ms_calloc(1, sizeof(T_CELMD_CTRL));
	if (!new_session->celmd_ctrl) {
		CEL_TRACE("ms_calloc celmd_ctrl failed\n");
		goto EXIT;
	}
	new_session->celmd_ctrl->chns_sec = (long *)ms_calloc(pb_ctrl->chn_cnt, sizeof(long));
	if (!new_session->celmd_ctrl->chns_sec) {
		CEL_TRACE("ms_calloc chns_sec error");
		goto EXIT;
	}
#endif
	if (!(new_session->init_pb_time = (unsigned long long *)ms_calloc(pb_ctrl->chn_cnt, sizeof(unsigned long long)))) {
		CEL_TRACE("ms_calloc init_pb_time error");
		goto EXIT;
	}
	if (!(new_session->prv_time_sec = (unsigned long long *)ms_calloc(pb_ctrl->chn_cnt, sizeof(unsigned long long)))) {
		CEL_TRACE("ms_calloc prv_time_sec error");
		goto EXIT;
	}
	if (!(new_session->prv_time_usec = (unsigned long long *)ms_calloc(pb_ctrl->chn_cnt, sizeof(unsigned long long)))) {
		CEL_TRACE("ms_calloc prv_time_usec error");
		goto EXIT;
	}
	if (!(new_session->start_play_time = (unsigned long long *)ms_calloc(pb_ctrl->chn_cnt, sizeof(unsigned long long)))) {
		CEL_TRACE("ms_calloc start_play_time error");
		goto EXIT;
	}
	if (!(new_session->start_play_time_msec = (unsigned long long *)ms_calloc(pb_ctrl->chn_cnt, sizeof(unsigned long long)))) {
		CEL_TRACE("ms_calloc init_pb_time error");
		goto EXIT;
	}
	if (!(new_session->start_ref_time_msec = (unsigned long long *)ms_calloc(pb_ctrl->chn_cnt, sizeof(unsigned long long)))) {
		CEL_TRACE("ms_calloc start_ref_time_msec error");
		goto EXIT;
	}
	if (!(new_session->start_time = (unsigned long long *)ms_calloc(pb_ctrl->chn_cnt, sizeof(unsigned long long)))) {
		CEL_TRACE("ms_calloc start_time error");
		goto EXIT;
	}
	if (!(new_session->its = (I_TS *)ms_calloc(pb_ctrl->chn_cnt, sizeof(I_TS)))) {
		CEL_TRACE("ms_calloc its");
		goto EXIT;
	}
	if (!(new_session->frame_cnt = (int *)ms_calloc(pb_ctrl->chn_cnt, sizeof(int)))) {
		CEL_TRACE("ms_calloc frame cnt");
		goto EXIT;
	}
	pthread_mutex_init(&new_session->lock, NULL);
//	if (pb_ctrl->ss_cnt) {
//		new_session->prev = pb_ctrl->tail;
//		pb_ctrl->tail->next = new_session;
//		pb_ctrl->tail = pb_ctrl->tail->next;
//	} else {
//		pb_ctrl->head = pb_ctrl->tail = new_session;
//	}
//	pb_ctrl->tail->sid = sid;
	new_session->sid = sid;
	*pps = new_session;
	list_add_tail(&new_session->list, &pb_ctrl->head);
	pb_ctrl->ss_cnt++;
	pthread_mutex_unlock(&g_celpb_ctrl.mutex);
	return sid;
EXIT:
	if (sid != -1)
		free_sid(sid);
	if (new_session) {
		CEL_TRACE("delete new_session\n");
		if (new_session->init_pb_time)
			ms_free(new_session->init_pb_time);
		if (new_session->prv_time_sec)
			ms_free(new_session->prv_time_sec);
		if (new_session->prv_time_usec)
			ms_free(new_session->prv_time_usec);
		if (new_session->start_play_time)
			ms_free(new_session->start_play_time);
		if (new_session->start_play_time_msec)
			ms_free(new_session->start_play_time_msec);
		if (new_session->start_ref_time_msec)
			ms_free(new_session->start_ref_time_msec);
		if (new_session->start_time)
			ms_free(new_session->start_time);
		if (new_session->its)
			ms_free(new_session->its);
		if (new_session->frame_cnt)
			ms_free(new_session->frame_cnt);
		if (new_session->celmd_ctrl) {
#ifdef HDR_ENABLE
			celmd_dtor(&new_session->celmd_ctrl);
#else
			if (new_session->celmd_ctrl->chns_sec)
				ms_free(new_session->celmd_ctrl->chns_sec);
			ms_free(new_session->celmd_ctrl);
#endif
		}
		ms_free(new_session);
	}
	pthread_mutex_unlock(&g_celpb_ctrl.mutex);
	return -1;
}

static void celpb_free(struct pb_session *ss)
{
	if (!ss)
		return;
	pthread_mutex_lock(&ss->lock);
	if (ss->init_pb_time)
		ms_free(ss->init_pb_time);
	if (ss->prv_time_sec)
		ms_free(ss->prv_time_sec);
	if (ss->prv_time_usec)
		ms_free(ss->prv_time_usec);
	if (ss->start_play_time)
		ms_free(ss->start_play_time);
	if (ss->start_play_time_msec)
		ms_free(ss->start_play_time_msec);
	if (ss->start_ref_time_msec)
		ms_free(ss->start_ref_time_msec);
	if (ss->start_time)
		ms_free(ss->start_time);
	if (ss->its)
		ms_free(ss->its);
	if (ss->frame_cnt)
		ms_free(ss->frame_cnt);
	if (ss->celmd_ctrl) {
#ifdef HDR_ENABLE
		celmd_dtor(&ss->celmd_ctrl);
#else
		if (ss->celmd_ctrl->chns_sec)
			ms_free(ss->celmd_ctrl->chns_sec);
		ms_free(ss->celmd_ctrl);
#endif
	}
	pthread_mutex_unlock(&ss->lock);
	pthread_mutex_destroy(&ss->lock);
	ms_free(ss);
}

static int celpb_del_session(int sid)
{
	struct pb_ctrl *pb_ctrl = &g_celpb_ctrl;
	//struct pb_session *ss, *tmp_ss;
	struct pb_session *ss = search_pb_session(sid);

	if (!ss)
		return -1;
//	pthread_mutex_lock(&g_celpb_ctrl.mutex);
//	if (pb_ctrl->ss_cnt == 1) {
//		celpb_free(ss);
//		pb_ctrl->head = pb_ctrl->tail = NULL;
//	} else {
//		if(ss->prev == NULL) {///< head
//			tmp_ss = ss->next;
//			celpb_free(ss);
//			tmp_ss->prev = NULL;
//			pb_ctrl->head = tmp_ss;
//		} else if (ss->next == NULL) {///< tail
//			tmp_ss = ss->prev;
//			celpb_free(ss);
//			tmp_ss->next = NULL;
//			pb_ctrl->tail = tmp_ss;
//		} else {
//			tmp_ss = ss->next;
//			ss->prev->next = tmp_ss;
//			tmp_ss->prev = ss->prev;
//			celpb_free(ss);
//		}
//	}
	list_del(&ss->list);
	celpb_free(ss);
	free_sid(sid);
	pb_ctrl->ss_cnt--;
//	pthread_mutex_unlock(&g_celpb_ctrl.mutex);
	CEL_TRACE("delete session done, sid: %d, cnt: %d\n", sid, pb_ctrl->ss_cnt);
	return 0;

}
/////////////////////////////linklist operation end////////////////////////////////////////////

int celpb_init(int chn_cnt, CALLBACK_EXCEPTION fn, void *arg, int maxpb)
{
	memset(&g_celpb_ctrl, 0, sizeof(g_celpb_ctrl));
	g_celpb_ctrl.cb_exc = fn;
	g_celpb_ctrl.exc_arg = arg;
	g_celpb_ctrl.chn_cnt = chn_cnt;
	g_celpb_ctrl.maxpb = maxpb;
	pthread_mutex_init(&g_celpb_ctrl.mutex, NULL);
	INIT_LIST_HEAD(&g_celpb_ctrl.head);
	return 0;
}

int celpb_deinit(void)
{
	int ss_cnt;

	ss_cnt = g_celpb_ctrl.ss_cnt;
	CEL_TRACE("session count: %d\n", ss_cnt);
	pthread_mutex_lock(&g_celpb_ctrl.mutex);
	struct pb_session *cur = NULL, *tmp = NULL;
	list_for_each_entry_safe(cur, tmp, &g_celpb_ctrl.head, list) 
	{
		if (cur)
			celpb_stop(cur->sid);
	}
	pthread_mutex_unlock(&g_celpb_ctrl.mutex);
	pthread_mutex_destroy(&g_celpb_ctrl.mutex);
	memset(&g_celpb_ctrl, 0, sizeof(g_celpb_ctrl));
	return 0;
}

int celpb_start(CH_MASK* ch_mask, long sec, CALLBACK_DEC fn, void *arg, int *sid)
{
	int tmp_sid = 0;
	int tmp_sec = 0;
	int i;
	char mntpath[64] = {0};
	T_CELMD_CTRL *celmd_ctrl;
	struct pb_session *pss;

	if (!sid || *sid >= 1) {
		return -1;
	}
	if ((tmp_sid = celpb_add_session(&pss)) == -1) {
		CEL_TRACE("add session error\n");
		return -1;
	}
	if ((tmp_sec = (celmd_get_first_rec_time(sec, ch_mask, mntpath))) == CEL_ERR) {
		celpb_del_session(tmp_sid);
		CEL_TRACE("get start time error\n");
		return -1;
	}
//	celmd_ctrl = g_celpb_ctrl.tail->celmd_ctrl;
//	pss = g_celpb_ctrl.tail;
	celmd_ctrl = pss->celmd_ctrl;
	if (celmd_set_ctrl_disk(mntpath, celmd_ctrl) != CEL_OK) {
		celpb_del_session(tmp_sid);
		CEL_TRACE("get tar disk error\n");
		return -1;
	}
	/****防止open返回值从0开始****/
	celmd_ctrl->hcel = -1;
	celmd_ctrl->hidx = -1;
	if (celmd_open(celmd_ctrl, ch_mask, tmp_sec, CEL_FORWARD, g_celpb_ctrl.maxpb) != CEL_OK) {
		celpb_del_session(tmp_sid);
		CEL_TRACE("open error\n");
		return -1;
	}
	CEL_TRACE("open cid: %ld\n",celmd_ctrl->cid);
	*sid = pss->sid;
	pss->state = PB_START;
	
	//pss->ch_mask = ch_mask;
	memset(&pss->ch_mask, 0, sizeof(CH_MASK));
	int nCnt = 0;
	for(nCnt = 0; nCnt < MAX_CH_MASK; nCnt++){
		pss->ch_mask.mask[nCnt] = ch_mask->mask[nCnt];
	}
	
	pss->cb_dec = fn;
	pss->dec_arg = arg;
	if (pthread_create(&pss->tid, NULL, celpb_task_main, pss)) {
		celmd_close(pss->celmd_ctrl, TRUE);
		celpb_del_session(tmp_sid);
		return -1;
	}
	pss->cb_exc = g_celpb_ctrl.cb_exc;
	pss->exc_arg = g_celpb_ctrl.exc_arg;
	pss->except.arg = arg;
	for (i = 0; i < g_celpb_ctrl.maxpb; i++) {
		//if (ch_mask & ((unsigned long long)1 << i)) {
		//	pss->chncnt++;
		//}
		//david.milesight
		if(ms_celmd_get_chmask_status(ch_mask, i))
			pss->chncnt++;
	}
	CEL_TRACE("open cid: %ld, sid: %d, channels: %d \n",
			celmd_ctrl->cid, pss->sid, pss->chncnt);
	return 0;
}

int celpb_stop(int sid)
{
	CEL_TRACE("**********stop sid: %d*******", sid);

	pthread_mutex_lock(&g_celpb_ctrl.mutex);
	struct pb_session *pss = search_pb_session(sid);
	if (!pss) {
		CEL_TRACE("can't find pss, sid: %d", sid);
		pthread_mutex_unlock(&g_celpb_ctrl.mutex);
		return -1;
	}
	pss->stop = 1;//bruce.milesight add
	main_sleep = 1;
	pthread_mutex_lock(&pss->lock);
	//CEL_TRACE("**********stop sid: %d****6666***", sid);
	celmd_close(pss->celmd_ctrl, TRUE);
	//CEL_TRACE("**********stop sid: %d****7777***", sid);
	pss->state = PB_INVALID;
	pss->stop = 1;
	pthread_mutex_unlock(&pss->lock);
	main_sleep = 0;
	pthread_join(pss->tid, NULL);
	//CEL_TRACE("**********stop sid: %d****8888***", sid);
	celpb_del_session(sid);	
	//CEL_TRACE("**********stop sid: %d****9999***", sid);
	pthread_mutex_unlock(&g_celpb_ctrl.mutex);
	return 0;
}

int celpb_pause(int sid)
{
	CEL_TRACE("sid: %d\n", sid);
	pthread_mutex_lock(&g_celpb_ctrl.mutex);
	struct pb_session *pss = search_pb_session(sid);
	if (!pss) {
		pthread_mutex_unlock(&g_celpb_ctrl.mutex);
		return -1;
	}
	main_sleep = 1;
	pthread_mutex_lock(&pss->lock);
	pthread_mutex_unlock(&g_celpb_ctrl.mutex);
	pss->prev_state = pss->state;
	pss->state = PB_PAUSE;
//	CEL_TRACE("pause cid: %ld\n", pss->celmd_ctrl->cid);
//	memset(pss->now_play_time, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->prv_time_sec, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->prv_time_usec, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->start_play_time, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->start_play_time_msec, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->start_ref_time_msec, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->init_pb_time, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->start_time, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->frame_cnt, 0, sizeof(int) * g_celpb_ctrl.chn_cnt);
	memset(pss->its, 0, sizeof(I_TS) * g_celpb_ctrl.chn_cnt);
	memset(pss->celmd_ctrl->chns_sec, 0, sizeof(long) * g_celpb_ctrl.chn_cnt);
	pthread_mutex_unlock(&pss->lock);
	main_sleep = 0;
	return 0;
}

int celpb_step(int sid, int direct)
{
	pthread_mutex_lock(&g_celpb_ctrl.mutex);
	struct pb_session *pss = search_pb_session(sid);
	if (!pss) {
		CEL_TRACE("invalid sid: %d", sid);
		pthread_mutex_unlock(&g_celpb_ctrl.mutex);
		return -1;
	}
	main_sleep = 1;
	pthread_mutex_lock(&pss->lock);
	pthread_mutex_unlock(&g_celpb_ctrl.mutex);
	pss->state = PB_STEP;
	pss->direction = direct;
	CEL_TRACE("step direct: %d", direct);
#if 0
	memset(pss->now_play_time, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->prv_time_sec, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->prv_time_usec, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->start_play_time, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->start_play_time_msec, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->start_ref_time_msec, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->init_pb_time, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->start_time, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->frame_cnt, 0, sizeof(int) * g_celpb_ctrl.chn_cnt);
	memset(pss->its, 0, sizeof(I_TS) * g_celpb_ctrl.chn_cnt);
#endif
	pthread_mutex_unlock(&pss->lock);
	main_sleep = 0;

	return 0;
}

int celpb_resume(int sid)
{
	CEL_TRACE("sid: %d\n", sid);
	pthread_mutex_lock(&g_celpb_ctrl.mutex);
	struct pb_session *pss = search_pb_session(sid);
	if (!pss) {
		CEL_TRACE("invalid sid: %d", sid);
		pthread_mutex_unlock(&g_celpb_ctrl.mutex);
		return -1;
	}
	main_sleep = 1;
	pthread_mutex_lock(&pss->lock);
	pthread_mutex_unlock(&g_celpb_ctrl.mutex);
	pss->state = pss->prev_state;
	// if (pss->state == PB_STOP || pss->state == PB_INVALID || pss->state == PB_PAUSE || pss->state == PB_STEP) {
		pss->state = PB_START;
	// } // bruce.milesight modify
//	memset(pss->now_play_time, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->prv_time_sec, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->prv_time_usec, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->start_play_time, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->start_play_time_msec, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->start_ref_time_msec, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->init_pb_time, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->start_time, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->frame_cnt, 0, sizeof(int) * g_celpb_ctrl.chn_cnt);
	memset(pss->its, 0, sizeof(I_TS) * g_celpb_ctrl.chn_cnt);
	memset(pss->celmd_ctrl->chns_sec, 0, sizeof(long) * g_celpb_ctrl.chn_cnt);
	CEL_TRACE("sid: %d, speed: %d\n", pss->sid, pss->speed);
	pthread_mutex_unlock(&pss->lock);
	main_sleep = 0;
	return 0;
}

int celpb_change_speed(int sid, int speed, int direct)
{
	CEL_TRACE("sid: %d, spend:%d, direct:%d\n", sid, speed, direct);
	pthread_mutex_lock(&g_celpb_ctrl.mutex);
	struct pb_session *pss = search_pb_session(sid);
	if (!pss) {
		CEL_TRACE("invalid sid: %d", sid);
		pthread_mutex_unlock(&g_celpb_ctrl.mutex);
		return -1;
	}
	main_sleep = 1;
	pthread_mutex_lock(&pss->lock);
	pthread_mutex_unlock(&g_celpb_ctrl.mutex);
	if (speed < PB_SPEED_1X)
		pss->speed = PB_SPEED_1X;
	else if(speed > PB_SPEED_0_5X)
		speed = PB_SPEED_0_5X;
	else
		pss->speed = speed;
	pss->direction = direct;
	pss->state = PB_SPEED;
	if (pss->speed == PB_SPEED_1X && direct == PB_DIRECT_FORWARD) {
		CEL_TRACE("1x speed, change state to start");
		pss->state = PB_START;
	}
//	memset(pss->now_play_time, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->prv_time_sec, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->prv_time_usec, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->start_play_time, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->start_play_time_msec, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->start_ref_time_msec, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->init_pb_time, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->start_time, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->frame_cnt, 0, sizeof(int) * g_celpb_ctrl.chn_cnt);
	memset(pss->its, 0, sizeof(I_TS) * g_celpb_ctrl.chn_cnt);
	memset(pss->celmd_ctrl->chns_sec, 0, sizeof(long) * g_celpb_ctrl.chn_cnt);
	CEL_TRACE("speed: %d, direct: %d, sid: %d, pss->sid: %d", pss->speed, pss->direction, sid, pss->sid);
	pthread_mutex_unlock(&pss->lock);
	main_sleep = 0;
	return 0;
}

int celpb_jump(int sid, long sec)
{
	CEL_TRACE("sid: %d\n", sid);
	pthread_mutex_lock(&g_celpb_ctrl.mutex);
	struct pb_session *pss = search_pb_session(sid);
	long tmp_sec = 0;
	char mntpath[64] = {0};

	if (!pss) {
		pthread_mutex_unlock(&g_celpb_ctrl.mutex);
		return -1;
	}
	main_sleep = 1;
	pthread_mutex_lock(&pss->lock);
	pthread_mutex_unlock(&g_celpb_ctrl.mutex);
	if ((tmp_sec = celmd_get_first_rec_time(sec, &pss->ch_mask, mntpath)) == CEL_ERR) {
		CEL_TRACE("get first rec time error\n");
		pthread_mutex_unlock(&pss->lock);
		main_sleep = 0;
		return -1;
	}
	if (celmd_set_ctrl_disk(mntpath, pss->celmd_ctrl) == CEL_ERR) {
		CEL_TRACE("get target disk error\n");
		pthread_mutex_unlock(&pss->lock);
		main_sleep = 0;
		return -1;
	}
	celmd_close(pss->celmd_ctrl, TRUE);
	if (celmd_open(pss->celmd_ctrl, &pss->ch_mask, tmp_sec, pss->direction, g_celpb_ctrl.maxpb) == CEL_ERR) {
		CEL_TRACE("open error\n");
		pthread_mutex_unlock(&pss->lock);
		main_sleep = 0;
		return -1;
	}
//	memset(pss->now_play_time, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->prv_time_sec, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->prv_time_usec, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->start_play_time, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->start_play_time_msec, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->start_ref_time_msec, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->init_pb_time, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->start_time, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->its, 0, sizeof(I_TS) * g_celpb_ctrl.chn_cnt);
	memset(pss->frame_cnt, 0, sizeof(int) * g_celpb_ctrl.chn_cnt);
	memset(pss->celmd_ctrl->chns_sec, 0, sizeof(long) * g_celpb_ctrl.chn_cnt);
	pthread_mutex_unlock(&pss->lock);
	main_sleep = 0;
	
	return 0;
}

int celpb_single_jump(int sid, long sec)
{
	CEL_TRACE("sid: %d\n", sid);
	pthread_mutex_lock(&g_celpb_ctrl.mutex);
	struct pb_session *pss = search_pb_session(sid);
	long tmp_sec = 0;
	char mntpath[64] = {0};

	if (!pss) {
		pthread_mutex_unlock(&g_celpb_ctrl.mutex);
		return -1;
	}
	main_sleep = 1;
	pthread_mutex_lock(&pss->lock);
	pthread_mutex_unlock(&g_celpb_ctrl.mutex);
	if ((tmp_sec = celmd_get_first_rec_time(sec, &pss->ch_mask, mntpath)) == CEL_ERR) {
		CEL_TRACE("get first rec time error\n");
		pthread_mutex_unlock(&pss->lock);
		main_sleep = 0;
		return -1;
	}
	if (celmd_set_ctrl_disk(mntpath, pss->celmd_ctrl) == CEL_ERR) {
		CEL_TRACE("get target disk error\n");
		pthread_mutex_unlock(&pss->lock);
		main_sleep = 0;
		return -1;
	}
	celmd_close(pss->celmd_ctrl, TRUE);
	if (celmd_open(pss->celmd_ctrl, &pss->ch_mask, tmp_sec, pss->direction, g_celpb_ctrl.maxpb) == CEL_ERR) {
		CEL_TRACE("open error\n");
		pthread_mutex_unlock(&pss->lock);
		main_sleep = 0;
		return -1;
	}
//	memset(pss->now_play_time, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->prv_time_sec, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->prv_time_usec, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->start_play_time, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->start_play_time_msec, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->start_ref_time_msec, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->init_pb_time, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->start_time, 0, sizeof(unsigned long long) * g_celpb_ctrl.chn_cnt);
	memset(pss->its, 0, sizeof(I_TS) * g_celpb_ctrl.chn_cnt);
	memset(pss->frame_cnt, 0, sizeof(int) * g_celpb_ctrl.chn_cnt);
	memset(pss->celmd_ctrl->chns_sec, 0, sizeof(long) * g_celpb_ctrl.chn_cnt);
	pss->state = PB_SINGLE_FRAME;
	pthread_mutex_unlock(&pss->lock);
	main_sleep = 0;
	return 0;
}

int celpb_get_rec_days(long sec, char *day_tbl)
{
	return celmd_get_rec_days(sec, day_tbl);
}

int celpb_get_rec_mins(int ch, long sec, char *min_tbl)
{
	return celmd_get_rec_mins(ch, sec, min_tbl);
}

int celpb_get_rec_res(int ch, long sec, char *res_tbl, int *maxres)
{
	return celmd_get_rec_res(ch, sec, res_tbl, maxres);
}

long celpb_get_play_time(int sid)
{
	pthread_mutex_lock(&g_celpb_ctrl.mutex);
	struct pb_session *pss = search_pb_session(sid);
	if (!pss) {
		CEL_TRACE("invalid sid");
		pthread_mutex_unlock(&g_celpb_ctrl.mutex);
		return -1;
	}
	pthread_mutex_unlock(&g_celpb_ctrl.mutex);
	return celmd_get_cur_play_time(pss->celmd_ctrl);
}

int celpb_get_play_stats(struct pbstat *arr, int len)
{
	if (!arr)
		return -1;
	pthread_mutex_lock(&g_celpb_ctrl.mutex);
	struct pb_session *curr = NULL;
	int i = 0;
	list_for_each_entry(curr, &g_celpb_ctrl.head, list) {
		if (i >= len)
			break;
		arr[i].sid = curr->sid;
		strncpy(arr[i].disk, curr->celmd_ctrl->target_path, sizeof(arr[i].disk));
		arr[i].cid = curr->celmd_ctrl->cid;
		arr[i].secs = curr->celmd_ctrl->sec;
		i++;
	}
	pthread_mutex_unlock(&g_celpb_ctrl.mutex);
	return i;
}

int celpb_stop_all(void)
{
////	pthread_mutex_lock(&g_celpb_ctrl.mutex);
//	struct pb_session *curr = NULL, *tmp = NULL;
//	int i = 0;
//	list_for_each_entry_safe(curr, tmp, &g_celpb_ctrl.head, list) {
//		celpb_stop(curr->sid);
//	}
////	pthread_mutex_unlock(&g_celpb_ctrl.mutex);
	int i = 0;
	for (; i < g_celpb_ctrl.maxpb; i++) {
		CEL_TRACE("stop sid: %d", i);
		celpb_stop(i+1);
	}
	return 0;
}

int celpb_snapshot()
{
	pthread_mutex_lock(&g_celpb_ctrl.mutex);
	struct pb_ctrl *pb_ctrl = &g_celpb_ctrl;
	struct pb_session *pss = NULL, *tmp = NULL;
	list_for_each_entry_safe(pss, tmp, &pb_ctrl->head, list)
	{
		main_sleep = 1;
		pthread_mutex_lock(&pss->lock);
        if(pss->state == PB_PAUSE)//2015.5.9 bruce.milesight add
        {
            pss->state = PB_PAUSE_SNAPSHOT;
			//CEL_TRACE("ch cnt:%d mask:%lld\n", pss->chncnt, pss->ch_mask);
			//david.milesight
			CEL_TRACE("===ch cnt:%d===", pss->chncnt);
        }
		pthread_mutex_unlock(&pss->lock);
		main_sleep = 0;
	}
	pthread_mutex_unlock(&g_celpb_ctrl.mutex);

	return 0;
}

/************david modify 2015-09-10 start **************/
int ms_celpb_get_rec_mins(int ch, long sec, char *min_tbl, long curdaysec, int expdatecnt)
{
	return ms_celmd_get_rec_mins(ch, sec, min_tbl, curdaysec, expdatecnt);
}

int ms_celpb_get_rec_days(long sec, char *day_tbl, long curdaysec, void* records)
{
	return ms_celmd_get_rec_days(sec, day_tbl, curdaysec, records);
}

/************david modify 2015-09-10 end**************/

int celpb_set_mask(int sid, CH_MASK* ch_mask)
{
	CEL_TRACE("celpb_set_mask current sid:%d [%d,%d,%d,%d,%d,%d,%d,%d]\n", sid, ch_mask->mask[0], sid, ch_mask->mask[1], sid, ch_mask->mask[2]
		, sid, ch_mask->mask[3], sid, ch_mask->mask[4], sid, ch_mask->mask[5], sid, ch_mask->mask[6], sid, ch_mask->mask[7]);
	if(sid < 0 || sid > MAX_PB_SID) 
		return CEL_ERR;

	pthread_mutex_lock(&g_celpb_ctrl.mutex);
	struct pb_session *pss = search_pb_session(sid);
	if (!pss) {
		CEL_TRACE("can't find pss, sid: %d", sid);
		pthread_mutex_unlock(&g_celpb_ctrl.mutex);
		return CEL_ERR;
	}

	main_sleep = 1;
	pthread_mutex_lock(&pss->lock);
	ms_celmd_masks_copy(ch_mask, &pss->celmd_ctrl->ch_mask);
	pthread_mutex_unlock(&pss->lock);
	main_sleep = 0;

	pthread_mutex_unlock(&g_celpb_ctrl.mutex);
	
	return CEL_OK;
}

