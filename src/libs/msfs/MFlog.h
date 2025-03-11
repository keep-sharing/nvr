#ifndef _MFLOG_H
#define _MFLOG_H

#include "msfs_log.h"
#include "msfs_notify.h"


/* storage action flag */
struct log_stat {
    //MF_S32 flag_lang;
    MF_BOOL search_free;
    //MF_BOOL export_free;
};

#define ZERO_BLOCK_SIZE (940 * 1024) // should be best 940k.solin

#define LOG_W_ERR_TOTAL_DEF (4) //default
#define LOG_W_ERR_TOTAL_NAS (2)

#define TRACK_BUFF_LEN      (512 * 1024)
#define VIDEO_TRACK_LEN     (5 * 1024 * 1024)

typedef struct searchTrack {
    MF_S32 fd;
    MF_U64 offStart;
    MF_U64 offEnd;
    char *buff;          //len: TRACK_BUFF_LEN
} SEARCH_TRACK_S;


void
mf_log_search_all(struct search_criteria *cond, struct log_search_result_n *result,
                  MF_BOOL bnotify);

void
mf_log_search_release(struct log_search_result_n *result);

void
mf_log_show_seg(MF_U8 port);

void
mf_log_show_disks();


MF_S32
mf_log_write(struct log_data *log_data, MF_U8 *port);

void
mf_log_search_cancel(MF_BOOL flag);

MF_S32
mf_log_unpack_detail(struct log_detail_pkg *pkg, struct log_data *l_data);

MF_S32
mf_log_pack_detail(struct log_data *l_data, MF_U32 type, void *body, MF_S32 size);

void
mf_log_mark_debug(MF_S8 *detail, MF_S32 subtype);

void
mf_log_search_dup(struct log_data_info *dst, struct log_data_node *dnode);

struct log_data_node *
mf_log_data_dup(struct log_data *data);

void
mf_log_notify_bar(PROGRESS_BAR_E enBar, MF_S32 percent);

void
mf_log_show_mem_usage();

void
mf_log_cache_push(struct log_data_node *cache);//struct log_cache *log_cache,

MF_S32
mf_log_disk_format(MF_S32 fd, struct diskObj *pDev);

MF_S32
mf_log_init(log_search_mac_type *lmac, LOG_USER_CB user_cb);

MF_S32
mf_log_deinit();



#endif

