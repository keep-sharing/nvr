#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "bplustree.h"
#include "disk.h"
#include "msstd.h"
#include "utils.h"

enum {
    INVALID_OFFSET = -1,
};

enum {
    BPLUS_TREE_LEAF,
    BPLUS_TREE_NON_LEAF = 1,
};

enum {
    LEFT_SIBLING,
    RIGHT_SIBLING = 1,
};


/* 5 node caches are needed at least for self, left and right sibling, sibling
 * of sibling, parent and node seeking */

#define offset_ptr(node) ((MF_S8 *)node + sizeof(*node))
#define key(node) ((struct bplus_key *)offset_ptr(node))
#define data(node) ((struct bplus_data *)(offset_ptr(node) + node->maxEntries * sizeof(struct bplus_key)))
#define sub(node) ((off64_t *)(offset_ptr(node) + (node->maxOrder - 1) * sizeof(struct bplus_key)))

static MF_S32
set_boot_state(struct bplus_tree *tree, enum bplus_state enState)
{
    MF_S32 res;
    struct diskObj *pstDisk = tree->owner;
    struct bplus_boot *boot = ms_malloc(sizeof(*boot));

    res = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                       tree->fd, boot, sizeof(*boot), tree->boot);
    if (res < 0) {
        ms_free(boot);
        MFerr("disk_read failed");
        return -1;
    }

    boot->stInfo.enState = enState;

    res = pstDisk->ops->disk_file_write(pstDisk->pstPrivate,
                                        tree->fd, boot, sizeof(*boot), tree->boot);
    if (res < 0) {
        MFerr("disk_write failed");
    }
    ms_free(boot);

    return res;

}

static MF_S32
set_boot_crc(struct bplus_tree *tree, MF_U32 crc)
{
    struct diskObj *pstDisk = tree->owner;
    MF_S32 res;

    struct bplus_boot *boot = ms_malloc(sizeof(*boot));

    res = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                       tree->fd, boot, sizeof(*boot), tree->boot);
    if (res < 0) {
        ms_free(boot);
        MFerr("disk_read failed");
        return -1;
    }

    boot->stInfo.crc = crc;

    res = pstDisk->ops->disk_file_write(pstDisk->pstPrivate,
                                        tree->fd, boot, sizeof(*boot), tree->boot);
    if (res < 0) {
        MFerr("disk_write failed");
    }

    ms_free(boot);
    MFinfo("boot CRC is %x with res %d", crc, res);
    return res;

}

static inline void
set_boot_info(struct bplus_tree *tree)
{
    struct list_head *pos = NULL;
    struct list_head *n = NULL;
    struct diskObj *pstDisk = tree->owner;
    struct bplus_boot *boot = ms_malloc(sizeof(*boot));
    MF_S32 res;

    assert(boot != NULL);

    /* store boot information  for future reuse */
    boot->stInfo.magic = BPTREE_MAGIC;
    boot->stInfo.block_size = tree->block_size;
    boot->stInfo.block_count = tree->block_count;
    boot->stInfo.file_size = tree->file_size;
    boot->stInfo.total = tree->total;
    boot->stInfo.fileBad = tree->fileBad;
    boot->stInfo.root = tree->root;
    boot->stInfo.level = tree->level;
    boot->stInfo.free_blocks = 0;
    boot->stInfo.update_time = time(0);
    boot->stInfo.enState = STATE_DONE;
    list_for_each_safe(pos, n, &tree->free_blocks) {
        struct free_block *block = list_entry(pos, struct free_block, link);
        boot->free_offset[boot->stInfo.free_blocks++] = block->offset;
    }
    res = pstDisk->ops->disk_file_write(pstDisk->pstPrivate,
                                        tree->fd, boot, sizeof(*boot), tree->boot);
    if (res < 0) {
        MFerr("disk_write failed");
    }
    ms_free(boot);

}

static inline MF_S32
get_boot_info(struct bplus_tree *tree, MF_BOOL isReset)
{
    MF_S32 i;
    MF_S32 res;
    struct diskObj *pstDisk = tree->owner;
    struct bplus_boot *boot = ms_malloc(sizeof(*boot));
    /* read boot and check index integrity*/
    res = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                       tree->fd, boot, sizeof(*boot), tree->boot);
    if (res < 0) {
        MFerr("disk_read failed");
        ms_free(boot);
        return MF_FAILURE;
    }

    if (isReset == MF_YES
        || boot->stInfo.magic != BPTREE_MAGIC
        || boot->stInfo.enState != STATE_DONE) {
        boot->stInfo.block_size = MAX_BLOCK_SIZE;
        boot->stInfo.block_count = 0;
        boot->stInfo.level = 0;
        boot->stInfo.root = INVALID_OFFSET;
        boot->stInfo.file_size = tree->boot + sizeof(*boot);
        boot->stInfo.free_blocks = 0;
        boot->stInfo.total = 0;
        boot->stInfo.fileBad = 0;
        MFinfo("B+tree is reset");
    }

    /* get free blocks  to reuse*/
    INIT_LIST_HEAD(&tree->free_blocks);
    for (i = 0; i < boot->stInfo.free_blocks; i++) {
        struct free_block *block = ms_malloc(sizeof(*block));
        assert(block != NULL);
        block->offset = boot->free_offset[i];
        list_add(&block->link, &tree->free_blocks);
    }

    tree->root = boot->stInfo.root;
    tree->level = boot->stInfo.level;
    tree->block_size = boot->stInfo.block_size;
    tree->block_count = boot->stInfo.block_count;
    tree->file_size = boot->stInfo.file_size;
    tree->total = boot->stInfo.total;
    tree->fileBad = boot->stInfo.fileBad;

    /* update boot if abnormal*/
    if (isReset == MF_YES || boot->stInfo.magic != BPTREE_MAGIC) {
        set_boot_info(tree);
    }

    ms_free(boot);
    return MF_SUCCESS;
}

static inline MF_S32
is_leaf(struct bplus_node *node)
{
    return node->type == BPLUS_TREE_LEAF;
}

