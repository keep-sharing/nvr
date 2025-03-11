/* SPDX-License-Identifier: GPL-2.0 */
/*
 * This header provides constants specific to Novatek NA51103 pinctrl bindings.
 */

#ifndef __DT_BINDINGS_PINCTRL_NA51103_PINCTRL_H
#define __DT_BINDINGS_PINCTRL_NA51103_PINCTRL_H

/*
	Follow the naming rule for pinctrl tool parsing:
	1. The name of enum variable should be "PIN_xxx_CFG" such as PIN_UART_CFG
	2. The name of enum should be prefixed with "PIN_xxx_CFG" such as PIN_UART_CFG_CH0_1ST_PINMUX
	3. The value of enum should be less than 0x80000000 such as PIN_UART_CFG_CH0_1ST_PINMUX  0x01
*/
#define PINMUX_DEBUGPORT_CKG    0x00     ///< CKGen
                           
#define PINMUX_DEBUGPORT_GPIO   0x01     ///< GPIO
#define PINMUX_DEBUGPORT_CPU    0x02     ///< CPU
#define PINMUX_DEBUGPORT_WDT    0x03     ///< WDT
#define PINMUX_DEBUGPORT_TIMER  0x04     ///< TIMER
#define PINMUX_DEBUGPORT_PWM    0x05     ///< PWM
#define PINMUX_DEBUGPORT_REMOTE 0x06     ///< REMOTE
#define PINMUX_DEBUGPORT_TRNG   0x07     ///< TRNG
#define PINMUX_DEBUGPORT_TSEN   0x08     ///< TSEN
#define PINMUX_DEBUGPORT_SMC    0x09     ///< SMCNAND
#define PINMUX_DEBUGPORT_SDIO   0x0A     ///< SDIO
#define PINMUX_DEBUGPORT_DAI    0x0B     ///< DAI
#define PINMUX_DEBUGPORT_DAI2   0x0C     ///< DAI2
#define PINMUX_DEBUGPORT_DAI3   0x0D     ///< DAI3
#define PINMUX_DEBUGPORT_DAI4   0x0E     ///< DAI4
#define PINMUX_DEBUGPORT_DAI5   0x0F     ///< DAI5
#define PINMUX_DEBUGPORT_UART2  0x10     ///< UART2
#define PINMUX_DEBUGPORT_UART3  0x11     ///< UART3
#define PINMUX_DEBUGPORT_UART4  0x12     ///< UART4
#define PINMUX_DEBUGPORT_I2C    0x13     ///< I2C
#define PINMUX_DEBUGPORT_I2C2   0x14     ///< I2C2
#define PINMUX_DEBUGPORT_I2C3   0x15     ///< I2C3
#define PINMUX_DEBUGPORT_I2C4   0x16     ///< I2C4
#define PINMUX_DEBUGPORT_JPEG   0x17     ///< JPEG
#define PINMUX_DEBUGPORT_OSG    0x18     ///< OSG
#define PINMUX_DEBUGPORT_SSCA   0x19     ///< SSCA
#define PINMUX_DEBUGPORT_EFUSE  0x1A     ///< EFUSE
#define PINMUX_DEBUGPORT_SCE    0x1B     ///< SCE
#define PINMUX_DEBUGPORT_HASH   0x1C     ///< HASH
#define PINMUX_DEBUGPORT_RSA    0x1D     ///< RSA
#define PINMUX_DEBUGPORT_USB    0x1E     ///< USB
#define PINMUX_DEBUGPORT_USB2   0x1F     ///< USB2
#define PINMUX_DEBUGPORT_ETH    0x20     ///< ETH
#define PINMUX_DEBUGPORT_ETH2   0x21     ///< ETH2
#define PINMUX_DEBUGPORT_SATA   0x22     ///< SATA
#define PINMUX_DEBUGPORT_SATA2  0x23     ///< SATA2
#define PINMUX_DEBUGPORT_HWCPY  0x24     ///< HYCPY
#define PINMUX_DEBUGPORT_CNN    0x25     ///< CNN
#define PINMUX_DEBUGPORT_NUE    0x26     ///< NUE
#define PINMUX_DEBUGPORT_NUE2   0x27     ///< NUE2
#define PINMUX_DEBUGPORT_VPE    0x28     ///< VPE
#define PINMUX_DEBUGPORT_VENC   0x29     ///< VENC
#define PINMUX_DEBUGPORT_VDEC   0x2A     ///< VDEC
#define PINMUX_DEBUGPORT_DEI    0x2B     ///< DEI
#define PINMUX_DEBUGPORT_VCAP   0x2C     ///< VCAP
#define PINMUX_DEBUGPORT_LCD310 0x2D     ///< LCD310
#define PINMUX_DEBUGPORT_LCD210 0x2E     ///< LCD210
#define PINMUX_DEBUGPORT_HDMI   0x2F     ///< HDMI
#define PINMUX_DEBUGPORT_TV     0x30     ///< TV
#define PINMUX_DEBUGPORT_HVLD   0x31     ///< HEAVYLOAD
#define PINMUX_DEBUGPORT_HVLD2  0x32     ///< HEAVYLOAD2
#define PINMUX_DEBUGPORT_HDLD3  0x33     ///< HEAVYLOAD3
#define PINMUX_DEBUGPORT_DDRH_A 0x34     ///< DDR A
#define PINMUX_DEBUGPORT_DDRH_B 0x35     ///< DDR B
#define PINMUX_DEBUGPORT_HRTMR  0x36     ///< HR TIMER
#define PINMUX_DEBUGPORT_HRTMR2 0x37     ///< HR TIMER2
#define PINMUX_DEBUGPORT_SPI    0x38     ///< SPI
	    
