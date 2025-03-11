/*
	TOP controller header

	Sets the pinmux of each module.

	@file       top.h
	@ingroup    mIDrvSys_TOP
	@note       Refer NA51102 data sheet for PIN/PAD naming

	Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/

#ifndef __ASM_ARCH_NA51102_TOP_H
#define __ASM_ARCH_NA51102_TOP_H

#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/rcw_macro.h>
#if !defined(__LINUX)
#include <stdio.h>
#endif

/**
    Debug port select ID

    Debug port select value for pinmux_select_debugport().
*/
typedef enum {
	PINMUX_DEBUGPORT_CKG =          0x0000,     ///< CKGen
	PINMUX_DEBUGPORT_MAU =          0x0001,     ///< MAU
	PINMUX_DEBUGPORT_GPIO =         0x0002,     ///< GPIO
	PINMUX_DEBUGPORT_CPU =          0x0003,     ///< CPU
	PINMUX_DEBUGPORT_TIMER =        0x0004,     ///< Timer
	PINMUX_DEBUGPORT_TIMER2 =       0x0005,     ///< Timer2
	PINMUX_DEBUGPORT_TIMER3 =       0x0006,     ///< Timer3
	PINMUX_DEBUGPORT_CC =           0x0007,     ///< CC
	PINMUX_DEBUGPORT_WDT =          0x0008,     ///< WDT
	PINMUX_DEBUGPORT_INTC =         0x0009,     ///< INTC
	PINMUX_DEBUGPORT_PWM =          0x000A,     ///< PWM
	PINMUX_DEBUGPORT_REMOTE =       0x000B,     ///< Remote
	PINMUX_DEBUGPORT_TRNG =         0x000C,     ///< TRNG
	PINMUX_DEBUGPORT_SDIO =         0x000D,     ///< SDIO
	PINMUX_DEBUGPORT_SDIO2 =        0x000E,     ///< SDIO2
	PINMUX_DEBUGPORT_SDIO3 =        0x000F,     ///< SDIO3
	PINMUX_DEBUGPORT_SMC =          0x0010,     ///< NAND/SMC
	PINMUX_DEBUGPORT_DAI =          0x0011,     ///< DAI
	PINMUX_DEBUGPORT_DAI2 =         0x0012,     ///< DAI2
	PINMUX_DEBUGPORT_AUDIO =        0x0013,     ///< AUDIO
	PINMUX_DEBUGPORT_TSE =          0x0014,     ///< TSE
	PINMUX_DEBUGPORT_IFE =          0x0015,     ///< IFE
	PINMUX_DEBUGPORT_UART2 =        0x0016,     ///< UART2
	PINMUX_DEBUGPORT_UART3 =        0x0017,     ///< UART3
	PINMUX_DEBUGPORT_UART4 =        0x0018,     ///< UART4
	PINMUX_DEBUGPORT_UART5 =        0x0019,     ///< UART5
	PINMUX_DEBUGPORT_UART6 =        0x001A,     ///< UART6
	PINMUX_DEBUGPORT_UART7 =        0x001B,     ///< UART7
	PINMUX_DEBUGPORT_UART8 =        0x001C,     ///< UART8
	PINMUX_DEBUGPORT_UART9 =        0x001D,     ///< UART9
	PINMUX_DEBUGPORT_I2C =     	    0x001E,     ///< I2C
	PINMUX_DEBUGPORT_I2C2 =         0x001F,     ///< I2C2
	PINMUX_DEBUGPORT_I2C3 =         0x0020,     ///< I2C3
	PINMUX_DEBUGPORT_I2C4 =         0x0021,     ///< I2C4
	PINMUX_DEBUGPORT_I2C5 =         0x0022,     ///< I2C5
	PINMUX_DEBUGPORT_I2C6 =         0x0023,     ///< I2C6
	PINMUX_DEBUGPORT_I2C7 =         0x0024,     ///< I2C7
	PINMUX_DEBUGPORT_I2C8 =         0x0025,     ///< I2C8
	PINMUX_DEBUGPORT_I2C9 =         0x0026,     ///< I2C9
	PINMUX_DEBUGPORT_I2C10 =        0x0027,     ///< I2C10
	PINMUX_DEBUGPORT_I2C11 =        0x0028,     ///< I2C11
	PINMUX_DEBUGPORT_SPI =          0x0029,     ///< SPI
	PINMUX_DEBUGPORT_SPI2 =         0x002A,     ///< SPI2
	PINMUX_DEBUGPORT_SPI3 =         0x002B,     ///< SPI3
	PINMUX_DEBUGPORT_SPI4 =         0x002C,     ///< SPI4
	PINMUX_DEBUGPORT_SPI5 =         0x002D,     ///< SPI5
	PINMUX_DEBUGPORT_SIF =          0x002E,     ///< SIF
	PINMUX_DEBUGPORT_DRTC =         0x002F,     ///< DRTC
	PINMUX_DEBUGPORT_SDP =          0x0030,     ///< SDP
	PINMUX_DEBUGPORT_ADM =          0x0031,     ///< ADM
	PINMUX_DEBUGPORT_TSEN =         0x0032,     ///< TSEN
	PINMUX_DEBUGPORT_HASH =         0x0033,     ///< HASH
	PINMUX_DEBUGPORT_SCE =          0x0034,     ///< SCE
	PINMUX_DEBUGPORT_HWCP =         0x0035,     ///< HWCP
	PINMUX_DEBUGPORT_RSA =          0x0036,     ///< RSA
	PINMUX_DEBUGPORT_DSI =          0x0037,     ///< DSI
	PINMUX_DEBUGPORT_CSITX =        0x0038,     ///< CSITX
	PINMUX_DEBUGPORT_MPU =          0x0039,     ///< MPU
	PINMUX_DEBUGPORT_USB =          0x003A,     ///< USB
	PINMUX_DEBUGPORT_USB3 =         0x003B,     ///< USB3
	PINMUX_DEBUGPORT_ETH =          0x003C,     ///< ETH
	PINMUX_DEBUGPORT_HVYLD =        0x003D,     ///< HVYLD
	PINMUX_DEBUGPORT_UVCP =         0x003E,     ///< UVCP
	PINMUX_DEBUGPORT_UVCP2 =        0x003F,     ///< UVCP2
	PINMUX_DEBUGPORT_EFUSE =        0x0040,     ///< EFUSE
	PINMUX_DEBUGPORT_SATA =         0x0041,     ///< SATA
	PINMUX_DEBUGPORT_IDE =          0x0042,     ///< IDE
	PINMUX_DEBUGPORT_IDE2 =         0x0043,     ///< IDE2
	PINMUX_DEBUGPORT_HVYLD2 =       0x0044,     ///< HVYLD2
	PINMUX_DEBUGPORT_CSIRX =        0x0045,     ///< CSIRX
	PINMUX_DEBUGPORT_CSIRX2 =       0x0046,     ///< CSIRX2
	PINMUX_DEBUGPORT_CSIRX3 =       0x0047,     ///< CSIRX3
	PINMUX_DEBUGPORT_CSIRX4 =       0x0048,     ///< CSIRX4
	PINMUX_DEBUGPORT_CSIRX5 =       0x0049,     ///< CSIRX5
	PINMUX_DEBUGPORT_SIE =          0x004A,     ///< SIE
	PINMUX_DEBUGPORT_SIE2 =         0x004B,     ///< SIE2
	PINMUX_DEBUGPORT_SIE3 =         0x004C,     ///< SIE3
	PINMUX_DEBUGPORT_SIE4 =         0x004D,     ///< SIE4
	PINMUX_DEBUGPORT_SIE5 =         0x004E,     ///< SIE5
	PINMUX_DEBUGPORT_SIE6 =         0x004F,     ///< SIE6
	PINMUX_DEBUGPORT_VIE =          0x0050,     ///< VIE
	PINMUX_DEBUGPORT_VIE2 =         0x0051,     ///< VIE2
	PINMUX_DEBUGPORT_TGE =          0x0052,     ///< TGE
	PINMUX_DEBUGPORT_VPE =          0x0053,     ///< VPE
	PINMUX_DEBUGPORT_VPEL =         0x0054,     ///< VPEL
	PINMUX_DEBUGPORT_JPEG =         0x0055,     ///< JPEG
	PINMUX_DEBUGPORT_JPEG2 =        0x0056,     ///< JPEG2
	PINMUX_DEBUGPORT_GRAPHIC =      0x0057,     ///< GRAPHIC
	PINMUX_DEBUGPORT_GRAPHIC2 =     0x0058,     ///< GRAPHIC2
	PINMUX_DEBUGPORT_GRAPHIC3 =     0x0059,     ///< GRAPHIC3
	PINMUX_DEBUGPORT_ISE =          0x005A,     ///< ISE
	/// < 005B Reserved
	PINMUX_DEBUGPORT_MDBC =         0x005C,     ///< MDBC
	PINMUX_DEBUGPORT_DIS =          0x005D,     ///< DIS
	PINMUX_DEBUGPORT_DRE =          0x005E,     ///< DRE
	PINMUX_DEBUGPORT_IVE =          0x005F,     ///< IVE
	PINMUX_DEBUGPORT_SDE =          0x0060,     ///< SDE
	PINMUX_DEBUGPORT_TRKE =         0x0061,     ///< TRKE
	PINMUX_DEBUGPORT_CNN =          0x0062,     ///< CNN
	PINMUX_DEBUGPORT_CNN2 =         0x0063,     ///< CNN2
	PINMUX_DEBUGPORT_DCE =          0x0064,     ///< DCE
	PINMUX_DEBUGPORT_NUE =          0x0065,     ///< NUE
	PINMUX_DEBUGPORT_NUE2 =         0x0066,     ///< NUE2
	PINMUX_DEBUGPORT_IPE =          0x0067,     ///< IPE
	PINMUX_DEBUGPORT_IME =          0x0068,     ///< IME
	PINMUX_DEBUGPORT_HVYLD3 =       0x0069,     ///< HVYLD3
	PINMUX_DEBUGPORT_HDMI =         0x006A,     ///< HDMI
	PINMUX_DEBUGPORT_DSP =          0x006B,     ///< DSP
	PINMUX_DEBUGPORT_PMC =          0x006C,     ///< PMC
	PINMUX_DEBUGPORT_ETH2 =         0x006D,     ///< ETH2
	PINMUX_DEBUGPORT_STBCG =        0x006E,     ///< STBCG
	PINMUX_DEBUGPORT_VDEC =         0x006F,     ///< VDEC
	PINMUX_DEBUGPORT_PAD =          0x0070,     ///< PAD
	PINMUX_DEBUGPORT_JPEGL =        0x0071,     ///< JPEGL

	PINMUX_DEBUGPORT_GROUP_NONE =   0x0000,     ///< No debug port is output
	PINMUX_DEBUGPORT_GROUP1 =       0x0100,     ///< Output debug port to MC[18..0]
	PINMUX_DEBUGPORT_GROUP2 =       0x0200,     ///< Output debug port to LCD[18..0]

	ENUM_DUMMY4WORD(PINMUX_DEBUGPORT)
} PINMUX_DEBUGPORT;


/**
    Function group

    @note For pinmux_init()
*/
typedef enum {
	PIN_FUNC_SDIO,      ///< SDIO. Configuration refers to PIN_SDIO_CFG.
	PIN_FUNC_NAND,      ///< NAND. Configuration refers to PIN_NAND_CFG.
	PIN_FUNC_ETH,       ///< ETH. Configuration refers to PINMUX_ETH_CFG
	PIN_FUNC_I2C,       ///< I2C. Configuration refers to PIN_I2C_CFG.
	PIN_FUNC_I2CII,     ///< I2CII. Configuration refers to PIN_I2CII_CFG.
	PIN_FUNC_PWM,       ///< PWM. Configuration refers to PIN_PWM_CFG.
	PIN_FUNC_PWMII,     ///< PWMII. Configuration refers to PIN_PWMII_CFG.
	PIN_FUNC_CCNT,      ///< CCNT. Configuration refers to PIN_CCNT_CFG.
	PIN_FUNC_SENSOR,    ///< sensor interface. Configuration refers to PIN_SENSOR_CFG.
	PIN_FUNC_SENSOR2,   ///< sensor2 interface. Configuration refers to PIN_SENSOR2_CFG.
	PIN_FUNC_SENSOR3,   ///< sensor3 interface. Configuration refers to PIN_SENSOR3_CFG.
	PIN_FUNC_SENSORMISC,///< sensor misc interface. Configuration refers to PIN_SENSORMISC_CFG.
	PIN_FUNC_SENSORSYNC,///< sensor sync interface. Configuration refers to PIN_SENSORSYNC_CFG.
	PIN_FUNC_MIPI_LVDS, ///< MIPI/LVDS interface configuration. Configuration refers to PIN_MIPI_LVDS_CFG.
	PIN_FUNC_AUDIO,     ///< AUDIO. Configuration refers to PIN_AUDIO_CFG.
	PIN_FUNC_UART,      ///< UART. Configuration refers to PIN_UART_CFG.
	PIN_FUNC_UARTII,    ///< UARTII. Configuration refers to PIN_UARTII_CFG.
	PIN_FUNC_REMOTE,    ///< REMOTE. Configuration refers to PIN_REMOTE_CFG.
	PIN_FUNC_SDP,       ///< SDP. Configuration refers to PIN_SDP_CFG.
	PIN_FUNC_SPI,       ///< SPI. Configuration refers to PIN_SPI_CFG.
	PIN_FUNC_SIF,       ///< SIF. Configuration refers to PIN_SIF_CFG.
	PIN_FUNC_MISC,      ///< MISC. Configuration refers to PINMUX_MISC_CFG
	PIN_FUNC_LCD,       ///< Software records LCD interface. Configuration refers to PINMUX_LCDINIT, PINMUX_DISPMUX_SEL.
	PIN_FUNC_LCD2,      ///< Software records LCD2 interface. Configuration refers to PINMUX_LCDINIT, PINMUX_DISPMUX_SEL.
	PIN_FUNC_TV,        ///< Software recores TV interface. Configuration refers to PINMUX_TV_HDMI_CFG.
	PIN_FUNC_SEL_LCD,   ///< LCD interface. Configuration refers to PINMUX_LCD_SEL.
	PIN_FUNC_SEL_LCD2,  ///< LCD2 interface. Configuration refers to PINMUX_LCD_SEL.

	PIN_FUNC_MAX,

	ENUM_DUMMY4WORD(PIN_FUNC)
} PIN_FUNC;

