/*
 * ***************************************************************
 * Filename:        msfs_backup.c
 * Created at:      2017.10.10
 * Description:     msfs backup API
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */
#include "mf_type.h"
#include "MFrecord.h"
#include "MFretrieve.h"
#include "MFdisk.h"
#include "msfs_bkp.h"

MF_S32
msfs_bkp_record_range(MF_U64 ipcMask, FILE_TYPE_EN enType, MF_TIME *time)
{
    MF_S32 i;
    MF_U64 n = 1;

    MFinfo("ipcMask[%#llx] enType[%d]", ipcMask, enType);
    time->startTime = INIT_START_TIME;
    time->endTime = INIT_END_TIME;

    for (i = 0; i < 64; i++) {
        if (ipcMask & (n << i)) {
            mf_retr_ipc_record_range(i, enType, time);
        }
    }
    MFdbg("startTime: %s", mf_time_to_string(time->startTime));
    MFdbg("endTime: %s", mf_time_to_string(time->endTime));

    return MF_SUCCESS;
}

struct bkp_res_t *
msfs_bkp_common_begin(struct bkp_comm_t *pstComm, MF_BOOL progress)
{
    if (!pstComm) {
        MFerr("pstComm is NULL");
        return NULL;
    }
    struct bkp_res_t *pstRes = ms_calloc(1, sizeof(struct bkp_res_t));
    MF_U64 n = 1;
    MF_S32 i;
    MF_U32  ipcNum = pstComm->ipcNum;
    MF_U32  count = 0;
    MF_S32 percent;
    MF_U8 searchid = pstComm->searchid;
    MF_U8 searchfrom = pstComm->searchfrom;

    MFinfo("ipcMask[%#llx] ipcNum[%d] enEvent[%#x] enType[%d] searchfrom[%d] searchid[%d]",
           pstComm->ipcMask, pstComm->ipcNum, pstComm->enEvent, pstComm->enType, pstComm->searchfrom, pstComm->searchid);
    MFinfo("startTime[%s]", mf_time_to_string(pstComm->time.startTime));
    MFinfo("endTime[%s]", mf_time_to_string(pstComm->time.endTime));

    INIT_LIST_HEAD(&pstRes->list);
    INIT_MF_TIME(&pstRes->range);
    pstRes->count = 0;
    pstRes->size = 0;

    for (i = 0; i < 64; i++) {
        if (pstComm->bCancel == MF_YES) {
            break;
        }
        if (pstComm->ipcMask & (n << i)) {
            struct bkp_node_t *pstNode = NULL;

            pstNode = ms_calloc(1, sizeof(struct bkp_node_t));
            INIT_LIST_HEAD(&pstNode->list);
            INIT_MF_TIME(&pstNode->range);
            pstNode->count = 0;
            pstNode->chnId = i;
            if (ipcNum && progress) {
                percent = mf_random(100 * count / ipcNum, 100 * (count + 1) / ipcNum);
                mf_retr_notify_bar_state(PROGRESS_BAR_RETRIEVE, percent, searchfrom, searchid);
            }
            mf_retr_ipc_comm_begin(i,
                                   pstComm->enType,
                                   pstComm->enEvent,
                                   pstComm->enState,
                                   &pstComm->time,
                                   pstNode,
                                   pstRes->count);
            if (pstNode->count) {
                pstRes->count += pstNode->count;
                pstRes->size  += pstNode->size;
                pstRes->range.startTime = MF_MIN(pstRes->range.startTime, pstNode->range.startTime);
                pstRes->range.endTime   = MF_MAX(pstRes->range.endTime, pstNode->range.endTime);
                LIST_INSERT_SORT_ASC(pstNode, &pstRes->list, chnId);
                if (pstRes->count >= BKP_MAX_RESULT_NUM) {
                    break;
                }
            } else {
                ms_free(pstNode);
            }
            count++;
            if (count == ipcNum && progress) {
                mf_retr_notify_bar_state(PROGRESS_BAR_RETRIEVE, 100, searchfrom, searchid);
            }
        }
    }

    if (mf_retr_is_dbg()) {
        msfs_bkp_common_dump(pstRes);
    }

    return pstRes;
}

MF_S32
msfs_bkp_common_end(struct bkp_res_t *pstRes)
{
    if (!pstRes) {
        MFerr("pstRes is NULL");
        return MF_FAILURE;
    }
    MFinfo("msfs_bkp_common_end");

    struct bkp_node_t *pstNode = NULL;
    struct bkp_node_t *n = NULL;

    list_for_each_entry_safe(pstNode, n, &pstRes->list, node) {
        mf_retr_ipc_comm_end(pstNode);
        list_del(&pstNode->node);
        ms_free(pstNode);
    }

    ms_free(pstRes);

    return MF_SUCCESS;
}

