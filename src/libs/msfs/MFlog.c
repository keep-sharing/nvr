/*
 * ***************************************************************
 * Filename:        MFlog.c
 * Created at:      2017.05.10
 * Description:     log API
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>

#include "MFdisk.h"
#include "MFindex.h"
#include "MFlog.h"
#include "MFcommon.h"
#include "MFmemory.h"
#include "utils.h"
#include "log.h"

struct log_msg {
    MF_U8  loadok; /// will write to nand without completely when boot.solin
    MUTEX_OBJECT loMutex;
    struct log_stat stat;
    struct log_cache cache;
    struct mf_mem_t *pstMem;

    log_search_mac_type lmac;
    LOG_USER_CB user_cb;
};
static struct log_msg g_log;

static struct log_disk_info_n gLogs;

#define LOG_HEAD_NODE_BY_PORT(hnode, port, list) \
    { \
        struct log_head_node *pos = NULL, *n = NULL; \
        if (!list_empty(list)) { \
            list_for_each_entry_safe(pos, n, list, node) { \
                if (pos->port == port) { \
                    hnode = pos; \
                    break; \
                } \
            } \
        } \
    } \

#define LOG_SEG_ADD_ASC(snew, head, member)   \
    {   \
        typeof(*snew) *prev; \
        typeof(*snew) *next; \
        if (list_empty(head) || snew->segInfo.segData.member <= 0) { \
            list_add_tail(&snew->node, head); \
        } else { \
            for (prev = list_entry((head)->prev, typeof(*snew), node); \
                 &prev->node != (head);  prev = next) { \
                if (prev->node.prev == head) { \
                    if (prev->segInfo.segData.member <= snew->segInfo.segData.member) { \
                        list_add_tail(&snew->node, head); \
                    } else { \
                        list_add(&snew->node, head); \
                    } \
                    break; \
                } else { \
                    next = list_entry(prev->node.prev, typeof(*snew), node); \
                    if (prev->segInfo.segData.member > snew->segInfo.segData.member && \
                        next->segInfo.segData.member <= snew->segInfo.segData.member) { \
                        next->node.next = &snew->node; \
                        snew->node.prev = &next->node; \
                        snew->node.next = &prev->node; \
                        prev->node.prev = &snew->node; \
                        break; \
                    } \
                } \
            } \
        } \
    }

#define LOG_DATA_ADD_ASC(nnode, head) \
    { \
        struct log_data_node *dnode = NULL, *dn = NULL; \
        list_for_each_entry_safe_reverse(dnode, dn, head, node) { \
            if (dnode->node.prev == head) { \
                if (dnode->data.cell.pts <= nnode->data.cell.pts) { \
                    list_add(&nnode->node, head); \
                } else if (dnode->node.next != head) { \
                    dnode->node.next->prev = &nnode->node; \
                    nnode->node.next       = dnode->node.next; \
                    nnode->node.prev       = &dnode->node; \
                    dnode->node.next       = &nnode->node; \
                } else { \
                    list_add_tail(&nnode->node, head); \
                } \
                break; \
            } else { \
                if (dnode->data.cell.pts >= nnode->data.cell.pts) { \
                    dnode->node.next->prev = &nnode->node; \
                    nnode->node.next       = dnode->node.next; \
                    nnode->node.prev       = &dnode->node; \
                    dnode->node.next       = &nnode->node; \
                    break; \
                } \
            } \
        } \
    }

///////////////////// log memory manager /////////////////////
static void log_mem_init(struct log_msg *lm)
{
    struct mem_pool_t *pstMpool = mf_comm_mem_get();

    if (pstMpool->logSize) {
        lm->pstMem = mf_mem_pool_create("mflog", pstMpool->logSize);
    }
}

static void log_mem_uninit(struct log_msg *lm)
{
    if (lm->pstMem) {
        mf_mem_state(lm->pstMem, MF_YES);
        mf_mem_pool_destory(lm->pstMem);
    }
}

static inline void *log_mem_alloc(MF_U32 size)
{
    void *p;

    if (g_log.pstMem) {
        p = mf_mem_alloc(g_log.pstMem, size);
    } else {
        p = ms_malloc(size);
    }

    return p;
}

static inline void *log_mem_calloc(MF_U32 n, MF_U32 size)
{
    void *p;

    if (g_log.pstMem) {
        p = mf_mem_calloc(g_log.pstMem, n, size);
    } else {
        p = ms_calloc(n, size);
    }

    return p;
}

static inline void log_mem_free(void *p)
{
    if (g_log.pstMem) {
        mf_mem_free(g_log.pstMem, p);
    } else {
        ms_free(p);
    }
}
//////////////////////////////////////////////////////////////

static MF_U64 log_disk_is_online(MF_U8 port)
{
    return gLogs.diskStat & (BIT1_LSHFT(port - 1));
}

static void log_disk_put_bit(MF_U8 port)
{
    gLogs.diskStat &= ~(BIT1_LSHFT(port - 1));
}

static void log_disk_set_bit(MF_U8 port)
{
    gLogs.diskStat |= (BIT1_LSHFT(port - 1));
}


static MF_S32 log_get_data_block_size(TYPE_EN type)
{
    if (type == DISK_TYPE_NAND) {
        return LOG_NAND_DATA_BLOCK_SIZE;
    } else {
        return DATA_BLOCK_SIZE;
    }
}

static MF_S32 log_get_seg_max(TYPE_EN type)
{
    if (type == DISK_TYPE_NAND) {
        return LOG_NAND_SEG_MAX;
    } else {
        return SEG_ONE_MAX_;
    }
}

static MF_U64 log_get_index_off(MF_U64 off, TYPE_EN type)
{
    if (type == DISK_TYPE_NAND) {
        return REAL_NAND_INDEX_OFF(off);
    } else {
        return REAL_INDEX_OFF(off);
    }
}

static MF_S8 log_get_error_max(TYPE_EN type)
{
    if (type == DISK_TYPE_NAS || type == DISK_TYPE_CIFS) {
        return LOG_W_ERR_TOTAL_NAS;
    } else {
        return LOG_W_ERR_TOTAL_DEF;
    }
}

static void log_seg_node_del(struct log_seg_node *snode)
{
    list_del(&snode->node);
    gLogs.segNum--;
}

static void log_seg_node_add(struct log_head_node *hnode, struct log_seg_node *snode)
{
    ms_mutex_lock(&gLogs.segMutex);
    snode->head = hnode;
    LOG_SEG_ADD_ASC(snode, &gLogs.segList, endTime);
    //list_add_tail(&snode->node, &hnode->segList);
    gLogs.segNum++;
    ms_mutex_unlock(&gLogs.segMutex);
}

static void log_seg_node_free(struct log_head_node *hnode)
{
    struct log_seg_node *pos = NULL, *n = NULL;

    if (!hnode) {
        return ;
    }

    ms_mutex_lock(&gLogs.headMutex);
    if (!list_empty(&gLogs.segList)) {
        ms_mutex_lock(&gLogs.segMutex);
        list_for_each_entry_safe(pos, n, &gLogs.segList, node) {
            if (pos->head == hnode) {
                log_seg_node_del(pos);
                log_mem_free(pos);
            }
        }
        ms_mutex_unlock(&gLogs.segMutex);
    }
    ms_mutex_unlock(&gLogs.headMutex);
}

static void log_head_node_del(struct log_head_node *hnode)
{
    ms_mutex_lock(&gLogs.headMutex);
    list_del(&hnode->node);
    gLogs.diskNum--;
    ms_mutex_unlock(&gLogs.headMutex);
}

static void log_head_node_add(struct log_head_node *hnode)
{
    ms_mutex_lock(&gLogs.headMutex);
    list_add_tail(&hnode->node, &gLogs.headList);
    gLogs.diskNum++;
    ms_mutex_unlock(&gLogs.headMutex);
}

static void log_head_node_free(struct list_head *list)
{
    struct log_head_node *hnode = NULL, *hn = NULL;

    if (!list) {
        return ;
    }

    list_for_each_entry_safe(hnode, hn, list, node) {
        log_seg_node_free(hnode);
        log_head_node_del(hnode);
        hnode->pDev->ops->disk_file_close(hnode->pDev->pstPrivate, hnode->fd);
        log_mem_free(hnode);
    }
}

static void log_head_init(struct log_head_node *hnode, struct diskObj *pDev)
{
    if (!hnode || !pDev) {
        return ;
    }
    memset(hnode, 0, HEAD_NODE_SIZE);
    hnode->port    = pDev->port;
    hnode->type    = pDev->enType;
    hnode->headOff = REAL_HEAD_OFF(pDev->pstHeader->mainLogOff);
    hnode->segOff  = REAL_SEG_OFF(pDev->pstHeader->mainLogOff);
    hnode->indOff  = log_get_index_off(pDev->pstHeader->mainLogOff, pDev->enType);
    hnode->fd      = pDev->ops->disk_log_init(pDev);
    hnode->sector  = pDev->sector;
    hnode->pDev    = pDev;
}

static MF_S32 log_head_info_get(struct log_head_node *hnode)
{
    MF_S32 res = MF_SUCCESS;
    if (!hnode) {
        return MF_FAILURE;
    }

    ms_mutex_lock(&gLogs.headMutex);
    if (hnode->pDev->ops->disk_file_read(hnode->pDev->pstPrivate, \
                                         hnode->fd, &hnode->headInfo, LOG_HEAD_INFO_SIZE_, hnode->headOff) < 0) {
        MFdbg("read head info fail !");
        res = MF_FAILURE;
    }
    ms_mutex_unlock(&gLogs.headMutex);
    return res;
}


static MF_S32 log_head_info_reset(struct log_head_node *hnode)
{
    MF_S32 res = MF_SUCCESS;
    struct log_head_data *hdata = NULL;

    if (!hnode) {
        return MF_FAILURE;
    }

    memset(&hnode->headInfo, 0, LOG_HEAD_INFO_SIZE_);

    hdata = &hnode->headInfo.headData;
    snprintf(hdata->logVer, LOG_VER_SIZE_, "%s", LOG_VER_);
    res = hnode->pDev->ops->disk_file_write(hnode->pDev->pstPrivate, \
                                            hnode->fd, hdata, LOG_HEAD_INFO_SIZE_, hnode->headOff);
    if (res < 0) {
        MFerr("disk_write failed");
    }
    return res;
}

static MF_S32 log_seg_info_1_reset(struct log_head_node *hnode, struct log_seg_info_n *sinfo)
{
    MF_U64 tsegOff;
    struct log_seg_data tsdata;
    struct log_seg_data *sdata;

    if (!hnode || !sinfo) {
        return MF_FAILURE;
    }

    //MFinfo("reset port[%d].seg[%d]", hnode->port, sinfo->segData.segNo);

    sdata   = &sinfo->segData;
    tsegOff = hnode->segOff + (sdata->segNo * LOG_SEG_INFO_SIZE_);
    memcpy(&tsdata, sdata, sizeof(struct log_seg_data));
    sdata->indNum    = 0;
    sdata->full      = LOG_SEG_AVAI;
    sdata->mType     = 0;
    sdata->startTime = 0;
    sdata->endTime   = 0;
    sdata->iCurrOff  = sdata->endOff;
    sdata->dCurrOff  = sdata->startOff;

    if (hnode->fd > 0 && hnode->pDev->ops->disk_file_write(hnode->pDev->pstPrivate, \
                                                           hnode->fd, sinfo, LOG_SEG_INFO_SIZE_, tsegOff) < 0) {
        memcpy(sdata, &tsdata, sizeof(struct log_seg_data));
        hnode->errNum++;
        MFerr("seg reset fail");
        return MF_FAILURE;
    }
    return MF_SUCCESS;
}

static MF_S32 log_seg_info_reset(struct log_head_node *hnode)
{
    MF_S32 i, res = MF_SUCCESS;
    MF_S32 segMax = 0, blkSize = 0;
    struct log_seg_info_n sinfo;
    struct log_seg_data *sdata = NULL;

    if (!hnode) {
        return MF_FAILURE;
    }

    memset(&sinfo, 0, LOG_SEG_INFO_SIZE_);
    sdata = &sinfo.segData;

    segMax  = log_get_seg_max(hnode->type);
    blkSize = log_get_data_block_size(hnode->type);
    for (i = 0; i < segMax; i++) {
        sdata->segNo     = i;
        sdata->startOff  = hnode->indOff + (i * blkSize);
        sdata->endOff    = sdata->startOff + blkSize;

        if (log_seg_info_1_reset(hnode, &sinfo) < 0) {
            MFerr("port[%d] seg info write fail ", hnode->port);
            res = MF_FAILURE;
            break;
        }
    }

    return res;
}

static MF_S32 log_seg_info_get(struct log_head_node *hnode)
{
    MF_S32 i, res = MF_SUCCESS;
    MF_S32 segMax = 0;
    MF_U64 tsegOff;
    struct log_seg_node *snode = NULL;
    struct log_seg_info_n *sinfo = NULL;

    if (!hnode) {
        return MF_FAILURE;
    }

    segMax = log_get_seg_max(hnode->type);
    for (i = 0; i < segMax; i++) {
        snode = log_mem_calloc(1, SEG_NODE_SIZE);
        if (!snode) {
            res = MF_FAILURE;
            break;
        }

        sinfo   = &snode->segInfo;
        tsegOff = hnode->segOff + (i * LOG_SEG_INFO_SIZE_);
        res = hnode->pDev->ops->disk_file_read(hnode->pDev->pstPrivate, \
                                               hnode->fd, sinfo, LOG_SEG_INFO_SIZE_, tsegOff);
        if (res < 0) {
            log_mem_free(snode);
            res = MF_FAILURE;
            MFerr("disk_read failed");
            break;
        }

        if (sinfo->segData.isBadBlk) {
            MFinfo("port:%u seg:%d is badblock", hnode->port, sinfo->segData.segNo);
        }

        log_seg_node_add(hnode, snode);

        //MFinfo("disk[%d] seg[%d] ind[%d] full[%d] surplus:%d", hnode->port, sinfo->segData.segNo,
        //  sinfo->segData.indNum, sinfo->segData.full, hnode->segNum);
    }

    return res;
}

static MF_S32 log_disk_format(struct log_head_node *hnode)
{
    MF_S32 res = MF_SUCCESS;

    if (!hnode) {
        return MF_FAILURE;
    }

    log_seg_node_free(hnode);
    ms_mutex_lock(&gLogs.headMutex);
    if (log_head_info_reset(hnode) < 0) {
        MFerr("head info set fail [%d]", hnode->port);
        res = MF_FAILURE;
    }
    if (res == MF_SUCCESS && log_seg_info_reset(hnode) < 0) {
        MFerr("seg info set fail [%d]", hnode->port);
        res = MF_FAILURE;
    }
    if (res == MF_SUCCESS && log_seg_info_get(hnode) < 0) {
        MFerr("seg info get fail [%d]", hnode->port);
        res = MF_FAILURE;
    }
    ms_mutex_unlock(&gLogs.headMutex);
    MFinfo("format head seg ok");

    return res;
}

static void log_fill_zero(MF_S32 fd, struct diskObj *pDev)
{
    MF_S32 i = LOG_MAX_SIZE / ZERO_BLOCK_SIZE;
    MF_S8 *zero = NULL;
    zero = (MF_S8 *)ms_calloc(1, ZERO_BLOCK_SIZE);

    while (i) {
        if (pDev->ops->disk_file_write(pDev->pstPrivate, fd, zero, ZERO_BLOCK_SIZE, (i - 1) * (ZERO_BLOCK_SIZE)) < 0) {
            MFerr("fill log file error !");
            break;
        }
        i--;
    }
    ms_free(zero);
}

MF_S32 mf_log_disk_format(MF_S32 fd, struct diskObj *pDev)
{
    MF_S32 res = MF_FAILURE;
    struct log_head_node *hnode = NULL;
    hnode = log_mem_calloc(1, HEAD_NODE_SIZE);

    if (fd <= 0 || !pDev || hnode == NULL) {
        return res;
    }

    if (pDev->enType == DISK_TYPE_NAS || pDev->enType == DISK_TYPE_CIFS) {
        log_fill_zero(fd, pDev);
    }

    //memset(hnode, 0, HEAD_NODE_SIZE);
    hnode->fd      = fd;
    hnode->pDev    = pDev;
    hnode->type    = pDev->enType;
    hnode->headOff = REAL_HEAD_OFF(pDev->pstHeader->mainLogOff);
    hnode->segOff  = REAL_SEG_OFF(pDev->pstHeader->mainLogOff);
    hnode->indOff  = log_get_index_off(pDev->pstHeader->mainLogOff, pDev->enType);

    res = log_head_info_reset(hnode);
    if (res >= 0) {
        res = log_seg_info_reset(hnode);
    }

    log_mem_free(hnode);

    return res;
}

static MF_S32 log_head_info_check(struct log_head_node *hnode)
{
    MF_S32 res = MF_SUCCESS;
    if (!hnode) {
        return  res;
    }

    if (strcmp(hnode->headInfo.headData.logVer, LOG_VER_)) {
        MFinfo("[%d] %s version not same !!!", hnode->port, hnode->headInfo.headData.logVer);
        res = MF_FAILURE;
    }

    return res;
}


static MF_BOOL log_search_cond_subtype(MF_U64 ssubtype[], MF_S32 count, MF_U64 wsubtype)
{
    int i = 0;
    for (; i < count; i++) {
        if (ssubtype[i] == wsubtype) {
            return MF_TRUE;
        }
    }
    return MF_FALSE;
}


static MF_S32 log_search_cond_cmp(struct log_data_data *ddata, struct search_criteria *cond)
{
    MF_S32 chnId;
    MF_S32 res = MF_FAILURE;

    if (!ddata || !cond) {
        return res;
    }

    /* for msfs debug */
    if ((ddata->cell.mType & g_log.lmac.mDbg) && !(cond->mainType & g_log.lmac.mDbg)) { // ust debug to show
        return res;
    }

    if (!((ddata->cell.mType | g_log.lmac.mAll) & cond->mainType)) {
        return res;
    }

    if ((cond->count > 0 && cond->subType[0] != g_log.lmac.sAll) &&
        !log_search_cond_subtype(cond->subType, cond->count, ddata->cell.sType)) {
        return res;
    }

    if (ddata->cell.pts <= 0 || ddata->cell.pts < cond->start_time ||
        ddata->cell.pts > cond->end_time) {
        return res;
    }

    chnId = ddata->cell.chanNo;
    if (chnId > 0 && !log_channo_is_disk(ddata->cell.mType, ddata->cell.sType)) {
        if ((cond->chnMask & ((MF_U64)1 << (chnId - 1))) == 0) {
            return res;
        }
    } else if (cond->bSeachNoChn == MF_NO) {
        return res;
    }

    return MF_SUCCESS;
}