/**
    PIN config for SDIO

    @note For pinmux_init() with PIN_FUNC_SDIO.
*/
typedef enum {
	PIN_SDIO_CFG_NONE,

	PIN_SDIO_CFG_SDIO_1           = 0x1,      ///< SDIO  (C_GPIO[11..16]) @MC11[]|MC12[]|MC13[]|MC14[]|MC15[]|MC16[]

	PIN_SDIO_CFG_SDIO2_1          = 0x10,     ///< SDIO2 (C_GPIO[17..22]) @MC17[]|MC18[]|MC19[]|MC20[]|MC21[]|MC22[]

	PIN_SDIO_CFG_SDIO3_1          = 0x100,    ///< SDIO3 (C_GPIO[0..3] C_GPIO[8..9]) @MC0[]|MC1[]|MC2[]|MC3[]|MC8[]|MC9[]
	PIN_SDIO_CFG_SDIO3_BUS_WIDTH  = 0x200,    ///< SDIO3 bus width 8 bits (C_GPIO[4..7])@MC4[]|MC5[]|MC6[]|MC7[]
	PIN_SDIO_CFG_SDIO3_DS         = 0x400,    ///< SDIO3 data strobe      (C_GPIO[10])  @MC10[]

	ENUM_DUMMY4WORD(PIN_SDIO_CFG)
} PIN_SDIO_CFG;

/**
    PIN config for NAND

    @note For pinmux_init() with PIN_FUNC_NAND.
*/
typedef enum {
	PIN_NAND_CFG_NONE,

	PIN_NAND_CFG_NAND_1           = 0x1,      ///< FSPI     (C_GPIO[0..3], C_GPIO[8..9]) @MC0[]|MC1[]|MC2[]|MC3[]|MC8[]|MC9[]
	PIN_NAND_CFG_NAND_CS1         = 0x2,      ///< FSPI CS1 (C_GPIO[10]) @MC10[]

	ENUM_DUMMY4WORD(PIN_NAND_CFG)
} PIN_NAND_CFG;

/**
    PIN config for ETH

    @note For pinmux_init() for PIN_FUNC_ETH
*/
typedef enum {
	PIN_ETH_CFG_NONE,

	PIN_ETH_CFG_ETH_RGMII_1     = 0x1,        ///< RGMII (L_GPIO[19..30],D_GPIO[5..6])  @L_GPIO19[]|L_GPIO20[]|L_GPIO21[]|L_GPIO22[]|L_GPIO23[]|L_GPIO24[]|L_GPIO25[]|L_GPIO26[]|L_GPIO27[]|L_GPIO28[]|L_GPIO29[]|L_GPIO30[]|D_GPIO5[]|D_GPIO6[]
	PIN_ETH_CFG_ETH_RMII_1      = 0x2,        ///< RMII  (L_GPIO[19..21],L_GPIO[25..28],D_GPIO[5..6])  @L_GPIO19[]|L_GPIO20[]|L_GPIO21[]|L_GPIO25[]|L_GPIO26[]|L_GPIO27[]|L_GPIO28[]|D_GPIO5[]|D_GPIO6[]
	PIN_ETH_CFG_ETH_EXTPHYCLK   = 0x4,        ///< ETH_EXT_PHY_CLK (D_GPIO[7]) @D_GPIO7[]
	PIN_ETH_CFG_ETH_PTP         = 0x8,        ///< ETH_PTP (P_GPIO[28]) @P_GPIO28[]

	PIN_ETH_CFG_ETH2_RGMII_1    = 0x10,       ///< RGMII2 (L_GPIO[3..16])  @L_GPIO3[]|L_GPIO4[]|L_GPIO5[]|L_GPIO6[]|L_GPIO7[]|L_GPIO8[]|L_GPIO9[]|L_GPIO10[]|L_GPIO11[]|L_GPIO12[]|L_GPIO13[]|L_GPIO14[]|L_GPIO15[]|L_GPIO16[]
	PIN_ETH_CFG_ETH2_RMII_1     = 0x20,       ///< RMII2  (L_GPIO[3..5],L_GPIO[9..12],L_GPIO[15..16])   @L_GPIO3[]|L_GPIO4[]|L_GPIO5[]|L_GPIO9[]|L_GPIO10[]|L_GPIO11[]|L_GPIO12[]|L_GPIO15[]|L_GPIO16[]
	PIN_ETH_CFG_ETH2_EXTPHYCLK  = 0x40,       ///< ETH2_EXT_PHY_CLK(L_GPIO[17]) @L_GPIO17[]
	PIN_ETH_CFG_ETH2_PTP        = 0x80,       ///< ETH2_PTP(P_GPIO[24])  @P_GPIO24[]

	ENUM_DUMMY4WORD(PINMUX_ETH_CFG)
} PINMUX_ETH_CFG;

/**
    PIN config for I2C

    @note For pinmux_init() with PIN_FUNC_I2C.
*/
typedef enum {
	PIN_I2C_CFG_NONE,

	PIN_I2C_CFG_I2C_1     = 0x1,         ///< I2C_1   (P_GPIO[28..29])  @P_GPIO28[]|P_GPIO29[]

	PIN_I2C_CFG_I2C2_1    = 0x10,        ///< I2C2_1  (P_GPIO[30..31])  @P_GPIO30[]|P_GPIO31[]

	PIN_I2C_CFG_I2C3_1    = 0x100,       ///< I2C3_1  (P_GPIO[12..13])  @P_GPIO12[]|P_GPIO13[]
	PIN_I2C_CFG_I2C3_2    = 0x200,       ///< I2C3_2  (S_GPIO[11..12])  @S_GPIO11[]|S_GPIO12[]
	PIN_I2C_CFG_I2C3_3    = 0x400,       ///< I2C3_3  (HSI_GPIO[22..23])  @HSI_GPIO22[]|HSI_GPIO23[]

	PIN_I2C_CFG_I2C4_1    = 0x1000,      ///< I2C4_1  (P_GPIO[14..15])  @P_GPIO14[]|P_GPIO15[]
	PIN_I2C_CFG_I2C4_2    = 0x2000,      ///< I2C4_2  (S_GPIO[13..14])  @S_GPIO13[]|S_GPIO14[]
	PIN_I2C_CFG_I2C4_3    = 0x4000,      ///< I2C4_3  (DSI_GPIO[4..5])  @DSI_GPIO4[]|DSI_GPIO5[]
	PIN_I2C_CFG_I2C4_4    = 0x8000,      ///< I2C4_4  (HSI_GPIO[20..21])  @HSI_GPIO20[]|HSI_GPIO21[]

	PIN_I2C_CFG_I2C5_1    = 0x10000,     ///< I2C5_1  (P_GPIO[16..17])  @P_GPIO16[]|P_GPIO17[]
	PIN_I2C_CFG_I2C5_2    = 0x20000,     ///< I2C5_2  (S_GPIO[15..16])  @S_GPIO15[]|S_GPIO16[]
	PIN_I2C_CFG_I2C5_3    = 0x40000,     ///< I2C5_3  (C_GPIO[4..5])    @MC4[]|MC5[]

	ENUM_DUMMY4WORD(PIN_I2C_CFG)
} PIN_I2C_CFG;

/**
    PIN config for I2CII

    @note For pinmux_init() with PIN_FUNC_I2CII.
*/
typedef enum {
	PIN_I2CII_CFG_NONE,

	PIN_I2CII_CFG_I2C6_1  = 0x1,         ///< I2C6_1  (P_GPIO[18..19])  @P_GPIO18[]|P_GPIO19[]
	PIN_I2CII_CFG_I2C6_2  = 0x2,         ///< I2C6_2  (S_GPIO[23..24])  @S_GPIO23[]|S_GPIO24[]
	PIN_I2CII_CFG_I2C6_3  = 0x4,         ///< I2C6_3  (C_GPIO[6..7])    @MC6[]|MC7[]

	PIN_I2CII_CFG_I2C7_1  = 0x10,        ///< I2C7_1  (P_GPIO[20..21])  @P_GPIO20[]|P_GPIO21[]
	PIN_I2CII_CFG_I2C7_2  = 0x20,        ///< I2C7_2  (S_GPIO[25..26])  @S_GPIO25[]|S_GPIO26[]

	PIN_I2CII_CFG_I2C8_1  = 0x100,       ///< I2C8_1  (P_GPIO[22..23])  @P_GPIO22[]|P_GPIO23[]
	PIN_I2CII_CFG_I2C8_2  = 0x200,       ///< I2C8_2  (S_GPIO[7..8])    @S_GPIO7[]|S_GPIO8[]

	PIN_I2CII_CFG_I2C9_1  = 0x1000,      ///< I2C9_1  (P_GPIO[24..25])  @P_GPIO24[]|P_GPIO25[]
	PIN_I2CII_CFG_I2C9_2  = 0x2000,      ///< I2C3_1  (L_GPIO[26..27])  @L_GPIO26[]|L_GPIO27[]

	PIN_I2CII_CFG_I2C10_1 = 0x10000,     ///< I2C10_1 (P_GPIO[26..27])  @P_GPIO26[]|P_GPIO27[]
	PIN_I2CII_CFG_I2C10_2 = 0x20000,     ///< I2C10_2 (D_GPIO[0..1])    @D_GPIO0[]|D_GPIO1[]

	PIN_I2CII_CFG_I2C11_1 = 0x100000,    ///< I2C11_1 (P_GPIO[36..37])  @P_GPIO36[]|P_GPIO37[]

	ENUM_DUMMY4WORD(PIN_I2CII_CFG)
} PIN_I2CII_CFG;

/**
    PIN config for PWM

    @note For pinmux_init() with PIN_FUNC_PWM.
*/
typedef enum {
	PIN_PWM_CFG_NONE,

	PIN_PWM_CFG_PWM0_1    = 0x1,         ///< PWM0_1  (P_GPIO[0])       @P_GPIO0[]
	PIN_PWM_CFG_PWM0_2    = 0x2,         ///< PWM0_2  (DSI_GPIO[6])     @DSI_GPIO6[]

	PIN_PWM_CFG_PWM1_1    = 0x10,        ///< PWM1_1  (P_GPIO[1])       @P_GPIO1[]
	PIN_PWM_CFG_PWM1_2    = 0x20,        ///< PWM1_2  (DSI_GPIO[7])     @DSI_GPIO7[]

	PIN_PWM_CFG_PWM2_1    = 0x100,       ///< PWM2_1  (P_GPIO[2])       @P_GPIO2[]
	PIN_PWM_CFG_PWM2_2    = 0x200,       ///< PWM2_2  (DSI_GPIO[8])     @DSI_GPIO8[]

	PIN_PWM_CFG_PWM3_1    = 0x1000,      ///< PWM3_1  (P_GPIO[3])       @P_GPIO3[]
	PIN_PWM_CFG_PWM3_2    = 0x2000,      ///< PWM3_2  (DSI_GPIO[9])     @DSI_GPIO9[]

	PIN_PWM_CFG_PWM4_1    = 0x10000,     ///< PWM4_1  (P_GPIO[4])       @P_GPIO4[]
	PIN_PWM_CFG_PWM4_2    = 0x20000,     ///< PWM4_2  (D_GPIO[2])       @D_GPIO2[]

	PIN_PWM_CFG_PWM5_1    = 0x100000,    ///< PWM5_1  (P_GPIO[5])       @P_GPIO5[]
	PIN_PWM_CFG_PWM5_2    = 0x200000,    ///< PWM5_2  (D_GPIO[3])       @D_GPIO3[]

	ENUM_DUMMY4WORD(PIN_PWM_CFG)
} PIN_PWM_CFG;

/**
    PIN config for PWMII

    @note For pinmux_init() with PIN_FUNC_PWMII.
*/
typedef enum {
	PIN_PWMII_CFG_NONE,

	PIN_PWMII_CFG_PWM6_1  = 0x1,         ///< PWM6_1  (P_GPIO[6])       @P_GPIO6[]
	PIN_PWMII_CFG_PWM6_2  = 0x2,         ///< PWM6_2  (D_GPIO[4])       @D_GPIO4[]

	PIN_PWMII_CFG_PWM7_1  = 0x10,        ///< PWM7_1  (P_GPIO[7])       @P_GPIO7[]
	PIN_PWMII_CFG_PWM7_2  = 0x20,        ///< PWM7_2  (D_GPIO[5])       @D_GPIO5[]

	PIN_PWMII_CFG_PWM8_1  = 0x100,       ///< PWM8_1  (P_GPIO[8])       @P_GPIO8[]
	PIN_PWMII_CFG_PWM8_2  = 0x200,       ///< PWM8_2  (C_GPIO[4])       @MC4[]

	PIN_PWMII_CFG_PWM9_1  = 0x1000,      ///< PWM9_1  (P_GPIO[9])       @P_GPIO9[]
	PIN_PWMII_CFG_PWM9_2  = 0x2000,      ///< PWM9_2  (C_GPIO[5])       @MC5[]

	PIN_PWMII_CFG_PWM10_1 = 0x10000,     ///< PWM10_1 (P_GPIO[10])      @P_GPIO10[]
	PIN_PWMII_CFG_PWM10_2 = 0x20000,     ///< PWM10_2 (C_GPIO[6])       @MC6[]

	PIN_PWMII_CFG_PWM11_1 = 0x100000,    ///< PWM11_1 (P_GPIO[11])      @P_GPIO11[]
	PIN_PWMII_CFG_PWM11_2 = 0x200000,    ///< PWM11_2 (C_GPIO[7])       @MC7[]

	ENUM_DUMMY4WORD(PIN_PWMII_CFG)
} PIN_PWMII_CFG;

