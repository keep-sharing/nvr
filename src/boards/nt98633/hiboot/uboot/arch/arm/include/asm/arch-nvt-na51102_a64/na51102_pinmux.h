#ifndef __PINMUX_DRV_H__
#define __PINMUX_DRV_H__
/*
#include <plat/top_reg.h>
#include <plat/hardware.h>
#include <plat/top.h>
#include <plat/nvt-gpio.h>
#include <plat/pad.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
*/
#include "top_reg.h"
#include "top.h"
#include <asm/nvt-common/nvt_common.h>
#include <asm/arch/hardware.h>

#define DRV_VERSION "1.00.01"
#define C_GPIO_NUM			23
#define L_GPIO_NUM			31
#define P_GPIO_NUM			38
#define HSI_GPIO_NUM		24
#define D_GPIO_NUM			14
#define S_GPIO_NUM			27
#define DSI_GPIO_NUM		11
#define A_GPIO_NUM          4
#if 0
#define TOP_SETREG(info, ofs,value)	   \
{\
	printf("%p, %x\n", info->top_base+(ofs), value); \
	OUTW(info->top_base+(ofs),(value));\
}
#else
#define TOP_SETREG(info, ofs,value) OUTW(info->top_base+(ofs),(value));
#endif
#define TOP_GETREG(info, ofs)		   INW(info->top_base+(ofs))

#define TGE_REG_ADDR(ofs)       (NVT_TGE_BASE_VIRT+(ofs))
#define TGE_GETREG(ofs)         INW(TGE_REG_ADDR(ofs))
#define TGE_SETREG(ofs,value)   OUTW(TGE_REG_ADDR(ofs), (value))
#define TGE_CONTROL_OFS 0x0

#define MAX_PAD_NUM  236

#define GPIO_SETREG(info, ofs,value)	   OUTW(info->gpio_base+(ofs),(value))
#define GPIO_GETREG(info, ofs)		   INW(info->gpio_base+(ofs))

struct nvt_pad_info {
	unsigned long pad_ds_pin;
	unsigned long driving;
	unsigned long pad_gpio_pin;
	unsigned long direction;
};