static MF_S32 log_search_data_read(struct log_head_node *hnode, struct log_data_node *dnode,
                                   struct log_index_data *idata, struct search_criteria *cond, struct searchTrack *track)
{
    if (!hnode || !dnode || !idata || !cond || !track) {
        return MF_FAILURE;
    }

    /* get data */
    /* Promblem Number: ID1044905     Author:wuzh,   Date:2021/4/16
       Description    : 通道过滤搜索log耗时长；优化：一次读 TRACK_BUFF_LEN(512k) 字节到内存，减少io */
    if (hnode->fd == track->fd && idata->dataOff >= track->offStart && idata->dataOff <= track->offEnd) {
        memcpy(&dnode->data, &track->buff[idata->dataOff - track->offStart], idata->dataSize);
    } else {
        track->fd = hnode->fd;
        track->offEnd = idata->dataOff;
        track->offStart = (track->offEnd + idata->dataSize) > TRACK_BUFF_LEN ?
                          (track->offEnd + idata->dataSize) - TRACK_BUFF_LEN : 0;
        memset(track->buff, 0, TRACK_BUFF_LEN);
        if (hnode->pDev->ops->disk_file_read(hnode->pDev->pstPrivate,
                                             hnode->fd, track->buff, TRACK_BUFF_LEN, track->offStart) < 0) {
            MFerr("disk_read failed");
            return MF_FAILURE;
        }
        memcpy(&dnode->data, &track->buff[idata->dataOff - track->offStart], idata->dataSize);
    }

