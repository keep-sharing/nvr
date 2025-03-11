/*
 * ***************************************************************
 * Filename:        utils.h
 * Created at:      2017.05.10
 * Description:     index api.
 * Author:          zbing
 * Copyright (C)    milesight
 * ***************************************************************
 */

#ifndef __UTILS_H__
#define __UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <time.h>
#include "mf_type.h"
#include "msdefs.h"

#if 0
#define LIST_INSERT_SORT_ASC(new, head, member)   \
    {   \
        typeof(*new) *prev; \
        typeof(*new) *next; \
        if (list_empty(head))\
        { \
            list_add(&new->node, head); \
        } \
        else \
        { \
            for (prev = list_entry((head)->prev, typeof(*new), node); \
                 &prev->node != (head);  prev = next) \
            { \
                if (prev->node.prev == head) \
                { \
                    if (prev->member <= new->member) \
                    { \
                        list_add_tail(&new->node, head); \
                    } \
                    else \
                    { \
                        list_add(&new->node, head); \
                    } \
                    break; \
                } \
                else \
                { \
                    next = list_entry(prev->node.prev, typeof(*new), node); \
                    if (prev->member > new->member && next->member <= new->member) \
                    { \
                        next->node.next = &new->node; \
                        new->node.prev = &next->node; \
                        new->node.next = &prev->node; \
                        prev->node.prev = &new->node; \
                        break; \
                    } \
                } \
            } \
        } \
    }
#else // reverse traversal comparison
#define LIST_INSERT_SORT_ASC(new, head, member) \
{ \
    typeof(*new) *prev; \
    if (list_empty(head)) { \
        list_add(&new->node, head); \
    } else { \
        prev = list_entry((head)->prev, typeof(*new), node); \
        if (prev->member <= new->member) { \
            list_add_tail(&new->node, head); \
        } else { \
            for (prev = list_entry((head)->prev, typeof(*new), node); \
                &prev->node != (head) && prev->member > new->member; \
                prev = list_entry(prev->node.prev, typeof(*new), node)); \
            list_add(&new->node, &prev->node); \
        } \
    } \
}
#endif
#define INIT_MF_TIME(T)    {(T)->startTime = INIT_START_TIME; (T)->endTime = INIT_END_TIME;}

typedef struct {
    MF_U32 count[2];
    MF_U32 state[4];
    MF_U8 buffer[64];
} MD5_CTX;

//////////// gpt head check //////////////
#define GPT_HEADER_SIGNATURE 0x5452415020494645ULL
typedef struct {
    unsigned char b[16];
} efi_guid_t;

typedef struct _gpt_header {
    unsigned char signature[8];
    unsigned char revision[4];
    unsigned char header_size[4];
    unsigned char header_crc32[4];
    unsigned char reserved1[4];
    unsigned char my_lba[8];
    unsigned char alternate_lba[8];
    unsigned char first_usable_lba[8];
    unsigned char last_usable_lba[8];
    efi_guid_t disk_guid;
    unsigned char partition_entry_lba[8];
    unsigned char num_partition_entries[4];
    unsigned char sizeof_partition_entry[4];
    unsigned char partition_entry_array_crc32[4];
    unsigned char reserved2[512 - 92];
} __attribute__((packed)) gpt_header;
////////////////

#define MD5_LEN   (40)
#define F(x,y,z) ((x & y) | (~x & z))
#define G(x,y,z) ((x & z) | (y & ~z))
#define H(x,y,z) (x^y^z)
#define I(x,y,z) (y ^ (x | ~z))
#define ROTATE_LEFT(x,n) ((x << n) | (x >> (32-n)))
#define FF(a,b,c,d,x,s,ac) \
    { \
        a += F(b,c,d) + x + ac; \
        a = ROTATE_LEFT(a,s); \
        a += b; \
    }
#define GG(a,b,c,d,x,s,ac) \
    { \
        a += G(b,c,d) + x + ac; \
        a = ROTATE_LEFT(a,s); \
        a += b; \
    }
#define HH(a,b,c,d,x,s,ac) \
    { \
        a += H(b,c,d) + x + ac; \
        a = ROTATE_LEFT(a,s); \
        a += b; \
    }
#define II(a,b,c,d,x,s,ac) \
    { \
        a += I(b,c,d) + x + ac; \
        a = ROTATE_LEFT(a,s); \
        a += b; \
    }

void
MD5string(MF_S8 *in, MF_U8 out[MD5_LEN]);
MF_S32
disk_open(const char *path, MF_BOOL bO_DIRECT);
MF_S32
disk_close(MF_S32 fd);
inline MF_S32
disk_read(MF_S32 fd, void *buf, MF_S32 count, off64_t offset);
inline MF_S32
disk_write(MF_S32 fd, const void *buf, MF_S32 count, off64_t offset);
MF_S32
disk_find_gpt_sig(MF_S8 *devname, MF_U32 *pSector);
MF_S32
disk_erase_MBR512(MF_S8 *devname);
inline MF_U32
crc_32(const void *buf, MF_U32 size);
MF_U32
check_sum(void *pData, MF_S32 size);
void
select_usleep(MF_U32 us);
MF_U64 inline
get_time_us();
MF_U32 inline
get_time_ms();
void
time_to_string_utc(MF_PTS ntime, MF_S8 stime[32]);
void
time_to_string_local(MF_PTS ntime, MF_S8 stime[32]);
MF_PTS
string_to_time_local(const MF_S8 stime[32]);
MF_S8 *
mf_trim(MF_S8 *str);
MF_S32
mf_cond_init(MF_COND_T *cond);
MF_S32
mf_cond_uninit(MF_COND_T *cond);
MF_S32
mf_cond_wait(MF_COND_T *cond, MF_MUTEX_T *mutex);
MF_S32
mf_cond_timedwait(MF_COND_T *cond, MF_MUTEX_T *mutex, MF_U64 us);
MF_S32
mf_cond_signal(MF_COND_T *cond);
MF_S8 *
mf_time_to_string(MF_PTS time);
void
mf_time_to_string2(MF_PTS time, MF_S8 *buff);
MF_U32
mf_time_second_to_day(MF_PTS second);
MF_U32
mf_date_to_second(MF_U32 year, MF_U32 month);
MF_U32
mf_date_to_day(MF_U32 year, MF_U32 month);
MF_S32
mf_random(MF_S32 min, MF_S32 max);
MF_S32
mf_random2(MF_S32 min, MF_S32 max, MF_U32 seed);
MF_S32
mf_set_task_policy(TASK_HANDLE thread_id, MF_S32 priority);

#ifdef __cplusplus
}
#endif

#endif

