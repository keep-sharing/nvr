/**
 * @ingroup cellular
 * @defgroup multiple-decode
 * @{
 */

/**
 @file cel_muldec.c
 @brief cellular multiple decode
 @author chimmu
 @date 20130-7-4
*/
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <time.h>

#include "cel_muldec.h"
#include "cel_rec.h"
#include "msdb.h"

#define MAX_FRAME_SIZE 4194304 ///< 4M

#define HDRCNT		10000
#define HBUFSIZE	(SIZEOF_HDR * HDRCNT)
#define MAX_FIND_FRAME_CNT_LIMIT (5000)
#define MAX_FIND_FRAME_TIMEOUT_LIMIT (3600)

static CEL_MULDEC_CTRL g_cel_ctrl;
static int g_max_camera = MAX_CAMERA;

static int alphasort_reverse(const struct dirent **a, const struct dirent **b)
{
    return alphasort(b, a);
}


/**
 * @brief 关闭打开的cellular文件
 * @param[in] celmd_ctrl - 回放控制参数指针
 * @param[in] stop - 是否全部关闭
 * @retval 0
 */
long celmd_close(CEL_MULDEC_CTRL *celmd_ctrl, int stop)
{
//	CEL_TRACE("close cellular decoding. CH:%d, BID:%ld\n", celmd_ctrl->ch_mask, celmd_ctrl->cid);

	if (stop)
		celmd_ctrl->cel_status = CELMD_STATUS_STOP;

	SAFE_CLOSE_CEL(celmd_ctrl->hcel);
	SAFE_CLOSE_CEL(celmd_ctrl->hidx);
#ifdef HDR_ENABLE
	SAFE_CLOSE_CEL(celmd_ctrl->hhdr);
	celmd_ctrl->hcurr = 0;
#endif

	//memset(&celmd_ctrl->ch_mask, 0, sizeof(CH_MASK));

	celmd_ctrl->cid = 0;

	// BK-1222
	celmd_ctrl->fpos_cel =0;

	celmd_ctrl->iframe_cnt = 0;
	celmd_ctrl->iframe_cur = 0;
	return 0;
}

#ifdef HDR_ENABLE

static inline long celmd_find_hdr(CEL_MULDEC_CTRL *celmd_ctrl, int count)
{
	T_STREAM_HDR shd = {0};
	long flag = CEL_ERR, start = 0;
	CEL_TRACE("^^^^^^^^^^^^count: %d", count);
	while (start <= count) {
		memcpy(&shd, celmd_ctrl->hbuf + celmd_ctrl->hcurr * SIZEOF_HDR, SIZEOF_STREAM_HDR);
		if(!ms_celmd_get_chmask_status(&celmd_ctrl->ch_mask, shd.ch)){	
			celmd_ctrl->hcurr++;
			start++;
			continue;
		}
		if (shd.ts.sec < celmd_ctrl->sec) {
			celmd_ctrl->hcurr++;
			start++;
			continue;
		}
		flag = CEL_OK;
		break;
	} // while
	return flag;
}

static long celmd_open_hdrfile(CEL_MULDEC_CTRL *celmd_ctrl, const char *file, long fpos, long direct)
{
	T_HDRFILE_HDR hdr = {0};
	T_STREAM_HDR shd = {0};
	int loop = 0, flag = 0, i = 0, size = 0, left = 0;

//	CEL_TRACE("##############entered");
	do {
		if (!OPEN_RDONLY(celmd_ctrl->hhdr, file)) {
			PERROR("open hdr");
			break;
		}
		if (!READ_ST(celmd_ctrl->hhdr, hdr)) {
			PERROR("read hdrfile hdr");
			break;
		}
		celmd_ctrl->htotal = hdr.cnt;
		celmd_ctrl->hcurr = (int)(fpos - SIZEOF_HDRFILE_HDR) / SIZEOF_HDR;
		CEL_TRACE("*****hdr.cnt: %d, curr: %d, fpos: %ld", hdr.cnt, celmd_ctrl->hcurr, fpos);
		if (!LSEEK_SET(celmd_ctrl->hhdr, fpos)) {
			celmd_ctrl->hcurr = 0;
			CEL_TRACE("hdr seek set error##########");
			PERROR("seek");
			if (HDRCNT > hdr.cnt) {
				if (!READ_PTSZ(celmd_ctrl->hhdr, celmd_ctrl->hbuf, hdr.cnt * SIZEOF_HDR)) {
					CEL_TRACE("read error");
					break;
				}
				celmd_ctrl->hnr = hdr.cnt;
				if (celmd_find_hdr(celmd_ctrl, hdr.cnt) != CEL_OK) {
					CEL_TRACE("can't find time at: %ld", celmd_ctrl->sec);
					break;
				}
			} else {
				left = hdr.cnt % HDRCNT;
				loop = hdr.cnt / HDRCNT;
				if (left)
					loop++;
				while (i < loop) {
					if (left && i+1 == loop) {
						size = left * SIZEOF_HDR;
					} else {
						size = HBUFSIZE;
					}
					if (!READ_PTSZ(celmd_ctrl->hhdr, celmd_ctrl->hbuf, size)) {
						CEL_TRACE("read error");
						flag = -1;
						break;
					}
					celmd_ctrl->hnr += size / SIZEOF_HDR;
					if (celmd_find_hdr(celmd_ctrl, size / SIZEOF_HDR) == CEL_OK)
						break;
					i++;
				}
				if (flag)
					break;
			}
		}
		CEL_TRACE("after, curr: %d", celmd_ctrl->hcurr);
		return CEL_OK;
	} while (0);

	SAFE_CLOSE_CEL(celmd_ctrl->hhdr);
	celmd_ctrl->hcurr = 0;
	celmd_ctrl->htotal = 0;
	return CEL_ERR;
}
#endif
/**
 * @brief 打开回放文件
 * @param[in] celmd_ctrl - 回放控制参数指针
 * @param[in] ch_mask - 通道屏蔽字，通道号从１开始
 * @param[in] sec - 开始时间，以秒计
 * @param[in] direct - 播放方向
 * @retval CEL_ERR - 失败
 * @retval CEL_OK - 成功
 */
long celmd_open(CEL_MULDEC_CTRL *celmd_ctrl, CH_MASK* ch_mask, long sec, int direct, int max_camera)
{
//	struct timeval start = {0}, end = {0};
//	celmd_close(celmd_ctrl, TRUE);

//	gettimeofday(&start, NULL);
	T_CEL_TM tm1;
	cel_get_local_time(sec, &tm1);
	CEL_FHANDLE fd;

	char path[LEN_PATH] = {0};
	T_CEL_RDB rdb = {0};
//	char evts[TOTAL_MINUTES_DAY*CEL_MAX_CHANNEL] = {0};
//	T_CEL_RDB rdbs[TOTAL_MINUTES_DAY*CEL_MAX_CHANNEL] = {0};
	char *evts = NULL;
	T_CEL_RDB *rdbs = NULL;
	T_RDB_HDR rhdr;
	memset(&rhdr, 0, sizeof(rhdr));

	rdb.cid = 0;
	rdb.idx_pos = 0;

//	int size_evt = TOTAL_MINUTES_DAY*CEL_MAX_CHANNEL;
	int i, ch, found = 0;

	g_max_camera = max_camera;
	sprintf(path, "%s/rdb/%04d%02d%02d", celmd_ctrl->target_path, tm1.year, tm1.mon, tm1.day);
	/**start reading rdb*/
	if(!OPEN_RDONLY(fd, path))
	{
		CEL_TRACE("failed open rdb %s\n", path);
		return CEL_ERR;
	}
	CEL_TRACE("opened rdb file for remote playback. %s\n", path);
	if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr))) {
		CEL_TRACE("failed read rdb header");
		CLOSE_CEL(fd);
		return CEL_ERR;
	}
	if (rhdr.chn_max <= 0 || rhdr.chn_max > MAX_CAMERA) {
		CEL_TRACE("invalid rdb hdr channel: %d", rhdr.chn_max);
		CLOSE_CEL(fd);
		return CEL_ERR;
	}

	if(!ms_celmd_masks_cmp(&rhdr.ch_mask, ch_mask)){
		CEL_TRACE("current day has no record for ch_mask");
		CLOSE_CEL(fd);
		return CEL_ERR;
	}
	if ((evts = (char *)ms_calloc(rhdr.chn_max * TOTAL_MINUTES_DAY, sizeof(char))) == NULL) {
		CEL_TRACE("ms_calloc evts error");
		CLOSE_CEL(fd);
		return CEL_ERR;
	}
	if(!READ_PTSZ(fd, evts, sizeof(char) * rhdr.chn_max * TOTAL_MINUTES_DAY))
 	{
 		CEL_TRACE("failed read evt data\n");
 		ms_free(evts);
 		CLOSE_CEL(fd);
 		return CEL_ERR;
	}
	if ((rdbs = (T_CEL_RDB *)ms_calloc(rhdr.chn_max * TOTAL_MINUTES_DAY, sizeof(T_CEL_RDB))) == NULL) {
 		CEL_TRACE("ms_calloc rdbs error");
 		ms_free(evts);
 		CLOSE_CEL(fd);
 		return CEL_ERR;
	}
	// read rdb(cid, pos)
	if(!READ_PTSZ(fd, rdbs, rhdr.chn_max * TOTAL_MINUTES_DAY * sizeof(T_CEL_RDB)))
 	{
 		CEL_TRACE("failed read rdb data\n");
 		ms_free(evts);
 		ms_free(rdbs);
 		CLOSE_CEL(fd);
 		return CEL_ERR;
	}

	CLOSE_CEL(fd); // done read rec info.

	if(direct == CEL_FORWARD ) // forward
	{
		for(i=tm1.hour*60+tm1.min;i<TOTAL_MINUTES_DAY;i++)
		{
			for (ch = 0; ch < rhdr.chn_max; ch++) {
				//if (ch_mask & ((unsigned long long)1 << ch)) {
				if(ms_celmd_get_chmask_status(ch_mask, ch)){
					if (evts[TOTAL_MINUTES_DAY * ch + i] != 0 && rdbs[TOTAL_MINUTES_DAY * ch + i].cid != 0) {
						rdb.cid = rdbs[TOTAL_MINUTES_DAY * ch + i].cid;
						rdb.idx_pos = rdbs[TOTAL_MINUTES_DAY * ch + i].idx_pos;
						CEL_TRACE("found rec time forward, CH:%d, %02d:%02d, BID:%ld, idx_pos:%ld\n", ch, i/60, i%60, rdb.cid, rdb.idx_pos);
						found = 1;
						break;
					}
				}
			}
			if (found)
				break;
		}
	}
	else // backward
	{
		for(i=tm1.hour*60+tm1.min;i>=0 ;i--)
		{
			for (ch = 0; ch < rhdr.chn_max; ch++) {
				//if (ch_mask & ((unsigned long long)1 << ch)) {
				if(ms_celmd_get_chmask_status(ch_mask, ch)){
					if (evts[TOTAL_MINUTES_DAY * ch + i] != 0 && rdbs[TOTAL_MINUTES_DAY * ch + i].cid != 0) {
						rdb.cid = rdbs[TOTAL_MINUTES_DAY * ch + i].cid;
						rdb.idx_pos = rdbs[TOTAL_MINUTES_DAY * ch + i].idx_pos;
						found = 1;
						CEL_TRACE("found rec time backward, %02d:%02d, BID:%ld, idx_pos:%ld\n", i/60, i%60, rdb.cid, rdb.idx_pos);

						break;
					}
				}
			}
			if (found)
				break;
		}
	}
	/**reading rdb done*/

	ms_free(evts);
	ms_free(rdbs);
	if(rdb.cid ==0)
	{
		CEL_TRACE("failed open cellular for streaming -- path:%s, ts:%s\n", path, ctime(&sec));
		return CEL_ERR;
	}

	//////////////////// open cellular file and index file/////////////////////
	//////////////////////////////////////////////////////////////////////////
	sprintf(path, "%s/%06ld.cel", celmd_ctrl->target_path, rdb.cid);
	CEL_TRACE("open cid: %ld, idx_pos: %ld", rdb.cid, rdb.idx_pos);
	if(!OPEN_RDONLY(celmd_ctrl->hcel, path))
//	if (!OPEN_RDSYNC(celmd_ctrl->hcel, path))
	{
		CEL_TRACE("failed open cellular file for decoding -- path:%s\n", path);
		return CEL_ERR;
	}

	//////////////////////////////////////////////////////////////////////////
	sprintf(path, "%s/%06ld.ind", celmd_ctrl->target_path, rdb.cid);
	if(!OPEN_RDONLY(celmd_ctrl->hidx, path))
//	if (!OPEN_RDSYNC(celmd_ctrl->hidx, path))
	{
		CEL_TRACE("failed open index file for decoding -- path:%s\n", path);
		CLOSE_CEL(celmd_ctrl->hcel);
		return CEL_ERR;
	}

	T_INDEX_HDR ihd;
	if(!READ_ST(celmd_ctrl->hidx, ihd))
	{
		CEL_TRACE("failed read index hdr\n");
		celmd_close(celmd_ctrl, 1);

		return CEL_ERR;
	}
	celmd_ctrl->iframe_cnt = cel_is_recording(rdb.cid, celmd_ctrl->target_path) ? cel_get_rec_idx_cnt() : ihd.cnt;

	if(!LSEEK_SET(celmd_ctrl->hidx, rdb.idx_pos))
	{
		CEL_TRACE("failed seek index.\n");
		celmd_close(celmd_ctrl, 1);
		return CEL_ERR;
	}

	// search index file position.
	T_INDEX_DATA idd;
	long idx_pos=rdb.idx_pos;

	if (!READ_ST(celmd_ctrl->hidx, idd)) {
		CEL_TRACE("failed read index data. idx_pos:%ld\n", idx_pos);
		celmd_close(celmd_ctrl, TRUE);
		return CEL_ERR;
	}
	idx_pos = LTELL(celmd_ctrl->hidx) - sizeof(idd);
	celmd_ctrl->iframe_cur = cel_trans_pos2num(idx_pos);

	CEL_TRACE("read index hdr, curiframe:%d, totiframe:%d\n", celmd_ctrl->iframe_cur, celmd_ctrl->iframe_cnt);

	if(!LSEEK_SET(celmd_ctrl->hcel, idd.fpos))
	{
		CEL_TRACE("failed seek cellular:%lld.\n", idd.fpos);
		celmd_close(celmd_ctrl, 1);
		return CEL_ERR;
	}

	if(!LSEEK_SET(celmd_ctrl->hidx, idx_pos))
	{
		CEL_TRACE("failed seek index:%ld.\n", idx_pos);
		celmd_close(celmd_ctrl, 1);
		return CEL_ERR;
	}

	ms_celmd_masks_copy(ch_mask, &celmd_ctrl->ch_mask);
	
	celmd_ctrl->cid = rdb.cid;
	celmd_ctrl->sec = idd.ts.sec;
	celmd_ctrl->cel_status = CELMD_STATUS_OPEN;

#ifdef HDR_ENABLE
	snprintf(path,sizeof(path), "%s/%06ld.hdr", celmd_ctrl->target_path, celmd_ctrl->cid);
	CEL_TRACE("%%%%%%%%%%%%%%%%%%%%%%%%%%idd.hdrpos: %ld", idd.hdrpos);
	if (celmd_open_hdrfile(celmd_ctrl, path, idd.hdrpos, CEL_FORWARD) == CEL_ERR) {
		CEL_TRACE("open hdrfile error......................");
		celmd_close(celmd_ctrl, 1);
		return CEL_ERR;
	}
#endif
//	gettimeofday(&end, NULL);
//	CEL_TRACE("elapse time sec: %ld, usec: %ld", end.tv_sec - start.tv_sec, end.tv_usec - start.tv_usec);
	CEL_TRACE("open cellular file for streaming -- CH:%ld, BID:%ld, B-POS:%lld, I-POS:%ld: time:%ld, %s\n", idd.ch, rdb.cid, idd.fpos, idx_pos, idd.ts.sec, ctime(&idd.ts.sec));

	return CEL_OK;
}

/**
 * @brief 打开下一个cellular文件
 * @param[in] celmd_ctrl - 回放控制参数指针
 * @param[in] ch_mask - 通道屏蔽字，从１开始
 * @param[in] nextcid - 要打开的cellular文件ID
 * @retval CEL_ERR - 失败
 * @retval CEL_OK - 成功
 */
long celmd_open_next(CEL_MULDEC_CTRL *celmd_ctrl, CH_MASK* ch_mask, long nextcid)
{
	celmd_close(celmd_ctrl, FALSE);

	char path[LEN_PATH];
	sprintf(path, "%s/%06ld.cel", celmd_ctrl->target_path, nextcid);

	// open cellular
	if(!OPEN_RDONLY(celmd_ctrl->hcel, path))
	{
		CEL_TRACE("failed open cellular file -- path:%s\n", path);
		celmd_close(celmd_ctrl, TRUE);
		return CEL_ERR;
	}

	T_CEL_HDR bhdr;
	if(!READ_ST(celmd_ctrl->hcel, bhdr))
	{
		CEL_TRACE("failed read cellular header.\n");
		celmd_close(celmd_ctrl, TRUE);
		return CEL_ERR;
	}

	// open index
	sprintf(path, "%s/%06ld.ind", celmd_ctrl->target_path, nextcid);
	if(!OPEN_RDONLY(celmd_ctrl->hidx, path))
	{
		CEL_TRACE("failed open index file -- path:%s\n", path);
		celmd_close(celmd_ctrl, TRUE);
		return CEL_ERR;
	}

	// read index hdr
	T_INDEX_HDR ihd;
	if(!READ_ST(celmd_ctrl->hidx, ihd))
	{
		CEL_TRACE("failed read index hdr\n");
		celmd_close(celmd_ctrl, TRUE);
		return CEL_ERR;
	}
#ifdef HDR_ENABLE
	snprintf(path, sizeof(path), "%s/%06ld.hdr", celmd_ctrl->target_path, nextcid);
	T_HDRFILE_HDR hdr = {0};
	if (!OPEN_RDONLY(celmd_ctrl->hhdr, path)) {
		CEL_TRACE("open error for %s", path);
		PERROR("open");
		celmd_close(celmd_ctrl, TRUE);
		return CEL_ERR;
	}
	if (!READ_ST(celmd_ctrl->hhdr, hdr)) {
		CEL_TRACE("failed read hdr");
		PERROR("read");
		celmd_close(celmd_ctrl, TRUE);
		return CEL_ERR;
	}
	celmd_ctrl->hcurr = 0;
	celmd_ctrl->hnr = 0;
	celmd_ctrl->htotal = hdr.cnt;
#endif
	ms_celmd_masks_copy(ch_mask, &celmd_ctrl->ch_mask);
	celmd_ctrl->cid = nextcid;
	celmd_ctrl->iframe_cnt = cel_is_recording(nextcid, celmd_ctrl->target_path)? cel_get_rec_idx_cnt():ihd.cnt;
	celmd_ctrl->sec = cel_get_min_ts(bhdr.ts.t1);
	if(celmd_ctrl->sec == 0)
		celmd_ctrl->sec = cel_get_min_ts(ihd.ts.t1);

	// BK - 1223 - Start
	celmd_ctrl->iframe_cur = 0;
	celmd_ctrl->fpos_cel = LTELL(celmd_ctrl->hcel);

	//////////////////////////////////////////////////////////////////////////
	// set open status because audio thread
	celmd_ctrl->cel_status = CELMD_STATUS_OPEN;
	//////////////////////////////////////////////////////////////////////////
	// BK - 1223 - End

	CEL_TRACE("open next cellular mul. BID:%ld, iframe-current:%d, iframe_total:%d, last_sec:%ld, %s\n", nextcid, celmd_ctrl->iframe_cur, celmd_ctrl->iframe_cnt, celmd_ctrl->sec, ctime(&celmd_ctrl->sec));

	return CEL_OK;
}

/**
 * @brief 打开前一个cellular文件
 * @see celmd_open_next
 * @param[in] celmd_ctrl - 回放控制参数指针
 * @param[in] ch_mask - 通道屏蔽字，从１开始
 * @param[in] prevcid - 前一个cellular文件
 * @retval CEL_ERR - 失败
 * @retval CEL_OK - 成功
 */
long celmd_open_prev(CEL_MULDEC_CTRL *celmd_ctrl, CH_MASK* ch_mask, long prevcid)
{
	celmd_close(celmd_ctrl, FALSE);

	char path[LEN_PATH];
	sprintf(path, "%s/%06ld.cel", celmd_ctrl->target_path, prevcid);

	// open cellular
	if(!OPEN_RDONLY(celmd_ctrl->hcel, path))
	{
		CEL_TRACE("failed open cellular file -- path:%s\n", path);
		celmd_close(celmd_ctrl, TRUE);
		return CEL_ERR;
	}

	T_CEL_HDR bhdr;
	if(!READ_ST(celmd_ctrl->hcel, bhdr))
	{
		CEL_TRACE("failed read cellular vdo header.\n");
		celmd_close(celmd_ctrl, TRUE);
		return CEL_ERR;
	}

	// open index
	sprintf(path, "%s/%06ld.ind", celmd_ctrl->target_path, prevcid);
	if(!OPEN_RDONLY(celmd_ctrl->hidx, path))
	{
		CEL_TRACE("failed open index file -- path:%s\n", path);
		celmd_close(celmd_ctrl, 1);
		return CEL_ERR;
	}

	T_INDEX_HDR ihd;
	if(!READ_ST(celmd_ctrl->hidx, ihd))
	{
		CEL_TRACE("failed read index hdr\n");
		celmd_close(celmd_ctrl, 1);
		return CEL_ERR;
	}

	if(ihd.cnt != 0)
	{
		if(!LSEEK_CUR(celmd_ctrl->hidx, sizeof(T_INDEX_DATA)*(ihd.cnt-1)))
		{
			CEL_TRACE("failed seek idx. ihd.cnt=%ld\n", ihd.cnt);
			celmd_close(celmd_ctrl, 1);
			return CEL_ERR;
		}

		T_INDEX_DATA idd;
		if(!READ_ST(celmd_ctrl->hidx, idd))
		{
			CEL_TRACE("failed read index data\n");
			celmd_close(celmd_ctrl, TRUE);
			return CEL_ERR;
		}

		if(!LSEEK_SET(celmd_ctrl->hcel, idd.fpos))
		{
			CEL_TRACE("failed seek cellular.\n");
			celmd_close(celmd_ctrl, TRUE);
			return CEL_ERR;
		}
	}
	else
	{
		long prev_sec = 0;
		long prev_usec = 0;
		int index_count=0;
		long long cel_fpos=0;
		T_INDEX_DATA idd;
		while(READ_ST(celmd_ctrl->hidx, idd))
		{
			if( (idd.ts.sec <= prev_sec && idd.ts.usec <= prev_usec)
				|| idd.id != ID_INDEX_DATA
				|| idd.fpos <= cel_fpos)
			{
				CEL_TRACE("Fount last record index pointer. sec=%ld, usec=%ld, count=%d\n", idd.ts.sec, idd.ts.usec, index_count);
				break;
			}

			prev_sec  = idd.ts.sec;
			prev_usec = idd.ts.usec;
			cel_fpos = idd.fpos;
			index_count++;
		}

		if(index_count != 0)
		{
			//////////////////////////////////////////////////////////////////////////
			if(!LSEEK_SET(celmd_ctrl->hcel, cel_fpos))
			{
				CEL_TRACE("failed seek cellular.\n");
				celmd_close(celmd_ctrl, 1);
				return CEL_ERR;
			}

			if(!LSEEK_SET(celmd_ctrl->hidx, sizeof(idd)*index_count))
			{
				CEL_TRACE("failed seek index.\n");
				celmd_close(celmd_ctrl, 1);
				return CEL_ERR;
			}
		}
		else
		{
			if(!LSEEK_END(celmd_ctrl->hcel, 0))
			{
				CEL_TRACE("failed seek cellular.\n");
				celmd_close(celmd_ctrl, 1);
				return CEL_ERR;
			}

			int offset = sizeof(T_INDEX_DATA);
			if(!LSEEK_END(celmd_ctrl->hidx, -offset))
			{
				CEL_TRACE("failed seek idx.\n");
				celmd_close(celmd_ctrl, 1);
				return CEL_ERR;
			}
		}

	}

	ms_celmd_masks_copy(ch_mask, &celmd_ctrl->ch_mask);
	
	celmd_ctrl->cid = prevcid;
	celmd_ctrl->iframe_cnt = cel_is_recording(prevcid, celmd_ctrl->target_path)? cel_get_rec_idx_cnt():ihd.cnt;
	celmd_ctrl->sec = cel_get_max_ts(bhdr.ts.t2);
	if(celmd_ctrl->sec == 0)
		celmd_ctrl->sec = cel_get_max_ts(ihd.ts.t2);

	celmd_ctrl->iframe_cur = celmd_ctrl->iframe_cnt - 1;

	// BK - 1223
	celmd_ctrl->fpos_cel = LTELL(celmd_ctrl->hcel);
	CEL_TRACE("open prev cellular multi. BID:%ld, iframe-current:%d, iframe_total:%d\n", prevcid, celmd_ctrl->iframe_cur, celmd_ctrl->iframe_cnt);

	return CEL_OK;
}

// search functions
/**
 * @brief 后退时，查找前一个cellular文件ID
 * @param[in] path - 要查找的文件的硬盘挂载的路径
 * @param[in] sec - 当前播放的时间点
 * @param[in] ch_mask - 通道号屏蔽字，从１开始
 * @retval CEL_ERR - 失败
 * @retval cellular文件ID - 成功
 */
