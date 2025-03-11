/*
    Display HDMI device object

    @file       dispdev_hdmi.c
    @ingroup    mIDISPHDMITx
    @note       Nothing

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/

#include "hdmi_port.h"
#include "hdmitx_int.h"
#include "hdmitx_reg.h"
#include <asm/arch/dispdevctrl.h>
#include "dispdev_hdmi.h"





/* macros */
#define DEFAULT_VID		HDMI_1920X1080P60


#if 1//HDMI_DISOBJUSED
	#define DISPDEVIOUSED   1
#else
	#define DISPDEVIOUSED   0
#endif



#if DISPDEVIOUSED
static DISPDEV_IOCTRL   p_hdmi_io_control;
#endif
static ER               dispdev_open_hdmi(void);
static ER               dispdev_close_hdmi(void);
static ER 				dispdev_preconfighdmi(PDISP_HDMI_CFG config);

static void             dispdev_set_hdmi_ioctrl(FP p_io_ctrl_func);
static ER               dispdev_get_hdmi_size(DISPDEV_GET_PRESIZE *t_size);

static DISPDEV_OBJ dispdev_hdmi_obj = { dispdev_open_hdmi, dispdev_close_hdmi, NULL, NULL, NULL, dispdev_get_hdmi_size, dispdev_set_hdmi_ioctrl, {NULL, NULL }, dispdev_preconfighdmi };

#if DISPDEVIOUSED
static DISPDEVHDMI_CTRL dispdevhdmi_control = {dispdev_get_hdmi_dev_obj, hdmitx_set_config, hdmitx_get_edid_info, hdmitx_get_video_ability};
#endif

#define     VIDMAX      (76)
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

	/*97*/ {HDMI_3840X2160P60,    ACT_LOW, ACT_LOW, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     88, (560 + 3840), (88 + 296), (88 + 296 + 3840),      10, (2160 + 90), (10 + 72), (10 + 72 + 2160), (10 + 72), (10 + 72 + 2160), 90,     60   },

	/*98*/ {HDMI_4096X2160P24,    ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     88, (1404 + 4096), (88 + 296), (88 + 296 + 4096),      10, (2160 + 90), (10 + 72), (10 + 72 + 2160), (10 + 72), (10 + 72 + 2160), 90,     60   },
	/*99*/ {HDMI_4096X2160P25,    ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     88, (1184 + 4096), (88 + 128), (88 + 128 + 4096),      10, (2160 + 90), (10 + 72), (10 + 72 + 2160), (10 + 72), (10 + 72 + 2160), 90,     60   },
#if !_FPGA_EMULATION_
	/*100*/{HDMI_4096X2160P30,    ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     88, (304 + 4096), (88 + 128), (88 + 128 + 4096),      10, (2160 + 90), (10 + 72), (10 + 72 + 2160), (10 + 72), (10 + 72 + 2160), 90,     60   },
#else
// This is fpga test size to output width 4096 to SC monitor. because VA1809 would be clamped to 4095.
	/*100*/{HDMI_4096X2160P30,    ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     88, (304 + 4096), (88 + 128), (88 + 128 + 4096),      5, (272 + 15), (5 + 5), (5 + 5 + 272), (5 + 5), (5 + 5 + 272), 90,     8   },
#endif

 /*128+95*/{HDMI14B_3840X2160P30, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     88, (560 + 3840), (88 + 296), (88 + 296 + 3840),      10, (2160 + 90), (10 + 72), (10 + 72 + 2160), (10 + 72), (10 + 72 + 2160), 90,     60   },
 /*128+94*/{HDMI14B_3840X2160P25, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     88, (1440 + 3840), (88 + 296), (88 + 296 + 3840),      10, (2160 + 90), (10 + 72), (10 + 72 + 2160), (10 + 72), (10 + 72 + 2160), 90,     60   },
 /*128+93*/{HDMI14B_3840X2160P24, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     88, (1660 + 3840), (88 + 296), (88 + 296 + 3840),      10, (2160 + 90), (10 + 72), (10 + 72 + 2160), (10 + 72), (10 + 72 + 2160), 90,     60   },
 /*128+98*/{HDMI14B_4096X2160P24, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     88, (1404 + 4096), (88 + 296), (88 + 296 + 4096),      10, (2160 + 90), (10 + 72), (10 + 72 + 2160), (10 + 72), (10 + 72 + 2160), 90,     60   },

	/*            VID                 HS       VS       HVLD      VVLD      CLK       DE  PROGRESSIVE HSYNC  HTOT      HSTART       HEND                   VSYNC    VTOT       VOSTART     VOEND       VESTART       VEEND      WINX,   WINY */
 /*128+ 4*/{HDMI_1280X720P30,     ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     40, (370 + 1280), (40 + 220), (40 + 220 + 1280),      5, (720 + 30), (5 + 20), (5 + 20 + 720), (5 + 20), (5 + 20 + 720),   30,     20   },
 /*128+19*/{HDMI_1280X720P25,     ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,     40, (700 + 1280), (40 + 220), (40 + 220 + 1280),      5, (720 + 30), (5 + 20), (5 + 20 + 720), (5 + 20), (5 + 20 + 720),   30,     20   },
 /*128+35*/{HDMI_1024X768P60,     ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,    136, (1024 + 320), (136 + 160), (136+ 160 + 1024),       6, (768 + 38),  (6 + 29), (6 + 29 +  768), (6 + 29), (6 + 29 +  768), 45,     30   },
 /*128+36*/{HDMI_1280X1024P60,    ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,    112, (1280 + 408), (112 + 248), (112+ 248 + 1280),       3, (1024 + 42), (3 + 38), (3 + 38 + 1024), (3 + 38), (3 + 38 + 1024), 45,     30   },
 /*128+37*/{HDMI_1600X1200P60,    ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,    192, (1600 + 560), (192 + 304), (192+ 304 + 1600),       3, (1200 + 50), (3 + 46), (3 + 46 + 1200), (3 + 46), (3 + 46 + 1200), 45,     30   },
 /*128+38*/{HDMI_2560X1440P60,    ACT_LOW,  ACT_HIGH, ACT_HIGH, ACT_HIGH, ACT_LOW, ACT_HIGH, TRUE,    32,  (2560 + 160), (32  + 80),  (32 +  80 + 2560),       5, (1440 + 41), (5 + 34), (5 + 34 + 1440), (5 + 34), (5 + 34 + 1440), 45,     30   },


};

#if DISPDEVIOUSED
static UINT32           g_dispdev_src_freq;
#endif

static HDMI_MODE        g_dispdev_hdmi_mode = HDMI_MODE_YCBCR444;
static HDMI_VIDEOID     g_dispdev_hdmi_vid = HDMI_720X480P60;
static HDMI_AUDIOFMT    g_dispdev_aud_fmt  = HDMI_AUDIO48KHZ;
static HDMI_SRCFMT		g_dispdev_src_fmt  = HDMI_IDE_0;

static UINT32           g_dispdev_win_hozi_ofs = 0;
static UINT32           g_dispdev_win_vert_ofs = 0;
static UINT32           g_dispdev_hdmi_index = 1;

static BOOL dispdev_init_hdmi_vid_format(HDMI_VIDEOID vid_code)
{
	UINT8                   i, valid = 0;
#if DISPDEVIOUSED
	DISPDEV_IOCTRL_PARAM    dev_io_ctrl;
	DISPDEV_IOCTRL          p_dispdev_control = p_hdmi_io_control;
#endif
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
#if DISPDEVIOUSED
		dev_io_ctrl.SEL.SET_CONST_OFS.ui_win_const_ofs_x = disp_video_timing[g_dispdev_hdmi_index].WINX_OFS;
		dev_io_ctrl.SEL.SET_CONST_OFS.ui_win_const_ofs_y = disp_video_timing[g_dispdev_hdmi_index].WINY_OFS;
#endif
		g_dispdev_win_hozi_ofs = disp_video_timing[g_dispdev_hdmi_index].WINX_OFS << 1;
		g_dispdev_win_vert_ofs = disp_video_timing[g_dispdev_hdmi_index].WINY_OFS << 1;
	} else {
		// underscan or EDID Parsing fail
#if DISPDEVIOUSED
		dev_io_ctrl.SEL.SET_CONST_OFS.ui_win_const_ofs_x = 0;
		dev_io_ctrl.SEL.SET_CONST_OFS.ui_win_const_ofs_y = 0;
#endif
		g_dispdev_win_hozi_ofs = 0x0;
		g_dispdev_win_vert_ofs = 0x0;
	}
