/*
    Display HDMI device object

    @file       dispdev_hdmi.c
    @ingroup    mIDISPHDMITx
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/

#include "hdmi_port.h"
#include "hdmitx.h"
#include "nvt_hdmi.h"
#include "dispdev_hdmi.h"
#include "dispdev_hdmi_if.h"

/* macros */
#define DEFAULT_VID		HDMI_1920X1080P60

/* Externs
 */
extern void hdmi_isr(void);

/* local functions declaration
 */
static ER               dispdev_open_hdmi(void);
static ER               dispdev_close_hdmi(void);
static ER 				dispdev_preconfighdmi(PDISP_HDMI_CFG config);
static UINT32 			dispdev_read_hdmi_edid(void);
static void             dispdev_set_hdmi_ioctrl(FP p_io_ctrl_func);
#define dispdev_isr	 hdmi_isr


static DISPDEV_OBJ dispdev_hdmi_obj = {dispdev_open_hdmi, dispdev_close_hdmi, dispdev_read_hdmi_edid, dispdev_preconfighdmi, dispdev_set_hdmi_ioctrl, dispdev_isr};

/* local variables declaration
 */
#define     VIDMAX      (59+19)
const static DISPDEV_HDMI_TIMING  disp_video_timing[VIDMAX] = {
	/* Video Format timing structure defined in CEA-861-D */
	/*           VID                 HS       VS       HVLD      VVLD      CLK       DE  PROGRESSIVE HSYNC  HTOT      HSTART       HEND            VSYNC    VTOT       VOSTART     VOEND       VESTART       VEEND      WINX,   WINY */
	/* 1*/{HDMI_640X480P60,       ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     96, (160 + 640), (96 + 48), (96 + 48 + 640),        2, (480 + 45), (2 + 33), (2 + 33 + 480), (2 + 33), (2 + 33 + 480),   16,     12   },
	/* 2*/{HDMI_720X480P60,       ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     62, (138 + 720), (62 + 60), (62 + 60 + 720),        6, (480 + 45), (6 + 30), (6 + 30 + 480), (6 + 30), (6 + 30 + 480),   20,     12   },
	/* 3*/{HDMI_720X480P60_16X9,  ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     62, (138 + 720), (62 + 60), (62 + 60 + 720),        6, (480 + 45), (6 + 30), (6 + 30 + 480), (6 + 30), (6 + 30 + 480),   20,     12   },
	/* 4*/{HDMI_1280X720P60,      ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     40, (370 + 1280), (40 + 220), (40 + 220 + 1280),      5, (720 + 30), (5 + 20), (5 + 20 + 720), (5 + 20), (5 + 20 + 720),   30,     20   },
	/* 5*/{HDMI_1920X1080I60,     ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, FALSE,    44, (280 + 1920), (44 + 148), (44 + 148 + 1920),     10, (1080 + 45), (10 + 15 * 2), (10 + 30 + 1080), (10 + 30 + 1), (10 + 30 + 1080 + 1),   45,     30   },
	/* 6*/{HDMI_720X480I60,       ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, FALSE, 124 / 2, (276 + 1440) / 2, (124 + 114) / 2, (124 + 114 + 1440) / 2, 6, (480 + 45), (6 + 15 * 2), (6 + 30 + 480), (6 + 15 * 2 + 1), (6 + 30 + 480 + 1),   16,     12   },
	/* 7*/{HDMI_720X480I60_16X9,  ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, FALSE, 124 / 2, (276 + 1440) / 2, (124 + 114) / 2, (124 + 114 + 1440) / 2, 6, (480 + 45), (6 + 15 * 2), (6 + 30 + 480), (6 + 15 * 2 + 1), (6 + 30 + 480 + 1),   16,     12   },
	/* 8*/{HDMI_720X240P60,       ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,  124 / 2, (276 + 1440) / 2, (124 + 114) / 2, (124 + 114 + 1440) / 2, 3, (240 + 22), (3 + 15), (3 + 15 + 240), (3 + 15), (3 + 15 + 240),   16,      6   },
	/* 9*/{HDMI_720X240P60_16X9,  ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,  124 / 2, (276 + 1440) / 2, (124 + 114) / 2, (124 + 114 + 1440) / 2, 3, (240 + 22), (3 + 15), (3 + 15 + 240), (3 + 15), (3 + 15 + 240),   16,      6   },
	/*10*/{HDMI_1440X480I60,      ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, FALSE, 248 / 2, (552 + 2880) / 2, (248 + 228) / 2, (248 + 228 + 2880) / 2, 6, (480 + 45), (6 + 30), (6 + 30 + 480), (6 + 30 + 1), (6 + 30 + 480 + 1),   32,     12   },
	/*11*/{HDMI_1440X480I60_16X9, ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, FALSE, 248 / 2, (552 + 2880) / 2, (248 + 228) / 2, (248 + 228 + 2880) / 2, 6, (480 + 45), (6 + 30), (6 + 30 + 480), (6 + 30 + 1), (6 + 30 + 480 + 1),   32,     12   },
	/*12*/{HDMI_1440X240P60,      ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,  248 / 2, (552 + 2880) / 2, (248 + 228) / 2, (248 + 228 + 2880) / 2, 3, (240 + 22), (3 + 15), (3 + 15 + 240), (3 + 15), (3 + 15 + 240),   32,      6   },
	/*13*/{HDMI_1440X240P60_16X9, ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,  248 / 2, (552 + 2880) / 2, (248 + 228) / 2, (248 + 228 + 2880) / 2, 3, (240 + 22), (3 + 15), (3 + 15 + 240), (3 + 15), (3 + 15 + 240),   32,      6   },
	/*14*/{HDMI_1440X480P60,      ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,    124, (276 + 1440), (124 + 120), (124 + 120 + 1440),     6, (480 + 45), (6 + 30), (6 + 30 + 480), (6 + 30), (6 + 30 + 480),   32,     12   },
	/*15*/{HDMI_1440X480P60_16X9, ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,    124, (276 + 1440), (124 + 120), (124 + 120 + 1440),     6, (480 + 45), (6 + 30), (6 + 30 + 480), (6 + 30), (6 + 30 + 480),   32,     12   },
	/*16*/{HDMI_1920X1080P60,     ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     44, (280 + 1920), (44 + 148), (44 + 148 + 1920),      5, (1080 + 45), (5 + 36), (5 + 36 + 1080), (5 + 36), (5 + 36 + 1080),   45,     30   },
	/*17*/{HDMI_720X576P50,       ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     64, (144 + 720), (64 + 68), (64 + 68 + 720),        5, (576 + 49), (5 + 39), (5 + 39 + 576), (5 + 39), (5 + 39 + 576),   20,     16   },
	/*18*/{HDMI_720X576P50_16X9,  ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     64, (144 + 720), (64 + 68), (64 + 68 + 720),        5, (576 + 49), (5 + 39), (5 + 39 + 576), (5 + 39), (5 + 39 + 576),   20,     16   },
	/*19*/{HDMI_1280X720P50,      ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     40, (700 + 1280), (40 + 220), (40 + 220 + 1280),      5, (720 + 30), (5 + 20), (5 + 20 + 720), (5 + 20), (5 + 20 + 720),   30,     20   },
	/*20*/{HDMI_1920X1080I50,     ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, FALSE,    44, (720 + 1920), (44 + 148), (44 + 148 + 1920),     10, (1080 + 45), (10 + 15 * 2), (10 + 30 + 1080), (10 + 30 + 1), (10 + 30 + 1080 + 1),   45,     30   },
	/*21*/{HDMI_720X576I50,       ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, FALSE, 126 / 2, (288 + 1440) / 2, (126 + 138) / 2, (126 + 138 + 1440) / 2, 6, (576 + 49), (6 + 19 * 2), (6 + 38 + 576), (6 + 38 + 1), (6 + 38 + 576 + 1),   16,     16   },
	/*22*/{HDMI_720X576I50_16X9,  ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, FALSE, 126 / 2, (288 + 1440) / 2, (126 + 138) / 2, (126 + 138 + 1440) / 2, 6, (576 + 49), (6 + 19 * 2), (6 + 38 + 576), (6 + 38 + 1), (6 + 38 + 576 + 1),   16,     16   },
	/* Warning: VID 23/24 's V-BP in CEA-861D is 18 lines but in Astro VA-1809 is 19 lines. */
	/*23*/{HDMI_720X288P50,       ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,  126 / 2, (288 + 1440) / 2, (126 + 138) / 2, (126 + 138 + 1440) / 2, 3, (288 + 24), (3 + 19), (3 + 19 + 288), (3 + 19), (3 + 19 + 288),   16,      8   },
	/*24*/{HDMI_720X288P50_16X9,  ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,  126 / 2, (288 + 1440) / 2, (126 + 138) / 2, (126 + 138 + 1440) / 2, 3, (288 + 24), (3 + 19), (3 + 19 + 288), (3 + 19), (3 + 19 + 288),   16,      8   },
	/*25*/{HDMI_1440X576I50,      ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, FALSE, 252 / 2, (576 + 2880) / 2, (252 + 276) / 2, (252 + 276 + 2880) / 2, 6, (576 + 49), (6 + 19 * 2), (6 + 38 + 576), (6 + 38 + 1), (6 + 38 + 576 + 1),   32,     16   },
	/*26*/{HDMI_1440X576I50_16X9, ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, FALSE, 252 / 2, (576 + 2880) / 2, (252 + 276) / 2, (252 + 276 + 2880) / 2, 6, (576 + 49), (6 + 19 * 2), (6 + 38 + 576), (6 + 38 + 1), (6 + 38 + 576 + 1),   32,     16   },
	/*27*/{HDMI_1440X288P50,      ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,  252 / 2, (576 + 2880) / 2, (252 + 276) / 2, (252 + 276 + 2880) / 2, 3, (288 + 24), (3 + 19), (3 + 19 + 288), (3 + 19), (3 + 19 + 288),   32,      8   },
	/*28*/{HDMI_1440X288P50_16X9, ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,  252 / 2, (576 + 2880) / 2, (252 + 276) / 2, (252 + 276 + 2880) / 2, 3, (288 + 24), (3 + 19), (3 + 19 + 288), (3 + 19), (3 + 19 + 288),   32,      8   },
	/*29*/{HDMI_1440X576P50,      ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,    128, (288 + 1440), (128 + 136), (128 + 136 + 1440),     5, (576 + 49), (5 + 39), (5 + 39 + 576), (5 + 39), (5 + 39 + 576),   32,     16   },
	/*30*/{HDMI_1440X576P50_16X9, ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,    128, (288 + 1440), (128 + 136), (128 + 136 + 1440),     5, (576 + 49), (5 + 39), (5 + 39 + 576), (5 + 39), (5 + 39 + 576),   32,     16   },
	/*31*/{HDMI_1920X1080P50,     ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     44, (720 + 1920), (44 + 148), (44 + 148 + 1920),      5, (1080 + 45), (5 + 36), (5 + 36 + 1080), (5 + 36), (5 + 36 + 1080),   45,     30   },
	/*32*/{HDMI_1920X1080P24,     ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     44, (830 + 1920), (44 + 148), (44 + 148 + 1920),      5, (1080 + 45), (5 + 36), (5 + 36 + 1080), (5 + 36), (5 + 36 + 1080),   45,     30   },
	/*33*/{HDMI_1920X1080P25,     ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     44, (720 + 1920), (44 + 148), (44 + 148 + 1920),      5, (1080 + 45), (5 + 36), (5 + 36 + 1080), (5 + 36), (5 + 36 + 1080),   45,     30   },
	/*34*/{HDMI_1920X1080P30,     ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     44, (280 + 1920), (44 + 148), (44 + 148 + 1920),      5, (1080 + 45), (5 + 36), (5 + 36 + 1080), (5 + 36), (5 + 36 + 1080),   45,     30   },
	//35~38 Width is 2880  IDE Un-Supported
	/*35*/{35,                    ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,  248 / 2, (552 + 2880) / 2, (248 + 240) / 2, (248 + 240 + 2880) / 2, 6, (480 + 45), (6 + 30), (6 + 30 + 480), (6 + 30), (6 + 30 + 480),   64,     12   },
	/*36*/{36,                    ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,  248 / 2, (552 + 2880) / 2, (248 + 240) / 2, (248 + 240 + 2880) / 2, 6, (480 + 45), (6 + 30), (6 + 30 + 480), (6 + 30), (6 + 30 + 480),   64,     12   },
	/*37*/{37,                    ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,  256 / 2, (576 + 2880) / 2, (256 + 272) / 2, (256 + 272 + 2880) / 2, 5, (576 + 49), (5 + 39), (5 + 39 + 576), (5 + 39), (5 + 39 + 576),   64,     16   },
	/*38*/{38,                    ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,  256 / 2, (576 + 2880) / 2, (256 + 272) / 2, (256 + 272 + 2880) / 2, 5, (576 + 49), (5 + 39), (5 + 39 + 576), (5 + 39), (5 + 39 + 576),   64,     16   },
	/*39*/{HDMI_1920X1080I50_VT1250, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, FALSE,   168, (384 + 1920), (168 + 184), (168 + 184 + 1920),    10, (1080 + 170), (10 + 57 * 2), (10 + 114 + 1080), (10 + 57 * 2 + 1), (10 + 114 + 1080 + 1), 45,     30   },
	/*40*/{HDMI_1920X1080I100,    ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, FALSE,    44, (720 + 1920), (44 + 148), (44 + 148 + 1920),     10, (1080 + 45), (10 + 15 * 2), (10 + 30 + 1080), (10 + 30 + 1), (10 + 30 + 1080 + 1),  45,     30   },
	/*41*/{HDMI_1280X720P100,     ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     40, (700 + 1280), (40 + 220), (40 + 220 + 1280),      5, (720 + 30), (5 + 20), (5 + 20 + 720), (5 + 20), (5 + 20 + 720),   32,     20   },
	/*42*/{HDMI_720X576P100,      ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     64, (144 + 720), (64 + 68), (64 + 68 + 720),        5, (576 + 49), (5 + 39), (5 + 39 + 576), (5 + 39), (5 + 39 + 576),   20,     16   },
	/*43*/{HDMI_720X576P100_16X9, ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     64, (144 + 720), (64 + 68), (64 + 68 + 720),        5, (576 + 49), (5 + 39), (5 + 39 + 576), (5 + 39), (5 + 39 + 576),   20,     16   },
	/*44*/{HDMI_720X576I100,      ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, FALSE, 126 / 2, (288 + 1440) / 2, (126 + 138) / 2, (126 + 138 + 1440) / 2, 6, (576 + 49), (6 + 19 * 2), (6 + 38 + 576), (6 + 38 + 1), (6 + 38 + 576 + 1), 16,     16   },
	/*45*/{HDMI_720X576I100_16X9, ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, FALSE, 126 / 2, (288 + 1440) / 2, (126 + 138) / 2, (126 + 138 + 1440) / 2, 6, (576 + 49), (6 + 19 * 2), (6 + 38 + 576), (6 + 38 + 1), (6 + 38 + 576 + 1), 16,     16   },
	/*46*/{HDMI_1920X1080I120,    ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, FALSE,    44, (280 + 1920), (44 + 148), (44 + 148 + 1920),     10, (1080 + 45), (10 + 15 * 2), (10 + 30 + 1080), (10 + 30 + 1), (10 + 30 + 1080 + 1),  45,     30   },
	/*47*/{HDMI_1280X720P120,     ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     40, (370 + 1280), (40 + 220), (40 + 220 + 1280),      5, (720 + 30), (5 + 20), (5 + 20 + 720), (5 + 20), (5 + 20 + 720),   30,     20   },
	/*48*/{HDMI_720X480P120,      ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     62, (138 + 720), (62 + 60), (62 + 60 + 720),        6, (480 + 45), (6 + 30), (6 + 30 + 480), (6 + 30), (6 + 30 + 480),   20,     12   },
	/*49*/{HDMI_720X480P120_16X9, ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     62, (138 + 720), (62 + 60), (62 + 60 + 720),        6, (480 + 45), (6 + 30), (6 + 30 + 480), (6 + 30), (6 + 30 + 480),   20,     12   },
	/*50*/{HDMI_720X480I120,      ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, FALSE, 124 / 2, (276 + 1440) / 2, (124 + 114) / 2, (124 + 114 + 1440) / 2, 6, (480 + 45), (6 + 30), (6 + 30 + 480), (6 + 30 + 1), (6 + 30 + 480 + 1),   16,     12   },
	/*51*/{HDMI_720X480I120_16X9, ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, FALSE, 124 / 2, (276 + 1440) / 2, (124 + 114) / 2, (124 + 114 + 1440) / 2, 6, (480 + 45), (6 + 30), (6 + 30 + 480), (6 + 30 + 1), (6 + 30 + 480 + 1),   16,     12   },
	/*52*/{HDMI_720X576P200,      ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     64, (144 + 720), (64 + 68), (64 + 68 + 720),        5, (576 + 49), (5 + 39), (5 + 39 + 576), (5 + 39), (5 + 39 + 576),   20,     16   },
	/*53*/{HDMI_720X576P200_16X9, ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     64, (144 + 720), (64 + 68), (64 + 68 + 720),        5, (576 + 49), (5 + 39), (5 + 39 + 576), (5 + 39), (5 + 39 + 576),   20,     16   },
	/*54*/{HDMI_720X576I200,      ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, FALSE, 126 / 2, (288 + 1440) / 2, (126 + 138) / 2, (126 + 138 + 1440) / 2, 6, (576 + 49), (6 + 19 * 2), (6 + 38 + 576), (6 + 38 + 1), (6 + 38 + 576 + 1), 16,     16   },
	/*55*/{HDMI_720X576I200_16X9, ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, FALSE, 126 / 2, (288 + 1440) / 2, (126 + 138) / 2, (126 + 138 + 1440) / 2, 6, (576 + 49), (6 + 19 * 2), (6 + 38 + 576), (6 + 38 + 1), (6 + 38 + 576 + 1), 16,     16   },
	/*56*/{HDMI_720X480P240,      ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     62, (138 + 720), (62 + 60), (62 + 60 + 720),        6, (480 + 45), (6 + 30), (6 + 30 + 480), (6 + 30), (6 + 30 + 480),   20,     12   },
	/*57*/{HDMI_720X480P240_16X9, ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     62, (138 + 720), (62 + 60), (62 + 60 + 720),        6, (480 + 45), (6 + 30), (6 + 30 + 480), (6 + 30), (6 + 30 + 480),   20,     12   },
	/*58*/{HDMI_720X480I240,      ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, FALSE, 124 / 2, (276 + 1440) / 2, (124 + 114) / 2, (124 + 114 + 1440) / 2, 6, (480 + 45), (6 + 30), (6 + 30 + 480), (6 + 30 + 1), (6 + 30 + 480 + 1),   16,     12   },
	/*59*/{HDMI_720X480I240_16X9, ACT_LOW,  ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, FALSE, 124 / 2, (276 + 1440) / 2, (124 + 114) / 2, (124 + 114 + 1440) / 2, 6, (480 + 45), (6 + 30), (6 + 30 + 480), (6 + 30 + 1), (6 + 30 + 480 + 1),   16,     12   },

	/*93*/ {HDMI_3840X2160P24,    ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     88, (1660 + 3840), (88 + 296), (88 + 296 + 3840),      10, (2160 + 90), (10 + 72), (10 + 72 + 2160), (10 + 72), (10 + 72 + 2160), 90,     60   },
	/*94*/ {HDMI_3840X2160P25,    ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     88, (1440 + 3840), (88 + 296), (88 + 296 + 3840),      10, (2160 + 90), (10 + 72), (10 + 72 + 2160), (10 + 72), (10 + 72 + 2160), 90,     60   },
	/*95*/ {HDMI_3840X2160P30,    ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     88, (560 + 3840), (88 + 296), (88 + 296 + 3840),      10, (2160 + 90), (10 + 72), (10 + 72 + 2160), (10 + 72), (10 + 72 + 2160), 90,     60   },
	/*98*/ {HDMI_4096X2160P24,    ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     88, (1404 + 4096), (88 + 296), (88 + 296 + 4096),      10, (2160 + 90), (10 + 72), (10 + 72 + 2160), (10 + 72), (10 + 72 + 2160), 90,     60   },
	/*99*/ {HDMI_4096X2160P25,    ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     88, (1184 + 4096), (88 + 128), (88 + 128 + 4096),      10, (2160 + 90), (10 + 72), (10 + 72 + 2160), (10 + 72), (10 + 72 + 2160), 90,     60   },
#if 1//!_FPGA_EMULATION_
	/*100*/{HDMI_4096X2160P30,    ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     88, (304 + 4096), (88 + 128), (88 + 128 + 4096),      10, (2160 + 90), (10 + 72), (10 + 72 + 2160), (10 + 72), (10 + 72 + 2160), 90,     60   },
#else
// This is fpga test size to output width 4096 to SC monitor. because VA1809 would be clamped to 4095.
	/*100*/{HDMI_4096X2160P30,    ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     88, (304 + 4096), (88 + 128), (88 + 128 + 4096),      5, (272 + 15), (5 + 5), (5 + 5 + 272), (5 + 5), (5 + 5 + 272), 90,     8   },
#endif
	/*            VID                 HS       VS       HVLD      VVLD      CLK       DE  PROGRESSIVE HSYNC  HTOT      HSTART       HEND            VSYNC    VTOT       VOSTART     VOEND       VESTART       VEEND      WINX,   WINY */
 /*128+95*/ {HDMI14B_3840X2160P30, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     88, (560 + 3840), (88 + 296), (88 + 296 + 3840),      10, (2160 + 90), (10 + 72), (10 + 72 + 2160), (10 + 72), (10 + 72 + 2160), 90,     60   },
 /*128+94*/ {HDMI14B_3840X2160P25, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     88, (1440 + 3840), (88 + 296), (88 + 296 + 3840),      10, (2160 + 90), (10 + 72), (10 + 72 + 2160), (10 + 72), (10 + 72 + 2160), 90,     60   },
 /*128+93*/ {HDMI14B_3840X2160P24, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     88, (1660 + 3840), (88 + 296), (88 + 296 + 3840),      10, (2160 + 90), (10 + 72), (10 + 72 + 2160), (10 + 72), (10 + 72 + 2160), 90,     60   },
 /*128+98*/ {HDMI14B_4096X2160P24, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     88, (1404 + 4096), (88 + 296), (88 + 296 + 4096),      10, (2160 + 90), (10 + 72), (10 + 72 + 2160), (10 + 72), (10 + 72 + 2160), 90,     60   },

	/*            VID                 HS       VS       HVLD      VVLD      CLK       DE  PROGRESSIVE HSYNC  HTOT      HSTART       HEND                   VSYNC    VTOT       VOSTART     VOEND       VESTART       VEEND      WINX,   WINY */
 /*128+35*/ {HDMI_1024X768P60,     ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,    136, (1024 + 320), (136 + 160), (136+ 160 + 1024),       6, (768 + 38),  (6 + 29), (6 + 29 +  768), (6 + 29), (6 + 29 +  768), 45,     30   },
 /*128+36*/ {HDMI_1280X1024P60,    ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,    112, (1280 + 408), (112 + 248), (112+ 248 + 1280),       3, (1024 + 42), (3 + 38), (3 + 38 + 1024), (3 + 38), (3 + 38 + 1024), 45,     30   },
 /*128+37*/ {HDMI_1600X1200P60,    ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,    192, (1600 + 560), (192 + 304), (192+ 304 + 1600),       3, (1200 + 50), (3 + 46), (3 + 46 + 1200), (3 + 46), (3 + 46 + 1200), 45,     30   },
 /*128+38*/ {HDMI_2560X1440P60,    ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,    32,  (2560 + 160), (32  + 80),  (32 +  80 + 2560),       5, (1440 + 41), (5 + 34), (5 + 34 + 1440), (5 + 34), (5 + 34 + 1440), 45,     30   },
 /*128+39*/	{HDMI_1280X720_FPGA,   ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     40, (370 + 1280), (40 + 220), (40 + 220 + 1280),      5, (720 + 30), (5 + 20), (5 + 20 + 720), (5 + 20), (5 + 20 + 720),   30,     20   },
 /*128+40*/	{HDMI_1920X1080_FPGA,  ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     44, (280 + 1920), (44 + 148), (44 + 148 + 1920),      5, (1080 + 45), (5 + 36), (5 + 36 + 1080), (5 + 36), (5 + 36 + 1080),   45,     30   },
 /*128+41*/ {HDMI_1440X900P60,     ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,    152,  1904,        (152  + 312),  (152  + 312 + 1440),   6, 934,         (6 + 28), (6 + 28 + 900),  (6 + 28), (6 + 28 + 900),  45,     30   },
 /*128+42*/ {HDMI_1680X1050P60,    ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,    176,  2240,        (176  + 384),  (176  + 384 + 1680),   6, 1089,        (6 + 33), (6 + 33 + 1050), (6 + 33), (6 + 33 + 1050), 45,     30   },
 /*128+43*/ {HDMI_1920X1200P60,    ACT_HIGH,  ACT_LOW, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,    32,   2080,         (32  + 128),  (32  + 128 + 1920),    6, 1235,        (6 + 29), (6 + 29 + 1200), (6 + 29), (6 + 29 + 1200), 45,     30   },

};

