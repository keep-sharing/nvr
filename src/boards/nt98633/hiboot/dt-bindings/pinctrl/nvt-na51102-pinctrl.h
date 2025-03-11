/* SPDX-License-Identifier: GPL-2.0 */
/*
 * This header provides constants specific to Novatek NA51102 pinctrl bindings.
 */

#ifndef __DT_BINDINGS_PINCTRL_NA51102_PINCTRL_H
#define __DT_BINDINGS_PINCTRL_NA51102_PINCTRL_H

/**
    Function group

    @note For pinmux_init()
*/
#define PIN_FUNC_SDIO                   0
#define PIN_FUNC_NAND                   1
#define PIN_FUNC_ETH                    2
#define PIN_FUNC_I2C                    3
#define PIN_FUNC_I2CII                  4
#define PIN_FUNC_PWM                    5
#define PIN_FUNC_PWMII                  6
#define PIN_FUNC_CCNT                   7
#define PIN_FUNC_SENSOR                 8
#define PIN_FUNC_SENSOR2                9
#define PIN_FUNC_SENSOR3                10
#define PIN_FUNC_SENSORMISC             11
#define PIN_FUNC_SENSORSYNC             12
#define PIN_FUNC_MIPI_LVDS              13
#define PIN_FUNC_AUDIO                  14
#define PIN_FUNC_UART                   15
#define PIN_FUNC_UARTII                 16
#define PIN_FUNC_REMOTE                 17
#define PIN_FUNC_SDP                    18
#define PIN_FUNC_SPI                    19
#define PIN_FUNC_SIF                    20
#define PIN_FUNC_MISC                   21
#define PIN_FUNC_LCD                    22
#define PIN_FUNC_LCD2                   23
#define PIN_FUNC_TV                     24
#define PIN_FUNC_SEL_LCD                25
#define PIN_FUNC_SEL_LCD2               26
#define PIN_FUNC_MAX                    27

/**
    PIN config for SDIO

    @note For pinmux_init() with PIN_FUNC_SDIO.
*/
#define PIN_SDIO_CFG_NONE               0x0

#define PIN_SDIO_CFG_SDIO_1             0x1         ///< SDIO  (C_GPIO[11..16])

#define PIN_SDIO_CFG_SDIO2_1            0x10        ///< SDIO2 (C_GPIO[17..22])

#define PIN_SDIO_CFG_SDIO3_1            0x100       ///< SDIO3 (C_GPIO[0..3] C_GPIO[8..9])
#define PIN_SDIO_CFG_SDIO3_BUS_WIDTH    0x200       ///< SDIO3 bus width 8 bits (C_GPIO[4..7])
#define PIN_SDIO_CFG_SDIO3_DS           0x400       ///< SDIO3 data strobe      (C_GPIO[10])

// compatible with na51090, na51103
#define PIN_SDIO_CFG_1ST_PINMUX         PIN_SDIO_CFG_SDIO_1
#define PIN_SDIO2_CFG_1ST_PINMUX        PIN_SDIO_CFG_SDIO2_1

// host_id refer to SDIO_HOST_ID
#define PIN_SDIO_CFG_MASK(host_id)   (0xF << host_id)

/**
    PIN config for NAND

    @note For pinmux_init() with PIN_FUNC_NAND.
*/
#define PIN_NAND_CFG_NONE               0x0

#define PIN_NAND_CFG_NAND_1             0x1          ///< FSPI     (C_GPIO[0..3]  C_GPIO[8..9])
#define PIN_NAND_CFG_NAND_CS1           0x2          ///< FSPI CS1 (C_GPIO[10])

/**
    PIN config for ETH

    @note For pinmux_init() for PIN_FUNC_ETH
*/
#define PIN_ETH_CFG_NONE                0x0

#define PIN_ETH_CFG_ETH_RGMII_1         0x1
#define PIN_ETH_CFG_ETH_RMII_1          0x2
#define PIN_ETH_CFG_ETH_EXTPHYCLK       0x4
#define PIN_ETH_CFG_ETH_PTP             0x8

#define PIN_ETH_CFG_ETH2_RGMII_1        0x10
#define PIN_ETH_CFG_ETH2_RMII_1         0x20
#define PIN_ETH_CFG_ETH2_EXTPHYCLK      0x40
#define PIN_ETH_CFG_ETH2_PTP            0x80

/**
    PIN config for I2C

    @note For pinmux_init() with PIN_FUNC_I2C.
*/
#define PIN_I2C_CFG_NONE                0x0

#define PIN_I2C_CFG_I2C_1               0x1          ///< I2C_1   (P_GPIO[28..29])

#define PIN_I2C_CFG_I2C2_1              0x10         ///< I2C2_1  (P_GPIO[30..31])

#define PIN_I2C_CFG_I2C3_1              0x100        ///< I2C3_1  (P_GPIO[12..13])
#define PIN_I2C_CFG_I2C3_2              0x200        ///< I2C3_2  (S_GPIO[11..12])
#define PIN_I2C_CFG_I2C3_3              0x400        ///< I2C3_3  (HSI_GPIO[22..23])

#define PIN_I2C_CFG_I2C4_1              0x1000       ///< I2C4_1  (P_GPIO[14..15])
#define PIN_I2C_CFG_I2C4_2              0x2000       ///< I2C4_2  (S_GPIO[13..14])
#define PIN_I2C_CFG_I2C4_3              0x4000       ///< I2C4_3  (DSI_GPIO[4..5])
#define PIN_I2C_CFG_I2C4_4              0x8000       ///< I2C4_4  (HSI_GPIO[20..21])