#if DISPDEVIOUSED
	p_dispdev_control(DISPDEV_IOCTRL_SET_CONST_OFS, &dev_io_ctrl);
#endif

	if ((!hdmitx_get_config(HDMI_CONFIG_ID_FORCE_RGB)) && (hdmitx_get_edid_info()&HDMITX_TV_YCBCR444)) {
		g_dispdev_hdmi_mode = HDMI_MODE_YCBCR444;
	} else if ((!hdmitx_get_config(HDMI_CONFIG_ID_FORCE_RGB)) && (hdmitx_get_edid_info()&HDMITX_TV_YCBCR422)) {
		g_dispdev_hdmi_mode = HDMI_MODE_YCBCR422;
	} else {
		g_dispdev_hdmi_mode = HDMI_MODE_RGB444;
	}
#if DISPDEVIOUSED
	if (hdmitx_get_config(HDMI_CONFIG_ID_FORCE_YUV444)) {
		g_dispdev_hdmi_mode = HDMI_MODE_YCBCR444;
	} else if (hdmitx_get_config(HDMI_CONFIG_ID_FORCE_YUV422)) {
		g_dispdev_hdmi_mode = HDMI_MODE_YCBCR422;
	}
#endif
	return TRUE;
}


static void dispdev_init_hdmi_display(void)
{
#if !DISPDEVIOUSED
	hdmitx_set_config(HDMI_CONFIG_ID_COMPONENT0, 0);
	hdmitx_set_config(HDMI_CONFIG_ID_COMPONENT1, 1);
	hdmitx_set_config(HDMI_CONFIG_ID_COMPONENT2, 2);
#else
	DISPDEV_IOCTRL_PARAM    dev_io_ctrl;
	DISPDEV_IOCTRL          p_dispdev_control = p_hdmi_io_control;

	disphdmi_debug(("DEVHDMI: initHdmiDisplay\r\n"));

	dev_io_ctrl.SEL.SET_DITHER_EN.b_en = FALSE;
	p_dispdev_control(DISPDEV_IOCTRL_SET_DITHER_EN, &dev_io_ctrl);

	if (g_dispdev_hdmi_mode == HDMI_MODE_YCBCR422) {
		/* 16bits */
		dev_io_ctrl.SEL.SET_DEVICE.disp_dev_type = DISPDEV_TYPE_INF_HDMI_16BIT;
	} else {
		/* 24bits */
		dev_io_ctrl.SEL.SET_DEVICE.disp_dev_type = DISPDEV_TYPE_EMBD_HDMI;
	}
	p_dispdev_control(DISPDEV_IOCTRL_SET_DEVICE, &dev_io_ctrl);

#if 0//_FPGA_EMULATION_
	hdmitx_set_pg_1(0, disp_video_timing[g_dispdev_hdmi_index].H_TOTAL);
	hdmitx_set_pg_2(disp_video_timing[g_dispdev_hdmi_index].H_SYNCTIME, disp_video_timing[g_dispdev_hdmi_index].H_STARTTIME);
	hdmitx_set_pg_3((disp_video_timing[g_dispdev_hdmi_index].H_ENDTIME - disp_video_timing[g_dispdev_hdmi_index].H_STARTTIME), (disp_video_timing[g_dispdev_hdmi_index].V_TOTAL));
	hdmitx_set_pg_4((disp_video_timing[g_dispdev_hdmi_index].V_SYNCTIME), disp_video_timing[g_dispdev_hdmi_index].V_ODD_STARTTIME);
	hdmitx_set_pg_5((disp_video_timing[g_dispdev_hdmi_index].V_ODD_ENDTIME - disp_video_timing[g_dispdev_hdmi_index].V_ODD_STARTTIME));
	hdmitx_set_pg_pattern(0x0A);
#endif

	hdmitx_set_config(HDMI_CONFIG_ID_VSYNC_INVERT, !disp_video_timing[g_dispdev_hdmi_index].VS);
	hdmitx_set_config(HDMI_CONFIG_ID_HSYNC_INVERT, !disp_video_timing[g_dispdev_hdmi_index].HS);

	dev_io_ctrl.SEL.SET_VLD_INVERT.b_hvld_inv    = disp_video_timing[g_dispdev_hdmi_index].HVLD;
	dev_io_ctrl.SEL.SET_VLD_INVERT.b_vvld_inv    = disp_video_timing[g_dispdev_hdmi_index].VVLD;
	dev_io_ctrl.SEL.SET_VLD_INVERT.b_field_inv   = FALSE;
	dev_io_ctrl.SEL.SET_VLD_INVERT.b_de_inv      = disp_video_timing[g_dispdev_hdmi_index].DE;
	p_dispdev_control(DISPDEV_IOCTRL_SET_VLD_INVERT, &dev_io_ctrl);

	dev_io_ctrl.SEL.SET_WINDOW_H_TIMING.ui_hsync        = disp_video_timing[g_dispdev_hdmi_index].H_SYNCTIME - 1;
	dev_io_ctrl.SEL.SET_WINDOW_H_TIMING.ui_htotal       = disp_video_timing[g_dispdev_hdmi_index].H_TOTAL - 1;
	dev_io_ctrl.SEL.SET_WINDOW_H_TIMING.ui_hvld_start    = disp_video_timing[g_dispdev_hdmi_index].H_STARTTIME;
	dev_io_ctrl.SEL.SET_WINDOW_H_TIMING.ui_hvld_end      = disp_video_timing[g_dispdev_hdmi_index].H_ENDTIME - 1;
	p_dispdev_control(DISPDEV_IOCTRL_SET_WINDOW_H_TIMING, &dev_io_ctrl);

	dev_io_ctrl.SEL.SET_WINDOW_V_TIMING.ui_vsync        = disp_video_timing[g_dispdev_hdmi_index].V_SYNCTIME - 1;
	dev_io_ctrl.SEL.SET_WINDOW_V_TIMING.ui_vtotal       = disp_video_timing[g_dispdev_hdmi_index].V_TOTAL - 1;
	dev_io_ctrl.SEL.SET_WINDOW_V_TIMING.ui_vvld_odd_start = disp_video_timing[g_dispdev_hdmi_index].V_ODD_STARTTIME;
	dev_io_ctrl.SEL.SET_WINDOW_V_TIMING.ui_vvld_odd_end   = disp_video_timing[g_dispdev_hdmi_index].V_ODD_ENDTIME - 1;
	dev_io_ctrl.SEL.SET_WINDOW_V_TIMING.ui_vvld_even_start = disp_video_timing[g_dispdev_hdmi_index].V_EVEN_STARTTIME;
	dev_io_ctrl.SEL.SET_WINDOW_V_TIMING.ui_vvld_even_end  = disp_video_timing[g_dispdev_hdmi_index].V_EVEN_ENDTIME - 1;
	p_dispdev_control(DISPDEV_IOCTRL_SET_WINDOW_V_TIMING, &dev_io_ctrl);

	dev_io_ctrl.SEL.SET_CSB_EN.b_en = FALSE;
	p_dispdev_control(DISPDEV_IOCTRL_SET_CSB_EN, &dev_io_ctrl);

	dev_io_ctrl.SEL.SET_YC_EXCHG.b_cbcr_exchg = FALSE;
	dev_io_ctrl.SEL.SET_YC_EXCHG.b_yc_exchg = FALSE;
	p_dispdev_control(DISPDEV_IOCTRL_SET_YC_EXCHG, &dev_io_ctrl);

	dev_io_ctrl.SEL.SET_SUBPIXEL.b_odd_r     = FALSE;
	dev_io_ctrl.SEL.SET_SUBPIXEL.b_odd_g     = FALSE;
	dev_io_ctrl.SEL.SET_SUBPIXEL.b_odd_b     = FALSE;
	dev_io_ctrl.SEL.SET_SUBPIXEL.b_even_r    = FALSE;
	dev_io_ctrl.SEL.SET_SUBPIXEL.b_even_g    = FALSE;
	dev_io_ctrl.SEL.SET_SUBPIXEL.b_even_b    = FALSE;
	p_dispdev_control(DISPDEV_IOCTRL_SET_SUBPIXEL, &dev_io_ctrl);

	dev_io_ctrl.SEL.SET_CLK1_2.b_clk1_2     = FALSE;
	p_dispdev_control(DISPDEV_IOCTRL_SET_CLK1_2, &dev_io_ctrl);

	#if (defined __UITRON || defined __ECOS)
	ide_set_tv_power_down(TRUE);
	#endif

	if (g_dispdev_hdmi_mode == HDMI_MODE_RGB444) { /* output=RGB for 444 mode */
		/* Output=RGB for 444 mode */
		dev_io_ctrl.SEL.SET_ICST_EN.b_en    = TRUE;
		dev_io_ctrl.SEL.SET_ICST_EN.select = CST_YCBCR2RGB;
		p_dispdev_control(DISPDEV_IOCTRL_SET_ICST_EN, &dev_io_ctrl);
		hdmitx_set_config(HDMI_CONFIG_ID_COMPONENT0, 0);
		hdmitx_set_config(HDMI_CONFIG_ID_COMPONENT1, 1);
		hdmitx_set_config(HDMI_CONFIG_ID_COMPONENT2, 2);
	} else if (g_dispdev_hdmi_mode == HDMI_MODE_YCBCR444) {
		/* output=YCbCr for 444 mode */
		dev_io_ctrl.SEL.SET_ICST_EN.b_en    = FALSE;
		dev_io_ctrl.SEL.SET_ICST_EN.select = CST_RGB2YCBCR;
		p_dispdev_control(DISPDEV_IOCTRL_SET_ICST_EN, &dev_io_ctrl);
		hdmitx_set_config(HDMI_CONFIG_ID_COMPONENT0, 1);
		hdmitx_set_config(HDMI_CONFIG_ID_COMPONENT1, 2);
		hdmitx_set_config(HDMI_CONFIG_ID_COMPONENT2, 0);
	} else {
		/* output=YCbCr for 422 mode */
		dev_io_ctrl.SEL.SET_ICST_EN.b_en = FALSE;
		dev_io_ctrl.SEL.SET_ICST_EN.select = CST_RGB2YCBCR;
		p_dispdev_control(DISPDEV_IOCTRL_SET_ICST_EN, &dev_io_ctrl);

		hdmitx_set_config(HDMI_CONFIG_ID_COMPONENT0, 0);
		hdmitx_set_config(HDMI_CONFIG_ID_COMPONENT1, 2);
		hdmitx_set_config(HDMI_CONFIG_ID_COMPONENT2, 1);
	}

	dev_io_ctrl.SEL.SET_WINDOW_OUT_TYPE.b_interlaced = !(disp_video_timing[g_dispdev_hdmi_index].PROGRESSIVE);
	dev_io_ctrl.SEL.SET_WINDOW_OUT_TYPE.b_field_odd_st = TRUE;
	p_dispdev_control(DISPDEV_IOCTRL_SET_WINDOW_OUT_TYPE, &dev_io_ctrl);

	dev_io_ctrl.SEL.SET_OUT_LIMIT.ui_y_low   = 0x00;
	dev_io_ctrl.SEL.SET_OUT_LIMIT.ui_y_up    = 0xFF;
	dev_io_ctrl.SEL.SET_OUT_LIMIT.ui_cb_low  = 0x00;
	dev_io_ctrl.SEL.SET_OUT_LIMIT.ui_cb_up   = 0xFF;
	dev_io_ctrl.SEL.SET_OUT_LIMIT.ui_cr_low  = 0x00;
	dev_io_ctrl.SEL.SET_OUT_LIMIT.ui_cr_up   = 0xFF;
	p_dispdev_control(DISPDEV_IOCTRL_SET_OUT_LIMIT, &dev_io_ctrl);
#endif
}

