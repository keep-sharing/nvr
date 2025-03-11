// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef __CIPHER_EXT_H__
#define __CIPHER_EXT_H__

typedef enum {
    OT_PRIVATE_ID_SYMC,
    OT_PRIVATE_ID_HASH,
    OT_PRIVATE_ID_KEYSLOT,
    OT_PRIVATE_ID_KLAD,
    OT_PRIVATE_ID_BUTT,
} ot_private_id;

/* -------------------------------------------------------------------------------------------------------------
 * Definition of basic data types. The data types are applicable to both the application layer and kernel codes.
 * -------------------------------------------------------------------------------------------------------------
 *
 * define of TD_HANDLE :
 * bit31                                                                bit0
 *   |<----- 8bit ----->|<----- 8bit ----->|<---------- 16bit ---------->|
 *   |-------------------------------------------------------------------|
 *   |     ot_mod_id    | mod defined data |           chan_id           |
 *   |-------------------------------------------------------------------|
 * mod defined data: private data define by each module(for example: sub-mod id), usually, set to 0.
 */
#define td_handle_init(mod, private_data, chnid) \
    (td_handle)((((mod) & 0xff) << 24) | ((((private_data) & 0xff) << 16)) | (((chnid) & 0xffff)))

#define td_handle_get_modid(handle)         (((handle) >> 24) & 0xff)

#define td_handle_get_private_data(handle)  (((handle) >> 16) & 0xff)

#define td_handle_get_chnid(handle)         (((handle)) & 0xffff)

#endif