struct bkp_res_t *
msfs_bkp_event_begin(struct bkp_event_t *pstEvt, MF_BOOL progress)
{
    if (!pstEvt) {
        MFerr("pstEvt is NULL");
        return NULL;
    }

    struct bkp_res_t *pstRes = ms_calloc(1, sizeof(struct bkp_res_t));
    struct src_node_t *srcNode;
    MF_U64 n = 1;
    MF_S32 i;
    MF_U32  ipcNum = pstEvt->ipcNum ? pstEvt->ipcNum : MAX_REC_CHN_NUM;
    MF_U32  count = 0;
    MF_S32 percent;
    MF_U8 searchid = pstEvt->searchid;
    MF_U8 searchfrom = pstEvt->searchfrom;
    MF_S32 minP = 0;
    MF_S32 maxP = 0;

    MFinfo("ipcMask[%#llx] ipcNum[%d] enMajor[%#x] enMinor[%#x] enType[%d] searchfrom[%d] searchid[%d]",
           pstEvt->ipcMask, pstEvt->ipcNum, pstEvt->enMajor, pstEvt->enMinor, pstEvt->enType, pstEvt->searchfrom,
           pstEvt->searchid);
    MFinfo("startTime[%s]", mf_time_to_string(pstEvt->time.startTime));
    MFinfo("endTime[%s]", mf_time_to_string(pstEvt->time.endTime));
    MFinfo("ahead[%d] delay[%d]", pstEvt->ahead, pstEvt->delay);
    list_for_each_entry(srcNode, &pstEvt->srcList, node)
    MFinfo("srcName[%s]", srcNode->srcName);

    INIT_LIST_HEAD(&pstRes->list);
    INIT_MF_TIME(&pstRes->range);
    pstRes->count = 0;
    pstRes->size = 0;

    for (i = 0; i < 64; i++) {
        if (pstEvt->bCancel == MF_YES) {
            break;
        }
        if (pstEvt->ipcMask & (n << i) || pstEvt->srcCount) {
            struct bkp_node_t *pstNode = NULL;

            pstNode = ms_calloc(1, sizeof(struct bkp_node_t));
            INIT_LIST_HEAD(&pstNode->list);
            INIT_MF_TIME(&pstNode->range);
            pstNode->count = 0;
            pstNode->chnId = i;
            if (ipcNum && progress) {
                minP = 100 * count / ipcNum;
                maxP = 99 * (count + 1) / ipcNum;
                percent = mf_random(minP, maxP);
                mf_retr_notify_bar_state(PROGRESS_BAR_RETRIEVE, percent, searchfrom, searchid);
            }

            mf_retr_ipc_event_begin(i, pstEvt, pstNode, pstRes->count, percent, maxP - percent, progress);
            if (pstNode->count) {
                pstRes->count += pstNode->count;
                pstRes->size  += pstNode->size;
                pstRes->range.startTime = MF_MIN(pstRes->range.startTime, pstNode->range.startTime);
                pstRes->range.endTime   = MF_MAX(pstRes->range.endTime, pstNode->range.endTime);
                LIST_INSERT_SORT_ASC(pstNode, &pstRes->list, chnId);
                if (pstRes->count >= pstEvt->maxNum) {
                    break;
                }
            } else {
                ms_free(pstNode);
            }
            count++;
            if (count == ipcNum && progress) {
                mf_retr_notify_bar_state(PROGRESS_BAR_RETRIEVE, 100, searchfrom, searchid);
            }
        }
    }

    if (mf_retr_is_dbg()) {
        msfs_bkp_event_dump(pstRes);
    }

    return pstRes;
}

MF_S32
msfs_bkp_event_end(struct bkp_res_t *pstRes)
{
    if (!pstRes) {
        MFerr("pstRes is NULL");
        return MF_FAILURE;
    }
    MFinfo("msfs_bkp_event_end");

    struct bkp_node_t *pstNode = NULL;
    struct bkp_node_t *n = NULL;

    list_for_each_entry_safe(pstNode, n, &pstRes->list, node) {
        mf_retr_ipc_event_end(pstNode);
        list_del(&pstNode->node);
        ms_free(pstNode);
    }

    ms_free(pstRes);

    return MF_SUCCESS;
}

struct bkp_res_t *
msfs_bkp_pic_begin(struct bkp_pic_t *pstPic, MF_BOOL progress)
{
    if (!pstPic) {
        MFerr("pstPic is NULL");
        return NULL;
    }

    struct bkp_res_t *pstRes = ms_calloc(1, sizeof(struct bkp_res_t));
    MF_U64 n = 1;
    MF_S32 i;
    MF_U32  ipcNum = pstPic->ipcNum;
    MF_U32  count = 0;
    MF_S32 percent;
    MF_U8 searchid = pstPic->searchid;
    MF_U8 searchfrom = pstPic->searchfrom;

    MFinfo("ipcMask[%#llx] ipcNum[%d] enMajor[%#x] searchfrom[%d] searchid[%d]",
           pstPic->ipcMask, pstPic->ipcNum, pstPic->enMajor, pstPic->searchfrom, pstPic->searchid);
    MFinfo("startTime[%s]", mf_time_to_string(pstPic->time.startTime));
    MFinfo("endTime[%s]", mf_time_to_string(pstPic->time.endTime));

    INIT_LIST_HEAD(&pstRes->list);
    INIT_MF_TIME(&pstRes->range);
    pstRes->count = 0;
    pstRes->size = 0;

    for (i = 0; i < 64; i++) {
        if (pstPic->bCancel == MF_YES) {
            break;
        }
        if (pstPic->ipcMask & (n << i)) {
            struct bkp_node_t *pstNode = NULL;

            pstNode = ms_calloc(1, sizeof(struct bkp_node_t));
            INIT_LIST_HEAD(&pstNode->list);
            INIT_MF_TIME(&pstNode->range);
            pstNode->chnId = i;
            if (ipcNum && progress) {
                percent = mf_random(100 * count / ipcNum, 100 * (count + 1) / ipcNum);
                mf_retr_notify_bar_state(PROGRESS_BAR_RETRIEVE, percent, searchfrom, searchid);
            }
            mf_retr_ipc_pic_begin(i,
                                  pstPic->enMajor,
                                  pstPic->enMinor,
                                  &pstPic->time,
                                  pstNode,
                                  pstRes->count,
                                  pstPic->pri,
                                  &pstPic->vca,
                                  pstPic->maxNum);
            if (pstNode->count) {
                pstRes->count += pstNode->count;
                pstRes->size  += pstNode->size;
                pstRes->range.startTime = MF_MIN(pstRes->range.startTime, pstNode->range.startTime);
                pstRes->range.endTime   = MF_MAX(pstRes->range.endTime, pstNode->range.endTime);
                LIST_INSERT_SORT_ASC(pstNode, &pstRes->list, chnId);
                if (pstRes->count >= pstPic->maxNum) {
                    break;
                }
            } else {
                ms_free(pstNode);
            }
            count++;
            if (count == ipcNum && progress) {
                mf_retr_notify_bar_state(PROGRESS_BAR_RETRIEVE, 100, searchfrom, searchid);
            }
        }
    }

    if (mf_retr_is_dbg()) {
        msfs_bkp_pic_dump(pstRes);
    }

    return pstRes;
}