#if 1
static void dispdev_disable_ide(void)
{
	DISPDEV_IOCTRL          p_dispdev_control = p_hdmi_io_control;
	DISPDEV_IOCTRL_PARAM    dev_io_ctrl;

	p_dispdev_control(DISPDEV_IOCTRL_GET_ENABLE, &dev_io_ctrl);
	if (dev_io_ctrl.SEL.GET_ENABLE.b_en == TRUE) {
		disphdmi_debug(("DEVHDMI: dispdev_disable_ide\r\n"));

		dev_io_ctrl.SEL.SET_ENABLE.b_en = FALSE;
		p_dispdev_control(DISPDEV_IOCTRL_SET_ENABLE,    &dev_io_ctrl);
		p_dispdev_control(DISPDEV_IOCTRL_WAIT_FRAMEEND, NULL);

		// Close IDE clock
		dev_io_ctrl.SEL.SET_CLK_EN.b_clk_en = FALSE;
		p_dispdev_control(DISPDEV_IOCTRL_SET_CLK_EN, &dev_io_ctrl);
	}
}
#endif


static BOOL dispdev_chg_hdmi_clk_freq(BOOL b_native)
{
#if DISPDEVIOUSED
#if 0
	UINT32                  ui_target_freq = 27000000;
	DISPDEV_IOCTRL          p_dispdev_control = p_hdmi_io_control;
	DISPDEV_IOCTRL_PARAM    dev_io_ctrl;
	UINT32                  cur_freq;
	struct clk *pll_clk = NULL;

	disphdmi_debug(("DEVHDMI: chgHdmiClkFreq (%d)\r\n", b_native));

	p_dispdev_control(DISPDEV_IOCTRL_GET_SRCCLK, &dev_io_ctrl);

	switch (dev_io_ctrl.SEL.GET_SRCCLK.src_clk) {
		
	case DISPCTRL_SRCCLK_PLL9:{
			//pll_clk = clk_get(NULL, "pll9");
			disphdmi_debug(("DEVHDMI: Original src_clk = pll[9]\r\n"));
		}break;
	
	case DISPCTRL_SRCCLK_PLL10:{
			//pll_clk = clk_get(NULL, "pll10");
			disphdmi_debug(("DEVHDMI: Original src_clk = pll[10]\r\n"));
		}break;
	
	default: {
			//pll_clk = clk_get(NULL, "pll9");
			disphdmi_debug(("DEVHDMI: Original default src_clk = pll[9]\r\n"));
		}

	}

	if (pll_clk != NULL) {

		//cur_freq = clk_get_rate(pll_clk);
		disphdmi_debug(("DEVHDMI: Original src_clk(%d)Hertz\r\n", cur_freq));

		if (b_native) {
			g_dispdev_src_freq = cur_freq;
		}

		switch (g_dispdev_hdmi_vid) {

		/*
		    Change PLL2 to 216Mhz
		*/
		// Target 54MHz
		case HDMI_1440X480P60:          /* VID14 */
		case HDMI_1440X480P60_16X9:     /* VID15 */
		case HDMI_1440X576P50:          /* VID29 */
		case HDMI_1440X576P50_16X9:     /* VID30 */
		case HDMI_720X576P100:          /* VID42 */
		case HDMI_720X576P100_16X9:     /* VID43 */
		case HDMI_720X480P120:          /* VID48 */
		case HDMI_720X480P120_16X9:     /* VID49 */
		case HDMI_720X576I200:          /* VID54 */
		case HDMI_720X576I200_16X9:     /* VID55 */
		case HDMI_720X480I240:          /* VID58 */
		case HDMI_720X480I240_16X9:     /* VID59 */
		// Target 108MHz
		case HDMI_1280X1024P60:
		case HDMI_720X576P200:          /* VID52 */
		case HDMI_720X576P200_16X9:     /* VID53 */
		case HDMI_720X480P240:          /* VID56 */
		case HDMI_720X480P240_16X9: {   /* VID57 */
				//clk_set_rate(pll_clk, 216000000);
			}
			break;

		/*
		    Change PLL2 to 252Mhz
		*/
		// Target 25.2MHz
		case HDMI_640X480P60: {         /* VID1 */
				//clk_set_rate(pll_clk, 252000000);
			}
			break;

		/*
		    Change PLL2 to 297Mhz
		*/
		// Target 13.5MHz
		case HDMI_720X480I60:           /* VID6 */
		case HDMI_720X480I60_16X9:      /* VID7 */
		case HDMI_720X240P60:           /* VID8 */
		case HDMI_720X240P60_16X9:      /* VID9 */
		case HDMI_720X576I50:           /* VID21 */
		case HDMI_720X576I50_16X9:      /* VID22 */
		case HDMI_720X288P50:           /* VID23 */
		case HDMI_720X288P50_16X9:      /* VID24 */
		// Target 27MHz
		case HDMI_720X480P60:           /* VID2 */
		case HDMI_720X480P60_16X9:      /* VID3 */
		case HDMI_1440X480I60:          /* VID10 */
		case HDMI_1440X480I60_16X9:     /* VID11 */
		case HDMI_1440X240P60:          /* VID12 */
		case HDMI_1440X240P60_16X9:     /* VID13 */
		case HDMI_720X576P50:           /* VID17 */
		case HDMI_720X576P50_16X9:      /* VID18 */
		case HDMI_1440X576I50:          /* VID25 */
		case HDMI_1440X576I50_16X9:     /* VID26 */
		case HDMI_1440X288P50:          /* VID27 */
		case HDMI_1440X288P50_16X9:     /* VID28 */
		case HDMI_720X576I100:          /* VID44 */
		case HDMI_720X576I100_16X9:     /* VID45 */
		case HDMI_720X480I120:          /* VID50 */
		case HDMI_720X480I120_16X9:     /* VID51 */
		// Target 37.125MHz
		case HDMI_1280X720P30:          /* VID132 */
		case HDMI_1280X720P25:
		// Target 74.25MHz
		case HDMI_1280X720P60:          /* VID4 */
		case HDMI_1920X1080I60:         /* VID5 */
		case HDMI_1280X720P50:          /* VID19 */
		case HDMI_1920X1080I50:         /* VID20 */
		case HDMI_1920X1080P24:         /* VID32 */
		case HDMI_1920X1080P25:         /* VID33 */
		case HDMI_1920X1080P30:         /* VID34 */
		// Target 148.5MHz
		case HDMI_1920X1080P60:         /* VID16 */
		case HDMI_1920X1080P50:         /* VID31 */
		case HDMI_1920X1080I100:        /* VID40 */
		case HDMI_1280X720P100:         /* VID41 */
		case HDMI_1920X1080I120:        /* VID46 */
		case HDMI_1280X720P120:         /* VID47 */
		// Target 297MHz
		case HDMI_3840X2160P24:         /* VID93 */
		case HDMI_3840X2160P25:         /* VID94 */
		case HDMI_3840X2160P30:         /* VID95 */
		case HDMI_3840X2160P60: 		/* VID97*/			
		case HDMI_4096X2160P24:         /* VID98 */
		case HDMI14B_3840X2160P24:
		case HDMI14B_3840X2160P25:
		case HDMI14B_3840X2160P30:
		case HDMI14B_4096X2160P24:
		case HDMI_4096X2160P25:         /* VID99 */
		case HDMI_4096X2160P30: {       /* VID100*/
				//clk_set_rate(pll_clk, 594000000);
			}
			break;

		case HDMI_1024X768P60: {
				//clk_set_rate(pll_clk, 260000000);
			}
			break;

		case HDMI_1600X1200P60: {
				//clk_set_rate(pll_clk, 324000000);
			}
			break;
		case HDMI_2560X1440P60: {
				//clk_set_rate(pll_clk, 241500000);
			}
			break;


		case HDMI_1920X1080I50_VT1250:  /* VID39 */
		default:
			//clk_put(pll_clk);
			DBG_ERR("No support HDMI mode(%d)\r\n", g_dispdev_hdmi_vid);
			return TRUE;
		}

		//clk_put(pll_clk);
	}



	//
	//  Configure IDE Clock frequency according to HDMI Video ID
	//
	switch (g_dispdev_hdmi_vid) {

	/* Target frequency 13.5Mhz */
	case HDMI_720X480I60:          /* VID6 */
	case HDMI_720X480I60_16X9:     /* VID7 */
	case HDMI_720X240P60:          /* VID8 */
	case HDMI_720X240P60_16X9:     /* VID9 */
	case HDMI_720X576I50:          /* VID21 */
	case HDMI_720X576I50_16X9:     /* VID22 */
	case HDMI_720X288P50:          /* VID23 */
	case HDMI_720X288P50_16X9: {   /* VID24 */
			ui_target_freq = 13500000;
		}
		break;

	/* Target frequency 25.2Mhz */
	case HDMI_640X480P60: {         /* VID1 */
			ui_target_freq = 25200000;
		}
		break;

	/* Target frequency 27Mhz */
	case HDMI_720X480P60:           /* VID2 */
	case HDMI_720X480P60_16X9:      /* VID3 */
	case HDMI_1440X480I60:          /* VID10 */
	case HDMI_1440X480I60_16X9:     /* VID11 */
	case HDMI_1440X240P60:          /* VID12 */
	case HDMI_1440X240P60_16X9:     /* VID13 */
	case HDMI_720X576P50:           /* VID17 */
	case HDMI_720X576P50_16X9:      /* VID18 */
	case HDMI_1440X576I50:          /* VID25 */
	case HDMI_1440X576I50_16X9:     /* VID26 */
	case HDMI_1440X288P50:          /* VID27 */
	case HDMI_1440X288P50_16X9:     /* VID28 */
	case HDMI_720X576I100:          /* VID44 */
	case HDMI_720X576I100_16X9:     /* VID45 */
	case HDMI_720X480I120:          /* VID50 */
	case HDMI_720X480I120_16X9: {   /* VID51 */
			ui_target_freq = 27000000;
		}
		break;

	/* Target frequency 54Mhz */
	case HDMI_1440X480P60:          /* VID14 */
	case HDMI_1440X480P60_16X9:     /* VID15 */
	case HDMI_1440X576P50:          /* VID29 */
	case HDMI_1440X576P50_16X9:     /* VID30 */
	case HDMI_720X576P100:          /* VID42 */
	case HDMI_720X576P100_16X9:     /* VID43 */
	case HDMI_720X480P120:          /* VID48 */
	case HDMI_720X480P120_16X9:     /* VID49 */
	case HDMI_720X576I200:          /* VID54 */
	case HDMI_720X576I200_16X9:     /* VID55 */
	case HDMI_720X480I240:          /* VID58 */
	case HDMI_720X480I240_16X9: {   /* VID59 */
			ui_target_freq = 54000000;
		}
		break;

	/* Target frequency 37.125Mhz */
	case HDMI_1280X720P25:
	case HDMI_1280X720P30: {       /* VID132 */
			ui_target_freq = 37125000;
		}
		break;

	/* Target frequency 74.25Mhz */
	case HDMI_1280X720P60:          /* VID4 */
	case HDMI_1920X1080I60:         /* VID5 */
	case HDMI_1280X720P50:          /* VID19 */
	case HDMI_1920X1080I50:         /* VID20 */
	case HDMI_1920X1080P24:         /* VID32 */
	case HDMI_1920X1080P25:         /* VID33 */
	case HDMI_1920X1080P30: {       /* VID34 */
			ui_target_freq = 74250000;
		}
		break;

	/* Target frequency 108Mhz */
	case HDMI_1280X1024P60:
	case HDMI_720X576P200:          /* VID52 */
	case HDMI_720X576P200_16X9:     /* VID53 */
	case HDMI_720X480P240:          /* VID56 */
	case HDMI_720X480P240_16X9: {   /* VID57 */
			ui_target_freq = 108000000;
		}
		break;

	/* Target frequency 148.5Mhz */
	case HDMI_1920X1080P60:         /* VID16 */
	case HDMI_1920X1080P50:         /* VID31 */
	case HDMI_1920X1080I100:        /* VID40 */
	case HDMI_1280X720P100:         /* VID41 */
	case HDMI_1920X1080I120:        /* VID46 */
	case HDMI_1280X720P120: {       /* VID47 */
			ui_target_freq = 148500000;
		}
		break;

	case HDMI_3840X2160P24:         /* VID93 */
	case HDMI_3840X2160P25:         /* VID94 */
	case HDMI_3840X2160P30:         /* VID95 */
	case HDMI_4096X2160P24:         /* VID98 */
	case HDMI14B_3840X2160P24:
	case HDMI14B_3840X2160P25:
	case HDMI14B_3840X2160P30:
	case HDMI14B_4096X2160P24:
	case HDMI_4096X2160P25:         /* VID99 */
	case HDMI_4096X2160P30: {       /* VID100*/
			ui_target_freq = 297000000;
		}
		break;

	case HDMI_1024X768P60: {
			ui_target_freq = 65000000;
		}
		break;
	case HDMI_1600X1200P60: {
			ui_target_freq = 162000000;
		}
		break;
	case HDMI_2560X1440P60: {
			ui_target_freq = 241500000;
		}
		break;

	case HDMI_3840X2160P60: {       /* VID97*/
			ui_target_freq = 594000000;
		}
		break;		

	default: {
			return TRUE;
		}

	}

	// set clock rate
	dev_io_ctrl.SEL.SET_CLK_FREQ.ui_freq   = ui_target_freq;
	dev_io_ctrl.SEL.SET_CLK_FREQ.b_ycc8bit = FALSE;
	p_dispdev_control(DISPDEV_IOCTRL_SET_CLK_FREQ, &dev_io_ctrl);
	clock_rate = ui_target_freq;

	disphdmi_debug(("HDMI Target Frequency = %d Hertz\r\n", ui_target_freq));
#else

	UINT32                  ui_target_freq = 27000000;
	DISPDEV_IOCTRL          p_dispdev_control = p_hdmi_io_control;
	DISPDEV_IOCTRL_PARAM    dev_io_ctrl;
#if !_FPGA_EMULATION_
	UINT32                  cur_freq;
	PLL_ID                  src_pll = PLL_ID_9;

	disphdmi_debug(("DEVHDMI: chgHdmiClkFreq (%d)\r\n", b_native));

	p_dispdev_control(DISPDEV_IOCTRL_GET_SRCCLK, &dev_io_ctrl);
#if 1

	switch (dev_io_ctrl.SEL.GET_SRCCLK.src_clk) {
	case DISPCTRL_SRCCLK_PLL9: {
			src_pll =                                    PLL_ID_9;
		}
		break;

	case DISPCTRL_SRCCLK_PLL10: {
			src_pll =                                    PLL_ID_10;
		}
		break;


	default: {
			return TRUE;
		}

	}
#endif
	cur_freq              = pll_get_pll_freq_hdmi(src_pll) / 1000000;
	disphdmi_debug(("DEVHDMI: Original pll[%d] src_clk(%d)Mhz\r\n",src_pll, cur_freq));

	if (b_native) {
		g_dispdev_src_freq = cur_freq;
	}

	// Calculate PLL Ratio
	cur_freq = cur_freq * 131072 / 12;

	switch (g_dispdev_hdmi_vid) {

	/*
	    Change PLL9 to 216Mhz
	*/
	// Target 54MHz
	case HDMI_1440X480P60:          /* VID14 */
	case HDMI_1440X480P60_16X9:     /* VID15 */
	case HDMI_1440X576P50:          /* VID29 */
	case HDMI_1440X576P50_16X9:     /* VID30 */
	case HDMI_720X576P100:          /* VID42 */
	case HDMI_720X576P100_16X9:     /* VID43 */
	case HDMI_720X480P120:          /* VID48 */
	case HDMI_720X480P120_16X9:     /* VID49 */
	case HDMI_720X576I200:          /* VID54 */
	case HDMI_720X576I200_16X9:     /* VID55 */
	case HDMI_720X480I240:          /* VID58 */
	case HDMI_720X480I240_16X9:     /* VID59 */
	// Target 108MHz
	case HDMI_1280X1024P60:
	case HDMI_720X576P200:          /* VID52 */
	case HDMI_720X576P200_16X9:     /* VID53 */
	case HDMI_720X480P240:          /* VID56 */
	case HDMI_720X480P240_16X9:    /* VID57 */
#if 1
		if ((cur_freq != HDMI_PLL216) && (src_pll != PLL_ID_1)) {
			dispdev_disable_ide();
			pll_set_driver_pll_hdmi(src_pll,      HDMI_PLL216);
			disphdmi_debug(("DEVHDMI: Chg PLL%d to(%d)Mhz\r\n", (src_pll + 1), pll_get_pll_freq_hdmi(src_pll) / 1000000));
		}
		break;
#endif
	/*
	    Change PLL2 to 252Mhz
	*/
	// Target 25.2MHz
	case HDMI_640X480P60:          /* VID1 */
#if 1
		if ((cur_freq != HDMI_PLL252) && (src_pll != PLL_ID_1)) {
			dispdev_disable_ide();
			pll_set_driver_pll_hdmi(src_pll,      HDMI_PLL252);
			disphdmi_debug(("DEVHDMI: Chg PLL%d to(%d)Mhz\r\n", (src_pll + 1), pll_get_pll_freq_hdmi(src_pll) / 1000000));
		}
#endif
		break;

	/*
	    Change PLL9 to 297Mhz
	*/
	// Target 13.5MHz
	case HDMI_720X480I60:           /* VID6 */
	case HDMI_720X480I60_16X9:      /* VID7 */
	case HDMI_720X240P60:           /* VID8 */
	case HDMI_720X240P60_16X9:      /* VID9 */
	case HDMI_720X576I50:           /* VID21 */
	case HDMI_720X576I50_16X9:      /* VID22 */
	case HDMI_720X288P50:           /* VID23 */
	case HDMI_720X288P50_16X9:      /* VID24 */
	// Target 27MHz
	case HDMI_720X480P60:           /* VID2 */
	case HDMI_720X480P60_16X9:      /* VID3 */
	case HDMI_1440X480I60:          /* VID10 */
	case HDMI_1440X480I60_16X9:     /* VID11 */
	case HDMI_1440X240P60:          /* VID12 */
	case HDMI_1440X240P60_16X9:     /* VID13 */
	case HDMI_720X576P50:           /* VID17 */
	case HDMI_720X576P50_16X9:      /* VID18 */
	case HDMI_1440X576I50:          /* VID25 */
	case HDMI_1440X576I50_16X9:     /* VID26 */
	case HDMI_1440X288P50:          /* VID27 */
	case HDMI_1440X288P50_16X9:     /* VID28 */
	case HDMI_720X576I100:          /* VID44 */
	case HDMI_720X576I100_16X9:     /* VID45 */
	case HDMI_720X480I120:          /* VID50 */
	case HDMI_720X480I120_16X9:     /* VID51 */
	// Target 37.125MHz
	case HDMI_1280X720P30:          /* VID132 */
	case HDMI_1280X720P25:
	// Target 74.25MHz
	case HDMI_1280X720P60:          /* VID4 */
	case HDMI_1920X1080I60:         /* VID5 */
	case HDMI_1280X720P50:          /* VID19 */
	case HDMI_1920X1080I50:         /* VID20 */
	case HDMI_1920X1080P24:         /* VID32 */
	case HDMI_1920X1080P25:         /* VID33 */
	case HDMI_1920X1080P30:         /* VID34 */
	// Target 148.5MHz
	case HDMI_1920X1080P60:         /* VID16 */
	case HDMI_1920X1080P50:         /* VID31 */
	case HDMI_1920X1080I100:        /* VID40 */
	case HDMI_1280X720P100:         /* VID41 */
	case HDMI_1920X1080I120:        /* VID46 */
	case HDMI_1280X720P120:         /* VID47 */
	// Target 594MHz
	case HDMI_3840X2160P24:         /* VID93 */
	case HDMI_3840X2160P25:         /* VID94 */
	case HDMI_3840X2160P30:         /* VID95 */
	case HDMI_3840X2160P60:         /* VID97 */		
	case HDMI_4096X2160P24:         /* VID98 */
	case HDMI14B_3840X2160P24:
	case HDMI14B_3840X2160P25:
	case HDMI14B_3840X2160P30:
	case HDMI14B_4096X2160P24:
	case HDMI_4096X2160P25:         /* VID99 */
	case HDMI_4096X2160P30: {       /* VID100*/
			if ((cur_freq != HDMI_PLL594) && (src_pll != PLL_ID_1)) {
				dispdev_disable_ide();
				pll_set_driver_pll_hdmi(src_pll,      HDMI_PLL594);
				disphdmi_debug(("DEVHDMI: Chg PLL%d to(%d)Mhz\r\n", (src_pll), pll_get_pll_freq_hdmi(src_pll) / 1000000));
			}
		}
		break;

	case HDMI_1024X768P60: {
			if ((cur_freq != HDMI_PLL260) && (src_pll != PLL_ID_1)) {
				dispdev_disable_ide();
				pll_set_driver_pll_hdmi(src_pll,      HDMI_PLL260);
				disphdmi_debug(("DEVHDMI: Chg PLL%d to(%d)Mhz\r\n", (src_pll), pll_get_pll_freq_hdmi(src_pll) / 1000000));
			}
		}
		break;

	case HDMI_1600X1200P60: {
			if ((cur_freq != HDMI_PLL324) && (src_pll != PLL_ID_1)) {
				dispdev_disable_ide();
				pll_set_driver_pll_hdmi(src_pll,      HDMI_PLL324);
				disphdmi_debug(("DEVHDMI: Chg PLL%d to(%d)Mhz\r\n", (src_pll), pll_get_pll_freq_hdmi(src_pll) / 1000000));
			}
		}
		break;
	case HDMI_2560X1440P60: {
			if ((cur_freq != HDMI_PLL241) && (src_pll != PLL_ID_1)) {
				dispdev_disable_ide();
				pll_set_driver_pll_hdmi(src_pll,      HDMI_PLL241);
				disphdmi_debug(("DEVHDMI: Chg PLL%d to(%d)Mhz\r\n", (src_pll), pll_get_pll_freq_hdmi(src_pll) / 1000000));
			}
		}
		break;


	case HDMI_1920X1080I50_VT1250:  /* VID39 */
	default:
		DBG_ERR("No support HDMI mode(%d)\r\n", g_dispdev_hdmi_vid);
		return TRUE;
	}


#endif

	//
	//  Configure IDE Clock frequency according to HDMI Video ID
	//
	switch (g_dispdev_hdmi_vid) {

	/* Target frequency 13.5Mhz */
	case HDMI_720X480I60:          /* VID6 */
	case HDMI_720X480I60_16X9:     /* VID7 */
	case HDMI_720X240P60:          /* VID8 */
	case HDMI_720X240P60_16X9:     /* VID9 */
	case HDMI_720X576I50:          /* VID21 */
	case HDMI_720X576I50_16X9:     /* VID22 */
	case HDMI_720X288P50:          /* VID23 */
	case HDMI_720X288P50_16X9: {   /* VID24 */
			ui_target_freq = 13500000;
		}
		break;

	/* Target frequency 25.2Mhz */
	case HDMI_640X480P60: {         /* VID1 */
#if _FPGA_EMULATION_
			ui_target_freq = 27000000;
#else
			ui_target_freq = 25200000;
#endif
		}
		break;

	/* Target frequency 27Mhz */
	case HDMI_720X480P60:           /* VID2 */
	case HDMI_720X480P60_16X9:      /* VID3 */
	case HDMI_1440X480I60:          /* VID10 */
	case HDMI_1440X480I60_16X9:     /* VID11 */
	case HDMI_1440X240P60:          /* VID12 */
	case HDMI_1440X240P60_16X9:     /* VID13 */
	case HDMI_720X576P50:           /* VID17 */
	case HDMI_720X576P50_16X9:      /* VID18 */
	case HDMI_1440X576I50:          /* VID25 */
	case HDMI_1440X576I50_16X9:     /* VID26 */
	case HDMI_1440X288P50:          /* VID27 */
	case HDMI_1440X288P50_16X9:     /* VID28 */
	case HDMI_720X576I100:          /* VID44 */
	case HDMI_720X576I100_16X9:     /* VID45 */
	case HDMI_720X480I120:          /* VID50 */
	case HDMI_720X480I120_16X9: {   /* VID51 */
			ui_target_freq = 27000000;
		}
		break;

	/* Target frequency 54Mhz */
	case HDMI_1440X480P60:          /* VID14 */
	case HDMI_1440X480P60_16X9:     /* VID15 */
	case HDMI_1440X576P50:          /* VID29 */
	case HDMI_1440X576P50_16X9:     /* VID30 */
	case HDMI_720X576P100:          /* VID42 */
	case HDMI_720X576P100_16X9:     /* VID43 */
	case HDMI_720X480P120:          /* VID48 */
	case HDMI_720X480P120_16X9:     /* VID49 */
	case HDMI_720X576I200:          /* VID54 */
	case HDMI_720X576I200_16X9:     /* VID55 */
	case HDMI_720X480I240:          /* VID58 */
	case HDMI_720X480I240_16X9: {   /* VID59 */
			ui_target_freq = 54000000;
		}
		break;

	/* Target frequency 37.125Mhz */
	case HDMI_1280X720P25:
	case HDMI_1280X720P30: {       /* VID132 */
			ui_target_freq = 37125000;
		}
		break;

	/* Target frequency 74.25Mhz */
	case HDMI_1280X720P60:          /* VID4 */
	case HDMI_1920X1080I60:         /* VID5 */
	case HDMI_1280X720P50:          /* VID19 */
	case HDMI_1920X1080I50:         /* VID20 */
	case HDMI_1920X1080P24:         /* VID32 */
	case HDMI_1920X1080P25:         /* VID33 */
	case HDMI_1920X1080P30: {       /* VID34 */
			ui_target_freq = 74250000;
		}
		break;

	/* Target frequency 108Mhz */
	case HDMI_1280X1024P60:
	case HDMI_720X576P200:          /* VID52 */
	case HDMI_720X576P200_16X9:     /* VID53 */
	case HDMI_720X480P240:          /* VID56 */
	case HDMI_720X480P240_16X9: {   /* VID57 */
			ui_target_freq = 108000000;
		}
		break;

	/* Target frequency 148.5Mhz */
	case HDMI_1920X1080P60:         /* VID16 */
	case HDMI_1920X1080P50:         /* VID31 */
	case HDMI_1920X1080I100:        /* VID40 */
	case HDMI_1280X720P100:         /* VID41 */
	case HDMI_1920X1080I120:        /* VID46 */
	case HDMI_1280X720P120: {       /* VID47 */
			ui_target_freq = 148500000;
		}
		break;

	case HDMI_3840X2160P24:         /* VID93 */
	case HDMI_3840X2160P25:         /* VID94 */
	case HDMI_3840X2160P30:         /* VID95 */
	case HDMI_4096X2160P24:         /* VID98 */
	case HDMI14B_3840X2160P24:
	case HDMI14B_3840X2160P25:
	case HDMI14B_3840X2160P30:
	case HDMI14B_4096X2160P24:
	case HDMI_4096X2160P25:         /* VID99 */
	case HDMI_4096X2160P30: {       /* VID100*/
			ui_target_freq = 297000000;
		}
		break;

	case HDMI_1024X768P60: {
			ui_target_freq = 65000000;
		}
		break;
	case HDMI_1600X1200P60: {
			ui_target_freq = 162000000;
		}
		break;
	case HDMI_2560X1440P60: {
			ui_target_freq = 241500000;
		}
		break;
	case HDMI_3840X2160P60:{         /* VID97 */
			ui_target_freq = 594000000;
		}
		break;
	default: {
			return TRUE;
		}

	}

	// set clock rate
	dev_io_ctrl.SEL.SET_CLK_FREQ.ui_freq   = ui_target_freq;
#if _FPGA_EMULATION_
	dev_io_ctrl.SEL.SET_CLK_FREQ.b_ycc8bit = TRUE;
#else
	dev_io_ctrl.SEL.SET_CLK_FREQ.b_ycc8bit = FALSE;
#endif
	p_dispdev_control(DISPDEV_IOCTRL_SET_CLK_FREQ, &dev_io_ctrl);

	clock_rate = ui_target_freq;

	disphdmi_debug(("HDMI Target Frequency = %d Hertz\r\n", ui_target_freq));

#endif
#endif
	return FALSE;

}