#define PIN_I2C_CFG_I2C5_1              0x10000      ///< I2C5_1  (P_GPIO[16..17])
#define PIN_I2C_CFG_I2C5_2              0x20000      ///< I2C5_2  (S_GPIO[15..16])
#define PIN_I2C_CFG_I2C5_3              0x40000      ///< I2C5_3  (C_GPIO[4..5])

/**
    PIN config for I2CII

    @note For pinmux_init() with PIN_FUNC_I2CII.
*/
#define PIN_I2CII_CFG_NONE              0x0

#define PIN_I2CII_CFG_I2C6_1            0x1          ///< I2C6_1  (P_GPIO[18..19])
#define PIN_I2CII_CFG_I2C6_2            0x2          ///< I2C6_2  (S_GPIO[23..24])
#define PIN_I2CII_CFG_I2C6_3            0x4          ///< I2C6_3  (C_GPIO[6..7])

#define PIN_I2CII_CFG_I2C7_1            0x10         ///< I2C7_1  (P_GPIO[20..21])
#define PIN_I2CII_CFG_I2C7_2            0x20         ///< I2C7_2  (S_GPIO[25..26])

#define PIN_I2CII_CFG_I2C8_1            0x100        ///< I2C8_1  (P_GPIO[22..23])
#define PIN_I2CII_CFG_I2C8_2            0x200        ///< I2C8_2  (S_GPIO[7..8])

#define PIN_I2CII_CFG_I2C9_1            0x1000       ///< I2C9_1  (P_GPIO[24..25])
#define PIN_I2CII_CFG_I2C9_2            0x2000       ///< I2C3_1  (L_GPIO[26..27])

#define PIN_I2CII_CFG_I2C10_1           0x10000      ///< I2C10_1 (P_GPIO[26..27])
#define PIN_I2CII_CFG_I2C10_2           0x20000      ///< I2C10_2 (D_GPIO[0..1])

#define PIN_I2CII_CFG_I2C11_1           0x100000     ///< I2C11_1 (P_GPIO[36..37])

/**
    PIN config for PWM

    @note For pinmux_init() with PIN_FUNC_PWM.
*/
#define PIN_PWM_CFG_NONE                0x0

#define PIN_PWM_CFG_PWM0_1              0x1          ///< PWM0_1  (P_GPIO[0])
#define PIN_PWM_CFG_PWM0_2              0x2          ///< PWM0_2  (DSI_GPIO[6])

#define PIN_PWM_CFG_PWM1_1              0x10         ///< PWM1_1  (P_GPIO[1])
#define PIN_PWM_CFG_PWM1_2              0x20         ///< PWM1_2  (DSI_GPIO[7])

#define PIN_PWM_CFG_PWM2_1              0x100        ///< PWM2_1  (P_GPIO[2])
#define PIN_PWM_CFG_PWM2_2              0x200        ///< PWM2_2  (DSI_GPIO[8])

#define PIN_PWM_CFG_PWM3_1              0x1000       ///< PWM3_1  (P_GPIO[3])
#define PIN_PWM_CFG_PWM3_2              0x2000       ///< PWM3_2  (DSI_GPIO[9])

#define PIN_PWM_CFG_PWM4_1              0x10000      ///< PWM4_1  (P_GPIO[4])
#define PIN_PWM_CFG_PWM4_2              0x20000      ///< PWM4_2  (D_GPIO[2])

#define PIN_PWM_CFG_PWM5_1              0x100000     ///< PWM5_1  (P_GPIO[5])
#define PIN_PWM_CFG_PWM5_2              0x200000     ///< PWM5_2  (D_GPIO[3])

/**
    PIN config for PWMII

    @note For pinmux_init() with PIN_FUNC_PWMII.
*/
#define PIN_PWMII_CFG_NONE              0x0

#define PIN_PWMII_CFG_PWM6_1            0x1          ///< PWM6_1  (P_GPIO[6])
#define PIN_PWMII_CFG_PWM6_2            0x2          ///< PWM6_2  (D_GPIO[4])

#define PIN_PWMII_CFG_PWM7_1            0x10         ///< PWM7_1  (P_GPIO[7])
#define PIN_PWMII_CFG_PWM7_2            0x20         ///< PWM7_2  (D_GPIO[5])

#define PIN_PWMII_CFG_PWM8_1            0x100        ///< PWM8_1  (P_GPIO[8])
#define PIN_PWMII_CFG_PWM8_2            0x200        ///< PWM8_2  (C_GPIO[4])

#define PIN_PWMII_CFG_PWM9_1            0x1000       ///< PWM9_1  (P_GPIO[9])
#define PIN_PWMII_CFG_PWM9_2            0x2000       ///< PWM9_2  (C_GPIO[5])

#define PIN_PWMII_CFG_PWM10_1           0x10000      ///< PWM10_1 (P_GPIO[10])
#define PIN_PWMII_CFG_PWM10_2           0x20000      ///< PWM10_2 (C_GPIO[6])

#define PIN_PWMII_CFG_PWM11_1           0x100000     ///< PWM11_1 (P_GPIO[11])
#define PIN_PWMII_CFG_PWM11_2           0x200000     ///< PWM11_2 (C_GPIO[7])

/**
    PIN config for CCNT

    @note For pinmux_init() with PIN_FUNC_CCNT.
*/
#define PIN_CCNT_CFG_NONE               0x0

#define PIN_CCNT_CFG_CCNT_1             0x1          ///< PICNT_1  (P_GPIO[21])

