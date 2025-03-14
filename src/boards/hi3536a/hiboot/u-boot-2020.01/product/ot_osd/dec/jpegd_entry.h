// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */
#ifndef __JPEGD_ENTRY_H__
#define __JPEGD_ENTRY_H__

#ifdef __cplusplus
extern "C" {
#endif /* End of #ifdef __cplusplus */
#define ENV_BUF_LEN 32
unsigned long get_ot_logo(void);
unsigned long get_jpeg_size_val(void);
unsigned long get_video_data_base(void);
unsigned long get_jpegd_emar_buf(void);
unsigned int get_output_format(void);

#ifdef __cplusplus
}
#endif /* End of #ifdef __cplusplus */

#endif /* _JPEGD_ENTRY_H_ */