static void dispdev_check_sample_rate(BOOL b_enable)
{
#if 0
#ifndef __KERNEL__
	BOOL                    b_aud_opened;
	AUDIO_SR                aud_sr;

	// Audio must be opened first and can apply set Parameter/Feature.
	b_aud_opened = aud_get_lock_status();
	if (b_aud_opened == FALSE) {
		if (aud_open() != E_OK) {
			disphdmi_debug(("aud op err\r\n"));
		}

	}

	aud_set_feature(AUDIO_FEATURE_CHECK_PLAY_SAMPLING_RATE, b_enable);

	if (b_enable) {
		// Config Audio Check Sampling Rate
		if (g_dispdev_aud_fmt == HDMI_AUDIO32KHZ) {
			aud_sr =  AUDIO_SR_32000;
		} else if (g_dispdev_aud_fmt == HDMI_AUDIO44_1KHZ) {
			aud_sr =  AUDIO_SR_44100;
		} else {
			aud_sr =  AUDIO_SR_48000;
		}
		aud_set_parameter(AUDIO_PARAMETER_CHECKED_PLAY_SAMPLING_RATE, aud_sr);
	}

	// Restore the original aud open status
	if (b_aud_opened == FALSE) {
		aud_close();
	}
#endif
#endif
}

#if 1

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
	#ifndef __KERNEL__
	//PINMUX_LCDINIT          lcd_init;
	#endif
