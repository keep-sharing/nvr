/**
    @brief Header file of gfx module.\n
    This file contains the functions which is related to gfx in the chip.

    @file hd_gfx.h

    @ingroup mhdal

    @note Nothing.

    Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#ifndef _HD_GFX_H_
#define _HD_GFX_H_

/********************************************************************
    INCLUDE FILES
 *******************************************************************/
#include "hd_type.h"
#include "hd_util.h"

/********************************************************************
    MACRO CONSTANT DEFINITIONS
 *******************************************************************/

#define HD_GFX_MAX_PLANE_NUM                 3
#define HD_GFX_MAX_MULTI_OUT_NUM             3

/********************************************************************
    MACRO FUNCTION DEFINITIONS
 *******************************************************************/

/********************************************************************
    TYPE DEFINITION
 *******************************************************************/
/**
	Job list handle
*/
typedef UINT32 HD_GFX_HANDLE;

/**
	algorithm and quality for scale
*/
typedef enum _HD_GFX_SCALE_QUALITY {
	HD_GFX_SCALE_QUALITY_NULL = 0,
	HD_GFX_SCALE_QUALITY_BICUBIC,
	HD_GFX_SCALE_QUALITY_BILINEAR,
	HD_GFX_SCALE_QUALITY_NEAREST,
	HD_GFX_SCALE_QUALITY_INTEGRATION,
	HD_GFX_SCALE_QUALITY_MAX,
	ENUM_DUMMY4WORD(HD_GFX_SCALE_QUALITY)
} HD_GFX_SCALE_QUALITY;

/**
	select a solid or a hollow rectangle
*/
typedef enum _HD_GFX_RECT_TYPE {
	HD_GFX_RECT_SOLID = 0,
	HD_GFX_RECT_HOLLOW,
	HD_GFX_RECT_MAX,
	ENUM_DUMMY4WORD(HD_GFX_RECT_TYPE)
} HD_GFX_RECT_TYPE;

/**
	select arithmetic operation
*/
typedef enum _HD_GFX_ARITH_OP{
	HD_GFX_ARITH_OP_NULL = 0,
	HD_GFX_ARITH_OP_PLUS,
	HD_GFX_ARITH_OP_MINUS,
	HD_GFX_ARITH_OP_MULTIPLY,
	HD_GFX_ARITH_OP_MAX,
	ENUM_DUMMY4WORD(HD_GFX_ARITH_OP)
} HD_GFX_ARITH_OP;

/**
	Job list operation
*/
typedef enum _HD_GFX_OP{
	HD_GFX_OP_NULL = 0,
	HD_GFX_OP_CROP,
	HD_GFX_OP_COPY,
	HD_GFX_OP_SCALE,
	HD_GFX_OP_ROTATE,
	HD_GFX_OP_COLOR_TRANSFORM,
	HD_GFX_OP_DRAW_LINE,
	HD_GFX_OP_DRAW_RECT,
	HD_GFX_OP_ARITHMETIC,
	HD_GFX_OP_AFFINE,
	HD_GFX_OP_MAX,
	ENUM_DUMMY4WORD(HD_GFX_OP)
} HD_GFX_OP;

/**
     source and destination images.
*/
typedef struct _HD_GFX_IMG_BUF {
	HD_DIM              dim;                              ///< image dimension
	HD_VIDEO_PXLFMT     format;                           ///< image format
	UINT32              p_phy_addr[HD_GFX_MAX_PLANE_NUM]; ///< image buffers
	UINT32              lineoffset[HD_GFX_MAX_PLANE_NUM]; ///< length in byte of p_phy_addr[]
	UINT8               ddr_id;                           ///< from which ddr is p_phy_addr[] allocated
	UINT32              palette[16];                      ///< for HD_VIDEO_PXLFMT_I1/2/4 images
} HD_GFX_IMG_BUF;

/**
     parameters for hd_gfx_copy.
*/
typedef struct _HD_GFX_COPY {
	HD_GFX_IMG_BUF          src_img;    ///< source image
	HD_GFX_IMG_BUF          dst_img;    ///< destination image
	HD_IRECT                src_region; ///< which part of source image to copy
	HD_IPOINT               dst_pos;    ///< paste origin
	UINT32                  colorkey;   ///< which source color should be transparent
	UINT32                  alpha;      ///< alpha value of source image
} HD_GFX_COPY;