/**
    PIN config for CCNT

    @note For pinmux_init() with PIN_FUNC_CCNT.
*/
typedef enum {
	PIN_CCNT_CFG_NONE,

	PIN_CCNT_CFG_CCNT_1  = 0x1,        ///< PICNT_1  (P_GPIO[21])       @P_GPIO21[]

	PIN_CCNT_CFG_CCNT2_1 = 0x10,       ///< PICNT2_1 (P_GPIO[22])       @P_GPIO22[]

	PIN_CCNT_CFG_CCNT3_1 = 0x100,      ///< PICNT3_1 (P_GPIO[23])       @P_GPIO23[]

	ENUM_DUMMY4WORD(PIN_CCNT_CFG)
} PIN_CCNT_CFG;

/**
    PIN config for Sensor

    @note For pinmux_init() with PIN_FUNC_SENSOR.
*/
typedef enum {
	PIN_SENSOR_CFG_NONE,

	PIN_SENSOR_CFG_12BITS           = 0x04,      ///< SN_D[0..11]/SN_PXCLK/SN_VD/SN_HD (HSI_GPIO[0..11]/S_GPIO[3..5])  @HSI_GPIO0[]|HSI_GPIO1[]|HSI_GPIO2[]|HSI_GPIO3[]|HSI_GPIO4[]|HSI_GPIO5[]|HSI_GPIO6[]|HSI_GPIO7[]|HSI_GPIO8[]|HSI_GPIO9[]|HSI_GPIO10[]|HSI_GPIO11[]|S_GPIO3[]|S_GPIO4[]|S_GPIO5[]

	PIN_SENSOR_CFG_SN3_MCLK_2       = 0x200,     ///< Enable SN3_MCLK_2 (HSI_GPIO[15]) for Sensor1   @HSI_GPIO15[]
	PIN_SENSOR_CFG_SN4_MCLK_2       = 0x400,     ///< Enable SN4_MCLK_2 (HSI_GPIO[16]) for Sensor1   @HSI_GPIO16[]

	ENUM_DUMMY4WORD(PIN_SENSOR_CFG)
} PIN_SENSOR_CFG;

/**
    PIN config for Sensor2

    @note For pinmux_init() with PIN_FUNC_SENSOR2.
*/
typedef enum {
	PIN_SENSOR2_CFG_NONE,

	PIN_SENSOR2_CFG_12BITS          = 0x100,     ///< SN_D[0..11]/SN_PXCLK/SN_VD/SN_HD (S_GPIO[9..20]/S_GPIO[6]/S_GPIO[7]/S_GPIO[8])   @S_GPIO9[]|S_GPIO10[]|S_GPIO11[]|S_GPIO12[]|S_GPIO13[]|S_GPIO14[]|S_GPIO15[]|S_GPIO16[]|S_GPIO17[]|S_GPIO18[]|S_GPIO19[]|S_GPIO20[]|S_GPIO6[]|S_GPIO7[]|S_GPIO8[]

	PIN_SENSOR2_CFG_CCIR8BITS_A     = 0x10000,   ///< CCIR601/656. CCIR_A_YC[0..7]/CCIR_A_CLK (S_GPIO[17..24]/S_GPIO[6])               @S_GPIO17[]|S_GPIO18[]|S_GPIO19[]|S_GPIO20[]|S_GPIO21[]|S_GPIO22[]|S_GPIO23[]|S_GPIO24[]|S_GPIO6[]
	PIN_SENSOR2_CFG_CCIR8BITS_B     = 0x20000,   ///< CCIR656. CCIR_B_YC[0..7]/CCIR_B_CLK (S_GPIO[9..16]/S_GPIO[2])                    @S_GPIO9[]|S_GPIO10[]|S_GPIO11[]|S_GPIO12[]|S_GPIO13[]|S_GPIO14[]|S_GPIO15[]|S_GPIO16[]|S_GPIO2[]
	PIN_SENSOR2_CFG_CCIR8BITS_AB    = 0x40000,   ///< CCIR601/656 + CCIR656. CCIR_A_YC[0..7]/CCIR_A_CLK (S_GPIO[17..24]/S_GPIO[6]) + CCIR_B_YC[0..7]/CCIR_B_CLK (S_GPIO[9..16]/S_GPIO[2])   @S_GPIO17[]|S_GPIO18[]|S_GPIO19[]|S_GPIO20[]|S_GPIO21[]|S_GPIO22[]|S_GPIO23[]|S_GPIO24[]|S_GPIO6[]|S_GPIO9[]|S_GPIO10[]|S_GPIO11[]|S_GPIO12[]|S_GPIO13[]|S_GPIO14[]|S_GPIO15[]|S_GPIO16[]|S_GPIO2[]

	PIN_SENSOR2_CFG_CCIR16BITS      = 0x10,      ///< CCIR601/656. CCIR_Y[0..7]/CCIR_C[0..7]/CCIR_CLK (S_GPIO[17..24]/S_GPIO[9..16]/S_GPIO[6])   @S_GPIO17[]|S_GPIO18[]|S_GPIO19[]|S_GPIO20[]|S_GPIO21[]|S_GPIO22[]|S_GPIO23[]|S_GPIO24[]|S_GPIO9[]|S_GPIO10[]|S_GPIO11[]|S_GPIO12[]|S_GPIO13[]|S_GPIO14[]|S_GPIO15[]|S_GPIO16[]|S_GPIO6[]
	PIN_SENSOR2_CFG_CCIR_VSHS       = 0x100000,  ///< For CCIR601. CCIR_VD/CCIR_HD/CCIR_FIELD (S_GPIO[7]/S_GPIO[8]/S_GPIO[2])          @S_GPIO7[]|S_GPIO8[]|S_GPIO2[]

	PIN_SENSOR2_CFG_SN1_MCLK_1      = 0x200,     ///< Enable SN1_MCLK_1 (S_GPIO[0]) for Sensor2   @S_GPIO0[]
	PIN_SENSOR2_CFG_SN2_MCLK_1      = 0x400,     ///< Enable SN2_MCLK_1 (S_GPIO[1]) for Sensor2   @S_GPIO1[]

	ENUM_DUMMY4WORD(PIN_SENSOR2_CFG)
} PIN_SENSOR2_CFG;

/**
    PIN config for Sensor3

    @note For pinmux_init() with PIN_FUNC_SENSOR3.
*/
typedef enum {
	PIN_SENSOR3_CFG_NONE,

	PIN_SENSOR3_CFG_12BITS          = 0x100,     ///< SN_D[0..11]/SN_PXCLK/SN_VD/SN_HD (P_GPIO[11..14][0..7]/P_GPIO[8]/P_GPIO[9]/P_GPIO[10])   @P_GPIO11[]|P_GPIO12[]|P_GPIO13[]|P_GPIO14[]|P_GPIO0[]|P_GPIO1[]|P_GPIO2[]|P_GPIO3[]|P_GPIO4[]|P_GPIO5[]|P_GPIO6[]|P_GPIO7[]|P_GPIO8[]|P_GPIO9[]|P_GPIO10[]

	PIN_SENSOR3_CFG_CCIR8BITS_A     = 0x10000,   ///< CCIR601/656. CCIR_A_YC[0..7]/CCIR_A_CLK (P_GPIO[0..7]/P_GPIO[8])   @P_GPIO0[]|P_GPIO1[]|P_GPIO2[]|P_GPIO3[]|P_GPIO4[]|P_GPIO5[]|P_GPIO6[]|P_GPIO7[]|P_GPIO8[]
	PIN_SENSOR3_CFG_CCIR8BITS_B     = 0x20000,   ///< CCIR656. CCIR_B_YC[0..7]/CCIR_B_CLK (P_GPIO[12..19]/P_GPIO[11])    @P_GPIO12[]|P_GPIO13[]|P_GPIO14[]|P_GPIO15[]|P_GPIO16[]|P_GPIO17[]|P_GPIO18[]|P_GPIO19[]|P_GPIO11[]
	PIN_SENSOR3_CFG_CCIR8BITS_AB    = 0x40000,   ///< CCIR601/656 + CCIR656. CCIR_A_YC[0..7]/CCIR_A_CLK (P_GPIO[0..7]/P_GPIO[8]) + CCIR_B_YC[0..7]/CCIR_B_CLK (P_GPIO[12..19]/P_GPIO[11])   @P_GPIO0[]|P_GPIO1[]|P_GPIO2[]|P_GPIO3[]|P_GPIO4[]|P_GPIO5[]|P_GPIO6[]|P_GPIO7[]|P_GPIO8[]|P_GPIO12[]|P_GPIO13[]|P_GPIO14[]|P_GPIO15[]|P_GPIO16[]|P_GPIO17[]|P_GPIO18[]|P_GPIO19[]|P_GPIO11[]

	PIN_SENSOR3_CFG_CCIR16BITS      = 0x10,      ///< CCIR601/656. CCIR_Y[0..7]/CCIR_C[0..7]/CCIR_CLK (P_GPIO[0..7]/P_GPIO[12..19]/P_GPIO[8])   @P_GPIO0[]|P_GPIO1[]|P_GPIO2[]|P_GPIO3[]|P_GPIO4[]|P_GPIO5[]|P_GPIO6[]|P_GPIO7[]|P_GPIO8[]|P_GPIO12[]|P_GPIO13[]|P_GPIO14[]|P_GPIO15[]|P_GPIO16[]|P_GPIO17[]|P_GPIO18[]|P_GPIO19[]
	PIN_SENSOR3_CFG_CCIR_VSHS       = 0x100000,  ///< For CCIR601. CCIR_VD/CCIR_HD/CCIR_FIELD (P_GPIO[9]/P_GPIO[10]/P_GPIO[11])   @P_GPIO9[]|P_GPIO10[]|P_GPIO11[]

	PIN_SENSOR3_CFG_SN5_MCLK_2      = 0x200,     ///< Enable SN5_MCLK_2 (P_GPIO[20]) for Sensor3    @P_GPIO20[]

	ENUM_DUMMY4WORD(PIN_SENSOR3_CFG)
} PIN_SENSOR3_CFG;

/**
    PIN config for Sensor Misc

    @note For pinmux_init() with PIN_FUNC_SENSORMISC.
*/
typedef enum {
	PIN_SENSORMISC_CFG_NONE,

	PIN_SENSORMISC_CFG_SN_MCLK_1         = 0x1,      ///< SN1_MCLK_1 (S_GPIO[0])   @S_GPIO0[]

	PIN_SENSORMISC_CFG_SN2_MCLK_1        = 0x2,      ///< SN2_MCLK_1 (S_GPIO[1])   @S_GPIO1[]

	PIN_SENSORMISC_CFG_SN3_MCLK_1        = 0x4,      ///< SN3_MCLK_1 (S_GPIO[2])   @S_GPIO2[]
	PIN_SENSORMISC_CFG_SN3_MCLK_2        = 0x8,      ///< SN3_MCLK_2 (HSI_GPIO[15]) @HSI_GPIO15[]

	PIN_SENSORMISC_CFG_SN4_MCLK_1        = 0x10,     ///< SN4_MCLK_1 (S_GPIO[21])  @S_GPIO21[]
	PIN_SENSORMISC_CFG_SN4_MCLK_2        = 0x20,     ///< SN4_MCLK_2 (HSI_GPIO[16]) @HSI_GPIO16[]

	PIN_SENSORMISC_CFG_SN5_MCLK_1        = 0x40,     ///< SN5_MCLK_1 (S_GPIO[22])  @S_GPIO22[]
	PIN_SENSORMISC_CFG_SN5_MCLK_2        = 0x80,     ///< SN5_MCLK_2 (P_GPIO[20])  @P_GPIO20[]

	PIN_SENSORMISC_CFG_SN_XVSXHS_1       = 0x100,    ///< SN1_XVSXHS_1 (S_GPIO[4..5])  @S_GPIO4[]|S_GPIO5[]

	PIN_SENSORMISC_CFG_SN2_XVSXHS_1      = 0x200,    ///< SN2_XVSXHS_1 (S_GPIO[7..8])  @S_GPIO7[]|S_GPIO8[]

	PIN_SENSORMISC_CFG_SN3_XVSXHS_1      = 0x400,    ///< SN3_XVSXHS_1 (S_GPIO[9..10]) @S_GPIO9[]|S_GPIO10[]

	PIN_SENSORMISC_CFG_SN4_XVSXHS_1      = 0x800,    ///< SN4_XVSXHS_1 (S_GPIO[17..18]) @S_GPIO17[]|S_GPIO18[]

	PIN_SENSORMISC_CFG_SN5_XVSXHS_1      = 0x1000,   ///< SN5_XVSXHS_1 (S_GPIO[19..20]) @S_GPIO19[]|S_GPIO20[]

	PIN_SENSORMISC_CFG_FLASH_TRIG_IN_1   = 0x10000,  ///< FLASH_TRIG_IN_1 (S_GPIO[25])  @S_GPIO25[]
	PIN_SENSORMISC_CFG_FLASH_TRIG_IN_2   = 0x20000,  ///< FLASH_TRIG_IN_2 (HSI_GPIO[17]) @HSI_GPIO17[]

	PIN_SENSORMISC_CFG_FLASH_TRIG_OUT_1  = 0x40000,  ///< FLASH_TRIG_OUT_1 (S_GPIO[26]) @S_GPIO26[]
	PIN_SENSORMISC_CFG_FLASH_TRIG_OUT_2  = 0x80000,  ///< FLASH_TRIG_OUT_2 (HSI_GPIO[18]) @HSI_GPIO18[]

	ENUM_DUMMY4WORD(PIN_SENSORMISC_CFG)
} PIN_SENSORMISC_CFG;