#if DISPDEVIOUSED
	DISPDEV_IOCTRL_PARAM    dev_io_ctrl;
	DISPDEV_IOCTRL          p_dispdev_control = p_hdmi_io_control;

	if (p_dispdev_control == NULL) {
		return E_PAR;
	}

	p_dispdev_control(DISPDEV_IOCTRL_GET_HDMIMODE, &dev_io_ctrl);
	g_dispdev_hdmi_vid = dev_io_ctrl.SEL.GET_HDMIMODE.video_id;
	g_dispdev_aud_fmt  = dev_io_ctrl.SEL.GET_HDMIMODE.audio_id;
#endif
	disphdmi_debug(("DEVHDMI: open (%d)\r\n", g_dispdev_hdmi_vid));

	#ifndef __KERNEL__
	//lcd_init             = pinmux_get_disp_mode(PINMUX_FUNC_ID_HDMI);
	#endif

	if (hdmitx_is_opened() == FALSE) {
	#if DISPDEVIOUSED
		p_dispdev_control(DISPDEV_IOCTRL_GET_ENABLE, &dev_io_ctrl);
		if (dev_io_ctrl.SEL.GET_ENABLE.b_en == TRUE) {
			dev_io_ctrl.SEL.SET_ENABLE.b_en = FALSE;
			p_dispdev_control(DISPDEV_IOCTRL_SET_ENABLE,    &dev_io_ctrl);
			p_dispdev_control(DISPDEV_IOCTRL_WAIT_FRAMEEND, NULL);
		}
	#endif
		// Open HDMI-Tx & Read EDID
		if (hdmitx_open() != E_OK) {
			//disphdmi_debug(("DEVHDMI: open fail(%d)\r\n", hdmi_cfg.vid));
			disphdmi_debug(("DEVHDMI: open fail (%d)\r\n", g_dispdev_hdmi_vid));
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
			hdmitx_close();
			return E_NOSPT;
		}
#if DISPDEVIOUSED
		// Enable IDE clock
		dev_io_ctrl.SEL.SET_CLK_EN.b_clk_en = TRUE;
		p_dispdev_control(DISPDEV_IOCTRL_SET_CLK_EN, &dev_io_ctrl);
#endif

		// init HDMI phy after clock ready
		hdmitx_set_config(HDMI_CONFIG_ID_PHY_INIT, TRUE);

		hdmitx_set_config(HDMI_CONFIG_ID_VIDEO, g_dispdev_hdmi_vid);
		hdmitx_set_config(HDMI_CONFIG_ID_AUDIO, g_dispdev_aud_fmt);
		hdmitx_set_config(HDMI_CONFIG_ID_SOURCE, g_dispdev_src_fmt);

		// Configure VID content
		if (dispdev_init_hdmi_vid_format(g_dispdev_hdmi_vid) == FALSE) {
			hdmitx_close();
			return E_NOSPT;
		}

		// Configure IDE
		dispdev_init_hdmi_display();
#if DISPDEVIOUSED
		dev_io_ctrl.SEL.SET_DISPSIZE.ui_buf_width     = (disp_video_timing[g_dispdev_hdmi_index].H_ENDTIME - disp_video_timing[g_dispdev_hdmi_index].H_STARTTIME);
		dev_io_ctrl.SEL.SET_DISPSIZE.ui_buf_height    = (disp_video_timing[g_dispdev_hdmi_index].V_ODD_ENDTIME - disp_video_timing[g_dispdev_hdmi_index].V_ODD_STARTTIME);
		dev_io_ctrl.SEL.SET_DISPSIZE.ui_win_width     = (disp_video_timing[g_dispdev_hdmi_index].H_ENDTIME - disp_video_timing[g_dispdev_hdmi_index].H_STARTTIME) - g_dispdev_win_hozi_ofs;
		dev_io_ctrl.SEL.SET_DISPSIZE.ui_win_height    = (disp_video_timing[g_dispdev_hdmi_index].V_ODD_ENDTIME - disp_video_timing[g_dispdev_hdmi_index].V_ODD_STARTTIME) - g_dispdev_win_vert_ofs;
		p_dispdev_control(DISPDEV_IOCTRL_SET_DISPSIZE, &dev_io_ctrl);

		dev_io_ctrl.SEL.SET_ENABLE.b_en = TRUE;
		p_dispdev_control(DISPDEV_IOCTRL_SET_ENABLE, &dev_io_ctrl);
		p_dispdev_control(DISPDEV_IOCTRL_WAIT_FRAMEEND, NULL);
#endif
		hdmitx_check_video_stable();
		hdmitx_set_config(HDMI_CONFIG_ID_OE,             TRUE);
		hdmitx_set_config(HDMI_CONFIG_ID_AUDIO_STREAM_EN, TRUE);
#ifdef __KERNEL__
		printk("hdmitx: audio stream enable. \n");
#endif
		hdmitx_set_config(HDMI_CONFIG_ID_SCDC_ENABLE,    ENABLE);

	} else {
		//
		// HDMI is already opened, Just changing HDMI format
		//
		hdmitx_set_config(HDMI_CONFIG_ID_AV_MUTE, TRUE);
		//p_dispdev_control(DISPDEV_IOCTRL_WAIT_FRAMEEND, NULL);

		hdmitx_set_config(HDMI_CONFIG_ID_AUDIO_STREAM_EN, FALSE);
		//p_dispdev_control(DISPDEV_IOCTRL_WAIT_FRAMEEND, NULL);

		if (hdmitx_get_config(HDMI_CONFIG_ID_VERIFY_VIDEOID)) {
			hdmitx_verify_video_id(&g_dispdev_hdmi_vid);
		}

		// Check & Change IDE Source frequency
		if (dispdev_chg_hdmi_clk_freq(FALSE)) {
			hdmitx_close();
			return E_NOSPT;
		}

		// Enable IDE clock
		#if DISPDEVIOUSED
		dev_io_ctrl.SEL.SET_CLK_EN.b_clk_en = TRUE;
		p_dispdev_control(DISPDEV_IOCTRL_SET_CLK_EN, &dev_io_ctrl);
		#endif

		// Configure VID content
		if (dispdev_init_hdmi_vid_format(g_dispdev_hdmi_vid) == FALSE) {
			hdmitx_close();
			return E_NOSPT;
		}

		// Configure IDE
		dispdev_init_hdmi_display();
		#if DISPDEVIOUSED
		dev_io_ctrl.SEL.SET_DISPSIZE.ui_buf_width     = (disp_video_timing[g_dispdev_hdmi_index].H_ENDTIME - disp_video_timing[g_dispdev_hdmi_index].H_STARTTIME);
		dev_io_ctrl.SEL.SET_DISPSIZE.ui_buf_height    = (disp_video_timing[g_dispdev_hdmi_index].V_ODD_ENDTIME - disp_video_timing[g_dispdev_hdmi_index].V_ODD_STARTTIME);
		dev_io_ctrl.SEL.SET_DISPSIZE.ui_win_width     = (disp_video_timing[g_dispdev_hdmi_index].H_ENDTIME - disp_video_timing[g_dispdev_hdmi_index].H_STARTTIME) - g_dispdev_win_hozi_ofs;
		dev_io_ctrl.SEL.SET_DISPSIZE.ui_win_height    = (disp_video_timing[g_dispdev_hdmi_index].V_ODD_ENDTIME - disp_video_timing[g_dispdev_hdmi_index].V_ODD_STARTTIME) - g_dispdev_win_vert_ofs;
		p_dispdev_control(DISPDEV_IOCTRL_SET_DISPSIZE, &dev_io_ctrl);
		#endif
		// Open HDMI-Tx
		hdmitx_set_config(HDMI_CONFIG_ID_VIDEO, g_dispdev_hdmi_vid);
		hdmitx_set_config(HDMI_CONFIG_ID_AUDIO, g_dispdev_aud_fmt);
		hdmitx_set_config(HDMI_CONFIG_ID_SOURCE, g_dispdev_src_fmt);
		#if DISPDEVIOUSED
		p_dispdev_control(DISPDEV_IOCTRL_SET_LOAD,      NULL);

		dev_io_ctrl.SEL.SET_ENABLE.b_en = TRUE;
		p_dispdev_control(DISPDEV_IOCTRL_SET_ENABLE, &dev_io_ctrl);
		p_dispdev_control(DISPDEV_IOCTRL_WAIT_FRAMEEND, NULL);
		#endif
		
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
#if DISPDEVIOUSED
	DISPDEV_IOCTRL_PARAM    dev_io_ctrl;
	DISPDEV_IOCTRL          p_dispdev_control = p_hdmi_io_control;
#ifdef __KERNEL__
	struct clk *pll_clk = NULL;
#else
#if !_FPGA_EMULATION_
	UINT32                  cur_freq;
	PLL_ID                  src_pll = PLL_ID_2;
#endif
#endif
#endif
	if (hdmitx_is_opened() == FALSE) {
		return E_OK;
	}

	disphdmi_debug(("DEVHDMI: close\r\n"));

	hdmitx_set_config(HDMI_CONFIG_ID_OE, FALSE);
	// Close HDMI
	hdmitx_close();
#if DISPDEVIOUSED
	// Disable IDE
	dev_io_ctrl.SEL.SET_ENABLE.b_en = FALSE;
	p_dispdev_control(DISPDEV_IOCTRL_SET_ENABLE, &dev_io_ctrl);
	p_dispdev_control(DISPDEV_IOCTRL_WAIT_FRAMEEND, NULL);

	dev_io_ctrl.SEL.SET_OUT_COMPONENT.comp0    = IDE_COMPONENT_R;
	dev_io_ctrl.SEL.SET_OUT_COMPONENT.comp1    = IDE_COMPONENT_G;
	dev_io_ctrl.SEL.SET_OUT_COMPONENT.comp2    = IDE_COMPONENT_B;
	dev_io_ctrl.SEL.SET_OUT_COMPONENT.b_bit_swap = FALSE;
	dev_io_ctrl.SEL.SET_OUT_COMPONENT.b_length  = FALSE;
	p_dispdev_control(DISPDEV_IOCTRL_SET_OUT_COMPONENT, &dev_io_ctrl);

	dev_io_ctrl.SEL.SET_CONST_OFS.ui_win_const_ofs_x = 0;
	dev_io_ctrl.SEL.SET_CONST_OFS.ui_win_const_ofs_y = 0;
	p_dispdev_control(DISPDEV_IOCTRL_SET_CONST_OFS, &dev_io_ctrl);

	// Close IDE clock
	dev_io_ctrl.SEL.SET_CLK_EN.b_clk_en = FALSE;
	p_dispdev_control(DISPDEV_IOCTRL_SET_CLK_EN, &dev_io_ctrl);

#ifdef __KERNEL__

	p_dispdev_control(DISPDEV_IOCTRL_GET_SRCCLK, &dev_io_ctrl);
	switch (dev_io_ctrl.SEL.GET_SRCCLK.src_clk) {
	case DISPCTRL_SRCCLK_PLL1:
	//case DISPCTRL_SRCCLK_IDE2_PLL1:
		break;
#if 0/*todo chk*/
	case DISPCTRL_SRCCLK_PLL2:
	//case DISPCTRL_SRCCLK_IDE2_PLL2: {
			pll_clk = clk_get(NULL, "pll2");
	//	}
		break;

	case DISPCTRL_SRCCLK_PLL4:
	//case DISPCTRL_SRCCLK_IDE2_PLL4: {
			pll_clk = clk_get(NULL, "pll4");
	//	}
		break;
#endif
	default:
		break;
	}

	if (pll_clk != NULL) {
		if (IS_ERR(pll_clk)) {
			DBG_ERR("get pll failed\r\n");
			return E_SYS;
		}
		clk_set_rate(pll_clk, g_dispdev_src_freq);
		clk_put(pll_clk);
	}

#else

#if !_FPGA_EMULATION_
	p_dispdev_control(DISPDEV_IOCTRL_GET_SRCCLK, &dev_io_ctrl);
	switch (dev_io_ctrl.SEL.GET_SRCCLK.src_clk) {
	case DISPCTRL_SRCCLK_PLL1:
	//case DISPCTRL_SRCCLK_IDE2_PLL1: {
			src_pll = PLL_ID_1;
	//	}
		break;
#if 0 /*todo chk*/
	case DISPCTRL_SRCCLK_PLL2:
	//case DISPCTRL_SRCCLK_IDE2_PLL2: {
			src_pll = PLL_ID_2;
	//	}
		break;

	case DISPCTRL_SRCCLK_PLL4:
	//case DISPCTRL_SRCCLK_IDE2_PLL4: {
			src_pll = PLL_ID_4;
	//	}
		break;

	case DISPCTRL_SRCCLK_PLL14:
	//case DISPCTRL_SRCCLK_IDE2_PLL14: {
			src_pll = PLL_ID_14;
	//	}
		break;
#endif
	default:
		break;
	}

	cur_freq = pll_get_pll_freq_hdmi(src_pll) / 1000000;
	if ((cur_freq != g_dispdev_src_freq) && (src_pll != PLL_ID_1)) {
		cur_freq = g_dispdev_src_freq * 131072 / 12;

		//pll_set_driver_pll_hdmi_en(src_pll,    FALSE);
		pll_set_driver_pll_hdmi(src_pll,      cur_freq);
		//pll_set_driver_pll_hdmi_en(src_pll,    TRUE);
	}
#endif
#endif
#endif
	dispdev_check_sample_rate(DISABLE);

	return E_OK;
}