static FP	hdmi_callback = NULL;
static HDMI_MODE        g_dispdev_hdmi_mode = HDMI_MODE_YCBCR444;
static HDMI_VIDEOID     g_dispdev_hdmi_vid = HDMI_720X480P60;
static HDMI_AUDIOFMT    g_dispdev_aud_fmt  = HDMI_AUDIO48KHZ;
static nvt_hdmi_src_t   g_dispdev_hdmi_src  = NVT_HDMI_LCD210;

static UINT32           g_dispdev_win_hozi_ofs = 0;
static UINT32           g_dispdev_win_vert_ofs = 0;
static UINT32           g_dispdev_hdmi_index = 1;

/* Used to check if VID_Code is in edid list, configure it to hw if yes.
 */
static BOOL dispdev_init_hdmi_vid_format(HDMI_VIDEOID vid_code)
{
	UINT8                   i, valid = 0;

	disphdmi_debug(("DEVHDMI: initHdmiVidFormat (%d)\r\n", vid_code));

	//
	//Search the mapping Video ID index
	//
	for (i = 0; i < VIDMAX; i++) {
		if (disp_video_timing[i].VID == vid_code) {
			g_dispdev_hdmi_index   = i;
			valid           = 1;
			break;
		}
	}

	if (valid == 0) {
		DBG_ERR("No support Video Format! %d\r\n", vid_code);
		return FALSE;
	}

	//
	//  Handle the TV overscan/underscan capability.
	//
	if ((!hdmitx_get_config(HDMI_CONFIG_ID_FORCE_UNDERSCAN)) && ((hdmitx_get_edid_info()&HDMITX_TV_UNDERSCAN) == 0x0) && (hdmitx_get_edid_info() != 0x0)) {
		// Overscan
		g_dispdev_win_hozi_ofs = disp_video_timing[g_dispdev_hdmi_index].WINX_OFS << 1;
		g_dispdev_win_vert_ofs = disp_video_timing[g_dispdev_hdmi_index].WINY_OFS << 1;
	} else {
		// underscan or EDID Parsing fail
		g_dispdev_win_hozi_ofs = 0x0;
		g_dispdev_win_vert_ofs = 0x0;
	}

	if ((!hdmitx_get_config(HDMI_CONFIG_ID_FORCE_RGB)) && (hdmitx_get_edid_info()&HDMITX_TV_YCBCR444)) {
		g_dispdev_hdmi_mode = HDMI_MODE_YCBCR444;
	} else if ((!hdmitx_get_config(HDMI_CONFIG_ID_FORCE_RGB)) && (hdmitx_get_edid_info()&HDMITX_TV_YCBCR422)) {
		g_dispdev_hdmi_mode = HDMI_MODE_YCBCR422;
	} else {
		g_dispdev_hdmi_mode = HDMI_MODE_RGB444;
	}

	return TRUE;
}