MF_S32
msfs_bkp_pic_end(struct bkp_res_t *pstRes)
{
    if (!pstRes) {
        MFerr("pstRes is NULL");
        return MF_FAILURE;
    }
    MFinfo("msfs_bkp_pic_end");

    struct bkp_node_t *pstNode = NULL;
    struct bkp_node_t *n = NULL;

    list_for_each_entry_safe(pstNode, n, &pstRes->list, node) {
        mf_retr_ipc_pic_end(pstNode);
        list_del(&pstNode->node);
        ms_free(pstNode);
    }

    ms_free(pstRes);

    return MF_SUCCESS;
}

struct bkp_res_t *
msfs_bkp_lpr_begin(struct bkp_lpr_t *pstLpr, MF_BOOL progress)
{
    if (!pstLpr) {
        MFerr("pstLpr is NULL");
        return NULL;
    }

    struct bkp_res_t *pstRes = ms_calloc(1, sizeof(struct bkp_res_t));
    MF_U64 n = 1;
    MF_S32 i;
    MF_U32  ipcNum = pstLpr->ipcNum;
    MF_U32  count = 0;
    MF_S32 percent;
    MF_U8 searchid = pstLpr->searchid;
    MF_U8 searchfrom = pstLpr->searchfrom;

    MFinfo("ipcMask[%#llx] ipcNum[%d] searchfrom[%d] searchid[%d]",
           pstLpr->ipcMask, pstLpr->ipcNum, pstLpr->searchfrom, pstLpr->searchid);
    MFinfo("startTime[%s]", mf_time_to_string(pstLpr->time.startTime));
    MFinfo("endTime[%s]", mf_time_to_string(pstLpr->time.endTime));
    MFinfo("keyWord[%s]", pstLpr->keyWord ? pstLpr->keyWord : "NULL");

    INIT_LIST_HEAD(&pstRes->list);
    INIT_MF_TIME(&pstRes->range);
    pstRes->count = 0;
    pstRes->size = 0;

    for (i = 0; i < 64; i++) {
        if (pstLpr->bCancel == MF_YES) {
            break;
        }
        if (pstLpr->ipcMask & (n << i)) {
            struct bkp_node_t *pstNode = NULL;
            pstNode = ms_calloc(1, sizeof(struct bkp_node_t));
            INIT_LIST_HEAD(&pstNode->list);
            pstNode->chnId = i;
            if (ipcNum && progress) {
                percent = mf_random(100 * count / ipcNum, 100 * (count + 1) / ipcNum);
                mf_retr_notify_bar_state(PROGRESS_BAR_RETRIEVE, percent, searchfrom, searchid);
            }
            mf_retr_ipc_lpr_begin(i, &pstLpr->time,
                                  pstLpr->keyWord,
                                  pstLpr->match,
                                  &pstLpr->pri,
                                  pstNode,
                                  pstRes->count);
            if (pstNode->count) {
                pstRes->count += pstNode->count;
                LIST_INSERT_SORT_ASC(pstNode, &pstRes->list, chnId);
                if (pstRes->count >= BKP_MAX_RESULT_NUM) {
                    break;
                }
            } else {
                ms_free(pstNode);
            }
            count++;
            if (count == ipcNum && progress) {
                mf_retr_notify_bar_state(PROGRESS_BAR_RETRIEVE, 100, searchfrom, searchid);
            }
        }
    }

    if (mf_retr_is_dbg()) {
        msfs_bkp_lpr_dump(pstRes);
    }

    return pstRes;
}

MF_S32
msfs_bkp_lpr_end(struct bkp_res_t *pstRes)
{
    if (!pstRes) {
        MFerr("pstRes is NULL");
        return MF_FAILURE;
    }
    MFinfo("msfs_bkp_lpr_end");

    struct bkp_node_t *pstNode = NULL;
    struct bkp_node_t *n = NULL;

    list_for_each_entry_safe(pstNode, n, &pstRes->list, node) {
        mf_retr_ipc_lpr_end(pstNode);
        list_del(&pstNode->node);
        ms_free(pstNode);
    }

    ms_free(pstRes);

    return MF_SUCCESS;
}