long celmd_srch_prev_play_cid(const char *path, long sec, CH_MASK* ch_mask)
{
	long sec_seed = sec;

	T_CEL_TM tm1;
	cel_get_local_time(sec_seed, &tm1);

	char buf[30];

	struct dirent **ents;
	int nitems, i;

	sprintf(buf, "%s/rdb/", path);
	nitems = scandir(buf, &ents, NULL, alphasort_reverse);

	if(nitems < 0) {
	    perror("scandir search next hdd start");
		return CEL_ERR;
	}

	if(nitems == 0)
	{
		CEL_TRACE("There is no rdb directory %s.\n", buf);
		ms_free(ents); // BKKIM 20111101
		return CEL_ERR;
	}

	// base.
	sprintf(buf, "%04d%02d%02d", tm1.year, tm1.mon, tm1.day);
	int curr_rdb=atoi(buf);
	int prev_rdb=0;
	int m, ch, cid;
	
    CEL_FHANDLE fd;
	
	for(i=0;i<nitems;i++)
	{
		if (ents[i]->d_name[0] == '.') // skip [.] and [..]
		{
			if (!ents[i]->d_name[1] || (ents[i]->d_name[1] == '.' && !ents[i]->d_name[2]))
			{
				ms_free(ents[i]);
				continue;
			}
		}

		prev_rdb = atoi(ents[i]->d_name);
		CEL_TRACE("prev_rdb=%d,  curr_rdb=%d=======\n", prev_rdb, curr_rdb);

		if(prev_rdb <= curr_rdb)
		{
//			T_CEL_RDB rdbs[TOTAL_MINUTES_DAY*CEL_MAX_CHANNEL];
			T_CEL_RDB *rdbs = NULL;
			T_RDB_HDR rhdr;
			memset(&rhdr, 0, sizeof(rhdr));
			sprintf(buf, "%s/rdb/%s", path, ents[i]->d_name);

			if(!OPEN_RDONLY(fd, buf))
			{
				CEL_TRACE("failed open rdb file. %s\n", buf);
				goto lbl_err_prev_hdd_start_cid;
			}
			if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr))) {
				CEL_TRACE("read rdb hdr error");
				CLOSE_CEL(fd);
				goto lbl_err_prev_hdd_start_cid;
			}
			if (rhdr.chn_max <= 0 || rhdr.chn_max > MAX_CAMERA) {
				CEL_TRACE("invalid max channel: %d", rhdr.chn_max);
				CLOSE_CEL(fd);
				goto lbl_err_prev_hdd_start_cid;
			}

			if(!ms_celmd_masks_cmp(&rhdr.ch_mask, ch_mask)){
				CEL_TRACE("current day has no record for ch_mask");
				CLOSE_CEL(fd);
				goto lbl_err_prev_hdd_start_cid;
			}
			if ((rdbs = (T_CEL_RDB *)ms_calloc(rhdr.chn_max * TOTAL_MINUTES_DAY, sizeof(T_CEL_RDB))) == NULL) {
				CEL_TRACE("ms_calloc rdbs error");
				CLOSE_CEL(fd);
				goto lbl_err_prev_hdd_start_cid;
			}
			if(!LSEEK_SET(fd, TOTAL_MINUTES_DAY * rhdr.chn_max + sizeof(rhdr)))
			{
				CEL_TRACE("failed seek\n");
				ms_free(rdbs);
				CLOSE_CEL(fd);
				goto lbl_err_prev_hdd_start_cid;
			}

			if(!READ_PTSZ(fd, rdbs, rhdr.chn_max * TOTAL_MINUTES_DAY * sizeof(T_CEL_RDB)))
			{
				CEL_TRACE("failed read rdbs data\n");
				ms_free(rdbs);
				CLOSE_CEL(fd);
				goto lbl_err_prev_hdd_start_cid;
			}
			CLOSE_CEL(fd);

			CEL_TRACE("min = [%04d]=======\n", tm1.hour*60+tm1.min);
			for(m=tm1.hour*60+tm1.min-1;m>=0;m--)
			{
				for(ch = 0 ; ch < rhdr.chn_max ; ch++)
				{
					//if (ch_mask & ((unsigned long long)1 << ch)) {
					if(ms_celmd_get_chmask_status(ch_mask, ch)){
						if (rdbs[TOTAL_MINUTES_DAY * ch + m].cid != 0) {
							CEL_TRACE( "found prev HDD BID:%ld, first rec channel:%d\n", rdbs[TOTAL_MINUTES_DAY*ch+m].cid, ch);
							for (; i < nitems; i++) {
								ms_free(ents[i]);
							}
							ms_free(ents);
							cid = rdbs[TOTAL_MINUTES_DAY * ch + m].cid;
							ms_free(rdbs);
							return cid; // found!!
						}
					}
				}
			}

			// in case of First Fail
			tm1.hour = 23;
			tm1.min  = 60; // because index m is zero base.
			ms_free(rdbs);
		} //end if(prev_rdb >= curr_rdb)

		ms_free(ents[i]);

	} // end for

lbl_err_prev_hdd_start_cid:
    for(;i<nitems;i++) {
		ms_free(ents[i]);
	}
	ms_free(ents);
	CEL_TRACE("Can't find prev HDD cellular. CUR-TS:%04d-%02d-%02d %02d:%02d:%02d\n", tm1.year, tm1.mon, tm1.day, tm1.hour, tm1.min, tm1.sec);
	return CEL_ERR;// not found

}
//////////MULTI-HDD SKSUNG//////////
/**
 * @brief 前进时，播放至结束时，查找下一个cellular文件
 * @param[in] path - 要查找的文件的硬盘挂载的路径
 * @param[in] sec - 当前播放的时间点
 * @param[in] ch_mask - 通道号屏蔽字，从１开始
 * @retval CEL_ERR - 失败
 * @retval cellular文件ID - 成功
 */
long celmd_srch_next_play_cid(const char *path, long sec, CH_MASK* ch_mask)
{
	long sec_seed = sec;

	T_CEL_TM tm1;
	cel_get_local_time(sec_seed, &tm1);

	char buf[30];

	struct dirent **ents;
	int nitems, i;

	sprintf(buf, "%s/rdb/", path);
	nitems = scandir(buf, &ents, NULL, alphasort);

	if(nitems < 0) {
	    perror("scandir search next hdd start");
		return CEL_ERR;
	}

	if(nitems == 0)
	{
		CEL_TRACE("There is no rdb directory %s.\n", buf);
		ms_free(ents); // BKKIM 20111101
		return CEL_ERR;
	}

	// base.
	sprintf(buf, "%04d%02d%02d", tm1.year, tm1.mon, tm1.day);
	int curr_rdb=atoi(buf);
	int next_rdb=0;
	int m, ch, cid;
	
    CEL_FHANDLE fd;
	
	for(i=0;i<nitems;i++)
	{
		if (ents[i]->d_name[0] == '.') // except [.] and [..]
		{
			if (!ents[i]->d_name[1] || (ents[i]->d_name[1] == '.' && !ents[i]->d_name[2]))
			{
				ms_free(ents[i]);
				continue;
			}
		}

		next_rdb = atoi(ents[i]->d_name);
		CEL_TRACE("next_rdb=%d,  curr_rdb=%d=======\n", next_rdb, curr_rdb);

		if(next_rdb >= curr_rdb)
		{
//			T_CEL_RDB rdbs[TOTAL_MINUTES_DAY*CEL_MAX_CHANNEL];
			T_CEL_RDB *rdbs = NULL;
			T_RDB_HDR rhdr;
			memset(&rhdr, 0, sizeof(rhdr));
			sprintf(buf, "%s/rdb/%s", path, ents[i]->d_name);

			if(!OPEN_RDONLY(fd, buf))
			{
				CEL_TRACE("failed open rdb file. %s\n", buf);
				goto lbl_err_next_hdd_start_cid;
			}
			if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr))) {
				CEL_TRACE("read rdb header error");
				CLOSE_CEL(fd);
				goto lbl_err_next_hdd_start_cid;
			}
			if (rhdr.chn_max <= 0 || rhdr.chn_max > MAX_CAMERA) {
				CEL_TRACE("invalid rdb header channel: %d", rhdr.chn_max);
				CLOSE_CEL(fd);
				goto lbl_err_next_hdd_start_cid;
			}

			if(!ms_celmd_masks_cmp(&rhdr.ch_mask, ch_mask)){
				CEL_TRACE("current day has no record for ch_mask");
				CLOSE_CEL(fd);
				goto lbl_err_next_hdd_start_cid;
			}
			if ((rdbs = (T_CEL_RDB *)ms_calloc(rhdr.chn_max * TOTAL_MINUTES_DAY, sizeof(T_CEL_RDB))) == NULL) {
				CEL_TRACE("ms_calloc rdbs error");
				CLOSE_CEL(fd);
				goto lbl_err_next_hdd_start_cid;
			}
			if(!LSEEK_SET(fd, TOTAL_MINUTES_DAY*rhdr.chn_max + sizeof(rhdr)))
			{
				CEL_TRACE("failed seek\n");
				ms_free(rdbs);
				CLOSE_CEL(fd);
				goto lbl_err_next_hdd_start_cid;
			}

			if(!READ_PTSZ(fd, rdbs, sizeof(T_CEL_RDB) * rhdr.chn_max * TOTAL_MINUTES_DAY))
			{
				CEL_TRACE("failed read rdbs data\n");
				ms_free(rdbs);
				CLOSE_CEL(fd);
				goto lbl_err_next_hdd_start_cid;
			}
			CLOSE_CEL(fd);

			CEL_TRACE("min = [%04d]=======\n", tm1.hour*60+tm1.min);
			for(m = tm1.hour*60 + tm1.min; m < TOTAL_MINUTES_DAY; m++)
			{
				for(ch = 0; ch < rhdr.chn_max; ch++)
				{
					//if (ch_mask & ((unsigned long long)1 << ch)) {
					if(ms_celmd_get_chmask_status(ch_mask, ch)){
						if (rdbs[TOTAL_MINUTES_DAY * ch + m].cid != 0) {
							CEL_TRACE("found next HDD BID:%ld, first rec channel:%d\n", rdbs[TOTAL_MINUTES_DAY*ch+m].cid, ch);
							for (; i < nitems; i++) {
								ms_free(ents[i]);
							}
							ms_free(ents);
							cid = rdbs[TOTAL_MINUTES_DAY * ch + m].cid;
							ms_free(rdbs);
							return cid; // found!!
						}
					}
				}
			}

			// in case of First Fail
			tm1.hour = 0;
			tm1.min  = -1;
			ms_free(rdbs);
		} //end if(next_rdb >= curr_rdb)

		ms_free(ents[i]);

	} // end for

lbl_err_next_hdd_start_cid:
    for(;i<nitems;i++) {
		ms_free(ents[i]);
	}
	ms_free(ents);
	CEL_TRACE("Can't find next HDD cellular. CUR-TS:%04d-%02d-%02d %02d:%02d:%02d\n", tm1.year, tm1.mon, tm1.day, tm1.hour, tm1.min, tm1.sec);
	return CEL_ERR;// not found

}
//////////END MULTI-HDD SKSUNG//////
/**
 * @brief 同块硬盘之内搜索
 */
long celmd_srch_next_cid(CEL_MULDEC_CTRL *celmd_ctrl, CH_MASK* ch_mask, long curcid, long sec)
{
//	printf("111111111111111\n");
	CEL_TRACE("current cid: %ld, ts: %s", curcid, ctime(&sec));
//	fflush(NULL);
	if(cel_is_recording(curcid, celmd_ctrl->target_path))
	{
		CEL_TRACE("There is no a next cellular. BID:%ld is recording.\n", curcid);
		return CEL_ERR;
	}

//	CEL_TRACE("param time: %s", ctime(&sec));

	long tmp_sec, sec_seed;
	tmp_sec = sec_seed = sec;
	//if(sec_seed == 0)
	{
		long bts=0,ets=0;
		if(CEL_ERR == celmd_get_time(curcid, ch_mask, tmp_sec, &bts, &ets))
		{
			CEL_TRACE("There is no a next cellular. BID:%ld is recording.\n", curcid);
			return CEL_ERR;
		}

		if(tmp_sec < ets)
			sec_seed = ets;
		//CEL_TRACE("last play time is zero. getting last time stamp from cellular. CH:%02d, BID:%ld, BEGIN_SEC:%ld, END_SEC:%ld, DURATION\n", ch, curcid, bts, ets, ets-bts);
	}

	T_CEL_TM tm1;
	cel_get_local_time(sec_seed, &tm1);

	CEL_TRACE("search time: %s, min: %d, curcid: %ld, celmd_ctrl.cid: %ld", ctime(&sec_seed), tm1.hour * 60 + tm1.min, curcid, celmd_ctrl->cid);
	char buf[30];
	sprintf(buf, "%s/rdb/", celmd_ctrl->target_path);

    struct dirent **ents;
    int nitems, i;

    nitems = scandir(buf, &ents, NULL, alphasort);
    
	if(nitems < 0) {
	    perror("scandir - search next cid");
		return CEL_ERR;
	}

	if(nitems == 0)
	{
		CEL_TRACE("There is no rdb directory %s.\n", buf);
		ms_free(ents); // BKKIM 20111031
		return CEL_ERR;
	}

	// base.
	sprintf(buf, "%04d%02d%02d", tm1.year, tm1.mon, tm1.day);
	int curr_rdb=atoi(buf);
	int next_rdb=0;
	int ch, cid;
    CEL_FHANDLE fd;

    for(i=0;i<nitems;i++)
    {
		if (ents[i]->d_name[0] == '.') // except [.] and [..]
		{
			if (!ents[i]->d_name[1] || (ents[i]->d_name[1] == '.' && !ents[i]->d_name[2]))
			{
				ms_free(ents[i]);
				continue;
			}
		}

		next_rdb = atoi(ents[i]->d_name);
		CEL_TRACE("currdb: %d, next rdb: %d, dname: %s", curr_rdb, next_rdb, ents[i]->d_name);

		if(next_rdb >= curr_rdb)
		{
			int m;
//			T_CEL_RDB rdbs[TOTAL_MINUTES_DAY*CEL_MAX_CHANNEL];
			T_CEL_RDB *rdbs = NULL;
			T_RDB_HDR rhdr;
			memset(&rhdr, 0, sizeof(rhdr));
			sprintf(buf, "%s/rdb/%s", celmd_ctrl->target_path, ents[i]->d_name);

			if(!OPEN_RDONLY(fd, buf))
			{
				CEL_TRACE("failed open rdb file. %s\n", buf);
				goto lbl_err_search_next_cid;
			}
			if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr))) {
				CEL_TRACE("read rdb header error");
				CLOSE_CEL(fd);
				goto lbl_err_search_next_cid;
			}
			if (rhdr.chn_max <= 0 || rhdr.chn_max > MAX_CAMERA) {
				CEL_TRACE("invalid rdb header channel: %d", rhdr.chn_max);
				CLOSE_CEL(fd);
				goto lbl_err_search_next_cid;
			}

			if(!ms_celmd_masks_cmp(&rhdr.ch_mask, ch_mask)){
				CEL_TRACE("current day has no record for ch_mask");
				CLOSE_CEL(fd);
				goto lbl_err_search_next_cid;
			}
			if ((rdbs = (T_CEL_RDB *) ms_calloc(rhdr.chn_max * TOTAL_MINUTES_DAY,
					sizeof(T_CEL_RDB))) == NULL ) {
				CEL_TRACE("ms_calloc rdbs error");
				CLOSE_CEL(fd);
				goto lbl_err_search_next_cid;
			}
			if(!LSEEK_SET(fd, TOTAL_MINUTES_DAY*rhdr.chn_max + sizeof(rhdr)))
			{
				CEL_TRACE("failed seek\n");
				CLOSE_CEL(fd);
				ms_free(rdbs);
				goto lbl_err_search_next_cid;
			}

			if(!READ_PTSZ(fd, rdbs, sizeof(T_CEL_RDB) * rhdr.chn_max * TOTAL_MINUTES_DAY))
			{
				CEL_TRACE("failed read rdbs data\n");
				CLOSE_CEL(fd);
				ms_free(rdbs);
				goto lbl_err_search_next_cid;
			}
			CLOSE_CEL(fd);

			for(m=tm1.hour*60+tm1.min+1;m < TOTAL_MINUTES_DAY; m++)
			{
				for (ch = 0; ch < rhdr.chn_max; ch++) {
					//if (ch_mask & ((unsigned long long)1 << ch)) {
					if(ms_celmd_get_chmask_status(ch_mask, ch)){
						if (rdbs[TOTAL_MINUTES_DAY * ch + m].cid != 0 && rdbs[TOTAL_MINUTES_DAY * ch + m].cid != curcid) {
							CEL_TRACE("found next BID:%ld, first rec channel:%d, min: %d\n",rdbs[TOTAL_MINUTES_DAY*ch+m].cid, ch, m);

							for (; i < nitems; i++) {
								ms_free(ents[i]);
							}
							ms_free(ents);
							cid = rdbs[TOTAL_MINUTES_DAY * ch + m].cid;
							ms_free(rdbs);
							return cid; // found!!
						}
//						else if (rdbs[TOTAL_MINUTES_DAY * ch + m + 1].cid != 0 && rdbs[TOTAL_MINUTES_DAY * ch + m + 1].cid != curcid) {
//							CEL_TRACE("found next BID:%ld, first rec channel:%d\n",rdbs[TOTAL_MINUTES_DAY*ch+m+1].cid, ch);
//							for (; i < nitems; i++) {
//								ms_free(ents[i]);
//							}
//							ms_free(ents);
//
//							return rdbs[TOTAL_MINUTES_DAY * ch + m + 1].cid; // found!!
//						}
					}
				}
			}

			// in case of First Fail
			tm1.hour = 0;
			tm1.min  = -1;
			ms_free(rdbs);
		} //end if(next_rdb >= curr_rdb)

		ms_free(ents[i]);

    } // end for

lbl_err_search_next_cid:
    for(;i<nitems;i++) {
		ms_free(ents[i]);
	}
	ms_free(ents);

	CEL_TRACE("Can't find next cellular. CUR-BID:%ld, CUR-TS:%04d-%02d-%02d %02d:%02d:%02d\n",
                  curcid, tm1.year, tm1.mon, tm1.day, tm1.hour, tm1.min, tm1.sec);

	return CEL_ERR;// not found
}

//long BKTMS_SearchPrevBID(CEL_MULDEC_CTRL *celmd_ctrl, int ch, long curcid, long sec)
long celmd_srch_prev_cid(CEL_MULDEC_CTRL *celmd_ctrl, CH_MASK* ch_mask, long curcid, long sec)
{
	long sec_seed = sec;

	//if(sec_seed == 0)
	{
		long bts=0,ets=0;
		if(CEL_ERR == celmd_get_time(curcid, ch_mask, sec, &bts, &ets))
		{
			CEL_TRACE("There is no a previous cellular. CID:%ld.\n", curcid);
			return CEL_ERR;
		}

		if(bts < sec)
			sec_seed = bts;
		//CEL_TRACE("last play time is zero. getting last time stamp from cellular. CH:%02d, BID:%ld, BEGIN_SEC:%ld, END_SEC:%ld, DURATION\n", ch, curcid, bts, ets, ets-bts);
	}

	T_CEL_TM tm1;
	cel_get_local_time(sec_seed, &tm1);

	char buf[30];
	sprintf(buf, "%s/rdb/", celmd_ctrl->target_path);

    struct dirent **ents;
    int nitems, i;

    nitems = scandir(buf, &ents, NULL, alphasort_reverse);

	if(nitems < 0) {
	    perror("scandir reverse");
		return CEL_ERR;
	}

	if(nitems == 0)
	{
		CEL_TRACE("There is no rdb directory %s.\n", buf);
		ms_free(ents);
		return CEL_ERR;
	}

	// base.
	sprintf(buf, "%04d%02d%02d", tm1.year, tm1.mon, tm1.day);
	int curr_rdb=atoi(buf);
	int prev_rdb=0, cid;

    CEL_FHANDLE fd;

    for(i=0;i<nitems;i++)
    {
		if (ents[i]->d_name[0] == '.') // except [.] and [..]
		{
			if (!ents[i]->d_name[1] || (ents[i]->d_name[1] == '.' && !ents[i]->d_name[2]))
			{
				ms_free(ents[i]);
				continue;
			}
		}

		prev_rdb = atoi(ents[i]->d_name);

		if(prev_rdb <= curr_rdb)
		{
			int m, ch;
//			T_CEL_RDB rdbs[TOTAL_MINUTES_DAY*CEL_MAX_CHANNEL];
			T_CEL_RDB *rdbs = NULL;
			T_RDB_HDR rhdr;
			memset(&rhdr, 0, sizeof(rhdr));
			sprintf(buf, "%s/rdb/%s", celmd_ctrl->target_path, ents[i]->d_name);

			if(!OPEN_RDONLY(fd, buf))
			{
				CEL_TRACE("failed open rdb file. %s\n", buf);
				goto lbl_err_search_prev_cid;
			}
			if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr))) {
				CEL_TRACE("read rdb header error");
				CLOSE_CEL(fd);
				goto lbl_err_search_prev_cid;
			}
			if (rhdr.chn_max <= 0 || rhdr.chn_max > MAX_CAMERA) {
				CEL_TRACE("invalid rdb header channel: %d", rhdr.chn_max);
				CLOSE_CEL(fd);
				goto lbl_err_search_prev_cid;
			}
			
			if(!ms_celmd_masks_cmp(&rhdr.ch_mask, ch_mask)){
				CEL_TRACE("current day has no record for ch_mask");
				CLOSE_CEL(fd);
				goto lbl_err_search_prev_cid;
			}
			if ((rdbs = (T_CEL_RDB *) ms_calloc(rhdr.chn_max * TOTAL_MINUTES_DAY,
					sizeof(T_CEL_RDB))) == NULL ) {
				CEL_TRACE("ms_calloc rdbs error");
				CLOSE_CEL(fd);
				goto lbl_err_search_prev_cid;
			}
			if(!LSEEK_SET(fd, TOTAL_MINUTES_DAY * rhdr.chn_max + sizeof(rhdr)))
			{
				CEL_TRACE("failed seek\n");
				CLOSE_CEL(fd);
				ms_free(rdbs);
				goto lbl_err_search_prev_cid;
			}

			if(!READ_PTSZ(fd, rdbs, sizeof(T_CEL_RDB) * rhdr.chn_max * TOTAL_MINUTES_DAY))
			{
				CEL_TRACE("failed read rdbs data\n");
				CLOSE_CEL(fd);
				goto lbl_err_search_prev_cid;
			}
			CLOSE_CEL(fd);

			for(m=tm1.hour*60+tm1.min-1;m >= 0;m--)
			{
				for (ch = 0; ch < rhdr.chn_max; ch++) {
					//if (ch_mask & ((unsigned long long)1 << ch)) {
					if(ms_celmd_get_chmask_status(ch_mask, ch)){
						if (rdbs[TOTAL_MINUTES_DAY * ch + m].cid != 0 && rdbs[TOTAL_MINUTES_DAY * ch + m].cid != curcid) {
							CEL_TRACE( "found prev BID:%ld, last rec channel:%d\n", rdbs[TOTAL_MINUTES_DAY*ch+m].cid, ch);

							for (; i < nitems; i++) {
								ms_free(ents[i]);
							}
							ms_free(ents);
							cid = rdbs[TOTAL_MINUTES_DAY * ch + m].cid;
							ms_free(rdbs);
							return cid; // found!!
						}
					}
				}
			}

			// in case of First fail...by reverse
			tm1.hour = 23;
			tm1.min  = 60;
			ms_free(rdbs);
		} // if(prev_rdb <= curr_rdb)

		ms_free(ents[i]);

    } // end for

lbl_err_search_prev_cid:
    for(;i<nitems;i++) {
		ms_free(ents[i]);
	}
	ms_free(ents);

	CEL_TRACE("Can't find prev cellular. CUR-BID:%ld, CUR-TS:%04d-%02d-%02d %02d:%02d:%02d\n",
	              curcid, tm1.year, tm1.mon, tm1.day, tm1.hour, tm1.min, tm1.sec);

	return CEL_ERR;// not found
}

/**
 * @brief　当前通道是否有录像信息
 * @param[in] ch - 通道号
 * @param[in] sec - 起始时间
 * @param[in] direct - 播放方向
 * @retval CEL_OK - 成功
 * @retval CEL_ERR - 失败
 */
int celmd_has_rec_data(int ch, long sec, int direct)
{
	T_CEL_TM tm1;
	cel_get_local_time(sec, &tm1);

	CEL_FHANDLE fd;

	int i, j;
	char path[LEN_PATH];
//	char evts[TOTAL_MINUTES_DAY*CEL_MAX_CHANNEL];
	char *evts = NULL;
	T_RDB_HDR rhdr;
	T_CEL_MNT_INFO hdd_info = {0};

	memset(&rhdr, 0, sizeof(rhdr));
//	if (celmd_get_tar_disk(&g_cel_ctrl, ch, sec) != 0) return -1;
	cel_get_hdd_info(&hdd_info);
	
	for (j = 0; j < MAX_HDD_COUNT; j++) {
		if (hdd_info.hdd_info[j].is_mnt) {
			snprintf(path, sizeof(path), "%s/rdb/%04d%02d%02d", hdd_info.hdd_info[j].mnt_path, tm1.year, tm1.mon, tm1.day);
			if(!OPEN_RDONLY(fd, path))
			{
				CEL_TRACE("failed open rdb %s\n", path);
				return CEL_ERR;
			}
			if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr))) {
				CEL_TRACE("read rdb header error");
				CLOSE_CEL(fd);
				return CEL_ERR;
			}
			if (rhdr.chn_max <= 0 || rhdr.chn_max > MAX_CAMERA) {
				CEL_TRACE("invalid rdb hdr max channel: %d", rhdr.chn_max);
				CLOSE_CEL(fd);
				return CEL_ERR;
			}
			/*if (!(rhdr.ch_mask & ((unsigned long long)1 << ch))) {
				CEL_TRACE("current day has no record for ch_mask: %d", ch);
				CLOSE_CEL(fd);
				continue;
			}*/
			//david.mileisight
			if(ms_celmd_chn_cmp_masks(&rhdr.ch_mask, ch) == -1){
				CEL_TRACE("current day has no record for ch_mask: %d", ch);
				CLOSE_CEL(fd);
				continue;
			}
			if ((evts = (char *)ms_calloc(rhdr.chn_max * TOTAL_MINUTES_DAY, sizeof(char))) == NULL) {
				CEL_TRACE("ms_calloc rdbs error");
				CLOSE_CEL(fd);
				return CEL_ERR;
			}
			if(!READ_PTSZ(fd, evts, sizeof(char) * rhdr.chn_max * TOTAL_MINUTES_DAY))
			{
				CEL_TRACE("failed read rec data\n");
				ms_free(evts);
				CLOSE_CEL(fd);
				return CEL_ERR;
			}
			CLOSE_CEL(fd);
			if(CEL_FORWARD == direct)
			{
				for(i=tm1.hour*60+tm1.min;i<TOTAL_MINUTES_DAY;i++)
				{
					if(evts[ch*TOTAL_MINUTES_DAY+i] != 0)
					{
						ms_free(evts);
						return CEL_OK;
					}
				}
			}
			else if(CEL_BACKWARD == direct)
			{
				for(i=tm1.hour*60+tm1.min; i>=0 ; i--)
				{
					if(evts[ch*TOTAL_MINUTES_DAY+i] != 0)
					{
						ms_free(evts);
						return CEL_OK;
					}
				}
			}
			else //(BKT_NOSEARCHDIRECTION== direction)
			{
				if(evts[ch*TOTAL_MINUTES_DAY+tm1.hour*60+tm1.min] != 0)
				{
					ms_free(evts);
					return CEL_OK;
				}
			}
		}
	}

	CEL_TRACE("There is no record data. %04d-%02d-%02d %02d:%02d:%02d\n", tm1.year, tm1.mon, tm1.day, tm1.hour, tm1.min, tm1.sec);

	if (evts)
		ms_free(evts);
	return CEL_ERR;
}

