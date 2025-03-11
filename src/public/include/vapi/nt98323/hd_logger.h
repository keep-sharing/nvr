/**
	@brief Header file of directive printf function to difference interfaces.

	@file hd_logger.h

	@ingroup mhdal

	@note Nothing.

	Here provides several types message for modules, list as below,
	_ERR: error message including prompt names of module, function and line number. (default: on)
	_WRN: warning message including prompt names of module, function and line number. (default: on)
	_IND: verbose message including prompt names of module, function and line number. (default: off)
	_MSG: hints message such as a menu list not including prompt names of module, function and line number. (default: on)
	_FUNC: only use to trace at the begging and the end of function. (default: off)

	To simply the writing codes, we suggest using an internal header for a module
	and make a copy of definition as following,
	@code
		#define HD_MODULE_NAME HD_AUDIOCAPTURE
		#define DBG_ERR(fmtstr, args...) HD_LOG_BIND(HD_MODULE_NAME, _ERR)("\033[1;31m" fmtstr "\033[0m", ##args)
		#define DBG_WRN(fmtstr, args...) HD_LOG_BIND(HD_MODULE_NAME, _WRN)("\033[1;33m" fmtstr "\033[0m", ##args)
		#define DBG_IND(fmtstr, args...) HD_LOG_BIND(HD_MODULE_NAME, _IND)(fmtstr, ##args)
		#define DBG_DUMP(fmtstr, args...) HD_LOG_BIND(HD_MODULE_NAME, _MSG)(fmtstr, ##args)
		#define DBG_FUNC_BEGIN(fmtstr, args...) HD_LOG_BIND(HD_MODULE_NAME, _FUNC)("BEGIN: " fmtstr, ##args)
		#define DBG_FUNC_END(fmtstr, args...) HD_LOG_BIND(HD_MODULE_NAME, _FUNC)("END: " fmtstr, ##args)
	@endcode

	After making a copy, just replace HD_AUDIOCAPTURE with another module name defined in HD_LOG_MASK_.
	e.g.: HD_AUDIOOUT

	Following is an example to explain how to use the macros as stated above,
	@code
		int foo(void)
		{
			DBG_FUNC_BEGIN("with init val=%d\n", 0);
			DBG_ERR("this is error message, er=%d\n", -1);
			DBG_WRN("this is warning message, wr=%d\n", -2);
			DBG_IND("this is debug message, got val=%d\n", 3);
			DBG_DUMP("this is normal message without function name, module name and line number.\n");
			DBG_FUNC_END("with end val=%d\n", 99);
			return 0;
		}
	@endcode

	The result shows as below,
		--- [AUDIOCAPTURE][foo()] BEGIN: with init val=0
	ERR: [AUDIOCAPTURE][foo():156]: this is error message, er=-1 (with red color)
	WRN: [AUDIOCAPTURE][foo():157]: this is warning message, wr=-2 (with yellow color)
	IND: [AUDIOCAPTURE][foo():158]: this is debug message, got val=3
	this is normal message without function name, module name and line number.
	    --- [AUDIOCAPTURE][foo()] END: with end val=99


	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#ifndef _HD_LOGGER_H_
#define _HD_LOGGER_H_
#ifdef __cplusplus
extern  "C" {
#endif

#include <stdio.h>

/**
 *  Output a formatted message
 *
 *  All of module's debug message output via HD_PRINTF. User can replace
 *  HD_PRINTF with owns one. For example is to save messages into a file,
 *  only just implement the body of user_printf and define PRINT_FUNC_USER ahead.
 */
#if defined(PRINT_FUNC_USER)
/* simple printf replacement */
extern void user_printf(const char *msg_with_format, ...) __attribute__((format(printf, 1, 2)));
#define HD_PRINTF(fmt...) user_printf(fmt)
#else
extern void hd_printf(const char *msg_with_format, ...) __attribute__((format(printf, 1, 2)));
#define HD_PRINTF(fmt...) hd_printf(fmt)
#endif // defined(PRINT_FUNC_USER)