    if (dnode->data.crc != crc_32(&dnode->data, LOG_DATA_POS(crc))) {
        return MF_FAILURE;
    }

    /* compare data by condition */
    if (log_search_cond_cmp(&dnode->data, cond) < 0) {
        return MF_FAILURE;
    }

    /* get detail */
    if (idata->detailSize > 0 &&
        hnode->pDev->ops->disk_file_read(hnode->pDev->pstPrivate, \
                                         hnode->fd, &dnode->detail, idata->detailSize, idata->detailOff) < 0) {
        MFerr("disk_read failed");
        hnode->errNum++;
        return MF_FAILURE;
    }

    return MF_SUCCESS;
}

static void log_search_data_add(struct log_data_node *nnode, struct list_head *head)
{
    if (!nnode || !head) {
        return ;
    }
    if (list_empty(head)) {
        list_add(&nnode->node, head);
        return ;
    }
    LOG_DATA_ADD_ASC(nnode, head);
}

static void log_search_default_option(struct search_criteria *cond, struct log_search_result_n *result,
                                      MF_BOOL bnotify, MF_S32 start)
{
    MF_S32 count = 0, i, cur = 0;
    MF_S32 tno   = 0, tsla = 0;
    MF_U64 tindOff;

    MF_S32 sgap = 0, sstart = 0;

    struct log_seg_node *snode   = NULL, *sn = NULL;
    struct log_seg_data *sdata   = NULL;
    struct log_index_data *idata = NULL;
    struct log_data_node *dnode  = NULL;
    struct log_index_info_n *iinfos = NULL;
    struct searchTrack track;

    if (!cond || !result) {
        return ;
    }

    // sync search progress %
    if (bnotify) {
        sgap = gLogs.segNum > 0 ? ((94 - start) / gLogs.segNum) : (94 - start);
    }

    if (g_log.stat.search_free) {
        return ;
    }

    iinfos = (struct log_index_info_n *)log_mem_calloc(1, LOG_INDEX_SIZE_ * LOG_SEARCH_MAX_);
    if (iinfos == NULL) {
        MFerr("log_index_info_n malloc null!");
        return ;
    }

    memset(&track, 0, sizeof(track));
    track.buff = (char *)log_mem_calloc(1, TRACK_BUFF_LEN);
    if (track.buff == NULL) {
        MFerr("search track buff malloc null!");
        log_mem_free(iinfos);
        return ;
    }

    list_for_each_entry_safe_reverse(snode, sn, &gLogs.segList, node) {
        if (g_log.stat.search_free) {
            break;
        }

        /* check head-cell info and use available disk */
        if (snode->head->buninit || snode->segInfo.segData.isBadBlk ||
            snode->head->errNum >= log_get_error_max(snode->head->type)) {
            continue;
        }

        /* filter the range of time */
        if (cond->start_time > snode->segInfo.segData.endTime ||
            cond->end_time < snode->segInfo.segData.startTime) {
            continue;
        }

        cur = LOG_SEARCH_MAX_ - result->total;
        cur = cur < 0 ? LOG_SEARCH_MAX_ : cur;

        sdata = &snode->segInfo.segData;
        count = sdata->indNum;
        tsla  = 0;
        tno   = 0;

        if (!((sdata->mType | g_log.lmac.mAll) & cond->mainType)) {
            continue;
        }

        while (count > 0 && result->total < LOG_SEARCH_MAX_) {
            if (g_log.stat.search_free) {
                break;
            }

            tindOff = sdata->iCurrOff + (tsla * LOG_INDEX_SIZE_);
            if (tindOff >= sdata->endOff) {
                break;
            }

            tno    = count > cur ? cur : count;
            tsla  += tno;
            count -= tno;

            if (snode->head->pDev->ops->disk_file_read(snode->head->pDev->pstPrivate, \
                                                       snode->head->fd, iinfos, tno * LOG_INDEX_SIZE_, tindOff) < 0) {
                MFerr("index fail port[%d].seg[%d]", snode->head->port, sdata->segNo);
                snode->head->errNum++;
                continue;
            }

            for (i = 0; i < tno; i++) {
                if (g_log.stat.search_free) {
                    break;
                }

                idata = &iinfos[i].indexData;
                if (idata->crc != crc_32(idata, LOG_INDEX_POS(crc))) {
                    continue;
                }

                dnode = log_mem_calloc(1, DATA_NODE_SIZE);
                if (!dnode) {
                    continue;
                }

                memset(dnode, 0, DATA_NODE_SIZE);
                if (log_search_data_read(snode->head, dnode, idata, cond, &track) < 0) {
                    log_mem_free(dnode);
                    continue;
                }

                dnode->detailSize = idata->detailSize;
                log_search_data_add(dnode, &result->list);
                result->total++;

                if (result->total >= LOG_SEARCH_MAX_) {
                    break;
                }

                //printf("logadd: p[%d].seg[%d].mtype[%u] stype[%llu] r-total[%d]\n",
                //  hnode->port, sdata->segNo, dnode->data.mType, dnode->data.sType,
                //  result->total);
            }
        }

        if (bnotify) {
            sstart += sgap;
            mf_log_notify_bar(PROGRESS_BAR_LOG_SEARCH, start + sstart);
        }
        if (result->total >= LOG_SEARCH_MAX_) {
            break;
        }
    }

    log_mem_free(iinfos);
    log_mem_free(track.buff);
}