#define PIN_CCNT_CFG_CCNT2_1            0x10         ///< PICNT2_1 (P_GPIO[22])

#define PIN_CCNT_CFG_CCNT3_1            0x100        ///< PICNT3_1 (P_GPIO[23])

/**
    PIN config for Sensor

    @note For pinmux_init() with PIN_FUNC_SENSOR.
*/
#define PIN_SENSOR_CFG_NONE                     0x0

#define PIN_SENSOR_CFG_12BITS                   0x04         ///< SN_D[0..11]/SN_PXCLK/SN_VD/SN_HD (HSI_GPIO[0..11]/S_GPIO[3..5])

#define PIN_SENSOR_CFG_SN3_MCLK_2               0x200        ///< Enable SN3_MCLK_2 (HSI_GPIO[15]) for Sensor1
#define PIN_SENSOR_CFG_SN4_MCLK_2               0x400        ///< Enable SN4_MCLK_2 (HSI_GPIO[16]) for Sensor1

/**
    PIN config for Sensor2

    @note For pinmux_init() with PIN_FUNC_SENSOR2.
*/
#define PIN_SENSOR2_CFG_NONE                    0x0

#define PIN_SENSOR2_CFG_12BITS                  0x100         ///< SN_D[0..11]/SN_PXCLK/SN_VD/SN_HD (S_GPIO[9..20]/S_GPIO[6]/S_GPIO/[7]/S_GPIO[8])

#define PIN_SENSOR2_CFG_CCIR8BITS_A             0x10000       ///< CCIR601/656. CCIR_A_YC[0..7]/CCIR_A_CLK (S_GPIO[17..24]/S_GPIO[6])
#define PIN_SENSOR2_CFG_CCIR8BITS_B             0x20000       ///< CCIR656. CCIR_B_YC[0..7]/CCIR_B_CLK (S_GPIO[9..16]/S_GPIO[2])
#define PIN_SENSOR2_CFG_CCIR8BITS_AB            0x40000       ///< CCIR601/656 + CCIR656. CCIR_A_YC[0..7]/CCIR_A_CLK (S_GPIO[17..24]/S_GPIO[6]) + CCIR_B_YC[0..7]/CCIR_B_CLK (S_GPIO[9..16]/S_GPIO[2])

#define PIN_SENSOR2_CFG_CCIR16BITS              0x10          ///< CCIR601/656. CCIR_Y[0..7]/CCIR_C[0..7]/CCIR_CLK (S_GPIO[17..24]/S_GPIO[9..16]/S_GPIO[6])
#define PIN_SENSOR2_CFG_CCIR_VSHS               0x100000      ///< For CCIR601. CCIR_VD/CCIR_HD/CCIR_FIELD (S_GPIO[7]/S_GPIO[8]/S_GPIO[2])

#define PIN_SENSOR2_CFG_SN1_MCLK_1              0x200         ///< Enable SN1_MCLK_1 (S_GPIO[0]) for Sensor2
#define PIN_SENSOR2_CFG_SN2_MCLK_1              0x400         ///< Enable SN2_MCLK_1 (S_GPIO[1]) for Sensor2

/**
    PIN config for Sensor3

    @note For pinmux_init() with PIN_FUNC_SENSOR3.
*/
#define PIN_SENSOR3_CFG_NONE                    0x0

#define PIN_SENSOR3_CFG_12BITS                  0x100         ///< SN_D[0..11]/SN_PXCLK/SN_VD/SN_HD (P_GPIO[11..14][0..7]/P_GPIO[8]/P_GPIO[9]/P_GPIO[10])

#define PIN_SENSOR3_CFG_CCIR8BITS_A             0x10000       ///< CCIR601/656. CCIR_A_YC[0..7]/CCIR_A_CLK (P_GPIO[0..7]/P_GPIO[8])
#define PIN_SENSOR3_CFG_CCIR8BITS_B             0x20000       ///< CCIR656. CCIR_B_YC[0..7]/CCIR_B_CLK (P_GPIO[12..19]/P_GPIO[11])
#define PIN_SENSOR3_CFG_CCIR8BITS_AB            0x40000       ///< CCIR601/656 + CCIR656. CCIR_A_YC[0..7]/CCIR_A_CLK (P_GPIO[0..7]/P_GPIO[8]) + CCIR_B_YC[0..7]/CCIR_B_CLK (P_GPIO[12..19]/P_GPIO[11])

#define PIN_SENSOR3_CFG_CCIR16BITS              0x10          ///< CCIR601/656. CCIR_Y[0..7]/CCIR_C[0..7]/CCIR_CLK (P_GPIO[0..7]/P_GPIO[12..19]/P_GPIO[8])
#define PIN_SENSOR3_CFG_CCIR_VSHS               0x100000      ///< For CCIR601. CCIR_VD/CCIR_HD/CCIR_FIELD (P_GPIO[9]/P_GPIO[10]/P_GPIO[11])

#define PIN_SENSOR3_CFG_SN5_MCLK_2              0x200         ///< Enable SN5_MCLK_2 (P_GPIO[20]) for Sensor3

/**
    PIN config for Sensor Misc

    @note For pinmux_init() with PIN_FUNC_SENSORMISC.
*/
#define PIN_SENSORMISC_CFG_NONE                 0x0

#define PIN_SENSORMISC_CFG_SN_MCLK_1            0x1           ///< SN1_MCLK_1 (S_GPIO[0])

