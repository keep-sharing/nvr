// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef __OT_DEBUG_H__
#define __OT_DEBUG_H__

#include "ot_type.h"
#include "common.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define OT_DBG_EMERG      0   /* system is unusable                   */
#define OT_DBG_ALERT      1   /* action must be taken immediately     */
#define OT_DBG_CRIT       2   /* critical conditions                  */
#define OT_DBG_ERR        3   /* error conditions                     */
#define OT_DBG_WARN       4   /* warning conditions                   */
#define OT_DBG_NOTICE     5   /* normal but significant condition     */
#define OT_DBG_INFO       6   /* informational                        */
#define OT_DBG_DEBUG      7   /* debug-level messages                 */

#define OT_PRINT      printf

#define OT_ASSERT(expr)
#define OT_TRACE(level, enModId, fmt...)

#define OT_ERR_TRACE(mod, fmt...) printf(fmt)
#define OT_WARN_TRACE(mod, fmt...)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* __OT_DEBUG_H__ */
