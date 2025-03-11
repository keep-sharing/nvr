/**
 @file cel_muldec.h
 @defgroup cellular multiple decoding
 @ingroup cellular
 @brief
 */


///////////////////////////////////////////////////////////////////////////////
#ifndef _BASKET_MULSTM_H_
#define _BASKET_MULSTM_H_
///////////////////////////////////////////////////////////////////////////////


#include "udcellular.h"

#define CELMD_STATUS_OPEN 1
#define CELMD_STATUS_STOP 0

typedef struct
{
	CH_MASK ch_mask;          ///< particular channel david.milesight
	long cid;         ///< particular cellular's id

	long sec;         ///< seconds
	long *chns_sec; ///< timestamp all channels
	int  cel_status;  ///< basket status, can set CELMS_STATUS_OPEN or CELMS_STATUS_STOP

	CEL_FHANDLE hcel; ///< handle of cellular file
	CEL_FHANDLE hidx; ///< handle of index file

#ifdef HDR_ENABLE
	CEL_FHANDLE hhdr;
	void *hbuf; ///< header file buffer
	int hsize; ///< header buffer size
	int htotal; ///< header count total
	int hcurr; ///< current header
	int hnr; ///< total headers read from start
	int hreload;
	long hreload_fpos;
#endif

	// BK-1222
	long long fpos_cel;    ///< cellular file pointer
	int  iframe_cnt;  ///< total count
	int  iframe_cur;  ///< current iframe, zero base
	int  cel_cnt ;     ///< count of cellular
	int  cur_hdd ;     ///< current hard disk
	int  next_hdd ;    ///< next hard disk
	char target_path[LEN_PATH]; ///< target path

}T_CELMD_CTRL;

typedef T_CELMD_CTRL CEL_MULDEC_CTRL;

typedef struct
{
//    long fpos;///<
    long ch;///< channel mask

    T_TS ts; ///< nvr's timestamp
    int time_lower; ///< ipc's
    int time_upper; ///< ipc's

    long  frm_size;///< frame size
    short frm_rate;///< frame rate
    short frm_width;///< frame width
    short frm_height;///< frame height
    int   frm_type;///< frame type; I,P...
    long  strm_type; ///< stream type: audio or video
    long  strm_fmt; ///< stream format: sub or main
//    long  evt;///< event type
    long  smp_rate;///< sampling rate
    long  achn; ///< mono or stereo
    long  bps;///< bits per seconds

    char *vbuffer; ///< video read buffer pointer
//    char *abuffer; ///< audio read buffer pointer
    int  buf_size; ///< current buffer size
//	int  fc_chn; ///< focus channel

    long cid; ///< cellular id
//    long fpos_cel;///< position of current open file
    int codec_type;

//    short audio_on; // AYK - 0201
//	int   cap_mode;  // 2D1, 4CIF, 8CIF, 4D1
}T_CELMD_PARAM;
//////////////////////////////////////////////////////////////////////////

long celmd_close(CEL_MULDEC_CTRL *celmd_ctrl, int stop);

long celmd_open(CEL_MULDEC_CTRL *celmd_ctrl, CH_MASK* ch_mask, long sec, int direct, int max_camera);

long celmd_open_next(CEL_MULDEC_CTRL *celmd_ctrl, CH_MASK* ch_mask, long nextcid);

long celmd_open_prev(CEL_MULDEC_CTRL *celmd_ctrl, CH_MASK* ch_mask, long prevcid);

long celmd_srch_prev_play_cid(const char *path, long sec, CH_MASK* ch_mask);

long celmd_srch_next_play_cid(const char *path, long sec, CH_MASK* ch_mask);

long celmd_srch_next_cid(CEL_MULDEC_CTRL *celmd_ctrl, CH_MASK* ch_mask, long curcid, long sec);

long celmd_srch_prev_cid(CEL_MULDEC_CTRL *celmd_ctrl, CH_MASK* ch_mask, long curcid, long sec);

