/*
    HDMI module internal defintion header

    HDMI module internal defintion header.

    @file       hdmitx_int.h
    @ingroup    mIDrvDisp_HDMITx
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/

#ifndef  _HDMITX_INT_H
#define  _HDMITX_INT_H

#include "hdmi_port.h"
#include "hdmitx.h"

/*
    @addtogroup mIDrvDisp_HDMITx
*/
//@{

#define HDMITX_SCDC_SUPPORT         DISABLE
#define HDMITX_SCDC_READREQUEST     DISABLE
#define HDMITX_SCDC_RRTEST          DISABLE
#define HDMITX_DEBUG                DISABLE
#define HDMICEC_DEBUG               DISABLE
#define HDMITX_DEBUG_FORCE_DVI      DISABLE
#if HDMITX_DEBUG
#define hdmitx_debug(msg)           DBG_WRN msg

#define HDMITX_DEBUG_FORCE_RGB      DISABLE
#define HDMITX_DEBUG_FORCE_YUV      DISABLE
#define HDMITX_DEBUG_FORCE_YUV422   DISABLE
#define HDMITX_DEBUG_FORCE_OVSCAN   DISABLE
#define HDMITX_DEBUG_SKIP_EDID      ENABLE
#else
#define hdmitx_debug(msg)
#endif

#if HDMICEC_DEBUG
#define hdmicec_debug(msg)          printf(msg)
#else
#define hdmicec_debug(msg)
#endif

#define HDMITX_CHECK_TIMEOUT        0x1800000

#define HDMITX_HWDDC_SPEED          80000 // DDC must not exceed 100KHz

#define HDMITX_SWDDC_SPEED          10000 // SW DDC fixed using 10KHz only
#define HDMITX_SWDDC_DELAY          (1000000/(HDMITX_SWDDC_SPEED)/2)

// Define for hdmi resist trimming
#define HDMITX_RTRIM_DEFAULT        0xE
#define HDMITX_RTRIM_EFUSE_MASK     0x1F


// Hidden definition of "HDMI_3DFMT"
#define HDMI3D_FRAMEPACKING         0x04


#define HDMI2_SCDC_VERSION              0x01
#define HDMI2_SCDC_SLAVEADDR            0xA8

// HDMI 2.0 VSDB Capability
#define HDMI2VSDB_VERSION_MASK          0x000000FF
#define HDMI2VSDB_SPEED_MASK            0x0000FF00
#define HDMI2VSDB_3DOSD_DISPARITY       0x00010000
#define HDMI2VSDB_DUALVIEW              0x00020000
#define HDMI2VSDB_INDEPENDENTVIEW       0x00040000
#define HDMI2VSDB_LTE340MSCRAMBLE       0x00080000
#define HDMI2VSDB_RR_CAP                0x00400000
#define HDMI2VSDB_SCDC_PRESENT          0x00800000
#define HDMI2VSDB_DC30YUV420            0x01000000
#define HDMI2VSDB_DC36YUV420            0x02000000
#define HDMI2VSDB_DC48YUV420            0x04000000

//
//  HDMI Parser Error Code
//
#define HDMITX_ER_BADHEADER             1
#define HDMITX_ER_NOSUPPORT_VERSION     2
#define HDMITX_ER_CRC                   3
#define HDMITX_ER_TAGCODE               4


#define HDMITX_EDID_BLKSIZE             128
#define HDMITX_EDIDNUM                  31  // Max Data Block Size is 31

typedef enum {
	HDMICFG_OPTION_FORCE_RGB,
	HDMICFG_OPTION_FORCE_UNDERSCAN,
	HDMICFG_OPTION_VERIFY_VID,

	HDMICFG_OPTION_ALL,
	ENUM_DUMMY4WORD(HDMICFG_OPTION)
} HDMICFG_OPTION;



typedef enum {
	HDMIINFO_EN_AVI        = 0x0002,
	HDMIINFO_EN_AUD        = 0x0020,
	HDMIINFO_EN_GEN1       = 0x0200,
	HDMIINFO_EN_GEN2       = 0x2000,
	HDMIINFO_EN_GCP        = 0x0800,

	ENUM_DUMMY4WORD(HDMIINFO_EN)
} HDMIINFO_EN;

typedef enum {
	HDMIINFO_RPT_AVI        = 0x0001,
	HDMIINFO_RPT_AUD        = 0x0010,
	HDMIINFO_RPT_GEN1       = 0x0100,
	HDMIINFO_RPT_GEN2       = 0x1000,
	HDMIINFO_RPT_GCP        = 0x0400,

	ENUM_DUMMY4WORD(HDMIINFO_RPT)
} HDMIINFO_RPT;


typedef enum {
	HDMI_INTF_RGB444 = 0, //    HDMI Output format is RGB-444
	HDMI_INTF_YCBCR422,  //    HDMI Output format is YCbCr-422
	HDMI_INTF_YCBCR444,  //    HDMI Output format is YCbCr-444

	ENUM_DUMMY4WORD(HDMI_INTF_MODE)
} HDMI_INTF_MODE;



