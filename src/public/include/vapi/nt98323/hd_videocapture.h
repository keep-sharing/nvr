/**
	@brief Header file of video capture module.\n
	This file contains the functions which is related to video capture in the chip.

	@file hd_videocapture.h

	@ingroup mhdal

	@note Nothing.

	Copyright Novatek Microelectronics Corp. 2018.  All rights reserved.
*/

#ifndef	_HD_VIDEOCAPTURE_H_
#define	_HD_VIDEOCAPTURE_H_

/********************************************************************
	INCLUDE FILES
********************************************************************/
#include "hd_type.h"
#include "hd_util.h"

/********************************************************************
	MACRO CONSTANT DEFINITIONS
********************************************************************/

#define HD_VIDEOCAP_MAX_IN  1 ///< max count of input of this device (interface)
#define HD_VIDEOCAP_MAX_OUT 8 ///< max count of output of this device (interface)
#define HD_VIDEOCAP_MAX_DATA_TYPE  4  ///< max count of output pool of this device (interface)
#define HD_VIDEOCAP_MAX_MASK       16 ///< max mask NUM
/********************************************************************
	MACRO FUNCTION DEFINITIONS
********************************************************************/


#define HD_VIDEOCAP_RATIO(w, h) (((UINT32)(UINT16)(w) << 16) | (UINT32)(UINT16)(h))    ///< ratio of width and height
#define HD_VIDEOCAP_SET_COUNT(a, b)		((a)*10)+(b)	///< ex: use HD_VIDEOCAP_SET_COUNT(1, 5) for setting 1.5

/********************************************************************
	DRIVER RELATED DEFINITION
********************************************************************/

/*************************************************************************************
	----------------------------------------------------------------------------------
	[Mode]
	----------------------------------------------------------------------------------
	<< Single Edge Latch >>
	1CH Bypass   : X_CAP#0->VI#0-P0------------------->CH#0     (BT656/BT656DH/BT1120)
	4CH Mux      : X_CAP#0->VI#0-P0->Demux#0---------->CH#0,1   (BT656)
						  ->VI#0-P1->Demux#1---------->CH#2,3   (BT656)
	2CH Mux      : X_CAP#0->VI#0-P0->Demux#0---------->CH#0,2   (BT656)
	2CH 2P Bypass: X_CAP#0->VI#0-P0------------------->CH#0     (BT656/BT656DH)
				 : X_CAP#1->VI#0-P1------------------->CH#2     (BT656/BT656DH)
	4CH 2P Mux   : X_CAP#0->VI#0-P0->Demux#0---------->CH#0,1   (BT656)
				 : X_CAP#1->VI#0-P1->Demux#1---------->CH#2,3   (BT656)
	3CH 2P Mix1  : X_CAP#0->VI#0-P0------------------->CH#0     (BT656/BT656DH)
				 : X_CAP#1->VI#0-P1->Demux#1---------->CH#2,3   (BT656)
	3CH 2P Mix2  : X_CAP#0->VI#0-P0->Demux#0---------->CH#0,1   (BT656)
				   X_CAP#1->VI#0-P1------------------->CH#2     (BT656/BT656DH)

	<< Dual Edge Latch >>
	1CH Bypass   : X_CAP#0->VI#0-P0(rising+falling)--->CH#0     (BT656DE)
	2CH 2P Bypass: X_CAP#0->VI#0-P0(rising+falling)--->CH#0     (BT656DE)
				 : X_CAP#1->VI#0-P1(rising+falling)--->CH#2     (BT656DE)
	2CH DualEdge : X_CAP#0->VI#0-P0(rising )---------->CH#0     (BT656)
						  ->VI#0-P1(falling)---------->CH#2     (BT656)
	4CH Mux      : X_CAP#0->VI#0-P0(rising )->Demux#0->CH#0,1   (BT656)
						  ->VI#0-P1(falling)->Demux#1->CH#2,3   (BT656)

**************************************************************************************/
#define HD_VIDEOCAP_VI_MAX 16
#define HD_VIDEOCAP_VI_VPORT_MAX 2

typedef enum _HD_VIDEOCAP_VI_MODE {
	HD_VIDEOCAP_VI_MODE_DISABLE = 0,           ///< Disable
	HD_VIDEOCAP_VI_MODE_1CH,                   ///< CH#0       from VI-P#0
	HD_VIDEOCAP_VI_MODE_2CH,                   ///< CH#0,2     from VI-P#0
	HD_VIDEOCAP_VI_MODE_4CH,                   ///< CH#0,1,2,3 from VI-P#0
	HD_VIDEOCAP_VI_MODE_2CH_2P,                ///< CH#0       from VI-P#0, CH#2   from VI-P#1
	HD_VIDEOCAP_VI_MODE_4CH_2P,                ///< CH#0,1     from VI-P#0, CH#2,3 from VI-P#1
	HD_VIDEOCAP_VI_MODE_3CH_2P_MIX0,           ///< CH#0       from VI-P#0, CH#2,3 from VI-P#1
	HD_VIDEOCAP_VI_MODE_3CH_2P_MIX1,           ///< CH#0,1     from VI-P#0, CH#2   from VI-P#1
	HD_VIDEOCAP_VI_MODE_MAX
} HD_VIDEOCAP_VI_MODE;

typedef enum _HD_VIDEOCAP_VI_SRC {
	HD_VIDEOCAP_VI_SRC_XCAP0 = 0,             ///< VI input source from X_CAP#0             (bt656)
	HD_VIDEOCAP_VI_SRC_XCAP1,                 ///< VI input source from X_CAP#1             (bt656)
	HD_VIDEOCAP_VI_SRC_XCAP2,                 ///< VI input source from X_CAP#2             (bt656)
	HD_VIDEOCAP_VI_SRC_XCAP3,                 ///< VI input source from X_CAP#3             (bt656)
	HD_VIDEOCAP_VI_SRC_XCAP4,                 ///< VI input source from X_CAP#4             (bt656)
	HD_VIDEOCAP_VI_SRC_XCAP5,                 ///< VI input source from X_CAP#5             (bt656)
	HD_VIDEOCAP_VI_SRC_XCAP6,                 ///< VI input source from X_CAP#6             (bt656)
	HD_VIDEOCAP_VI_SRC_XCAP7,                 ///< VI input source from X_CAP#7             (bt656)

	HD_VIDEOCAP_VI_SRC_XCAP8,                 ///< VI input source from X_CAP#8             (bt656)
	HD_VIDEOCAP_VI_SRC_XCAP9,                 ///< VI input source from X_CAP#9             (bt656)
	HD_VIDEOCAP_VI_SRC_XCAP10,                ///< VI input source from X_CAP#10            (bt656)
	HD_VIDEOCAP_VI_SRC_XCAP11,                ///< VI input source from X_CAP#11            (bt656)
	HD_VIDEOCAP_VI_SRC_XCAP12,                ///< VI input source from X_CAP#12            (bt656)
	HD_VIDEOCAP_VI_SRC_XCAP13,                ///< VI input source from X_CAP#13            (bt656)
	HD_VIDEOCAP_VI_SRC_XCAP14,                ///< VI input source from X_CAP#14            (bt656)
	HD_VIDEOCAP_VI_SRC_XCAP15,                ///< VI input source from X_CAP#15            (bt656)

	HD_VIDEOCAP_VI_SRC_XCAP0_1,               ///< VI input source from X_CAP#0  + X_CAP#1  (bt656/bt656x2/bt1120)
	HD_VIDEOCAP_VI_SRC_XCAP2_3,               ///< VI input source from X_CAP#2  + X_CAP#3  (bt656/bt656x2/bt1120)
	HD_VIDEOCAP_VI_SRC_XCAP4_5,               ///< VI input source from X_CAP#4  + X_CAP#5  (bt656/bt656x2/bt1120)
	HD_VIDEOCAP_VI_SRC_XCAP6_7,               ///< VI input source from X_CAP#6  + X_CAP#7  (bt656/bt656x2/bt1120)
	HD_VIDEOCAP_VI_SRC_XCAP8_9,               ///< VI input source from X_CAP#8  + X_CAP#9  (bt656/bt656x2/bt1120)
	HD_VIDEOCAP_VI_SRC_XCAP10_11,             ///< VI input source from X_CAP#10 + X_CAP#11 (bt656/bt656x2/bt1120)
	HD_VIDEOCAP_VI_SRC_XCAP12_13,             ///< VI input source from X_CAP#12 + X_CAP#13 (bt656/bt656x2/bt1120)
	HD_VIDEOCAP_VI_SRC_XCAP14_15,             ///< VI input source from X_CAP#14 + X_CAP#15 (bt656/bt656x2/bt1120)

	HD_VIDEOCAP_VI_SRC_MAX,
	ENUM_DUMMY4WORD(HD_VIDEOCAP_VI_SRC)
} HD_VIDEOCAP_VI_SRC;