/**
     parameters for hd_gfx_scale.
*/
typedef struct _HD_GFX_SCALE {
	HD_GFX_IMG_BUF          src_img;    ///< source image
	HD_GFX_IMG_BUF          dst_img;    ///< destination image
	HD_IRECT                src_region; ///< which part of source image to scale
	HD_IRECT                dst_region; ///< paste rectangle
	HD_GFX_SCALE_QUALITY    quality;    ///< select scale algorithm
} HD_GFX_SCALE;

/**
     parameters for hd_gfx_rotate.
*/
typedef struct _HD_GFX_ROTATE {
	HD_GFX_IMG_BUF          src_img;    ///< source image
	HD_GFX_IMG_BUF          dst_img;    ///< destination image
	HD_IRECT                src_region; ///< which part of source image to rotate
	HD_IPOINT               dst_pos;    ///< paste origin
	UINT32                  angle;      ///< rotate angle
} HD_GFX_ROTATE;

/**
     parameters for hd_gfx_color_transform.
*/
typedef struct _HD_GFX_COLOR_TRANSFORM {
	HD_GFX_IMG_BUF          src_img;                              ///< source image
	HD_GFX_IMG_BUF          dst_img;                              ///< destination image
	UINT32                  p_tmp_buf;                            ///< transformation between certain formats needs extra buffer
	UINT32                  tmp_buf_size;                         ///< length of p_tmp_buf
	UINT32                  p_lookup_table[HD_GFX_MAX_PLANE_NUM]; ///< for advancing palette tranform
} HD_GFX_COLOR_TRANSFORM;

/**
     parameters for hd_gfx_draw_line.
*/
typedef struct _HD_GFX_DRAW_LINE {
	HD_GFX_IMG_BUF          dst_img;   ///< destination image
	UINT32                  color;     ///< line color
	HD_UPOINT               start;     ///< coordinate of start point
	HD_UPOINT               end;       ///< coordinate of end point
	UINT32                  thickness; ///< border size
} HD_GFX_DRAW_LINE;

/**
     parameters for hd_gfx_draw_rect.
*/
typedef struct _HD_GFX_DRAW_RECT {
	HD_GFX_IMG_BUF          dst_img;   ///< destination image
	UINT32                  color;     ///< rectangle color
	HD_IRECT                rect;      ///< coordinate and dimension
	HD_GFX_RECT_TYPE        type;      ///< solid or hollow rectangle
	UINT32                  thickness; ///< border size for hollow rectangle
} HD_GFX_DRAW_RECT;

/**
     parameters for hd_gfx_arithmetic.
*/
typedef struct _HD_GFX_ARITHMETIC {
	UINT32                  p_op1;     ///< array of operands 1
	UINT32                  p_op2;     ///< array of operands 2
	UINT32                  p_out;     ///< output array
	UINT32                  size;      ///< how many elements in array
	HD_GFX_ARITH_OP         operation; ///< +, -, *, /
	UINT32                  bits;      ///< op is 8 bits or 16 bits
} HD_GFX_ARITHMETIC;

/**
     parameters for hd_gfx_affine.
*/
typedef struct _HD_GFX_AFFINE {
	HD_GFX_IMG_BUF          src_img; ///< source image
	HD_GFX_IMG_BUF          dst_img; ///< destination image
	float                   coeff_a; ///< coefficient a
	float                   coeff_b; ///< coefficient b
	float                   coeff_c; ///< coefficient c
	float                   coeff_d; ///< coefficient d
	float                   coeff_e; ///< coefficient e
	float                   coeff_f; ///< coefficient f
} HD_GFX_AFFINE;

/********************************************************************
    EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/

/**
 * @brief Init gfx framework.
 */
HD_RESULT hd_gfx_init(VOID);

/**
 * @brief Uninit gfx framework.
 */
HD_RESULT hd_gfx_uninit(VOID);

/**
    Copy a portion of source image to destination image.

    @param src_img: configuration of source image.
    @param src_region: which part of source image to copy.
    @param dst_img: configuration of destination image.

    @return HD_OK for success, other value indicates an error.
*/
HD_RESULT hd_gfx_crop_img(HD_GFX_IMG_BUF src_img, HD_IRECT src_region, HD_GFX_IMG_BUF dst_img);

