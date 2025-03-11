/**
 * @file cel_public.h
 * @date Created on: 2013-7-8
 * @author chimmu
 * @brief cellular storage system common interface
 */

#ifndef CEL_PUBLIC_H_
#define CEL_PUBLIC_H_

#include "msdefs.h"
//#include "disk_op.h"
////////////////////////////////////enum start////////////////////////////////////////////////////////
#define TOTAL_MINUTES_DAY (1440)
#define MAX_PB_SID	50

#define CEL_SIZE_G 	8
#define SIZE_GB (1 << 30)
#define NAME_TOPMGR "mgr_top.cfg"		///MULTI-HDD SKSUNG
#define NAME_HDDMGR "mgr_hdd.cfg"
#define NAME_CELMGR "mgr_cel.cfg"
#define NAME_CELREC_INFO "cel_rec.cfg"  ///< basket record info...for currently using file
#define NAME_CEL_FILE "000001.cel"

//struct hdd_info_list;
//struct reco_frame;

/// recycle mode
enum rcy_mode{
	CEL_RECYCLE_ONCE = 0, ///< not recycle. if all cellular are full, will stop record
	CEL_RECYCLE_ON  ///< recycle mode on
};

/// 回放方向
enum pb_direct{
	PB_DIRECT_FORWARD = 0,
	PB_DIRECT_BACKWARD,
	PB_DIRECT_NONE
};

/// 回放速度
enum pb_speed {
	PB_SPEED_1X = 0,
	PB_SPEED_2X,
	PB_SPEED_4X,
	PB_SPEED_8X,
	PB_SPEED_16X,
	PB_SPEED_32X,
	PB_SPEED_64X,
	PB_SPEED_128X,
	PB_SPEED_0_125X,
	PB_SPEED_0_25X,
	PB_SPEED_0_5X
};

/// 异常类型
enum except_type {
	ET_NONE = 0,
	ET_HDD_FULL,
	ET_NO_HDD,
	ET_RECORD_FAIL,
	ET_PLAYBACK_FAIL,
	ET_DISK_FAIL,
	ET_RECORD_STREAM, ///< 录像码流不存在
	ET_NO_FORMAT,
};

enum except_snap_type
{
	SNAP_SAVE_OK,
	SNAP_SAVE_NO_DISK,
	SNAP_SAVE_NO_FORMAT,
	SNAP_SAVE_FAIL,
};

enum acrossday_type{
	ACROSSDAY_NO = 0,
	ACROSSDAY_PREVIOUS,
	ACROSSDAY_AFTER,
	ACROSSDAY_OTHER,
};
//////////////////////////////////////enum end//////////////////////////////////////////////////////

/////////////////////////////////////structure start////////////////////////////////////////////////

struct ms_exception {
	int type; ///< exception type
	int sid; ///< playback session id
	char disk_path[20]; ///< disk path
	void *arg;
};

struct pbstat {
	int sid;
	char disk[20];
	long cid;
	long secs;
};

//david.milesight 
#define MAX_CH_MASK 8
#define MASK_LEN 32
typedef struct _ch_mask{
	unsigned int mask[MAX_CH_MASK];
}CH_MASK;

///////////////////////////////////structure end////////////////////////////////////////////////////

//////////////////////////////////callback start///////////////////////////////////////////////////

typedef int (*CALLBACK_EXCEPTION)(void *arg, struct ms_exception *except);///< exception handler

typedef int (*CALLBACK_DEC)(void *arg, struct reco_frame *frm); ///< decode handler

/////////////////////////////////callback end//////////////////////////////////////////////////////

////////////////////////////////function start/////////////////////////////////////////////////////

int cel_init(int chn_cnt, CALLBACK_EXCEPTION fn, void *arg, int maxpb); ///< init cellular file system

int cel_deinit(void); ///< deinit cellular file system

void cel_debug(int enable); ///< whether to print debug message

int cel_show_cel_mgr(const char *hd_path);

/******************************** playback start ******************************************/

int celpb_get_rec_days(long sec, char *day_tbl); ///< get record days, size of day_tbl is 31

int celpb_get_rec_mins(int ch, long sec, char *min_tbl);///< get record minutes, size of min_tbl is 1440

int celpb_get_rec_res(int ch, long sec, char *res_tbl, int *maxres);

int celpb_start(CH_MASK* ch_mask, long sec, CALLBACK_DEC fn, void *arg, int *sid);///< start playback

int celpb_stop(int sid); ///< stop playback

int celpb_pause(int sid); ///< pause playback

int celpb_step(int sid, int direct); ///< single frame step

int celpb_resume(int sid); ///< resume playback

int celpb_change_speed(int sid, int speed, int direct); ///< change playback speed

int celpb_jump(int sid, long sec); ///< jump to the sec time point

int celpb_get_play_stats(struct pbstat *arr, int len);

int celpb_stop_all(void);

long celpb_get_play_time(int sid); ///< get current play time

int celpb_single_jump(int sid, long sec);


/**********************************playback end *******************************************/


/********************************** record start ******************************************/
int cel_get_cur_port(void);

int cel_get_rec_path(char *path);

int cel_start_record(int chn);

int cel_stop_record(int chn);

int cel_write_strm(struct reco_frame *frm, int evt); ///< start to write stream

int cel_create_fs(const char *mnt_path); ///< create cellular filesystem

int cel_check_fs(const char *mnt_path);///< check cellular filesystem's version
#ifdef MSFS_DAVID
int cel_update_disk_info(struct hdd_info_list *hdd_list); ///< update mounted disk info
#endif
int cel_update_disk_health_status(int smart_res, int port);

void cel_set_recycle(int port, int mode);

int cel_get_status();



/********************************* record end *********************************************/


/********************************* backup start *******************************************/

int celbk_init(CH_MASK* ch_mask, long start, long end, int max_camera); ///< init backup

int celbk_deinit(void); ///< deinit backup. when backup is done, call it.

int celbk_read_frame(struct reco_frame *bk_frm); ///< get backup frame.

int celbk_get_frame_sizes(void);

/********************************* backup end **********************************************/

/************david modify 2015-09-10 start **************/
int ms_celpb_get_rec_mins(int ch, long sec, char *min_tbl, long curdaysec, int expdatecnt);
int ms_celpb_get_rec_days(long sec, char *day_tbl, long curdaysec, void* records);

/************david modify 2015-09-10 end**************/
int celpb_snapshot(); //bruce.milesight add

int cellular_chn_to_masks(CH_MASK* ch_mask, int channel);
int cellular_chns_to_masks(CH_MASK* ch_mask, long long channels);
int cellular_masks_to_masks(CH_MASK* des, CH_MASK* src);
int cellular_chn_is_being(CH_MASK* ch_mask, int channel);
int cellular_masks_is_empty(CH_MASK* ch_mask);
int cellular_masks_clear(CH_MASK* ch_mask);
int cellular_masks_to_chn_num(CH_MASK* ch_mask);

////////////////////
int celpb_set_mask(int sid, CH_MASK* ch_mask); ///< set playback play mask

///////////////////////////////function end////////////////////////////////////////////////////////
#endif /* CEL_PUBLIC_H_ */