typedef enum _HD_VIDEOCAP_VI_FMT {
	HD_VIDEOCAP_VI_FMT_BT656 = 0,             ///< VI input source format BT656
	HD_VIDEOCAP_VI_FMT_BT1120,                ///< VI input source format BT1120
	HD_VIDEOCAP_VI_FMT_MAX,
	ENUM_DUMMY4WORD(HD_VIDEOCAP_VI_FMT)
} HD_VIDEOCAP_VI_FMT;

typedef enum _HD_VIDEOCAP_VI_TDM {
	HD_VIDEOCAP_VI_TDM_1CH_BYPASS = 0,        ///< VI-CH#0,      rising edge from VI-P0
	HD_VIDEOCAP_VI_TDM_4CH_MUX,               ///< VI-CH#0,1,2,3 rising edge from VI-P0
	HD_VIDEOCAP_VI_TDM_2CH_DUALEDGE,          ///< VI-CH#0       rising edge from VI-P0 + VI-CH#2   falling edge from VI-P1
	HD_VIDEOCAP_VI_TDM_2CH_MUX,               ///< VI-CH#0,2     rising edge from VI-P0
	HD_VIDEOCAP_VI_TDM_2CH_2P_BYPASS,         ///< VI-CH#0       rising edge from VI-P0 + VI-CH#2   rising  edge from VI-P1
	HD_VIDEOCAP_VI_TDM_4CH_2P_MUX,            ///< VI-CH#0,1     rising edge from VI-P0 + VI-CH#2,3 rising  edge from VI-P1
	HD_VIDEOCAP_VI_TDM_3CH_2P_MIX1,           ///< VI-CH#0       rising edge from VI-P0 + VI-CH#2,3 rising  edge from VI-P1
	HD_VIDEOCAP_VI_TDM_3CH_2P_MIX2,           ///< VI-CH#0,1     rising edge from VI-P0 + VI-CH#2   rising  edge from VI-P1
	HD_VIDEOCAP_VI_TDM_1CH_2P_DUP,            ///< VI-CH#0       rising edge from VI-P0 + VI-CH#2   rising  edge from VI-P1 duplicate from VI-P0
	HD_VIDEOCAP_VI_TDM_MAX,
	ENUM_DUMMY4WORD(HD_VIDEOCAP_VI_TDM)
} HD_VIDEOCAP_VI_TDM;

typedef enum _HD_VIDEOCAP_VI_CHID {
	HD_VIDEOCAP_VI_CHID_NONE = 0,             ///< channel id extract disable
	HD_VIDEOCAP_VI_CHID_EAV_SAV,              ///< channel id extract from fourth byte of EAV/SAV
	HD_VIDEOCAP_VI_CHID_HORIZ_ACTIVE,         ///< channel id extract from horizontal active in vertical blank
	HD_VIDEOCAP_VI_CHID_HORIZ_BLANK,          ///< channel id extract from horizontal blank
	HD_VIDEOCAP_VI_CHID_MAX,
	ENUM_DUMMY4WORD(HD_VIDEOCAP_VI_CHID)
} HD_VIDEOCAP_VI_CHID;

typedef enum _HD_VIDEOCAP_VI_LATCH_EDGE {
	HD_VIDEOCAP_VI_LATCH_EDGE_SINGLE = 0,     ///< data latch on falling or  rising edge
	HD_VIDEOCAP_VI_LATCH_EDGE_DUAL,           ///< data latch on falling and rising edge
	HD_VIDEOCAP_VI_LATCH_EDGE_MAX,
	ENUM_DUMMY4WORD(HD_VIDEOCAP_VI_LATCH_EDGE)
} HD_VIDEOCAP_VI_LATCH_EDGE;

typedef enum _HD_VIDEOCAP_VI_VPORT_DATA_SWAP {
	HD_VIDEOCAP_VI_VPORT_DATA_SWAP_NONE = 0,  ///< disable data bit swap
	HD_VIDEOCAP_VI_VPORT_DATA_SWAP_BIT,       ///< enable  data bit swap
	HD_VIDEOCAP_VI_VPORT_DATA_SWAP_MAX,
	ENUM_DUMMY4WORD(HD_VIDEOCAP_VI_VPORT_DATA_SWAP)
} HD_VIDEOCAP_VI_VPORT_DATA_SWAP;

typedef enum _HD_VIDEOCAP_VI_CH_FMT {
	HD_VIDEOCAP_VI_CH_FMT_BT656 = 0,           ///< BT656  Standard
	HD_VIDEOCAP_VI_CH_FMT_BT1120,              ///< BT1120 Standard
	HD_VIDEOCAP_VI_CH_FMT_BT656DH,             ///< BT656  DualHeader
	HD_VIDEOCAP_VI_CH_FMT_MAX,
	ENUM_DUMMY4WORD(HD_VIDEOCAP_VI_CH_FMT)
} HD_VIDEOCAP_VI_CH_FMT;

typedef enum _HD_VIDEOCAP_VI_CH_PROG {
	HD_VIDEOCAP_VI_CH_PROG_INTERLACE = 0,      ///< interlace   signal
	HD_VIDEOCAP_VI_CH_PROG_PROGRESSIVE,        ///< progressive signal
	HD_VIDEOCAP_VI_CH_PROG_MAX,
	ENUM_DUMMY4WORD(HD_VIDEOCAP_VI_CH_PROG)
} HD_VIDEOCAP_VI_CH_PROG;

typedef enum _HD_VIDEOCAP_VI_CH_DATA_RATE {
	HD_VIDEOCAP_VI_CH_DATA_RATE_SINGLE = 0,    ///< single data rate
	HD_VIDEOCAP_VI_CH_DATA_RATE_DOUBLE,        ///< double date rate, byte duplicate
	HD_VIDEOCAP_VI_CH_DATA_RATE_MAX,
	ENUM_DUMMY4WORD(HD_VIDEOCAP_VI_CH_DATA_RATE)
} HD_VIDEOCAP_VI_CH_DATA_RATE;

typedef enum _HD_VIDEOCAP_VI_CH_DATA_LATCH {
	HD_VIDEOCAP_VI_CH_DATA_LATCH_SINGLE = 0,   ///< data latch on rising or  falling
	HD_VIDEOCAP_VI_CH_DATA_LATCH_DUAL,         ///< data latch on rising and falling
	HD_VIDEOCAP_VI_CH_DATA_LATCH_MAX,
	ENUM_DUMMY4WORD(HD_VIDEOCAP_VI_CH_DATA_LATCH)
} HD_VIDEOCAP_VI_CH_DATA_LATCH;