#define PIN_SENSORMISC_CFG_SN2_MCLK_1           0x2           ///< SN2_MCLK_1 (S_GPIO[1])

#define PIN_SENSORMISC_CFG_SN3_MCLK_1           0x4           ///< SN3_MCLK_1 (S_GPIO[2])
#define PIN_SENSORMISC_CFG_SN3_MCLK_2           0x8           ///< SN3_MCLK_2 (HSI_GPIO[15])

#define PIN_SENSORMISC_CFG_SN4_MCLK_1           0x10          ///< SN4_MCLK_1 (S_GPIO[21])
#define PIN_SENSORMISC_CFG_SN4_MCLK_2           0x20          ///< SN4_MCLK_2 (HSI_GPIO[16])

#define PIN_SENSORMISC_CFG_SN5_MCLK_1           0x40          ///< SN5_MCLK_1 (S_GPIO[22])
#define PIN_SENSORMISC_CFG_SN5_MCLK_2           0x80          ///< SN5_MCLK_2 (P_GPIO[20])

#define PIN_SENSORMISC_CFG_SN_XVSXHS_1          0x100         ///< SN1_XVSXHS_1 (S_GPIO[4..5])

#define PIN_SENSORMISC_CFG_SN2_XVSXHS_1         0x200         ///< SN2_XVSXHS_1 (S_GPIO[7..8])

#define PIN_SENSORMISC_CFG_SN3_XVSXHS_1         0x400         ///< SN3_XVSXHS_1 (S_GPIO[9..10])

#define PIN_SENSORMISC_CFG_SN4_XVSXHS_1         0x800         ///< SN4_XVSXHS_1 (S_GPIO[17..18])

#define PIN_SENSORMISC_CFG_SN5_XVSXHS_1         0x1000        ///< SN5_XVSXHS_1 (S_GPIO[19..20])

#define PIN_SENSORMISC_CFG_FLASH_TRIG_IN_1      0x10000       ///< FLASH_TRIG_IN_1 (S_GPIO[25])
#define PIN_SENSORMISC_CFG_FLASH_TRIG_IN_2      0x20000       ///< FLASH_TRIG_IN_2 (HSI_GPIO[17])

#define PIN_SENSORMISC_CFG_FLASH_TRIG_OUT_1     0x40000       ///< FLASH_TRIG_OUT_1 (S_GPIO[26])
#define PIN_SENSORMISC_CFG_FLASH_TRIG_OUT_2     0x80000       ///< FLASH_TRIG_OUT_2 (HSI_GPIO[18])

/**
    PIN config for Sensor Sync

    @note For pinmux_init() with PIN_FUNC_SENSORSYNC.
*/
#define PIN_SENSORSYNC_CFG_NONE                 0x0

#define PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SN       0x1

#define PIN_SENSORSYNC_CFG_SN3_MCLKSRC_SN       0x10
#define PIN_SENSORSYNC_CFG_SN3_MCLKSRC_SN2      0x20

#define PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN       0x100
#define PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN2      0x200
#define PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN3      0x400

#define PIN_SENSORSYNC_CFG_SN5_MCLKSRC_SN       0x1000
#define PIN_SENSORSYNC_CFG_SN5_MCLKSRC_SN2      0x2000
#define PIN_SENSORSYNC_CFG_SN5_MCLKSRC_SN3      0x4000
#define PIN_SENSORSYNC_CFG_SN5_MCLKSRC_SN4      0x8000

#define PIN_SENSORSYNC_CFG_SN2_XVSXHSSRC_SN     0x10000

#define PIN_SENSORSYNC_CFG_SN3_XVSXHSSRC_SN     0x100000
#define PIN_SENSORSYNC_CFG_SN3_XVSXHSSRC_SN2    0x200000

#define PIN_SENSORSYNC_CFG_SN4_XVSXHSSRC_SN     0x1000000
#define PIN_SENSORSYNC_CFG_SN4_XVSXHSSRC_SN2    0x2000000
#define PIN_SENSORSYNC_CFG_SN4_XVSXHSSRC_SN3    0x4000000

#define PIN_SENSORSYNC_CFG_SN5_XVSXHSSRC_SN     0x10000000
#define PIN_SENSORSYNC_CFG_SN5_XVSXHSSRC_SN2    0x20000000
#define PIN_SENSORSYNC_CFG_SN5_XVSXHSSRC_SN3    0x40000000
#define PIN_SENSORSYNC_CFG_SN5_XVSXHSSRC_SN4    0x80000000

/**
    PIN config for MIPI/LVDS

    @note For pinmux_init() with PIN_FUNC_MIPI_LVDS.
*/
#define PIN_MIPI_LVDS_CFG_NONE                  0x0

#define PIN_MIPI_LVDS_CFG_HSI2HSI3_TO_CSI       0x100000      ///< HSI2 and HSI3 are configured as CSIx/LVDS (HSI_GPIO[0..23])

/**
    PIN config for AUDIO

    @note For pinmux_init() with PIN_FUNC_AUDIO.
*/
#define PIN_AUDIO_CFG_NONE              0x0
#define PIN_AUDIO_CFG_I2S_1             0x1          ///< I2S_1       (PGPIO[24..27])
#define PIN_AUDIO_CFG_I2S_2             0x2          ///< I2S_2       (CGPIO[18..21])

#define PIN_AUDIO_CFG_I2S_MCLK_1        0x4          ///< I2S_MCLK_1  (PGPIO[28])
#define PIN_AUDIO_CFG_I2S_MCLK_2        0x8          ///< I2S_MCLK_2  (CGPIO[17])

