/*
	Pinmux module internal header file

	@file		top_reg.h
	@ingroup
	@note		Nothing

	Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 2 as
	published by the Free Software Foundation.
*/

#ifndef _PINMUX_INT_H
#define _PINMUX_INT_H

#include <asm/nvt-common/rcw_macro.h>
#define ENUM_DUMMY4WORD(name)   E_##name = 0x10000000

#define MUX_0                    0x00
#define MUX_1                    0x01
#define MUX_2                    0x02
#define MUX_3                    0x03
#define MUX_4                    0x04
#define MUX_5                    0x05
#define MUX_6                    0x06
#define MUX_7                    0x07
#define MUX_8                    0x08
#define MUX_9                    0x09
#define MUX_10                   0x0A

/*
	GPIO_ID_ENUM
*/
typedef enum
{
	GPIO_ID_EMUM_FUNC,                  //< pinmux is mapping to function
	GPIO_ID_EMUM_GPIO,                  //< pinmux is mapping to gpio

	ENUM_DUMMY4WORD(GPIO_ID_ENUM)
} GPIO_ID_ENUM;

// 0x00 TOP Control Register 0 (BOOT)
#define TOP_REG0_OFS                        0x00
union TOP_REG0 {
	uint32_t reg;
	struct {
	unsigned int BOOT_SRC:4;                // Boot Source Selection
	unsigned int EJTAG_CH_SEL:1;            // EJTAG channel select (EJTAG_1 or EJTAG_2)
	unsigned int EJTAG_SEL:1;               // EJTAG select
	unsigned int MPLL_CLKSELL:1;            // PLL clock output mux
	unsigned int DSI_PROT_EN:1;
	unsigned int ETH_MODE_SEL:2;            // eth mode select
	unsigned int DEBUG_MODE_SEL:2;          // MIPS debug mode select
	unsigned int WDT_FAIL_RESET_EN:1;       // WDT fail reset function enable
	unsigned int reserved0:19;
	} bit;
};

// 0x04 TOP Control Register 1 (Storage)
#define TOP_REG1_OFS                        0x04
union TOP_REG1 {
	uint32_t reg;
	struct {
	unsigned int FSPI_EN:1;                 // FSPI enable control
	unsigned int reserved0:1;
	unsigned int FSPI_CS1_EN:1;
	unsigned int reserved1:1;
	unsigned int SDIO_EN:1;                 // SDIO enable control
	unsigned int reserved2:3;
	unsigned int SDIO2_EN:1;                // SDIO2 enable control
	unsigned int reserved3:3;
	unsigned int SDIO3_EN:1;                // SDIO3 enable control
	unsigned int reserved4:1;
	unsigned int SDIO3_BUS_WIDTH:1;         // Select SDIO3 bus width
	unsigned int SDIO3_DS_EN:1;             // Select SDIO3 data strobe
	unsigned int EJTAG_EN:1;                // EJTAG enable control
	unsigned int EXTROM_EN:1;		        // EXTROM enable control
	unsigned int reserved5:14;
	} bit;
};

// 0x08 TOP Control Register 2 (LCD)
#define TOP_REG2_OFS                        0x08
union TOP_REG2 {
	uint32_t reg;
	struct {
	unsigned int LCD_TYPE:4;                // Pinmux of LCD interface
	unsigned int PLCD_DE:1;                 // Pinmux of DE (for parallel LCD)
	unsigned int CCIR_DATA_WIDTH:1;         // CCIR data width
	unsigned int CCIR_HVLD_VVLD:1;          // CCIR VVLD, HVLD select
	unsigned int CCIR_FIELD:1;              // CCIR FIELD select
	unsigned int TE_SEL:1;                  // Memory Interface TE select
	unsigned int reserved0:3;
	unsigned int LCD2_TYPE:4;
	unsigned int PLCD2_DE:1;
	unsigned int reserved1:3;
	unsigned int MEMIF_TYPE:2;
	unsigned int MEMIF_SEL:1;
	unsigned int SMEMIF_DATA_WIDTH:1;
	unsigned int PMEMIF_DATA_WIDTH:2;
	unsigned int MEMIF_TE_SEL:1;
	unsigned int reserved2:1;
	unsigned int MIPI_SEL:1;
	unsigned int reserved3:3;
	} bit;
};