/**
 *  Internal macro to make the bit on mask to indicate a module.
 *  Don't use it outside.
 */
#define _HD_LOG_MASK_BIT(n) ((unsigned int)(1 << (n)))

/**
 *  Definitions of the masks for each module
 */
#define HD_LOG_MASK_AUDIOCAPTURE   _HD_LOG_MASK_BIT(0)
#define HD_LOG_MASK_AUDIOOUT       _HD_LOG_MASK_BIT(1)
#define HD_LOG_MASK_AUDIOENC       _HD_LOG_MASK_BIT(2)
#define HD_LOG_MASK_AUDIODEC       _HD_LOG_MASK_BIT(3)
#define HD_LOG_MASK_VIDEOCAPTURE   _HD_LOG_MASK_BIT(4)
#define HD_LOG_MASK_VIDEOOUT       _HD_LOG_MASK_BIT(5)
#define HD_LOG_MASK_VIDEOPROCESS   _HD_LOG_MASK_BIT(6)
#define HD_LOG_MASK_VIDEOENC       _HD_LOG_MASK_BIT(7)
#define HD_LOG_MASK_VIDEODEC       _HD_LOG_MASK_BIT(8)
#define HD_LOG_MASK_GFX            _HD_LOG_MASK_BIT(9)
#define HD_LOG_MASK_COMMON         _HD_LOG_MASK_BIT(10)
#define HD_LOG_MASK_UTIL           _HD_LOG_MASK_BIT(11)
#define HD_LOG_MASK_DEBUG          _HD_LOG_MASK_BIT(12)

/**
 *  Variables to indicate if the message is enabled. Each bit of the mask
 *  indicate the module which defined HD_LOG_MASK_.
 *  Don't use it outside.
 */
extern unsigned int g_hd_mask_err;
extern unsigned int g_hd_mask_wrn;
extern unsigned int g_hd_mask_ind;
extern unsigned int g_hd_mask_msg;
extern unsigned int g_hd_mask_func;

/**
 *  Internal macros to transfer the mask variables to upper case
 *  for macros of module printf. Don't use it outside.
 */
#define _HD_MASK_ERR g_hd_mask_err
#define _HD_MASK_WRN g_hd_mask_wrn
#define _HD_MASK_IND g_hd_mask_ind
#define _HD_MASK_MSG g_hd_mask_msg
#define _HD_MASK_FUNC g_hd_mask_func

/**
 *  Internal macros to bind two words
 *  Don't use it outside.
 */
#define _HD_LOG_BIND(a, b) a ## b
#define HD_LOG_BIND(a, b) _HD_LOG_BIND(a, b)

/**
 *  Internal macros to make the module's printf macro easier.
 *  Don't use it outside.
 */
#define _HD_MODULE_PRINT(module, type, fmtstr, args...) do { \
		if ((_HD_LOG_BIND(_HD_MASK_, type) & _HD_LOG_BIND(HD_LOG_MASK_, module))) { \
			HD_PRINTF("%s: [%s][%s():%d]: " fmtstr, #type, #module, __FUNCTION__, __LINE__, ##args); \
		} \
	} while (0)

#define _HD_MODULE_PRINT_MSG(module, type, fmtstr, args...) do { \
		if ((_HD_LOG_BIND(_HD_MASK_, type) & _HD_LOG_BIND(HD_LOG_MASK_, module))) { \
			HD_PRINTF(fmtstr, ##args); \
		} \
	} while (0)

#define _HD_MODULE_PRINT_FUNC(module, type, fmtstr, args...) do { \
		if ((_HD_LOG_BIND(_HD_MASK_, type) & _HD_LOG_BIND(HD_LOG_MASK_, module))) { \
			HD_PRINTF("\t--- [%s][%s()] " fmtstr, #module, __FUNCTION__, ##args); \
		} \
	} while (0)

/**
 *  AUDIOCAPTURE message macros
 */