static MF_S32
key_record_compare(struct bplus_key *key1, MF_S8 type, struct bplus_key *key2)
{
    MF_S32 result = 0;

    switch (type) {
        case '<':
            if (key1->stRecord.condition < key2->stRecord.condition
                || (key1->stRecord.condition == key2->stRecord.condition
                    && key1->stRecord.chnId < key2->stRecord.chnId)
                || (key1->stRecord.condition == key2->stRecord.condition
                    && key1->stRecord.chnId == key2->stRecord.chnId
                    && key1->stRecord.type < key2->stRecord.type)
                || (key1->stRecord.condition == key2->stRecord.condition
                    && key1->stRecord.chnId == key2->stRecord.chnId
                    && key1->stRecord.type == key2->stRecord.type
                    && key1->stRecord.isLocked < key2->stRecord.isLocked)
                || (key1->stRecord.condition == key2->stRecord.condition
                    && key1->stRecord.chnId == key2->stRecord.chnId
                    && key1->stRecord.type == key2->stRecord.type
                    && key1->stRecord.isLocked == key2->stRecord.isLocked
                    && key1->stRecord.state < key2->stRecord.state)
                || (key1->stRecord.condition == key2->stRecord.condition
                    && key1->stRecord.chnId == key2->stRecord.chnId
                    && key1->stRecord.type == key2->stRecord.type
                    && key1->stRecord.isLocked == key2->stRecord.isLocked
                    && key1->stRecord.state == key2->stRecord.state
                    && key1->stRecord.startTime < key2->stRecord.startTime)
                || (key1->stRecord.condition == key2->stRecord.condition
                    && key1->stRecord.chnId == key2->stRecord.chnId
                    && key1->stRecord.type == key2->stRecord.type
                    && key1->stRecord.isLocked == key2->stRecord.isLocked
                    && key1->stRecord.state == key2->stRecord.state
                    && key1->stRecord.startTime == key2->stRecord.startTime
                    && key1->stRecord.endTime < key2->stRecord.endTime)
                || (key1->stRecord.condition == key2->stRecord.condition
                    && key1->stRecord.chnId == key2->stRecord.chnId
                    && key1->stRecord.type == key2->stRecord.type
                    && key1->stRecord.isLocked == key2->stRecord.isLocked
                    && key1->stRecord.state == key2->stRecord.state
                    && key1->stRecord.startTime == key2->stRecord.startTime
                    && key1->stRecord.endTime == key2->stRecord.endTime
                    && key1->stRecord.fileNo < key2->stRecord.fileNo)
               )

            {
                result = 1;
            } else {
                result = 0;
            }
            break;
        case '>':
            if (key1->stRecord.condition > key2->stRecord.condition
                || (key1->stRecord.condition == key2->stRecord.condition
                    && key1->stRecord.chnId > key2->stRecord.chnId)
                || (key1->stRecord.condition == key2->stRecord.condition
                    && key1->stRecord.chnId == key2->stRecord.chnId
                    && key1->stRecord.type > key2->stRecord.type)
                || (key1->stRecord.condition == key2->stRecord.condition
                    && key1->stRecord.chnId == key2->stRecord.chnId
                    && key1->stRecord.type == key2->stRecord.type
                    && key1->stRecord.isLocked > key2->stRecord.isLocked)
                || (key1->stRecord.condition == key2->stRecord.condition
                    && key1->stRecord.chnId == key2->stRecord.chnId
                    && key1->stRecord.type == key2->stRecord.type
                    && key1->stRecord.isLocked == key2->stRecord.isLocked
                    && key1->stRecord.state > key2->stRecord.state)
                || (key1->stRecord.condition == key2->stRecord.condition
                    && key1->stRecord.chnId == key2->stRecord.chnId
                    && key1->stRecord.type == key2->stRecord.type
                    && key1->stRecord.isLocked == key2->stRecord.isLocked
                    && key1->stRecord.state == key2->stRecord.state
                    && key1->stRecord.startTime > key2->stRecord.startTime)
                || (key1->stRecord.condition == key2->stRecord.condition
                    && key1->stRecord.chnId == key2->stRecord.chnId
                    && key1->stRecord.type == key2->stRecord.type
                    && key1->stRecord.isLocked == key2->stRecord.isLocked
                    && key1->stRecord.state == key2->stRecord.state
                    && key1->stRecord.startTime == key2->stRecord.startTime
                    && key1->stRecord.endTime > key2->stRecord.endTime)
                || (key1->stRecord.condition == key2->stRecord.condition
                    && key1->stRecord.chnId == key2->stRecord.chnId
                    && key1->stRecord.type == key2->stRecord.type
                    && key1->stRecord.isLocked == key2->stRecord.isLocked
                    && key1->stRecord.state == key2->stRecord.state
                    && key1->stRecord.startTime == key2->stRecord.startTime
                    && key1->stRecord.endTime == key2->stRecord.endTime
                    && key1->stRecord.fileNo > key2->stRecord.fileNo)
               )

            {
                result = 1;
            } else {
                result = 0;
            }
            break;
        case '=':
            if (key1->stRecord.condition == key2->stRecord.condition
                && key1->stRecord.chnId == key2->stRecord.chnId
                && key1->stRecord.type == key2->stRecord.type
                && key1->stRecord.isLocked == key2->stRecord.isLocked
                && key1->stRecord.state == key2->stRecord.state
                && key1->stRecord.startTime == key2->stRecord.startTime
                && key1->stRecord.endTime == key2->stRecord.endTime
                && key1->stRecord.fileNo == key2->stRecord.fileNo
               ) {
                result = 1;
            } else {
                result = 0;
            }
            break;
        default:
            result = 0;
            break;
    }

    return result;
}

static MF_S32
key_log_compare(struct bplus_key *key1, MF_S8 type, struct bplus_key *key2)
{
    MF_S32 result = 0;
    // TODO:
    return result;
}

static MF_S32
key_compare(struct bplus_key *key1, MF_S8 type, struct bplus_key *key2)
{
    MF_S32 result = 0;

//    assert(key1->enType == key2->enType);

    switch (key1->enType) {
        case TYPE_RECORD:
            result = key_record_compare(key1, type, key2);
            break;
        case TYPE_LOG:
            result = key_log_compare(key1, type, key2);
            break;
        default:
            break;
    }
    return result;
}

static MF_S32
key_binary_search(struct bplus_node *node, struct bplus_key *target)
{
    struct bplus_key *arr = key(node);
//    printf("=====lenode->countn = %d\n", node->count);
    MF_S32 len = is_leaf(node) ? node->children : node->children - 1;
    MF_S32 low = -1;
    MF_S32 high = len;
//    printf("=====len = %d\n", len);
    while (low + 1 < high) {
        MF_S32 mid = low + (high - low) / 2;
        if (key_compare(target, '>', &arr[mid])) {
            low = mid;
        } else {
            high = mid;
        }
    }

    if (high >= len || !key_compare(&arr[high], '=', target)) {
        return -high - 1;
    } else {
        return high;
    }
}

static inline MF_S32
parent_key_index(struct bplus_node *parent, struct bplus_key *key)
{
    MF_S32 index = key_binary_search(parent, key);
    return index >= 0 ? index : -index - 2;
}

static inline struct bplus_node *
cache_refer(struct bplus_tree *tree)
{
    MF_S32 i;
    for (i = 0; i < MIN_CACHE_NUM; i++) {
        if (!tree->used[i]) {
            tree->used[i] = 1;
            char *buf = tree->caches + tree->block_size * i;
            memset(buf, 0, tree->block_size);
            return (struct bplus_node *) buf;
        }
    }
    assert(0);
}

static inline void
cache_defer(struct bplus_tree *tree, struct bplus_node *node)
{
    /* return the node cache borrowed from */
    MF_S8 *buf = (MF_S8 *) node;
    MF_S32 i = (buf - tree->caches) / tree->block_size;
    tree->used[i] = 0;
}

static struct bplus_node *
node_new(struct bplus_tree *tree)
{
    struct bplus_node *node = cache_refer(tree);
    node->prev = INVALID_OFFSET;
    node->next = INVALID_OFFSET;
    node->self = INVALID_OFFSET;
    node->parent = INVALID_OFFSET;
    node->children = 0;
    node->maxOrder = tree->order;
    node->maxEntries = tree->entries;
    tree->block_count++;
    return node;
}

static inline struct bplus_node *
non_leaf_new(struct bplus_tree *tree)
{
    struct bplus_node *node = node_new(tree);
    node->type = BPLUS_TREE_NON_LEAF;
    return node;
}

static inline struct bplus_node *
leaf_new(struct bplus_tree *tree)
{
    struct bplus_node *node = node_new(tree);
    node->type = BPLUS_TREE_LEAF;
    return node;
}

static struct bplus_node *
node_fetch(struct bplus_tree *tree, off64_t offset)
{
    if (offset == INVALID_OFFSET) {
        return NULL;
    }

    struct bplus_node *node = cache_refer(tree);
    struct diskObj *pstDisk = tree->owner;
    MF_S32 len = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                              tree->fd, node, tree->block_size, offset);
    if (len != tree->block_size) {
        MFerr("len != tree->block_size with %llx\n", offset);
        cache_defer(tree, node);
        node = NULL;
    }
    return node;
}

