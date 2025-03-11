/**
	@brief Header file of basic type definition.

	@file hd_type.h

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#ifndef _HD_TYPE_H_
#define _HD_TYPE_H_

/********************************************************************
    INCLUDE FILES
 ********************************************************************/
#if 0 //obsoleted, to improve portability on various OS!
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/mman.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#endif

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/********************************************************************
    MACRO CONSTANT DEFINITIONS
 ********************************************************************/
#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifndef NULL
#define NULL    ((void *)0)
#endif


/**
    Macro to generate dummy element for enum type to expand enum size to word (4 bytes)
*/
//@{
//#define ENUM_DUMMY4WORD(name)   E_##name = 0x10000000
//@}

/**
    @name Align macros

    Align floor, align round, and align ceil

    @note base must be 2^n, where n = 1, 2, 3, ...
*/
//@{
#define ALIGN_FLOOR(value, base)  ((value) & ~((base)-1))                   ///< Align Floor
#define ALIGN_ROUND(value, base)  ALIGN_FLOOR((value) + ((base)/2), base)   ///< Align Round
#define ALIGN_CEIL(value, base)   ALIGN_FLOOR((value) + ((base)-1), base)   ///< Align Ceil
//@}

/**
    @name Align (round off)

    Round Off to 32, 16, 8 and 4

    @note Example: ALIGN_ROUND_32(32) = 32, ALIGN_ROUND_32(47) = 32, ALIGN_ROUND_32(48) = 64
*/
//@{
#define ALIGN_ROUND_64(a)       ALIGN_ROUND(a, 64)  ///< Round Off to 64
#define ALIGN_ROUND_32(a)       ALIGN_ROUND(a, 32)  ///< Round Off to 32
#define ALIGN_ROUND_16(a)       ALIGN_ROUND(a, 16)  ///< Round Off to 16
#define ALIGN_ROUND_8(a)        ALIGN_ROUND(a, 8)   ///< Round Off to 8
#define ALIGN_ROUND_4(a)        ALIGN_ROUND(a, 4)   ///< Round Off to 4
//@}

/**
    @name Align (round up)

    Round Up to 32, 16, 8 and 4.

    @note Example: ALIGN_CEIL_32(32) = 32, ALIGN_CEIL_32(33) = 64, ALIGN_CEIL_32(63) = 64
*/
//@{
#define ALIGN_CEIL_64(a)        ALIGN_CEIL(a, 64)   ///< Round Up to 64
#define ALIGN_CEIL_32(a)        ALIGN_CEIL(a, 32)   ///< Round Up to 32
#define ALIGN_CEIL_16(a)        ALIGN_CEIL(a, 16)   ///< Round Up to 16
#define ALIGN_CEIL_8(a)         ALIGN_CEIL(a, 8)    ///< Round Up to 8
#define ALIGN_CEIL_4(a)         ALIGN_CEIL(a, 4)    ///< Round Up to 4
//@}

/**
    @name Align (round down)

    Round Down to 32, 16, 8 and 4.

    @note Example: ALIGN_FLOOR_32(32) = 32, ALIGN_FLOOR_32(33) = 32, ALIGN_FLOOR_32(63) = 32
*/
//@{
#define ALIGN_FLOOR_64(a)       ALIGN_FLOOR(a, 64)  ///< Round down to 64
#define ALIGN_FLOOR_32(a)       ALIGN_FLOOR(a, 32)  ///< Round down to 32
#define ALIGN_FLOOR_16(a)       ALIGN_FLOOR(a, 16)  ///< Round down to 16
#define ALIGN_FLOOR_8(a)        ALIGN_FLOOR(a, 8)   ///< Round down to 8
#define ALIGN_FLOOR_4(a)        ALIGN_FLOOR(a, 4)   ///< Round down to 4
//@}

/**
    @name Assert macros
*/
//@{
#define ASSERT_CONCAT_(a, b)    a##b
#define ASSERT_CONCAT(a, b)     ASSERT_CONCAT_(a, b)

#if defined(__COUNTER__)

#define STATIC_ASSERT(expr) \
	enum { ASSERT_CONCAT(FAILED_STATIC_ASSERT_, __COUNTER__) = 1/(expr) }

#else

// This might cause compile error when writing STATIC_ASSERT at the same line
// in two (or more) files and one file include another one.
#define STATIC_ASSERT(expr) \
	enum { ASSERT_CONCAT(FAILED_STATIC_ASSERT_, __LINE__) = 1/(expr) }

#endif
//@}

/**
    Macro to generate dummy element for enum type to expand enum size to word (4 bytes)
*/
//@{
#define ENUM_DUMMY4WORD(name)   E_##name = 0x10000000
//@}


/********************************************************************
    TYPE DEFINITION
 ********************************************************************/
typedef void                    VOID;
typedef int                     BOOLEAN;
typedef unsigned long long      UINT64;
typedef signed long long        INT64;
typedef unsigned long           UINT32;
typedef signed long             INT32;
typedef unsigned short          UINT16;
typedef signed short            INT16;
typedef unsigned char           UINT8;
typedef signed char             INT8;
typedef unsigned int            UINT;
typedef signed int              INT;
typedef float                   FLOAT;
typedef double                  DOUBLE;
typedef char                    CHAR;
typedef unsigned int            BOOL;

/**
     make high/low 16bits to 32bits
*/
#define MAKE_UINT16_UINT16(h, l)        (((((UINT32)(h)) & 0x0000ffff)<<16) | (((UINT32)(l)) & 0x0000ffff))

/**
     get high/low 32bits from 64bits
*/
//@{
#define GET_HI_UINT32(v)                ((UINT32)((v) >> 32))
#define GET_LO_UINT32(v)                ((UINT32)(v))
//@}

/**
     get high/low 16bits from 32bits
*/
//@{
#define GET_HI_UINT16(v)                ((((UINT32)(v)) >> 16) & 0x0000ffff)
#define GET_LO_UINT16(v)                (((UINT32)(v)) & 0x0000ffff)
//@}


/**
    Convert four characters into a four-character code.
*/
//@{
#define MAKEFOURCC(ch0, ch1, ch2, ch3)  ((UINT32)(UINT8)(ch0) | ((UINT32)(UINT8)(ch1) << 8) | ((UINT32)(UINT8)(ch2) << 16) | ((UINT32)(UINT8)(ch3) << 24))   ///<
//@}


typedef struct _HD_UPOINT{
	UINT32  x;                          ///< x coordinate of the point
	UINT32  y;                          ///< y coordinate of the point
} HD_UPOINT;

typedef struct _HD_URECT {
	UINT32  x;                          ///< x coordinate of the top-left point of the rectangle
	UINT32  y;                          ///< y coordinate of the top-left point of the rectangle
	UINT32  w;                          ///< rectangle width
	UINT32  h;                          ///< rectangle height
} HD_URECT;

typedef struct _HD_IPOINT {
	INT32  x;                           ///< x coordinate of the point
	INT32  y;                           ///< y coordinate of the point
} HD_IPOINT;

typedef struct _HD_IRECT {
	INT32  x;                           ///< x coordinate of the top-left point of the rectangle
	INT32  y;                           ///< y coordinate of the top-left point of the rectangle
	INT32  w;                           ///< rectangle width
	INT32  h;                           ///< rectangle height
} HD_IRECT;

typedef struct _HD_BUFINFO {
	UINT32 phy_addr;                    ///< physical start address of buffer (page aligned)
	UINT32 buf_size;                    ///< size of buffer
} HD_BUFINFO;

typedef enum _HD_RESULT {
	HD_OK =                             0,

	///< general error
	HD_ERR_NOT_SUPPORT =                -1, ///< not support

	///< execute error
	HD_ERR_NG =                         -10, ///< general failure
	HD_ERR_SYS =                        -11, ///< operating system call failure
	HD_ERR_DRV =                        -12, ///< driver call failure
	HD_ERR_NOT_ALLOW =                  -13, ///< not allow in this case
	HD_ERR_ABORT =                      -14, ///< ignored or skipped
	HD_ERR_TIMEDOUT =                   -15, ///< timeout occured
	HD_ERR_FAIL =                       -19, ///< already executed and failed

	///< module state error
	HD_ERR_INIT =                       -20, ///< module is already initialised
	HD_ERR_UNINIT =                     -21, ///< module is not initialised yet
	HD_ERR_STATE =                      -22, ///< invalid state to invoke function
	HD_ERR_TERM =                       -23, ///< module has terminated
	HD_ERR_NOT_OPEN =                   -24, ///< path is not open yet
	HD_ERR_ALREADY_OPEN =               -25, ///< path is already open
	HD_ERR_NOT_START =                  -26, ///< path is not start yet
	HD_ERR_ALREADY_START =              -27, ///< path is already start
	HD_ERR_NOT_BIND =                   -28, ///< io is not bind yet
	HD_ERR_ALREADY_BIND =               -29, ///< io is already bind

	///< parameter error
	HD_ERR_UNIQUE =                     -30, ///< unique id needed
	HD_ERR_DEV =                        -31, ///< invalid device id
	HD_ERR_IO =                         -32, ///< invalid io id
	HD_ERR_PATH =                       -33, ///< invalid path id
	HD_ERR_PARAM =                      -34, ///< invalid param id
	HD_ERR_INV =                        -35, ///< invalid argument passed
	HD_ERR_LIMIT =                      -36, ///< parameter value limit was reached
	HD_ERR_NULL_PTR =                   -37, ///< null pointer
	HD_ERR_NO_CONFIG =                  -38, ///< module device config or path config is not set
	HD_ERR_NO_DRV_CONFIG =              -39, ///< module driver config is not set

	///< object or resource error
	HD_ERR_RESOURCE =                   -40, ///< insufficient resources for request
	HD_ERR_NOT_AVAIL =                  -41, ///< resources not available
	HD_ERR_NOT_FOUND =                  -42, ///< not found
	HD_ERR_EOL =                        -43, ///< a list traversal has ended
	HD_ERR_IN_USE =                     -44, ///< object is in use.
	HD_ERR_NOT_USE =                    -45, ///< object is not in use.
	HD_ERR_LOCKED =                     -46, ///< object locked
	HD_ERR_DELETED =                    -47, ///< object has been deleted

	///< data or buffer error
	HD_ERR_HEAP =                       -50, ///< heap full (application)
	HD_ERR_NOMEM =                      -51, ///< no memory (not enough system mem for create pool)
	HD_ERR_NOBUF =                      -52, ///< no buffer (not enough pool size for new blk)
	HD_ERR_SIGN =                       -53, ///< invalid sign in data
	HD_ERR_BAD_DATA =                   -54, ///< bad content in data
	HD_ERR_OVERRUN =                    -55, ///< buffer overrun occured, or data queue full
	HD_ERR_UNDERRUN =                   -56, ///< buffer underrun occured, or data queue empty

	///< other error
	HD_ERR_INLINE_SUCCESS =             -90,
	HD_ERR_INTERMEDIATE =               -91, ///< intermediate return - successful but not complete
	HD_ERR_DEFERRED_SUCCESS =           -92,
	HD_ERR_DEFERRED =                   -93, ///< action has been deferred
	HD_ERR_INV_PTR =                    -94, ///< invalid pointer
	HD_ERR_USER =                       -99, ///< start of subsystem specific errors
} HD_RESULT; ///< HDAL error code

/**
	@name type of DAL
*/
typedef unsigned int HD_DAL;            ///< unsigned 32 bits data type

/**
     macro: total device count of DAL.
*/
#define HD_DAL_AUDIOCAP_COUNT           16
#define HD_DAL_AUDIOOUT_COUNT           16
#define HD_DAL_AUDIOENC_COUNT           16
#define HD_DAL_AUDIODEC_COUNT           16
#define HD_DAL_VIDEOCAP_COUNT           64
#define HD_DAL_VIDEOPROC_COUNT          512
#define HD_DAL_VIDEOOUT_COUNT           16
#define HD_DAL_VIDEOENC_COUNT           16
#define HD_DAL_VIDEODEC_COUNT           16
#define HD_DAL_GFX_COUNT                16
/**
     macro: combine of DAL id, devie id.
*/
#define HD_DAL_AUDIOCAP_BASE            0x1000
#define HD_DAL_AUDIOCAP(did)            (HD_DAL_AUDIOCAP_BASE + (did))
#define HD_DAL_AUDIOCAP_MAX             (HD_DAL_AUDIOCAP_BASE + HD_DAL_AUDIOCAP_COUNT - 1)

#define HD_DAL_AUDIOOUT_BASE            0x1100
#define HD_DAL_AUDIOOUT(did)            (HD_DAL_AUDIOOUT_BASE + (did))
#define HD_DAL_AUDIOOUT_MAX             (HD_DAL_AUDIOOUT_BASE + HD_DAL_AUDIOOUT_COUNT - 1)

#define HD_DAL_AUDIOENC_BASE            0x1200
#define HD_DAL_AUDIOENC(did)            (HD_DAL_AUDIOENC_BASE + (did))
#define HD_DAL_AUDIOENC_MAX             (HD_DAL_AUDIOENC_BASE + HD_DAL_AUDIOENC_COUNT - 1)

#define HD_DAL_AUDIODEC_BASE            0x1300
#define HD_DAL_AUDIODEC(did)            (HD_DAL_AUDIODEC_BASE + (did))
#define HD_DAL_AUDIODEC_MAX             (HD_DAL_AUDIODEC_BASE + HD_DAL_AUDIODEC_COUNT - 1)

#define HD_DAL_VIDEOCAP_BASE            0x2000
#define HD_DAL_VIDEOCAP(did)            (HD_DAL_VIDEOCAP_BASE + (did))
#define HD_DAL_VIDEOCAP_MAX             (HD_DAL_VIDEOCAP_BASE + HD_DAL_VIDEOCAP_COUNT - 1)

#define HD_DAL_VIDEOOUT_BASE            0x2100
#define HD_DAL_VIDEOOUT(did)            (HD_DAL_VIDEOOUT_BASE + (did))
#define HD_DAL_VIDEOOUT_MAX             (HD_DAL_VIDEOOUT_BASE + HD_DAL_VIDEOOUT_COUNT - 1)

#define HD_DAL_VIDEOENC_BASE            0x2200
#define HD_DAL_VIDEOENC(did)            (HD_DAL_VIDEOENC_BASE + (did))
#define HD_DAL_VIDEOENC_MAX             (HD_DAL_VIDEOENC_BASE + HD_DAL_VIDEOENC_COUNT - 1)

#define HD_DAL_VIDEODEC_BASE            0x2300
#define HD_DAL_VIDEODEC(did)            (HD_DAL_VIDEODEC_BASE + (did))
#define HD_DAL_VIDEODEC_MAX             (HD_DAL_VIDEODEC_BASE + HD_DAL_VIDEODEC_COUNT - 1)

#define HD_DAL_VIDEOPROC_BASE           0x2400
#define HD_DAL_VIDEOPROC(did)           (HD_DAL_VIDEOPROC_BASE + (did))
#define HD_DAL_VIDEOPROC_MAX            (HD_DAL_VIDEOPROC_BASE + HD_DAL_VIDEOPROC_COUNT - 1)

#define HD_DAL_VIDEOPROC_EXTEND_BASE    0x2500 ///< for extend HD_DAL_VIDEOPROC_BASE (0x2400, 0x2500) max count 512

/**
	@name device id max count
*/
typedef struct _HD_DEVCOUNT {
	UINT32 max_dev_count;               ///< supported max count of HD_DAL_XXXX()
} HD_DEVCOUNT;


/**
	@name type of io
*/
typedef unsigned int HD_IO;             ///< unsigned 32 bits data type

/**
     @name output id.
*/
#define HD_OUT_BASE                     0x01
#define HD_OUT_MAX                      (HD_OUT_BASE+128-1)

#define HD_OUT(i)                       (HD_OUT_BASE+(i))

/**
     @name input id.
*/
#define HD_IN_BASE                      0x01
#define HD_IN_MAX                       (HD_IN_BASE+128-1)

#define HD_IN(i)                        (HD_IN_BASE+(i))

/**
     ctrl id.
*/
#define HD_CTRL                         0xffff ///< unit control


/********************************************************************
    in id and out id
********************************************************************/

#define HD_VIDEOCAP_CTRL(dev_id)                ((HD_DAL_VIDEOCAP(dev_id) << 16) | HD_CTRL)
#define HD_VIDEOCAP_IN(dev_id, in_id)           ((HD_DAL_VIDEOCAP(dev_id) << 16) | ((HD_IN(in_id) & 0x00ff) << 8))
#define HD_VIDEOCAP_OUT(dev_id, out_id)         ((HD_DAL_VIDEOCAP(dev_id) << 16) | (HD_OUT(out_id) & 0x00ff))

#define HD_VIDEOOUT_CTRL(dev_id)                ((HD_DAL_VIDEOOUT(dev_id) << 16) | HD_CTRL)
#define HD_VIDEOOUT_IN(dev_id, in_id)           ((HD_DAL_VIDEOOUT(dev_id) << 16) | ((HD_IN(in_id) & 0x00ff) << 8))
#define HD_VIDEOOUT_OUT(dev_id, out_id)         ((HD_DAL_VIDEOOUT(dev_id) << 16) | (HD_OUT(out_id) & 0x00ff))

#define HD_VIDEOENC_CTRL(dev_id)                ((HD_DAL_VIDEOENC(dev_id) << 16) | HD_CTRL)
#define HD_VIDEOENC_IN(dev_id, in_id)           ((HD_DAL_VIDEOENC(dev_id) << 16) | ((HD_IN(in_id) & 0x00ff) << 8))
#define HD_VIDEOENC_OUT(dev_id, out_id)         ((HD_DAL_VIDEOENC(dev_id) << 16) | (HD_OUT(out_id) & 0x00ff))

#define HD_VIDEODEC_CTRL(dev_id)                ((HD_DAL_VIDEODEC(dev_id) << 16) | HD_CTRL)
#define HD_VIDEODEC_IN(dev_id, in_id)           ((HD_DAL_VIDEODEC(dev_id) << 16) | ((HD_IN(in_id) & 0x00ff) << 8))
#define HD_VIDEODEC_OUT(dev_id, out_id)         ((HD_DAL_VIDEODEC(dev_id) << 16) | (HD_OUT(out_id) & 0x00ff))

#define HD_VIDEOPROC_CTRL(dev_id)               ((HD_DAL_VIDEOPROC(dev_id) << 16) | HD_CTRL)
#define HD_VIDEOPROC_IN(dev_id, in_id)          ((HD_DAL_VIDEOPROC(dev_id) << 16) | ((HD_IN(in_id) & 0x00ff) << 8))
#define HD_VIDEOPROC_OUT(dev_id, out_id)        ((HD_DAL_VIDEOPROC(dev_id) << 16) | (HD_OUT(out_id) & 0x00ff))

#define HD_AUDIOCAP_CTRL(dev_id)                ((HD_DAL_AUDIOCAP(dev_id) << 16) | HD_CTRL)
#define HD_AUDIOCAP_IN(dev_id, in_id)           ((HD_DAL_AUDIOCAP(dev_id) << 16) | ((HD_IN(in_id) & 0x00ff) << 8))
#define HD_AUDIOCAP_OUT(dev_id, out_id)         ((HD_DAL_AUDIOCAP(dev_id) << 16) | (HD_OUT(out_id) & 0x00ff))

#define HD_AUDIOOUT_CTRL(dev_id)                ((HD_DAL_AUDIOOUT(dev_id) << 16) | HD_CTRL)
#define HD_AUDIOOUT_IN(dev_id, in_id)           ((HD_DAL_AUDIOOUT(dev_id) << 16) | ((HD_IN(in_id) & 0x00ff) << 8))
#define HD_AUDIOOUT_OUT(dev_id, out_id)         ((HD_DAL_AUDIOOUT(dev_id) << 16) | (HD_OUT(out_id) & 0x00ff))

#define HD_AUDIOENC_CTRL(dev_id)                ((HD_DAL_AUDIOENC(dev_id) << 16) | HD_CTRL)
#define HD_AUDIOENC_IN(dev_id, in_id)           ((HD_DAL_AUDIOENC(dev_id) << 16) | ((HD_IN(in_id) & 0x00ff) << 8))
#define HD_AUDIOENC_OUT(dev_id, out_id)         ((HD_DAL_AUDIOENC(dev_id) << 16) | (HD_OUT(out_id) & 0x00ff))

#define HD_AUDIODEC_CTRL(dev_id)                ((HD_DAL_AUDIODEC(dev_id) << 16) | HD_CTRL)
#define HD_AUDIODEC_IN(dev_id, in_id)           ((HD_DAL_AUDIODEC(dev_id) << 16) | ((HD_IN(in_id) & 0x00ff) << 8))
#define HD_AUDIODEC_OUT(dev_id, out_id)         ((HD_DAL_AUDIODEC(dev_id) << 16) | (HD_OUT(out_id) & 0x00ff))

#define HD_GET_DEV(id)                          ((id) >> 16)
#define HD_GET_IN(id)                           (((id) & 0xff00) >> 8)
#define HD_GET_OUT(id)                          ((id) & 0x00ff)
#define HD_GET_CTRL(id)                         ((id) & 0xffff)


/**
     @name type of device ctrl id.
*/
typedef enum _HD_CTRL_ID {

	/* DEVICE 0 */
	HD_VIDEOCAP_0_CTRL = HD_VIDEOCAP_CTRL(0),
	/* DEVICE 1 */
	HD_VIDEOCAP_1_CTRL = HD_VIDEOCAP_CTRL(1),
	/* DEVICE 2 */
	HD_VIDEOCAP_2_CTRL = HD_VIDEOCAP_CTRL(2),
	/* DEVICE 3 */
	HD_VIDEOCAP_3_CTRL = HD_VIDEOCAP_CTRL(3),
	/* DEVICE 4 */
	HD_VIDEOCAP_4_CTRL = HD_VIDEOCAP_CTRL(4),
	/* DEVICE 5 */
	HD_VIDEOCAP_5_CTRL = HD_VIDEOCAP_CTRL(5),
	/* DEVICE 6 */
	HD_VIDEOCAP_6_CTRL = HD_VIDEOCAP_CTRL(6),
	/* DEVICE 7 */
	HD_VIDEOCAP_7_CTRL = HD_VIDEOCAP_CTRL(7),


	/* DEVICE 0 */
	HD_VIDEOOUT_0_CTRL = HD_VIDEOOUT_CTRL(0),
	/* DEVICE 1 */
	HD_VIDEOOUT_1_CTRL = HD_VIDEOOUT_CTRL(1),
	/* DEVICE 2 */
	HD_VIDEOOUT_2_CTRL = HD_VIDEOOUT_CTRL(2),

	/* DEVICE 0 */
	HD_VIDEOENC_0_CTRL = HD_VIDEOENC_CTRL(0),


	/* DEVICE 0 */
	HD_VIDEODEC_0_CTRL = HD_VIDEODEC_CTRL(0),


	/* DEVICE 0 */
	HD_VIDEOPROC_0_CTRL = HD_VIDEOPROC_CTRL(0),
	/* DEVICE 1 */
	HD_VIDEOPROC_1_CTRL = HD_VIDEOPROC_CTRL(1),
	/* DEVICE 2 */
	HD_VIDEOPROC_2_CTRL = HD_VIDEOPROC_CTRL(2),
	/* DEVICE 3 */
	HD_VIDEOPROC_3_CTRL = HD_VIDEOPROC_CTRL(3),
	/* DEVICE 4 */
	HD_VIDEOPROC_4_CTRL = HD_VIDEOPROC_CTRL(4),
	/* DEVICE 5 */
	HD_VIDEOPROC_5_CTRL = HD_VIDEOPROC_CTRL(5),
	/* DEVICE 6 */
	HD_VIDEOPROC_6_CTRL = HD_VIDEOPROC_CTRL(6),
	/* DEVICE 7 */
	HD_VIDEOPROC_7_CTRL = HD_VIDEOPROC_CTRL(7),
	/* DEVICE 8 */
	HD_VIDEOPROC_8_CTRL = HD_VIDEOPROC_CTRL(8),
	/* DEVICE 9 */
	HD_VIDEOPROC_9_CTRL = HD_VIDEOPROC_CTRL(9),
	/* DEVICE 10 */
	HD_VIDEOPROC_10_CTRL = HD_VIDEOPROC_CTRL(10),
	/* DEVICE 11 */
	HD_VIDEOPROC_11_CTRL = HD_VIDEOPROC_CTRL(11),
	/* DEVICE 12 */
	HD_VIDEOPROC_12_CTRL = HD_VIDEOPROC_CTRL(12),
	/* DEVICE 13 */
	HD_VIDEOPROC_13_CTRL = HD_VIDEOPROC_CTRL(13),
	/* DEVICE 14 */
	HD_VIDEOPROC_14_CTRL = HD_VIDEOPROC_CTRL(14),
	/* DEVICE 15 */
	HD_VIDEOPROC_15_CTRL = HD_VIDEOPROC_CTRL(15),
	/* DEVICE 16 */
	HD_VIDEOPROC_16_CTRL = HD_VIDEOPROC_CTRL(16),
	/* DEVICE 17 */
	HD_VIDEOPROC_17_CTRL = HD_VIDEOPROC_CTRL(17),
	/* DEVICE 18 */
	HD_VIDEOPROC_18_CTRL = HD_VIDEOPROC_CTRL(18),
	/* DEVICE 19 */
	HD_VIDEOPROC_19_CTRL = HD_VIDEOPROC_CTRL(19),
	/* DEVICE 20 */
	HD_VIDEOPROC_20_CTRL = HD_VIDEOPROC_CTRL(20),
	/* DEVICE 21 */
	HD_VIDEOPROC_21_CTRL = HD_VIDEOPROC_CTRL(21),
	/* DEVICE 22 */
	HD_VIDEOPROC_22_CTRL = HD_VIDEOPROC_CTRL(22),
	/* DEVICE 23 */
	HD_VIDEOPROC_23_CTRL = HD_VIDEOPROC_CTRL(23),
	/* DEVICE 24 */
	HD_VIDEOPROC_24_CTRL = HD_VIDEOPROC_CTRL(24),
	/* DEVICE 25 */
	HD_VIDEOPROC_25_CTRL = HD_VIDEOPROC_CTRL(25),
	/* DEVICE 26 */
	HD_VIDEOPROC_26_CTRL = HD_VIDEOPROC_CTRL(26),
	/* DEVICE 27 */
	HD_VIDEOPROC_27_CTRL = HD_VIDEOPROC_CTRL(27),
	/* DEVICE 28 */
	HD_VIDEOPROC_28_CTRL = HD_VIDEOPROC_CTRL(28),
	/* DEVICE 29 */
	HD_VIDEOPROC_29_CTRL = HD_VIDEOPROC_CTRL(29),
	/* DEVICE 30 */
	HD_VIDEOPROC_30_CTRL = HD_VIDEOPROC_CTRL(30),
	/* DEVICE 31 */
	HD_VIDEOPROC_31_CTRL = HD_VIDEOPROC_CTRL(31),
	/* DEVICE 32 */
	HD_VIDEOPROC_32_CTRL = HD_VIDEOPROC_CTRL(32),
	/* DEVICE 33 */
	HD_VIDEOPROC_33_CTRL = HD_VIDEOPROC_CTRL(33),
	/* DEVICE 34 */
	HD_VIDEOPROC_34_CTRL = HD_VIDEOPROC_CTRL(34),
	/* DEVICE 35 */
	HD_VIDEOPROC_35_CTRL = HD_VIDEOPROC_CTRL(35),
	/* DEVICE 36 */
	HD_VIDEOPROC_36_CTRL = HD_VIDEOPROC_CTRL(36),
	/* DEVICE 37 */
	HD_VIDEOPROC_37_CTRL = HD_VIDEOPROC_CTRL(37),
	/* DEVICE 38 */
	HD_VIDEOPROC_38_CTRL = HD_VIDEOPROC_CTRL(38),
	/* DEVICE 39 */
	HD_VIDEOPROC_39_CTRL = HD_VIDEOPROC_CTRL(39),
	/* DEVICE 40 */
	HD_VIDEOPROC_40_CTRL = HD_VIDEOPROC_CTRL(40),
	/* DEVICE 41 */
	HD_VIDEOPROC_41_CTRL = HD_VIDEOPROC_CTRL(41),
	/* DEVICE 42 */
	HD_VIDEOPROC_42_CTRL = HD_VIDEOPROC_CTRL(42),
	/* DEVICE 43 */
	HD_VIDEOPROC_43_CTRL = HD_VIDEOPROC_CTRL(43),
	/* DEVICE 44 */
	HD_VIDEOPROC_44_CTRL = HD_VIDEOPROC_CTRL(44),
	/* DEVICE 45 */
	HD_VIDEOPROC_45_CTRL = HD_VIDEOPROC_CTRL(45),
	/* DEVICE 46 */
	HD_VIDEOPROC_46_CTRL = HD_VIDEOPROC_CTRL(46),
	/* DEVICE 47 */
	HD_VIDEOPROC_47_CTRL = HD_VIDEOPROC_CTRL(47),
	/* DEVICE 48 */
	HD_VIDEOPROC_48_CTRL = HD_VIDEOPROC_CTRL(48),
	/* DEVICE 49 */
	HD_VIDEOPROC_49_CTRL = HD_VIDEOPROC_CTRL(49),
	/* DEVICE 50 */
	HD_VIDEOPROC_50_CTRL = HD_VIDEOPROC_CTRL(50),
	/* DEVICE 51 */
	HD_VIDEOPROC_51_CTRL = HD_VIDEOPROC_CTRL(51),
	/* DEVICE 52 */
	HD_VIDEOPROC_52_CTRL = HD_VIDEOPROC_CTRL(52),
	/* DEVICE 53 */
	HD_VIDEOPROC_53_CTRL = HD_VIDEOPROC_CTRL(53),
	/* DEVICE 54 */
	HD_VIDEOPROC_54_CTRL = HD_VIDEOPROC_CTRL(54),
	/* DEVICE 55 */
	HD_VIDEOPROC_55_CTRL = HD_VIDEOPROC_CTRL(55),
	/* DEVICE 56 */
	HD_VIDEOPROC_56_CTRL = HD_VIDEOPROC_CTRL(56),
	/* DEVICE 57 */
	HD_VIDEOPROC_57_CTRL = HD_VIDEOPROC_CTRL(57),
	/* DEVICE 58 */
	HD_VIDEOPROC_58_CTRL = HD_VIDEOPROC_CTRL(58),
	/* DEVICE 59 */
	HD_VIDEOPROC_59_CTRL = HD_VIDEOPROC_CTRL(59),
	/* DEVICE 60 */
	HD_VIDEOPROC_60_CTRL = HD_VIDEOPROC_CTRL(60),
	/* DEVICE 61 */
	HD_VIDEOPROC_61_CTRL = HD_VIDEOPROC_CTRL(61),
	/* DEVICE 62 */
	HD_VIDEOPROC_62_CTRL = HD_VIDEOPROC_CTRL(62),
	/* DEVICE 63 */
	HD_VIDEOPROC_63_CTRL = HD_VIDEOPROC_CTRL(63),
	/* DEVICE 64 */
	HD_VIDEOPROC_64_CTRL = HD_VIDEOPROC_CTRL(64),
	/* DEVICE 65 */
	HD_VIDEOPROC_65_CTRL = HD_VIDEOPROC_CTRL(65),
	/* DEVICE 66 */
	HD_VIDEOPROC_66_CTRL = HD_VIDEOPROC_CTRL(66),
	/* DEVICE 67 */
	HD_VIDEOPROC_67_CTRL = HD_VIDEOPROC_CTRL(67),
	/* DEVICE 68 */
	HD_VIDEOPROC_68_CTRL = HD_VIDEOPROC_CTRL(68),
	/* DEVICE 69 */
	HD_VIDEOPROC_69_CTRL = HD_VIDEOPROC_CTRL(69),
	/* DEVICE 70 */
	HD_VIDEOPROC_70_CTRL = HD_VIDEOPROC_CTRL(70),
	/* DEVICE 71 */
	HD_VIDEOPROC_71_CTRL = HD_VIDEOPROC_CTRL(71),
	/* DEVICE 72 */
	HD_VIDEOPROC_72_CTRL = HD_VIDEOPROC_CTRL(72),
	/* DEVICE 73 */
	HD_VIDEOPROC_73_CTRL = HD_VIDEOPROC_CTRL(73),
	/* DEVICE 74 */
	HD_VIDEOPROC_74_CTRL = HD_VIDEOPROC_CTRL(74),
	/* DEVICE 75 */
	HD_VIDEOPROC_75_CTRL = HD_VIDEOPROC_CTRL(75),
	/* DEVICE 76 */
	HD_VIDEOPROC_76_CTRL = HD_VIDEOPROC_CTRL(76),
	/* DEVICE 77 */
	HD_VIDEOPROC_77_CTRL = HD_VIDEOPROC_CTRL(77),
	/* DEVICE 78 */
	HD_VIDEOPROC_78_CTRL = HD_VIDEOPROC_CTRL(78),
	/* DEVICE 79 */
	HD_VIDEOPROC_79_CTRL = HD_VIDEOPROC_CTRL(79),
	/* DEVICE 80 */
	HD_VIDEOPROC_80_CTRL = HD_VIDEOPROC_CTRL(80),
	/* DEVICE 81 */
	HD_VIDEOPROC_81_CTRL = HD_VIDEOPROC_CTRL(81),
	/* DEVICE 82 */
	HD_VIDEOPROC_82_CTRL = HD_VIDEOPROC_CTRL(82),
	/* DEVICE 83 */
	HD_VIDEOPROC_83_CTRL = HD_VIDEOPROC_CTRL(83),
	/* DEVICE 84 */
	HD_VIDEOPROC_84_CTRL = HD_VIDEOPROC_CTRL(84),
	/* DEVICE 85 */
	HD_VIDEOPROC_85_CTRL = HD_VIDEOPROC_CTRL(85),
	/* DEVICE 86 */
	HD_VIDEOPROC_86_CTRL = HD_VIDEOPROC_CTRL(86),
	/* DEVICE 87 */
	HD_VIDEOPROC_87_CTRL = HD_VIDEOPROC_CTRL(87),
	/* DEVICE 88 */
	HD_VIDEOPROC_88_CTRL = HD_VIDEOPROC_CTRL(88),
	/* DEVICE 89 */
	HD_VIDEOPROC_89_CTRL = HD_VIDEOPROC_CTRL(89),
	/* DEVICE 90 */
	HD_VIDEOPROC_90_CTRL = HD_VIDEOPROC_CTRL(90),
	/* DEVICE 91 */
	HD_VIDEOPROC_91_CTRL = HD_VIDEOPROC_CTRL(91),
	/* DEVICE 92 */
	HD_VIDEOPROC_92_CTRL = HD_VIDEOPROC_CTRL(92),
	/* DEVICE 93 */
	HD_VIDEOPROC_93_CTRL = HD_VIDEOPROC_CTRL(93),
	/* DEVICE 94 */
	HD_VIDEOPROC_94_CTRL = HD_VIDEOPROC_CTRL(94),
	/* DEVICE 95 */
	HD_VIDEOPROC_95_CTRL = HD_VIDEOPROC_CTRL(95),
	/* DEVICE 96 */
	HD_VIDEOPROC_96_CTRL = HD_VIDEOPROC_CTRL(96),
	/* DEVICE 97 */
	HD_VIDEOPROC_97_CTRL = HD_VIDEOPROC_CTRL(97),
	/* DEVICE 98 */
	HD_VIDEOPROC_98_CTRL = HD_VIDEOPROC_CTRL(98),
	/* DEVICE 99 */
	HD_VIDEOPROC_99_CTRL = HD_VIDEOPROC_CTRL(99),
	/* DEVICE 100 */
	HD_VIDEOPROC_100_CTRL = HD_VIDEOPROC_CTRL(100),
	/* DEVICE 101 */
	HD_VIDEOPROC_101_CTRL = HD_VIDEOPROC_CTRL(101),
	/* DEVICE 102 */
	HD_VIDEOPROC_102_CTRL = HD_VIDEOPROC_CTRL(102),
	/* DEVICE 103 */
	HD_VIDEOPROC_103_CTRL = HD_VIDEOPROC_CTRL(103),
	/* DEVICE 104 */
	HD_VIDEOPROC_104_CTRL = HD_VIDEOPROC_CTRL(104),
	/* DEVICE 105 */
	HD_VIDEOPROC_105_CTRL = HD_VIDEOPROC_CTRL(105),
	/* DEVICE 106 */
	HD_VIDEOPROC_106_CTRL = HD_VIDEOPROC_CTRL(106),
	/* DEVICE 107 */
	HD_VIDEOPROC_107_CTRL = HD_VIDEOPROC_CTRL(107),
	/* DEVICE 108 */
	HD_VIDEOPROC_108_CTRL = HD_VIDEOPROC_CTRL(108),
	/* DEVICE 109 */
	HD_VIDEOPROC_109_CTRL = HD_VIDEOPROC_CTRL(109),
	/* DEVICE 110 */
	HD_VIDEOPROC_110_CTRL = HD_VIDEOPROC_CTRL(110),
	/* DEVICE 111 */
	HD_VIDEOPROC_111_CTRL = HD_VIDEOPROC_CTRL(111),
	/* DEVICE 112 */
	HD_VIDEOPROC_112_CTRL = HD_VIDEOPROC_CTRL(112),
	/* DEVICE 113 */
	HD_VIDEOPROC_113_CTRL = HD_VIDEOPROC_CTRL(113),
	/* DEVICE 114 */
	HD_VIDEOPROC_114_CTRL = HD_VIDEOPROC_CTRL(114),
	/* DEVICE 115 */
	HD_VIDEOPROC_115_CTRL = HD_VIDEOPROC_CTRL(115),
	/* DEVICE 116 */
	HD_VIDEOPROC_116_CTRL = HD_VIDEOPROC_CTRL(116),
	/* DEVICE 117 */
	HD_VIDEOPROC_117_CTRL = HD_VIDEOPROC_CTRL(117),
	/* DEVICE 118 */
	HD_VIDEOPROC_118_CTRL = HD_VIDEOPROC_CTRL(118),
	/* DEVICE 119 */
	HD_VIDEOPROC_119_CTRL = HD_VIDEOPROC_CTRL(119),
	/* DEVICE 120 */
	HD_VIDEOPROC_120_CTRL = HD_VIDEOPROC_CTRL(120),
	/* DEVICE 121 */
	HD_VIDEOPROC_121_CTRL = HD_VIDEOPROC_CTRL(121),
	/* DEVICE 122 */
	HD_VIDEOPROC_122_CTRL = HD_VIDEOPROC_CTRL(122),
	/* DEVICE 123 */
	HD_VIDEOPROC_123_CTRL = HD_VIDEOPROC_CTRL(123),
	/* DEVICE 124 */
	HD_VIDEOPROC_124_CTRL = HD_VIDEOPROC_CTRL(124),
	/* DEVICE 125 */
	HD_VIDEOPROC_125_CTRL = HD_VIDEOPROC_CTRL(125),
	/* DEVICE 126 */
	HD_VIDEOPROC_126_CTRL = HD_VIDEOPROC_CTRL(126),
	/* DEVICE 127 */
	HD_VIDEOPROC_127_CTRL = HD_VIDEOPROC_CTRL(127),
	/* DEVICE 128 */
	HD_VIDEOPROC_128_CTRL = HD_VIDEOPROC_CTRL(128),
	/* DEVICE 129 */
	HD_VIDEOPROC_129_CTRL = HD_VIDEOPROC_CTRL(129),
	/* DEVICE 130 */
	HD_VIDEOPROC_130_CTRL = HD_VIDEOPROC_CTRL(130),
	/* DEVICE 131 */
	HD_VIDEOPROC_131_CTRL = HD_VIDEOPROC_CTRL(131),
	/* DEVICE 132 */
	HD_VIDEOPROC_132_CTRL = HD_VIDEOPROC_CTRL(132),
	/* DEVICE 133 */
	HD_VIDEOPROC_133_CTRL = HD_VIDEOPROC_CTRL(133),
	/* DEVICE 134 */
	HD_VIDEOPROC_134_CTRL = HD_VIDEOPROC_CTRL(134),
	/* DEVICE 135 */
	HD_VIDEOPROC_135_CTRL = HD_VIDEOPROC_CTRL(135),
	/* DEVICE 136 */
	HD_VIDEOPROC_136_CTRL = HD_VIDEOPROC_CTRL(136),
	/* DEVICE 137 */
	HD_VIDEOPROC_137_CTRL = HD_VIDEOPROC_CTRL(137),
	/* DEVICE 138 */
	HD_VIDEOPROC_138_CTRL = HD_VIDEOPROC_CTRL(138),
	/* DEVICE 139 */
	HD_VIDEOPROC_139_CTRL = HD_VIDEOPROC_CTRL(139),
	/* DEVICE 140 */
	HD_VIDEOPROC_140_CTRL = HD_VIDEOPROC_CTRL(140),
	/* DEVICE 141 */
	HD_VIDEOPROC_141_CTRL = HD_VIDEOPROC_CTRL(141),
	/* DEVICE 142 */
	HD_VIDEOPROC_142_CTRL = HD_VIDEOPROC_CTRL(142),
	/* DEVICE 143 */
	HD_VIDEOPROC_143_CTRL = HD_VIDEOPROC_CTRL(143),
	/* DEVICE 144 */
	HD_VIDEOPROC_144_CTRL = HD_VIDEOPROC_CTRL(144),
	/* DEVICE 145 */
	HD_VIDEOPROC_145_CTRL = HD_VIDEOPROC_CTRL(145),
	/* DEVICE 146 */
	HD_VIDEOPROC_146_CTRL = HD_VIDEOPROC_CTRL(146),
	/* DEVICE 147 */
	HD_VIDEOPROC_147_CTRL = HD_VIDEOPROC_CTRL(147),
	/* DEVICE 148 */
	HD_VIDEOPROC_148_CTRL = HD_VIDEOPROC_CTRL(148),
	/* DEVICE 149 */
	HD_VIDEOPROC_149_CTRL = HD_VIDEOPROC_CTRL(149),
	/* DEVICE 150 */
	HD_VIDEOPROC_150_CTRL = HD_VIDEOPROC_CTRL(150),
	/* DEVICE 151 */
	HD_VIDEOPROC_151_CTRL = HD_VIDEOPROC_CTRL(151),
	/* DEVICE 152 */
	HD_VIDEOPROC_152_CTRL = HD_VIDEOPROC_CTRL(152),
	/* DEVICE 153 */
	HD_VIDEOPROC_153_CTRL = HD_VIDEOPROC_CTRL(153),
	/* DEVICE 154 */
	HD_VIDEOPROC_154_CTRL = HD_VIDEOPROC_CTRL(154),
	/* DEVICE 155 */
	HD_VIDEOPROC_155_CTRL = HD_VIDEOPROC_CTRL(155),
	/* DEVICE 156 */
	HD_VIDEOPROC_156_CTRL = HD_VIDEOPROC_CTRL(156),
	/* DEVICE 157 */
	HD_VIDEOPROC_157_CTRL = HD_VIDEOPROC_CTRL(157),
	/* DEVICE 158 */
	HD_VIDEOPROC_158_CTRL = HD_VIDEOPROC_CTRL(158),
	/* DEVICE 159 */
	HD_VIDEOPROC_159_CTRL = HD_VIDEOPROC_CTRL(159),
	/* DEVICE 160 */
	HD_VIDEOPROC_160_CTRL = HD_VIDEOPROC_CTRL(160),
	/* DEVICE 161 */
	HD_VIDEOPROC_161_CTRL = HD_VIDEOPROC_CTRL(161),
	/* DEVICE 162 */
	HD_VIDEOPROC_162_CTRL = HD_VIDEOPROC_CTRL(162),
	/* DEVICE 163 */
	HD_VIDEOPROC_163_CTRL = HD_VIDEOPROC_CTRL(163),
	/* DEVICE 164 */
	HD_VIDEOPROC_164_CTRL = HD_VIDEOPROC_CTRL(164),
	/* DEVICE 165 */
	HD_VIDEOPROC_165_CTRL = HD_VIDEOPROC_CTRL(165),
	/* DEVICE 166 */
	HD_VIDEOPROC_166_CTRL = HD_VIDEOPROC_CTRL(166),
	/* DEVICE 167 */
	HD_VIDEOPROC_167_CTRL = HD_VIDEOPROC_CTRL(167),
	/* DEVICE 168 */
	HD_VIDEOPROC_168_CTRL = HD_VIDEOPROC_CTRL(168),
	/* DEVICE 169 */
	HD_VIDEOPROC_169_CTRL = HD_VIDEOPROC_CTRL(169),
	/* DEVICE 170 */
	HD_VIDEOPROC_170_CTRL = HD_VIDEOPROC_CTRL(170),
	/* DEVICE 171 */
	HD_VIDEOPROC_171_CTRL = HD_VIDEOPROC_CTRL(171),
	/* DEVICE 172 */
	HD_VIDEOPROC_172_CTRL = HD_VIDEOPROC_CTRL(172),
	/* DEVICE 173 */
	HD_VIDEOPROC_173_CTRL = HD_VIDEOPROC_CTRL(173),
	/* DEVICE 174 */
	HD_VIDEOPROC_174_CTRL = HD_VIDEOPROC_CTRL(174),
	/* DEVICE 175 */
	HD_VIDEOPROC_175_CTRL = HD_VIDEOPROC_CTRL(175),
	/* DEVICE 176 */
	HD_VIDEOPROC_176_CTRL = HD_VIDEOPROC_CTRL(176),
	/* DEVICE 177 */
	HD_VIDEOPROC_177_CTRL = HD_VIDEOPROC_CTRL(177),
	/* DEVICE 178 */
	HD_VIDEOPROC_178_CTRL = HD_VIDEOPROC_CTRL(178),
	/* DEVICE 179 */
	HD_VIDEOPROC_179_CTRL = HD_VIDEOPROC_CTRL(179),
	/* DEVICE 180 */
	HD_VIDEOPROC_180_CTRL = HD_VIDEOPROC_CTRL(180),
	/* DEVICE 181 */
	HD_VIDEOPROC_181_CTRL = HD_VIDEOPROC_CTRL(181),
	/* DEVICE 182 */
	HD_VIDEOPROC_182_CTRL = HD_VIDEOPROC_CTRL(182),
	/* DEVICE 183 */
	HD_VIDEOPROC_183_CTRL = HD_VIDEOPROC_CTRL(183),
	/* DEVICE 184 */
	HD_VIDEOPROC_184_CTRL = HD_VIDEOPROC_CTRL(184),
	/* DEVICE 185 */
	HD_VIDEOPROC_185_CTRL = HD_VIDEOPROC_CTRL(185),
	/* DEVICE 186 */
	HD_VIDEOPROC_186_CTRL = HD_VIDEOPROC_CTRL(186),
	/* DEVICE 187 */
	HD_VIDEOPROC_187_CTRL = HD_VIDEOPROC_CTRL(187),
	/* DEVICE 188 */
	HD_VIDEOPROC_188_CTRL = HD_VIDEOPROC_CTRL(188),
	/* DEVICE 189 */
	HD_VIDEOPROC_189_CTRL = HD_VIDEOPROC_CTRL(189),
	/* DEVICE 190 */
	HD_VIDEOPROC_190_CTRL = HD_VIDEOPROC_CTRL(190),
	/* DEVICE 191 */
	HD_VIDEOPROC_191_CTRL = HD_VIDEOPROC_CTRL(191),
	/* DEVICE 192 */
	HD_VIDEOPROC_192_CTRL = HD_VIDEOPROC_CTRL(192),
	/* DEVICE 193 */
	HD_VIDEOPROC_193_CTRL = HD_VIDEOPROC_CTRL(193),
	/* DEVICE 194 */
	HD_VIDEOPROC_194_CTRL = HD_VIDEOPROC_CTRL(194),
	/* DEVICE 195 */
	HD_VIDEOPROC_195_CTRL = HD_VIDEOPROC_CTRL(195),
	/* DEVICE 196 */
	HD_VIDEOPROC_196_CTRL = HD_VIDEOPROC_CTRL(196),
	/* DEVICE 197 */
	HD_VIDEOPROC_197_CTRL = HD_VIDEOPROC_CTRL(197),
	/* DEVICE 198 */
	HD_VIDEOPROC_198_CTRL = HD_VIDEOPROC_CTRL(198),
	/* DEVICE 199 */
	HD_VIDEOPROC_199_CTRL = HD_VIDEOPROC_CTRL(199),
	/* DEVICE 200 */
	HD_VIDEOPROC_200_CTRL = HD_VIDEOPROC_CTRL(200),
	/* DEVICE 201 */
	HD_VIDEOPROC_201_CTRL = HD_VIDEOPROC_CTRL(201),
	/* DEVICE 202 */
	HD_VIDEOPROC_202_CTRL = HD_VIDEOPROC_CTRL(202),
	/* DEVICE 203 */
	HD_VIDEOPROC_203_CTRL = HD_VIDEOPROC_CTRL(203),
	/* DEVICE 204 */
	HD_VIDEOPROC_204_CTRL = HD_VIDEOPROC_CTRL(204),
	/* DEVICE 205 */
	HD_VIDEOPROC_205_CTRL = HD_VIDEOPROC_CTRL(205),
	/* DEVICE 206 */
	HD_VIDEOPROC_206_CTRL = HD_VIDEOPROC_CTRL(206),
	/* DEVICE 207 */
	HD_VIDEOPROC_207_CTRL = HD_VIDEOPROC_CTRL(207),
	/* DEVICE 208 */
	HD_VIDEOPROC_208_CTRL = HD_VIDEOPROC_CTRL(208),
	/* DEVICE 209 */
	HD_VIDEOPROC_209_CTRL = HD_VIDEOPROC_CTRL(209),
	/* DEVICE 210 */
	HD_VIDEOPROC_210_CTRL = HD_VIDEOPROC_CTRL(210),
	/* DEVICE 211 */
	HD_VIDEOPROC_211_CTRL = HD_VIDEOPROC_CTRL(211),
	/* DEVICE 212 */
	HD_VIDEOPROC_212_CTRL = HD_VIDEOPROC_CTRL(212),
	/* DEVICE 213 */
	HD_VIDEOPROC_213_CTRL = HD_VIDEOPROC_CTRL(213),
	/* DEVICE 214 */
	HD_VIDEOPROC_214_CTRL = HD_VIDEOPROC_CTRL(214),
	/* DEVICE 215 */
	HD_VIDEOPROC_215_CTRL = HD_VIDEOPROC_CTRL(215),
	/* DEVICE 216 */
	HD_VIDEOPROC_216_CTRL = HD_VIDEOPROC_CTRL(216),
	/* DEVICE 217 */
	HD_VIDEOPROC_217_CTRL = HD_VIDEOPROC_CTRL(217),
	/* DEVICE 218 */
	HD_VIDEOPROC_218_CTRL = HD_VIDEOPROC_CTRL(218),
	/* DEVICE 219 */
	HD_VIDEOPROC_219_CTRL = HD_VIDEOPROC_CTRL(219),
	/* DEVICE 220 */
	HD_VIDEOPROC_220_CTRL = HD_VIDEOPROC_CTRL(220),
	/* DEVICE 221 */
	HD_VIDEOPROC_221_CTRL = HD_VIDEOPROC_CTRL(221),
	/* DEVICE 222 */
	HD_VIDEOPROC_222_CTRL = HD_VIDEOPROC_CTRL(222),
	/* DEVICE 223 */
	HD_VIDEOPROC_223_CTRL = HD_VIDEOPROC_CTRL(223),
	/* DEVICE 224 */
	HD_VIDEOPROC_224_CTRL = HD_VIDEOPROC_CTRL(224),
	/* DEVICE 225 */
	HD_VIDEOPROC_225_CTRL = HD_VIDEOPROC_CTRL(225),
	/* DEVICE 226 */
	HD_VIDEOPROC_226_CTRL = HD_VIDEOPROC_CTRL(226),
	/* DEVICE 227 */
	HD_VIDEOPROC_227_CTRL = HD_VIDEOPROC_CTRL(227),
	/* DEVICE 228 */
	HD_VIDEOPROC_228_CTRL = HD_VIDEOPROC_CTRL(228),
	/* DEVICE 229 */
	HD_VIDEOPROC_229_CTRL = HD_VIDEOPROC_CTRL(229),
	/* DEVICE 230 */
	HD_VIDEOPROC_230_CTRL = HD_VIDEOPROC_CTRL(230),
	/* DEVICE 231 */
	HD_VIDEOPROC_231_CTRL = HD_VIDEOPROC_CTRL(231),
	/* DEVICE 232 */
	HD_VIDEOPROC_232_CTRL = HD_VIDEOPROC_CTRL(232),
	/* DEVICE 233 */
	HD_VIDEOPROC_233_CTRL = HD_VIDEOPROC_CTRL(233),
	/* DEVICE 234 */
	HD_VIDEOPROC_234_CTRL = HD_VIDEOPROC_CTRL(234),
	/* DEVICE 235 */
	HD_VIDEOPROC_235_CTRL = HD_VIDEOPROC_CTRL(235),
	/* DEVICE 236 */
	HD_VIDEOPROC_236_CTRL = HD_VIDEOPROC_CTRL(236),
	/* DEVICE 237 */
	HD_VIDEOPROC_237_CTRL = HD_VIDEOPROC_CTRL(237),
	/* DEVICE 238 */
	HD_VIDEOPROC_238_CTRL = HD_VIDEOPROC_CTRL(238),
	/* DEVICE 239 */
	HD_VIDEOPROC_239_CTRL = HD_VIDEOPROC_CTRL(239),
	/* DEVICE 240 */
	HD_VIDEOPROC_240_CTRL = HD_VIDEOPROC_CTRL(240),
	/* DEVICE 241 */
	HD_VIDEOPROC_241_CTRL = HD_VIDEOPROC_CTRL(241),
	/* DEVICE 242 */
	HD_VIDEOPROC_242_CTRL = HD_VIDEOPROC_CTRL(242),
	/* DEVICE 243 */
	HD_VIDEOPROC_243_CTRL = HD_VIDEOPROC_CTRL(243),
	/* DEVICE 244 */
	HD_VIDEOPROC_244_CTRL = HD_VIDEOPROC_CTRL(244),
	/* DEVICE 245 */
	HD_VIDEOPROC_245_CTRL = HD_VIDEOPROC_CTRL(245),
	/* DEVICE 246 */
	HD_VIDEOPROC_246_CTRL = HD_VIDEOPROC_CTRL(246),
	/* DEVICE 247 */
	HD_VIDEOPROC_247_CTRL = HD_VIDEOPROC_CTRL(247),
	/* DEVICE 248 */
	HD_VIDEOPROC_248_CTRL = HD_VIDEOPROC_CTRL(248),
	/* DEVICE 249 */
	HD_VIDEOPROC_249_CTRL = HD_VIDEOPROC_CTRL(249),
	/* DEVICE 250 */
	HD_VIDEOPROC_250_CTRL = HD_VIDEOPROC_CTRL(250),
	/* DEVICE 251 */
	HD_VIDEOPROC_251_CTRL = HD_VIDEOPROC_CTRL(251),
	/* DEVICE 252 */
	HD_VIDEOPROC_252_CTRL = HD_VIDEOPROC_CTRL(252),
	/* DEVICE 253 */
	HD_VIDEOPROC_253_CTRL = HD_VIDEOPROC_CTRL(253),
	/* DEVICE 254 */
	HD_VIDEOPROC_254_CTRL = HD_VIDEOPROC_CTRL(254),
	/* DEVICE 255 */
	HD_VIDEOPROC_255_CTRL = HD_VIDEOPROC_CTRL(255),
	/* DEVICE 256 */
	HD_VIDEOPROC_256_CTRL = HD_VIDEOPROC_CTRL(256),
	/* DEVICE 257 */
	HD_VIDEOPROC_257_CTRL = HD_VIDEOPROC_CTRL(257),
	/* DEVICE 258 */
	HD_VIDEOPROC_258_CTRL = HD_VIDEOPROC_CTRL(258),
	/* DEVICE 259 */
	HD_VIDEOPROC_259_CTRL = HD_VIDEOPROC_CTRL(259),
	/* DEVICE 260 */
	HD_VIDEOPROC_260_CTRL = HD_VIDEOPROC_CTRL(260),
	/* DEVICE 261 */
	HD_VIDEOPROC_261_CTRL = HD_VIDEOPROC_CTRL(261),
	/* DEVICE 262 */
	HD_VIDEOPROC_262_CTRL = HD_VIDEOPROC_CTRL(262),
	/* DEVICE 263 */
	HD_VIDEOPROC_263_CTRL = HD_VIDEOPROC_CTRL(263),
	/* DEVICE 264 */
	HD_VIDEOPROC_264_CTRL = HD_VIDEOPROC_CTRL(264),
	/* DEVICE 265 */
	HD_VIDEOPROC_265_CTRL = HD_VIDEOPROC_CTRL(265),
	/* DEVICE 266 */
	HD_VIDEOPROC_266_CTRL = HD_VIDEOPROC_CTRL(266),
	/* DEVICE 267 */
	HD_VIDEOPROC_267_CTRL = HD_VIDEOPROC_CTRL(267),
	/* DEVICE 268 */
	HD_VIDEOPROC_268_CTRL = HD_VIDEOPROC_CTRL(268),
	/* DEVICE 269 */
	HD_VIDEOPROC_269_CTRL = HD_VIDEOPROC_CTRL(269),
	/* DEVICE 270 */
	HD_VIDEOPROC_270_CTRL = HD_VIDEOPROC_CTRL(270),
	/* DEVICE 271 */
	HD_VIDEOPROC_271_CTRL = HD_VIDEOPROC_CTRL(271),
	/* DEVICE 272 */
	HD_VIDEOPROC_272_CTRL = HD_VIDEOPROC_CTRL(272),
	/* DEVICE 273 */
	HD_VIDEOPROC_273_CTRL = HD_VIDEOPROC_CTRL(273),
	/* DEVICE 274 */
	HD_VIDEOPROC_274_CTRL = HD_VIDEOPROC_CTRL(274),
	/* DEVICE 275 */
	HD_VIDEOPROC_275_CTRL = HD_VIDEOPROC_CTRL(275),
	/* DEVICE 276 */
	HD_VIDEOPROC_276_CTRL = HD_VIDEOPROC_CTRL(276),
	/* DEVICE 277 */
	HD_VIDEOPROC_277_CTRL = HD_VIDEOPROC_CTRL(277),
	/* DEVICE 278 */
	HD_VIDEOPROC_278_CTRL = HD_VIDEOPROC_CTRL(278),
	/* DEVICE 279 */
	HD_VIDEOPROC_279_CTRL = HD_VIDEOPROC_CTRL(279),
	/* DEVICE 280 */
	HD_VIDEOPROC_280_CTRL = HD_VIDEOPROC_CTRL(280),
	/* DEVICE 281 */
	HD_VIDEOPROC_281_CTRL = HD_VIDEOPROC_CTRL(281),
	/* DEVICE 282 */
	HD_VIDEOPROC_282_CTRL = HD_VIDEOPROC_CTRL(282),
	/* DEVICE 283 */
	HD_VIDEOPROC_283_CTRL = HD_VIDEOPROC_CTRL(283),
	/* DEVICE 284 */
	HD_VIDEOPROC_284_CTRL = HD_VIDEOPROC_CTRL(284),
	/* DEVICE 285 */
	HD_VIDEOPROC_285_CTRL = HD_VIDEOPROC_CTRL(285),
	/* DEVICE 286 */
	HD_VIDEOPROC_286_CTRL = HD_VIDEOPROC_CTRL(286),
	/* DEVICE 287 */
	HD_VIDEOPROC_287_CTRL = HD_VIDEOPROC_CTRL(287),
	/* DEVICE 288 */
	HD_VIDEOPROC_288_CTRL = HD_VIDEOPROC_CTRL(288),
	/* DEVICE 289 */
	HD_VIDEOPROC_289_CTRL = HD_VIDEOPROC_CTRL(289),
	/* DEVICE 290 */
	HD_VIDEOPROC_290_CTRL = HD_VIDEOPROC_CTRL(290),
	/* DEVICE 291 */
	HD_VIDEOPROC_291_CTRL = HD_VIDEOPROC_CTRL(291),
	/* DEVICE 292 */
	HD_VIDEOPROC_292_CTRL = HD_VIDEOPROC_CTRL(292),
	/* DEVICE 293 */
	HD_VIDEOPROC_293_CTRL = HD_VIDEOPROC_CTRL(293),
	/* DEVICE 294 */
	HD_VIDEOPROC_294_CTRL = HD_VIDEOPROC_CTRL(294),
	/* DEVICE 295 */
	HD_VIDEOPROC_295_CTRL = HD_VIDEOPROC_CTRL(295),
	/* DEVICE 296 */
	HD_VIDEOPROC_296_CTRL = HD_VIDEOPROC_CTRL(296),
	/* DEVICE 297 */
	HD_VIDEOPROC_297_CTRL = HD_VIDEOPROC_CTRL(297),
	/* DEVICE 298 */
	HD_VIDEOPROC_298_CTRL = HD_VIDEOPROC_CTRL(298),
	/* DEVICE 299 */
	HD_VIDEOPROC_299_CTRL = HD_VIDEOPROC_CTRL(299),
	/* DEVICE 300 */
	HD_VIDEOPROC_300_CTRL = HD_VIDEOPROC_CTRL(300),
	/* DEVICE 301 */
	HD_VIDEOPROC_301_CTRL = HD_VIDEOPROC_CTRL(301),
	/* DEVICE 302 */
	HD_VIDEOPROC_302_CTRL = HD_VIDEOPROC_CTRL(302),
	/* DEVICE 303 */
	HD_VIDEOPROC_303_CTRL = HD_VIDEOPROC_CTRL(303),
	/* DEVICE 304 */
	HD_VIDEOPROC_304_CTRL = HD_VIDEOPROC_CTRL(304),
	/* DEVICE 305 */
	HD_VIDEOPROC_305_CTRL = HD_VIDEOPROC_CTRL(305),
	/* DEVICE 306 */
	HD_VIDEOPROC_306_CTRL = HD_VIDEOPROC_CTRL(306),
	/* DEVICE 307 */
	HD_VIDEOPROC_307_CTRL = HD_VIDEOPROC_CTRL(307),
	/* DEVICE 308 */
	HD_VIDEOPROC_308_CTRL = HD_VIDEOPROC_CTRL(308),
	/* DEVICE 309 */
	HD_VIDEOPROC_309_CTRL = HD_VIDEOPROC_CTRL(309),
	/* DEVICE 310 */
	HD_VIDEOPROC_310_CTRL = HD_VIDEOPROC_CTRL(310),
	/* DEVICE 311 */
	HD_VIDEOPROC_311_CTRL = HD_VIDEOPROC_CTRL(311),
	/* DEVICE 312 */
	HD_VIDEOPROC_312_CTRL = HD_VIDEOPROC_CTRL(312),
	/* DEVICE 313 */
	HD_VIDEOPROC_313_CTRL = HD_VIDEOPROC_CTRL(313),
	/* DEVICE 314 */
	HD_VIDEOPROC_314_CTRL = HD_VIDEOPROC_CTRL(314),
	/* DEVICE 315 */
	HD_VIDEOPROC_315_CTRL = HD_VIDEOPROC_CTRL(315),
	/* DEVICE 316 */
	HD_VIDEOPROC_316_CTRL = HD_VIDEOPROC_CTRL(316),
	/* DEVICE 317 */
	HD_VIDEOPROC_317_CTRL = HD_VIDEOPROC_CTRL(317),
	/* DEVICE 318 */
	HD_VIDEOPROC_318_CTRL = HD_VIDEOPROC_CTRL(318),
	/* DEVICE 319 */
	HD_VIDEOPROC_319_CTRL = HD_VIDEOPROC_CTRL(319),
	/* DEVICE 320 */
	HD_VIDEOPROC_320_CTRL = HD_VIDEOPROC_CTRL(320),
	/* DEVICE 321 */
	HD_VIDEOPROC_321_CTRL = HD_VIDEOPROC_CTRL(321),
	/* DEVICE 322 */
	HD_VIDEOPROC_322_CTRL = HD_VIDEOPROC_CTRL(322),
	/* DEVICE 323 */
	HD_VIDEOPROC_323_CTRL = HD_VIDEOPROC_CTRL(323),
	/* DEVICE 324 */
	HD_VIDEOPROC_324_CTRL = HD_VIDEOPROC_CTRL(324),
	/* DEVICE 325 */
	HD_VIDEOPROC_325_CTRL = HD_VIDEOPROC_CTRL(325),
	/* DEVICE 326 */
	HD_VIDEOPROC_326_CTRL = HD_VIDEOPROC_CTRL(326),
	/* DEVICE 327 */
	HD_VIDEOPROC_327_CTRL = HD_VIDEOPROC_CTRL(327),
	/* DEVICE 328 */
	HD_VIDEOPROC_328_CTRL = HD_VIDEOPROC_CTRL(328),
	/* DEVICE 329 */
	HD_VIDEOPROC_329_CTRL = HD_VIDEOPROC_CTRL(329),
	/* DEVICE 330 */
	HD_VIDEOPROC_330_CTRL = HD_VIDEOPROC_CTRL(330),
	/* DEVICE 331 */
	HD_VIDEOPROC_331_CTRL = HD_VIDEOPROC_CTRL(331),
	/* DEVICE 332 */
	HD_VIDEOPROC_332_CTRL = HD_VIDEOPROC_CTRL(332),
	/* DEVICE 333 */
	HD_VIDEOPROC_333_CTRL = HD_VIDEOPROC_CTRL(333),
	/* DEVICE 334 */
	HD_VIDEOPROC_334_CTRL = HD_VIDEOPROC_CTRL(334),
	/* DEVICE 335 */
	HD_VIDEOPROC_335_CTRL = HD_VIDEOPROC_CTRL(335),
	/* DEVICE 336 */
	HD_VIDEOPROC_336_CTRL = HD_VIDEOPROC_CTRL(336),
	/* DEVICE 337 */
	HD_VIDEOPROC_337_CTRL = HD_VIDEOPROC_CTRL(337),
	/* DEVICE 338 */
	HD_VIDEOPROC_338_CTRL = HD_VIDEOPROC_CTRL(338),
	/* DEVICE 339 */
	HD_VIDEOPROC_339_CTRL = HD_VIDEOPROC_CTRL(339),
	/* DEVICE 340 */
	HD_VIDEOPROC_340_CTRL = HD_VIDEOPROC_CTRL(340),
	/* DEVICE 341 */
	HD_VIDEOPROC_341_CTRL = HD_VIDEOPROC_CTRL(341),
	/* DEVICE 342 */
	HD_VIDEOPROC_342_CTRL = HD_VIDEOPROC_CTRL(342),
	/* DEVICE 343 */
	HD_VIDEOPROC_343_CTRL = HD_VIDEOPROC_CTRL(343),
	/* DEVICE 344 */
	HD_VIDEOPROC_344_CTRL = HD_VIDEOPROC_CTRL(344),
	/* DEVICE 345 */
	HD_VIDEOPROC_345_CTRL = HD_VIDEOPROC_CTRL(345),
	/* DEVICE 346 */
	HD_VIDEOPROC_346_CTRL = HD_VIDEOPROC_CTRL(346),
	/* DEVICE 347 */
	HD_VIDEOPROC_347_CTRL = HD_VIDEOPROC_CTRL(347),
	/* DEVICE 348 */
	HD_VIDEOPROC_348_CTRL = HD_VIDEOPROC_CTRL(348),
	/* DEVICE 349 */
	HD_VIDEOPROC_349_CTRL = HD_VIDEOPROC_CTRL(349),
	/* DEVICE 350 */
	HD_VIDEOPROC_350_CTRL = HD_VIDEOPROC_CTRL(350),
	/* DEVICE 351 */
	HD_VIDEOPROC_351_CTRL = HD_VIDEOPROC_CTRL(351),
	/* DEVICE 352 */
	HD_VIDEOPROC_352_CTRL = HD_VIDEOPROC_CTRL(352),
	/* DEVICE 353 */
	HD_VIDEOPROC_353_CTRL = HD_VIDEOPROC_CTRL(353),
	/* DEVICE 354 */
	HD_VIDEOPROC_354_CTRL = HD_VIDEOPROC_CTRL(354),
	/* DEVICE 355 */
	HD_VIDEOPROC_355_CTRL = HD_VIDEOPROC_CTRL(355),
	/* DEVICE 356 */
	HD_VIDEOPROC_356_CTRL = HD_VIDEOPROC_CTRL(356),
	/* DEVICE 357 */
	HD_VIDEOPROC_357_CTRL = HD_VIDEOPROC_CTRL(357),
	/* DEVICE 358 */
	HD_VIDEOPROC_358_CTRL = HD_VIDEOPROC_CTRL(358),
	/* DEVICE 359 */
	HD_VIDEOPROC_359_CTRL = HD_VIDEOPROC_CTRL(359),
	/* DEVICE 360 */
	HD_VIDEOPROC_360_CTRL = HD_VIDEOPROC_CTRL(360),
	/* DEVICE 361 */
	HD_VIDEOPROC_361_CTRL = HD_VIDEOPROC_CTRL(361),
	/* DEVICE 362 */
	HD_VIDEOPROC_362_CTRL = HD_VIDEOPROC_CTRL(362),
	/* DEVICE 363 */
	HD_VIDEOPROC_363_CTRL = HD_VIDEOPROC_CTRL(363),
	/* DEVICE 364 */
	HD_VIDEOPROC_364_CTRL = HD_VIDEOPROC_CTRL(364),
	/* DEVICE 365 */
	HD_VIDEOPROC_365_CTRL = HD_VIDEOPROC_CTRL(365),
	/* DEVICE 366 */
	HD_VIDEOPROC_366_CTRL = HD_VIDEOPROC_CTRL(366),
	/* DEVICE 367 */
	HD_VIDEOPROC_367_CTRL = HD_VIDEOPROC_CTRL(367),
	/* DEVICE 368 */
	HD_VIDEOPROC_368_CTRL = HD_VIDEOPROC_CTRL(368),
	/* DEVICE 369 */
	HD_VIDEOPROC_369_CTRL = HD_VIDEOPROC_CTRL(369),
	/* DEVICE 370 */
	HD_VIDEOPROC_370_CTRL = HD_VIDEOPROC_CTRL(370),
	/* DEVICE 371 */
	HD_VIDEOPROC_371_CTRL = HD_VIDEOPROC_CTRL(371),
	/* DEVICE 372 */
	HD_VIDEOPROC_372_CTRL = HD_VIDEOPROC_CTRL(372),
	/* DEVICE 373 */
	HD_VIDEOPROC_373_CTRL = HD_VIDEOPROC_CTRL(373),
	/* DEVICE 374 */
	HD_VIDEOPROC_374_CTRL = HD_VIDEOPROC_CTRL(374),
	/* DEVICE 375 */
	HD_VIDEOPROC_375_CTRL = HD_VIDEOPROC_CTRL(375),
	/* DEVICE 376 */
	HD_VIDEOPROC_376_CTRL = HD_VIDEOPROC_CTRL(376),
	/* DEVICE 377 */
	HD_VIDEOPROC_377_CTRL = HD_VIDEOPROC_CTRL(377),
	/* DEVICE 378 */
	HD_VIDEOPROC_378_CTRL = HD_VIDEOPROC_CTRL(378),
	/* DEVICE 379 */
	HD_VIDEOPROC_379_CTRL = HD_VIDEOPROC_CTRL(379),
	/* DEVICE 380 */
	HD_VIDEOPROC_380_CTRL = HD_VIDEOPROC_CTRL(380),
	/* DEVICE 381 */
	HD_VIDEOPROC_381_CTRL = HD_VIDEOPROC_CTRL(381),
	/* DEVICE 382 */
	HD_VIDEOPROC_382_CTRL = HD_VIDEOPROC_CTRL(382),
	/* DEVICE 383 */
	HD_VIDEOPROC_383_CTRL = HD_VIDEOPROC_CTRL(383),
	/* DEVICE 384 */
	HD_VIDEOPROC_384_CTRL = HD_VIDEOPROC_CTRL(384),
	/* DEVICE 385 */
	HD_VIDEOPROC_385_CTRL = HD_VIDEOPROC_CTRL(385),
	/* DEVICE 386 */
	HD_VIDEOPROC_386_CTRL = HD_VIDEOPROC_CTRL(386),
	/* DEVICE 387 */
	HD_VIDEOPROC_387_CTRL = HD_VIDEOPROC_CTRL(387),
	/* DEVICE 388 */
	HD_VIDEOPROC_388_CTRL = HD_VIDEOPROC_CTRL(388),
	/* DEVICE 389 */
	HD_VIDEOPROC_389_CTRL = HD_VIDEOPROC_CTRL(389),
	/* DEVICE 390 */
	HD_VIDEOPROC_390_CTRL = HD_VIDEOPROC_CTRL(390),
	/* DEVICE 391 */
	HD_VIDEOPROC_391_CTRL = HD_VIDEOPROC_CTRL(391),
	/* DEVICE 392 */
	HD_VIDEOPROC_392_CTRL = HD_VIDEOPROC_CTRL(392),
	/* DEVICE 393 */
	HD_VIDEOPROC_393_CTRL = HD_VIDEOPROC_CTRL(393),
	/* DEVICE 394 */
	HD_VIDEOPROC_394_CTRL = HD_VIDEOPROC_CTRL(394),
	/* DEVICE 395 */
	HD_VIDEOPROC_395_CTRL = HD_VIDEOPROC_CTRL(395),
	/* DEVICE 396 */
	HD_VIDEOPROC_396_CTRL = HD_VIDEOPROC_CTRL(396),
	/* DEVICE 397 */
	HD_VIDEOPROC_397_CTRL = HD_VIDEOPROC_CTRL(397),
	/* DEVICE 398 */
	HD_VIDEOPROC_398_CTRL = HD_VIDEOPROC_CTRL(398),
	/* DEVICE 399 */
	HD_VIDEOPROC_399_CTRL = HD_VIDEOPROC_CTRL(399),
	/* DEVICE 400 */
	HD_VIDEOPROC_400_CTRL = HD_VIDEOPROC_CTRL(400),
	/* DEVICE 401 */
	HD_VIDEOPROC_401_CTRL = HD_VIDEOPROC_CTRL(401),
	/* DEVICE 402 */
	HD_VIDEOPROC_402_CTRL = HD_VIDEOPROC_CTRL(402),
	/* DEVICE 403 */
	HD_VIDEOPROC_403_CTRL = HD_VIDEOPROC_CTRL(403),
	/* DEVICE 404 */
	HD_VIDEOPROC_404_CTRL = HD_VIDEOPROC_CTRL(404),
	/* DEVICE 405 */
	HD_VIDEOPROC_405_CTRL = HD_VIDEOPROC_CTRL(405),
	/* DEVICE 406 */
	HD_VIDEOPROC_406_CTRL = HD_VIDEOPROC_CTRL(406),
	/* DEVICE 407 */
	HD_VIDEOPROC_407_CTRL = HD_VIDEOPROC_CTRL(407),
	/* DEVICE 408 */
	HD_VIDEOPROC_408_CTRL = HD_VIDEOPROC_CTRL(408),
	/* DEVICE 409 */
	HD_VIDEOPROC_409_CTRL = HD_VIDEOPROC_CTRL(409),
	/* DEVICE 410 */
	HD_VIDEOPROC_410_CTRL = HD_VIDEOPROC_CTRL(410),
	/* DEVICE 411 */
	HD_VIDEOPROC_411_CTRL = HD_VIDEOPROC_CTRL(411),
	/* DEVICE 412 */
	HD_VIDEOPROC_412_CTRL = HD_VIDEOPROC_CTRL(412),
	/* DEVICE 413 */
	HD_VIDEOPROC_413_CTRL = HD_VIDEOPROC_CTRL(413),
	/* DEVICE 414 */
	HD_VIDEOPROC_414_CTRL = HD_VIDEOPROC_CTRL(414),
	/* DEVICE 415 */
	HD_VIDEOPROC_415_CTRL = HD_VIDEOPROC_CTRL(415),
	/* DEVICE 416 */
	HD_VIDEOPROC_416_CTRL = HD_VIDEOPROC_CTRL(416),
	/* DEVICE 417 */
	HD_VIDEOPROC_417_CTRL = HD_VIDEOPROC_CTRL(417),
	/* DEVICE 418 */
	HD_VIDEOPROC_418_CTRL = HD_VIDEOPROC_CTRL(418),
	/* DEVICE 419 */
	HD_VIDEOPROC_419_CTRL = HD_VIDEOPROC_CTRL(419),
	/* DEVICE 420 */
	HD_VIDEOPROC_420_CTRL = HD_VIDEOPROC_CTRL(420),
	/* DEVICE 421 */
	HD_VIDEOPROC_421_CTRL = HD_VIDEOPROC_CTRL(421),
	/* DEVICE 422 */
	HD_VIDEOPROC_422_CTRL = HD_VIDEOPROC_CTRL(422),
	/* DEVICE 423 */
	HD_VIDEOPROC_423_CTRL = HD_VIDEOPROC_CTRL(423),
	/* DEVICE 424 */
	HD_VIDEOPROC_424_CTRL = HD_VIDEOPROC_CTRL(424),
	/* DEVICE 425 */
	HD_VIDEOPROC_425_CTRL = HD_VIDEOPROC_CTRL(425),
	/* DEVICE 426 */
	HD_VIDEOPROC_426_CTRL = HD_VIDEOPROC_CTRL(426),
	/* DEVICE 427 */
	HD_VIDEOPROC_427_CTRL = HD_VIDEOPROC_CTRL(427),
	/* DEVICE 428 */
	HD_VIDEOPROC_428_CTRL = HD_VIDEOPROC_CTRL(428),
	/* DEVICE 429 */
	HD_VIDEOPROC_429_CTRL = HD_VIDEOPROC_CTRL(429),
	/* DEVICE 430 */
	HD_VIDEOPROC_430_CTRL = HD_VIDEOPROC_CTRL(430),
	/* DEVICE 431 */
	HD_VIDEOPROC_431_CTRL = HD_VIDEOPROC_CTRL(431),
	/* DEVICE 432 */
	HD_VIDEOPROC_432_CTRL = HD_VIDEOPROC_CTRL(432),
	/* DEVICE 433 */
	HD_VIDEOPROC_433_CTRL = HD_VIDEOPROC_CTRL(433),
	/* DEVICE 434 */
	HD_VIDEOPROC_434_CTRL = HD_VIDEOPROC_CTRL(434),
	/* DEVICE 435 */
	HD_VIDEOPROC_435_CTRL = HD_VIDEOPROC_CTRL(435),
	/* DEVICE 436 */
	HD_VIDEOPROC_436_CTRL = HD_VIDEOPROC_CTRL(436),
	/* DEVICE 437 */
	HD_VIDEOPROC_437_CTRL = HD_VIDEOPROC_CTRL(437),
	/* DEVICE 438 */
	HD_VIDEOPROC_438_CTRL = HD_VIDEOPROC_CTRL(438),
	/* DEVICE 439 */
	HD_VIDEOPROC_439_CTRL = HD_VIDEOPROC_CTRL(439),
	/* DEVICE 440 */
	HD_VIDEOPROC_440_CTRL = HD_VIDEOPROC_CTRL(440),
	/* DEVICE 441 */
	HD_VIDEOPROC_441_CTRL = HD_VIDEOPROC_CTRL(441),
	/* DEVICE 442 */
	HD_VIDEOPROC_442_CTRL = HD_VIDEOPROC_CTRL(442),
	/* DEVICE 443 */
	HD_VIDEOPROC_443_CTRL = HD_VIDEOPROC_CTRL(443),
	/* DEVICE 444 */
	HD_VIDEOPROC_444_CTRL = HD_VIDEOPROC_CTRL(444),
	/* DEVICE 445 */
	HD_VIDEOPROC_445_CTRL = HD_VIDEOPROC_CTRL(445),
	/* DEVICE 446 */
	HD_VIDEOPROC_446_CTRL = HD_VIDEOPROC_CTRL(446),
	/* DEVICE 447 */
	HD_VIDEOPROC_447_CTRL = HD_VIDEOPROC_CTRL(447),
	/* DEVICE 448 */
	HD_VIDEOPROC_448_CTRL = HD_VIDEOPROC_CTRL(448),
	/* DEVICE 449 */
	HD_VIDEOPROC_449_CTRL = HD_VIDEOPROC_CTRL(449),
	/* DEVICE 450 */
	HD_VIDEOPROC_450_CTRL = HD_VIDEOPROC_CTRL(450),
	/* DEVICE 451 */
	HD_VIDEOPROC_451_CTRL = HD_VIDEOPROC_CTRL(451),
	/* DEVICE 452 */
	HD_VIDEOPROC_452_CTRL = HD_VIDEOPROC_CTRL(452),
	/* DEVICE 453 */
	HD_VIDEOPROC_453_CTRL = HD_VIDEOPROC_CTRL(453),
	/* DEVICE 454 */
	HD_VIDEOPROC_454_CTRL = HD_VIDEOPROC_CTRL(454),
	/* DEVICE 455 */
	HD_VIDEOPROC_455_CTRL = HD_VIDEOPROC_CTRL(455),
	/* DEVICE 456 */
	HD_VIDEOPROC_456_CTRL = HD_VIDEOPROC_CTRL(456),
	/* DEVICE 457 */
	HD_VIDEOPROC_457_CTRL = HD_VIDEOPROC_CTRL(457),
	/* DEVICE 458 */
	HD_VIDEOPROC_458_CTRL = HD_VIDEOPROC_CTRL(458),
	/* DEVICE 459 */
	HD_VIDEOPROC_459_CTRL = HD_VIDEOPROC_CTRL(459),
	/* DEVICE 460 */
	HD_VIDEOPROC_460_CTRL = HD_VIDEOPROC_CTRL(460),
	/* DEVICE 461 */
	HD_VIDEOPROC_461_CTRL = HD_VIDEOPROC_CTRL(461),
	/* DEVICE 462 */
	HD_VIDEOPROC_462_CTRL = HD_VIDEOPROC_CTRL(462),
	/* DEVICE 463 */
	HD_VIDEOPROC_463_CTRL = HD_VIDEOPROC_CTRL(463),
	/* DEVICE 464 */
	HD_VIDEOPROC_464_CTRL = HD_VIDEOPROC_CTRL(464),
	/* DEVICE 465 */
	HD_VIDEOPROC_465_CTRL = HD_VIDEOPROC_CTRL(465),
	/* DEVICE 466 */
	HD_VIDEOPROC_466_CTRL = HD_VIDEOPROC_CTRL(466),
	/* DEVICE 467 */
	HD_VIDEOPROC_467_CTRL = HD_VIDEOPROC_CTRL(467),
	/* DEVICE 468 */
	HD_VIDEOPROC_468_CTRL = HD_VIDEOPROC_CTRL(468),
	/* DEVICE 469 */
	HD_VIDEOPROC_469_CTRL = HD_VIDEOPROC_CTRL(469),
	/* DEVICE 470 */
	HD_VIDEOPROC_470_CTRL = HD_VIDEOPROC_CTRL(470),
	/* DEVICE 471 */
	HD_VIDEOPROC_471_CTRL = HD_VIDEOPROC_CTRL(471),
	/* DEVICE 472 */
	HD_VIDEOPROC_472_CTRL = HD_VIDEOPROC_CTRL(472),
	/* DEVICE 473 */
	HD_VIDEOPROC_473_CTRL = HD_VIDEOPROC_CTRL(473),
	/* DEVICE 474 */
	HD_VIDEOPROC_474_CTRL = HD_VIDEOPROC_CTRL(474),
	/* DEVICE 475 */
	HD_VIDEOPROC_475_CTRL = HD_VIDEOPROC_CTRL(475),
	/* DEVICE 476 */
	HD_VIDEOPROC_476_CTRL = HD_VIDEOPROC_CTRL(476),
	/* DEVICE 477 */
	HD_VIDEOPROC_477_CTRL = HD_VIDEOPROC_CTRL(477),
	/* DEVICE 478 */
	HD_VIDEOPROC_478_CTRL = HD_VIDEOPROC_CTRL(478),
	/* DEVICE 479 */
	HD_VIDEOPROC_479_CTRL = HD_VIDEOPROC_CTRL(479),
	/* DEVICE 480 */
	HD_VIDEOPROC_480_CTRL = HD_VIDEOPROC_CTRL(480),
	/* DEVICE 481 */
	HD_VIDEOPROC_481_CTRL = HD_VIDEOPROC_CTRL(481),
	/* DEVICE 482 */
	HD_VIDEOPROC_482_CTRL = HD_VIDEOPROC_CTRL(482),
	/* DEVICE 483 */
	HD_VIDEOPROC_483_CTRL = HD_VIDEOPROC_CTRL(483),
	/* DEVICE 484 */
	HD_VIDEOPROC_484_CTRL = HD_VIDEOPROC_CTRL(484),
	/* DEVICE 485 */
	HD_VIDEOPROC_485_CTRL = HD_VIDEOPROC_CTRL(485),
	/* DEVICE 486 */
	HD_VIDEOPROC_486_CTRL = HD_VIDEOPROC_CTRL(486),
	/* DEVICE 487 */
	HD_VIDEOPROC_487_CTRL = HD_VIDEOPROC_CTRL(487),
	/* DEVICE 488 */
	HD_VIDEOPROC_488_CTRL = HD_VIDEOPROC_CTRL(488),
	/* DEVICE 489 */
	HD_VIDEOPROC_489_CTRL = HD_VIDEOPROC_CTRL(489),
	/* DEVICE 490 */
	HD_VIDEOPROC_490_CTRL = HD_VIDEOPROC_CTRL(490),
	/* DEVICE 491 */
	HD_VIDEOPROC_491_CTRL = HD_VIDEOPROC_CTRL(491),
	/* DEVICE 492 */
	HD_VIDEOPROC_492_CTRL = HD_VIDEOPROC_CTRL(492),
	/* DEVICE 493 */
	HD_VIDEOPROC_493_CTRL = HD_VIDEOPROC_CTRL(493),
	/* DEVICE 494 */
	HD_VIDEOPROC_494_CTRL = HD_VIDEOPROC_CTRL(494),
	/* DEVICE 495 */
	HD_VIDEOPROC_495_CTRL = HD_VIDEOPROC_CTRL(495),
	/* DEVICE 496 */
	HD_VIDEOPROC_496_CTRL = HD_VIDEOPROC_CTRL(496),
	/* DEVICE 497 */
	HD_VIDEOPROC_497_CTRL = HD_VIDEOPROC_CTRL(497),
	/* DEVICE 498 */
	HD_VIDEOPROC_498_CTRL = HD_VIDEOPROC_CTRL(498),
	/* DEVICE 499 */
	HD_VIDEOPROC_499_CTRL = HD_VIDEOPROC_CTRL(499),
	/* DEVICE 500 */
	HD_VIDEOPROC_500_CTRL = HD_VIDEOPROC_CTRL(500),
	/* DEVICE 501 */
	HD_VIDEOPROC_501_CTRL = HD_VIDEOPROC_CTRL(501),
	/* DEVICE 502 */
	HD_VIDEOPROC_502_CTRL = HD_VIDEOPROC_CTRL(502),
	/* DEVICE 503 */
	HD_VIDEOPROC_503_CTRL = HD_VIDEOPROC_CTRL(503),
	/* DEVICE 504 */
	HD_VIDEOPROC_504_CTRL = HD_VIDEOPROC_CTRL(504),
	/* DEVICE 505 */
	HD_VIDEOPROC_505_CTRL = HD_VIDEOPROC_CTRL(505),
	/* DEVICE 506 */
	HD_VIDEOPROC_506_CTRL = HD_VIDEOPROC_CTRL(506),
	/* DEVICE 507 */
	HD_VIDEOPROC_507_CTRL = HD_VIDEOPROC_CTRL(507),
	/* DEVICE 508 */
	HD_VIDEOPROC_508_CTRL = HD_VIDEOPROC_CTRL(508),
	/* DEVICE 509 */
	HD_VIDEOPROC_509_CTRL = HD_VIDEOPROC_CTRL(509),
	/* DEVICE 510 */
	HD_VIDEOPROC_510_CTRL = HD_VIDEOPROC_CTRL(510),
	/* DEVICE 511 */
	HD_VIDEOPROC_511_CTRL = HD_VIDEOPROC_CTRL(511),


	/* DEVICE 0 */
	HD_AUDIOCAP_0_CTRL = HD_AUDIOCAP_CTRL(0),
	/* DEVICE 1 */
	HD_AUDIOCAP_1_CTRL = HD_AUDIOCAP_CTRL(1),
	/* DEVICE 2 */
	HD_AUDIOCAP_2_CTRL = HD_AUDIOCAP_CTRL(2),
	/* DEVICE 3 */
	HD_AUDIOCAP_3_CTRL = HD_AUDIOCAP_CTRL(3),


	/* DEVICE 0 */
	HD_AUDIOOUT_0_CTRL = HD_AUDIOOUT_CTRL(0),
	/* DEVICE 1 */
	HD_AUDIOOUT_1_CTRL = HD_AUDIOOUT_CTRL(1),
	/* DEVICE 2 */
	HD_AUDIOOUT_2_CTRL = HD_AUDIOOUT_CTRL(2),
	/* DEVICE 3 */
	HD_AUDIOOUT_3_CTRL = HD_AUDIOOUT_CTRL(3),


	/* DEVICE 0 */
	HD_AUDIOENC_0_CTRL = HD_AUDIOENC_CTRL(0),


	/* DEVICE 0 */
	HD_AUDIODEC_0_CTRL = HD_AUDIODEC_CTRL(0),


	ENUM_DUMMY4WORD(HD_CTRL_ID)
} HD_CTRL_ID;


/**
     @name type of device output id.
*/
typedef enum _HD_IN_ID {

	/* DEVICE 0 */
	HD_VIDEOCAP_0_IN_0 = HD_VIDEOCAP_IN(0, 0),
	/* DEVICE 1 */
	HD_VIDEOCAP_1_IN_0 = HD_VIDEOCAP_IN(1, 0),
	/* DEVICE 2 */
	HD_VIDEOCAP_2_IN_0 = HD_VIDEOCAP_IN(2, 0),
	/* DEVICE 3 */
	HD_VIDEOCAP_3_IN_0 = HD_VIDEOCAP_IN(3, 0),
	/* DEVICE 4 */
	HD_VIDEOCAP_4_IN_0 = HD_VIDEOCAP_IN(4, 0),
	/* DEVICE 5 */
	HD_VIDEOCAP_5_IN_0 = HD_VIDEOCAP_IN(5, 0),
	/* DEVICE 6 */
	HD_VIDEOCAP_6_IN_0 = HD_VIDEOCAP_IN(6, 0),
	/* DEVICE 7 */
	HD_VIDEOCAP_7_IN_0 = HD_VIDEOCAP_IN(7, 0),
	/* DEVICE 8 */
	HD_VIDEOCAP_8_IN_0 = HD_VIDEOCAP_IN(8, 0),
	/* DEVICE 9 */
	HD_VIDEOCAP_9_IN_0 = HD_VIDEOCAP_IN(9, 0),
	/* DEVICE 10 */
	HD_VIDEOCAP_10_IN_0 = HD_VIDEOCAP_IN(10, 0),
	/* DEVICE 11 */
	HD_VIDEOCAP_11_IN_0 = HD_VIDEOCAP_IN(11, 0),
	/* DEVICE 12 */
	HD_VIDEOCAP_12_IN_0 = HD_VIDEOCAP_IN(12, 0),
	/* DEVICE 13 */
	HD_VIDEOCAP_13_IN_0 = HD_VIDEOCAP_IN(13, 0),
	/* DEVICE 14 */
	HD_VIDEOCAP_14_IN_0 = HD_VIDEOCAP_IN(14, 0),
	/* DEVICE 15 */
	HD_VIDEOCAP_15_IN_0 = HD_VIDEOCAP_IN(15, 0),
	/* DEVICE 16 */
	HD_VIDEOCAP_16_IN_0 = HD_VIDEOCAP_IN(16, 0),
	/* DEVICE 17 */
	HD_VIDEOCAP_17_IN_0 = HD_VIDEOCAP_IN(17, 0),
	/* DEVICE 18 */
	HD_VIDEOCAP_18_IN_0 = HD_VIDEOCAP_IN(18, 0),
	/* DEVICE 19 */
	HD_VIDEOCAP_19_IN_0 = HD_VIDEOCAP_IN(19, 0),
	/* DEVICE 20 */
	HD_VIDEOCAP_20_IN_0 = HD_VIDEOCAP_IN(20, 0),
	/* DEVICE 21 */
	HD_VIDEOCAP_21_IN_0 = HD_VIDEOCAP_IN(21, 0),
	/* DEVICE 22 */
	HD_VIDEOCAP_22_IN_0 = HD_VIDEOCAP_IN(22, 0),
	/* DEVICE 23 */
	HD_VIDEOCAP_23_IN_0 = HD_VIDEOCAP_IN(23, 0),
	/* DEVICE 24 */
	HD_VIDEOCAP_24_IN_0 = HD_VIDEOCAP_IN(24, 0),
	/* DEVICE 25 */
	HD_VIDEOCAP_25_IN_0 = HD_VIDEOCAP_IN(25, 0),
	/* DEVICE 26 */
	HD_VIDEOCAP_26_IN_0 = HD_VIDEOCAP_IN(26, 0),
	/* DEVICE 27 */
	HD_VIDEOCAP_27_IN_0 = HD_VIDEOCAP_IN(27, 0),
	/* DEVICE 28 */
	HD_VIDEOCAP_28_IN_0 = HD_VIDEOCAP_IN(28, 0),
	/* DEVICE 29 */
	HD_VIDEOCAP_29_IN_0 = HD_VIDEOCAP_IN(29, 0),
	/* DEVICE 30 */
	HD_VIDEOCAP_30_IN_0 = HD_VIDEOCAP_IN(30, 0),
	/* DEVICE 31 */
	HD_VIDEOCAP_31_IN_0 = HD_VIDEOCAP_IN(31, 0),


	/* DEVICE 0 */
	HD_VIDEOOUT_0_IN_0 = HD_VIDEOOUT_IN(0, 0),
	HD_VIDEOOUT_0_IN_1 = HD_VIDEOOUT_IN(0, 1),
	HD_VIDEOOUT_0_IN_2 = HD_VIDEOOUT_IN(0, 2),
	HD_VIDEOOUT_0_IN_3 = HD_VIDEOOUT_IN(0, 3),
	HD_VIDEOOUT_0_IN_4 = HD_VIDEOOUT_IN(0, 4),
	HD_VIDEOOUT_0_IN_5 = HD_VIDEOOUT_IN(0, 5),
	HD_VIDEOOUT_0_IN_6 = HD_VIDEOOUT_IN(0, 6),
	HD_VIDEOOUT_0_IN_7 = HD_VIDEOOUT_IN(0, 7),
	HD_VIDEOOUT_0_IN_8 = HD_VIDEOOUT_IN(0, 8),
	HD_VIDEOOUT_0_IN_9 = HD_VIDEOOUT_IN(0, 9),
	HD_VIDEOOUT_0_IN_10 = HD_VIDEOOUT_IN(0, 10),
	HD_VIDEOOUT_0_IN_11 = HD_VIDEOOUT_IN(0, 11),
	HD_VIDEOOUT_0_IN_12 = HD_VIDEOOUT_IN(0, 12),
	HD_VIDEOOUT_0_IN_13 = HD_VIDEOOUT_IN(0, 13),
	HD_VIDEOOUT_0_IN_14 = HD_VIDEOOUT_IN(0, 14),
	HD_VIDEOOUT_0_IN_15 = HD_VIDEOOUT_IN(0, 15),
	HD_VIDEOOUT_0_IN_16 = HD_VIDEOOUT_IN(0, 16),
	HD_VIDEOOUT_0_IN_17 = HD_VIDEOOUT_IN(0, 17),
	HD_VIDEOOUT_0_IN_18 = HD_VIDEOOUT_IN(0, 18),
	HD_VIDEOOUT_0_IN_19 = HD_VIDEOOUT_IN(0, 19),
	HD_VIDEOOUT_0_IN_20 = HD_VIDEOOUT_IN(0, 20),
	HD_VIDEOOUT_0_IN_21 = HD_VIDEOOUT_IN(0, 21),
	HD_VIDEOOUT_0_IN_22 = HD_VIDEOOUT_IN(0, 22),
	HD_VIDEOOUT_0_IN_23 = HD_VIDEOOUT_IN(0, 23),
	HD_VIDEOOUT_0_IN_24 = HD_VIDEOOUT_IN(0, 24),
	HD_VIDEOOUT_0_IN_25 = HD_VIDEOOUT_IN(0, 25),
	HD_VIDEOOUT_0_IN_26 = HD_VIDEOOUT_IN(0, 26),
	HD_VIDEOOUT_0_IN_27 = HD_VIDEOOUT_IN(0, 27),
	HD_VIDEOOUT_0_IN_28 = HD_VIDEOOUT_IN(0, 28),
	HD_VIDEOOUT_0_IN_29 = HD_VIDEOOUT_IN(0, 29),
	HD_VIDEOOUT_0_IN_30 = HD_VIDEOOUT_IN(0, 30),
	HD_VIDEOOUT_0_IN_31 = HD_VIDEOOUT_IN(0, 31),
	HD_VIDEOOUT_0_IN_32 = HD_VIDEOOUT_IN(0, 32),
	HD_VIDEOOUT_0_IN_33 = HD_VIDEOOUT_IN(0, 33),
	HD_VIDEOOUT_0_IN_34 = HD_VIDEOOUT_IN(0, 34),
	HD_VIDEOOUT_0_IN_35 = HD_VIDEOOUT_IN(0, 35),
	HD_VIDEOOUT_0_IN_36 = HD_VIDEOOUT_IN(0, 36),
	HD_VIDEOOUT_0_IN_37 = HD_VIDEOOUT_IN(0, 37),
	HD_VIDEOOUT_0_IN_38 = HD_VIDEOOUT_IN(0, 38),
	HD_VIDEOOUT_0_IN_39 = HD_VIDEOOUT_IN(0, 39),
	HD_VIDEOOUT_0_IN_40 = HD_VIDEOOUT_IN(0, 40),
	HD_VIDEOOUT_0_IN_41 = HD_VIDEOOUT_IN(0, 41),
	HD_VIDEOOUT_0_IN_42 = HD_VIDEOOUT_IN(0, 42),
	HD_VIDEOOUT_0_IN_43 = HD_VIDEOOUT_IN(0, 43),
	HD_VIDEOOUT_0_IN_44 = HD_VIDEOOUT_IN(0, 44),
	HD_VIDEOOUT_0_IN_45 = HD_VIDEOOUT_IN(0, 45),
	HD_VIDEOOUT_0_IN_46 = HD_VIDEOOUT_IN(0, 46),
	HD_VIDEOOUT_0_IN_47 = HD_VIDEOOUT_IN(0, 47),
	HD_VIDEOOUT_0_IN_48 = HD_VIDEOOUT_IN(0, 48),
	HD_VIDEOOUT_0_IN_49 = HD_VIDEOOUT_IN(0, 49),
	HD_VIDEOOUT_0_IN_50 = HD_VIDEOOUT_IN(0, 50),
	HD_VIDEOOUT_0_IN_51 = HD_VIDEOOUT_IN(0, 51),
	HD_VIDEOOUT_0_IN_52 = HD_VIDEOOUT_IN(0, 52),
	HD_VIDEOOUT_0_IN_53 = HD_VIDEOOUT_IN(0, 53),
	HD_VIDEOOUT_0_IN_54 = HD_VIDEOOUT_IN(0, 54),
	HD_VIDEOOUT_0_IN_55 = HD_VIDEOOUT_IN(0, 55),
	HD_VIDEOOUT_0_IN_56 = HD_VIDEOOUT_IN(0, 56),
	HD_VIDEOOUT_0_IN_57 = HD_VIDEOOUT_IN(0, 57),
	HD_VIDEOOUT_0_IN_58 = HD_VIDEOOUT_IN(0, 58),
	HD_VIDEOOUT_0_IN_59 = HD_VIDEOOUT_IN(0, 59),
	HD_VIDEOOUT_0_IN_60 = HD_VIDEOOUT_IN(0, 60),
	HD_VIDEOOUT_0_IN_61 = HD_VIDEOOUT_IN(0, 61),
	HD_VIDEOOUT_0_IN_62 = HD_VIDEOOUT_IN(0, 62),
	HD_VIDEOOUT_0_IN_63 = HD_VIDEOOUT_IN(0, 63),
	/* DEVICE 1 */
	HD_VIDEOOUT_1_IN_0 = HD_VIDEOOUT_IN(1, 0),
	HD_VIDEOOUT_1_IN_1 = HD_VIDEOOUT_IN(1, 1),
	HD_VIDEOOUT_1_IN_2 = HD_VIDEOOUT_IN(1, 2),
	HD_VIDEOOUT_1_IN_3 = HD_VIDEOOUT_IN(1, 3),
	HD_VIDEOOUT_1_IN_4 = HD_VIDEOOUT_IN(1, 4),
	HD_VIDEOOUT_1_IN_5 = HD_VIDEOOUT_IN(1, 5),
	HD_VIDEOOUT_1_IN_6 = HD_VIDEOOUT_IN(1, 6),
	HD_VIDEOOUT_1_IN_7 = HD_VIDEOOUT_IN(1, 7),
	HD_VIDEOOUT_1_IN_8 = HD_VIDEOOUT_IN(1, 8),
	HD_VIDEOOUT_1_IN_9 = HD_VIDEOOUT_IN(1, 9),
	HD_VIDEOOUT_1_IN_10 = HD_VIDEOOUT_IN(1, 10),
	HD_VIDEOOUT_1_IN_11 = HD_VIDEOOUT_IN(1, 11),
	HD_VIDEOOUT_1_IN_12 = HD_VIDEOOUT_IN(1, 12),
	HD_VIDEOOUT_1_IN_13 = HD_VIDEOOUT_IN(1, 13),
	HD_VIDEOOUT_1_IN_14 = HD_VIDEOOUT_IN(1, 14),
	HD_VIDEOOUT_1_IN_15 = HD_VIDEOOUT_IN(1, 15),


	/* DEVICE 0 */
	HD_VIDEOENC_0_IN_0 = HD_VIDEOENC_IN(0, 0),
	HD_VIDEOENC_0_IN_1 = HD_VIDEOENC_IN(0, 1),
	HD_VIDEOENC_0_IN_2 = HD_VIDEOENC_IN(0, 2),
	HD_VIDEOENC_0_IN_3 = HD_VIDEOENC_IN(0, 3),
	HD_VIDEOENC_0_IN_4 = HD_VIDEOENC_IN(0, 4),
	HD_VIDEOENC_0_IN_5 = HD_VIDEOENC_IN(0, 5),
	HD_VIDEOENC_0_IN_6 = HD_VIDEOENC_IN(0, 6),
	HD_VIDEOENC_0_IN_7 = HD_VIDEOENC_IN(0, 7),
	HD_VIDEOENC_0_IN_8 = HD_VIDEOENC_IN(0, 8),
	HD_VIDEOENC_0_IN_9 = HD_VIDEOENC_IN(0, 9),
	HD_VIDEOENC_0_IN_10 = HD_VIDEOENC_IN(0, 10),
	HD_VIDEOENC_0_IN_11 = HD_VIDEOENC_IN(0, 11),
	HD_VIDEOENC_0_IN_12 = HD_VIDEOENC_IN(0, 12),
	HD_VIDEOENC_0_IN_13 = HD_VIDEOENC_IN(0, 13),
	HD_VIDEOENC_0_IN_14 = HD_VIDEOENC_IN(0, 14),
	HD_VIDEOENC_0_IN_15 = HD_VIDEOENC_IN(0, 15),
	HD_VIDEOENC_0_IN_16 = HD_VIDEOENC_IN(0, 16),
	HD_VIDEOENC_0_IN_17 = HD_VIDEOENC_IN(0, 17),
	HD_VIDEOENC_0_IN_18 = HD_VIDEOENC_IN(0, 18),
	HD_VIDEOENC_0_IN_19 = HD_VIDEOENC_IN(0, 19),
	HD_VIDEOENC_0_IN_20 = HD_VIDEOENC_IN(0, 20),
	HD_VIDEOENC_0_IN_21 = HD_VIDEOENC_IN(0, 21),
	HD_VIDEOENC_0_IN_22 = HD_VIDEOENC_IN(0, 22),
	HD_VIDEOENC_0_IN_23 = HD_VIDEOENC_IN(0, 23),
	HD_VIDEOENC_0_IN_24 = HD_VIDEOENC_IN(0, 24),
	HD_VIDEOENC_0_IN_25 = HD_VIDEOENC_IN(0, 25),
	HD_VIDEOENC_0_IN_26 = HD_VIDEOENC_IN(0, 26),
	HD_VIDEOENC_0_IN_27 = HD_VIDEOENC_IN(0, 27),
	HD_VIDEOENC_0_IN_28 = HD_VIDEOENC_IN(0, 28),
	HD_VIDEOENC_0_IN_29 = HD_VIDEOENC_IN(0, 29),
	HD_VIDEOENC_0_IN_30 = HD_VIDEOENC_IN(0, 30),
	HD_VIDEOENC_0_IN_31 = HD_VIDEOENC_IN(0, 31),
	HD_VIDEOENC_0_IN_32 = HD_VIDEOENC_IN(0, 32),
	HD_VIDEOENC_0_IN_33 = HD_VIDEOENC_IN(0, 33),
	HD_VIDEOENC_0_IN_34 = HD_VIDEOENC_IN(0, 34),
	HD_VIDEOENC_0_IN_35 = HD_VIDEOENC_IN(0, 35),
	HD_VIDEOENC_0_IN_36 = HD_VIDEOENC_IN(0, 36),
	HD_VIDEOENC_0_IN_37 = HD_VIDEOENC_IN(0, 37),
	HD_VIDEOENC_0_IN_38 = HD_VIDEOENC_IN(0, 38),
	HD_VIDEOENC_0_IN_39 = HD_VIDEOENC_IN(0, 39),
	HD_VIDEOENC_0_IN_40 = HD_VIDEOENC_IN(0, 40),
	HD_VIDEOENC_0_IN_41 = HD_VIDEOENC_IN(0, 41),
	HD_VIDEOENC_0_IN_42 = HD_VIDEOENC_IN(0, 42),
	HD_VIDEOENC_0_IN_43 = HD_VIDEOENC_IN(0, 43),
	HD_VIDEOENC_0_IN_44 = HD_VIDEOENC_IN(0, 44),
	HD_VIDEOENC_0_IN_45 = HD_VIDEOENC_IN(0, 45),
	HD_VIDEOENC_0_IN_46 = HD_VIDEOENC_IN(0, 46),
	HD_VIDEOENC_0_IN_47 = HD_VIDEOENC_IN(0, 47),
	HD_VIDEOENC_0_IN_48 = HD_VIDEOENC_IN(0, 48),
	HD_VIDEOENC_0_IN_49 = HD_VIDEOENC_IN(0, 49),
	HD_VIDEOENC_0_IN_50 = HD_VIDEOENC_IN(0, 50),
	HD_VIDEOENC_0_IN_51 = HD_VIDEOENC_IN(0, 51),
	HD_VIDEOENC_0_IN_52 = HD_VIDEOENC_IN(0, 52),
	HD_VIDEOENC_0_IN_53 = HD_VIDEOENC_IN(0, 53),
	HD_VIDEOENC_0_IN_54 = HD_VIDEOENC_IN(0, 54),
	HD_VIDEOENC_0_IN_55 = HD_VIDEOENC_IN(0, 55),
	HD_VIDEOENC_0_IN_56 = HD_VIDEOENC_IN(0, 56),
	HD_VIDEOENC_0_IN_57 = HD_VIDEOENC_IN(0, 57),
	HD_VIDEOENC_0_IN_58 = HD_VIDEOENC_IN(0, 58),
	HD_VIDEOENC_0_IN_59 = HD_VIDEOENC_IN(0, 59),
	HD_VIDEOENC_0_IN_60 = HD_VIDEOENC_IN(0, 60),
	HD_VIDEOENC_0_IN_61 = HD_VIDEOENC_IN(0, 61),
	HD_VIDEOENC_0_IN_62 = HD_VIDEOENC_IN(0, 62),
	HD_VIDEOENC_0_IN_63 = HD_VIDEOENC_IN(0, 63),
	HD_VIDEOENC_0_IN_64 = HD_VIDEOENC_IN(0, 64),
	HD_VIDEOENC_0_IN_65 = HD_VIDEOENC_IN(0, 65),
	HD_VIDEOENC_0_IN_66 = HD_VIDEOENC_IN(0, 66),
	HD_VIDEOENC_0_IN_67 = HD_VIDEOENC_IN(0, 67),
	HD_VIDEOENC_0_IN_68 = HD_VIDEOENC_IN(0, 68),
	HD_VIDEOENC_0_IN_69 = HD_VIDEOENC_IN(0, 69),
	HD_VIDEOENC_0_IN_70 = HD_VIDEOENC_IN(0, 70),
	HD_VIDEOENC_0_IN_71 = HD_VIDEOENC_IN(0, 71),
	HD_VIDEOENC_0_IN_72 = HD_VIDEOENC_IN(0, 72),
	HD_VIDEOENC_0_IN_73 = HD_VIDEOENC_IN(0, 73),
	HD_VIDEOENC_0_IN_74 = HD_VIDEOENC_IN(0, 74),
	HD_VIDEOENC_0_IN_75 = HD_VIDEOENC_IN(0, 75),
	HD_VIDEOENC_0_IN_76 = HD_VIDEOENC_IN(0, 76),
	HD_VIDEOENC_0_IN_77 = HD_VIDEOENC_IN(0, 77),
	HD_VIDEOENC_0_IN_78 = HD_VIDEOENC_IN(0, 78),
	HD_VIDEOENC_0_IN_79 = HD_VIDEOENC_IN(0, 79),
	HD_VIDEOENC_0_IN_80 = HD_VIDEOENC_IN(0, 80),
	HD_VIDEOENC_0_IN_81 = HD_VIDEOENC_IN(0, 81),
	HD_VIDEOENC_0_IN_82 = HD_VIDEOENC_IN(0, 82),
	HD_VIDEOENC_0_IN_83 = HD_VIDEOENC_IN(0, 83),
	HD_VIDEOENC_0_IN_84 = HD_VIDEOENC_IN(0, 84),
	HD_VIDEOENC_0_IN_85 = HD_VIDEOENC_IN(0, 85),
	HD_VIDEOENC_0_IN_86 = HD_VIDEOENC_IN(0, 86),
	HD_VIDEOENC_0_IN_87 = HD_VIDEOENC_IN(0, 87),
	HD_VIDEOENC_0_IN_88 = HD_VIDEOENC_IN(0, 88),
	HD_VIDEOENC_0_IN_89 = HD_VIDEOENC_IN(0, 89),
	HD_VIDEOENC_0_IN_90 = HD_VIDEOENC_IN(0, 90),
	HD_VIDEOENC_0_IN_91 = HD_VIDEOENC_IN(0, 91),
	HD_VIDEOENC_0_IN_92 = HD_VIDEOENC_IN(0, 92),
	HD_VIDEOENC_0_IN_93 = HD_VIDEOENC_IN(0, 93),
	HD_VIDEOENC_0_IN_94 = HD_VIDEOENC_IN(0, 94),
	HD_VIDEOENC_0_IN_95 = HD_VIDEOENC_IN(0, 95),
	HD_VIDEOENC_0_IN_96 = HD_VIDEOENC_IN(0, 96),
	HD_VIDEOENC_0_IN_97 = HD_VIDEOENC_IN(0, 97),
	HD_VIDEOENC_0_IN_98 = HD_VIDEOENC_IN(0, 98),
	HD_VIDEOENC_0_IN_99 = HD_VIDEOENC_IN(0, 99),
	HD_VIDEOENC_0_IN_100 = HD_VIDEOENC_IN(0, 100),
	HD_VIDEOENC_0_IN_101 = HD_VIDEOENC_IN(0, 101),
	HD_VIDEOENC_0_IN_102 = HD_VIDEOENC_IN(0, 102),
	HD_VIDEOENC_0_IN_103 = HD_VIDEOENC_IN(0, 103),
	HD_VIDEOENC_0_IN_104 = HD_VIDEOENC_IN(0, 104),
	HD_VIDEOENC_0_IN_105 = HD_VIDEOENC_IN(0, 105),
	HD_VIDEOENC_0_IN_106 = HD_VIDEOENC_IN(0, 106),
	HD_VIDEOENC_0_IN_107 = HD_VIDEOENC_IN(0, 107),
	HD_VIDEOENC_0_IN_108 = HD_VIDEOENC_IN(0, 108),
	HD_VIDEOENC_0_IN_109 = HD_VIDEOENC_IN(0, 109),
	HD_VIDEOENC_0_IN_110 = HD_VIDEOENC_IN(0, 110),
	HD_VIDEOENC_0_IN_111 = HD_VIDEOENC_IN(0, 111),
	HD_VIDEOENC_0_IN_112 = HD_VIDEOENC_IN(0, 112),
	HD_VIDEOENC_0_IN_113 = HD_VIDEOENC_IN(0, 113),
	HD_VIDEOENC_0_IN_114 = HD_VIDEOENC_IN(0, 114),
	HD_VIDEOENC_0_IN_115 = HD_VIDEOENC_IN(0, 115),
	HD_VIDEOENC_0_IN_116 = HD_VIDEOENC_IN(0, 116),
	HD_VIDEOENC_0_IN_117 = HD_VIDEOENC_IN(0, 117),
	HD_VIDEOENC_0_IN_118 = HD_VIDEOENC_IN(0, 118),
	HD_VIDEOENC_0_IN_119 = HD_VIDEOENC_IN(0, 119),
	HD_VIDEOENC_0_IN_120 = HD_VIDEOENC_IN(0, 120),
	HD_VIDEOENC_0_IN_121 = HD_VIDEOENC_IN(0, 121),
	HD_VIDEOENC_0_IN_122 = HD_VIDEOENC_IN(0, 122),
	HD_VIDEOENC_0_IN_123 = HD_VIDEOENC_IN(0, 123),
	HD_VIDEOENC_0_IN_124 = HD_VIDEOENC_IN(0, 124),
	HD_VIDEOENC_0_IN_125 = HD_VIDEOENC_IN(0, 125),
	HD_VIDEOENC_0_IN_126 = HD_VIDEOENC_IN(0, 126),
	HD_VIDEOENC_0_IN_127 = HD_VIDEOENC_IN(0, 127),


	/* DEVICE 0 */
	HD_VIDEODEC_0_IN_0 = HD_VIDEODEC_IN(0, 0),
	HD_VIDEODEC_0_IN_1 = HD_VIDEODEC_IN(0, 1),
	HD_VIDEODEC_0_IN_2 = HD_VIDEODEC_IN(0, 2),
	HD_VIDEODEC_0_IN_3 = HD_VIDEODEC_IN(0, 3),
	HD_VIDEODEC_0_IN_4 = HD_VIDEODEC_IN(0, 4),
	HD_VIDEODEC_0_IN_5 = HD_VIDEODEC_IN(0, 5),
	HD_VIDEODEC_0_IN_6 = HD_VIDEODEC_IN(0, 6),
	HD_VIDEODEC_0_IN_7 = HD_VIDEODEC_IN(0, 7),
	HD_VIDEODEC_0_IN_8 = HD_VIDEODEC_IN(0, 8),
	HD_VIDEODEC_0_IN_9 = HD_VIDEODEC_IN(0, 9),
	HD_VIDEODEC_0_IN_10 = HD_VIDEODEC_IN(0, 10),
	HD_VIDEODEC_0_IN_11 = HD_VIDEODEC_IN(0, 11),
	HD_VIDEODEC_0_IN_12 = HD_VIDEODEC_IN(0, 12),
	HD_VIDEODEC_0_IN_13 = HD_VIDEODEC_IN(0, 13),
	HD_VIDEODEC_0_IN_14 = HD_VIDEODEC_IN(0, 14),
	HD_VIDEODEC_0_IN_15 = HD_VIDEODEC_IN(0, 15),
	HD_VIDEODEC_0_IN_16 = HD_VIDEODEC_IN(0, 16),
	HD_VIDEODEC_0_IN_17 = HD_VIDEODEC_IN(0, 17),
	HD_VIDEODEC_0_IN_18 = HD_VIDEODEC_IN(0, 18),
	HD_VIDEODEC_0_IN_19 = HD_VIDEODEC_IN(0, 19),
	HD_VIDEODEC_0_IN_20 = HD_VIDEODEC_IN(0, 20),
	HD_VIDEODEC_0_IN_21 = HD_VIDEODEC_IN(0, 21),
	HD_VIDEODEC_0_IN_22 = HD_VIDEODEC_IN(0, 22),
	HD_VIDEODEC_0_IN_23 = HD_VIDEODEC_IN(0, 23),
	HD_VIDEODEC_0_IN_24 = HD_VIDEODEC_IN(0, 24),
	HD_VIDEODEC_0_IN_25 = HD_VIDEODEC_IN(0, 25),
	HD_VIDEODEC_0_IN_26 = HD_VIDEODEC_IN(0, 26),
	HD_VIDEODEC_0_IN_27 = HD_VIDEODEC_IN(0, 27),
	HD_VIDEODEC_0_IN_28 = HD_VIDEODEC_IN(0, 28),
	HD_VIDEODEC_0_IN_29 = HD_VIDEODEC_IN(0, 29),
	HD_VIDEODEC_0_IN_30 = HD_VIDEODEC_IN(0, 30),
	HD_VIDEODEC_0_IN_31 = HD_VIDEODEC_IN(0, 31),
	HD_VIDEODEC_0_IN_32 = HD_VIDEODEC_IN(0, 32),
	HD_VIDEODEC_0_IN_33 = HD_VIDEODEC_IN(0, 33),
	HD_VIDEODEC_0_IN_34 = HD_VIDEODEC_IN(0, 34),
	HD_VIDEODEC_0_IN_35 = HD_VIDEODEC_IN(0, 35),
	HD_VIDEODEC_0_IN_36 = HD_VIDEODEC_IN(0, 36),
	HD_VIDEODEC_0_IN_37 = HD_VIDEODEC_IN(0, 37),
	HD_VIDEODEC_0_IN_38 = HD_VIDEODEC_IN(0, 38),
	HD_VIDEODEC_0_IN_39 = HD_VIDEODEC_IN(0, 39),
	HD_VIDEODEC_0_IN_40 = HD_VIDEODEC_IN(0, 40),
	HD_VIDEODEC_0_IN_41 = HD_VIDEODEC_IN(0, 41),
	HD_VIDEODEC_0_IN_42 = HD_VIDEODEC_IN(0, 42),
	HD_VIDEODEC_0_IN_43 = HD_VIDEODEC_IN(0, 43),
	HD_VIDEODEC_0_IN_44 = HD_VIDEODEC_IN(0, 44),
	HD_VIDEODEC_0_IN_45 = HD_VIDEODEC_IN(0, 45),
	HD_VIDEODEC_0_IN_46 = HD_VIDEODEC_IN(0, 46),
	HD_VIDEODEC_0_IN_47 = HD_VIDEODEC_IN(0, 47),
	HD_VIDEODEC_0_IN_48 = HD_VIDEODEC_IN(0, 48),
	HD_VIDEODEC_0_IN_49 = HD_VIDEODEC_IN(0, 49),
	HD_VIDEODEC_0_IN_50 = HD_VIDEODEC_IN(0, 50),
	HD_VIDEODEC_0_IN_51 = HD_VIDEODEC_IN(0, 51),
	HD_VIDEODEC_0_IN_52 = HD_VIDEODEC_IN(0, 52),
	HD_VIDEODEC_0_IN_53 = HD_VIDEODEC_IN(0, 53),
	HD_VIDEODEC_0_IN_54 = HD_VIDEODEC_IN(0, 54),
	HD_VIDEODEC_0_IN_55 = HD_VIDEODEC_IN(0, 55),
	HD_VIDEODEC_0_IN_56 = HD_VIDEODEC_IN(0, 56),
	HD_VIDEODEC_0_IN_57 = HD_VIDEODEC_IN(0, 57),
	HD_VIDEODEC_0_IN_58 = HD_VIDEODEC_IN(0, 58),
	HD_VIDEODEC_0_IN_59 = HD_VIDEODEC_IN(0, 59),
	HD_VIDEODEC_0_IN_60 = HD_VIDEODEC_IN(0, 60),
	HD_VIDEODEC_0_IN_61 = HD_VIDEODEC_IN(0, 61),
	HD_VIDEODEC_0_IN_62 = HD_VIDEODEC_IN(0, 62),
	HD_VIDEODEC_0_IN_63 = HD_VIDEODEC_IN(0, 63),

	/* DEVICE 0 */
	HD_VIDEOPROC_0_IN_0 = HD_VIDEOPROC_IN(0, 0),
	/* DEVICE 1 */
	HD_VIDEOPROC_1_IN_0 = HD_VIDEOPROC_IN(1, 0),
	/* DEVICE 2 */
	HD_VIDEOPROC_2_IN_0 = HD_VIDEOPROC_IN(2, 0),
	/* DEVICE 3 */
	HD_VIDEOPROC_3_IN_0 = HD_VIDEOPROC_IN(3, 0),
	/* DEVICE 4 */
	HD_VIDEOPROC_4_IN_0 = HD_VIDEOPROC_IN(4, 0),
	/* DEVICE 5 */
	HD_VIDEOPROC_5_IN_0 = HD_VIDEOPROC_IN(5, 0),
	/* DEVICE 6 */
	HD_VIDEOPROC_6_IN_0 = HD_VIDEOPROC_IN(6, 0),
	/* DEVICE 7 */
	HD_VIDEOPROC_7_IN_0 = HD_VIDEOPROC_IN(7, 0),
	/* DEVICE 8 */
	HD_VIDEOPROC_8_IN_0 = HD_VIDEOPROC_IN(8, 0),
	/* DEVICE 9 */
	HD_VIDEOPROC_9_IN_0 = HD_VIDEOPROC_IN(9, 0),
	/* DEVICE 10 */
	HD_VIDEOPROC_10_IN_0 = HD_VIDEOPROC_IN(10, 0),
	/* DEVICE 11 */
	HD_VIDEOPROC_11_IN_0 = HD_VIDEOPROC_IN(11, 0),
	/* DEVICE 12 */
	HD_VIDEOPROC_12_IN_0 = HD_VIDEOPROC_IN(12, 0),
	/* DEVICE 13 */
	HD_VIDEOPROC_13_IN_0 = HD_VIDEOPROC_IN(13, 0),
	/* DEVICE 14 */
	HD_VIDEOPROC_14_IN_0 = HD_VIDEOPROC_IN(14, 0),
	/* DEVICE 15 */
	HD_VIDEOPROC_15_IN_0 = HD_VIDEOPROC_IN(15, 0),
	/* DEVICE 16 */
	HD_VIDEOPROC_16_IN_0 = HD_VIDEOPROC_IN(16, 0),
	/* DEVICE 17 */
	HD_VIDEOPROC_17_IN_0 = HD_VIDEOPROC_IN(17, 0),
	/* DEVICE 18 */
	HD_VIDEOPROC_18_IN_0 = HD_VIDEOPROC_IN(18, 0),
	/* DEVICE 19 */
	HD_VIDEOPROC_19_IN_0 = HD_VIDEOPROC_IN(19, 0),
	/* DEVICE 20 */
	HD_VIDEOPROC_20_IN_0 = HD_VIDEOPROC_IN(20, 0),
	/* DEVICE 21 */
	HD_VIDEOPROC_21_IN_0 = HD_VIDEOPROC_IN(21, 0),
	/* DEVICE 22 */
	HD_VIDEOPROC_22_IN_0 = HD_VIDEOPROC_IN(22, 0),
	/* DEVICE 23 */
	HD_VIDEOPROC_23_IN_0 = HD_VIDEOPROC_IN(23, 0),
	/* DEVICE 24 */
	HD_VIDEOPROC_24_IN_0 = HD_VIDEOPROC_IN(24, 0),
	/* DEVICE 25 */
	HD_VIDEOPROC_25_IN_0 = HD_VIDEOPROC_IN(25, 0),
	/* DEVICE 26 */
	HD_VIDEOPROC_26_IN_0 = HD_VIDEOPROC_IN(26, 0),
	/* DEVICE 27 */
	HD_VIDEOPROC_27_IN_0 = HD_VIDEOPROC_IN(27, 0),
	/* DEVICE 28 */
	HD_VIDEOPROC_28_IN_0 = HD_VIDEOPROC_IN(28, 0),
	/* DEVICE 29 */
	HD_VIDEOPROC_29_IN_0 = HD_VIDEOPROC_IN(29, 0),
	/* DEVICE 30 */
	HD_VIDEOPROC_30_IN_0 = HD_VIDEOPROC_IN(30, 0),
	/* DEVICE 31 */
	HD_VIDEOPROC_31_IN_0 = HD_VIDEOPROC_IN(31, 0),
	/* DEVICE 32 */
	HD_VIDEOPROC_32_IN_0 = HD_VIDEOPROC_IN(32, 0),
	/* DEVICE 33 */
	HD_VIDEOPROC_33_IN_0 = HD_VIDEOPROC_IN(33, 0),
	/* DEVICE 34 */
	HD_VIDEOPROC_34_IN_0 = HD_VIDEOPROC_IN(34, 0),
	/* DEVICE 35 */
	HD_VIDEOPROC_35_IN_0 = HD_VIDEOPROC_IN(35, 0),
	/* DEVICE 36 */
	HD_VIDEOPROC_36_IN_0 = HD_VIDEOPROC_IN(36, 0),
	/* DEVICE 37 */
	HD_VIDEOPROC_37_IN_0 = HD_VIDEOPROC_IN(37, 0),
	/* DEVICE 38 */
	HD_VIDEOPROC_38_IN_0 = HD_VIDEOPROC_IN(38, 0),
	/* DEVICE 39 */
	HD_VIDEOPROC_39_IN_0 = HD_VIDEOPROC_IN(39, 0),
	/* DEVICE 40 */
	HD_VIDEOPROC_40_IN_0 = HD_VIDEOPROC_IN(40, 0),
	/* DEVICE 41 */
	HD_VIDEOPROC_41_IN_0 = HD_VIDEOPROC_IN(41, 0),
	/* DEVICE 42 */
	HD_VIDEOPROC_42_IN_0 = HD_VIDEOPROC_IN(42, 0),
	/* DEVICE 43 */
	HD_VIDEOPROC_43_IN_0 = HD_VIDEOPROC_IN(43, 0),
	/* DEVICE 44 */
	HD_VIDEOPROC_44_IN_0 = HD_VIDEOPROC_IN(44, 0),
	/* DEVICE 45 */
	HD_VIDEOPROC_45_IN_0 = HD_VIDEOPROC_IN(45, 0),
	/* DEVICE 46 */
	HD_VIDEOPROC_46_IN_0 = HD_VIDEOPROC_IN(46, 0),
	/* DEVICE 47 */
	HD_VIDEOPROC_47_IN_0 = HD_VIDEOPROC_IN(47, 0),
	/* DEVICE 48 */
	HD_VIDEOPROC_48_IN_0 = HD_VIDEOPROC_IN(48, 0),
	/* DEVICE 49 */
	HD_VIDEOPROC_49_IN_0 = HD_VIDEOPROC_IN(49, 0),
	/* DEVICE 50 */
	HD_VIDEOPROC_50_IN_0 = HD_VIDEOPROC_IN(50, 0),
	/* DEVICE 51 */
	HD_VIDEOPROC_51_IN_0 = HD_VIDEOPROC_IN(51, 0),
	/* DEVICE 52 */
	HD_VIDEOPROC_52_IN_0 = HD_VIDEOPROC_IN(52, 0),
	/* DEVICE 53 */
	HD_VIDEOPROC_53_IN_0 = HD_VIDEOPROC_IN(53, 0),
	/* DEVICE 54 */
	HD_VIDEOPROC_54_IN_0 = HD_VIDEOPROC_IN(54, 0),
	/* DEVICE 55 */
	HD_VIDEOPROC_55_IN_0 = HD_VIDEOPROC_IN(55, 0),
	/* DEVICE 56 */
	HD_VIDEOPROC_56_IN_0 = HD_VIDEOPROC_IN(56, 0),
	/* DEVICE 57 */
	HD_VIDEOPROC_57_IN_0 = HD_VIDEOPROC_IN(57, 0),
	/* DEVICE 58 */
	HD_VIDEOPROC_58_IN_0 = HD_VIDEOPROC_IN(58, 0),
	/* DEVICE 59 */
	HD_VIDEOPROC_59_IN_0 = HD_VIDEOPROC_IN(59, 0),
	/* DEVICE 60 */
	HD_VIDEOPROC_60_IN_0 = HD_VIDEOPROC_IN(60, 0),
	/* DEVICE 61 */
	HD_VIDEOPROC_61_IN_0 = HD_VIDEOPROC_IN(61, 0),
	/* DEVICE 62 */
	HD_VIDEOPROC_62_IN_0 = HD_VIDEOPROC_IN(62, 0),
	/* DEVICE 63 */
	HD_VIDEOPROC_63_IN_0 = HD_VIDEOPROC_IN(63, 0),
	/* DEVICE 64 */
	HD_VIDEOPROC_64_IN_0 = HD_VIDEOPROC_IN(64, 0),
	/* DEVICE 65 */
	HD_VIDEOPROC_65_IN_0 = HD_VIDEOPROC_IN(65, 0),
	/* DEVICE 66 */
	HD_VIDEOPROC_66_IN_0 = HD_VIDEOPROC_IN(66, 0),
	/* DEVICE 67 */
	HD_VIDEOPROC_67_IN_0 = HD_VIDEOPROC_IN(67, 0),
	/* DEVICE 68 */
	HD_VIDEOPROC_68_IN_0 = HD_VIDEOPROC_IN(68, 0),
	/* DEVICE 69 */
	HD_VIDEOPROC_69_IN_0 = HD_VIDEOPROC_IN(69, 0),
	/* DEVICE 70 */
	HD_VIDEOPROC_70_IN_0 = HD_VIDEOPROC_IN(70, 0),
	/* DEVICE 71 */
	HD_VIDEOPROC_71_IN_0 = HD_VIDEOPROC_IN(71, 0),
	/* DEVICE 72 */
	HD_VIDEOPROC_72_IN_0 = HD_VIDEOPROC_IN(72, 0),
	/* DEVICE 73 */
	HD_VIDEOPROC_73_IN_0 = HD_VIDEOPROC_IN(73, 0),
	/* DEVICE 74 */
	HD_VIDEOPROC_74_IN_0 = HD_VIDEOPROC_IN(74, 0),
	/* DEVICE 75 */
	HD_VIDEOPROC_75_IN_0 = HD_VIDEOPROC_IN(75, 0),
	/* DEVICE 76 */
	HD_VIDEOPROC_76_IN_0 = HD_VIDEOPROC_IN(76, 0),
	/* DEVICE 77 */
	HD_VIDEOPROC_77_IN_0 = HD_VIDEOPROC_IN(77, 0),
	/* DEVICE 78 */
	HD_VIDEOPROC_78_IN_0 = HD_VIDEOPROC_IN(78, 0),
	/* DEVICE 79 */
	HD_VIDEOPROC_79_IN_0 = HD_VIDEOPROC_IN(79, 0),
	/* DEVICE 80 */
	HD_VIDEOPROC_80_IN_0 = HD_VIDEOPROC_IN(80, 0),
	/* DEVICE 81 */
	HD_VIDEOPROC_81_IN_0 = HD_VIDEOPROC_IN(81, 0),
	/* DEVICE 82 */
	HD_VIDEOPROC_82_IN_0 = HD_VIDEOPROC_IN(82, 0),
	/* DEVICE 83 */
	HD_VIDEOPROC_83_IN_0 = HD_VIDEOPROC_IN(83, 0),
	/* DEVICE 84 */
	HD_VIDEOPROC_84_IN_0 = HD_VIDEOPROC_IN(84, 0),
	/* DEVICE 85 */
	HD_VIDEOPROC_85_IN_0 = HD_VIDEOPROC_IN(85, 0),
	/* DEVICE 86 */
	HD_VIDEOPROC_86_IN_0 = HD_VIDEOPROC_IN(86, 0),
	/* DEVICE 87 */
	HD_VIDEOPROC_87_IN_0 = HD_VIDEOPROC_IN(87, 0),
	/* DEVICE 88 */
	HD_VIDEOPROC_88_IN_0 = HD_VIDEOPROC_IN(88, 0),
	/* DEVICE 89 */
	HD_VIDEOPROC_89_IN_0 = HD_VIDEOPROC_IN(89, 0),
	/* DEVICE 90 */
	HD_VIDEOPROC_90_IN_0 = HD_VIDEOPROC_IN(90, 0),
	/* DEVICE 91 */
	HD_VIDEOPROC_91_IN_0 = HD_VIDEOPROC_IN(91, 0),
	/* DEVICE 92 */
	HD_VIDEOPROC_92_IN_0 = HD_VIDEOPROC_IN(92, 0),
	/* DEVICE 93 */
	HD_VIDEOPROC_93_IN_0 = HD_VIDEOPROC_IN(93, 0),
	/* DEVICE 94 */
	HD_VIDEOPROC_94_IN_0 = HD_VIDEOPROC_IN(94, 0),
	/* DEVICE 95 */
	HD_VIDEOPROC_95_IN_0 = HD_VIDEOPROC_IN(95, 0),
	/* DEVICE 96 */
	HD_VIDEOPROC_96_IN_0 = HD_VIDEOPROC_IN(96, 0),
	/* DEVICE 97 */
	HD_VIDEOPROC_97_IN_0 = HD_VIDEOPROC_IN(97, 0),
	/* DEVICE 98 */
	HD_VIDEOPROC_98_IN_0 = HD_VIDEOPROC_IN(98, 0),
	/* DEVICE 99 */
	HD_VIDEOPROC_99_IN_0 = HD_VIDEOPROC_IN(99, 0),
	/* DEVICE 100 */
	HD_VIDEOPROC_100_IN_0 = HD_VIDEOPROC_IN(100, 0),
	/* DEVICE 101 */
	HD_VIDEOPROC_101_IN_0 = HD_VIDEOPROC_IN(101, 0),
	/* DEVICE 102 */
	HD_VIDEOPROC_102_IN_0 = HD_VIDEOPROC_IN(102, 0),
	/* DEVICE 103 */
	HD_VIDEOPROC_103_IN_0 = HD_VIDEOPROC_IN(103, 0),
	/* DEVICE 104 */
	HD_VIDEOPROC_104_IN_0 = HD_VIDEOPROC_IN(104, 0),
	/* DEVICE 105 */
	HD_VIDEOPROC_105_IN_0 = HD_VIDEOPROC_IN(105, 0),
	/* DEVICE 106 */
	HD_VIDEOPROC_106_IN_0 = HD_VIDEOPROC_IN(106, 0),
	/* DEVICE 107 */
	HD_VIDEOPROC_107_IN_0 = HD_VIDEOPROC_IN(107, 0),
	/* DEVICE 108 */
	HD_VIDEOPROC_108_IN_0 = HD_VIDEOPROC_IN(108, 0),
	/* DEVICE 109 */
	HD_VIDEOPROC_109_IN_0 = HD_VIDEOPROC_IN(109, 0),
	/* DEVICE 110 */
	HD_VIDEOPROC_110_IN_0 = HD_VIDEOPROC_IN(110, 0),
	/* DEVICE 111 */
	HD_VIDEOPROC_111_IN_0 = HD_VIDEOPROC_IN(111, 0),
	/* DEVICE 112 */
	HD_VIDEOPROC_112_IN_0 = HD_VIDEOPROC_IN(112, 0),
	/* DEVICE 113 */
	HD_VIDEOPROC_113_IN_0 = HD_VIDEOPROC_IN(113, 0),
	/* DEVICE 114 */
	HD_VIDEOPROC_114_IN_0 = HD_VIDEOPROC_IN(114, 0),
	/* DEVICE 115 */
	HD_VIDEOPROC_115_IN_0 = HD_VIDEOPROC_IN(115, 0),
	/* DEVICE 116 */
	HD_VIDEOPROC_116_IN_0 = HD_VIDEOPROC_IN(116, 0),
	/* DEVICE 117 */
	HD_VIDEOPROC_117_IN_0 = HD_VIDEOPROC_IN(117, 0),
	/* DEVICE 118 */
	HD_VIDEOPROC_118_IN_0 = HD_VIDEOPROC_IN(118, 0),
	/* DEVICE 119 */
	HD_VIDEOPROC_119_IN_0 = HD_VIDEOPROC_IN(119, 0),
	/* DEVICE 120 */
	HD_VIDEOPROC_120_IN_0 = HD_VIDEOPROC_IN(120, 0),
	/* DEVICE 121 */
	HD_VIDEOPROC_121_IN_0 = HD_VIDEOPROC_IN(121, 0),
	/* DEVICE 122 */
	HD_VIDEOPROC_122_IN_0 = HD_VIDEOPROC_IN(122, 0),
	/* DEVICE 123 */
	HD_VIDEOPROC_123_IN_0 = HD_VIDEOPROC_IN(123, 0),
	/* DEVICE 124 */
	HD_VIDEOPROC_124_IN_0 = HD_VIDEOPROC_IN(124, 0),
	/* DEVICE 125 */
	HD_VIDEOPROC_125_IN_0 = HD_VIDEOPROC_IN(125, 0),
	/* DEVICE 126 */
	HD_VIDEOPROC_126_IN_0 = HD_VIDEOPROC_IN(126, 0),
	/* DEVICE 127 */
	HD_VIDEOPROC_127_IN_0 = HD_VIDEOPROC_IN(127, 0),
	/* DEVICE 128 */
	HD_VIDEOPROC_128_IN_0 = HD_VIDEOPROC_IN(128, 0),
	/* DEVICE 129 */
	HD_VIDEOPROC_129_IN_0 = HD_VIDEOPROC_IN(129, 0),
	/* DEVICE 130 */
	HD_VIDEOPROC_130_IN_0 = HD_VIDEOPROC_IN(130, 0),
	/* DEVICE 131 */
	HD_VIDEOPROC_131_IN_0 = HD_VIDEOPROC_IN(131, 0),
	/* DEVICE 132 */
	HD_VIDEOPROC_132_IN_0 = HD_VIDEOPROC_IN(132, 0),
	/* DEVICE 133 */
	HD_VIDEOPROC_133_IN_0 = HD_VIDEOPROC_IN(133, 0),
	/* DEVICE 134 */
	HD_VIDEOPROC_134_IN_0 = HD_VIDEOPROC_IN(134, 0),
	/* DEVICE 135 */
	HD_VIDEOPROC_135_IN_0 = HD_VIDEOPROC_IN(135, 0),
	/* DEVICE 136 */
	HD_VIDEOPROC_136_IN_0 = HD_VIDEOPROC_IN(136, 0),
	/* DEVICE 137 */
	HD_VIDEOPROC_137_IN_0 = HD_VIDEOPROC_IN(137, 0),
	/* DEVICE 138 */
	HD_VIDEOPROC_138_IN_0 = HD_VIDEOPROC_IN(138, 0),
	/* DEVICE 139 */
	HD_VIDEOPROC_139_IN_0 = HD_VIDEOPROC_IN(139, 0),
	/* DEVICE 140 */
	HD_VIDEOPROC_140_IN_0 = HD_VIDEOPROC_IN(140, 0),
	/* DEVICE 141 */
	HD_VIDEOPROC_141_IN_0 = HD_VIDEOPROC_IN(141, 0),
	/* DEVICE 142 */
	HD_VIDEOPROC_142_IN_0 = HD_VIDEOPROC_IN(142, 0),
	/* DEVICE 143 */
	HD_VIDEOPROC_143_IN_0 = HD_VIDEOPROC_IN(143, 0),
	/* DEVICE 144 */
	HD_VIDEOPROC_144_IN_0 = HD_VIDEOPROC_IN(144, 0),
	/* DEVICE 145 */
	HD_VIDEOPROC_145_IN_0 = HD_VIDEOPROC_IN(145, 0),
	/* DEVICE 146 */
	HD_VIDEOPROC_146_IN_0 = HD_VIDEOPROC_IN(146, 0),
	/* DEVICE 147 */
	HD_VIDEOPROC_147_IN_0 = HD_VIDEOPROC_IN(147, 0),
	/* DEVICE 148 */
	HD_VIDEOPROC_148_IN_0 = HD_VIDEOPROC_IN(148, 0),
	/* DEVICE 149 */
	HD_VIDEOPROC_149_IN_0 = HD_VIDEOPROC_IN(149, 0),
	/* DEVICE 150 */
	HD_VIDEOPROC_150_IN_0 = HD_VIDEOPROC_IN(150, 0),
	/* DEVICE 151 */
	HD_VIDEOPROC_151_IN_0 = HD_VIDEOPROC_IN(151, 0),
	/* DEVICE 152 */
	HD_VIDEOPROC_152_IN_0 = HD_VIDEOPROC_IN(152, 0),
	/* DEVICE 153 */
	HD_VIDEOPROC_153_IN_0 = HD_VIDEOPROC_IN(153, 0),
	/* DEVICE 154 */
	HD_VIDEOPROC_154_IN_0 = HD_VIDEOPROC_IN(154, 0),
	/* DEVICE 155 */
	HD_VIDEOPROC_155_IN_0 = HD_VIDEOPROC_IN(155, 0),
	/* DEVICE 156 */
	HD_VIDEOPROC_156_IN_0 = HD_VIDEOPROC_IN(156, 0),
	/* DEVICE 157 */
	HD_VIDEOPROC_157_IN_0 = HD_VIDEOPROC_IN(157, 0),
	/* DEVICE 158 */
	HD_VIDEOPROC_158_IN_0 = HD_VIDEOPROC_IN(158, 0),
	/* DEVICE 159 */
	HD_VIDEOPROC_159_IN_0 = HD_VIDEOPROC_IN(159, 0),
	/* DEVICE 160 */
	HD_VIDEOPROC_160_IN_0 = HD_VIDEOPROC_IN(160, 0),
	/* DEVICE 161 */
	HD_VIDEOPROC_161_IN_0 = HD_VIDEOPROC_IN(161, 0),
	/* DEVICE 162 */
	HD_VIDEOPROC_162_IN_0 = HD_VIDEOPROC_IN(162, 0),
	/* DEVICE 163 */
	HD_VIDEOPROC_163_IN_0 = HD_VIDEOPROC_IN(163, 0),
	/* DEVICE 164 */
	HD_VIDEOPROC_164_IN_0 = HD_VIDEOPROC_IN(164, 0),
	/* DEVICE 165 */
	HD_VIDEOPROC_165_IN_0 = HD_VIDEOPROC_IN(165, 0),
	/* DEVICE 166 */
	HD_VIDEOPROC_166_IN_0 = HD_VIDEOPROC_IN(166, 0),
	/* DEVICE 167 */
	HD_VIDEOPROC_167_IN_0 = HD_VIDEOPROC_IN(167, 0),
	/* DEVICE 168 */
	HD_VIDEOPROC_168_IN_0 = HD_VIDEOPROC_IN(168, 0),
	/* DEVICE 169 */
	HD_VIDEOPROC_169_IN_0 = HD_VIDEOPROC_IN(169, 0),
	/* DEVICE 170 */
	HD_VIDEOPROC_170_IN_0 = HD_VIDEOPROC_IN(170, 0),
	/* DEVICE 171 */
	HD_VIDEOPROC_171_IN_0 = HD_VIDEOPROC_IN(171, 0),
	/* DEVICE 172 */
	HD_VIDEOPROC_172_IN_0 = HD_VIDEOPROC_IN(172, 0),
	/* DEVICE 173 */
	HD_VIDEOPROC_173_IN_0 = HD_VIDEOPROC_IN(173, 0),
	/* DEVICE 174 */
	HD_VIDEOPROC_174_IN_0 = HD_VIDEOPROC_IN(174, 0),
	/* DEVICE 175 */
	HD_VIDEOPROC_175_IN_0 = HD_VIDEOPROC_IN(175, 0),
	/* DEVICE 176 */
	HD_VIDEOPROC_176_IN_0 = HD_VIDEOPROC_IN(176, 0),
	/* DEVICE 177 */
	HD_VIDEOPROC_177_IN_0 = HD_VIDEOPROC_IN(177, 0),
	/* DEVICE 178 */
	HD_VIDEOPROC_178_IN_0 = HD_VIDEOPROC_IN(178, 0),
	/* DEVICE 179 */
	HD_VIDEOPROC_179_IN_0 = HD_VIDEOPROC_IN(179, 0),
	/* DEVICE 180 */
	HD_VIDEOPROC_180_IN_0 = HD_VIDEOPROC_IN(180, 0),
	/* DEVICE 181 */
	HD_VIDEOPROC_181_IN_0 = HD_VIDEOPROC_IN(181, 0),
	/* DEVICE 182 */
	HD_VIDEOPROC_182_IN_0 = HD_VIDEOPROC_IN(182, 0),
	/* DEVICE 183 */
	HD_VIDEOPROC_183_IN_0 = HD_VIDEOPROC_IN(183, 0),
	/* DEVICE 184 */
	HD_VIDEOPROC_184_IN_0 = HD_VIDEOPROC_IN(184, 0),
	/* DEVICE 185 */
	HD_VIDEOPROC_185_IN_0 = HD_VIDEOPROC_IN(185, 0),
	/* DEVICE 186 */
	HD_VIDEOPROC_186_IN_0 = HD_VIDEOPROC_IN(186, 0),
	/* DEVICE 187 */
	HD_VIDEOPROC_187_IN_0 = HD_VIDEOPROC_IN(187, 0),
	/* DEVICE 188 */
	HD_VIDEOPROC_188_IN_0 = HD_VIDEOPROC_IN(188, 0),
	/* DEVICE 189 */
	HD_VIDEOPROC_189_IN_0 = HD_VIDEOPROC_IN(189, 0),
	/* DEVICE 190 */
	HD_VIDEOPROC_190_IN_0 = HD_VIDEOPROC_IN(190, 0),
	/* DEVICE 191 */
	HD_VIDEOPROC_191_IN_0 = HD_VIDEOPROC_IN(191, 0),
	/* DEVICE 192 */
	HD_VIDEOPROC_192_IN_0 = HD_VIDEOPROC_IN(192, 0),
	/* DEVICE 193 */
	HD_VIDEOPROC_193_IN_0 = HD_VIDEOPROC_IN(193, 0),
	/* DEVICE 194 */
	HD_VIDEOPROC_194_IN_0 = HD_VIDEOPROC_IN(194, 0),
	/* DEVICE 195 */
	HD_VIDEOPROC_195_IN_0 = HD_VIDEOPROC_IN(195, 0),
	/* DEVICE 196 */
	HD_VIDEOPROC_196_IN_0 = HD_VIDEOPROC_IN(196, 0),
	/* DEVICE 197 */
	HD_VIDEOPROC_197_IN_0 = HD_VIDEOPROC_IN(197, 0),
	/* DEVICE 198 */
	HD_VIDEOPROC_198_IN_0 = HD_VIDEOPROC_IN(198, 0),
	/* DEVICE 199 */
	HD_VIDEOPROC_199_IN_0 = HD_VIDEOPROC_IN(199, 0),
	/* DEVICE 200 */
	HD_VIDEOPROC_200_IN_0 = HD_VIDEOPROC_IN(200, 0),
	/* DEVICE 201 */
	HD_VIDEOPROC_201_IN_0 = HD_VIDEOPROC_IN(201, 0),
	/* DEVICE 202 */
	HD_VIDEOPROC_202_IN_0 = HD_VIDEOPROC_IN(202, 0),
	/* DEVICE 203 */
	HD_VIDEOPROC_203_IN_0 = HD_VIDEOPROC_IN(203, 0),
	/* DEVICE 204 */
	HD_VIDEOPROC_204_IN_0 = HD_VIDEOPROC_IN(204, 0),
	/* DEVICE 205 */
	HD_VIDEOPROC_205_IN_0 = HD_VIDEOPROC_IN(205, 0),
	/* DEVICE 206 */
	HD_VIDEOPROC_206_IN_0 = HD_VIDEOPROC_IN(206, 0),
	/* DEVICE 207 */
	HD_VIDEOPROC_207_IN_0 = HD_VIDEOPROC_IN(207, 0),
	/* DEVICE 208 */
	HD_VIDEOPROC_208_IN_0 = HD_VIDEOPROC_IN(208, 0),
	/* DEVICE 209 */
	HD_VIDEOPROC_209_IN_0 = HD_VIDEOPROC_IN(209, 0),
	/* DEVICE 210 */
	HD_VIDEOPROC_210_IN_0 = HD_VIDEOPROC_IN(210, 0),
	/* DEVICE 211 */
	HD_VIDEOPROC_211_IN_0 = HD_VIDEOPROC_IN(211, 0),
	/* DEVICE 212 */
	HD_VIDEOPROC_212_IN_0 = HD_VIDEOPROC_IN(212, 0),
	/* DEVICE 213 */
	HD_VIDEOPROC_213_IN_0 = HD_VIDEOPROC_IN(213, 0),
	/* DEVICE 214 */
	HD_VIDEOPROC_214_IN_0 = HD_VIDEOPROC_IN(214, 0),
	/* DEVICE 215 */
	HD_VIDEOPROC_215_IN_0 = HD_VIDEOPROC_IN(215, 0),
	/* DEVICE 216 */
	HD_VIDEOPROC_216_IN_0 = HD_VIDEOPROC_IN(216, 0),
	/* DEVICE 217 */
	HD_VIDEOPROC_217_IN_0 = HD_VIDEOPROC_IN(217, 0),
	/* DEVICE 218 */
	HD_VIDEOPROC_218_IN_0 = HD_VIDEOPROC_IN(218, 0),
	/* DEVICE 219 */
	HD_VIDEOPROC_219_IN_0 = HD_VIDEOPROC_IN(219, 0),
	/* DEVICE 220 */
	HD_VIDEOPROC_220_IN_0 = HD_VIDEOPROC_IN(220, 0),
	/* DEVICE 221 */
	HD_VIDEOPROC_221_IN_0 = HD_VIDEOPROC_IN(221, 0),
	/* DEVICE 222 */
	HD_VIDEOPROC_222_IN_0 = HD_VIDEOPROC_IN(222, 0),
	/* DEVICE 223 */
	HD_VIDEOPROC_223_IN_0 = HD_VIDEOPROC_IN(223, 0),
	/* DEVICE 224 */
	HD_VIDEOPROC_224_IN_0 = HD_VIDEOPROC_IN(224, 0),
	/* DEVICE 225 */
	HD_VIDEOPROC_225_IN_0 = HD_VIDEOPROC_IN(225, 0),
	/* DEVICE 226 */
	HD_VIDEOPROC_226_IN_0 = HD_VIDEOPROC_IN(226, 0),
	/* DEVICE 227 */
	HD_VIDEOPROC_227_IN_0 = HD_VIDEOPROC_IN(227, 0),
	/* DEVICE 228 */
	HD_VIDEOPROC_228_IN_0 = HD_VIDEOPROC_IN(228, 0),
	/* DEVICE 229 */
	HD_VIDEOPROC_229_IN_0 = HD_VIDEOPROC_IN(229, 0),
	/* DEVICE 230 */
	HD_VIDEOPROC_230_IN_0 = HD_VIDEOPROC_IN(230, 0),
	/* DEVICE 231 */
	HD_VIDEOPROC_231_IN_0 = HD_VIDEOPROC_IN(231, 0),
	/* DEVICE 232 */
	HD_VIDEOPROC_232_IN_0 = HD_VIDEOPROC_IN(232, 0),
	/* DEVICE 233 */
	HD_VIDEOPROC_233_IN_0 = HD_VIDEOPROC_IN(233, 0),
	/* DEVICE 234 */
	HD_VIDEOPROC_234_IN_0 = HD_VIDEOPROC_IN(234, 0),
	/* DEVICE 235 */
	HD_VIDEOPROC_235_IN_0 = HD_VIDEOPROC_IN(235, 0),
	/* DEVICE 236 */
	HD_VIDEOPROC_236_IN_0 = HD_VIDEOPROC_IN(236, 0),
	/* DEVICE 237 */
	HD_VIDEOPROC_237_IN_0 = HD_VIDEOPROC_IN(237, 0),
	/* DEVICE 238 */
	HD_VIDEOPROC_238_IN_0 = HD_VIDEOPROC_IN(238, 0),
	/* DEVICE 239 */
	HD_VIDEOPROC_239_IN_0 = HD_VIDEOPROC_IN(239, 0),
	/* DEVICE 240 */
	HD_VIDEOPROC_240_IN_0 = HD_VIDEOPROC_IN(240, 0),
	/* DEVICE 241 */
	HD_VIDEOPROC_241_IN_0 = HD_VIDEOPROC_IN(241, 0),
	/* DEVICE 242 */
	HD_VIDEOPROC_242_IN_0 = HD_VIDEOPROC_IN(242, 0),
	/* DEVICE 243 */
	HD_VIDEOPROC_243_IN_0 = HD_VIDEOPROC_IN(243, 0),
	/* DEVICE 244 */
	HD_VIDEOPROC_244_IN_0 = HD_VIDEOPROC_IN(244, 0),
	/* DEVICE 245 */
	HD_VIDEOPROC_245_IN_0 = HD_VIDEOPROC_IN(245, 0),
	/* DEVICE 246 */
	HD_VIDEOPROC_246_IN_0 = HD_VIDEOPROC_IN(246, 0),
	/* DEVICE 247 */
	HD_VIDEOPROC_247_IN_0 = HD_VIDEOPROC_IN(247, 0),
	/* DEVICE 248 */
	HD_VIDEOPROC_248_IN_0 = HD_VIDEOPROC_IN(248, 0),
	/* DEVICE 249 */
	HD_VIDEOPROC_249_IN_0 = HD_VIDEOPROC_IN(249, 0),
	/* DEVICE 250 */
	HD_VIDEOPROC_250_IN_0 = HD_VIDEOPROC_IN(250, 0),
	/* DEVICE 251 */
	HD_VIDEOPROC_251_IN_0 = HD_VIDEOPROC_IN(251, 0),
	/* DEVICE 252 */
	HD_VIDEOPROC_252_IN_0 = HD_VIDEOPROC_IN(252, 0),
	/* DEVICE 253 */
	HD_VIDEOPROC_253_IN_0 = HD_VIDEOPROC_IN(253, 0),
	/* DEVICE 254 */
	HD_VIDEOPROC_254_IN_0 = HD_VIDEOPROC_IN(254, 0),
	/* DEVICE 255 */
	HD_VIDEOPROC_255_IN_0 = HD_VIDEOPROC_IN(255, 0),
	/* DEVICE 256 */
	HD_VIDEOPROC_256_IN_0 = HD_VIDEOPROC_IN(256, 0),
	/* DEVICE 257 */
	HD_VIDEOPROC_257_IN_0 = HD_VIDEOPROC_IN(257, 0),
	/* DEVICE 258 */
	HD_VIDEOPROC_258_IN_0 = HD_VIDEOPROC_IN(258, 0),
	/* DEVICE 259 */
	HD_VIDEOPROC_259_IN_0 = HD_VIDEOPROC_IN(259, 0),
	/* DEVICE 260 */
	HD_VIDEOPROC_260_IN_0 = HD_VIDEOPROC_IN(260, 0),
	/* DEVICE 261 */
	HD_VIDEOPROC_261_IN_0 = HD_VIDEOPROC_IN(261, 0),
	/* DEVICE 262 */
	HD_VIDEOPROC_262_IN_0 = HD_VIDEOPROC_IN(262, 0),
	/* DEVICE 263 */
	HD_VIDEOPROC_263_IN_0 = HD_VIDEOPROC_IN(263, 0),
	/* DEVICE 264 */
	HD_VIDEOPROC_264_IN_0 = HD_VIDEOPROC_IN(264, 0),
	/* DEVICE 265 */
	HD_VIDEOPROC_265_IN_0 = HD_VIDEOPROC_IN(265, 0),
	/* DEVICE 266 */
	HD_VIDEOPROC_266_IN_0 = HD_VIDEOPROC_IN(266, 0),
	/* DEVICE 267 */
	HD_VIDEOPROC_267_IN_0 = HD_VIDEOPROC_IN(267, 0),
	/* DEVICE 268 */
	HD_VIDEOPROC_268_IN_0 = HD_VIDEOPROC_IN(268, 0),
	/* DEVICE 269 */
	HD_VIDEOPROC_269_IN_0 = HD_VIDEOPROC_IN(269, 0),
	/* DEVICE 270 */
	HD_VIDEOPROC_270_IN_0 = HD_VIDEOPROC_IN(270, 0),
	/* DEVICE 271 */
	HD_VIDEOPROC_271_IN_0 = HD_VIDEOPROC_IN(271, 0),
	/* DEVICE 272 */
	HD_VIDEOPROC_272_IN_0 = HD_VIDEOPROC_IN(272, 0),
	/* DEVICE 273 */
	HD_VIDEOPROC_273_IN_0 = HD_VIDEOPROC_IN(273, 0),
	/* DEVICE 274 */
	HD_VIDEOPROC_274_IN_0 = HD_VIDEOPROC_IN(274, 0),
	/* DEVICE 275 */
	HD_VIDEOPROC_275_IN_0 = HD_VIDEOPROC_IN(275, 0),
	/* DEVICE 276 */
	HD_VIDEOPROC_276_IN_0 = HD_VIDEOPROC_IN(276, 0),
	/* DEVICE 277 */
	HD_VIDEOPROC_277_IN_0 = HD_VIDEOPROC_IN(277, 0),
	/* DEVICE 278 */
	HD_VIDEOPROC_278_IN_0 = HD_VIDEOPROC_IN(278, 0),
	/* DEVICE 279 */
	HD_VIDEOPROC_279_IN_0 = HD_VIDEOPROC_IN(279, 0),
	/* DEVICE 280 */
	HD_VIDEOPROC_280_IN_0 = HD_VIDEOPROC_IN(280, 0),
	/* DEVICE 281 */
	HD_VIDEOPROC_281_IN_0 = HD_VIDEOPROC_IN(281, 0),
	/* DEVICE 282 */
	HD_VIDEOPROC_282_IN_0 = HD_VIDEOPROC_IN(282, 0),
	/* DEVICE 283 */
	HD_VIDEOPROC_283_IN_0 = HD_VIDEOPROC_IN(283, 0),
	/* DEVICE 284 */
	HD_VIDEOPROC_284_IN_0 = HD_VIDEOPROC_IN(284, 0),
	/* DEVICE 285 */
	HD_VIDEOPROC_285_IN_0 = HD_VIDEOPROC_IN(285, 0),
	/* DEVICE 286 */
	HD_VIDEOPROC_286_IN_0 = HD_VIDEOPROC_IN(286, 0),
	/* DEVICE 287 */
	HD_VIDEOPROC_287_IN_0 = HD_VIDEOPROC_IN(287, 0),
	/* DEVICE 288 */
	HD_VIDEOPROC_288_IN_0 = HD_VIDEOPROC_IN(288, 0),
	/* DEVICE 289 */
	HD_VIDEOPROC_289_IN_0 = HD_VIDEOPROC_IN(289, 0),
	/* DEVICE 290 */
	HD_VIDEOPROC_290_IN_0 = HD_VIDEOPROC_IN(290, 0),
	/* DEVICE 291 */
	HD_VIDEOPROC_291_IN_0 = HD_VIDEOPROC_IN(291, 0),
	/* DEVICE 292 */
	HD_VIDEOPROC_292_IN_0 = HD_VIDEOPROC_IN(292, 0),
	/* DEVICE 293 */
	HD_VIDEOPROC_293_IN_0 = HD_VIDEOPROC_IN(293, 0),
	/* DEVICE 294 */
	HD_VIDEOPROC_294_IN_0 = HD_VIDEOPROC_IN(294, 0),
	/* DEVICE 295 */
	HD_VIDEOPROC_295_IN_0 = HD_VIDEOPROC_IN(295, 0),
	/* DEVICE 296 */
	HD_VIDEOPROC_296_IN_0 = HD_VIDEOPROC_IN(296, 0),
	/* DEVICE 297 */
	HD_VIDEOPROC_297_IN_0 = HD_VIDEOPROC_IN(297, 0),
	/* DEVICE 298 */
	HD_VIDEOPROC_298_IN_0 = HD_VIDEOPROC_IN(298, 0),
	/* DEVICE 299 */
	HD_VIDEOPROC_299_IN_0 = HD_VIDEOPROC_IN(299, 0),
	/* DEVICE 300 */
	HD_VIDEOPROC_300_IN_0 = HD_VIDEOPROC_IN(300, 0),
	/* DEVICE 301 */
	HD_VIDEOPROC_301_IN_0 = HD_VIDEOPROC_IN(301, 0),
	/* DEVICE 302 */
	HD_VIDEOPROC_302_IN_0 = HD_VIDEOPROC_IN(302, 0),
	/* DEVICE 303 */
	HD_VIDEOPROC_303_IN_0 = HD_VIDEOPROC_IN(303, 0),
	/* DEVICE 304 */
	HD_VIDEOPROC_304_IN_0 = HD_VIDEOPROC_IN(304, 0),
	/* DEVICE 305 */
	HD_VIDEOPROC_305_IN_0 = HD_VIDEOPROC_IN(305, 0),
	/* DEVICE 306 */
	HD_VIDEOPROC_306_IN_0 = HD_VIDEOPROC_IN(306, 0),
	/* DEVICE 307 */
	HD_VIDEOPROC_307_IN_0 = HD_VIDEOPROC_IN(307, 0),
	/* DEVICE 308 */
	HD_VIDEOPROC_308_IN_0 = HD_VIDEOPROC_IN(308, 0),
	/* DEVICE 309 */
	HD_VIDEOPROC_309_IN_0 = HD_VIDEOPROC_IN(309, 0),
	/* DEVICE 310 */
	HD_VIDEOPROC_310_IN_0 = HD_VIDEOPROC_IN(310, 0),
	/* DEVICE 311 */
	HD_VIDEOPROC_311_IN_0 = HD_VIDEOPROC_IN(311, 0),
	/* DEVICE 312 */
	HD_VIDEOPROC_312_IN_0 = HD_VIDEOPROC_IN(312, 0),
	/* DEVICE 313 */
	HD_VIDEOPROC_313_IN_0 = HD_VIDEOPROC_IN(313, 0),
	/* DEVICE 314 */
	HD_VIDEOPROC_314_IN_0 = HD_VIDEOPROC_IN(314, 0),
	/* DEVICE 315 */
	HD_VIDEOPROC_315_IN_0 = HD_VIDEOPROC_IN(315, 0),
	/* DEVICE 316 */
	HD_VIDEOPROC_316_IN_0 = HD_VIDEOPROC_IN(316, 0),
	/* DEVICE 317 */
	HD_VIDEOPROC_317_IN_0 = HD_VIDEOPROC_IN(317, 0),
	/* DEVICE 318 */
	HD_VIDEOPROC_318_IN_0 = HD_VIDEOPROC_IN(318, 0),
	/* DEVICE 319 */
	HD_VIDEOPROC_319_IN_0 = HD_VIDEOPROC_IN(319, 0),
	/* DEVICE 320 */
	HD_VIDEOPROC_320_IN_0 = HD_VIDEOPROC_IN(320, 0),
	/* DEVICE 321 */
	HD_VIDEOPROC_321_IN_0 = HD_VIDEOPROC_IN(321, 0),
	/* DEVICE 322 */
	HD_VIDEOPROC_322_IN_0 = HD_VIDEOPROC_IN(322, 0),
	/* DEVICE 323 */
	HD_VIDEOPROC_323_IN_0 = HD_VIDEOPROC_IN(323, 0),
	/* DEVICE 324 */
	HD_VIDEOPROC_324_IN_0 = HD_VIDEOPROC_IN(324, 0),
	/* DEVICE 325 */
	HD_VIDEOPROC_325_IN_0 = HD_VIDEOPROC_IN(325, 0),
	/* DEVICE 326 */
	HD_VIDEOPROC_326_IN_0 = HD_VIDEOPROC_IN(326, 0),
	/* DEVICE 327 */
	HD_VIDEOPROC_327_IN_0 = HD_VIDEOPROC_IN(327, 0),
	/* DEVICE 328 */
	HD_VIDEOPROC_328_IN_0 = HD_VIDEOPROC_IN(328, 0),
	/* DEVICE 329 */
	HD_VIDEOPROC_329_IN_0 = HD_VIDEOPROC_IN(329, 0),
	/* DEVICE 330 */
	HD_VIDEOPROC_330_IN_0 = HD_VIDEOPROC_IN(330, 0),
	/* DEVICE 331 */
	HD_VIDEOPROC_331_IN_0 = HD_VIDEOPROC_IN(331, 0),
	/* DEVICE 332 */
	HD_VIDEOPROC_332_IN_0 = HD_VIDEOPROC_IN(332, 0),
	/* DEVICE 333 */
	HD_VIDEOPROC_333_IN_0 = HD_VIDEOPROC_IN(333, 0),
	/* DEVICE 334 */
	HD_VIDEOPROC_334_IN_0 = HD_VIDEOPROC_IN(334, 0),
	/* DEVICE 335 */
	HD_VIDEOPROC_335_IN_0 = HD_VIDEOPROC_IN(335, 0),
	/* DEVICE 336 */
	HD_VIDEOPROC_336_IN_0 = HD_VIDEOPROC_IN(336, 0),
	/* DEVICE 337 */
	HD_VIDEOPROC_337_IN_0 = HD_VIDEOPROC_IN(337, 0),
	/* DEVICE 338 */
	HD_VIDEOPROC_338_IN_0 = HD_VIDEOPROC_IN(338, 0),
	/* DEVICE 339 */
	HD_VIDEOPROC_339_IN_0 = HD_VIDEOPROC_IN(339, 0),
	/* DEVICE 340 */
	HD_VIDEOPROC_340_IN_0 = HD_VIDEOPROC_IN(340, 0),
	/* DEVICE 341 */
	HD_VIDEOPROC_341_IN_0 = HD_VIDEOPROC_IN(341, 0),
	/* DEVICE 342 */
	HD_VIDEOPROC_342_IN_0 = HD_VIDEOPROC_IN(342, 0),
	/* DEVICE 343 */
	HD_VIDEOPROC_343_IN_0 = HD_VIDEOPROC_IN(343, 0),
	/* DEVICE 344 */
	HD_VIDEOPROC_344_IN_0 = HD_VIDEOPROC_IN(344, 0),
	/* DEVICE 345 */
	HD_VIDEOPROC_345_IN_0 = HD_VIDEOPROC_IN(345, 0),
	/* DEVICE 346 */
	HD_VIDEOPROC_346_IN_0 = HD_VIDEOPROC_IN(346, 0),
	/* DEVICE 347 */
	HD_VIDEOPROC_347_IN_0 = HD_VIDEOPROC_IN(347, 0),
	/* DEVICE 348 */
	HD_VIDEOPROC_348_IN_0 = HD_VIDEOPROC_IN(348, 0),
	/* DEVICE 349 */
	HD_VIDEOPROC_349_IN_0 = HD_VIDEOPROC_IN(349, 0),
	/* DEVICE 350 */
	HD_VIDEOPROC_350_IN_0 = HD_VIDEOPROC_IN(350, 0),
	/* DEVICE 351 */
	HD_VIDEOPROC_351_IN_0 = HD_VIDEOPROC_IN(351, 0),
	/* DEVICE 352 */
	HD_VIDEOPROC_352_IN_0 = HD_VIDEOPROC_IN(352, 0),
	/* DEVICE 353 */
	HD_VIDEOPROC_353_IN_0 = HD_VIDEOPROC_IN(353, 0),
	/* DEVICE 354 */
	HD_VIDEOPROC_354_IN_0 = HD_VIDEOPROC_IN(354, 0),
	/* DEVICE 355 */
	HD_VIDEOPROC_355_IN_0 = HD_VIDEOPROC_IN(355, 0),
	/* DEVICE 356 */
	HD_VIDEOPROC_356_IN_0 = HD_VIDEOPROC_IN(356, 0),
	/* DEVICE 357 */
	HD_VIDEOPROC_357_IN_0 = HD_VIDEOPROC_IN(357, 0),
	/* DEVICE 358 */
	HD_VIDEOPROC_358_IN_0 = HD_VIDEOPROC_IN(358, 0),
	/* DEVICE 359 */
	HD_VIDEOPROC_359_IN_0 = HD_VIDEOPROC_IN(359, 0),
	/* DEVICE 360 */
	HD_VIDEOPROC_360_IN_0 = HD_VIDEOPROC_IN(360, 0),
	/* DEVICE 361 */
	HD_VIDEOPROC_361_IN_0 = HD_VIDEOPROC_IN(361, 0),
	/* DEVICE 362 */
	HD_VIDEOPROC_362_IN_0 = HD_VIDEOPROC_IN(362, 0),
	/* DEVICE 363 */
	HD_VIDEOPROC_363_IN_0 = HD_VIDEOPROC_IN(363, 0),
	/* DEVICE 364 */
	HD_VIDEOPROC_364_IN_0 = HD_VIDEOPROC_IN(364, 0),
	/* DEVICE 365 */
	HD_VIDEOPROC_365_IN_0 = HD_VIDEOPROC_IN(365, 0),
	/* DEVICE 366 */
	HD_VIDEOPROC_366_IN_0 = HD_VIDEOPROC_IN(366, 0),
	/* DEVICE 367 */
	HD_VIDEOPROC_367_IN_0 = HD_VIDEOPROC_IN(367, 0),
	/* DEVICE 368 */
	HD_VIDEOPROC_368_IN_0 = HD_VIDEOPROC_IN(368, 0),
	/* DEVICE 369 */
	HD_VIDEOPROC_369_IN_0 = HD_VIDEOPROC_IN(369, 0),
	/* DEVICE 370 */
	HD_VIDEOPROC_370_IN_0 = HD_VIDEOPROC_IN(370, 0),
	/* DEVICE 371 */
	HD_VIDEOPROC_371_IN_0 = HD_VIDEOPROC_IN(371, 0),
	/* DEVICE 372 */
	HD_VIDEOPROC_372_IN_0 = HD_VIDEOPROC_IN(372, 0),
	/* DEVICE 373 */
	HD_VIDEOPROC_373_IN_0 = HD_VIDEOPROC_IN(373, 0),
	/* DEVICE 374 */
	HD_VIDEOPROC_374_IN_0 = HD_VIDEOPROC_IN(374, 0),
	/* DEVICE 375 */
	HD_VIDEOPROC_375_IN_0 = HD_VIDEOPROC_IN(375, 0),
	/* DEVICE 376 */
	HD_VIDEOPROC_376_IN_0 = HD_VIDEOPROC_IN(376, 0),
	/* DEVICE 377 */
	HD_VIDEOPROC_377_IN_0 = HD_VIDEOPROC_IN(377, 0),
	/* DEVICE 378 */
	HD_VIDEOPROC_378_IN_0 = HD_VIDEOPROC_IN(378, 0),
	/* DEVICE 379 */
	HD_VIDEOPROC_379_IN_0 = HD_VIDEOPROC_IN(379, 0),
	/* DEVICE 380 */
	HD_VIDEOPROC_380_IN_0 = HD_VIDEOPROC_IN(380, 0),
	/* DEVICE 381 */
	HD_VIDEOPROC_381_IN_0 = HD_VIDEOPROC_IN(381, 0),
	/* DEVICE 382 */
	HD_VIDEOPROC_382_IN_0 = HD_VIDEOPROC_IN(382, 0),
	/* DEVICE 383 */
	HD_VIDEOPROC_383_IN_0 = HD_VIDEOPROC_IN(383, 0),
	/* DEVICE 384 */
	HD_VIDEOPROC_384_IN_0 = HD_VIDEOPROC_IN(384, 0),
	/* DEVICE 385 */
	HD_VIDEOPROC_385_IN_0 = HD_VIDEOPROC_IN(385, 0),
	/* DEVICE 386 */
	HD_VIDEOPROC_386_IN_0 = HD_VIDEOPROC_IN(386, 0),
	/* DEVICE 387 */
	HD_VIDEOPROC_387_IN_0 = HD_VIDEOPROC_IN(387, 0),
	/* DEVICE 388 */
	HD_VIDEOPROC_388_IN_0 = HD_VIDEOPROC_IN(388, 0),
	/* DEVICE 389 */
	HD_VIDEOPROC_389_IN_0 = HD_VIDEOPROC_IN(389, 0),
	/* DEVICE 390 */
	HD_VIDEOPROC_390_IN_0 = HD_VIDEOPROC_IN(390, 0),
	/* DEVICE 391 */
	HD_VIDEOPROC_391_IN_0 = HD_VIDEOPROC_IN(391, 0),
	/* DEVICE 392 */
	HD_VIDEOPROC_392_IN_0 = HD_VIDEOPROC_IN(392, 0),
	/* DEVICE 393 */
	HD_VIDEOPROC_393_IN_0 = HD_VIDEOPROC_IN(393, 0),
	/* DEVICE 394 */
	HD_VIDEOPROC_394_IN_0 = HD_VIDEOPROC_IN(394, 0),
	/* DEVICE 395 */
	HD_VIDEOPROC_395_IN_0 = HD_VIDEOPROC_IN(395, 0),
	/* DEVICE 396 */
	HD_VIDEOPROC_396_IN_0 = HD_VIDEOPROC_IN(396, 0),
	/* DEVICE 397 */
	HD_VIDEOPROC_397_IN_0 = HD_VIDEOPROC_IN(397, 0),
	/* DEVICE 398 */
	HD_VIDEOPROC_398_IN_0 = HD_VIDEOPROC_IN(398, 0),
	/* DEVICE 399 */
	HD_VIDEOPROC_399_IN_0 = HD_VIDEOPROC_IN(399, 0),
	/* DEVICE 400 */
	HD_VIDEOPROC_400_IN_0 = HD_VIDEOPROC_IN(400, 0),
	/* DEVICE 401 */
	HD_VIDEOPROC_401_IN_0 = HD_VIDEOPROC_IN(401, 0),
	/* DEVICE 402 */
	HD_VIDEOPROC_402_IN_0 = HD_VIDEOPROC_IN(402, 0),
	/* DEVICE 403 */
	HD_VIDEOPROC_403_IN_0 = HD_VIDEOPROC_IN(403, 0),
	/* DEVICE 404 */
	HD_VIDEOPROC_404_IN_0 = HD_VIDEOPROC_IN(404, 0),
	/* DEVICE 405 */
	HD_VIDEOPROC_405_IN_0 = HD_VIDEOPROC_IN(405, 0),
	/* DEVICE 406 */
	HD_VIDEOPROC_406_IN_0 = HD_VIDEOPROC_IN(406, 0),
	/* DEVICE 407 */
	HD_VIDEOPROC_407_IN_0 = HD_VIDEOPROC_IN(407, 0),
	/* DEVICE 408 */
	HD_VIDEOPROC_408_IN_0 = HD_VIDEOPROC_IN(408, 0),
	/* DEVICE 409 */
	HD_VIDEOPROC_409_IN_0 = HD_VIDEOPROC_IN(409, 0),
	/* DEVICE 410 */
	HD_VIDEOPROC_410_IN_0 = HD_VIDEOPROC_IN(410, 0),
	/* DEVICE 411 */
	HD_VIDEOPROC_411_IN_0 = HD_VIDEOPROC_IN(411, 0),
	/* DEVICE 412 */
	HD_VIDEOPROC_412_IN_0 = HD_VIDEOPROC_IN(412, 0),
	/* DEVICE 413 */
	HD_VIDEOPROC_413_IN_0 = HD_VIDEOPROC_IN(413, 0),
	/* DEVICE 414 */
	HD_VIDEOPROC_414_IN_0 = HD_VIDEOPROC_IN(414, 0),
	/* DEVICE 415 */
	HD_VIDEOPROC_415_IN_0 = HD_VIDEOPROC_IN(415, 0),
	/* DEVICE 416 */
	HD_VIDEOPROC_416_IN_0 = HD_VIDEOPROC_IN(416, 0),
	/* DEVICE 417 */
	HD_VIDEOPROC_417_IN_0 = HD_VIDEOPROC_IN(417, 0),
	/* DEVICE 418 */
	HD_VIDEOPROC_418_IN_0 = HD_VIDEOPROC_IN(418, 0),
	/* DEVICE 419 */
	HD_VIDEOPROC_419_IN_0 = HD_VIDEOPROC_IN(419, 0),
	/* DEVICE 420 */
	HD_VIDEOPROC_420_IN_0 = HD_VIDEOPROC_IN(420, 0),
	/* DEVICE 421 */
	HD_VIDEOPROC_421_IN_0 = HD_VIDEOPROC_IN(421, 0),
	/* DEVICE 422 */
	HD_VIDEOPROC_422_IN_0 = HD_VIDEOPROC_IN(422, 0),
	/* DEVICE 423 */
	HD_VIDEOPROC_423_IN_0 = HD_VIDEOPROC_IN(423, 0),
	/* DEVICE 424 */
	HD_VIDEOPROC_424_IN_0 = HD_VIDEOPROC_IN(424, 0),
	/* DEVICE 425 */
	HD_VIDEOPROC_425_IN_0 = HD_VIDEOPROC_IN(425, 0),
	/* DEVICE 426 */
	HD_VIDEOPROC_426_IN_0 = HD_VIDEOPROC_IN(426, 0),
	/* DEVICE 427 */
	HD_VIDEOPROC_427_IN_0 = HD_VIDEOPROC_IN(427, 0),
	/* DEVICE 428 */
	HD_VIDEOPROC_428_IN_0 = HD_VIDEOPROC_IN(428, 0),
	/* DEVICE 429 */
	HD_VIDEOPROC_429_IN_0 = HD_VIDEOPROC_IN(429, 0),
	/* DEVICE 430 */
	HD_VIDEOPROC_430_IN_0 = HD_VIDEOPROC_IN(430, 0),
	/* DEVICE 431 */
	HD_VIDEOPROC_431_IN_0 = HD_VIDEOPROC_IN(431, 0),
	/* DEVICE 432 */
	HD_VIDEOPROC_432_IN_0 = HD_VIDEOPROC_IN(432, 0),
	/* DEVICE 433 */
	HD_VIDEOPROC_433_IN_0 = HD_VIDEOPROC_IN(433, 0),
	/* DEVICE 434 */
	HD_VIDEOPROC_434_IN_0 = HD_VIDEOPROC_IN(434, 0),
	/* DEVICE 435 */
	HD_VIDEOPROC_435_IN_0 = HD_VIDEOPROC_IN(435, 0),
	/* DEVICE 436 */
	HD_VIDEOPROC_436_IN_0 = HD_VIDEOPROC_IN(436, 0),
	/* DEVICE 437 */
	HD_VIDEOPROC_437_IN_0 = HD_VIDEOPROC_IN(437, 0),
	/* DEVICE 438 */
	HD_VIDEOPROC_438_IN_0 = HD_VIDEOPROC_IN(438, 0),
	/* DEVICE 439 */
	HD_VIDEOPROC_439_IN_0 = HD_VIDEOPROC_IN(439, 0),
	/* DEVICE 440 */
	HD_VIDEOPROC_440_IN_0 = HD_VIDEOPROC_IN(440, 0),
	/* DEVICE 441 */
	HD_VIDEOPROC_441_IN_0 = HD_VIDEOPROC_IN(441, 0),
	/* DEVICE 442 */
	HD_VIDEOPROC_442_IN_0 = HD_VIDEOPROC_IN(442, 0),
	/* DEVICE 443 */
	HD_VIDEOPROC_443_IN_0 = HD_VIDEOPROC_IN(443, 0),
	/* DEVICE 444 */
	HD_VIDEOPROC_444_IN_0 = HD_VIDEOPROC_IN(444, 0),
	/* DEVICE 445 */
	HD_VIDEOPROC_445_IN_0 = HD_VIDEOPROC_IN(445, 0),
	/* DEVICE 446 */
	HD_VIDEOPROC_446_IN_0 = HD_VIDEOPROC_IN(446, 0),
	/* DEVICE 447 */
	HD_VIDEOPROC_447_IN_0 = HD_VIDEOPROC_IN(447, 0),
	/* DEVICE 448 */
	HD_VIDEOPROC_448_IN_0 = HD_VIDEOPROC_IN(448, 0),
	/* DEVICE 449 */
	HD_VIDEOPROC_449_IN_0 = HD_VIDEOPROC_IN(449, 0),
	/* DEVICE 450 */
	HD_VIDEOPROC_450_IN_0 = HD_VIDEOPROC_IN(450, 0),
	/* DEVICE 451 */
	HD_VIDEOPROC_451_IN_0 = HD_VIDEOPROC_IN(451, 0),
	/* DEVICE 452 */
	HD_VIDEOPROC_452_IN_0 = HD_VIDEOPROC_IN(452, 0),
	/* DEVICE 453 */
	HD_VIDEOPROC_453_IN_0 = HD_VIDEOPROC_IN(453, 0),
	/* DEVICE 454 */
	HD_VIDEOPROC_454_IN_0 = HD_VIDEOPROC_IN(454, 0),
	/* DEVICE 455 */
	HD_VIDEOPROC_455_IN_0 = HD_VIDEOPROC_IN(455, 0),
	/* DEVICE 456 */
	HD_VIDEOPROC_456_IN_0 = HD_VIDEOPROC_IN(456, 0),
	/* DEVICE 457 */
	HD_VIDEOPROC_457_IN_0 = HD_VIDEOPROC_IN(457, 0),
	/* DEVICE 458 */
	HD_VIDEOPROC_458_IN_0 = HD_VIDEOPROC_IN(458, 0),
	/* DEVICE 459 */
	HD_VIDEOPROC_459_IN_0 = HD_VIDEOPROC_IN(459, 0),
	/* DEVICE 460 */
	HD_VIDEOPROC_460_IN_0 = HD_VIDEOPROC_IN(460, 0),
	/* DEVICE 461 */
	HD_VIDEOPROC_461_IN_0 = HD_VIDEOPROC_IN(461, 0),
	/* DEVICE 462 */
	HD_VIDEOPROC_462_IN_0 = HD_VIDEOPROC_IN(462, 0),
	/* DEVICE 463 */
	HD_VIDEOPROC_463_IN_0 = HD_VIDEOPROC_IN(463, 0),
	/* DEVICE 464 */
	HD_VIDEOPROC_464_IN_0 = HD_VIDEOPROC_IN(464, 0),
	/* DEVICE 465 */
	HD_VIDEOPROC_465_IN_0 = HD_VIDEOPROC_IN(465, 0),
	/* DEVICE 466 */
	HD_VIDEOPROC_466_IN_0 = HD_VIDEOPROC_IN(466, 0),
	/* DEVICE 467 */
	HD_VIDEOPROC_467_IN_0 = HD_VIDEOPROC_IN(467, 0),
	/* DEVICE 468 */
	HD_VIDEOPROC_468_IN_0 = HD_VIDEOPROC_IN(468, 0),
	/* DEVICE 469 */
	HD_VIDEOPROC_469_IN_0 = HD_VIDEOPROC_IN(469, 0),
	/* DEVICE 470 */
	HD_VIDEOPROC_470_IN_0 = HD_VIDEOPROC_IN(470, 0),
	/* DEVICE 471 */
	HD_VIDEOPROC_471_IN_0 = HD_VIDEOPROC_IN(471, 0),
	/* DEVICE 472 */
	HD_VIDEOPROC_472_IN_0 = HD_VIDEOPROC_IN(472, 0),
	/* DEVICE 473 */
	HD_VIDEOPROC_473_IN_0 = HD_VIDEOPROC_IN(473, 0),
	/* DEVICE 474 */
	HD_VIDEOPROC_474_IN_0 = HD_VIDEOPROC_IN(474, 0),
	/* DEVICE 475 */
	HD_VIDEOPROC_475_IN_0 = HD_VIDEOPROC_IN(475, 0),
	/* DEVICE 476 */
	HD_VIDEOPROC_476_IN_0 = HD_VIDEOPROC_IN(476, 0),
	/* DEVICE 477 */
	HD_VIDEOPROC_477_IN_0 = HD_VIDEOPROC_IN(477, 0),
	/* DEVICE 478 */
	HD_VIDEOPROC_478_IN_0 = HD_VIDEOPROC_IN(478, 0),
	/* DEVICE 479 */
	HD_VIDEOPROC_479_IN_0 = HD_VIDEOPROC_IN(479, 0),
	/* DEVICE 480 */
	HD_VIDEOPROC_480_IN_0 = HD_VIDEOPROC_IN(480, 0),
	/* DEVICE 481 */
	HD_VIDEOPROC_481_IN_0 = HD_VIDEOPROC_IN(481, 0),
	/* DEVICE 482 */
	HD_VIDEOPROC_482_IN_0 = HD_VIDEOPROC_IN(482, 0),
	/* DEVICE 483 */
	HD_VIDEOPROC_483_IN_0 = HD_VIDEOPROC_IN(483, 0),
	/* DEVICE 484 */
	HD_VIDEOPROC_484_IN_0 = HD_VIDEOPROC_IN(484, 0),
	/* DEVICE 485 */
	HD_VIDEOPROC_485_IN_0 = HD_VIDEOPROC_IN(485, 0),
	/* DEVICE 486 */
	HD_VIDEOPROC_486_IN_0 = HD_VIDEOPROC_IN(486, 0),
	/* DEVICE 487 */
	HD_VIDEOPROC_487_IN_0 = HD_VIDEOPROC_IN(487, 0),
	/* DEVICE 488 */
	HD_VIDEOPROC_488_IN_0 = HD_VIDEOPROC_IN(488, 0),
	/* DEVICE 489 */
	HD_VIDEOPROC_489_IN_0 = HD_VIDEOPROC_IN(489, 0),
	/* DEVICE 490 */
	HD_VIDEOPROC_490_IN_0 = HD_VIDEOPROC_IN(490, 0),
	/* DEVICE 491 */
	HD_VIDEOPROC_491_IN_0 = HD_VIDEOPROC_IN(491, 0),
	/* DEVICE 492 */
	HD_VIDEOPROC_492_IN_0 = HD_VIDEOPROC_IN(492, 0),
	/* DEVICE 493 */
	HD_VIDEOPROC_493_IN_0 = HD_VIDEOPROC_IN(493, 0),
	/* DEVICE 494 */
	HD_VIDEOPROC_494_IN_0 = HD_VIDEOPROC_IN(494, 0),
	/* DEVICE 495 */
	HD_VIDEOPROC_495_IN_0 = HD_VIDEOPROC_IN(495, 0),
	/* DEVICE 496 */
	HD_VIDEOPROC_496_IN_0 = HD_VIDEOPROC_IN(496, 0),
	/* DEVICE 497 */
	HD_VIDEOPROC_497_IN_0 = HD_VIDEOPROC_IN(497, 0),
	/* DEVICE 498 */
	HD_VIDEOPROC_498_IN_0 = HD_VIDEOPROC_IN(498, 0),
	/* DEVICE 499 */
	HD_VIDEOPROC_499_IN_0 = HD_VIDEOPROC_IN(499, 0),
	/* DEVICE 500 */
	HD_VIDEOPROC_500_IN_0 = HD_VIDEOPROC_IN(500, 0),
	/* DEVICE 501 */
	HD_VIDEOPROC_501_IN_0 = HD_VIDEOPROC_IN(501, 0),
	/* DEVICE 502 */
	HD_VIDEOPROC_502_IN_0 = HD_VIDEOPROC_IN(502, 0),
	/* DEVICE 503 */
	HD_VIDEOPROC_503_IN_0 = HD_VIDEOPROC_IN(503, 0),
	/* DEVICE 504 */
	HD_VIDEOPROC_504_IN_0 = HD_VIDEOPROC_IN(504, 0),
	/* DEVICE 505 */
	HD_VIDEOPROC_505_IN_0 = HD_VIDEOPROC_IN(505, 0),
	/* DEVICE 506 */
	HD_VIDEOPROC_506_IN_0 = HD_VIDEOPROC_IN(506, 0),
	/* DEVICE 507 */
	HD_VIDEOPROC_507_IN_0 = HD_VIDEOPROC_IN(507, 0),
	/* DEVICE 508 */
	HD_VIDEOPROC_508_IN_0 = HD_VIDEOPROC_IN(508, 0),
	/* DEVICE 509 */
	HD_VIDEOPROC_509_IN_0 = HD_VIDEOPROC_IN(509, 0),
	/* DEVICE 510 */
	HD_VIDEOPROC_510_IN_0 = HD_VIDEOPROC_IN(510, 0),
	/* DEVICE 511 */
	HD_VIDEOPROC_511_IN_0 = HD_VIDEOPROC_IN(511, 0),


	/* DEVICE 0 */
	HD_AUDIOCAP_0_IN_0 = HD_AUDIOCAP_IN(0, 0),
	/* DEVICE 1 */
	HD_AUDIOCAP_1_IN_0 = HD_AUDIOCAP_IN(1, 0),
	/* DEVICE 2 */
	HD_AUDIOCAP_2_IN_0 = HD_AUDIOCAP_IN(2, 0),
	/* DEVICE 3 */
	HD_AUDIOCAP_3_IN_0 = HD_AUDIOCAP_IN(3, 0),
	/* DEVICE 4 */
	HD_AUDIOCAP_4_IN_0 = HD_AUDIOCAP_IN(4, 0),
	/* DEVICE 5 */
	HD_AUDIOCAP_5_IN_0 = HD_AUDIOCAP_IN(5, 0),
	/* DEVICE 6 */
	HD_AUDIOCAP_6_IN_0 = HD_AUDIOCAP_IN(6, 0),
	/* DEVICE 7 */
	HD_AUDIOCAP_7_IN_0 = HD_AUDIOCAP_IN(7, 0),
	/* DEVICE 8 */
	HD_AUDIOCAP_8_IN_0 = HD_AUDIOCAP_IN(8, 0),
	/* DEVICE 9 */
	HD_AUDIOCAP_9_IN_0 = HD_AUDIOCAP_IN(9, 0),
	/* DEVICE 10 */
	HD_AUDIOCAP_10_IN_0 = HD_AUDIOCAP_IN(10, 0),
	/* DEVICE 11 */
	HD_AUDIOCAP_11_IN_0 = HD_AUDIOCAP_IN(11, 0),
	/* DEVICE 12 */
	HD_AUDIOCAP_12_IN_0 = HD_AUDIOCAP_IN(12, 0),
	/* DEVICE 13 */
	HD_AUDIOCAP_13_IN_0 = HD_AUDIOCAP_IN(13, 0),
	/* DEVICE 14 */
	HD_AUDIOCAP_14_IN_0 = HD_AUDIOCAP_IN(14, 0),
	/* DEVICE 15 */
	HD_AUDIOCAP_15_IN_0 = HD_AUDIOCAP_IN(15, 0),
	/* DEVICE 16 */
	HD_AUDIOCAP_16_IN_0 = HD_AUDIOCAP_IN(16, 0),
	/* DEVICE 17 */
	HD_AUDIOCAP_17_IN_0 = HD_AUDIOCAP_IN(17, 0),
	/* DEVICE 18 */
	HD_AUDIOCAP_18_IN_0 = HD_AUDIOCAP_IN(18, 0),
	/* DEVICE 19 */
	HD_AUDIOCAP_19_IN_0 = HD_AUDIOCAP_IN(19, 0),


	/* DEVICE 0 */
	HD_AUDIOOUT_0_IN_0 = HD_AUDIOOUT_IN(0, 0),
	/* DEVICE 1 */
	HD_AUDIOOUT_1_IN_0 = HD_AUDIOOUT_IN(1, 0),
	/* DEVICE 2 */
	HD_AUDIOOUT_2_IN_0 = HD_AUDIOOUT_IN(2, 0),
	/* DEVICE 3 */
	HD_AUDIOOUT_3_IN_0 = HD_AUDIOOUT_IN(3, 0),
	/* DEVICE 4 */
	HD_AUDIOOUT_4_IN_0 = HD_AUDIOOUT_IN(4, 0),
	/* DEVICE 5 */
	HD_AUDIOOUT_5_IN_0 = HD_AUDIOOUT_IN(5, 0),
	/* DEVICE 6 */
	HD_AUDIOOUT_6_IN_0 = HD_AUDIOOUT_IN(6, 0),
	/* DEVICE 7 */
	HD_AUDIOOUT_7_IN_0 = HD_AUDIOOUT_IN(7, 0),
	/* DEVICE 8 */
	HD_AUDIOOUT_8_IN_0 = HD_AUDIOOUT_IN(8, 0),
	/* DEVICE 9 */
	HD_AUDIOOUT_9_IN_0 = HD_AUDIOOUT_IN(9, 0),
	/* DEVICE 10 */
	HD_AUDIOOUT_10_IN_0 = HD_AUDIOOUT_IN(10, 0),
	/* DEVICE 11 */
	HD_AUDIOOUT_11_IN_0 = HD_AUDIOOUT_IN(11, 0),
	/* DEVICE 12 */
	HD_AUDIOOUT_12_IN_0 = HD_AUDIOOUT_IN(12, 0),
	/* DEVICE 13 */
	HD_AUDIOOUT_13_IN_0 = HD_AUDIOOUT_IN(13, 0),
	/* DEVICE 14 */
	HD_AUDIOOUT_14_IN_0 = HD_AUDIOOUT_IN(14, 0),
	/* DEVICE 15 */
	HD_AUDIOOUT_15_IN_0 = HD_AUDIOOUT_IN(15, 0),
	/* DEVICE 16 */
	HD_AUDIOOUT_16_IN_0 = HD_AUDIOOUT_IN(16, 0),
	/* DEVICE 17 */
	HD_AUDIOOUT_17_IN_0 = HD_AUDIOOUT_IN(17, 0),
	/* DEVICE 18 */
	HD_AUDIOOUT_18_IN_0 = HD_AUDIOOUT_IN(18, 0),
	/* DEVICE 19 */
	HD_AUDIOOUT_19_IN_0 = HD_AUDIOOUT_IN(19, 0),


	/* DEVICE 0 */
	HD_AUDIOENC_0_IN_0 = HD_AUDIOENC_IN(0, 0),
	HD_AUDIOENC_0_IN_1 = HD_AUDIOENC_IN(0, 1),
	HD_AUDIOENC_0_IN_2 = HD_AUDIOENC_IN(0, 2),
	HD_AUDIOENC_0_IN_3 = HD_AUDIOENC_IN(0, 3),
	HD_AUDIOENC_0_IN_4 = HD_AUDIOENC_IN(0, 4),
	HD_AUDIOENC_0_IN_5 = HD_AUDIOENC_IN(0, 5),
	HD_AUDIOENC_0_IN_6 = HD_AUDIOENC_IN(0, 6),
	HD_AUDIOENC_0_IN_7 = HD_AUDIOENC_IN(0, 7),
	HD_AUDIOENC_0_IN_8 = HD_AUDIOENC_IN(0, 8),
	HD_AUDIOENC_0_IN_9 = HD_AUDIOENC_IN(0, 9),
	HD_AUDIOENC_0_IN_10 = HD_AUDIOENC_IN(0, 10),
	HD_AUDIOENC_0_IN_11 = HD_AUDIOENC_IN(0, 11),
	HD_AUDIOENC_0_IN_12 = HD_AUDIOENC_IN(0, 12),
	HD_AUDIOENC_0_IN_13 = HD_AUDIOENC_IN(0, 13),
	HD_AUDIOENC_0_IN_14 = HD_AUDIOENC_IN(0, 14),
	HD_AUDIOENC_0_IN_15 = HD_AUDIOENC_IN(0, 15),
	HD_AUDIOENC_0_IN_16 = HD_AUDIOENC_IN(0, 16),
	HD_AUDIOENC_0_IN_17 = HD_AUDIOENC_IN(0, 17),
	HD_AUDIOENC_0_IN_18 = HD_AUDIOENC_IN(0, 18),
	HD_AUDIOENC_0_IN_19 = HD_AUDIOENC_IN(0, 19),
	HD_AUDIOENC_0_IN_20 = HD_AUDIOENC_IN(0, 20),
	HD_AUDIOENC_0_IN_21 = HD_AUDIOENC_IN(0, 21),
	HD_AUDIOENC_0_IN_22 = HD_AUDIOENC_IN(0, 22),
	HD_AUDIOENC_0_IN_23 = HD_AUDIOENC_IN(0, 23),
	HD_AUDIOENC_0_IN_24 = HD_AUDIOENC_IN(0, 24),
	HD_AUDIOENC_0_IN_25 = HD_AUDIOENC_IN(0, 25),
	HD_AUDIOENC_0_IN_26 = HD_AUDIOENC_IN(0, 26),
	HD_AUDIOENC_0_IN_27 = HD_AUDIOENC_IN(0, 27),
	HD_AUDIOENC_0_IN_28 = HD_AUDIOENC_IN(0, 28),
	HD_AUDIOENC_0_IN_29 = HD_AUDIOENC_IN(0, 29),
	HD_AUDIOENC_0_IN_30 = HD_AUDIOENC_IN(0, 30),
	HD_AUDIOENC_0_IN_31 = HD_AUDIOENC_IN(0, 31),


	/* DEVICE 0 */
	HD_AUDIODEC_0_IN_0 = HD_AUDIODEC_IN(0, 0),
	HD_AUDIODEC_0_IN_1 = HD_AUDIODEC_IN(0, 1),
	HD_AUDIODEC_0_IN_2 = HD_AUDIODEC_IN(0, 2),
	HD_AUDIODEC_0_IN_3 = HD_AUDIODEC_IN(0, 3),
	HD_AUDIODEC_0_IN_4 = HD_AUDIODEC_IN(0, 4),
	HD_AUDIODEC_0_IN_5 = HD_AUDIODEC_IN(0, 5),
	HD_AUDIODEC_0_IN_6 = HD_AUDIODEC_IN(0, 6),
	HD_AUDIODEC_0_IN_7 = HD_AUDIODEC_IN(0, 7),
	HD_AUDIODEC_0_IN_8 = HD_AUDIODEC_IN(0, 8),
	HD_AUDIODEC_0_IN_9 = HD_AUDIODEC_IN(0, 9),
	HD_AUDIODEC_0_IN_10 = HD_AUDIODEC_IN(0, 10),
	HD_AUDIODEC_0_IN_11 = HD_AUDIODEC_IN(0, 11),
	HD_AUDIODEC_0_IN_12 = HD_AUDIODEC_IN(0, 12),
	HD_AUDIODEC_0_IN_13 = HD_AUDIODEC_IN(0, 13),
	HD_AUDIODEC_0_IN_14 = HD_AUDIODEC_IN(0, 14),
	HD_AUDIODEC_0_IN_15 = HD_AUDIODEC_IN(0, 15),
	HD_AUDIODEC_0_IN_16 = HD_AUDIODEC_IN(0, 16),
	HD_AUDIODEC_0_IN_17 = HD_AUDIODEC_IN(0, 17),
	HD_AUDIODEC_0_IN_18 = HD_AUDIODEC_IN(0, 18),
	HD_AUDIODEC_0_IN_19 = HD_AUDIODEC_IN(0, 19),
	HD_AUDIODEC_0_IN_20 = HD_AUDIODEC_IN(0, 10),
	HD_AUDIODEC_0_IN_21 = HD_AUDIODEC_IN(0, 21),
	HD_AUDIODEC_0_IN_22 = HD_AUDIODEC_IN(0, 22),
	HD_AUDIODEC_0_IN_23 = HD_AUDIODEC_IN(0, 23),
	HD_AUDIODEC_0_IN_24 = HD_AUDIODEC_IN(0, 24),
	HD_AUDIODEC_0_IN_25 = HD_AUDIODEC_IN(0, 25),
	HD_AUDIODEC_0_IN_26 = HD_AUDIODEC_IN(0, 26),
	HD_AUDIODEC_0_IN_27 = HD_AUDIODEC_IN(0, 27),
	HD_AUDIODEC_0_IN_28 = HD_AUDIODEC_IN(0, 28),
	HD_AUDIODEC_0_IN_29 = HD_AUDIODEC_IN(0, 29),
	HD_AUDIODEC_0_IN_30 = HD_AUDIODEC_IN(0, 30),
	HD_AUDIODEC_0_IN_31 = HD_AUDIODEC_IN(0, 31),

	ENUM_DUMMY4WORD(HD_IN_ID)
} HD_IN_ID;


/**
     @name type of device output id.
*/
typedef enum _HD_OUT_ID {

	/* DEVICE 0 */
	HD_VIDEOCAP_0_OUT_0 = HD_VIDEOCAP_OUT(0, 0),
	HD_VIDEOCAP_0_OUT_1 = HD_VIDEOCAP_OUT(0, 1),
	HD_VIDEOCAP_0_OUT_2 = HD_VIDEOCAP_OUT(0, 2),
	HD_VIDEOCAP_0_OUT_3 = HD_VIDEOCAP_OUT(0, 3),
	HD_VIDEOCAP_0_OUT_4 = HD_VIDEOCAP_OUT(0, 4),
	HD_VIDEOCAP_0_OUT_5 = HD_VIDEOCAP_OUT(0, 5),
	HD_VIDEOCAP_0_OUT_6 = HD_VIDEOCAP_OUT(0, 6),
	HD_VIDEOCAP_0_OUT_7 = HD_VIDEOCAP_OUT(0, 7),
	/* DEVICE 1 */
	HD_VIDEOCAP_1_OUT_0 = HD_VIDEOCAP_OUT(1, 0),
	HD_VIDEOCAP_1_OUT_1 = HD_VIDEOCAP_OUT(1, 1),
	HD_VIDEOCAP_1_OUT_2 = HD_VIDEOCAP_OUT(1, 2),
	HD_VIDEOCAP_1_OUT_3 = HD_VIDEOCAP_OUT(1, 3),
	HD_VIDEOCAP_1_OUT_4 = HD_VIDEOCAP_OUT(1, 4),
	HD_VIDEOCAP_1_OUT_5 = HD_VIDEOCAP_OUT(1, 5),
	HD_VIDEOCAP_1_OUT_6 = HD_VIDEOCAP_OUT(1, 6),
	HD_VIDEOCAP_1_OUT_7 = HD_VIDEOCAP_OUT(1, 7),
	/* DEVICE 2 */
	HD_VIDEOCAP_2_OUT_0 = HD_VIDEOCAP_OUT(2, 0),
	HD_VIDEOCAP_2_OUT_1 = HD_VIDEOCAP_OUT(2, 1),
	HD_VIDEOCAP_2_OUT_2 = HD_VIDEOCAP_OUT(2, 2),
	HD_VIDEOCAP_2_OUT_3 = HD_VIDEOCAP_OUT(2, 3),
	HD_VIDEOCAP_2_OUT_4 = HD_VIDEOCAP_OUT(2, 4),
	HD_VIDEOCAP_2_OUT_5 = HD_VIDEOCAP_OUT(2, 5),
	HD_VIDEOCAP_2_OUT_6 = HD_VIDEOCAP_OUT(2, 6),
	HD_VIDEOCAP_2_OUT_7 = HD_VIDEOCAP_OUT(2, 7),
	/* DEVICE 3 */
	HD_VIDEOCAP_3_OUT_0 = HD_VIDEOCAP_OUT(3, 0),
	HD_VIDEOCAP_3_OUT_1 = HD_VIDEOCAP_OUT(3, 1),
	HD_VIDEOCAP_3_OUT_2 = HD_VIDEOCAP_OUT(3, 2),
	HD_VIDEOCAP_3_OUT_3 = HD_VIDEOCAP_OUT(3, 3),
	HD_VIDEOCAP_3_OUT_4 = HD_VIDEOCAP_OUT(3, 4),
	HD_VIDEOCAP_3_OUT_5 = HD_VIDEOCAP_OUT(3, 5),
	HD_VIDEOCAP_3_OUT_6 = HD_VIDEOCAP_OUT(3, 6),
	HD_VIDEOCAP_3_OUT_7 = HD_VIDEOCAP_OUT(3, 7),
	/* DEVICE 4 */
	HD_VIDEOCAP_4_OUT_0 = HD_VIDEOCAP_OUT(4, 0),
	HD_VIDEOCAP_4_OUT_1 = HD_VIDEOCAP_OUT(4, 1),
	HD_VIDEOCAP_4_OUT_2 = HD_VIDEOCAP_OUT(4, 2),
	HD_VIDEOCAP_4_OUT_3 = HD_VIDEOCAP_OUT(4, 3),
	HD_VIDEOCAP_4_OUT_4 = HD_VIDEOCAP_OUT(4, 4),
	HD_VIDEOCAP_4_OUT_5 = HD_VIDEOCAP_OUT(4, 5),
	HD_VIDEOCAP_4_OUT_6 = HD_VIDEOCAP_OUT(4, 6),
	HD_VIDEOCAP_4_OUT_7 = HD_VIDEOCAP_OUT(4, 7),
	/* DEVICE 5 */
	HD_VIDEOCAP_5_OUT_0 = HD_VIDEOCAP_OUT(5, 0),
	HD_VIDEOCAP_5_OUT_1 = HD_VIDEOCAP_OUT(5, 1),
	HD_VIDEOCAP_5_OUT_2 = HD_VIDEOCAP_OUT(5, 2),
	HD_VIDEOCAP_5_OUT_3 = HD_VIDEOCAP_OUT(5, 3),
	HD_VIDEOCAP_5_OUT_4 = HD_VIDEOCAP_OUT(5, 4),
	HD_VIDEOCAP_5_OUT_5 = HD_VIDEOCAP_OUT(5, 5),
	HD_VIDEOCAP_5_OUT_6 = HD_VIDEOCAP_OUT(5, 6),
	HD_VIDEOCAP_5_OUT_7 = HD_VIDEOCAP_OUT(5, 7),
	/* DEVICE 6 */
	HD_VIDEOCAP_6_OUT_0 = HD_VIDEOCAP_OUT(6, 0),
	HD_VIDEOCAP_6_OUT_1 = HD_VIDEOCAP_OUT(6, 1),
	HD_VIDEOCAP_6_OUT_2 = HD_VIDEOCAP_OUT(6, 2),
	HD_VIDEOCAP_6_OUT_3 = HD_VIDEOCAP_OUT(6, 3),
	HD_VIDEOCAP_6_OUT_4 = HD_VIDEOCAP_OUT(6, 4),
	HD_VIDEOCAP_6_OUT_5 = HD_VIDEOCAP_OUT(6, 5),
	HD_VIDEOCAP_6_OUT_6 = HD_VIDEOCAP_OUT(6, 6),
	HD_VIDEOCAP_6_OUT_7 = HD_VIDEOCAP_OUT(6, 7),
	/* DEVICE 7 */
	HD_VIDEOCAP_7_OUT_0 = HD_VIDEOCAP_OUT(7, 0),
	HD_VIDEOCAP_7_OUT_1 = HD_VIDEOCAP_OUT(7, 1),
	HD_VIDEOCAP_7_OUT_2 = HD_VIDEOCAP_OUT(7, 2),
	HD_VIDEOCAP_7_OUT_3 = HD_VIDEOCAP_OUT(7, 3),
	HD_VIDEOCAP_7_OUT_4 = HD_VIDEOCAP_OUT(7, 4),
	HD_VIDEOCAP_7_OUT_5 = HD_VIDEOCAP_OUT(7, 5),
	HD_VIDEOCAP_7_OUT_6 = HD_VIDEOCAP_OUT(7, 6),
	HD_VIDEOCAP_7_OUT_7 = HD_VIDEOCAP_OUT(7, 7),
	/* DEVICE 8 */
	HD_VIDEOCAP_8_OUT_0 = HD_VIDEOCAP_OUT(8, 0),
	HD_VIDEOCAP_8_OUT_1 = HD_VIDEOCAP_OUT(8, 1),
	HD_VIDEOCAP_8_OUT_2 = HD_VIDEOCAP_OUT(8, 2),
	HD_VIDEOCAP_8_OUT_3 = HD_VIDEOCAP_OUT(8, 3),
	HD_VIDEOCAP_8_OUT_4 = HD_VIDEOCAP_OUT(8, 4),
	HD_VIDEOCAP_8_OUT_5 = HD_VIDEOCAP_OUT(8, 5),
	HD_VIDEOCAP_8_OUT_6 = HD_VIDEOCAP_OUT(8, 6),
	HD_VIDEOCAP_8_OUT_7 = HD_VIDEOCAP_OUT(8, 7),
	/* DEVICE 9 */
	HD_VIDEOCAP_9_OUT_0 = HD_VIDEOCAP_OUT(9, 0),
	HD_VIDEOCAP_9_OUT_1 = HD_VIDEOCAP_OUT(9, 1),
	HD_VIDEOCAP_9_OUT_2 = HD_VIDEOCAP_OUT(9, 2),
	HD_VIDEOCAP_9_OUT_3 = HD_VIDEOCAP_OUT(9, 3),
	HD_VIDEOCAP_9_OUT_4 = HD_VIDEOCAP_OUT(9, 4),
	HD_VIDEOCAP_9_OUT_5 = HD_VIDEOCAP_OUT(9, 5),
	HD_VIDEOCAP_9_OUT_6 = HD_VIDEOCAP_OUT(9, 6),
	HD_VIDEOCAP_9_OUT_7 = HD_VIDEOCAP_OUT(9, 7),
	/* DEVICE 10 */
	HD_VIDEOCAP_10_OUT_0 = HD_VIDEOCAP_OUT(10, 0),
	HD_VIDEOCAP_10_OUT_1 = HD_VIDEOCAP_OUT(10, 1),
	HD_VIDEOCAP_10_OUT_2 = HD_VIDEOCAP_OUT(10, 2),
	HD_VIDEOCAP_10_OUT_3 = HD_VIDEOCAP_OUT(10, 3),
	HD_VIDEOCAP_10_OUT_4 = HD_VIDEOCAP_OUT(10, 4),
	HD_VIDEOCAP_10_OUT_5 = HD_VIDEOCAP_OUT(10, 5),
	HD_VIDEOCAP_10_OUT_6 = HD_VIDEOCAP_OUT(10, 6),
	HD_VIDEOCAP_10_OUT_7 = HD_VIDEOCAP_OUT(10, 7),
	/* DEVICE 11 */
	HD_VIDEOCAP_11_OUT_0 = HD_VIDEOCAP_OUT(11, 0),
	HD_VIDEOCAP_11_OUT_1 = HD_VIDEOCAP_OUT(11, 1),
	HD_VIDEOCAP_11_OUT_2 = HD_VIDEOCAP_OUT(11, 2),
	HD_VIDEOCAP_11_OUT_3 = HD_VIDEOCAP_OUT(11, 3),
	HD_VIDEOCAP_11_OUT_4 = HD_VIDEOCAP_OUT(11, 4),
	HD_VIDEOCAP_11_OUT_5 = HD_VIDEOCAP_OUT(11, 5),
	HD_VIDEOCAP_11_OUT_6 = HD_VIDEOCAP_OUT(11, 6),
	HD_VIDEOCAP_11_OUT_7 = HD_VIDEOCAP_OUT(11, 7),
	/* DEVICE 12 */
	HD_VIDEOCAP_12_OUT_0 = HD_VIDEOCAP_OUT(12, 0),
	HD_VIDEOCAP_12_OUT_1 = HD_VIDEOCAP_OUT(12, 1),
	HD_VIDEOCAP_12_OUT_2 = HD_VIDEOCAP_OUT(12, 2),
	HD_VIDEOCAP_12_OUT_3 = HD_VIDEOCAP_OUT(12, 3),
	HD_VIDEOCAP_12_OUT_4 = HD_VIDEOCAP_OUT(12, 4),
	HD_VIDEOCAP_12_OUT_5 = HD_VIDEOCAP_OUT(12, 5),
	HD_VIDEOCAP_12_OUT_6 = HD_VIDEOCAP_OUT(12, 6),
	HD_VIDEOCAP_12_OUT_7 = HD_VIDEOCAP_OUT(12, 7),
	/* DEVICE 13 */
	HD_VIDEOCAP_13_OUT_0 = HD_VIDEOCAP_OUT(13, 0),
	HD_VIDEOCAP_13_OUT_1 = HD_VIDEOCAP_OUT(13, 1),
	HD_VIDEOCAP_13_OUT_2 = HD_VIDEOCAP_OUT(13, 2),
	HD_VIDEOCAP_13_OUT_3 = HD_VIDEOCAP_OUT(13, 3),
	HD_VIDEOCAP_13_OUT_4 = HD_VIDEOCAP_OUT(13, 4),
	HD_VIDEOCAP_13_OUT_5 = HD_VIDEOCAP_OUT(13, 5),
	HD_VIDEOCAP_13_OUT_6 = HD_VIDEOCAP_OUT(13, 6),
	HD_VIDEOCAP_13_OUT_7 = HD_VIDEOCAP_OUT(13, 7),
	/* DEVICE 14 */
	HD_VIDEOCAP_14_OUT_0 = HD_VIDEOCAP_OUT(14, 0),
	HD_VIDEOCAP_14_OUT_1 = HD_VIDEOCAP_OUT(14, 1),
	HD_VIDEOCAP_14_OUT_2 = HD_VIDEOCAP_OUT(14, 2),
	HD_VIDEOCAP_14_OUT_3 = HD_VIDEOCAP_OUT(14, 3),
	HD_VIDEOCAP_14_OUT_4 = HD_VIDEOCAP_OUT(14, 4),
	HD_VIDEOCAP_14_OUT_5 = HD_VIDEOCAP_OUT(14, 5),
	HD_VIDEOCAP_14_OUT_6 = HD_VIDEOCAP_OUT(14, 6),
	HD_VIDEOCAP_14_OUT_7 = HD_VIDEOCAP_OUT(14, 7),
	/* DEVICE 15 */
	HD_VIDEOCAP_15_OUT_0 = HD_VIDEOCAP_OUT(15, 0),
	HD_VIDEOCAP_15_OUT_1 = HD_VIDEOCAP_OUT(15, 1),
	HD_VIDEOCAP_15_OUT_2 = HD_VIDEOCAP_OUT(15, 2),
	HD_VIDEOCAP_15_OUT_3 = HD_VIDEOCAP_OUT(15, 3),
	HD_VIDEOCAP_15_OUT_4 = HD_VIDEOCAP_OUT(15, 4),
	HD_VIDEOCAP_15_OUT_5 = HD_VIDEOCAP_OUT(15, 5),
	HD_VIDEOCAP_15_OUT_6 = HD_VIDEOCAP_OUT(15, 6),
	HD_VIDEOCAP_15_OUT_7 = HD_VIDEOCAP_OUT(15, 7),
	/* DEVICE 16 */
	HD_VIDEOCAP_16_OUT_0 = HD_VIDEOCAP_OUT(16, 0),
	HD_VIDEOCAP_16_OUT_1 = HD_VIDEOCAP_OUT(16, 1),
	HD_VIDEOCAP_16_OUT_2 = HD_VIDEOCAP_OUT(16, 2),
	HD_VIDEOCAP_16_OUT_3 = HD_VIDEOCAP_OUT(16, 3),
	HD_VIDEOCAP_16_OUT_4 = HD_VIDEOCAP_OUT(16, 4),
	HD_VIDEOCAP_16_OUT_5 = HD_VIDEOCAP_OUT(16, 5),
	HD_VIDEOCAP_16_OUT_6 = HD_VIDEOCAP_OUT(16, 6),
	HD_VIDEOCAP_16_OUT_7 = HD_VIDEOCAP_OUT(16, 7),
	/* DEVICE 17 */
	HD_VIDEOCAP_17_OUT_0 = HD_VIDEOCAP_OUT(17, 0),
	HD_VIDEOCAP_17_OUT_1 = HD_VIDEOCAP_OUT(17, 1),
	HD_VIDEOCAP_17_OUT_2 = HD_VIDEOCAP_OUT(17, 2),
	HD_VIDEOCAP_17_OUT_3 = HD_VIDEOCAP_OUT(17, 3),
	HD_VIDEOCAP_17_OUT_4 = HD_VIDEOCAP_OUT(17, 4),
	HD_VIDEOCAP_17_OUT_5 = HD_VIDEOCAP_OUT(17, 5),
	HD_VIDEOCAP_17_OUT_6 = HD_VIDEOCAP_OUT(17, 6),
	HD_VIDEOCAP_17_OUT_7 = HD_VIDEOCAP_OUT(17, 7),
	/* DEVICE 18 */
	HD_VIDEOCAP_18_OUT_0 = HD_VIDEOCAP_OUT(18, 0),
	HD_VIDEOCAP_18_OUT_1 = HD_VIDEOCAP_OUT(18, 1),
	HD_VIDEOCAP_18_OUT_2 = HD_VIDEOCAP_OUT(18, 2),
	HD_VIDEOCAP_18_OUT_3 = HD_VIDEOCAP_OUT(18, 3),
	HD_VIDEOCAP_18_OUT_4 = HD_VIDEOCAP_OUT(18, 4),
	HD_VIDEOCAP_18_OUT_5 = HD_VIDEOCAP_OUT(18, 5),
	HD_VIDEOCAP_18_OUT_6 = HD_VIDEOCAP_OUT(18, 6),
	HD_VIDEOCAP_18_OUT_7 = HD_VIDEOCAP_OUT(18, 7),
	/* DEVICE 19 */
	HD_VIDEOCAP_19_OUT_0 = HD_VIDEOCAP_OUT(19, 0),
	HD_VIDEOCAP_19_OUT_1 = HD_VIDEOCAP_OUT(19, 1),
	HD_VIDEOCAP_19_OUT_2 = HD_VIDEOCAP_OUT(19, 2),
	HD_VIDEOCAP_19_OUT_3 = HD_VIDEOCAP_OUT(19, 3),
	HD_VIDEOCAP_19_OUT_4 = HD_VIDEOCAP_OUT(19, 4),
	HD_VIDEOCAP_19_OUT_5 = HD_VIDEOCAP_OUT(19, 5),
	HD_VIDEOCAP_19_OUT_6 = HD_VIDEOCAP_OUT(19, 6),
	HD_VIDEOCAP_19_OUT_7 = HD_VIDEOCAP_OUT(19, 7),
	/* DEVICE 20 */
	HD_VIDEOCAP_20_OUT_0 = HD_VIDEOCAP_OUT(20, 0),
	HD_VIDEOCAP_20_OUT_1 = HD_VIDEOCAP_OUT(20, 1),
	HD_VIDEOCAP_20_OUT_2 = HD_VIDEOCAP_OUT(20, 2),
	HD_VIDEOCAP_20_OUT_3 = HD_VIDEOCAP_OUT(20, 3),
	HD_VIDEOCAP_20_OUT_4 = HD_VIDEOCAP_OUT(20, 4),
	HD_VIDEOCAP_20_OUT_5 = HD_VIDEOCAP_OUT(20, 5),
	HD_VIDEOCAP_20_OUT_6 = HD_VIDEOCAP_OUT(20, 6),
	HD_VIDEOCAP_20_OUT_7 = HD_VIDEOCAP_OUT(20, 7),
	/* DEVICE 21 */
	HD_VIDEOCAP_21_OUT_0 = HD_VIDEOCAP_OUT(21, 0),
	HD_VIDEOCAP_21_OUT_1 = HD_VIDEOCAP_OUT(21, 1),
	HD_VIDEOCAP_21_OUT_2 = HD_VIDEOCAP_OUT(21, 2),
	HD_VIDEOCAP_21_OUT_3 = HD_VIDEOCAP_OUT(21, 3),
	HD_VIDEOCAP_21_OUT_4 = HD_VIDEOCAP_OUT(21, 4),
	HD_VIDEOCAP_21_OUT_5 = HD_VIDEOCAP_OUT(21, 5),
	HD_VIDEOCAP_21_OUT_6 = HD_VIDEOCAP_OUT(21, 6),
	HD_VIDEOCAP_21_OUT_7 = HD_VIDEOCAP_OUT(21, 7),
	/* DEVICE 22 */
	HD_VIDEOCAP_22_OUT_0 = HD_VIDEOCAP_OUT(22, 0),
	HD_VIDEOCAP_22_OUT_1 = HD_VIDEOCAP_OUT(22, 1),
	HD_VIDEOCAP_22_OUT_2 = HD_VIDEOCAP_OUT(22, 2),
	HD_VIDEOCAP_22_OUT_3 = HD_VIDEOCAP_OUT(22, 3),
	HD_VIDEOCAP_22_OUT_4 = HD_VIDEOCAP_OUT(22, 4),
	HD_VIDEOCAP_22_OUT_5 = HD_VIDEOCAP_OUT(22, 5),
	HD_VIDEOCAP_22_OUT_6 = HD_VIDEOCAP_OUT(22, 6),
	HD_VIDEOCAP_22_OUT_7 = HD_VIDEOCAP_OUT(22, 7),
	/* DEVICE 23 */
	HD_VIDEOCAP_23_OUT_0 = HD_VIDEOCAP_OUT(23, 0),
	HD_VIDEOCAP_23_OUT_1 = HD_VIDEOCAP_OUT(23, 1),
	HD_VIDEOCAP_23_OUT_2 = HD_VIDEOCAP_OUT(23, 2),
	HD_VIDEOCAP_23_OUT_3 = HD_VIDEOCAP_OUT(23, 3),
	HD_VIDEOCAP_23_OUT_4 = HD_VIDEOCAP_OUT(23, 4),
	HD_VIDEOCAP_23_OUT_5 = HD_VIDEOCAP_OUT(23, 5),
	HD_VIDEOCAP_23_OUT_6 = HD_VIDEOCAP_OUT(23, 6),
	HD_VIDEOCAP_23_OUT_7 = HD_VIDEOCAP_OUT(23, 7),
	/* DEVICE 24 */
	HD_VIDEOCAP_24_OUT_0 = HD_VIDEOCAP_OUT(24, 0),
	HD_VIDEOCAP_24_OUT_1 = HD_VIDEOCAP_OUT(24, 1),
	HD_VIDEOCAP_24_OUT_2 = HD_VIDEOCAP_OUT(24, 2),
	HD_VIDEOCAP_24_OUT_3 = HD_VIDEOCAP_OUT(24, 3),
	HD_VIDEOCAP_24_OUT_4 = HD_VIDEOCAP_OUT(24, 4),
	HD_VIDEOCAP_24_OUT_5 = HD_VIDEOCAP_OUT(24, 5),
	HD_VIDEOCAP_24_OUT_6 = HD_VIDEOCAP_OUT(24, 6),
	HD_VIDEOCAP_24_OUT_7 = HD_VIDEOCAP_OUT(24, 7),
	/* DEVICE 25 */
	HD_VIDEOCAP_25_OUT_0 = HD_VIDEOCAP_OUT(25, 0),
	HD_VIDEOCAP_25_OUT_1 = HD_VIDEOCAP_OUT(25, 1),
	HD_VIDEOCAP_25_OUT_2 = HD_VIDEOCAP_OUT(25, 2),
	HD_VIDEOCAP_25_OUT_3 = HD_VIDEOCAP_OUT(25, 3),
	HD_VIDEOCAP_25_OUT_4 = HD_VIDEOCAP_OUT(25, 4),
	HD_VIDEOCAP_25_OUT_5 = HD_VIDEOCAP_OUT(25, 5),
	HD_VIDEOCAP_25_OUT_6 = HD_VIDEOCAP_OUT(25, 6),
	HD_VIDEOCAP_25_OUT_7 = HD_VIDEOCAP_OUT(25, 7),
	/* DEVICE 26 */
	HD_VIDEOCAP_26_OUT_0 = HD_VIDEOCAP_OUT(26, 0),
	HD_VIDEOCAP_26_OUT_1 = HD_VIDEOCAP_OUT(26, 1),
	HD_VIDEOCAP_26_OUT_2 = HD_VIDEOCAP_OUT(26, 2),
	HD_VIDEOCAP_26_OUT_3 = HD_VIDEOCAP_OUT(26, 3),
	HD_VIDEOCAP_26_OUT_4 = HD_VIDEOCAP_OUT(26, 4),
	HD_VIDEOCAP_26_OUT_5 = HD_VIDEOCAP_OUT(26, 5),
	HD_VIDEOCAP_26_OUT_6 = HD_VIDEOCAP_OUT(26, 6),
	HD_VIDEOCAP_26_OUT_7 = HD_VIDEOCAP_OUT(26, 7),
	/* DEVICE 27 */
	HD_VIDEOCAP_27_OUT_0 = HD_VIDEOCAP_OUT(27, 0),
	HD_VIDEOCAP_27_OUT_1 = HD_VIDEOCAP_OUT(27, 1),
	HD_VIDEOCAP_27_OUT_2 = HD_VIDEOCAP_OUT(27, 2),
	HD_VIDEOCAP_27_OUT_3 = HD_VIDEOCAP_OUT(27, 3),
	HD_VIDEOCAP_27_OUT_4 = HD_VIDEOCAP_OUT(27, 4),
	HD_VIDEOCAP_27_OUT_5 = HD_VIDEOCAP_OUT(27, 5),
	HD_VIDEOCAP_27_OUT_6 = HD_VIDEOCAP_OUT(27, 6),
	HD_VIDEOCAP_27_OUT_7 = HD_VIDEOCAP_OUT(27, 7),
	/* DEVICE 28 */
	HD_VIDEOCAP_28_OUT_0 = HD_VIDEOCAP_OUT(28, 0),
	HD_VIDEOCAP_28_OUT_1 = HD_VIDEOCAP_OUT(28, 1),
	HD_VIDEOCAP_28_OUT_2 = HD_VIDEOCAP_OUT(28, 2),
	HD_VIDEOCAP_28_OUT_3 = HD_VIDEOCAP_OUT(28, 3),
	HD_VIDEOCAP_28_OUT_4 = HD_VIDEOCAP_OUT(28, 4),
	HD_VIDEOCAP_28_OUT_5 = HD_VIDEOCAP_OUT(28, 5),
	HD_VIDEOCAP_28_OUT_6 = HD_VIDEOCAP_OUT(28, 6),
	HD_VIDEOCAP_28_OUT_7 = HD_VIDEOCAP_OUT(28, 7),
	/* DEVICE 29 */
	HD_VIDEOCAP_29_OUT_0 = HD_VIDEOCAP_OUT(29, 0),
	HD_VIDEOCAP_29_OUT_1 = HD_VIDEOCAP_OUT(29, 1),
	HD_VIDEOCAP_29_OUT_2 = HD_VIDEOCAP_OUT(29, 2),
	HD_VIDEOCAP_29_OUT_3 = HD_VIDEOCAP_OUT(29, 3),
	HD_VIDEOCAP_29_OUT_4 = HD_VIDEOCAP_OUT(29, 4),
	HD_VIDEOCAP_29_OUT_5 = HD_VIDEOCAP_OUT(29, 5),
	HD_VIDEOCAP_29_OUT_6 = HD_VIDEOCAP_OUT(29, 6),
	HD_VIDEOCAP_29_OUT_7 = HD_VIDEOCAP_OUT(29, 7),
	/* DEVICE 30 */
	HD_VIDEOCAP_30_OUT_0 = HD_VIDEOCAP_OUT(30, 0),
	HD_VIDEOCAP_30_OUT_1 = HD_VIDEOCAP_OUT(30, 1),
	HD_VIDEOCAP_30_OUT_2 = HD_VIDEOCAP_OUT(30, 2),
	HD_VIDEOCAP_30_OUT_3 = HD_VIDEOCAP_OUT(30, 3),
	HD_VIDEOCAP_30_OUT_4 = HD_VIDEOCAP_OUT(30, 4),
	HD_VIDEOCAP_30_OUT_5 = HD_VIDEOCAP_OUT(30, 5),
	HD_VIDEOCAP_30_OUT_6 = HD_VIDEOCAP_OUT(30, 6),
	HD_VIDEOCAP_30_OUT_7 = HD_VIDEOCAP_OUT(30, 7),
	/* DEVICE 31 */
	HD_VIDEOCAP_31_OUT_0 = HD_VIDEOCAP_OUT(31, 0),
	HD_VIDEOCAP_31_OUT_1 = HD_VIDEOCAP_OUT(31, 1),
	HD_VIDEOCAP_31_OUT_2 = HD_VIDEOCAP_OUT(31, 2),
	HD_VIDEOCAP_31_OUT_3 = HD_VIDEOCAP_OUT(31, 3),
	HD_VIDEOCAP_31_OUT_4 = HD_VIDEOCAP_OUT(31, 4),
	HD_VIDEOCAP_31_OUT_5 = HD_VIDEOCAP_OUT(31, 5),
	HD_VIDEOCAP_31_OUT_6 = HD_VIDEOCAP_OUT(31, 6),
	HD_VIDEOCAP_31_OUT_7 = HD_VIDEOCAP_OUT(31, 7),


	/* DEVICE 0 */
	HD_VIDEOOUT_0_OUT_0 = HD_VIDEOOUT_OUT(0, 0),
	/* DEVICE 1 */
	HD_VIDEOOUT_1_OUT_0 = HD_VIDEOOUT_OUT(1, 0),


	/* DEVICE 0 */
	HD_VIDEOENC_0_OUT_0 = HD_VIDEOENC_OUT(0, 0),
	HD_VIDEOENC_0_OUT_1 = HD_VIDEOENC_OUT(0, 1),
	HD_VIDEOENC_0_OUT_2 = HD_VIDEOENC_OUT(0, 2),
	HD_VIDEOENC_0_OUT_3 = HD_VIDEOENC_OUT(0, 3),
	HD_VIDEOENC_0_OUT_4 = HD_VIDEOENC_OUT(0, 4),
	HD_VIDEOENC_0_OUT_5 = HD_VIDEOENC_OUT(0, 5),
	HD_VIDEOENC_0_OUT_6 = HD_VIDEOENC_OUT(0, 6),
	HD_VIDEOENC_0_OUT_7 = HD_VIDEOENC_OUT(0, 7),
	HD_VIDEOENC_0_OUT_8 = HD_VIDEOENC_OUT(0, 8),
	HD_VIDEOENC_0_OUT_9 = HD_VIDEOENC_OUT(0, 9),
	HD_VIDEOENC_0_OUT_10 = HD_VIDEOENC_OUT(0, 10),
	HD_VIDEOENC_0_OUT_11 = HD_VIDEOENC_OUT(0, 11),
	HD_VIDEOENC_0_OUT_12 = HD_VIDEOENC_OUT(0, 12),
	HD_VIDEOENC_0_OUT_13 = HD_VIDEOENC_OUT(0, 13),
	HD_VIDEOENC_0_OUT_14 = HD_VIDEOENC_OUT(0, 14),
	HD_VIDEOENC_0_OUT_15 = HD_VIDEOENC_OUT(0, 15),
	HD_VIDEOENC_0_OUT_16 = HD_VIDEOENC_OUT(0, 16),
	HD_VIDEOENC_0_OUT_17 = HD_VIDEOENC_OUT(0, 17),
	HD_VIDEOENC_0_OUT_18 = HD_VIDEOENC_OUT(0, 18),
	HD_VIDEOENC_0_OUT_19 = HD_VIDEOENC_OUT(0, 19),
	HD_VIDEOENC_0_OUT_20 = HD_VIDEOENC_OUT(0, 20),
	HD_VIDEOENC_0_OUT_21 = HD_VIDEOENC_OUT(0, 21),
	HD_VIDEOENC_0_OUT_22 = HD_VIDEOENC_OUT(0, 22),
	HD_VIDEOENC_0_OUT_23 = HD_VIDEOENC_OUT(0, 23),
	HD_VIDEOENC_0_OUT_24 = HD_VIDEOENC_OUT(0, 24),
	HD_VIDEOENC_0_OUT_25 = HD_VIDEOENC_OUT(0, 25),
	HD_VIDEOENC_0_OUT_26 = HD_VIDEOENC_OUT(0, 26),
	HD_VIDEOENC_0_OUT_27 = HD_VIDEOENC_OUT(0, 27),
	HD_VIDEOENC_0_OUT_28 = HD_VIDEOENC_OUT(0, 28),
	HD_VIDEOENC_0_OUT_29 = HD_VIDEOENC_OUT(0, 29),
	HD_VIDEOENC_0_OUT_30 = HD_VIDEOENC_OUT(0, 30),
	HD_VIDEOENC_0_OUT_31 = HD_VIDEOENC_OUT(0, 31),
	HD_VIDEOENC_0_OUT_32 = HD_VIDEOENC_OUT(0, 32),
	HD_VIDEOENC_0_OUT_33 = HD_VIDEOENC_OUT(0, 33),
	HD_VIDEOENC_0_OUT_34 = HD_VIDEOENC_OUT(0, 34),
	HD_VIDEOENC_0_OUT_35 = HD_VIDEOENC_OUT(0, 35),
	HD_VIDEOENC_0_OUT_36 = HD_VIDEOENC_OUT(0, 36),
	HD_VIDEOENC_0_OUT_37 = HD_VIDEOENC_OUT(0, 37),
	HD_VIDEOENC_0_OUT_38 = HD_VIDEOENC_OUT(0, 38),
	HD_VIDEOENC_0_OUT_39 = HD_VIDEOENC_OUT(0, 39),
	HD_VIDEOENC_0_OUT_40 = HD_VIDEOENC_OUT(0, 40),
	HD_VIDEOENC_0_OUT_41 = HD_VIDEOENC_OUT(0, 41),
	HD_VIDEOENC_0_OUT_42 = HD_VIDEOENC_OUT(0, 42),
	HD_VIDEOENC_0_OUT_43 = HD_VIDEOENC_OUT(0, 43),
	HD_VIDEOENC_0_OUT_44 = HD_VIDEOENC_OUT(0, 44),
	HD_VIDEOENC_0_OUT_45 = HD_VIDEOENC_OUT(0, 45),
	HD_VIDEOENC_0_OUT_46 = HD_VIDEOENC_OUT(0, 46),
	HD_VIDEOENC_0_OUT_47 = HD_VIDEOENC_OUT(0, 47),
	HD_VIDEOENC_0_OUT_48 = HD_VIDEOENC_OUT(0, 48),
	HD_VIDEOENC_0_OUT_49 = HD_VIDEOENC_OUT(0, 49),
	HD_VIDEOENC_0_OUT_50 = HD_VIDEOENC_OUT(0, 50),
	HD_VIDEOENC_0_OUT_51 = HD_VIDEOENC_OUT(0, 51),
	HD_VIDEOENC_0_OUT_52 = HD_VIDEOENC_OUT(0, 52),
	HD_VIDEOENC_0_OUT_53 = HD_VIDEOENC_OUT(0, 53),
	HD_VIDEOENC_0_OUT_54 = HD_VIDEOENC_OUT(0, 54),
	HD_VIDEOENC_0_OUT_55 = HD_VIDEOENC_OUT(0, 55),
	HD_VIDEOENC_0_OUT_56 = HD_VIDEOENC_OUT(0, 56),
	HD_VIDEOENC_0_OUT_57 = HD_VIDEOENC_OUT(0, 57),
	HD_VIDEOENC_0_OUT_58 = HD_VIDEOENC_OUT(0, 58),
	HD_VIDEOENC_0_OUT_59 = HD_VIDEOENC_OUT(0, 59),
	HD_VIDEOENC_0_OUT_60 = HD_VIDEOENC_OUT(0, 60),
	HD_VIDEOENC_0_OUT_61 = HD_VIDEOENC_OUT(0, 61),
	HD_VIDEOENC_0_OUT_62 = HD_VIDEOENC_OUT(0, 62),
	HD_VIDEOENC_0_OUT_63 = HD_VIDEOENC_OUT(0, 63),

	/* DEVICE 0 */
	HD_VIDEODEC_0_OUT_0 = HD_VIDEODEC_OUT(0, 0),
	HD_VIDEODEC_0_OUT_1 = HD_VIDEODEC_OUT(0, 1),
	HD_VIDEODEC_0_OUT_2 = HD_VIDEODEC_OUT(0, 2),
	HD_VIDEODEC_0_OUT_3 = HD_VIDEODEC_OUT(0, 3),
	HD_VIDEODEC_0_OUT_4 = HD_VIDEODEC_OUT(0, 4),
	HD_VIDEODEC_0_OUT_5 = HD_VIDEODEC_OUT(0, 5),
	HD_VIDEODEC_0_OUT_6 = HD_VIDEODEC_OUT(0, 6),
	HD_VIDEODEC_0_OUT_7 = HD_VIDEODEC_OUT(0, 7),
	HD_VIDEODEC_0_OUT_8 = HD_VIDEODEC_OUT(0, 8),
	HD_VIDEODEC_0_OUT_9 = HD_VIDEODEC_OUT(0, 9),
	HD_VIDEODEC_0_OUT_10 = HD_VIDEODEC_OUT(0, 10),
	HD_VIDEODEC_0_OUT_11 = HD_VIDEODEC_OUT(0, 11),
	HD_VIDEODEC_0_OUT_12 = HD_VIDEODEC_OUT(0, 12),
	HD_VIDEODEC_0_OUT_13 = HD_VIDEODEC_OUT(0, 13),
	HD_VIDEODEC_0_OUT_14 = HD_VIDEODEC_OUT(0, 14),
	HD_VIDEODEC_0_OUT_15 = HD_VIDEODEC_OUT(0, 15),


	/* DEVICE 0 */
	HD_VIDEOPROC_0_OUT_0 = HD_VIDEOPROC_OUT(0, 0),
	HD_VIDEOPROC_0_OUT_1 = HD_VIDEOPROC_OUT(0, 1),
	HD_VIDEOPROC_0_OUT_2 = HD_VIDEOPROC_OUT(0, 2),
	HD_VIDEOPROC_0_OUT_3 = HD_VIDEOPROC_OUT(0, 3),
	HD_VIDEOPROC_0_OUT_4 = HD_VIDEOPROC_OUT(0, 4),
	HD_VIDEOPROC_0_OUT_5 = HD_VIDEOPROC_OUT(0, 5),
	HD_VIDEOPROC_0_OUT_6 = HD_VIDEOPROC_OUT(0, 6),
	HD_VIDEOPROC_0_OUT_7 = HD_VIDEOPROC_OUT(0, 7),
	HD_VIDEOPROC_0_OUT_8 = HD_VIDEOPROC_OUT(0, 8),
	HD_VIDEOPROC_0_OUT_9 = HD_VIDEOPROC_OUT(0, 9),
	HD_VIDEOPROC_0_OUT_10 = HD_VIDEOPROC_OUT(0, 10),
	HD_VIDEOPROC_0_OUT_11 = HD_VIDEOPROC_OUT(0, 11),
	HD_VIDEOPROC_0_OUT_12 = HD_VIDEOPROC_OUT(0, 12),
	HD_VIDEOPROC_0_OUT_13 = HD_VIDEOPROC_OUT(0, 13),
	HD_VIDEOPROC_0_OUT_14 = HD_VIDEOPROC_OUT(0, 14),
	HD_VIDEOPROC_0_OUT_15 = HD_VIDEOPROC_OUT(0, 15),
	/* DEVICE 1 */
	HD_VIDEOPROC_1_OUT_0 = HD_VIDEOPROC_OUT(1, 0),
	HD_VIDEOPROC_1_OUT_1 = HD_VIDEOPROC_OUT(1, 1),
	HD_VIDEOPROC_1_OUT_2 = HD_VIDEOPROC_OUT(1, 2),
	HD_VIDEOPROC_1_OUT_3 = HD_VIDEOPROC_OUT(1, 3),
	HD_VIDEOPROC_1_OUT_4 = HD_VIDEOPROC_OUT(1, 4),
	HD_VIDEOPROC_1_OUT_5 = HD_VIDEOPROC_OUT(1, 5),
	HD_VIDEOPROC_1_OUT_6 = HD_VIDEOPROC_OUT(1, 6),
	HD_VIDEOPROC_1_OUT_7 = HD_VIDEOPROC_OUT(1, 7),
	HD_VIDEOPROC_1_OUT_8 = HD_VIDEOPROC_OUT(1, 8),
	HD_VIDEOPROC_1_OUT_9 = HD_VIDEOPROC_OUT(1, 9),
	HD_VIDEOPROC_1_OUT_10 = HD_VIDEOPROC_OUT(1, 10),
	HD_VIDEOPROC_1_OUT_11 = HD_VIDEOPROC_OUT(1, 11),
	HD_VIDEOPROC_1_OUT_12 = HD_VIDEOPROC_OUT(1, 12),
	HD_VIDEOPROC_1_OUT_13 = HD_VIDEOPROC_OUT(1, 13),
	HD_VIDEOPROC_1_OUT_14 = HD_VIDEOPROC_OUT(1, 14),
	HD_VIDEOPROC_1_OUT_15 = HD_VIDEOPROC_OUT(1, 15),
	/* DEVICE 2 */
	HD_VIDEOPROC_2_OUT_0 = HD_VIDEOPROC_OUT(2, 0),
	HD_VIDEOPROC_2_OUT_1 = HD_VIDEOPROC_OUT(2, 1),
	HD_VIDEOPROC_2_OUT_2 = HD_VIDEOPROC_OUT(2, 2),
	HD_VIDEOPROC_2_OUT_3 = HD_VIDEOPROC_OUT(2, 3),
	HD_VIDEOPROC_2_OUT_4 = HD_VIDEOPROC_OUT(2, 4),
	HD_VIDEOPROC_2_OUT_5 = HD_VIDEOPROC_OUT(2, 5),
	HD_VIDEOPROC_2_OUT_6 = HD_VIDEOPROC_OUT(2, 6),
	HD_VIDEOPROC_2_OUT_7 = HD_VIDEOPROC_OUT(2, 7),
	HD_VIDEOPROC_2_OUT_8 = HD_VIDEOPROC_OUT(2, 8),
	HD_VIDEOPROC_2_OUT_9 = HD_VIDEOPROC_OUT(2, 9),
	HD_VIDEOPROC_2_OUT_10 = HD_VIDEOPROC_OUT(2, 10),
	HD_VIDEOPROC_2_OUT_11 = HD_VIDEOPROC_OUT(2, 11),
	HD_VIDEOPROC_2_OUT_12 = HD_VIDEOPROC_OUT(2, 12),
	HD_VIDEOPROC_2_OUT_13 = HD_VIDEOPROC_OUT(2, 13),
	HD_VIDEOPROC_2_OUT_14 = HD_VIDEOPROC_OUT(2, 14),
	HD_VIDEOPROC_2_OUT_15 = HD_VIDEOPROC_OUT(2, 15),
	/* DEVICE 3 */
	HD_VIDEOPROC_3_OUT_0 = HD_VIDEOPROC_OUT(3, 0),
	HD_VIDEOPROC_3_OUT_1 = HD_VIDEOPROC_OUT(3, 1),
	HD_VIDEOPROC_3_OUT_2 = HD_VIDEOPROC_OUT(3, 2),
	HD_VIDEOPROC_3_OUT_3 = HD_VIDEOPROC_OUT(3, 3),
	HD_VIDEOPROC_3_OUT_4 = HD_VIDEOPROC_OUT(3, 4),
	HD_VIDEOPROC_3_OUT_5 = HD_VIDEOPROC_OUT(3, 5),
	HD_VIDEOPROC_3_OUT_6 = HD_VIDEOPROC_OUT(3, 6),
	HD_VIDEOPROC_3_OUT_7 = HD_VIDEOPROC_OUT(3, 7),
	HD_VIDEOPROC_3_OUT_8 = HD_VIDEOPROC_OUT(3, 8),
	HD_VIDEOPROC_3_OUT_9 = HD_VIDEOPROC_OUT(3, 9),
	HD_VIDEOPROC_3_OUT_10 = HD_VIDEOPROC_OUT(3, 10),
	HD_VIDEOPROC_3_OUT_11 = HD_VIDEOPROC_OUT(3, 11),
	HD_VIDEOPROC_3_OUT_12 = HD_VIDEOPROC_OUT(3, 12),
	HD_VIDEOPROC_3_OUT_13 = HD_VIDEOPROC_OUT(3, 13),
	HD_VIDEOPROC_3_OUT_14 = HD_VIDEOPROC_OUT(3, 14),
	HD_VIDEOPROC_3_OUT_15 = HD_VIDEOPROC_OUT(3, 15),
	/* DEVICE 4 */
	HD_VIDEOPROC_4_OUT_0 = HD_VIDEOPROC_OUT(4, 0),
	HD_VIDEOPROC_4_OUT_1 = HD_VIDEOPROC_OUT(4, 1),
	HD_VIDEOPROC_4_OUT_2 = HD_VIDEOPROC_OUT(4, 2),
	HD_VIDEOPROC_4_OUT_3 = HD_VIDEOPROC_OUT(4, 3),
	HD_VIDEOPROC_4_OUT_4 = HD_VIDEOPROC_OUT(4, 4),
	HD_VIDEOPROC_4_OUT_5 = HD_VIDEOPROC_OUT(4, 5),
	HD_VIDEOPROC_4_OUT_6 = HD_VIDEOPROC_OUT(4, 6),
	HD_VIDEOPROC_4_OUT_7 = HD_VIDEOPROC_OUT(4, 7),
	HD_VIDEOPROC_4_OUT_8 = HD_VIDEOPROC_OUT(4, 8),
	HD_VIDEOPROC_4_OUT_9 = HD_VIDEOPROC_OUT(4, 9),
	HD_VIDEOPROC_4_OUT_10 = HD_VIDEOPROC_OUT(4, 10),
	HD_VIDEOPROC_4_OUT_11 = HD_VIDEOPROC_OUT(4, 11),
	HD_VIDEOPROC_4_OUT_12 = HD_VIDEOPROC_OUT(4, 12),
	HD_VIDEOPROC_4_OUT_13 = HD_VIDEOPROC_OUT(4, 13),
	HD_VIDEOPROC_4_OUT_14 = HD_VIDEOPROC_OUT(4, 14),
	HD_VIDEOPROC_4_OUT_15 = HD_VIDEOPROC_OUT(4, 15),
	/* DEVICE 5 */
	HD_VIDEOPROC_5_OUT_0 = HD_VIDEOPROC_OUT(5, 0),
	HD_VIDEOPROC_5_OUT_1 = HD_VIDEOPROC_OUT(5, 1),
	HD_VIDEOPROC_5_OUT_2 = HD_VIDEOPROC_OUT(5, 2),
	HD_VIDEOPROC_5_OUT_3 = HD_VIDEOPROC_OUT(5, 3),
	HD_VIDEOPROC_5_OUT_4 = HD_VIDEOPROC_OUT(5, 4),
	HD_VIDEOPROC_5_OUT_5 = HD_VIDEOPROC_OUT(5, 5),
	HD_VIDEOPROC_5_OUT_6 = HD_VIDEOPROC_OUT(5, 6),
	HD_VIDEOPROC_5_OUT_7 = HD_VIDEOPROC_OUT(5, 7),
	HD_VIDEOPROC_5_OUT_8 = HD_VIDEOPROC_OUT(5, 8),
	HD_VIDEOPROC_5_OUT_9 = HD_VIDEOPROC_OUT(5, 9),
	HD_VIDEOPROC_5_OUT_10 = HD_VIDEOPROC_OUT(5, 10),
	HD_VIDEOPROC_5_OUT_11 = HD_VIDEOPROC_OUT(5, 11),
	HD_VIDEOPROC_5_OUT_12 = HD_VIDEOPROC_OUT(5, 12),
	HD_VIDEOPROC_5_OUT_13 = HD_VIDEOPROC_OUT(5, 13),
	HD_VIDEOPROC_5_OUT_14 = HD_VIDEOPROC_OUT(5, 14),
	HD_VIDEOPROC_5_OUT_15 = HD_VIDEOPROC_OUT(5, 15),
	/* DEVICE 6 */
	HD_VIDEOPROC_6_OUT_0 = HD_VIDEOPROC_OUT(6, 0),
	HD_VIDEOPROC_6_OUT_1 = HD_VIDEOPROC_OUT(6, 1),
	HD_VIDEOPROC_6_OUT_2 = HD_VIDEOPROC_OUT(6, 2),
	HD_VIDEOPROC_6_OUT_3 = HD_VIDEOPROC_OUT(6, 3),
	HD_VIDEOPROC_6_OUT_4 = HD_VIDEOPROC_OUT(6, 4),
	HD_VIDEOPROC_6_OUT_5 = HD_VIDEOPROC_OUT(6, 5),
	HD_VIDEOPROC_6_OUT_6 = HD_VIDEOPROC_OUT(6, 6),
	HD_VIDEOPROC_6_OUT_7 = HD_VIDEOPROC_OUT(6, 7),
	HD_VIDEOPROC_6_OUT_8 = HD_VIDEOPROC_OUT(6, 8),
	HD_VIDEOPROC_6_OUT_9 = HD_VIDEOPROC_OUT(6, 9),
	HD_VIDEOPROC_6_OUT_10 = HD_VIDEOPROC_OUT(6, 10),
	HD_VIDEOPROC_6_OUT_11 = HD_VIDEOPROC_OUT(6, 11),
	HD_VIDEOPROC_6_OUT_12 = HD_VIDEOPROC_OUT(6, 12),
	HD_VIDEOPROC_6_OUT_13 = HD_VIDEOPROC_OUT(6, 13),
	HD_VIDEOPROC_6_OUT_14 = HD_VIDEOPROC_OUT(6, 14),
	HD_VIDEOPROC_6_OUT_15 = HD_VIDEOPROC_OUT(6, 15),
	/* DEVICE 7 */
	HD_VIDEOPROC_7_OUT_0 = HD_VIDEOPROC_OUT(7, 0),
	HD_VIDEOPROC_7_OUT_1 = HD_VIDEOPROC_OUT(7, 1),
	HD_VIDEOPROC_7_OUT_2 = HD_VIDEOPROC_OUT(7, 2),
	HD_VIDEOPROC_7_OUT_3 = HD_VIDEOPROC_OUT(7, 3),
	HD_VIDEOPROC_7_OUT_4 = HD_VIDEOPROC_OUT(7, 4),
	HD_VIDEOPROC_7_OUT_5 = HD_VIDEOPROC_OUT(7, 5),
	HD_VIDEOPROC_7_OUT_6 = HD_VIDEOPROC_OUT(7, 6),
	HD_VIDEOPROC_7_OUT_7 = HD_VIDEOPROC_OUT(7, 7),
	HD_VIDEOPROC_7_OUT_8 = HD_VIDEOPROC_OUT(7, 8),
	HD_VIDEOPROC_7_OUT_9 = HD_VIDEOPROC_OUT(7, 9),
	HD_VIDEOPROC_7_OUT_10 = HD_VIDEOPROC_OUT(7, 10),
	HD_VIDEOPROC_7_OUT_11 = HD_VIDEOPROC_OUT(7, 11),
	HD_VIDEOPROC_7_OUT_12 = HD_VIDEOPROC_OUT(7, 12),
	HD_VIDEOPROC_7_OUT_13 = HD_VIDEOPROC_OUT(7, 13),
	HD_VIDEOPROC_7_OUT_14 = HD_VIDEOPROC_OUT(7, 14),
	HD_VIDEOPROC_7_OUT_15 = HD_VIDEOPROC_OUT(7, 15),
	/* DEVICE 8 */
	HD_VIDEOPROC_8_OUT_0 = HD_VIDEOPROC_OUT(8, 0),
	HD_VIDEOPROC_8_OUT_1 = HD_VIDEOPROC_OUT(8, 1),
	HD_VIDEOPROC_8_OUT_2 = HD_VIDEOPROC_OUT(8, 2),
	HD_VIDEOPROC_8_OUT_3 = HD_VIDEOPROC_OUT(8, 3),
	HD_VIDEOPROC_8_OUT_4 = HD_VIDEOPROC_OUT(8, 4),
	HD_VIDEOPROC_8_OUT_5 = HD_VIDEOPROC_OUT(8, 5),
	HD_VIDEOPROC_8_OUT_6 = HD_VIDEOPROC_OUT(8, 6),
	HD_VIDEOPROC_8_OUT_7 = HD_VIDEOPROC_OUT(8, 7),
	HD_VIDEOPROC_8_OUT_8 = HD_VIDEOPROC_OUT(8, 8),
	HD_VIDEOPROC_8_OUT_9 = HD_VIDEOPROC_OUT(8, 9),
	HD_VIDEOPROC_8_OUT_10 = HD_VIDEOPROC_OUT(8, 10),
	HD_VIDEOPROC_8_OUT_11 = HD_VIDEOPROC_OUT(8, 11),
	HD_VIDEOPROC_8_OUT_12 = HD_VIDEOPROC_OUT(8, 12),
	HD_VIDEOPROC_8_OUT_13 = HD_VIDEOPROC_OUT(8, 13),
	HD_VIDEOPROC_8_OUT_14 = HD_VIDEOPROC_OUT(8, 14),
	HD_VIDEOPROC_8_OUT_15 = HD_VIDEOPROC_OUT(8, 15),
	/* DEVICE 9 */
	HD_VIDEOPROC_9_OUT_0 = HD_VIDEOPROC_OUT(9, 0),
	HD_VIDEOPROC_9_OUT_1 = HD_VIDEOPROC_OUT(9, 1),
	HD_VIDEOPROC_9_OUT_2 = HD_VIDEOPROC_OUT(9, 2),
	HD_VIDEOPROC_9_OUT_3 = HD_VIDEOPROC_OUT(9, 3),
	HD_VIDEOPROC_9_OUT_4 = HD_VIDEOPROC_OUT(9, 4),
	HD_VIDEOPROC_9_OUT_5 = HD_VIDEOPROC_OUT(9, 5),
	HD_VIDEOPROC_9_OUT_6 = HD_VIDEOPROC_OUT(9, 6),
	HD_VIDEOPROC_9_OUT_7 = HD_VIDEOPROC_OUT(9, 7),
	HD_VIDEOPROC_9_OUT_8 = HD_VIDEOPROC_OUT(9, 8),
	HD_VIDEOPROC_9_OUT_9 = HD_VIDEOPROC_OUT(9, 9),
	HD_VIDEOPROC_9_OUT_10 = HD_VIDEOPROC_OUT(9, 10),
	HD_VIDEOPROC_9_OUT_11 = HD_VIDEOPROC_OUT(9, 11),
	HD_VIDEOPROC_9_OUT_12 = HD_VIDEOPROC_OUT(9, 12),
	HD_VIDEOPROC_9_OUT_13 = HD_VIDEOPROC_OUT(9, 13),
	HD_VIDEOPROC_9_OUT_14 = HD_VIDEOPROC_OUT(9, 14),
	HD_VIDEOPROC_9_OUT_15 = HD_VIDEOPROC_OUT(9, 15),
	/* DEVICE 10 */
	HD_VIDEOPROC_10_OUT_0 = HD_VIDEOPROC_OUT(10, 0),
	HD_VIDEOPROC_10_OUT_1 = HD_VIDEOPROC_OUT(10, 1),
	HD_VIDEOPROC_10_OUT_2 = HD_VIDEOPROC_OUT(10, 2),
	HD_VIDEOPROC_10_OUT_3 = HD_VIDEOPROC_OUT(10, 3),
	HD_VIDEOPROC_10_OUT_4 = HD_VIDEOPROC_OUT(10, 4),
	HD_VIDEOPROC_10_OUT_5 = HD_VIDEOPROC_OUT(10, 5),
	HD_VIDEOPROC_10_OUT_6 = HD_VIDEOPROC_OUT(10, 6),
	HD_VIDEOPROC_10_OUT_7 = HD_VIDEOPROC_OUT(10, 7),
	HD_VIDEOPROC_10_OUT_8 = HD_VIDEOPROC_OUT(10, 8),
	HD_VIDEOPROC_10_OUT_9 = HD_VIDEOPROC_OUT(10, 9),
	HD_VIDEOPROC_10_OUT_10 = HD_VIDEOPROC_OUT(10, 10),
	HD_VIDEOPROC_10_OUT_11 = HD_VIDEOPROC_OUT(10, 11),
	HD_VIDEOPROC_10_OUT_12 = HD_VIDEOPROC_OUT(10, 12),
	HD_VIDEOPROC_10_OUT_13 = HD_VIDEOPROC_OUT(10, 13),
	HD_VIDEOPROC_10_OUT_14 = HD_VIDEOPROC_OUT(10, 14),
	HD_VIDEOPROC_10_OUT_15 = HD_VIDEOPROC_OUT(10, 15),
	/* DEVICE 11 */
	HD_VIDEOPROC_11_OUT_0 = HD_VIDEOPROC_OUT(11, 0),
	HD_VIDEOPROC_11_OUT_1 = HD_VIDEOPROC_OUT(11, 1),
	HD_VIDEOPROC_11_OUT_2 = HD_VIDEOPROC_OUT(11, 2),
	HD_VIDEOPROC_11_OUT_3 = HD_VIDEOPROC_OUT(11, 3),
	HD_VIDEOPROC_11_OUT_4 = HD_VIDEOPROC_OUT(11, 4),
	HD_VIDEOPROC_11_OUT_5 = HD_VIDEOPROC_OUT(11, 5),
	HD_VIDEOPROC_11_OUT_6 = HD_VIDEOPROC_OUT(11, 6),
	HD_VIDEOPROC_11_OUT_7 = HD_VIDEOPROC_OUT(11, 7),
	HD_VIDEOPROC_11_OUT_8 = HD_VIDEOPROC_OUT(11, 8),
	HD_VIDEOPROC_11_OUT_9 = HD_VIDEOPROC_OUT(11, 9),
	HD_VIDEOPROC_11_OUT_10 = HD_VIDEOPROC_OUT(11, 10),
	HD_VIDEOPROC_11_OUT_11 = HD_VIDEOPROC_OUT(11, 11),
	HD_VIDEOPROC_11_OUT_12 = HD_VIDEOPROC_OUT(11, 12),
	HD_VIDEOPROC_11_OUT_13 = HD_VIDEOPROC_OUT(11, 13),
	HD_VIDEOPROC_11_OUT_14 = HD_VIDEOPROC_OUT(11, 14),
	HD_VIDEOPROC_11_OUT_15 = HD_VIDEOPROC_OUT(11, 15),
	/* DEVICE 12 */
	HD_VIDEOPROC_12_OUT_0 = HD_VIDEOPROC_OUT(12, 0),
	HD_VIDEOPROC_12_OUT_1 = HD_VIDEOPROC_OUT(12, 1),
	HD_VIDEOPROC_12_OUT_2 = HD_VIDEOPROC_OUT(12, 2),
	HD_VIDEOPROC_12_OUT_3 = HD_VIDEOPROC_OUT(12, 3),
	HD_VIDEOPROC_12_OUT_4 = HD_VIDEOPROC_OUT(12, 4),
	HD_VIDEOPROC_12_OUT_5 = HD_VIDEOPROC_OUT(12, 5),
	HD_VIDEOPROC_12_OUT_6 = HD_VIDEOPROC_OUT(12, 6),
	HD_VIDEOPROC_12_OUT_7 = HD_VIDEOPROC_OUT(12, 7),
	HD_VIDEOPROC_12_OUT_8 = HD_VIDEOPROC_OUT(12, 8),
	HD_VIDEOPROC_12_OUT_9 = HD_VIDEOPROC_OUT(12, 9),
	HD_VIDEOPROC_12_OUT_10 = HD_VIDEOPROC_OUT(12, 10),
	HD_VIDEOPROC_12_OUT_11 = HD_VIDEOPROC_OUT(12, 11),
	HD_VIDEOPROC_12_OUT_12 = HD_VIDEOPROC_OUT(12, 12),
	HD_VIDEOPROC_12_OUT_13 = HD_VIDEOPROC_OUT(12, 13),
	HD_VIDEOPROC_12_OUT_14 = HD_VIDEOPROC_OUT(12, 14),
	HD_VIDEOPROC_12_OUT_15 = HD_VIDEOPROC_OUT(12, 15),
	/* DEVICE 13 */
	HD_VIDEOPROC_13_OUT_0 = HD_VIDEOPROC_OUT(13, 0),
	HD_VIDEOPROC_13_OUT_1 = HD_VIDEOPROC_OUT(13, 1),
	HD_VIDEOPROC_13_OUT_2 = HD_VIDEOPROC_OUT(13, 2),
	HD_VIDEOPROC_13_OUT_3 = HD_VIDEOPROC_OUT(13, 3),
	HD_VIDEOPROC_13_OUT_4 = HD_VIDEOPROC_OUT(13, 4),
	HD_VIDEOPROC_13_OUT_5 = HD_VIDEOPROC_OUT(13, 5),
	HD_VIDEOPROC_13_OUT_6 = HD_VIDEOPROC_OUT(13, 6),
	HD_VIDEOPROC_13_OUT_7 = HD_VIDEOPROC_OUT(13, 7),
	HD_VIDEOPROC_13_OUT_8 = HD_VIDEOPROC_OUT(13, 8),
	HD_VIDEOPROC_13_OUT_9 = HD_VIDEOPROC_OUT(13, 9),
	HD_VIDEOPROC_13_OUT_10 = HD_VIDEOPROC_OUT(13, 10),
	HD_VIDEOPROC_13_OUT_11 = HD_VIDEOPROC_OUT(13, 11),
	HD_VIDEOPROC_13_OUT_12 = HD_VIDEOPROC_OUT(13, 12),
	HD_VIDEOPROC_13_OUT_13 = HD_VIDEOPROC_OUT(13, 13),
	HD_VIDEOPROC_13_OUT_14 = HD_VIDEOPROC_OUT(13, 14),
	HD_VIDEOPROC_13_OUT_15 = HD_VIDEOPROC_OUT(13, 15),
	/* DEVICE 14 */
	HD_VIDEOPROC_14_OUT_0 = HD_VIDEOPROC_OUT(14, 0),
	HD_VIDEOPROC_14_OUT_1 = HD_VIDEOPROC_OUT(14, 1),
	HD_VIDEOPROC_14_OUT_2 = HD_VIDEOPROC_OUT(14, 2),
	HD_VIDEOPROC_14_OUT_3 = HD_VIDEOPROC_OUT(14, 3),
	HD_VIDEOPROC_14_OUT_4 = HD_VIDEOPROC_OUT(14, 4),
	HD_VIDEOPROC_14_OUT_5 = HD_VIDEOPROC_OUT(14, 5),
	HD_VIDEOPROC_14_OUT_6 = HD_VIDEOPROC_OUT(14, 6),
	HD_VIDEOPROC_14_OUT_7 = HD_VIDEOPROC_OUT(14, 7),
	HD_VIDEOPROC_14_OUT_8 = HD_VIDEOPROC_OUT(14, 8),
	HD_VIDEOPROC_14_OUT_9 = HD_VIDEOPROC_OUT(14, 9),
	HD_VIDEOPROC_14_OUT_10 = HD_VIDEOPROC_OUT(14, 10),
	HD_VIDEOPROC_14_OUT_11 = HD_VIDEOPROC_OUT(14, 11),
	HD_VIDEOPROC_14_OUT_12 = HD_VIDEOPROC_OUT(14, 12),
	HD_VIDEOPROC_14_OUT_13 = HD_VIDEOPROC_OUT(14, 13),
	HD_VIDEOPROC_14_OUT_14 = HD_VIDEOPROC_OUT(14, 14),
	HD_VIDEOPROC_14_OUT_15 = HD_VIDEOPROC_OUT(14, 15),
	/* DEVICE 15 */
	HD_VIDEOPROC_15_OUT_0 = HD_VIDEOPROC_OUT(15, 0),
	HD_VIDEOPROC_15_OUT_1 = HD_VIDEOPROC_OUT(15, 1),
	HD_VIDEOPROC_15_OUT_2 = HD_VIDEOPROC_OUT(15, 2),
	HD_VIDEOPROC_15_OUT_3 = HD_VIDEOPROC_OUT(15, 3),
	HD_VIDEOPROC_15_OUT_4 = HD_VIDEOPROC_OUT(15, 4),
	HD_VIDEOPROC_15_OUT_5 = HD_VIDEOPROC_OUT(15, 5),
	HD_VIDEOPROC_15_OUT_6 = HD_VIDEOPROC_OUT(15, 6),
	HD_VIDEOPROC_15_OUT_7 = HD_VIDEOPROC_OUT(15, 7),
	HD_VIDEOPROC_15_OUT_8 = HD_VIDEOPROC_OUT(15, 8),
	HD_VIDEOPROC_15_OUT_9 = HD_VIDEOPROC_OUT(15, 9),
	HD_VIDEOPROC_15_OUT_10 = HD_VIDEOPROC_OUT(15, 10),
	HD_VIDEOPROC_15_OUT_11 = HD_VIDEOPROC_OUT(15, 11),
	HD_VIDEOPROC_15_OUT_12 = HD_VIDEOPROC_OUT(15, 12),
	HD_VIDEOPROC_15_OUT_13 = HD_VIDEOPROC_OUT(15, 13),
	HD_VIDEOPROC_15_OUT_14 = HD_VIDEOPROC_OUT(15, 14),
	HD_VIDEOPROC_15_OUT_15 = HD_VIDEOPROC_OUT(15, 15),
	/* DEVICE 16 */
	HD_VIDEOPROC_16_OUT_0 = HD_VIDEOPROC_OUT(16, 0),
	/* DEVICE 17 */
	HD_VIDEOPROC_17_OUT_0 = HD_VIDEOPROC_OUT(17, 0),
	/* DEVICE 18 */
	HD_VIDEOPROC_18_OUT_0 = HD_VIDEOPROC_OUT(18, 0),
	/* DEVICE 19 */
	HD_VIDEOPROC_19_OUT_0 = HD_VIDEOPROC_OUT(19, 0),
	/* DEVICE 20 */
	HD_VIDEOPROC_20_OUT_0 = HD_VIDEOPROC_OUT(20, 0),
	/* DEVICE 21 */
	HD_VIDEOPROC_21_OUT_0 = HD_VIDEOPROC_OUT(21, 0),
	/* DEVICE 22 */
	HD_VIDEOPROC_22_OUT_0 = HD_VIDEOPROC_OUT(22, 0),
	/* DEVICE 23 */
	HD_VIDEOPROC_23_OUT_0 = HD_VIDEOPROC_OUT(23, 0),
	/* DEVICE 24 */
	HD_VIDEOPROC_24_OUT_0 = HD_VIDEOPROC_OUT(24, 0),
	/* DEVICE 25 */
	HD_VIDEOPROC_25_OUT_0 = HD_VIDEOPROC_OUT(25, 0),
	/* DEVICE 26 */
	HD_VIDEOPROC_26_OUT_0 = HD_VIDEOPROC_OUT(26, 0),
	/* DEVICE 27 */
	HD_VIDEOPROC_27_OUT_0 = HD_VIDEOPROC_OUT(27, 0),
	/* DEVICE 28 */
	HD_VIDEOPROC_28_OUT_0 = HD_VIDEOPROC_OUT(28, 0),
	/* DEVICE 29 */
	HD_VIDEOPROC_29_OUT_0 = HD_VIDEOPROC_OUT(29, 0),
	/* DEVICE 30 */
	HD_VIDEOPROC_30_OUT_0 = HD_VIDEOPROC_OUT(30, 0),
	/* DEVICE 31 */
	HD_VIDEOPROC_31_OUT_0 = HD_VIDEOPROC_OUT(31, 0),
	/* DEVICE 32 */
	HD_VIDEOPROC_32_OUT_0 = HD_VIDEOPROC_OUT(32, 0),
	/* DEVICE 33 */
	HD_VIDEOPROC_33_OUT_0 = HD_VIDEOPROC_OUT(33, 0),
	/* DEVICE 34 */
	HD_VIDEOPROC_34_OUT_0 = HD_VIDEOPROC_OUT(34, 0),
	/* DEVICE 35 */
	HD_VIDEOPROC_35_OUT_0 = HD_VIDEOPROC_OUT(35, 0),
	/* DEVICE 36 */
	HD_VIDEOPROC_36_OUT_0 = HD_VIDEOPROC_OUT(36, 0),
	/* DEVICE 37 */
	HD_VIDEOPROC_37_OUT_0 = HD_VIDEOPROC_OUT(37, 0),
	/* DEVICE 38 */
	HD_VIDEOPROC_38_OUT_0 = HD_VIDEOPROC_OUT(38, 0),
	/* DEVICE 39 */
	HD_VIDEOPROC_39_OUT_0 = HD_VIDEOPROC_OUT(39, 0),
	/* DEVICE 40 */
	HD_VIDEOPROC_40_OUT_0 = HD_VIDEOPROC_OUT(40, 0),
	/* DEVICE 41 */
	HD_VIDEOPROC_41_OUT_0 = HD_VIDEOPROC_OUT(41, 0),
	/* DEVICE 42 */
	HD_VIDEOPROC_42_OUT_0 = HD_VIDEOPROC_OUT(42, 0),
	/* DEVICE 43 */
	HD_VIDEOPROC_43_OUT_0 = HD_VIDEOPROC_OUT(43, 0),
	/* DEVICE 44 */
	HD_VIDEOPROC_44_OUT_0 = HD_VIDEOPROC_OUT(44, 0),
	/* DEVICE 45 */
	HD_VIDEOPROC_45_OUT_0 = HD_VIDEOPROC_OUT(45, 0),
	/* DEVICE 46 */
	HD_VIDEOPROC_46_OUT_0 = HD_VIDEOPROC_OUT(46, 0),
	/* DEVICE 47 */
	HD_VIDEOPROC_47_OUT_0 = HD_VIDEOPROC_OUT(47, 0),
	/* DEVICE 48 */
	HD_VIDEOPROC_48_OUT_0 = HD_VIDEOPROC_OUT(48, 0),
	/* DEVICE 49 */
	HD_VIDEOPROC_49_OUT_0 = HD_VIDEOPROC_OUT(49, 0),
	/* DEVICE 50 */
	HD_VIDEOPROC_50_OUT_0 = HD_VIDEOPROC_OUT(50, 0),
	/* DEVICE 51 */
	HD_VIDEOPROC_51_OUT_0 = HD_VIDEOPROC_OUT(51, 0),
	/* DEVICE 52 */
	HD_VIDEOPROC_52_OUT_0 = HD_VIDEOPROC_OUT(52, 0),
	/* DEVICE 53 */
	HD_VIDEOPROC_53_OUT_0 = HD_VIDEOPROC_OUT(53, 0),
	/* DEVICE 54 */
	HD_VIDEOPROC_54_OUT_0 = HD_VIDEOPROC_OUT(54, 0),
	/* DEVICE 55 */
	HD_VIDEOPROC_55_OUT_0 = HD_VIDEOPROC_OUT(55, 0),
	/* DEVICE 56 */
	HD_VIDEOPROC_56_OUT_0 = HD_VIDEOPROC_OUT(56, 0),
	/* DEVICE 57 */
	HD_VIDEOPROC_57_OUT_0 = HD_VIDEOPROC_OUT(57, 0),
	/* DEVICE 58 */
	HD_VIDEOPROC_58_OUT_0 = HD_VIDEOPROC_OUT(58, 0),
	/* DEVICE 59 */
	HD_VIDEOPROC_59_OUT_0 = HD_VIDEOPROC_OUT(59, 0),
	/* DEVICE 60 */
	HD_VIDEOPROC_60_OUT_0 = HD_VIDEOPROC_OUT(60, 0),
	/* DEVICE 61 */
	HD_VIDEOPROC_61_OUT_0 = HD_VIDEOPROC_OUT(61, 0),
	/* DEVICE 62 */
	HD_VIDEOPROC_62_OUT_0 = HD_VIDEOPROC_OUT(62, 0),
	/* DEVICE 63 */
	HD_VIDEOPROC_63_OUT_0 = HD_VIDEOPROC_OUT(63, 0),
	/* DEVICE 64 */
	HD_VIDEOPROC_64_OUT_0 = HD_VIDEOPROC_OUT(64, 0),
	/* DEVICE 65 */
	HD_VIDEOPROC_65_OUT_0 = HD_VIDEOPROC_OUT(65, 0),
	/* DEVICE 66 */
	HD_VIDEOPROC_66_OUT_0 = HD_VIDEOPROC_OUT(66, 0),
	/* DEVICE 67 */
	HD_VIDEOPROC_67_OUT_0 = HD_VIDEOPROC_OUT(67, 0),
	/* DEVICE 68 */
	HD_VIDEOPROC_68_OUT_0 = HD_VIDEOPROC_OUT(68, 0),
	/* DEVICE 69 */
	HD_VIDEOPROC_69_OUT_0 = HD_VIDEOPROC_OUT(69, 0),
	/* DEVICE 70 */
	HD_VIDEOPROC_70_OUT_0 = HD_VIDEOPROC_OUT(70, 0),
	/* DEVICE 71 */
	HD_VIDEOPROC_71_OUT_0 = HD_VIDEOPROC_OUT(71, 0),
	/* DEVICE 72 */
	HD_VIDEOPROC_72_OUT_0 = HD_VIDEOPROC_OUT(72, 0),
	/* DEVICE 73 */
	HD_VIDEOPROC_73_OUT_0 = HD_VIDEOPROC_OUT(73, 0),
	/* DEVICE 74 */
	HD_VIDEOPROC_74_OUT_0 = HD_VIDEOPROC_OUT(74, 0),
	/* DEVICE 75 */
	HD_VIDEOPROC_75_OUT_0 = HD_VIDEOPROC_OUT(75, 0),
	/* DEVICE 76 */
	HD_VIDEOPROC_76_OUT_0 = HD_VIDEOPROC_OUT(76, 0),
	/* DEVICE 77 */
	HD_VIDEOPROC_77_OUT_0 = HD_VIDEOPROC_OUT(77, 0),
	/* DEVICE 78 */
	HD_VIDEOPROC_78_OUT_0 = HD_VIDEOPROC_OUT(78, 0),
	/* DEVICE 79 */
	HD_VIDEOPROC_79_OUT_0 = HD_VIDEOPROC_OUT(79, 0),
	/* DEVICE 80 */
	HD_VIDEOPROC_80_OUT_0 = HD_VIDEOPROC_OUT(80, 0),
	/* DEVICE 81 */
	HD_VIDEOPROC_81_OUT_0 = HD_VIDEOPROC_OUT(81, 0),
	/* DEVICE 82 */
	HD_VIDEOPROC_82_OUT_0 = HD_VIDEOPROC_OUT(82, 0),
	/* DEVICE 83 */
	HD_VIDEOPROC_83_OUT_0 = HD_VIDEOPROC_OUT(83, 0),
	/* DEVICE 84 */
	HD_VIDEOPROC_84_OUT_0 = HD_VIDEOPROC_OUT(84, 0),
	/* DEVICE 85 */
	HD_VIDEOPROC_85_OUT_0 = HD_VIDEOPROC_OUT(85, 0),
	/* DEVICE 86 */
	HD_VIDEOPROC_86_OUT_0 = HD_VIDEOPROC_OUT(86, 0),
	/* DEVICE 87 */
	HD_VIDEOPROC_87_OUT_0 = HD_VIDEOPROC_OUT(87, 0),
	/* DEVICE 88 */
	HD_VIDEOPROC_88_OUT_0 = HD_VIDEOPROC_OUT(88, 0),
	/* DEVICE 89 */
	HD_VIDEOPROC_89_OUT_0 = HD_VIDEOPROC_OUT(89, 0),
	/* DEVICE 90 */
	HD_VIDEOPROC_90_OUT_0 = HD_VIDEOPROC_OUT(90, 0),
	/* DEVICE 91 */
	HD_VIDEOPROC_91_OUT_0 = HD_VIDEOPROC_OUT(91, 0),
	/* DEVICE 92 */
	HD_VIDEOPROC_92_OUT_0 = HD_VIDEOPROC_OUT(92, 0),
	/* DEVICE 93 */
	HD_VIDEOPROC_93_OUT_0 = HD_VIDEOPROC_OUT(93, 0),
	/* DEVICE 94 */
	HD_VIDEOPROC_94_OUT_0 = HD_VIDEOPROC_OUT(94, 0),
	/* DEVICE 95 */
	HD_VIDEOPROC_95_OUT_0 = HD_VIDEOPROC_OUT(95, 0),
	/* DEVICE 96 */
	HD_VIDEOPROC_96_OUT_0 = HD_VIDEOPROC_OUT(96, 0),
	/* DEVICE 97 */
	HD_VIDEOPROC_97_OUT_0 = HD_VIDEOPROC_OUT(97, 0),
	/* DEVICE 98 */
	HD_VIDEOPROC_98_OUT_0 = HD_VIDEOPROC_OUT(98, 0),
	/* DEVICE 99 */
	HD_VIDEOPROC_99_OUT_0 = HD_VIDEOPROC_OUT(99, 0),
	/* DEVICE 100 */
	HD_VIDEOPROC_100_OUT_0 = HD_VIDEOPROC_OUT(100, 0),
	/* DEVICE 101 */
	HD_VIDEOPROC_101_OUT_0 = HD_VIDEOPROC_OUT(101, 0),
	/* DEVICE 102 */
	HD_VIDEOPROC_102_OUT_0 = HD_VIDEOPROC_OUT(102, 0),
	/* DEVICE 103 */
	HD_VIDEOPROC_103_OUT_0 = HD_VIDEOPROC_OUT(103, 0),
	/* DEVICE 104 */
	HD_VIDEOPROC_104_OUT_0 = HD_VIDEOPROC_OUT(104, 0),
	/* DEVICE 105 */
	HD_VIDEOPROC_105_OUT_0 = HD_VIDEOPROC_OUT(105, 0),
	/* DEVICE 106 */
	HD_VIDEOPROC_106_OUT_0 = HD_VIDEOPROC_OUT(106, 0),
	/* DEVICE 107 */
	HD_VIDEOPROC_107_OUT_0 = HD_VIDEOPROC_OUT(107, 0),
	/* DEVICE 108 */
	HD_VIDEOPROC_108_OUT_0 = HD_VIDEOPROC_OUT(108, 0),
	/* DEVICE 109 */
	HD_VIDEOPROC_109_OUT_0 = HD_VIDEOPROC_OUT(109, 0),
	/* DEVICE 110 */
	HD_VIDEOPROC_110_OUT_0 = HD_VIDEOPROC_OUT(110, 0),
	/* DEVICE 111 */
	HD_VIDEOPROC_111_OUT_0 = HD_VIDEOPROC_OUT(111, 0),
	/* DEVICE 112 */
	HD_VIDEOPROC_112_OUT_0 = HD_VIDEOPROC_OUT(112, 0),
	/* DEVICE 113 */
	HD_VIDEOPROC_113_OUT_0 = HD_VIDEOPROC_OUT(113, 0),
	/* DEVICE 114 */
	HD_VIDEOPROC_114_OUT_0 = HD_VIDEOPROC_OUT(114, 0),
	/* DEVICE 115 */
	HD_VIDEOPROC_115_OUT_0 = HD_VIDEOPROC_OUT(115, 0),
	/* DEVICE 116 */
	HD_VIDEOPROC_116_OUT_0 = HD_VIDEOPROC_OUT(116, 0),
	/* DEVICE 117 */
	HD_VIDEOPROC_117_OUT_0 = HD_VIDEOPROC_OUT(117, 0),
	/* DEVICE 118 */
	HD_VIDEOPROC_118_OUT_0 = HD_VIDEOPROC_OUT(118, 0),
	/* DEVICE 119 */
	HD_VIDEOPROC_119_OUT_0 = HD_VIDEOPROC_OUT(119, 0),
	/* DEVICE 120 */
	HD_VIDEOPROC_120_OUT_0 = HD_VIDEOPROC_OUT(120, 0),
	/* DEVICE 121 */
	HD_VIDEOPROC_121_OUT_0 = HD_VIDEOPROC_OUT(121, 0),
	/* DEVICE 122 */
	HD_VIDEOPROC_122_OUT_0 = HD_VIDEOPROC_OUT(122, 0),
	/* DEVICE 123 */
	HD_VIDEOPROC_123_OUT_0 = HD_VIDEOPROC_OUT(123, 0),
	/* DEVICE 124 */
	HD_VIDEOPROC_124_OUT_0 = HD_VIDEOPROC_OUT(124, 0),
	/* DEVICE 125 */
	HD_VIDEOPROC_125_OUT_0 = HD_VIDEOPROC_OUT(125, 0),
	/* DEVICE 126 */
	HD_VIDEOPROC_126_OUT_0 = HD_VIDEOPROC_OUT(126, 0),
	/* DEVICE 127 */
	HD_VIDEOPROC_127_OUT_0 = HD_VIDEOPROC_OUT(127, 0),
	/* DEVICE 128 */
	HD_VIDEOPROC_128_OUT_0 = HD_VIDEOPROC_OUT(128, 0),
	/* DEVICE 129 */
	HD_VIDEOPROC_129_OUT_0 = HD_VIDEOPROC_OUT(129, 0),
	/* DEVICE 130 */
	HD_VIDEOPROC_130_OUT_0 = HD_VIDEOPROC_OUT(130, 0),
	/* DEVICE 131 */
	HD_VIDEOPROC_131_OUT_0 = HD_VIDEOPROC_OUT(131, 0),
	/* DEVICE 132 */
	HD_VIDEOPROC_132_OUT_0 = HD_VIDEOPROC_OUT(132, 0),
	/* DEVICE 133 */
	HD_VIDEOPROC_133_OUT_0 = HD_VIDEOPROC_OUT(133, 0),
	/* DEVICE 134 */
	HD_VIDEOPROC_134_OUT_0 = HD_VIDEOPROC_OUT(134, 0),
	/* DEVICE 135 */
	HD_VIDEOPROC_135_OUT_0 = HD_VIDEOPROC_OUT(135, 0),
	/* DEVICE 136 */
	HD_VIDEOPROC_136_OUT_0 = HD_VIDEOPROC_OUT(136, 0),
	/* DEVICE 137 */
	HD_VIDEOPROC_137_OUT_0 = HD_VIDEOPROC_OUT(137, 0),
	/* DEVICE 138 */
	HD_VIDEOPROC_138_OUT_0 = HD_VIDEOPROC_OUT(138, 0),
	/* DEVICE 139 */
	HD_VIDEOPROC_139_OUT_0 = HD_VIDEOPROC_OUT(139, 0),
	/* DEVICE 140 */
	HD_VIDEOPROC_140_OUT_0 = HD_VIDEOPROC_OUT(140, 0),
	/* DEVICE 141 */
	HD_VIDEOPROC_141_OUT_0 = HD_VIDEOPROC_OUT(141, 0),
	/* DEVICE 142 */
	HD_VIDEOPROC_142_OUT_0 = HD_VIDEOPROC_OUT(142, 0),
	/* DEVICE 143 */
	HD_VIDEOPROC_143_OUT_0 = HD_VIDEOPROC_OUT(143, 0),
	/* DEVICE 144 */
	HD_VIDEOPROC_144_OUT_0 = HD_VIDEOPROC_OUT(144, 0),
	/* DEVICE 145 */
	HD_VIDEOPROC_145_OUT_0 = HD_VIDEOPROC_OUT(145, 0),
	/* DEVICE 146 */
	HD_VIDEOPROC_146_OUT_0 = HD_VIDEOPROC_OUT(146, 0),
	/* DEVICE 147 */
	HD_VIDEOPROC_147_OUT_0 = HD_VIDEOPROC_OUT(147, 0),
	/* DEVICE 148 */
	HD_VIDEOPROC_148_OUT_0 = HD_VIDEOPROC_OUT(148, 0),
	/* DEVICE 149 */
	HD_VIDEOPROC_149_OUT_0 = HD_VIDEOPROC_OUT(149, 0),
	/* DEVICE 150 */
	HD_VIDEOPROC_150_OUT_0 = HD_VIDEOPROC_OUT(150, 0),
	/* DEVICE 151 */
	HD_VIDEOPROC_151_OUT_0 = HD_VIDEOPROC_OUT(151, 0),
	/* DEVICE 152 */
	HD_VIDEOPROC_152_OUT_0 = HD_VIDEOPROC_OUT(152, 0),
	/* DEVICE 153 */
	HD_VIDEOPROC_153_OUT_0 = HD_VIDEOPROC_OUT(153, 0),
	/* DEVICE 154 */
	HD_VIDEOPROC_154_OUT_0 = HD_VIDEOPROC_OUT(154, 0),
	/* DEVICE 155 */
	HD_VIDEOPROC_155_OUT_0 = HD_VIDEOPROC_OUT(155, 0),
	/* DEVICE 156 */
	HD_VIDEOPROC_156_OUT_0 = HD_VIDEOPROC_OUT(156, 0),
	/* DEVICE 157 */
	HD_VIDEOPROC_157_OUT_0 = HD_VIDEOPROC_OUT(157, 0),
	/* DEVICE 158 */
	HD_VIDEOPROC_158_OUT_0 = HD_VIDEOPROC_OUT(158, 0),
	/* DEVICE 159 */
	HD_VIDEOPROC_159_OUT_0 = HD_VIDEOPROC_OUT(159, 0),
	/* DEVICE 160 */
	HD_VIDEOPROC_160_OUT_0 = HD_VIDEOPROC_OUT(160, 0),
	/* DEVICE 161 */
	HD_VIDEOPROC_161_OUT_0 = HD_VIDEOPROC_OUT(161, 0),
	/* DEVICE 162 */
	HD_VIDEOPROC_162_OUT_0 = HD_VIDEOPROC_OUT(162, 0),
	/* DEVICE 163 */
	HD_VIDEOPROC_163_OUT_0 = HD_VIDEOPROC_OUT(163, 0),
	/* DEVICE 164 */
	HD_VIDEOPROC_164_OUT_0 = HD_VIDEOPROC_OUT(164, 0),
	/* DEVICE 165 */
	HD_VIDEOPROC_165_OUT_0 = HD_VIDEOPROC_OUT(165, 0),
	/* DEVICE 166 */
	HD_VIDEOPROC_166_OUT_0 = HD_VIDEOPROC_OUT(166, 0),
	/* DEVICE 167 */
	HD_VIDEOPROC_167_OUT_0 = HD_VIDEOPROC_OUT(167, 0),
	/* DEVICE 168 */
	HD_VIDEOPROC_168_OUT_0 = HD_VIDEOPROC_OUT(168, 0),
	/* DEVICE 169 */
	HD_VIDEOPROC_169_OUT_0 = HD_VIDEOPROC_OUT(169, 0),
	/* DEVICE 170 */
	HD_VIDEOPROC_170_OUT_0 = HD_VIDEOPROC_OUT(170, 0),
	/* DEVICE 171 */
	HD_VIDEOPROC_171_OUT_0 = HD_VIDEOPROC_OUT(171, 0),
	/* DEVICE 172 */
	HD_VIDEOPROC_172_OUT_0 = HD_VIDEOPROC_OUT(172, 0),
	/* DEVICE 173 */
	HD_VIDEOPROC_173_OUT_0 = HD_VIDEOPROC_OUT(173, 0),
	/* DEVICE 174 */
	HD_VIDEOPROC_174_OUT_0 = HD_VIDEOPROC_OUT(174, 0),
	/* DEVICE 175 */
	HD_VIDEOPROC_175_OUT_0 = HD_VIDEOPROC_OUT(175, 0),
	/* DEVICE 176 */
	HD_VIDEOPROC_176_OUT_0 = HD_VIDEOPROC_OUT(176, 0),
	/* DEVICE 177 */
	HD_VIDEOPROC_177_OUT_0 = HD_VIDEOPROC_OUT(177, 0),
	/* DEVICE 178 */
	HD_VIDEOPROC_178_OUT_0 = HD_VIDEOPROC_OUT(178, 0),
	/* DEVICE 179 */
	HD_VIDEOPROC_179_OUT_0 = HD_VIDEOPROC_OUT(179, 0),
	/* DEVICE 180 */
	HD_VIDEOPROC_180_OUT_0 = HD_VIDEOPROC_OUT(180, 0),
	/* DEVICE 181 */
	HD_VIDEOPROC_181_OUT_0 = HD_VIDEOPROC_OUT(181, 0),
	/* DEVICE 182 */
	HD_VIDEOPROC_182_OUT_0 = HD_VIDEOPROC_OUT(182, 0),
	/* DEVICE 183 */
	HD_VIDEOPROC_183_OUT_0 = HD_VIDEOPROC_OUT(183, 0),
	/* DEVICE 184 */
	HD_VIDEOPROC_184_OUT_0 = HD_VIDEOPROC_OUT(184, 0),
	/* DEVICE 185 */
	HD_VIDEOPROC_185_OUT_0 = HD_VIDEOPROC_OUT(185, 0),
	/* DEVICE 186 */
	HD_VIDEOPROC_186_OUT_0 = HD_VIDEOPROC_OUT(186, 0),
	/* DEVICE 187 */
	HD_VIDEOPROC_187_OUT_0 = HD_VIDEOPROC_OUT(187, 0),
	/* DEVICE 188 */
	HD_VIDEOPROC_188_OUT_0 = HD_VIDEOPROC_OUT(188, 0),
	/* DEVICE 189 */
	HD_VIDEOPROC_189_OUT_0 = HD_VIDEOPROC_OUT(189, 0),
	/* DEVICE 190 */
	HD_VIDEOPROC_190_OUT_0 = HD_VIDEOPROC_OUT(190, 0),
	/* DEVICE 191 */
	HD_VIDEOPROC_191_OUT_0 = HD_VIDEOPROC_OUT(191, 0),
	/* DEVICE 192 */
	HD_VIDEOPROC_192_OUT_0 = HD_VIDEOPROC_OUT(192, 0),
	/* DEVICE 193 */
	HD_VIDEOPROC_193_OUT_0 = HD_VIDEOPROC_OUT(193, 0),
	/* DEVICE 194 */
	HD_VIDEOPROC_194_OUT_0 = HD_VIDEOPROC_OUT(194, 0),
	/* DEVICE 195 */
	HD_VIDEOPROC_195_OUT_0 = HD_VIDEOPROC_OUT(195, 0),
	/* DEVICE 196 */
	HD_VIDEOPROC_196_OUT_0 = HD_VIDEOPROC_OUT(196, 0),
	/* DEVICE 197 */
	HD_VIDEOPROC_197_OUT_0 = HD_VIDEOPROC_OUT(197, 0),
	/* DEVICE 198 */
	HD_VIDEOPROC_198_OUT_0 = HD_VIDEOPROC_OUT(198, 0),
	/* DEVICE 199 */
	HD_VIDEOPROC_199_OUT_0 = HD_VIDEOPROC_OUT(199, 0),
	/* DEVICE 200 */
	HD_VIDEOPROC_200_OUT_0 = HD_VIDEOPROC_OUT(200, 0),
	/* DEVICE 201 */
	HD_VIDEOPROC_201_OUT_0 = HD_VIDEOPROC_OUT(201, 0),
	/* DEVICE 202 */
	HD_VIDEOPROC_202_OUT_0 = HD_VIDEOPROC_OUT(202, 0),
	/* DEVICE 203 */
	HD_VIDEOPROC_203_OUT_0 = HD_VIDEOPROC_OUT(203, 0),
	/* DEVICE 204 */
	HD_VIDEOPROC_204_OUT_0 = HD_VIDEOPROC_OUT(204, 0),
	/* DEVICE 205 */
	HD_VIDEOPROC_205_OUT_0 = HD_VIDEOPROC_OUT(205, 0),
	/* DEVICE 206 */
	HD_VIDEOPROC_206_OUT_0 = HD_VIDEOPROC_OUT(206, 0),
	/* DEVICE 207 */
	HD_VIDEOPROC_207_OUT_0 = HD_VIDEOPROC_OUT(207, 0),
	/* DEVICE 208 */
	HD_VIDEOPROC_208_OUT_0 = HD_VIDEOPROC_OUT(208, 0),
	/* DEVICE 209 */
	HD_VIDEOPROC_209_OUT_0 = HD_VIDEOPROC_OUT(209, 0),
	/* DEVICE 210 */
	HD_VIDEOPROC_210_OUT_0 = HD_VIDEOPROC_OUT(210, 0),
	/* DEVICE 211 */
	HD_VIDEOPROC_211_OUT_0 = HD_VIDEOPROC_OUT(211, 0),
	/* DEVICE 212 */
	HD_VIDEOPROC_212_OUT_0 = HD_VIDEOPROC_OUT(212, 0),
	/* DEVICE 213 */
	HD_VIDEOPROC_213_OUT_0 = HD_VIDEOPROC_OUT(213, 0),
	/* DEVICE 214 */
	HD_VIDEOPROC_214_OUT_0 = HD_VIDEOPROC_OUT(214, 0),
	/* DEVICE 215 */
	HD_VIDEOPROC_215_OUT_0 = HD_VIDEOPROC_OUT(215, 0),
	/* DEVICE 216 */
	HD_VIDEOPROC_216_OUT_0 = HD_VIDEOPROC_OUT(216, 0),
	/* DEVICE 217 */
	HD_VIDEOPROC_217_OUT_0 = HD_VIDEOPROC_OUT(217, 0),
	/* DEVICE 218 */
	HD_VIDEOPROC_218_OUT_0 = HD_VIDEOPROC_OUT(218, 0),
	/* DEVICE 219 */
	HD_VIDEOPROC_219_OUT_0 = HD_VIDEOPROC_OUT(219, 0),
	/* DEVICE 220 */
	HD_VIDEOPROC_220_OUT_0 = HD_VIDEOPROC_OUT(220, 0),
	/* DEVICE 221 */
	HD_VIDEOPROC_221_OUT_0 = HD_VIDEOPROC_OUT(221, 0),
	/* DEVICE 222 */
	HD_VIDEOPROC_222_OUT_0 = HD_VIDEOPROC_OUT(222, 0),
	/* DEVICE 223 */
	HD_VIDEOPROC_223_OUT_0 = HD_VIDEOPROC_OUT(223, 0),
	/* DEVICE 224 */
	HD_VIDEOPROC_224_OUT_0 = HD_VIDEOPROC_OUT(224, 0),
	/* DEVICE 225 */
	HD_VIDEOPROC_225_OUT_0 = HD_VIDEOPROC_OUT(225, 0),
	/* DEVICE 226 */
	HD_VIDEOPROC_226_OUT_0 = HD_VIDEOPROC_OUT(226, 0),
	/* DEVICE 227 */
	HD_VIDEOPROC_227_OUT_0 = HD_VIDEOPROC_OUT(227, 0),
	/* DEVICE 228 */
	HD_VIDEOPROC_228_OUT_0 = HD_VIDEOPROC_OUT(228, 0),
	/* DEVICE 229 */
	HD_VIDEOPROC_229_OUT_0 = HD_VIDEOPROC_OUT(229, 0),
	/* DEVICE 230 */
	HD_VIDEOPROC_230_OUT_0 = HD_VIDEOPROC_OUT(230, 0),
	/* DEVICE 231 */
	HD_VIDEOPROC_231_OUT_0 = HD_VIDEOPROC_OUT(231, 0),
	/* DEVICE 232 */
	HD_VIDEOPROC_232_OUT_0 = HD_VIDEOPROC_OUT(232, 0),
	/* DEVICE 233 */
	HD_VIDEOPROC_233_OUT_0 = HD_VIDEOPROC_OUT(233, 0),
	/* DEVICE 234 */
	HD_VIDEOPROC_234_OUT_0 = HD_VIDEOPROC_OUT(234, 0),
	/* DEVICE 235 */
	HD_VIDEOPROC_235_OUT_0 = HD_VIDEOPROC_OUT(235, 0),
	/* DEVICE 236 */
	HD_VIDEOPROC_236_OUT_0 = HD_VIDEOPROC_OUT(236, 0),
	/* DEVICE 237 */
	HD_VIDEOPROC_237_OUT_0 = HD_VIDEOPROC_OUT(237, 0),
	/* DEVICE 238 */
	HD_VIDEOPROC_238_OUT_0 = HD_VIDEOPROC_OUT(238, 0),
	/* DEVICE 239 */
	HD_VIDEOPROC_239_OUT_0 = HD_VIDEOPROC_OUT(239, 0),
	/* DEVICE 240 */
	HD_VIDEOPROC_240_OUT_0 = HD_VIDEOPROC_OUT(240, 0),
	/* DEVICE 241 */
	HD_VIDEOPROC_241_OUT_0 = HD_VIDEOPROC_OUT(241, 0),
	/* DEVICE 242 */
	HD_VIDEOPROC_242_OUT_0 = HD_VIDEOPROC_OUT(242, 0),
	/* DEVICE 243 */
	HD_VIDEOPROC_243_OUT_0 = HD_VIDEOPROC_OUT(243, 0),
	/* DEVICE 244 */
	HD_VIDEOPROC_244_OUT_0 = HD_VIDEOPROC_OUT(244, 0),
	/* DEVICE 245 */
	HD_VIDEOPROC_245_OUT_0 = HD_VIDEOPROC_OUT(245, 0),
	/* DEVICE 246 */
	HD_VIDEOPROC_246_OUT_0 = HD_VIDEOPROC_OUT(246, 0),
	/* DEVICE 247 */
	HD_VIDEOPROC_247_OUT_0 = HD_VIDEOPROC_OUT(247, 0),
	/* DEVICE 248 */
	HD_VIDEOPROC_248_OUT_0 = HD_VIDEOPROC_OUT(248, 0),
	/* DEVICE 249 */
	HD_VIDEOPROC_249_OUT_0 = HD_VIDEOPROC_OUT(249, 0),
	/* DEVICE 250 */
	HD_VIDEOPROC_250_OUT_0 = HD_VIDEOPROC_OUT(250, 0),
	/* DEVICE 251 */
	HD_VIDEOPROC_251_OUT_0 = HD_VIDEOPROC_OUT(251, 0),
	/* DEVICE 252 */
	HD_VIDEOPROC_252_OUT_0 = HD_VIDEOPROC_OUT(252, 0),
	/* DEVICE 253 */
	HD_VIDEOPROC_253_OUT_0 = HD_VIDEOPROC_OUT(253, 0),
	/* DEVICE 254 */
	HD_VIDEOPROC_254_OUT_0 = HD_VIDEOPROC_OUT(254, 0),
	/* DEVICE 255 */
	HD_VIDEOPROC_255_OUT_0 = HD_VIDEOPROC_OUT(255, 0),
	/* DEVICE 256 */
	HD_VIDEOPROC_256_OUT_0 = HD_VIDEOPROC_OUT(256, 0),
	/* DEVICE 257 */
	HD_VIDEOPROC_257_OUT_0 = HD_VIDEOPROC_OUT(257, 0),
	/* DEVICE 258 */
	HD_VIDEOPROC_258_OUT_0 = HD_VIDEOPROC_OUT(258, 0),
	/* DEVICE 259 */
	HD_VIDEOPROC_259_OUT_0 = HD_VIDEOPROC_OUT(259, 0),
	/* DEVICE 260 */
	HD_VIDEOPROC_260_OUT_0 = HD_VIDEOPROC_OUT(260, 0),
	/* DEVICE 261 */
	HD_VIDEOPROC_261_OUT_0 = HD_VIDEOPROC_OUT(261, 0),
	/* DEVICE 262 */
	HD_VIDEOPROC_262_OUT_0 = HD_VIDEOPROC_OUT(262, 0),
	/* DEVICE 263 */
	HD_VIDEOPROC_263_OUT_0 = HD_VIDEOPROC_OUT(263, 0),
	/* DEVICE 264 */
	HD_VIDEOPROC_264_OUT_0 = HD_VIDEOPROC_OUT(264, 0),
	/* DEVICE 265 */
	HD_VIDEOPROC_265_OUT_0 = HD_VIDEOPROC_OUT(265, 0),
	/* DEVICE 266 */
	HD_VIDEOPROC_266_OUT_0 = HD_VIDEOPROC_OUT(266, 0),
	/* DEVICE 267 */
	HD_VIDEOPROC_267_OUT_0 = HD_VIDEOPROC_OUT(267, 0),
	/* DEVICE 268 */
	HD_VIDEOPROC_268_OUT_0 = HD_VIDEOPROC_OUT(268, 0),
	/* DEVICE 269 */
	HD_VIDEOPROC_269_OUT_0 = HD_VIDEOPROC_OUT(269, 0),
	/* DEVICE 270 */
	HD_VIDEOPROC_270_OUT_0 = HD_VIDEOPROC_OUT(270, 0),
	/* DEVICE 271 */
	HD_VIDEOPROC_271_OUT_0 = HD_VIDEOPROC_OUT(271, 0),
	/* DEVICE 272 */
	HD_VIDEOPROC_272_OUT_0 = HD_VIDEOPROC_OUT(272, 0),
	/* DEVICE 273 */
	HD_VIDEOPROC_273_OUT_0 = HD_VIDEOPROC_OUT(273, 0),
	/* DEVICE 274 */
	HD_VIDEOPROC_274_OUT_0 = HD_VIDEOPROC_OUT(274, 0),
	/* DEVICE 275 */
	HD_VIDEOPROC_275_OUT_0 = HD_VIDEOPROC_OUT(275, 0),
	/* DEVICE 276 */
	HD_VIDEOPROC_276_OUT_0 = HD_VIDEOPROC_OUT(276, 0),
	/* DEVICE 277 */
	HD_VIDEOPROC_277_OUT_0 = HD_VIDEOPROC_OUT(277, 0),
	/* DEVICE 278 */
	HD_VIDEOPROC_278_OUT_0 = HD_VIDEOPROC_OUT(278, 0),
	/* DEVICE 279 */
	HD_VIDEOPROC_279_OUT_0 = HD_VIDEOPROC_OUT(279, 0),
	/* DEVICE 280 */
	HD_VIDEOPROC_280_OUT_0 = HD_VIDEOPROC_OUT(280, 0),
	/* DEVICE 281 */
	HD_VIDEOPROC_281_OUT_0 = HD_VIDEOPROC_OUT(281, 0),
	/* DEVICE 282 */
	HD_VIDEOPROC_282_OUT_0 = HD_VIDEOPROC_OUT(282, 0),
	/* DEVICE 283 */
	HD_VIDEOPROC_283_OUT_0 = HD_VIDEOPROC_OUT(283, 0),
	/* DEVICE 284 */
	HD_VIDEOPROC_284_OUT_0 = HD_VIDEOPROC_OUT(284, 0),
	/* DEVICE 285 */
	HD_VIDEOPROC_285_OUT_0 = HD_VIDEOPROC_OUT(285, 0),
	/* DEVICE 286 */
	HD_VIDEOPROC_286_OUT_0 = HD_VIDEOPROC_OUT(286, 0),
	/* DEVICE 287 */
	HD_VIDEOPROC_287_OUT_0 = HD_VIDEOPROC_OUT(287, 0),
	/* DEVICE 288 */
	HD_VIDEOPROC_288_OUT_0 = HD_VIDEOPROC_OUT(288, 0),
	/* DEVICE 289 */
	HD_VIDEOPROC_289_OUT_0 = HD_VIDEOPROC_OUT(289, 0),
	/* DEVICE 290 */
	HD_VIDEOPROC_290_OUT_0 = HD_VIDEOPROC_OUT(290, 0),
	/* DEVICE 291 */
	HD_VIDEOPROC_291_OUT_0 = HD_VIDEOPROC_OUT(291, 0),
	/* DEVICE 292 */
	HD_VIDEOPROC_292_OUT_0 = HD_VIDEOPROC_OUT(292, 0),
	/* DEVICE 293 */
	HD_VIDEOPROC_293_OUT_0 = HD_VIDEOPROC_OUT(293, 0),
	/* DEVICE 294 */
	HD_VIDEOPROC_294_OUT_0 = HD_VIDEOPROC_OUT(294, 0),
	/* DEVICE 295 */
	HD_VIDEOPROC_295_OUT_0 = HD_VIDEOPROC_OUT(295, 0),
	/* DEVICE 296 */
	HD_VIDEOPROC_296_OUT_0 = HD_VIDEOPROC_OUT(296, 0),
	/* DEVICE 297 */
	HD_VIDEOPROC_297_OUT_0 = HD_VIDEOPROC_OUT(297, 0),
	/* DEVICE 298 */
	HD_VIDEOPROC_298_OUT_0 = HD_VIDEOPROC_OUT(298, 0),
	/* DEVICE 299 */
	HD_VIDEOPROC_299_OUT_0 = HD_VIDEOPROC_OUT(299, 0),
	/* DEVICE 300 */
	HD_VIDEOPROC_300_OUT_0 = HD_VIDEOPROC_OUT(300, 0),
	/* DEVICE 301 */
	HD_VIDEOPROC_301_OUT_0 = HD_VIDEOPROC_OUT(301, 0),
	/* DEVICE 302 */
	HD_VIDEOPROC_302_OUT_0 = HD_VIDEOPROC_OUT(302, 0),
	/* DEVICE 303 */
	HD_VIDEOPROC_303_OUT_0 = HD_VIDEOPROC_OUT(303, 0),
	/* DEVICE 304 */
	HD_VIDEOPROC_304_OUT_0 = HD_VIDEOPROC_OUT(304, 0),
	/* DEVICE 305 */
	HD_VIDEOPROC_305_OUT_0 = HD_VIDEOPROC_OUT(305, 0),
	/* DEVICE 306 */
	HD_VIDEOPROC_306_OUT_0 = HD_VIDEOPROC_OUT(306, 0),
	/* DEVICE 307 */
	HD_VIDEOPROC_307_OUT_0 = HD_VIDEOPROC_OUT(307, 0),
	/* DEVICE 308 */
	HD_VIDEOPROC_308_OUT_0 = HD_VIDEOPROC_OUT(308, 0),
	/* DEVICE 309 */
	HD_VIDEOPROC_309_OUT_0 = HD_VIDEOPROC_OUT(309, 0),
	/* DEVICE 310 */
	HD_VIDEOPROC_310_OUT_0 = HD_VIDEOPROC_OUT(310, 0),
	/* DEVICE 311 */
	HD_VIDEOPROC_311_OUT_0 = HD_VIDEOPROC_OUT(311, 0),
	/* DEVICE 312 */
	HD_VIDEOPROC_312_OUT_0 = HD_VIDEOPROC_OUT(312, 0),
	/* DEVICE 313 */
	HD_VIDEOPROC_313_OUT_0 = HD_VIDEOPROC_OUT(313, 0),
	/* DEVICE 314 */
	HD_VIDEOPROC_314_OUT_0 = HD_VIDEOPROC_OUT(314, 0),
	/* DEVICE 315 */
	HD_VIDEOPROC_315_OUT_0 = HD_VIDEOPROC_OUT(315, 0),
	/* DEVICE 316 */
	HD_VIDEOPROC_316_OUT_0 = HD_VIDEOPROC_OUT(316, 0),
	/* DEVICE 317 */
	HD_VIDEOPROC_317_OUT_0 = HD_VIDEOPROC_OUT(317, 0),
	/* DEVICE 318 */
	HD_VIDEOPROC_318_OUT_0 = HD_VIDEOPROC_OUT(318, 0),
	/* DEVICE 319 */
	HD_VIDEOPROC_319_OUT_0 = HD_VIDEOPROC_OUT(319, 0),
	/* DEVICE 320 */
	HD_VIDEOPROC_320_OUT_0 = HD_VIDEOPROC_OUT(320, 0),
	/* DEVICE 321 */
	HD_VIDEOPROC_321_OUT_0 = HD_VIDEOPROC_OUT(321, 0),
	/* DEVICE 322 */
	HD_VIDEOPROC_322_OUT_0 = HD_VIDEOPROC_OUT(322, 0),
	/* DEVICE 323 */
	HD_VIDEOPROC_323_OUT_0 = HD_VIDEOPROC_OUT(323, 0),
	/* DEVICE 324 */
	HD_VIDEOPROC_324_OUT_0 = HD_VIDEOPROC_OUT(324, 0),
	/* DEVICE 325 */
	HD_VIDEOPROC_325_OUT_0 = HD_VIDEOPROC_OUT(325, 0),
	/* DEVICE 326 */
	HD_VIDEOPROC_326_OUT_0 = HD_VIDEOPROC_OUT(326, 0),
	/* DEVICE 327 */
	HD_VIDEOPROC_327_OUT_0 = HD_VIDEOPROC_OUT(327, 0),
	/* DEVICE 328 */
	HD_VIDEOPROC_328_OUT_0 = HD_VIDEOPROC_OUT(328, 0),
	/* DEVICE 329 */
	HD_VIDEOPROC_329_OUT_0 = HD_VIDEOPROC_OUT(329, 0),
	/* DEVICE 330 */
	HD_VIDEOPROC_330_OUT_0 = HD_VIDEOPROC_OUT(330, 0),
	/* DEVICE 331 */
	HD_VIDEOPROC_331_OUT_0 = HD_VIDEOPROC_OUT(331, 0),
	/* DEVICE 332 */
	HD_VIDEOPROC_332_OUT_0 = HD_VIDEOPROC_OUT(332, 0),
	/* DEVICE 333 */
	HD_VIDEOPROC_333_OUT_0 = HD_VIDEOPROC_OUT(333, 0),
	/* DEVICE 334 */
	HD_VIDEOPROC_334_OUT_0 = HD_VIDEOPROC_OUT(334, 0),
	/* DEVICE 335 */
	HD_VIDEOPROC_335_OUT_0 = HD_VIDEOPROC_OUT(335, 0),
	/* DEVICE 336 */
	HD_VIDEOPROC_336_OUT_0 = HD_VIDEOPROC_OUT(336, 0),
	/* DEVICE 337 */
	HD_VIDEOPROC_337_OUT_0 = HD_VIDEOPROC_OUT(337, 0),
	/* DEVICE 338 */
	HD_VIDEOPROC_338_OUT_0 = HD_VIDEOPROC_OUT(338, 0),
	/* DEVICE 339 */
	HD_VIDEOPROC_339_OUT_0 = HD_VIDEOPROC_OUT(339, 0),
	/* DEVICE 340 */
	HD_VIDEOPROC_340_OUT_0 = HD_VIDEOPROC_OUT(340, 0),
	/* DEVICE 341 */
	HD_VIDEOPROC_341_OUT_0 = HD_VIDEOPROC_OUT(341, 0),
	/* DEVICE 342 */
	HD_VIDEOPROC_342_OUT_0 = HD_VIDEOPROC_OUT(342, 0),
	/* DEVICE 343 */
	HD_VIDEOPROC_343_OUT_0 = HD_VIDEOPROC_OUT(343, 0),
	/* DEVICE 344 */
	HD_VIDEOPROC_344_OUT_0 = HD_VIDEOPROC_OUT(344, 0),
	/* DEVICE 345 */
	HD_VIDEOPROC_345_OUT_0 = HD_VIDEOPROC_OUT(345, 0),
	/* DEVICE 346 */
	HD_VIDEOPROC_346_OUT_0 = HD_VIDEOPROC_OUT(346, 0),
	/* DEVICE 347 */
	HD_VIDEOPROC_347_OUT_0 = HD_VIDEOPROC_OUT(347, 0),
	/* DEVICE 348 */
	HD_VIDEOPROC_348_OUT_0 = HD_VIDEOPROC_OUT(348, 0),
	/* DEVICE 349 */
	HD_VIDEOPROC_349_OUT_0 = HD_VIDEOPROC_OUT(349, 0),
	/* DEVICE 350 */
	HD_VIDEOPROC_350_OUT_0 = HD_VIDEOPROC_OUT(350, 0),
	/* DEVICE 351 */
	HD_VIDEOPROC_351_OUT_0 = HD_VIDEOPROC_OUT(351, 0),
	/* DEVICE 352 */
	HD_VIDEOPROC_352_OUT_0 = HD_VIDEOPROC_OUT(352, 0),
	/* DEVICE 353 */
	HD_VIDEOPROC_353_OUT_0 = HD_VIDEOPROC_OUT(353, 0),
	/* DEVICE 354 */
	HD_VIDEOPROC_354_OUT_0 = HD_VIDEOPROC_OUT(354, 0),
	/* DEVICE 355 */
	HD_VIDEOPROC_355_OUT_0 = HD_VIDEOPROC_OUT(355, 0),
	/* DEVICE 356 */
	HD_VIDEOPROC_356_OUT_0 = HD_VIDEOPROC_OUT(356, 0),
	/* DEVICE 357 */
	HD_VIDEOPROC_357_OUT_0 = HD_VIDEOPROC_OUT(357, 0),
	/* DEVICE 358 */
	HD_VIDEOPROC_358_OUT_0 = HD_VIDEOPROC_OUT(358, 0),
	/* DEVICE 359 */
	HD_VIDEOPROC_359_OUT_0 = HD_VIDEOPROC_OUT(359, 0),
	/* DEVICE 360 */
	HD_VIDEOPROC_360_OUT_0 = HD_VIDEOPROC_OUT(360, 0),
	/* DEVICE 361 */
	HD_VIDEOPROC_361_OUT_0 = HD_VIDEOPROC_OUT(361, 0),
	/* DEVICE 362 */
	HD_VIDEOPROC_362_OUT_0 = HD_VIDEOPROC_OUT(362, 0),
	/* DEVICE 363 */
	HD_VIDEOPROC_363_OUT_0 = HD_VIDEOPROC_OUT(363, 0),
	/* DEVICE 364 */
	HD_VIDEOPROC_364_OUT_0 = HD_VIDEOPROC_OUT(364, 0),
	/* DEVICE 365 */
	HD_VIDEOPROC_365_OUT_0 = HD_VIDEOPROC_OUT(365, 0),
	/* DEVICE 366 */
	HD_VIDEOPROC_366_OUT_0 = HD_VIDEOPROC_OUT(366, 0),
	/* DEVICE 367 */
	HD_VIDEOPROC_367_OUT_0 = HD_VIDEOPROC_OUT(367, 0),
	/* DEVICE 368 */
	HD_VIDEOPROC_368_OUT_0 = HD_VIDEOPROC_OUT(368, 0),
	/* DEVICE 369 */
	HD_VIDEOPROC_369_OUT_0 = HD_VIDEOPROC_OUT(369, 0),
	/* DEVICE 370 */
	HD_VIDEOPROC_370_OUT_0 = HD_VIDEOPROC_OUT(370, 0),
	/* DEVICE 371 */
	HD_VIDEOPROC_371_OUT_0 = HD_VIDEOPROC_OUT(371, 0),
	/* DEVICE 372 */
	HD_VIDEOPROC_372_OUT_0 = HD_VIDEOPROC_OUT(372, 0),
	/* DEVICE 373 */
	HD_VIDEOPROC_373_OUT_0 = HD_VIDEOPROC_OUT(373, 0),
	/* DEVICE 374 */
	HD_VIDEOPROC_374_OUT_0 = HD_VIDEOPROC_OUT(374, 0),
	/* DEVICE 375 */
	HD_VIDEOPROC_375_OUT_0 = HD_VIDEOPROC_OUT(375, 0),
	/* DEVICE 376 */
	HD_VIDEOPROC_376_OUT_0 = HD_VIDEOPROC_OUT(376, 0),
	/* DEVICE 377 */
	HD_VIDEOPROC_377_OUT_0 = HD_VIDEOPROC_OUT(377, 0),
	/* DEVICE 378 */
	HD_VIDEOPROC_378_OUT_0 = HD_VIDEOPROC_OUT(378, 0),
	/* DEVICE 379 */
	HD_VIDEOPROC_379_OUT_0 = HD_VIDEOPROC_OUT(379, 0),
	/* DEVICE 380 */
	HD_VIDEOPROC_380_OUT_0 = HD_VIDEOPROC_OUT(380, 0),
	/* DEVICE 381 */
	HD_VIDEOPROC_381_OUT_0 = HD_VIDEOPROC_OUT(381, 0),
	/* DEVICE 382 */
	HD_VIDEOPROC_382_OUT_0 = HD_VIDEOPROC_OUT(382, 0),
	/* DEVICE 383 */
	HD_VIDEOPROC_383_OUT_0 = HD_VIDEOPROC_OUT(383, 0),
	/* DEVICE 384 */
	HD_VIDEOPROC_384_OUT_0 = HD_VIDEOPROC_OUT(384, 0),
	/* DEVICE 385 */
	HD_VIDEOPROC_385_OUT_0 = HD_VIDEOPROC_OUT(385, 0),
	/* DEVICE 386 */
	HD_VIDEOPROC_386_OUT_0 = HD_VIDEOPROC_OUT(386, 0),
	/* DEVICE 387 */
	HD_VIDEOPROC_387_OUT_0 = HD_VIDEOPROC_OUT(387, 0),
	/* DEVICE 388 */
	HD_VIDEOPROC_388_OUT_0 = HD_VIDEOPROC_OUT(388, 0),
	/* DEVICE 389 */
	HD_VIDEOPROC_389_OUT_0 = HD_VIDEOPROC_OUT(389, 0),
	/* DEVICE 390 */
	HD_VIDEOPROC_390_OUT_0 = HD_VIDEOPROC_OUT(390, 0),
	/* DEVICE 391 */
	HD_VIDEOPROC_391_OUT_0 = HD_VIDEOPROC_OUT(391, 0),
	/* DEVICE 392 */
	HD_VIDEOPROC_392_OUT_0 = HD_VIDEOPROC_OUT(392, 0),
	/* DEVICE 393 */
	HD_VIDEOPROC_393_OUT_0 = HD_VIDEOPROC_OUT(393, 0),
	/* DEVICE 394 */
	HD_VIDEOPROC_394_OUT_0 = HD_VIDEOPROC_OUT(394, 0),
	/* DEVICE 395 */
	HD_VIDEOPROC_395_OUT_0 = HD_VIDEOPROC_OUT(395, 0),
	/* DEVICE 396 */
	HD_VIDEOPROC_396_OUT_0 = HD_VIDEOPROC_OUT(396, 0),
	/* DEVICE 397 */
	HD_VIDEOPROC_397_OUT_0 = HD_VIDEOPROC_OUT(397, 0),
	/* DEVICE 398 */
	HD_VIDEOPROC_398_OUT_0 = HD_VIDEOPROC_OUT(398, 0),
	/* DEVICE 399 */
	HD_VIDEOPROC_399_OUT_0 = HD_VIDEOPROC_OUT(399, 0),


	/* DEVICE 400 */
	HD_VIDEOPROC_400_OUT_0 = HD_VIDEOPROC_OUT(400, 0),
	/* DEVICE 401 */
	HD_VIDEOPROC_401_OUT_0 = HD_VIDEOPROC_OUT(401, 0),
	/* DEVICE 402 */
	HD_VIDEOPROC_402_OUT_0 = HD_VIDEOPROC_OUT(402, 0),
	/* DEVICE 403 */
	HD_VIDEOPROC_403_OUT_0 = HD_VIDEOPROC_OUT(403, 0),
	/* DEVICE 404 */
	HD_VIDEOPROC_404_OUT_0 = HD_VIDEOPROC_OUT(404, 0),
	/* DEVICE 405 */
	HD_VIDEOPROC_405_OUT_0 = HD_VIDEOPROC_OUT(405, 0),
	/* DEVICE 406 */
	HD_VIDEOPROC_406_OUT_0 = HD_VIDEOPROC_OUT(406, 0),
	/* DEVICE 407 */
	HD_VIDEOPROC_407_OUT_0 = HD_VIDEOPROC_OUT(407, 0),
	/* DEVICE 408 */
	HD_VIDEOPROC_408_OUT_0 = HD_VIDEOPROC_OUT(408, 0),
	/* DEVICE 409 */
	HD_VIDEOPROC_409_OUT_0 = HD_VIDEOPROC_OUT(409, 0),
	/* DEVICE 410 */
	HD_VIDEOPROC_410_OUT_0 = HD_VIDEOPROC_OUT(410, 0),
	/* DEVICE 411 */
	HD_VIDEOPROC_411_OUT_0 = HD_VIDEOPROC_OUT(411, 0),
	/* DEVICE 412 */
	HD_VIDEOPROC_412_OUT_0 = HD_VIDEOPROC_OUT(412, 0),
	/* DEVICE 413 */
	HD_VIDEOPROC_413_OUT_0 = HD_VIDEOPROC_OUT(413, 0),
	/* DEVICE 414 */
	HD_VIDEOPROC_414_OUT_0 = HD_VIDEOPROC_OUT(414, 0),
	/* DEVICE 415 */
	HD_VIDEOPROC_415_OUT_0 = HD_VIDEOPROC_OUT(415, 0),
	/* DEVICE 416 */
	HD_VIDEOPROC_416_OUT_0 = HD_VIDEOPROC_OUT(416, 0),
	/* DEVICE 417 */
	HD_VIDEOPROC_417_OUT_0 = HD_VIDEOPROC_OUT(417, 0),
	/* DEVICE 418 */
	HD_VIDEOPROC_418_OUT_0 = HD_VIDEOPROC_OUT(418, 0),
	/* DEVICE 419 */
	HD_VIDEOPROC_419_OUT_0 = HD_VIDEOPROC_OUT(419, 0),
	/* DEVICE 420 */
	HD_VIDEOPROC_420_OUT_0 = HD_VIDEOPROC_OUT(420, 0),
	/* DEVICE 421 */
	HD_VIDEOPROC_421_OUT_0 = HD_VIDEOPROC_OUT(421, 0),
	/* DEVICE 422 */
	HD_VIDEOPROC_422_OUT_0 = HD_VIDEOPROC_OUT(422, 0),
	/* DEVICE 423 */
	HD_VIDEOPROC_423_OUT_0 = HD_VIDEOPROC_OUT(423, 0),
	/* DEVICE 424 */
	HD_VIDEOPROC_424_OUT_0 = HD_VIDEOPROC_OUT(424, 0),
	/* DEVICE 425 */
	HD_VIDEOPROC_425_OUT_0 = HD_VIDEOPROC_OUT(425, 0),
	/* DEVICE 426 */
	HD_VIDEOPROC_426_OUT_0 = HD_VIDEOPROC_OUT(426, 0),
	/* DEVICE 427 */
	HD_VIDEOPROC_427_OUT_0 = HD_VIDEOPROC_OUT(427, 0),
	/* DEVICE 428 */
	HD_VIDEOPROC_428_OUT_0 = HD_VIDEOPROC_OUT(428, 0),
	/* DEVICE 429 */
	HD_VIDEOPROC_429_OUT_0 = HD_VIDEOPROC_OUT(429, 0),
	/* DEVICE 430 */
	HD_VIDEOPROC_430_OUT_0 = HD_VIDEOPROC_OUT(430, 0),
	/* DEVICE 431 */
	HD_VIDEOPROC_431_OUT_0 = HD_VIDEOPROC_OUT(431, 0),
	/* DEVICE 432 */
	HD_VIDEOPROC_432_OUT_0 = HD_VIDEOPROC_OUT(432, 0),
	/* DEVICE 433 */
	HD_VIDEOPROC_433_OUT_0 = HD_VIDEOPROC_OUT(433, 0),
	/* DEVICE 434 */
	HD_VIDEOPROC_434_OUT_0 = HD_VIDEOPROC_OUT(434, 0),
	/* DEVICE 435 */
	HD_VIDEOPROC_435_OUT_0 = HD_VIDEOPROC_OUT(435, 0),
	/* DEVICE 436 */
	HD_VIDEOPROC_436_OUT_0 = HD_VIDEOPROC_OUT(436, 0),
	/* DEVICE 437 */
	HD_VIDEOPROC_437_OUT_0 = HD_VIDEOPROC_OUT(437, 0),
	/* DEVICE 438 */
	HD_VIDEOPROC_438_OUT_0 = HD_VIDEOPROC_OUT(438, 0),
	/* DEVICE 439 */
	HD_VIDEOPROC_439_OUT_0 = HD_VIDEOPROC_OUT(439, 0),
	/* DEVICE 440 */
	HD_VIDEOPROC_440_OUT_0 = HD_VIDEOPROC_OUT(440, 0),
	/* DEVICE 441 */
	HD_VIDEOPROC_441_OUT_0 = HD_VIDEOPROC_OUT(441, 0),
	/* DEVICE 442 */
	HD_VIDEOPROC_442_OUT_0 = HD_VIDEOPROC_OUT(442, 0),
	/* DEVICE 443 */
	HD_VIDEOPROC_443_OUT_0 = HD_VIDEOPROC_OUT(443, 0),
	/* DEVICE 444 */
	HD_VIDEOPROC_444_OUT_0 = HD_VIDEOPROC_OUT(444, 0),
	/* DEVICE 445 */
	HD_VIDEOPROC_445_OUT_0 = HD_VIDEOPROC_OUT(445, 0),
	/* DEVICE 446 */
	HD_VIDEOPROC_446_OUT_0 = HD_VIDEOPROC_OUT(446, 0),
	/* DEVICE 447 */
	HD_VIDEOPROC_447_OUT_0 = HD_VIDEOPROC_OUT(447, 0),
	/* DEVICE 448 */
	HD_VIDEOPROC_448_OUT_0 = HD_VIDEOPROC_OUT(448, 0),
	/* DEVICE 449 */
	HD_VIDEOPROC_449_OUT_0 = HD_VIDEOPROC_OUT(449, 0),
	/* DEVICE 450 */
	HD_VIDEOPROC_450_OUT_0 = HD_VIDEOPROC_OUT(450, 0),
	/* DEVICE 451 */
	HD_VIDEOPROC_451_OUT_0 = HD_VIDEOPROC_OUT(451, 0),
	/* DEVICE 452 */
	HD_VIDEOPROC_452_OUT_0 = HD_VIDEOPROC_OUT(452, 0),
	/* DEVICE 453 */
	HD_VIDEOPROC_453_OUT_0 = HD_VIDEOPROC_OUT(453, 0),
	/* DEVICE 454 */
	HD_VIDEOPROC_454_OUT_0 = HD_VIDEOPROC_OUT(454, 0),
	/* DEVICE 455 */
	HD_VIDEOPROC_455_OUT_0 = HD_VIDEOPROC_OUT(455, 0),
	/* DEVICE 456 */
	HD_VIDEOPROC_456_OUT_0 = HD_VIDEOPROC_OUT(456, 0),
	/* DEVICE 457 */
	HD_VIDEOPROC_457_OUT_0 = HD_VIDEOPROC_OUT(457, 0),
	/* DEVICE 458 */
	HD_VIDEOPROC_458_OUT_0 = HD_VIDEOPROC_OUT(458, 0),
	/* DEVICE 459 */
	HD_VIDEOPROC_459_OUT_0 = HD_VIDEOPROC_OUT(459, 0),
	/* DEVICE 460 */
	HD_VIDEOPROC_460_OUT_0 = HD_VIDEOPROC_OUT(460, 0),
	/* DEVICE 461 */
	HD_VIDEOPROC_461_OUT_0 = HD_VIDEOPROC_OUT(461, 0),
	/* DEVICE 462 */
	HD_VIDEOPROC_462_OUT_0 = HD_VIDEOPROC_OUT(462, 0),
	/* DEVICE 463 */
	HD_VIDEOPROC_463_OUT_0 = HD_VIDEOPROC_OUT(463, 0),
	/* DEVICE 464 */
	HD_VIDEOPROC_464_OUT_0 = HD_VIDEOPROC_OUT(464, 0),
	/* DEVICE 465 */
	HD_VIDEOPROC_465_OUT_0 = HD_VIDEOPROC_OUT(465, 0),
	/* DEVICE 466 */
	HD_VIDEOPROC_466_OUT_0 = HD_VIDEOPROC_OUT(466, 0),
	/* DEVICE 467 */
	HD_VIDEOPROC_467_OUT_0 = HD_VIDEOPROC_OUT(467, 0),
	/* DEVICE 468 */
	HD_VIDEOPROC_468_OUT_0 = HD_VIDEOPROC_OUT(468, 0),
	/* DEVICE 469 */
	HD_VIDEOPROC_469_OUT_0 = HD_VIDEOPROC_OUT(469, 0),
	/* DEVICE 470 */
	HD_VIDEOPROC_470_OUT_0 = HD_VIDEOPROC_OUT(470, 0),
	/* DEVICE 471 */
	HD_VIDEOPROC_471_OUT_0 = HD_VIDEOPROC_OUT(471, 0),
	/* DEVICE 472 */
	HD_VIDEOPROC_472_OUT_0 = HD_VIDEOPROC_OUT(472, 0),
	/* DEVICE 473 */
	HD_VIDEOPROC_473_OUT_0 = HD_VIDEOPROC_OUT(473, 0),
	/* DEVICE 474 */
	HD_VIDEOPROC_474_OUT_0 = HD_VIDEOPROC_OUT(474, 0),
	/* DEVICE 475 */
	HD_VIDEOPROC_475_OUT_0 = HD_VIDEOPROC_OUT(475, 0),
	/* DEVICE 476 */
	HD_VIDEOPROC_476_OUT_0 = HD_VIDEOPROC_OUT(476, 0),
	/* DEVICE 477 */
	HD_VIDEOPROC_477_OUT_0 = HD_VIDEOPROC_OUT(477, 0),
	/* DEVICE 478 */
	HD_VIDEOPROC_478_OUT_0 = HD_VIDEOPROC_OUT(478, 0),
	/* DEVICE 479 */
	HD_VIDEOPROC_479_OUT_0 = HD_VIDEOPROC_OUT(479, 0),
	/* DEVICE 480 */
	HD_VIDEOPROC_480_OUT_0 = HD_VIDEOPROC_OUT(480, 0),
	/* DEVICE 481 */
	HD_VIDEOPROC_481_OUT_0 = HD_VIDEOPROC_OUT(481, 0),
	/* DEVICE 482 */
	HD_VIDEOPROC_482_OUT_0 = HD_VIDEOPROC_OUT(482, 0),
	/* DEVICE 483 */
	HD_VIDEOPROC_483_OUT_0 = HD_VIDEOPROC_OUT(483, 0),
	/* DEVICE 484 */
	HD_VIDEOPROC_484_OUT_0 = HD_VIDEOPROC_OUT(484, 0),
	/* DEVICE 485 */
	HD_VIDEOPROC_485_OUT_0 = HD_VIDEOPROC_OUT(485, 0),
	/* DEVICE 486 */
	HD_VIDEOPROC_486_OUT_0 = HD_VIDEOPROC_OUT(486, 0),
	/* DEVICE 487 */
	HD_VIDEOPROC_487_OUT_0 = HD_VIDEOPROC_OUT(487, 0),
	/* DEVICE 488 */
	HD_VIDEOPROC_488_OUT_0 = HD_VIDEOPROC_OUT(488, 0),
	/* DEVICE 489 */
	HD_VIDEOPROC_489_OUT_0 = HD_VIDEOPROC_OUT(489, 0),
	/* DEVICE 490 */
	HD_VIDEOPROC_490_OUT_0 = HD_VIDEOPROC_OUT(490, 0),
	/* DEVICE 491 */
	HD_VIDEOPROC_491_OUT_0 = HD_VIDEOPROC_OUT(491, 0),
	/* DEVICE 492 */
	HD_VIDEOPROC_492_OUT_0 = HD_VIDEOPROC_OUT(492, 0),
	/* DEVICE 493 */
	HD_VIDEOPROC_493_OUT_0 = HD_VIDEOPROC_OUT(493, 0),
	/* DEVICE 494 */
	HD_VIDEOPROC_494_OUT_0 = HD_VIDEOPROC_OUT(494, 0),
	/* DEVICE 495 */
	HD_VIDEOPROC_495_OUT_0 = HD_VIDEOPROC_OUT(495, 0),
	/* DEVICE 496 */
	HD_VIDEOPROC_496_OUT_0 = HD_VIDEOPROC_OUT(496, 0),
	/* DEVICE 497 */
	HD_VIDEOPROC_497_OUT_0 = HD_VIDEOPROC_OUT(497, 0),
	/* DEVICE 498 */
	HD_VIDEOPROC_498_OUT_0 = HD_VIDEOPROC_OUT(498, 0),
	/* DEVICE 499 */
	HD_VIDEOPROC_499_OUT_0 = HD_VIDEOPROC_OUT(499, 0),
	/* DEVICE 500 */
	HD_VIDEOPROC_500_OUT_0 = HD_VIDEOPROC_OUT(500, 0),
	/* DEVICE 501 */
	HD_VIDEOPROC_501_OUT_0 = HD_VIDEOPROC_OUT(501, 0),
	/* DEVICE 502 */
	HD_VIDEOPROC_502_OUT_0 = HD_VIDEOPROC_OUT(502, 0),
	/* DEVICE 503 */
	HD_VIDEOPROC_503_OUT_0 = HD_VIDEOPROC_OUT(503, 0),
	/* DEVICE 504 */
	HD_VIDEOPROC_504_OUT_0 = HD_VIDEOPROC_OUT(504, 0),
	/* DEVICE 505 */
	HD_VIDEOPROC_505_OUT_0 = HD_VIDEOPROC_OUT(505, 0),
	/* DEVICE 506 */
	HD_VIDEOPROC_506_OUT_0 = HD_VIDEOPROC_OUT(506, 0),
	/* DEVICE 507 */
	HD_VIDEOPROC_507_OUT_0 = HD_VIDEOPROC_OUT(507, 0),
	/* DEVICE 508 */
	HD_VIDEOPROC_508_OUT_0 = HD_VIDEOPROC_OUT(508, 0),
	/* DEVICE 509 */
	HD_VIDEOPROC_509_OUT_0 = HD_VIDEOPROC_OUT(509, 0),
	/* DEVICE 510 */
	HD_VIDEOPROC_510_OUT_0 = HD_VIDEOPROC_OUT(510, 0),
	/* DEVICE 511 */
	HD_VIDEOPROC_511_OUT_0 = HD_VIDEOPROC_OUT(511, 0),

	/* DEVICE 0 */
	HD_AUDIOCAP_0_OUT_0 = HD_AUDIOCAP_OUT(0, 0),
	HD_AUDIOCAP_0_OUT_1 = HD_AUDIOCAP_OUT(0, 1),
	HD_AUDIOCAP_0_OUT_2 = HD_AUDIOCAP_OUT(0, 2),
	HD_AUDIOCAP_0_OUT_3 = HD_AUDIOCAP_OUT(0, 3),
	HD_AUDIOCAP_0_OUT_4 = HD_AUDIOCAP_OUT(0, 4),
	HD_AUDIOCAP_0_OUT_5 = HD_AUDIOCAP_OUT(0, 5),
	HD_AUDIOCAP_0_OUT_6 = HD_AUDIOCAP_OUT(0, 6),
	HD_AUDIOCAP_0_OUT_7 = HD_AUDIOCAP_OUT(0, 7),
	HD_AUDIOCAP_0_OUT_8 = HD_AUDIOCAP_OUT(0, 8),
	HD_AUDIOCAP_0_OUT_9 = HD_AUDIOCAP_OUT(0, 9),
	HD_AUDIOCAP_0_OUT_10 = HD_AUDIOCAP_OUT(0, 10),
	HD_AUDIOCAP_0_OUT_11 = HD_AUDIOCAP_OUT(0, 11),
	HD_AUDIOCAP_0_OUT_12 = HD_AUDIOCAP_OUT(0, 12),
	HD_AUDIOCAP_0_OUT_13 = HD_AUDIOCAP_OUT(0, 13),
	HD_AUDIOCAP_0_OUT_14 = HD_AUDIOCAP_OUT(0, 14),
	HD_AUDIOCAP_0_OUT_15 = HD_AUDIOCAP_OUT(0, 15),
	/* DEVICE 1 */
	HD_AUDIOCAP_1_OUT_0 = HD_AUDIOCAP_OUT(1, 0),
	HD_AUDIOCAP_1_OUT_1 = HD_AUDIOCAP_OUT(1, 1),
	HD_AUDIOCAP_1_OUT_2 = HD_AUDIOCAP_OUT(1, 2),
	HD_AUDIOCAP_1_OUT_3 = HD_AUDIOCAP_OUT(1, 3),
	HD_AUDIOCAP_1_OUT_4 = HD_AUDIOCAP_OUT(1, 4),
	HD_AUDIOCAP_1_OUT_5 = HD_AUDIOCAP_OUT(1, 5),
	HD_AUDIOCAP_1_OUT_6 = HD_AUDIOCAP_OUT(1, 6),
	HD_AUDIOCAP_1_OUT_7 = HD_AUDIOCAP_OUT(1, 7),
	HD_AUDIOCAP_1_OUT_8 = HD_AUDIOCAP_OUT(1, 8),
	HD_AUDIOCAP_1_OUT_9 = HD_AUDIOCAP_OUT(1, 9),
	HD_AUDIOCAP_1_OUT_10 = HD_AUDIOCAP_OUT(1, 10),
	HD_AUDIOCAP_1_OUT_11 = HD_AUDIOCAP_OUT(1, 11),
	HD_AUDIOCAP_1_OUT_12 = HD_AUDIOCAP_OUT(1, 12),
	HD_AUDIOCAP_1_OUT_13 = HD_AUDIOCAP_OUT(1, 13),
	HD_AUDIOCAP_1_OUT_14 = HD_AUDIOCAP_OUT(1, 14),
	HD_AUDIOCAP_1_OUT_15 = HD_AUDIOCAP_OUT(1, 15),
	/* DEVICE 2 */
	HD_AUDIOCAP_2_OUT_0 = HD_AUDIOCAP_OUT(2, 0),
	HD_AUDIOCAP_2_OUT_1 = HD_AUDIOCAP_OUT(2, 1),
	HD_AUDIOCAP_2_OUT_2 = HD_AUDIOCAP_OUT(2, 2),
	HD_AUDIOCAP_2_OUT_3 = HD_AUDIOCAP_OUT(2, 3),
	HD_AUDIOCAP_2_OUT_4 = HD_AUDIOCAP_OUT(2, 4),
	HD_AUDIOCAP_2_OUT_5 = HD_AUDIOCAP_OUT(2, 5),
	HD_AUDIOCAP_2_OUT_6 = HD_AUDIOCAP_OUT(2, 6),
	HD_AUDIOCAP_2_OUT_7 = HD_AUDIOCAP_OUT(2, 7),
	HD_AUDIOCAP_2_OUT_8 = HD_AUDIOCAP_OUT(2, 8),
	HD_AUDIOCAP_2_OUT_9 = HD_AUDIOCAP_OUT(2, 9),
	HD_AUDIOCAP_2_OUT_10 = HD_AUDIOCAP_OUT(2, 10),
	HD_AUDIOCAP_2_OUT_11 = HD_AUDIOCAP_OUT(2, 11),
	HD_AUDIOCAP_2_OUT_12 = HD_AUDIOCAP_OUT(2, 12),
	HD_AUDIOCAP_2_OUT_13 = HD_AUDIOCAP_OUT(2, 13),
	HD_AUDIOCAP_2_OUT_14 = HD_AUDIOCAP_OUT(2, 14),
	HD_AUDIOCAP_2_OUT_15 = HD_AUDIOCAP_OUT(2, 15),
	/* DEVICE 3 */
	HD_AUDIOCAP_3_OUT_0 = HD_AUDIOCAP_OUT(3, 0),
	HD_AUDIOCAP_3_OUT_1 = HD_AUDIOCAP_OUT(3, 1),
	HD_AUDIOCAP_3_OUT_2 = HD_AUDIOCAP_OUT(3, 2),
	HD_AUDIOCAP_3_OUT_3 = HD_AUDIOCAP_OUT(3, 3),
	HD_AUDIOCAP_3_OUT_4 = HD_AUDIOCAP_OUT(3, 4),
	HD_AUDIOCAP_3_OUT_5 = HD_AUDIOCAP_OUT(3, 5),
	HD_AUDIOCAP_3_OUT_6 = HD_AUDIOCAP_OUT(3, 6),
	HD_AUDIOCAP_3_OUT_7 = HD_AUDIOCAP_OUT(3, 7),
	HD_AUDIOCAP_3_OUT_8 = HD_AUDIOCAP_OUT(3, 8),
	HD_AUDIOCAP_3_OUT_9 = HD_AUDIOCAP_OUT(3, 9),
	HD_AUDIOCAP_3_OUT_10 = HD_AUDIOCAP_OUT(3, 10),
	HD_AUDIOCAP_3_OUT_11 = HD_AUDIOCAP_OUT(3, 11),
	HD_AUDIOCAP_3_OUT_12 = HD_AUDIOCAP_OUT(3, 12),
	HD_AUDIOCAP_3_OUT_13 = HD_AUDIOCAP_OUT(3, 13),
	HD_AUDIOCAP_3_OUT_14 = HD_AUDIOCAP_OUT(3, 14),
	HD_AUDIOCAP_3_OUT_15 = HD_AUDIOCAP_OUT(3, 15),


	/* DEVICE 0 */
	HD_AUDIOOUT_0_OUT_0 = HD_AUDIOOUT_OUT(0, 0),
	/* DEVICE 0 */
	HD_AUDIOOUT_1_OUT_0 = HD_AUDIOOUT_OUT(1, 0),
	/* DEVICE 0 */
	HD_AUDIOOUT_2_OUT_0 = HD_AUDIOOUT_OUT(2, 0),
	/* DEVICE 0 */
	HD_AUDIOOUT_3_OUT_0 = HD_AUDIOOUT_OUT(3, 0),


	/* DEVICE 0 */
	HD_AUDIOENC_0_OUT_0 = HD_AUDIOENC_OUT(0, 0),
	HD_AUDIOENC_0_OUT_1 = HD_AUDIOENC_OUT(0, 1),
	HD_AUDIOENC_0_OUT_2 = HD_AUDIOENC_OUT(0, 2),
	HD_AUDIOENC_0_OUT_3 = HD_AUDIOENC_OUT(0, 3),
	HD_AUDIOENC_0_OUT_4 = HD_AUDIOENC_OUT(0, 4),
	HD_AUDIOENC_0_OUT_5 = HD_AUDIOENC_OUT(0, 5),
	HD_AUDIOENC_0_OUT_6 = HD_AUDIOENC_OUT(0, 6),
	HD_AUDIOENC_0_OUT_7 = HD_AUDIOENC_OUT(0, 7),
	HD_AUDIOENC_0_OUT_8 = HD_AUDIOENC_OUT(0, 8),
	HD_AUDIOENC_0_OUT_9 = HD_AUDIOENC_OUT(0, 9),
	HD_AUDIOENC_0_OUT_10 = HD_AUDIOENC_OUT(0, 10),
	HD_AUDIOENC_0_OUT_11 = HD_AUDIOENC_OUT(0, 11),
	HD_AUDIOENC_0_OUT_12 = HD_AUDIOENC_OUT(0, 12),
	HD_AUDIOENC_0_OUT_13 = HD_AUDIOENC_OUT(0, 13),
	HD_AUDIOENC_0_OUT_14 = HD_AUDIOENC_OUT(0, 14),
	HD_AUDIOENC_0_OUT_15 = HD_AUDIOENC_OUT(0, 15),
	HD_AUDIOENC_0_OUT_16 = HD_AUDIOENC_OUT(0, 16),
	HD_AUDIOENC_0_OUT_17 = HD_AUDIOENC_OUT(0, 17),
	HD_AUDIOENC_0_OUT_18 = HD_AUDIOENC_OUT(0, 18),
	HD_AUDIOENC_0_OUT_19 = HD_AUDIOENC_OUT(0, 19),
	HD_AUDIOENC_0_OUT_20 = HD_AUDIOENC_OUT(0, 20),
	HD_AUDIOENC_0_OUT_21 = HD_AUDIOENC_OUT(0, 21),
	HD_AUDIOENC_0_OUT_22 = HD_AUDIOENC_OUT(0, 22),
	HD_AUDIOENC_0_OUT_23 = HD_AUDIOENC_OUT(0, 23),
	HD_AUDIOENC_0_OUT_24 = HD_AUDIOENC_OUT(0, 24),
	HD_AUDIOENC_0_OUT_25 = HD_AUDIOENC_OUT(0, 25),
	HD_AUDIOENC_0_OUT_26 = HD_AUDIOENC_OUT(0, 26),
	HD_AUDIOENC_0_OUT_27 = HD_AUDIOENC_OUT(0, 27),
	HD_AUDIOENC_0_OUT_28 = HD_AUDIOENC_OUT(0, 28),
	HD_AUDIOENC_0_OUT_29 = HD_AUDIOENC_OUT(0, 29),
	HD_AUDIOENC_0_OUT_30 = HD_AUDIOENC_OUT(0, 30),
	HD_AUDIOENC_0_OUT_31 = HD_AUDIOENC_OUT(0, 31),

	/* DEVICE 0 */
	HD_AUDIODEC_0_OUT_0 = HD_AUDIODEC_OUT(0, 0),
	HD_AUDIODEC_0_OUT_1 = HD_AUDIODEC_OUT(0, 1),
	HD_AUDIODEC_0_OUT_2 = HD_AUDIODEC_OUT(0, 2),
	HD_AUDIODEC_0_OUT_3 = HD_AUDIODEC_OUT(0, 3),
	HD_AUDIODEC_0_OUT_4 = HD_AUDIODEC_OUT(0, 4),
	HD_AUDIODEC_0_OUT_5 = HD_AUDIODEC_OUT(0, 5),
	HD_AUDIODEC_0_OUT_6 = HD_AUDIODEC_OUT(0, 6),
	HD_AUDIODEC_0_OUT_7 = HD_AUDIODEC_OUT(0, 7),
	HD_AUDIODEC_0_OUT_8 = HD_AUDIODEC_OUT(0, 8),
	HD_AUDIODEC_0_OUT_9 = HD_AUDIODEC_OUT(0, 9),
	HD_AUDIODEC_0_OUT_10 = HD_AUDIODEC_OUT(0, 10),
	HD_AUDIODEC_0_OUT_11 = HD_AUDIODEC_OUT(0, 11),
	HD_AUDIODEC_0_OUT_12 = HD_AUDIODEC_OUT(0, 12),
	HD_AUDIODEC_0_OUT_13 = HD_AUDIODEC_OUT(0, 13),
	HD_AUDIODEC_0_OUT_14 = HD_AUDIODEC_OUT(0, 14),
	HD_AUDIODEC_0_OUT_15 = HD_AUDIODEC_OUT(0, 15),
	HD_AUDIODEC_0_OUT_16 = HD_AUDIODEC_OUT(0, 16),
	HD_AUDIODEC_0_OUT_17 = HD_AUDIODEC_OUT(0, 17),
	HD_AUDIODEC_0_OUT_18 = HD_AUDIODEC_OUT(0, 18),
	HD_AUDIODEC_0_OUT_19 = HD_AUDIODEC_OUT(0, 19),
	HD_AUDIODEC_0_OUT_20 = HD_AUDIODEC_OUT(0, 20),
	HD_AUDIODEC_0_OUT_21 = HD_AUDIODEC_OUT(0, 21),
	HD_AUDIODEC_0_OUT_22 = HD_AUDIODEC_OUT(0, 22),
	HD_AUDIODEC_0_OUT_23 = HD_AUDIODEC_OUT(0, 23),
	HD_AUDIODEC_0_OUT_24 = HD_AUDIODEC_OUT(0, 24),
	HD_AUDIODEC_0_OUT_25 = HD_AUDIODEC_OUT(0, 25),
	HD_AUDIODEC_0_OUT_26 = HD_AUDIODEC_OUT(0, 26),
	HD_AUDIODEC_0_OUT_27 = HD_AUDIODEC_OUT(0, 27),
	HD_AUDIODEC_0_OUT_28 = HD_AUDIODEC_OUT(0, 28),
	HD_AUDIODEC_0_OUT_29 = HD_AUDIODEC_OUT(0, 29),
	HD_AUDIODEC_0_OUT_30 = HD_AUDIODEC_OUT(0, 30),
	HD_AUDIODEC_0_OUT_31 = HD_AUDIODEC_OUT(0, 31),

	ENUM_DUMMY4WORD(HD_OUT_ID)
} HD_OUT_ID;


/********************************************************************
	path id
********************************************************************/
/**
	@name path id of module, user should get path id by open() API
*/
typedef UINT32 HD_PATH_ID;


/********************************************************************
	isp id
********************************************************************/
/**
	@name type of isp id.
*/
typedef enum _HD_ISP_ID {
	HD_ISP_0 =                          0,
	HD_ISP_1 =                          1,
	HD_ISP_2 =                          2,
	HD_ISP_3 =                          3,
	HD_ISP_4 =                          4,
	HD_ISP_5 =                          5,
	HD_ISP_6 =                          6,
	HD_ISP_7 =                          7,
	HD_ISP_DONT_CARE =                  0xffff,
	ENUM_DUMMY4WORD(HD_ISP_ID)
} HD_ISP_ID;


/********************************************************************
	bit-mask of device major capability
********************************************************************/

/**
	@name device general capability
*/
typedef enum _HD_DEVICE_CAPS {
	HD_CAPS_DEVCONFIG =                 0x00000001, ///< require: set HD_XXXX_PARAM_DEV_CONFIG with HD_XXXX_DEV_CONFIG struct
	HD_CAPS_DRVCONFIG =                 0x00000002, ///< support: set HD_XXXX_PARAM_DRV_CONFIG with HD_XXXX_DRV_CONFIG struct
	HD_CAPS_PATHCONFIG =                0x00000004, ///< require: set HD_XXXX_PARAM_PATH_CONFIG with HD_XXXX_PATH_CONFIG struct
	HD_CAPS_LISTFUNC =                  0x00000008, ///< support: hd_xxxx_start_list(), hd_xxxx_stop_list(), hd_xxxx_poll_list(), hd_xxxx_recv_list(), hd_xxxx_send_list() API
	HD_CAPS_ALL =                       0x000000ff,
	ENUM_DUMMY4WORD(HD_DEVICE_CAPS)
} HD_DEVICE_CAPS;

/**
	@name video general capability
*/
typedef enum _HD_VIDEO_CAPS {
	HD_VIDEO_CAPS_AUTO_SYNC =           0x00000001, ///< support auto sync w, h, pxlfmt settings from binding side (if user not set)
	HD_VIDEO_CAPS_RAW =                 0x00000002, ///< support raw pxlfmt
	HD_VIDEO_CAPS_COMPRESS =            0x00000004, ///< support nvx (yuv compress) or nrx (raw compress) pxlfmt
	HD_VIDEO_CAPS_MB =                  0x00000008, ///< support macro-block (non-progressive) pxlfmt
	HD_VIDEO_CAPS_YUV400 =              0x00000010, ///< support yuv400 pxlfmt (y only)
	HD_VIDEO_CAPS_YUV420 =              0x00000020, ///< support yuv420 pxlfmt
	HD_VIDEO_CAPS_YUV422 =              0x00000040, ///< support yuv422 pxlfmt
	HD_VIDEO_CAPS_YUV444 =              0x00000080, ///< support yuv444 pxlfmt
	HD_VIDEO_CAPS_CROP_WIN =            0x00000100, ///< support crop window
	HD_VIDEO_CAPS_CROP_AUTO =           0x00000200, ///< support crop window with auto (dzoom mode)
	HD_VIDEO_CAPS_CAST_WIN =            0x00000400, ///< support cast window
	HD_VIDEO_CAPS_DIR_ROTATE180 =       0x00000800, ///< support rotate 180 degree
	HD_VIDEO_CAPS_SCALE_UP =            0x00001000, ///< support scaling up
	HD_VIDEO_CAPS_SCALE_DOWN =          0x00002000, ///< support scaling down
	HD_VIDEO_CAPS_FRC_UP =              0x00004000, ///< support framerate upsample
	HD_VIDEO_CAPS_FRC_DOWN =            0x00008000, ///< support framerate downsample
	HD_VIDEO_CAPS_DIR_MIRRORX =         0x00010000, ///< support mirror in x direction
	HD_VIDEO_CAPS_DIR_MIRRORY =         0x00020000, ///< support mirror in y direction
	HD_VIDEO_CAPS_DIR_ROTATER =         0x00040000, ///< support rotate 90 degree clockwise
	HD_VIDEO_CAPS_DIR_ROTATEL =         0x00080000, ///< support rotate 270 degree clockwise
	HD_VIDEO_CAPS_STAMP =               0x00100000, ///< support stamp effect
	HD_VIDEO_CAPS_MASK =                0x00200000, ///< support mask effect
	HD_VIDEO_CAPS_ALL =                 0x00ffffff,
	ENUM_DUMMY4WORD(HD_VIDEO_CAPS)
} HD_VIDEO_CAPS;

/**
	@name audio general capability
*/
typedef enum _HD_AUDIO_CAPS {
	HD_AUDIO_CAPS_AUTO_SYNC =           0x00000001, ///< support auto sync bits, ch, sample settings from binding side (if user not set)
	HD_AUDIO_CAPS_BITS_8 =              0x00000010, ///< support 8 bits
	HD_AUDIO_CAPS_BITS_16 =             0x00000020, ///< support 16 bits
	HD_AUDIO_CAPS_CH_MONO =             0x00000100, ///< support mono channel
	HD_AUDIO_CAPS_CH_STEREO =           0x00000200, ///< support stereo pack channel
	HD_AUDIO_CAPS_CH_STEREO_PLANER =    0x00000400, ///< support stereo planer channel
	HD_AUDIO_CAPS_BITS_UP =             0x00001000, ///< support bits up
	HD_AUDIO_CAPS_BITS_DOWN =           0x00002000, ///< support bits down
	HD_AUDIO_CAPS_RESAMPLE_UP =         0x00004000, ///< support samplerate upsample
	HD_AUDIO_CAPS_RESAMPLE_DOWN =       0x00008000, ///< support samplerate downsample
	HD_AUDIO_CAPS_ALL =                 0x00ffffff,
	ENUM_DUMMY4WORD(HD_AUDIO_CAPS)
} HD_AUDIO_CAPS;


/********************************************************************
	METADATA types
********************************************************************/

typedef struct _HD_METADATA {
	UINT32 sign;                        ///< signature = MAKEFOURCC(?,?,?,?)
	struct _HD_METADATA *p_next;        ///< pointer to next meta
} HD_METADATA;


/********************************************************************
	VIDEO FRAME related types and macro
********************************************************************/

typedef struct _HD_DIM {
	UINT32 w;                           ///< image width
	UINT32 h;                           ///< image height
} HD_DIM;

/**
	@name video pixel format
*/
typedef enum _HD_VIDEO_PXLFMT {
	HD_VIDEO_PXLFMT_NONE =              0, ///< don't care
	/* 0 = osd icon index format */
	HD_VIDEO_PXLFMT_I1_ICON =           0x01010001, ///< 1 plane, pixel=INDEX(w,h), w/o padding bits
	HD_VIDEO_PXLFMT_I2_ICON =           0x01020002, ///< 1 plane, pixel=INDEX(w,h), w/o padding bits
	HD_VIDEO_PXLFMT_I4_ICON =           0x01040004, ///< 1 plane, pixel=INDEX(w,h), w/o padding bits
	HD_VIDEO_PXLFMT_I8_ICON =           0x01080008, ///< 1 plane, pixel=INDEX(w,h), w/o padding bits
	/* 1 = osd index format */
	HD_VIDEO_PXLFMT_I1 =                0x11010001, ///< 1 plane, pixel=INDEX(w,h)
	HD_VIDEO_PXLFMT_I2 =                0x11020002, ///< 1 plane, pixel=INDEX(w,h)
	HD_VIDEO_PXLFMT_I4 =                0x11040004, ///< 1 plane, pixel=INDEX(w,h)
	HD_VIDEO_PXLFMT_I8 =                0x11080008, ///< 1 plane, pixel=INDEX(w,h)
	/* 2 = osd argb format */
	HD_VIDEO_PXLFMT_RGB888_PLANAR =     0x23180888, ///< 3 plane, pixel=R(w,h), G(w,h), B(w,h)
	HD_VIDEO_PXLFMT_RGB888 =            0x21180888, ///< 1 plane, pixel=RGB(3w,h)
	HD_VIDEO_PXLFMT_RGB565 =            0x21100565, ///< 1 plane, pixel=RGB(2w,h)
	HD_VIDEO_PXLFMT_RGBA5551 =          0x21105551, ///< 1 plane, pixel=ARGB(2w,h)
	HD_VIDEO_PXLFMT_ARGB1555 =          0x21101555, ///< 1 plane, pixel=ARGB(2w,h)
	HD_VIDEO_PXLFMT_ARGB4444 =          0x21104444, ///< 1 plane, pixel=ARGB(2w,h)
	HD_VIDEO_PXLFMT_A4 =                0x21044000, ///< 1 plane, pixel=A(w,h)
	HD_VIDEO_PXLFMT_ARGB4565 =          0x22404565, ///< 2 plane, pixel=A4(w,h),RGB565(2w,h)
	HD_VIDEO_PXLFMT_A8 =                0x21088000, ///< 1 plane, pixel=A(w,h)
	HD_VIDEO_PXLFMT_ARGB8565 =          0x22188565, ///< 2 plane, pixel=A8(w,h),RGB565(2w,h)
	HD_VIDEO_PXLFMT_ARGB8888 =          0x21208888, ///< 1 plane, pixel=ARGB(4w,h)
	/* 3 = osd compressed argb format */
	HD_VIDEO_PXLFMT_RGB888_RLE =        0x31180888, ///< novatek-rgb-compress
	HD_VIDEO_PXLFMT_RGB565_RLE =        0x31100565, ///< novatek-rgb-compress
	HD_VIDEO_PXLFMT_RGBA5551_RLE =      0x31105551, ///< novatek-rgb-compress
	HD_VIDEO_PXLFMT_ARGB1555_RLE =      0x31101555, ///< novatek-rgb-compress
	HD_VIDEO_PXLFMT_ARGB8888_RLE =      0x31208888, ///< novatek-rgb-compress
	/* 4 = video raw format */
	HD_VIDEO_PXLFMT_RAW8 =              0x41080000, ///< 1 plane, pixel=RAW(w,h) x 8bits
	HD_VIDEO_PXLFMT_RAW10 =             0x410a0000, ///< 1 plane, pixel=RAW(w,h) x 10bits
	HD_VIDEO_PXLFMT_RAW12 =             0x410c0000, ///< 1 plane, pixel=RAW(w,h) x 12bits
	HD_VIDEO_PXLFMT_RAW14 =             0x410e0000, ///< 1 plane, pixel=RAW(w,h) x 14bits
	HD_VIDEO_PXLFMT_RAW16 =             0x41100000, ///< 1 plane, pixel=RAW(w,h) x 16bits
	HD_VIDEO_PXLFMT_RAW8_SHDR2 =        0x42080000, ///< 2 plane, pixel=RAW(w,h) x 8bits, for two-frame SHDR
	HD_VIDEO_PXLFMT_RAW10_SHDR2 =       0x420a0000, ///< 2 plane, pixel=RAW(w,h) x 10bits, for two-frame SHDR
	HD_VIDEO_PXLFMT_RAW12_SHDR2 =       0x420c0000, ///< 2 plane, pixel=RAW(w,h) x 12bits, for two-frame SHDR
	HD_VIDEO_PXLFMT_RAW14_SHDR2 =       0x420e0000, ///< 2 plane, pixel=RAW(w,h) x 14bits, for two-frame SHDR
	HD_VIDEO_PXLFMT_RAW16_SHDR2 =       0x42100000, ///< 2 plane, pixel=RAW(w,h) x 16bits, for two-frame SHDR
	HD_VIDEO_PXLFMT_RAW8_SHDR3 =        0x43080000, ///< 3 plane, pixel=RAW(w,h) x 8bits, for three-frame SHDR
	HD_VIDEO_PXLFMT_RAW10_SHDR3 =       0x430a0000, ///< 3 plane, pixel=RAW(w,h) x 10bits, for three-frame SHDR
	HD_VIDEO_PXLFMT_RAW12_SHDR3 =       0x430c0000, ///< 3 plane, pixel=RAW(w,h) x 12bits, for three-frame SHDR
	HD_VIDEO_PXLFMT_RAW14_SHDR3 =       0x430e0000, ///< 3 plane, pixel=RAW(w,h) x 14bits, for three-frame SHDR
	HD_VIDEO_PXLFMT_RAW16_SHDR3 =       0x43100000, ///< 3 plane, pixel=RAW(w,h) x 16bits, for three-frame SHDR
	HD_VIDEO_PXLFMT_RAW8_SHDR4 =        0x44080000, ///< 4 plane, pixel=RAW(w,h) x 8bits, for four-frame SHDR
	HD_VIDEO_PXLFMT_RAW10_SHDR4 =       0x440a0000, ///< 4 plane, pixel=RAW(w,h) x 10bits, for four-frame SHDR
	HD_VIDEO_PXLFMT_RAW12_SHDR4 =       0x440c0000, ///< 4 plane, pixel=RAW(w,h) x 12bits, for four-frame SHDR
	HD_VIDEO_PXLFMT_RAW14_SHDR4 =       0x440e0000, ///< 4 plane, pixel=RAW(w,h) x 14bits, for four-frame SHDR
	HD_VIDEO_PXLFMT_RAW16_SHDR4 =       0x44100000, ///< 4 plane, pixel=RAW(w,h) x 16bits, for four-frame SHDR
	/* 5 = video yuv format */
	HD_VIDEO_PXLFMT_Y8 =                0x51080400, ///< 1 plane, pixel=Y(w,h)
	HD_VIDEO_PXLFMT_YUV400 =            HD_VIDEO_PXLFMT_Y8,
	HD_VIDEO_PXLFMT_YUV420_PLANAR =     0x530c0420, ///< 3 plane, pixel=Y(w,h), U(w/2,h/2), and V(w/2,h/2)
	HD_VIDEO_PXLFMT_YUV420 =            0x520c0420, ///< 2 plane, pixel=Y(w,h), UV(w/2,h/2), semi-planer format with U1V1
	HD_VIDEO_PXLFMT_YUV422_PLANAR =     0x53100422, ///< 3 plane, pixel=Y(w,h), U(w/2,h), and V(w/2,h)
	HD_VIDEO_PXLFMT_YUV422 =            0x52100422, ///< 2 plane, pixel=Y(w,h), UV(w/2,h), semi-planer format with U1V1
	HD_VIDEO_PXLFMT_YUV422_ONE =        0x51100422, ///< 1 plane, pixel=UYVY(w,h), packed format with Y2U1V1
	HD_VIDEO_PXLFMT_YUV444_PLANAR =     0x53180444, ///< 3 plane, pixel=Y(w,h), U(w,h), and V(w,h)
	HD_VIDEO_PXLFMT_YUV444 =            0x52180444, ///< 2 plane, pixel=Y(w,h), UV(w,h), semi-planer format with U1V1
	HD_VIDEO_PXLFMT_YUV444_ONE =        0x51180444, ///< 1 plane, pixel=YUV(w,h), packed format with Y1U1V1
	HD_VIDEO_PXLFMT_YUV420_W8 =         0x520ca420, ///< 2 plane, pixel=Y(w,h), UV(w/2,h/2), semi-planer format with U8V8
	HD_VIDEO_PXLFMT_YUV420_MB =         0x510cb420, ///< 1 plane, 16x16 macro block format
	HD_VIDEO_PXLFMT_YUV420_MB2 =        0x510cc420, ///< 1 plane, 16x2 macro block format
	/* 6 = video yuv compress format */
	HD_VIDEO_PXLFMT_YUV420_NVX1_H264 =  0x610c0420, ///< novatek-yuv-compress-1 of YUV420 for h264
	HD_VIDEO_PXLFMT_YUV420_NVX1_H265 =  0x610c1420, ///< novatek-yuv-compress-1 of YUV420 for h265
	HD_VIDEO_PXLFMT_YUV420_NVX2 =       0x610c2420, ///< novatek-yuv-compress-2 of YUV420
	HD_VIDEO_PXLFMT_YUV420_NVX3 =       0x610c3420, ///< novatek-yuv-compress-3 of YUV420
	HD_VIDEO_PXLFMT_YUV422_NVX3 =       0x61103422, ///< novatek-yuv-compress-3 of YUV422
	/* f = video raw compress format */
	HD_VIDEO_PXLFMT_NRX8 =              0xf1080000, ///< novatek-raw-compress-1 of RAW8
	HD_VIDEO_PXLFMT_NRX10 =             0xf10a0000, ///< novatek-raw-compress-1 of RAW10
	HD_VIDEO_PXLFMT_NRX12 =             0xf10c0000, ///< novatek-raw-compress-1 of RAW12
	HD_VIDEO_PXLFMT_NRX14 =             0xf10e0000, ///< novatek-raw-compress-1 of RAW14
	HD_VIDEO_PXLFMT_NRX16 =             0xf1100000, ///< novatek-raw-compress-1 of RAW16
	HD_VIDEO_PXLFMT_NRX8_SHDR2 =        0xf2080000, ///< novatek-raw-compress-1 of RAW8
	HD_VIDEO_PXLFMT_NRX10_SHDR2 =       0xf20a0000, ///< novatek-raw-compress-1 of RAW10
	HD_VIDEO_PXLFMT_NRX12_SHDR2 =       0xf20c0000, ///< novatek-raw-compress-1 of RAW12
	HD_VIDEO_PXLFMT_NRX14_SHDR2 =       0xf20e0000, ///< novatek-raw-compress-1 of RAW14
	HD_VIDEO_PXLFMT_NRX16_SHDR2 =       0xf2100000, ///< novatek-raw-compress-1 of RAW16
	HD_VIDEO_PXLFMT_NRX8_SHDR3 =        0xf3080000, ///< novatek-raw-compress-1 of RAW8
	HD_VIDEO_PXLFMT_NRX10_SHDR3 =       0xf30a0000, ///< novatek-raw-compress-1 of RAW10
	HD_VIDEO_PXLFMT_NRX12_SHDR3 =       0xf30c0000, ///< novatek-raw-compress-1 of RAW12
	HD_VIDEO_PXLFMT_NRX14_SHDR3 =       0xf30e0000, ///< novatek-raw-compress-1 of RAW14
	HD_VIDEO_PXLFMT_NRX16_SHDR3 =       0xf3100000, ///< novatek-raw-compress-1 of RAW16
	HD_VIDEO_PXLFMT_NRX8_SHDR4 =        0xf4080000, ///< novatek-raw-compress-1 of RAW8
	HD_VIDEO_PXLFMT_NRX10_SHDR4 =       0xf40a0000, ///< novatek-raw-compress-1 of RAW10
	HD_VIDEO_PXLFMT_NRX12_SHDR4 =       0xf40c0000, ///< novatek-raw-compress-1 of RAW12
	HD_VIDEO_PXLFMT_NRX14_SHDR4 =       0xf40e0000, ///< novatek-raw-compress-1 of RAW14
	HD_VIDEO_PXLFMT_NRX16_SHDR4 =       0xf4100000, ///< novatek-raw-compress-1 of RAW16
	ENUM_DUMMY4WORD(HD_VIDEO_PXLFMT)
} HD_VIDEO_PXLFMT;

#define HD_VIDEO_PXLFMT_DEFALUT         HD_VIDEO_PXLFMT_YUV420

/* mask of HD_VIDEO_PXLFMT */
#define HD_VIDEO_PXLFMT_CLASS_MASK      0xf0000000
#define HD_VIDEO_PXLFMT_PLANE_MASK      0x0f000000
#define HD_VIDEO_PXLFMT_BPP_MASK        0x00ff0000
#define HD_VIDEO_PXLFMT_PIX_MASK        0x0000ffff

/* class of HD_VIDEO_PXLFMT */
#define HD_VIDEO_PXLFMT_CLASS_ICON      0x0
#define HD_VIDEO_PXLFMT_CLASS_I         0x1
#define HD_VIDEO_PXLFMT_CLASS_ARGB      0x2
#define HD_VIDEO_PXLFMT_CLASS_RLE       0x3
#define HD_VIDEO_PXLFMT_CLASS_RAW       0x4
#define HD_VIDEO_PXLFMT_CLASS_YUV       0x5
#define HD_VIDEO_PXLFMT_CLASS_NVX       0x6
#define HD_VIDEO_PXLFMT_CLASS_NRX       0xf
#define HD_VIDEO_PXLFMT_CLASS(pxlfmt)   (((pxlfmt) & HD_VIDEO_PXLFMT_CLASS_MASK) >> 28)

/* plane of HD_VIDEO_PXLFMT */
#define HD_VIDEO_PXLFMT_PLANE(pxlfmt)   (((pxlfmt) & HD_VIDEO_PXLFMT_PLANE_MASK) >> 24)

/* bpp of HD_VIDEO_PXLFMT */
#define HD_VIDEO_PXLFMT_BPP(pxlfmt)     (((pxlfmt) & HD_VIDEO_PXLFMT_BPP_MASK) >> 16)

/* pixel arrange of HD_VIDEO_PXLFMT */
#define HD_VIDEO_PIX_BAYER_MASK         0xff00
#define HD_VIDEO_PIX_RGGB               0x0100
#define HD_VIDEO_PIX_RGGB_R             0x0100
#define HD_VIDEO_PIX_RGGB_GR            0x0101
#define HD_VIDEO_PIX_RGGB_GB            0x0102
#define HD_VIDEO_PIX_RGGB_B             0x0103
#define HD_VIDEO_PIX_RCCB               0x0200
#define HD_VIDEO_PIX_RCCB_RC            0x0200
#define HD_VIDEO_PIX_RCCB_CR            0x0201
#define HD_VIDEO_PIX_RCCB_CB            0x0202
#define HD_VIDEO_PIX_RCCB_BC            0x0203
#define HD_VIDEO_PIX_RGBIR              0x0300
#define HD_VIDEO_PIX_RGBIR_RIR          0x0300
#define HD_VIDEO_PIX_RGBIR_RG           0x0301
#define HD_VIDEO_PIX_RGBIR44            0x0400
#define HD_VIDEO_PIX_RGBIR44_RGBG_GIGI  0x0400
#define HD_VIDEO_PIX_RGBIR44_GBGR_IGIG  0x0401
#define HD_VIDEO_PIX_RGBIR44_GIGI_BGRG  0x0402
#define HD_VIDEO_PIX_RGBIR44_IGIG_GRGB  0x0403
#define HD_VIDEO_PIX_RGBIR44_BGRG_GIGI  0x0404
#define HD_VIDEO_PIX_RGBIR44_GRGB_IGIG  0x0405
#define HD_VIDEO_PIX_RGBIR44_GIGI_RGBG  0x0406
#define HD_VIDEO_PIX_RGBIR44_IGIG_GBGR  0x0407
#define HD_VIDEO_PXLFMT_PIX(pxlfmt)     ((pxlfmt) & HD_VIDEO_PXLFMT_PIX_MASK)

/* type of HD_VIDEO_PXLFMT */
#define HD_VIDEO_PXLFMT_TYPE(pxlfmt)    (((pxlfmt) & (HD_VIDEO_PXLFMT_CLASS_MASK | HD_VIDEO_PXLFMT_PLANE_MASK | HD_VIDEO_PXLFMT_BPP_MASK))


/*

for RAW and NRX format:

[usage example-2]
switch (HD_VIDEO_PXLFMT_TYPE(pxlfmt)) {
case HD_VIDEO_PXLFMT_RAW8:
case HD_VIDEO_PXLFMT_RAW10:
case HD_VIDEO_PXLFMT_RAW12:
case HD_VIDEO_PXLFMT_RAW16:
	{
	    UINT32 stpx = HD_VIDEO_PXLFMT_PIX(pxlfmt);
	}
	break;
}

[usage example-3]
switch (HD_VIDEO_PXLFMT_CLASS(pxlfmt)) {
case HD_VIDEO_PXLFMT_CLASS_RAW:
	{
	    UINT32 bpp = HD_VIDEO_PXLFMT_BPP(pxlfmt);
	    UINT32 stpx = HD_VIDEO_PXLFMT_PIX(pxlfmt);
	}
	break;
}

*/


/* generate HD_VIDEO_PXLFMT of RAW, NRX */
#define HD_VIDEO_PXLFMT_MAKE_RAW(bpp, plane, stpx)      ((HD_VIDEO_PXLFMT_CLASS_RAW << 28) | ((plane)<<24) | (((bpp) << 16) & HD_VIDEO_PXLFMT_BPP_MASK) | ((stpx) & HD_VIDEO_PXLFMT_PIX_MASK))
#define HD_VIDEO_PXLFMT_MAKE_NRX(bpp, plane, stpx)      ((HD_VIDEO_PXLFMT_CLASS_NRX << 28) | ((plane)<<24) | (((bpp) << 16) & HD_VIDEO_PXLFMT_BPP_MASK) | ((stpx) & HD_VIDEO_PXLFMT_PIX_MASK))

/*

for RAW and NRX format:

[gen example-1]
pxlfmt = HD_VIDEO_PXLFMT_MAKE_RAW(16, HD_VIDEO_PIX_RGGB_R);
pxlfmt = HD_VIDEO_PXLFMT_MAKE_NRX(16, HD_VIDEO_PIX_RGGB_R);

*/

/* maximum plane number */
#define HD_VIDEO_MAX_PLANE              4

/* index of osd index plane */
#define HD_VIDEO_PINDEX_I               0
/* index of osd argb plane */
#define HD_VIDEO_PINDEX_R               0
#define HD_VIDEO_PINDEX_G               1
#define HD_VIDEO_PINDEX_B               2
#define HD_VIDEO_PINDEX_A               3
#define HD_VIDEO_PINDEX_RGB             0
#define HD_VIDEO_PINDEX_ARGB            0
/* index of video yuv plane */
#define HD_VIDEO_PINDEX_Y               0
#define HD_VIDEO_PINDEX_U               1
#define HD_VIDEO_PINDEX_V               2
#define HD_VIDEO_PINDEX_UV              1
#define HD_VIDEO_PINDEX_YUV             0
/* index of video raw plane */
#define HD_VIDEO_PINDEX_RAW             0
#define HD_VIDEO_PINDEX_NRX             0



/**
	type of direction of video
*/
typedef unsigned int HD_VIDEO_DIR;      ///< unsigned 32 bits data type
/**
	@name direction of video
*/
#define HD_VIDEO_DIR_NONE               0x00000000 ///< off (default value)
#define HD_VIDEO_DIR_MIRRORX            0x10000000 ///< mirror in x direction
#define HD_VIDEO_DIR_MIRRORY            0x20000000 ///< mirror in y direction
#define HD_VIDEO_DIR_MIRRORXY           0x30000000 ///< mirror in x and y direction
#define HD_VIDEO_DIR_ROTATE_MASK        0x01ffffff ///< rotate mask
#define HD_VIDEO_DIR_ROTATE_0           0x00000000 ///< rotate 0 degrees clockwise
#define HD_VIDEO_DIR_ROTATE_90          0x005A0000 ///< rotate 90 degrees clockwise
#define HD_VIDEO_DIR_ROTATE_180         0x00B40000 ///< rotate 180 degrees clockwise
#define HD_VIDEO_DIR_ROTATE_270         0x010E0000 ///< rotate 270 degrees clockwise
#define HD_VIDEO_DIR_ROTATE_360         0x01680000 ///< rotate 360 degrees clockwise
#define HD_VIDEO_DIR_ROTATE(n)          ((UINT32)((n)*0x10000)) ///< rotate (n) degrees clockwise
#define HD_VIDEO_DIR_DEFAULT            HD_VIDEO_DIR_NONE ///< default

/**
	type of frame-rate-control of video
*/
typedef unsigned int HD_VIDEO_FRC;      ///< unsigned 32 bits data type, please using HD_VIDEO_FRC_RATIO()
/**
	@name frame-rate-control ratio of video
*/
#define HD_VIDEO_FRC_RATIO(dst_fr, src_fr)      MAKE_UINT16_UINT16(dst_fr, src_fr)


typedef enum _HD_CROP_MODE {
	HD_CROP_OFF = 0,                    ///< disable crop
	HD_CROP_ON,                         ///< enable crop
	HD_CROP_AUTO,                       ///< auto calculate crop by DZOOM
	HD_CROP_MODE_MAX,
	ENUM_DUMMY4WORD(HD_CROP_MODE)
} HD_CROP_MODE;

typedef struct _HD_VIDEO_CROP {
	HD_DIM coord;                       ///< {0,0} if using pixel coordinate / {w,h} if using virtual coordinate
	HD_IRECT rect;                      ///< crop window x,y,w,h
} HD_VIDEO_CROP;

typedef struct _HD_PALETTE_TBL {
	UINT32 sign;                        ///< signature = MAKEFOURCC('V','P','A','L')
	HD_METADATA* p_next;                ///< pointer to next meta
	UINT32 reserved[1];                 ///< reserved info
	HD_VIDEO_PXLFMT pxlfmt;             ///< pixel format enum
	UINT32  table_size;                 ///< table size
	UINT32 *p_table;                    ///< pointer to array (UINT32 array[table_size])
} HD_PALETTE_TBL;

typedef struct _HD_VIDEO_FRAME {
	UINT32 sign;                        ///< signature = MAKEFOURCC('V','F','R','M')
	HD_METADATA* p_next;                ///< pointer to next meta
	UINT32 ddr_id;                      ///< ddr id
	HD_VIDEO_PXLFMT pxlfmt;             ///< pixel format enum
	HD_DIM dim;                         ///< video size w,h (of plane 0)
	UINT64 count;                       ///< frame count
	UINT64 timestamp;                   ///< time stamp (unit: microsecond)
	UINT32 pw[HD_VIDEO_MAX_PLANE];      ///< width of each plane
	UINT32 ph[HD_VIDEO_MAX_PLANE];      ///< height of each plane
	UINT32 loff[HD_VIDEO_MAX_PLANE];    ///< line offset of each plane
	UINT32 phy_addr[HD_VIDEO_MAX_PLANE];        ///< physical address of each plane
	UINT32 poc_info;                    ///< poc_info for user trigger
	UINT32 reserved[7];                 ///< reserved info
	INT32  blk;                         ///< memory block
} HD_VIDEO_FRAME;

/********************************************************************
	VIDEO BITSTREAM related types and macro
********************************************************************/

typedef enum _HD_VIDEO_CODEC {
	HD_CODEC_TYPE_JPEG = 0,             ///< jpeg codec type
	HD_CODEC_TYPE_H264,                 ///< h.264 codec type
	HD_CODEC_TYPE_H265,                 ///< h.265 codec type
	HD_CODEC_TYPE_RAW,                  ///< Raw data type (Not encode)
	HD_CODEC_TYPE_MAX,
	ENUM_DUMMY4WORD(HD_VIDEO_CODEC)
} HD_VIDEO_CODEC;


typedef enum _H264_NALU_TYPE {
	H264_NALU_TYPE_SLICE = 1,           ///< Coded slice (P-Frame)
	H264_NALU_TYPE_DPA,                 ///< Coded data partition A
	H264_NALU_TYPE_DPB,                 ///< Coded data partition B
	H264_NALU_TYPE_DPC,                 ///< Coded data partition C
	H264_NALU_TYPE_IDR = 5,             ///< Instantaneous decoder refresh (I-Frame)
	H264_NALU_TYPE_SEI,                 ///< Supplement Enhancement Information
	H264_NALU_TYPE_SPS = 7,             ///< Sequence parameter sets
	H264_NALU_TYPE_PPS = 8,             ///< Picture parameter sets
	H264_NALU_TYPE_AUD,                 ///< AUD
	H264_NALU_TYPE_EOSEQ,               ///< End sequence
	H264_NALU_TYPE_EOSTREAM,            ///< End stream
	H264_NALU_TYPE_FILL,                ///< Filler data
	H264_NALU_TYPE_SPSEXT,              ///< SPS extension
	H264_NALU_TYPE_AUXILIARY,           ///< Auxiliary slice
	H264_NALU_TYPE_MAX,
	ENUM_DUMMY4WORD(H264_NALU_TYPE)
} H264_NALU_TYPE;

typedef enum _H265_NALU_TYPE {
	H265_NALU_TYPE_SLICE = 1,           ///< Coded slice segment of a non-TSA, non-STSA trailing picture
	H265_NALU_TYPE_IDR = 19,            ///< Instantaneous decoder refresh
	H265_NALU_TYPE_VPS = 32,            ///< Video parameter set
	H265_NALU_TYPE_SPS = 33,            ///< Sequence parameter set
	H265_NALU_TYPE_PPS = 34,            ///< Picture parameter set
	H265_NALU_TYPE_AUD = 35,            ///< Access unit delimiter
	H265_NALU_TYPE_EOS_NUT = 36,        ///< End of sequence
	H265_NALU_TYPE_EOB_NUT = 37,        ///< End of bitstream
	H265_NALU_TYPE_SEI_PREFIX = 39,     ///< Supplemental enhancement information
	H265_NALU_TYPE_SEI_SUFFIX = 40,     ///< Supplemental enhancement information
	H265_NALU_TYPE_MAX,
	ENUM_DUMMY4WORD(H265_NALU_TYPE)
} H265_NALU_TYPE;

typedef enum _JPEG_PACK_TYPE {
	JPEG_PACK_TYPE_PIC,                 ///< JPEG picture data
	JPEG_PACK_TYPE_MAX,
	ENUM_DUMMY4WORD(JPEG_PACK_TYPE)
} JPEG_PACK_TYPE;



typedef union _HD_NALU_TYPE {
	H264_NALU_TYPE h264_type;           ///< H.264 pack type
	JPEG_PACK_TYPE jpeg_type;           ///< JPEG pack type
	H265_NALU_TYPE h265_type;           ///< H.265 pack type
} HD_NALU_TYPE;

typedef enum _HD_SVC_LAYER_TYPE {
	HD_SVC_LAYER_1X = 0,                ///< SVC 1X
	HD_SVC_LAYER_2X,                    ///< SVC 2X
	HD_SVC_LAYER_4X,                    ///< SVC 4X
	HD_SVC_LAYER_MAX,                   ///< max count of this enum
	ENUM_DUMMY4WORD(HD_SVC_LAYER_TYPE)
} HD_SVC_LAYER_TYPE;

typedef enum _HD_VIDEO_EVBR_STATE {
	HD_EVBR_STATE_MOTION = 0,           ///< EVBR state - Motion
	HD_EVBR_STATE_STILL = 1,            ///< EVBR state - Still
	HD_EVBR_STATE_MAX,
	ENUM_DUMMY4WORD(HD_VIDEO_EVBR_STATE)
} HD_VIDEO_EVBR_STATE;

typedef enum _HD_FRAME_TYPE {
	HD_FRAME_TYPE_IDR,                  ///< IDR-frame
	HD_FRAME_TYPE_I,                    ///< I-frame
	HD_FRAME_TYPE_P,                    ///< P-frame
	HD_FRAME_TYPE_KP,                   ///< Key P-frame
	HD_FRAME_TYPE_MAX,
	ENUM_DUMMY4WORD(HD_FRAME_TYPE)
} HD_FRAME_TYPE;

typedef struct _HD_VIDEO_BS_PSNR_INFO {
	UINT32 y_mse;                       ///< mean square error of y. psnr = 10 x log10(255*255 / mse)
	UINT32 u_mse;                       ///< mean square error of cb. psnr = 10 x log10(255*255 / mse)
	UINT32 v_mse;                       ///< mean square error of cr. psnr = 10 x log10(255*255 / mse)
} HD_VIDEO_BS_PSNR_INFO;


typedef struct _HD_VIDEO_H264_BLOCK_PARTITION_INFO {
	UINT32 intra_16x16_mb_num;          ///< intra 16x16 mb num
	UINT32 intra_8x8_mb_Num;            ///< intra 8x8 mb num
	UINT32 intra_4x4_mb_num;            ///< intra 4x4 mb num
	UINT32 inter_mb_num;                ///< inter mb num
	UINT32 skip_mb_num;                 ///< skipped mb num
} HD_VIDEO_H264_BLOCK_PARTITION_INFO;

typedef struct _HD_VIDEO_H265_BLOCK_PARTITION_INFO {
	UINT32 intra_32x32_cu_num;          ///< intra 32x32 cu num
	UINT32 intra_16x16_cu_num;          ///< intra 16x16 cu num
	UINT32 intra_8x8_cu_num;            ///< intra 8x8 cu num
	UINT32 inter_64x64_cu_num;          ///< inter 64x64 cu num
	UINT32 inter_32x32_cu_num;          ///< inter 32x32 cu num
	UINT32 inter_16x16_cu_num;          ///< inter 16x16 cu num
	UINT32 skip_cu_num;                 ///< skipped cu num
	UINT32 merge_cu_num;                ///< merged cu num
} HD_VIDEO_H265_BLOCK_PARTITION_INFO;

typedef union _HD_VIDEO_BS_BLOCK_PARTITION_INFO {
	HD_VIDEO_H264_BLOCK_PARTITION_INFO h264_info;       ///< h264 block partition info
	HD_VIDEO_H265_BLOCK_PARTITION_INFO h265_info;       ///< h265 block partition info
} HD_VIDEO_BS_BLOCK_PARTITION_INFO;

typedef struct _HD_VIDEO_BS_DATA {
	HD_NALU_TYPE pack_type;             ///< Pack Type
	UINT32 phy_addr;                    ///< Physical address of encoded data
	UINT32 size;                        ///< size of encoded data
} HD_VIDEO_BS_DATA;

#define VENC_BS_PACK_MAX                4

/**
	video bitstream data descriptor.
*/
typedef struct _HD_VIDEOENC_BS {
	UINT32 sign;                        ///< signature = MAKEFOURCC('V','S','T','M')
	HD_METADATA* p_next;                ///< pointer to next meta
	HD_VIDEO_CODEC vcodec_format;       ///< Encoded format of video frame
	UINT32 pack_num;                    ///< Pack number in video frame
	UINT64 timestamp;                   ///< Encoded timestamp (unit: microsecond)
	HD_FRAME_TYPE frame_type;           ///< Video Frame type
	HD_SVC_LAYER_TYPE svc_layer_type;   ///< SVC temporal layer ID
	HD_VIDEO_BS_DATA video_pack[VENC_BS_PACK_MAX];      ///< Pack array of encoded data
	HD_VIDEO_BS_PSNR_INFO psnr_info;    ///< The PSNR information
	HD_VIDEO_BS_BLOCK_PARTITION_INFO blk_info;          ///< The block partition information
	HD_VIDEO_EVBR_STATE evbr_state;     ///< The evbr state value (significant when rate-control is EVBR)
	UINT32 qp;                          ///< The qp value
	UINT32 ddr_id;                      ///< ddr id
	INT32  blk;                         ///< memory block
	UINT32 motion_ratio;                ///< Encode motion ratio
} HD_VIDEOENC_BS;

/**
	video bitstream data descriptor.
*/
typedef struct _HD_VIDEODEC_BS {
	UINT32 sign;                        ///< signature = MAKEFOURCC('V','S','T','M')
	HD_METADATA* p_next;                ///< pointer to next meta
	HD_VIDEO_CODEC vcodec_format;       ///< codec format of video frame
	UINT64 count;                       ///< frame count
	UINT64 timestamp;                   ///< decode timestamp (unit: microsecond)
	UINT32 ddr_id;                      ///< ddr id
	UINT32 phy_addr;                    ///< physical address of bitstream data
	UINT32 size;                        ///< size of bitstream data
	INT32  blk;                         ///< memory block
} HD_VIDEODEC_BS;

/********************************************************************
	AUDIO FRAME related types and macro
********************************************************************/

typedef enum _HD_AUDIO_SR {
	HD_AUDIO_SR_8000  = 8000,           ///< sample rate 8000 Hz
	HD_AUDIO_SR_11025 = 11025,          ///< sample rate 11025 Hz
	HD_AUDIO_SR_12000 = 12000,          ///< sample rate 12000 Hz
	HD_AUDIO_SR_16000 = 16000,          ///< sample rate 16000 Hz
	HD_AUDIO_SR_22050 = 22050,          ///< sample rate 22050 Hz
	HD_AUDIO_SR_24000 = 24000,          ///< sample rate 24000 Hz
	HD_AUDIO_SR_32000 = 32000,          ///< sample rate 32000 Hz
	HD_AUDIO_SR_44100 = 44100,          ///< sample rate 44100 Hz
	HD_AUDIO_SR_48000 = 48000,          ///< sample rate 48000 Hz
	HD_AUDIO_SR_MAX_CNT = 9,            ///< maximum count
	ENUM_DUMMY4WORD(HD_AUDIO_SR)
} HD_AUDIO_SR;

typedef enum _HD_AUDIO_BIT_WIDTH {
	HD_AUDIO_BIT_WIDTH_8  = 8,          ///< 8bit width
	HD_AUDIO_BIT_WIDTH_16 = 16,         ///< 16bit width
	HD_AUDIO_BIT_WIDTH_MAX_CNT = 2,     ///< maximum count
	ENUM_DUMMY4WORD(HD_AUDIO_BIT_WIDTH)
} HD_AUDIO_BIT_WIDTH;

typedef enum _HD_AUDIO_SOUND_MODE {
	HD_AUDIO_SOUND_MODE_MONO = 1,       ///< Mono
	HD_AUDIO_SOUND_MODE_STEREO = 2,     ///< Stereo packed
	HD_AUDIO_SOUND_MODE_STEREO_PLANAR = 3,    ///< Stereo planar. Each channel is stored in the different buffer.
	HD_AUDIO_SOUND_MODE_MAX_CNT = 3,    ///< maximum count
	ENUM_DUMMY4WORD(HD_AUDIO_SOUND_MODE)
} HD_AUDIO_SOUND_MODE;

typedef enum _HD_AUDIO_MONO {
	HD_AUDIO_MONO_LEFT  = 0,            ///< mono left
	HD_AUDIO_MONO_RIGHT = 1,            ///< mono right
	HD_AUDIO_MONO_MAX_CNT = 2,          ///< maximum count
	ENUM_DUMMY4WORD(HD_AUDIO_MONO)
} HD_AUDIO_MONO;

#define HD_AUDIO_MAX_CH 2

typedef struct _HD_AUDIO_FRAME {
	UINT32 sign;                        ///< signature = MAKEFOURCC('A','F','R','M')
	HD_METADATA* p_next;                ///< pointer to next meta
	UINT32 ddr_id;                      ///< ddr id
	UINT32 size;                        ///< data size (byte per channel)
	UINT32 phy_addr[HD_AUDIO_MAX_CH];   ///< physical address of each channel
	HD_AUDIO_BIT_WIDTH bit_width;       ///< audio sample bit length
	HD_AUDIO_SOUND_MODE sound_mode;     ///< audio channel
	HD_AUDIO_SR sample_rate;            ///< audio sample rate
	UINT32 reserved1;                   ///< reserved
	UINT64 count;                       ///< frame count
	UINT64 timestamp;                   ///< time stamp (unit: microsecond)
	INT32  blk;                         ///< memory block
} HD_AUDIO_FRAME;

/********************************************************************
	AUDIO CODEC related types and macro
********************************************************************/

typedef enum _HD_AUDIO_CODEC {
	HD_AUDIO_CODEC_PCM = 1,             ///< PCM codec type
	HD_AUDIO_CODEC_AAC = 2,             ///< AAC codec type
	HD_AUDIO_CODEC_ADPCM = 3,           ///< ADPCM codec type
	HD_AUDIO_CODEC_ULAW = 4,            ///< G.711 u-law codec type
	HD_AUDIO_CODEC_ALAW = 5,            ///< G.711 A-law codec type
	HD_AUDIO_CODEC_MAX_CNT = 5,         ///< maximum count
	ENUM_DUMMY4WORD(HD_AUDIO_CODEC)
} HD_AUDIO_CODEC;

typedef struct _HD_AUDIO_CODEC_INFO {
	UINT32                 is_support;  ///< this codec type is supported
	HD_AUDIO_SR            support_sr[HD_AUDIO_SR_MAX_CNT];             ///< supported sample rate
	HD_AUDIO_BIT_WIDTH     support_bit[HD_AUDIO_BIT_WIDTH_MAX_CNT];     ///< supported bit width
	HD_AUDIO_SOUND_MODE    support_mode[HD_AUDIO_SOUND_MODE_MAX_CNT];   ///< supported sound mode
} HD_AUDIO_CODEC_INFO;

/********************************************************************
	AUDIO BITSTREAM related types and macro
********************************************************************/
/**
	audio bitstream data descriptor.
*/
typedef struct _HD_AUDIO_BS {
	UINT32 sign;                        ///< signature = MAKEFOURCC('A','S','T','M')
	HD_METADATA* p_next;                ///< pointer to next meta
	HD_AUDIO_CODEC acodec_format;       ///< encoded format of audio frame
	UINT64 timestamp;                   ///< encoded timestamp (unit: microsecond)
	UINT32 size;                        ///< size of encoded data
	UINT32 ddr_id;                      ///< ddr id
	UINT32 phy_addr;                    ///< physical address of encoded data
	INT32  blk;                         ///< memory block
} HD_AUDIO_BS;

/********************************************************************
	OSG related types and macro
********************************************************************/
/**
	@name stamp id.
*/
#define HD_STAMP_BASE                   (HD_IN_MAX+1)
#define HD_STAMP_MAX                    (HD_STAMP_BASE+32-1)
#define HD_STAMP(i)                     (HD_STAMP_BASE+(i))

/**
	@name stamp_ex id.
*/
#define HD_STAMP_EX_BASE                (HD_STAMP_MAX+1)
#define HD_STAMP_EX_MAX                 (HD_STAMP_EX_BASE+64-1)
#define HD_STAMP_EX(i)                  (HD_STAMP_EX_BASE+(i))

/**
	@name mask id.
*/
#define HD_MASK_BASE                    (HD_STAMP_EX_MAX+1)
#define HD_MASK_MAX                     (HD_MASK_BASE+8-1)
#define HD_MASK(i)                      (HD_MASK_BASE+(i))

/**
	@name mask_ex id.
*/
#define HD_MASK_EX_BASE                 (HD_MASK_MAX+1)
#define HD_MASK_EX_MAX                  (HD_MASK_EX_BASE+16-1)
#define HD_MASK_EX(i)                   (HD_MASK_EX_BASE+(i))


/**
	@name type of osg id.
*/
typedef enum _HD_OSG_ID {

	HD_STAMP_0 = HD_STAMP(0),
	HD_STAMP_1 = HD_STAMP(1),
	HD_STAMP_2 = HD_STAMP(2),
	HD_STAMP_3 = HD_STAMP(3),
	HD_STAMP_4 = HD_STAMP(4),
	HD_STAMP_5 = HD_STAMP(5),
	HD_STAMP_6 = HD_STAMP(6),
	HD_STAMP_7 = HD_STAMP(7),
	HD_STAMP_8 = HD_STAMP(8),
	HD_STAMP_9 = HD_STAMP(9),
	HD_STAMP_10 = HD_STAMP(10),
	HD_STAMP_11 = HD_STAMP(11),
	HD_STAMP_12 = HD_STAMP(12),
	HD_STAMP_13 = HD_STAMP(13),
	HD_STAMP_14 = HD_STAMP(14),
	HD_STAMP_15 = HD_STAMP(15),
	HD_STAMP_16 = HD_STAMP(16),
	HD_STAMP_17 = HD_STAMP(17),
	HD_STAMP_18 = HD_STAMP(18),
	HD_STAMP_19 = HD_STAMP(19),
	HD_STAMP_20 = HD_STAMP(20),
	HD_STAMP_21 = HD_STAMP(21),
	HD_STAMP_22 = HD_STAMP(22),
	HD_STAMP_23 = HD_STAMP(23),
	HD_STAMP_24 = HD_STAMP(24),
	HD_STAMP_25 = HD_STAMP(25),
	HD_STAMP_26 = HD_STAMP(26),
	HD_STAMP_27 = HD_STAMP(27),
	HD_STAMP_28 = HD_STAMP(28),
	HD_STAMP_29 = HD_STAMP(29),
	HD_STAMP_30 = HD_STAMP(30),
	HD_STAMP_31 = HD_STAMP(31),

	HD_STAMP_EX_0 = HD_STAMP_EX(0),
	HD_STAMP_EX_1 = HD_STAMP_EX(1),
	HD_STAMP_EX_2 = HD_STAMP_EX(2),
	HD_STAMP_EX_3 = HD_STAMP_EX(3),
	HD_STAMP_EX_4 = HD_STAMP_EX(4),
	HD_STAMP_EX_5 = HD_STAMP_EX(5),
	HD_STAMP_EX_6 = HD_STAMP_EX(6),
	HD_STAMP_EX_7 = HD_STAMP_EX(7),
	HD_STAMP_EX_8 = HD_STAMP_EX(8),
	HD_STAMP_EX_9 = HD_STAMP_EX(9),
	HD_STAMP_EX_10 = HD_STAMP_EX(10),
	HD_STAMP_EX_11 = HD_STAMP_EX(11),
	HD_STAMP_EX_12 = HD_STAMP_EX(12),
	HD_STAMP_EX_13 = HD_STAMP_EX(13),
	HD_STAMP_EX_14 = HD_STAMP_EX(14),
	HD_STAMP_EX_15 = HD_STAMP_EX(15),
	HD_STAMP_EX_16 = HD_STAMP_EX(16),
	HD_STAMP_EX_17 = HD_STAMP_EX(17),
	HD_STAMP_EX_18 = HD_STAMP_EX(18),
	HD_STAMP_EX_19 = HD_STAMP_EX(19),
	HD_STAMP_EX_20 = HD_STAMP_EX(20),
	HD_STAMP_EX_21 = HD_STAMP_EX(21),
	HD_STAMP_EX_22 = HD_STAMP_EX(22),
	HD_STAMP_EX_23 = HD_STAMP_EX(23),
	HD_STAMP_EX_24 = HD_STAMP_EX(24),
	HD_STAMP_EX_25 = HD_STAMP_EX(25),
	HD_STAMP_EX_26 = HD_STAMP_EX(26),
	HD_STAMP_EX_27 = HD_STAMP_EX(27),
	HD_STAMP_EX_28 = HD_STAMP_EX(28),
	HD_STAMP_EX_29 = HD_STAMP_EX(29),
	HD_STAMP_EX_30 = HD_STAMP_EX(30),
	HD_STAMP_EX_31 = HD_STAMP_EX(31),
	HD_STAMP_EX_32 = HD_STAMP_EX(32),
	HD_STAMP_EX_33 = HD_STAMP_EX(33),
	HD_STAMP_EX_34 = HD_STAMP_EX(34),
	HD_STAMP_EX_35 = HD_STAMP_EX(35),
	HD_STAMP_EX_36 = HD_STAMP_EX(36),
	HD_STAMP_EX_37 = HD_STAMP_EX(37),
	HD_STAMP_EX_38 = HD_STAMP_EX(38),
	HD_STAMP_EX_39 = HD_STAMP_EX(39),
	HD_STAMP_EX_40 = HD_STAMP_EX(40),
	HD_STAMP_EX_41 = HD_STAMP_EX(41),
	HD_STAMP_EX_42 = HD_STAMP_EX(42),
	HD_STAMP_EX_43 = HD_STAMP_EX(43),
	HD_STAMP_EX_44 = HD_STAMP_EX(44),
	HD_STAMP_EX_45 = HD_STAMP_EX(45),
	HD_STAMP_EX_46 = HD_STAMP_EX(46),
	HD_STAMP_EX_47 = HD_STAMP_EX(47),
	HD_STAMP_EX_48 = HD_STAMP_EX(48),
	HD_STAMP_EX_49 = HD_STAMP_EX(49),
	HD_STAMP_EX_50 = HD_STAMP_EX(50),
	HD_STAMP_EX_51 = HD_STAMP_EX(51),
	HD_STAMP_EX_52 = HD_STAMP_EX(52),
	HD_STAMP_EX_53 = HD_STAMP_EX(53),
	HD_STAMP_EX_54 = HD_STAMP_EX(54),
	HD_STAMP_EX_55 = HD_STAMP_EX(55),
	HD_STAMP_EX_56 = HD_STAMP_EX(56),
	HD_STAMP_EX_57 = HD_STAMP_EX(57),
	HD_STAMP_EX_58 = HD_STAMP_EX(58),
	HD_STAMP_EX_59 = HD_STAMP_EX(59),
	HD_STAMP_EX_60 = HD_STAMP_EX(60),
	HD_STAMP_EX_61 = HD_STAMP_EX(61),
	HD_STAMP_EX_62 = HD_STAMP_EX(62),
	HD_STAMP_EX_63 = HD_STAMP_EX(63),

	HD_MASK_0 = HD_MASK(0),
	HD_MASK_1 = HD_MASK(1),
	HD_MASK_2 = HD_MASK(2),
	HD_MASK_3 = HD_MASK(3),
	HD_MASK_4 = HD_MASK(4),
	HD_MASK_5 = HD_MASK(5),
	HD_MASK_6 = HD_MASK(6),
	HD_MASK_7 = HD_MASK(7),

	HD_MASK_EX_0 = HD_MASK_EX(0),
	HD_MASK_EX_1 = HD_MASK_EX(1),
	HD_MASK_EX_2 = HD_MASK_EX(2),
	HD_MASK_EX_3 = HD_MASK_EX(3),
	HD_MASK_EX_4 = HD_MASK_EX(4),
	HD_MASK_EX_5 = HD_MASK_EX(5),
	HD_MASK_EX_6 = HD_MASK_EX(6),
	HD_MASK_EX_7 = HD_MASK_EX(7),
	HD_MASK_EX_8 = HD_MASK_EX(8),
	HD_MASK_EX_9 = HD_MASK_EX(9),
	HD_MASK_EX_10 = HD_MASK_EX(10),
	HD_MASK_EX_11 = HD_MASK_EX(11),
	HD_MASK_EX_12 = HD_MASK_EX(12),
	HD_MASK_EX_13 = HD_MASK_EX(13),
	HD_MASK_EX_14 = HD_MASK_EX(14),
	HD_MASK_EX_15 = HD_MASK_EX(15),

	ENUM_DUMMY4WORD(HD_OSG_ID)
} HD_OSG_ID;


typedef enum _HD_OSG_MASK_TYPE {
	HD_OSG_MASK_TYPE_NULL,
	HD_OSG_MASK_TYPE_SOLID,             ///> mask is solid
	HD_OSG_MASK_TYPE_HOLLOW,            ///> mask is hollow
	HD_OSG_MASK_TYPE_INVERSE,           ///> whole streaming is black-out, only mask region is visible
	HD_OSG_MASK_TYPE_MAX,
	ENUM_DUMMY4WORD(HD_OSG_MASK_TYPE)
} HD_OSG_MASK_TYPE;

typedef enum _HD_OSG_BUF_TYPE {
	HD_OSG_BUF_TYPE_NULL,
	HD_OSG_BUF_TYPE_SINGLE,             ///< image uses singe buffer
	HD_OSG_BUF_TYPE_PING_PONG,          ///< image uses ping pong buffer
	HD_OSG_BUF_TYPE_MAX,
	ENUM_DUMMY4WORD(HD_OSG_BUF_TYPE)
} HD_OSG_BUF_TYPE;

typedef enum _HD_OSG_ALIGN_TYPE {
	HD_OSG_ALIGN_TYPE_TOP_LEFT,         ///< aligned to top in vertical and left in horizon
	HD_OSG_ALIGN_TYPE_TOP_MIDDLE,       ///< aligned to top in vertical and center in horizon
	HD_OSG_ALIGN_TYPE_TOP_RIGHT,        ///< aligned to top in vertical and right in horizon
	HD_OSG_ALIGN_TYPE_CENTER_LEFT,      ///< aligned to center in vertical and left in horizon
	HD_OSG_ALIGN_TYPE_CENTER_MIDDLE,    ///< aligned to center in vertical and center in horizon
	HD_OSG_ALIGN_TYPE_CENTER_RIGHT,     ///< aligned to center in vertical and right in horizon
	HD_OSG_ALIGN_TYPE_BOTTOM_LEFT,      ///< aligned to bottom in vertical and left in horizon
	HD_OSG_ALIGN_TYPE_BOTTOM_MIDDLE,    ///< aligned to bottom in vertical and center in horizon
	HD_OSG_ALIGN_TYPE_BOTTOM_RIGHT,     ///< aligned to bottom in vertical and right in horizon
	HD_OSG_ALIGN_TYPE_MAX,
	ENUM_DUMMY4WORD(HD_OSG_ALIGN_TYPE)
} HD_OSG_ALIGN_TYPE;

typedef enum _HD_OSG_COPY_HW {
	HD_OSG_COPY_HW_NULL,
	HD_OSG_COPY_HW_CPU,
	HD_OSG_COPY_HW_DMA,
	HD_OSG_COPY_HW_MAX,
	ENUM_DUMMY4WORD(HD_OSG_COPY_HW)
} HD_OSG_COPY_HW;

typedef struct _HD_OSG_STAMP_BUF {
	HD_OSG_BUF_TYPE type;               /// ping pong buffer or single buffer
	UINT32 size;                        /// buffer's size in byte
	UINT32 p_addr;                      /// buffer's physical address
	UINT8 ddr_id;                       /// p_addr's ddr_id
} HD_OSG_STAMP_BUF;

typedef struct _HD_OSG_STAMP_IMG {
	HD_VIDEO_PXLFMT fmt;                ///< RGB565/ARGB1555/ARGB4444/ARGB8888/
	HD_DIM dim;                         ///< image's width and height
	UINT32 p_addr;                      ///< image's bitmap content
	UINT8 ddr_id;                       ///< p_addr's ddr_id
	HD_OSG_COPY_HW copy;                ///< use cpu or dma to copy image. only for hd_xxx_get()
} HD_OSG_STAMP_IMG;

typedef struct _HD_OSG_STAMP_ATTR {
	HD_OSG_ALIGN_TYPE align_type;       ///< to which corner is stamp aligned
	UINT32 alpha;                       ///< (DISP)alpha value
	HD_IPOINT position;                 ///< (DISP)stamp's x,y position
	UINT32 colorkey_en;                 ///< is colorkey used to filter background
	UINT32 colorkey_val;                ///< filtered background color
	UINT32 qp_en;                       ///< does stamp have its own qp
	UINT32 qp_fix;                      ///< qp_val is fixed or relative to streaming qp
	UINT32 qp_val;                      ///< qp value
	UINT32 layer;                       ///< set layer attribute for videoenc
	UINT32 region;                      ///< set region attribute for videoenc
	UINT8 gcac_enable;                  ///< (GCAC)gcac enable
	UINT8 gcac_blk_width;               ///< unit width of GCAC
	UINT8 gcac_blk_height;              ///< unit height of GCAC. Note: OSG dim / (gcac_blk_width* gcac_blk_height) must less than 64
} HD_OSG_STAMP_ATTR;

typedef struct _HD_OSG_MASK_ATTR {
	HD_OSG_MASK_TYPE type;              ///< mask is solid or hollow.
	UINT32 color;                       ///< mask color in rgb, mask palette index
	UINT32 alpha;                       ///< mask transparency
	HD_UPOINT position[4];              ///< 4 vertices' position
	UINT32 thickness;                   ///< border width for hollow mask
} HD_OSG_MASK_ATTR;

typedef struct _HD_OSG_MOSAIC_ATTR {
	HD_OSG_MASK_TYPE type;              ///< mask is solid or inversion.
	UINT32 alpha;                       ///< mask alpha blending. range: 0 ~ 256 (0: foreground, 256: background)
	UINT32 mosaic_blk_w;                ///< witdh of internal block
	UINT32 mosaic_blk_h;                ///< height of internal block
	HD_UPOINT position[4];              ///< 4 vertices' position
} HD_OSG_MOSAIC_ATTR;

typedef struct _HD_OSG_PALETTE_TBL {
	UINT8 pal_y[8];                     ///< palette colors y. range: 0 ~ 255
	UINT8 pal_cb[8];                    ///< palette colors cb. range: 0 ~ 255
	UINT8 pal_cr[8];                    ///< palette colors cr. range: 0 ~ 255
} HD_OSG_PALETTE_TBL;


/********************************************************************
	EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
 ********************************************************************/
#endif