static void log_search_all(struct search_criteria cond, struct log_search_result_n *result, MF_BOOL bnotify)
{
    MF_S32 start = 0;

    ms_mutex_lock(&gLogs.segMutex);
    if (bnotify) {
        start = mf_random(1, 5);
        mf_log_notify_bar(PROGRESS_BAR_LOG_SEARCH, start);
    }

    // search from disks
    log_search_default_option(&cond, result, bnotify, start);

    if (bnotify) {
        mf_log_notify_bar(PROGRESS_BAR_LOG_SEARCH, 95);
    }

    MFinfo("search total:%d", result->total);
    ms_mutex_unlock(&gLogs.segMutex);
}

void mf_log_search_all(struct search_criteria *cond, struct log_search_result_n *result, MF_BOOL bnotify)
{
    result->total = 0;
    INIT_LIST_HEAD(&result->list);
    log_search_all(*cond, result, bnotify);
}

void mf_log_search_cancel(MF_BOOL flag)
{
    g_log.stat.search_free = flag;
    return ;
}

void mf_log_search_dup(struct log_data_info *dst, struct log_data_node *dnode)
{
    if (!dst || !dnode) {
        return ;
    }

    memset(dst, 0x0, sizeof(struct log_data_info));

    dst->chan_no        = dnode->data.cell.chanNo;
    dst->mainType       = dnode->data.cell.mType;
    dst->parameter_type = dnode->data.cell.paraType;
    dst->pts            = dnode->data.cell.pts;
    dst->remote         = dnode->data.cell.remote;
    dst->subType        = dnode->data.cell.sType;
    dst->stream_type    = dnode->data.cell.streamType;
    dst->isplay         = dnode->data.cell.isPlay;
    dst->dataSize       = dnode->detailSize;
    snprintf(dst->ip, sizeof(dst->ip), "%s", dnode->data.cell.ip);
    snprintf(dst->user, sizeof(dst->user), "%s", dnode->data.cell.user);
    memcpy(dst->detail, dnode->detail, LOG_DETAIL_MAX_);
}