struct bkp_res_t *
msfs_bkp_face_begin(struct bkp_face_t *pstFace, MF_BOOL progress)
{
    if (!pstFace) {
        MFerr("pstFace is NULL");
        return NULL;
    }

    struct bkp_res_t *pstRes = ms_calloc(1, sizeof(struct bkp_res_t));
    MF_U64 n = 1;
    MF_S32 i;
    MF_U32  ipcNum = pstFace->ipcNum;
    MF_U32  count = 0;
    MF_S32 percent;
    MF_U8 searchid = pstFace->searchid;
    MF_U8 searchfrom = pstFace->searchfrom;

    MFinfo("ipcMask[%#llx] ipcNum[%d] searchfrom[%d] searchid[%d]",
           pstFace->ipcMask, pstFace->ipcNum, pstFace->searchfrom, pstFace->searchid);
    MFinfo("startTime[%s]", mf_time_to_string(pstFace->time.startTime));
    MFinfo("endTime[%s]", mf_time_to_string(pstFace->time.endTime));

    INIT_LIST_HEAD(&pstRes->list);
    INIT_MF_TIME(&pstRes->range);
    pstRes->count = 0;
    pstRes->size = 0;

    for (i = 0; i < 64; i++) {
        if (pstFace->bCancel == MF_YES) {
            break;
        }
        if (pstFace->ipcMask & (n << i)) {
            struct bkp_node_t *pstNode = NULL;
            pstNode = ms_calloc(1, sizeof(struct bkp_node_t));
            INIT_LIST_HEAD(&pstNode->list);
            pstNode->chnId = i;
            if (ipcNum && progress) {
                percent = mf_random(100 * count / ipcNum, 100 * (count + 1) / ipcNum);
                mf_retr_notify_bar_state(PROGRESS_BAR_RETRIEVE, percent, searchfrom, searchid);
            }
            mf_retr_ipc_face_begin(i, &pstFace->time,
                                  pstFace->match,
                                  &pstFace->pri,
                                  pstNode,
                                  pstRes->count);
            if (pstNode->count) {
                pstRes->count += pstNode->count;
                LIST_INSERT_SORT_ASC(pstNode, &pstRes->list, chnId);
                if (pstRes->count >= BKP_MAX_RESULT_NUM) {
                    break;
                }
            } else {
                ms_free(pstNode);
            }
            count++;
            if (count == ipcNum && progress) {
                mf_retr_notify_bar_state(PROGRESS_BAR_RETRIEVE, 100, searchfrom, searchid);
            }
        }
    }

    if (mf_retr_is_dbg()) {
        msfs_bkp_face_dump(pstRes);
    }

    return pstRes;
}

MF_S32
msfs_bkp_face_end(struct bkp_res_t *pstRes)
{
    if (!pstRes) {
        MFerr("pstRes is NULL");
        return MF_FAILURE;
    }
    MFinfo("msfs_bkp_face_end");

    struct bkp_node_t *pstNode = NULL;
    struct bkp_node_t *n = NULL;

    list_for_each_entry_safe(pstNode, n, &pstRes->list, node) {
        mf_retr_ipc_face_end(pstNode);
        list_del(&pstNode->node);
        ms_free(pstNode);
    }

    ms_free(pstRes);

    return MF_SUCCESS;
}


struct bkp_res_t *
msfs_bkp_smart_begin(struct bkp_smart_t *pstSmart, MF_BOOL progress)
{
    if (!pstSmart) {
        MFerr("pstSmart is NULL");
        return NULL;
    }

    struct bkp_res_t *pstRes = ms_calloc(1, sizeof(struct bkp_res_t));
    MF_U64 n = 1;
    MF_S32 i;
    MF_U32 ipcNum = pstSmart->ipcNum;
    MF_U32 count = 0;
    MF_S32 percent;
    MF_U8 searchid = pstSmart->searchid;
    MF_U8 searchfrom = pstSmart->searchfrom;
    void *infoBuff = NULL;
    void *dataBuff = NULL;
    
    MFinfo("ipcMask[%#llx] ipcNum[%d] searchfrom[%d] searchid[%d]",
           pstSmart->ipcMask, pstSmart->ipcNum, pstSmart->searchfrom, pstSmart->searchid);
    MFinfo("startTime[%s]", mf_time_to_string(pstSmart->time.startTime));
    MFinfo("endTime[%s]", mf_time_to_string(pstSmart->time.endTime));

    INIT_LIST_HEAD(&pstRes->list);
    INIT_MF_TIME(&pstRes->range);
    pstRes->count = 0;
    pstRes->size = 0;

    for (i = 0; i < 64; i++) {
        if (pstSmart->bCancel == MF_YES) {
            break;
        }

        if (pstSmart->ipcMask & (n << i)) {
            struct bkp_node_t *pstNode = NULL;
            pstNode = ms_calloc(1, sizeof(struct bkp_node_t));
            INIT_LIST_HEAD(&pstNode->list);
            pstNode->chnId = i;
            if (ipcNum && progress) {
                percent = mf_random(100 * count / ipcNum, 100 * (count + 1) / ipcNum);
                mf_retr_notify_bar_state(PROGRESS_BAR_RETRIEVE, percent, searchfrom, searchid);
            }

            if (!infoBuff) {
                infoBuff = ms_valloc(MAX_INFO_SIZE);
                if (!infoBuff) {
                    MFerr("ms_valloc failed");
                    ms_free(pstRes);
                    return NULL;
                }
            }
            
            if (!dataBuff) {
                dataBuff = ms_valloc(MAX_DATA_SIZE);
                if (!dataBuff) {
                    MFerr("ms_valloc failed");
                    ms_free(infoBuff);
                    ms_free(pstRes);
                    return NULL;
                }
            }
            
            mf_retr_ipc_smart_begin(i, pstSmart, pstNode, pstRes->count, infoBuff, dataBuff);
            if (pstNode->count) {
                pstRes->count += pstNode->count;
                LIST_INSERT_SORT_ASC(pstNode, &pstRes->list, chnId);
                if (pstRes->count >= BKP_MAX_RESULT_NUM) {
                    break;
                }
            } else {
                ms_free(pstNode);
            }
            count++;
            if (count == ipcNum && progress) {
                mf_retr_notify_bar_state(PROGRESS_BAR_RETRIEVE, 100, searchfrom, searchid);
            }
        }
    }

    if (mf_retr_is_dbg()) {
        msfs_bkp_smt_dump(pstRes); 
    }

    if (infoBuff) {
        ms_free(infoBuff);
    }
    
    if (dataBuff) {
        ms_free(dataBuff);
    }
    
    return pstRes;
}

MF_S32
msfs_bkp_smart_end(struct bkp_res_t *pstRes)
{
    if (!pstRes) {
        MFerr("pstRes is NULL");
        return MF_FAILURE;
    }
    MFinfo("msfs_bkp_smart_end");

    struct bkp_node_t *pstNode = NULL;
    struct bkp_node_t *n = NULL;

    list_for_each_entry_safe(pstNode, n, &pstRes->list, node) {
        mf_retr_ipc_smart_end(pstNode);
        list_del(&pstNode->node);
        ms_free(pstNode);
    }

    ms_free(pstRes);

    return MF_SUCCESS;
}