/**
 * @brief　读取下一帧，1x速度及以下调用
 * @param[in] celmd_ctrl - 回放信息控制指针
 * @param[out] dp - 读取到的帧信息
 * @retval CEL_OK - 成功
 * @retval CEL_ERR - 失败
 */
long celmd_read_next_frame(CEL_MULDEC_CTRL *celmd_ctrl, T_CELMD_PARAM *dp)
{
	//CEL_TRACE("celmd_read_next_frame Start\n");
	T_STREAM_HDR shd;
	T_VIDEO_STREAM_HDR vhd;
	T_AUDIO_STREAM_HDR ahd;
	char hdr[SIZEOF_HDR] = {0};
#ifdef HDR_ENABLE
	T_HDRFILE_HDR fhdr = {0};
	off_t fpos = 0;
#endif
	while(celmd_ctrl->cel_status == CELMD_STATUS_OPEN)
	{
		// read stream header
#ifdef HDR_ENABLE
		recflag = 0;
		if( cel_is_recording(celmd_ctrl->cid, celmd_ctrl->target_path)) {
			recflag = 1;
			celmd_ctrl->iframe_cnt = cel_get_rec_idx_cnt();
		}
		if (!recflag) {
//			CEL_TRACE("cid: %ld not recording, reload: %d", celmd_ctrl->cid, celmd_ctrl->hreload);
			if (celmd_ctrl->hreload) {
				celmd_ctrl->hreload = 0;
				celmd_ctrl->hcurr = cel_trans_hdr_pos2num(
						celmd_ctrl->hreload_fpos);
				CEL_TRACE("reaload, hcurr: %d", celmd_ctrl->hreload, celmd_ctrl->hcurr);
				if (!LSEEK_SET(celmd_ctrl->hhdr, 0)) {
					CEL_TRACE("seek hhdr to start error");
					PERROR("SEEK");
					return CEL_ERR;
				}
				CEL_TRACE("seek to start done................");
				if (!READ_ST(celmd_ctrl->hhdr, fhdr)) {
					CEL_TRACE("read hdr error");
					PERROR("read");
					return CEL_ERR;
				}
				CEL_TRACE("read hdr header done, cnt: %d", fhdr.cnt);
				celmd_ctrl->htotal = fhdr.cnt;
				if (!LSEEK_SET(celmd_ctrl->hhdr, celmd_ctrl->hreload_fpos)) {
					CEL_TRACE("seek set to %ld error", celmd_ctrl->hreload_fpos);
					PERROR("seek");
					return CEL_ERR;
				}
				celmd_ctrl->hnr = celmd_ctrl->hcurr;
				celmd_ctrl->hnr += min(celmd_ctrl->htotal - celmd_ctrl->hnr, HDRCNT);
				CEL_TRACE("hnr: %d, hcurr: %d", celmd_ctrl->hnr, celmd_ctrl->hcurr);
				if (!READ_PTSZ(celmd_ctrl->hhdr, celmd_ctrl->hbuf, (celmd_ctrl->hnr - celmd_ctrl->hcurr)* SIZEOF_HDR)) {
					CEL_TRACE("error reload hdr file, hnr: %d, hcurr: %d, total: %d",
							celmd_ctrl->hnr, celmd_ctrl->hcurr, celmd_ctrl->htotal);
					PERROR("read");
					return CEL_ERR;
				}
			} //if reload
			if (celmd_ctrl->hcurr >= celmd_ctrl->hnr) {
				celmd_ctrl->hnr = celmd_ctrl->hcurr;
				if (celmd_ctrl->htotal - celmd_ctrl->hnr > HDRCNT) {
					size = HBUFSIZE;
				} else {
					size = (celmd_ctrl->htotal - celmd_ctrl->hnr) * SIZEOF_HDR;
				}
				if (!READ_PTSZ(celmd_ctrl->hhdr, celmd_ctrl->hbuf, size)) {
					CEL_TRACE("read hdr error, size: %d, total: %d, curr: %d, hnr: %d",
							size / SIZEOF_HDR, celmd_ctrl->htotal, celmd_ctrl->hcurr, celmd_ctrl->hnr);
					PERROR("read");
					return CEL_ERR;
				}
				celmd_ctrl->hnr += size / SIZEOF_HDR;
			}
			memcpy(&shd, celmd_ctrl->hbuf + (celmd_ctrl->hcurr % HDRCNT) * SIZEOF_HDR,
					SIZEOF_STREAM_HDR);
#ifdef HDR_ENABLE
			celmd_ctrl->hcurr++;
#endif
		} //if recflag
		else {
#endif
//			CEL_TRACE("cid %ld is recording.....", celmd_ctrl->cid);
			if (!READ_HDR(celmd_ctrl->hcel, hdr)) {
				PERROR("read stream header");
				CEL_TRACE(
						"failed read stream header. go next iframe. shd.id:0x%08lx, frm_size:%ld, cur-iframe:%d, tot-iframe:%d\n",
						shd.id, shd.frm_size, celmd_ctrl->iframe_cur,
						celmd_ctrl->iframe_cnt);
				break;
			}
			memcpy(&shd, hdr, SIZEOF_STREAM_HDR);
#ifdef HDR_ENABLE
		}
#endif
		if(shd.id == ID_VIDEO_HEADER)
		{
			if (shd.ch < 0 || shd.ch > MAX_CAMERA)
				break;
			if(shd.frm_type == FT_IFRAME)
			{
//				CEL_TRACE("meet iframe: %ld, ch: %ld", celmd_ctrl->iframe_cur + 1, shd.ch);
				celmd_ctrl->iframe_cur += 1;
#ifdef HDR_ENABLE
				if( cel_is_recording(celmd_ctrl->cid, celmd_ctrl->target_path))
					celmd_ctrl->iframe_cnt = cel_get_rec_idx_cnt();
#endif
			}
			if (celmd_ctrl->sec && shd.ts.sec - celmd_ctrl->sec > /*4*/(10*60)) {
				CEL_TRACE("time interval is too long. So search next frame. CH:%ld, shd.id:0x%08lx, frm_type:%ld, shd.ts.sec: %ld, ctrl->sec: %ld 0000\n",
						shd.ch, shd.id, shd.frm_type, shd.ts.sec, celmd_ctrl->sec);
				break;
			}
			if(ms_celmd_get_chmask_status(&celmd_ctrl->ch_mask, shd.ch))
			{
				// maybe max interval frame. for broken stream or motion.
				if((celmd_ctrl->chns_sec[shd.ch] && shd.ts.sec - celmd_ctrl->chns_sec[shd.ch] > /*4*/(10*60)) || shd.ts.sec - celmd_ctrl->chns_sec[shd.ch] < 0)
				{
					CEL_TRACE("time interval is too long. So search next frame. CH:%ld, shd.id:0x%08lx, frm_type:%ld, shd.ts.sec: %ld, ctrl->sec: %ld 1111\n",
							shd.ch, shd.id, shd.frm_type, shd.ts.sec, celmd_ctrl->chns_sec[shd.ch]);
					celmd_ctrl->chns_sec[shd.ch] = shd.ts.sec;
					break;
				}
#ifdef HDR_ENABLE
				if (!recflag) {
					memcpy(&vhd,celmd_ctrl->hbuf + (celmd_ctrl->hcurr % HDRCNT) * SIZEOF_HDR + SIZEOF_STREAM_HDR, SIZEOF_FRAME_HDR);
					if (!LSEEK_SET(celmd_ctrl->hcel, shd.data_pos)) {
						PERROR("lseek");
						CEL_TRACE("seek to %ld error", shd.data_pos);
						break;
					}
				} else {
#endif
					memcpy(&vhd, hdr + SIZEOF_STREAM_HDR, SIZEOF_FRAME_HDR);
#ifdef HDR_ENABLE
				}
#endif
				// read video data
				if (shd.frm_size > MAX_FRAME_SIZE) {
					CEL_TRACE("frame size is too big\n");
					break;
				}
				if (dp->buf_size < shd.frm_size) {
					char *tmp = dp->vbuffer;
					if ((dp->vbuffer = (char *)realloc(tmp, shd.frm_size)) == NULL) {
						dp->vbuffer = tmp;
						CEL_TRACE("realloc vbuffer error\n");
						break;
					}
					dp->buf_size = shd.frm_size;
				}
				if(!READ_PTSZ(celmd_ctrl->hcel, dp->vbuffer, shd.frm_size)) {
					perror("read video data");
					CEL_TRACE("failed read video data\n");
					break;
				}

				dp->strm_type  = ST_VIDEO;
				dp->ch          = shd.ch;
				dp->frm_size   = shd.frm_size;
				dp->frm_type   = shd.frm_type;
				dp->ts.sec      = shd.ts.sec;
				dp->ts.usec     = shd.ts.usec;
				dp->time_lower = shd.time_lower;
				dp->time_upper = shd.time_upper;
				dp->frm_rate   = vhd.frm_rate;
				dp->frm_width  = vhd.width;
				dp->frm_height = vhd.height;
//				dp->evt       = vhd.evt;
				dp->codec_type = vhd.codec_type;
				dp->strm_fmt = vhd.strm_fmt;
				celmd_ctrl->sec = shd.ts.sec;
				celmd_ctrl->chns_sec[shd.ch] = shd.ts.sec;

#ifdef HDR_ENABLE
				celmd_ctrl->fpos_cel = (long long)shd.data_pos + shd.frm_size;
#else
				celmd_ctrl->fpos_cel = LTELL(celmd_ctrl->hcel);
#endif
				dp->cid      = celmd_ctrl->cid;
//				dp->fpos_cel   = celmd_ctrl->fpos_cel;

//				CEL_TRACE("Read frame, frameType:%d, iframe-num:%d, size:%ld\n", shd.frm_type, celmd_ctrl->iframe_cur, shd.frm_size);
				return CEL_OK;
			}
			else
			{
#ifdef HDR_ENABLE
				if (recflag) {
#endif
					// skip
//				if(!LSEEK_CUR(celmd_ctrl->hcel, sizeof(vhd) + shd.frm_size))
					if (!LSEEK_CUR(celmd_ctrl->hcel, shd.frm_size)) {
						perror("lseek");
						CEL_TRACE("failed seek cur cellular.\n");
						break;
					}
					//CEL_TRACE("skip ch: %ld, size: %ld", shd.ch, shd.frm_size);
					celmd_ctrl->fpos_cel = LTELL(celmd_ctrl->hcel);
#ifdef HDR_ENABLE
				}
#endif
			}
		} else if( shd.id == ID_AUDIO_HEADER ) {
			// check enable audio channel
			if(!ms_celmd_get_chmask_status(&celmd_ctrl->ch_mask, shd.ch)){
				// skip
#ifdef HDR_ENABLE
				if (!recflag) {
#endif
//				if(!LSEEK_CUR(celmd_ctrl->hcel, sizeof(ahd) + shd.frm_size)) {
					if (!LSEEK_CUR(celmd_ctrl->hcel, shd.frm_size)) {
						perror("lseek");
						CEL_TRACE("failed seek cellular.\n");
						break; // broken stream...
					}
					//CEL_TRACE("skip ch: %ld, size: %ld", shd.ch, shd.frm_size);
					celmd_ctrl->fpos_cel = LTELL(celmd_ctrl->hcel); // save current file pointer
#ifdef HDR_ENABLE
				}
#endif
			}
			else
			{
#ifdef HDR_ENABLE
				if (!recflag) {
					memcpy(&ahd, celmd_ctrl->hbuf + (celmd_ctrl->hcurr % HDRCNT) * SIZEOF_HDR
									+ SIZEOF_STREAM_HDR, SIZEOF_FRAME_HDR);
					if (!LSEEK_SET(celmd_ctrl->hcel, shd.data_pos)) {
						PERROR("lseek");
						CEL_TRACE("seek to %ld error", shd.data_pos);
						break;
					}
				} else {
#endif
					memcpy(&ahd, hdr + SIZEOF_STREAM_HDR, SIZEOF_FRAME_HDR);
#ifdef HDR_ENABLE
				}
#endif
				if(!READ_PTSZ(celmd_ctrl->hcel, dp->vbuffer, shd.frm_size)) { // read audio data
					perror("read aud data");
					CEL_TRACE("failed read audio data\n");
					return CEL_ERR;
				}
				dp->ch		    = shd.ch;
				dp->ts.sec      = shd.ts.sec;
				dp->ts.usec     = shd.ts.usec;
				dp->strm_type  = ST_AUDIO;
				dp->frm_size   = shd.frm_size;
				dp->smp_rate= ahd.smp_rate;
				dp->codec_type = ahd.codec_type;

				celmd_ctrl->sec = shd.ts.sec; // AYK - 1215
				celmd_ctrl->chns_sec[shd.ch] = shd.ts.sec;

//				celmd_ctrl->fpos_cel = LTELL(celmd_ctrl->hcel); // save current file pointer
//				printf("%%%%%%%%%%%%%%%%%%%read audio\n");
				return CEL_OK;
			}
		}
		else {// if(shd.id != ID_VIDEO_HEADER && shd.id != ID_AUDIO_HEADER)
			CEL_TRACE("maybe appeared broken stream .Search next i-frame. shd.id:0x%08lx, frm_size:%ld\n", shd.id, shd.frm_size);
			break;
		}

	}// end while

	if(shd.id == ID_CEL_END) // Normal sequence closed cellular
	{
		return celmd_read_next_first_frame(celmd_ctrl, dp); // reading any FirstFrame on next cellular
	}
	else
	{
		return celmd_read_next_iframe(celmd_ctrl, dp); // Reading First Iframe on cellular
	}
}

/**
 * @brief　打开下一个cellular文件，并读出一帧
 * @param[in] celmd_ctrl - 回放信息控制指针
 * @param[out] dp - 读取到的帧信息
 * @retval CEL_OK -　成功,否则调用@ref celmd_read_next_iframe
 */
long celmd_read_next_first_frame(CEL_MULDEC_CTRL *celmd_ctrl, T_CELMD_PARAM *dp)
{
	CEL_TRACE("read next first frame");
	long res = 0;
	
	CH_MASK ch_mask_tmp;
	memset(&ch_mask_tmp, 0, sizeof(CH_MASK));
	ms_celmd_masks_copy(&celmd_ctrl->ch_mask, &ch_mask_tmp);
	
	long nextcid = celmd_srch_next_cid(celmd_ctrl, &ch_mask_tmp,
			celmd_ctrl->cid, celmd_ctrl->sec);

	T_CEL_MNT_INFO hdd_info = { 0 };

	cel_get_hdd_info(&hdd_info);
	
	if (nextcid != CEL_ERR) {
		if ((res = celmd_open_next(celmd_ctrl, &ch_mask_tmp, nextcid))
				== CEL_ERR) {
			CEL_TRACE("Error nextcid = %ld\n", nextcid);
			return CEL_ERR;
		}
	}

	////MULTI-HDD SKSUNG/////
	if (hdd_info.hdd_cnt == 1 && nextcid == CEL_ERR) {
		return CEL_ERR;
	}
	if (res == CEL_ERR && celmd_jmp2next(celmd_ctrl, CEL_FORWARD) == CEL_ERR)
		return CEL_ERR;

	if (!LSEEK_SET(celmd_ctrl->hidx, cel_trans_idx_num2pos(celmd_ctrl->iframe_cur))) {
		CEL_TRACE("failed seek read next frame. iframe:%d\n",
				celmd_ctrl->iframe_cur);
		return CEL_ERR;
	}
//		printf("success seek read next iframe cur:%d\n", celmd_ctrl->iframe_cur);

	T_INDEX_DATA idd;
	T_STREAM_HDR shd;
	T_VIDEO_STREAM_HDR vhd;
	char hdr[SIZEOF_HDR] = { 0 };
	do {
		if (celmd_ctrl->cel_status == CELMD_STATUS_OPEN) {
			if (!READ_ST(celmd_ctrl->hidx, idd)) {
				PERROR("read");
				CEL_TRACE("read idx header error");
				break;
			}
#ifdef HDR_ENABLE
			celmd_ctrl->hnr = min(celmd_ctrl->htotal, HDRCNT);
			if (!READ_PTSZ(celmd_ctrl->hhdr, celmd_ctrl->hbuf, celmd_ctrl->hnr)) {
				PERROR("read");
				CEL_TRACE("read hdr file error");
				break;
			}
			memcpy(&shd, celmd_ctrl->hbuf, SIZEOF_STREAM_HDR);
#else
			if (!READ_HDR(celmd_ctrl->hcel, hdr)) {
				PERROR("read");
				CEL_TRACE("read stream header error");
				break;
			}
			memcpy(&shd, hdr, SIZEOF_STREAM_HDR);
#endif
			if (shd.ch < 0 || shd.ch > MAX_CAMERA) {
				CEL_TRACE("error, shd.ch: %ld", shd.ch);
				break;
			}
			/*if (!(celmd_ctrl->ch_mask & ((long long) 1 << shd.ch))) {
				CEL_TRACE("error, ch: %ld out of need", shd.ch);
				break;
			}*/

			//david.milesight 
			if(!ms_celmd_get_chmask_status(&celmd_ctrl->ch_mask, shd.ch)){
				CEL_TRACE("error, ch: %ld out of need", shd.ch);
				break;
			}
			if (shd.id != ID_VIDEO_HEADER) {
				CEL_TRACE("error: first frame must be video frame");
				break;
			}
			if (shd.frm_size > MAX_FRAME_SIZE) {
				CEL_TRACE("frame size too big: %ld", shd.frm_size);
				break;
			}
			if (dp->buf_size < shd.frm_size) {
				char *tmp = dp->vbuffer;
				if ((dp->vbuffer = (char *)realloc(tmp, shd.frm_size)) == NULL ) {
					dp->vbuffer = tmp;
					CEL_TRACE("realloc vbuffer error\n");
					break;
				}
				dp->buf_size = shd.frm_size;
			}
#ifdef HDR_ENABLE
			if (!LSEEK_SET(celmd_ctrl->hcel, shd.data_pos)) {
				PERROR("seek");
				CEL_TRACE("seek to pos: %ld error", shd.data_pos);
				break;
			}
#endif
			if (READ_PTSZ(celmd_ctrl->hcel, dp->vbuffer, shd.frm_size )) {
#ifdef HDR_ENABLE
				memcpy(&vhd, celmd_ctrl->hbuf + SIZEOF_VSTREAM_HDR, SIZEOF_FRAME_HDR);
				celmd_ctrl->hcurr++;
#else
				memcpy(&vhd, hdr + SIZEOF_STREAM_HDR, SIZEOF_FRAME_HDR);
#endif
				dp->ch = shd.ch;
				dp->frm_size = shd.frm_size;
				dp->frm_type = shd.frm_type;
				dp->ts.sec = shd.ts.sec;
				dp->ts.usec = shd.ts.usec;
				dp->time_lower = shd.time_lower;
				dp->time_upper = shd.time_upper;
				dp->frm_rate = vhd.frm_rate;
				dp->frm_width = vhd.width;
				dp->frm_height = vhd.height;
//				dp->fpos = 1;
				dp->strm_type = ST_VIDEO;
//				dp->evt = vhd.evt;
				dp->codec_type = vhd.codec_type;
				dp->strm_fmt = vhd.strm_fmt;
				celmd_ctrl->sec = shd.ts.sec;
				celmd_ctrl->chns_sec[shd.ch] = shd.ts.sec;
#ifdef HDR_ENABLE
				celmd_ctrl->fpos_cel = (long long)shd.data_pos + shd.frm_size;
#else
				celmd_ctrl->fpos_cel = LTELL(celmd_ctrl->hcel);
#endif
				dp->cid = celmd_ctrl->cid;
//				dp->fpos_cel = celmd_ctrl->fpos_cel;
				return CEL_OK;
			}
		} // end if
	} while (0);
	return celmd_read_next_iframe(celmd_ctrl, dp);
}


/**
 * @brief 读取一个Ｉ帧
 * @param[in] celmd_ctrl - 回放信息控制指针
 * @param[out] dp - 读取到的帧信息
 * @retval CEL_OK -　成功
 * @retval CEL_ERR - 失败
 * @note 通常变速播放时调用
 */
long celmd_read_next_iframe(CEL_MULDEC_CTRL *celmd_ctrl, T_CELMD_PARAM *dp)
{
	//CEL_TRACE("celmd_read_next_iframe Start\n");
	time_t last_read = 0;
	long res = 0;
	int nfindframeCnt = 0; 
	T_CEL_MNT_INFO hdd_info = { 0 };
	cel_get_hdd_info(&hdd_info);
	CH_MASK ch_mask_tmp ;
	memset(&ch_mask_tmp, 0, sizeof(CH_MASK));
	ms_celmd_masks_copy(&celmd_ctrl->ch_mask, &ch_mask_tmp);
	while (1) {
		if (cel_is_recording(celmd_ctrl->cid, celmd_ctrl->target_path)) {
			celmd_ctrl->iframe_cnt = cel_get_rec_idx_cnt();
		}
		if (celmd_ctrl->iframe_cur + 1 >= celmd_ctrl->iframe_cnt) {
			long nextcid = celmd_srch_next_cid(celmd_ctrl, &ch_mask_tmp,
					celmd_ctrl->cid, celmd_ctrl->sec);
			if (nextcid != CEL_ERR) {
				if ((res = celmd_open_next(celmd_ctrl, &ch_mask_tmp,
						nextcid)) == CEL_ERR)
					break;
			}
			////MULTI-HDD SKSUNG/////
			if (hdd_info.hdd_cnt == 1 && nextcid == CEL_ERR) {
				break;
			}
			if (nextcid == CEL_ERR
					&& celmd_jmp2next(celmd_ctrl, CEL_FORWARD) == CEL_ERR) {
				CEL_TRACE("can't find next cid, break");
				break;
			}
		} else {
			celmd_ctrl->iframe_cur += 1;
		}
		if (!LSEEK_SET(celmd_ctrl->hidx, cel_trans_idx_num2pos(celmd_ctrl->iframe_cur))) {
			CEL_TRACE("failed seek read next frame. iframe:%d\n",
					celmd_ctrl->iframe_cur);
			return CEL_ERR;
		}

		T_INDEX_DATA idd;
		T_STREAM_HDR shd;
		T_VIDEO_STREAM_HDR vhd;
		char hdr[SIZEOF_HDR] = { 0 };
		if (celmd_ctrl->cel_status != CELMD_STATUS_OPEN) {
			CEL_TRACE("cel %ld is closed", celmd_ctrl->cid);
			break;
		}
		if ((last_read > 0 && last_read - celmd_ctrl->sec >= 59)) { // in case system hang up, if read no frame wanted in one minute
			CEL_TRACE("read failed,  celmd_ctrl.sec: %ld, last_read: %ld\n",
					celmd_ctrl->sec, last_read);
			if (celmd_jmp2next(celmd_ctrl, CEL_FORWARD) != CEL_OK)
				break;
			continue;
		}
		if (!READ_ST(celmd_ctrl->hidx, idd)) {
			PERROR("read");
			CEL_TRACE("read idd error");
			break;
		}
		if (idd.fpos < celmd_ctrl->fpos_cel) {
			CEL_TRACE("idd.fpos < celmd_ctrl->fpos_cel");
			continue;
		}
		if (idd.ch < 0 || idd.ch > MAX_CAMERA) {
			CEL_TRACE("[david debug] cid:%ld ch:%ld fpos:%lld lseek:%lld ind head:%d data:%d error!!!\n", celmd_ctrl->cid, idd.ch, idd.fpos, LTELL(celmd_ctrl->hidx), sizeof(T_INDEX_HDR), sizeof(T_INDEX_DATA));
			continue;
		}
		if (!LSEEK_SET(celmd_ctrl->hcel, idd.fpos)) {
			PERROR("seek");
			CEL_TRACE("seek set error, idd fpos: %lld", idd.fpos);
			break;
		}
		if (!READ_HDR(celmd_ctrl->hcel, hdr)) {
			PERROR("read");
			CEL_TRACE("read hdr error");
			break;
		}
		memcpy(&shd, hdr, SIZEOF_STREAM_HDR);
		if (shd.ch < 0 || shd.ch > MAX_CAMERA) {
			CEL_TRACE("error, shd.ch: %ld\n", shd.ch);
			continue;
		}
		if (shd.id != ID_VIDEO_HEADER) {
			CEL_TRACE("error, invalid stream header, should be video header");
			continue;
		}
		if (shd.frm_size > MAX_FRAME_SIZE) {
			CEL_TRACE("frame size too big: %ld", shd.frm_size);
			continue;
		}
		/*if (!(celmd_ctrl->ch_mask & ((unsigned long long) 1 << shd.ch))) {
			continue;
		}*/
		//david.milesight
		
		if(last_read == 0)
			last_read = shd.ts.sec;	
		if(!ms_celmd_get_chmask_status(&celmd_ctrl->ch_mask, shd.ch)){
			if(nfindframeCnt++ < MAX_FIND_FRAME_CNT_LIMIT)
				CEL_TRACE("get_chmask_status mask0:%d mask1:%d chn:%d last_read:%d sec:%d iframe_cur:%d fpos:%lld", celmd_ctrl->ch_mask.mask[0], celmd_ctrl->ch_mask.mask[1], shd.ch, last_read, shd.ts.sec
				,celmd_ctrl->iframe_cur, idd.fpos);
			if(nfindframeCnt > MAX_FIND_FRAME_CNT_LIMIT && (shd.ts.sec - last_read) >= MAX_FIND_FRAME_TIMEOUT_LIMIT)
				break;
			
			continue;
		}
		last_read = shd.ts.sec;			//david modify 20161229
		memcpy(&vhd, hdr + SIZEOF_STREAM_HDR, SIZEOF_FRAME_HDR);
		if (dp->buf_size < shd.frm_size) {
			char *tmp = dp->vbuffer;
			if ((dp->vbuffer = (char *) realloc(tmp, shd.frm_size)) == NULL ) {
				dp->vbuffer = tmp;
				CEL_TRACE("realloc vbuffer error\n");
				continue;
			}
			dp->buf_size = shd.frm_size;
		}
		if (!READ_PTSZ(celmd_ctrl->hcel, dp->vbuffer, shd.frm_size )) {
			CEL_TRACE("read frame error");
			continue;
		}
		//@todo update hdr
		dp->ch = shd.ch;
		dp->frm_size = shd.frm_size;
		dp->frm_type = shd.frm_type;
		dp->ts.sec = shd.ts.sec;
		dp->ts.usec = shd.ts.usec;
		dp->time_lower = shd.time_lower;
		dp->time_upper = shd.time_upper;
		dp->frm_rate = vhd.frm_rate;
		dp->frm_width = vhd.width;
		dp->frm_height = vhd.height;
//		dp->fpos = 1;
		dp->strm_type = ST_VIDEO;
//		dp->evt = vhd.evt;
		dp->codec_type = vhd.codec_type;
		dp->strm_fmt = vhd.strm_fmt;

		celmd_ctrl->sec = shd.ts.sec;
		celmd_ctrl->chns_sec[shd.ch] = shd.ts.sec;

		dp->cid = celmd_ctrl->cid;
//		dp->fpos_cel = celmd_ctrl->fpos_cel;
#ifdef HDR_ENABLE
		//if (!recflag) {
			celmd_ctrl->hreload = 1;
			celmd_ctrl->hreload_fpos = idd.hdrpos;
		//}
		CEL_TRACE("hcurr: %d", celmd_ctrl->hcurr);
		celmd_ctrl->fpos_cel = (long long)shd.data_pos + shd.frm_size;
#else
		celmd_ctrl->fpos_cel = LTELL(celmd_ctrl->hcel);
#endif
		CEL_TRACE("celmd_read_next_iframe End\n");
		nfindframeCnt = 0;
		return CEL_OK;
	}
	//CEL_TRACE("celmd_read_next_iframe End\n");
	return CEL_ERR;
}