struct nvt_gpio_info {
	unsigned long gpio_pin;
	unsigned long direction;
};
typedef enum
{
	CGPIO_0,
	CGPIO_1,
	CGPIO_2,
	CGPIO_3,
	CGPIO_4,
	CGPIO_5,
	CGPIO_6,
	CGPIO_7,
	CGPIO_8,
	CGPIO_9,
	CGPIO_10,
	CGPIO_11,
	CGPIO_12,
	CGPIO_13,
	CGPIO_14,
	CGPIO_15,
	CGPIO_16,
	CGPIO_17,
	CGPIO_18,
	CGPIO_19,
	CGPIO_20,
	CGPIO_21,
	CGPIO_22,
	CGPIO_23,
	C_GPIO_all,
	HSIGPIO_0,
	HSIGPIO_1,
	HSIGPIO_2,
	HSIGPIO_3,
	HSIGPIO_4,
	HSIGPIO_5,
	HSIGPIO_6,
	HSIGPIO_7,
	HSIGPIO_8,
	HSIGPIO_9,
	HSIGPIO_10,
	HSIGPIO_11,
	HSIGPIO_12,
	HSIGPIO_13,
	HSIGPIO_14,
	HSIGPIO_15,
	HSIGPIO_16,
	HSIGPIO_17,
	HSIGPIO_18,
	HSIGPIO_19,
	HSIGPIO_20,
	HSIGPIO_21,
	HSIGPIO_22,
	HSIGPIO_23,
	HSI_GPIO_all,
	SGPIO_0,
	SGPIO_1,
	SGPIO_2,
	SGPIO_3,
	SGPIO_4,
	SGPIO_5,
	SGPIO_6,
	SGPIO_7,
	SGPIO_8,
	SGPIO_9,
	SGPIO_10,
	SGPIO_11,
	SGPIO_12,
	SGPIO_13,
	SGPIO_14,
	SGPIO_15,
	SGPIO_16,
	SGPIO_17,
	SGPIO_18,
	SGPIO_19,
	SGPIO_20,
	SGPIO_21,
	SGPIO_22,
	SGPIO_23,
	SGPIO_24,
	SGPIO_25,
	SGPIO_26,
	S_GPIO_all,
	LGPIO_0,
	LGPIO_1,
	LGPIO_2,
	LGPIO_3,
	LGPIO_4,
	LGPIO_5,
	LGPIO_6,
	LGPIO_7,
	LGPIO_8,
	LGPIO_9,
	LGPIO_10,
	LGPIO_11,
	LGPIO_12,
	LGPIO_13,
	LGPIO_14,
	LGPIO_15,
	LGPIO_16,
	LGPIO_17,
	LGPIO_18,
	LGPIO_19,
	LGPIO_20,
	LGPIO_21,
	LGPIO_22,
	LGPIO_23,
	LGPIO_24,
	LGPIO_25,
	LGPIO_26,
	LGPIO_27,
	LGPIO_28,
	LGPIO_29,
	LGPIO_30,
	L_GPIO_all,
	DSIGPIO_0,
	DSIGPIO_1,
	DSIGPIO_2,
	DSIGPIO_3,
	DSIGPIO_4,
	DSIGPIO_5,
	DSIGPIO_6,
	DSIGPIO_7,
	DSIGPIO_8,
	DSIGPIO_9,
	DSIGPIO_10,
	DSI_GPIO_all,
	AGPIO_0,
	AGPIO_1,
	AGPIO_2,
	AGPIO_3,
	A_GPIO_all,
	PGPIO_0,
	PGPIO_1,
	PGPIO_2,
	PGPIO_3,
	PGPIO_4,
	PGPIO_5,
	PGPIO_6,
	PGPIO_7,
	PGPIO_8,
	PGPIO_9,
	PGPIO_10,
	PGPIO_11,
	PGPIO_12,
	PGPIO_13,
	PGPIO_14,
	PGPIO_15,
	PGPIO_16,
	PGPIO_17,
	PGPIO_18,
	PGPIO_19,
	PGPIO_20,
	PGPIO_21,
	PGPIO_22,
	PGPIO_23,
	PGPIO_24,
	PGPIO_25,
	PGPIO_26,
	PGPIO_27,
	PGPIO_28,
	PGPIO_29,
	PGPIO_30,
	PGPIO_31,
	PGPIO_32,
	PGPIO_33,
	PGPIO_35,
	PGPIO_36,
	PGPIO_37,
	P_GPIO_all,
	DGPIO_0,
	DGPIO_1,
	DGPIO_2,
	DGPIO_3,
	DGPIO_4,
	DGPIO_5,
	DGPIO_6,
	DGPIO_7,
	DGPIO_8,
	DGPIO_9,
	DGPIO_10,
	DGPIO_11,
	DGPIO_12,
	DGPIO_13,
	D_GPIO_all,

	GPIO_total,



	ENUM_DUMMY4WORD(GPIO_ALL)
} GPIO_ALL;