#define PINMUX_DEBUGPORT_GROUP_NONE    0x0000     ///< No debug port is output
#define PINMUX_DEBUGPORT_GROUP1        0x0100     ///< Output debug port to S_GPIO[17..0] P_GPIO[23]
#define PINMUX_DEBUGPORT_GROUP2        0x0200     ///< Output debug port to D_GPIO[9..0] P_GPIO[18..16] P_GPIO[7..4] P_GPIO[21..20]

/**
    Function group

    @note For pinmux_init()
*/
#define PIN_FUNC_UART      0      ///< UART. Configuration refers to PIN_UART_CFG.
#define PIN_FUNC_I2C       1      ///< I2C. Configuration refers to PIN_I2C_CFG.
#define PIN_FUNC_SDIO      2      ///< SDIO. Configuration refers to PIN_SDIO_CFG.
#define PIN_FUNC_SPI       3      ///< SPI. Configuration refers to PIN_SPI_CFG.
#define PIN_FUNC_EXTCLK    4      ///< EXTCLK. Configuration refers to PIN_EXTCLK_CFG
#define PIN_FUNC_SSP       5      ///< AUDIO/SSP. Configuration refers to PIN_SSP_CFG.
#define PIN_FUNC_LCD       6      ///< LCD interface. Configuration refers to PIN_LCD_CFG.
#define PIN_FUNC_REMOTE    7      ///< REMOTE. Configuration refers to PIN_REMOTE_CFG.
#define PIN_FUNC_VCAP      8      ///< VCAP. Configuration refers to PIN_VCAP_CFG
#define PIN_FUNC_ETH       9      ///< ETH. Configuration refers to PIN_ETH_CFG
#define PIN_FUNC_MISC      10     ///< MISC. Configuration refers to PIN_MISC_CFG
#define PIN_FUNC_PWM       11     ///< PWM. Configuration refers to PIN_PWM_CFG.
#define PIN_FUNC_VCAPINT   12     ///< VCAP INTERNAL MUX. Configuration refers to PIN_VCAPINT_CFG