typedef enum _HD_VIDEOCAP_VI_CH_HORIZ_DUP {
	HD_VIDEOCAP_VI_CH_HORIZ_DUP_OFF = 0,       ///< video horizontal pixel duplicate off
	HD_VIDEOCAP_VI_CH_HORIZ_DUP_2X,            ///< video horizontal pixel duplicate 2x, pixel duplicate
	HD_VIDEOCAP_VI_CH_HORIZ_DUP_4X,            ///< video horizontal pixel duplicate 4x, pixel duplicate
	HD_VIDEOCAP_VI_CH_HORIZ_DUP_2X_OVS,        ///< video horizontal pixel duplicate 2x, pixel over sampling
	HD_VIDEOCAP_VI_CH_HORIZ_DUP_MAX,
	ENUM_DUMMY4WORD(HD_VIDEOCAP_VI_CH_HORIZ_DUP)
} HD_VIDEOCAP_VI_CH_HORIZ_DUP;

typedef enum _HD_VIDEOCAP_VI_CH_PARAM_ID {
	HD_VIDEOCAP_VI_CH_PARAM_DATA_RANGE,        ///< channel output data range, parameter value as HD_VIDEOCAP_VI_CH_DATA_RATE_T
	HD_VIDEOCAP_VI_CH_PARAM_YC_SWAP,           ///< channel output ycbcr swap, parameter value as HD_VIDEOCAP_VI_CH_SWAP_T
	HD_VIDEOCAP_VI_CH_PARAM_FPS,               ///< channel source frame rate, parameter value must > 0
	HD_VIDEOCAP_VI_CH_PARAM_TIMEOUT_MS,        ///< channel capture frame timeout ms, parameter value must > 0
	HD_VIDEOCAP_VI_CH_PARAM_VCH_ID,            ///< channel video index => mapping to physical connector index, means VCH index, parameter value as int
	HD_VIDEOCAP_VI_CH_PARAM_VLOS,              ///< channel video loss status, 0:video present 1:video loss
	HD_VIDEOCAP_VI_CH_PARAM_MAX,
	ENUM_DUMMY4WORD(HD_VIDEOCAP_VI_CH_PARAM_ID)
} HD_VIDEOCAP_VI_CH_PARAM_ID;

typedef enum _HD_VIDEOCAP_VI_VPORT_PARAM_ID {
	HD_VIDEOCAP_VI_VPORT_PARAM_CLK_INV = 0,    ///< vi vport pixel clock invertion, parameter value as 0:disable 1:enable
	HD_VIDEOCAP_VI_VPORT_PARAM_CLK_DELAY,      ///< vi vport pixel clock delay, parameter value as 0 ~ 0xff
	HD_VIDEOCAP_VI_VPORT_PARAM_CLK_PDELAY,     ///< vi vport pixel clock polarity delay, parameter value as 0 ~ 0xff, not support in GM8220
	HD_VIDEOCAP_VI_VPORT_PARAM_DATA_SWAP,      ///< vi vport data swap control, parameter value as HD_VIDEOCAP_VI_VPORT_DATA_SWAP
	HD_VIDEOCAP_VI_VPORT_PARAM_MAX,
	ENUM_DUMMY4WORD(HD_VIDEOCAP_VI_VPORT_PARAM_ID)
} HD_VIDEOCAP_VI_VPORT_PARAM_ID;

typedef struct _HD_VIDEOCAP_HOST_ID {
	UINT8 host;                     ///< host index
} HD_VIDEOCAP_HOST_ID;

typedef struct _HD_VIDEOCAP_VI_ID {
	UINT8 chip;                     ///< chip index
	UINT8 vcap;                     ///< vcap index
	UINT8 vi;                       ///< vi   index
} HD_VIDEOCAP_VI_ID;

typedef struct _HD_VIDEOCAP_HOST  {
	UINT8 host;                     ///< host index

	/* System VI Parameter */
	UINT32 nr_of_vi;                ///< number of video interface
	struct {
		UINT32 chip	: 8;             ///< chip index
		UINT32 vcap	: 8;             ///< vcap index
		UINT32 vi   : 8;             ///< vi index
		UINT32 mode	: 8;             ///< vi mode, value as HD_VIDEOCAP_VI_MODE
		UINT32 reserved;
	} vi[HD_VIDEOCAP_VI_MAX];

	/* System Motion Detection Parameter */
	struct {
		UINT32 enable      : 1;     ///< enable/disable capture motion/tamper detection support
		UINT32 mb_x_num_max:12;     ///< specify horizontial motion block max number, 0 ~ 128
		UINT32 mb_y_num_max:12;     ///< specify vertical motion block max number, 0 ~ 128
		UINT32 buf_src     : 1;     ///< motion detection buffer allocate source, 0:driver 1:library
		UINT32 reserved    : 6;
	} md;

	/* Reserved */
	UINT32 reserved[8];
} HD_VIDEOCAP_HOST;

typedef struct _HD_VIDEOCAP_VI  {
	UINT8 chip;          ///< chip index
	UINT8 vcap;          ///< vcap index
	UINT8 vi;            ///< vi   index

	/* VI Global Parameter */
	struct {
		HD_VIDEOCAP_VI_SRC src;           ///< vi input source
		HD_VIDEOCAP_VI_FMT format;        ///< vi input source format
		HD_VIDEOCAP_VI_TDM tdm;           ///< vi time division multiplexed mode
		HD_VIDEOCAP_VI_CHID id_extract;   ///< vi channel id extract mode for 2CH_MUX or 4CH_MUX
		HD_VIDEOCAP_VI_LATCH_EDGE latch_edge;    ///< vi data latch edge for 2CH_DualEdge or 4CH_MUX_DualEdge
		UINT32  reserved[4];              ///< reserved
	} global;

	/* VI VPort Parameter */
	struct {
		HD_VIDEOCAP_VI_VPORT_DATA_SWAP data_swap;     ///< vport data swap control
		UINT32 clk_pin;       ///< vport pixel clock pin source
		UINT32 clk_inv;       ///< vport pixel clock invertion, 0:disable 1:enable
		UINT32 clk_dly;       ///< vport pixel clock delay
		UINT32 clk_pdly;      ///< vport pixel clock polarity delay
		UINT32 reserved[4];   ///< reserved
	} vport[HD_VIDEOCAP_VI_VPORT_MAX];                     ///< [0]=>VI#-P0, [1]=>VI#-P1
} HD_VIDEOCAP_VI;

typedef struct _HD_VIDEOCAP_VI_VPORT {
	UINT8 chip;              ///< chip  index
	UINT8 vcap;              ///< vcap  index
	UINT8 vi;                ///< vi    index
	UINT8 vport;             ///< vport index

	UINT32 pid;              ///< vport parameter index, value as HD_VIDEOCAP_VI_VPORT_PARAM_ID
	UINT32 value;            ///< vport parameter get/set value
	UINT32 reserved[4];
} HD_VIDEOCAP_VI_VPORT;

typedef struct _HD_VIDEOCAP_VI_CH_PARAM {
	UINT8 chip;              ///< chip index
	UINT8 vcap;              ///< vcap index
	UINT8 vi;                ///< vi   index
	UINT8 ch;                ///< ch   index

	UINT32 pid;              ///< vi channel parameter index, value as HD_VIDEOCAP_VI_CH_PARAM_ID
	UINT32 value;            ///< vi channel parameter get/set value
	UINT32 reserved[4];
} HD_VIDEOCAP_VI_CH_PARAM;

