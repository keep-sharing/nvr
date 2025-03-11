///////////////////////////////////////////////////////////////////////////////
#ifndef _UDCEL_H_
#define _UDCEL_H_
///////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "msstd.h" 
//#include "memcpy_neon.h"
#include "cel_public.h" //for struct reco_frame

#define likely(x)	__builtin_expect (!!(x), 1)
#define unlikely(x)	__builtin_expect (!!(x), 0)

extern int g_debug; // in udcellular.c
#define CEL_TRACE(msg,args...) \
	{ if (g_debug) \
	msdebug(DEBUG_INF,msg,##args);}

#define PERROR(msg) { \
	if (g_debug) \
		perror(msg); }

//////////////////////////////////////////////////////////////////////////
#define SEP_AUD_TASK
#define PREV_FPOS          // AYK - 0201
//////////////////////////////////////////////////////////////////////////

//#define HDR_ENABLE

//////////////////////////////////////////////////////////////////////////
/// determine use file descriptor(int) or FILE*
#define CEL_SYSIO_CALL
//////////////////////////////////////////////////////////////////////////

/////MULTI-HDD SKSUNG/////
//#define MAX_HDD_COUNT	MAX_PARTITION
#define MAX_HDD_COUNT	9 //local: 8 + reversed: 1
//////////////////////////


/// basket max channel. same as supported dvr max channel
#define CEL_MAX_CHANNEL 	MAX_CAMERA

#define CEL_OK 1        ///< OK
#define CEL_FULL 2      ///< basket status is full.  Using Only if g_bkt_rec_status is CEL_STATUS_FULL
#define CEL_NOAUDFR   3   // AYK - 0201
#define CEL_NONEXTBID 4   // AYK - 0201

////MULTI-HDD SKSUNG////
#define CEL_OVERHDD		5
#define CEL_CREATE_FAIL 6
#define CEL_OPEN_FAIL 	7
#define CEL_READ_FAIL 	8
#define CEL_WRITE_FAIL 	9
////////////////////////

//#define CEL_MMAP	1//bruce.milesight add version7.0.4
#define RDB_UTC	1	//bruce.milesight add version7.0.4
#define CEL_ERR -1      ///< Failed or error
#define CEL_FAIL	-2 ///< read fail

#define S_OK   1
#define S_FAIL 0

#ifndef BOOL
#define BOOL int
#endif

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

#define MAX_NUM_FRAME_SEARCH 64 // AYK - 0201
///////////////////////////////////////////////////////////////////////////////
#define SIZE_KB (1 << 10) ///< kilo bytes
#define SIZE_MB (1 << 20)

//#define SIZE_GB (SIZE_MB*SIZE_MB)
#ifdef CEL_MMAP
#define SIZE_MMAP (1<<25)//64M
#endif

#define LEN_PATH   64    ///< length of path
#define LEN_TITLE  32    ///< length of title
#define CEL_PATH_LEN 64  ///< length of basket path
///////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// seconds
#define SEC_DAY  86400 ///< total seconds of a day, 3600*24
#define SEC_HOUR 3600  ///< total secodns of a hour, 60*60
#define SEC_MIN  60    ///< total seconds of a minute
//////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
#define CEL_SNAP_SPACE	20 // 20G
///////////////////////////////////////////////////////////////////////////////

#define MAX_TIME   0x7FFFFFFF ///< 4 bytes
#define MAX_TIME64 0x7FFFFFFFFFFFFFFF ///< 8 bytes
///////////////////////////////////////////////////////////////////////////////
///< basket structure version
#define VER_UD_CEL_FORMAT 1

#define CEL_PLAYDIR_BACKWARD 0  ///< currently playback reverse
#define CEL_PLAYDIR_FORWARD  1  ///< currently playback forward

#define CEL_FORWARD 0          ///< read basket for playback forward
#define CEL_BACKWARD 1          ///< read basket for playback reverse
#define CEL_TIMEPOINT 2         ///< read basket playback time indexing
#define CEL_NOSEARCH DIRECTION 3 ///< not searching...read record flag for only particular time point

//#define CEL_PLAYMODE_NORMAL 0   ///< normal playback mode
//#define CEL_PLAYMODE_IFRAME 1   ///< iframe only playback mode
//
//#define PLAY_STATUS_STOP  0 ///< playback status is stopped
//#define PLAY_STATUS_PLAY  1 ///< playback status is playing
//#define PLAY_STATUS_PAUSE 2 ///< playback status is paused
//#define PLAY_STATUS_SKIP  3 ///< playback status is skip frame mode

// basket status ...
// empty means that remains EMPTY baskets.
// full  means that there is no empty baskets.
#define CEL_SPACE_REMAIN (0) ///< basket not full
#define CEL_SPACE_FULL (1)   ///< basket full

#define CEL_VER_LEN	10 ///< basket version len new add

///////////////////////////////////////////////////////////////////////////////
/// basket priority
enum _enumBasketPriority{BP_NONE=0, BP_LOW, BP_NORMAL, BP_HIGH, MAX_BP};
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/// save flag
enum _enumSaveFlag{SF_EMPTY=0, SF_USING, SF_STOP, SF_FULL, SF_ERR, SF_BACKUP, MAX_SF};

///////////////////////////////////////////////////////////////////////////////
typedef enum {
	HDD_IDLE = 0,
	HDD_USED,
	HDD_FULL
}HDD_STATE;
///////////////////////////////////////////////////////////////////////////////
#define HDDMGR_PATH "bin/dvrapp_cfg"
#define RECO_PATH	"rec"
#define MAX_MGR_PATH	64

#define CELMGR_INFO_ENABLE	0//ԭ���� zzf

//#define PREREC_ENABLE 1 // BKKIM, pre-record enable

///////////////////////////////////////////////////////////////////////////////
// identifier
#define ID_TOPMGR_HEADER   0xA0000001   ///< header structure of topmgr.udf file
#define ID_TOPMGR_BODY	   0xA0000002   ///< body structure of tomgr.udf file

#define ID_CELMGR_HEADER   0xB0000001   ///< header structure of bktmgr.udf
#define ID_CELMGR_BODY	   0xB0000002   ///< body structure of bktmgr.udf

#define ID_CEL_HDR      0xC0000001
#define ID_CEL_BODY     0xC0000002

#define ID_VIDEO_HEADER    0xD0000001
#define ID_VIDEO_STREAM	   0xD0000002

#define ID_AUDIO_HEADER    0xE0000001
#define ID_AUDIO_STREAM	   0xE0000002

#define ID_INDEX_HEADER	   0xF0000001
#define ID_INDEX_DATA	   0xF0000002
#define ID_INDEX_FOOTER    0xF0000004

#ifdef HDR_ENABLE
#define ID_HDRFILE_HDR		0xF0000010
#endif
#define ID_CEL_END      0xFFFFFFFF
///////////////////////////////////////////////////////////////////////////////

/// 1~31, 0 is not used
#define MAX_RECDATE 32

#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))
#endif//max

#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif//min

#define HI32(h) ((short)((long)(h) >> 16))
#define LO32(l) ((short)((long)(l) & 0xffff))
#define MK32(h, l) ((long)(((short)((long)(l) & 0xffff)) | ((long)((short)((long)(h) & 0xffff))) << 16))

#define HI64(l) ((unsigned long)((int64_t)(l) >> 32))
#define LO64(l) ((unsigned long)((int64_t)(l) & 0xffffffff))
#define MK64(h, l) ((int64_t)(((long)((int64_t)(l) & 0xffffffff)) | ((int64_t)((long)((int64_t)(h) & 0xffffffff))) << 32))


#ifndef SAFE_FREE
#define SAFE_FREE(p) if(NULL != p){free(p);p=NULL;}
#endif

#ifdef CEL_SYSIO_CALL

/// basket file descriptor
#define CEL_FHANDLE int
#define CEL_ISVALIDHANDLE(fd) (fd > 0)


#ifndef O_DIRECT
#define O_DIRECT	0400000
#endif

#define OPEN_CREATE(fd, path) (-1 != (fd = open(path, O_RDWR|O_CREAT|O_TRUNC, 0644)))
#define OPEN_EMPTY(fd, path)  (-1 != (fd = open(path, O_RDWR|O_TRUNC)))
#define OPEN_RDWR(fd, path)   (-1 != (fd = open(path, O_RDWR)))
#define OPEN_RDONLY(fd, path) (-1 != (fd = open(path, O_RDONLY)))
#define OPEN_RDSYNC(fd, path) (-1 != (fd = open(path, O_RDONLY | O_DIRECT | O_ASYNC))) //added by chimmu

#define SAFE_CLOSE_CEL(fd) if(fd > 0){ close(fd);fd = -1;};
#define CLOSE_CEL(fd) close(fd) //(-1 != close(fd))

#define READ_ST(fd, st) ( 0 < read(fd, &st, sizeof(st)))
#define READ_HDR(fd, st) ( 0 < read(fd, &st, sizeof(st)))
#define READ_STRC(fd, st) read(fd, &st, sizeof(st))
#define READ_PTSZ(fd, pt, sz)   ( 0 < read(fd, pt, sz))

#if 1
#define WRITE_ST(fd, st) ( 0 < write(fd, &st, sizeof(st)))
#else
#define WRITE_ST(fd, st) (ms_write_data(fd, (void*)&st, sizeof(st)))
#endif

#define FSNYC_ST(fd) (fsync(fd))

#define WRITE_PTSZ(fd, pt, sz) ( 0 < write(fd, pt, sz))

#define LSEEK_END(fd, pos) ( -1 != lseek(fd, (off_t)pos, SEEK_END))
#define LSEEK_CUR(fd, pos) ( -1 != lseek(fd, (off_t)pos, SEEK_CUR))
#define LSEEK_SET(fd, pos) ( -1 != lseek(fd, (off_t)pos, SEEK_SET))
#define LSEEK_RSET(fd, pos, ret) ((ret = lseek(fd, (off_t)pos, SEEK_SET)) != -1) //added by chimmu

#define LTELL(fd) lseek(fd, (off_t)0, SEEK_CUR)

#else

/// basket file pointer
#define CEL_FHANDLE FILE*
#define CEL_ISVALIDHANDLE(fd) (fd != NULL)

#define OPEN_CREATE(fd, path) (NULL != (fd = fopen(path, "wb+")))
#define OPEN_EMPTY(fd, path) (NULL != (fd = fopen(path, "wb+")))
#define OPEN_RDONLY(fd, path) (NULL != (fd = fopen(path, "rb")))
#define OPEN_RDWR(fd, path) (NULL != (fd = fopen(path, "rb+")))

#define SAFE_CLOSE_BKT(fd) if(fd){ fclose(fd);fd=NULL;};
#define CLOSE_BKT(fd) fclose(fd)

#define READ_ST(fd, st)  ( sizeof(st) == fread(&st, 1, sizeof(st), fd))
#define READ_HDR(fd, st) ( sizeof(st) == fread(&st, 1, sizeof(st), fd))
#define READ_STRC(fd, st) fread(&st, 1, sizeof(st), fd)
#define READ_PTSZ(fd, pt, size)   ( size == fread(pt, 1, size, fd))

#define WRITE_ST(fd, st) ( sizeof(st) == fwrite(&st, 1, sizeof(st), fd))
#define WRITE_PTSZ(fd, pt, sz)   ( sz == fwrite(pt, 1, sz, fd))

#define FSNYC_ST(fd) (fflush(fd))

#define LSEEK_END(fd, pos) (-1 != fseek(fd, pos, SEEK_END))
#define LSEEK_CUR(fd, pos) (-1 != fseek(fd, pos, SEEK_CUR))
#define LSEEK_SET(fd, pos) (-1 != fseek(fd, pos, SEEK_SET))

#define LTELL(fd) ftell(fd)

#endif//CEL_SYSIO_CALL
//////////////////////////////////////////////////////////////////////////


#define CEL_STATUS_CLOSED (0)
#define CEL_STATUS_OPENED (1)
#define CEL_STATUS_IDLE (2)
#define CEL_STATUS_FULL (3)

////////////////////////////////////////////////////////////////////////////////


#define SIZEOF_CHAR 1
//#define TOTAL_MINUTES_DAY (1440)//24*60
////////////////////////////////////////////////////////////////////////////////

//#pragma pack(push, 1)
/**
  @struct T_TIMESTAMP
  @ingroup CEL
  @brief  cellular timestamp
  */
typedef struct
{
    long sec;	///< seconds
	long usec;  ///< microseconds (1/1000000 seconds)
}T_TIMESTAMP, T_TS;

/// size of structure timestamp
#define SIZEOF_TIMESTAMP sizeof(T_TS)

/**
  @struct T_CEL_TIME
  @ingroup CEL
  @brief  bundle all channel for basket timestamp
  */
typedef struct
{
    T_TIMESTAMP t1[MAX_CAMERA]; ///< start time stamps
    T_TIMESTAMP t2[MAX_CAMERA]; ///< end time stamps
}T_CEL_TIME;

/**
  @brief basket tm value. same as struct tm
  @struct T_CEL_TM
  @ingroup CEL
  */
typedef struct
{
	int year; ///< 1970~
	int mon;  ///< 1~12
	int day;  ///< 1~31
	int hour; ///< 0~23
	int min;  ///< 0~59
	int sec;  ///< 0~59
}T_CEL_TM;

//////////////////////////////////////////////////////////////////////////
#define CEL_BBUF_SIZE (24 * 1024 * 1024)//#define CEL_BBUF_SIZE (12 * 1024 * 1024) ///< basket stream buffer size//ԭ����
#define CEL_IBUF_SIZE (10 * 1024)//#define CEL_IBUF_SIZE (1 * 1024) ///< basket index buffer size//ԭ����
#define CEL_RBUF_SIZE (10 * 1024) //#define CEL_RBUF_SIZE 1024 ///< basket rdb buffer size//ԭ����
//////////////////////////////////////////////////////////////////////////

/**
 @struct T_CEL_RDB_PARAM
 @ingroup CEL_REC
 @brief basket RDB parameter
 */
typedef struct
{
	int  ch;      ///< channel
	long sec;     ///< seconds
	int  evt;     ///< evt type, save flag(continuous, motion, alarm )
	long cid;     ///< basket id
	long idx_pos; ///< index file point
}T_CEL_RDB_PARAM;
#define SIZEOF_RDB_PRM sizeof(T_CEL_RDB_PARAM)

/**
 @struct T_CEL_RDB
 @ingroup CEL_REC
 @brief RDB
 */
typedef struct
{
	long cid;     ///< basket id
	long idx_pos; ///< index file point
}T_CEL_RDB;
#define SIZEOF_RDB 8

/**
 @struct T_BKTREC_INFO
 @ingroup CEL_REC
 @brief  basket recording information
 */
typedef struct
{
	int  save_flag;      ///< save flag. @see _enumSaveFlag
	long cid;            ///< basket id
	char path[LEN_PATH]; ///< current record basket's path
}T_CELREC_INFO;


/**
 @struct CEL_HDD_Info
 @ingroup CEL
 @brief basket MULTI-HDD SKSUNG
 */
typedef struct {
	int  use_stat; //new add
	int  port_num; // new add
	int  smart_res;
	int  is_mnt;
	int  prev_idx;
	int  next_idx;
	long fmt_date;
	long full_date; //new add
	long init_used_size;
	char mnt_path[LEN_PATH];
} T_CEL_HDD_INFO;

/**
 @struct CEL_Mount_Info
 @ingroup CEL
 @brief basket HDD mount information SKSUNG
 */
typedef struct {
	int hdd_cnt;
	int cur_idx;
	int bad_cnt;
	int no_format_cnt;//bruce.milesight version:7.0.4-beta3
	int total_cnt;
	T_CEL_HDD_INFO hdd_info[MAX_HDD_COUNT];
} T_CEL_MNT_INFO;
////////////////////////

/**
 @struct T_TOPMGR
 @ingroup CEL
 @brief management information for multi hdd
 */
typedef struct
{
	long id ;			///< top manager id
	long hdd_idx;		///< hdd index (0 base)
	long next_idx;	///< next recording hdd index
	long fmt_date;	///< format date (seconds)
	long init_used_size;	///< initial used size after hdd format.
	long full_date; ///< time when disk is full
//	char dvrMAC[LEN_PATH] ;	///< DVR's MAC address on mounted.
	int hdd_stat;// full, idle
} T_TOPMGR ;
/////////////////////////

typedef struct _tHddMgr
{
	long rec_date; ///< new add
	long cur_id ;
	long next_id ;
	int  hdd_cnt ;
//	int  hddfull[2] ;
//	char path[2][LEN_PATH] ;
} T_HDDMGR ;

/**
 @struct T_CELMGR_HDR
 @ingroup CEL
 @brief  basket manager header
 */
typedef struct
{
    long id;            ///< header id
    long mgr_id;	        ///< manager file numbering
    long date;		    ///< seconds since the Epoch
    long latest_update;	///<  date of latest update
    long cel_cnt;     ///< basket cnt
    long reserved;
}T_CELMGR_HDR;

/**
 @struct T_CELMGR_BODY
 @ingroup CEL
 @brief  basket manager body. same as Basket header info..@see T_CEL_HDR
 */
typedef struct
{
    long id;              ///< header id
    long cid;             ///< basket id
    char path[LEN_PATH];  ///< basket path
    long latest_update;   ///< latest update
    long pri;       ///< priority, @ref _enumBasketPriority
    long s_type;
	T_CEL_TIME ts;
    long save_flag;
    long rcl_cnt; ///< recycle cnt
    long reserved;
}T_CELMGR_BODY;

/**
 @struct T_CEL_HDR
 @ingroup CEL
 @brief  basket header.
 */
typedef struct
{
    long id;            ///< basket header id
    long cid;           ///< basket number
    long latest_update; ///< update seconds
    long pri;     ///< priority, @ref _enumBasketPriority
    long s_type;        ///< video or audio
	T_CEL_TIME ts;   ///< time stamp
    long save_flag;     ///< basket save flag, stop, full, using...
    long tot_frm_cnt; ///< total frame cnt
    long reserved;  
}T_CEL_HDR;


/**
 @struct T_VIDEO_REC_PARAM
 @ingroup CEL_REC
 @brief  video record param for write stream
 */
typedef struct
{
    long  ch;         ///< channel
    long  frm_size;  ///< frame size
    T_TS  ts;         ///< nvr's timestamp
    int time_lower;  ///< ipc's
    int time_upper; ///< ipc's
    long  evt;      ///< evt type
    int codec_type; ///< codec type
    int strm_fmt; ///< stream format: main or sub
    short frm_type;  ///< frame type
    short frm_rate;  ///< frame rate
    short width;      ///< frame width
    short height;     ///< frame height
    char* buff;       ///< frame buffer
//    char cam_name[16]; ///< channel title

//    short  audio_on; ///< AYK - 0201, with audio or not
//	int   cap_mode;     ///< N/A, BK - 0428 : 2D1, 4CIF, 8CIF, 4D1

}T_VIDEO_REC_PARAM;

/**
 @struct T_AUDIO_REC_PARAM
 @ingroup CEL_REC
 @brief  audio record param for write stream
 */
typedef struct
{
    long ch;            ///< channel
    long frm_size;     ///< frame size
    int codec_type; ///< codec type
    T_TS ts;            ///< timestamp

    long smp_rate;  ///< sampling rate
    long bps; ///< bitrate
    long achn;      /// stero or mono?

    char* buff;         ///< frame buffer
}T_AUDIO_REC_PARAM;

//**************************************new add**************************************************//

//***************************************new add end*******************************************//
///////////////////////////////////////////////////////////////////////////////
/**
 @struct T_STREAM_HDR
 @ingroup CEL
 @brief stream common header
 */
typedef struct
{
    long id;         ///< video or audio
    long ch;         ///< channel
    long frm_size;  ///< frame size
    T_TS ts;         ///< timestamp

    long frm_type;  /// video frame type.i or p frame...

#ifdef PREV_FPOS    // AYK - 0201
    long prev_fpos;   ///< previous file point
#endif

//	long reserved1;
    long long data_pos;
//	long reserved2;
//	long reserved3;
	int time_upper;  ///< ipc's
	int time_lower; ///< ipc's
    long reserved4;

}T_STREAM_HDR;

#ifdef PREV_FPOS    // AYK - 0201
    #define SIZEOF_STREAM_HDR sizeof(T_STREAM_HDR)
#else
    #define SIZEOF_STREAM_HDR (40)
#endif

#ifdef HDR_ENABLE
typedef struct {
	long id;
	long cnt;
	long cid;
	long fpos;
	T_CEL_TIME ts;
	long reserved[6];
}T_HDRFILE_HDR;

#define SIZEOF_HDRFILE_HDR	sizeof(T_HDRFILE_HDR)
#endif
///////////////////////////////////////////////////////////////////////////////
/**
 @struct T_VIDEO_STREAM_HDR
 @ingroup CEL
 @brief video stream header
 */
typedef struct
{
	//    short frm_rate;    ///< frame rate
	long frm_rate;
	long  evt;		///< Continuous, Motion, Sensor, ...
	short width;        ///< frame width
	short height;       ///< frame height
	int codec_type; ///< codec type
	long strm_fmt;
	long reserved2;
}T_VIDEO_STREAM_HDR;

#define SIZEOF_VSTREAM_HDR sizeof(T_VIDEO_STREAM_HDR)

///////////////////////////////////////////////////////////////////////////////
/**
 @struct T_AUDIO_STREAM_HDR
 @ingroup CEL
 @brief audio stream header
 */
typedef struct
{
    long smp_rate;  ///< sampling rate
    long bps; ///< bits per sample
    long achn;      ///< stero or mono?
    long codec_type;         ///< codec type

    long  reserved1;
	long  reserved2;
}T_AUDIO_STREAM_HDR;
#define SIZEOF_ASTREAM_HDR sizeof(T_AUDIO_STREAM_HDR)

#define SIZEOF_FRAME_HDR	SIZEOF_VSTREAM_HDR
#define SIZEOF_HDR 			(SIZEOF_STREAM_HDR + SIZEOF_FRAME_HDR)
///////////////////////////////////////////////////////////////////////////////
/**
 @struct T_INDEX_HDR
 @ingroup CEL
 @brief index header
 */
typedef struct
{
	long   id;        ///< index header id
    long   cid;       ///< Maybe this is same as basket id...
    long   cnt;     ///< cnt index data
	T_CEL_TIME ts; ///< timestamp
    long   reserved;
}T_INDEX_HDR;
#define SIZEOF_INDEX_HDR sizeof(T_INDEX_HDR)

/**
 @struct T_INDEX_DATA
 @ingroup CEL
 @brief index data
 */
typedef struct
{
	long  id;		///< index data struct id
	T_TS  ts;		///< time stamp..sec and usec
	long long  fpos;		///< file position
	long  ch;       ///< channel
	long  s_type;	///< stream type audio, video, etc..
	long  evt;    ///< evt type
	short width;	///< video frame width
	short height;	///< video frame height

//	int   cap_mode;  ///< N/A, capture mode...
#ifndef HDR_ENABLE
    long  reserved1;
#else
	long  hdrpos;
#endif
	long  reserved2;

}T_INDEX_DATA;
#define SIZEOF_INDEX_DATA sizeof(T_INDEX_DATA)

typedef struct {
	CH_MASK ch_mask; ///< record channels mask.
	int chn_max; ///< platform max channels
	int reserved1;
	int reserved2;
	int reserved3;
	int reserved4;
}T_RDB_HDR;
#define SIZEOF_RDB_HDR	sizeof(T_RDB_HDR)

/***************************cellular version********************************************/
//#define CEL_VERSION	"1.0.0" ///< cellular zcm
#define CEL_VERSION	"1.0.1"
typedef struct {
	char ver[CEL_VER_LEN]; ///< cellular version
	long date;///< format date
}T_CEL_INFO;
/************************cellular version***********************************************/

#define CEL_REC_CLOSE_SLIENT 0     ///< N/A
#define CEL_REC_CLOSE_BUF_FLUSH 1  ///< flush cellular stream buffer before file close.

//typedef long long Int64;

//int alphasort_reverse(const struct dirent **a, const struct dirent **b);

void cel_get_local_time(long sec, T_CEL_TM* ptm);

long cel_trans_idx_num2pos(int index_number);

long cel_trans_pos2num(long fpos_index);

long cel_get_min_ts(T_TIMESTAMP *arr_t);

long cel_get_max_ts(T_TIMESTAMP *arr_t);

void cel_set_debug(int on);

int ms_write_data(int fd, void *data, int len);

#ifdef HDR_ENABLE

long cel_trans_hdr_num2pos(long num);

long cel_trans_hdr_pos2num(long fpos);

#endif//end HDR_ENABLE


#ifdef __cplusplus
}
#endif

///////////////////////////////////////////////////////////////////////////////
#endif//_CEL_H_
///////////////////////////////////////////////////////////////////////////////