/**
    PIN config for Sensor Sync

    @note For pinmux_init() with PIN_FUNC_SENSORSYNC.
*/
typedef enum {
	PIN_SENSORSYNC_CFG_NONE,

	PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SN     = 0x1,

	PIN_SENSORSYNC_CFG_SN3_MCLKSRC_SN     = 0x10,
	PIN_SENSORSYNC_CFG_SN3_MCLKSRC_SN2    = 0x20,

	PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN     = 0x100,
	PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN2    = 0x200,
	PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN3    = 0x400,

	PIN_SENSORSYNC_CFG_SN5_MCLKSRC_SN     = 0x1000,
	PIN_SENSORSYNC_CFG_SN5_MCLKSRC_SN2    = 0x2000,
	PIN_SENSORSYNC_CFG_SN5_MCLKSRC_SN3    = 0x4000,
	PIN_SENSORSYNC_CFG_SN5_MCLKSRC_SN4    = 0x8000,

	PIN_SENSORSYNC_CFG_SN2_XVSXHSSRC_SN   = 0x10000,

	PIN_SENSORSYNC_CFG_SN3_XVSXHSSRC_SN   = 0x100000,
	PIN_SENSORSYNC_CFG_SN3_XVSXHSSRC_SN2  = 0x200000,

	PIN_SENSORSYNC_CFG_SN4_XVSXHSSRC_SN   = 0x1000000,
	PIN_SENSORSYNC_CFG_SN4_XVSXHSSRC_SN2  = 0x2000000,
	PIN_SENSORSYNC_CFG_SN4_XVSXHSSRC_SN3  = 0x4000000,

	PIN_SENSORSYNC_CFG_SN5_XVSXHSSRC_SN   = 0x10000000,
	PIN_SENSORSYNC_CFG_SN5_XVSXHSSRC_SN2  = 0x20000000,
	PIN_SENSORSYNC_CFG_SN5_XVSXHSSRC_SN3  = 0x40000000,
	PIN_SENSORSYNC_CFG_SN5_XVSXHSSRC_SN4  = 0x80000000,

	ENUM_DUMMY4WORD(PIN_SENSORSYNC_CFG)
} PIN_SENSORSYNC_CFG;

/**
    PIN config for MIPI/LVDS

    @note For pinmux_init() with PIN_FUNC_MIPI_LVDS.
*/
typedef enum {
	PIN_MIPI_LVDS_CFG_NONE,
	PIN_MIPI_LVDS_CFG_MIPI_SEL         = 0x1000000,   ///< Select CSI_TX to MIPI D-PHY, note the default is DSI to MIPI D-PHY
	PIN_MIPI_LVDS_CFG_HSI2HSI3_TO_CSI  = 0x100000,    ///< HSI2 and HSI3 are configured as CSIx/LVDS (HSI_GPIO[0..23])   @HSI_GPIO0[]|HSI_GPIO1[]|HSI_GPIO2[]|HSI_GPIO3[]|HSI_GPIO4[]|HSI_GPIO5[]|HSI_GPIO6[]|HSI_GPIO7[]|HSI_GPIO8[]|HSI_GPIO9[]|HSI_GPIO10[]|HSI_GPIO11[]|HSI_GPIO12[]|HSI_GPIO13[]|HSI_GPIO14[]|HSI_GPIO15[]|HSI_GPIO16[]|HSI_GPIO17[]|HSI_GPIO18[]|HSI_GPIO19[]|HSI_GPIO20[]|HSI_GPIO21[]|HSI_GPIO22[]|HSI_GPIO23[]
	ENUM_DUMMY4WORD(PIN_MIPI_LVDS_CFG)
} PIN_MIPI_LVDS_CFG;

/**
    PIN config for AUDIO

    @note For pinmux_init() with PIN_FUNC_AUDIO.
*/
typedef enum {
	PIN_AUDIO_CFG_NONE,

	PIN_AUDIO_CFG_I2S_1         = 0x1,      ///< I2S_1       (PGPIO[24..27])   @P_GPIO24[]|P_GPIO25[]|P_GPIO26[]|P_GPIO27[]
	PIN_AUDIO_CFG_I2S_2         = 0x2,      ///< I2S_2       (CGPIO[18..21])   @MC18[]|MC19[]|MC20[]|MC21[]

	PIN_AUDIO_CFG_I2S_MCLK_1    = 0x4,      ///< I2S_MCLK_1  (PGPIO[28])       @P_GPIO28[]
	PIN_AUDIO_CFG_I2S_MCLK_2    = 0x8,      ///< I2S_MCLK_2  (CGPIO[17])       @MC17[]

	PIN_AUDIO_CFG_I2S2_1        = 0x10,     ///< I2S2_1      (LGPIO[20..23])   @L_GPIO20[]|L_GPIO21[]|L_GPIO22[]|L_GPIO23[]

	PIN_AUDIO_CFG_I2S2_MCLK_1   = 0x20,     ///< I2S2_MCLK_1 (LGPIO[24])       @L_GPIO24[]

	PIN_AUDIO_CFG_DMIC_1        = 0x100,    ///< DMIC_1 (PGPIO[26]) DATA0 (PGPIO[27]) DATA1 (PGPIO[25])   @P_GPIO26[]|P_GPIO27[]|P_GPIO25[]
	PIN_AUDIO_CFG_DMIC_DATA0    = 0x200,
	PIN_AUDIO_CFG_DMIC_DATA1    = 0x400,

	PIN_AUDIO_CFG_EXT_EAC_MCLK  = 0x1000,   ///< EXT_EAC_MCLK (CGPIO[16])      @MC16[]

	ENUM_DUMMY4WORD(PIN_AUDIO_CFG)
} PIN_AUDIO_CFG;

/**
    PIN config for UART

    @note For pinmux_init() with PIN_FUNC_UART.
*/
typedef enum {
	PIN_UART_CFG_NONE,

	PIN_UART_CFG_UART_1          = 0x1,           ///< UART_1  (P_GPIO[32..33])   @P_GPIO32[]|P_GPIO33[]

	PIN_UART_CFG_UART2_1         = 0x10,          ///< UART2_1 (P_GPIO[30..31]) RTSCTS (P_GPIO[28..29])   @P_GPIO28[]|P_GPIO29[]|P_GPIO30[]|P_GPIO31[]

	PIN_UART_CFG_UART3_1         = 0x100,         ///< UART3_1 (P_GPIO[24..25]) RTSCTS (P_GPIO[26..27])   @P_GPIO26[]|P_GPIO27[]|P_GPIO24[]|P_GPIO25[]

	PIN_UART_CFG_UART4_1         = 0x1000,        ///< UART4_1 (P_GPIO[20..21]) RTSCTS (P_GPIO[22..23])   @P_GPIO22[]|P_GPIO23[]|P_GPIO20[]|P_GPIO21[]
	PIN_UART_CFG_UART4_2         = 0x2000,        ///< UART4_2 (A_GPIO[0..1])   RTSCTS (A_GPIO[2..3])     @A_GPIO2[]|A_GPIO3[]|A_GPIO0[]|A_GPIO1[]

	PIN_UART_CFG_UART5_1         = 0x10000,       ///< UART5_1 (P_GPIO[16..17]) RTSCTS (P_GPIO[18..19])   @P_GPIO18[]|P_GPIO19[]|P_GPIO16[]|P_GPIO17[]
	PIN_UART_CFG_UART5_2         = 0x20000,       ///< UART5_2 (DSI_GPIO[0..1]) RTSCTS (DSI_GPIO[2..3])   @DSI_GPIO2[]|DSI_GPIO3[]|DSI_GPIO0[]|DSI_GPIO1[]

	PIN_UART_CFG_UART2_RTSCTS    = 0x100000,      ///< RTSCTS UART2_1(P_GPIO[28..29])
	PIN_UART_CFG_UART2_DIROE     = 0x200000,      ///< RTS (P_GPIO[28)

	PIN_UART_CFG_UART3_RTSCTS    = 0x400000,      ///< RTSCTS (P_GPIO[26..27])
	PIN_UART_CFG_UART3_DIROE     = 0x800000,      ///< RTS (P_GPIO[26])

	PIN_UART_CFG_UART4_RTSCTS    = 0x1000000,      ///< RTSCTS UART4_1(P_GPIO[22..23]) UART4_2 (A_GPIO[2..3])
	PIN_UART_CFG_UART4_DIROE     = 0x2000000,      ///< RTS UART4_1(P_GPIO[22]) UART4_2 (A_GPIO[2])

	PIN_UART_CFG_UART5_RTSCTS    = 0x4000000,      ///< RTSCTS UART5_1(P_GPIO[18..19]) UART5_2 (DSI_GPIO[2..3])
	PIN_UART_CFG_UART5_DIROE     = 0x8000000,      ///< RTS UART5_1(P_GPIO[18])    UART5_2 (DSI_GPIO[2])

	ENUM_DUMMY4WORD(PIN_UART_CFG)
} PIN_UART_CFG;

/**
    PIN config for UARTII

    @note For pinmux_init() with PIN_FUNC_UARTII.
*/
typedef enum {
	PIN_UARTII_CFG_NONE,

	PIN_UARTII_CFG_UART6_1       = 0x1,           ///< UART6_1 (P_GPIO[12..13]) RTSCTS (P_GPIO[14..15])  @P_GPIO12[]|P_GPIO13[]|P_GPIO14[]|P_GPIO15[]
	PIN_UARTII_CFG_UART6_2       = 0x2,           ///< UART6_2 (DSI_GPIO[4..5]) RTSCTS (DSI_GPIO[6..7])  @DSI_GPIO4[]|DSI_GPIO5[]|DSI_GPIO6[]|DSI_GPIO7[]

	PIN_UARTII_CFG_UART7_1       = 0x10,          ///< UART7_1 (P_GPIO[8..9])   RTSCTS (P_GPIO[10..11])  @P_GPIO8[]|P_GPIO9[]|P_GPIO10[]|P_GPIO11[]
	PIN_UARTII_CFG_UART7_2       = 0x20,          ///< UART7_2 (C_GPIO[4..5])   RTSCTS (C_GPIO[6..7])    @MC4[]|MC5[]|MC6[]|MC7[]

	PIN_UARTII_CFG_UART8_1       = 0x100,         ///< UART8_1 (P_GPIO[4..5])   RTSCTS (P_GPIO[6..7])    @P_GPIO4[]|P_GPIO5[]|P_GPIO6[]|P_GPIO7[]
	PIN_UARTII_CFG_UART8_2       = 0x200,         ///< UART8_2 (C_GPIO[17..18]) RTSCTS (C_GPIO[19..20])  @MC17[]|MC18[]|MC19[]|MC20[]

	PIN_UARTII_CFG_UART9_1       = 0x1000,        ///< UART9_1 (P_GPIO[0..1])   RTSCTS (P_GPIO[2..3])    @P_GPIO0[]|P_GPIO1[]|P_GPIO2[]|P_GPIO3[]
	PIN_UARTII_CFG_UART9_2       = 0x2000,        ///< UART9_2 (D_GPIO[5..6])   RTSCTS (D_GPIO[7..8])    @D_GPIO5[]|D_GPIO6[]|D_GPIO7[]|D_GPIO8[]

	PIN_UARTII_CFG_UART6_RTSCTS  = 0x10000,       ///<RTSCTS UART6_1(P_GPIO[14..15]) UART6_2(DSI_GPIO[6..7])
	PIN_UARTII_CFG_UART6_DIROE   = 0x20000,       ///<RTSCTS UART6_1(P_GPIO[14]) UART6_2(DSI_GPIO[6])

	PIN_UARTII_CFG_UART7_RTSCTS  = 0x40000,       ///<RTSCTS UART7_1(P_GPIO[10..11]) UART7_2(C_GPIO[6..7])
	PIN_UARTII_CFG_UART7_DIROE   = 0x80000,       ///<RTSCTS UART7_1(P_GPIO[10]) UART7_2(C_GPIO[6])

	PIN_UARTII_CFG_UART8_RTSCTS  = 0x100000,      ///<RTSCTS UART8_1(P_GPIO[6..7])  UART8_2(C_GPIO[19..20])

	PIN_UARTII_CFG_UART8_DIROE   = 0x200000,      ///<RTSCTS UART8_1(P_GPIO[6])  UART8_2(C_GPIO[19])

	PIN_UARTII_CFG_UART9_RTSCTS  = 0x400000,      ///<RTSCTS UART9_1(P_GPIO[2..3])   UART9_2(D_GPIO[7..8])
	PIN_UARTII_CFG_UART9_DIROE   = 0x800000,      ///<RTSCTS UART9_1(P_GPIO[2])   UART9_2(D_GPIO[7])

	ENUM_DUMMY4WORD(PIN_UARTII_CFG)
} PIN_UARTII_CFG;

/**
    PIN config for REMOTE

    @note For pinmux_init() with PIN_FUNC_REMOTE.
*/
typedef enum {
	PIN_REMOTE_CFG_NONE,

	PIN_REMOTE_CFG_REMOTE_1      = 0x1,    ///< REMOTE_1     (P_GPIO[31])   @P_GPIO31[]
	PIN_REMOTE_CFG_REMOTE_EXT_1  = 0x2,    ///< REMOTE_EXT_1 (P_GPIO[30])   @P_GPIO30[]

	ENUM_DUMMY4WORD(PIN_REMOTE_CFG)
} PIN_REMOTE_CFG;