#define HD_AUDIOCAPTURE_ERR(fmtstr, args...) _HD_MODULE_PRINT(AUDIOCAPTURE, ERR, fmtstr, ##args)
#define HD_AUDIOCAPTURE_WRN(fmtstr, args...) _HD_MODULE_PRINT(AUDIOCAPTURE, WRN, fmtstr, ##args)
#define HD_AUDIOCAPTURE_IND(fmtstr, args...) _HD_MODULE_PRINT(AUDIOCAPTURE, IND, fmtstr, ##args)
#define HD_AUDIOCAPTURE_MSG(fmtstr, args...) _HD_MODULE_PRINT_MSG(AUDIOCAPTURE, MSG, fmtstr, ##args)
#define HD_AUDIOCAPTURE_FUNC(fmtstr, args...) _HD_MODULE_PRINT_FUNC(AUDIOCAPTURE, FUNC, fmtstr, ##args)

/**
 *  AUDIOOUT message macros
 */
#define HD_AUDIOOUT_ERR(fmtstr, args...) _HD_MODULE_PRINT(AUDIOOUT, ERR, fmtstr, ##args)
#define HD_AUDIOOUT_WRN(fmtstr, args...) _HD_MODULE_PRINT(AUDIOOUT, WRN, fmtstr, ##args)
#define HD_AUDIOOUT_IND(fmtstr, args...) _HD_MODULE_PRINT(AUDIOOUT, IND, fmtstr, ##args)
#define HD_AUDIOOUT_MSG(fmtstr, args...) _HD_MODULE_PRINT_MSG(AUDIOOUT, MSG, fmtstr, ##args)
#define HD_AUDIOOUT_FUNC(fmtstr, args...) _HD_MODULE_PRINT_FUNC(AUDIOOUT, FUNC, fmtstr, ##args)

/**
 *  AUDIOENC message macros
 */
#define HD_AUDIOENC_ERR(fmtstr, args...) _HD_MODULE_PRINT(AUDIOENC, ERR, fmtstr, ##args)
#define HD_AUDIOENC_WRN(fmtstr, args...) _HD_MODULE_PRINT(AUDIOENC, WRN, fmtstr, ##args)
#define HD_AUDIOENC_IND(fmtstr, args...) _HD_MODULE_PRINT(AUDIOENC, IND, fmtstr, ##args)
#define HD_AUDIOENC_MSG(fmtstr, args...) _HD_MODULE_PRINT_MSG(AUDIOENC, MSG, fmtstr, ##args)
#define HD_AUDIOENC_FUNC(fmtstr, args...) _HD_MODULE_PRINT_FUNC(AUDIOENC, FUNC, fmtstr, ##args)

/**
 *  AUDIODEC message macros
 */
#define HD_AUDIODEC_ERR(fmtstr, args...) _HD_MODULE_PRINT(AUDIODEC, ERR, fmtstr, ##args)
#define HD_AUDIODEC_WRN(fmtstr, args...) _HD_MODULE_PRINT(AUDIODEC, WRN, fmtstr, ##args)
#define HD_AUDIODEC_IND(fmtstr, args...) _HD_MODULE_PRINT(AUDIODEC, IND, fmtstr, ##args)
#define HD_AUDIODEC_MSG(fmtstr, args...) _HD_MODULE_PRINT_MSG(AUDIODEC, MSG, fmtstr, ##args)
#define HD_AUDIODEC_FUNC(fmtstr, args...) _HD_MODULE_PRINT_FUNC(AUDIODEC, FUNC, fmtstr, ##args)

/**
 *  VIDEOCAPTURE message macros
 */
#define HD_VIDEOCAPTURE_ERR(fmtstr, args...) _HD_MODULE_PRINT(VIDEOCAPTURE, ERR, fmtstr, ##args)
#define HD_VIDEOCAPTURE_WRN(fmtstr, args...) _HD_MODULE_PRINT(VIDEOCAPTURE, WRN, fmtstr, ##args)
#define HD_VIDEOCAPTURE_IND(fmtstr, args...) _HD_MODULE_PRINT(VIDEOCAPTURE, IND, fmtstr, ##args)
#define HD_VIDEOCAPTURE_MSG(fmtstr, args...) _HD_MODULE_PRINT_MSG(VIDEOCAPTURE, MSG, fmtstr, ##args)
#define HD_VIDEOCAPTURE_FUNC(fmtstr, args...) _HD_MODULE_PRINT_FUNC(VIDEOCAPTURE, FUNC, fmtstr, ##args)