typedef struct _HD_VIDEOCAP_VI_CH_NORM {
	UINT8 chip;               ///< chip index
	UINT8 vcap;               ///< vcap index
	UINT8 vi;                 ///< vi   index
	UINT8 ch;                 ///< ch   index

	UINT32 cap_width;         ///< channel capture width
	UINT32 cap_height;        ///< channel capture height
	UINT32 org_width;         ///< channel original width,  video source width
	UINT32 org_height;        ///< channel original height, video source height
	UINT32 fps;               ///< channel frame rate, must > 0
	HD_VIDEOCAP_VI_CH_FMT            format;             ///< channel format
	HD_VIDEOCAP_VI_CH_PROG           prog;               ///< channel progressive/interlace
	HD_VIDEOCAP_VI_CH_DATA_RATE      data_rate;          ///< channel data rate, for specify byte duplicate mode
	HD_VIDEOCAP_VI_CH_DATA_LATCH     data_latch;         ///< channel data latch mode
	HD_VIDEOCAP_VI_CH_HORIZ_DUP      horiz_dup;          ///< channel horizontal pixel duplicate mode
	UINT32                    reserved[8];        ///< reserved
} HD_VIDEOCAP_VI_CH_NORM;

typedef enum _HD_VIDEOCAP_DRV_PARAM_ID {
	HD_VIDEOCAP_DRV_PARAM_INIT_HOST,          ///<  support set, vcap host init, to specify vcap system vi usage and prepare requirement memory, using HD_VIDEOCAP_HOST struct
	HD_VIDEOCAP_DRV_PARAM_UNINIT_HOST,        ///<  support set, uninit vcap host to force clear setting, using HD_VIDEOCAP_HOST_ID struct
	HD_VIDEOCAP_DRV_PARAM_REGISTER_VI,        ///<  support set, VCAP VI Register, using HD_VIDEOCAP_VI struct
	HD_VIDEOCAP_DRV_PARAM_DEREGISTER_VI,      ///<  support set, deregister all vcap vi to force clear setting, using HD_VIDEOCAP_VI_ID struct
	HD_VIDEOCAP_DRV_PARAM_GET_VI,             ///<  support get, vcap vi info, using HD_VIDEOCAP_VI struct
	HD_VIDEOCAP_DRV_PARAM_VI_VPORT,           ///<  support get/set, vcap vi vport info, using HD_VIDEOCAP_VI_VPORT struct
	HD_VIDEOCAP_DRV_PARAM_VI_CH_PARAM,        ///<  support set/get HD_VIDEOCAP_VI_CH_PARAM item, using HD_VIDEOCAP_VI_CH_PARAM;
	HD_VIDEOCAP_DRV_PARAM_VI_CH_NORM,         ///<  support set/get video norm, using HD_VIDEOCAP_VI_CH_NORM struct
	ENUM_DUMMY4WORD(HD_VIDEOCAP_DRV_PARAM_ID)
} HD_VIDEOCAP_DRV_PARAM_ID;

#define HD_VIDEOCAP_SEN_SER_MAX_DATALANE 8    ///< serial sensor max data lane
#define HD_VIDEOCAP_SEN_NAME_LEN 32           ///< sensor ko name max length
#define HD_VIDEOCAP_SEN_IGNORE  0xFFFFFFFF    ///< sensor ignore information
#define HD_VIDEOCAP_SEN_PAT_GEN "PATTERN_GEN" ///< fake sensor name for using videocap pattern generation

#define HD_VIDEOCAP_PATGEN_MODE(sel, value) MAKE_UINT16_UINT16(sel, value)

#define HD_VIDEOCAP_0   (1 << 0)
#define HD_VIDEOCAP_1   (1 << 1)
#define HD_VIDEOCAP_2   (1 << 2)
#define HD_VIDEOCAP_3   (1 << 3)
#define HD_VIDEOCAP_4   (1 << 4)
#define HD_VIDEOCAP_5   (1 << 5)
#define HD_VIDEOCAP_6   (1 << 6)
#define HD_VIDEOCAP_7   (1 << 7)

#define HD_VIDEOCAP_HDR_SENSOR1   0
#define HD_VIDEOCAP_HDR_SENSOR2   1

#define HD_VIDEOCAP_SHDR_MAP(hdr_sensor, vdocap_id) MAKE_UINT16_UINT16(hdr_sensor, vdocap_id)


typedef enum _HD_VIDEOCAP_SEN_PATGEN_SEL {
	HD_VIDEOCAP_SEN_PAT_COLORBAR    = 1,    ///< Color bar, set each color width pat_gen_val, width should be 2's multiples
	HD_VIDEOCAP_SEN_PAT_RANDOM,             ///< Random, frame based
	HD_VIDEOCAP_SEN_PAT_FIXED,              ///< Fixed, set pixel value by pat_gen_val
	HD_VIDEOCAP_SEN_PAT_HINCREASE,          ///< 1D Increment, set pixel value by pat_gen_val and reset for every line head
	HD_VIDEOCAP_SEN_PAT_HVINCREASE,         ///< 2D increment, set to line number count for every line head
	ENUM_DUMMY4WORD(HD_VIDEOCAP_SEN_PATGEN_SEL)
} HD_VIDEOCAP_SEN_PATGEN_SEL;


typedef struct _HD_VIDEOCAP_SEN_PINMUX_CFG {
	UINT32 sensor_pinmux;       ///< need mapping to top.h define (PIN_SENSOR_CFG/PIN_SENSOR2_CFG/.../PIN_SENSOR8_CFG)
	UINT32 serial_if_pinmux;    ///< need mapping to top.h define (PIN_MIPI_LVDS_CFG)
	UINT32 cmd_if_pinmux;       ///< need mapping to top.h define (PIN_I2C_CFG/PIN_SIF_CFG)
} HD_VIDEOCAP_SEN_PINMUX_CFG;

typedef enum _HD_VIDEOCAP_SEN_CLANE_SEL {
	HD_VIDEOCAP_SEN_CLANE_SEL_CSI0_USE_C0,      ///< MIPI CSI0 uses HSI_CK0 as clock lane.
	HD_VIDEOCAP_SEN_CLANE_SEL_CSI0_USE_C2,      ///< MIPI CSI0 uses HSI_CK2 as clock lane.
	HD_VIDEOCAP_SEN_CLANE_SEL_CSI1_USE_C4,      ///< MIPI CSI1 uses HSI_CK4 as clock lane.
	HD_VIDEOCAP_SEN_CLANE_SEL_CSI1_USE_C6,      ///< MIPI CSI1 uses HSI_CK6 as clock lane.
	HD_VIDEOCAP_SEN_CLANE_SEL_CSI2_USE_C2,      ///< MIPI CSI2 uses HSI_CK2 as clock lane.
	HD_VIDEOCAP_SEN_CLANE_SEL_CSI3_USE_C6,      ///< MIPI CSI3 uses HSI_CK6 as clock lane.
	HD_VIDEOCAP_SEN_CLANE_SEL_CSI4_USE_C1,      ///< MIPI CSI4 uses HSI_CK1 as clock lane.
	HD_VIDEOCAP_SEN_CLANE_SEL_CSI5_USE_C3,      ///< MIPI CSI5 uses HSI_CK3 as clock lane.
	HD_VIDEOCAP_SEN_CLANE_SEL_CSI6_USE_C5,      ///< MIPI CSI6 uses HSI_CK5 as clock lane.
	HD_VIDEOCAP_SEN_CLANE_SEL_CSI7_USE_C7,      ///< MIPI CSI7 uses HSI_CK7 as clock lane.

	HD_VIDEOCAP_SEN_CLANE_SEL_LVDS0_USE_C0C4,   ///< LVDS0 uses HSI_CK0 as clock lane in one clock lane sensor.
												///< LVDS0 uses HSI_CK0+CK4 as clock lanes in two clock lanes sensor.(such as HiSPi 8 lanes)
	HD_VIDEOCAP_SEN_CLANE_SEL_LVDS0_USE_C2C6,   ///< LVDS0 uses HSI_CK2 as clock lane in one clock lane sensor.
												///< LVDS0 uses HSI_CK2+CK6 as clock lanes in two clock lanes sensor.(such as HiSPi 8 lanes)
	HD_VIDEOCAP_SEN_CLANE_SEL_LVDS1_USE_C4,     ///< LVDS1 uses HSI_CK4 as clock lane.
	HD_VIDEOCAP_SEN_CLANE_SEL_LVDS1_USE_C6,     ///< LVDS1 uses HSI_CK6 as clock lane.
	HD_VIDEOCAP_SEN_CLANE_SEL_LVDS2_USE_C2,     ///< LVDS2 uses HSI_CK2 as clock lane.
	HD_VIDEOCAP_SEN_CLANE_SEL_LVDS3_USE_C6,     ///< LVDS3 uses HSI_CK6 as clock lane.
	HD_VIDEOCAP_SEN_CLANE_SEL_LVDS4_USE_C1,     ///< LVDS4 uses HSI_CK1 as clock lane.
	HD_VIDEOCAP_SEN_CLANE_SEL_LVDS5_USE_C3,     ///< LVDS5 uses HSI_CK3 as clock lane.
	HD_VIDEOCAP_SEN_CLANE_SEL_LVDS6_USE_C5,     ///< LVDS6 uses HSI_CK5 as clock lane.
	HD_VIDEOCAP_SEN_CLANE_SEL_LVDS7_USE_C7,     ///< LVDS7 uses HSI_CK7 as clock lane.

	HD_VIDEOCAP_SEN_CLANE_SEL_CSI1_USE_C1,      ///< MIPI CSI1 uses HSI_CK1 as clock lane.
	HD_VIDEOCAP_SEN_CLANE_SEL_LVDS1_USE_C1,     ///< LVDS1 uses HSI_CK1 as clock lane in one clock lane sensor.
	ENUM_DUMMY4WORD(HD_VIDEOCAP_SEN_CLANE_SEL)
} HD_VIDEOCAP_SEN_CLANE_SEL;