/**
    PIN config for SDP

    @note For pinmux_init() with PIN_FUNC_SDP.
*/
typedef enum {
	PIN_SDP_CFG_NONE,

	PIN_SDP_CFG_SDP_1  = 0x1,    ///< SDP_1 (P_GPIO[15..19])   @P_GPIO15[]|P_GPIO16[]|P_GPIO17[]|P_GPIO18[]|P_GPIO19[]
	PIN_SDP_CFG_SDP_2  = 0x2,    ///< SDP_2 (C_GPIO[17..21])   @MC17[]|MC18[]|MC19[]|MC20[]|MC21[]

	ENUM_DUMMY4WORD(PIN_SDP_CFG)
} PIN_SDP_CFG;

/**
    PIN config for SPI

    @note For pinmux_init() with PIN_FUNC_SPI.
*/
typedef enum {
	PIN_SPI_CFG_NONE,

	PIN_SPI_CFG_SPI_1           = 0x1,        ///< SPI_1  (P_GPIO[12..14])  BUS_WIDTH (P_GPIO[15])   @P_GPIO12[]|P_GPIO13[]|P_GPIO14[]|P_GPIO15[]
	PIN_SPI_CFG_SPI_2           = 0x2,        ///< SPI1_2 (DSI_GPIO[0..2])  BUS_WIDTH (DSI_GPIO[3])  @DSI_GPIO0[]|DSI_GPIO1[]|DSI_GPIO2[]|DSI_GPIO3[]
	PIN_SPI_CFG_SPI_3           = 0x4,        ///< SPI1_3 (C_GPIO[4..6])    BUS_WIDTH (C_GPIO[7])    @MC4[]|MC5[]|MC6[]|MC7[]
	PIN_SPI_CFG_SPI_BUS_WIDTH   = 0x8,

	PIN_SPI_CFG_SPI2_1          = 0x10,       ///< SPI2_1 (P_GPIO[16..18])  BUS_WIDTH (P_GPIO[19])   @P_GPIO16[]|P_GPIO17[]|P_GPIO18[]|P_GPIO19[]
	PIN_SPI_CFG_SPI2_2          = 0x20,       ///< SPI2_2 (S_GPIO[17..19])  BUS_WIDTH (S_GPIO[20])   @S_GPIO17[]|S_GPIO18[]|S_GPIO19[]|S_GPIO20[]
	PIN_SPI_CFG_SPI2_BUS_WIDTH  = 0x40,

	PIN_SPI_CFG_SPI3_1          = 0x100,      ///< SPI3_1 (P_GPIO[20..22])  BUS_WIDTH (P_GPIO[23])   @P_GPIO20[]|P_GPIO21[]|P_GPIO22[]|P_GPIO23[]
	PIN_SPI_CFG_SPI3_2          = 0x200,      ///< SPI3_2 (C_GPIO[17..19])  BUS_WIDTH (C_GPIO[20])   @MC17[]|MC18[]|MC19[]|MC20[]
	PIN_SPI_CFG_SPI3_BUS_WIDTH  = 0x400,

	PIN_SPI_CFG_SPI4_1          = 0x1000,     ///< SPI4_1 (P_GPIO[24..26])  BUS_WIDTH (P_GPIO[27])   @P_GPIO24[]|P_GPIO25[]|P_GPIO26[]|P_GPIO27[]
	PIN_SPI_CFG_SPI4_2          = 0x2000,     ///< SPI4_2 (D_GPIO[2..4])    BUS_WIDTH (D_GPIO[5])    @D_GPIO2[]|D_GPIO3[]|D_GPIO4[]|D_GPIO5[]
	PIN_SPI_CFG_SPI4_BUS_WIDTH  = 0x4000,

	PIN_SPI_CFG_SPI5_1          = 0x10000,    ///< SPI5_1 (P_GPIO[28..30])  BUS_WIDTH (P_GPIO[31])   @P_GPIO28[]|P_GPIO29[]|P_GPIO30[]|P_GPIO31[]
	PIN_SPI_CFG_SPI5_2          = 0x20000,    ///< SPI5_2 (L_GPIO[20..22])  BUS_WIDTH (L_GPIO[23])   @L_GPIO20[]|L_GPIO21[]|L_GPIO22[]|L_GPIO23[]
	PIN_SPI_CFG_SPI5_BUS_WIDTH  = 0x40000,

	PIN_SPI_CFG_SPI3_RDY_1      = 0x100000,   ///< SPI3RDY_1 (P_GPIO[27])   @P_GPIO27[]
	PIN_SPI_CFG_SPI3_RDY_2      = 0x200000,   ///< SPI3RDY_2 (C_GPIO[21])   @MC21[]

	ENUM_DUMMY4WORD(PIN_SPI_CFG)
} PIN_SPI_CFG;

/**
    PIN config for SIF

    *note For pinmux_init() with PIN_FUNC_SIF.
*/
typedef enum {
	PIN_SIF_CFG_NONE,

	PIN_SIF_CFG_SIF0_1  = 0x1,         ///< SIF0_1 (P_GPIO[12..14])              @P_GPIO12[]|P_GPIO13[]|P_GPIO14[]
	PIN_SIF_CFG_SIF0_2  = 0x2,         ///< SIF0_2 (L_GPIO[25..27])              @L_GPIO25[]|L_GPIO26[]|L_GPIO27[]
	PIN_SIF_CFG_SIF0_3  = 0x4,         ///< SIF0_3 (DSI_GPIO[8..10])             @DSI_GPIO8[]|DSI_GPIO9[]|DSI_GPIO10[]

	PIN_SIF_CFG_SIF1_1  = 0x10,        ///< SIF1_1 (P_GPIO[16..18])              @P_GPIO16[]|P_GPIO17[]|P_GPIO18[]
	PIN_SIF_CFG_SIF1_2  = 0x20,        ///< SIF1_2 (L_GPIO[28..30])              @L_GPIO28[]|L_GPIO29[]|L_GPIO30[]

	PIN_SIF_CFG_SIF2_1  = 0x100,       ///< SIF2_1 (P_GPIO[20..22])              @P_GPIO20[]|P_GPIO21[]|P_GPIO22[]
	PIN_SIF_CFG_SIF2_2  = 0x200,       ///< SIF2_2 (S_GPIO[11..12] S_GPIO[15])   @S_GPIO11[]|S_GPIO12[]|S_GPIO15[]

	PIN_SIF_CFG_SIF3_1  = 0x1000,      ///< SIF3_1 (P_GPIO[24..26])              @P_GPIO24[]|P_GPIO25[]|P_GPIO26[]
	PIN_SIF_CFG_SIF3_2  = 0x2000,      ///< SIF3_2 (S_GPIO[13..14] S_GPIO[16])   @S_GPIO13[]|S_GPIO14[]|S_GPIO16[]

	PIN_SIF_CFG_SIF4_1  = 0x10000,     ///< SIF4_1 (P_GPIO[28..30])              @P_GPIO28[]|P_GPIO29[]|P_GPIO30[]
	PIN_SIF_CFG_SIF4_2  = 0x20000,     ///< SIF4_2 (S_GPIO[23..25])              @S_GPIO23[]|S_GPIO24[]|S_GPIO25[]

	PIN_SIF_CFG_SIF5_1  = 0x100000,    ///< SIF5_1 (P_GPIO[8..10])               @P_GPIO8[]|P_GPIO9[]|P_GPIO10[]
	PIN_SIF_CFG_SIF5_2  = 0x200000,    ///< SIF5_2 (C_GPIO[4..6])                @MC4[]|MC5[]|MC6[]

	ENUM_DUMMY4WORD(PIN_SIF_CFG)
} PIN_SIF_CFG;

/**
    PIN config for MISC

    @note For pinmux_init() for PIN_FUNC_MISC.
*/
typedef enum {
	PIN_MISC_CFG_NONE,

	PIN_MISC_CFG_RTC_EXT_CLK_1   = 0x2, 	  ///< RTC_EXT_CLK_1 (P_GPIO[23]) @P_GPIO23[]

	PIN_MISC_CFG_RTC_DIV_OUT_1   = 0x4, 	  ///< RTC_DIV_OUT_1 (P_GPIO[22]) @P_GPIO22[]

	PIN_MISC_CFG_RTC_CLK_1       = 0x1,       ///< RTC_CLK_1 (P_GPIO[29])   @P_GPIO29[]

	PIN_MISC_CFG_SP_CLK_1        = 0x10,      ///< SP_CLK_1  (D_GPIO[6])    @D_GPIO6[]

	PIN_MISC_CFG_SP2_CLK_1       = 0x100,     ///< SP2_CLK_1 (DSI_GPIO[10]) @DSI_GPIO10[]

	PIN_MISC_CFG_SATA_LED_1      = 0x1000,    ///< SATA_LED_1 (D_GPIO[4])   @D_GPIO4[]

	ENUM_DUMMY4WORD(PINMUX_MISC_CFG)
} PINMUX_MISC_CFG;

