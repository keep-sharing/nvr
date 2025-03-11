/*
 * ***************************************************************
 * Filename:        MFindex.h
 * Created at:      2017.05.10
 * Description:     index api.
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */

#ifndef __MFINDEX_H__
#define __MFINDEX_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "bplustree.h"
#include "utils.h"
#include "MFrecord.h"
//#include "index.h"

MF_S32
mf_index_init();
MF_S32
mf_index_deinit();
MF_S32
mf_index_data_init(struct bplus_tree *tree, struct bplus_key *key);
MF_S32
mf_index_key_init(struct bplus_key *key, enum bplus_type enType);
MF_S32
mf_index_key_insert(struct bplus_tree *tree, struct bplus_key *key, struct bplus_data *data);
MF_S32
mf_index_key_delete(struct bplus_tree *tree, struct bplus_key *key);
MF_S32
mf_index_key_backup(struct bplus_tree *tree);
MF_S32
mf_index_key_recover(struct bplus_tree *tree);
MF_BOOL
mf_index_key_find(struct bplus_tree *tree, struct bplus_key *key, struct bplus_result *result);
void
mf_index_key_print(struct bplus_key *key);
struct bplus_node *
mf_index_node_fetch(struct bplus_tree *tree, off64_t offset);
void
mf_index_node_release(struct bplus_tree *tree, struct bplus_node *node);
struct bplus_key *
mf_index_node_key(struct bplus_node *node, MF_S32 index);
MF_BOOL
mf_index_find_used(struct bplus_tree *tree, MF_U8 chnId, MF_U8 type, struct bplus_result *result);
MF_BOOL
mf_index_find_using(struct bplus_tree *tree, MF_U8 chnId, MF_U8 type, struct bplus_result *result);
MF_BOOL
mf_index_find_chnid(struct bplus_tree *tree, MF_U8 chnId, struct bplus_result *result);
MF_BOOL
mf_index_find_empty(struct bplus_tree *tree, struct bplus_result *result);
void
mf_index_set_state(struct bplus_tree *tree, struct bplus_key *key, MF_U8 state);
MF_BOOL
mf_index_set_state_to_using(struct bplus_tree *tree, MF_U8 chnId, FILE_TYPE_EN enType, FILE_STATE_EN enState,
                            struct bplus_key *key);
struct bplus_tree *
mf_index_rebuild(struct bplus_tree *tree, char *filename, off64_t boot, void *owner, MF_BOOL isReset);
MF_U32
mf_index_size(struct bplus_tree *tree);
MF_S32
mf_index_set_crc(struct bplus_tree *tree, MF_U32 crc);
MF_U32
mf_index_get_crc(struct bplus_tree *tree, MF_S8 *data, MF_U32 size);
MF_BOOL
mf_index_is_valid(struct bplus_tree *tree);
void
mf_index_dump(struct bplus_tree *tree);
struct bplus_tree *
mf_index_fetch(char *filename, off64_t boot, void *owner);
void
mf_index_release(struct bplus_tree *tree);
MF_S32
mf_index_read_out(struct bplus_tree *tree, MF_S8 *out, MF_U32 size);


#ifdef __cplusplus
}
#endif

#endif