typedef struct _HD_VIDEOCAP_SEN_INIT_PIN_CFG {
	HD_VIDEOCAP_SEN_PINMUX_CFG pinmux;                             ///< set gpio for function
	HD_VIDEOCAP_SEN_CLANE_SEL clk_lane_sel;                        ///< only LVDS and MIPI need to set
	UINT32 sen_2_serial_pin_map[HD_VIDEOCAP_SEN_SER_MAX_DATALANE]; ///< Serial (lvds/csi/slvsec) pin 2 hw map
	BOOL ccir_msblsb_switch;                                       ///< only for ccir sensor output HD mode (16 bits)
	BOOL ccir_vd_hd_pin;                                           ///< CCIR601 need HW VD/HD output pin
	BOOL vx1_tx241_cko_pin;                                        ///< ENABLE / DISABLE the THCV241's CKO pin output. [limit]: For MIPI-CSI-HDR-Sensor/Tx-THCV241 only.
	BOOL vx1_tx241_cfg_2lane_mode;                                 ///< set FALSE for 1 lane mode, set TRUE for 2 lanes mode. Only KDRV_SSENIFVX1_TXTYPE_THCV241 has 2 lanes mode.
} HD_VIDEOCAP_SEN_INIT_PIN_CFG;

typedef enum _HD_VIDEOCAP_SEN_VX1_IF_SEL{
	HD_VIDEOCAP_SEN_VX1_IF_SEL_0,   ///< vx1 interface 0, support parallel sensor linear mode and mipi sensor linear mode
	HD_VIDEOCAP_SEN_VX1_IF_SEL_1,   ///< vx1 interface 1, support mipi sensor linear mode and hdr mode
	ENUM_DUMMY4WORD(HD_VIDEOCAP_SEN_VX1_IF_SEL)
} HD_VIDEOCAP_SEN_VX1_IF_SEL;

typedef enum _HD_VIDEOCAP_SEN_VX1_CTL_SEL{
	HD_VIDEOCAP_SEN_VX1_CTL_0,   ///< vx1 controller 0, mapping to KDRV_SSENIF_ENGINE_VX1_0, NT96680 only support VIDEOCAP 0~4
	HD_VIDEOCAP_SEN_VX1_CTL_1,   ///< vx1 controller 1, mapping to KDRV_SSENIF_ENGINE_VX1_1, NT96680 only support VIDEOCAP 4,6
	ENUM_DUMMY4WORD(HD_VIDEOCAP_SEN_VX1_CTL_SEL)
} HD_VIDEOCAP_SEN_VX1_CTL_SEL;

/*
    mapping to KDRV_SSENIFVX1_TXTYPE_THCV235/KDRV_SSENIFVX1_TXTYPE_THCV231/KDRV_SSENIFVX1_TXTYPE_THCV241
*/
typedef enum _HD_VIDEOCAP_SEN_VX1_TXTYPE{
	HD_VIDEOCAP_SEN_VX1_TXTYPE_THCV235, ///< Vx1 Transmitter IC is THCV235.
										///< This only support Vx1 One-lane none-HDR parallel sensor.
	HD_VIDEOCAP_SEN_VX1_TXTYPE_THCV231, ///< Vx1 Transmitter IC is THCV231.
										///< This only support Vx1 One-lane none-HDR parallel sensor.

	HD_VIDEOCAP_SEN_VX1_TXTYPE_THCV241, ///< Vx1 Transmitter IC is THCV241. This is MIPI CSI input interface Vx1 Tx.
										///< Which can support 1 or 2 lanes Vx1 interface. HDR MIPI Sensor such as SONY_LI or OV/ON-SEMI is also supported.
										///< The maximum MIPI Sensor support spec is 4 lanes mipi with 1Gbps per lane by using Vx1 2 lanes.
										///< The maximum MIPI Sensor support spec is 4 lanes mipi with 500Mbps per lane by using Vx1 1 lanes.
	ENUM_DUMMY4WORD(HD_VIDEOCAP_SEN_VX1_TXTYPE)
} HD_VIDEOCAP_SEN_VX1_TXTYPE;

typedef struct _HD_VIDEOCAP_SEN_IF_VX1{
	BOOL en;
	HD_VIDEOCAP_SEN_VX1_IF_SEL  if_sel;     ///< interface select
	HD_VIDEOCAP_SEN_VX1_CTL_SEL ctl_sel;    ///< controller select
	HD_VIDEOCAP_SEN_VX1_TXTYPE tx_type;     ///< tx module select
} HD_VIDEOCAP_SEN_IF_VX1;

/**
    VIDEOCAP 0/2 VD/HD signal source (only in parallel mode + slave sensor)
*/
typedef enum _HD_VIDEOCAP_SEN_TGE_CH_VD_TO_VCAP {
	HD_VIDEOCAP_SEN_TGE_CH1_VD_TO_VCAP0 = 0,              // use tge ch1 to be VIDEOCAP0 VD source
	HD_VIDEOCAP_SEN_TGE_CH3_VD_TO_VCAP0,                  // use tge ch3 to be VIDEOCAP0 VD source
	HD_VIDEOCAP_SEN_TGE_CH5_VD_TO_VCAP2,                  // use tge ch5 to be VIDEOCAP2 VD source
	HD_VIDEOCAP_SEN_TGE_CH7_VD_TO_VCAP2,                  // use tge ch7 to be VIDEOCAP2 VD source
	ENUM_DUMMY4WORD(HD_VIDEOCAP_SEN_TGE_CH_VD_TO_VCAP)
} HD_VIDEOCAP_SEN_TGE_CH_VD_TO_VCAP;

typedef struct _HD_VIDEOCAP_SEN_IF_TGE{
	BOOL tge_en;                                   ///< set TRUE for used tge, slave sensor (HD_VIDEOCAP_SEN_SIGNAL_SLAVE) must be enable
	BOOL swap;                                     ///< swap CH1 & 5, or CH2 & 6, or CH3 & 7, or CH4 & 8
	HD_VIDEOCAP_SEN_TGE_CH_VD_TO_VCAP vcap_vd_src; ///< VIDEOCAP 0/2 VD/HD signal source (only in parallel mode + slave sensor)
	UINT32 vcap_sync_set;                          ///< indicate which VIDEOCAPs need to sync, bitwise setting for VIDEOCAPx, e.g. set 0x3 to sync VIDEOCAP0 and VIDEOCAP1
} HD_VIDEOCAP_SEN_IF_TGE;

