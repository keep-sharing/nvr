/*
 * Copyright (C) 2017, Leo Ma <begeekmyfriend@gmail.com>
 * https://github.com/begeekmyfriend/bplustree/commits/disk-io/lib
 */

#ifndef _BPLUS_TREE_H
#define _BPLUS_TREE_H

#include "list.h"
#include "mf_type.h"

#define MIN_CACHE_NUM 20
#define MAX_BLOCK_SIZE 4096
#define BPTREE_MAGIC 0x20180509
#define BPTREE_VER   "Jan 28, 2018"
#define MAX_BLOCK_NUM (MAX_BLOCK_SIZE - sizeof(struct bplus_info))/sizeof(off64_t)

typedef enum {
    FILE_CONDITION_GOOD = 1,
    FILE_CONDITION_BAD,
}FILE_CONDITION_EN;

typedef enum bplus_state
{
    STATE_UPDATE = 1,
    STATE_DONE,
}bplus_state;

typedef enum bplus_type
{
    TYPE_RECORD = 0,
    TYPE_LOG,
    TYPE_COUNT,
}bplus_type;

typedef struct free_block {
    struct list_head link;
    off64_t offset;
} free_block;

struct bplus_info{
    MF_S32 magic;
    MF_S32 block_size;
    MF_S32 block_count;
    MF_S32 free_blocks;
    MF_S32 level;
    MF_PTS update_time;
    enum bplus_state enState;
    MF_U32 crc;
    MF_S32 res1;
    MF_S32 res2;
    off64_t root;
    off64_t file_size;
    MF_S32 total;
    MF_S32 fileBad;
};

struct bplus_boot{
    struct bplus_info stInfo;
    off64_t free_offset[MAX_BLOCK_NUM];
};

typedef struct bplus_node {
    off64_t self;
    off64_t parent;
    off64_t prev;
    off64_t next;
    MF_S32  type;
    MF_S32  children;
    MF_S32  maxOrder;
    MF_S32  maxEntries;
    MF_S32  reserve;
} bplus_node;

typedef struct key_record
{
    MF_U8   chnId;
    MF_U8   type;
    MF_U8   state;
    MF_U8   isLocked;	//0:no lock -- 1:locked
    MF_U32  fileNo;
    MF_U32  infoType;//事件类型 major|minor @STID_MAJOR_EN @INFO_MINOR_EN
    MF_U32  event;//录像类型 @REC_EVENT_EN
    MF_U8   segNumLsb;  //LSB 0-256
    MF_U8   condition;
    MF_U8   segNumMsb; //MSB n*256
    MF_U8   reserve;
    MF_PTS  startTime;
    MF_PTS  endTime;    
    off64_t fileOffset;
}key_record;

typedef struct key_log
{
    MF_U32  mainType;
    MF_U32  subType;
    MF_PTS  pts;    
    MF_PTS  reserve2;
    off64_t fileOffset;
}key_log;

typedef struct data_record
{
    off64_t   fileOffset;

}data_record;

typedef struct data_log
{
    off64_t   phyaddr;
    MF_S32    size;
}data_log;

typedef struct bplus_key{
    enum bplus_type  enType;
    union
    {
        struct key_record stRecord;
        struct key_log    stLog;
    };
}bplus_key;

typedef struct bplus_data{
    enum bplus_type  enType;
    union
    {
        struct data_record stRecord;
        struct data_log    stLog;
    };
}bplus_data;

typedef struct bplus_result{
    off64_t nodeSelf;
    off64_t nodeParent;
    off64_t nodePrev;
    off64_t nodeNext;
    struct bplus_key key;
    MF_S32  keyCount;
    MF_S32  keyIndex;
}bplus_result;

typedef struct bplus_tree {
    MF_S8    filename[1024];
    MF_S8    *caches;
    MF_S32   used[MIN_CACHE_NUM];
    pthread_mutex_t mutex;
    MF_S32    fd;
    MF_S32    order;
    MF_S32    entries;
    MF_S32    level;
    MF_S32    block_size;
    MF_S32    block_count;
    MF_S32    total;
    MF_S32    fileBad;
    off64_t   boot;
    off64_t   root;
    off64_t   file_size;
    struct list_head free_blocks;
    struct diskObj *owner;
    MF_BOOL   isUpdate;
}bplus_tree;

void 
bplus_tree_dump(struct bplus_tree *tree);
MF_BOOL
bplus_tree_find(struct bplus_tree *tree, struct bplus_key *key, struct bplus_result *result);
MF_S32
bplus_tree_insert(struct bplus_tree *tree, struct bplus_key *key, struct bplus_data *data);
MF_S32
bplus_tree_delete(struct bplus_tree *tree, struct bplus_key *key);
struct bplus_node *
bplus_tree_fetch_node(struct bplus_tree *tree, off64_t offset);
void
bplus_tree_release_node(struct bplus_tree * tree,struct bplus_node * node);
struct bplus_key *
bplus_tree_node_key(struct bplus_node * node, MF_S32 index);
MF_S32
bplus_tree_update_begin(struct bplus_tree *tree);
MF_S32
bplus_tree_update_end(struct bplus_tree *tree);
MF_S32
bplus_tree_update_crc(struct bplus_tree *tree, MF_U32 crc);
MF_S32
bplus_tree_calc_crc(struct bplus_tree *tree, MF_S8 *data, MF_U32 size);
MF_BOOL
bplus_tree_backup(struct bplus_tree *tree);
MF_BOOL
bplus_tree_recover(struct bplus_tree *tree);
MF_S32
bplus_tree_read_from_disk(struct bplus_tree *tree, MF_S8 *out, MF_U32 size);
MF_U32
bplus_tree_file_size(struct bplus_tree *tree);
MF_U32
bplus_tree_node_size(struct bplus_tree *tree);
MF_BOOL
bplus_tree_is_valid(bplus_tree *tree);
struct bplus_tree *
bplus_tree_init(MF_S8 *filename, off64_t boot, void *owner, MF_BOOL isReset);
void 
bplus_tree_deinit(struct bplus_tree *tree);

#endif  /* _BPLUS_TREE_H */