void mf_log_search_release(struct log_search_result_n *result)
{
    struct log_data_node *pos = NULL, *n = NULL;

    list_for_each_entry_safe(pos, n, &result->list, node) {
        list_del(&pos->node);
        log_mem_free(pos);
    }
}


struct log_data_node *mf_log_data_dup(struct log_data *data)
{
    if (!data) {
        return NULL;
    }

    struct log_data_node *dnode = log_mem_calloc(1, DATA_NODE_SIZE);

    if (dnode) {
        dnode->logLevel             = data->logLevel;
        dnode->detailSize           = data->log_data_info.dataSize;//data->logLevel & LOG_LEVEL_CRIT ? 0 :
        dnode->data.cell.chanNo     = data->log_data_info.chan_no < 0 ? 0 : data->log_data_info.chan_no;
        dnode->data.cell.mType      = data->log_data_info.mainType;
        dnode->data.cell.paraType   = data->log_data_info.parameter_type;
        dnode->data.cell.pts        = data->log_data_info.pts;
        dnode->data.cell.remote     = data->log_data_info.remote;
        dnode->data.cell.sType      = data->log_data_info.subType;
        dnode->data.cell.streamType = data->log_data_info.isplay ? data->log_data_info.stream_type : STREAM_TYPE_NONE;
        dnode->data.cell.isPlay     = data->log_data_info.isplay;
        dnode->data.cell.enDtc      = data->log_data_info.enDtc;
        //strncpy(dnode->data.ptc, dst->ptc, strlen(dst->ptc));
        strncpy(dnode->data.cell.ip, data->log_data_info.ip, strlen(data->log_data_info.ip));
        strncpy(dnode->data.cell.user, data->log_data_info.user, strlen(data->log_data_info.user));
        memcpy(dnode->detail, data->log_data_info.detail, LOG_DETAIL_MAX_);
    }
    return dnode;
}

static void log_data_write_error(struct log_seg_node *snode, MF_S32 wsize, MF_U64 woff)
{
    MF_U64 tsegOff;

    if (!snode || snode->head->type == DISK_TYPE_NAS || snode->head->type == DISK_TYPE_RAID
        || snode->head->type == DISK_TYPE_CIFS) {
        return ;
    }
    if (DISK_ERR_BADBLOCK == mf_disk_error(snode->head->fd, woff, wsize, snode->head->sector)) {
        MFinfo("port:%u seg:%d is bad seg", snode->head->port, snode->segInfo.segData.segNo);
        snode->segInfo.segData.isBadBlk = MF_TRUE;
        tsegOff   = snode->head->segOff + (snode->segInfo.segData.segNo * LOG_SEG_INFO_SIZE_);
        if (snode->head->pDev->ops->disk_file_write(snode->head->pDev->pstPrivate, \
                                                    snode->head->fd, &snode->segInfo, LOG_SEG_INFO_SIZE_, tsegOff) < 0) {
            MFerr("seg%d write error set badblock fail !", snode->segInfo.segData.segNo);
        }
    }
}

static MF_S32 log_data_set(struct log_data_node *dnode, struct log_seg_node *snode)
{
    MF_U64 tsegOff;
    MF_U64 tsegiCOff;
    MF_U64 tsegdCOff;
    struct log_index_info_n iinfo;
    struct log_index_data *idata = NULL;
    struct log_data_data *ddata = NULL;
    struct log_seg_data *sdata = NULL;
    struct log_seg_data tsdata;

    if (!dnode || !snode) {
        return MF_UNKNOWN;
    }

    sdata     = &snode->segInfo.segData;
    tsegiCOff = sdata->iCurrOff - LOG_INDEX_SIZE_;
    tsegdCOff = sdata->dCurrOff + LOG_DATA_SIZE_ + dnode->detailSize;
    tsegOff   = snode->head->segOff + (sdata->segNo * LOG_SEG_INFO_SIZE_);

    if (tsegdCOff > tsegiCOff || tsegdCOff > sdata->endOff || tsegiCOff < sdata->startOff) {
        sdata->full = LOG_SEG_FULL;

        if (snode->head->pDev->ops->disk_file_write(snode->head->pDev->pstPrivate, \
                                                    snode->head->fd, &snode->segInfo, LOG_SEG_INFO_SIZE_, tsegOff) < 0) {
            MFerr("seg full set fail !");
        }

        return MF_UNKNOWN;
    }

    memset(&iinfo, 0, LOG_INDEX_SIZE_);
    ddata             = &dnode->data;
    idata             = &iinfo.indexData;
    idata->mType     |= ddata->cell.mType;
    idata->pts        = ddata->cell.pts;
    idata->dataSize   = LOG_DATA_SIZE_;
    idata->dataOff    = sdata->dCurrOff;
    idata->detailSize = dnode->detailSize;
    idata->detailOff  = sdata->dCurrOff + LOG_DATA_SIZE_;

    //MFinfo("W data.t[%ld] mai[%d] sub[%llu]", ddata->pts, ddata->mType, ddata->sType);

    ddata->crc = crc_32(ddata, LOG_DATA_POS(crc));
    if (snode->head->pDev->ops->disk_file_write(snode->head->pDev->pstPrivate, \
                                                snode->head->fd, ddata, idata->dataSize, idata->dataOff) < 0) {
        MFerr("write data fail %d", snode->head->port);
        log_data_write_error(snode, idata->dataSize, idata->dataOff);
        return MF_FAILURE;
    }
    if (snode->head->pDev->ops->disk_file_write(snode->head->pDev->pstPrivate, \
                                                snode->head->fd, dnode->detail, idata->detailSize, idata->detailOff) < 0) {
        MFerr("write detail fail %d", snode->head->port);
        log_data_write_error(snode, idata->detailSize, idata->detailOff);
        return MF_FAILURE;
    }

    idata->crc = crc_32(idata, LOG_INDEX_POS(crc));
    if (snode->head->pDev->ops->disk_file_write(snode->head->pDev->pstPrivate, \
                                                snode->head->fd, &iinfo, LOG_INDEX_SIZE_, tsegiCOff) < 0) {
        MFerr("write index fail %d", snode->head->port);
        log_data_write_error(snode, LOG_INDEX_SIZE_, tsegiCOff);
        return MF_FAILURE;
    }

    memcpy(&tsdata, sdata, sizeof(struct log_seg_data));
    sdata->indNum++;
    sdata->mType    |= ddata->cell.mType;
    sdata->iCurrOff  = tsegiCOff;
    sdata->dCurrOff  = tsegdCOff;

    if (sdata->startTime == 0 && sdata->endTime == 0) {
        sdata->startTime = sdata->endTime = ddata->cell.pts;
    } else if (ddata->cell.pts > sdata->endTime) {
        sdata->endTime = ddata->cell.pts;
    } else if (ddata->cell.pts < sdata->startTime) {
        sdata->startTime = ddata->cell.pts;
    }

    if (snode->head->pDev->ops->disk_file_write(snode->head->pDev->pstPrivate, \
                                                snode->head->fd, &snode->segInfo, LOG_SEG_INFO_SIZE_, tsegOff) < 0) {
        memcpy(sdata, &tsdata, sizeof(struct log_seg_data));
        MFerr("write seg fail %d", snode->head->port);
        return MF_FAILURE;
    }

    return MF_SUCCESS;
}