/**
 * @brief 读取前一个Ｉ帧
 * @param[in] celmd_ctrl - 回放信息控制指针
 * @param[out] dp - 读取到的帧信息
 * @retval CEL_OK -　成功
 * @retval CEL_ERR - 失败
 */
long celmd_read_prev_iframe(CEL_MULDEC_CTRL *celmd_ctrl, T_CELMD_PARAM *dp)
{
	T_INDEX_DATA idd;
	T_STREAM_HDR shd;
	T_VIDEO_STREAM_HDR vhd;
	long prevcid = -1, res = 0;
	long last_read = 0;
	T_CEL_MNT_INFO hdd = {0};
	char hdr[SIZEOF_HDR] = {0};
	CH_MASK ch_mask_tmp ;
	memset(&ch_mask_tmp, 0, sizeof(CH_MASK));
	ms_celmd_masks_copy(&celmd_ctrl->ch_mask, &ch_mask_tmp);
	
	cel_get_hdd_info(&hdd);
	while(celmd_ctrl->cel_status == CELMD_STATUS_OPEN)
	{
		// find prev cellular
		if(celmd_ctrl->iframe_cur  - 1 < 0){
			
			prevcid = celmd_srch_prev_cid(celmd_ctrl, &ch_mask_tmp, celmd_ctrl->cid, celmd_ctrl->sec);

			if (prevcid != CEL_ERR) {
				// open previous cellular with prevcid
				if((res = celmd_open_prev(celmd_ctrl, &ch_mask_tmp, prevcid)) == CEL_ERR)
					break;
			}
			if (hdd.hdd_cnt == 1 && prevcid == CEL_ERR)
				break;
			if (prevcid == CEL_ERR && celmd_jmp2next(celmd_ctrl, CEL_BACKWARD) == CEL_ERR)
				break;
		}
		else {
			celmd_ctrl->iframe_cur -= 1;
		}

		if (last_read > 0 && celmd_ctrl->sec - last_read > 59) {
//			celmd_ctrl->sec = last_read;
			if (celmd_jmp2next(celmd_ctrl, CEL_BACKWARD) != CEL_OK)
				break;
			continue;
		}
		if(!LSEEK_SET(celmd_ctrl->hidx, cel_trans_idx_num2pos(celmd_ctrl->iframe_cur)))
		{
			CEL_TRACE("failed seek read prev frame. iframe:%d\n", celmd_ctrl->iframe_cur);
			break;
		}

		// read index data with current iframe number
		if(!READ_ST(celmd_ctrl->hidx, idd)) {
			CEL_TRACE("failed read index data. go prev iframe. idd.id:0x%08lx, cur-iframe:%d, tot-iframe:%d\n", idd.id, celmd_ctrl->iframe_cur, celmd_ctrl->iframe_cnt);
			continue;
		}

		if(!LSEEK_SET(celmd_ctrl->hcel, idd.fpos)) {
			CEL_TRACE("failed seek prev index data. i-fpos:%lld\n", idd.fpos);
			continue;
		}

//		if(!READ_HDR(celmd_ctrl->hcel, shd)) {
//			CEL_TRACE("failed read stream header.\n");
//			continue;
//		}
		if (!READ_HDR(celmd_ctrl->hcel, hdr)) {
			CEL_TRACE("read header error");
			continue;
		}
		memcpy(&shd, hdr, SIZEOF_STREAM_HDR);
		if(shd.id != ID_VIDEO_HEADER) {
			CEL_TRACE("Invalid index file pointer.maybe broken iframe-index file.\n");
			continue;
		}

		last_read = shd.ts.sec;
		/*if((celmd_ctrl->ch_mask & ((unsigned long long)1 << shd.ch)) == 0) {
			continue;
		}*/
		//david.milesight 
		if(!ms_celmd_get_chmask_status(&celmd_ctrl->ch_mask, shd.ch)){
			continue;
		}
		if (shd.frm_size > MAX_FRAME_SIZE) {
			CEL_TRACE("frame size is too big\n");
			continue;
		}
		memcpy(&vhd, hdr + SIZEOF_STREAM_HDR, SIZEOF_FRAME_HDR);
		if (dp->buf_size < shd.frm_size) {
			char *tmp = dp->vbuffer;
			if ((dp->vbuffer = (char *)realloc(tmp, shd.frm_size)) == NULL) {
				dp->vbuffer = tmp;
				CEL_TRACE("realloc vbuffer error\n");
				continue;
			}
			dp->buf_size = shd.frm_size;
		}

		if(!READ_PTSZ(celmd_ctrl->hcel, dp->vbuffer, shd.frm_size)) {
			CEL_TRACE("failed read video raw data.\n");
			continue;
		}

		dp->ch          = shd.ch;
		dp->frm_size   = shd.frm_size;
		dp->frm_type   = shd.frm_type;
		dp->ts.sec      = shd.ts.sec;
		dp->ts.usec     = shd.ts.usec;
		dp->time_lower = shd.time_lower;
		dp->time_upper = shd.time_upper;
		dp->frm_rate   = vhd.frm_rate;
		dp->frm_width  = vhd.width;
		dp->frm_height = vhd.height;
//		dp->fpos        = 1;
		dp->strm_type  = ST_VIDEO;
//		dp->evt       = vhd.evt;
		dp->codec_type = vhd.codec_type;
		dp->strm_fmt = vhd.strm_fmt;

		celmd_ctrl->sec = shd.ts.sec;
		celmd_ctrl->chns_sec[shd.ch] = shd.ts.sec;
		celmd_ctrl->fpos_cel = LTELL(celmd_ctrl->hcel);

		dp->cid    = celmd_ctrl->cid;
//		dp->fpos_cel = celmd_ctrl->fpos_cel;
		return CEL_OK;

	}// end while

	return CEL_ERR;
}

//long BKTMS_GetBasketTime(long cid, int ch, long sec, long *o_begin_sec, long *o_end_sec)
/**
 * @brief 获取cellular开始及结束时间
 * @param[in] cid - cellular文件ID
 * @param[in] ch_mask - 通道屏蔽字，从１开始
 * @param[in] sec - 开始时间点
 * @param[out] o_begin_sec -　起始时间
 * @param[out] o_end_sec -　结束时间
 * @retval CEL_ERR - 失败
 * @retval CEL_OK - 成功
 */
long celmd_get_time(long cid, CH_MASK* ch_mask, long sec, long *o_begin_sec, long *o_end_sec)
{
	char path[30];
	T_CEL_HDR bhdr;

    CEL_FHANDLE fd;

	if (celmd_get_tar_disk(&g_cel_ctrl, ch_mask, sec) != CEL_OK)
		return CEL_ERR;
	sprintf(path, "%s/%06ld.cel", g_cel_ctrl.target_path, cid);
	
	CEL_TRACE("BKTMS_GetBasketTime path = %s\n", path);

	// open cellular
	if(!OPEN_RDONLY(fd, path))
	{
		CEL_TRACE("failed open cellular file -- path:%s\n", path);
		return CEL_ERR;
	}
	if(!READ_ST(fd, bhdr))
	{
		CEL_TRACE("failed read cellular header.\n");
		CLOSE_CEL(fd);
		return CEL_ERR;
	}

	CLOSE_CEL(fd);

	*o_begin_sec = cel_get_min_ts(bhdr.ts.t1);
	*o_end_sec   = cel_get_max_ts(bhdr.ts.t2);

	CEL_TRACE("Get Basket Time. BID:%ld, B_SEC:%ld, E_SEC:%ld, Duration:%ld\n", cid, *o_begin_sec,  *o_end_sec, *o_end_sec - *o_begin_sec);

	return CEL_OK;
}


//long BKTMS_setTargetDisk(CEL_MULDEC_CTRL *celmd_ctrl, int hddid, int cid)
/**
 *
 */
long celmd_set_tar_disk(CEL_MULDEC_CTRL *celmd_ctrl, int hddid, int cid)
{
#ifdef CEL_SYSIO_CALL
    int fb_cel ;
#else
    FILE *fb_cel ;
#endif
	T_CEL_MNT_INFO hdd_info = {0};
	T_CELMGR_HDR  hd;

	char diskpath[30] ;
	long first_cid = 0;


	//////MULTI-HDD SKSUNG/////
	cel_get_hdd_info(&hdd_info);
	celmd_ctrl->cur_hdd 	= hddid;
	celmd_ctrl->next_hdd 	= hdd_info.hdd_info[hddid].next_idx;
	

	first_cid = celmd_srch_next_play_cid(hdd_info.hdd_info[hddid].mnt_path,celmd_ctrl->sec, &celmd_ctrl->ch_mask);
	if(first_cid == CEL_ERR)
		return first_cid;	//not found.
	
	sprintf(diskpath, "%s/%s", hdd_info.hdd_info[hddid].mnt_path, NAME_CELMGR) ;

    if(!OPEN_RDWR(fb_cel, diskpath)) // open bktmgr.udf file
    {
        CEL_TRACE("failed open cellular manager file in decoding. %s\n", diskpath);
        return CEL_ERR;
    }

    if(!READ_ST(fb_cel, hd))
    {
        CEL_TRACE("failed read cellular manager header in decoding.\n");
        CLOSE_CEL(fb_cel);
        return CEL_ERR;
    }
	celmd_ctrl->cel_cnt = hd.cel_cnt ;

	sprintf(celmd_ctrl->target_path, "%s", hdd_info.hdd_info[hddid].mnt_path);

	celmd_ctrl->cid = first_cid;
	CLOSE_CEL(fb_cel);

	return first_cid;	//found first  playback cid
	///////////////////////////

//	celmd_ctrl->cid = cid ;
}

//////MULTI-HDD SKSUNG///////
//int BKTMS_getTargetDisk(CEL_MULDEC_CTRL *celmd_ctrl, int ch, long sec)
int celmd_get_tar_disk(CEL_MULDEC_CTRL *celmd_ctrl, CH_MASK* ch_mask, long sec)
{

#ifdef CEL_SYSIO_CALL
    int fd, fb_cel;
#else
    FILE *fd, *fb_cel;
#endif

    int i = 0 , j = 0, searchmin = 0, hdd_idx = -1, flag = 0;
    long res = CEL_ERR;
//    char rec_tbl[TOTAL_MINUTES_DAY * CEL_MAX_CHANNEL] = {0};
    char *rec_tbl = NULL;
    T_RDB_HDR rhdr;
	T_CEL_TM tm1;
	T_CELMGR_HDR  hd;
	T_CEL_MNT_INFO hdd_info = {0};
	char diskpath[30];	
	char fullpath[30] ;	

	memset(&rhdr, 0, sizeof(rhdr));
    cel_get_local_time(sec, &tm1);
    cel_get_hdd_info(&hdd_info);
	searchmin = tm1.hour*60 + tm1.min ;
	int ch;

	for(j = 0; j < MAX_HDD_COUNT; j++)
    {
    	if(hdd_info.hdd_info[j].is_mnt)
   		{
    		flag = 0;
            sprintf(diskpath, "%s", hdd_info.hdd_info[j].mnt_path) ;
			
	        sprintf(fullpath, "%s/rdb/%04d%02d%02d", diskpath, tm1.year, tm1.mon, tm1.day);
	        CEL_TRACE("fullpath: %s\n", fullpath);
	        if(!OPEN_RDONLY(fd, fullpath))
	        {
	            continue;
	        }
	        if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr))) {
	        	CEL_TRACE("read rdb header error");
	        	CLOSE_CEL(fd);
	        	continue;
	        }
	        if (rhdr.chn_max <= 0 || rhdr.chn_max > MAX_CAMERA) {
	        	CEL_TRACE("read rdb header error");
	        	CLOSE_CEL(fd);
	        	continue;
	        }

			if(!ms_celmd_masks_cmp(&rhdr.ch_mask, ch_mask)){
				CEL_TRACE("current day has no record for ch_mask");
				CLOSE_CEL(fd);
				continue;
			}
	        if ((rec_tbl = (char *)ms_calloc(rhdr.chn_max * TOTAL_MINUTES_DAY, sizeof(char))) == NULL) {
	        	CEL_TRACE("ms_calloc rdb header error");
	        	CLOSE_CEL(fd);
	        	continue;
	        }
	        if (!READ_PTSZ(fd, rec_tbl, sizeof(char) * rhdr.chn_max * TOTAL_MINUTES_DAY)) {
	        	CLOSE_CEL(fd);
	        	ms_free(rec_tbl);
	        	continue;
	        }
			
			for(i = searchmin; i < TOTAL_MINUTES_DAY; i++ ) {
				for (ch = 0; ch < rhdr.chn_max; ch++) {
					//if ((ch_mask & ((unsigned long long)1 << ch))) {
					if(ms_celmd_get_chmask_status(ch_mask, ch)){
//						CEL_TRACE("ch %d is on\n", ch);
						if ((int) rec_tbl[i + ch * TOTAL_MINUTES_DAY]/* || (int)rec_tbl[i + 1 + ch * TOTAL_MINUTES_DAY]*/) {
//							hdd_idx = j;
							if (res == CEL_ERR || res > i + ch * TOTAL_MINUTES_DAY) {
								hdd_idx = j;
								res = i + ch * TOTAL_MINUTES_DAY;
							}
							CLOSE_CEL(fd);
							flag = 1;
							break;
						}
					}
				}
				if (flag)
					break;
			}
			ms_free(rec_tbl);
   		}
    }
	memset(diskpath, 0, 30) ;

	if(hdd_idx >= 0)
	{
		sprintf(diskpath, "%s/%s", hdd_info.hdd_info[hdd_idx].mnt_path, NAME_CELMGR) ;

		if(!OPEN_RDWR(fb_cel, diskpath)) // open bktmgr.udf file
        {
            CEL_TRACE("failed open cellular manager file in decoding. %s\n", diskpath);
            return CEL_ERR;
        }

        if(!READ_ST(fb_cel, hd))
        {
            CEL_TRACE("failed read cellular manager header in decoding.\n");
            CLOSE_CEL(fb_cel);
            return CEL_ERR;
        }

		celmd_ctrl->next_hdd 	= hdd_info.hdd_info[hdd_idx].next_idx;
		celmd_ctrl->cur_hdd 	= hdd_idx;
		celmd_ctrl->cel_cnt 	= hd.cel_cnt ;

		sprintf(celmd_ctrl->target_path, "%s", hdd_info.hdd_info[hdd_idx].mnt_path);

		CLOSE_CEL(fb_cel) ;

		CEL_TRACE("set multi playback target disk path :%s\n", celmd_ctrl->target_path);

		return CEL_OK;
	}

	CEL_TRACE("Failed set multi playback target disk path !!! %s\n",celmd_ctrl->target_path);

	return CEL_ERR;
}
////////////////////////////

//int BKTMS_exgetTargetDisk(CEL_MULDEC_CTRL *celmd_ctrl, char *path)
int celmd_exget_tar_disk(CEL_MULDEC_CTRL *celmd_ctrl, char *path)
{
	sprintf(path, "%s",celmd_ctrl->target_path) ;
	return 0;
}

#ifdef RDB_UTC
#include <sys/time.h>
#include <unistd.h>
static int get_dst_hour(time_t timetmp)
{
	time_t now = timetmp;
	struct tm tm;
	localtime_r(&now, &tm);
	CEL_TRACE("dst:%d\n", tm.tm_isdst);
	return tm.tm_isdst;
	//return daylight;// the real daylight
}

static float get_tz_hour()
{
	return (float)(0-timezone)/3600;
#if 0	
	float tz_hour;
	struct tm t1, t2;
	tzset();
	time_t now = time(NULL);
	gmtime_r(&now, &t1);
	localtime_r(&now, &t2);

	int minu1 = t1.tm_mday*24*60 + t1.tm_hour*60 + t1.tm_min;
	int minu2 = t2.tm_mday*24*60 + t2.tm_hour*60 + t2.tm_min;

	tz_hour = (float)(minu2-minu1)/60;
	/*tz_hour = (t2.tm_mday - t1.tm_mday)*24 + (t2.tm_hour - t1.tm_hour);
	if(tz_hour > 0)
		tz_hour += fabs((float)(t2.tm_min - t1.tm_min)/30 * 30/60);// dsttime (the half timezone)
	else if(tz_hour < 0)
		tz_hour -= fabs((float)(t2.tm_min - t1.tm_min)/30 * 30/60);// dsttime*/
	//printf("====day:%d==hour:%d min:%d===day:%d==hour:%d min:%d===\n", t2.tm_mday, t2.tm_hour, t2.tm_min, t1.tm_mday, t1.tm_hour, t1.tm_min);
	//if(tz_hour < -12)
	//	tz_hour += 24;
	//else if(tz_hour > 12)
	//	tz_hour -= 24;
	CEL_TRACE("==time_zone:%f====\n", tz_hour);
	
	/*struct timeval tv;
	struct timezone tz;
	if(gettimeofday(&tv, &tz))
	{
		printf("==gettimeofday failed====\n");
		perror("::");
	}
	printf("=======get tz min==sec:%d==u_sec:%d==tz_min:%d===dsttime:%d=======\n", tv.tv_sec, tv.tv_usec, tz.tz_minuteswest, tz.tz_dsttime);
*/
	return tz_hour;
#endif	
}

static void cel_get_local_time_ex(long sec, T_CEL_TM* ptm)//RDB_UTC
{
	struct tm t1 = {0};
	localtime_r(&sec, &t1);

	ptm->year = t1.tm_year+1900;
	ptm->mon  = t1.tm_mon+1;
	ptm->day  = t1.tm_mday;
	ptm->hour = t1.tm_hour;
	ptm->min  = t1.tm_min;
	ptm->sec  = t1.tm_sec;
}

static int getlastday(int year, int mon)
{
	int daycnt[] = {0, 31, 0, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	if(mon != 2)
		return daycnt[mon];
	if(year % 100 == 0)
	{
		if(year % 400 == 0)
			return 29;
		return 28;
	}
	else
	{
		if(year % 4 == 0)
			return 29;
		return 28;
	}
	return -1;
}

static int get_pbday_index(const char *path, int *index_min, int *index_max)
{
#ifdef CEL_SYSIO_CALL
		int fd;
#else
		FILE *fd;
#endif

	T_RDB_HDR rhdr;
	char tmp_rec[TOTAL_MINUTES_DAY] = {0};
	int i,j;
	long offset = 0;
	int min = -1, max = -1;

	memset(&rhdr, 0, sizeof(rhdr));
	if(!OPEN_RDONLY(fd, path))
	{
		CEL_TRACE("get_pbday_index==failed open rdb %s\n", path);
		return -1;
	}
	if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr)))
	{
		CEL_TRACE("get_pbday_index==read rdb header error");
		CLOSE_CEL(fd);
		return -1;
	}		
	//printf("get_pbday_index mac chl:%d\n", rhdr.chn_max);
	for(i = 0; i< rhdr.chn_max; i++)
	{
	 	offset = TOTAL_MINUTES_DAY*i + sizeof(rhdr);
		if(!LSEEK_SET(fd, offset))
		{
			CEL_TRACE("get_pbday_index==failed seek rdb.\n");
			continue;
		}
		
		if(!READ_PTSZ(fd, tmp_rec, TOTAL_MINUTES_DAY))
		{
			CEL_TRACE("get_pbday_index==failed read rdboffset:%ld\n", offset);
			continue;
		}

		for (j = 0; j < TOTAL_MINUTES_DAY; j++)
		{
			if(tmp_rec[j])
			{
				if(min == -1)
					min = j;			
				else if(min > j)
					min = j;

				if(max == -1)
					max = j;
				else if(max < j)
					max = j;
			}
		}		
	}
	
	CLOSE_CEL(fd);
	if(min == max)
		max = -1;
	*index_min = min;
	*index_max = max;
	return 0;
}

long get_cur_day_to_sec(T_CEL_TM ptm)
{
	char tmp_time[32] = {0};
	struct tm m_tm = {0};
	m_tm.tm_isdst = -1;
	long nAcrosstime;

	snprintf(tmp_time, sizeof(tmp_time), "%04d-%02d-%02d %02d:%02d:%02d", ptm.year, ptm.mon, ptm.day,
		ptm.hour, ptm.min, ptm.sec);
	strptime(tmp_time, "%Y-%m-%d %H:%M:%S", &m_tm);
	nAcrosstime = mktime(&m_tm);

	CEL_TRACE("get_cur_day_to_sec tmp_time:%s, nAcrosstime:%ld\n", tmp_time, nAcrosstime);
	return nAcrosstime;
}

int print_cur_localday_by_sec(long sec, char tmp_time[32])
{
	T_CEL_TM ptm;
	cel_get_local_time_ex(sec, &ptm);
	snprintf(tmp_time, 32, "%04d-%02d-%02d %02d:%02d:%02d", ptm.year, ptm.mon, ptm.day,
		ptm.hour, ptm.min, ptm.sec);

	return 0;
}

long get_cur_localday_start_sec(long sec)
{
	T_CEL_TM ptm;
	char tmp_time[32] = {0};
	struct tm m_tm = {0};
	m_tm.tm_isdst = -1;
	long nAcrosstime;

	cel_get_local_time_ex(sec, &ptm);
	snprintf(tmp_time, sizeof(tmp_time), "%04d-%02d-%02d 00:00:00", ptm.year, ptm.mon, ptm.day);
	strptime(tmp_time, "%Y-%m-%d %H:%M:%S", &m_tm);
	nAcrosstime = mktime(&m_tm);

	//T_CEL_TM t_tm;
	//cel_get_local_time_ex(nAcrosstime, &t_tm);
	CEL_TRACE("get_cur_localday_start_sec tmp_time:%s, nAcrosstime:%ld\n", tmp_time, nAcrosstime);
	return nAcrosstime;
}

long get_cur_localday_end_sec(long sec)
{
	T_CEL_TM ptm;
	char tmp_time[32] = {0};
    struct tm m_tm = {0};
	m_tm.tm_isdst = -1;
	long nAcrosstime;

	cel_get_local_time_ex(sec, &ptm);
	snprintf(tmp_time, sizeof(tmp_time), "%04d-%02d-%02d 23:59:59", ptm.year, ptm.mon, ptm.day);
	strptime(tmp_time, "%Y-%m-%d %H:%M:%S", &m_tm);
	nAcrosstime = mktime(&m_tm);

	//T_CEL_TM t_tm;
	//cel_get_local_time_ex(nAcrosstime, &t_tm);
	CEL_TRACE("get_cur_localday_end_sec tmp_time:%s, nAcrosstime:%ld\n", tmp_time, nAcrosstime);
	return nAcrosstime;
}

int get_cursec_is_acrossday(long sec)
{
	T_CEL_TM local_tm, gm_tm;
	cel_get_local_time_ex(sec, &local_tm);
	cel_get_local_time(sec, &gm_tm);
	CEL_TRACE("get_cursec_is_acrossday local:%04d-%02d-%02d %02d:%02d:%02d gm:%04d-%02d-%02d %02d:%02d:%02d",
		local_tm.year, local_tm.mon, local_tm.day, local_tm.hour, local_tm.min, local_tm.sec,
		gm_tm.year, gm_tm.mon, gm_tm.day, gm_tm.hour, gm_tm.min, gm_tm.sec);
	if(local_tm.day > gm_tm.day){
		return ACROSSDAY_PREVIOUS;
	}else if(local_tm.day < gm_tm.day){
		return ACROSSDAY_AFTER;
	}else
		return ACROSSDAY_NO;
}

#else
static int get_dst_hour()
{
	return 0;
}

static float get_tz_hour()
{
	return 0;
}

static void cel_get_local_time_ex(long sec, T_CEL_TM* ptm)
{
	cel_get_local_time(sec, ptm);
}
#endif //RDB_UTC