static struct bplus_node *
node_seek(struct bplus_tree *tree, off64_t offset)
{
    if (offset == INVALID_OFFSET) {
        return NULL;
    }

    MF_S32 i;
    struct diskObj *pstDisk = tree->owner;
    for (i = 0; i < MIN_CACHE_NUM; i++) {
        if (!tree->used[i]) {
            MF_S8 *buf = tree->caches + tree->block_size * i;
            MF_S32 len = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                                      tree->fd, buf, tree->block_size, offset);
            if (len != tree->block_size) {
                MFerr("len != tree->block_size with %llx\n", offset);
                return NULL;
            }
            return (struct bplus_node *) buf;
        }
    }
    assert(0);
}

static inline void
node_flush(struct bplus_tree *tree, struct bplus_node *node)
{

    if (node != NULL) {
        struct diskObj *pstDisk = tree->owner;
        MF_S32 len = pstDisk->ops->disk_file_write(pstDisk->pstPrivate,
                                                   tree->fd, node, tree->block_size, node->self);
        if (len != tree->block_size) {
            MFerr("len != tree->block_size with %llx\n", node->self);
        }
        cache_defer(tree, node);
    }
}

static off64_t
new_node_append(struct bplus_tree *tree, struct bplus_node *node)
{
    /* assign new offset to the new node */
    if (list_empty(&tree->free_blocks)) {
        node->self = tree->file_size;
        tree->file_size += tree->block_size;
    } else {
        struct free_block *block;
        block = list_first_entry(&tree->free_blocks, struct free_block, link);
        list_del(&block->link);
        node->self = block->offset;
        ms_free(block);
    }
    return node->self;
}

static void
node_delete(struct bplus_tree *tree, struct bplus_node *node,
            struct bplus_node *left, struct bplus_node *right)
{
    if (left != NULL) {
        if (right != NULL) {
            left->next = right->self;
            right->prev = left->self;
            node_flush(tree, right);
        } else {
            left->next = INVALID_OFFSET;
        }
        node_flush(tree, left);
    } else {
        if (right != NULL) {
            right->prev = INVALID_OFFSET;
            node_flush(tree, right);
        }
    }

    assert(node->self != INVALID_OFFSET);
    struct free_block *block = ms_malloc(sizeof(*block));
    assert(block != NULL);
    /* deleted blocks can be allocated for other nodes */
    block->offset = node->self;
    tree->block_count--;
    list_add_tail(&block->link, &tree->free_blocks);
    /* return the cache borrowed from */
    cache_defer(tree, node);
}

static inline void
sub_node_update(struct bplus_tree *tree, struct bplus_node *parent,
                MF_S32 index, struct bplus_node *sub_node)
{
    assert(sub_node->self != INVALID_OFFSET);
    sub(parent)[index] = sub_node->self;
    sub_node->parent = parent->self;
    node_flush(tree, sub_node);
}

static inline void
sub_node_flush(struct bplus_tree *tree, struct bplus_node *parent,
               off64_t sub_offset)
{
    struct bplus_node *sub_node = node_fetch(tree, sub_offset);
    assert(sub_node != NULL);
    sub_node->parent = parent->self;
    node_flush(tree, sub_node);
}

static void
node_dump(struct bplus_node *node, MF_S32 i);

static MF_S32
bplus_tree_search(struct bplus_tree *tree, struct bplus_key *key, off64_t *offset)
{
    struct bplus_node *node = node_seek(tree, tree->root);
    MF_S32 i;

    while (node != NULL) {
        i = key_binary_search(node, key);
        if (is_leaf(node)) {
            if (i < 0) {
                i = -i - 1;
            }
            if (i == node->children) {
                *offset = node->next;
                i = 0;
            } else {
                *offset = node->self;
            }
            break;
        } else {
            if (i >= 0) {
                node = node_seek(tree, sub(node)[i + 1]);
            } else {
                i = -i - 1;
                node = node_seek(tree, sub(node)[i]);
            }
        }
    }
    return i;
}

static void
left_node_add(struct bplus_tree *tree, struct bplus_node *node, struct bplus_node *left)
{
    new_node_append(tree, left);

    struct bplus_node *prev = node_fetch(tree, node->prev);
    if (prev != NULL) {
        prev->next = left->self;
        left->prev = prev->self;
        node_flush(tree, prev);
    } else {
        left->prev = INVALID_OFFSET;
    }
    left->next = node->self;
    node->prev = left->self;
}

static void
right_node_add(struct bplus_tree *tree, struct bplus_node *node, struct bplus_node *right)
{
    new_node_append(tree, right);

    struct bplus_node *next = node_fetch(tree, node->next);
    if (next != NULL) {
        next->prev = right->self;
        right->next = next->self;
        node_flush(tree, next);
    } else {
        right->next = INVALID_OFFSET;
    }
    right->prev = node->self;
    node->next = right->self;
}

static MF_S32
non_leaf_insert(struct bplus_tree *tree, struct bplus_node *node,
                struct bplus_node *l_ch, struct bplus_node *r_ch, struct bplus_key *key);

static MF_S32
parent_node_build(struct bplus_tree *tree, struct bplus_node *l_ch,
                  struct bplus_node *r_ch, struct bplus_key *key)
{
    if (l_ch->parent == INVALID_OFFSET && r_ch->parent == INVALID_OFFSET) {
        /* new parent */
        struct bplus_node *parent = non_leaf_new(tree);
        key(parent)[0] = *key;
        sub(parent)[0] = l_ch->self;
        sub(parent)[1] = r_ch->self;
        parent->children = 2;
        /* write new parent and update root */
        tree->root = new_node_append(tree, parent);
        MFerr("root:%lld", tree->root);
        l_ch->parent = parent->self;
        r_ch->parent = parent->self;
        tree->level++;
        /* flush parent, left and right child */
        node_flush(tree, l_ch);
        node_flush(tree, r_ch);
        node_flush(tree, parent);
        return 0;
    } else if (r_ch->parent == INVALID_OFFSET) {
        return non_leaf_insert(tree, node_fetch(tree, l_ch->parent),
                               l_ch, r_ch, key);
    } else {
        return non_leaf_insert(tree, node_fetch(tree, r_ch->parent),
                               l_ch, r_ch, key);
    }
}

static void
non_leaf_split_left(struct bplus_tree *tree, struct bplus_node *node,
                    struct bplus_node *left, struct bplus_node *l_ch,
                    struct bplus_node *r_ch, struct bplus_key *key, MF_S32 insert, struct bplus_key *split_key)
{
    MF_S32 i;

    /* split = [m/2] */
    MF_S32 split = (tree->order + 1) / 2;

    /* split as left sibling */
    left_node_add(tree, node, left);

    /* calculate split nodes' children (sum as (order + 1))*/
    MF_S32 pivot = insert;
    left->children = split;
    node->children = tree->order - split + 1;

    /* sum = left->children = pivot + (split - pivot - 1) + 1 */
    /* replicate from key[0] to key[insert] in original node */
    memmove(&key(left)[0], &key(node)[0], pivot * sizeof(struct bplus_key));
    memmove(&sub(left)[0], &sub(node)[0], pivot * sizeof(off64_t));

    /* replicate from key[insert] to key[split - 1] in original node */
    memmove(&key(left)[pivot + 1], &key(node)[pivot], (split - pivot - 1) * sizeof(struct bplus_key));
    memmove(&sub(left)[pivot + 1], &sub(node)[pivot], (split - pivot - 1) * sizeof(off64_t));