/**
    PIN config for UART

*/
#define PIN_UART_CFG_NONE            0

#define PIN_UART_CFG_CH_1ST_PINMUX   0x01     ///< Enable UART.  (P_GPIO[1..0])
#define PIN_UART_CFG_CH_2ND_PINMUX   0x02     ///< Not support
#define PIN_UART_CFG_CH_3RD_PINMUX   0x04     ///< Not support
#define PIN_UART_CFG_CH_CTSRTS       0x08     ///< Enable UART_CTS/RTS (P_GPIO[3..2])									
#define PIN_UART_CFG_CH2_1ST_PINMUX  0x10     ///< Enable UART2. (P_GPIO[5..4])
#define PIN_UART_CFG_CH2_2ND_PINMUX  0x20     ///< Enable UART2_2. (P_GPIO[15..14])
#define PIN_UART_CFG_CH2_CTSRTS      0x40     ///< Enable UART2_CTS/RTS. (E_GPIO[1][6])
#define PIN_UART_CFG_CH2_3RD_PINMUX  0x80     ///< Enable UART2_3 (D_GPIO[1..0])									
#define PIN_UART_CFG_CH3_1ST_PINMUX  0x100    ///< Enable UART3. (P_GPIO[7..6])
#define PIN_UART_CFG_CH3_RTS         0x200    ///< Enalbe UART3_RTS (D_GPIO[5])									
#define PIN_UART_CFG_CH4_1ST_PINMUX  0x1000   ///< Enable UART4. (J_GPIO[4..3])
#define PIN_UART_CFG_CH4_2ND_PINMUX  0x2000   ///< Enable UART4_2. (P_GPIO[9..8])
#define PIN_UART_CFG_CH4_3RD_PINMUX  0x4000   ///< Enable UART4_3. (P_GPIO[17..16])
#define PIN_UART_CFG_CH4_4TH_PINMUX  0x8000   ///< Enable UART4_4. (P_GPIO[21..20])									
#define PIN_UART_CFG_CH5_1ST_PINMUX  0x10000  ///< Not Support
#define PIN_UART_CFG_CH5_2ND_PINMUX  0x20000  ///< Not Support
#define PIN_UART_CFG_CH4_5TH_PINMUX  0x100000 ///< Enable UART4_5. (D_GPIO[3..2])
#define PIN_UART_CFG_CH4_6TH_PINMUX  0x200000 ///< Enable UART4_6. (D_GPIO[8..7])

#define PIN_UART_CFG_CH0_1ST_PINMUX PIN_UART_CFG_CH_1ST_PINMUX
#define PIN_UART_CFG_CH0_CTSRTS PIN_UART_CFG_CH_CTSRTS

/**
    PIN config for I2C

*/
#define PIN_I2C_CFG_NONE             0
#define PIN_I2C_CFG_CH_1ST_PINMUX	 0x01 	///< Enable I2C. (P_GPIO[9..8])
#define PIN_I2C_CFG_CH_2ND_PINMUX	 0x02 	///< Enable I2C_2. (D_GPIO[1..0])									
#define PIN_I2C_CFG_CH2_1ST_PINMUX	 0x10 	///< Enable I2C2. (P_GPIO[15..14])
#define PIN_I2C_CFG_CH2_2ND_PINMUX	 0x20 	///< Enable I2C2_2. (D_GPIO[3..2])
#define PIN_I2C_CFG_CH2_3RD_PINMUX	 0x40 	///< Enable I2C2_3. (D_GPIO[6..5])
#define PIN_I2C_CFG_CH2_4TH_PINMUX	 0x80 	///< Enable I2C2_4. (J_GPIO[1..0])
#define PIN_I2C_CFG_CH2_5TH_PINMUX	 0x100 	///< Enable I2C2_5. (P_GPIO[5..4])
#define PIN_I2C_CFG_CH3_1ST_PINMUX	 0x1000 	///< Enable I2C3. (P_GPIO[21..20])
#define PIN_I2C_CFG_CH3_2ND_PINMUX	 0x2000 	///< Enable I2C3_2. (P_GPIO[17..16])
#define PIN_I2C_CFG_CH3_3RD_PINMUX   0x4000   ///< Not Support
#define PIN_I2C_CFG_CH3_4TH_PINMUX   0x8000   ///< Not Support	
#define PIN_I2C_CFG_HDMI_1ST_PINMUX	 0x10000	///< Enable HDMI_I2C. (P_GPIO[30..29])