static void dispdev_init_hdmi_display(void)
{
	UINT32 edid_bitmap;

#if 1
	hdmitx_set_config(HDMI_CONFIG_ID_COMPONENT0, 0);
	hdmitx_set_config(HDMI_CONFIG_ID_COMPONENT1, 1);
	hdmitx_set_config(HDMI_CONFIG_ID_COMPONENT2, 2);
#else
	if (g_dispdev_hdmi_mode == HDMI_MODE_RGB444) { /* output=RGB for 444 mode */
		hdmitx_set_config(HDMI_CONFIG_ID_COMPONENT0, 0);
		hdmitx_set_config(HDMI_CONFIG_ID_COMPONENT1, 1);
		hdmitx_set_config(HDMI_CONFIG_ID_COMPONENT2, 2);
	} else if (g_dispdev_hdmi_mode == HDMI_MODE_YCBCR444) {
		hdmitx_set_config(HDMI_CONFIG_ID_COMPONENT0, 1);
		hdmitx_set_config(HDMI_CONFIG_ID_COMPONENT1, 2);
		hdmitx_set_config(HDMI_CONFIG_ID_COMPONENT2, 0);
	} else {
		hdmitx_set_config(HDMI_CONFIG_ID_COMPONENT0, 0);
		hdmitx_set_config(HDMI_CONFIG_ID_COMPONENT1, 2);
		hdmitx_set_config(HDMI_CONFIG_ID_COMPONENT2, 1);
	}
#endif

	/* bitmap is defined in Hdmitx.h */
	edid_bitmap = hdmitx_get_edid_info();
	hdmi_callback(edid_bitmap);
}