typedef struct {
	ER(*open)(PI2C_SESSION p_session);
	ER(*close)(I2C_SESSION session);

//#if (defined __UITRON || defined __ECOS)//#ifndef __KERNEL__

	ER(*lock)(I2C_SESSION session);
	ER(*unlock)(I2C_SESSION session);
//#endif

	void (*set_config)(I2C_SESSION session, I2C_CONFIG_ID config_id, UINT32 ui_config);

//#if (defined __UITRON || defined __ECOS)//#ifndef __KERNEL__

	I2C_STS(*transmit)(PI2C_DATA p_data);
	I2C_STS(*receive)(PI2C_DATA p_data);
} HDMI_I2C_OBJ, *PHDMI_I2C_OBJ;




#define HDMI_TCLK_1X                                0x01
#define HDMI_TCLK_2X                                0x02

#define HDMI_NVAL6144                               0x1800
#define HDMI_NVAL6272                               0x1880
#define HDMI_NVAL4096                               0x1000

//
//  PCM Header Definition
//
#define HDMI_PCM_FS44_1KHZ                          0x0
#define HDMI_PCM_FS48KHZ                            0x2
#define HDMI_PCM_FS32KHZ                            0x3


//
//  InfoFrame Definitions
//
#define HDMIINFO_AVI_TYPE                           0x82
#define HDMIINFO_AVI_VER                            0x02
#define HDMIINFO_AVI_LEN                            0x0D

#define HDMIINFO_AVI_16_9_SAMEASPIC_ASPECTRATIO		0x28 /* default is 16:9 */
#define HDMIINFO_AVI_D1_SAMEASPIC_ASPECTRATIO       0x08

#define HDMIINFO_AUD_TYPE                           0x84
#define HDMIINFO_AUD_VER                            0x01
#define HDMIINFO_AUD_LEN                            0x0A
#define HDMIINFO_AUD_D1_CHCNT_2CH                   0x01
#define HDMIINFO_AUD_REFER_TO_STREAM_HEADER         0x00

#define HDMIINFO_VSI_TYPE                           0x81
#define HDMIINFO_VSI_VER                            0x01

#define HDMIINFO_VSI_DB1                            0x03
#define HDMIINFO_VSI_DB2                            0x0C
#define HDMIINFO_VSI_DB3                            0x00
#define HDMIINFO_VSI_3D_PRESENT                     0x40
#define HDMIINFO_VSI_SIDEBYSIDE_HALF                0x80
#define HDMIINFO_VSI_TOP_N_BOTTOM                   0x60
#define HDMIINFO_VSI_HORI_SUBSAMPLE                 0x00

// DDC Usage
#define HDMI_DDCSLAVE_ADDR                          0xA0


#define HDMI_SETREG(ofs, value)     OUTW((IOADDR_HDMI_REG_BASE+(ofs)), (value))
#define HDMI_GETREG(ofs)            INW(IOADDR_HDMI_REG_BASE+(ofs))

#define CEC_SETREG(ofs, value)      OUTW((IOADDR_HDMI_REG_BASE+0x400+(ofs)), (value))
#define CEC_GETREG(ofs)             INW(IOADDR_HDMI_REG_BASE+0x400+(ofs))

extern unsigned int skip_edid;
extern unsigned int print_edid;

//
//  hdmitx.c
//
extern BOOL			 b_hdmi_edid_msg;
extern void          hdmitx_set_int_en(HDMI_INT_TYPE int_type, BOOL en);
extern HDMI_INT_TYPE hdmitx_wait_flag(HDMI_INT_TYPE waiting_int);
extern void          hdmitx_enable_infofrm_tx(HDMIINFO_EN  info_en, HDMIINFO_RPT info_rpt_en);
extern void          hdmitx_disable_infofrm_tx(HDMIINFO_EN  info_dis, HDMIINFO_RPT info_rpt_dis);

extern void          hdmitx_start_trimming(void);
extern UINT32        hdmitx_get_trimming_data(void);

#if _FPGA_EMULATION_
extern void hdmitx_set_pg_enable(BOOL b_en);
extern void hdmitx_set_pg_pattern(UINT32 ui_pat_id);
extern void hdmitx_set_pg_1(UINT32 gray, UINT32 HTOT);
extern void hdmitx_set_pg_2(UINT32 HSW, UINT32 HAB);
extern void hdmitx_set_pg_3(UINT32 HAW, UINT32 VTOT);
extern void hdmitx_set_pg_4(UINT32 VSW, UINT32 VAB);
extern void hdmitx_set_pg_5(UINT32 VAW);
extern void hdmitx_reset_sil9002(void);
#endif

//
//  hdmitx_edid.c
//
extern void     hdmitx_init_ddc(BOOL b_ddc_en);
extern ER       hdmitx_scdc_write(UINT32 offset, UINT32 data);
extern ER       hdmitx_scdc_read(UINT32 offset, UINT32 *pdata);
extern UINT32   hdmitx_get_hdmi_2_vsdb_cap(void);

//
//  hdmitx_ddc.c
//
extern ER       hdmiddc_open(PI2C_SESSION p_session);
extern ER       hdmiddc_close(I2C_SESSION session);
extern ER       hdmiddc_lock(I2C_SESSION session);
extern ER       hdmiddc_unlock(I2C_SESSION session);
extern void     hdmiddc_set_config(I2C_SESSION session, I2C_CONFIG_ID config_id, UINT32 ui_config);
extern I2C_STS  hdmiddc_transmit(PI2C_DATA p_data);
extern I2C_STS  hdmiddc_receive(PI2C_DATA p_data);

//@}

#endif