#define PIN_AUDIO_CFG_I2S2_1            0x10         ///< I2S2_1      (LGPIO[20..23])

#define PIN_AUDIO_CFG_I2S2_MCLK_1       0x20         ///< I2S2_MCLK_1 (LGPIO[24])

#define PIN_AUDIO_CFG_DMIC_1            0x100        ///< DMIC_1 (PGPIO[26]) DATA0 (PGPIO[27]) DATA1 (PGPIO[25])
#define PIN_AUDIO_CFG_DMIC_DATA0        0x200
#define PIN_AUDIO_CFG_DMIC_DATA1        0x400

#define PIN_AUDIO_CFG_EXT_EAC_MCLK      0x1000       ///< EXT_EAC_MCLK (CGPIO[16])

/**
    PIN config for UART

    @note For pinmux_init() with PIN_FUNC_UART.
*/
#define PIN_UART_CFG_NONE               0x0

#define PIN_UART_CFG_UART_1             0x1          ///< UART_1  (P_GPIO[32..33])

#define PIN_UART_CFG_UART2_1            0x10         ///< UART2_1 (P_GPIO[30..31]) RTSCTS (P_GPIO[28..29])

#define PIN_UART_CFG_UART3_1            0x100        ///< UART3_1 (P_GPIO[24..25]) RTSCTS (P_GPIO[26..27])

#define PIN_UART_CFG_UART4_1            0x1000       ///< UART4_1 (P_GPIO[20..21]) RTSCTS (P_GPIO[22..23])
#define PIN_UART_CFG_UART4_2            0x2000       ///< UART4_2 (A_GPIO[0..1])   RTSCTS (A_GPIO[2..3])

#define PIN_UART_CFG_UART5_1            0x10000      ///< UART5_1 (P_GPIO[16..17]) RTSCTS (P_GPIO[18..19])
#define PIN_UART_CFG_UART5_2            0x20000      ///< UART5_2 (DSI_GPIO[0..1]) RTSCTS (DSI_GPIO[2..3])

#define PIN_UART_CFG_UART2_RTSCTS       0x100000
#define PIN_UART_CFG_UART2_DIROE        0x200000

#define PIN_UART_CFG_UART3_RTSCTS       0x400000
#define PIN_UART_CFG_UART3_DIROE        0x800000

#define PIN_UART_CFG_UART4_RTSCTS       0x1000000
#define PIN_UART_CFG_UART4_DIROE        0x2000000

#define PIN_UART_CFG_UART5_RTSCTS       0x4000000
#define PIN_UART_CFG_UART5_DIROE        0x8000000

/**
    PIN config for UARTII

    @note For pinmux_init() with PIN_FUNC_UARTII.
*/
#define PIN_UARTII_CFG_NONE             0x0

#define PIN_UARTII_CFG_UART6_1          0x1          ///< UART6_1 (P_GPIO[12..13]) RTSCTS (P_GPIO[14..15])
#define PIN_UARTII_CFG_UART6_2          0x2          ///< UART6_2 (DSI_GPIO[4..5]) RTSCTS (DSI_GPIO[6..7])

#define PIN_UARTII_CFG_UART7_1          0x10         ///< UART7_1 (P_GPIO[8..9])   RTSCTS (P_GPIO[10..11])
#define PIN_UARTII_CFG_UART7_2          0x20         ///< UART7_2 (C_GPIO[4..5])   RTSCTS (C_GPIO[6..7])

#define PIN_UARTII_CFG_UART8_1          0x100        ///< UART8_1 (P_GPIO[4..5])   RTSCTS (P_GPIO[6..7])
#define PIN_UARTII_CFG_UART8_2          0x200        ///< UART8_2 (C_GPIO[17..18]) RTSCTS (C_GPIO[19..20])

#define PIN_UARTII_CFG_UART9_1          0x1000       ///< UART9_1 (P_GPIO[0..1])   RTSCTS (P_GPIO[2..3])
#define PIN_UARTII_CFG_UART9_2          0x2000       ///< UART9_2 (D_GPIO[5..6])   RTSCTS (D_GPIO[7..8])

#define PIN_UARTII_CFG_UART6_RTSCTS     0x10000
#define PIN_UARTII_CFG_UART6_DIROE      0x20000

#define PIN_UARTII_CFG_UART7_RTSCTS     0x40000
#define PIN_UARTII_CFG_UART7_DIROE      0x80000

#define PIN_UARTII_CFG_UART8_RTSCTS     0x100000
#define PIN_UARTII_CFG_UART8_DIROE      0x200000

#define PIN_UARTII_CFG_UART9_RTSCTS     0x400000
#define PIN_UARTII_CFG_UART9_DIROE      0x800000

/**
    PIN config for REMOTE

    @note For pinmux_init() with PIN_FUNC_REMOTE.
*/
#define PIN_REMOTE_CFG_NONE             0x0

#define PIN_REMOTE_CFG_REMOTE_1         0x1          ///< REMOTE_1     (P_GPIO[31])
#define PIN_REMOTE_CFG_REMOTE_EXT_1     0x2          ///< REMOTE_EXT_1 (P_GPIO[30])

/**
    PIN config for SDP

    @note For pinmux_init() with PIN_FUNC_SDP.
*/
#define PIN_SDP_CFG_NONE                0x0

#define PIN_SDP_CFG_SDP_1               0x1          ///< SDP_1 (P_GPIO[15..19])
#define PIN_SDP_CFG_SDP_2               0x2          ///< SDP_2 (C_GPIO[17..21])