/**
    PIN config of LCD modes

    @note For pinmux_init() with PIN_FUNC_LCD or PIN_FUNC_LCD2.\n
    For example, you can use {PIN_FUNC_LCD, PINMUX_DISPMUX_SEL_LCD | PINMUX_LCDMODE_RGB_SERIAL}
    to tell display object that PIN_FUNC_LCD is located on primary LCD pinmux,
    and it's LCD mode is RGB serial.
*/
typedef enum {
	PINMUX_LCDMODE_RGB_SERIAL           =    0,     ///< LCD MODE is RGB Serial or UPS051
	PINMUX_LCDMODE_RGB_PARALL           =    1,     ///< LCD MODE is RGB Parallel (888)
	PINMUX_LCDMODE_YUV640               =    2,     ///< LCD MODE is YUV640
	PINMUX_LCDMODE_YUV720               =    3,     ///< LCD MODE is YUV720
	PINMUX_LCDMODE_RGBD360              =    4,     ///< LCD MODE is RGB Dummy 360
	PINMUX_LCDMODE_RGBD320              =    5,     ///< LCD MODE is RGB Dummy 320
	PINMUX_LCDMODE_RGB_THROUGH          =    6,     ///< LCD MODE is RGB through mode
	PINMUX_LCDMODE_CCIR601              =    7,     ///< LCD MODE is CCIR601
	PINMUX_LCDMODE_CCIR656              =    8,     ///< LCD MODE is CCIR656
	PINMUX_LCDMODE_RGB_PARALL666        =    9,     ///< LCD MODE is RGB Parallel 666
	PINMUX_LCDMODE_RGB_PARALL565        =   10,     ///< LCD MODE is RGB Parallel 565
	PINMUX_LCDMODE_RGB_PARALL_DELTA     =   11,     ///< LCD MODE is RGB Parallel Delta
	PINMUX_LCDMODE_MIPI                 =   12,     ///< LCD MODE is MIPI Display

	PINMUX_LCDMODE_MI_OFFSET            = 32,       ///< Memory LCD MODE offset
	PINMUX_LCDMODE_MI_FMT0              = 32 + 0,   ///< LCD MODE is Memory(Parallel Interface) 8bits
	PINMUX_LCDMODE_MI_FMT1              = 32 + 1,   ///< LCD MODE is Memory(Parallel Interface) 8bits
	PINMUX_LCDMODE_MI_FMT2              = 32 + 2,   ///< LCD MODE is Memory(Parallel Interface) 8bits
	PINMUX_LCDMODE_MI_FMT3              = 32 + 3,   ///< LCD MODE is Memory(Parallel Interface) 8bits
	PINMUX_LCDMODE_MI_FMT4              = 32 + 4,   ///< LCD MODE is Memory(Parallel Interface) 8bits
	PINMUX_LCDMODE_MI_FMT5              = 32 + 5,   ///< LCD MODE is Memory(Parallel Interface) 8bits
	PINMUX_LCDMODE_MI_FMT6              = 32 + 6,   ///< LCD MODE is Memory(Parallel Interface) 8bits
	PINMUX_LCDMODE_MI_FMT7              = 32 + 7,   ///< LCD MODE is Memory(Parallel Interface) 9bits
	PINMUX_LCDMODE_MI_FMT8              = 32 + 8,   ///< LCD MODE is Memory(Parallel Interface) 16bits
	PINMUX_LCDMODE_MI_FMT9              = 32 + 9,   ///< LCD MODE is Memory(Parallel Interface) 16bits
	PINMUX_LCDMODE_MI_FMT10             = 32 + 10,  ///< LCD MODE is Memory(Parallel Interface) 18bits
	PINMUX_LCDMODE_MI_FMT11             = 32 + 11,  ///< LCD MODE is Memory(Parallel Interface)  8bits
	PINMUX_LCDMODE_MI_FMT12             = 32 + 12,  ///< LCD MODE is Memory(Parallel Interface) 16bits
	PINMUX_LCDMODE_MI_FMT13             = 32 + 13,  ///< LCD MODE is Memory(Parallel Interface) 16bits
	PINMUX_LCDMODE_MI_FMT14             = 32 + 14,  ///< LCD MODE is Memory(Parallel Interface) 16bits
	PINMUX_LCDMODE_MI_SERIAL_BI         = 32 + 20,  ///< LCD MODE is Serial Interface bi-direction
	PINMUX_LCDMODE_MI_SERIAL_SEP        = 32 + 21,  ///< LCD MODE is Serial Interface separation

	PINMUX_LCDMODE_AUTO_PINMUX          = 0x01 << 22, ///< Set display device to GPIO mode when display device is closed. Select this filed will inform display object to switch to GPIO when display device is closed.


	PINMUX_HDMIMODE_OFFSET              = 64,       ///< HDMI MODE offset (not support HDMI. Below is backward compatible)
	PINMUX_HDMIMODE_640X480P60          = 64 + 1,   ///< HDMI Video format is 640x480 & Progressive 60fps
	PINMUX_HDMIMODE_720X480P60          = 64 + 2,   ///< HDMI Video format is 720x480 & Progressive 60fps & 4:3
	PINMUX_HDMIMODE_720X480P60_16X9     = 64 + 3,   ///< HDMI Video format is 720x480 & Progressive 60fps & 16:9
	PINMUX_HDMIMODE_1280X720P60         = 64 + 4,   ///< HDMI Video format is 1280x720 & Progressive 60fps
	PINMUX_HDMIMODE_1920X1080I60        = 64 + 5,   ///< HDMI Video format is 1920x1080 & Interlaced 60fps
	PINMUX_HDMIMODE_720X480I60          = 64 + 6,   ///< HDMI Video format is 720x480 & Interlaced 60fps
	PINMUX_HDMIMODE_720X480I60_16X9     = 64 + 7,   ///< HDMI Video format is 720x480 & Interlaced 60fps & 16:9
	PINMUX_HDMIMODE_720X240P60          = 64 + 8,   ///< HDMI Video format is 720x240 & Progressive 60fps
	PINMUX_HDMIMODE_720X240P60_16X9     = 64 + 9,   ///< HDMI Video format is 720x240 & Progressive 60fps & 16:9
	PINMUX_HDMIMODE_1440X480I60         = 64 + 10,  ///< HDMI Video format is 1440x480 & Interlaced 60fps
	PINMUX_HDMIMODE_1440X480I60_16X9    = 64 + 11,  ///< HDMI Video format is 1440x480 & Interlaced 60fps & 16:9
	PINMUX_HDMIMODE_1440X240P60         = 64 + 12,  ///< HDMI Video format is 1440x240 & Progressive 60fps
	PINMUX_HDMIMODE_1440X240P60_16X9    = 64 + 13,  ///< HDMI Video format is 1440x240 & Progressive 60fps & 16:9
	PINMUX_HDMIMODE_1440X480P60         = 64 + 14,  ///< HDMI Video format is 1440x480 & Progressive 60fps
	PINMUX_HDMIMODE_1440X480P60_16X9    = 64 + 15,  ///< HDMI Video format is 1440x480 & Progressive 60fps & 16:9
	PINMUX_HDMIMODE_720X576P50          = 64 + 17,  ///< HDMI Video format is 720x576 & Progressive 50fps
	PINMUX_HDMIMODE_720X576P50_16X9     = 64 + 18,  ///< HDMI Video format is 720x576 & Progressive 50fps & 16:9
	PINMUX_HDMIMODE_1280X720P50         = 64 + 19,  ///< HDMI Video format is 1280x720 & Progressive 50fps
	PINMUX_HDMIMODE_1920X1080I50        = 64 + 20,  ///< HDMI Video format is 1920x1080 & Interlaced 50fps
	PINMUX_HDMIMODE_720X576I50          = 64 + 21,  ///< HDMI Video format is 720x576 & Interlaced 50fps
	PINMUX_HDMIMODE_720X576I50_16X9     = 64 + 22,  ///< HDMI Video format is 720x576 & Interlaced 50fps & 16:9
	PINMUX_HDMIMODE_720X288P50          = 64 + 23,  ///< HDMI Video format is 720x288 & Progressive 50fps
	PINMUX_HDMIMODE_720X288P50_16X9     = 64 + 24,  ///< HDMI Video format is 720x288 & Progressive 50fps & 16:9
	PINMUX_HDMIMODE_1440X576I50         = 64 + 25,  ///< HDMI Video format is 1440x576 & Interlaced 50fps
	PINMUX_HDMIMODE_1440X576I50_16X9    = 64 + 26,  ///< HDMI Video format is 1440x576 & Interlaced 50fps & 16:9
	PINMUX_HDMIMODE_1440X288P50         = 64 + 27,  ///< HDMI Video format is 1440x288 & Progressive 50fps
	PINMUX_HDMIMODE_1440X288P50_16X9    = 64 + 28,  ///< HDMI Video format is 1440x288 & Progressive 50fps & 16:9
	PINMUX_HDMIMODE_1440X576P50         = 64 + 29,  ///< HDMI Video format is 1440x576 & Progressive 50fps
	PINMUX_HDMIMODE_1440X576P50_16X9    = 64 + 30,  ///< HDMI Video format is 1440x576 & Progressive 50fps & 16:9
	PINMUX_HDMIMODE_1920X1080P50        = 64 + 31,  ///< HDMI Video format is 1920x1080 & Progressive 50fps
	PINMUX_HDMIMODE_1920X1080P24        = 64 + 32,  ///< HDMI Video format is 1920x1080 & Progressive 24fps
	PINMUX_HDMIMODE_1920X1080P25        = 64 + 33,  ///< HDMI Video format is 1920x1080 & Progressive 25fps
	PINMUX_HDMIMODE_1920X1080P30        = 64 + 34,  ///< HDMI Video format is 1920x1080 & Progressive 30fps
	PINMUX_HDMIMODE_1920X1080I50_VT1250 = 64 + 39,  ///< HDMI Video format is 1920x1080 & Interlaced  50fps & V-total is 1250 lines
	PINMUX_HDMIMODE_1920X1080I100       = 64 + 40,  ///< HDMI Video format is 1920x1080 & Interlaced  100fps
	PINMUX_HDMIMODE_1280X720P100        = 64 + 41,  ///< HDMI Video format is 1280X720  & Progressive 100fps
	PINMUX_HDMIMODE_720X576P100         = 64 + 42,  ///< HDMI Video format is 720X576   & Progressive 100fps
	PINMUX_HDMIMODE_720X576P100_16X9    = 64 + 43,  ///< HDMI Video format is 720X576   & Progressive 100fps & 16:9
	PINMUX_HDMIMODE_720X576I100         = 64 + 44,  ///< HDMI Video format is 720X576  & Interlaced 100fps
	PINMUX_HDMIMODE_720X576I100_16X9    = 64 + 45,  ///< HDMI Video format is 720X576  & Interlaced 100fps & 16:9
	PINMUX_HDMIMODE_1920X1080I120       = 64 + 46,  ///< HDMI Video format is 1920X1080 & Interlaced 120fps
	PINMUX_HDMIMODE_1280X720P120        = 64 + 47,  ///< HDMI Video format is 1280X720  & Progressive 120fps
	PINMUX_HDMIMODE_720X480P120         = 64 + 48,  ///< HDMI Video format is 720X480   & Progressive 120fps
	PINMUX_HDMIMODE_720X480P120_16X9    = 64 + 49,  ///< HDMI Video format is 720X480   & Progressive 120fps & 16:9
	PINMUX_HDMIMODE_720X480I120         = 64 + 50,  ///< HDMI Video format is 720X480  & Interlaced 120fps
	PINMUX_HDMIMODE_720X480I120_16X9    = 64 + 51,  ///< HDMI Video format is 720X480  & Interlaced 120fps & 16:9
	PINMUX_HDMIMODE_720X576P200         = 64 + 52,  ///< HDMI Video format is 720X576  & Progressive 200fps
	PINMUX_HDMIMODE_720X576P200_16X9    = 64 + 53,  ///< HDMI Video format is 720X576  & Progressive 200fps & 16:9
	PINMUX_HDMIMODE_720X576I200         = 64 + 54,  ///< HDMI Video format is 720X576  & Interlaced 200fps
	PINMUX_HDMIMODE_720X576I200_16X9    = 64 + 55,  ///< HDMI Video format is 720X576  & Interlaced 200fps & 16:9
	PINMUX_HDMIMODE_720X480P240         = 64 + 56,  ///< HDMI Video format is 720X480  & Progressive 240fps
	PINMUX_HDMIMODE_720X480P240_16X9    = 64 + 57,  ///< HDMI Video format is 720X480  & Progressive 240fps & 16:9
	PINMUX_HDMIMODE_720X480I240         = 64 + 58,  ///< HDMI Video format is 720X480  & Interlaced 240fps
	PINMUX_HDMIMODE_720X480I240_16X9    = 64 + 59,  ///< HDMI Video format is 720X480  & Interlaced 240fps & 16:9

	PINMUX_DSI_1_LANE_CMD_MODE_RGB565           = 128 + 0, ///< DSI command mode with RGB565 format   (not support DSI. Below is backward compatible)
	PINMUX_DSI_1_LANE_CMD_MODE_RGB666P          = 128 + 1, ///< DSI command mode with RGB666 packed
	PINMUX_DSI_1_LANE_CMD_MODE_RGB666L          = 128 + 2, ///< DSI command mode with RGB666 loosely
	PINMUX_DSI_1_LANE_CMD_MODE_RGB888           = 128 + 3, ///< DSI command mode with RGB888

	PINMUX_DSI_1_LANE_VDO_SYNC_PULSE_RGB565     = 128 + 4, ///< DSI video sync pulse mode with RGB565 format
	PINMUX_DSI_1_LANE_VDO_SYNC_PULSE_RGB666P    = 128 + 5, ///< DSI video sync pulse mode with RGB666 packed
	PINMUX_DSI_1_LANE_VDO_SYNC_PULSE_RGB666L    = 128 + 6, ///< DSI video sync pulse mode with RGB666 loosely
	PINMUX_DSI_1_LANE_VDO_SYNC_PULSE_RGB888     = 128 + 7, ///< DSI video sync pulse mode with RGB888

	PINMUX_DSI_1_LANE_VDO_SYNC_EVENT_RGB565     = 128 + 8, ///< DSI video sync event burst mode with RGB565 format
	PINMUX_DSI_1_LANE_VDO_SYNC_EVENT_RGB666P    = 128 + 9, ///< DSI video sync event burst mode with RGB666 packed
	PINMUX_DSI_1_LANE_VDO_SYNC_EVENT_RGB666L    = 128 + 10, ///< DSI video sync event burst mode with RGB666 loosely
	PINMUX_DSI_1_LANE_VDO_SYNC_EVENT_RGB888     = 128 + 11, ///< DSI video sync event burst mode with RGB888

	PINMUX_DSI_2_LANE_CMD_MODE_RGB565           = 128 + 12, ///< DSI command mode with RGB565 format
	PINMUX_DSI_2_LANE_CMD_MODE_RGB666P          = 128 + 13, ///< DSI command mode with RGB666 packed
	PINMUX_DSI_2_LANE_CMD_MODE_RGB666L          = 128 + 14, ///< DSI command mode with RGB666 loosely
	PINMUX_DSI_2_LANE_CMD_MODE_RGB888           = 128 + 15, ///< DSI command mode with RGB888

	PINMUX_DSI_2_LANE_VDO_SYNC_PULSE_RGB565     = 128 + 16, ///< DSI video sync pulse mode with RGB565 format
	PINMUX_DSI_2_LANE_VDO_SYNC_PULSE_RGB666P    = 128 + 17, ///< DSI video sync pulse mode with RGB666 packed
	PINMUX_DSI_2_LANE_VDO_SYNC_PULSE_RGB666L    = 128 + 18, ///< DSI video sync pulse mode with RGB666 loosely
	PINMUX_DSI_2_LANE_VDO_SYNC_PULSE_RGB888     = 128 + 19, ///< DSI video sync pulse mode with RGB888

	PINMUX_DSI_2_LANE_VDO_SYNC_EVENT_RGB565     = 128 + 20, ///< DSI video sync event burst mode with RGB565 format
	PINMUX_DSI_2_LANE_VDO_SYNC_EVENT_RGB666P    = 128 + 21, ///< DSI video sync event burst mode with RGB666 packed
	PINMUX_DSI_2_LANE_VDO_SYNC_EVENT_RGB666L    = 128 + 22, ///< DSI video sync event burst mode with RGB666 loosely
	PINMUX_DSI_2_LANE_VDO_SYNC_EVENT_RGB888     = 128 + 23, ///< DSI video sync event burst mode with RGB888

	PINMUX_DSI_4_LANE_CMD_MODE_RGB565           = 128 + 24, ///< DSI command mode with RGB565 format
	PINMUX_DSI_4_LANE_CMD_MODE_RGB666P          = 128 + 25, ///< DSI command mode with RGB666 packed
	PINMUX_DSI_4_LANE_CMD_MODE_RGB666L          = 128 + 26, ///< DSI command mode with RGB666 loosely
	PINMUX_DSI_4_LANE_CMD_MODE_RGB888           = 128 + 27, ///< DSI command mode with RGB888

	PINMUX_DSI_4_LANE_VDO_SYNC_PULSE_RGB565     = 128 + 28, ///< DSI video sync pulse mode with RGB565 format
	PINMUX_DSI_4_LANE_VDO_SYNC_PULSE_RGB666P    = 128 + 29, ///< DSI video sync pulse mode with RGB666 packed
	PINMUX_DSI_4_LANE_VDO_SYNC_PULSE_RGB666L    = 128 + 30, ///< DSI video sync pulse mode with RGB666 loosely
	PINMUX_DSI_4_LANE_VDO_SYNC_PULSE_RGB888     = 128 + 31, ///< DSI video sync pulse mode with RGB888

	PINMUX_DSI_4_LANE_VDO_SYNC_EVENT_RGB565     = 128 + 32, ///< DSI video sync event burst mode with RGB565 format
	PINMUX_DSI_4_LANE_VDO_SYNC_EVENT_RGB666P    = 128 + 33, ///< DSI video sync event burst mode with RGB666 packed
	PINMUX_DSI_4_LANE_VDO_SYNC_EVENT_RGB666L    = 128 + 34, ///< DSI video sync event burst mode with RGB666 loosely
	PINMUX_DSI_4_LANE_VDO_SYNC_EVENT_RGB888     = 128 + 35, ///< DSI video sync event burst mode with RGB888



	ENUM_DUMMY4WORD(PINMUX_LCDINIT)
} PINMUX_LCDINIT;

/**
    PIN config for TV/HDMI

    @note For pinmux_init() for PIN_FUNC_TV or PIN_FUNC_HDMI.
    For example, you can use {PIN_FUNC_HDMI, PINMUX_TV_HDMI_CFG_NORMAL|PINMUX_HDMIMODE_1280X720P60}
    to tell display object that HDMI activation will disable PANEL,
    and HDMI mode is 1280x720 P60.
*/
typedef enum {
	PINMUX_TV_HDMI_CFG_GPIO = 0x00,                 ///< TV activation will disable PINMUX to GPIO
	PINMUX_TV_HDMI_CFG_NORMAL = 0x00,               ///< TV activation will disable PANEL which shared the same IDE
	PINMUX_TV_HDMI_CFG_PINMUX_ON = 0x01 << 28,      ///< TV activation will keep PINMUX setting

	PINMUX_TV_HDMI_CFG_MASK = 0x03 << 28,
	ENUM_DUMMY4WORD(PINMUX_TV_HDMI_CFG)
} PINMUX_TV_HDMI_CFG;