/*
    Set HDMI Display Device IOCTRL function pointer
*/
static void dispdev_set_hdmi_ioctrl(FP p_io_ctrl_func)
{
#if DISPDEVIOUSED
	p_hdmi_io_control = (DISPDEV_IOCTRL)p_io_ctrl_func;
#endif
}

/*
    PreGet HDMI size

    PreGet HDMI size

    @return void
*/
static ER dispdev_get_hdmi_size(DISPDEV_GET_PRESIZE *t_size)
{
	UINT32                  idx = 0;
#if DISPDEVIOUSED
	DISPDEV_IOCTRL_PARAM    dev_io_ctrl;
	DISPDEV_IOCTRL          p_dispdev_control = p_hdmi_io_control;


	disphdmi_debug(("getHDMISize START\r\n"));

	if (p_dispdev_control == NULL) {
		return E_PAR;
	}

	//
	//Search the mapping Video ID index
	//
	p_dispdev_control(DISPDEV_IOCTRL_GET_HDMIMODE, &dev_io_ctrl);
	for (idx = 0; idx < VIDMAX; idx++) {
		if (disp_video_timing[idx].VID == dev_io_ctrl.SEL.GET_HDMIMODE.video_id) {
			break;
		}
	}
#endif

	if (idx < VIDMAX) {
		t_size->ui_buf_width   = (disp_video_timing[idx].H_ENDTIME - disp_video_timing[idx].H_STARTTIME);
		t_size->ui_buf_height  = (disp_video_timing[idx].V_ODD_ENDTIME - disp_video_timing[idx].V_ODD_STARTTIME);
	} else {
		t_size->ui_buf_width   = 0;
		t_size->ui_buf_height  = 0;
	}


	return E_OK;
}