/**
    PIN config for SPI

    @note For pinmux_init() with PIN_FUNC_SPI.
*/
#define PIN_SPI_CFG_NONE                0x0

#define PIN_SPI_CFG_SPI_1               0x1          ///< SPI_1  (P_GPIO[12..14])  BUS_WIDTH (P_GPIO[15])
#define PIN_SPI_CFG_SPI_2               0x2          ///< SPI1_2 (DSI_GPIO[0..2])  BUS_WIDTH (DSI_GPIO[3])
#define PIN_SPI_CFG_SPI_3               0x4          ///< SPI1_3 (C_GPIO[4..6])    BUS_WIDTH (C_GPIO[7])
#define PIN_SPI_CFG_SPI_BUS_WIDTH       0x8

#define PIN_SPI_CFG_SPI2_1              0x10         ///< SPI2_1 (P_GPIO[16..18])  BUS_WIDTH (P_GPIO[19])
#define PIN_SPI_CFG_SPI2_2              0x20         ///< SPI2_2 (S_GPIO[17..19])  BUS_WIDTH (S_GPIO[20])
#define PIN_SPI_CFG_SPI2_BUS_WIDTH      0x40

#define PIN_SPI_CFG_SPI3_1              0x100        ///< SPI3_1 (P_GPIO[20..22])  BUS_WIDTH (P_GPIO[23])
#define PIN_SPI_CFG_SPI3_2              0x200        ///< SPI3_2 (C_GPIO[17..19])  BUS_WIDTH (C_GPIO[20])
#define PIN_SPI_CFG_SPI3_BUS_WIDTH      0x400

#define PIN_SPI_CFG_SPI4_1              0x1000       ///< SPI4_1 (P_GPIO[24..26])  BUS_WIDTH (P_GPIO[27])
#define PIN_SPI_CFG_SPI4_2              0x2000       ///< SPI4_2 (D_GPIO[2..4])    BUS_WIDTH (D_GPIO[5])
#define PIN_SPI_CFG_SPI4_BUS_WIDTH      0x4000

#define PIN_SPI_CFG_SPI5_1              0x10000      ///< SPI5_1 (P_GPIO[28..30])  BUS_WIDTH (P_GPIO[31])
#define PIN_SPI_CFG_SPI5_2              0x20000      ///< SPI5_2 (L_GPIO[20..22])  BUS_WIDTH (L_GPIO[23])
#define PIN_SPI_CFG_SPI5_BUS_WIDTH      0x40000

#define PIN_SPI_CFG_SPI3_RDY_1          0x100000     ///< SPI3RDY_1 (P_GPIO[27])
#define PIN_SPI_CFG_SPI3_RDY_2          0x200000     ///< SPI3RDY_2 (C_GPIO[21])

/**
    PIN config for SIF

    *note For pinmux_init() with PIN_FUNC_SIF.
*/
#define PIN_SIF_CFG_NONE                0x0

#define PIN_SIF_CFG_SIF0_1              0x1          ///< SIF0_1 (P_GPIO[12..14])
#define PIN_SIF_CFG_SIF0_2              0x2          ///< SIF0_2 (L_GPIO[25..27])
#define PIN_SIF_CFG_SIF0_3              0x4          ///< SIF0_3 (DSI_GPIO[8..10])

#define PIN_SIF_CFG_SIF1_1              0x10         ///< SIF1_1 (P_GPIO[16..18])
#define PIN_SIF_CFG_SIF1_2              0x20         ///< SIF1_2 (L_GPIO[28..30])

#define PIN_SIF_CFG_SIF2_1              0x100        ///< SIF2_1 (P_GPIO[20..22])
#define PIN_SIF_CFG_SIF2_2              0x200        ///< SIF2_2 (S_GPIO[11..12] S_GPIO[15])

#define PIN_SIF_CFG_SIF3_1              0x1000       ///< SIF3_1 (P_GPIO[24..26])
#define PIN_SIF_CFG_SIF3_2              0x2000       ///< SIF3_2 (S_GPIO[13..14] S_GPIO[16])

#define PIN_SIF_CFG_SIF4_1              0x10000      ///< SIF4_1 (P_GPIO[28..30])
#define PIN_SIF_CFG_SIF4_2              0x20000      ///< SIF4_2 (S_GPIO[23..25])

#define PIN_SIF_CFG_SIF5_1              0x100000     ///< SIF5_1 (P_GPIO[8..10])
#define PIN_SIF_CFG_SIF5_2              0x200000     ///< SIF5_2 (C_GPIO[4..6])

/**
    PIN config for MISC

    @note For pinmux_init() for PIN_FUNC_MISC.
*/
#define PIN_MISC_CFG_NONE               0x0

#define PIN_MISC_CFG_RTC_CLK_1          0x1          ///< RTC_CLK_1 (P_GPIO[29])
#define PIN_MISC_CFG_RTC_EXT_CLK_1      0x2          ///< RTC_EXT_CLK_1 (P_GPIO[23])
#define PIN_MISC_CFG_RTC_DIV_OUT_1      0x4          ///< RTC_DIV_OUT_1 (P_GPIO[22])

#define PIN_MISC_CFG_SP_CLK_1           0x10         ///< SP_CLK_1  (D_GPIO[6])

#define PIN_MISC_CFG_SP2_CLK_1          0x100        ///< SP2_CLK_1 (DSI_GPIO[10])

#define PIN_MISC_CFG_SATA_LED_1         0x1000       ///< SATA_LED_1 (D_GPIO[4])