// 0x0C TOP Control Register 3 (ETH)
#define TOP_REG3_OFS                        0x0C
union TOP_REG3 {
	uint32_t reg;
	struct {
	unsigned int ETH:4;
	unsigned int ETH2:4;
	unsigned int ETH_EXT_PHY_CLK:1;
	unsigned int reserved0:1;
	unsigned int ETH2_EXT_PHY_CLK:1;
	unsigned int reserved1:3;
	unsigned int ETH_PTP:2;
	unsigned int ETH2_PTP:2;
	unsigned int reserved2:14;
	} bit;
};

// 0x10 TOP Control Register 4 (I2C[1/2])
#define TOP_REG4_OFS                        0x10
union TOP_REG4 {
	uint32_t reg;
	struct {
	unsigned int I2C:4;
	unsigned int I2C2:4;
	unsigned int I2C3:4;
	unsigned int I2C4:4;
	unsigned int I2C5:4;
	unsigned int I2C6:4;
	unsigned int I2C7:4;
	unsigned int I2C8:4;
	} bit;
};

// 0x14 TOP Control Register 5 (I2C[2/2])
#define TOP_REG5_OFS                        0x14
union TOP_REG5 {
	uint32_t reg;
	struct {
	unsigned int I2C9:4;
	unsigned int I2C10:4;
	unsigned int I2C11:4;
	unsigned int reserved0:20;
	} bit;
};

// 0x18 TOP Control Register 6 (PWM[1/2])
#define TOP_REG6_OFS                        0x18
union TOP_REG6 {
	uint32_t reg;
	struct {
	unsigned int PWM0:4;
	unsigned int PWM1:4;
	unsigned int PWM2:4;
	unsigned int PWM3:4;
	unsigned int PWM4:4;
	unsigned int PWM5:4;
	unsigned int PWM6:4;
	unsigned int PWM7:4;
	} bit;
};

// 0x1C TOP Control Register 7 (PWM[2/2])
#define TOP_REG7_OFS                        0x1C
union TOP_REG7 {
	uint32_t reg;
	struct {
	unsigned int PWM8:4;
	unsigned int PWM9:4;
	unsigned int PWM10:4;
	unsigned int PWM11:4;
	unsigned int PICNT:4;
	unsigned int PICNT2:4;
	unsigned int PICNT3:4;
	unsigned int reserved0:4;
	} bit;
};

//0x20 TOP Control Register 8 (System)
#define TOP_REG8_OFS                        0x20
union TOP_REG8 {
	uint32_t reg;
	struct {
	unsigned int RAM_SRC_SELECT:1;          // RAM SRC Select
	unsigned int reserved0:3;
	unsigned int RAM_SELECT:4;
	unsigned int DSP_EJTAG_EN:1;
	unsigned int reserved1:7;
	unsigned int MAU_PROT_EN:1;
	unsigned int USB_PROT_EN:1;
	unsigned int USB3_PROT_EN:1;
	unsigned int SATA_PROT_EN:1;
	unsigned int reserved2:12;
	} bit;
};

// 0x24 TOP Control Register 9 (Sensor[1/3])
#define TOP_REG9_OFS                        0x24
union TOP_REG9 {
	uint32_t reg;
	struct {
	unsigned int SENSOR:4;                  // Pinmux of SN interface
	unsigned int SENSOR2:4;                 // Pinmux of SN2 interface
	unsigned int SENSOR3:4;                 // Pinmux of SN3 interface
	unsigned int SN2_CCIR_VSHS:2;           // Pinmux of SN2 CCIR VS/HS/FIELD
	unsigned int SN3_CCIR_VSHS:2;           // Pinmux of SN3 CCIR VS/HS/FIELD
	unsigned int FLASH_TRIG_IN:2;
	unsigned int FLASH_TRIG_OUT:2;
	unsigned int HSI2HSI3_SEL:1;
	unsigned int reserved0:11;
	} bit;
};