struct bkp_res_t *
msfs_bkp_tag_begin(struct bkp_tag_t *pstTag, MF_BOOL progress)
{
    if (!pstTag) {
        MFerr("pstTag is NULL");
        return NULL;
    }

    struct bkp_res_t *pstRes = ms_calloc(1, sizeof(struct bkp_res_t));
    MF_S8 *buff = ms_valloc(TAG_MAX_NUM * TAG_HEAD_SIZE);
    MF_U64 n = 1;
    MF_S32 i;
    MF_U32  ipcNum = pstTag->ipcNum;
    MF_U32  count = 0;
    MF_S32 percent;
    MF_U8 searchid = pstTag->searchid;
    MF_U8 searchfrom = pstTag->searchfrom;

    MFinfo("ipcMask[%#llx] enType[%d] searchfrom[%d] searchid[%d]",
           pstTag->ipcMask, pstTag->enType, pstTag->searchfrom, pstTag->searchid);
    MFinfo("startTime[%s]", mf_time_to_string(pstTag->time.startTime));
    MFinfo("endTime[%s]", mf_time_to_string(pstTag->time.endTime));
    MFinfo("keyWord[%s]", pstTag->keyWord ? pstTag->keyWord : "NULL");

    INIT_LIST_HEAD(&pstRes->list);
    INIT_MF_TIME(&pstRes->range);

    for (i = 0; i < 64; i++) {
        if (pstTag->bCancel == MF_YES) {
            break;
        }
        if (pstTag->ipcMask & (n << i)) {
            struct bkp_node_t *pstNode = NULL;

            pstNode = ms_calloc(1, sizeof(struct bkp_node_t));
            INIT_LIST_HEAD(&pstNode->list);
            pstNode->chnId = i;
            if (ipcNum && progress) {
                percent = mf_random(100 * count / ipcNum, 100 * (count + 1) / ipcNum);
                mf_retr_notify_bar_state(PROGRESS_BAR_RETRIEVE, percent, searchfrom, searchid);
            }
            mf_retr_ipc_tag_begin(i,
                                  pstTag->enType,
                                  &pstTag->time,
                                  pstTag->keyWord,
                                  buff,
                                  pstNode);
            if (pstNode->count) {
                pstRes->count += pstNode->count;
                LIST_INSERT_SORT_ASC(pstNode, &pstRes->list, chnId);
            } else {
                ms_free(pstNode);
            }
            count++;
            if (count == ipcNum && progress) {
                mf_retr_notify_bar_state(PROGRESS_BAR_RETRIEVE, 100, searchfrom, searchid);
            }
        }
    }

    if (mf_retr_is_dbg()) {
        msfs_bkp_tag_dump(pstRes);
    }

    ms_free(buff);

    return pstRes;
}

MF_S32
msfs_bkp_tag_end(struct bkp_res_t *pstRes)
{
    if (!pstRes) {
        MFerr("pstRes is NULL");
        return MF_FAILURE;
    }
    MFinfo("msfs_bkp_tag_end");

    struct bkp_node_t *pstNode = NULL;
    struct bkp_node_t *n = NULL;

    list_for_each_entry_safe(pstNode, n, &pstRes->list, node) {
        mf_retr_ipc_tag_end(pstNode);
        list_del(&pstNode->node);
        ms_free(pstNode);
    }

    ms_free(pstRes);

    return MF_SUCCESS;
}

void
msfs_bkp_event_dump(struct bkp_res_t *pstRes)
{
    struct bkp_node_t *pstNode;
    struct item_evt_t *item;
    struct stream_seg *stream;
    MF_S8 startTime [32];
    MF_S8 endTime [32];
    MF_S32 i;

    MFinfo("msfs_bkp_event_dump -->count[%d]", pstRes->count);
    MFinfo("msfs_bkp_event_dump -->size[%lld]", pstRes->size);
    list_for_each_entry(pstNode, &pstRes->list, node) {
        MFinfo("\t chnId[%d] count[%d]", pstNode->chnId, pstNode->count);
        i = 0;
        list_for_each_entry(item, &pstNode->list, node) {
            MFinfo("\t\t [%d]", i++);
            MFinfo("\t\t item source %s", item->srcName);
            MFinfo("\t\t item startTime[%s]", mf_time_to_string(item->time.startTime));
            MFinfo("\t\t item endtime[%s]", mf_time_to_string(item->time.endTime));
            list_for_each_entry(stream, &item->streamList, node) {
                time_to_string_local(stream->startTime, startTime);
                time_to_string_local(stream->endTime, endTime);
                MFinfo("\t\t\t stream disk[%s] fileNoe[%d]",
                       stream->pstDisk->name,
                       stream->fileNo);
                MFinfo("\t\t\t stream time[%s - %s] size[%dK]",
                       startTime,
                       endTime,
                       stream->size / 1024);

            }
        }
    }
}