typedef struct _HD_VIDEOCAP_SEN_INIT_IF_CFG {
	HD_VIDEOCAP_SEN_IF_VX1 vx1;     ///< vx1 information, only valid when HD_COMMON_VIDEO_IN_TYPE is HD_COMMON_VIDEO_IN_MIPI_VX1 or HD_COMMON_VIDEO_IN_P_RAW_VX1
	HD_VIDEOCAP_SEN_IF_TGE tge;     ///< tge information
} HD_VIDEOCAP_SEN_INIT_IF_CFG;

typedef enum _HD_VIDEOCAP_SEN_OPTION_ENABLE {
	HD_VIDEOCAP_SEN_ENABLE_MAP_IF     = 0x00000001,  ///<  enable setting for sensor map interface
	HD_VIDEOCAP_SEN_ENABLE_IF_TIMEOUT = 0x00000002,  ///<  enable setting for interface timeout
	ENUM_DUMMY4WORD(HD_VIDEOCAP_SEN_OPTION_ENABLE)
} HD_VIDEOCAP_SEN_OPTION_ENABLE;

/**
    Sensor interface mapping
*/
typedef enum _HD_VIDEOCAP_SEN_MAP_IF {
	HD_VIDEOCAP_SEN_MAP_IF_DEFAULT      = 0,         ///< default mapping
	// LVDS & MIPI
	HD_VIDEOCAP_SEN_VCAP1_MAP_IF_LVDSMIPI_1  = 101,  ///< VIDEOCAP1 map to LVDSMIPI1
	HD_VIDEOCAP_SEN_VCAP2_MAP_IF_LVDSMIPI_1  = 102,  ///< VIDEOCAP2 map to LVDSMIPI1
	HD_VIDEOCAP_SEN_VCAP3_MAP_IF_LVDSMIPI_1  = 103,  ///< VIDEOCAP3 map to LVDSMIPI1
	HD_VIDEOCAP_SEN_VCAP3_MAP_IF_LVDSMIPI_2  = 104,  ///< VIDEOCAP3 map to LVDSMIPI2
	// SLVSEC
	HD_VIDEOCAP_SEN_VCAP1_MAP_IF_SLVSEC_1    = 201,  ///< VIDEOCAP1 map to SLVSEC_1
	HD_VIDEOCAP_SEN_IF_MAP_MAX_NUM = 0xFFFF,
	ENUM_DUMMY4WORD(HD_VIDEOCAP_SEN_MAP_IF)
} HD_VIDEOCAP_SEN_MAP_IF;


typedef struct _HD_VIDEOCAP_SEN_INIT_OPTION {
	HD_VIDEOCAP_SEN_OPTION_ENABLE en_mask;   ///< enable mask, bitwise, for the following settings
	HD_VIDEOCAP_SEN_MAP_IF sen_map_if;       ///< sensor map interface, default: HD_VIDEOCAP_SEN_MAP_IF_DEFAULT
	UINT32 if_time_out;                      ///< ms, default: 1000ms, only support LVDS/MIPI/SLVS-EC
} HD_VIDEOCAP_SEN_INIT_OPTION;


typedef struct _HD_VIDEOCAP_SENSOR_DEVICE {
	CHAR driver_name[HD_VIDEOCAP_SEN_NAME_LEN]; ///< sensor ko name
	HD_COMMON_VIDEO_IN_TYPE if_type;            ///< interface type
	HD_VIDEOCAP_SEN_INIT_IF_CFG if_cfg;         ///< interface configuration
	HD_VIDEOCAP_SEN_INIT_PIN_CFG pin_cfg;       ///< pin configuration
	HD_VIDEOCAP_SEN_INIT_OPTION  option;        ///< option setting for sensor driver
} HD_VIDEOCAP_SENSOR_DEVICE;

/**
     Obselete enum.
	 Do NOT use this anymore.
*/
typedef enum HD_VIDEOCAP_SEN_HDR_MAP {
	HD_VIDEOCAP_SEN_HDR_NONE        = 0,     ///< for not using sensor HDR
	HD_VIDEOCAP_SEN_HDR_SET1_MAIN,           ///< indicate this videocapture connect to the main path of sensor HDR set1
	HD_VIDEOCAP_SEN_HDR_SET1_SUB1,           ///< indicate this videocapture connect to the sub path 1 of sensor HDR set1
	HD_VIDEOCAP_SEN_HDR_SET1_SUB2,           ///< indicate this videocapture connect to the sub path 2 of sensor HDR set1
	HD_VIDEOCAP_SEN_HDR_SET1_SUB3,           ///< indicate this videocapture connect to the sub path 3 of sensor HDR set1
	HD_VIDEOCAP_SEN_HDR_SET2_MAIN,           ///< indicate this videocapture connect to the main path of sensor HDR set2
	HD_VIDEOCAP_SEN_HDR_SET2_SUB1,           ///< indicate this videocapture connect to the sub path 1 of sensor HDR set2
	ENUM_DUMMY4WORD(HD_VIDEOCAP_SEN_HDR_MAP)
} HD_VIDEOCAP_SEN_HDR_MAP;

typedef struct _HD_VIDEOCAP_SEN_CONFIG {
	HD_VIDEOCAP_SENSOR_DEVICE sen_dev;  ///< sensor device setting
	UINT32 shdr_map;                    ///< SHDR mode only, indicate which VIDEOCAPs use the same HDR sensor,
										///< e.g. HD_VIDEOCAP_SHDR_MAP(HD_VIDEOCAP_HDR_SENSOR1, (HD_VIDEOCAP_0|HD_VIDEOCAP_2)) for HDR sensor1 using VIDEOCAP0/2
										///< e.g. HD_VIDEOCAP_SHDR_MAP(HD_VIDEOCAP_HDR_SENSOR2, (HD_VIDEOCAP_1|HD_VIDEOCAP_3)) for HDR sensor2 using VIDEOCAP1/3
} HD_VIDEOCAP_SEN_CONFIG;

typedef struct _HD_VIDEOCAP_CONFIG {
	HD_VIDEOCAP_SEN_CONFIG  sen_cfg;  ///< sensor config
} HD_VIDEOCAP_DRV_CONFIG;
/********************************************************************
	TYPE DEFINITION
********************************************************************/

/**
     @name option of ctrl function (whole device)
*/
typedef enum _HD_VIDEOCAP_CTRLFUNC {
	HD_VIDEOCAP_FUNC_AE   = 0x00000100, ///< enable AE
	HD_VIDEOCAP_FUNC_AWB  = 0x00000200, ///< enable AWB
	HD_VIDEOCAP_FUNC_AF   = 0x00000400, ///< enable AF
	HD_VIDEOCAP_FUNC_WDR  = 0x00000800, ///< enable WDR effect (single frame)
	HD_VIDEOCAP_FUNC_SHDR = 0x00001000, ///< enable Sensor HDR effect (multi frame)
	HD_VIDEOCAP_FUNC_ETH  = 0x00002000, ///< enable ETH
	ENUM_DUMMY4WORD(HD_VIDEOCAP_CTRLFUNC)
} HD_VIDEOCAP_CTRLFUNC;

/**
     @name ctrl function (whole device)
*/
typedef struct _HD_VIDEOCAP_CTRL {
	HD_VIDEOCAP_CTRLFUNC func;    ///< additional function of ctrl path (whole device) (bit-wise mask)
} HD_VIDEOCAP_CTRL;