// 0x28 TOP Control Register 10 (Sensor[2/3])
#define TOP_REG10_OFS                       0x28
union TOP_REG10 {
	uint32_t reg;
	struct {
	unsigned int SN_MCLK:2;                 // Pinmux of SN MCLK
	unsigned int SN_XVSXHS:2;               // Pinmux of SN XVS/XHS
	unsigned int SN2_MCLK:2;                // Pinmux of SN2 MCLK
	unsigned int SN2_XVSXHS:2;              // Pinmux of SN2 XVS/XHS
	unsigned int SN3_MCLK:2;                // Pinmux of SN3 MCLK
	unsigned int SN3_XVSXHS:2;              // Pinmux of SN3 XVS/XHS
	unsigned int SN4_MCLK:2;                // Pinmux of SN4 MCLK
	unsigned int SN4_XVSXHS:2;              // Pinmux of SN4 XVS/XHS
	unsigned int SN5_MCLK:2;                // Pinmux of SN5 MCLK
	unsigned int SN5_XVSXHS:2;              // Pinmux of SN5 XVS/XHS
	unsigned int reserved0:12;
	} bit;
};

// 0x2C TOP Control Register 11 (Sensor[3/3])
#define TOP_REG11_OFS                       0x2C
union TOP_REG11 {
	uint32_t reg;
	struct {
	unsigned int SN2_MCLK_SRC:4;            // Pinmux of SN2_MCLK_SRC (from CG)
	unsigned int SN2_XVSHS_SRC:4;           // Pinmux of SN2_XVSHS_SRC (TG)
	unsigned int SN3_MCLK_SRC:4;
	unsigned int SN3_XVSHS_SRC:4;
	unsigned int SN4_MCLK_SRC:4;
	unsigned int SN4_XVSHS_SRC:4;
	unsigned int SN5_MCLK_SRC:4;
	unsigned int SN5_XVSHS_SRC:4;
	} bit;
};

// 0x30 TOP Control Register 12 (Audio)
#define TOP_REG12_OFS                       0x30
union TOP_REG12 {
	uint32_t reg;
	struct {
	unsigned int I2S:2;
	unsigned int I2S_MCLK:2;
	unsigned int DMIC_DATA0:1;
	unsigned int DMIC_DATA1:1;
	unsigned int DMIC:2;
	unsigned int reserved0:8;
	unsigned int I2S2:2;
	unsigned int I2S2_MCLK:2;
	unsigned int EXT_EAC_MCLK:1;
	unsigned int reserved1:11;
	} bit;
};

// 0x34 TOP Control Register 13 (UART[1/2])
#define TOP_REG13_OFS                       0x34
union TOP_REG13 {
	uint32_t reg;
	struct {
	unsigned int UART:4;
	unsigned int UART2:4;
	unsigned int UART3:4;
	unsigned int UART4:4;
	unsigned int UART5:4;
	unsigned int UART6:4;
	unsigned int UART7:4;
	unsigned int UART8:4;
	} bit;
};

// 0x38 TOP Control Register 14 (UART[2/2])
#define TOP_REG14_OFS                       0x38
union TOP_REG14 {
	uint32_t reg;
	struct {
	unsigned int UART9:4;
	unsigned int reserved0:2;
	unsigned int UART2_RTSCTS:2;
	unsigned int UART3_RTSCTS:2;
	unsigned int UART4_RTSCTS:2;
	unsigned int UART5_RTSCTS:2;
	unsigned int UART6_RTSCTS:2;
	unsigned int UART7_RTSCTS:2;
	unsigned int UART8_RTSCTS:2;
	unsigned int UART9_RTSCTS:2;
	unsigned int reserved1:10;
	} bit;
};

// 0x3C Reserved

// 0x40 TOP Control Register 15 (Remote/SDP)
#define TOP_REG15_OFS                       0x40
union TOP_REG15 {
	uint32_t reg;
	struct {
	unsigned int REMOTE:8;
	unsigned int REMOTE_EXT:4;
	unsigned int reserved0:4;
	unsigned int SDP:2;
	unsigned int reserved1:14;
	} bit;
};