//int BKTMS_get_rec_days(int ch, long sec, char *recday_tbl)
int celmd_get_rec_days(long sec, char *recday_tbl)
{
	//david test.
	//struct record *myrecords;
	//end
	T_CEL_TM tm1;
	T_CEL_TM gm;
	int bFindOK=0, j;
	char path[LEN_PATH];
	T_CEL_MNT_INFO hdd_info = {0};

	cel_get_local_time_ex(sec, &tm1);
	cel_get_local_time(sec, &gm);
	cel_get_hdd_info(&hdd_info);
	memset(recday_tbl, 0, 31);
	
	for (j = 0; j < MAX_HDD_COUNT; j++) 
	{
		if (hdd_info.hdd_info[j].is_mnt) 
		{
#if RDB_UTC
		    struct dirent *ent;
		    DIR *dp;
			char buf[LEN_PATH] = {0};
			sprintf(buf, "%s/rdb", hdd_info.hdd_info[j].mnt_path);
			//printf("celmd_get_rec_days=====david=====rdb path:%s,gm(%d, %d)=====\n", buf, gm.year, gm.mon);
		    dp = opendir(buf);
			if(dp == NULL)
			{
				//printf("celmd_get_rec_days=====david=====rdb path:%s==0000===\n", buf);
				continue;
			}
		    while(1)
		    {
		        ent = readdir(dp);
				//printf("celmd_get_rec_days=====david=====rdb path:%s===1111==\n", buf);
		        if (ent == NULL)
		            break;

				if (ent->d_name[0] == '.') // except [.] and [..]
				{
					if (!ent->d_name[1] || (ent->d_name[1] == '.' && !ent->d_name[2]))
					{
						continue;
					}
				}
				
				int day = (ent->d_name[6]-'0')*10 + (ent->d_name[7]- '0');
				int mon = (ent->d_name[4]-'0')*10 + (ent->d_name[5]- '0');
				int year = (ent->d_name[0]-'0')*1000 + (ent->d_name[1]- '0')*100 + (ent->d_name[2]-'0')*10 + (ent->d_name[3]-'0');
				int min = 0, max = 0;
				sprintf(path, "%s/rdb/%s", hdd_info.hdd_info[j].mnt_path, ent->d_name);
				//printf("celmd_get_rec_days=====david=====rdb path:%s==2222===\n", path);
				CEL_TRACE("celmd_get_rec_days=path:%s==year:%d==mon:%d==day:%d=====\n", path, year, mon, day);
				if(gm.year == year && gm.mon == mon && !get_pbday_index(path, &min, &max))
				{
					float tz_hour = get_tz_hour();
					CEL_TRACE("celmd_get_rec_days==path:%s==min:%d==max:%d==tz_hour:%f=\n", path, min, max, tz_hour);
					//printf("celmd_get_rec_days==david==path:%s==min:%d==max:%d==tz_hour:%f=\n", path, min, max, tz_hour);
					if(tz_hour > 0)
					{
						if(min > 0 && min <= ((int)((24-tz_hour)*60))%TOTAL_MINUTES_DAY && day >= 1)
							recday_tbl[day-1] = 1;
						if(max > 0 && max > ((int)((24-tz_hour)*60))%TOTAL_MINUTES_DAY && day < 31)
							recday_tbl[day] = 1;
					}
					else if(tz_hour < 0)
					{
						if(min > 0 && min <= ((int)((24-tz_hour)*60))%TOTAL_MINUTES_DAY && day >= 2)
							recday_tbl[day-2] = 1;
						else if(min > 0 && min <= ((int)((24-tz_hour)*60))%TOTAL_MINUTES_DAY && day < 2)
						{
							if(tm1.day == getlastday(year, mon-1))
								recday_tbl[getlastday(year, mon-1)-1] = 1;
						}
						if(max > 0 && max > ((int)((24-tz_hour)*60))%TOTAL_MINUTES_DAY && day >= 1)
							recday_tbl[day-1] = 1;
					}
					else
					{
						if(day >= 1)
							recday_tbl[day-1] = 1;
					}
					
					if (!bFindOK)
						bFindOK = 1;				
				}
		    }
		    closedir(dp);			
#else
			for (i = 0; i < 31; i++) 
			{
				sprintf(path, "%s/rdb/%04d%02d%02d", hdd_info.hdd_info[j].mnt_path, tm1.year, tm1.mon, i + 1);
				if (-1 != access(path, 0)) 
				{
					recday_tbl[i] = 1;		
					if (!bFindOK)
						bFindOK = 1;
				}
			}
#endif				
		}
	}
	return bFindOK;
}

//int BKTMS_get_rec_mins(int ch_mask, long sec, char *recmin_tbl)
int celmd_get_rec_mins(int ch, long sec, char *recmin_tbl)
{
#ifdef CEL_SYSIO_CALL
	int fd;
#else
	FILE *fd;
#endif
	T_CEL_TM tm1, tm2;
	T_RDB_HDR rhdr;
	char path[LEN_PATH];
	T_CEL_MNT_INFO hdd_info = {0};
	int i, j;
	char tmp_rec[TOTAL_MINUTES_DAY] = {0};
	
	memset(&rhdr, 0, sizeof(rhdr));
	cel_get_local_time_ex(sec, &tm1);
	cel_get_hdd_info(&hdd_info);
	memset(recmin_tbl, 0, TOTAL_MINUTES_DAY);

	float tz_hour = get_tz_hour();
	int dst = get_dst_hour(sec);	
	if(dst != 1)
		dst = 0;
	CEL_TRACE("gui time %04d%02d%02d %02d:%02d:%02d===tz_hour:%f dst:%d\n", tm1.year, tm1.mon, tm1.day, tm1.hour, tm1.min, tm1.sec, tz_hour, dst);
	for (i = 0; i < MAX_HDD_COUNT; i++) 
	{
#ifndef RDB_UTC	
		if (hdd_info.hdd_info[i].is_mnt)
		{
			snprintf(path, sizeof(path), "%s/rdb/%04d%02d%02d", hdd_info.hdd_info[i].mnt_path, tm1.year, tm1.mon, tm1.day);
			CEL_TRACE("open rdb path:%s===tz_hour:%f\n", path, tz_hour);
			long offset = TOTAL_MINUTES_DAY*ch + sizeof(rhdr);
			if(!OPEN_RDONLY(fd, path))
			{
				CEL_TRACE("failed open rdb %s\n", path);
				continue;
			}
			if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr))) {
				CEL_TRACE("read rdb header error");
				CLOSE_CEL(fd);
				continue;
			}
			if (ch >= rhdr.chn_max) {
				CLOSE_CEL(fd);
				CEL_TRACE("invalid chn:%d, chn_max: %d", ch, rhdr.chn_max);
				continue;
			}
			if(!LSEEK_SET(fd, offset))
			{
				CEL_TRACE("failed seek rdb.\n");
				CLOSE_CEL(fd);
				continue;
			}

			if(!READ_PTSZ(fd, tmp_rec, TOTAL_MINUTES_DAY))
			{
				CEL_TRACE("failed read rdb CH:%d, offset:%ld\n", ch, offset);
				CLOSE_CEL(fd);
				continue;
			}
			for (j = 0; j < TOTAL_MINUTES_DAY; j++)
			{
				recmin_tbl[j] |= (tmp_rec[j]);//bruce.milesight modify version:7.0.4
			}
			CLOSE_CEL(fd);
		}
#else	
		if(hdd_info.hdd_info[i].is_mnt && (!tz_hour))
		{
			//printf("===dst:%d===tz_hour:%d====\n", dst, tz_hour);
			//v 7.0.6
			do{
				if(dst)
				{
					cel_get_local_time_ex(sec-86400, &tm2);
					snprintf(path, sizeof(path), "%s/rdb/%04d%02d%02d", hdd_info.hdd_info[i].mnt_path, tm2.year, tm2.mon, tm2.day);
					CEL_TRACE("open pre_rdb path:%s===tz_hour:%f\n", path, tz_hour);
					long offset = TOTAL_MINUTES_DAY*ch + sizeof(rhdr);
					if(!OPEN_RDONLY(fd, path))
					{
						CEL_TRACE("failed open rdb %s\n", path);
						continue;
					}
					if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr))) {
						CEL_TRACE("read rdb header error");
						CLOSE_CEL(fd);
						continue;
					}
					if (ch >= rhdr.chn_max) {
						CLOSE_CEL(fd);
						CEL_TRACE("invalid chn:%d, chn_max: %d", ch, rhdr.chn_max);
						continue;
					}
					if(!LSEEK_SET(fd, offset))
					{
						CEL_TRACE("failed seek rdb.\n");
						CLOSE_CEL(fd);
						continue;
					}
					
					if(!READ_PTSZ(fd, tmp_rec, TOTAL_MINUTES_DAY))
					{
						CEL_TRACE("failed read rdb CH:%d, offset:%ld\n", ch, offset);
						CLOSE_CEL(fd);
						continue;
					}
					for (j = 0; j < 60*dst; j++)
					{
						recmin_tbl[j] |= (tmp_rec[TOTAL_MINUTES_DAY-60*dst+j]);//bruce.milesight modify version:7.0.4
						/*if(tmp_rec[TOTAL_MINUTES_DAY-60*dst+j] && (g_sec_res > (sec-86400+(TOTAL_MINUTES_DAY-60*dst+j)*60) || !g_sec_res))
						{
							g_sec_res = sec-86400+(TOTAL_MINUTES_DAY-60*dst+j)*60;	
							g_cur_hdd = i;
							printf("1111 g_sec_res:%d index:%d\n", g_sec_res, TOTAL_MINUTES_DAY-60*dst+j);
						}*/
					}
					CLOSE_CEL(fd);
				}
			}while(0);
			
			snprintf(path, sizeof(path), "%s/rdb/%04d%02d%02d", hdd_info.hdd_info[i].mnt_path, tm1.year, tm1.mon, tm1.day);
			//CEL_TRACE("open rdb path:%s===tz_hour:%f\n", path, tz_hour);
			long offset = TOTAL_MINUTES_DAY*ch + sizeof(rhdr);
			if(!OPEN_RDONLY(fd, path))
			{
				CEL_TRACE("failed open rdb %s\n", path);
				continue;
			}
			if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr))) {
				CEL_TRACE("read rdb header error");
				CLOSE_CEL(fd);
				continue;
			}
			if (ch >= rhdr.chn_max) {
				CLOSE_CEL(fd);
				CEL_TRACE("invalid chn:%d, chn_max: %d", ch, rhdr.chn_max);
				continue;
			}
			if(!LSEEK_SET(fd, offset))
			{
				CEL_TRACE("failed seek rdb.\n");
				CLOSE_CEL(fd);
				continue;
			}

			if(!READ_PTSZ(fd, tmp_rec, TOTAL_MINUTES_DAY))
			{
				CEL_TRACE("failed read rdb CH:%d, offset:%ld\n", ch, offset);
				CLOSE_CEL(fd);
				continue;
			}
			for (j = 0; j < TOTAL_MINUTES_DAY-60*dst; j++)
			{
				recmin_tbl[j+60*dst] |= (tmp_rec[j]);//bruce.milesight modify version:7.0.4
				/*if(tmp_rec[j] && (g_sec_res > (sec+j*60) || !g_sec_res))
				{
					g_sec_res = sec+j*60;
					g_cur_hdd = i;
					printf("0000 g_sec_res:%d index:%d\n", g_sec_res, j);
				}*/
			}
			CLOSE_CEL(fd);
		}
		else if(hdd_info.hdd_info[i].is_mnt)
		{
			struct tm tm_cur = {0};
			tm_cur.tm_year = tm1.year-1900;
			tm_cur.tm_mon = tm1.mon-1;
			tm_cur.tm_mday = tm1.day;
			tm_cur.tm_hour = 0;//tz_hour;
			tm_cur.tm_min = tm_cur.tm_sec = 0;
			time_t sec_cur = mktime(&tm_cur);
			struct tm tm_start = {0};struct tm tm_end = {0};
			gmtime_r(&sec_cur, &tm_start);			
			
			CEL_TRACE("start:%04d%02d%02d %02d:%02d:%02d\n", tm_start.tm_year+1900, tm_start.tm_mon+1, tm_start.tm_mday,
				tm_start.tm_hour, tm_start.tm_min, tm_start.tm_sec);

			{
				snprintf(path, sizeof(path), "%s/rdb/%04d%02d%02d", hdd_info.hdd_info[i].mnt_path, tm_start.tm_year+1900, tm_start.tm_mon+1, tm_start.tm_mday);
				//CEL_TRACE("open rdb path:%s==1111==tz_hour:%f\n", path, tz_hour);
				long offset = TOTAL_MINUTES_DAY*ch + sizeof(rhdr);
				if(!OPEN_RDONLY(fd, path))
				{
					CEL_TRACE("failed open rdb %s\n", path);
					goto STEP_2;
				}
				if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr))) {
					CEL_TRACE("read rdb header error");
					CLOSE_CEL(fd);
					goto STEP_2;
				}
				if (ch >= rhdr.chn_max) {
					CLOSE_CEL(fd);
					CEL_TRACE("invalid chn:%d, chn_max: %d", ch, rhdr.chn_max);
					goto STEP_2;
				}
				if(!LSEEK_SET(fd, offset))
				{
					CEL_TRACE("failed seek rdb.\n");
					CLOSE_CEL(fd);
					goto STEP_2;
				}

				if(!READ_PTSZ(fd, tmp_rec, TOTAL_MINUTES_DAY))
				{
					CEL_TRACE("failed read rdb CH:%d, offset:%ld\n", ch, offset);
					CLOSE_CEL(fd);
					goto STEP_2;
				}				
				int start_min = tm_start.tm_hour*60 + tm_start.tm_min-60*dst;
				//CEL_TRACE("1111 start_min:%d\n", start_min);
				for(j = start_min; j < TOTAL_MINUTES_DAY; j++)
				{
					recmin_tbl[(j-start_min)] |= tmp_rec[j];
					/*if(tmp_rec[j] && (g_sec_res > (sec_cur+j*60) || !g_sec_res))
					{
						g_sec_res = sec_cur+j*60;
						g_cur_hdd = i;
						printf("2222 g_sec_res:%d index:%d\n", g_sec_res, j);
					}*/
				}
				CLOSE_CEL(fd);
			}
STEP_2: 	
			sec_cur += 86440;// 24 *3600
			gmtime_r(&sec_cur, &tm_end);			
			CEL_TRACE("end:%04d%02d%02d %02d:%02d:%02d\n", tm_end.tm_year+1900, tm_end.tm_mon+1, tm_end.tm_mday,
				tm_end.tm_hour, tm_end.tm_min, tm_end.tm_sec);

			{
				snprintf(path, sizeof(path), "%s/rdb/%04d%02d%02d", hdd_info.hdd_info[i].mnt_path, tm_end.tm_year+1900, tm_end.tm_mon+1, tm_end.tm_mday);
				CEL_TRACE("open rdb path:%s==2222==tz_hour:%f\n", path, tz_hour);
				long offset = TOTAL_MINUTES_DAY*ch + sizeof(rhdr);
				if(!OPEN_RDONLY(fd, path))
				{
					CEL_TRACE("failed open rdb %s\n", path);
					continue;
				}
				if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr))) {
					CEL_TRACE("read rdb header error");
					CLOSE_CEL(fd);
					continue;
				}
				if (ch >= rhdr.chn_max) {
					CLOSE_CEL(fd);
					CEL_TRACE("invalid chn:%d, chn_max: %d", ch, rhdr.chn_max);
					continue;
				}
				if(!LSEEK_SET(fd, offset))
				{
					CEL_TRACE("failed seek rdb.\n");
					CLOSE_CEL(fd);
					continue;
				}

				if(!READ_PTSZ(fd, tmp_rec, TOTAL_MINUTES_DAY))
				{
					CEL_TRACE("failed read rdb CH:%d, offset:%ld\n", ch, offset);
					CLOSE_CEL(fd);
					continue;
				}
				int start_min = tm_start.tm_hour*60 + tm_start.tm_min-60*dst;
				//CEL_TRACE("2222 start_min:%d\n", start_min);
				for(j = 0; j < start_min; j++)
				{
					recmin_tbl[(j+TOTAL_MINUTES_DAY-start_min)] |= tmp_rec[j];
					/*if(tmp_rec[j] && (g_sec_res > (sec_cur+j*60) || !g_sec_res))
					{
						g_sec_res = sec_cur+j*60;
						g_cur_hdd = i;
						printf("3333 g_sec_res:%d index:%d\n", g_sec_res, j);
					}*/
				}
				CLOSE_CEL(fd);
			}		
		}
#endif			
	}

//debug
#if 0
	for(i = 0; i < TOTAL_MINUTES_DAY; i++)
	{
		if(recmin_tbl[i])
		{
			printf("%d ", i);
		}
	}	
	CEL_TRACE("\n");
#endif
	return 0;
}

int celmd_get_rec_res(int ch, long sec, char *recmin_tbl, int *maxres)
{
	CEL_FHANDLE fd;
	T_CEL_TM tm1;
	T_RDB_HDR rhdr;
	char path[LEN_PATH];
	T_CEL_MNT_INFO hdd_info = {0};
	int i, j, res = 0;
	char tmp_rec[TOTAL_MINUTES_DAY] = {0};
	
	memset(&rhdr, 0, sizeof(rhdr));
	cel_get_local_time(sec, &tm1);
	cel_get_hdd_info(&hdd_info);
	memset(recmin_tbl, 0, TOTAL_MINUTES_DAY);
	*maxres = 0;

	for (i = 0; i < MAX_HDD_COUNT; i++) {
		if (hdd_info.hdd_info[i].is_mnt) {
			snprintf(path, sizeof(path), "%s/rdb/%04d%02d%02d", hdd_info.hdd_info[i].mnt_path, tm1.year, tm1.mon, tm1.day);
			long offset = TOTAL_MINUTES_DAY*ch + sizeof(rhdr);
			if(!OPEN_RDONLY(fd, path))
			{
				CEL_TRACE("failed open rdb %s\n", path);
				continue;
			}
			if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr))) {
				CEL_TRACE("read rdb header error");
				CLOSE_CEL(fd);
				continue;
			}
			if (ch >= rhdr.chn_max) {
				CLOSE_CEL(fd);
				CEL_TRACE("invalid chn:%d, chn_max: %d", ch, rhdr.chn_max);
				continue;
			}
			if(!LSEEK_SET(fd, offset))
			{
				CEL_TRACE("failed seek rdb.\n");
				CLOSE_CEL(fd);
				continue;
			}

			if(!READ_PTSZ(fd, tmp_rec, TOTAL_MINUTES_DAY))
			{
				CEL_TRACE("failed read rdb CH:%d, offset:%ld\n", ch, offset);
				CLOSE_CEL(fd);
				continue;
			}
			for (j = 0; j < TOTAL_MINUTES_DAY; j++) {
				res = (int)((tmp_rec[j] >> 4) & 0x0f);
				recmin_tbl[j] |= res;
				*maxres = max(res, *maxres);
			}
			CLOSE_CEL(fd);
		}
	}

	return 0;
}
//int BKTMS_get_cur_playtime(CEL_MULDEC_CTRL *celmd_ctrl)
long celmd_get_cur_play_time(CEL_MULDEC_CTRL *celmd_ctrl)
{
	return celmd_ctrl->sec ;
}

long celmd_set_ctrl_disk(const char *mntpath, T_CELMD_CTRL *celmd_ctrl)
{
	char diskpath[48] = { 0 };
	int fb_cel, i;
	T_CELMGR_HDR hd = {0};
	T_CEL_MNT_INFO info = {0};

	cel_get_hdd_info(&info);
	for (i = 0; i < MAX_HDD_COUNT; i++) {
		if (!info.hdd_info[i].is_mnt)
			continue;
		if (!strcmp(info.hdd_info[i].mnt_path, mntpath))
			break;
	}
	if (i == MAX_HDD_COUNT)
		return CEL_ERR;
	sprintf(diskpath, "%s/%s", mntpath, NAME_CELMGR);
	if (!OPEN_RDWR(fb_cel, diskpath)) // open mgr_cel.cfg file
	{
		CEL_TRACE("failed open cellular manager file in decoding. %s\n", diskpath);
		return CEL_ERR;
	}

	if (!READ_ST(fb_cel, hd)) {
		CEL_TRACE("failed read cellular manager header in decoding.\n");
		CLOSE_CEL(fb_cel);
		return CEL_ERR;
	}

	celmd_ctrl->next_hdd = info.hdd_info[i].next_idx;
	celmd_ctrl->cur_hdd = i;
	celmd_ctrl->cel_cnt = hd.cel_cnt;

	sprintf(celmd_ctrl->target_path, "%s", mntpath);

	CLOSE_CEL(fb_cel);
	CEL_TRACE("set multi playback target disk path :%s\n", celmd_ctrl->target_path);

	return CEL_OK;
}

/**
 * @brief 获取最早有录像信息的时间
 */
long celmd_get_first_rec_time(long sec, CH_MASK* ch_mask, char *mntpath)
{
#ifdef CEL_SYSIO_CALL
	int fd;
#else
	FILE *fd;
#endif

	int i = 0 , j = 0, k = 0, searchmin = 0, flag = 0;
	long offset = 0, res = CEL_ERR;
	char *evts;
	T_RDB_HDR rhdr;
	T_CEL_TM tm1, tm2;
	T_CEL_MNT_INFO hdd_info = {0};
	char diskpath[30];
	char fullpath[30];

	memset(&rhdr, 0, sizeof(rhdr));
	if (!mntpath) {
		CEL_TRACE("mnt path is null");
		return CEL_ERR;
	}
	cel_get_local_time(sec, &tm1);
	cel_get_hdd_info(&hdd_info);
	searchmin = tm1.hour*60 + tm1.min;
	CEL_TRACE("searchmin: %d sec:%ld", searchmin, sec);
#ifdef RDB_UTC
	float tz_hour = get_tz_hour();
	int dst = get_dst_hour(sec);
	if(dst != 1) 
		dst = 0;
	int cnt = 0;
	int nAcrossday = 0; 
	long nCLocaldayStart, nCLocaldayEnd;
	nAcrossday = get_cursec_is_acrossday(sec);
	nCLocaldayStart = get_cur_localday_start_sec(sec);
	nCLocaldayEnd = get_cur_localday_end_sec(sec);
	CEL_TRACE("nAcrossday:%d, nCLocaldayStart:%ld, nCLocaldayEnd:%ld\n", nAcrossday, nCLocaldayStart, nCLocaldayEnd);
	do
	{
#endif	
		cel_get_local_time_ex(sec, &tm2);
		CEL_TRACE("sec:%ld gmtime:%04d-%02d-%02d %02d:%02d:%02d localtime:%04d-%02d-%02d %02d:%02d:%02d tz_hour:%f dst:%d\n", 
			sec, tm1.year, tm1.mon, tm1.day, tm1.hour, tm1.min, tm1.sec, tm2.year, tm2.mon, tm2.day, tm2.hour, tm2.min, tm2.sec, tz_hour, dst); 
				
		for (j = 0; j < MAX_HDD_COUNT; j++) {
			if (!hdd_info.hdd_info[j].is_mnt)
				continue;
			flag = 0;
			sprintf(diskpath, "%s", hdd_info.hdd_info[j].mnt_path);
			sprintf(fullpath, "%s/rdb/%04d%02d%02d", diskpath, tm1.year, tm1.mon, tm1.day);
			CEL_TRACE("fullpath: %s", fullpath);
			if (!OPEN_RDONLY(fd, fullpath))
				continue;
			if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr))) {
				CEL_TRACE("read rdb header error");
				CLOSE_CEL(fd);
				continue;
			}
			if (rhdr.chn_max <= 0 || rhdr.chn_max > MAX_CAMERA) {
				CEL_TRACE("invalid rdb header channel: %d > MAX_CAMERA:%d", rhdr.chn_max, MAX_CAMERA);
				CLOSE_CEL(fd);
				continue;
			}

			if(!ms_celmd_masks_cmp(&rhdr.ch_mask, ch_mask)){
				CEL_TRACE("current day has no record for ch_mask");
				CLOSE_CEL(fd);
				continue;
			}
			if (!(evts = ms_calloc(rhdr.chn_max * TOTAL_MINUTES_DAY, sizeof(char)))) {
				CEL_TRACE("ms_calloc rdb evts error");
				CLOSE_CEL(fd);
				continue;
			}
			if (!READ_PTSZ(fd, evts, sizeof(char) * rhdr.chn_max * TOTAL_MINUTES_DAY)) {
				CEL_TRACE("failed read rec evt data\n");
				CLOSE_CEL(fd);
				ms_free(evts);
				continue;
			}
			CLOSE_CEL(fd);
			CEL_TRACE("searchmin: %d\n", searchmin);
			for (i = searchmin; i < TOTAL_MINUTES_DAY; i++) {
				for (k = 0; k < rhdr.chn_max; k++) {
					if(ms_celmd_get_chmask_status(ch_mask, k)){
						offset = TOTAL_MINUTES_DAY * k;
						if (evts[offset + i] != 0) {
							if (res == CEL_ERR || res > sec + (i - searchmin) * 60) {
								res = sec + (i - searchmin) * 60;
								strcpy(mntpath, hdd_info.hdd_info[j].mnt_path);
							}
							CEL_TRACE("min: %d, chn: %d, sec:%ld, res: %ld, disk: %s", i, k, sec, res, hdd_info.hdd_info[j].mnt_path);
							flag = 1;
							break;
						}
					}
				}
				if (flag)
					break;
			}
			ms_free(evts);
		}
#ifdef RDB_UTC				
		if(nAcrossday == ACROSSDAY_PREVIOUS){
			sec -= (tm1.hour*3600 + tm1.min*60 + tm1.sec);
			sec += 24*60*60;					//add one day sec
			cel_get_local_time(sec, &tm1);
		}else if(nAcrossday == ACROSSDAY_AFTER){
			if(res > nCLocaldayEnd || res < nCLocaldayStart){
				sec = nCLocaldayStart;
				cel_get_local_time(sec, &tm1);
				searchmin = tm1.hour*60 + tm1.min;
				res = -1;
			}
			if(res == -1 && cnt == 1){
#if 0				
				//tm1.day += 1;
				//tm1.hour = tm1.min = tm1.sec = 0;
				//sec = get_cur_day_to_sec(tm1);
#else
				sec -= (tm1.hour*3600 + tm1.min*60 + tm1.sec);
				sec += 24*60*60;					//add one day sec
				cel_get_local_time(sec, &tm1);
#endif				
				cnt--;
			}
		}else{
			if((int)tz_hour<0){
				sec -= (tm1.hour*3600 + tm1.min*60 + tm1.sec);
				sec += 24*60*60;				//add one day sec
				cel_get_local_time(sec, &tm1);
			}else{
				if(res == -1){
					sec -= (tm1.hour*3600 + tm1.min*60 + tm1.sec);
					tm1.hour = tm1.min = tm1.sec = 0;
				}else if(res > nCLocaldayEnd || res < nCLocaldayStart){
					sec = nCLocaldayStart;
					cel_get_local_time(sec, &tm1);
					searchmin = tm1.hour*60 + tm1.min;
					res = -1;
				}
				if(res == -1 && cnt == 1){
#if 0					
					//tm1.day += 1;
					//tm1.hour = tm1.min = tm1.sec = 0;
					//sec = get_cur_day_to_sec(tm1);
#else
					sec -= (tm1.hour*3600 + tm1.min*60 + tm1.sec);
					sec += 24*60*60;					//add one day sec
					cel_get_local_time(sec, &tm1);
#endif					
					cnt--;
				}
			}		
		}
		
		searchmin = tm1.hour*60 + tm1.min;	
		CEL_TRACE("gmtime %04d-%02d-%02d %02d:%02d:%02d\n", tm1.year, tm1.mon, tm1.day, tm1.hour, tm1.min, tm1.sec);
		CEL_TRACE("cnt:%d tz_hour:%f dst:%d sec:%ld\n", cnt, tz_hour, dst, sec);
		if(sec > nCLocaldayEnd) break;
	}
	while(res == -1 && cnt++ < 1);