/**
 *  VIDEOOUT message macros
 */
#define HD_VIDEOOUT_ERR(fmtstr, args...) _HD_MODULE_PRINT(VIDEOOUT, ERR, fmtstr, ##args)
#define HD_VIDEOOUT_WRN(fmtstr, args...) _HD_MODULE_PRINT(VIDEOOUT, WRN, fmtstr, ##args)
#define HD_VIDEOOUT_IND(fmtstr, args...) _HD_MODULE_PRINT(VIDEOOUT, IND, fmtstr, ##args)
#define HD_VIDEOOUT_MSG(fmtstr, args...) _HD_MODULE_PRINT_MSG(VIDEOOUT, MSG, fmtstr, ##args)
#define HD_VIDEOOUT_FUNC(fmtstr, args...) _HD_MODULE_PRINT_FUNC(VIDEOOUT, FUNC, fmtstr, ##args)

/**
 *  VIDEOPROCESS message macros
 */
#define HD_VIDEOPROC_ERR(fmtstr, args...) _HD_MODULE_PRINT(VIDEOPROCESS, ERR, fmtstr, ##args)
#define HD_VIDEOPROC_WRN(fmtstr, args...) _HD_MODULE_PRINT(VIDEOPROCESS, WRN, fmtstr, ##args)
#define HD_VIDEOPROC_IND(fmtstr, args...) _HD_MODULE_PRINT(VIDEOPROCESS, IND, fmtstr, ##args)
#define HD_VIDEOPROC_MSG(fmtstr, args...) _HD_MODULE_PRINT_MSG(VIDEOPROCESS, MSG, fmtstr, ##args)
#define HD_VIDEOPROC_FUNC(fmtstr, args...) _HD_MODULE_PRINT_FUNC(VIDEOPROCESS, FUNC, fmtstr, ##args)

/**
 *  VIDEOENC message macros
 */
#define HD_VIDEOENC_ERR(fmtstr, args...) _HD_MODULE_PRINT(VIDEOENC, ERR, fmtstr, ##args)
#define HD_VIDEOENC_WRN(fmtstr, args...) _HD_MODULE_PRINT(VIDEOENC, WRN, fmtstr, ##args)
#define HD_VIDEOENC_IND(fmtstr, args...) _HD_MODULE_PRINT(VIDEOENC, IND, fmtstr, ##args)
#define HD_VIDEOENC_MSG(fmtstr, args...) _HD_MODULE_PRINT_MSG(VIDEOENC, MSG, fmtstr, ##args)
#define HD_VIDEOENC_FUNC(fmtstr, args...) _HD_MODULE_PRINT_FUNC(VIDEOENC, FUNC, fmtstr, ##args)

/**
 *  VIDEODEC message macros
 */
#define HD_VIDEODEC_ERR(fmtstr, args...) _HD_MODULE_PRINT(VIDEODEC, ERR, fmtstr, ##args)
#define HD_VIDEODEC_WRN(fmtstr, args...) _HD_MODULE_PRINT(VIDEODEC, WRN, fmtstr, ##args)
#define HD_VIDEODEC_IND(fmtstr, args...) _HD_MODULE_PRINT(VIDEODEC, IND, fmtstr, ##args)
#define HD_VIDEODEC_MSG(fmtstr, args...) _HD_MODULE_PRINT_MSG(VIDEODEC, MSG, fmtstr, ##args)
#define HD_VIDEODEC_FUNC(fmtstr, args...) _HD_MODULE_PRINT_FUNC(VIDEODEC, FUNC, fmtstr, ##args)