static BOOL dispdev_chg_hdmi_clk_freq(BOOL b_native)
{
	return FALSE;
}

/* enable audio */
static void dispdev_check_sample_rate(BOOL bEnable)
{
	return;
}

/*
    Display Device API of open HDMI Transmitter

    Display Device API of open HDMI Transmitter

    @param[in] pDevIF   Please refer to dispdevctrl.h for details.

    @return
     - @b E_PAR:    Error pDevIF content.
     - @b E_NOSPT:  Desired HDMI format no support.
     - @b E_OK:     HDMI Open done and success.
*/
static ER dispdev_open_hdmi(void)
{
	if (hdmi_callback == NULL)
		return E_PAR;

	disphdmi_debug(("DEVHDMI: open (%d)\r\n", hdmi_cfg.vid));

	// Open HDMI-Tx & Read EDID
	if (hdmitx_is_opened() == FALSE) {
		// Open HDMI-Tx & Read EDID
		if (hdmitx_open() != E_OK) {
			disphdmi_debug(("DEVHDMI: open fail(%d)\r\n", hdmi_cfg.vid));
			return E_NOSPT;
		}

		if (hdmitx_get_config(HDMI_CONFIG_ID_VERIFY_VIDEOID)) {
			hdmitx_verify_video_id(&g_dispdev_hdmi_vid);
			/* if vid is not supported, it will be changed to HDMI_640X480P60 */
			if (g_dispdev_hdmi_vid == HDMI_640X480P60)
				g_dispdev_hdmi_vid = DEFAULT_VID;
		}

		// Check & Change IDE Source frequency
		if (dispdev_chg_hdmi_clk_freq(TRUE)) {
			return E_NOSPT;
		}

		// Enable IDE clock
		// Set Pinmux

		hdmitx_set_config(HDMI_CONFIG_ID_VIDEO, g_dispdev_hdmi_vid);
		hdmitx_set_config(HDMI_CONFIG_ID_AUDIO, g_dispdev_aud_fmt);
		hdmitx_set_config(HDMI_CONFIG_ID_SOURCE, g_dispdev_hdmi_src);


		// Configure VID content
		if (dispdev_init_hdmi_vid_format(g_dispdev_hdmi_vid) == FALSE) {
			return E_NOSPT;
		}

		// Configure IDE
		dispdev_init_hdmi_display();

		hdmitx_check_video_stable();
		hdmitx_set_config(HDMI_CONFIG_ID_OE,             TRUE);
		hdmitx_set_config(HDMI_CONFIG_ID_AUDIO_STREAM_EN, TRUE);
#ifdef LINUX
		printf("hdmi: audio stream enable. \n");
#endif
		hdmitx_set_config(HDMI_CONFIG_ID_SCDC_ENABLE,    ENABLE);

	} else {
		//
		// HDMI is already opened, Just changing HDMI format
		//
		hdmitx_set_config(HDMI_CONFIG_ID_AV_MUTE, TRUE);
		hdmitx_set_config(HDMI_CONFIG_ID_AUDIO_STREAM_EN, FALSE);

		if (hdmitx_get_config(HDMI_CONFIG_ID_VERIFY_VIDEOID)) {
			hdmitx_verify_video_id(&g_dispdev_hdmi_vid);
		}

		// Check & Change IDE Source frequency
		if (dispdev_chg_hdmi_clk_freq(FALSE)) {
			return E_NOSPT;
		}
		// Enable IDE clock

		// Configure VID content
		if (dispdev_init_hdmi_vid_format(g_dispdev_hdmi_vid) == FALSE) {
			return E_NOSPT;
		}

		// Configure IDE
		dispdev_init_hdmi_display();

		// Open HDMI-Tx
		hdmitx_set_config(HDMI_CONFIG_ID_VIDEO, g_dispdev_hdmi_vid);
		hdmitx_set_config(HDMI_CONFIG_ID_AUDIO, g_dispdev_aud_fmt);
		hdmitx_set_config(HDMI_CONFIG_ID_SOURCE, g_dispdev_hdmi_src);

		hdmitx_check_video_stable();
		hdmitx_set_config(HDMI_CONFIG_ID_OE,             TRUE);
		hdmitx_set_config(HDMI_CONFIG_ID_AUDIO_STREAM_EN, TRUE);

		hdmitx_set_config(HDMI_CONFIG_ID_SCDC_ENABLE,    ENABLE);

		// Wait TV Video Stable and then clear AV-Mute
		hdmitx_set_config(HDMI_CONFIG_ID_AV_MUTE, FALSE);
	}

	dispdev_check_sample_rate(ENABLE);

	return E_OK;
}