// 0x44 TOP Control Register 16 (SPI)
#define TOP_REG16_OFS                       0x44
union TOP_REG16 {
	uint32_t reg;
	struct {
	unsigned int SPI:4;
	unsigned int SPI2:4;
	unsigned int SPI3:4;
	unsigned int SPI4:4;
	unsigned int SPI5:4;
	unsigned int SPI_BUS_WIDTH:1;
	unsigned int SPI2_BUS_WIDTH:1;
	unsigned int SPI3_BUS_WIDTH:1;
	unsigned int SPI4_BUS_WIDTH:1;
	unsigned int SPI5_BUS_WIDTH:1;
	unsigned int reserved0:3;
	unsigned int SPI3_RDY:4;
	} bit;
};

// 0x48 TOP Control Register 17 (SIF)
#define TOP_REG17_OFS                       0x48
union TOP_REG17 {
	uint32_t reg;
	struct {
	unsigned int SIF0:4;
	unsigned int SIF1:4;
	unsigned int SIF2:4;
	unsigned int SIF3:4;
	unsigned int SIF4:4;
	unsigned int SIF5:4;
	unsigned int reserved0:8;
	} bit;
};

// 0x4C Reserved

// 0x50 TOP Control Register 18 (SP_CLK/RTC)
#define TOP_REG18_OFS                       0x50
union TOP_REG18 {
	uint32_t reg;
	struct {
	unsigned int SP_CLK:2;
	unsigned int SP_CLK2:2;
	unsigned int reserved0:12;
	unsigned int RTC_CLK:2;
	unsigned int RTC_EXT_CLK:1;
	unsigned int RTC_DIV_OUT:1;
	unsigned int reserved1:12;
	} bit;
};

// 0x54 TOP Control Register 19 (SATA)
#define TOP_REG19_OFS                       0x54
union TOP_REG19 {
	uint32_t reg;
	struct {
	unsigned int SATA_LED:4;
	unsigned int reserved0:28;
	} bit;
};

// 0xA0 CGPIO Control Register 0 (C_GPIO)
#define TOP_REGCGPIO0_OFS                   0xA0
union TOP_REGCGPIO0 {
	uint32_t reg;
	struct {
	unsigned int CGPIO_0:1;
	unsigned int CGPIO_1:1;
	unsigned int CGPIO_2:1;
	unsigned int CGPIO_3:1;
	unsigned int CGPIO_4:1;
	unsigned int CGPIO_5:1;
	unsigned int CGPIO_6:1;
	unsigned int CGPIO_7:1;
	unsigned int CGPIO_8:1;
	unsigned int CGPIO_9:1;
	unsigned int CGPIO_10:1;
	unsigned int CGPIO_11:1;
	unsigned int CGPIO_12:1;
	unsigned int CGPIO_13:1;
	unsigned int CGPIO_14:1;
	unsigned int CGPIO_15:1;
	unsigned int CGPIO_16:1;
	unsigned int CGPIO_17:1;
	unsigned int CGPIO_18:1;
	unsigned int CGPIO_19:1;
	unsigned int CGPIO_20:1;
	unsigned int CGPIO_21:1;
	unsigned int CGPIO_22:1;
	unsigned int reserved0:9;
	} bit;
};

// 0xA4 Reserved

// 0xA8 PGPIO Control Register 0 (P_GPIO[1/2])
#define TOP_REGPGPIO0_OFS                   0xA8
union TOP_REGPGPIO0 {
	uint32_t reg;
	struct {
	unsigned int PGPIO_0:1;
	unsigned int PGPIO_1:1;
	unsigned int PGPIO_2:1;
	unsigned int PGPIO_3:1;
	unsigned int PGPIO_4:1;
	unsigned int PGPIO_5:1;
	unsigned int PGPIO_6:1;
	unsigned int PGPIO_7:1;
	unsigned int PGPIO_8:1;
	unsigned int PGPIO_9:1;
	unsigned int PGPIO_10:1;
	unsigned int PGPIO_11:1;
	unsigned int PGPIO_12:1;
	unsigned int PGPIO_13:1;
	unsigned int PGPIO_14:1;
	unsigned int PGPIO_15:1;
	unsigned int PGPIO_16:1;
	unsigned int PGPIO_17:1;
	unsigned int PGPIO_18:1;
	unsigned int PGPIO_19:1;
	unsigned int PGPIO_20:1;
	unsigned int PGPIO_21:1;
	unsigned int PGPIO_22:1;
	unsigned int PGPIO_23:1;
	unsigned int PGPIO_24:1;
	unsigned int PGPIO_25:1;
	unsigned int PGPIO_26:1;
	unsigned int PGPIO_27:1;
	unsigned int PGPIO_28:1;
	unsigned int PGPIO_29:1;
	unsigned int PGPIO_30:1;
	unsigned int PGPIO_31:1;
	} bit;
};