static MF_S32 log_disk_write(struct log_data_node *dnode)
{
    MF_PTS tmin = 0;
    MF_S32 res = MF_SUCCESS;
    MF_S32 wstat = MF_FALSE;
    MF_BOOL bcrit;

    struct log_seg_node *snode = NULL, *sn = NULL, *snodeMin = NULL;
    struct log_seg_data *sdata = NULL;

    if (!dnode) {
        return MF_FAILURE;
    }
    if ((g_log.loadok & (SE_BOOT_COMPLETE | SE_DISK_LOAD_OK)) &&
        (dnode->data.cell.mType & g_log.lmac.mDbg)) {
        dnode->logLevel |= LOG_LEVEL_CRIT;
    }

    bcrit = dnode->logLevel & LOG_LEVEL_CRIT ? MF_TRUE : MF_FALSE;

    ms_mutex_lock(&gLogs.segMutex);
    list_for_each_entry_safe(snode, sn, &gLogs.segList, node) {
        sdata = &snode->segInfo.segData;

        /* check head-cell info and use available disk */
        if (snode->head->buninit || sdata->isBadBlk ||
            snode->head->errNum >= log_get_error_max(snode->head->type)) {
            continue;
        }

        /* strip nand if not critical */
        if ((bcrit == MF_TRUE && snode->head->type != DISK_TYPE_NAND) ||
            (bcrit == MF_FALSE && snode->head->type == DISK_TYPE_NAND)) {
            continue;
        }

        /* save the mininum start-time for seg-reset when no seg availabe */
        if (tmin == 0 || sdata->startTime < tmin) {
            tmin     = sdata->startTime;
            snodeMin = snode;
        }

        if ((sdata->full & LOG_SEG_FULL) && (sdata->startTime != 0)) {
            continue;
        }

        /* start to write data */
        res = log_data_set(dnode, snode);
        if (res == MF_FAILURE && !snode->segInfo.segData.isBadBlk) {
            snode->head->errNum++;
            continue;
        }

        wstat = MF_TRUE;
        break;
    }

    if (wstat == MF_FALSE && snodeMin) {
        if ((snodeMin->head->buninit == MF_FALSE) &&
            (snodeMin->segInfo.segData.isBadBlk == MF_FALSE) && // do not write to badblock in the seg
            (snodeMin->head->errNum < log_get_error_max(snodeMin->head->type)) &&
            log_seg_info_1_reset(snodeMin->head, &snodeMin->segInfo) == MF_SUCCESS) {
            /* readd to list by sorting */
            list_del(&snodeMin->node);
            LOG_SEG_ADD_ASC(snodeMin, &gLogs.segList, endTime);

            if (snodeMin->head->buninit == MF_FALSE) {
                res = log_data_set(dnode, snodeMin);
                if (res == MF_FAILURE && !snode->segInfo.segData.isBadBlk) {
                    snodeMin->head->errNum++;
                }
            }
        } else {
            res = MF_FAILURE;
            //MFinfo("no seg and reset seg fail");
        }
    }
    ms_mutex_unlock(&gLogs.segMutex);

    return res;
}


void mf_log_show_disks()
{
    struct log_head_node *hnode = NULL, *hn = NULL;
    struct log_seg_node *snode = NULL, *sn = NULL;
    MF_S32 indnum = 0;

    ms_mutex_lock(&gLogs.headMutex);
    MFprint("\n---- all ----\n");
    list_for_each_entry_safe(hnode, hn, &gLogs.headList, node) {
        MFprint("-port[%2d]", hnode->port);
    }
    MFprint("-segN[%2u] diskS[0x%llx]", gLogs.segNum, gLogs.diskStat);
    ms_mutex_unlock(&gLogs.headMutex);

    MFprint("\n\n---- used ----\n");
    list_for_each_entry_safe(hnode, hn, &gLogs.headList, node) {
        indnum = 0;
        ms_mutex_lock(&gLogs.segMutex);
        list_for_each_entry_safe(snode, sn, &gLogs.segList, node) {
            if (snode->head == hnode && !snode->segInfo.segData.isBadBlk) {
                indnum += snode->segInfo.segData.indNum;
            }
        }
        MFprint("-port[%2d]count[%6d]", hnode->port, indnum);
        ms_mutex_unlock(&gLogs.segMutex);
    }
    MFprint("\n---- ----\n");
}

void mf_log_show_seg(MF_U8 port)
{
    struct log_seg_data *sdata = NULL;
    struct log_seg_node *snode = NULL, *sn = NULL;
    struct log_head_node *hnode = NULL;
    MF_S8 stime[32] = {0};
    MF_S8 etime[32] = {0};

    LOG_HEAD_NODE_BY_PORT(hnode, port, &gLogs.headList);
    if (hnode) {
        ms_mutex_lock(&gLogs.segMutex);
        MFprint("ver[%s]->\n", hnode->headInfo.headData.logVer);
        MFprint("port[%2d]->\n", hnode->port);
        list_for_each_entry_safe(snode, sn, &gLogs.segList, node) {
            if (snode->head == hnode) {
                sdata = &snode->segInfo.segData;
                time_to_string_local(sdata->startTime, stime);
                time_to_string_local(sdata->endTime, etime);
                MFprint(" seg%2d bad[%d]-> ind[%6d] full[%3u] st[%s] et[%s]\n",
                        sdata->segNo, sdata->isBadBlk, sdata->indNum, sdata->full,
                        sdata->startTime > 0 ? stime : "0",
                        sdata->endTime > 0 ? etime : "0");
                MFprint("    ->so[%10llu] eo[%10llu] ico[%10llu] dco[%10llu]\n",
                        sdata->startOff, sdata->endOff, sdata->iCurrOff, sdata->dCurrOff);
            }
        }
        ms_mutex_unlock(&gLogs.segMutex);
    }
}