#define PIN_I2C_CFG_CH0_1ST_PINMUX PIN_I2C_CFG_CH_1ST_PINMUX
#define PIN_I2C_CFG_CH1_1ST_PINMUX PIN_I2C_CFG_CH2_1ST_PINMUX
/**
    PIN config for SDIO

    @note For pinmux_init() with PIN_FUNC_SDIO.
*/
#define PIN_SDIO_CFG_NONE         0
#define PIN_SDIO_CFG_1ST_PINMUX   0x01	 ///< SDIO (P_GPIO[15..14] P_GPIO[19..17] P_GPIO[23])
#define PIN_SDIO_CFG_2ND_PINMUX   0x02	 ///< SDIO_2 (D_GPIO[9..6]) P_GPIO[21..22]
#define PIN_SDIO2_CFG_1ST_PINMUX  0x10 ///< Not Support
#define PIN_SDIO2_CFG_BUS_WIDTH   0x20 ///< Not Support
#define PIN_SDIO2_CFG_DS          0x40 ///< Not Support

/**
    PIN config for SPI

*/
#define PIN_SPI_CFG_NONE            0
#define PIN_SPI_CFG_CH_1ST_PINMUX   0x01   ///< Enable SPI. (C_GPIO[3..0])
#define PIN_SPI_CFG_CH_BUS_WIDTH    0x10   ///< SPI bus width  4 (C_GPIO[5..4])
#define PIN_SPI_CFG_CH_CS1          0x20   ///< Enable SPI CS1. (C_GPIO[6])
#define PIN_SPI_CFG_CH2_1ST_PINMUX  0x100  ///< Enable SPI2. (J_GPIO[1..0] P_GPIO[2])
#define PIN_SPI_CFG_CH2_2ND_PINMUX  0x200  ///< Enalbe SPI2_2. (P_GPIO[21..20] D_GPIO[6])
#define PIN_SPI_CFG_CH2_BUS_WIDTH   0x1000 ///< SPI2 bus with  2 (P_GPIO[3] or D_GPIO[7])


#define PIN_EXTCLK_CFG_NONE            0
#define PIN_EXTCLK_CFG_CH_1ST_PINMUX   0x01   ///< Enable EXT_CLK. (P_GPIO[22])
#define PIN_EXTCLK_CFG_CH2_1ST_PINMUX  0x02   ///< Enable EXT2_CLK. (P_GPIO[23])
#define PIN_EXTCLK_CFG_CH3_1ST_PINMUX  0x04   ///< Enable EXT3_CLK.(EXT_2_CLK) (S_GPIO[9])
#define PIN_EXTCLK_CFG_CH4_1ST_PINMUX  0x08   ///< Enable EXT4_CLK.(EXT2_2_CLK) (S_GPIO[18])

#define PIN_EXTCLK_CFG_CH0_1ST_PINMUX PIN_EXTCLK_CFG_CH_1ST_PINMUX
#define PIN_EXTCLK_CFG_CH1_1ST_PINMUX PIN_EXTCLK_CFG_CH2_1ST_PINMUX