#define PIN_MISC_CFG_ADC                0x100000     ///< ADC (A_GPIO[0..3])

#define PIN_MISC_CFG_MIPI_SEL_DSI       0x1000000    ///< MIPI D-PHY as DSI    (DSI_GPIO[0..9])
#define PIN_MISC_CFG_MIPI_SEL_CSI_TX    0x2000000    ///< MIPI D-PHY as CSI_TX (DSI_GPIO[0..9])

#define PIN_MISC_CFG_CPU_ICE            0x10000000   ///< CPU_ICE (D_GPIO[9..13])

/**
    PIN location of LCD

    @note For pinmux_init() with PIN_FUNC_LCD or PIN_FUNC_LCD2.
    For example  you can use {PIN_FUNC_LCD  PINMUX_DISPMUX_SEL_LCD2|PINMUX_LCDMODE_XXX}
    to tell display object that PIN_FUNC_LCD is located on secondary LCD pinmux.
*/
#define PINMUX_DISPMUX_SEL_NONE                 (0x00 << 28)    ///< PINMUX none
#define PINMUX_DISPMUX_SEL_LCD                  (0x01 << 28)    ///< PINMUX at LCD interface
#define PINMUX_DISPMUX_SEL_LCD2                 (0x02 << 28)    ///< PINMUX at LCD2 interface

#define PINMUX_DISPMUX_SEL_MASK                 (0x03 << 28)

/**
    Pinmux selection for LCD

    @note For pinmux_init() with PIN_FUNC_SEL_LCD or PIN_FUNC_SEL_LCD2.
    For example  you can use {PIN_FUNC_SEL_LCD  PINMUX_LCD_SEL_RGB_16BITS | PINMUX_LCD_SEL_DE_ENABLE}
    to tell pinmux driver that the register of primary LCD should be set to RGB 16 bits
    and the register of PLCD_DE should be set.
*/
#define PINMUX_LCD_SEL_GPIO                     0               ///< GPIO

#define PINMUX_LCD_SEL_CCIR656                  1               ///< CCIR-656 8 bits.
                                                                ///< When PIN_FUNC_SEL_LCD  CCIR_YC[0..7]/CCIR_CLK (L_GPIO[0..8])
                                                                ///< When PIN_FUNC_SEL_LCD2  CCIR_YC[0..7]/CCIR_CLK (L_GPIO[13..21])

#define PINMUX_LCD_SEL_CCIR656_16BITS           2               ///< CCIR-656 16 bits. CCIR_Y[0..7]/CCIR_CLK/CCIR_C[0..7] (L_GPIO[0..8] L_GPIO[12..19])

#define PINMUX_LCD_SEL_CCIR601                  3               ///< CCIR-601 8 bits.
                                                                ///< When PIN_FUNC_SEL_LCD  CCIR_YC[0..7]/CCIR_CLK/CCIR_VD/CCIR_HD (L_GPIO[0..10])
                                                                ///< When PIN_FUNC_SEL_LCD2, CCIR_YC[0..7]/CCIR_CLK/CCIR_VD/CCIR_HD/CCIR_FIELD (L_GPIO[13..23] L_GPIO[25])

#define PINMUX_LCD_SEL_CCIR601_16BITS           4               ///< CCIR-601 16 bits. CCIR_Y[0..7]/CCIR_CLK/CCIR_VD/CCIR_HD/CCIR_C[0..7] (L_GPIO[0..10] L_GPIO[12..19])

#define PINMUX_LCD_SEL_SERIAL_RGB_6BITS         5               ///< Serial RGB 6 bits.
                                                                ///< When PIN_FUNC_SEL_LCD  RGB_D[2..7]/RGB_CLK/RGB_VD/RGB_HD (L_GPIO[2..10])
                                                                ///< When PIN_FUNC_SEL_LCD2  RGB_D[2..7]/RGB_CLK/RGB_VD/RGB_HD (L_GPIO[15..23])

#define PINMUX_LCD_SEL_SERIAL_RGB_8BITS         6               ///< Serial RGB 8 bits.
                                                                ///< When PIN_FUNC_SEL_LCD  RGB_D[0..7]/RGB_CLK/RGB_VD/RGB_HD (L_GPIO[0..10])
                                                                ///< When PIN_FUNC_SEL_LCD2  RGB_D[0..7]/RGB_CLK/RGB_VD/RGB_HD (L_GPIO[13..23])

#define PINMUX_LCD_SEL_SERIAL_YCbCr_8BITS       7               ///< Serial YCbCr 8 bits.
                                                                ///< When PIN_FUNC_SEL_LCD  CCIR_YC[0..7]/CCIR_CLK/CCIR_VD/CCIR_HD (L_GPIO[0..10])
                                                                ///< When PIN_FUNC_SEL_LCD2  CCIR_YC[0..7]/CCIR_CLK/CCIR_VD/CCIR_HD (L_GPIO[13..23])

#define PINMUX_LCD_SEL_PARALLE_RGB565           8               ///< Parallel RGB565. RGB_C0_[0..4]/RGB_DCLK/RGB_VS/RGB_HS/RGB_C1_[0..5]/RGB_C2_[0..4] (L_GPIO[0..10] L_GPIO[12..19])

#define PINMUX_LCD_SEL_PARALLE_RGB666           9               ///< Parallel RGB666. RGB_C0_[0..5]/RGB_DCLK/RGB_VS/RGB_HS/RGB_C1_[0..5]/RGB_C2_[0..5] (L_GPIO[2..10] L_GPIO[14..19] L_GPIO[22..27])

