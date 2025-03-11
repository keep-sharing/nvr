/**
	@brief Header file of debug function.\n
	This file contains the debug function, and debug menu entry point.

	@file hd_debug.h

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#ifndef _HD_DEBUG_H_
#define _HD_DEBUG_H_
#ifdef __cplusplus
extern  "C" {
#endif

#include "hd_type.h"

typedef enum _HD_DEBUG_PARAM_ID {
	///@{
	/**
	 * To set or get the mask of each message type for enable or disable
	 * Refer hd_logger.h to get the description of message type.
	 * @param[in] A pointer as unsigned int, each bit present
	 *  a certain module that defined by HD_LOG_MASK_ on hd_logger.h
	 */
	HD_DEBUG_PARAM_ERR_MASK, ///< the mask of ERR message type
	HD_DEBUG_PARAM_WRN_MASK, ///< the mask of WRN message type
	HD_DEBUG_PARAM_IND_MASK, ///< the mask of IND message type
	HD_DEBUG_PARAM_MSG_MASK, ///< the mask of MSG message type
	HD_DEBUG_PARAM_FUNC_MASK, ///< the mask of FUNC message type
	///@}
	HD_DEBUG_PARAM_MAX, ///< total idx count
	ENUM_DUMMY4WORD(HD_DEBUG_PARAM_ID)
} HD_DEBUG_PARAM_ID;

/**
 *  Initiate the requirement of debug, such as open a file handle
 *  to save the log to file.
 */
HD_RESULT hd_debug_init(void);

/**
* A universal interface to get data from hd_debug
* @param[in] idx refers HD_DEBUG_PARAM_ID
* @param[out] A pointer that address to returned data.
*  The pointer type depends on what HD_DEBUG_PARAM_ID get to.
*  The has defined on description of HD_DEBUG_PARAM_ID.
*/
HD_RESULT hd_debug_get(HD_DEBUG_PARAM_ID idx, void *p_data);

/**
* A universal interface to set data into hd_debug
* @param[in] idx refers HD_DEBUG_PARAM_ID
* @param[in] A pointer that address to returned data.
*  The pointer type depends on what HD_DEBUG_PARAM_ID set to.
*  The has defined on description of HD_DEBUG_PARAM_ID.
*/
HD_RESULT hd_debug_set(HD_DEBUG_PARAM_ID idx, void *p_data);

/**
 *  Un-initiate the debug. There is nothing to do in it, currently.
 */
HD_RESULT hd_debug_uninit(void);

/**
 *  Display a interactive menu on console and can,
 *  1. Enable or disable message mask of each module.
 *  2. Dump module's information.
 *  3. Run module's test flow.
 */
HD_RESULT hd_debug_run_menu(void);

#ifdef __cplusplus
}
#endif
#endif