#define PIN_SSP_CFG_NONE                  0
#define PIN_SSP_CFG_CH_1ST_PINMUX		 0x01   	///< Enable I2S. (P_GPIO[11..10])
#define PIN_SSP_CFG_CH_1ST_MCLK          0x02     ///< Enable I2S_MCLK. (P_GPIO[22])
#define PIN_SSP_CFG_CH_1ST_TX            0x04     ///< Enable I2S_TX. (P_GPIO[13])
#define PIN_SSP_CFG_CH_1ST_RX            0x08     ///< Enable I2S_RX. (P_GPIO[12])
#define PIN_SSP_CFG_CH2_1ST_PINMUX 		 0x10  	///< Enable I2S2. (P_GPIO[17..16])
#define PIN_SSP_CFG_CH2_1ST_MCLK         0x20     ///< Enable I2S2_MCLK (P_GPIO[23])
#define PIN_SSP_CFG_CH2_1ST_TX           0x40     ///< Enable I2S2_TX (P_GPIO[19])
#define PIN_SSP_CFG_CH2_1ST_RX           0x80     ///< Enable I2S2_RX (P_GPIO[18])
#define PIN_SSP_CFG_CH3_1ST_PINMUX		 0x100 	///< Enable I2S3. (D_GPIO[7..6])
#define PIN_SSP_CFG_CH3_2ND_PINMUX		 0x200 	///< Enable I2S3_2. (P_GPIO[15..14])
#define PIN_SSP_CFG_CH3_3RD_PINMUX		 0x400 	///< Enable I2S3_3. (P_GPIO[5] P_GPIO[14])
#define PIN_SSP_CFG_CH3_1ST_MCLK		 0x1000 	///< Enable I2S3_MCLK. (D_GPIO[5])
#define PIN_SSP_CFG_CH3_2ND_MCLK		 0x2000 	///< Not Support
#define PIN_SSP_CFG_CH3_3RD_MCLK		 0x4000 	///< Enable I2S3_3_MCLK. (P_GPIO[4])
#define PIN_SSP_CFG_CH3_1ST_TX   		 0x10000 	///< Enable I2S3_TX. (D_GPIO[9])
#define PIN_SSP_CFG_CH3_2ND_TX	    	 0x20000 	///< Enable I2S3_2_TX. (P_GPIO[19])
#define PIN_SSP_CFG_CH3_3RD_TX		     0x40000 	///< Not Support
#define PIN_SSP_CFG_CH3_1ST_RX   		 0x100000 ///< Enable I2S3_RX. (D_GPIO[8])
#define PIN_SSP_CFG_CH3_2ND_RX	    	 0x200000	///< Not Support
#define PIN_SSP_CFG_CH3_3RD_RX		     0x400000	///< Enable I2S3_3_RX. (P_GPIO[15])
#define PIN_SSP_CFG_CH4_1ST_PINMUX 		 0x1000000///< Enable I2S4. (J_GPIO[2..1])
#define PIN_SSP_CFG_CH4_1ST_MCLK         0x2000000///< Enable I2S4_MCLK (J_GPIO[0])
#define PIN_SSP_CFG_CH4_1ST_TX           0x4000000///< Enable I2S4_TX (J_GPIO[4])
#define PIN_SSP_CFG_CH4_1ST_RX           0x8000000///< Enable I2S4_RX (J_GPIO[3])
#define PIN_SSP_CFG_CH4_2ND_PINMUX 		 0x10000000///< Enable I2S4_2. (P_GPIO[9..8])
#define PIN_SSP_CFG_CH4_2ND_TX           0x20000000///< Enable I2S4_2_TX (P_GPIO[13])