void
msfs_bkp_common_dump(struct bkp_res_t *pstRes)
{
    struct bkp_node_t *pstNode;
    struct item_comm_t *item;
    struct stream_seg *stream;
    MF_S8 startTime [32];
    MF_S8 endTime [32];
    MF_S32 i;

    MFinfo("msfs_bkp_common_dump -->count[%d]", pstRes->count);
    MFinfo("msfs_bkp_common_dump -->size[%lld]", pstRes->size);
    list_for_each_entry(pstNode, &pstRes->list, node) {
        MFinfo("\t chnId[%d] count[%d]", pstNode->chnId, pstNode->count);
        MFinfo("\t from[%s]", mf_time_to_string(pstRes->range.startTime));
        MFinfo("\t to  [%s]", mf_time_to_string(pstRes->range.endTime));
        i = 0;
        list_for_each_entry(item, &pstNode->list, node) {
            MFinfo("\t\t [%d]", i++);
            MFinfo("\t\t item startTime[%s]", mf_time_to_string(item->time.startTime));
            MFinfo("\t\t item endtime[%s]", mf_time_to_string(item->time.endTime));
            list_for_each_entry(stream, &item->streamList, node) {
                time_to_string_local(stream->startTime, startTime);
                time_to_string_local(stream->endTime, endTime);
                MFinfo("\t\t\t stream disk[%s] fileNoe[%d]",
                       stream->pstDisk->name,
                       stream->fileNo);
                MFinfo("\t\t\t stream time[%s - %s] size[%dK]",
                       startTime,
                       endTime,
                       stream->size / 1024);
            }
        }
    }
}

void
msfs_bkp_pic_dump(struct bkp_res_t *pstRes)
{
    struct bkp_node_t *pstNode;
    struct item_pic_t *item;
    MF_S32 i;

    MFinfo("msfs_bkp_pic_dump -->count[%d]", pstRes->count);
    MFinfo("msfs_bkp_pic_dump -->size[%lld]", pstRes->size);
    list_for_each_entry(pstNode, &pstRes->list, node) {
        MFinfo("\t chnId[%d] count[%d]", pstNode->chnId, pstNode->count);
        MFinfo("\t from[%s]", mf_time_to_string(pstRes->range.startTime));
        MFinfo("\t to  [%s]", mf_time_to_string(pstRes->range.endTime));
        i = 0;
        list_for_each_entry(item, &pstNode->list, node) {
            MFinfo("\t\t [%d]", i++);
            MFinfo("\t\t item time[%s]", mf_time_to_string(item->pts / 1000000));
            MFinfo("\t\t item type[%#x]", item->enMajor);
            MFinfo("\t\t item size[%lluK]", item->size / 1024);
            MFinfo("\t\t item port[%d]", item->port);
            MFinfo("\t\t item WH[%dx%d]", item->width, item->height);
        }
    }
}

void
msfs_bkp_lpr_dump(struct bkp_res_t *pstRes)
{
    struct bkp_node_t *pstNode;
    struct item_lpr_t *item;
    MF_S32 i;

    MFinfo("msfs_bkp_lpr_dump -->count[%d]", pstRes->count);
    list_for_each_entry(pstNode, &pstRes->list, node) {
        MFinfo("\t chnId[%d] count[%d]", pstNode->chnId, pstNode->count);
        MFinfo("\t from[%s]", mf_time_to_string(pstRes->range.startTime));
        MFinfo("\t to  [%s]", mf_time_to_string(pstRes->range.endTime));
        i = 0;
        list_for_each_entry(item, &pstNode->list, node) {
            MFinfo("\t\t [%d]", i++);
            MFinfo("\t\t item time[%s]", mf_time_to_string(item->pts / 1000000));
            MFinfo("\t\t item licence[%s]", item->licence);
            MFinfo("\t\t item port[%d]", item->port);
            MFinfo("\t\t item WH[%dx%d]", item->picMainWidth, item->picMainHeight);
            MFinfo("\t\t item WH[%dx%d]", item->picSubWidth, item->picSubHeight);
        }
    }
}

void
msfs_bkp_face_dump(struct bkp_res_t *pstRes)
{
    struct bkp_node_t *pstNode;
    struct item_face_t *item;
    MF_S32 i;

    MFinfo("msfs_bkp_lpr_dump -->count[%d]", pstRes->count);
    list_for_each_entry(pstNode, &pstRes->list, node) {
        MFinfo("\t chnId[%d] count[%d]", pstNode->chnId, pstNode->count);
        MFinfo("\t from[%s]", mf_time_to_string(pstRes->range.startTime));
        MFinfo("\t to  [%s]", mf_time_to_string(pstRes->range.endTime));
        i = 0;
        list_for_each_entry(item, &pstNode->list, node) {
            MFinfo("\t\t [%d]", i++);
            MFinfo("\t\t item time[%s]", mf_time_to_string(item->pts / 1000000));
            MFinfo("\t\t item port[%d]", item->port);
            MFinfo("\t\t item WH[%dx%d]", item->picMainWidth, item->picMainHeight);
            MFinfo("\t\t item WH[%dx%d]", item->picSubWidth, item->picSubHeight);
        }
    }
}

void
msfs_bkp_smt_dump(struct bkp_res_t *pstRes)
{
    struct bkp_node_t *pstNode;
    struct item_smt_t *item;
    MF_S32 i;

    MFinfo("msfs_bkp_smt_dump -->count[%d]", pstRes->count);
    list_for_each_entry(pstNode, &pstRes->list, node) {
        MFinfo("\t chnId[%d] count[%d]", pstNode->chnId, pstNode->count);
        MFinfo("\t from[%s]", mf_time_to_string(pstRes->range.startTime));
        MFinfo("\t to  [%s]", mf_time_to_string(pstRes->range.endTime));
        i = 0;
        list_for_each_entry(item, &pstNode->list, node) {
            MFinfo("\t\t [%d]", i++);
            MFinfo("\t\t item time[%s]", mf_time_to_string(item->pts / 1000000));
            MFinfo("\t\t item port[%d]", item->port);
            MFinfo("\t\t item minor[%d]", item->enMinor);
        }
    }
}

