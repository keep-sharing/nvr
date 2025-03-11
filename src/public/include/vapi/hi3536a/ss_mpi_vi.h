/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#ifndef __OT_MPI_VI_H__
#define __OT_MPI_VI_H__

#include "ot_common_vi.h"
#include "ot_common_vb.h"

#ifdef __cplusplus
extern "C" {
#endif

td_s32 ss_mpi_vi_set_dev_attr(ot_vi_dev vi_dev, const ot_vi_dev_attr *dev_attr);
td_s32 ss_mpi_vi_get_dev_attr(ot_vi_dev vi_dev, ot_vi_dev_attr *dev_attr);

td_s32 ss_mpi_vi_enable_dev(ot_vi_dev vi_dev);
td_s32 ss_mpi_vi_disable_dev(ot_vi_dev vi_dev);

td_s32 ss_mpi_vi_set_chn_attr(ot_vi_chn vi_chn, const ot_vi_chn_attr *attr);
td_s32 ss_mpi_vi_get_chn_attr(ot_vi_chn vi_chn, ot_vi_chn_attr *attr);

td_s32 ss_mpi_vi_set_minor_chn_attr(ot_vi_chn vi_chn, const ot_vi_chn_attr *attr);
td_s32 ss_mpi_vi_get_minor_chn_attr(ot_vi_chn vi_chn, ot_vi_chn_attr *attr);

td_s32 ss_mpi_vi_enable_chn(ot_vi_chn vi_chn);
td_s32 ss_mpi_vi_disable_chn(ot_vi_chn vi_chn);

td_s32 ss_mpi_vi_get_frame(ot_vi_chn vi_chn, ot_video_frame_info *frame_info, td_s32 milli_sec);
td_s32 ss_mpi_vi_release_frame(ot_vi_chn vi_chn, const ot_video_frame_info *frame_info);
td_s32 ss_mpi_vi_set_frame_depth(ot_vi_chn vi_chn, td_u32 depth);
td_s32 ss_mpi_vi_get_frame_depth(ot_vi_chn vi_chn, td_u32 *depth);

td_s32 ss_mpi_vi_set_user_pic(ot_vi_chn vi_chn, const ot_vi_user_pic_attr *user_pic);
td_s32 ss_mpi_vi_enable_user_pic(ot_vi_chn vi_chn);
td_s32 ss_mpi_vi_disable_user_pic(ot_vi_chn vi_chn);

td_s32 ss_mpi_vi_set_vbi_attr(ot_vi_chn vi_chn, const ot_vi_vbi_arg *vbi_attr);
td_s32 ss_mpi_vi_get_vbi_attr(ot_vi_chn vi_chn, ot_vi_vbi_arg *vbi_attr);
td_s32 ss_mpi_vi_set_vbi_mode(ot_vi_chn vi_chn, const ot_vi_vbi_mode mode);
td_s32 ss_mpi_vi_get_vbi_mode(ot_vi_chn vi_chn, ot_vi_vbi_mode *mode);
td_s32 ss_mpi_vi_enable_vbi(ot_vi_chn vi_chn);
td_s32 ss_mpi_vi_disable_vbi(ot_vi_chn vi_chn);
td_s32 ss_mpi_vi_enable_cas_chn(ot_vi_chn vi_chn);
td_s32 ss_mpi_vi_disable_cas_chn(ot_vi_chn vi_chn);

td_s32 ss_mpi_vi_bind_chn(ot_vi_chn vi_chn, const ot_vi_chn_bind_attr *chn_bind_attr);
td_s32 ss_mpi_vi_unbind_chn(ot_vi_chn vi_chn);
td_s32 ss_mpi_vi_get_chn_bind(ot_vi_chn vi_chn, ot_vi_chn_bind_attr *chn_bind_attr);

td_s32 ss_mpi_vi_set_dev_attr_ex(ot_vi_dev vi_dev, const ot_vi_dev_attr_ex *dev_attr_ex);
td_s32 ss_mpi_vi_get_dev_attr_ex(ot_vi_dev vi_dev, ot_vi_dev_attr_ex *dev_attr_ex);

td_s32 ss_mpi_vi_get_fd(ot_vi_chn vi_chn);
td_s32 ss_mpi_vi_close_fd(td_void);

td_s32 ss_mpi_vi_query(ot_vi_chn vi_chn, ot_vi_chn_status *status);

td_s32 ss_mpi_vi_enable_chn_interrupt(ot_vi_chn vi_chn);
td_s32 ss_mpi_vi_disable_chn_interrupt(ot_vi_chn vi_chn);

td_s32 ss_mpi_vi_get_chn_luma(ot_vi_chn vi_chn, ot_vi_chn_luma *luma);

td_s32 ss_mpi_vi_set_skip_mode(ot_vi_chn vi_chn, ot_vi_skip_mode skip_mode);
td_s32 ss_mpi_vi_get_skip_mode(ot_vi_chn vi_chn, ot_vi_skip_mode *skip_mode);

td_s32 ss_mpi_vi_enable_dll_slave(ot_vi_dev vi_dev);
td_s32 ss_mpi_vi_disable_dll_slave(ot_vi_dev vi_dev);

td_s32 ss_mpi_vi_set_skip_mode_ex(ot_vi_chn vi_chn, const ot_vi_skip_mode_ex *mode_ex);
td_s32 ss_mpi_vi_get_skip_mode_ex(ot_vi_chn vi_chn, ot_vi_skip_mode_ex *mode_ex);

td_s32 ss_mpi_vi_attach_vb_pool(ot_vi_chn vi_chn, ot_vb_pool pool);
td_s32 ss_mpi_vi_detach_vb_pool(ot_vi_chn vi_chn);
td_s32 ss_mpi_vi_detach_vb_pool_by_pool_id(ot_vi_chn vi_chn, ot_vb_pool pool);

td_s32 ss_mpi_vi_set_chn_vb_src(ot_vi_chn vi_chn, ot_vb_src vb_src);
td_s32 ss_mpi_vi_get_chn_vb_src(ot_vi_chn vi_chn, ot_vb_src *vb_src);

td_s32 ss_mpi_vi_set_mod_param(const ot_vi_mod_param *mod_param);
td_s32 ss_mpi_vi_get_mod_param(ot_vi_mod_param *mod_param);

td_s32 ss_mpi_vi_set_rotation(ot_vi_chn vi_chn, const ot_rotation rotation);
td_s32 ss_mpi_vi_get_rotation(ot_vi_chn vi_chn, ot_rotation *rotation);

td_s32 ss_mpi_vi_set_frame_interrupt_attr(ot_vi_chn vi_chn, const ot_frame_interrupt_attr *frame_interrupt_attr);
td_s32 ss_mpi_vi_get_frame_interrupt_attr(ot_vi_chn vi_chn, ot_frame_interrupt_attr *frame_interrupt_attr);

#ifdef __cplusplus
}
#endif

#endif