#define PIN_LCD_CFG_NONE
#define PIN_LCD_CFG_LCD310_RGB888_1ST_PINMUX  0x01   ///< Enable LCD310 RGB888. (S_GPIO[17..0] P_GPIO[15..14] P_GPIO[12..10] P_GPIO[18..16])
#define PIN_LCD_CFG_LCD310_BT1120_1ST_PINMUX  0x02   ///< Enable LCD310 BT1120. (S_GPIO[8..0] S_GPIO[17..10])
#define PIN_LCD_CFG_LCD310_BT1120_2ND_PINMUX  0x04   ///< Enable LCD310_2 BT1120. (E_GPIO[15..0] D_GPIO[5])
#define PIN_LCD_CFG_LCD210_BT1120_1ST_PINMUX  0x10   ///< Enable LCD210 BT1120. (S_GPIO[8..0] S_GPIO[17..10])
#define PIN_LCD_CFG_LCD210_BT1120_2ND_PINMUX  0x20   ///< Enable LCD210_2 BT1120. (E_GPIO[15..0] D_GPIO[5])
#define PIN_LCD_CFG_LCD310L_RGB888_1ST_PINMUX 0x100  ///< Not Support
#define PIN_LCD_CFG_LCD310L_BT1120_1ST_PINMUX 0x200  ///< Not Support
#define PIN_LCD_CFG_LCD310_DE_PINMUX          0x1000 ///< Enable LCD310_DE. (P_GPIO[19])

#define PIN_REMOTE_CFG_NONE          0
#define PIN_REMOTE_CFG_1ST_PINMUX   0x001   ///< Enable Remote/IrDA. (J_GPIO[1])
#define PIN_REMOTE_CFG_2ND_PINMUX   0x002   ///< Enable Remote/IrDA. (J_GPIO[2])
#define PIN_REMOTE_CFG_3RD_PINMUX   0x004   ///< Enable Remote/IrDA. (J_GPIO[3])
#define PIN_REMOTE_CFG_4TH_PINMUX   0x008   ///< Enable Remote/IrDA. (J_GPIO[4])
#define PIN_REMOTE_CFG_5TH_PINMUX   0x010   ///< Enable Remote/IrDA. (P_GPIO[4])
#define PIN_REMOTE_CFG_6TH_PINMUX   0x020   ///< Enable Remote/IrDA. (P_GPIO[5])
#define PIN_REMOTE_CFG_7TH_PINMUX   0x040   ///< Enable Remote/IrDA. (D_GPIO[0])
#define PIN_REMOTE_CFG_8TH_PINMUX   0x080   ///< Enable Remote/IrDA. (D_GPIO[1])
#define PIN_REMOTE_CFG_9TH_PINMUX   0x100   ///< Enable Remote/IrDA. (D_GPIO[2])
#define PIN_REMOTE_CFG_10TH_PINMUX  0x200   ///< Enable Remote/IrDA. (D_GPIO[3])
#define PIN_REMOTE_CFG_11TH_PINMUX  0x400   ///< Enable Remote/IrDA. (D_GPIO[4])
#define PIN_REMOTE_CFG_12TH_PINMUX  0x800   ///< Not Support
#define PIN_REMOTE_CFG_13TH_PINMUX  0x1000  ///< Not Support