/**
    copy two-dimensional images.

    @param p_param: parameter for copy operation.

    @return HD_OK for success, other value indicates an error.
*/
HD_RESULT hd_gfx_copy(HD_GFX_COPY *p_param);

/**
    scale up/down an image.

    @param p_param: parameter for scale operation.

    @return HD_OK for success, other value indicates an error.
*/
HD_RESULT hd_gfx_scale(HD_GFX_SCALE *p_param);

/**
    rotate an image.

    @param p_param: parameter for rotate operation.

    @return HD_OK for success, other value indicates an error.
*/
HD_RESULT hd_gfx_rotate(HD_GFX_ROTATE *p_param);

/**
    change an image's format(e.g. transform a rgb image to yuv image).

    @param p_param: parameter for transform operation.

    @return HD_OK for success, other value indicates an error.
*/
HD_RESULT hd_gfx_color_transform(HD_GFX_COLOR_TRANSFORM *p_param);

/**
    draw a line on an image.

    @param p_param: parameter for line drawing operation.

    @return HD_OK for success, other value indicates an error.
*/
HD_RESULT hd_gfx_draw_line(HD_GFX_DRAW_LINE *p_param);

/**
    draw a rectangle on an image.

    @param p_param: parameter for rectangle drawing operation.

    @return HD_OK for success, other value indicates an error.
*/
HD_RESULT hd_gfx_draw_rect(HD_GFX_DRAW_RECT *p_param);

/**
    create a job list.

    @param p_handle: created handle. should be used in subsequent job list API

    @return HD_OK for success, other value indicates an error.
*/
HD_RESULT hd_gfx_begin_job(HD_GFX_HANDLE *p_handle);

/**
    trigger all jobs in a list.

    @param handle: which job list to trigger

    @return HD_OK for success, other value indicates an error.
*/
HD_RESULT hd_gfx_end_job(HD_GFX_HANDLE handle);

/**
    cancel all jobs in a list.

    @param handle: which job list to cancel

    @return HD_OK for success, other value indicates an error.
*/
HD_RESULT hd_gfx_cancel_job(HD_GFX_HANDLE handle);

/**
    append a new job to a list.

    @param handle: which job list to append new job
    @param op: type of the new job
    @param p_param: detailed configuration of the new job

    @return HD_OK for success, other value indicates an error.
*/
HD_RESULT hd_gfx_add_job(HD_GFX_HANDLE handle, HD_GFX_OP op, void *p_param);

/**
    append all line drawing jobs in an array to a list.

    @param handle: which job list to append new line drawing job
    @param param: array of new line drawing jobs
    @param num: element number of param

    @return HD_OK for success, other value indicates an error.
*/
HD_RESULT hd_gfx_add_draw_line_list(HD_GFX_HANDLE handle, HD_GFX_DRAW_LINE param[], UINT32 num);

/**
    append all rectangle drawing jobs in an array to a list.

    @param handle: which job list to append new rectangle drawing job
    @param param: array of new rectangle drawing jobs
    @param num: element number of param

    @return HD_OK for success, other value indicates an error.
*/
HD_RESULT hd_gfx_add_draw_rect_list(HD_GFX_HANDLE handle, HD_GFX_DRAW_RECT param[], UINT32 num);

/**
    copy 1-dimensional arrays.

    @param dest: destination buffer
    @param src: source buffer
    @param n: data length in byte to copy

    @return HD_OK for success, other value indicates an error.
*/
void *hd_gfx_memcpy(UINT32 dest, const UINT32 src, size_t n);

/**
    use specific hardware to do add, minus, multiply, divide on two arrays.

    @param p_param: parameters for arithmetic operation.

    @return HD_OK for success, other value indicates an error.
*/
HD_RESULT hd_gfx_arithmetic(HD_GFX_ARITHMETIC *p_param);

/**
    apply affine effect to an image.

    @param p_param: parameter for affine operation.

    @return HD_OK for success, other value indicates an error.
*/
HD_RESULT hd_gfx_affine(HD_GFX_AFFINE *p_param);

#endif //_HD_GFX_H_

