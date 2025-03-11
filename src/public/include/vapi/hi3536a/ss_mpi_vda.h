/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#ifndef __SS_MPI_VDA_H__
#define __SS_MPI_VDA_H__

#include "ot_common_vda.h"

#ifdef __cplusplus
extern "C" {
#endif

td_s32 ss_mpi_vda_create_chn(ot_vda_chn vda_chn, const ot_vda_chn_attr *attr);
td_s32 ss_mpi_vda_destroy_chn(ot_vda_chn vda_chn);

td_s32 ss_mpi_vda_get_chn_attr(ot_vda_chn vda_chn, ot_vda_chn_attr *attr);
td_s32 ss_mpi_vda_set_chn_attr(ot_vda_chn vda_chn, const ot_vda_chn_attr *attr);

td_s32 ss_mpi_vda_start_recv_pic(ot_vda_chn vda_chn);
td_s32 ss_mpi_vda_stop_recv_pic(ot_vda_chn vda_chn);

td_s32 ss_mpi_vda_get_data(ot_vda_chn vda_chn, ot_vda_data *vda_data, td_s32 milli_sec);
td_s32 ss_mpi_vda_release_data(ot_vda_chn vda_chn, const ot_vda_data *vda_data);

td_s32 ss_mpi_vda_reset_od_rgn(ot_vda_chn vda_chn, td_s32 rgn_idx);

td_s32 ss_mpi_vda_query(ot_vda_chn vda_chn, ot_vda_chn_status *chn_status);

td_s32 ss_mpi_vda_get_fd(ot_vda_chn vda_chn);

td_s32 ss_mpi_vda_update_ref(ot_vda_chn vda_chn, const ot_video_frame_info *ref_frame);

td_s32 ss_mpi_vda_send_pic(ot_vda_chn vda_chn, const ot_video_frame_info *user_frame, td_s32 milli_sec);

#ifdef __cplusplus
}
#endif

#endif