#endif
	CEL_TRACE("res = %ld", res);
	return res;
}


long celmd_get_first_rec_time_ex(long sec, CH_MASK* ch_mask, char *mntpath)
{
#ifdef CEL_SYSIO_CALL
	int fd;
#else
	FILE *fd;
#endif
	int cnt = 0;
	int nAcrossday = 0; 
	long nCLocaldayStart, nCLocaldayEnd;
	int i = 0 , j = 0, k = 0, searchmin = 0, flag = 0;
	long offset = 0, res = CEL_ERR;
	char *evts;
	T_RDB_HDR rhdr;
	T_CEL_TM tm1, tm2;
	T_CEL_TM path_tm1, path_tm2;
	T_CEL_MNT_INFO hdd_info = {0};
	char diskpath[30];
	char fullpath[30];

	memset(&rhdr, 0, sizeof(rhdr));
	if (!mntpath) {
		CEL_TRACE("mnt path is null");
		return CEL_ERR;
	}
	
	float tz_hour = get_tz_hour();
	int dst = get_dst_hour(sec);
	if(dst != 1) 
		dst = 0;
	/*else
		sec = sec - 60*60;		//one hour*/
	
	cel_get_local_time(sec, &tm1);
	cel_get_hdd_info(&hdd_info);
	searchmin = tm1.hour*60 + tm1.min;
	CEL_TRACE("searchmin: %d sec:%ld", searchmin, sec);

	nAcrossday = get_cursec_is_acrossday(sec);
	nCLocaldayStart = get_cur_localday_start_sec(sec);
	nCLocaldayEnd = get_cur_localday_end_sec(sec);
	cel_get_local_time(nCLocaldayStart, &path_tm1);
	cel_get_local_time(nCLocaldayEnd, &path_tm2);
	CEL_TRACE("nAcrossday:%d, nCLocaldayStart:%ld, nCLocaldayEnd:%ld\n", nAcrossday, nCLocaldayStart, nCLocaldayEnd);
	
	do
	{
		cel_get_local_time_ex(sec, &tm2);
		CEL_TRACE("sec:%ld gmtime:%04d-%02d-%02d %02d:%02d:%02d localtime:%04d-%02d-%02d %02d:%02d:%02d tz_hour:%f dst:%d\n", 
			sec, tm1.year, tm1.mon, tm1.day, tm1.hour, tm1.min, tm1.sec, tm2.year, tm2.mon, tm2.day, tm2.hour, tm2.min, tm2.sec, tz_hour, dst); 
		
		for (j = 0; j < MAX_HDD_COUNT; j++) {
			if (!hdd_info.hdd_info[j].is_mnt)
				continue;
			flag = 0;
			sprintf(diskpath, "%s", hdd_info.hdd_info[j].mnt_path);
			sprintf(fullpath, "%s/rdb/%04d%02d%02d", diskpath, tm1.year, tm1.mon, tm1.day);
			CEL_TRACE("fullpath: %s", fullpath);
			if (!OPEN_RDONLY(fd, fullpath))
				continue;
			if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr))) {
				CEL_TRACE("read rdb header error");
				CLOSE_CEL(fd);
				continue;
			}
			if (rhdr.chn_max <= 0 || rhdr.chn_max > MAX_CAMERA) {
				CEL_TRACE("invalid rdb header channel: %d > MAX_CAMERA:%d", rhdr.chn_max, MAX_CAMERA);
				CLOSE_CEL(fd);
				continue;
			}

			if(!ms_celmd_masks_cmp(&rhdr.ch_mask, ch_mask)){
				CEL_TRACE("current day has no record for ch_mask");
				CLOSE_CEL(fd);
				continue;
			}
			if (!(evts = ms_calloc(rhdr.chn_max * TOTAL_MINUTES_DAY, sizeof(char)))) {
				CEL_TRACE("ms_calloc rdb evts error");
				CLOSE_CEL(fd);
				continue;
			}
			if (!READ_PTSZ(fd, evts, sizeof(char) * rhdr.chn_max * TOTAL_MINUTES_DAY)) {
				CEL_TRACE("failed read rec evt data\n");
				CLOSE_CEL(fd);
				ms_free(evts);
				continue;
			}
			CLOSE_CEL(fd);
			CEL_TRACE("searchmin: %d\n", searchmin);
			for (i = searchmin; i < TOTAL_MINUTES_DAY; i++) {
				for (k = 0; k < rhdr.chn_max; k++) {
					if(ms_celmd_get_chmask_status(ch_mask, k)){
						offset = TOTAL_MINUTES_DAY * k;
						if (evts[offset + i] != 0) {
							if (res == CEL_ERR || res > sec + (i - searchmin) * 60) {
								res = sec + (i - searchmin) * 60;
								strcpy(mntpath, hdd_info.hdd_info[j].mnt_path);
							}
							CEL_TRACE("min: %d, chn: %d, sec:%ld, res: %ld, disk: %s", i, k, sec, res, hdd_info.hdd_info[j].mnt_path);
							flag = 1;
							break;
						}
					}
				}
				if (flag)
					break;
			}
			ms_free(evts);
		}

		cnt++;
		CEL_TRACE("cnt:%d tz_hour:%f dst:%d sec:%ld\n", cnt, tz_hour, dst, sec);
		if(res > nCLocaldayEnd || res < nCLocaldayStart)
			res = -1;
	}
	while(res == -1 && cnt++ < 1);
	CEL_TRACE("res = %ld", res);
	return res;
}


/**
 * @brief 获取最晚有录像信息的时间
 */
long celmd_get_last_rec_time(long sec, CH_MASK* ch_mask)
{
#ifdef CEL_SYSIO_CALL
	int fd;
#else
	FILE *fd;
#endif

	int i = 0 , j = 0, k = 0, searchmin = 0, flag = 0;
	long offset = 0, res = CEL_ERR;
//	char evts[TOTAL_MINUTES_DAY*CEL_MAX_CHANNEL];
	char *evts;
	T_RDB_HDR rhdr;
	T_CEL_TM tm1;
	T_CEL_MNT_INFO hdd_info = {0};
	char diskpath[30];
	char fullpath[30];

	memset(&rhdr, 0, sizeof(rhdr));
	cel_get_local_time(sec, &tm1);
	cel_get_hdd_info(&hdd_info);
	searchmin = tm1.hour*60 + tm1.min ;
	CEL_TRACE("searchmin: %d sec:%ld", searchmin, sec);
	for (j = 0; j < MAX_HDD_COUNT; j++) {
		if (!hdd_info.hdd_info[j].is_mnt)
			continue;
		flag = 0;
		sprintf(diskpath, "%s", hdd_info.hdd_info[j].mnt_path);
		sprintf(fullpath, "%s/rdb/%04d%02d%02d", diskpath, tm1.year, tm1.mon, tm1.day);

		if (!OPEN_RDONLY(fd, fullpath))
			continue;

		if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr))) {
			CEL_TRACE("read rdb header error");
			CLOSE_CEL(fd);
			continue;
		}
		if (rhdr.chn_max <= 0 || rhdr.chn_max > MAX_CAMERA) {
			CEL_TRACE("invalid rdb header channel: %d", rhdr.chn_max);
			CLOSE_CEL(fd);
			continue;
		}
		
		if(!ms_celmd_masks_cmp(&rhdr.ch_mask, ch_mask)){
			CEL_TRACE("current day has no record for ch_mask");
			CLOSE_CEL(fd);
			continue;
		}
		if (!(evts = ms_calloc(rhdr.chn_max * TOTAL_MINUTES_DAY, sizeof(char)))) {
			CEL_TRACE("ms_calloc rdb evts error");
			CLOSE_CEL(fd);
			continue;
		}
		if (!READ_PTSZ(fd, evts, sizeof(char) * rhdr.chn_max * TOTAL_MINUTES_DAY)) {
			CEL_TRACE("failed read rec evt data\n");
			ms_free(evts);
			CLOSE_CEL(fd);
			continue;
		}
		CLOSE_CEL(fd);
		for (i = searchmin; i >= 0; i--) {
			for (k = 0; k < rhdr.chn_max; k++) {
				//if (ch_mask & ((unsigned long long)1 << k)) {
				if(ms_celmd_get_chmask_status(ch_mask, k)){
					offset = TOTAL_MINUTES_DAY * k;
					if (evts[offset + i] != 0) {
						CEL_TRACE("last record time: %ld ===offset:%ld i:%d k:%d event:%d==\n", (long)sec - (searchmin - i)* 60, offset, i, k, evts[offset+i]);
//						ms_free(evts);
//						return sec - (searchmin - i) * 60;
						if (res < sec - (searchmin - i) * 60)
							res = sec - (searchmin - i) * 60;
						flag = 1;
						break;
					}
				}
			}
			if (flag)
				break;
		}
		ms_free(evts);
	}
	return res;
}

/**
 * @brief 改进
 * @date 2013.10.11
 * @detail 文件在不同硬盘间搜索改进
 */
static long celmd_srch_next_play_time(T_CELMD_CTRL *celmd_ctrl)
{
#ifdef CEL_SYSIO_CALL
	int fd;
#else
	FILE *fd;
#endif

	int i = 0 , j = 0, k = 0, searchmin = 0, idx, flag;
	int recday = 0, curday = 0, minday = 0;
	long tmp, res = CEL_ERR;
	char *rec_tbl;
	T_RDB_HDR rhdr;
	T_CEL_TM tm1;
	//DIR *dir = NULL;
	struct dirent **ents = NULL;
	int nitems = 0;
	char fullpath[30];
	char tmppath[64] = {0};
	char tmpday[48] = {0};
	T_CEL_MNT_INFO hdd_info = {0};

	memset(&rhdr, 0, sizeof(rhdr));
	cel_get_local_time(celmd_ctrl->sec, &tm1);
	cel_get_hdd_info(&hdd_info);
	searchmin = tm1.hour*60 + tm1.min ;
	snprintf(tmpday, sizeof(tmpday), "%04d%02d%02d", tm1.year, tm1.mon, tm1.day);
	curday = atoi(tmpday);
	for(j = 0; j < MAX_HDD_COUNT; j++) {
		if(!hdd_info.hdd_info[j].is_mnt)
			continue;
		flag = 0;
		sprintf(fullpath, "%s/rdb", hdd_info.hdd_info[j].mnt_path);
		if ((nitems = scandir(fullpath, &ents, NULL, alphasort)) < 0) {
			CEL_TRACE("scandir error\n");
			continue;
		} else if (nitems == 0) {
			CEL_TRACE("there is no rdb items\n");
			ms_free(ents);
			continue;
		}
		for (idx = 0; idx < nitems; idx++) {
			if (ents[idx]->d_name[0] == '.') {
				if (!ents[idx]->d_name[1] || (ents[idx]->d_name[1] == '.' && !ents[idx]->d_name[2])) {//skip . and ..
					ms_free(ents[idx]);
					continue;
				}
			}
			recday = atoi(ents[idx]->d_name);
			if (recday >= curday) {
				snprintf(tmppath, sizeof(tmppath), "%s/%8d", fullpath, recday);
				if(!OPEN_RDONLY(fd, tmppath)) {
					ms_free(ents[idx]);
					continue;
				}
				if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr)) || (rhdr.chn_max <= 0 || rhdr.chn_max > MAX_CAMERA)) {
					ms_free(ents[idx]);
					CLOSE_CEL(fd);
					continue;
				}
				/*if (!(rhdr.ch_mask & celmd_ctrl->ch_mask)) {
					CEL_TRACE("current day has no record for ch_mask: %lld", celmd_ctrl->ch_mask);
					ms_free(ents[idx]);
					CLOSE_CEL(fd);
					continue;
				}*/
				//david.milesight
				int nCnt = 0, nmask = 0;
				for(nCnt = 0; nCnt < MAX_CH_MASK; nCnt++){
					if(nmask)
						break;
					if(celmd_ctrl->ch_mask.mask[nCnt]){
						if(rhdr.ch_mask.mask[nCnt] & celmd_ctrl->ch_mask.mask[nCnt]){
							if(nCnt == MAX_CH_MASK-1){
								nmask = 0;
							}		
						}else{
							nmask = 1;
						}
					}	
				}
				if(nmask){
					CEL_TRACE("current day has no record for ch_mask");
					ms_free(ents[idx]);
					CLOSE_CEL(fd);
					continue;
				}
				if (!(rec_tbl = (char *)ms_calloc(rhdr.chn_max * TOTAL_MINUTES_DAY, sizeof(char)))) {
					ms_free(ents[idx]);
					CLOSE_CEL(fd);
					continue;
				}
				if (!READ_PTSZ(fd, rec_tbl, sizeof(char) * rhdr.chn_max * TOTAL_MINUTES_DAY)) {
					ms_free(ents[idx]);
					ms_free(rec_tbl);
					CLOSE_CEL(fd);
					continue;
				}
				CLOSE_CEL(fd);
				if (recday == curday) {
					for (i = searchmin + 1; i < TOTAL_MINUTES_DAY; i++) {
						for (k = 0; k < rhdr.chn_max; k++) {
							//if (!(celmd_ctrl->ch_mask & ((unsigned long long)1 << k)))
							//david.milesight
							if(!ms_celmd_get_chmask_status(&celmd_ctrl->ch_mask, k))
								continue;
							if (0 != (int) rec_tbl[i + k * TOTAL_MINUTES_DAY]) {
//								for (; idx < nitems; idx++)
//									ms_free(ents[idx]);
//								ms_free(ents);
//								ms_free(rec_tbl);
								tmp = celmd_ctrl->sec + (i - searchmin) * 60;
								if (res == CEL_ERR || res > tmp)
									res = tmp;
								//CEL_TRACE(" Cur disk:%s first record time: %ld: %s\n",hdd_info.hdd_info[j].mnt_path ,celmd_ctrl->sec + (i - searchmin) * 60, ctime(&tmp));
//								return celmd_ctrl->sec + (i - searchmin) * 60;
								minday = curday;
								flag = 1;
								break;
							}
						}
						if (flag) // 在当前硬盘找到最小的有录像的时间
							break;
					}
				} else {
					if (minday && recday > minday) { //若已找到且下一天的时间比找到的时间大
						flag = 1;
						goto SEARCH_NEXT;
					}
					for (i = 0; i < TOTAL_MINUTES_DAY; i++) {
						for (k = 0; k < rhdr.chn_max; k++) {
							//if (!(celmd_ctrl->ch_mask & ((unsigned long long)1 << k)))
							//david.milesight
							if(!ms_celmd_get_chmask_status(&celmd_ctrl->ch_mask, k))
								continue;
						}
						if (0 != (int) rec_tbl[i + k * TOTAL_MINUTES_DAY]) {
//							for (; idx < nitems; idx++)
//								ms_free(ents[idx]);
//							ms_free(ents);
							struct tm tm2 = { 0 };
							tm2.tm_year = recday / 10000 - 1900;
							tm2.tm_mon = (recday % 10000) / 100 - 1;
							tm2.tm_mday = (recday % 10000) % 100;
							flag = 1;
							minday = recday;
							CEL_TRACE("first record time: year:%d, month: %d, day: %d\n",tm2.tm_year, tm2.tm_mon, tm2.tm_mday);
//							ms_free(rec_tbl);
//							return mktime(&tm2) + i * 60;
							tmp = mktime(&tm2) + i * 60;
							if (res == CEL_ERR || res > tmp)
								res = tmp;
							break;
						}
						if (flag)
							break;
					}
				}
				ms_free(rec_tbl);
				if (flag) { // 当前已找到，搜索下一块硬盘
SEARCH_NEXT:
					for (; idx < nitems; idx++)
						ms_free(ents[idx]);
//					ms_free(ents);
					break;
				} // if flag
			} // if recday >= curday
			ms_free(ents[idx]);
		} // for idx...
		ms_free(ents);
    }
	CEL_TRACE("res: %ld", res);
	return res;
}

static long celmd_srch_prev_play_time(T_CELMD_CTRL *celmd_ctrl)
{
#ifdef CEL_SYSIO_CALL
	int fd;
#else
	FILE *fd;
#endif

	int i = 0 , j = 0, k = 0, searchmin = 0, idx, flag = 0;
	int recday = 0, curday = 0, maxday = 0;
	long res = CEL_ERR, tmp;
//	char rec_tbl[TOTAL_MINUTES_DAY * CEL_MAX_CHANNEL] ;
	char *rec_tbl;
	T_RDB_HDR rhdr;
	T_CEL_TM tm1;
	//DIR *dir = NULL;
	struct dirent **ents = NULL;
	int nitems = 0;

	char fullpath[30];
	char tmppath[64] = {0};
	char tmpday[48] = {0};
	T_CEL_MNT_INFO hdd_info = {0};

	memset(&rhdr, 0, sizeof(rhdr));
	cel_get_local_time(celmd_ctrl->sec, &tm1);
	cel_get_hdd_info(&hdd_info);
	searchmin = tm1.hour*60 + tm1.min ;
	snprintf(tmpday, sizeof(tmpday), "%04d%02d%02d", tm1.year, tm1.mon, tm1.day);
	curday = atoi(tmpday);
	for(j = 0; j < MAX_HDD_COUNT; j++) {
		if(!hdd_info.hdd_info[j].is_mnt)
			continue;
		flag = 0;
		sprintf(fullpath, "%s/rdb", hdd_info.hdd_info[j].mnt_path);
		if ((nitems = scandir(fullpath, &ents, NULL, alphasort_reverse)) < 0) {
			CEL_TRACE("scandir error\n");
			continue;
		} else if (nitems == 0) {
			CEL_TRACE("there is no rdb items\n");
			ms_free(ents);
			continue;
		}
		for (idx = 0; idx < nitems; idx++) {
			if (ents[idx]->d_name[0] == '.') {
				if (!ents[idx]->d_name[1] || (ents[idx]->d_name[1] == '.' && !ents[idx]->d_name[2])) {//skip . and ..
					ms_free(ents[idx]);
					continue;
				}
			}
			recday = atoi(ents[idx]->d_name);
			if (recday <= curday) {
				snprintf(tmppath, sizeof(tmppath), "%s/%8d", fullpath, recday);
				if(!OPEN_RDONLY(fd, tmppath)) {
					ms_free(ents[idx]);
					continue;
				}

				if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr)) || (rhdr.chn_max <= 0 || rhdr.chn_max > MAX_CAMERA)) {
					ms_free(ents[idx]);
					CLOSE_CEL(fd);
					continue;
				}
				/*if (!(rhdr.ch_mask & celmd_ctrl->ch_mask)) {
					CEL_TRACE("current day has no record for ch_mask: %lld", celmd_ctrl->ch_mask);
					ms_free(ents[idx]);
					CLOSE_CEL(fd);
					continue;
				}*/
				//david.milesight
				int nCnt = 0, nmask = 0;
				for(nCnt = 0; nCnt < MAX_CH_MASK; nCnt++){
					if(nmask)
						break;
					if(celmd_ctrl->ch_mask.mask[nCnt]){
						if(rhdr.ch_mask.mask[nCnt] & celmd_ctrl->ch_mask.mask[nCnt]){
							if(nCnt == MAX_CH_MASK-1){
								nmask = 0;
							}		
						}else{
							nmask = 1;
						}
					}	
				}
				if(nmask){
					CEL_TRACE("current day has no record for ch_mask");
					ms_free(ents[idx]);
					CLOSE_CEL(fd);
					continue;
				}
				if (!(rec_tbl = (char *) ms_calloc(rhdr.chn_max * TOTAL_MINUTES_DAY, sizeof(char)))) {
					ms_free(ents[idx]);
					CLOSE_CEL(fd);
					continue;
				}
				if (!READ_PTSZ(fd, rec_tbl, sizeof(char) * rhdr.chn_max * TOTAL_MINUTES_DAY)) {
					ms_free(ents[idx]);
					ms_free(rec_tbl);
					CLOSE_CEL(fd);
					continue;
				}
				CLOSE_CEL(fd);
				if (recday == curday) {
					for (i = searchmin - 1; i >= 0 ; i--) {
						for (k = 0; k < rhdr.chn_max; k++) {
							//if (!(celmd_ctrl->ch_mask & ((unsigned long long)1 << k)))
							//david.mileisight 
							if(!ms_celmd_get_chmask_status(&celmd_ctrl->ch_mask, k))
								continue;
							if (0 != (int) rec_tbl[i + k * TOTAL_MINUTES_DAY]) {
//								for (; idx < nitems; idx++)
//									ms_free(ents[idx]);
//								ms_free(ents);
								tmp = celmd_ctrl->sec + (i - searchmin) * 60;
//								CEL_TRACE("first record time: %ld\n", tmp);
//								ms_free(rec_tbl);
								if (res == CEL_ERR || res < tmp)
									res = tmp;
								flag = 1;
								maxday = recday;
								break;
							}
						}
					}
				} else {
					if (maxday && recday < maxday) {
						flag = 1;
						goto SEARCH_NEXT;
					}
					for (i = TOTAL_MINUTES_DAY - 1; i >=0; i--) {
						for (k = 0; k < rhdr.chn_max; k++) {
							//if (!(celmd_ctrl->ch_mask & ((unsigned long long)1 << k)))
							//david.milesight
							if(!ms_celmd_get_chmask_status(&celmd_ctrl->ch_mask, k))
								continue;
						}
						if (0 != (int) rec_tbl[i + k * TOTAL_MINUTES_DAY]) {
//							for (; idx < nitems; idx++)
//								ms_free(ents[idx]);
//							ms_free(ents);
							struct tm tm2 = { 0 };
							tm2.tm_year = recday / 10000 - 1900;
							tm2.tm_mon = (recday % 10000) / 100 - 1;
							tm2.tm_mday = (recday % 10000) % 100;
							CEL_TRACE("first record time: year:%d, month: %d, day: %d\n",tm2.tm_year, tm2.tm_mon, tm2.tm_mday);
//							ms_free(rec_tbl);
//							return mktime(&tm2) + i * 60;
							tmp = mktime(&tm2) + i * 60;
							maxday = recday;
							flag = 1;
							if (res == CEL_ERR || res < tmp)
								res = tmp;
							break;
						}
					}
				}
				ms_free(rec_tbl);
				if (flag) {
SEARCH_NEXT:
					for (; idx < nitems; idx++)
						ms_free(ents[idx]);
					break;
				}
			}
			ms_free(ents[idx]);
		}//for (idx = 0; idx < nitems; idx++)
		ms_free(ents);
    }//for(j = 0; j < MAX_HDD_COUNT; j++)
	CEL_TRACE("prev play time: %ld", res);
	return res;
}
/**
 * @brief 当播放通道一分钟内都没有录像信息时，跳转至有录像的时间
 */
long celmd_jmp2next(T_CELMD_CTRL *celmd_ctrl, int direct)
{
	long rv;
	if (direct == CEL_FORWARD)
		rv = celmd_srch_next_play_time(celmd_ctrl);
	else if (direct == CEL_BACKWARD)
		rv = celmd_srch_prev_play_time(celmd_ctrl);
	if (rv == CEL_ERR) {
		CEL_TRACE("error\n");
		return CEL_ERR;
	}
	if (celmd_get_tar_disk(celmd_ctrl, &celmd_ctrl->ch_mask, rv) != CEL_OK) {
		CEL_TRACE("md get target disk error\n");
		return CEL_ERR;
	}
	celmd_close(celmd_ctrl, TRUE);
	return celmd_open(celmd_ctrl, &celmd_ctrl->ch_mask, rv, direct, g_max_camera);
}