// 0xAC PGPIO Control Register 1 (P_GPIO[2/2])
#define TOP_REGPGPIO1_OFS                   0xAC
union TOP_REGPGPIO1 {
	uint32_t reg;
	struct {
	unsigned int PGPIO_32:1;
	unsigned int PGPIO_33:1;
	unsigned int PGPIO_34:1;
	unsigned int PGPIO_35:1;
	unsigned int PGPIO_36:1;
	unsigned int PGPIO_37:1;
	unsigned int reserved0:26;
	} bit;
};

// 0xB0 Reserved

// 0xB4 DGPIO Control Register 0 (D_GPIO)
#define TOP_REGDGPIO0_OFS                   0xB4
union TOP_REGDGPIO0 {
	uint32_t reg;
	struct {
	unsigned int DGPIO_0:1;
	unsigned int DGPIO_1:1;
	unsigned int DGPIO_2:1;
	unsigned int DGPIO_3:1;
	unsigned int DGPIO_4:1;
	unsigned int DGPIO_5:1;
	unsigned int DGPIO_6:1;
	unsigned int DGPIO_7:1;
	unsigned int DGPIO_8:1;
	unsigned int DGPIO_9:1;
	unsigned int DGPIO_10:1;
	unsigned int DGPIO_11:1;
	unsigned int DGPIO_12:1;
	unsigned int DGPIO_13:1;
	unsigned int reserved0:18;
	} bit;
};

// 0xB8 LGPIO Control Register 0 (L_GPIO)
#define TOP_REGLGPIO0_OFS                    0xB8
union TOP_REGLGPIO0 {
	uint32_t reg;
	struct {
	unsigned int LGPIO_0:1;
	unsigned int LGPIO_1:1;
	unsigned int LGPIO_2:1;
	unsigned int LGPIO_3:1;
	unsigned int LGPIO_4:1;
	unsigned int LGPIO_5:1;
	unsigned int LGPIO_6:1;
	unsigned int LGPIO_7:1;
	unsigned int LGPIO_8:1;
	unsigned int LGPIO_9:1;
	unsigned int LGPIO_10:1;
	unsigned int LGPIO_11:1;
	unsigned int LGPIO_12:1;
	unsigned int LGPIO_13:1;
	unsigned int LGPIO_14:1;
	unsigned int LGPIO_15:1;
	unsigned int LGPIO_16:1;
	unsigned int LGPIO_17:1;
	unsigned int LGPIO_18:1;
	unsigned int LGPIO_19:1;
	unsigned int LGPIO_20:1;
	unsigned int LGPIO_21:1;
	unsigned int LGPIO_22:1;
	unsigned int LGPIO_23:1;
	unsigned int LGPIO_24:1;
	unsigned int LGPIO_25:1;
	unsigned int LGPIO_26:1;
	unsigned int LGPIO_27:1;
	unsigned int LGPIO_28:1;
	unsigned int LGPIO_29:1;
	unsigned int LGPIO_30:1;
	unsigned int reserved0:1;
	} bit;
};

// 0xBC Reserved

// 0xC0 SGPIO Control Register 0 (S_GPIO)
#define TOP_REGSGPIO0_OFS                    0xC0
union TOP_REGSGPIO0 {
	uint32_t reg;
	struct {
	unsigned int SGPIO_0:1;
	unsigned int SGPIO_1:1;
	unsigned int SGPIO_2:1;
	unsigned int SGPIO_3:1;
	unsigned int SGPIO_4:1;
	unsigned int SGPIO_5:1;
	unsigned int SGPIO_6:1;
	unsigned int SGPIO_7:1;
	unsigned int SGPIO_8:1;
	unsigned int SGPIO_9:1;
	unsigned int SGPIO_10:1;
	unsigned int SGPIO_11:1;
	unsigned int SGPIO_12:1;
	unsigned int SGPIO_13:1;
	unsigned int SGPIO_14:1;
	unsigned int SGPIO_15:1;
	unsigned int SGPIO_16:1;
	unsigned int SGPIO_17:1;
	unsigned int SGPIO_18:1;
	unsigned int SGPIO_19:1;
	unsigned int SGPIO_20:1;
	unsigned int SGPIO_21:1;
	unsigned int SGPIO_22:1;
	unsigned int SGPIO_23:1;
	unsigned int SGPIO_24:1;
	unsigned int SGPIO_25:1;
	unsigned int SGPIO_26:1;
	unsigned int reserved0:5;
	} bit;
};