    /* flush sub-nodes of the new splitted left node */
    for (i = 0; i < left->children; i++) {
        if (i != pivot && i != pivot + 1) {
            sub_node_flush(tree, left, sub(left)[i]);
        }
    }

    /* insert new key and sub-nodes and locate the split key */
    key(left)[pivot] = *key;
    if (pivot == split - 1) {
        /* left child in split left node and right child in original right one */
        sub_node_update(tree, left, pivot, l_ch);
        sub_node_update(tree, node, 0, r_ch);
        *split_key = *key;
    } else {
        /* both new children in split left node */
        sub_node_update(tree, left, pivot, l_ch);
        sub_node_update(tree, left, pivot + 1, r_ch);
        sub(node)[0] = sub(node)[split - 1];
        *split_key = key(node)[split - 2];
    }

    /* sum = node->children = 1 + (node->children - 1) */
    /* right node left shift from key[split - 1] to key[children - 2] */
    memmove(&key(node)[0], &key(node)[split - 1], (node->children - 1) * sizeof(struct bplus_key));
    memmove(&sub(node)[1], &sub(node)[split], (node->children - 1) * sizeof(off64_t));

}

static void
non_leaf_split_right1(struct bplus_tree *tree, struct bplus_node *node,
                      struct bplus_node *right, struct bplus_node *l_ch,
                      struct bplus_node *r_ch, struct bplus_key *key, MF_S32 insert, struct bplus_key *split_key)
{
    MF_S32 i;

    /* split = [m/2] */
    MF_S32 split = (tree->order + 1) / 2;

    /* split as right sibling */
    right_node_add(tree, node, right);

    /* split key is key[split - 1] */
    *split_key = key(node)[split - 1];

    /* calculate split nodes' children (sum as (order + 1))*/
    MF_S32 pivot = 0;
    node->children = split;
    right->children = tree->order - split + 1;

    /* insert new key and sub-nodes */
    key(right)[0] = *key;
    sub_node_update(tree, right, pivot, l_ch);
    sub_node_update(tree, right, pivot + 1, r_ch);

    /* sum = right->children = 2 + (right->children - 2) */
    /* replicate from key[split] to key[_max_order - 2] */
    memmove(&key(right)[pivot + 1], &key(node)[split], (right->children - 2) * sizeof(struct bplus_key));
    memmove(&sub(right)[pivot + 2], &sub(node)[split + 1], (right->children - 2) * sizeof(off64_t));

    /* flush sub-nodes of the new splitted right node */
    for (i = pivot + 2; i < right->children; i++) {
        sub_node_flush(tree, right, sub(right)[i]);
    }

}

static void
non_leaf_split_right2(struct bplus_tree *tree, struct bplus_node *node,
                      struct bplus_node *right, struct bplus_node *l_ch,
                      struct bplus_node *r_ch, struct bplus_key *key, MF_S32 insert, struct bplus_key *split_key)
{
    MF_S32 i;

    /* split = [m/2] */
    MF_S32 split = (tree->order + 1) / 2;

    /* split as right sibling */
    right_node_add(tree, node, right);

    /* split key is key[split] */
    *split_key = key(node)[split];

    /* calculate split nodes' children (sum as (order + 1))*/
    MF_S32 pivot = insert - split - 1;
    node->children = split + 1;
    right->children = tree->order - split;

    /* sum = right->children = pivot + 2 + (_max_order - insert - 1) */
    /* replicate from key[split + 1] to key[insert] */
    memmove(&key(right)[0], &key(node)[split + 1], pivot * sizeof(struct bplus_key));
    memmove(&sub(right)[0], &sub(node)[split + 1], pivot * sizeof(off64_t));

    /* insert new key and sub-node */
    key(right)[pivot] = *key;
    sub_node_update(tree, right, pivot, l_ch);
    sub_node_update(tree, right, pivot + 1, r_ch);

    /* replicate from key[insert] to key[order - 1] */
    memmove(&key(right)[pivot + 1], &key(node)[insert], (tree->order - insert - 1) * sizeof(struct bplus_key));
    memmove(&sub(right)[pivot + 2], &sub(node)[insert + 1], (tree->order - insert - 1) * sizeof(off64_t));

    /* flush sub-nodes of the new splitted right node */
    for (i = 0; i < right->children; i++) {
        if (i != pivot && i != pivot + 1) {
            sub_node_flush(tree, right, sub(right)[i]);
        }
    }

}

static void
non_leaf_simple_insert(struct bplus_tree *tree, struct bplus_node *node,
                       struct bplus_node *l_ch, struct bplus_node *r_ch,
                       struct bplus_key *key, MF_S32 insert)
{
    memmove(&key(node)[insert + 1], &key(node)[insert], (node->children - 1 - insert) * sizeof(struct bplus_key));
    memmove(&sub(node)[insert + 2], &sub(node)[insert + 1], (node->children - 1 - insert) * sizeof(off64_t));
    /* insert new key and sub-nodes */
    key(node)[insert] = *key;
    sub_node_update(tree, node, insert, l_ch);
    sub_node_update(tree, node, insert + 1, r_ch);
    node->children++;
}

static MF_S32
non_leaf_insert(struct bplus_tree *tree, struct bplus_node *node,
                struct bplus_node *l_ch, struct bplus_node *r_ch, struct bplus_key *key)
{
    /* Search key location */
    MF_S32 insert = key_binary_search(node, key);
//    assert(insert < 0);
    if (insert >= 0) {
        return -1;
    }
    insert = -insert - 1;

    /* node is full */
    if (node->children == tree->order) {
        struct bplus_key split_key;
        /* split = [m/2] */
        MF_S32 split = (node->children + 1) / 2;
        struct bplus_node *sibling = non_leaf_new(tree);
        if (insert < split) {
            non_leaf_split_left(tree, node, sibling, l_ch, r_ch, key, insert, &split_key);
        } else if (insert == split) {
            non_leaf_split_right1(tree, node, sibling, l_ch, r_ch, key, insert, &split_key);
        } else {
            non_leaf_split_right2(tree, node, sibling, l_ch, r_ch, key, insert, &split_key);
        }
        /* build new parent */
        if (insert < split) {
            return parent_node_build(tree, sibling, node, &split_key);
        } else {
            return parent_node_build(tree, node, sibling, &split_key);
        }
    } else {
        non_leaf_simple_insert(tree, node, l_ch, r_ch, key, insert);
        node_flush(tree, node);
    }
    return 0;
}

static void
leaf_split_left(struct bplus_tree *tree, struct bplus_node *leaf,
                struct bplus_node *left, struct bplus_key *key, struct bplus_data *data,
                MF_S32 insert, struct bplus_key *split_key)
{
    /* split = [m/2] */
    MF_S32 split = (leaf->children + 1) / 2;

    /* split as left sibling */
    left_node_add(tree, leaf, left);

    /* calculate split leaves' children (sum as (entries + 1)) */
    MF_S32 pivot = insert;
    left->children = split;
    leaf->children = tree->entries - split + 1;

    /* sum = left->children = pivot + 1 + (split - pivot - 1) */
    /* replicate from key[0] to key[insert] */
    memmove(&key(left)[0], &key(leaf)[0], pivot * sizeof(struct bplus_key));
    memmove(&data(left)[0], &data(leaf)[0], pivot * sizeof(struct bplus_data));

    /* insert new key and data */
    key(left)[pivot] = *key;
    data(left)[pivot] = *data;

    /* replicate from key[insert] to key[split - 1] */
    memmove(&key(left)[pivot + 1], &key(leaf)[pivot], (split - pivot - 1) * sizeof(struct bplus_key));
    memmove(&data(left)[pivot + 1], &data(leaf)[pivot], (split - pivot - 1) * sizeof(struct bplus_data));

    /* original leaf left shift */
    memmove(&key(leaf)[0], &key(leaf)[split - 1], leaf->children * sizeof(struct bplus_key));
    memmove(&data(leaf)[0], &data(leaf)[split - 1], leaf->children * sizeof(struct bplus_data));

    *split_key = key(leaf)[0];

}