#define PINMUX_LCD_SEL_PARALLE_RGB888           10              ///< Parallel RGB888. RGB_C0_[0..7]/RGB_DCLK/RGB_VS/RGB_HS/RGB_C1_[0..7]/RGB_C2_[0..7] (L_GPIO[0..10] L_GPIO[12..27])

#define PINMUX_LCD_SEL_RGB_16BITS               11              ///< RGB 16 bits. CCIR_Y[0..7]/CCIR_CLK/CCIR_VD/CCIR_HD/CCIR_C[0..7] (L_GPIO[0..10] L_GPIO[12..19])

#define PINMUX_LCD_SEL_MIPI                     12              ///< MIPI DSI

#define PINMUX_LCD_SEL_PARALLE_MI_8BITS         13              ///< Parallel MI 8 bits.
                                                                ///< When PIN_FUNC_SEL_LCD  MPU_D[0..7]/MPU_CS/MPU_RS/MPU_WR/MPU_RD (L_GPIO[0..7] L_GPIO[8..11])
                                                                ///< When PIN_FUNC_SEL_LCD2  MPU2_D[0..7]/MPU2_RS/MPU2_CS/MPU2_WR/MPU2_RD (L_GPIO[12..19] L_GPIO[21..24])

#define PINMUX_LCD_SEL_PARALLE_MI_9BITS         14              ///< Parallel MI 9 bits.
                                                                ///< When PIN_FUNC_SEL_LCD  MPU_D[0..8]/MPU_CS/MPU_RS/MPU_WR/MPU_RD (L_GPIO[0..7][12] L_GPIO[8..11])
                                                                ///< When PIN_FUNC_SEL_LCD2  MPU2_D[0..8]/MPU2_RS/MPU2_CS/MPU2_WR/MPU2_RD (L_GPIO[12..20] L_GPIO[21..24])

#define PINMUX_LCD_SEL_PARALLE_MI_16BITS        15              ///< Parallel MI 16 bits. MPU_D[0..15]/MPU_CS/MPU_RS/MPU_WR/MPU_RD (L_GPIO[0..7][12..19] L_GPIO[8..11])

#define PINMUX_LCD_SEL_PARALLE_MI_18BITS        16              ///< Parallel MI 18 bits. MPU_D[0..17]/MPU_CS/MPU_RS/MPU_WR/MPU_RD (L_GPIO[0..7][12..21] L_GPIO[8..11])

#define PINMUX_LCD_SEL_SERIAL_MI_SDIO           17              ///< Serial MI SDIO bi-direction.
                                                                ///< When PIN_FUNC_SEL_LCD  MPU_CLK/MPU_RS/MPU_CS/MPU_SDIO (L_GPIO[8..10] L_GPIO[7])
                                                                ///< When PIN_FUNC_SEL_LCD2  MPU_CLK/MPU_RS/MPU_CS/MPU_SDIO (L_GPIO[15..17] L_GPIO[14])

#define PINMUX_LCD_SEL_SERIAL_MI_SDI_SDO        18              ///< Serial MI SDI/SDO seperate.
                                                                ///< When PIN_FUNC_SEL_LCD  MPU_CLK/MPU_RS/MPU_CS/MPU_SDO/MPU_SDI (L_GPIO[8..10] L_GPIO[5..6])
                                                                ///< When PIN_FUNC_SEL_LCD2  MPU_CLK/MPU_RS/MPU_CS/MPU_SDO/MPU_SDI (L_GPIO[15..17] L_GPIO[12..13])

#define PINMUX_LCD_SEL_TE_ENABLE                (0x01 << 23)    ///< TE Enable.
                                                                ///< When PIN_FUNC_SEL_LCD  DSI_TE (DSI_GPIO[10])
                                                                ///< When PIN_FUNC_SEL_LCD2  DSI_TE (DSI_GPIO[10])
                                                                ///< When PIN_FUNC_SEL_LCD and PARALLE_MI  MPU_TE (L_GPIO[11])
                                                                ///< When PIN_FUNC_SEL_LCD2 and PARALLE_MI  MPU_TE (L_GPIO[18])
                                                                ///< When PIN_FUNC_SEL_LCD and SERIAL_MI  MI_TE (L_GPIO[22])
                                                                ///< When PIN_FUNC_SEL_LCD2 and SERIAL_MI  MI_TE (L_GPIO[25])

#define PINMUX_LCD_SEL_DE_ENABLE                (0x01 << 24)    ///< DE Enable.
                                                                ///< When PIN_FUNC_SEL_LCD  CCIR_DE (L_GPIO[11])
                                                                ///< When PIN_FUNC_SEL_LCD2  CCIR2_DE (L_GPIO[24])

#define PINMUX_LCD_SEL_HVLD_VVLD                (0x01 << 25)    ///< HVLD/VVLD Enable (For CCIR-601 8 bits). CCIR_HVLD/CCIR_VVLD (L_GPIO[12..13])

#define PINMUX_LCD_SEL_FIELD                    (0x01 << 26)    ///< FIELD Enable (For CCIR-601). CCIR_FIELD: CCIR-601 8 bits(L_GPIO[14])  CCIR-601 16 bits(L_GPIO[25])

#define PINMUX_LCD_SEL_NO_HVSYNC                (0x1 << 27)     ///< No HSYNC/VSYNC (backward compatible)

#define PINMUX_LCD_SEL_FEATURE_MSK              (0x1F << 23)

#endif