typedef enum _HD_VIDEOCAP_NOTIFY {
	HD_VIDEOCAP_NOTIFY_SIGNAL_LOSS,        ///< signal loss
	HD_VIDEOCAP_NOTIFY_SIGNAL_PRESENT,     ///< signal present
	HD_VIDEOCAP_NOTIFY_FRAMERATE_CHANGE,   ///< frame rate changed
	HD_VIDEOCAP_NOTIFY_HW_CONFIG_CHANGE,   ///< HW configuration changed
	HD_VIDEOCAP_NOTIFY_MAX,
	ENUM_DUMMY4WORD(HD_VIDEOCAP_NOTIFY)
} HD_VIDEOCAP_NOTIFY;

/*!
 * @fn int HD_VIDEOCAP_NOTIFY_HANDLER(HD_VIDEOCAP_NOTIFY notify)
 * @brief callback function for notify
 * @param notify the type of this notification
 * @param data the private data from registered function
 * @return return none
 */
typedef void (*HD_VIDEOCAP_NOTIFY_HANDLER)(HD_DAL did, HD_VIDEOCAP_NOTIFY notify, void *data);

#define HD_VIDEOCAP_SEN_MODE_AUTO   0xFFFFFFFF

typedef enum _HD_VIDEOCAP_SEN_FRAME_NUM {
	HD_VIDEOCAP_SEN_FRAME_NUM_1 = 1,      ///< linear mode
	HD_VIDEOCAP_SEN_FRAME_NUM_2 = 2,      ///< SHDR 2 frame
	HD_VIDEOCAP_SEN_FRAME_NUM_3 = 3,      ///< SHDR 3 frame
	HD_VIDEOCAP_SEN_FRAME_NUM_4 = 4,      ///< SHDR 4 frame
	ENUM_DUMMY4WORD(HD_VIDEOCAP_SEN_FRAME_NUM)
} HD_VIDEOCAP_SEN_FRAME_NUM;


typedef struct _HD_VIDEOCAP_IN {
	UINT32 sen_mode;                         ///<  referring to sensor driver, or set HD_VIDEOCAP_SEN_MODE_AUTO for AUTO selecting
	HD_VIDEO_FRC frc;                        ///<  frame rate
	HD_DIM dim;                              ///<  dim w,h. only valid when sen_mode is HD_VIDEOCAP_SEN_MODE_AUTO
	HD_VIDEO_PXLFMT pxlfmt;                  ///<  pixel format. only valid when sen_mode is HD_VIDEOCAP_SEN_MODE_AUTO
	HD_VIDEOCAP_SEN_FRAME_NUM out_frame_num; ///<  sensor output frame number, 1 for linear mode and 2/3/4 for sensor HDR mode.
} HD_VIDEOCAP_IN;


typedef struct _HD_VIDEOCAP_OUT {
	HD_DIM dim;                         ///< output dim w,h
	HD_VIDEO_PXLFMT pxlfmt;             ///< output pixel format.
	HD_VIDEO_DIR dir;                   ///< output direction, like mirror/flip
	HD_VIDEO_FRC frc;                   ///< output frame-control or frame rate
	UINT32 depth;                       ///< output queue depth, set larger than 0 to allow pull_out
} HD_VIDEOCAP_OUT;

typedef struct _HD_VIDEOCAP_CROP_AUTO_INFO {
	UINT32 ratio_w_h;               ///< ratio of width and height (W:bit[31:16], H:bit[15:0]), referring to HD_VIDEOCAP_RATIO
	UINT32 factor;                  ///< max 1000, for no digital zoom effect (w = w * factor / 1000, h = h * factor / 1000)
	HD_IPOINT sft;                  ///< based on active window center, unit: pixel
} HD_VIDEOCAP_CROP_AUTO_INFO;

typedef struct _HD_VIDEOCAP_CROP {
	HD_CROP_MODE mode;                        ///< cropping mode, referring to #_HD_CROP_MODE
	HD_VIDEO_CROP win;                        ///< set region while the cropping mode is HD_CROP_ON, referring to #_HD_VIDEO_CROP
	HD_VIDEOCAP_CROP_AUTO_INFO auto_info;     ///< only valid while the cropping mode is HD_CROP_AUTO, referring to #_HD_VIDEOCAP_CROP_AUTO_INFO
	HD_DIM align;                             ///< alignment, must be a multiple of 4
} HD_VIDEOCAP_CROP;

/**
     @name capability of device (extend from common HD_DEVICE_CAPS)
*/
typedef enum _HD_VIDEOCAP_DEVCAPS {
	HD_VIDEOCAP_DEVCAPS_AE   = 0x00000100, ///< caps of HD_VIDEOCAP_FUNC_AE
	HD_VIDEOCAP_DEVCAPS_AWB  = 0x00000200, ///< caps of HD_VIDEOCAP_FUNC_AWB
	HD_VIDEOCAP_DEVCAPS_AF   = 0x00000400, ///< caps of HD_VIDEOCAP_FUNC_AF
	HD_VIDEOCAP_DEVCAPS_WDR  = 0x00000800, ///< caps of HD_VIDEOCAP_FUNC_WDR
	HD_VIDEOCAP_DEVCAPS_SHDR = 0x00001000, ///< caps of HD_VIDEOCAP_FUNC_SHDR
	HD_VIDEOCAP_DEVCAPS_ETH  = 0x00002000, ///< caps of HD_VIDEOCAP_FUNC_ETH
	ENUM_DUMMY4WORD(HD_VIDEOCAP_DEVCAPS)
} HD_VIDEOCAP_DEVCAPS;


/**
     @name capability of input (extend from common HD_VIDEO_CAPS)
*/
typedef enum HD_VIDEOCAP_INCAPS {
	HD_VIDEOCAP_INCAPS_SEN_MODE =           0x01000000, ///< caps of HD_VIDEOCAP_IN.sen_mode
	ENUM_DUMMY4WORD(HD_VIDEOCAP_INCAPS)
} HD_VIDEOCAP_INCAPS;

/**
     @name capability of output (extend from common HD_VIDEO_CAPS)
*/
typedef enum HD_VIDEOCAP_OUTCAPS {
	HD_VIDEOCAP_OUTCAPS_NOTIFY_HANDLER =       0x01000000, ///< caps of HD_VIDEOCAP_NOTIFY_HANDLER
	HD_VIDEOCAP_OUTCAPS_MD_STATUS      =       0x02000000, ///< caps of HD_VIDEOCAP_MD_STATUS
	HD_VIDEOCAP_OUTCAPS_DIRECT         =       0x04000000, ///< caps of HD_VIDEOCAP_OUTFUNC_DIRECT
	HD_VIDEOCAP_OUTCAPS_ONEBUF         =       0x08000000, ///< caps of HD_VIDEOCAP_OUTFUNC_ONEBUF
	ENUM_DUMMY4WORD(HD_VIDEOCAP_OUTCAPS)
} HD_VIDEOCAP_OUTCAPS;



/**
     @name system capability
*/
typedef struct _HD_VIDEOCAP_SYSCAPS {
	HD_DAL dev_id;                              ///< device id
	UINT32 chip_id;                             ///< chip id of this device
	UINT32 max_in_count;                        ///< max count of input of this device (supported)
	UINT32 max_out_count;                       ///< max count of output of this device (supported)
	HD_DEVICE_CAPS dev_caps;                    ///< capability of device, combine caps of HD_DEVICE_CAPS and HD_VIDEOCAP_DEVCAPS
	HD_VIDEO_CAPS in_caps[HD_VIDEOCAP_MAX_IN];  ///< capability of input, combine caps of HD_VIDEO_CAPS and HD_VIDEOCAP_INCAPS
	HD_VIDEO_CAPS out_caps[HD_VIDEOCAP_MAX_OUT];///< capability of output, combine caps of HD_VIDEO_CAPS and HD_VIDEOCAP_OUTCAPS
	HD_DIM max_dim;                           ///< max dimension width/height
	UINT32 max_frame_rate;                      ///< max frame rate
	UINT32 max_w_scaleup;                       ///< max scaling up ratio
	HD_VIDEO_PXLFMT pxlfmt;                     ///< pixel format
} HD_VIDEOCAP_SYSCAPS;