// 0xC4~0xCC Reserved

// 0xD0 HSIGPIO Control Register 0 (HSI_GPIO)
#define TOP_REGHSIGPIO0_OFS                  0xD0
union TOP_REGHSIGPIO0 {
	uint32_t reg;
	struct {
	unsigned int HSIGPIO_0:1;
	unsigned int HSIGPIO_1:1;
	unsigned int HSIGPIO_2:1;
	unsigned int HSIGPIO_3:1;
	unsigned int HSIGPIO_4:1;
	unsigned int HSIGPIO_5:1;
	unsigned int HSIGPIO_6:1;
	unsigned int HSIGPIO_7:1;
	unsigned int HSIGPIO_8:1;
	unsigned int HSIGPIO_9:1;
	unsigned int HSIGPIO_10:1;
	unsigned int HSIGPIO_11:1;
	unsigned int HSIGPIO_12:1;
	unsigned int HSIGPIO_13:1;
	unsigned int HSIGPIO_14:1;
	unsigned int HSIGPIO_15:1;
	unsigned int HSIGPIO_16:1;
	unsigned int HSIGPIO_17:1;
	unsigned int HSIGPIO_18:1;
	unsigned int HSIGPIO_19:1;
	unsigned int HSIGPIO_20:1;
	unsigned int HSIGPIO_21:1;
	unsigned int HSIGPIO_22:1;
	unsigned int HSIGPIO_23:1;
	unsigned int reserved0:8;
	} bit;
};

// 0xD4 Reserved

// 0xD8 DSIGPIO Control Register 0 (DSI_GPIO)
#define TOP_REGDSIGPIO0_OFS                  0xD8
union TOP_REGDSIGPIO0 {
	uint32_t reg;
	struct {
	unsigned int DSIGPIO_0:1;
	unsigned int DSIGPIO_1:1;
	unsigned int DSIGPIO_2:1;
	unsigned int DSIGPIO_3:1;
	unsigned int DSIGPIO_4:1;
	unsigned int DSIGPIO_5:1;
	unsigned int DSIGPIO_6:1;
	unsigned int DSIGPIO_7:1;
	unsigned int DSIGPIO_8:1;
	unsigned int DSIGPIO_9:1;
	unsigned int DSIGPIO_10:1;
	unsigned int reserved0:21;
	} bit;
};

// 0xDC~0xE4 Reserved

// 0xE8 AGPIO Control Register 0 (A_GPIO)
#define TOP_REGAGPIO0_OFS                    0xE8
union TOP_REGAGPIO0 {
	uint32_t reg;
	struct {
	unsigned int AGPIO_0:1;
	unsigned int AGPIO_1:1;
	unsigned int AGPIO_2:1;
	unsigned int AGPIO_3:1;
	unsigned int reserved0:28;
	} bit;
};

// 0xF0 Version Code Register
#define TOP_VERSION_REG_OFS                  0xF0
union TOP_VERSION_REG {
	uint32_t reg;
	struct {
	unsigned int reserved0:8;
	unsigned int ROM_CODE_VERSION:4;         // ROM Code Version
	unsigned int ECO_VERSION:4;              // ECO Version
	unsigned int CHIP_ID:16;                 // Chip ID
	} bit;
};

// 0xFC Debug Port Register
#define TOP_REG_DEBUG_OFS                    0xFC
union TOP_REG_DEBUG {
	uint32_t reg;
	struct {
	unsigned int DEBUG_SEL:8;                // Debug Port Selection
	unsigned int DEBUG_OUTSEL:2;             // Debug Port output select
	unsigned int reserved1:22;
	} bit;
};

#endif
