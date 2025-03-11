/*
 * ***************************************************************
 * Filename:        MFindex.c
 * Created at:      2017.05.10
 * Description:     index API
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

#include "MFdisk.h"
#include "MFrecord.h"

typedef struct mfindex {
    TASK_HANDLE      pthread;
    MF_BOOL          bRun;
    MUTEX_OBJECT     mutex;
    MF_COND_T        cond;
    struct list_head list;
} mfindex;

static struct mfindex g_stIndex = {0};

static void *
index_task_pthread(void *argv)
{
    struct mfindex *pstIndex = (struct mfindex *)argv;
    struct list_head *diskList = NULL;
    struct diskObj *disk;

    ms_task_set_name("msfs_index_state_pthread");

    while (pstIndex->bRun) {
        ms_mutex_lock(&pstIndex->mutex);
        mf_cond_timedwait(&pstIndex->cond, &pstIndex->mutex, 10 * 1000000);
        ms_mutex_unlock(&pstIndex->mutex);

        diskList = mf_disk_get_rd_list(MF_YES);
        if (!diskList) {
            continue;
        }
        list_for_each_entry(disk, diskList, node) {
            if (disk->bRec == MF_NO) {
                continue;
            }

            if (disk->enState != DISK_STATE_NORMAL) {
                continue;
            }
            ms_mutex_lock(&disk->mutex);
            mf_index_key_recover(disk->tree);
            ms_mutex_unlock(&disk->mutex);
        }
        mf_disk_put_rw_list(diskList);
    }

    return MF_SUCCESS;
}

static MF_S32
index_task_create(struct mfindex *pstIndex)
{
    pstIndex->bRun = MF_YES;
    ms_task_create_join_stack_size(&pstIndex->pthread,
                                   256 * 1024, index_task_pthread,
                                   pstIndex);

    return MF_SUCCESS;
}

static MF_S32
index_task_destroy(struct mfindex *pstIndex)
{
    ms_mutex_lock(&pstIndex->mutex);
    pstIndex->bRun = MF_NO;
    mf_cond_signal(&pstIndex->cond);
    ms_mutex_unlock(&pstIndex->mutex);

    if (pstIndex->pthread) {
        ms_task_join(&pstIndex->pthread);
    }

    return MF_SUCCESS;
}

MF_BOOL
mf_index_find_empty(struct bplus_tree *tree,
                    struct bplus_result *result)
{
    struct bplus_key stKey;

    memset(result, 0, sizeof(struct bplus_result));
    mf_index_key_init(&stKey, TYPE_RECORD);
    stKey.stRecord.condition = FILE_CONDITION_GOOD;
    stKey.stRecord.state = FILE_STATE_EMPTY;
    stKey.stRecord.fileNo = 0;
    if (mf_index_key_find(tree, &stKey, result) == MF_NO) {
        return MF_NO;
    }
    if (result->key.stRecord.state == FILE_STATE_EMPTY) {
        return MF_YES;
    } else {
        MFwarm("state dismatch fileNO[%d]chnID[%d]state[%d]\n",
               result->key.stRecord.fileNo, result->key.stRecord.chnId, result->key.stRecord.state);
        return MF_NO;
    }
}

MF_BOOL
mf_index_find_used(struct bplus_tree *tree,
                   MF_U8 chnId, MF_U8 type,
                   struct bplus_result *result)
{
    struct bplus_key stKey;

    memset(result, 0, sizeof(struct bplus_result));
    mf_index_key_init(&stKey, TYPE_RECORD);
    stKey.stRecord.condition = FILE_CONDITION_GOOD;
    stKey.stRecord.chnId = chnId;
    stKey.stRecord.type = type;
    stKey.stRecord.state = FILE_STATE_USED;
    stKey.stRecord.startTime = 0;
    stKey.stRecord.endTime = 0;

    if (mf_index_key_find(tree, &stKey, result) == MF_NO) {
        MFerr("no find !\n");
        return MF_NO;
    }
    if (result->key.stRecord.chnId == chnId &&
        result->key.stRecord.type == type   &&
        result->key.stRecord.state == FILE_STATE_USED) {
        return MF_YES;
    } else {
        return MF_NO;
    }
}

MF_BOOL
mf_index_find_using(struct bplus_tree *tree,
                    MF_U8 chnId, MF_U8 type,
                    struct bplus_result *result)
{
    struct bplus_key stKey;

    memset(result, 0, sizeof(struct bplus_result));
    mf_index_key_init(&stKey, TYPE_RECORD);
    stKey.stRecord.condition = FILE_CONDITION_GOOD;
    stKey.stRecord.chnId = chnId;
    stKey.stRecord.type = type;
    stKey.stRecord.state = FILE_STATE_USING;
    stKey.stRecord.startTime = 0;
    stKey.stRecord.endTime = 0;

    if (mf_index_key_find(tree, &stKey, result) == MF_NO) {
        MFerr("no find !\n");
        return MF_NO;
    }
    if (result->key.stRecord.chnId == chnId &&
        result->key.stRecord.type == type   &&
        result->key.stRecord.state == FILE_STATE_USING) {
        return MF_YES;
    } else {
        return MF_NO;
    }
}

MF_BOOL
mf_index_find_chnid(struct bplus_tree *tree,
                    MF_U8 chnId,
                    struct bplus_result *result)
{
    struct bplus_key stKey;

    memset(result, 0, sizeof(struct bplus_result));
    memset(&stKey, 0, sizeof(struct bplus_key));
    stKey.enType = TYPE_RECORD;
    stKey.stRecord.condition = FILE_CONDITION_GOOD;
    stKey.stRecord.chnId = chnId;

    if (mf_index_key_find(tree, &stKey, result) == MF_NO) {
        return MF_NO;
    }

    if (result->key.stRecord.chnId == chnId) {
        return MF_YES;
    } else {
        return MF_NO;
    }
}

void
mf_index_set_state(struct bplus_tree *tree,
                   struct bplus_key *key,
                   MF_U8 state)
{
    struct bplus_data stData;
    MF_S32 res;

    if (key->stRecord.state == state) {
        return;
    }
    res = bplus_tree_delete(tree, key);
    if (res == MF_SUCCESS) {
        stData.enType = TYPE_RECORD;
        stData.stRecord.fileOffset = key->stRecord.fileOffset;
        key->stRecord.state = state;
        res = bplus_tree_insert(tree, key, &stData);
        if (res == MF_SUCCESS) {
            bplus_tree_backup(tree);
        } else {
            bplus_tree_recover(tree);
        }
    }

}

MF_BOOL
mf_index_set_state_to_using(struct bplus_tree *tree,
                            MF_U8 chnId,
                            FILE_TYPE_EN enType,
                            FILE_STATE_EN enState,
                            struct bplus_key *key)
{
    struct bplus_data stData;
    MF_S32 res;

    MFinfo("######step 1 bplus_tree_delete");
    if (bplus_tree_delete(tree, key) == MF_SUCCESS) {
        MFinfo("######step 2 bplus_tree_insert");
        stData.enType = TYPE_RECORD;
        stData.stRecord.fileOffset = key->stRecord.fileOffset;
        key->stRecord.chnId = chnId;
        key->stRecord.type = enType;
        key->stRecord.state = enState;
        key->stRecord.event = REC_EVENT_NONE;
        key->stRecord.startTime = INIT_START_TIME;
        key->stRecord.endTime = INIT_END_TIME;
        key->stRecord.infoType = 0;
        key->stRecord.segNumLsb = 0;
        key->stRecord.segNumMsb = 0;
        res = bplus_tree_insert(tree, key, &stData);
        MFinfo("######step 3 res = %d", res);
        if (res == MF_SUCCESS) {
            bplus_tree_backup(tree);
            return MF_YES;
        } else {
            bplus_tree_recover(tree);
            return MF_NO;
        }
    }

    return MF_NO;
}
inline MF_S32
mf_index_data_init(struct bplus_tree *tree, struct bplus_key *key)
{
    struct mf_file *pstFileHeader = ms_valloc(FILE_HEAD_MAX_SIZE);
    struct mf_segment *pstSegHeader = ms_valloc(SEG_HEAD_MAX_SIZE);
    struct mf_tag *pstTagHeader = ms_valloc(TAG_HEAD_MAX_SIZE);
    struct diskObj *pstDisk = tree->owner;
    MF_S32 fd;
    MF_S32 res;

    fd = pstDisk->ops->disk_file_open(pstDisk->pstPrivate, key->stRecord.fileNo, MF_YES);
    memset(pstFileHeader, 0, FILE_HEAD_MAX_SIZE);
    pstFileHeader->chnId            = key->stRecord.chnId;
    pstFileHeader->fileType         = key->stRecord.type;
    pstFileHeader->fileState        = key->stRecord.state;
    pstFileHeader->fileOffset       = key->stRecord.fileOffset;
    pstFileHeader->fileNo           = key->stRecord.fileNo;
    pstFileHeader->recStartOffset   = 0;
    pstFileHeader->recEndOffset     = 0;
    pstFileHeader->recStartTime     = INIT_START_TIME;
    pstFileHeader->recEndTime       = INIT_END_TIME;
    pstFileHeader->infoStartOffset  = INFO_START_OFFSET;
    pstFileHeader->infoEndOffset    = INFO_START_OFFSET;
    pstFileHeader->infoStartTime    = INIT_START_TIME;
    pstFileHeader->infoEndTime      = INIT_END_TIME;
    pstFileHeader->infoTypes        = INFO_MAJOR_INIT;
    pstFileHeader->infoSubTypes     = INFO_MINOR_INIT;
    pstFileHeader->recEvent         = REC_EVENT_NONE;
    pstFileHeader->magic            = FILE_MAGIC;
    res = pstDisk->ops->disk_file_write(pstDisk->pstPrivate,
                                        fd, pstFileHeader, FILE_HEAD_MAX_SIZE,
                                        pstFileHeader->fileOffset + FILE_HEAD_OFFSET);
    if (res < 0) {
        MFerr("disk_write failed");
    }

    memset(pstSegHeader, 0, SEG_HEAD_MAX_SIZE);
    res = pstDisk->ops->disk_file_write(pstDisk->pstPrivate,
                                        fd, pstSegHeader, SEG_HEAD_MAX_SIZE,
                                        pstFileHeader->fileOffset + SEG_HEAD_OFFSET);
    if (res < 0) {
        MFerr("disk_write failed");
    }

    memset(pstTagHeader, 0, TAG_HEAD_MAX_SIZE);
    res = pstDisk->ops->disk_file_write(pstDisk->pstPrivate,
                                        fd, pstTagHeader, TAG_HEAD_MAX_SIZE,
                                        pstFileHeader->fileOffset + TAG_HEAD_OFFSET);
    if (res < 0) {
        MFerr("disk_write failed");
    }

    ms_free(pstFileHeader);
    ms_free(pstSegHeader);
    ms_free(pstTagHeader);
    pstDisk->ops->disk_file_close(pstDisk->pstPrivate, fd);

    return MF_SUCCESS;
}

inline MF_S32
mf_index_key_init(struct bplus_key *key, enum bplus_type enType)
{
    memset(key, 0xff, sizeof(struct bplus_key));
    key->enType = enType;
    if (enType == TYPE_RECORD) {
        key->stRecord.chnId     = INIT_CHN_ID;
        key->stRecord.type      = FILE_TYPE_INIT;
        key->stRecord.state     = FILE_STATE_EMPTY;
        key->stRecord.condition = FILE_CONDITION_GOOD;
        key->stRecord.event     = REC_EVENT_NONE;
        key->stRecord.startTime = INIT_START_TIME;
        key->stRecord.endTime   = INIT_END_TIME;
        key->stRecord.infoType  = 0;
        key->stRecord.segNumLsb = 0;
        key->stRecord.segNumMsb = 0;
    } else if (enType == TYPE_LOG) {

    }

    return MF_SUCCESS;
}

MF_BOOL
mf_index_key_find(struct bplus_tree *tree,
                  struct bplus_key *key, struct bplus_result *result)
{
    return bplus_tree_find(tree, key, result);
}

MF_S32
mf_index_key_insert(struct bplus_tree *tree,
                    struct bplus_key *key, struct bplus_data *data)
{
    MF_S32 res;
    res = bplus_tree_insert(tree, key, data);

    return res;
}

MF_S32
mf_index_key_delete(struct bplus_tree *tree, struct bplus_key *key)
{
    MF_S32 res;
    res =  bplus_tree_delete(tree, key);

    return res;
}

MF_S32
mf_index_key_backup(struct bplus_tree *tree)
{
    if (!tree) {
        return MF_FAILURE;
    }

    if (bplus_tree_backup(tree) == MF_YES) {
        return MF_SUCCESS;
    } else {
        return MF_FAILURE;
    }
}

MF_S32
mf_index_key_recover(struct bplus_tree *tree)
{
    if (!tree) {
        return MF_FAILURE;
    }

    if (bplus_tree_recover(tree) == MF_YES) {
        return MF_SUCCESS;
    } else {
        return MF_FAILURE;
    }
}

void
mf_index_key_print(struct bplus_key *key)
{

    if (key->enType == TYPE_RECORD) {
        MF_S8 stime1[32] = {0};
        MF_S8 stime2[32] = {0};
        MFinfo("record key info >>>");
        MFinfo("cond: \t%u", key->stRecord.condition);
        MFinfo("chnId: \t%u", key->stRecord.chnId);
        MFinfo("type: \t%u", key->stRecord.type);
        MFinfo("state: \t%u", key->stRecord.state);
        MFinfo("segNum: \t%d", key->stRecord.segNumLsb + (
            key->stRecord.segNumMsb == 255 ? 0 : key->stRecord.segNumMsb * 256));
        MFinfo("event: \t0x%x", key->stRecord.event);
        MFinfo("info: \t0x%x", key->stRecord.infoType);
        MFinfo("file: \t%u-%#llx", key->stRecord.fileNo, key->stRecord.fileOffset);
        time_to_string_local(key->stRecord.startTime, stime1);
        time_to_string_local(key->stRecord.endTime, stime2);
        MFinfo("time: \t%s-%s", stime1, stime2);
    } else {
        MFinfo("log key info >>>\n");
    }
}

//static int count = 0;

struct bplus_node *
mf_index_node_fetch(struct bplus_tree *tree, off64_t offset)
{
    struct bplus_node *node;

    node = bplus_tree_fetch_node(tree, offset);
//    if (node)
//    {
//        count++;
//        printf("=========count = %d\n", count);
//    }
    return node;
}

void
mf_index_node_release(struct bplus_tree *tree, struct bplus_node *node)
{
    if (node) {
//        count--;
//        printf("=========count = %d\n", count);
        bplus_tree_release_node(tree, node);
    }
}

struct bplus_key *
mf_index_node_key(struct bplus_node *node, MF_S32 index)
{
    return bplus_tree_node_key(node, index);
}

void
mf_index_dump(struct bplus_tree *tree)
{
    if (tree) {
        bplus_tree_dump(tree);
    }
}

MF_U32
mf_index_size(struct bplus_tree *tree)
{
    return bplus_tree_file_size(tree);
}

MF_BOOL
mf_index_is_valid(struct bplus_tree *tree)
{
    if (!tree) {
        return MF_NO;
    } else {
        return bplus_tree_is_valid(tree);
    }
}

struct bplus_tree *
mf_index_fetch(MF_S8 *filename, off64_t boot, void *owner)
{
    struct bplus_tree *tree;
    MF_BOOL bDone;

    tree = bplus_tree_init(filename, boot, owner, MF_NO);
    if (tree) {
        // check if necessary
        bDone = bplus_tree_recover(tree);
        if (bDone == MF_YES) {
            // if recover must rebuild new tree
            tree = mf_index_rebuild(tree, filename, boot, owner, MF_NO);
        }
    }

    return tree;
}

void
mf_index_release(struct bplus_tree *tree)
{
    if (tree != NULL) {
        msprintf("[wcm] bplus_tree_deinit begin");
        bplus_tree_deinit(tree);
        msprintf("[wcm] bplus_tree_deinit end");
    }
}

struct bplus_tree *
mf_index_rebuild(struct bplus_tree *tree,
                 MF_S8 *filename, off64_t boot,
                 void *owner,
                 MF_BOOL isReset)
{
    if (tree != NULL) {
        msprintf("[wcm] bplus_tree_deinit begin");
        bplus_tree_deinit(tree);
        msprintf("[wcm] bplus_tree_deinit end");
    }

    return bplus_tree_init(filename, boot, owner, isReset);
}


MF_S32
mf_index_read_out(struct bplus_tree *tree, MF_S8 *out, MF_U32 size)
{
    return bplus_tree_read_from_disk(tree, out, size);
}

MF_U32
mf_index_get_crc(struct bplus_tree *tree, MF_S8 *data, MF_U32 size)
{
    return bplus_tree_calc_crc(tree, data, size);
}

MF_S32
mf_index_set_crc(struct bplus_tree *tree, MF_U32 crc)
{
    return bplus_tree_update_crc(tree, crc);
}

MF_S32
mf_index_init()
{
    ms_mutex_init(&g_stIndex.mutex);
    mf_cond_init(&g_stIndex.cond);
    index_task_create(&g_stIndex);

    return MF_SUCCESS;
}

MF_S32
mf_index_deinit()
{
    index_task_destroy(&g_stIndex);
    ms_mutex_uninit(&g_stIndex.mutex);
    mf_cond_uninit(&g_stIndex.cond);

    return MF_SUCCESS;
}