/**
     @name system information
*/
typedef struct _HD_VIDEOCAP_SYSINFO {
	HD_DAL dev_id;                       ///< device id
	UINT32 cur_fps[HD_VIDEOCAP_MAX_OUT]; ///< current fps
	UINT64 vd_count;                     ///< VD count.
	BOOL   output_started;               ///< indicate output path started or not
	HD_DIM cur_dim;                      ///< indicate the output size of current sensor mode
} HD_VIDEOCAP_SYSINFO;

typedef struct _HD_VIDEOCAP_MD_STATUS {
	BOOL  enabled;              ///< Enable/Disable output motion meta data
} HD_VIDEOCAP_MD_STATUS;

typedef enum _HD_VIDEOCAP_POOL_MODE {
	HD_VIDEOCAP_POOL_AUTO = 0,
	HD_VIDEOCAP_POOL_ENABLE = 1,
	HD_VIDEOCAP_POOL_DISABLE = 2,
	ENUM_DUMMY4WORD(HD_VIDEOCAP_POOL_MODE),
} HD_VIDEOCAP_POOL_MODE;

typedef struct _HD_VIDEOCAP_POOL {
	INT		ddr_id;				///< DDR ID
	HD_VIDEO_PXLFMT	pxlfmt;		///< pixel format of buffer
	HD_DIM	dim;				///< dim w,h of buffer
	UINT32	counts;				///< count of buffer, use HD_VIDEOCAP_SET_COUNT to set
	UINT32	max_counts;			///< max counts of buffer, use HD_VIDEOCAP_SET_COUNT to set
	UINT32	min_counts;			///< min counts of buffer, use HD_VIDEOCAP_SET_COUNT to set
	INT	mode;					///< pool mode, 0: auto, 1:enable, 2:disable
} HD_VIDEOCAP_POOL;

typedef struct _HD_VIDEOCAP_PATH_CONFIG {
	HD_VIDEOCAP_POOL data_pool[HD_VIDEOCAP_MAX_DATA_TYPE]; ///< pool memory information
} HD_VIDEOCAP_PATH_CONFIG;

typedef enum _HD_VIDEOCAP_OUTFUNC {
	HD_VIDEOCAP_OUTFUNC_DIRECT = 0x00000001, ///< enable ouput direct to vdoenc (zero-buffer)
	HD_VIDEOCAP_OUTFUNC_ONEBUF = 0x00000002, ///< enable one-buffer to vdoprc
	ENUM_DUMMY4WORD(HD_VIDEOCAP_OUTFUNC)
} HD_VIDEOCAP_OUTFUNC;

typedef struct _HD_VIDEOCAP_FUNC_CONFIG {
	UINT32 ddr_id;                      ///< DDR ID
	HD_VIDEOCAP_OUTFUNC out_func;       ///< additional function of out (bit-wise mask)
} HD_VIDEOCAP_FUNC_CONFIG;

typedef enum _HD_VIDEOCAP_PARAM_ID{
	HD_VIDEOCAP_PARAM_DEVCOUNT,          ///< support get with ctrl path, using HD_DEVCOUNT struct
	HD_VIDEOCAP_PARAM_SYSCAPS,           ///< support get with ctrl path, using HD_VIDEOCAP_SYSCAPS struct
	HD_VIDEOCAP_PARAM_SYSINFO,           ///< support get with ctrl path, using HD_VIDEOCAP_SYSINFO struct
	HD_VIDEOCAP_PARAM_DRV_CONFIG,        ///< support set with ctrl path, using HD_VIDEOCAP_DRV_CONFIG struct
	HD_VIDEOCAP_PARAM_PATH_CONFIG,       ///< support get/set with i/o path, using HD_VIDEOCAP_PATH_CONFIG struct
	HD_VIDEOCAP_PARAM_FUNC_CONFIG,       ///< support get/set with i/o path, using HD_VIDEOCAP_FUNC_CONFIG struct
	HD_VIDEOCAP_PARAM_IN_CROP,           ///< support get/set with i/o path, using HD_VIDEOCAP_CROP struct
	HD_VIDEOCAP_PARAM_OUT_CROP,          ///< obselete item, just the same with HD_VIDEOCAP_PARAM_IN_CROP
	HD_VIDEOCAP_PARAM_OUT,               ///< support get/set with i/o path, using HD_VIDEOCAP_OUT struct
	HD_VIDEOCAP_PARAM_OUT_NOTIFY_HANDLER,///< support get/set with i/o path, using HD_VIDEOCAP_NOTIFY_HANDLER
	HD_VIDEOCAP_PARAM_OUT_MD_STATUS,     ///< support get/set with i/o path, using HD_VIDEOCAP_MD_STATUS struct
	HD_VIDEOCAP_PARAM_CTRL,              ///< support get/set with ctrl path, using HD_VIDEOCAP_CTRL struct (effect of whole device)
	HD_VIDEOCAP_PARAM_IN,                ///< support get/set with i/o path, using HD_VIDEOCAP_IN struct
	HD_VIDEOCAP_PARAM_OUT_MASK_ATTR,     ///< support get/set with o/mask path, using HD_OSG_MASK_ATTR struct (mask display attribute)
	HD_VIDEOCAP_PARAM_IN_PALETTE_TABLE,  ///< support get/set with i path, using HD_OSG_PALETTE_TBL struct
	HD_VIDEOCAP_PARAM_MAX,
	ENUM_DUMMY4WORD(HD_VIDEOCAP_PARAM_ID)
} HD_VIDEOCAP_PARAM_ID;

/********************************************************************
    EXTERN VARIABLES & FUNCTION PROTOTYPES DECLARATIONS
********************************************************************/

HD_RESULT hd_videocap_init(VOID);
HD_RESULT hd_videocap_bind(HD_OUT_ID out_id, HD_IN_ID dest_in_id);
HD_RESULT hd_videocap_unbind(HD_OUT_ID out_id);
HD_RESULT hd_videocap_open(HD_IN_ID in_id, HD_OUT_ID out_id, HD_PATH_ID *p_path_id);

HD_RESULT hd_videocap_start(HD_PATH_ID path_id);
HD_RESULT hd_videocap_stop(HD_PATH_ID path_id);
HD_RESULT hd_videocap_start_list(HD_PATH_ID *path_id, UINT num);
HD_RESULT hd_videocap_stop_list(HD_PATH_ID *path_id, UINT num);

HD_RESULT hd_videocap_get(HD_PATH_ID path_id, HD_VIDEOCAP_PARAM_ID id, VOID *p_param);
HD_RESULT hd_videocap_set(HD_PATH_ID path_id, HD_VIDEOCAP_PARAM_ID id, VOID *p_param);

HD_RESULT hd_videocap_drv_get(HD_VIDEOCAP_DRV_PARAM_ID id, VOID *p_param);
HD_RESULT hd_videocap_drv_set(HD_VIDEOCAP_DRV_PARAM_ID id, VOID *p_param);

HD_RESULT hd_videocap_push_in_buf(HD_PATH_ID path_id, HD_VIDEO_FRAME *p_video_frame, INT32 wait_ms);
HD_RESULT hd_videocap_pull_out_buf(HD_PATH_ID path_id, HD_VIDEO_FRAME *p_video_frame, INT32 wait_ms);
HD_RESULT hd_videocap_release_out_buf(HD_PATH_ID path_id, HD_VIDEO_FRAME *p_video_frame);

HD_RESULT hd_videocap_close(HD_PATH_ID path_id);
HD_RESULT hd_videocap_uninit(VOID);

#endif