static void
leaf_split_right(struct bplus_tree *tree, struct bplus_node *leaf,
                 struct bplus_node *right, struct bplus_key *key, struct bplus_data *data,
                 MF_S32 insert, struct bplus_key *split_key)
{
    /* split = [m/2] */
    MF_S32 split = (leaf->children + 1) / 2;

    /* split as right sibling */
    right_node_add(tree, leaf, right);

    /* calculate split leaves' children (sum as (entries + 1)) */
    MF_S32 pivot = insert - split;
    leaf->children = split;
    right->children = tree->entries - split + 1;

    /* sum = right->children = pivot + 1 + (_max_entries - pivot - split) */
    /* replicate from key[split] to key[children - 1] in original leaf */
    memmove(&key(right)[0], &key(leaf)[split], pivot * sizeof(struct bplus_key));
    memmove(&data(right)[0], &data(leaf)[split], pivot * sizeof(struct bplus_data));

    /* insert new key and data */
    key(right)[pivot] = *key;
    data(right)[pivot] = *data;

    /* replicate from key[insert] to key[children - 1] in original leaf */
    memmove(&key(right)[pivot + 1], &key(leaf)[insert], (tree->entries - insert) * sizeof(struct bplus_key));
    memmove(&data(right)[pivot + 1], &data(leaf)[insert], (tree->entries - insert) * sizeof(struct bplus_data));

    *split_key = key(right)[0];
}

static void
leaf_simple_insert(struct bplus_tree *tree, struct bplus_node *leaf,
                   struct bplus_key *key, struct bplus_data *data, MF_S32 insert)
{
    memmove(&key(leaf)[insert + 1], &key(leaf)[insert], (leaf->children - insert) * sizeof(struct bplus_key));
    memmove(&data(leaf)[insert + 1], &data(leaf)[insert], (leaf->children - insert) * sizeof(struct bplus_data));
    key(leaf)[insert] = *key;
    data(leaf)[insert] = *data;
    leaf->children++;
}

static MF_S32
leaf_insert(struct bplus_tree *tree, struct bplus_node *leaf, struct bplus_key *key, struct bplus_data *data)
{
    /* Search key location */
    MF_S32 insert = key_binary_search(leaf, key);
    if (insert >= 0) {
        /* Already exists */
        return -1;
    }
    insert = -insert - 1;

    /* fetch from free node caches */
    MF_S32 i = ((char *) leaf - tree->caches) / tree->block_size;
    tree->used[i] = 1;

    /* leaf is full */
    if (leaf->children == tree->entries) {
        struct bplus_key split_key;
        /* split = [m/2] */
        MF_S32 split = (tree->entries + 1) / 2;
        struct bplus_node *sibling = leaf_new(tree);
        /* sibling leaf replication due to location of insertion */
        if (insert < split) {
            leaf_split_left(tree, leaf, sibling, key, data, insert, &split_key);
        } else {
            leaf_split_right(tree, leaf, sibling, key, data, insert, &split_key);
        }
        /* build new parent */
        if (insert < split) {
            return parent_node_build(tree, sibling, leaf, &split_key);
        } else {
            return parent_node_build(tree, leaf, sibling, &split_key);
        }
    } else {
        leaf_simple_insert(tree, leaf, key, data, insert);
        node_flush(tree, leaf);
    }

    return 0;
}

MF_S32
bplus_tree_insert(struct bplus_tree *tree, struct bplus_key *key, struct bplus_data *data)
{
    ms_mutex_lock(&tree->mutex);
    bplus_tree_update_begin(tree);

    struct bplus_node *node = node_seek(tree, tree->root);
    MF_S32 res = -1;

    if (node == NULL) {
        /* new root */
        struct bplus_node *root = leaf_new(tree);
        key(root)[0] = *key;
        data(root)[0] = *data;
        root->children = 1;
        tree->root = new_node_append(tree, root);
        tree->level = 1;
        node_flush(tree, root);
        tree->total++;
        res = 0;
    } else {
        while (node != NULL) {
            if (is_leaf(node)) {
                res = leaf_insert(tree, node, key, data);
                if (res == 0) {
                    tree->total++;
                    if (key->stRecord.condition == FILE_CONDITION_BAD) {
                        tree->fileBad++;
                    }
                }
                break;
            } else {
                MF_S32 i = key_binary_search(node, key);
                if (i >= 0) {
                    node = node_seek(tree, sub(node)[i + 1]);
                } else {
                    i = -i - 1;
                    node = node_seek(tree, sub(node)[i]);
                }
            }
        }
    }

    bplus_tree_update_end(tree);
    ms_mutex_unlock(&tree->mutex);
    return res;
}

static inline MF_S32
sibling_select(struct bplus_node *l_sib, struct bplus_node *r_sib,
               struct bplus_node *parent, MF_S32 i)
{
    if (i == -1) {
        /* the frist sub-node, no left sibling, choose the right one */
        return RIGHT_SIBLING;
    } else if (i == parent->children - 2) {
        /* the last sub-node, no right sibling, choose the left one */
        return LEFT_SIBLING;
    } else {
        /* if both left and right sibling found, choose the one with more children */
        return l_sib->children >= r_sib->children ? LEFT_SIBLING : RIGHT_SIBLING;
    }
}

static void
non_leaf_shift_from_left(struct bplus_tree *tree, struct bplus_node *node,
                         struct bplus_node *left, struct bplus_node *parent,
                         MF_S32 parent_key_index, MF_S32 remove)
{
    /* node's elements right shift */
    memmove(&key(node)[1], &key(node)[0], remove * sizeof(struct bplus_key));
    memmove(&sub(node)[1], &sub(node)[0], (remove + 1) * sizeof(off64_t));

    /* parent key right rotation */
    key(node)[0] = key(parent)[parent_key_index];
    key(parent)[parent_key_index] = key(left)[left->children - 2];

    /* borrow the last sub-node from left sibling */
    sub(node)[0] = sub(left)[left->children - 1];
    sub_node_flush(tree, node, sub(node)[0]);

    left->children--;
}

static void
non_leaf_merge_into_left(struct bplus_tree *tree, struct bplus_node *node,
                         struct bplus_node *left, struct bplus_node *parent,
                         MF_S32 parent_key_index, MF_S32 remove)
{
    /* move parent key down */
    key(left)[left->children - 1] = key(parent)[parent_key_index];

    /* merge into left sibling */
    /* key sum = node->children - 2 */
    memmove(&key(left)[left->children], &key(node)[0], remove * sizeof(struct bplus_key));
    memmove(&sub(left)[left->children], &sub(node)[0], (remove + 1) * sizeof(off64_t));

    /* sub-node sum = node->children - 1 */
    memmove(&key(left)[left->children + remove], &key(node)[remove + 1],
            (node->children - remove - 2) * sizeof(struct bplus_key));
    memmove(&sub(left)[left->children + remove + 1], &sub(node)[remove + 2],
            (node->children - remove - 2) * sizeof(off64_t));

    /* flush sub-nodes of the new merged left node */
    MF_S32 i, j;
    for (i = left->children, j = 0; j < node->children - 1; i++, j++) {
        sub_node_flush(tree, left, sub(left)[i]);
    }

    left->children += node->children - 1;

}