int celmd_has_rec_data(int ch, long sec, int direct);

long celmd_read_next_frame(CEL_MULDEC_CTRL *celmd_ctrl, T_CELMD_PARAM *dp);

long celmd_read_next_first_frame(CEL_MULDEC_CTRL *celmd_ctrl, T_CELMD_PARAM *dp);

long celmd_read_next_iframe(CEL_MULDEC_CTRL *celmd_ctrl, T_CELMD_PARAM *dp);

long celmd_read_prev_iframe(CEL_MULDEC_CTRL *celmd_ctrl, T_CELMD_PARAM *dp);

long celmd_get_time(long cid, CH_MASK* ch_mask, long sec, long *o_begin_sec, long *o_end_sec);

long celmd_set_tar_disk(CEL_MULDEC_CTRL *celmd_ctrl, int hddid, int cid);

int celmd_get_tar_disk(CEL_MULDEC_CTRL *celmd_ctrl, CH_MASK* ch_mask, long sec);

int celmd_exget_tar_disk(CEL_MULDEC_CTRL *celmd_ctrl, char *path);

int celmd_get_rec_days(long sec, char *recday_tbl);

int celmd_get_rec_mins(int ch, long sec, char *recmin_tbl);

int celmd_get_rec_res(int ch, long sec, char *recmin_tbl, int *maxres);

long celmd_get_cur_play_time(CEL_MULDEC_CTRL *celmd_ctrl);

long celmd_get_first_rec_time(long sec , CH_MASK* ch_mask, char *mntpath);

long celmd_get_first_rec_time_ex(long sec , CH_MASK* ch_mask, char *mntpath);

long celmd_get_last_rec_time(long sec, CH_MASK* ch_mask);

long celmd_set_ctrl_disk(const char *mntpath, T_CELMD_CTRL *celmd_ctrl);

long celmd_jmp2next(T_CELMD_CTRL *celmd_ctrl, int direct);

int celmd_get_rec_file_cnt(T_CELMD_CTRL *celmd_ctrl, CH_MASK* ch_mask, long t1, long t2);

int celmd_get_frame_sizes(CEL_MULDEC_CTRL *celmd_ctrl, T_CELMD_PARAM *dp, long endtime);

void celmd_cpfrm2dst(T_CELMD_PARAM *src, struct reco_frame *dst);

int ms_celmd_get_rec_mins(int ch, long sec, char *recmin_tbl, long curdaysec, int expdatecnt);

int ms_celmd_get_rec_days(long sec, char *recday_tbl, long curdaysec, void* records);

int ms_celmd_get_chmask_status(CH_MASK* ch_mask, int channel);

int ms_celmd_chn_to_masks(CH_MASK* ch_mask, int channel);

int ms_celmd_masks_to_chn(CH_MASK* ch_mask);

int ms_celmd_masks_copy(CH_MASK* src_mask, CH_MASK* dst_mask);

int ms_celmd_set_masks(CH_MASK* ch_mask, int channel);

int ms_celmd_masks_is_empty(CH_MASK* ch_mask);

int ms_celmd_chn_cmp_masks(CH_MASK* ch_mask, int channel);

int ms_celmd_masks_cmp(CH_MASK* ch_mask1, CH_MASK* ch_mask2);

long get_cur_day_to_sec(T_CEL_TM ptm);
long get_cur_localday_start_sec(long sec);
long get_cur_localday_end_sec(long sec);
int get_cursec_is_acrossday(long sec);
int print_cur_localday_by_sec(long sec, char tmp_time[32]);


#ifdef HDR_ENABLE
T_CELMD_CTRL *celmd_ctor(int chns);

void celmd_dtor(T_CELMD_CTRL **ctrl);
#endif
///////////////////////////////////////////////////////////////////////////////
#endif//_BASKET_MULSTM_H_
///////////////////////////////////////////////////////////////////////////////