/**
 *  GFX message macros
 */
#define HD_GFX_ERR(fmtstr, args...) _HD_MODULE_PRINT(GFX, ERR, fmtstr, ##args)
#define HD_GFX_WRN(fmtstr, args...) _HD_MODULE_PRINT(GFX, WRN, fmtstr, ##args)
#define HD_GFX_IND(fmtstr, args...) _HD_MODULE_PRINT(GFX, IND, fmtstr, ##args)
#define HD_GFX_MSG(fmtstr, args...) _HD_MODULE_PRINT_MSG(GFX, MSG, fmtstr, ##args)
#define HD_GFX_FUNC(fmtstr, args...) _HD_MODULE_PRINT_FUNC(GFX, FUNC, fmtstr, ##args)

/**
 *  OSG message macros
 */
#define HD_OSG_ERR(fmtstr, args...) _HD_MODULE_PRINT(GFX, ERR, fmtstr, ##args)
#define HD_OSG_WRN(fmtstr, args...) _HD_MODULE_PRINT(GFX, WRN, fmtstr, ##args)
#define HD_OSG_IND(fmtstr, args...) _HD_MODULE_PRINT(GFX, IND, fmtstr, ##args)
#define HD_OSG_MSG(fmtstr, args...) _HD_MODULE_PRINT_MSG(GFX, MSG, fmtstr, ##args)
#define HD_OSG_FUNC(fmtstr, args...) _HD_MODULE_PRINT_FUNC(GFX, FUNC, fmtstr, ##args)

/**
 *  COMMON message macros
 */
#define HD_COMMON_ERR(fmtstr, args...) _HD_MODULE_PRINT(COMMON, ERR, fmtstr, ##args)
#define HD_COMMON_WRN(fmtstr, args...) _HD_MODULE_PRINT(COMMON, WRN, fmtstr, ##args)
#define HD_COMMON_IND(fmtstr, args...) _HD_MODULE_PRINT(COMMON, IND, fmtstr, ##args)
#define HD_COMMON_MSG(fmtstr, args...) _HD_MODULE_PRINT_MSG(COMMON, MSG, fmtstr, ##args)
#define HD_COMMON_FUNC(fmtstr, args...) _HD_MODULE_PRINT_FUNC(COMMON, FUNC, fmtstr, ##args)

/**
 *  UTIL message macros
 */
#define HD_UTIL_ERR(fmtstr, args...) _HD_MODULE_PRINT(UTIL, ERR, fmtstr, ##args)
#define HD_UTIL_WRN(fmtstr, args...) _HD_MODULE_PRINT(UTIL, WRN, fmtstr, ##args)
#define HD_UTIL_IND(fmtstr, args...) _HD_MODULE_PRINT(UTIL, IND, fmtstr, ##args)
#define HD_UTIL_MSG(fmtstr, args...) _HD_MODULE_PRINT_MSG(UTIL, MSG, fmtstr, ##args)
#define HD_UTIL_FUNC(fmtstr, args...) _HD_MODULE_PRINT_FUNC(UTIL, FUNC, fmtstr, ##args)

/**
 *  DEBUG message macros
 */
#define HD_DEBUG_ERR(fmtstr, args...) _HD_MODULE_PRINT(DEBUG, ERR, fmtstr, ##args)
#define HD_DEBUG_WRN(fmtstr, args...) _HD_MODULE_PRINT(DEBUG, WRN, fmtstr, ##args)
#define HD_DEBUG_IND(fmtstr, args...) _HD_MODULE_PRINT(DEBUG, IND, fmtstr, ##args)
#define HD_DEBUG_MSG(fmtstr, args...) _HD_MODULE_PRINT_MSG(DEBUG, MSG, fmtstr, ##args)
#define HD_DEBUG_FUNC(fmtstr, args...) _HD_MODULE_PRINT_FUNC(DEBUG, FUNC, fmtstr, ##args)

#ifdef __cplusplus
}
#endif
#endif /* _HD_LOGGER_H_ */
