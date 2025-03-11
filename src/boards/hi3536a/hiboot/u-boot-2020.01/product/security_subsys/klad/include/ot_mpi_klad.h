// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef OT_MPI_KLAD_H
#define OT_MPI_KLAD_H

#include "ot_common_klad.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * brief This API is used to init the klad device.
 * param N/A
 * retval ::TD_SUCCESS      Call this API successful.
 * retval ::Others          The klad device fails to be initialized, See the klad errno.
 */
td_s32 ot_mpi_klad_init(td_void);

/*
 * brief This API is used to deinit the klad device.
 * param N/A
 * retval ::TD_SUCCESS      Call this API successful.
 * retval ::Others          The klad device fails to be Deinitialized, See the klad errno.
 */
td_s32 ot_mpi_klad_deinit(td_void);

/*
 * brief This API is used to create the klad handle.
 * param[in] klad           Handle of key ladder.
 * retval ::TD_SUCCESS      Call this API successful.
 * retval ::Others          The klad handle fails to be created, See the klad errno.
 */
td_s32 ot_mpi_klad_create(td_handle *klad);

/*
 * brief This API is used to destroy the klad handle.
 * param[in] klad           Handle of key ladder.
 * retval ::TD_SUCCESS      Call this API successful.
 * retval ::Others          The klad handle fails to be destroyed, See the klad errno.
 */
td_s32 ot_mpi_klad_destroy(td_handle klad);

/*
 * brief This API is used to attach the klad handle & the keyslot handle
 * param[in] klad           Handle of key ladder.
 * param[in] target         Handle of keyslot.
 * retval ::TD_SUCCESS      Call this API successful.
 * retval ::Others          The klad handle and the keyslot handle fail to be attached, See the klad errno.
 */
td_s32 ot_mpi_klad_attach(td_handle klad, td_handle target);

/*
 * brief This API is used to detach the klad handle & the keyslot handle
 * param[in] klad           Handle of key ladder.
 * param[in] target         Handle of keyslot.
 * retval ::TD_SUCCESS      Call this API successful.
 * retval ::Others          The klad handle and the keyslot handle fail to be detached, See the klad errno.
 */
td_s32 ot_mpi_klad_detach(td_handle klad, td_handle target);

/*
 * brief This API is used to set the attributes of a key ladder.
 * param[in] klad           Handle of key ladder.
 * param[in] attr           Pointer to the attributes of a key ladder.
 * retval ::TD_SUCCESS      Call this API successful.
 * retval ::Others          The klad attributes fails to be set, See the klad errno.
 */
td_s32 ot_mpi_klad_set_attr(td_handle klad, const ot_klad_attr *attr);

/*
 * brief This API is used to get the attributes of a key ladder.
 * param[in] klad           Handle of key ladder.
 * param[out] attr          Pointer to the attributes of a key ladder.
 * retval ::TD_SUCCESS      Call this API successful.
 * retval ::Others          The klad attributes fails to be obtained, See the klad errno.
 */
td_s32 ot_mpi_klad_get_attr(td_handle klad, ot_klad_attr *attr);

/*
 * brief This API is used to set 1~n-1 stage common route klad
 * param[in] klad           Handle of key ladder.
 * param[in] key          Pointer to the session key
 * retval ::TD_SUCCESS      Call this API successful.
 * retval ::Others          The session key fails to be set, See the klad errno.
 */
td_s32 ot_mpi_klad_set_session_key(td_handle klad, const ot_klad_session_key *key);

/*
 * brief This API is used to set n stage common route klad
 * param[in] klad           Handle of key ladder.
 * param[in] key            Pointer to the content key
 * retval ::TD_SUCCESS      Call this API successful.
 * retval ::Others          The content key fails to be set, See the klad errno.
 */
td_s32 ot_mpi_klad_set_content_key(td_handle klad, const ot_klad_content_key *key);

/*
 * brief This API is used to set clear route klad
 * param[in] klad           Handle of key ladder.
 * param[in] key            Pointer to the clear key
 * retval ::TD_SUCCESS      Call this API successful.
 * retval ::Others          The clear key fails to be set, See the klad errno.
 */
td_s32 ot_mpi_klad_set_clear_key(td_handle klad, const ot_klad_clear_key *key);

#ifdef __cplusplus
}
#endif

#endif /* OT_MPI_KLAD_H */