void
msfs_bkp_tag_dump(struct bkp_res_t *pstRes)
{
    struct bkp_node_t *pstNode;
    struct item_tag_t *item;
    MF_S32 i;

    MFinfo("msfs_bkp_tag_dump -->count[%d]", pstRes->count);
    list_for_each_entry(pstNode, &pstRes->list, node) {
        MFinfo("\t chnId[%d] count[%d]", pstNode->chnId, pstNode->count);
        MFinfo("\t from[%s]", mf_time_to_string(pstRes->range.startTime));
        MFinfo("\t to  [%s]", mf_time_to_string(pstRes->range.endTime));
        i = 0;
        list_for_each_entry(item, &pstNode->list, node) {
            MFinfo("\t\t [%d]", i++);
            MFinfo("\t\t item time[%s]", mf_time_to_string(item->time));
            MFinfo("\t\t item name[%s]", item->name);
            MFinfo("\t\t item id[%d]", item->id);
        }
    }
}

MF_S32
msfs_bkp_common_lock(struct item_comm_t *item, MF_BOOL bLock)
{
    if (!item) {
        MFerr("item is NULL");
        return MF_FAILURE;
    }
    MFinfo("bLock[%d]", bLock);

    struct stream_seg *stream;

    list_for_each_entry(stream, &item->streamList, node) {
        if (mf_retr_stream_is_valid(stream) == MF_YES) {
            mf_retr_seg_lock(stream, bLock);
        }
    }

    return MF_SUCCESS;
}

MF_S32
msfs_bkp_event_lock(struct item_evt_t *item, MF_BOOL bLock)
{
    if (!item) {
        MFerr("item is NULL");
        return MF_FAILURE;
    }
    MFinfo("bLock[%d]", bLock);

    struct stream_seg *stream;

    list_for_each_entry(stream, &item->streamList, node) {
        if (mf_retr_stream_is_valid(stream) == MF_YES) {
            mf_retr_seg_lock(stream, bLock);
        }
    }

    return MF_SUCCESS;
}

MF_S32
msfs_bkp_tag_edit(struct item_tag_t *item, MF_S8 *reName)
{
    if (!item) {
        MFerr("item is NULL");
        return MF_FAILURE;
    }

    MFinfo("reName[%s]", reName ? reName : "NULL");

    return mf_retr_tag_edit(item, reName);
}

MF_S32
msfs_bkp_lpr_get_pic(struct item_lpr_t *item, MF_BOOL bMainPic, struct jpg_frame *f)
{
    return mf_retr_get_lpr_pic(item, bMainPic, f);
}

MF_S32
msfs_bkp_lpr_auto_get_pic(struct item_lpr_t *item, MF_BOOL bMainPic, struct jpg_frame *f, int fd)
{
    return mf_retr_get_lpr_auto_pic(item, bMainPic, f, fd);
}

void
msfs_bkp_lpr_put_pic(struct jpg_frame *f)
{
    if (f) {
        mf_retr_put_lpr_pic(f);
    }
}

MF_S32
msfs_bkp_face_get_pic(struct item_face_t *item, MF_BOOL bMainPic, struct jpg_frame *f)
{
    return mf_retr_get_face_pic(item, bMainPic, f);
}

void
msfs_bkp_face_put_pic(struct jpg_frame *f)
{
    if (f) {
        mf_retr_put_face_pic(f);
    }
}

MF_S32
msfs_bkp_get_Iframe(struct stream_seg *s, struct mf_frame *f)
{
    if (mf_retr_stream_is_valid(s) == MF_NO) {
        return MF_FAILURE;
    }

    return mf_retr_get_first_Iframe(s, f);;
}

void
msfs_bkp_put_Iframe(struct mf_frame *f)
{
    if (f) {
        mf_retr_put_Iframe(f);
    }
}

MF_S32
msfs_bkp_get_frame(MF_S8 *addr, MF_S8 *end, struct mf_frame *f, MF_FRAME_DATA_TYPE type)
{
    return mf_retr_get_a_frame(addr, end, f, type);
}

void
msfs_bkp_put_frame(struct mf_frame *f)
{
    if (f) {
        mf_retr_put_a_frame(f);
    }
}

MF_S32
msfs_bkp_get_pic(struct stream_seg *s, INFO_MAJOR_EN m, struct jpg_frame *f)
{
    if (mf_retr_stream_is_valid(s) == MF_NO) {
        return MF_FAILURE;
    }

    return mf_retr_get_a_pic(s, m, f);
}

MF_S32
msfs_bkp_get_wh_pic(struct stream_seg *s, MF_U32 w, MF_U32 h, struct jpg_frame *f)
{
    if (mf_retr_stream_is_valid(s) == MF_NO) {
        return MF_FAILURE;
    }

    return mf_retr_get_a_wh_pic(s, w, h, f);
}

void
msfs_bkp_put_pic(struct jpg_frame *f)
{
    if (f) {
        mf_retr_put_a_pic(f);
    }
}

MF_S32
msfs_bkp_export_open(struct stream_seg *s)
{
    struct file_info *file = s->pstSegInfo->owner;
    struct diskObj *pstDisk = file->owner->owner;
    struct key_record *keyRec = &file->key.stRecord;
    MF_S32 fd;

    fd = pstDisk->ops->disk_file_open(pstDisk->pstPrivate, keyRec->fileNo, MF_YES);

    return fd;
}

MF_S32
msfs_bkp_anpr_open(struct item_lpr_t *item)
{
    struct diskObj *pstDisk = item->pstDisk;
    MF_S32 fd;

    fd = pstDisk->ops->disk_file_open(pstDisk->pstPrivate, item->fileNo, MF_YES);
    return fd;
}

void
msfs_bkp_export_close(MF_S32 fd)
{
    disk_close(fd);
}

MF_S32
msfs_bkp_export_read(MF_S32 fd, void *buf, MF_S32 count, MF_U64 offset)
{
    MF_S32 res = disk_read(fd, buf,  count, offset);
    if (res < 0) {
        MFerr("read disk failed");
    }
    return res;
}

