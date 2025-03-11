/**
 @file cellular_rec.h
 @defgroup CEL_REC BSK_RECORD
 @ingroup CEL
 @brief
 */

///////////////////////////////////////////////////////////////////////////////
#ifndef _CEL_REC_H_
#define _CEL_REC_H_
///////////////////////////////////////////////////////////////////////////////

#include "udcellular.h"
//#include "disk_op.h"
///////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C"{
#endif
///////////////////////////////////////////////////////////////////////////////

/**
  @struct BKTREC_CTRL
  @ingroup CEL_REC
  @brief value of cellular record control
  */

typedef struct
{
	char path_cel[LEN_PATH]; ///< cellular file path
	char path_idx[LEN_PATH]; ///< index file path
	CEL_FHANDLE fp_cel;
	CEL_FHANDLE fp_idx;
	long long fpos_cel;      ///< file position of cel
	long fpos_idx;      ///< file position of idx
#if HDR_ENABLE
	char path_hdr[LEN_PATH];
	CEL_FHANDLE fp_hdr;
	long fpos_hdr;
#endif



//	char b_buf[CEL_BBUF_SIZE]; ///< stream buffer
	char *b_buf;///< stream buffer
	int  b_pos;
	int  b_size; ///< stream buffer size

//	char i_buf[CEL_IBUF_SIZE]; ///< index buffer
	char *i_buf; ///< index file buffer
	int  i_pos; ///< index file pos
	int  i_size; ///< index buffer size
//	int  idx_pos[CEL_MAX_CHANNEL];
	int *idx_pos;
#ifdef HDR_ENABLE
	void *h_buf; //header buf
	int h_pos;
	int h_size;
#endif
//	char r_buf[CEL_RBUF_SIZE]; ///< rdb buffer
	char *r_buf; ///< rdb buffer
	int  r_pos; ///< buffer pos
	int  r_size; ///< rdb buffer size
	int  r_skip;
//	int  target_status ;
	char target_path[LEN_PATH];
	int  cel_cnt ;

	long cid;		    ///< cellular id
	long prev_day;      ///< previous day
//	long prev_min[CEL_MAX_CHANNEL]; ///< previous minutes. each cellular time stamp all channel
	long *prev_min;
	T_CEL_TIME cel_secs; ///< for all channel
	T_CEL_TIME idx_secs; ///< for all channel

	CALLBACK_EXCEPTION cb_exc;
	void *exc_arg;

	// rdb parameter
	char *evts;
	T_CEL_RDB *rdbs;
	char *mapbuf;

	CEL_FHANDLE fp_cache;
	pthread_t thr_cache;
	int cache_init;
	int cache_stop;

	int *resolution; //resolutions
//	int mapfd;
#ifdef CEL_MMAP
	char *fp_mmap;
	long pos_mmap;
#endif
}T_CELREC_CTRL;

struct rec_ctrl {
	CH_MASK ch_mask;//david.milesight
	int rec_cnt; ///< 录像通道数
	int buf_cur; ///< 当前已打开通道的缓存配额
};

long cel_get_rec_idx_cnt(void);
#ifdef HDR_ENABLE
long cel_get_frm_cnt(void);
#endif
void cel_set_recycle_mode(int port, int mode);

int cel_get_cur_disk_path(char *path);

void cel_get_cur_path(char *dst, int size);
#ifdef MSFS_DAVID
int cel_update_mnt_info(struct hdd_info_list *info);
#endif
int cel_update_disk_health(int smart_res, int port);

void cel_close(void);

int cel_is_opened(void);

int cel_is_closed(void);

int cel_set_tar_disk(const int cur_idx);

int cel_ref_cur_disk(void);

long cel_write_audio_strm(T_AUDIO_REC_PARAM *pa);

long cel_write_video_strm(T_VIDEO_REC_PARAM *pv);

long cel_flush_buffer(void);

int cel_flush_rdb(void);

void cel_get_hdd_info(T_CEL_MNT_INFO *mnt_info);

int cel_is_recording(long cid, const char *path);


void cel_get_ctrl_handler(T_CELREC_CTRL **celrec_ctrl);

int cel_rec_stop(int chn);

int cel_rec_start(int chn);

int cel_init_rec_ctrl(int chn_cnt, CALLBACK_EXCEPTION fn, void *arg);

int cel_deinit_rec_ctrl(void);
//david modify 2015-09-14 start
int ms_cel_copy_rdb_out(void);
int ms_cel_paste_rdb_in(void);
//david modify 2015-09-14 end

long cel_creat(const char *diskpath);
long cel_exit(int save_flag, int flushbuffer);

//david.milesight
int cel_get_chmask_status(int channel);

int cel_set_chmask_status(int channel, int status);

int cel_set_chns_per_day(int channel, int status);


///////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
#endif//_CEL_REC_H_
///////////////////////////////////////////////////////////////////////////////