/**
    PIN config for HDMI, not support (Backward compatible)

    @note For pinmux_init() for PIN_FUNC_HDMI.\n
            For example, you can use {PIN_FUNC_HDMI, PINMUX_HDMI_CFG_CEC|PINMUX_TV_HDMI_CFG_NORMAL}\n
            to declare HDMI CEC pinmux is enabled.
*/
typedef enum {
	PINMUX_HDMI_CFG_GPIO = 0x00,                    ///< HDMI specific PIN to GPIO
	PINMUX_HDMI_CFG_HOTPLUG = 0x01 << 26,           ///< HDMI HOTPLUG.
	PINMUX_HDMI_CFG_CEC = 0x02 << 26,               ///< HDMI CEC.

	PINMUX_HDMI_CFG_MASK = 0x03 << 26,
	ENUM_DUMMY4WORD(PINMUX_HDMI_CFG)
} PINMUX_HDMI_CFG;

/**
    PIN location of LCD

    @note For pinmux_init() with PIN_FUNC_LCD or PIN_FUNC_LCD2.
    For example, you can use {PIN_FUNC_LCD, PINMUX_DISPMUX_SEL_LCD2|PINMUX_LCDMODE_XXX}
    to tell display object that PIN_FUNC_LCD is located on secondary LCD pinmux.
*/
typedef enum {
	PINMUX_DISPMUX_SEL_NONE = 0x00 << 28,           ///< PINMUX none
	PINMUX_DISPMUX_SEL_LCD = 0x01 << 28,            ///< PINMUX at LCD interface
	PINMUX_DISPMUX_SEL_LCD2 = 0x02 << 28,           ///< PINMUX at LCD2 interface

	PINMUX_DISPMUX_SEL_MASK = 0x03 << 28,
	ENUM_DUMMY4WORD(PINMUX_DISPMUX_SEL)
} PINMUX_DISPMUX_SEL;

/**
    PIN config for USB

    @note For pinmux_init() for PIN_FUNC_USB.\n
            For example, you can use {PIN_FUNC_USB, PINMUX_USB_CFG_DEVICE}\n
            to select USB as device.
*/
typedef enum {
	PINMUX_USB_CFG_NONE = 0x00,                     ///< USB as device
	PINMUX_USB_CFG_DEVICE = 0x00,                   ///< USB as device
	PINMUX_USB_CFG_HOST = 0x01,                     ///< USB as host

	ENUM_DUMMY4WORD(PINMUX_USB_CFG)
} PINMUX_USB_CFG;

/**
    Pinmux power domain isolated function enable/disable

    @note For pinmux_set_direct_channel()
*/
typedef enum {
	PINMUX_ISOFUNC_DISABLE = 0x00,               ///< DISABLE power domain isolated function
	PINMUX_ISOFUNC_ENABLE = 0x01,                ///< ENABLE  power domain isolated function
	ENUM_DUMMY4WORD(PINMUX_POWER_ISOFUNC)
} PINMUX_POWER_ISOFUNC;

/**
    Pinmux Function identifier

    @note For pinmux_getDispMode(), pinmux_setPinmux().
*/
typedef enum {
	PINMUX_FUNC_ID_LCD,                             ///< 1st Panel (LCD), pinmux can be:
	///< - @b PINMUX_LCD_SEL_GPIO
	///< - @b PINMUX_LCD_SEL_CCIR656
	///< - @b PINMUX_LCD_SEL_CCIR656_16BITS
	///< - @b PINMUX_LCD_SEL_CCIR601
	///< - @b PINMUX_LCD_SEL_CCIR601_16BITS
	///< - @b PINMUX_LCD_SEL_SERIAL_RGB_6BITS
	///< - @b PINMUX_LCD_SEL_SERIAL_RGB_8BITS
	///< - @b PINMUX_LCD_SEL_SERIAL_YCbCr_8BITS
	///< - @b PINMUX_LCD_SEL_PARALLE_RGB565
	///< - @b PINMUX_LCD_SEL_PARALLE_RGB666
	///< - @b PINMUX_LCD_SEL_PARALLE_RGB888
	///< - @b PINMUX_LCD_SEL_RGB_16BITS
	///< - @b PINMUX_LCD_SEL_MIPI
	///< - @b PINMUX_LCD_SEL_PARALLE_MI_8BITS
	///< - @b PINMUX_LCD_SEL_PARALLE_MI_9BITS
	///< - @b PINMUX_LCD_SEL_PARALLE_MI_16BITS
	///< - @b PINMUX_LCD_SEL_PARALLE_MI_18BITS
	///< - @b PINMUX_LCD_SEL_SERIAL_MI_SDIO
	///< - @b PINMUX_LCD_SEL_SERIAL_MI_SDI_SDO
	///< ORed with
	///< - @b PINMUX_LCD_SEL_DE_ENABLE
	///< - @b PINMUX_LCD_SEL_TE_ENABLE
	///< - @b PINMUX_LCD_SEL_HVLD_VVLD
	///< - @b PINMUX_LCD_SEL_FIELD
	PINMUX_FUNC_ID_LCD2,                            ///< 2nd Panel (LCD), pinmux can be:
	///< - @b PINMUX_LCD_SEL_GPIO
	///< - @b PINMUX_LCD_SEL_CCIR656
	///< - @b PINMUX_LCD_SEL_CCIR601
	///< - @b PINMUX_LCD_SEL_SERIAL_RGB_6BITS
	///< - @b PINMUX_LCD_SEL_SERIAL_RGB_8BITS
	///< - @b PINMUX_LCD_SEL_SERIAL_YCbCr_8BITS
	///< - @b PINMUX_LCD_SEL_PARALLE_MI_8BITS
	///< - @b PINMUX_LCD_SEL_PARALLE_MI_9BITS
	///< - @b PINMUX_LCD_SEL_SERIAL_MI_SDIO
	///< - @b PINMUX_LCD_SEL_SERIAL_MI_SDI_SDO
	///< ORed with
	///< - @b PINMUX_LCD_SEL_DE_ENABLE
	PINMUX_FUNC_ID_TV,                              ///< TV, pinmux can be:
	///< - @b PINMUX_LCD_SEL_GPIO
	PINMUX_FUNC_ID_HDMI,                            ///< HDMI, pinmux can be:
	///< - @b PINMUX_LCD_SEL_GPIO
	PINMUX_FUNC_ID_COUNT,                           //< Total function count

	ENUM_DUMMY4WORD(PINMUX_FUNC_ID)
} PINMUX_FUNC_ID;

/**
    Pinmux selection for LCD

    @note For pinmux_init() with PIN_FUNC_SEL_LCD or PIN_FUNC_SEL_LCD2.
    For example, you can use {PIN_FUNC_SEL_LCD, PINMUX_LCD_SEL_RGB_16BITS | PINMUX_LCD_SEL_DE_ENABLE}
    to tell pinmux driver that the register of primary LCD should be set to RGB 16 bits
    and the register of PLCD_DE should be set.
*/
typedef enum {
	PINMUX_LCD_SEL_GPIO,                            ///< GPIO
	PINMUX_LCD_SEL_CCIR656,                         ///< CCIR-656 8 bits.
	                                                ///< When PIN_FUNC_SEL_LCD, CCIR_YC[0..7]/CCIR_CLK (L_GPIO[0..8])
	                                                ///< When PIN_FUNC_SEL_LCD2, CCIR_YC[0..7]/CCIR_CLK (L_GPIO[13..21])
	PINMUX_LCD_SEL_CCIR656_16BITS,                  ///< CCIR-656 16 bits. CCIR_Y[0..7]/CCIR_CLK/CCIR_C[0..7] (L_GPIO[0..8] L_GPIO[12..19])
	PINMUX_LCD_SEL_CCIR601,                         ///< CCIR-601 8 bits.
	                                                ///< When PIN_FUNC_SEL_LCD, CCIR_YC[0..7]/CCIR_CLK/CCIR_VD/CCIR_HD (L_GPIO[0..10])
	                                                ///< When PIN_FUNC_SEL_LCD2, CCIR_YC[0..7]/CCIR_CLK/CCIR_VD/CCIR_HD/CCIR_FIELD (L_GPIO[13..23] L_GPIO[25])
	PINMUX_LCD_SEL_CCIR601_16BITS,                  ///< CCIR-601 16 bits. CCIR_Y[0..7]/CCIR_CLK/CCIR_VD/CCIR_HD/CCIR_C[0..7] (L_GPIO[0..10] L_GPIO[12..19])
	PINMUX_LCD_SEL_SERIAL_RGB_6BITS,                ///< Serial RGB 6 bits.
	                                                ///< When PIN_FUNC_SEL_LCD, RGB_D[2..7]/RGB_CLK/RGB_VD/RGB_HD (L_GPIO[2..10])
	                                                ///< When PIN_FUNC_SEL_LCD2, RGB_D[2..7]/RGB_CLK/RGB_VD/RGB_HD (L_GPIO[15..23])
	PINMUX_LCD_SEL_SERIAL_RGB_8BITS,                ///< Serial RGB 8 bits.
	                                                ///< When PIN_FUNC_SEL_LCD, RGB_D[0..7]/RGB_CLK/RGB_VD/RGB_HD (L_GPIO[0..10])
	                                                ///< When PIN_FUNC_SEL_LCD2, RGB_D[0..7]/RGB_CLK/RGB_VD/RGB_HD (L_GPIO[13..23])
	PINMUX_LCD_SEL_SERIAL_YCbCr_8BITS,              ///< Serial YCbCr 8 bits.
	                                                ///< When PIN_FUNC_SEL_LCD, CCIR_YC[0..7]/CCIR_CLK/CCIR_VD/CCIR_HD (L_GPIO[0..10])
	                                                ///< When PIN_FUNC_SEL_LCD2, CCIR_YC[0..7]/CCIR_CLK/CCIR_VD/CCIR_HD (L_GPIO[13..23])
	PINMUX_LCD_SEL_PARALLE_RGB565,                  ///< Parallel RGB565. RGB_C0_[0..4]/RGB_DCLK/RGB_VS/RGB_HS/RGB_C1_[0..5]/RGB_C2_[0..4] (L_GPIO[0..10] L_GPIO[12..19])
	PINMUX_LCD_SEL_PARALLE_RGB666,                  ///< Parallel RGB666. RGB_C0_[0..5]/RGB_DCLK/RGB_VS/RGB_HS/RGB_C1_[0..5]/RGB_C2_[0..5] (L_GPIO[2..10] L_GPIO[14..19] L_GPIO[22..27])
	PINMUX_LCD_SEL_PARALLE_RGB888,                  ///< Parallel RGB888. RGB_C0_[0..7]/RGB_DCLK/RGB_VS/RGB_HS/RGB_C1_[0..7]/RGB_C2_[0..7] (L_GPIO[0..10] L_GPIO[12..27])
	PINMUX_LCD_SEL_RGB_16BITS,                      ///< RGB 16 bits. CCIR_Y[0..7]/CCIR_CLK/CCIR_VD/CCIR_HD/CCIR_C[0..7] (L_GPIO[0..10] L_GPIO[12..19])
	PINMUX_LCD_SEL_MIPI,                            ///< MIPI DSI
	PINMUX_LCD_SEL_PARALLE_MI_8BITS,                ///< Parallel MI 8 bits.
	                                                ///< When PIN_FUNC_SEL_LCD, MPU_D[0..7]/MPU_CS/MPU_RS/MPU_WR/MPU_RD (L_GPIO[0..7] L_GPIO[8..11])
	                                                ///< When PIN_FUNC_SEL_LCD2, MPU2_D[0..7]/MPU2_RS/MPU2_CS/MPU2_WR/MPU2_RD (L_GPIO[12..19] L_GPIO[21..24])
	PINMUX_LCD_SEL_PARALLE_MI_9BITS,                ///< Parallel MI 9 bits.
	                                                ///< When PIN_FUNC_SEL_LCD, MPU_D[0..8]/MPU_CS/MPU_RS/MPU_WR/MPU_RD (L_GPIO[0..7][12] L_GPIO[8..11])
	                                                ///< When PIN_FUNC_SEL_LCD2, MPU2_D[0..8]/MPU2_RS/MPU2_CS/MPU2_WR/MPU2_RD (L_GPIO[12..20] L_GPIO[21..24])
	PINMUX_LCD_SEL_PARALLE_MI_16BITS,               ///< Parallel MI 16 bits. MPU_D[0..15]/MPU_CS/MPU_RS/MPU_WR/MPU_RD (L_GPIO[0..7][12..19] L_GPIO[8..11])
	PINMUX_LCD_SEL_PARALLE_MI_18BITS,               ///< Parallel MI 18 bits. MPU_D[0..17]/MPU_CS/MPU_RS/MPU_WR/MPU_RD (L_GPIO[0..7][12..21] L_GPIO[8..11])
	PINMUX_LCD_SEL_SERIAL_MI_SDIO,                  ///< Serial MI SDIO bi-direction.
	                                                ///< When PIN_FUNC_SEL_LCD, MPU_CLK/MPU_RS/MPU_CS/MPU_SDIO (L_GPIO[8..10] L_GPIO[7])
	                                                ///< When PIN_FUNC_SEL_LCD2, MPU_CLK/MPU_RS/MPU_CS/MPU_SDIO (L_GPIO[15..17] L_GPIO[14])
	PINMUX_LCD_SEL_SERIAL_MI_SDI_SDO,               ///< Serial MI SDI/SDO seperate.
	                                                ///< When PIN_FUNC_SEL_LCD, MPU_CLK/MPU_RS/MPU_CS/MPU_SDO/MPU_SDI (L_GPIO[8..10] L_GPIO[5..6])
	                                                ///< When PIN_FUNC_SEL_LCD2, MPU_CLK/MPU_RS/MPU_CS/MPU_SDO/MPU_SDI (L_GPIO[15..17] L_GPIO[12..13])

	PINMUX_LCD_SEL_TE_ENABLE = 0x01 << 23,          ///< TE Enable.
	                                                ///< When PIN_FUNC_SEL_LCD, DSI_TE (DSI_GPIO[10])
	                                                ///< When PIN_FUNC_SEL_LCD2, DSI_TE (DSI_GPIO[10])
	                                                ///< When PIN_FUNC_SEL_LCD and PARALLE_MI, MPU_TE (L_GPIO[11])
	                                                ///< When PIN_FUNC_SEL_LCD2 and PARALLE_MI, MPU_TE (L_GPIO[18])
	                                                ///< When PIN_FUNC_SEL_LCD and SERIAL_MI, MI_TE (L_GPIO[22])
	                                                ///< When PIN_FUNC_SEL_LCD2 and SERIAL_MI, MI_TE (L_GPIO[25])
	PINMUX_LCD_SEL_DE_ENABLE = 0x01 << 24,          ///< DE Enable.
	                                                ///< When PIN_FUNC_SEL_LCD, CCIR_DE (L_GPIO[11])
	                                                ///< When PIN_FUNC_SEL_LCD2, CCIR2_DE (L_GPIO[24])
	PINMUX_LCD_SEL_HVLD_VVLD = 0x01 << 25,          ///< HVLD/VVLD Enable (For CCIR-601 8 bits). CCIR_HVLD/CCIR_VVLD (L_GPIO[12..13])
	PINMUX_LCD_SEL_FIELD = 0x01 << 26,              ///< FIELD Enable (For CCIR-601). CCIR_FIELD: CCIR-601 8 bits(L_GPIO[14]), CCIR-601 16 bits(L_GPIO[25])
	PINMUX_LCD_SEL_NO_HVSYNC = 0x1 << 27,           ///< No HSYNC/VSYNC (backward compatible)
	PINMUX_LCD_SEL_FEATURE_MSK = 0x1F << 23,

	ENUM_DUMMY4WORD(PINMUX_LCD_SEL)
} PINMUX_LCD_SEL;