static void
non_leaf_shift_from_right(struct bplus_tree *tree, struct bplus_node *node,
                          struct bplus_node *right, struct bplus_node *parent,
                          MF_S32 parent_key_index)
{
    /* parent key left rotation */
    key(node)[node->children - 1] = key(parent)[parent_key_index];
    key(parent)[parent_key_index] = key(right)[0];

    /* borrow the frist sub-node from right sibling */
    sub(node)[node->children] = sub(right)[0];
    sub_node_flush(tree, node, sub(node)[node->children]);
    node->children++;

    /* right sibling left shift*/
    memmove(&key(right)[0], &key(right)[1], (right->children - 2) * sizeof(struct bplus_key));
    memmove(&sub(right)[0], &sub(right)[1], (right->children - 1) * sizeof(off64_t));

    right->children--;

}

static void
non_leaf_merge_from_right(struct bplus_tree *tree, struct bplus_node *node,
                          struct bplus_node *right, struct bplus_node *parent,
                          MF_S32 parent_key_index)
{
    /* move parent key down */
    key(node)[node->children - 1] = key(parent)[parent_key_index];
    node->children++;

    /* merge from right sibling */
    memmove(&key(node)[node->children - 1], &key(right)[0], (right->children - 1) * sizeof(struct bplus_key));
    memmove(&sub(node)[node->children - 1], &sub(right)[0], right->children * sizeof(off64_t));

    /* flush sub-nodes of the new merged node */
    MF_S32 i, j;
    for (i = node->children - 1, j = 0; j < right->children; i++, j++) {
        sub_node_flush(tree, node, sub(node)[i]);
    }

    node->children += right->children - 1;

}

static inline void
non_leaf_simple_remove(struct bplus_tree *tree, struct bplus_node *node, MF_S32 remove)
{
    assert(node->children >= 2);
    memmove(&key(node)[remove], &key(node)[remove + 1], (node->children - remove - 2) * sizeof(struct bplus_key));
    memmove(&sub(node)[remove + 1], &sub(node)[remove + 2], (node->children - remove - 2) * sizeof(off64_t));
    node->children--;
}

static void
non_leaf_remove(struct bplus_tree *tree, struct bplus_node *node, MF_S32 remove)
{
    if (node->parent == INVALID_OFFSET) {
        /* node is the root */
        if (node->children == 2) {
            /* replace old root with the first sub-node */
            struct bplus_node *root = node_fetch(tree, sub(node)[0]);
            root->parent = INVALID_OFFSET;
            tree->root = root->self;
            tree->level--;
            node_delete(tree, node, NULL, NULL);
            node_flush(tree, root);
        } else {
            non_leaf_simple_remove(tree, node, remove);
            node_flush(tree, node);
        }
    } else if (node->children <= (tree->order + 1) / 2) {
        struct bplus_node *l_sib = node_fetch(tree, node->prev);
        struct bplus_node *r_sib = node_fetch(tree, node->next);
        struct bplus_node *parent = node_fetch(tree, node->parent);

        MF_S32 i = parent_key_index(parent, &key(node)[0]);

        /* decide which sibling to be borrowed from */
        if (sibling_select(l_sib, r_sib, parent, i)  == LEFT_SIBLING) {
            if (l_sib->children > (tree->order + 1) / 2) {
                non_leaf_shift_from_left(tree, node, l_sib, parent, i, remove);
                /* flush nodes */
                node_flush(tree, node);
                node_flush(tree, l_sib);
                node_flush(tree, r_sib);
                node_flush(tree, parent);
            } else {
                non_leaf_merge_into_left(tree, node, l_sib, parent, i, remove);
                /* delete empty node and flush */
                node_delete(tree, node, l_sib, r_sib);
                /* trace upwards */
                non_leaf_remove(tree, parent, i);
            }
        } else {
            /* remove at first in case of overflow during merging with sibling */
            non_leaf_simple_remove(tree, node, remove);

            if (r_sib->children > (tree->order + 1) / 2) {
                non_leaf_shift_from_right(tree, node, r_sib, parent, i + 1);
                /* flush nodes */
                node_flush(tree, node);
                node_flush(tree, l_sib);
                node_flush(tree, r_sib);
                node_flush(tree, parent);
            } else {
                non_leaf_merge_from_right(tree, node, r_sib, parent, i + 1);
                /* delete empty right sibling and flush */
                struct bplus_node *rr_sib = node_fetch(tree, r_sib->next);
                node_delete(tree, r_sib, node, rr_sib);
                node_flush(tree, l_sib);
                /* trace upwards */
                non_leaf_remove(tree, parent, i + 1);
            }
        }
    } else {
        non_leaf_simple_remove(tree, node, remove);
        node_flush(tree, node);
    }
}

static void
leaf_shift_from_left(struct bplus_tree *tree, struct bplus_node *leaf,
                     struct bplus_node *left, struct bplus_node *parent,
                     MF_S32 parent_key_index, MF_S32 remove)
{
    /* right shift in leaf node */
    memmove(&key(leaf)[1], &key(leaf)[0], remove * sizeof(struct bplus_key));
    memmove(&data(leaf)[1], &data(leaf)[0], remove * sizeof(struct bplus_data));

    /* borrow the last element from left sibling */
    key(leaf)[0] = key(left)[left->children - 1];
    data(leaf)[0] = data(left)[left->children - 1];
    left->children--;

    /* update parent key */
    key(parent)[parent_key_index] = key(leaf)[0];
}

static void
leaf_merge_into_left(struct bplus_tree *tree, struct bplus_node *leaf,
                     struct bplus_node *left, MF_S32 parent_key_index, MF_S32 remove)
{
    /* merge into left sibling, sum = leaf->children - 1*/
    memmove(&key(left)[left->children], &key(leaf)[0], remove * sizeof(struct bplus_key));
    memmove(&data(left)[left->children], &data(leaf)[0], remove * sizeof(struct bplus_data));
    memmove(&key(left)[left->children + remove], &key(leaf)[remove + 1],
            (leaf->children - remove - 1) * sizeof(struct bplus_key));
    memmove(&data(left)[left->children + remove], &data(leaf)[remove + 1],
            (leaf->children - remove - 1) * sizeof(struct bplus_data));
    left->children += leaf->children - 1;
}

static void
leaf_shift_from_right(struct bplus_tree *tree, struct bplus_node *leaf,
                      struct bplus_node *right, struct bplus_node *parent,
                      MF_S32 parent_key_index)
{
    /* borrow the first element from right sibling */
    key(leaf)[leaf->children] = key(right)[0];
    data(leaf)[leaf->children] = data(right)[0];
    leaf->children++;

    /* left shift in right sibling */
    memmove(&key(right)[0], &key(right)[1], (right->children - 1) * sizeof(struct bplus_key));
    memmove(&data(right)[0], &data(right)[1], (right->children - 1) * sizeof(struct bplus_data));
    right->children--;

    /* update parent key */
    key(parent)[parent_key_index] = key(right)[0];
}

static void
leaf_merge_from_right(struct bplus_tree *tree, struct bplus_node *leaf,
                      struct bplus_node *right)
{
    memmove(&key(leaf)[leaf->children], &key(right)[0], right->children * sizeof(struct bplus_key));
    memmove(&data(leaf)[leaf->children], &data(right)[0], right->children * sizeof(struct bplus_data));
    leaf->children += right->children;
}

static inline void
leaf_simple_remove(struct bplus_tree *tree, struct bplus_node *leaf, MF_S32 remove)
{
    memmove(&key(leaf)[remove], &key(leaf)[remove + 1], (leaf->children - remove - 1) * sizeof(struct bplus_key));
    memmove(&data(leaf)[remove], &data(leaf)[remove + 1], (leaf->children - remove - 1) * sizeof(struct bplus_data));
    leaf->children--;
}

