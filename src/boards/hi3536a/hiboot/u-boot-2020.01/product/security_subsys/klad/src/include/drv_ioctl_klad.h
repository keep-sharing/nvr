// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef DRV_IOCTL_KLAD_H
#define DRV_IOCTL_KLAD_H

#include <asm-generic/ioctl.h>
#include "ot_common_klad.h"
#include "ot_common.h"
#include "mkp_ioctl.h"
#include "dev_ext.h"

typedef struct {
    td_handle klad;
} klad_ctl_handle;

typedef struct {
    td_handle klad;
    td_handle target;
} klad_ctl_target;

typedef struct {
    td_handle klad;
    ot_klad_attr attr;
} klad_ctl_attr;

typedef struct {
    td_handle klad;
    ot_klad_session_key key;
} klad_ctl_session_key;

typedef struct {
    td_handle klad;
    ot_klad_content_key key;
} klad_ctl_content_key;

typedef struct {
    td_handle klad;
    ot_klad_clear_key key;
} klad_ctl_clear_key;

typedef enum {
    KLAD_IOC_NR_CREATE = 0,
    KLAD_IOC_NR_DESTROY,
    KLAD_IOC_NR_ATTACH,
    KLAD_IOC_NR_DETACH,
    KLAD_IOC_NR_SET_ATTR,
    KLAD_IOC_NR_GET_ATTR,
    KLAD_IOC_NR_SESSION_KEY,
    KLAD_IOC_NR_CONTENT_KEY,
    KLAD_IOC_NR_CLEAR_KEY,
    KLAD_IOC_NR_BUTT,
} klad_ioc_nr;

#define CMD_KLAD_CREATE_HANDLE      _IOWR(IOC_TYPE_KLAD, KLAD_IOC_NR_CREATE,      klad_ctl_handle)
#define CMD_KLAD_DESTROY_HANDLE     _IOW (IOC_TYPE_KLAD, KLAD_IOC_NR_DESTROY,     klad_ctl_handle)
#define CMD_KLAD_ATTACH_TARGET      _IOW (IOC_TYPE_KLAD, KLAD_IOC_NR_ATTACH,      klad_ctl_target)
#define CMD_KLAD_DETACH_TARGET      _IOW (IOC_TYPE_KLAD, KLAD_IOC_NR_DETACH,      klad_ctl_target)
#define CMD_KLAD_SET_ATTR           _IOW (IOC_TYPE_KLAD, KLAD_IOC_NR_SET_ATTR,    klad_ctl_attr)
#define CMD_KLAD_GET_ATTR           _IOWR(IOC_TYPE_KLAD, KLAD_IOC_NR_GET_ATTR,    klad_ctl_attr)
#define CMD_KLAD_SESSION_KEY        _IOW (IOC_TYPE_KLAD, KLAD_IOC_NR_SESSION_KEY, klad_ctl_session_key)
#define CMD_KLAD_CONTENT_KEY        _IOW (IOC_TYPE_KLAD, KLAD_IOC_NR_CONTENT_KEY, klad_ctl_content_key)
#define CMD_KLAD_CLEAR_KEY          _IOW (IOC_TYPE_KLAD, KLAD_IOC_NR_CLEAR_KEY,   klad_ctl_clear_key)
#define CMD_KLAD_MAX_NUM            KLAD_IOC_NR_BUTT

#endif /* DRV_IOCTL_KLAD_H */