/**
    Pinmux selection for Storage

    @note For PINMUX_FUNC_ID_SDIO, PINMUX_FUNC_ID_SPI, PINMUX_FUNC_ID_NAND
*/
typedef enum {
	PINMUX_STORAGE_SEL_INACTIVE,                    ///< Inactive storage
	PINMUX_STORAGE_SEL_ACTIVE,                      ///< Active storage
	PINMUX_STORAGE_SEL_INEXIST,                     ///< Inexist storage
	PINMUX_STORAGE_SEL_EXIST,                       ///< Exist storage
	ENUM_DUMMY4WORD(PINMUX_STORAGE_SEL)
} PINMUX_STORAGE_SEL;

/*
    Pinmux Function identifier for driver only

    @note For pinmux_setPinmux().
*/
typedef enum {
	PINMUX_FUNC_ID_SDIO = 0x8000000,                ///< SDIO, pinmux can be:
	///< - @b PINMUX_STORAGE_SEL_INACTIVE
	///< - @b PINMUX_STORAGE_SEL_ACTIVE
	PINMUX_FUNC_ID_SDIO2,                           ///< SDIO2, pinmux can be:
	///< - @b PINMUX_STORAGE_SEL_INACTIVE
	///< - @b PINMUX_STORAGE_SEL_ACTIVE
	PINMUX_FUNC_ID_SDIO3,                           ///< SDIO3, pinmux can be:
	///< - @b PINMUX_STORAGE_SEL_INACTIVE
	///< - @b PINMUX_STORAGE_SEL_ACTIVE
	PINMUX_FUNC_ID_SPI,                             ///< SPI, pinmux can be:
	///< - @b PINMUX_STORAGE_SEL_INACTIVE
	///< - @b PINMUX_STORAGE_SEL_ACTIVE
	PINMUX_FUNC_ID_NAND,                            ///< NAND, pinmux can be:
	///< - @b PINMUX_STORAGE_SEL_INACTIVE
	///< - @b PINMUX_STORAGE_SEL_ACTIVE
	PINMUX_FUNC_ID_BMC,                             ///< BMC, pinmux can be:
	///< - @b PINMUX_STORAGE_SEL_INACTIVE
	///< - @b PINMUX_STORAGE_SEL_ACTIVE
	PINMUX_FUNC_ID_USB_VBUSI,                       ///< USB VBUSI, pinmux can be:
	///< - @b PINMUX_USB_SEL_INACTIVE
	///< - @b PINMUX_USB_SEL_ACTIVE
	PINMUX_FUNC_ID_USB2_VBUSI,                      ///< USB2 VBUSI, pinmux can be:
	///< - @b PINMUX_USB_SEL_INACTIVE
	///< - @b PINMUX_USB_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C1_1ST,                        ///< I2C channel1 1st pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C1_2ND,                        ///< I2C channel1 2nd pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C2_1ST,                        ///< I2C channel2 1st pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C2_2ND,                        ///< I2C channel2 2nd pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C3_1ST,                        ///< I2C channel3 1st pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C3_2ND,                        ///< I2C channel3 2nd pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C3_3RD,                        ///< I2C channel3 3rd pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C4_1ST,                        ///< I2C channel4 1st pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C4_2ND,                        ///< I2C channel4 2nd pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C4_3RD,                        ///< I2C channel4 3rd pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C4_4TH,                        ///< I2C channel4 4th pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C5_1ST,                        ///< I2C channel5 1st pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_I2C5_2ND,                        ///< I2C channel5 2nd pinmux, pinmux can be:
	///< - @b PINMUX_I2C_SEL_INACTIVE
	///< - @b PINMUX_I2C_SEL_ACTIVE
	PINMUX_FUNC_ID_SN_VDHD,                         ///< SN VD/HD, pinmux can be:
	///< - @b PINMUX_SENSOR_SEL_INACTIVE
	///< - @b PINMUX_SENSOR_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_0,                           ///< PWM channel 0
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_1,                           ///< PWM channel 1
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_2,                           ///< PWM channel 2
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_3,                           ///< PWM channel 3
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_4,                           ///< PWM channel 4
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_5,                           ///< PWM channel 5
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_6,                           ///< PWM channel 6
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_7,                           ///< PWM channel 7
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_8,                           ///< PWM channel 8
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_9,                           ///< PWM channel 9
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_10,                          ///< PWM channel 10
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_11,                          ///< PWM channel 11
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_12,                          ///< PWM channel 12
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_13,                          ///< PWM channel 13
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_14,                          ///< PWM channel 14
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_15,                          ///< PWM channel 15
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_16,                          ///< PWM channel 16
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_17,                          ///< PWM channel 17
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_18,                          ///< PWM channel 18
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_PWM_19,                          ///< PWM channel 19
	///< - @b PINMUX_PWM_SEL_INACTIVE
	///< - @b PINMUX_PWM_SEL_ACTIVE
	PINMUX_FUNC_ID_SIF_0,                           ///< SIF channel 0
	///< - @b PINMUX_SIF_SEL_INACTIVE
	///< - @b PINMUX_SIF_SEL_ACTIVE
	PINMUX_FUNC_ID_SIF_1,                           ///< SIF channel 1
	///< - @b PINMUX_SIF_SEL_INACTIVE
	///< - @b PINMUX_SIF_SEL_ACTIVE
	PINMUX_FUNC_ID_SIF_2,                           ///< SIF channel 2
	///< - @b PINMUX_SIF_SEL_INACTIVE
	///< - @b PINMUX_SIF_SEL_ACTIVE
	PINMUX_FUNC_ID_SIF_3,                           ///< SIF channel 3
	///< - @b PINMUX_SIF_SEL_INACTIVE
	///< - @b PINMUX_SIF_SEL_ACTIVE
	PINMUX_FUNC_ID_SIF_4,                           ///< SIF channel 4
	///< - @b PINMUX_SIF_SEL_INACTIVE
	///< - @b PINMUX_SIF_SEL_ACTIVE
	PINMUX_FUNC_ID_SIF_5,                           ///< SIF channel 5
	///< - @b PINMUX_SIF_SEL_INACTIVE
	///< - @b PINMUX_SIF_SEL_ACTIVE
	PINMUX_FUNC_ID_SIF_6,                           ///< SIF channel 6
	///< - @b PINMUX_SIF_SEL_INACTIVE
	///< - @b PINMUX_SIF_SEL_ACTIVE
	PINMUX_FUNC_ID_SIF_7,                           ///< SIF channel 7
	///< - @b PINMUX_SIF_SEL_INACTIVE
	///< - @b PINMUX_SIF_SEL_ACTIVE
	PINMUX_FUNC_ID_UART_1,                          ///< UART channel 1
	///< - @b PINMUX_UART_SEL_INACTIVE
	///< - @b PINMUX_UART_SEL_ACTIVE
	PINMUX_FUNC_ID_UART_2,                          ///< UART channel 2
	///< - @b PINMUX_UART_SEL_INACTIVE
	///< - @b PINMUX_UART_SEL_ACTIVE
	PINMUX_FUNC_ID_UART_3,                          ///< UART channel 3
	///< - @b PINMUX_UART_SEL_INACTIVE
	///< - @b PINMUX_UART_SEL_ACTIVE
	PINMUX_FUNC_ID_UART_4,                          ///< UART channel 4
	///< - @b PINMUX_UART_SEL_INACTIVE
	///< - @b PINMUX_UART_SEL_ACTIVE
	PINMUX_FUNC_ID_I2S,                             ///< I2S
	///< - @b PINMUX_I2S_SEL_INACTIVE
	///< - @b PINMUX_I2S_SEL_ACTIVE
	PINMUX_FUNC_ID_I2S_MCLK,                        ///< I2S MCLK
	///< - @b PINMUX_I2S_SEL_INACTIVE
	///< - @b PINMUX_I2S_SEL_ACTIVE
	PINMUX_FUNC_ID_DSP_PERI_CH_EN,                  ///< DSP PERI channel enable
	///< - @b PINMUX_DSP_CH_INACTIVE
	///< - @b PINMUX_DSP_CH_ACTIVE
	PINMUX_FUNC_ID_DSP_IOP_CH_EN,                   ///< DSP IOP channel enable
	///< - @b PINMUX_DSP_CH_INACTIVE
	///< - @b PINMUX_DSP_CH_ACTIVE
	PINMUX_FUNC_ID_DSP2_PERI_CH_EN,                 ///< DSP2 PERI channel enable
	///< - @b PINMUX_DSP_CH_INACTIVE
	///< - @b PINMUX_DSP_CH_ACTIVE
	PINMUX_FUNC_ID_DSP2_IOP_CH_EN,                  ///< DSP2 IOP channel enable
	///< - @b PINMUX_DSP_CH_INACTIVE
	///< - @b PINMUX_DSP_CH_ACTIVE

	ENUM_DUMMY4WORD(PINMUX_FUNC_ID_DRV)
} PINMUX_FUNC_ID_DRV;

/**
    Pinmux selection for boot source

    @note For BOOT_SRC_ENUM
*/
typedef enum
{
	BOOT_SRC_SPINOR,                    //< Boot from SPI NOR
	BOOT_SRC_CARD,                      //< Boot from CARD
	BOOT_SRC_SPINAND,                   //< Boot from SPI NAND with on die ECC (2K page)
	BOOT_SRC_SPINAND_RS,                //< Boot from SPI NAND (Reed Solomon ECC) (2K page)
	BOOT_SRC_ETH,                       //< Boot from Ethernet
	BOOT_SRC_USB_HS,                    //< Boot from USB (High Speed)
	BOOT_SRC_SPINAND_4K,                //< Boot from SPI NAND with on die ECC (4K page)
	BOOT_SRC_RESERVED,                  //< RESERVED
	BOOT_SRC_EMMC_4BIT,                 //< Boot from eMMC 4-bit
	BOOT_SRC_EMMC_8BIT,                 //< Boot from eMMC 8-bit
	BOOT_SRC_SPINAND_RS_4K,             //< Boot from SPI NAND (Reed Solomon ECC) (4K page)
	BOOT_SRC_USB_FS,                    //< Boot from USB (Full Speed)
	ENUM_DUMMY4WORD(BOOT_SRC_ENUM)
} BOOT_SRC_ENUM;


/**
    Pinmux group

    @note For pinmux_init()
*/
typedef struct {
	PIN_FUNC    pin_function;            ///< PIN Function group
	UINT32      config;                 ///< Configuration for pinFunction
} PIN_GROUP_CONFIG;


/*
enum CHIP_ID {
	CHIP_NA51055 = 0x4821,
	CHIP_NA51084 = 0x5021,
	CHIP_NA51089 = 0x7021,
	CHIP_NA51090 = 0xBC21,
	CHIP_NA51102 = 0x5221,
};*/

int nvt_pinmux_capture(PIN_GROUP_CONFIG *pinmux_config, int count);
int nvt_pinmux_update(PIN_GROUP_CONFIG *pinmux_config, int count);
int pinmux_set_config(PINMUX_FUNC_ID id, uint32_t pinmux);
extern void gpio_func_show(void);
int nvt_pinmux_probe(void);
//UINT32 nvt_get_chip_id(void);
PINMUX_LCDINIT pinmux_get_dispmode(PINMUX_FUNC_ID id);

int nvt_pinmux_init(void);
int pinmux_select_debugport(PINMUX_DEBUGPORT uiDebug);
UINT32 top_get_bs(void);

#endif