static MF_S32 log_disk_init(struct diskObj *pDev)
{
    MF_S32 res = MF_FAILURE;
    struct log_head_node *hnode = NULL;

    if (!pDev) {
        return res;
    }

    ms_mutex_lock(&gLogs.headMutex);
    if (log_disk_is_online(pDev->port)) {
        ms_mutex_unlock(&gLogs.headMutex);
        return res;
    }

    if ((hnode = log_mem_calloc(1, HEAD_NODE_SIZE)) == NULL) {
        ms_mutex_unlock(&gLogs.headMutex);
        return res;
    }

    log_disk_set_bit(pDev->port);
    ms_mutex_unlock(&gLogs.headMutex);

    log_head_init(hnode, pDev);
    if (hnode->fd <= 0) {
        MFerr("[%d][%s] fd NG", pDev->port, pDev->vendor);
        goto ERR;
    }

    if (log_head_info_get(hnode) < 0) {
        MFerr("[%d][%s] head read NG", pDev->port, pDev->vendor);
        hnode->pDev->ops->disk_file_close(hnode->pDev->pstPrivate, hnode->fd);
        goto ERR;
    }

    if (log_head_info_check(hnode) < 0) {
        if (log_disk_format(hnode) < 0) {
            log_seg_node_free(hnode);
            hnode->pDev->ops->disk_file_close(hnode->pDev->pstPrivate, hnode->fd);
            MFerr("[%d]head check reset NG", pDev->port);
            goto ERR;
        }
    } else {
        if (log_seg_info_get(hnode) < 0) {
            MFerr("[%d][%s] seg read NG", pDev->port, pDev->vendor);
            log_seg_node_free(hnode);
            hnode->pDev->ops->disk_file_close(hnode->pDev->pstPrivate, hnode->fd);
            goto ERR;
        }
    }

    log_head_node_add(hnode);
    MFinfo("port[%d]", pDev->port);

    return MF_SUCCESS;

ERR:
    log_disk_put_bit(pDev->port);
    log_mem_free(hnode);
    return res;
}

static MF_S32 log_disk_uninit(MF_U8 port)
{
    struct log_head_node *hnode = NULL;

    if (port <= 0) {
        return MF_FAILURE;
    }

    ms_mutex_lock(&gLogs.headMutex);
    if (log_disk_is_online(port)) {
        log_disk_put_bit(port);
        LOG_HEAD_NODE_BY_PORT(hnode, port, &gLogs.headList);
    }
    ms_mutex_unlock(&gLogs.headMutex);

    if (hnode) {
        MFinfo("port[%d]", port);
        hnode->buninit = MF_TRUE;
        log_head_node_del(hnode);
        log_seg_node_free(hnode);
        hnode->pDev->ops->disk_file_close(hnode->pDev->pstPrivate, hnode->fd);
        log_mem_free(hnode);
    }

    return MF_SUCCESS;
}


MF_S32 mf_log_unpack_detail(struct log_detail_pkg *pkg, struct log_data *l_data)
{
    if (!l_data || !pkg) {
        return MF_FAILURE;
    }

    memset(pkg, 0, sizeof(struct log_detail_pkg));
    memcpy(pkg, l_data->log_data_info.detail, sizeof(struct log_detail_pkg));
    if (pkg->crc > 0 && pkg->crc == check_sum(pkg, LOG_DETAIL_POS(crc))) {
        return MF_SUCCESS;
    } else {
        return MF_FAILURE;
    }
}

MF_S32 mf_log_pack_detail(struct log_data *l_data, MF_U32 type, void *body, MF_S32 size)
{
    struct log_detail_pkg pkg;

    memset(&pkg, 0, sizeof(pkg));
    pkg.head.type = type;
    pkg.head.size = size;
    memcpy(pkg.body, body, size);
    pkg.crc = check_sum(&pkg, LOG_DETAIL_POS(crc));// check sum data
    memcpy(l_data->log_data_info.detail, &pkg, sizeof(pkg));
    l_data->log_data_info.dataSize = LOG_DETAIL_POS(reserved);
    return MF_SUCCESS;
}


static void log_cache_free(struct log_data_node *dnode)
{
    if (!dnode) {
        return;
    }
    log_mem_free(dnode);
}

static struct log_data_node *log_cache_pop(struct log_cache *log_cache)
{
    struct log_data_node *dnode;

    ms_mutex_lock(&log_cache->mutex);
    dnode = list_first_entry_or_null(&log_cache->node, struct log_data_node, node);
    if (dnode) {
        list_del(&dnode->node);
        g_log.cache.count--;
    }
    ms_mutex_unlock(&log_cache->mutex);

    return dnode;
}

void mf_log_cache_push(struct log_data_node *dnode)
{
    if (g_log.cache.run != MF_YES) {
        log_cache_free(dnode);
        return ;
    }

    if (g_log.cache.count > MAX_LOG_CACHE) {
        log_cache_free(log_cache_pop(&g_log.cache));
    }

    ms_mutex_lock(&g_log.cache.mutex);
    dnode->data.cell.pts = time(NULL);
    //time_to_string_local(dnode->data.pts, dnode->data.ptc);

    list_add_tail(&dnode->node, &g_log.cache.node);
    g_log.cache.count++;
    ms_mutex_unlock(&g_log.cache.mutex);
}


static MF_U64 log_mark_debug_subtype(MF_S32 subtype)
{
    switch (subtype) {
        case MF_WAR:
            return SUB_DEBUG_WAR;
        case MF_INFO:
            return SUB_DEBUG_INF;
        case MF_DBG:
            return SUB_DEBUG_DBG;
        case MF_ERR:
            return SUB_DEBUG_ERR;
        default:
            break;
    }
    return 0;
}

void mf_log_mark_debug(MF_S8 *detail, MF_S32 subtype)
{
    if (g_log.cache.run != MF_YES || !detail) {
        return ;
    }

    struct log_data ldata;
    struct log_data_node *dnode = NULL;

    memset(&ldata, 0, sizeof(ldata));
    ldata.log_data_info.mainType = MAIN_DEBUG;
    ldata.log_data_info.subType  = log_mark_debug_subtype(subtype);
    mf_log_pack_detail(&ldata, OP_MS_DEBUG, detail, LOG_PKG_DATA_MAX);

    dnode = mf_log_data_dup(&ldata);
    if (dnode) {
        mf_log_cache_push(dnode);
    }

}


static MF_U8 log_write_err_over()
{
    MF_U8 port = 0;
    struct log_head_node *hnode = NULL, *hn = NULL;

    ms_mutex_lock(&gLogs.headMutex);
    list_for_each_entry_safe(hnode, hn, &gLogs.headList, node) {
        if (hnode->errNum >= log_get_error_max(hnode->type)) {
            port = hnode->port;
            break;
        }
    }
    ms_mutex_unlock(&gLogs.headMutex);
    return port;
}

static void *log_write_task(void *argv)
{
    MF_S32 ret = 0;
    MF_U8 port = 0;
    struct log_cache *g_caches = (struct log_cache *)argv;
    struct log_data_node *dnode = NULL;

    ms_task_set_name("LOG_WRITE");
    while (g_caches->run) {
        dnode = log_cache_pop(g_caches);
        if (dnode) {
            ret = log_disk_write(dnode);
            log_cache_free(dnode);

            if (ret < 0 && (port = log_write_err_over()) > 0) {
                log_disk_uninit(port);
                MFerr("write over err port:%d", port);
            }
        } else {
            usleep(50000);
        }
    }

    return MF_SUCCESS;
}