/*
    Display Device API of Close HDMI Transmitter

    Display Device API of Close HDMI Transmitter

    @param[in] pDevIF   Please refer to dispdevctrl.h for details.

    @return
     - @b E_OK:     HDMI Close done and success.
*/
static ER dispdev_close_hdmi(void)
{
	if (hdmitx_is_opened() == FALSE) {
		return E_OK;
	}

	disphdmi_debug(("DEVHDMI: close\r\n"));

	hdmitx_set_config(HDMI_CONFIG_ID_OE, FALSE);
	// Close HDMI
	hdmitx_close();

	dispdev_check_sample_rate(DISABLE);
	hdmi_callback(0);
	return E_OK;
}
/*
    Set HDMI Display Device IOCTRL function pointer
*/
static void dispdev_set_hdmi_ioctrl(FP callback)
{
	hdmi_callback = callback;
}

/* Read HDMI register
 */
static UINT32 dispdev_read_hdmi_edid(void)
{
	UINT32 uiVal = 0;

	return uiVal;
}



/*
    Set HDMI Display config before open
*/
static ER dispdev_preconfighdmi(PDISP_HDMI_CFG config)
{
	g_dispdev_aud_fmt = (HDMI_AUDIOFMT)config->audio_fmt;
	g_dispdev_hdmi_vid = (HDMI_VIDEOID)config->vid;
	g_dispdev_hdmi_mode = (HDMI_MODE)config->output_mode;
	g_dispdev_hdmi_src = (nvt_hdmi_src_t)config->source;

	return E_OK;
}
/**
    Get HDMI Display Device Object

    Get HDMI Display Device Object. This Object is used to control the HDMI Transmitter.

    @return The pointer of HDMI Display Device object.
*/
PDISPDEV_OBJ dispdev_get_hdmi_dev_obj(void)
{
	return &dispdev_hdmi_obj;
}