static MF_S32
leaf_remove(struct bplus_tree *tree, struct bplus_node *leaf, struct bplus_key *key)
{
    MF_S32 remove = key_binary_search(leaf, key);
    if (remove < 0) {
        /* Not exist */
        return -1;
    }

    /* fetch from free node caches */
    MF_S32 i = ((char *) leaf - tree->caches) / tree->block_size;
    tree->used[i] = 1;

    if (leaf->parent == INVALID_OFFSET) {
        /* leaf as the root */
        if (leaf->children == 1) {
            /* delete the only last node */
            assert(key_compare(key, '=', &key(leaf)[0]));
            tree->root = INVALID_OFFSET;
            tree->level = 0;
            node_delete(tree, leaf, NULL, NULL);
        } else {
            leaf_simple_remove(tree, leaf, remove);
            node_flush(tree, leaf);
        }
    } else if (leaf->children <= (tree->entries + 1) / 2) {
        struct bplus_node *l_sib = node_fetch(tree, leaf->prev);
        struct bplus_node *r_sib = node_fetch(tree, leaf->next);
        struct bplus_node *parent = node_fetch(tree, leaf->parent);

        i = parent_key_index(parent, &key(leaf)[0]);

        /* decide which sibling to be borrowed from */
        if (sibling_select(l_sib, r_sib, parent, i) == LEFT_SIBLING) {
            if (l_sib->children > (tree->entries + 1) / 2) {
                leaf_shift_from_left(tree, leaf, l_sib, parent, i, remove);
                /* flush leaves */
                node_flush(tree, leaf);
                node_flush(tree, l_sib);
                node_flush(tree, r_sib);
                node_flush(tree, parent);
            } else {
                leaf_merge_into_left(tree, leaf, l_sib, i, remove);
                /* delete empty leaf and flush */
                node_delete(tree, leaf, l_sib, r_sib);
                /* trace upwards */
                non_leaf_remove(tree, parent, i);
            }
        } else {
            /* remove at first in case of overflow during merging with sibling */
            leaf_simple_remove(tree, leaf, remove);

            if (r_sib->children > (tree->entries + 1) / 2) {
                leaf_shift_from_right(tree, leaf, r_sib, parent, i + 1);
                /* flush leaves */
                node_flush(tree, leaf);
                node_flush(tree, l_sib);
                node_flush(tree, r_sib);
                node_flush(tree, parent);
            } else {
                leaf_merge_from_right(tree, leaf, r_sib);
                /* delete empty right sibling flush */
                struct bplus_node *rr_sib = node_fetch(tree, r_sib->next);
                node_delete(tree, r_sib, leaf, rr_sib);
                node_flush(tree, l_sib);
                /* trace upwards */
                non_leaf_remove(tree, parent, i + 1);
            }
        }
    } else {
        leaf_simple_remove(tree, leaf, remove);
        node_flush(tree, leaf);
    }

    return 0;
}

MF_S32
bplus_tree_delete(struct bplus_tree *tree, struct bplus_key *key)
{
    ms_mutex_lock(&tree->mutex);
    struct bplus_node *node = node_seek(tree, tree->root);
    MF_S32 res = -1;

    bplus_tree_update_begin(tree);
    while (node != NULL) {
        if (is_leaf(node)) {
            res = leaf_remove(tree, node, key);
            if (res == 0) {
                tree->total--;
            }
            break;
        } else {
            MF_S32 i = key_binary_search(node, key);
            if (i >= 0) {
                node = node_seek(tree, sub(node)[i + 1]);
            } else {
                i = -i - 1;
                node = node_seek(tree, sub(node)[i]);
            }
        }
    }

    bplus_tree_update_end(tree);
    ms_mutex_unlock(&tree->mutex);

    return res;
}

struct bplus_node *
bplus_tree_fetch_node(struct bplus_tree *tree, off64_t offset)
{
    return node_fetch(tree, offset);
}

void
bplus_tree_release_node(struct bplus_tree *tree, struct bplus_node *node)
{
    cache_defer(tree, node);
}

struct bplus_key *
bplus_tree_node_key(struct bplus_node *node, MF_S32 index)
{
    return &key(node)[index];
}

MF_BOOL
bplus_tree_find(struct bplus_tree *tree,
                struct bplus_key *key, struct bplus_result *result)
{
    struct bplus_node *node = NULL;
    off64_t offset = INVALID_OFFSET;
    MF_S32 index = 0;

    ms_mutex_lock(&tree->mutex);
    index = bplus_tree_search(tree, key, &offset);
    if (offset != INVALID_OFFSET) {
        node = bplus_tree_fetch_node(tree, offset);
        if (node != NULL) {
            result->nodeSelf    = node->self;
            result->nodeParent  = node->parent;
            result->nodePrev    = node->prev;
            result->nodeNext    = node->next;
            result->keyCount    = node->children;
            result->keyIndex    = index;
            result->key         = key(node)[index];

            bplus_tree_release_node(tree, node);
            ms_mutex_unlock(&tree->mutex);
            return MF_YES;
        }
    }

    ms_mutex_unlock(&tree->mutex);
    return MF_NO;
}

MF_S32
bplus_tree_update_begin(struct bplus_tree *tree)
{
    return set_boot_state(tree, STATE_UPDATE);
}

MF_S32
bplus_tree_update_end(struct bplus_tree *tree)
{
    /* update boot in case power off */
    set_boot_info(tree);
    tree->isUpdate = MF_YES;

    return 0;
}

MF_S32
bplus_tree_update_crc(struct bplus_tree *tree, MF_U32 crc)
{
    return set_boot_crc(tree, crc);
}

MF_S32
bplus_tree_calc_crc(struct bplus_tree *tree, MF_S8 *data, MF_U32 size)
{
    MF_U32 NodeSize;
    MF_S32 NodeOff;
    MF_U32 crc = 0;

    if (tree == NULL) {
        NodeSize = size;
    } else {
        NodeSize = bplus_tree_node_size(tree);
    }

    NodeOff = size - NodeSize;
    if (NodeOff >= 0) {
        crc = crc_32(data + NodeOff, NodeSize);
    }
    return crc;
}

MF_BOOL
bplus_tree_backup(struct bplus_tree *tree)
{
    struct diskObj *disk = tree->owner;
    MF_BOOL bDone = MF_NO;

    ms_mutex_lock(&tree->mutex);

    if (tree->isUpdate == MF_YES) {
        if (disk->ops->disk_index_backup) {
            MFinfo("Start backup index");
            disk->ops->disk_index_backup(tree);
            bDone = MF_YES;
        }
        tree->isUpdate = MF_NO;
    }

    ms_mutex_unlock(&tree->mutex);

    return bDone;
}

MF_BOOL
bplus_tree_recover(struct bplus_tree *tree)
{
    struct diskObj *disk = tree->owner;
    MF_BOOL bDone = MF_NO;
    MF_S32 res;

    ms_mutex_lock(&tree->mutex);

    if (bplus_tree_is_valid(tree) == MF_NO) {
        if (disk->ops->disk_index_recover) {
            MFinfo("Start recover index");
            res = disk->ops->disk_index_recover(tree);
            if (res < 0) {
                MFerr("recover failed");
                bDone = MF_NO;
//                disk->enState = DISK_STATE_UNFORMATTED;
            } else {
                bDone = MF_YES;
            }
        }
    }

    ms_mutex_unlock(&tree->mutex);

    return bDone;
}

