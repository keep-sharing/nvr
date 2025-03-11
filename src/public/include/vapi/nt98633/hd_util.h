/**
	@brief Header file of utility.

	@file hd_util.h

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#ifndef	_HD_UTIL_H_
#define	_HD_UTIL_H_

/********************************************************************
	INCLUDE FILES
********************************************************************/
#include "hd_type.h"

/********************************************************************
	MACRO CONSTANT DEFINITIONS
********************************************************************/

/********************************************************************
	MACRO FUNCTION DEFINITIONS
********************************************************************/

/********************************************************************
	TYPE DEFINITION
********************************************************************/

/**
 * @brief Decimal Number Input Function.
 * @remarks
 * @code
 * @endcode
 * @return UINT32
 * @see
 */
UINT32 hd_read_decimal_key_input(const CHAR *comment);

/**
 * @brief It gets the current hardware time with 32 bit ms.
 * @remarks
 * @code
 * @endcode
 * @return UINT32
 * @see
 */
UINT32 hd_gettime_ms(VOID);

/**
 * @brief It gets the current hardware time with 64 bit us.
 * @remarks
 * @code
 * @endcode
 * @return UINT64
 * @see
 */
UINT64 hd_gettime_us(VOID);

#endif