int celmd_get_rec_file_cnt(T_CELMD_CTRL *celmd_ctrl, CH_MASK* ch_mask, long t1, long t2)
{
	int ret = 0, pathcnt = 0 ;
    int cel_cnt=0;

    char path1[LEN_PATH];
    char path2[LEN_PATH];

	char pathbuf[LEN_PATH] ;

	ret = celmd_get_tar_disk(celmd_ctrl, ch_mask, t1) ;
    T_CEL_TM tm1;
    cel_get_local_time(t1, &tm1);

	if(ret != CEL_OK)
		return CEL_ERR;

    sprintf(path1, "%s/rdb/%04d%02d%02d", celmd_ctrl->target_path, tm1.year, tm1.mon, tm1.day);

    T_CEL_TM tm2;
    cel_get_local_time(t2, &tm2);

	ret = celmd_get_tar_disk(celmd_ctrl, ch_mask, t2) ;
	if(ret != CEL_OK)
		return CEL_ERR;

    sprintf(path2, "%s/rdb/%04d%02d%02d", celmd_ctrl->target_path, tm2.year, tm2.mon, tm2.day);

    if(strcmp(path1, path2) != 0)
    {
        CEL_TRACE("different from-date:%s and to-date:%s\n", path1, path2);
		pathcnt = 2 ;
	}
	else
	{
		pathcnt = 1 ;
	}
	sprintf(pathbuf, "%s", path1) ;

    if(-1 == access(pathbuf, 0)) // existence only
    {
       	CEL_TRACE("There is no rdb file. %s\n", pathbuf);
       	return CEL_ERR;
    }

#ifdef CEL_SYSIO_CALL
    int fd;
#else
    FILE *fd;
#endif

    if(!OPEN_RDONLY(fd, pathbuf))
    {
       	CEL_TRACE("failed open rdb %s\n", pathbuf);
       	return CEL_ERR;
   	}

//    char evts[TOTAL_MINUTES_DAY*CEL_MAX_CHANNEL];
//    T_CEL_RDB rdbs[TOTAL_MINUTES_DAY*CEL_MAX_CHANNEL];
    T_CEL_RDB *rdbs;
    T_RDB_HDR rhdr;
	memset(&rhdr, 0, sizeof(rhdr));

	if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr)) || (rhdr.chn_max <= 0 || rhdr.chn_max > MAX_CAMERA)) {
		CLOSE_CEL(fd);
		return CEL_ERR;
	}

	if(!ms_celmd_masks_cmp(&rhdr.ch_mask, ch_mask)){
		CEL_TRACE("current day has no record for ch_mask");
		CLOSE_CEL(fd);
		return CEL_ERR;
	}
	if (!LSEEK_SET(fd, rhdr.chn_max * TOTAL_MINUTES_DAY *sizeof(char) + sizeof(rhdr))) {
		CEL_TRACE("seek to rdb error");
		return CEL_ERR;
	}

	if (!(rdbs = (T_CEL_RDB *)ms_calloc(rhdr.chn_max * TOTAL_MINUTES_DAY, sizeof(T_CEL_RDB)))) {
		CLOSE_CEL(fd);
		return CEL_ERR;
	}
    if(!READ_PTSZ(fd, rdbs, rhdr.chn_max * TOTAL_MINUTES_DAY *sizeof(T_CEL_RDB)))
   	{
       	CEL_TRACE("failed read rdbs data\n");
       	CLOSE_CEL(fd);
       	ms_free(rdbs);
       	return CEL_ERR;
   	}
   	CLOSE_CEL(fd);

   	long prev_cid=0;
   	int m, c;

	if(pathcnt == 1)
	{
   		for(m=tm1.hour*60+tm1.min ; m<=tm2.hour*60+tm2.min ; m++)
   		{
       		for(c = 0 ; c < rhdr.chn_max ; c++)
       		{
           		/*if(rdbs[TOTAL_MINUTES_DAY*c+m].cid != 0
           	    	&& prev_cid != rdbs[TOTAL_MINUTES_DAY*c+m].cid
           	    	&& (ch_mask & ((unsigned long long)1 << c)) == 1)*/
           	    //david.mileisight
           	    if(rdbs[TOTAL_MINUTES_DAY*c+m].cid != 0
           	    	&& prev_cid != rdbs[TOTAL_MINUTES_DAY*c+m].cid
           	    	&& ms_celmd_get_chmask_status(ch_mask, c))
           		{
               		cel_cnt++;
               		prev_cid = rdbs[TOTAL_MINUTES_DAY*c+m].cid;
               		break;
           		}
      	 	}
   		}
	}
	else ///< @todo bug
	{
		m = tm1.hour*60+tm1.min ;
		while(1)
		{
			for(c = 0 ; c < rhdr.chn_max ; c ++)
            {
                /*if(rdbs[TOTAL_MINUTES_DAY*c+m].cid != 0
                    && prev_cid != rdbs[TOTAL_MINUTES_DAY*c+m].cid
                    && ((ch_mask & (unsigned long long)(1 << c)) == 1))*/
             	//david.milesight
             	if(rdbs[TOTAL_MINUTES_DAY*c+m].cid != 0
                    && prev_cid != rdbs[TOTAL_MINUTES_DAY*c+m].cid
                    && ms_celmd_get_chmask_status(ch_mask, c))
                {
                    cel_cnt++;
                    prev_cid = rdbs[TOTAL_MINUTES_DAY*c+m].cid;
                    break;
                }
				else if(rdbs[TOTAL_MINUTES_DAY*c+m].cid == 0)
				{
    				CEL_TRACE("get backup file count on HDD end :%d\n", cel_cnt);
    				ms_free(rdbs);
					return cel_cnt ;
				}
            }
			m++ ;
		}
	}

    CEL_TRACE("get backup file count :%d\n", cel_cnt);
    ms_free(rdbs);
    return cel_cnt;
}

int celmd_get_frame_sizes(CEL_MULDEC_CTRL *celmd_ctrl, T_CELMD_PARAM *dp, long endtime)
{
	T_STREAM_HDR shd;
	T_VIDEO_STREAM_HDR vhd;
	//T_AUDIO_STREAM_HDR ahd;
	int movilen = 4;
	while (celmd_ctrl->cel_status == CELMD_STATUS_OPEN) {
		// read stream header
		if (!READ_HDR(celmd_ctrl->hcel, shd)) {
//			perror("read stream header");
			if (celmd_read_next_iframe(celmd_ctrl, dp) != CEL_OK) {
				CEL_TRACE(
						"failed read stream header. go next iframe. shd.id:0x%08lx, frm_size:%ld, cur-iframe:%d, tot-iframe:%d\n",
						shd.id, shd.frm_size, celmd_ctrl->iframe_cur,
						celmd_ctrl->iframe_cnt);
				break;
			}
			shd.frm_size = dp->frm_size;
		}
		if (shd.id == ID_CEL_END) // Normal sequence closed cellular
		{
			if (celmd_read_next_first_frame(celmd_ctrl, dp) != CEL_OK){
				break;// reading any FirstFrame on next cellular
			}
		} else {
			if (shd.frm_type == FT_IFRAME) {
				celmd_ctrl->iframe_cur += 1;
			}
			//if (!(celmd_ctrl->ch_mask & ((unsigned long long) 1 << shd.ch))) {
			//david.milesight
			if(ms_celmd_get_chmask_status(&celmd_ctrl->ch_mask, shd.ch)){
				if (!LSEEK_CUR(celmd_ctrl->hcel, sizeof(vhd) + shd.frm_size)){
					if (celmd_read_next_iframe(celmd_ctrl,dp) != CEL_OK) {
						break;
					}
					shd.frm_size = dp->frm_size;
				}
				continue;
			} else {
				celmd_ctrl->chns_sec[shd.ch] = shd.ts.sec;
				if (shd.ts.sec > endtime){
					break;
				}
			}
		}
		celmd_ctrl->fpos_cel = LTELL(celmd_ctrl->hcel);
		movilen += 8 + shd.frm_size;
		if(shd.frm_size % 2) movilen += 1;

		if (!LSEEK_CUR(celmd_ctrl->hcel, sizeof(vhd) + shd.frm_size))
			break;
	}

	return movilen;
}

void celmd_cpfrm2dst(T_CELMD_PARAM *src, struct reco_frame *dst)
{
	unsigned long long cap_time;
	dst->achn = src->achn;
	dst->ch = (int)(src->ch);
	dst->codec_type = src->codec_type;
	dst->bps = src->bps;
	dst->frame_rate = src->frm_rate;
	dst->frame_type = src->frm_type;
	dst->sample = src->smp_rate;
	dst->height = src->frm_height;
	dst->size = src->frm_size;
	dst->strm_type = src->strm_type;
	dst->stream_format = src->strm_fmt;
	dst->width = src->frm_width;
	dst->time_usec = (unsigned long long) (src->ts.sec) * 1000000 + src->ts.usec;
	if (!src->time_lower) {
		cap_time = (unsigned long long)(src->ts.sec * 1000) + src->ts.usec / 1000;
		dst->time_lower = (unsigned int) (cap_time & 0xffffffff);
		dst->time_upper = (unsigned int) (cap_time >> 32);
	} else {
		dst->time_lower = src->time_lower;
		dst->time_upper = src->time_upper;
	}
	dst->data = src->vbuffer;
}

//david modify 2015-09-10 start 
int ms_celmd_get_rec_mins(int ch, long sec, char *recmin_tbl, long curdaysec, int expdatecnt)
{
	//printf("=====david test====ms_celmd_get_rec_mins curdaysec:%ld, expdatecnt:%d===0000=====\n", curdaysec, expdatecnt);
#ifdef CEL_SYSIO_CALL
	int fd;
#else
	FILE *fd;
#endif
	T_CEL_TM tm1, tm2;
	T_RDB_HDR rhdr;
	char path[LEN_PATH];
	T_CEL_MNT_INFO hdd_info = {0};
	int i, j;
	char tmp_rec[TOTAL_MINUTES_DAY] = {0};
	memset(&rhdr, 0, sizeof(rhdr));
	
	cel_get_local_time_ex(sec, &tm1);
	cel_get_hdd_info(&hdd_info);
	memset(recmin_tbl, 0, TOTAL_MINUTES_DAY);

	float tz_hour = get_tz_hour();
	int dst = get_dst_hour(sec);	
	if(dst != 1)
		dst = 0;
	int expdateflag = 0; 		//david modify 2015-09-11; 
	CEL_TRACE("gui time %04d%02d%02d %02d:%02d:%02d===tz_hour:%f dst:%d\n", tm1.year, tm1.mon, tm1.day, tm1.hour, tm1.min, tm1.sec, tz_hour, dst);
	for (i = 0; i < MAX_HDD_COUNT; i++) 
	{
#ifndef RDB_UTC	
		if (hdd_info.hdd_info[i].is_mnt)
		{
			//printf("=====david test====ms_celmd_get_rec_mins===1111=====\n");
			snprintf(path, sizeof(path), "%s/rdb/%04d%02d%02d", hdd_info.hdd_info[i].mnt_path, tm1.year, tm1.mon, tm1.day);
			CEL_TRACE("open rdb path:%s===tz_hour:%f\n", path, tz_hour);
			long offset = TOTAL_MINUTES_DAY*ch + sizeof(rhdr);
			//if(!OPEN_RDONLY(fd, path))    //david modify 2015-09-10
			if(!OPEN_RDWR(fd,path))
			{
				CEL_TRACE("failed open rdb %s\n", path);
				continue;
			}
			if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr))) {
				CEL_TRACE("read rdb header error");
				CLOSE_CEL(fd);
				continue;
			}
			if (ch >= rhdr.chn_max) {
				CLOSE_CEL(fd);
				CEL_TRACE("invalid chn:%d, chn_max: %d", ch, rhdr.chn_max);
				continue;
			}
			if(!LSEEK_SET(fd, offset))
			{
				CEL_TRACE("failed seek rdb.\n");
				CLOSE_CEL(fd);
				continue;
			}

			if(!READ_PTSZ(fd, tmp_rec, TOTAL_MINUTES_DAY))
			{
				CEL_TRACE("failed read rdb CH:%d, offset:%ld\n", ch, offset);
				CLOSE_CEL(fd);
				continue;
			}
			//david modify 2015-09-10
			if((curdaysec-expdatecnt*86400)>sec){  
				memset(tmp_rec, 0, sizeof(tmp_rec));
				if(!LSEEK_SET(fd, offset))
				{
					CEL_TRACE("failed seek rdb.\n");
					CLOSE_CEL(fd);
					continue;
				}
				if(!WRITE_PTSZ(fd, tmp_rec, TOTAL_MINUTES_DAY))
				{
					CEL_TRACE("failed write rdb CH:%d, offset:%ld\n", ch, offset);
					CLOSE_CEL(fd);
					continue;
				}
			}
			//////////
			for (j = 0; j < TOTAL_MINUTES_DAY; j++)
			{
				recmin_tbl[j] |= (tmp_rec[j]);//bruce.milesight modify version:7.0.4
			}
			CLOSE_CEL(fd);
		}
#else	
		if(hdd_info.hdd_info[i].is_mnt && (!tz_hour))
		{
			do{
				if(dst)
				{
					cel_get_local_time_ex(sec-86400, &tm2);
					snprintf(path, sizeof(path), "%s/rdb/%04d%02d%02d", hdd_info.hdd_info[i].mnt_path, tm2.year, tm2.mon, tm2.day);
					CEL_TRACE("open pre_rdb path:%s===tz_hour:%f\n", path, tz_hour);
					long offset = TOTAL_MINUTES_DAY*ch + sizeof(rhdr);
					//if(!OPEN_RDONLY(fd, path))    //david modify 2015-09-10
					if(!OPEN_RDWR(fd,path))
					{
						CEL_TRACE("failed open rdb %s\n", path);
						continue;
					}
					if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr))) {
						CEL_TRACE("read rdb header error");
						CLOSE_CEL(fd);
						continue;
					}
					if (ch >= rhdr.chn_max) {
						CLOSE_CEL(fd);
						CEL_TRACE("invalid chn:%d, chn_max: %d", ch, rhdr.chn_max);
						continue;
					}
					if(!LSEEK_SET(fd, offset))
					{
						CEL_TRACE("failed seek rdb.\n");
						CLOSE_CEL(fd);
						continue;
					}
					
					if(!READ_PTSZ(fd, tmp_rec, TOTAL_MINUTES_DAY))
					{
						CEL_TRACE("failed read rdb CH:%d, offset:%ld\n", ch, offset);
						CLOSE_CEL(fd);
						continue;
					}
					//david modify 2015-09-10
					if((curdaysec-expdatecnt*86400)>sec){
						expdateflag = 1;
					}
					//////////
					for (j = 0; j < 60*dst; j++)
					{
						if(expdateflag){					//david modify 2015-09-11
							tmp_rec[TOTAL_MINUTES_DAY-60*dst+j] = 0;
						}
						recmin_tbl[j] |= (tmp_rec[TOTAL_MINUTES_DAY-60*dst+j]);//bruce.milesight modify version:7.0.4
						/*if(tmp_rec[TOTAL_MINUTES_DAY-60*dst+j] && (g_sec_res > (sec-86400+(TOTAL_MINUTES_DAY-60*dst+j)*60) || !g_sec_res))
						{
							g_sec_res = sec-86400+(TOTAL_MINUTES_DAY-60*dst+j)*60;	
							g_cur_hdd = i;
							printf("1111 g_sec_res:%d index:%d\n", g_sec_res, TOTAL_MINUTES_DAY-60*dst+j);
						}*/
					}
					if(expdateflag){						//david modify 2015-09-11
						expdateflag = 0;
						if(!LSEEK_SET(fd, offset))
						{
							CEL_TRACE("failed seek rdb.\n");
							CLOSE_CEL(fd);
							continue;
						}
						if(!WRITE_PTSZ(fd, tmp_rec, TOTAL_MINUTES_DAY))
						{
							CEL_TRACE("failed write rdb CH:%d, offset:%ld\n", ch, offset);
							CLOSE_CEL(fd);
							continue;
						}
					}
					CLOSE_CEL(fd);
				}
			}while(0);
			
			snprintf(path, sizeof(path), "%s/rdb/%04d%02d%02d", hdd_info.hdd_info[i].mnt_path, tm1.year, tm1.mon, tm1.day);
			//CEL_TRACE("open rdb path:%s===tz_hour:%f\n", path, tz_hour);
			long offset = TOTAL_MINUTES_DAY*ch + sizeof(rhdr);
			//if(!OPEN_RDONLY(fd, path))    //david modify 2015-09-10
			if(!OPEN_RDWR(fd,path))
			{
				CEL_TRACE("failed open rdb %s\n", path);
				continue;
			}
			if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr))) {
				CEL_TRACE("read rdb header error");
				CLOSE_CEL(fd);
				continue;
			}
			if (ch >= rhdr.chn_max) {
				CLOSE_CEL(fd);
				CEL_TRACE("invalid chn:%d, chn_max: %d", ch, rhdr.chn_max);
				continue;
			}
			if(!LSEEK_SET(fd, offset))
			{
				CEL_TRACE("failed seek rdb.\n");
				CLOSE_CEL(fd);
				continue;
			}

			if(!READ_PTSZ(fd, tmp_rec, TOTAL_MINUTES_DAY))
			{
				CEL_TRACE("failed read rdb CH:%d, offset:%ld\n", ch, offset);
				CLOSE_CEL(fd);
				continue;
			}
			//david modify 2015-09-10
			if((curdaysec-expdatecnt*86400)>sec){  
				expdateflag = 1;			
			}
			//////////
			for (j = 0; j < TOTAL_MINUTES_DAY-60*dst; j++)
			{
				if(expdateflag){					//david modify 2015-09-11
					tmp_rec[j] = 0;
				}
				recmin_tbl[j+60*dst] |= (tmp_rec[j]);//bruce.milesight modify version:7.0.4
				/*if(tmp_rec[j] && (g_sec_res > (sec+j*60) || !g_sec_res))
				{
					g_sec_res = sec+j*60;
					g_cur_hdd = i;
					printf("0000 g_sec_res:%d index:%d\n", g_sec_res, j);
				}*/
			}
			if(expdateflag){					  //david modify 2015-09-11
				expdateflag = 0;
				if(!LSEEK_SET(fd, offset))
				{
					CEL_TRACE("failed seek rdb.\n");
					CLOSE_CEL(fd);
					continue;
				}
				if(!WRITE_PTSZ(fd, tmp_rec, TOTAL_MINUTES_DAY))
				{
					CEL_TRACE("failed write rdb CH:%d, offset:%ld\n", ch, offset);
					CLOSE_CEL(fd);
					continue;
				}
			}
			
			CLOSE_CEL(fd);
		}
		else if(hdd_info.hdd_info[i].is_mnt)
		{
			struct tm tm_cur = {0};
			tm_cur.tm_year = tm1.year-1900;
			tm_cur.tm_mon = tm1.mon-1;
			tm_cur.tm_mday = tm1.day;
			tm_cur.tm_hour = 0;//tz_hour;
			tm_cur.tm_min = tm_cur.tm_sec = 0;
			time_t sec_cur = mktime(&tm_cur);
			struct tm tm_start = {0};struct tm tm_end = {0};
			gmtime_r(&sec_cur, &tm_start);			
			
			CEL_TRACE("start:%04d%02d%02d %02d:%02d:%02d\n", tm_start.tm_year+1900, tm_start.tm_mon+1, tm_start.tm_mday,
				tm_start.tm_hour, tm_start.tm_min, tm_start.tm_sec);

			{
				snprintf(path, sizeof(path), "%s/rdb/%04d%02d%02d", hdd_info.hdd_info[i].mnt_path, tm_start.tm_year+1900, tm_start.tm_mon+1, tm_start.tm_mday);
				//CEL_TRACE("open rdb path:%s==1111==tz_hour:%f\n", path, tz_hour);
				long offset = TOTAL_MINUTES_DAY*ch + sizeof(rhdr);
				//if(!OPEN_RDONLY(fd, path))	//david modify 2015-09-10
				if(!OPEN_RDWR(fd,path))
				{
					CEL_TRACE("failed open rdb %s\n", path);
					goto STEP_2;
				}
				if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr))) {
					CEL_TRACE("read rdb header error");
					CLOSE_CEL(fd);
					goto STEP_2;
				}
				if (ch >= rhdr.chn_max) {
					CLOSE_CEL(fd);
					CEL_TRACE("invalid chn:%d, chn_max: %d", ch, rhdr.chn_max);
					goto STEP_2;
				}
				if(!LSEEK_SET(fd, offset))
				{
					CEL_TRACE("failed seek rdb.\n");
					CLOSE_CEL(fd);
					goto STEP_2;
				}
				if(!READ_PTSZ(fd, tmp_rec, TOTAL_MINUTES_DAY))
				{
					CEL_TRACE("failed read rdb CH:%d, offset:%ld\n", ch, offset);
					CLOSE_CEL(fd);
					goto STEP_2;
				}
				//david modify 2015-09-10
				if((curdaysec-expdatecnt*86400)>sec){
					expdateflag = 1;
				}
				//////////
				int start_min = tm_start.tm_hour*60 + tm_start.tm_min-60*dst;
				//CEL_TRACE("1111 start_min:%d\n", start_min);
				for(j = start_min; j < TOTAL_MINUTES_DAY; j++)
				{
					if(expdateflag){		//david modify 2015-09-11
						tmp_rec[j] = 0;
					}
					recmin_tbl[(j-start_min)] |= tmp_rec[j];
					/*if(tmp_rec[j] && (g_sec_res > (sec_cur+j*60) || !g_sec_res))
					{
						g_sec_res = sec_cur+j*60;
						g_cur_hdd = i;
						printf("2222 g_sec_res:%d index:%d\n", g_sec_res, j);
					}*/
				}
				if(expdateflag){			//david modify 2015-09-11
					expdateflag = 0;
					if(!LSEEK_SET(fd, offset))
					{
						CEL_TRACE("failed seek rdb.\n");
						CLOSE_CEL(fd);
						goto STEP_2;
					}
					if(!WRITE_PTSZ(fd, tmp_rec, TOTAL_MINUTES_DAY))
					{
						CEL_TRACE("failed write rdb CH:%d, offset:%ld\n", ch, offset);
						CLOSE_CEL(fd);
						goto STEP_2;
					}
				}
				CLOSE_CEL(fd);
			}
STEP_2: 	
			sec_cur += 86440;// 24 *3600
			gmtime_r(&sec_cur, &tm_end);			
			CEL_TRACE("end:%04d%02d%02d %02d:%02d:%02d\n", tm_end.tm_year+1900, tm_end.tm_mon+1, tm_end.tm_mday,
				tm_end.tm_hour, tm_end.tm_min, tm_end.tm_sec);

			{
				snprintf(path, sizeof(path), "%s/rdb/%04d%02d%02d", hdd_info.hdd_info[i].mnt_path, tm_end.tm_year+1900, tm_end.tm_mon+1, tm_end.tm_mday);
				CEL_TRACE("open rdb path:%s==2222==tz_hour:%f\n", path, tz_hour);
				long offset = TOTAL_MINUTES_DAY*ch + sizeof(rhdr);
				//if(!OPEN_RDONLY(fd, path)) 	//david modify 2015-09-10 
				if(!OPEN_RDWR(fd,path))
				{
					CEL_TRACE("failed open rdb %s\n", path);
					continue;
				}
				if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr))) {
					CEL_TRACE("read rdb header error");
					CLOSE_CEL(fd);
					continue;
				}
				if (ch >= rhdr.chn_max) {
					CLOSE_CEL(fd);
					CEL_TRACE("invalid chn:%d, chn_max: %d", ch, rhdr.chn_max);
					continue;
				}
				if(!LSEEK_SET(fd, offset))
				{
					CEL_TRACE("failed seek rdb.\n");
					CLOSE_CEL(fd);
					continue;
				}

				if(!READ_PTSZ(fd, tmp_rec, TOTAL_MINUTES_DAY))
				{
					CEL_TRACE("failed read rdb CH:%d, offset:%ld\n", ch, offset);
					CLOSE_CEL(fd);
					continue;
				}
				//david modify 2015-09-10
				if((curdaysec-expdatecnt*86400)>sec){
					expdateflag = 1;
				}
				//////////
				int start_min = tm_start.tm_hour*60 + tm_start.tm_min-60*dst;
				//CEL_TRACE("2222 start_min:%d\n", start_min);
				for(j = 0; j < start_min; j++)
				{
					if(expdateflag){		//david modify 2015-09-11
						tmp_rec[j] = 0;
					}
					recmin_tbl[(j+TOTAL_MINUTES_DAY-start_min)] |= tmp_rec[j];		
					/*if(tmp_rec[j] && (g_sec_res > (sec_cur+j*60) || !g_sec_res))
					{
						g_sec_res = sec_cur+j*60;
						g_cur_hdd = i;
						printf("3333 g_sec_res:%d index:%d\n", g_sec_res, j);
					}*/
				}
				if(expdateflag){			//david modify 2015-09-11
					expdateflag = 0;
					if(!LSEEK_SET(fd, offset))
					{
						CEL_TRACE("failed seek rdb.\n");
						CLOSE_CEL(fd);
						continue;
					}
					if(!WRITE_PTSZ(fd, tmp_rec, TOTAL_MINUTES_DAY))
					{
						CEL_TRACE("failed write rdb CH:%d, offset:%ld\n", ch, offset);
						CLOSE_CEL(fd);
						continue;
					}
				}
				CLOSE_CEL(fd);
			}		
		}
#endif			
	}

//debug
#if 0
	for(i = 0; i < TOTAL_MINUTES_DAY; i++)
	{
		if(recmin_tbl[i])
		{
			printf("%d ", i);
		}
	}	
	CEL_TRACE("\n");
#endif
	return 0;
}

int ms_celmd_get_rec_mins_ex(int ch, long sec, /*char *recmin_tbl,*/ long curdaysec, int expdatecnt)
{
#ifdef CEL_SYSIO_CALL
	int fd;
#else
	FILE *fd;
#endif
	T_CEL_TM tm1, tm2;
	T_RDB_HDR rhdr;
	char path[LEN_PATH];
	T_CEL_MNT_INFO hdd_info = {0};
	int i, j;
	char tmp_rec[TOTAL_MINUTES_DAY] = {0};
	memset(&rhdr, 0, sizeof(rhdr));
	
	cel_get_local_time_ex(sec, &tm1);
	cel_get_hdd_info(&hdd_info);
	//memset(recmin_tbl, 0, TOTAL_MINUTES_DAY);

	float tz_hour = get_tz_hour();
	int dst = get_dst_hour(sec);
	if(dst != 1)
		dst = 0;
			
	int expdateflag = 0; 		//david modify 2015-09-11; 
	CEL_TRACE("gui time %04d%02d%02d %02d:%02d:%02d===tz_hour:%f dst:%d\n", tm1.year, tm1.mon, tm1.day, tm1.hour, tm1.min, tm1.sec, tz_hour, dst);
	for (i = 0; i < MAX_HDD_COUNT; i++) 
	{
#ifndef RDB_UTC	
		if (hdd_info.hdd_info[i].is_mnt)
		{
			snprintf(path, sizeof(path), "%s/rdb/%04d%02d%02d", hdd_info.hdd_info[i].mnt_path, tm1.year, tm1.mon, tm1.day);
			CEL_TRACE("open rdb path:%s===tz_hour:%f\n", path, tz_hour);
			long offset = TOTAL_MINUTES_DAY*ch + sizeof(rhdr);
			//if(!OPEN_RDONLY(fd, path))    //david modify 2015-09-10
			if(!OPEN_RDWR(fd,path))
			{
				CEL_TRACE("failed open rdb %s\n", path);
				continue;
			}
			if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr))) {
				CEL_TRACE("read rdb header error");
				CLOSE_CEL(fd);
				continue;
			}
			if (ch >= rhdr.chn_max) {
				CLOSE_CEL(fd);
				CEL_TRACE("invalid chn:%d, chn_max: %d", ch, rhdr.chn_max);
				continue;
			}
			if(!LSEEK_SET(fd, offset))
			{
				CEL_TRACE("failed seek rdb.\n");
				CLOSE_CEL(fd);
				continue;
			}

			if(!READ_PTSZ(fd, tmp_rec, TOTAL_MINUTES_DAY))
			{
				CEL_TRACE("failed read rdb CH:%d, offset:%ld\n", ch, offset);
				CLOSE_CEL(fd);
				continue;
			}
			//david modify 2015-09-10
			if((curdaysec-expdatecnt*86400)>sec){  
				memset(tmp_rec, 0, sizeof(tmp_rec));
				if(!LSEEK_SET(fd, offset))
				{
					CEL_TRACE("failed seek rdb.\n");
					CLOSE_CEL(fd);
					continue;
				}
				if(!WRITE_PTSZ(fd, tmp_rec, TOTAL_MINUTES_DAY))
				{
					CEL_TRACE("failed write rdb CH:%d, offset:%ld\n", ch, offset);
					CLOSE_CEL(fd);
					continue;
				}
			}
			//////////
			//for (j = 0; j < TOTAL_MINUTES_DAY; j++)
			//{
			//	recmin_tbl[j] |= (tmp_rec[j]);//bruce.milesight modify version:7.0.4
			//}
			CLOSE_CEL(fd);
		}