MF_S32
bplus_tree_read_from_disk(struct bplus_tree *tree, MF_S8 *out, MF_U32 size)
{
    struct diskObj *pstDisk = tree->owner;
    MF_S32 res = pstDisk->ops->disk_file_read(pstDisk->pstPrivate,
                                              tree->fd, out, size, tree->boot);

    if (res < 0) {
        MFerr("disk_read failed");
    }
    return res;
}

MF_U32
bplus_tree_file_size(struct bplus_tree *tree)
{
    return (MF_U32)(tree->file_size - tree->boot);
}

MF_U32
bplus_tree_node_size(struct bplus_tree *tree)
{
    if (tree->root == INVALID_OFFSET) {
        return 0;
    } else {
        return (MF_U32)(tree->file_size - tree->root);
    }
}

MF_BOOL
bplus_tree_is_valid(bplus_tree *tree)
{

    if (tree->block_count == 0
        || tree->root == INVALID_OFFSET
        || tree->level == 0) {
        MFerr("index is null");
        return MF_NO;
    }

    MF_U32 size = bplus_tree_file_size(tree);
    MF_S8 *out = ms_malloc(size);
    MF_U32 crc = 0;

    if (bplus_tree_read_from_disk(tree, out, size) < 0) {
        ms_free(out);
        MFerr("read tree failed");
        return MF_NO;
    }

    struct bplus_boot *boot = (struct bplus_boot *)out;

    crc = bplus_tree_calc_crc(tree, out, size);
    if (crc != boot->stInfo.crc) {
        MFerr("CRC is dismatch [%x][%x][%lld][%lld]", crc, boot->stInfo.crc, tree->file_size, tree->boot);
        ms_free(out);
        return MF_NO;
    }

    ms_free(out);
    return MF_YES;
}

struct bplus_tree *
bplus_tree_init(MF_S8 *filename, off64_t boot, void *owner, MF_BOOL isReset)
{
    MF_S32 fd;
    struct bplus_node node;
    struct bplus_tree *tree = ms_calloc(1, sizeof(*tree));

    assert(tree != NULL);
    tree->owner = owner;
    /* open data file */
    fd = disk_open(filename, MF_NO);
//    assert(fd >= 0);
    if (fd < 0) {
        ms_free(tree);
        return NULL;
    }

    tree->fd = fd;
    strcpy(tree->filename, filename);

    /* load index boot information */
    tree->boot = boot;
    if (get_boot_info(tree, isReset) != MF_SUCCESS) {
        disk_close(fd);
        ms_free(tree);
        return NULL;
    }

    ms_mutex_init(&tree->mutex);
    /* set order and entries */
    tree->order = (tree->block_size - sizeof(node)) / (sizeof(struct bplus_key) + sizeof(off64_t));
    tree->entries = (tree->block_size - sizeof(node)) / (sizeof(struct bplus_key) + sizeof(struct bplus_data));

    /* init free node caches */
    tree->caches = ms_malloc(tree->block_size * MIN_CACHE_NUM);

    return tree;
}

void
bplus_tree_deinit(struct bplus_tree *tree)
{
    struct list_head *pos = NULL;
    struct list_head *n = NULL;

    ms_mutex_lock(&tree->mutex);
    /* free free blocks */
    list_for_each_safe(pos, n, &tree->free_blocks) {
        list_del(pos);
        struct free_block *block = list_entry(pos, struct free_block, link);
        ms_free(block);
    }
    ms_mutex_unlock(&tree->mutex);

    ms_mutex_uninit(&tree->mutex);
    disk_close(tree->fd);
    /* free node caches */
    ms_free(tree->caches);
    ms_free(tree);
}

static void
node_dump(struct bplus_node *node, MF_S32 i)
{
    if (key(node)[i].enType == TYPE_RECORD) {
        MF_S8 stime[64] = {0};
        MFprint("\t(%d", i);
        MFprint("\t%u", key(node)[i].stRecord.condition);
        MFprint("\t%u", key(node)[i].stRecord.chnId);
        MFprint("\t%u", key(node)[i].stRecord.type);
        MFprint("\t%u", key(node)[i].stRecord.state);
        MFprint("-%d", key(node)[i].stRecord.segNumLsb + 
        (key(node)[i].stRecord.segNumMsb == 255 ? 0 : key(node)[i].stRecord.segNumMsb * 256));
        MFprint("-%#x", key(node)[i].stRecord.event);
        MFprint("-%#x", key(node)[i].stRecord.infoType);
        MFprint("-%d ", key(node)[i].stRecord.isLocked);
//        MFprint("\t%u", key(node)[i].stRecrod.reserve1);
//        MFprint("\t%d", key(node)[i].stRecrod.reserve2);
        MFprint("\t%u-%#llx", key(node)[i].stRecord.fileNo, key(node)[i].stRecord.fileOffset);
        time_to_string_local(key(node)[i].stRecord.startTime, stime);
        MFprint("\t%s", stime);
        time_to_string_local(key(node)[i].stRecord.endTime, stime);
        MFprint("\t%s", stime);
    } else if (key(node)[i].enType == TYPE_LOG) {
        // TODO:
    }
}

static void
draw(struct bplus_node *node, MF_S32 level)
{
    MF_S32 i;
    MFprint("level %d : ", level);
    MFprint("node(%#llx) ", node->self);
    if (node->next != INVALID_OFFSET) {
        MFprint("sibling(%#llx) ", node->next);
    } else {
        MFprint("sibling(none) ");
    }
    if (is_leaf(node)) {
        MFprint("totals(%d) sub(none)\n", node->children);
        MFprint("\tidx\tgood\tchn\ttype\tstate-segs-event-info-lock\tfileNo-offset\tStarTime\tEndTime\n");
        for (i = 0; i < node->children; i++) {
            node_dump(node, i);
            MFprint("\t--)\n");
        }
    } else {
        MFprint("totals(%d) sub(%#llx)\n", node->children - 1, sub(node)[0]);
        MFprint("\tidx\tgood\tchn\ttype\tstate-segs-event-info-lock\tfileNo-offset\tStarTime\tEndTime\tsubnode\n");
        for (i = 0; i < node->children - 1; i++) {
            node_dump(node, i);
            MFprint("\t%#llx)\n", sub(node)[i + 1]);
        }
    }
    MFprint("\n");

}

static void
bplus_tree_sub_dump(struct bplus_tree *tree, struct bplus_node *node, MF_S32 level)
{
    off64_t son = 0;

    if (!is_leaf(node)) {
        son = sub(node)[0];
    }

    while (node != NULL) {
        draw(node, level);
        node = node_seek(tree, node->next);
    }
    if (son) {
        node = node_seek(tree, son);
        bplus_tree_sub_dump(tree, node, level + 1);
    }
}

void
bplus_tree_dump(struct bplus_tree *tree)
{
    ms_mutex_lock(&tree->mutex);
    struct bplus_node *node = node_seek(tree, tree->root);
    if (node != NULL) {
        MFprint("\nindex  name          [%s]\n", tree->filename);
        MFprint("\nindex  version    [%s]\n", BPTREE_VER);
        MFprint("index root          [%#llx]\n", tree->root);
        MFprint("index level         [%d]\n", tree->level);
        MFprint("index total         [%d]\n", tree->total);
        MFprint("index fileBad       [%d]\n", tree->fileBad);
        MFprint("index blk_size      [%d]\n", tree->block_size);
        MFprint("index blk_count     [%d]\n", tree->block_count);
        MFprint("index file_size     [%d]\n", (int)(tree->file_size - tree->boot));
        bplus_tree_sub_dump(tree, node, 0);
    }
    ms_mutex_unlock(&tree->mutex);
}