#endif

/*
    Set HDMI Display config before open
*/
static ER dispdev_preconfighdmi(PDISP_HDMI_CFG config)
{
	disphdmi_debug(("dispdev_preconfighdmi \r\n"));
	g_dispdev_aud_fmt = (HDMI_AUDIOFMT)config->audio_fmt;
	g_dispdev_hdmi_vid = (HDMI_VIDEOID)config->vid;
	g_dispdev_hdmi_mode = (HDMI_MODE)config->output_mode;
	g_dispdev_src_fmt = (HDMI_SRCFMT)config->source;

	return E_OK;
}
/**
    Get HDMI Display Device Object

    Get HDMI Display Device Object. This Object is used to control the HDMI Transmitter.

    @return The pointer of HDMI Display Device object.
*/
PDISPDEV_OBJ dispdev_get_hdmi_dev_obj(void)
{
	disphdmi_debug(("dispdev_get_hdmi_dev_obj \r\n"));
	return &dispdev_hdmi_obj;
}

#if DISPDEVIOUSED
PDISPDEVHDMI_CTRL dispdev_get_hdmi_dev_control(void)
{
	return &dispdevhdmi_control;
}
#endif

void  hdmitx_module_init(void)
{
#if DISPDEVIOUSED	
	PDISP_OBJ p_disp_obj;

	p_disp_obj = disp_get_display_object(DISP_1);

	p_disp_obj->phdmidev_control = dispdev_get_hdmi_dev_control();

	p_disp_obj = disp_get_display_object(DISP_2);

	p_disp_obj->phdmidev_control = dispdev_get_hdmi_dev_control();
#endif
	return;
}