#define PIN_VCAP_CFG_NONE                  0
#define PIN_VCAP_CFG_CAP0_1ST_PINMUX      0x1         ///< Enable CAP. (S_GPIO[8..1])
#define PIN_VCAP_CFG_CAP1_1ST_PINMUX      0x2         ///< Enable CAP1. (S_GPIO[17..10])
#define PIN_VCAP_CFG_CAP2_1ST_PINMUX      0x4         ///< Enable CAP2. (S_GPIO[26..19])
#define PIN_VCAP_CFG_CAP3_1ST_PINMUX      0x8         ///< Enable CAP3. (S_GPIO[35..28])
#define PIN_VCAP_CFG_CAP4_1ST_PINMUX      0x10        ///< Not Support
#define PIN_VCAP_CFG_CAP5_1ST_PINMUX      0x20        ///< Not Support
#define PIN_VCAP_CFG_CAP6_1ST_PINMUX      0x40        ///< Not Support
#define PIN_VCAP_CFG_CAP7_1ST_PINMUX      0x80        ///< Not Suppor
#define PIN_VCAP_CFG_CAP0_CLK_1ST_PINMUX  0x100       ///< Enable CAP_CLK. (S_GPIO[0])
#define PIN_VCAP_CFG_CAP0_CLK_2ND_PINMUX  0x200       ///< Enable CAP_2_CLK. (P_GPIO[23])
#define PIN_VCAP_CFG_CAP1_CLK_1ST_PINMUX  0x400       ///< Enable CAP1_CLK. (S_GPIO[9])
#define PIN_VCAP_CFG_CAP1_CLK_2ND_PINMUX  0x800       ///< Enable CAP1_2_CLK. (S_GPIO[0])
#define PIN_VCAP_CFG_CAP2_CLK_1ST_PINMUX  0x1000      ///< Enable CAP2_CLK. (S_GPIO[18])
#define PIN_VCAP_CFG_CAP2_CLK_2ND_PINMUX  0x2000      ///< Enable CAP2_2_CLK. (S_GPIO[9])
#define PIN_VCAP_CFG_CAP3_CLK_1ST_PINMUX  0x4000      ///< Enable CAP3_CLK. (S_GPIO[27])
#define PIN_VCAP_CFG_CAP3_CLK_2ND_PINMUX  0x8000      ///< Enable CAP3_2_CLK. (S_GPIO[18])
#define PIN_VCAP_CFG_CAP4_CLK_1ST_PINMUX  0x10000     ///< Not Support
#define PIN_VCAP_CFG_CAP4_CLK_2ND_PINMUX  0x20000     ///< Not Support
#define PIN_VCAP_CFG_CAP5_CLK_1ST_PINMUX  0x40000     ///< Not Support
#define PIN_VCAP_CFG_CAP5_CLK_2ND_PINMUX  0x80000     ///< Not Support
#define PIN_VCAP_CFG_CAP6_CLK_1ST_PINMUX  0x100000    ///< Not Support
#define PIN_VCAP_CFG_CAP6_CLK_2ND_PINMUX  0x200000    ///< Not Support
#define PIN_VCAP_CFG_CAP7_CLK_1ST_PINMUX  0x400000    ///< Not Support
#define PIN_VCAP_CFG_CAP7_CLK_2ND_PINMUX  0x800000    ///< Not Support
#define PIN_VCAP_CFG_CAP3_CLK_3RD_PINMUX  0x1000000   ///< Enable CAP3_3_CLK. (P_GPIO[14])


#define PIN_ETH_CFG_NONE                 0
#define PIN_ETH_CFG_RGMII_1ST_PINMUX     0x001    ///< Enable RGMII. (E_GPIO[13..2])
#define PIN_ETH_CFG_RGMII_2ND_PINMUX     0x002    ///< Not Support
#define PIN_ETH_CFG_RMII_1ST_PINMUX      0x010    ///< Enabl RMII. (E_GPIO[5..2] E_GPIO[11..9])
#define PIN_ETH_CFG_RMII_2ND_PINMUX      0x020    ///< Not Support
#define PIN_ETH_CFG_REFCLK_PINMUX        0x100    ///< Enable REFCLK. (E_GPIO[0])
#define PIN_ETH_CFG_RST_PINMUX           0x200    ///< Not Support
#define PIN_ETH_CFG_MDC_MDIO_PINMUX      0x400    ///< Enable MDC/MDIO. (E_GPIO[15..14])
#define PIN_ETH2_CFG_RGMII_1ST_PINMUX    0x1000  ///< Enable ETH2 RGMII.(S_GPIO[13..2])
#define PIN_ETH2_CFG_RGMII_2ND_PINMUX    0x2000  ///< Not Support
#define PIN_ETH2_CFG_RMII_1ST_PINMUX     0x10000 ///< Enable RMII.(S_GPIO[5..2] S_GPIO[11..9])
#define PIN_ETH2_CFG_RMII_2ND_PINMUX     0x20000 ///< Not Support
#define PIN_ETH2_CFG_REFCLK_PINMUX       0x100000///< Enable REFCLK. (S_GPIO[0])
#define PIN_ETH2_CFG_RST_PINMUX          0x200000///< Not Support
#define PIN_ETH2_CFG_MDC_MDIO_PINMUX     0x400000///< Enable MDC/MDIO. (S_GPIO[15..14])
#define PIN_ETH_CFG_ACT_LED_1ST_PINMUX   0x1000000 ///< ETH_ACT_LED (J_GPIO[1])
#define PIN_ETH_CFG_ACT_LED_2ND_PINMUX   0x2000000 ///< ETH_2_ACT_LED (D_GPIO[8])
#define PIN_ETH_CFG_LINK_LED_1ST_PINMUX  0x1000000 ///< ETH_LINK_LED (J_GPIO[2])
#define PIN_ETH_CFG_LINK_LED_2ND_PINMUX  0x2000000 ///< ETH_2_LINK_LED (D_GPIO[9])
#define PIN_ETH_CFG_PHY_SEL              0x10000000 ///< ETH PHY Sel from GMAC0 or GMAC1