typedef enum
{
	func_FSPI=1,
	func_SDIO,
	func_SDIO2,
	func_SDIO3,
	func_EJTAG,
	func_EXTROM,
	func_ETH,
	func_ETH2,
	/*func_CCIR626,
	func_CCIR601,
	func_LCD310_RGB888,
	func_LCD210_1,
	func_LCD210_2,
	func_LCD310_DE,*/
	/*I2C*/
	func_I2C,
	func_I2C2,
	func_I2C3_1,
	func_I2C3_2,
	func_I2C3_3,
	func_I2C4_1,
	func_I2C4_2,
	func_I2C4_3,
    func_I2C4_4,
    func_I2C5_1,
	func_I2C5_2,
	func_I2C5_3,
    func_I2C6_1,
	func_I2C6_2,
	func_I2C6_3,
    func_I2C7_1,
	func_I2C7_2,
	func_I2C8_1,
	func_I2C8_2,
	func_I2C9_1,
	func_I2C9_2,
	func_I2C10_1,
	func_I2C10_2,
	func_I2C11_1,
	/*PWM*/
	func_PWM_1,
	func_PWM_2,
	func_PWM1_1,
	func_PWM1_2,
	func_PWM2_1,
	func_PWM2_2,
	func_PWM3_1,
	func_PWM3_2,
	func_PWM4_1,
	func_PWM4_2,
	func_PWM5_1,
	func_PWM5_2,
	func_PWM6_1,
	func_PWM6_2,
	func_PWM7_1,
	func_PWM7_2,
	func_PWM8_1,
	func_PWM8_2,
	func_PWM9_1,
	func_PWM9_2,
	func_PWM10_1,
	func_PWM10_2,
	func_PWM11_1,
	func_PWM11_2,
	/*CCNT*/
    func_CCNT,
    func_CCNT2,
    func_CCNT3,
    /*SENSOR*/
    func_SENSOR,
    /*func_SENSOR_SN3_MCLK2,
    func_SENSOR_SN4_MCLK2,*/
    func_SENSOR2,
    /*func_SENSOR2_CCIR8_A,
    func_SENSOR2_CCIR8_B,
    func_SENSOR2_CCIR8_AB,
    func_SENSOR2_CCIR16,
    func_SENSOR2_CCIR_VSHS,
    func_SENSOR2_SN1_MCLK,
    func_SENSOR2_SN2_MCLK,*/
    func_SENSOR3,
    /*func_SENSOR3_CCIR8_A,
    func_SENSOR3_CCIR8_B,
    func_SENSOR3_CCIR8_AB,
    func_SENSOR3_CCIR16,
    func_SENSOR3_CCIR_VSHS,
    func_SENSOR3_SN5_MCLK2,*/
    func_SENSORMISC,
    func_SN1_MCLK,
    func_SN1_XVSXHS,
    func_SN2_MCLK,
    func_SN2_XVSXHS,
    func_SN3_MCLK,
    func_SN3_XVSXHS,
    func_SN4_MCLK,
    func_SN4_XVSXHS,
    func_SN5_MCLK,
    func_SN5_XVSXHS,
    func_MIPI,
    func_I2S_1,
	func_I2S_1_MCLK,
	func_I2S_2,
	func_I2S_2_MCLK,
	func_I2S2_1,
	func_I2S2_1_MCLK,
	func_AUDIO_DMIC,
    func_AUDIO_EXT_MCLK,
    func_UART,
	func_UART2,
	func_UART3,
	func_UART4_1,
	func_UART4_2,
	func_UART5_1,
	func_UART5_2,
	func_UART6_1,
	func_UART6_2,
	func_UART7_1,
	func_UART7_2,
	func_UART8_1,
	func_UART8_2,
	func_UART9_1,
	func_UART9_2,
	func_UART2_CTSRTS,
	func_UART2_DTROE,
	func_UART3_CTSRTS,
	func_UART3_DTROE,
	func_UART4_CTSRTS,
	func_UART4_DTROE,
	func_UART5_CTSRTS,
	func_UART5_DTROE,
	func_UART6_CTSRTS,
	func_UART6_DTROE,
	func_UART7_CTSRTS,
	func_UART7_DTROE,
	func_UART8_CTSRTS,
	func_UART8_DTROE,
	func_UART9_CTSRTS,
	func_UART9_DTROE,
	func_Remote,
    func_SDP_1,
    func_SDP_2,
    func_SPI_1,
    func_SPI_2,
    func_SPI_3,
    func_SPI2_1,
    func_SPI2_2,
    func_SPI3_1,
    func_SPI3_2,
    func_SPI4_1,
    func_SPI4_2,
    func_SPI5_1,
    func_SPI5_2,
    func_SPI3_RDY,
    func_SPI3_RDY2,
    func_SIF_1,
    func_SIF_2,
    func_SIF_3,
    func_SIF1_1,
    func_SIF1_2,
    func_SIF2_1,
    func_SIF2_2,
    func_SIF3_1,
    func_SIF3_2,
    func_SIF4_1,
    func_SIF4_2,
    func_SIF5_1,
    func_SIF5_2,
    func_MISC,
    func_LCD,
    func_LCD2,
	FUNC_total,



	ENUM_DUMMY4WORD(FUNC_ALL)
} FUNC_ALL;
struct nvt_pinctrl_info {
	void __iomem *top_base;
	void __iomem *pad_base;
	void __iomem *gpio_base;
	PIN_GROUP_CONFIG top_pinmux[PIN_FUNC_MAX];
	struct nvt_pad_info pad[MAX_PAD_NUM];
};

#define MAX_MODULE_NAME 10

ER pinmux_init(struct nvt_pinctrl_info *info);
ER pad_init(struct nvt_pinctrl_info *info, unsigned long nr_pad);
void gpio_init(struct nvt_gpio_info *gpio, int nr_gpio, struct nvt_pinctrl_info *info);
void pinmux_parsing(struct nvt_pinctrl_info *info);
int nvt_pinmux_proc_init(void);
void pinmux_preset(struct nvt_pinctrl_info *info);
void pad_preset(struct nvt_pinctrl_info *info);
void pinmux_gpio_parsing(struct nvt_pinctrl_info *info);
int pinmux_set_config(PINMUX_FUNC_ID id, u32 pinmux);

static inline int pinmux_set_host(struct nvt_pinctrl_info *info, PINMUX_FUNC_ID id, u32 pinmux) {return 0;};
#endif