#else	
		if(hdd_info.hdd_info[i].is_mnt && (!tz_hour))
		{
			do{
				if(dst)
				{
					cel_get_local_time_ex(sec-86400, &tm2);
					snprintf(path, sizeof(path), "%s/rdb/%04d%02d%02d", hdd_info.hdd_info[i].mnt_path, tm2.year, tm2.mon, tm2.day);
					CEL_TRACE("open pre_rdb path:%s===tz_hour:%f\n", path, tz_hour);
					long offset = TOTAL_MINUTES_DAY*ch + sizeof(rhdr);
					//if(!OPEN_RDONLY(fd, path))    //david modify 2015-09-10
					if(!OPEN_RDWR(fd,path))
					{
						CEL_TRACE("failed open rdb %s\n", path);
						continue;
					}
					if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr))) {
						CEL_TRACE("read rdb header error");
						CLOSE_CEL(fd);
						continue;
					}
					if (ch >= rhdr.chn_max) {
						CLOSE_CEL(fd);
						CEL_TRACE("invalid chn:%d, chn_max: %d", ch, rhdr.chn_max);
						continue;
					}
					if(!LSEEK_SET(fd, offset))
					{
						CEL_TRACE("failed seek rdb.\n");
						CLOSE_CEL(fd);
						continue;
					}
					
					if(!READ_PTSZ(fd, tmp_rec, TOTAL_MINUTES_DAY))
					{
						CEL_TRACE("failed read rdb CH:%d, offset:%ld\n", ch, offset);
						CLOSE_CEL(fd);
						continue;
					}
					//david modify 2015-09-10
					if((curdaysec-expdatecnt*86400)>sec){
						expdateflag = 1;
					}
					//////////
					for (j = 0; j < 60*dst; j++)
					{
						if(expdateflag){					//david modify 2015-09-11
							tmp_rec[TOTAL_MINUTES_DAY-60*dst+j] = 0;
						}
						//recmin_tbl[j] |= (tmp_rec[TOTAL_MINUTES_DAY-60*dst+j]);//bruce.milesight modify version:7.0.4
						/*if(tmp_rec[TOTAL_MINUTES_DAY-60*dst+j] && (g_sec_res > (sec-86400+(TOTAL_MINUTES_DAY-60*dst+j)*60) || !g_sec_res))
						{
							g_sec_res = sec-86400+(TOTAL_MINUTES_DAY-60*dst+j)*60;	
							g_cur_hdd = i;
							printf("1111 g_sec_res:%d index:%d\n", g_sec_res, TOTAL_MINUTES_DAY-60*dst+j);
						}*/
					}
					if(expdateflag){						//david modify 2015-09-11
						expdateflag = 0;
						if(!LSEEK_SET(fd, offset))
						{
							CEL_TRACE("failed seek rdb.\n");
							CLOSE_CEL(fd);
							continue;
						}
						if(!WRITE_PTSZ(fd, tmp_rec, TOTAL_MINUTES_DAY))
						{
							CEL_TRACE("failed write rdb CH:%d, offset:%ld\n", ch, offset);
							CLOSE_CEL(fd);
							continue;
						}
					}
					CLOSE_CEL(fd);
				}
			}while(0);
			snprintf(path, sizeof(path), "%s/rdb/%04d%02d%02d", hdd_info.hdd_info[i].mnt_path, tm1.year, tm1.mon, tm1.day);
			//CEL_TRACE("open rdb path:%s===tz_hour:%f\n", path, tz_hour);
			long offset = TOTAL_MINUTES_DAY*ch + sizeof(rhdr);
			//if(!OPEN_RDONLY(fd, path))    //david modify 2015-09-10
			if(!OPEN_RDWR(fd,path))
			{
				CEL_TRACE("failed open rdb %s\n", path);
				continue;
			}
			if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr))) {
				CEL_TRACE("read rdb header error");
				CLOSE_CEL(fd);
				continue;
			}
			if (ch >= rhdr.chn_max) {
				CLOSE_CEL(fd);
				CEL_TRACE("invalid chn:%d, chn_max: %d", ch, rhdr.chn_max);
				continue;
			}
			if(!LSEEK_SET(fd, offset))
			{
				CEL_TRACE("failed seek rdb.\n");
				CLOSE_CEL(fd);
				continue;
			}

			if(!READ_PTSZ(fd, tmp_rec, TOTAL_MINUTES_DAY))
			{
				CEL_TRACE("failed read rdb CH:%d, offset:%ld\n", ch, offset);
				CLOSE_CEL(fd);
				continue;
			}
			//david modify 2015-09-10
			if((curdaysec-expdatecnt*86400)>sec){  
				expdateflag = 1;			
			}
			//////////
			for (j = 0; j < TOTAL_MINUTES_DAY-60*dst; j++)
			{
				if(expdateflag){					//david modify 2015-09-11
					tmp_rec[j] = 0;
				}
				//recmin_tbl[j+60*dst] |= (tmp_rec[j]);//bruce.milesight modify version:7.0.4
				/*if(tmp_rec[j] && (g_sec_res > (sec+j*60) || !g_sec_res))
				{
					g_sec_res = sec+j*60;
					g_cur_hdd = i;
					printf("0000 g_sec_res:%d index:%d\n", g_sec_res, j);
				}*/
			}
			if(expdateflag){					  //david modify 2015-09-11
				expdateflag = 0;
				if(!LSEEK_SET(fd, offset))
				{
					CEL_TRACE("failed seek rdb.\n");
					CLOSE_CEL(fd);
					continue;
				}
				if(!WRITE_PTSZ(fd, tmp_rec, TOTAL_MINUTES_DAY))
				{
					CEL_TRACE("failed write rdb CH:%d, offset:%ld\n", ch, offset);
					CLOSE_CEL(fd);
					continue;
				}
			}
			
			CLOSE_CEL(fd);
		}
		else if(hdd_info.hdd_info[i].is_mnt)
		{
			struct tm tm_cur = {0};
			tm_cur.tm_year = tm1.year-1900;
			tm_cur.tm_mon = tm1.mon-1;
			tm_cur.tm_mday = tm1.day;
			tm_cur.tm_hour = 0;//tz_hour;
			tm_cur.tm_min = tm_cur.tm_sec = 0;
			time_t sec_cur = mktime(&tm_cur);
			struct tm tm_start = {0};struct tm tm_end = {0};
			gmtime_r(&sec_cur, &tm_start);			
			
			CEL_TRACE("start:%04d%02d%02d %02d:%02d:%02d\n", tm_start.tm_year+1900, tm_start.tm_mon+1, tm_start.tm_mday,
				tm_start.tm_hour, tm_start.tm_min, tm_start.tm_sec);

			{
				snprintf(path, sizeof(path), "%s/rdb/%04d%02d%02d", hdd_info.hdd_info[i].mnt_path, tm_start.tm_year+1900, tm_start.tm_mon+1, tm_start.tm_mday);
				//CEL_TRACE("open rdb path:%s==1111==tz_hour:%f\n", path, tz_hour);
				long offset = TOTAL_MINUTES_DAY*ch + sizeof(rhdr);
				//if(!OPEN_RDONLY(fd, path))	//david modify 2015-09-10
				if(!OPEN_RDWR(fd,path))
				{
					CEL_TRACE("failed open rdb %s\n", path);
					goto STEP_2;
				}
				if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr))) {
					CEL_TRACE("read rdb header error");
					CLOSE_CEL(fd);
					goto STEP_2;
				}
				if (ch >= rhdr.chn_max) {
					CLOSE_CEL(fd);
					CEL_TRACE("invalid chn:%d, chn_max: %d", ch, rhdr.chn_max);
					goto STEP_2;
				}
				if(!LSEEK_SET(fd, offset))
				{
					CEL_TRACE("failed seek rdb.\n");
					CLOSE_CEL(fd);
					goto STEP_2;
				}
				if(!READ_PTSZ(fd, tmp_rec, TOTAL_MINUTES_DAY))
				{
					CEL_TRACE("failed read rdb CH:%d, offset:%ld\n", ch, offset);
					CLOSE_CEL(fd);
					goto STEP_2;
				}
				//david modify 2015-09-10
				if((curdaysec-expdatecnt*86400)>sec){
					expdateflag = 1;
				}
				//////////
				int start_min = tm_start.tm_hour*60 + tm_start.tm_min-60*dst;
				//CEL_TRACE("1111 start_min:%d\n", start_min);
				for(j = start_min; j < TOTAL_MINUTES_DAY; j++)
				{
					if(expdateflag){		//david modify 2015-09-11
						tmp_rec[j] = 0;
					}
					//recmin_tbl[(j-start_min)] |= tmp_rec[j];
					/*if(tmp_rec[j] && (g_sec_res > (sec_cur+j*60) || !g_sec_res))
					{
						g_sec_res = sec_cur+j*60;
						g_cur_hdd = i;
						printf("2222 g_sec_res:%d index:%d\n", g_sec_res, j);
					}*/
				}
				if(expdateflag){			//david modify 2015-09-11
					expdateflag = 0;
					if(!LSEEK_SET(fd, offset))
					{
						CEL_TRACE("failed seek rdb.\n");
						CLOSE_CEL(fd);
						goto STEP_2;
					}
					if(!WRITE_PTSZ(fd, tmp_rec, TOTAL_MINUTES_DAY))
					{
						CEL_TRACE("failed write rdb CH:%d, offset:%ld\n", ch, offset);
						CLOSE_CEL(fd);
						goto STEP_2;
					}
				}
				CLOSE_CEL(fd);
			}
STEP_2: 	
			sec_cur += 86440;// 24 *3600
			gmtime_r(&sec_cur, &tm_end);			
			CEL_TRACE("end:%04d%02d%02d %02d:%02d:%02d\n", tm_end.tm_year+1900, tm_end.tm_mon+1, tm_end.tm_mday,
				tm_end.tm_hour, tm_end.tm_min, tm_end.tm_sec);

			{
				snprintf(path, sizeof(path), "%s/rdb/%04d%02d%02d", hdd_info.hdd_info[i].mnt_path, tm_end.tm_year+1900, tm_end.tm_mon+1, tm_end.tm_mday);
				CEL_TRACE("open rdb path:%s==2222==tz_hour:%f\n", path, tz_hour);
				long offset = TOTAL_MINUTES_DAY*ch + sizeof(rhdr);
				//if(!OPEN_RDONLY(fd, path)) 	//david modify 2015-09-10 
				if(!OPEN_RDWR(fd,path))
				{
					CEL_TRACE("failed open rdb %s\n", path);
					continue;
				}
				if (!READ_PTSZ(fd, &rhdr, sizeof(rhdr))) {
					CEL_TRACE("read rdb header error");
					CLOSE_CEL(fd);
					continue;
				}
				if (ch >= rhdr.chn_max) {
					CLOSE_CEL(fd);
					CEL_TRACE("invalid chn:%d, chn_max: %d", ch, rhdr.chn_max);
					continue;
				}
				if(!LSEEK_SET(fd, offset))
				{
					CEL_TRACE("failed seek rdb.\n");
					CLOSE_CEL(fd);
					continue;
				}

				if(!READ_PTSZ(fd, tmp_rec, TOTAL_MINUTES_DAY))
				{
					CEL_TRACE("failed read rdb CH:%d, offset:%ld\n", ch, offset);
					CLOSE_CEL(fd);
					continue;
				}
				//david modify 2015-09-10
				if((curdaysec-expdatecnt*86400)>sec){
					expdateflag = 1;
				}
				//////////
				int start_min = tm_start.tm_hour*60 + tm_start.tm_min-60*dst;
				//CEL_TRACE("2222 start_min:%d\n", start_min);
				for(j = 0; j < start_min; j++)
				{
					if(expdateflag){		//david modify 2015-09-11
						tmp_rec[j] = 0;
					}
					//recmin_tbl[(j+TOTAL_MINUTES_DAY-start_min)] |= tmp_rec[j];		
					/*if(tmp_rec[j] && (g_sec_res > (sec_cur+j*60) || !g_sec_res))
					{
						g_sec_res = sec_cur+j*60;
						g_cur_hdd = i;
						printf("3333 g_sec_res:%d index:%d\n", g_sec_res, j);
					}*/
				}
				if(expdateflag){			//david modify 2015-09-11
					expdateflag = 0;
					if(!LSEEK_SET(fd, offset))
					{
						CEL_TRACE("failed seek rdb.\n");
						CLOSE_CEL(fd);
						continue;
					}
					if(!WRITE_PTSZ(fd, tmp_rec, TOTAL_MINUTES_DAY))
					{
						CEL_TRACE("failed write rdb CH:%d, offset:%ld\n", ch, offset);
						CLOSE_CEL(fd);
						continue;
					}
				}
				CLOSE_CEL(fd);
			}		
		}
#endif			
	}

//debug
#if 0
	for(i = 0; i < TOTAL_MINUTES_DAY; i++)
	{
		if(recmin_tbl[i])
		{
			printf("%d ", i);
		}
	}	
	CEL_TRACE("\n");
#endif
	return 0;
}


int ms_celmd_get_rec_days(long sec, char *recday_tbl, long curdaysec, void* records)
{
	T_CEL_TM tm1;
	T_CEL_TM gm;
	int bFindOK=0, j;
	char path[LEN_PATH];
	T_CEL_MNT_INFO hdd_info = {0};

	cel_get_local_time_ex(sec, &tm1);
	cel_get_local_time(sec, &gm);
	cel_get_hdd_info(&hdd_info);
	//memset(recday_tbl, 0, 31);	//david mileisght
	int iCnt = 0;
	for(; iCnt < 31; iCnt++){
		recday_tbl[iCnt] = '0';
	}

	//david modify 2015-09-14  
	int idst = get_dst_hour(sec);
	if(idst != 1)
		idst = 0;
	struct record *chnrecord = (struct record*)records;	
	///end
	
	for (j = 0; j < MAX_HDD_COUNT; j++) 
	{
		if (hdd_info.hdd_info[j].is_mnt) 
		{
#if RDB_UTC
		    struct dirent *ent;
		    DIR *dp;
			char buf[LEN_PATH] = {0};
			sprintf(buf, "%s/rdb", hdd_info.hdd_info[j].mnt_path);
		    dp = opendir(buf);
			if(dp == NULL)
			{
				continue;
			}
		    while(1)
		    {
		        ent = readdir(dp);
		        if (ent == NULL)
		            break;

				if (ent->d_name[0] == '.') // except [.] and [..]
				{
					if (!ent->d_name[1] || (ent->d_name[1] == '.' && !ent->d_name[2]))
					{
						continue;
					}
				}
				
				int day = (ent->d_name[6]-'0')*10 + (ent->d_name[7]- '0');
				int mon = (ent->d_name[4]-'0')*10 + (ent->d_name[5]- '0');
				int year = (ent->d_name[0]-'0')*1000 + (ent->d_name[1]- '0')*100 + (ent->d_name[2]-'0')*10 + (ent->d_name[3]-'0');
				int min = 0, max = 0;
				sprintf(path, "%s/rdb/%s", hdd_info.hdd_info[j].mnt_path, ent->d_name);
				
				//david modify 2015-09-14 start  v 7.0.5
				//if(gm.year == year && gm.mon == mon){
				if(tm1.year == year && tm1.mon == mon){
					struct tm current_day = {0};
					long day_cnt = 0;
					int chncnt;
					char current_str[256] = {0};

					snprintf(current_str, sizeof(current_str), "%d-%d-%d %s", year, mon, day, "00:00:00");
					strptime(current_str, "%Y-%m-%d %H:%M:%S", &current_day);
					day_cnt = mktime(&current_day);
	
					for(chncnt=0; chncnt < g_max_camera; chncnt++){
						if(chnrecord[chncnt].record_expiration_date){
							CEL_TRACE("celmd_get_rec_days=path:%s==year:%d==mon:%d==day:%d=====\n", path, year, mon, day);
							ms_celmd_get_rec_mins_ex(chncnt, day_cnt, curdaysec, chnrecord[chncnt].record_expiration_date);
						}
					}	
				}
				//david modify 2015-09-14 end v 7.0.5

				// david v 7.0.6//
				float tz_hour = get_tz_hour();
				if(tz_hour >= 0){
					////pre mon
					if(tm1.year == year && tm1.mon-1 == mon && getlastday(year, mon) == day){		
						if(!get_pbday_index(path, &min, &max)){
							if(max > 0 && max > ((int)((24-tz_hour)*60-idst*60))%TOTAL_MINUTES_DAY){	
								recday_tbl[0] = '1';    
							}
						}
					}
					////pre year
					if(tm1.year-1 == year && tm1.mon == 1 && 12 == mon && getlastday(year, 12) == day){
						if(!get_pbday_index(path, &min, &max)){
							if(max > 0 && max > ((int)((24-tz_hour)*60-idst*60))%TOTAL_MINUTES_DAY){
								recday_tbl[0] = '1';    
							}
						}
					}
				}else if(tz_hour < 0){					
					// Rear mon
					if(tm1.year == year && tm1.mon+1 == mon && (1 == day)){
						if(!get_pbday_index(path, &min, &max)){
							if(max > 0 && min <= (((int)abs(tz_hour))*60-idst*60)){
								recday_tbl[getlastday(tm1.year, tm1.mon)-1] = 1;
							}
						}
					}
					// Rear year
					if(tm1.year+1 == year && (tm1.mon == 12)&&(1 == mon) && (1 == day)){
						if(!get_pbday_index(path, &min, &max)){
							if(max > 0 && min <= (((int)abs(tz_hour))*60-idst*60)){
								recday_tbl[30] = '1';
							}
						}
					}
				}
				/////////////
				
				//if(gm.year == year && gm.mon == mon && !get_pbday_index(path, &min, &max))
				if(tm1.year == year && tm1.mon == mon && !get_pbday_index(path, &min, &max))
				{
					float tz_hour = get_tz_hour();
					CEL_TRACE("celmd_get_rec_days==path:%s==year:%d==mon:%d==min:%d==max:%d=idst:%d=tz_hour:%f=\n", path, year, mon, min, max, idst, tz_hour);
					if(tz_hour > 0)
					{
						if(day == 1){			//V 7.0.6
							if(max > 0 && max < ((int)((24-tz_hour)*60-idst*60))%TOTAL_MINUTES_DAY){
								recday_tbl[0] = '1';
							}
							if(min > 0 && min <= ((int)((24-tz_hour)*60-idst*60))%TOTAL_MINUTES_DAY){
								recday_tbl[0] = '1';
							}
							if(max >= ((int)((24-tz_hour)*60-idst*60))%TOTAL_MINUTES_DAY && min <= ((int)((24-tz_hour)*60-idst*60))%TOTAL_MINUTES_DAY){
								recday_tbl[0] = '1';
							}
						}
						
						if(min > 0 && min <= ((int)((24-tz_hour)*60-idst*60))%TOTAL_MINUTES_DAY && day >= 1){
							recday_tbl[day-1] = '1';
						}
						if(max > 0 && max > ((int)((24-tz_hour)*60-idst*60))%TOTAL_MINUTES_DAY && day < 31){
							recday_tbl[day] = '1';
						}
					}
					else if(tz_hour < 0)
					{
						//printf("day:%d,min:%d,max:%d,tz_hour:%f,tz:%d,idst:%d\n", day,min,max,tz_hour,((int)((24-tz_hour)*60)-idst*60)%TOTAL_MINUTES_DAY,idst);
						if(min > 0 && min <= ((int)((24-tz_hour)*60)-idst*60)%TOTAL_MINUTES_DAY && day >= 2){
							recday_tbl[day-2] = '1';
						}else if(min > 0 && min <= ((int)((24-tz_hour)*60)-idst*60)%TOTAL_MINUTES_DAY && day < 2)
						{
							if(tm1.day == getlastday(year, mon-1))
								recday_tbl[getlastday(year, mon-1)-1] = '1';
						}
						if(max > 0 && max > ((int)((24-tz_hour)*60)-idst*60)%TOTAL_MINUTES_DAY && day >= 1){
							recday_tbl[day-1] = '1';
						}
						if(max == -1 && min > 0 && min >= (((int)abs(tz_hour))*60-idst*60)){	//v 7.0.6
							recday_tbl[day-1] = '1';
						}	
					}
					else
					{
						if(idst){
							if(max > 0 && min <= ((int)((24-tz_hour)*60)-idst*60)%TOTAL_MINUTES_DAY ){
								if(day >= 1)
									recday_tbl[day-1] = '1';
							}
						}else{
							if(day >= 1)
								recday_tbl[day-1] = '1';
						}	
						if(max == -1 && min > 0 && min <= ((int)((24-tz_hour)*60)-idst*60)%TOTAL_MINUTES_DAY){	//v 7.0.6
							recday_tbl[day-1] = '1';
						}
					}
					
					if (!bFindOK)
						bFindOK = 1;				
				}
		    }
		    closedir(dp);			
#else
			for (i = 0; i < 31; i++) 
			{
				sprintf(path, "%s/rdb/%04d%02d%02d", hdd_info.hdd_info[j].mnt_path, tm1.year, tm1.mon, i + 1);
				if (-1 != access(path, 0)) 
				{
					recday_tbl[i] = '1';		
					if (!bFindOK)
						bFindOK = 1;
				}
			}
#endif				
		}
	}
	return bFindOK;
}

//david modify 2015-09-10 end

//david.milesight
int ms_celmd_get_chmask_status(CH_MASK* ch_mask, int channel)
{
	if(channel<0 || channel >= MAX_CH_MASK*MASK_LEN)
		return 0;
	int modulus = channel/MASK_LEN;
	int remainder = channel%MASK_LEN;

	if(ch_mask->mask[modulus] & ((unsigned int)1 << remainder)){
		return 1;
	}else{
		//CEL_TRACE("===david test==channel:%d, modulus:%d, remainder:%d====\n", channel, modulus, remainder);
		return 0;
	}
}

int ms_celmd_chn_to_masks(CH_MASK* ch_mask, int channel)
{
	if(channel<0 || channel >= MAX_CH_MASK*MASK_LEN)
		return -1;
	int modulus = channel/MASK_LEN;
	int remainder = channel%MASK_LEN;

	//CEL_TRACE("===david test==channel:%d, modulus:%d, remainder:%d===\n", channel, modulus, remainder);
	ch_mask->mask[modulus] |= ((unsigned int)1 << remainder);
	
	return 0;
}

int ms_celmd_masks_to_chn(CH_MASK* ch_mask)
{
	int channel = -1;
	int i, nCnt = 0;
	for(nCnt = 0; nCnt < MAX_CH_MASK; nCnt++){
		if(ch_mask->mask[nCnt]){
			for(i =0; i < MASK_LEN; i++){
				if(ch_mask->mask[nCnt] & ((unsigned int)1<<i)){
					channel = nCnt*MASK_LEN + i ;
					//CEL_TRACE("===david test==channel:%d, nCnt:%d==mask[%d]:%d==\n", channel, nCnt, i, ch_mask->mask[nCnt]);
				}
			}
		}
	}
	
	return channel;
}

int ms_celmd_masks_copy(CH_MASK* src_mask, CH_MASK* dst_mask)
{
	int nCnt = 0;
	for(nCnt = 0; nCnt < MAX_CH_MASK; nCnt++){
		dst_mask->mask[nCnt] = src_mask->mask[nCnt];
	}
	
	return 0;
}

int ms_celmd_set_masks(CH_MASK* ch_mask, int channel)
{
	if(channel<0 || channel >= MAX_CH_MASK*MASK_LEN)
		return -1;
	int modulus = channel/MASK_LEN;
	int remainder = channel%MASK_LEN;

	//CEL_TRACE("===david test==channel:%d, modulus:%d, remainder:%d===\n", channel, modulus, remainder);

	memset(ch_mask, 0, sizeof(CH_MASK));

	ch_mask->mask[modulus] |= ((unsigned int)1 << remainder);
	
	return 0;
}

int ms_celmd_masks_is_empty(CH_MASK* ch_mask)
{
	int nCnt = 0;
	for(nCnt = 0; nCnt < MAX_CH_MASK; nCnt++){
		if(ch_mask->mask[nCnt] != 0)
			return 0;
	}
	return 1;	
}

int ms_celmd_chn_cmp_masks(CH_MASK* ch_mask, int channel)
{
	if(channel<0 || channel >= MAX_CH_MASK*MASK_LEN)
		return -1;
	int modulus = channel/MASK_LEN;
	int remainder = channel%MASK_LEN;
	
	CH_MASK tmp_mask;

	tmp_mask.mask[modulus] |= ((unsigned int)1 << remainder);
	
	if(tmp_mask.mask[modulus] & ch_mask->mask[modulus]){
		return 0;
	}
	
	return -1;
}

int ms_celmd_masks_cmp(CH_MASK* ch_mask1, CH_MASK* ch_mask2)
{
	int nCnt = 0, nmask = 0;;
	for(nCnt = 0; nCnt < MAX_CH_MASK; nCnt++){
		if(ch_mask2->mask[nCnt]){
			if(ch_mask1->mask[nCnt] & ch_mask2->mask[nCnt])
				continue;
			else{
				nmask = 1;
				break;
			}
		}
	}
	if(nmask)
		return 0;
	return 1;
}

#ifdef HDR_ENABLE
T_CELMD_CTRL *celmd_ctor(int chns)
{
	T_CELMD_CTRL *ctrl = NULL;
	do {
		if ((ctrl = ms_calloc(1, sizeof(T_CELMD_CTRL))) == NULL) {
			PERROR("ms_calloc CTRL");
			break;
		}
		if ((ctrl->chns_sec = ms_calloc(chns, sizeof(long))) == NULL) {
			PERROR("ms_calloc CHNS_SEC");
			break;
		}
		if ((ctrl->hbuf = ms_calloc(HBUFSIZE, 1)) == NULL) {
			PERROR("ms_calloc HBUF SIZE");
			break;
		}
		ctrl->hsize = HBUFSIZE;
		return ctrl;
	} while (0);
	if (ctrl) {
		if (ctrl->chns_sec)
			ms_free(ctrl->chns_sec);
		if (ctrl->hbuf)
			ms_free(ctrl->hbuf);
		ms_free(ctrl);
	}
	return NULL;
}

void celmd_dtor(T_CELMD_CTRL **ctrl)
{
	if (!(*ctrl))
		return;
	T_CELMD_CTRL *tmp = *ctrl;
	if (tmp) {
		if (tmp->chns_sec)
			ms_free(tmp->chns_sec);
		if (tmp->hbuf)
			ms_free(tmp->hbuf);
		ms_free(tmp);
	}
}

#endif
/**
 * @}
 */