#define PIN_MISC_CFG_NONE
#define PIN_MISC_CFG_CPU_ICE 			   0x00000001///< Enable CPU ICE @ CPU ICE interface
#define PIN_MISC_CFG_VGA_HS  			   0x00000002///< Enable VGA_HS (P_GPIO[27])
#define PIN_MISC_CFG_VGA_VS  			   0x00000004///< Enable VGA_VS (P_GPIO[28])
#define PIN_MISC_CFG_BMC                   0x00000008///< Enable BMC (D_GPIO[8..5])
#define PIN_MISC_CFG_RTC_CAL_OUT		   0x00000010///< Enable RTC_CAL_OUT (P_GPIO[2])
#define PIN_MISC_CFG_DAC_RAMP_TP		   0x00000020///< Enable DAC_RAMP_TP (P_GPIO[5])
#define PIN_MISC_CFG_HDMI_HOTPLUG		   0x00000040///< Enable HDMI_HPD (P_GPIO[31])
#define PIN_MISC_CFG_SATA_LED_1ST_PINMUX   0x00100000///< Enable SATA activity LED. (J_GPIO[3])
#define PIN_MISC_CFG_SATA_LED_2ND_PINMUX   0x00200000///< Enable SATA activity LED. (D_GPIO[3])
#define PIN_MISC_CFG_SATA_LED_3RD_PINMUX   0x00400000///< Not Support
#define PIN_MISC_CFG_SATA2_LED_1ST_PINMUX  0x01000000///< Enable SATA2 activity LED. (J_GPIO[4])
#define PIN_MISC_CFG_SATA2_LED_2ND_PINMUX  0x02000000///< Enable SATA2 activity LED. (D_GPIO[4])
#define PIN_MISC_CFG_SATA2_LED_3RD_PINMUX  0x04000000///< Not Support
#define PIN_MISC_CFG_SATA3_LED_1ST_PINMUX  0x10000000///< Not Support
#define PIN_MISC_CFG_SATA3_LED_2ND_PINMUX  0x20000000///< Not Support
#define PIN_MISC_CFG_SATA3_LED_3RD_PINMUX  0x40000000///< Not Support

#define PIN_PWM_CFG_NONE            0
#define PIN_PWM_CFG_CH_1ST_PINMUX   0x01  ///< Enable PWM. (P_GPIO[24])
#define PIN_PWM_CFG_CH2_1ST_PINMUX  0x10 ///< Enable PWM2. (P_GPIO[25])
#define PIN_PWM_CFG_CH3_1ST_PINMUX  0x100///< Enable PWM3. (P_GPIO[19])
#define PIN_PWM_CFG_CH4_1ST_PINMUX  0x1000///< Not Support

#define PIN_VCAPINT_CFG_NONE 0
#endif