struct bkp_exp_t *
msfs_bkp_export_start(struct stream_seg *stream, MF_BOOL bFromIframe)
{
    struct bkp_exp_t *pstExp = ms_calloc(1, sizeof(struct bkp_exp_t));

    pstExp->handle = mf_retr_export_start(stream, bFromIframe);
    if (pstExp->handle == NULL) {
        ms_free(pstExp);
        return NULL;
    }

    return pstExp;
}

MF_S32
msfs_bkp_export_stop(struct bkp_exp_t *pstExp)
{
    mf_retr_export_stop(pstExp->handle);
    ms_free(pstExp);
    return MF_SUCCESS;
}

MF_S32
msfs_bkp_export_get_frame(struct bkp_exp_t *pstExp, struct mf_frame *frame, MF_FRAME_DATA_TYPE type, MF_BOOL *pbEOF)
{
    MF_S32 res = mf_retr_export_get_frame(pstExp->handle, frame, type);

    if (!pbEOF) {
        return res;
    }

    if (res == MF_ERR_RETR_DATA_EOF) {
        *pbEOF = MF_YES;
    } else {
        *pbEOF = MF_NO;
    }

    return res;
}

MF_S32
msfs_bkp_export_get_custom(MF_S8 *data, MF_S32 len, struct mf_frame *frame)
{
    return mf_retr_export_get_custom(data, len, frame);
}

MF_S32
msfs_bkp_export_put_custom(struct mf_frame *frame)
{
    return mf_retr_export_put_custom(frame);
}

MF_S32
msfs_bkp_export_put_frame(struct bkp_exp_t *pstExp, struct mf_frame *frame)
{
    return mf_retr_export_put_frame(pstExp->handle, frame);
}

MF_S32
msfs_bkp_dev_format(struct item_dev_t *item, FORMAT_EN enFormat)
{
    if (!item) {
        MFerr("item is NULL");
        return MF_FAILURE;
    }
    MFinfo("enFormat[%d]", enFormat);

    struct diskObj *disk = item->disk;
    MF_S32 res = MF_FAILURE;

    mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, 0, disk->enType);
    mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, mf_random(10, 15), disk->enType);
    if (disk->ops->disk_format) {
        /* quote should be 0 and do not use it except for nas. solin 20190402 */
        res = disk->ops->disk_format(disk->pstPrivate, &enFormat, 0);
    }
    mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, mf_random(80, 90), disk->enType);
    if (res == MF_SUCCESS) { // fresh disk info
        mf_disk_attribute(disk);
    }
    mf_disk_notify_bar(PROGRESS_BAR_BKP_FORMAT, 0, 100, disk->enType);

    return res;
}

MF_S32
msfs_bkp_get_dev_list(struct bkp_dev_t *pstDev)
{
    if (!pstDev) {
        MFerr("pstDev is NULL");
        return MF_FAILURE;
    }
    MFinfo("msfs_bkp_get_dev_list");

    struct list_head *diskList;
    struct diskObj *disk;

    INIT_LIST_HEAD(&pstDev->list);
    mf_bkp_get_format();
    diskList = mf_disk_get_rd_list(MF_NO);

    list_for_each_entry(disk, diskList, node) {
        if (disk->enType == DISK_TYPE_NAS ||
            disk->enType == DISK_TYPE_USB ||
            disk->enType == DISK_TYPE_ESATA) {
            if (disk->enState == DISK_STATE_OFFLINE
                || disk->enState == DISK_STATE_BAD
                || disk->enState == DISK_STATE_NONE) {
                continue;
            }
            struct item_dev_t *item;

            item = ms_calloc(1, sizeof(struct item_dev_t));
            item->port     = disk->port;
            item->vendor   = disk->vendor;
            item->enType   = disk->enType;
            item->capacity = disk->capacity;
            item->enState  = disk->enState;
            item->bRec     = disk->bRec;
            item->disk     = disk;

            if (disk->ops->disk_mount) {
                item->bformat  = disk->ops->disk_mount(disk->pstPrivate) == MF_SUCCESS ? MF_YES : MF_NO;

                if (item->bformat) {
                    disk->ops->disk_size(disk->pstPrivate);// update size
                    item->free = disk->free / 1024 / 1024; // size Mb
                    item->bprotect = mf_disk_Udisk_protected(disk);
                }
                MFinfo("disk:%s bformat:%d bprotect:%d", disk->vendor, item->bformat, item->bprotect);
            }

            item->mnt      = disk->mnt;
            list_add(&item->node, &pstDev->list);
            pstDev->count++;
        }
    }
    mf_disk_put_rw_list(diskList);

    return pstDev->count;
}

void
msfs_bkp_put_dev_list(struct bkp_dev_t *pstDev)
{
    if (!pstDev) {
        MFerr("pstDev is NULL");
        return;
    }

    MFinfo("msfs_bkp_put_dev_list");
    struct item_dev_t *item;
    struct item_dev_t *n;

    list_for_each_entry_safe(item, n, &pstDev->list, node) {
        list_del(&item->node);
        ms_free(item);
        pstDev->count--;
    }
    mf_bkp_put_format();
}

MF_BOOL
msfs_bkp_stream_is_valid(struct stream_seg *stream)
{
    return mf_retr_stream_is_valid(stream);
}

void
msfs_bkp_dbg_mem_usage()
{
    MFinfo("msfs_bkp_dbg_mem_usage");
    mf_retr_dbg_show_mem_usage();
}

void
msfs_bkp_dbg_switch(MF_BOOL bDebug)
{
    MFinfo("bDebug[%d]", bDebug);
    mf_retr_dbg_switch(bDebug);
}

MF_S32
msfs_bkp_init(RETR_JPG_CB jpg_cb)
{
    mf_retr_jpg_encoder(jpg_cb);
    MFinfo("msfs_bkp_init");
    return MF_SUCCESS;
}

MF_S32
msfs_bkp_deinit()
{
    MFinfo("msfs_bkp_deinit");
    return MF_SUCCESS;
}