MF_S32 mf_log_write_start(void)
{
    INIT_LIST_HEAD(&g_log.cache.node);
    ms_mutex_init(&g_log.cache.mutex);
    g_log.cache.run = MF_YES;
    g_log.cache.count = 0;
    ms_task_create_join_stack_size(&g_log.cache.pthread, 16 * 1024, log_write_task, &g_log.cache);
    return MF_SUCCESS;
}

MF_S32 mf_log_write_end(void)
{
    g_log.cache.run = MF_FALSE;
    if (g_log.cache.pthread) {
        ms_task_join(&g_log.cache.pthread);
    }
    return MF_SUCCESS;
}

void mf_log_notify_bar(PROGRESS_BAR_E enBar, MF_S32 percent)
{
    PROGRESS_BAR_T bar;
    memset(&bar, 0, sizeof(PROGRESS_BAR_T));
    bar.percent = percent;
    msfs_notify_progress_bar_state(enBar, &bar);
}

void mf_log_show_mem_usage()
{
    if (g_log.pstMem) {
        MFinfo("mf_log_show_mem_usage");
        mf_mem_state(g_log.pstMem, MF_YES);
    }
}

static void log_user_notify(EVENT_E event, struct diskObj *disk)
{
    if (g_log.user_cb) {
        if (disk) {
            struct log_cb_para para;
            memset(&para, 0, sizeof(para));
            para.port   = disk->port;
            para.entype = disk->enType;
            para.stat   = disk->enState;
            para.rw     = disk->enRw;
            snprintf(para.vendor, sizeof(para.vendor), "%s", disk->vendor);
            g_log.user_cb(event, &para);
        } else {
            g_log.user_cb(event, NULL);
        }
    }
}

static void *log_init_task_loading(void *argv)
{
    struct list_head *diskList;
    struct diskObj *disk;

    ms_task_set_name("log_init_task_loading");
    ms_mutex_lock(&g_log.loMutex);
    if (!(g_log.loadok & SE_DISK_LOAD_OK)) {
        ms_mutex_unlock(&g_log.loMutex);
        return NULL;
    }

    diskList = mf_disk_get_rd_list(MF_NO);
    list_for_each_entry(disk, diskList, node) {
//      log_user_notify(MSFS_EVENT_DISK_ADD, disk);

        if (disk->enType == DISK_TYPE_USB ||
            disk->enType == DISK_TYPE_IN_RAID ||
            disk->enType == DISK_TYPE_GLOBAL_SPARE ||
            (disk->enType == DISK_TYPE_ESATA && disk->bRec == MF_NO)) {
            continue;
        }

        if (disk->enState == DISK_STATE_NORMAL && disk->ops->disk_log_init) {
            log_disk_init(disk);
        }
    }
    mf_disk_put_rw_list(diskList);

    g_log.loadok &= ~SE_DISK_LOAD_OK;
    ms_mutex_unlock(&g_log.loMutex);

    log_user_notify(MSFS_EVENT_DISK_ALL_OKAY, NULL);

    return NULL;
}

static void mf_log_event_cb(MODULE_E from, EVENT_E event, void *argv)
{
    struct diskObj *disk = (struct diskObj *)argv;

    if (event == MSFS_EVENT_DISK_ALL_OKAY) {
        mf_comm_task_submit(log_init_task_loading, NULL);
        return ;
    }

    MFinfo("from = %d event = %d begin\n", from, event);
    switch (event) {
        case MSFS_EVENT_BOOT_COMPLETE:
            g_log.loadok &= (~SE_BOOT_COMPLETE);
            break;
        case MSFS_EVENT_DISK_FORMAT:
            break;
        case MSFS_EVENT_DISK_DEL:
            if (disk->enType == DISK_TYPE_LOCAL ||
                disk->enType == DISK_TYPE_RAID ||
                disk->enType == DISK_TYPE_NAS ||
                disk->enType == DISK_TYPE_CIFS ||
                disk->enType == DISK_TYPE_NAND ||
                (disk->enType == DISK_TYPE_ESATA && disk->bRec == MF_YES)) {
                log_disk_uninit(disk->port);
            }
            break;
        case MSFS_EVENT_DISK_ADD:
            if (disk->enType == DISK_TYPE_LOCAL ||
                disk->enType == DISK_TYPE_RAID ||
                disk->enType == DISK_TYPE_NAS ||
                disk->enType == DISK_TYPE_CIFS ||
                disk->enType == DISK_TYPE_NAND ||
                (disk->enType == DISK_TYPE_ESATA && disk->bRec == MF_YES)) {
                if (disk->enState == DISK_STATE_NORMAL && disk->ops->disk_log_init) {
                    log_disk_init(disk);
                }
            }
            break;
        case MSFS_EVENT_DISK_ERR: {
            log_disk_uninit(disk->port);
        }
        break;
        default:
            break;
    }
    log_user_notify(event, disk);

    MFinfo("from = %d event = %d end\n", from, event);
}

MF_S32 mf_log_init(log_search_mac_type *lmac, LOG_USER_CB user_cb)
{
    memset(&g_log, 0, sizeof(struct log_msg));

    log_mem_init(&g_log);
    g_log.user_cb          = user_cb;
    g_log.loadok           = 0xFF;
    g_log.stat.search_free = MF_FALSE;
    ms_mutex_init(&g_log.loMutex);
    //g_log.stat.flag_lang = 2;// to sync the language type of client

    if (lmac) {
        memcpy(&g_log.lmac, lmac, sizeof(log_search_mac_type));
    }

    gLogs.diskStat = 0x0;
    gLogs.segNum   = 0x0;
    ms_mutex_init(&gLogs.headMutex);
    ms_mutex_init(&gLogs.segMutex);
    INIT_LIST_HEAD(&gLogs.headList);
    INIT_LIST_HEAD(&gLogs.segList);

    mf_comm_event_register(MODULE_LOG, mf_log_event_cb);
    mf_disk_nand_add("nand3", DISK_NAND_START_PORT, NULL);
    mf_log_write_start();
    mf_disk_ready_notify();

    MFinfo("mf_log_init end");
    return 0;
}

MF_S32 mf_log_deinit()
{
    printf("Export complete!!!\n");

    g_log.stat.search_free = MF_TRUE;

    mf_log_write_end();
    mf_disk_nand_del(DISK_NAND_START_PORT);
    log_head_node_free(&gLogs.headList);
    ms_mutex_uninit(&gLogs.headMutex);
    ms_mutex_uninit(&gLogs.segMutex);

    ms_mutex_uninit(&g_log.loMutex);
    ms_mutex_uninit(&g_log.cache.mutex);
    log_mem_uninit(&g_log);

    mf_comm_event_unregister(MODULE_LOG);
    printf("mf_log_deinit.....\n");
    return 0;
}

