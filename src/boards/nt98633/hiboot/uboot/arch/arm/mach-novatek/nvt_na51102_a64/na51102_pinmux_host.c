/*
	Pinmux module driver.

	This file is the driver of Piumux module.

	@file		na51102_pinmux_host.c
	@ingroup
	@note		Nothing.

	Copyright   Novatek Microelectronics Corp. 2021.  All rights reserved.
*/

#include <asm/arch/na51102_pinmux.h>
//#include <plat/pad.h>

//static DEFINE_SPINLOCK(top_lock);
//#define loc_cpu(flags) spin_lock_irqsave(&top_lock, flags)
//#define unl_cpu(flags) spin_unlock_irqrestore(&top_lock, flags)

#define loc_cpu(flags) 
#define unl_cpu(flags) 

union TOP_REG0         top_reg0;
union TOP_REG1         top_reg1;
union TOP_REG2         top_reg2;
union TOP_REG3         top_reg3;
union TOP_REG4         top_reg4;
union TOP_REG5         top_reg5;
union TOP_REG6         top_reg6;
union TOP_REG7         top_reg7;
union TOP_REG8         top_reg8;
union TOP_REG9         top_reg9;
union TOP_REG10        top_reg10;
union TOP_REG11        top_reg11;
union TOP_REG12        top_reg12;
union TOP_REG13        top_reg13;
union TOP_REG14        top_reg14;
union TOP_REG15        top_reg15;
union TOP_REG16        top_reg16;
union TOP_REG17        top_reg17;
union TOP_REG18        top_reg18;
union TOP_REG19        top_reg19;
union TOP_REGCGPIO0    top_reg_cgpio0;
union TOP_REGPGPIO0    top_reg_pgpio0;
union TOP_REGPGPIO1    top_reg_pgpio1;
union TOP_REGDGPIO0    top_reg_dgpio0;
union TOP_REGLGPIO0    top_reg_lgpio0;
union TOP_REGSGPIO0    top_reg_sgpio0;
union TOP_REGHSIGPIO0  top_reg_hsigpio0;
union TOP_REGDSIGPIO0  top_reg_dsigpio0;
union TOP_REGAGPIO0    top_reg_agpio0;

//int confl_detect;
static int dump_gpio_func[GPIO_total]={0};
static char *dump_func[FUNC_total]={"FSPI","SDIO1","SDIO1_2","SDIO1_3","EJTAG","EXTROM","ETH","ETH2","I2C","I2C2",
     "I2C3_1","I2C3_2","I2C3_3","I2C4_1","I2C4_2","I2C4_3","I2C4_4","I2C5_1","I2C5_2","I2C5_3","I2C6_1","I2C6_2","I2C6_3",
     "I2C7_1","I2C7_2","I2C8_1","I2C8_2","I2C9_1","I2C9_2","I2C10_1","I2C10_2","I2C11_1","PWM_1" ,"PWM_2" ,"PWM1_1","PWM1_2",
     "PWM2_1","PWM2_2","PWM3_1","PWM3_2","PWM4_1","PWM4_2","PWM5_1","PWM5_2","PWM6_1","PWM6_2","PWM7_1","PWM7_2","PWM8_1","PWM8_2",
     "PWM9_1","PWM9_2","PWM10_1","PWM10_2","PWM11_1","PWM11_2","CCNT","CCNT2","CCNT3" ,"SENSOR","SENSOR2","SENSOR3","SENSORMISC",
     "SN1_MCLK","SN1_XVSXHS","SN2_MCLK","SN2_MCLK","SN3_MCLK","SN3_MCLK","SN4_MCLK","SN4_MCLK","SN5_MCLK","SN5_MCLK","MIPI","I2S_1",
     "I2S_1_MCLK","I2S_2","I2S_2_MCLK","I2S2_1","I2S2_1_MCLK","AUDIO_DMIC","AUDIO_EXT_MCLK","UART","UART2","UART3","UART4_1","UART4_2",
     "UART5_1","UART5_2","UART6_1","UART6_2","UART7_1","UART7_2","UART8_1","UART8_2","UART9_1","UART9_2","UART2_CTS_RTS","UART2_DTROE",
     "UART3_CTS_RTS","UART3_DTROE","UART4_CTS_RTS","UART4_DTROE","UART5_CTS_RTS","UART5_DTROE","UART6_CTS_RTS","UART6_DTROE","UART7_CTS_RTS",
     "UART7_DTROE","UART8_CTS_RTS","UART8_DTROE","UART9_CTS_RTS","UART9_DTROE","Remote","SDP_1","SDP_2","SPI_1","SPI_2","SPI_3","SPI2_1",
     "SPI2_2","SPI3_1","SPI3_2","SPI4_1","SPI4_2","SPI5_1","SPI5_2","SPI3_RDY","SPI3_RDY2","SIF_1","SIF_2","SIF_3","SIF1_1","SIF1_2",
     "SIF2_1","SIF2_2","SIF3_1","SIF3_2","SIF4_1","SIF4_2","SIF5_1","SIF5_2","MISC","LCD","LCD2"};


struct nvt_pinctrl_info info_get_id[1] = {0};
#include <dm/of.h>
#if 0
uint32_t nvt_get_chip_id(void)
{
	union TOP_VERSION_REG top_version;
	struct device_node *top;
	static void __iomem *top_reg_addr = NULL;;
	//u32 value[6] = {};

	if (!top_reg_addr) {
		top = of_find_compatible_node(NULL, NULL, "nvt,nvt_top");
		if (top) {
			const __be32 *cell;
			cell = of_get_property(top, "reg", NULL);
			if (cell) {
				phys_addr_t top_pa = 0;
				top_pa = of_read_number(cell, of_n_addr_cells(top));
				//printk("size = %d\r\n", of_n_addr_cells(top));
				//printk("%s: get reg addr 0x%lx \r\n", __func__, top_pa);
				top_reg_addr = ioremap_nocache(top_pa, 0x100);
			} else {
				pr_err("*** %s not get top reg ***\n", __func__);
				return -ENOMEM;
			}
		} else {
			pr_err("*** %s not get dts node ***\n", __func__);
			return -ENOMEM;
		}
	}

	if (top_reg_addr) {
		info_get_id->top_base = top_reg_addr;
		top_version.reg = TOP_GETREG(info_get_id, TOP_VERSION_REG_OFS);
	} else {
		pr_err("invalid pinmux address\n");
		return -ENOMEM;
	}

	return top_version.bit.CHIP_ID;
}
EXPORT_SYMBOL(nvt_get_chip_id);
#endif

static void gpio_info_show(struct nvt_pinctrl_info *info, unsigned long gpio_number, unsigned long start_offset)
{
	int i = 0, j = 0;
	unsigned long reg_value;
	char* gpio_name[] = {"C_GPIO", "P_GPIO", "D_GPIO", "L_GPIO", "S_GPIO", "HSI_GPIO", "DSI_GPIO", "A_GPIO"};
	char name[10];

	if (start_offset == TOP_REGCGPIO0_OFS)
		strcpy(name, gpio_name[0]);
	else if (start_offset == TOP_REGPGPIO0_OFS)
		strcpy(name, gpio_name[1]);
	else if (start_offset == TOP_REGDGPIO0_OFS)
		strcpy(name, gpio_name[2]);
	else if (start_offset == TOP_REGLGPIO0_OFS)
		strcpy(name, gpio_name[3]);
	else if (start_offset == TOP_REGSGPIO0_OFS)
		strcpy(name, gpio_name[4]);
	else if (start_offset == TOP_REGHSIGPIO0_OFS)
		strcpy(name, gpio_name[5]);
	else if (start_offset == TOP_REGDSIGPIO0_OFS)
		strcpy(name, gpio_name[6]);
	else if (start_offset == TOP_REGAGPIO0_OFS)
		strcpy(name, gpio_name[7]);

	if (gpio_number > 0x20) {
		reg_value = TOP_GETREG(info, start_offset);

		for (i = 0; i < 0x20; i++) {
			if (reg_value & (1 << i))
				pr_info("%-12s%-4d      GPIO\n", name, i);
			else
				pr_info("%-12s%-4d      FUNCTION\n", name, i);
		}

		reg_value = TOP_GETREG(info, start_offset + 0x4);

		for (j = 0; j < (gpio_number - 0x20); j++) {
			if (reg_value & (1 << j))
				pr_info("%-12s%-4d      GPIO\n", name, i);
			else
				pr_info("%-12s%-4d      FUNCTION\n", name, i);
			i++;
		}
	} else {
		reg_value = TOP_GETREG(info, start_offset);
		for (i = 0; i < gpio_number; i++) {
			if (reg_value & (1 << i))
				pr_info("%-12s%-4d      GPIO\n", name, i);
			else
				pr_info("%-12s%-4d      FUNCTION\n", name, i);
		}
	}
}

void pinmux_gpio_parsing(struct nvt_pinctrl_info *info)
{
	pr_info("\n[PIN]       [NO]      [STATUS]\n");

	gpio_info_show(info, C_GPIO_NUM, TOP_REGCGPIO0_OFS);
	gpio_info_show(info, P_GPIO_NUM, TOP_REGPGPIO0_OFS);
	gpio_info_show(info, D_GPIO_NUM, TOP_REGDGPIO0_OFS);
	gpio_info_show(info, L_GPIO_NUM, TOP_REGLGPIO0_OFS);
	gpio_info_show(info, S_GPIO_NUM, TOP_REGSGPIO0_OFS);
	gpio_info_show(info, HSI_GPIO_NUM, TOP_REGHSIGPIO0_OFS);
	gpio_info_show(info, DSI_GPIO_NUM, TOP_REGDSIGPIO0_OFS);
	gpio_info_show(info, A_GPIO_NUM, TOP_REGAGPIO0_OFS);
}

static uint32_t disp_pinmux_config[] =
{
	PINMUX_DISPMUX_SEL_NONE,            // LCD
	PINMUX_DISPMUX_SEL_NONE,            // LCD2
	PINMUX_TV_HDMI_CFG_NORMAL,          // TV
	PINMUX_TV_HDMI_CFG_NORMAL           // HDMI
};

void pinmux_preset(struct nvt_pinctrl_info *info)
{
}

/**
	Get Display PINMUX setting

	Display driver (LCD/TV/HDMI) can get mode setting from pinmux_init()

	@param[in] id   LCD ID
			- @b PINMUX_DISP_ID_LCD: 1st LCD
			- @b PINMUX_DISP_ID_LCD2: 2nd LCD
			- @b PINMUX_DISP_ID_TV: TV
			- @b PINMUX_DISP_ID_HDMI: HDMI

	@return LCD pinmux setting
*/
PINMUX_LCDINIT pinmux_get_dispmode(PINMUX_FUNC_ID id)
{
	if (id <= PINMUX_FUNC_ID_LCD2) {
		return disp_pinmux_config[id] & ~(PINMUX_DISPMUX_SEL_MASK | PINMUX_LCD_SEL_FEATURE_MSK);
	} else if (id <= PINMUX_FUNC_ID_HDMI) {
		return disp_pinmux_config[id] & ~PINMUX_HDMI_CFG_MASK;
	}

	return 0;
}
//EXPORT_SYMBOL(pinmux_get_dispmode);

/**
	Read pinmux data from controller base

	@param[in] info	nvt_pinctrl_info
*/
void pinmux_parsing(struct nvt_pinctrl_info *info)
{
	u32 value;
	//unsigned long flags = 0;
	union TOP_REG1 local_top_reg1;
	union TOP_REG2 local_top_reg2;
	union TOP_REG3 local_top_reg3;
	union TOP_REG4 local_top_reg4;
	union TOP_REG5 local_top_reg5;
	union TOP_REG6 local_top_reg6;
	union TOP_REG7 local_top_reg7;
//	union TOP_REG8 local_top_reg8;
	union TOP_REG9 local_top_reg9;
	union TOP_REG10 local_top_reg10;
	union TOP_REG11 local_top_reg11;
	union TOP_REG12 local_top_reg12;
	union TOP_REG13 local_top_reg13;
	union TOP_REG14 local_top_reg14;
	union TOP_REG15 local_top_reg15;
	union TOP_REG16 local_top_reg16;
	union TOP_REG17 local_top_reg17;
	union TOP_REG18 local_top_reg18;
	union TOP_REG19 local_top_reg19;

//	union TOP_REGSGPIO0 local_top_reg_sgpio0;
//	union TOP_REGHSIGPIO0 local_top_reg_hgpio0;

	/* Enter critical section */
	//loc_cpu(flags);

	local_top_reg1.reg = TOP_GETREG(info, TOP_REG1_OFS);
	local_top_reg2.reg = TOP_GETREG(info, TOP_REG2_OFS);
	local_top_reg3.reg = TOP_GETREG(info, TOP_REG3_OFS);
	local_top_reg4.reg = TOP_GETREG(info, TOP_REG4_OFS);
	local_top_reg5.reg = TOP_GETREG(info, TOP_REG5_OFS);
	local_top_reg6.reg = TOP_GETREG(info, TOP_REG6_OFS);
	local_top_reg7.reg = TOP_GETREG(info, TOP_REG7_OFS);
//	local_top_reg8.reg = TOP_GETREG(info, TOP_REG8_OFS);
	local_top_reg9.reg = TOP_GETREG(info, TOP_REG9_OFS);
	local_top_reg10.reg = TOP_GETREG(info, TOP_REG10_OFS);
	local_top_reg11.reg = TOP_GETREG(info, TOP_REG11_OFS);
	local_top_reg12.reg = TOP_GETREG(info, TOP_REG12_OFS);
	local_top_reg13.reg = TOP_GETREG(info, TOP_REG13_OFS);
	local_top_reg14.reg = TOP_GETREG(info, TOP_REG14_OFS);
	local_top_reg15.reg = TOP_GETREG(info, TOP_REG15_OFS);
	local_top_reg16.reg = TOP_GETREG(info, TOP_REG16_OFS);
	local_top_reg17.reg = TOP_GETREG(info, TOP_REG17_OFS);
	local_top_reg18.reg = TOP_GETREG(info, TOP_REG18_OFS);
	local_top_reg19.reg = TOP_GETREG(info, TOP_REG19_OFS);

//	local_top_reg_sgpio0.reg = TOP_GETREG(info, TOP_REGSGPIO0_OFS);
//	local_top_reg_hgpio0.reg = TOP_GETREG(info, TOP_REGHSIGPIO0_OFS);

	//printf("%s 0x%x , %p\n", __func__, local_top_reg1.reg, info->top_base);
	/* Parsing SDIO */
	value = PIN_SDIO_CFG_NONE;

	if (local_top_reg1.bit.SDIO_EN == MUX_1) {
		value |= PIN_SDIO_CFG_SDIO_1;
	}

	if (local_top_reg1.bit.SDIO2_EN == MUX_1 )  {
		value |= PIN_SDIO_CFG_SDIO2_1;
	}

	if (local_top_reg1.bit.SDIO3_EN == MUX_1) {
		value |= PIN_SDIO_CFG_SDIO3_1;
	}
	if (local_top_reg1.bit.SDIO3_BUS_WIDTH == MUX_1) {
		value |= PIN_SDIO_CFG_SDIO3_BUS_WIDTH;
	}
	if (local_top_reg1.bit.SDIO3_DS_EN == MUX_1) {
		value |= PIN_SDIO_CFG_SDIO3_DS;
	}

	info->top_pinmux[PIN_FUNC_SDIO].config = value;
	info->top_pinmux[PIN_FUNC_SDIO].pin_function = PIN_FUNC_SDIO;

	/* Parsing NAND */
	value = PIN_NAND_CFG_NONE;

	if (local_top_reg1.bit.FSPI_EN) {
		value |= PIN_NAND_CFG_NAND_1;
	}
	if (local_top_reg1.bit.FSPI_CS1_EN) {
		value |= PIN_NAND_CFG_NAND_CS1;
	}

	info->top_pinmux[PIN_FUNC_NAND].config = value;
	info->top_pinmux[PIN_FUNC_NAND].pin_function = PIN_FUNC_NAND;

	/* Parsing ETH */
	value = PIN_ETH_CFG_NONE;

	if (local_top_reg3.bit.ETH == MUX_1) {
		value |= PIN_ETH_CFG_ETH_RGMII_1;
	} else if (local_top_reg3.bit.ETH == MUX_2) {
		value |= PIN_ETH_CFG_ETH_RMII_1;
	}
	if (local_top_reg3.bit.ETH_EXT_PHY_CLK == MUX_1) {
		value |= PIN_ETH_CFG_ETH_EXTPHYCLK;
	}
	if (local_top_reg3.bit.ETH_PTP == MUX_1) {
		value |= PIN_ETH_CFG_ETH_PTP;
	}

	if (local_top_reg3.bit.ETH2 == MUX_1) {
		value |= PIN_ETH_CFG_ETH2_RGMII_1;
	} else if (local_top_reg3.bit.ETH2 == MUX_2) {
		value |= PIN_ETH_CFG_ETH2_RMII_1;
	}
	if (local_top_reg3.bit.ETH2_EXT_PHY_CLK == MUX_1) {
		value |= PIN_ETH_CFG_ETH2_EXTPHYCLK;
	}
	if (local_top_reg3.bit.ETH2_PTP == MUX_1) {
		value |= PIN_ETH_CFG_ETH2_PTP;
	}

	info->top_pinmux[PIN_FUNC_ETH].config = value;
	info->top_pinmux[PIN_FUNC_ETH].pin_function = PIN_FUNC_ETH;

	/* Parsing I2C */
	value = PIN_I2C_CFG_NONE;

	if (local_top_reg4.bit.I2C == MUX_1) {
		value |= PIN_I2C_CFG_I2C_1;
	}

	if (local_top_reg4.bit.I2C2 == MUX_1) {
		value |= PIN_I2C_CFG_I2C2_1;
	}

	if (local_top_reg4.bit.I2C3 == MUX_1) {
		value |= PIN_I2C_CFG_I2C3_1;
	} else if (local_top_reg4.bit.I2C3 == MUX_2) {
		value |= PIN_I2C_CFG_I2C3_2;
	} else if (local_top_reg4.bit.I2C3 == MUX_3) {
		value |= PIN_I2C_CFG_I2C3_3;
	}

	if (local_top_reg4.bit.I2C4 == MUX_1) {
		value |= PIN_I2C_CFG_I2C4_1;
	} else if (local_top_reg4.bit.I2C4 == MUX_2) {
		value |= PIN_I2C_CFG_I2C4_2;
	} else if (local_top_reg4.bit.I2C4 == MUX_3) {
		value |= PIN_I2C_CFG_I2C4_3;
	} else if (local_top_reg4.bit.I2C4 == MUX_4) {
		value |= PIN_I2C_CFG_I2C4_4;
	}

	if (local_top_reg4.bit.I2C5 == MUX_1) {
		value |= PIN_I2C_CFG_I2C5_1;
	} else if (local_top_reg4.bit.I2C5 == MUX_2) {
		value |= PIN_I2C_CFG_I2C5_2;
	} else if (local_top_reg4.bit.I2C5 == MUX_3) {
		value |= PIN_I2C_CFG_I2C5_3;
	}

	info->top_pinmux[PIN_FUNC_I2C].config = value;
	info->top_pinmux[PIN_FUNC_I2C].pin_function = PIN_FUNC_I2C;

	/* Parsing I2CII */
	value = PIN_I2CII_CFG_NONE;

	if (local_top_reg4.bit.I2C6 == MUX_1) {
		value |= PIN_I2CII_CFG_I2C6_1;
	} else if (local_top_reg4.bit.I2C6 == MUX_2) {
		value |= PIN_I2CII_CFG_I2C6_2;
	} else if (local_top_reg4.bit.I2C6 == MUX_3) {
		value |= PIN_I2CII_CFG_I2C6_3;
	}

	if (local_top_reg4.bit.I2C7 == MUX_1) {
		value |= PIN_I2CII_CFG_I2C7_1;
	} else if (local_top_reg4.bit.I2C7 == MUX_2) {
		value |= PIN_I2CII_CFG_I2C7_2;
	}

	if (local_top_reg4.bit.I2C8 == MUX_1) {
		value |= PIN_I2CII_CFG_I2C8_1;
	} else if (local_top_reg4.bit.I2C8 == MUX_2) {
		value |= PIN_I2CII_CFG_I2C8_2;
	}

	if (local_top_reg5.bit.I2C9 == MUX_1) {
		value |= PIN_I2CII_CFG_I2C9_1;
	} else if (local_top_reg5.bit.I2C9 == MUX_2) {
		value |= PIN_I2CII_CFG_I2C9_2;
	}

	if (local_top_reg5.bit.I2C10 == MUX_1) {
		value |= PIN_I2CII_CFG_I2C10_1;
	} else if (local_top_reg5.bit.I2C10 == MUX_2) {
		value |= PIN_I2CII_CFG_I2C10_2;
	}

	if (local_top_reg5.bit.I2C11 == MUX_1) {
		value |= PIN_I2CII_CFG_I2C11_1;
	}

	info->top_pinmux[PIN_FUNC_I2CII].config = value;
	info->top_pinmux[PIN_FUNC_I2CII].pin_function = PIN_FUNC_I2CII;

	/* Parsing PWM */
	value = PIN_PWM_CFG_NONE;

	if (local_top_reg6.bit.PWM0 == MUX_1) {
		value |= PIN_PWM_CFG_PWM0_1;
	} else if (local_top_reg6.bit.PWM0 == MUX_2) {
		value |= PIN_PWM_CFG_PWM0_2;
	}

	if (local_top_reg6.bit.PWM1 == MUX_1) {
		value |= PIN_PWM_CFG_PWM1_1;
	} else if (local_top_reg6.bit.PWM1 == MUX_2) {
		value |= PIN_PWM_CFG_PWM1_2;
	}

	if (local_top_reg6.bit.PWM2 == MUX_1) {
		value |= PIN_PWM_CFG_PWM2_1;
	} else if (local_top_reg6.bit.PWM2 == MUX_2) {
		value |= PIN_PWM_CFG_PWM2_2;
	}

	if (local_top_reg6.bit.PWM3 == MUX_1) {
		value |= PIN_PWM_CFG_PWM3_1;
	} else if (local_top_reg6.bit.PWM3 == MUX_2) {
		value |= PIN_PWM_CFG_PWM3_2;
	}

	if (local_top_reg6.bit.PWM4 == MUX_1) {
		value |= PIN_PWM_CFG_PWM4_1;
	} else if (local_top_reg6.bit.PWM4 == MUX_2) {
		value |= PIN_PWM_CFG_PWM4_2;
	}

	if (local_top_reg6.bit.PWM5 == MUX_1) {
		value |= PIN_PWM_CFG_PWM5_1;
	} else if (local_top_reg6.bit.PWM5 == MUX_2) {
		value |= PIN_PWM_CFG_PWM5_2;
	}

	info->top_pinmux[PIN_FUNC_PWM].config = value;
	info->top_pinmux[PIN_FUNC_PWM].pin_function = PIN_FUNC_PWM;

	/* Parsing PWMII */
	value = PIN_PWMII_CFG_NONE;

	if (local_top_reg6.bit.PWM6 == MUX_1) {
		value |= PIN_PWMII_CFG_PWM6_1;
	} else if (local_top_reg6.bit.PWM6 == MUX_2) {
		value |= PIN_PWMII_CFG_PWM6_2;
	}

	if (local_top_reg6.bit.PWM7 == MUX_1) {
		value |= PIN_PWMII_CFG_PWM7_1;
	} else if (local_top_reg6.bit.PWM7 == MUX_2) {
		value |= PIN_PWMII_CFG_PWM7_2;
	}

	if (local_top_reg7.bit.PWM8 == MUX_1) {
		value |= PIN_PWMII_CFG_PWM8_1;
	} else if (local_top_reg7.bit.PWM8 == MUX_2) {
		value |= PIN_PWMII_CFG_PWM8_2;
	}

	if (local_top_reg7.bit.PWM9 == MUX_1) {
		value |= PIN_PWMII_CFG_PWM9_1;
	} else if (local_top_reg7.bit.PWM9 == MUX_2) {
		value |= PIN_PWMII_CFG_PWM9_2;
	}

	if (local_top_reg7.bit.PWM10 == MUX_1) {
		value |= PIN_PWMII_CFG_PWM10_1;
	} else if (local_top_reg7.bit.PWM10 == MUX_2) {
		value |= PIN_PWMII_CFG_PWM10_2;
	}

	if (local_top_reg7.bit.PWM11 == MUX_1) {
		value |= PIN_PWMII_CFG_PWM11_1;
	} else if (local_top_reg7.bit.PWM11 == MUX_2) {
		value |= PIN_PWMII_CFG_PWM11_2;
	}

	info->top_pinmux[PIN_FUNC_PWMII].config = value;
	info->top_pinmux[PIN_FUNC_PWMII].pin_function = PIN_FUNC_PWMII;

	/* Parsing CCNT */
	value = PIN_CCNT_CFG_NONE;

	if (local_top_reg7.bit.PICNT == MUX_1) {
		value |= PIN_CCNT_CFG_CCNT_1;
	}

	if (local_top_reg7.bit.PICNT2 == MUX_1) {
		value |= PIN_CCNT_CFG_CCNT2_1;
	}

	if (local_top_reg7.bit.PICNT3 == MUX_1) {
		value |= PIN_CCNT_CFG_CCNT3_1;
	}

	info->top_pinmux[PIN_FUNC_CCNT].config = value;
	info->top_pinmux[PIN_FUNC_CCNT].pin_function = PIN_FUNC_CCNT;

	/* Parsing SENSOR */
	value = PIN_SENSOR_CFG_NONE;

	if (local_top_reg9.bit.SENSOR == MUX_1) {
		value |= PIN_SENSOR_CFG_12BITS;
	}

	if (local_top_reg10.bit.SN3_MCLK == MUX_2) {
		value |= PIN_SENSOR_CFG_SN3_MCLK_2;
	}

	if (local_top_reg10.bit.SN4_MCLK == MUX_2) {
		value |= PIN_SENSOR_CFG_SN4_MCLK_2;
	}

	info->top_pinmux[PIN_FUNC_SENSOR].config = value;
	info->top_pinmux[PIN_FUNC_SENSOR].pin_function = PIN_FUNC_SENSOR;

	/* Parsing SENSOR2 */
	value = PIN_SENSOR2_CFG_NONE;

	if (local_top_reg9.bit.SENSOR2 == MUX_1) {
		value |= PIN_SENSOR2_CFG_12BITS;
	} else if (local_top_reg9.bit.SENSOR2 == MUX_2) {
		value |= PIN_SENSOR2_CFG_CCIR8BITS_A;
	} else if (local_top_reg9.bit.SENSOR2 == MUX_3) {
		value |= PIN_SENSOR2_CFG_CCIR8BITS_B;
	} else if (local_top_reg9.bit.SENSOR2 == MUX_4) {
		value |= PIN_SENSOR2_CFG_CCIR8BITS_AB;
	} else if (local_top_reg9.bit.SENSOR2 == MUX_5) {
		value |= PIN_SENSOR2_CFG_CCIR16BITS;
	}

	if (local_top_reg9.bit.SN2_CCIR_VSHS == MUX_1) {
		value |= PIN_SENSOR2_CFG_CCIR_VSHS;
	}

	if (local_top_reg10.bit.SN_MCLK == MUX_1) {
		value |= PIN_SENSOR2_CFG_SN1_MCLK_1;
	}

	if (local_top_reg10.bit.SN2_MCLK == MUX_1) {
		value |= PIN_SENSOR2_CFG_SN2_MCLK_1;
	}

	info->top_pinmux[PIN_FUNC_SENSOR2].config = value;
	info->top_pinmux[PIN_FUNC_SENSOR2].pin_function = PIN_FUNC_SENSOR2;

	/* Parsing SENSOR3 */
	value = PIN_SENSOR3_CFG_NONE;

	if (local_top_reg9.bit.SENSOR3 == MUX_1) {
		value |= PIN_SENSOR3_CFG_12BITS;
	} else if (local_top_reg9.bit.SENSOR3 == MUX_2) {
		value |= PIN_SENSOR3_CFG_CCIR8BITS_A;
	} else if (local_top_reg9.bit.SENSOR3 == MUX_3) {
		value |= PIN_SENSOR3_CFG_CCIR8BITS_B;
	} else if (local_top_reg9.bit.SENSOR3 == MUX_4) {
		value |= PIN_SENSOR3_CFG_CCIR8BITS_AB;
	} else if (local_top_reg9.bit.SENSOR3 == MUX_5) {
		value |= PIN_SENSOR3_CFG_CCIR16BITS;
	}

	if (local_top_reg9.bit.SN3_CCIR_VSHS == MUX_1) {
		value |= PIN_SENSOR3_CFG_CCIR_VSHS;
	}

	if (local_top_reg10.bit.SN5_MCLK == MUX_2) {
		value |= PIN_SENSOR3_CFG_SN5_MCLK_2;
	}

	info->top_pinmux[PIN_FUNC_SENSOR3].config = value;
	info->top_pinmux[PIN_FUNC_SENSOR3].pin_function = PIN_FUNC_SENSOR3;

	/* Parsing SENSORMISC */
	value = PIN_SENSORMISC_CFG_NONE;

	if (local_top_reg10.bit.SN_MCLK == MUX_1) {
		value |= PIN_SENSORMISC_CFG_SN_MCLK_1;
	}
	if (local_top_reg10.bit.SN2_MCLK == MUX_1) {
		value |= PIN_SENSORMISC_CFG_SN2_MCLK_1;
	}
	if (local_top_reg10.bit.SN3_MCLK == MUX_1) {
		value |= PIN_SENSORMISC_CFG_SN3_MCLK_1;
	} else if (local_top_reg10.bit.SN3_MCLK == MUX_2) {
		value |= PIN_SENSORMISC_CFG_SN3_MCLK_2;
	}
	if (local_top_reg10.bit.SN4_MCLK == MUX_1) {
		value |= PIN_SENSORMISC_CFG_SN4_MCLK_1;
	} else if (local_top_reg10.bit.SN4_MCLK == MUX_2) {
		value |= PIN_SENSORMISC_CFG_SN4_MCLK_2;
	}
	if (local_top_reg10.bit.SN5_MCLK == MUX_1) {
		value |= PIN_SENSORMISC_CFG_SN5_MCLK_1;
	} else if (local_top_reg10.bit.SN5_MCLK == MUX_2) {
		value |= PIN_SENSORMISC_CFG_SN5_MCLK_2;
	}

	if (local_top_reg10.bit.SN_XVSXHS == MUX_1) {
		value |= PIN_SENSORMISC_CFG_SN_XVSXHS_1;
	}
	if (local_top_reg10.bit.SN2_XVSXHS == MUX_1) {
		value |= PIN_SENSORMISC_CFG_SN2_XVSXHS_1;
	}
	if (local_top_reg10.bit.SN3_XVSXHS == MUX_1) {
		value |= PIN_SENSORMISC_CFG_SN3_XVSXHS_1;
	}
	if (local_top_reg10.bit.SN4_XVSXHS == MUX_1) {
		value |= PIN_SENSORMISC_CFG_SN4_XVSXHS_1;
	}
	if (local_top_reg10.bit.SN5_XVSXHS == MUX_1) {
		value |= PIN_SENSORMISC_CFG_SN5_XVSXHS_1;
	}

	if (local_top_reg9.bit.FLASH_TRIG_IN == MUX_1) {
		value |= PIN_SENSORMISC_CFG_FLASH_TRIG_IN_1;
	} else if (local_top_reg9.bit.FLASH_TRIG_IN == MUX_2) {
		value |= PIN_SENSORMISC_CFG_FLASH_TRIG_IN_2;
	}

	if (local_top_reg9.bit.FLASH_TRIG_OUT == MUX_1) {
		value |= PIN_SENSORMISC_CFG_FLASH_TRIG_OUT_1;
	} else if (local_top_reg9.bit.FLASH_TRIG_OUT == MUX_2) {
		value |= PIN_SENSORMISC_CFG_FLASH_TRIG_OUT_2;
	}

	info->top_pinmux[PIN_FUNC_SENSORMISC].config = value;
	info->top_pinmux[PIN_FUNC_SENSORMISC].pin_function = PIN_FUNC_SENSORMISC;

	/* Parsing SENSORSYNC */
	value = PIN_SENSORSYNC_CFG_NONE;

	if (local_top_reg11.bit.SN2_MCLK_SRC == MUX_1) {
		value |= PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SN;
	}

	if (local_top_reg11.bit.SN3_MCLK_SRC == MUX_1) {
		value |= PIN_SENSORSYNC_CFG_SN3_MCLKSRC_SN;
	} else if (local_top_reg11.bit.SN3_MCLK_SRC == MUX_2) {
		value |= PIN_SENSORSYNC_CFG_SN3_MCLKSRC_SN2;
	}

	if (local_top_reg11.bit.SN4_MCLK_SRC == MUX_1) {
		value |= PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN;
	} else if (local_top_reg11.bit.SN4_MCLK_SRC == MUX_2) {
		value |= PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN2;
	} else if (local_top_reg11.bit.SN4_MCLK_SRC == MUX_3) {
		value |= PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN3;
	}

	if (local_top_reg11.bit.SN5_MCLK_SRC == MUX_1) {
		value |= PIN_SENSORSYNC_CFG_SN5_MCLKSRC_SN;
	} else if (local_top_reg11.bit.SN5_MCLK_SRC == MUX_2) {
		value |= PIN_SENSORSYNC_CFG_SN5_MCLKSRC_SN2;
	} else if (local_top_reg11.bit.SN5_MCLK_SRC == MUX_3) {
		value |= PIN_SENSORSYNC_CFG_SN5_MCLKSRC_SN3;
	} else if (local_top_reg11.bit.SN5_MCLK_SRC == MUX_4) {
		value |= PIN_SENSORSYNC_CFG_SN5_MCLKSRC_SN4;
	}

	if (local_top_reg11.bit.SN2_XVSHS_SRC == MUX_1) {
		value |= PIN_SENSORSYNC_CFG_SN2_XVSXHSSRC_SN;
	}

	if (local_top_reg11.bit.SN3_XVSHS_SRC == MUX_1) {
		value |= PIN_SENSORSYNC_CFG_SN3_XVSXHSSRC_SN;
	} else if (local_top_reg11.bit.SN3_XVSHS_SRC == MUX_2) {
		value |= PIN_SENSORSYNC_CFG_SN3_XVSXHSSRC_SN2;
	}

	if (local_top_reg11.bit.SN4_XVSHS_SRC == MUX_1) {
		value |= PIN_SENSORSYNC_CFG_SN4_XVSXHSSRC_SN;
	} else if (local_top_reg11.bit.SN4_XVSHS_SRC == MUX_2) {
		value |= PIN_SENSORSYNC_CFG_SN4_XVSXHSSRC_SN2;
	} else if (local_top_reg11.bit.SN4_XVSHS_SRC == MUX_3) {
		value |= PIN_SENSORSYNC_CFG_SN4_XVSXHSSRC_SN3;
	}

	if (local_top_reg11.bit.SN5_XVSHS_SRC == MUX_1) {
		value |= PIN_SENSORSYNC_CFG_SN5_XVSXHSSRC_SN;
	} else if (local_top_reg11.bit.SN5_XVSHS_SRC == MUX_2) {
		value |= PIN_SENSORSYNC_CFG_SN5_XVSXHSSRC_SN2;
	} else if (local_top_reg11.bit.SN5_XVSHS_SRC == MUX_3) {
		value |= PIN_SENSORSYNC_CFG_SN5_XVSXHSSRC_SN3;
	} else if (local_top_reg11.bit.SN5_XVSHS_SRC == MUX_4) {
		value |= PIN_SENSORSYNC_CFG_SN5_XVSXHSSRC_SN4;
	}

	info->top_pinmux[PIN_FUNC_SENSORSYNC].config = value;
	info->top_pinmux[PIN_FUNC_SENSORSYNC].pin_function = PIN_FUNC_SENSORSYNC;

	/* Parsing MIPI_LVDS */
	value = PIN_MIPI_LVDS_CFG_NONE;

	if (local_top_reg9.bit.HSI2HSI3_SEL == MUX_0) {
		value |= PIN_MIPI_LVDS_CFG_HSI2HSI3_TO_CSI;
	}

	if (local_top_reg2.bit.MIPI_SEL == MUX_1) {
		value |= PIN_MIPI_LVDS_CFG_MIPI_SEL;
	}

	info->top_pinmux[PIN_FUNC_MIPI_LVDS].config = value;
	info->top_pinmux[PIN_FUNC_MIPI_LVDS].pin_function = PIN_FUNC_MIPI_LVDS;

	/* Parsing AUDIO */
	value = PIN_AUDIO_CFG_NONE;

	if (local_top_reg12.bit.I2S == MUX_1) {
		value |= PIN_AUDIO_CFG_I2S_1;
	} else if (local_top_reg12.bit.I2S == MUX_2) {
		value |= PIN_AUDIO_CFG_I2S_2;
	}
	if (local_top_reg12.bit.I2S_MCLK == MUX_1) {
		value |= PIN_AUDIO_CFG_I2S_MCLK_1;
	} else if (local_top_reg12.bit.I2S_MCLK == MUX_2) {
		value |= PIN_AUDIO_CFG_I2S_MCLK_2;
	}

	if (local_top_reg12.bit.I2S2 == MUX_1) {
		value |= PIN_AUDIO_CFG_I2S2_1;
	}
	if (local_top_reg12.bit.I2S2_MCLK == MUX_1) {
		value |= PIN_AUDIO_CFG_I2S2_MCLK_1;
	}

	if (local_top_reg12.bit.DMIC == MUX_1) {
		value |= PIN_AUDIO_CFG_DMIC_1;
	}
	if (local_top_reg12.bit.DMIC_DATA0 == MUX_1) {
		value |= PIN_AUDIO_CFG_DMIC_DATA0;
	}
	if (local_top_reg12.bit.DMIC_DATA1 == MUX_1) {
		value |= PIN_AUDIO_CFG_DMIC_DATA1;
	}

	if (local_top_reg12.bit.EXT_EAC_MCLK == MUX_1) {
		value |= PIN_AUDIO_CFG_EXT_EAC_MCLK;
	}

	info->top_pinmux[PIN_FUNC_AUDIO].config = value;
	info->top_pinmux[PIN_FUNC_AUDIO].pin_function = PIN_FUNC_AUDIO;

	/* Parsing UART */
	value = PIN_UART_CFG_NONE;

	if (local_top_reg13.bit.UART == MUX_1) {
		value |= PIN_UART_CFG_UART_1;
	}

	if (local_top_reg13.bit.UART2 == MUX_1) {
		value |= PIN_UART_CFG_UART2_1;
	}
	if (local_top_reg14.bit.UART2_RTSCTS == MUX_1) {
		value |= PIN_UART_CFG_UART2_RTSCTS;
	} else if (local_top_reg14.bit.UART2_RTSCTS == MUX_2) {
		value |= PIN_UART_CFG_UART2_DIROE;
	}

	if (local_top_reg13.bit.UART3 == MUX_1) {
		value |= PIN_UART_CFG_UART3_1;
	}
	if (local_top_reg14.bit.UART3_RTSCTS == MUX_1) {
		value |= PIN_UART_CFG_UART3_RTSCTS;
	} else if (local_top_reg14.bit.UART3_RTSCTS == MUX_2) {
		value |= PIN_UART_CFG_UART3_DIROE;
	}

	if (local_top_reg13.bit.UART4 == MUX_1) {
		value |= PIN_UART_CFG_UART4_1;
	} else if (local_top_reg13.bit.UART4 == MUX_2) {
		value |= PIN_UART_CFG_UART4_2;
	}
	if (local_top_reg14.bit.UART4_RTSCTS == MUX_1) {
		value |= PIN_UART_CFG_UART4_RTSCTS;
	} else if (local_top_reg14.bit.UART4_RTSCTS == MUX_2) {
		value |= PIN_UART_CFG_UART4_DIROE;
	}

	if (local_top_reg13.bit.UART5 == MUX_1) {
		value |= PIN_UART_CFG_UART5_1;
	} else if (local_top_reg13.bit.UART5 == MUX_2) {
		value |= PIN_UART_CFG_UART5_2;
	}
	if (local_top_reg14.bit.UART5_RTSCTS == MUX_1) {
		value |= PIN_UART_CFG_UART5_RTSCTS;
	} else if (local_top_reg14.bit.UART5_RTSCTS == MUX_2) {
		value |= PIN_UART_CFG_UART5_DIROE;
	}

	info->top_pinmux[PIN_FUNC_UART].config = value;
	info->top_pinmux[PIN_FUNC_UART].pin_function = PIN_FUNC_UART;

	/* Parsing UARTII */
	value = PIN_UARTII_CFG_NONE;

	if (local_top_reg13.bit.UART6 == MUX_1) {
		value |= PIN_UARTII_CFG_UART6_1;
	} else if (local_top_reg13.bit.UART6 == MUX_2) {
		value |= PIN_UARTII_CFG_UART6_2;
	}
	if (local_top_reg14.bit.UART6_RTSCTS == MUX_1) {
		value |= PIN_UARTII_CFG_UART6_RTSCTS;
	} else if (local_top_reg14.bit.UART6_RTSCTS == MUX_2) {
		value |= PIN_UARTII_CFG_UART6_DIROE;
	}

	if (local_top_reg13.bit.UART7 == MUX_1) {
		value |= PIN_UARTII_CFG_UART7_1;
	} else if (local_top_reg13.bit.UART7 == MUX_2) {
		value |= PIN_UARTII_CFG_UART7_2;
	}
	if (local_top_reg14.bit.UART7_RTSCTS == MUX_1) {
		value |= PIN_UARTII_CFG_UART7_RTSCTS;
	} else if (local_top_reg14.bit.UART7_RTSCTS == MUX_2) {
		value |= PIN_UARTII_CFG_UART7_DIROE;
	}

	if (local_top_reg13.bit.UART8 == MUX_1) {
		value |= PIN_UARTII_CFG_UART8_1;
	} else if (local_top_reg13.bit.UART8 == MUX_2) {
		value |= PIN_UARTII_CFG_UART8_2;
	}
	if (local_top_reg14.bit.UART8_RTSCTS == MUX_1) {
		value |= PIN_UARTII_CFG_UART8_RTSCTS;
	} else if (local_top_reg14.bit.UART8_RTSCTS == MUX_2) {
		value |= PIN_UARTII_CFG_UART8_DIROE;
	}

	if (local_top_reg14.bit.UART9 == MUX_1) {
		value |= PIN_UARTII_CFG_UART9_1;
	} else if (local_top_reg14.bit.UART9 == MUX_2) {
		value |= PIN_UARTII_CFG_UART9_2;
	}
	if (local_top_reg14.bit.UART9_RTSCTS == MUX_1) {
		value |= PIN_UARTII_CFG_UART9_RTSCTS;
	} else if (local_top_reg14.bit.UART9_RTSCTS == MUX_2) {
		value |= PIN_UARTII_CFG_UART9_DIROE;
	}

	info->top_pinmux[PIN_FUNC_UARTII].config = value;
	info->top_pinmux[PIN_FUNC_UARTII].pin_function = PIN_FUNC_UARTII;

	/* Parsing REMOTE */
	value = PIN_REMOTE_CFG_NONE;

	if (local_top_reg15.bit.REMOTE == MUX_1) {
		value |= PIN_REMOTE_CFG_REMOTE_1;
	}

	if (local_top_reg15.bit.REMOTE_EXT == MUX_1) {
		value |= PIN_REMOTE_CFG_REMOTE_EXT_1;
	}

	info->top_pinmux[PIN_FUNC_REMOTE].config = value;
	info->top_pinmux[PIN_FUNC_REMOTE].pin_function = PIN_FUNC_REMOTE;

	/* Parsing SDP */
	value = PIN_SDP_CFG_NONE;

	if (local_top_reg15.bit.SDP == MUX_1) {
		value |= PIN_SDP_CFG_SDP_1;
	} else if (local_top_reg15.bit.SDP == MUX_2) {
		value |= PIN_SDP_CFG_SDP_2;
	}

	info->top_pinmux[PIN_FUNC_SDP].config = value;
	info->top_pinmux[PIN_FUNC_SDP].pin_function = PIN_FUNC_SDP;

	/* Parsing SPI */
	value = PIN_SPI_CFG_NONE;

	if (local_top_reg16.bit.SPI == MUX_1) {
		value |= PIN_SPI_CFG_SPI_1;
	} else if (local_top_reg16.bit.SPI == MUX_2) {
		value |= PIN_SPI_CFG_SPI_2;
	} else if (local_top_reg16.bit.SPI == MUX_3) {
		value |= PIN_SPI_CFG_SPI_3;
	}
	if (local_top_reg16.bit.SPI_BUS_WIDTH == MUX_1) {
		value |= PIN_SPI_CFG_SPI_BUS_WIDTH;
	}

	if (local_top_reg16.bit.SPI2 == MUX_1) {
		value |= PIN_SPI_CFG_SPI2_1;
	} else if (local_top_reg16.bit.SPI2 == MUX_2) {
		value |= PIN_SPI_CFG_SPI2_2;
	}
	if (local_top_reg16.bit.SPI2_BUS_WIDTH == MUX_1) {
		value |= PIN_SPI_CFG_SPI2_BUS_WIDTH;
	}

	if (local_top_reg16.bit.SPI3 == MUX_1) {
		value |= PIN_SPI_CFG_SPI3_1;
	} else if (local_top_reg16.bit.SPI3 == MUX_2) {
		value |= PIN_SPI_CFG_SPI3_2;
	}
	if (local_top_reg16.bit.SPI3_BUS_WIDTH == MUX_1) {
		value |= PIN_SPI_CFG_SPI3_BUS_WIDTH;
	}

	if (local_top_reg16.bit.SPI3_RDY == MUX_1) {
		value |= PIN_SPI_CFG_SPI3_RDY_1;
	} else if (local_top_reg16.bit.SPI3_RDY == MUX_2) {
		value |= PIN_SPI_CFG_SPI3_RDY_2;
	}

	if (local_top_reg16.bit.SPI4 == MUX_1) {
		value |= PIN_SPI_CFG_SPI4_1;
	} else if (local_top_reg16.bit.SPI4 == MUX_2) {
		value |= PIN_SPI_CFG_SPI4_2;
	}
	if (local_top_reg16.bit.SPI4_BUS_WIDTH == MUX_1) {
		value |= PIN_SPI_CFG_SPI4_BUS_WIDTH;
	}

	if (local_top_reg16.bit.SPI5 == MUX_1) {
		value |= PIN_SPI_CFG_SPI5_1;
	} else if (local_top_reg16.bit.SPI5 == MUX_2) {
		value |= PIN_SPI_CFG_SPI5_2;
	}
	if (local_top_reg16.bit.SPI5_BUS_WIDTH == MUX_1) {
		value |= PIN_SPI_CFG_SPI5_BUS_WIDTH;
	}

	info->top_pinmux[PIN_FUNC_SPI].config = value;
	info->top_pinmux[PIN_FUNC_SPI].pin_function = PIN_FUNC_SPI;

	/* Parsing SIF */
	value = PIN_SIF_CFG_NONE;

	if (local_top_reg17.bit.SIF0 == MUX_1) {
		value |= PIN_SIF_CFG_SIF0_1;
	} else if (local_top_reg17.bit.SIF0 == MUX_2) {
		value |= PIN_SIF_CFG_SIF0_2;
	} else if (local_top_reg17.bit.SIF0 == MUX_3) {
		value |= PIN_SIF_CFG_SIF0_3;
	}

	if (local_top_reg17.bit.SIF1 == MUX_1) {
		value |= PIN_SIF_CFG_SIF1_1;
	} else if (local_top_reg17.bit.SIF1 == MUX_2) {
		value |= PIN_SIF_CFG_SIF1_2;
	}

	if (local_top_reg17.bit.SIF2 == MUX_1) {
		value |= PIN_SIF_CFG_SIF2_1;
	} else if (local_top_reg17.bit.SIF2 == MUX_2) {
		value |= PIN_SIF_CFG_SIF2_2;
	}

	if (local_top_reg17.bit.SIF3 == MUX_1) {
		value |= PIN_SIF_CFG_SIF3_1;
	} else if (local_top_reg17.bit.SIF3 == MUX_2) {
		value |= PIN_SIF_CFG_SIF3_2;
	}

	if (local_top_reg17.bit.SIF4 == MUX_1) {
		value |= PIN_SIF_CFG_SIF4_1;
	} else if (local_top_reg17.bit.SIF4 == MUX_2) {
		value |= PIN_SIF_CFG_SIF4_2;
	}

	if (local_top_reg17.bit.SIF5 == MUX_1) {
		value |= PIN_SIF_CFG_SIF5_1;
	} else if (local_top_reg17.bit.SIF5 == MUX_2) {
		value |= PIN_SIF_CFG_SIF5_2;
	}

	info->top_pinmux[PIN_FUNC_SIF].config = value;
	info->top_pinmux[PIN_FUNC_SIF].pin_function = PIN_FUNC_SIF;

	/* Parsing MISC */
	value = PIN_MISC_CFG_NONE;

	if (local_top_reg18.bit.RTC_CLK == MUX_1) {
		value |= PIN_MISC_CFG_RTC_CLK_1;
	}

	if (local_top_reg18.bit.SP_CLK == MUX_1) {
		value |= PIN_MISC_CFG_SP_CLK_1;
	}

	if (local_top_reg18.bit.SP_CLK2 == MUX_1) {
		value |= PIN_MISC_CFG_SP2_CLK_1;
	}

	if (local_top_reg19.bit.SATA_LED == MUX_1) {
		value |= PIN_MISC_CFG_SATA_LED_1;
	}

	info->top_pinmux[PIN_FUNC_MISC].config = value;
	info->top_pinmux[PIN_FUNC_MISC].pin_function = PIN_FUNC_MISC;

	/* Parsing LCD */
	info->top_pinmux[PIN_FUNC_LCD].config = disp_pinmux_config[PINMUX_FUNC_ID_LCD];
	info->top_pinmux[PIN_FUNC_LCD].pin_function = PIN_FUNC_LCD;

	/* Parsing LCD2 */
	info->top_pinmux[PIN_FUNC_LCD2].config = disp_pinmux_config[PINMUX_FUNC_ID_LCD2];
	info->top_pinmux[PIN_FUNC_LCD2].pin_function = PIN_FUNC_LCD2;

	/* Parsing TV */
	info->top_pinmux[PIN_FUNC_TV].config = disp_pinmux_config[PINMUX_FUNC_ID_TV];
	info->top_pinmux[PIN_FUNC_TV].pin_function = PIN_FUNC_TV;

	/* Parsing SEL_LCD */
	value = PINMUX_LCD_SEL_GPIO;

	if (local_top_reg2.bit.LCD_TYPE == MUX_1) {
		if (local_top_reg2.bit.CCIR_DATA_WIDTH == MUX_1) {
			value = PINMUX_LCD_SEL_CCIR656_16BITS;
		} else {
			value = PINMUX_LCD_SEL_CCIR656;
		}
	} else if (local_top_reg2.bit.LCD_TYPE == MUX_2) {
		if (local_top_reg2.bit.CCIR_DATA_WIDTH == MUX_1) {
			value = PINMUX_LCD_SEL_CCIR601_16BITS;

			if (local_top_reg2.bit.CCIR_FIELD == MUX_1) {
				value |= PINMUX_LCD_SEL_FIELD;
			}
		} else {
			value = PINMUX_LCD_SEL_CCIR601;

			if (local_top_reg2.bit.CCIR_HVLD_VVLD == MUX_1) {
				value |= PINMUX_LCD_SEL_HVLD_VVLD;
			}

			if (local_top_reg2.bit.CCIR_FIELD == MUX_1) {
				value |= PINMUX_LCD_SEL_FIELD;
			}
		}
	} else if (local_top_reg2.bit.LCD_TYPE == MUX_3) {
		value = PINMUX_LCD_SEL_PARALLE_RGB565;
	} else if (local_top_reg2.bit.LCD_TYPE == MUX_4) {
		value = PINMUX_LCD_SEL_SERIAL_RGB_8BITS;
	} else if (local_top_reg2.bit.LCD_TYPE == MUX_5) {
		value = PINMUX_LCD_SEL_SERIAL_RGB_6BITS;
	} else if (local_top_reg2.bit.LCD_TYPE == MUX_6) {
		value = PINMUX_LCD_SEL_SERIAL_YCbCr_8BITS;
	} else if (local_top_reg2.bit.LCD_TYPE == MUX_7) {
		value = PINMUX_LCD_SEL_RGB_16BITS;
	} else if (local_top_reg2.bit.LCD_TYPE == MUX_8) {
		value = PINMUX_LCD_SEL_PARALLE_RGB666;
	} else if (local_top_reg2.bit.LCD_TYPE == MUX_9) {
		value = PINMUX_LCD_SEL_MIPI;
	} else if (local_top_reg2.bit.LCD_TYPE == MUX_10) {
		value = PINMUX_LCD_SEL_PARALLE_RGB888;
	} else if (local_top_reg2.bit.MEMIF_SEL == MUX_0) {
		if (local_top_reg2.bit.MEMIF_TYPE == MUX_1) {
			if (local_top_reg2.bit.SMEMIF_DATA_WIDTH == MUX_0) {
				value = PINMUX_LCD_SEL_SERIAL_MI_SDIO;
			} else if (local_top_reg2.bit.SMEMIF_DATA_WIDTH == MUX_1) {
				value = PINMUX_LCD_SEL_SERIAL_MI_SDI_SDO;
			}
		} else if (local_top_reg2.bit.MEMIF_TYPE == MUX_2) {
			if (local_top_reg2.bit.PMEMIF_DATA_WIDTH == MUX_0) {
				value = PINMUX_LCD_SEL_PARALLE_MI_8BITS;
			} else if (local_top_reg2.bit.PMEMIF_DATA_WIDTH == MUX_1) {
				value = PINMUX_LCD_SEL_PARALLE_MI_9BITS;
			} else if (local_top_reg2.bit.PMEMIF_DATA_WIDTH == MUX_2) {
				value = PINMUX_LCD_SEL_PARALLE_MI_16BITS;
			} else if (local_top_reg2.bit.PMEMIF_DATA_WIDTH == MUX_3) {
				value = PINMUX_LCD_SEL_PARALLE_MI_18BITS;
			}
		}
	}

	if (local_top_reg2.bit.TE_SEL == MUX_1) {
		value |= PINMUX_LCD_SEL_TE_ENABLE;
	}

	if (local_top_reg2.bit.PLCD_DE == MUX_1) {
		value |= PINMUX_LCD_SEL_DE_ENABLE;
	}

	info->top_pinmux[PIN_FUNC_SEL_LCD].config = value;
	info->top_pinmux[PIN_FUNC_SEL_LCD].pin_function = PIN_FUNC_SEL_LCD;

	/* Parsing SEL_LCD2 */
	value = PINMUX_LCD_SEL_GPIO;

	if (local_top_reg2.bit.LCD2_TYPE == MUX_1) {
		value = PINMUX_LCD_SEL_CCIR656;
	} else if (local_top_reg2.bit.LCD2_TYPE == MUX_2) {
		value = PINMUX_LCD_SEL_CCIR601;
	} else if (local_top_reg2.bit.LCD2_TYPE == MUX_4) {
		value |= PINMUX_LCD_SEL_SERIAL_RGB_8BITS;
	} else if (local_top_reg2.bit.LCD2_TYPE == MUX_5) {
		value = PINMUX_LCD_SEL_SERIAL_RGB_6BITS;
	} else if (local_top_reg2.bit.LCD2_TYPE == MUX_6) {
		value = PINMUX_LCD_SEL_SERIAL_YCbCr_8BITS;
	} else if (local_top_reg2.bit.LCD2_TYPE == MUX_9) {
		value = PINMUX_LCD_SEL_MIPI;
	} else if (local_top_reg2.bit.MEMIF_SEL == MUX_1) {
		if (local_top_reg2.bit.MEMIF_TYPE == MUX_1) {
			if (local_top_reg2.bit.SMEMIF_DATA_WIDTH == MUX_0) {
				value = PINMUX_LCD_SEL_SERIAL_MI_SDIO;
			} else if (local_top_reg2.bit.SMEMIF_DATA_WIDTH == MUX_1) {
				value = PINMUX_LCD_SEL_SERIAL_MI_SDI_SDO;
			}
		} else if (local_top_reg2.bit.MEMIF_TYPE == MUX_2) {
			if (local_top_reg2.bit.PMEMIF_DATA_WIDTH == MUX_0) {
				value = PINMUX_LCD_SEL_PARALLE_MI_8BITS;
			} else if (local_top_reg2.bit.PMEMIF_DATA_WIDTH == MUX_1) {
				value = PINMUX_LCD_SEL_PARALLE_MI_9BITS;
			}
		}
	}

	if (local_top_reg2.bit.TE_SEL == MUX_1) {
		value |= PINMUX_LCD_SEL_TE_ENABLE;
	}

	if (local_top_reg2.bit.PLCD2_DE == MUX_1) {
		value |= PINMUX_LCD_SEL_DE_ENABLE;
	}

	info->top_pinmux[PIN_FUNC_SEL_LCD2].config = value;
	info->top_pinmux[PIN_FUNC_SEL_LCD2].pin_function = PIN_FUNC_SEL_LCD2;

	/* Leave critical section */
	//unl_cpu(flags);
}

void gpio_func_keep(int start,int count,int func)
{
    int i=0;

    for(i=start;i<start+count;i++)
    {
        dump_gpio_func[i]=func;
    }
}

/*int gpio_conflict_detect(int start,int count,int func)
{
    int i=0;
    int confl_mod;
    int confl_flag=0;
    for(i=start;i<start+count;i++)
    {
        confl_mod=dump_gpio_func[i];
        //printf("%d\r\n",confl_mod);

        if(confl_mod>0)
        {
            printf("%s conflict with %s\r\n",dump_func[func-1],dump_func[confl_mod-1]);
            confl_flag++;
            break;
        }
    }
    return confl_flag;

}*/



/*----------------------------------------------*/
/*          PINMUX Interface Functions          */
/*----------------------------------------------*/
static int pinmux_config_sdio(uint32_t config)
{
	if (config == PIN_SDIO_CFG_NONE) {
	} else {
		if (config & PIN_SDIO_CFG_SDIO_1) {
			if (top_reg12.bit.EXT_EAC_MCLK == MUX_1) {
				pr_err("SDIO_1 conflict with EXT_EAC_MCLK\r\n");
				return E_PAR;
			}

			top_reg1.bit.SDIO_EN = MUX_1;
			top_reg_cgpio0.bit.CGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_16 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(CGPIO_11,6,func_SDIO);
		}

		if (config & PIN_SDIO_CFG_SDIO2_1) {
			if (top_reg13.bit.UART8 == MUX_2) {
				pr_err("SDIO2_1 conflict with UART8_2\r\n");
				return E_PAR;
			}
			if (top_reg1.bit.EXTROM_EN == MUX_1) {
				pr_err("SDIO2_1 conflict with EXTROM\r\n");
				return E_PAR;
			}
			if (top_reg12.bit.I2S == MUX_2) {
				pr_err("SDIO2_1 conflict with I2S_2\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI3 == MUX_2) {
				pr_err("SDIO2_1 conflict with SPI3_2\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI3_RDY == MUX_2) {
				pr_err("SDIO2_1 conflict with SPI3_RDY_2\r\n");
				return E_PAR;
			}
			if (top_reg8.bit.DSP_EJTAG_EN == MUX_1) {
				pr_err("SDIO2_1 conflict with DSP_EJTAG_EN\r\n");
				return E_PAR;
			}
			if (top_reg15.bit.SDP == MUX_2) {
				pr_err("SDIO2_1 conflict with SDP_2\r\n");
				return E_PAR;
			}

			top_reg1.bit.SDIO2_EN = MUX_1;
			top_reg_cgpio0.bit.CGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_22 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(CGPIO_17,6,func_SDIO2);
		}

		if (config & PIN_SDIO_CFG_SDIO3_1) {
			if (top_reg1.bit.FSPI_EN == MUX_1) {
				pr_err("SDIO3_1 conflict with NAND_1\r\n");
				return E_PAR;
			}
			if ((top_reg0.bit.EJTAG_SEL == MUX_1) && (top_reg0.bit.EJTAG_CH_SEL == MUX_1)) {
				pr_err("SDIO3_1 conflict with EJTAG_2\r\n");
				return E_PAR;
			}

			top_reg1.bit.SDIO3_EN = MUX_1;
			top_reg_cgpio0.bit.CGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_9 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(CGPIO_0,4,func_SDIO3);
            gpio_func_keep(CGPIO_8,2,func_SDIO3);
			if (config & PIN_SDIO_CFG_SDIO3_BUS_WIDTH) {
				if (top_reg4.bit.I2C5 == MUX_3) {
					pr_err("SDIO3_BUS_WIDTH conflict with I2C5_3\r\n");
					return E_PAR;
				}
				if (top_reg4.bit.I2C6 == MUX_3) {
					pr_err("SDIO3_BUS_WIDTH conflict with I2C6_3\r\n");
					return E_PAR;
				}
				if (top_reg7.bit.PWM8 == MUX_2) {
					pr_err("SDIO3_BUS_WIDTH conflict with PWM8_2\r\n");
					return E_PAR;
				}
				if (top_reg7.bit.PWM9 == MUX_2) {
					pr_err("SDIO3_BUS_WIDTH conflict with PWM9_2\r\n");
					return E_PAR;
				}
				if (top_reg16.bit.SPI == MUX_3) {
					pr_err("SDIO3_BUS_WIDTH conflict with SPI_3\r\n");
					return E_PAR;
				}
				if (top_reg13.bit.UART7 == MUX_2) {
					pr_err("SDIO3_BUS_WIDTH conflict with UART7_2\r\n");
					return E_PAR;
				}
				if (top_reg17.bit.SIF5 == MUX_2) {
					pr_err("SDIO3_BUS_WIDTH conflict with SIF5_2\r\n");
					return E_PAR;
				}

				top_reg1.bit.SDIO3_BUS_WIDTH = MUX_1;
				top_reg_cgpio0.bit.CGPIO_4 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_5 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_6 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_7 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(CGPIO_4,4,func_SDIO3);
			}

			if (config & PIN_SDIO_CFG_SDIO3_DS) {
				if (top_reg1.bit.FSPI_CS1_EN == MUX_1) {
					pr_err("SDIO3_BUS_WIDTH conflict with NAND_CS1\r\n");
					return E_PAR;
				}

				top_reg1.bit.SDIO3_DS_EN = MUX_1;
				top_reg_cgpio0.bit.CGPIO_10 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(CGPIO_10,1,func_SDIO3);
			}
		}
	}

	return E_OK;
}

static int pinmux_config_nand(uint32_t config)
{
	if (config == PIN_NAND_CFG_NONE) {
	} else {
		if (config & PIN_NAND_CFG_NAND_1) {
			if (top_reg1.bit.SDIO3_EN == MUX_1) {
				pr_err("NAND_1 conflict with SDIO3_1, 0x%x\r\n", top_reg1.reg);
				return E_PAR;
			}
			if ((top_reg0.bit.EJTAG_SEL == MUX_1) && (top_reg0.bit.EJTAG_CH_SEL == MUX_1)) {
				pr_err("NAND_1 conflict with EJTAG_2\r\n");
				return E_PAR;
			}

			top_reg1.bit.FSPI_EN = MUX_1;
			top_reg_cgpio0.bit.CGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_9 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(CGPIO_0,4,func_FSPI);
            gpio_func_keep(CGPIO_8,2,func_FSPI);
			if (config & PIN_NAND_CFG_NAND_CS1) {
				if (top_reg1.bit.SDIO3_DS_EN == MUX_1) {
					pr_err("NAND_CS1 conflict with SDIO3_DS_EN\r\n");
					return E_PAR;
				}

				top_reg1.bit.FSPI_CS1_EN = MUX_1;
				top_reg_cgpio0.bit.CGPIO_10 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(CGPIO_10,1,func_FSPI);
			}
		}
	}

	return E_OK;
}

static int pinmux_config_eth(uint32_t config)
{
	if (config == PIN_ETH_CFG_NONE) {
	} else {
		if (config & PIN_ETH_CFG_ETH_RGMII_1) {
			if (top_reg12.bit.I2S2 == MUX_1) {
				pr_err("ETH_RGMII_1 conflict with I2S2_1\r\n");
				return E_PAR;
			}
			if (top_reg2.bit.LCD_TYPE == MUX_3 || top_reg2.bit.LCD_TYPE == MUX_7 ||
				top_reg2.bit.LCD_TYPE == MUX_8 || top_reg2.bit.LCD_TYPE == MUX_10) {
				pr_err("ETH_RGMII_1 conflict with LCD_TYPE\r\n");
				return E_PAR;
			}
			if (top_reg2.bit.LCD2_TYPE == MUX_1 || top_reg2.bit.LCD2_TYPE == MUX_2 ||
				top_reg2.bit.LCD2_TYPE == MUX_4 || top_reg2.bit.LCD2_TYPE == MUX_5 || top_reg2.bit.LCD2_TYPE == MUX_6) {
				pr_err("ETH_RGMII_1 conflict with LCD2_TYPE\r\n");
				return E_PAR;
			}
			if (top_reg2.bit.MEMIF_TYPE == MUX_2) {
				if ((top_reg2.bit.MEMIF_SEL == MUX_0) &&
					(top_reg2.bit.PMEMIF_DATA_WIDTH == MUX_2 || top_reg2.bit.PMEMIF_DATA_WIDTH == MUX_3)) {
					pr_err("ETH_RGMII_1 conflict with (MEMIF_TYPE = 2 && MEMIF_SEL = 0 && PMEMIF_DATA_WIDTH = 0x2 or 0x3)\r\n");
					return E_PAR;
				} else if (top_reg2.bit.MEMIF_SEL == MUX_1) {
					pr_err("ETH_RGMII_1 conflict with (MEMIF_TYPE = 2 && MEMIF_SEL = 1)\r\n");
					return E_PAR;
				}
			}
			if (top_reg14.bit.UART9 == MUX_2) {
				pr_err("ETH_RGMII_1 conflict with UART9_2\r\n");
				return E_PAR;
			}
			if (top_reg6.bit.PWM7 == MUX_2) {
				pr_err("ETH_RGMII_1 conflict with PWM7_2\r\n");
				return E_PAR;
			}
			if (top_reg18.bit.SP_CLK == MUX_1) {
				pr_err("ETH_RGMII_1 conflict with SP_CLK\r\n");
				return E_PAR;
			}
			if ((top_reg16.bit.SPI4 == MUX_2) && (top_reg16.bit.SPI4_BUS_WIDTH == MUX_1)) {
				pr_err("ETH_RGMII_1 conflict with SPI4_2 BUS_WIDTH\r\n");
				return E_PAR;
			}

			top_reg3.bit.ETH = MUX_1;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_24 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_25 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_26 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_27 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_28 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_29 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_30 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_6 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_19,12,func_ETH);
            gpio_func_keep(DGPIO_5,2,func_ETH);
		} else if (config & PIN_ETH_CFG_ETH_RMII_1) {
			if (top_reg12.bit.I2S2 == MUX_1) {
				pr_err("ETH_RMII_1 conflict with I2S2_1\r\n");
				return E_PAR;
			}
			if (top_reg2.bit.LCD_TYPE == MUX_3 || top_reg2.bit.LCD_TYPE == MUX_7 ||
				top_reg2.bit.LCD_TYPE == MUX_8 || top_reg2.bit.LCD_TYPE == MUX_10) {
				pr_err("ETH_RMII_1 conflict with LCD_TYPE\r\n");
				return E_PAR;
			}
			if (top_reg2.bit.LCD2_TYPE == MUX_1 || top_reg2.bit.LCD2_TYPE == MUX_2 ||
				top_reg2.bit.LCD2_TYPE == MUX_4 || top_reg2.bit.LCD2_TYPE == MUX_5 || top_reg2.bit.LCD2_TYPE == MUX_6) {
				pr_err("ETH_RMII_1 conflict with LCD2_TYPE\r\n");
				return E_PAR;
			}
			if (top_reg2.bit.MEMIF_TYPE == MUX_2) {
				if ((top_reg2.bit.MEMIF_SEL == MUX_0) &&
				    (top_reg2.bit.PMEMIF_DATA_WIDTH == MUX_2 || top_reg2.bit.PMEMIF_DATA_WIDTH == MUX_3)) {
					pr_err("ETH_RMII_1 conflict with (MEMIF_TYPE = 2 && MEMIF_SEL = 0 && PMEMIF_DATA_WIDTH = 0x2 or 0x3)\r\n");
					return E_PAR;
				} else if (top_reg2.bit.MEMIF_SEL == MUX_1) {
					pr_err("ETH_RMII_1 conflict with (MEMIF_TYPE = 2 && MEMIF_SEL = 1)\r\n");
					return E_PAR;
				}
			}
			if (top_reg14.bit.UART9 == MUX_2) {
				pr_err("ETH_RMII_1 conflict with UART9_2\r\n");
				return E_PAR;
			}
			if (top_reg6.bit.PWM7 == MUX_2) {
				pr_err("ETH_RMII_1 conflict with PWM7_2\r\n");
				return E_PAR;
			}
			if (top_reg18.bit.SP_CLK == MUX_1) {
				pr_err("ETH_RMII_1 conflict with SP_CLK\r\n");
				return E_PAR;
			}
			if ((top_reg16.bit.SPI4 == MUX_2) && (top_reg16.bit.SPI4_BUS_WIDTH == MUX_1)) {
				pr_err("ETH_RMII_1 conflict with SPI4_2 BUS_WIDTH\r\n");
				return E_PAR;
			}

			top_reg3.bit.ETH = MUX_2;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_25 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_26 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_27 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_28 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_6 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_19,3,func_ETH);
            gpio_func_keep(LGPIO_25,4,func_ETH);
            gpio_func_keep(DGPIO_5,2,func_ETH);
		}

		if (config & PIN_ETH_CFG_ETH_EXTPHYCLK) {
			if ((top_reg14.bit.UART9 == MUX_2) &&
				(top_reg14.bit.UART9_RTSCTS == MUX_1 || top_reg14.bit.UART9_RTSCTS == MUX_2)) {
				pr_err("ETH_EXTPHYCLK conflict with UART9_2 RTS\r\n");
				return E_PAR;
			}

			top_reg3.bit.ETH_EXT_PHY_CLK = MUX_1;
			top_reg_dgpio0.bit.DGPIO_7 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(DGPIO_7,1,func_ETH);
		}

		if (config & PIN_ETH_CFG_ETH_PTP) {
			if ((top_reg13.bit.UART2 == MUX_2) &&
			    (top_reg14.bit.UART2_RTSCTS == MUX_1 || top_reg14.bit.UART2_RTSCTS == MUX_2)) {
				pr_err("ETH_PTP conflict with UART2_2 RTS\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C == MUX_1) {
				pr_err("ETH_PTP conflict with I2C_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI5 == MUX_1) {
				pr_err("ETH_PTP conflict with SPI5_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF4 == MUX_1) {
				pr_err("ETH_PTP conflict with SIF4_1\r\n");
				return E_PAR;
			}
			if (top_reg12.bit.I2S_MCLK == MUX_1) {
				pr_err("ETH_PTP conflict with I2S_MCLK_1\r\n");
				return E_PAR;
			}

			top_reg3.bit.ETH_PTP = MUX_1;
			top_reg_pgpio0.bit.PGPIO_28 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_28,1,func_ETH);
		}

		if (config & PIN_ETH_CFG_ETH2_RGMII_1) {
			if (top_reg2.bit.LCD_TYPE) {
				if (top_reg2.bit.LCD_TYPE != MUX_9) {
					pr_err("ETH2_RGMII_1 conflict with LCD_TYPE\r\n");
					return E_PAR;
				}
			}
			if (top_reg2.bit.LCD2_TYPE) {
				if (top_reg2.bit.LCD2_TYPE != MUX_9) {
					pr_err("ETH2_RGMII_1 conflict with LCD2_TYPE\r\n");
					return E_PAR;
				}
			}
			if (top_reg2.bit.MEMIF_TYPE) {
				pr_err("ETH2_RGMII_1 conflict with MEMIF_TYPE\r\n");
				return E_PAR;
			}

			top_reg3.bit.ETH2 = MUX_1;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_3,14,func_ETH2);
		} else if (config & PIN_ETH_CFG_ETH2_RMII_1) {
			if (top_reg2.bit.LCD_TYPE) {
				if (top_reg2.bit.LCD_TYPE != MUX_9) {
					pr_err("ETH2_RGMII_1 conflict with LCD_TYPE\r\n");
					return E_PAR;
				}
			}
			if (top_reg2.bit.LCD2_TYPE) {
				if (top_reg2.bit.LCD2_TYPE != MUX_9) {
					pr_err("ETH2_RGMII_1 conflict with LCD2_TYPE\r\n");
					return E_PAR;
				}
			}
			if (top_reg2.bit.MEMIF_TYPE) {
				pr_err("ETH2_RGMII_1 conflict with MEMIF_TYPE\r\n");
				return E_PAR;
			}

			top_reg3.bit.ETH2 = MUX_2;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_3,3,func_ETH2);
            gpio_func_keep(LGPIO_9,4,func_ETH2);
            gpio_func_keep(LGPIO_15,2,func_ETH2);
		}

		if (config & PIN_ETH_CFG_ETH2_EXTPHYCLK) {
			if (top_reg2.bit.LCD_TYPE) {
				if (top_reg2.bit.LCD_TYPE != MUX_9) {
					pr_err("ETH2_EXTPHYCLK conflict with LCD_TYPE\r\n");
					return E_PAR;
				}
			}
			if (top_reg2.bit.LCD2_TYPE) {
				if (top_reg2.bit.LCD2_TYPE != MUX_9) {
					pr_err("ETH2_EXTPHYCLK conflict with LCD2_TYPE\r\n");
					return E_PAR;
				}
			}
			if (top_reg2.bit.MEMIF_TYPE) {
				pr_err("ETH2_EXTPHYCLK conflict with MEMIF_TYPE\r\n");
				return E_PAR;
			}

			top_reg3.bit.ETH2_EXT_PHY_CLK = MUX_1;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_17,1,func_ETH2);
		}

		if (config & PIN_ETH_CFG_ETH2_PTP) {
			if (top_reg13.bit.UART3 == MUX_1) {
				pr_err("ETH2_PTP conflict with UART3_1\r\n");
				return E_PAR;
			}
			if (top_reg5.bit.I2C9 == MUX_1) {
				pr_err("ETH2_PTP conflict with I2C9_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI4 == MUX_1) {
				pr_err("ETH2_PTP conflict with SPI4_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF3 == MUX_1) {
				pr_err("ETH2_PTP conflict with SIF3_1\r\n");
				return E_PAR;
			}
			if (top_reg12.bit.I2S == MUX_1) {
				pr_err("ETH2_PTP conflict with I2S_1\r\n");
				return E_PAR;
			}

			top_reg3.bit.ETH2_PTP = MUX_1;
			top_reg_pgpio0.bit.PGPIO_24 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_24,1,func_ETH2);
		}
	}

	return E_OK;
}

static int pinmux_config_i2c(uint32_t config)
{
	if (config == PIN_I2C_CFG_NONE) {
	} else {
		if (config & PIN_I2C_CFG_I2C_1) {
			if ((top_reg13.bit.UART2 == 1) &&
			    (top_reg14.bit.UART2_RTSCTS == MUX_1 || top_reg14.bit.UART2_RTSCTS == MUX_2)) {
				pr_err("I2C_1 conflict with UART2_1 RTS\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI5 == MUX_1) {
				pr_err("I2C_1 conflict with SPI5_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF4 == MUX_1) {
				pr_err("I2C_1 conflict with SIF4_1\r\n");
				return E_PAR;
			}
			if (top_reg12.bit.I2S_MCLK == MUX_1) {
				pr_err("I2C_1 conflict with I2S_MCLK_1\r\n");
				return E_PAR;
			}
			if (top_reg18.bit.RTC_CLK == MUX_1) {
				pr_err("I2C_1 conflict with RTC_CLK\r\n");
				return E_PAR;
			}
			if (top_reg3.bit.ETH_PTP == MUX_1) {
				pr_err("I2C_1 conflict with ETH_PTP\r\n");
				return E_PAR;
			}

			top_reg4.bit.I2C = MUX_1;
			top_reg_pgpio0.bit.PGPIO_28 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_29 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_PGPIO28, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_PGPIO29, PAD_NONE);
            gpio_func_keep(PGPIO_28,2,func_I2C);
		}

		if (config & PIN_I2C_CFG_I2C2_1) {
			if (top_reg13.bit.UART2 == MUX_1) {
				pr_err("I2C2_1 conflict with UART2_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI5 == MUX_1) {
				pr_err("I2C2_1 conflict with SPI5_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF4 == MUX_1) {
				pr_err("I2C2_1 conflict with SIF4_1\r\n");
				return E_PAR;
			}
			if (top_reg15.bit.REMOTE_EXT == MUX_1) {
				pr_err("I2C2_1 conflict with REMOTE_EXT_1\r\n");
				return E_PAR;
			}
			if (top_reg15.bit.REMOTE == MUX_1) {
				pr_err("I2C2_1 conflict with REMOTE_1\r\n");
				return E_PAR;
			}

			top_reg4.bit.I2C2 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_30 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_31 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_PGPIO30, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_PGPIO31, PAD_NONE);
            gpio_func_keep(PGPIO_30,2,func_I2C2);
		}

		if (config & PIN_I2C_CFG_I2C3_1) {
			if (top_reg13.bit.UART6 == MUX_1) {
				pr_err("I2C3_1 conflict with UART6_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI == MUX_1) {
				pr_err("I2C3_1 conflict with SPI_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF0 == MUX_1) {
				pr_err("I2C3_1 conflict with SIF0_1\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR3 == MUX_1 || top_reg9.bit.SENSOR3 == MUX_3 ||
				top_reg9.bit.SENSOR3 == MUX_4 || top_reg9.bit.SENSOR3 == MUX_5) {
				pr_err("I2C3_1 conflict with SENSOR3\r\n");
				return E_PAR;
			}

			top_reg4.bit.I2C3 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_PGPIO12, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_PGPIO13, PAD_NONE);
            gpio_func_keep(PGPIO_12,2,func_I2C3_1);
		}
		if (config & PIN_I2C_CFG_I2C3_2) {
			if (top_reg17.bit.SIF2 == MUX_2) {
				pr_err("I2C3_2 conflict with SIF2_2\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR2 == MUX_1 || top_reg9.bit.SENSOR2 == MUX_3 ||
				top_reg9.bit.SENSOR2 == MUX_4 || top_reg9.bit.SENSOR2 == MUX_5) {
				pr_err("I2C3_2 conflict with SENSOR2\r\n");
				return E_PAR;
			}

			top_reg4.bit.I2C3 = MUX_2;
			top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_12 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_SGPIO11, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_SGPIO12, PAD_NONE);
            gpio_func_keep(SGPIO_11,2,func_I2C3_2);
		}
		if (config & PIN_I2C_CFG_I2C3_3) {
			if (top_reg9.bit.HSI2HSI3_SEL == MUX_0) {
				pr_err("I2C3_3 conflict with HSI2HSI3_SEL as CSI/LVDS mode\r\n");
				return E_PAR;
			}

			top_reg4.bit.I2C3 = MUX_3;
			top_reg_hsigpio0.bit.HSIGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_23 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_HSIGPIO22, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_HSIGPIO23, PAD_NONE);
            gpio_func_keep(HSIGPIO_22,2,func_I2C3_3);
		}

		if (config & PIN_I2C_CFG_I2C4_1) {
			if ((top_reg13.bit.UART6 == MUX_1) &&
			    (top_reg14.bit.UART6_RTSCTS == MUX_1 || top_reg14.bit.UART6_RTSCTS == MUX_2)) {
				pr_err("I2C4_1 conflict with UART6_1 RTS\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI == MUX_1) {
				pr_err("I2C4_1 conflict with SPI_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF0 == MUX_1) {
				pr_err("I2C4_1 conflict with SIF0_1\r\n");
				return E_PAR;
			}
			if (top_reg15.bit.SDP == MUX_1) {
				pr_err("I2C4_1 conflict with SDP_1\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR3 == MUX_1 || top_reg9.bit.SENSOR3 == MUX_3 ||
				top_reg9.bit.SENSOR3 == MUX_4 || top_reg9.bit.SENSOR3 == MUX_5) {
				pr_err("I2C4_1 conflict with SENSOR3\r\n");
				return E_PAR;
			}

			top_reg4.bit.I2C4 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_PGPIO14, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_PGPIO15, PAD_NONE);
            gpio_func_keep(PGPIO_14,2,func_I2C4_1);
		}
		if (config & PIN_I2C_CFG_I2C4_2) {
			if (top_reg17.bit.SIF3 == MUX_2) {
				pr_err("I2C4_2 conflict with SIF3_2\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR2 == MUX_1 || top_reg9.bit.SENSOR2 == MUX_3 ||
				top_reg9.bit.SENSOR2 == MUX_4 || top_reg9.bit.SENSOR2 == MUX_5) {
				pr_err("I2C4_2 conflict with SENSOR2\r\n");
				return E_PAR;
			}

			top_reg4.bit.I2C4 = MUX_2;
			top_reg_sgpio0.bit.SGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_14 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_SGPIO13, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_SGPIO14, PAD_NONE);
            gpio_func_keep(SGPIO_13,2,func_I2C4_2);
		}
		if (config & PIN_I2C_CFG_I2C4_3) {
			if (top_reg0.bit.DSI_PROT_EN == MUX_1) {
				pr_err("I2C4_3 conflict with DSI_PROT_EN\r\n");
				return E_PAR;
			}
			if (top_reg13.bit.UART6 == MUX_2) {
				pr_err("I2C4_3 conflict with UART6_2\r\n");
				return E_PAR;
			}

			top_reg4.bit.I2C4 = MUX_3;
			top_reg_dsigpio0.bit.DSIGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_dsigpio0.bit.DSIGPIO_5 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_DSIGPIO4, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_DSIGPIO5, PAD_NONE);
            gpio_func_keep(DSIGPIO_4,2,func_I2C4_3);
		}
		if (config & PIN_I2C_CFG_I2C4_4) {
			if (top_reg9.bit.HSI2HSI3_SEL == MUX_0) {
				pr_err("I2C4_4 conflict with HSI2HSI3_SEL as CSI/LVDS mode\r\n");
				return E_PAR;
			}

			top_reg4.bit.I2C4 = MUX_4;
			top_reg_hsigpio0.bit.HSIGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_21 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_HSIGPIO20, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_HSIGPIO21, PAD_NONE);
            gpio_func_keep(HSIGPIO_20,2,func_I2C4_4);
		}

		if (config & PIN_I2C_CFG_I2C5_1) {
			if (top_reg13.bit.UART5 == MUX_1) {
				pr_err("I2C5_1 conflict with UART5_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI2 == MUX_1) {
				pr_err("I2C5_1 conflict with SPI2_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF1 == MUX_1) {
				pr_err("I2C5_1 conflict with SIF1_1\r\n");
				return E_PAR;
			}
			if (top_reg15.bit.SDP == MUX_1) {
				pr_err("I2C5_1 conflict with SDP_1\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR3 == MUX_1 || top_reg9.bit.SENSOR3 == MUX_3 ||
				top_reg9.bit.SENSOR3 == MUX_4 || top_reg9.bit.SENSOR3 == MUX_5) {
				pr_err("I2C5_1 conflict with SENSOR3\r\n");
				return E_PAR;
			}

			top_reg4.bit.I2C5 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_PGPIO16, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_PGPIO17, PAD_NONE);
            gpio_func_keep(PGPIO_16,2,func_I2C5_1);
		}
		if (config & PIN_I2C_CFG_I2C5_2) {
			if (top_reg17.bit.SIF2 == MUX_2) {
				pr_err("I2C5_2 conflict with SIF2_2\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF3 == MUX_2) {
				pr_err("I2C5_2 conflict with SIF3_2\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR2 == MUX_1 || top_reg9.bit.SENSOR2 == MUX_3 ||
				top_reg9.bit.SENSOR2 == MUX_4 || top_reg9.bit.SENSOR2 == MUX_5) {
				pr_err("I2C5_2 conflict with SENSOR2\r\n");
				return E_PAR;
			}

			top_reg4.bit.I2C5 = MUX_2;
			top_reg_sgpio0.bit.SGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_16 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_SGPIO15, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_SGPIO16, PAD_NONE);
            gpio_func_keep(SGPIO_15,2,func_I2C5_2);
		}
		if (config & PIN_I2C_CFG_I2C5_3) {
			if (top_reg1.bit.SDIO3_BUS_WIDTH == MUX_1) {
				pr_err("I2C5_3 conflict with SDIO3_BUS_WIDTH\r\n");
				return E_PAR;
			}
			if (top_reg7.bit.PWM8 == MUX_2) {
				pr_err("I2C5_3 conflict with PWM8_2\r\n");
				return E_PAR;
			}
			if (top_reg7.bit.PWM9 == MUX_2) {
				pr_err("I2C5_3 conflict with PWM9_2\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI == MUX_3) {
				pr_err("I2C5_3 conflict with SPI_3\r\n");
				return E_PAR;
			}
			if (top_reg13.bit.UART7 == MUX_2) {
				pr_err("I2C5_3 conflict with UART7_2\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF5 == MUX_2) {
				pr_err("I2C5_3 conflict with SIF5_2\r\n");
				return E_PAR;
			}

			top_reg4.bit.I2C5 = MUX_3;
			top_reg_cgpio0.bit.CGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_5 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_CGPIO4, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_CGPIO5, PAD_NONE);
            gpio_func_keep(CGPIO_4,2,func_I2C5_3);
		}
	}

	return E_OK;
}

static int pinmux_config_i2cII(uint32_t config)
{
	if (config == PIN_I2CII_CFG_NONE) {
	} else {
		if (config & PIN_I2CII_CFG_I2C6_1) {
			if ((top_reg13.bit.UART5 == MUX_1) &&
				(top_reg14.bit.UART5_RTSCTS == MUX_1 || top_reg14.bit.UART5_RTSCTS == MUX_2)) {
				pr_err("I2C6_1 conflict with UART5_1 RTS\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI2 == MUX_1) {
				pr_err("I2C6_1 conflict with SPI2_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF1 == MUX_1) {
				pr_err("I2C6_1 conflict with SIF1_1\r\n");
				return E_PAR;
			}
			if (top_reg15.bit.SDP == MUX_1) {
				pr_err("I2C6_1 conflict with SDP_1\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR3 == MUX_1 || top_reg9.bit.SENSOR3 == MUX_3 ||
				top_reg9.bit.SENSOR3 == MUX_4 || top_reg9.bit.SENSOR3 == MUX_5) {
				pr_err("I2C6_1 conflict with SENSOR3\r\n");
				return E_PAR;
			}

			top_reg4.bit.I2C6 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_PGPIO18, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_PGPIO19, PAD_NONE);
            gpio_func_keep(PGPIO_18,2,func_I2C6_1);
		}
		if (config & PIN_I2CII_CFG_I2C6_2) {
			if (top_reg17.bit.SIF4 == MUX_2) {
				pr_err("I2C6_2 conflict with SIF4_2\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR2 == MUX_1 || top_reg9.bit.SENSOR2 == MUX_2 ||
				top_reg9.bit.SENSOR2 == MUX_4 || top_reg9.bit.SENSOR2 == MUX_5) {
				pr_err("I2C6_2 conflict with SENSOR2\r\n");
				return E_PAR;
			}

			top_reg4.bit.I2C6 = MUX_2;
			top_reg_sgpio0.bit.SGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_24 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_SGPIO23, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_SGPIO24, PAD_NONE);
            gpio_func_keep(SGPIO_23,2,func_I2C6_2);
		}
		if (config & PIN_I2CII_CFG_I2C6_3) {
			if (top_reg1.bit.SDIO3_BUS_WIDTH == MUX_1) {
				pr_err("I2C6_3 conflict with SDIO3_BUS_WIDTH\r\n");
				return E_PAR;
			}
			if (top_reg7.bit.PWM10 == MUX_2) {
				pr_err("I2C6_3 conflict with PWM10_2\r\n");
				return E_PAR;
			}
			if (top_reg7.bit.PWM11 == MUX_2) {
				pr_err("I2C6_3 conflict with PWM11_2\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI3 == MUX_3) {
				pr_err("I2C6_3 conflict with SPI_3\r\n");
				return E_PAR;
			}
			if ((top_reg13.bit.UART7 == MUX_2) &&
				(top_reg14.bit.UART7_RTSCTS == MUX_1 || top_reg14.bit.UART7_RTSCTS == MUX_2)) {
				pr_err("I2C6_3 conflict with UART7_2 RTS\r\n");
				return E_PAR;
			}

			if (top_reg17.bit.SIF5 == MUX_2) {
				pr_err("I2C6_3 conflict with SIF5_2\r\n");
				return E_PAR;
			}

			top_reg4.bit.I2C6 = MUX_3;
			top_reg_cgpio0.bit.CGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_7 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_CGPIO6, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_CGPIO7, PAD_NONE);
            gpio_func_keep(CGPIO_6,2,func_I2C6_3);
		}

		if (config & PIN_I2CII_CFG_I2C7_1) {
			if (top_reg13.bit.UART4 == MUX_1) {
				pr_err("I2C7_1 conflict with UART4_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI3 == MUX_1) {
				pr_err("I2C7_1 conflict with SPI3_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF2 == MUX_1) {
				pr_err("I2C7_1 conflict with SIF2_1\r\n");
				return E_PAR;
			}
			if (top_reg7.bit.PICNT == MUX_1) {
				pr_err("I2C7_1 conflict with PICNT\r\n");
				return E_PAR;
			}
			if (top_reg10.bit.SN5_MCLK == MUX_2) {
				pr_err("I2C7_1 conflict with SN5_MCLK_2\r\n");
				return E_PAR;
			}

			top_reg4.bit.I2C7 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_21 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_PGPIO20, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_PGPIO21, PAD_NONE);
            gpio_func_keep(PGPIO_20,2,func_I2C7_1);
		}
		if (config & PIN_I2CII_CFG_I2C7_2) {
			if (top_reg9.bit.FLASH_TRIG_IN == MUX_1) {
				pr_err("I2C7_2 conflict with FLASH_TRIG_IN_1\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.FLASH_TRIG_OUT == MUX_1) {
				pr_err("I2C7_2 conflict with FLASH_TRIG_OUT_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF4 == MUX_2) {
				pr_err("I2C7_2 conflict with SIF4_2\r\n");
				return E_PAR;
			}

			top_reg4.bit.I2C7 = MUX_2;
			top_reg_sgpio0.bit.SGPIO_25 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_26 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_SGPIO25, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_SGPIO26, PAD_NONE);
            gpio_func_keep(SGPIO_25,2,func_I2C7_2);
		}

		if (config & PIN_I2CII_CFG_I2C8_1) {
			if ((top_reg13.bit.UART4 == MUX_1) &&
				(top_reg14.bit.UART4_RTSCTS == MUX_1 || top_reg14.bit.UART4_RTSCTS == MUX_2)) {
				pr_err("I2C8_1 conflict with UART4_1 RTS\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI3 == MUX_1) {
				pr_err("I2C8_1 conflict with SPI3_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF2 == MUX_1) {
				pr_err("I2C8_1 conflict with SIF2_1\r\n");
				return E_PAR;
			}
			if (top_reg7.bit.PICNT2 == MUX_1) {
				pr_err("I2C8_1 conflict with PICNT2\r\n");
				return E_PAR;
			}
			if (top_reg7.bit.PICNT3 == MUX_1) {
				pr_err("I2C8_1 conflict with PICNT3\r\n");
				return E_PAR;
			}

			top_reg4.bit.I2C8 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_23 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_PGPIO22, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_PGPIO23, PAD_NONE);
            gpio_func_keep(PGPIO_22,2,func_I2C8_1);
		}
		if (config & PIN_I2CII_CFG_I2C8_2) {
			if (top_reg10.bit.SN2_XVSXHS == MUX_1) {
				pr_err("I2C8_2 conflict with SN2_XVSXHS_1\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SN2_CCIR_VSHS == MUX_1) {
				pr_err("I2C8_2 conflict with SN2_CCIR_VSHS\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR2 == MUX_1) {
				pr_err("I2C8_2 conflict with SENSOR2\r\n");
				return E_PAR;
			}

			top_reg4.bit.I2C8 = MUX_2;
			top_reg_sgpio0.bit.SGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_8 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_SGPIO7, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_SGPIO8, PAD_NONE);
            gpio_func_keep(SGPIO_7,2,func_I2C8_2);
		}

		if (config & PIN_I2CII_CFG_I2C9_1) {
			if (top_reg13.bit.UART3 == MUX_1) {
				pr_err("I2C9_1 conflict with UART3_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI4 == MUX_1) {
				pr_err("I2C9_1 conflict with SPI4_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF3 == MUX_1) {
				pr_err("I2C9_1 conflict with SIF3_1\r\n");
				return E_PAR;
			}
			if (top_reg12.bit.I2S == MUX_1) {
				pr_err("I2C9_1 conflict with I2S_1\r\n");
				return E_PAR;
			}
			if (top_reg3.bit.ETH2_PTP == MUX_1) {
				pr_err("I2C9_1 conflict with ETH2_PTP\r\n");
				return E_PAR;
			}
			if (top_reg12.bit.DMIC_DATA1 == MUX_1) {
				pr_err("I2C9_1 conflict with DMIC_DATA1\r\n");
				return E_PAR;
			}

			top_reg5.bit.I2C9 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_24 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_25 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_PGPIO24, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_PGPIO25, PAD_NONE);
            gpio_func_keep(PGPIO_24,2,func_I2C9_1);
		}
		if (config & PIN_I2CII_CFG_I2C9_2) {
			if (top_reg17.bit.SIF0 == MUX_2) {
				pr_err("I2C9_2 conflict with SIF0_2\r\n");
				return E_PAR;
			}
			if (top_reg2.bit.LCD_TYPE == MUX_7 || top_reg2.bit.LCD_TYPE == MUX_8 ||
				top_reg2.bit.LCD_TYPE == MUX_10) {
				pr_err("I2C9_2 conflict with LCD_TYPE\r\n");
				return E_PAR;
			}
			if (top_reg3.bit.ETH == MUX_1 || top_reg3.bit.ETH == MUX_2) {
				pr_err("I2C9_2 conflict with ETH\r\n");
				return E_PAR;
			}

			top_reg5.bit.I2C9 = MUX_2;
			top_reg_lgpio0.bit.LGPIO_26 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_27 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_LGPIO26, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_LGPIO27, PAD_NONE);
            gpio_func_keep(LGPIO_26,2,func_I2C9_2);
		}

		if (config & PIN_I2CII_CFG_I2C10_1) {
			if ((top_reg13.bit.UART3 == MUX_1) &&
				(top_reg14.bit.UART3_RTSCTS == MUX_1 || top_reg14.bit.UART3_RTSCTS == MUX_2)) {
				pr_err("I2C10_1 conflict with UART3_1 RTS\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI4 == MUX_1) {
				pr_err("I2C10_1 conflict with SPI4_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF3 == MUX_1) {
				pr_err("I2C10_1 conflict with SIF3_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI3_RDY == MUX_1) {
				pr_err("I2C10_1 conflict with SPI3_RDY_1\r\n");
				return E_PAR;
			}
			if (top_reg12.bit.I2S == MUX_1) {
				pr_err("I2C10_1 conflict with I2S_1\r\n");
				return E_PAR;
			}
			if (top_reg12.bit.DMIC == MUX_1) {
				pr_err("I2C10_1 conflict with DMIC\r\n");
				return E_PAR;
			}

			top_reg5.bit.I2C10 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_26 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_27 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_PGPIO26, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_PGPIO27, PAD_NONE);
            gpio_func_keep(PGPIO_26,2,func_I2C10_1);
		}
		if (config & PIN_I2CII_CFG_I2C10_2) {
			top_reg5.bit.I2C10 = MUX_2;
			top_reg_dgpio0.bit.DGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_1 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_DGPIO0, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_DGPIO1, PAD_NONE);
            gpio_func_keep(DGPIO_0,2,func_I2C10_2);
		}

		if (config & PIN_I2CII_CFG_I2C11_1) {
			top_reg5.bit.I2C11 = MUX_1;
			top_reg_pgpio1.bit.PGPIO_36 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio1.bit.PGPIO_37 = GPIO_ID_EMUM_FUNC;
			//pad_set_pull_updown(PAD_PIN_PGPIO36, PAD_NONE);
			//pad_set_pull_updown(PAD_PIN_PGPIO37, PAD_NONE);
            gpio_func_keep(PGPIO_36,2,func_I2C11_1);
		}
	}

	return E_OK;
}

static int pinmux_config_pwm(uint32_t config)
{
	if (config == PIN_PWM_CFG_NONE) {
	} else {
		if (config & PIN_PWM_CFG_PWM0_1) {
			if (top_reg14.bit.UART9 == MUX_1) {
				pr_err("PWM0_1 conflict with UART9_1\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR3 == MUX_1 || top_reg9.bit.SENSOR3 == MUX_2 ||
				top_reg9.bit.SENSOR3 == MUX_4 || top_reg9.bit.SENSOR3 == MUX_5) {
				pr_err("PWM0_1 conflict with SENSOR3\r\n");
				return E_PAR;
			}

			top_reg6.bit.PWM0 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_0,1,func_PWM_1);
		} else if (config & PIN_PWM_CFG_PWM0_2) {
			if (top_reg0.bit.DSI_PROT_EN == MUX_1) {
				pr_err("PWM0_2 conflict with DSI_PROT_EN\r\n");
				return E_PAR;
			}
			if ((top_reg13.bit.UART6 == MUX_2) &&
			    (top_reg14.bit.UART6_RTSCTS == MUX_1 || top_reg14.bit.UART6_RTSCTS == MUX_2)) {
				pr_err("PWM0_2 conflict with UART6_2 RTS\r\n");
				return E_PAR;
			}

			top_reg6.bit.PWM0 = MUX_2;
			top_reg_dsigpio0.bit.DSIGPIO_6 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(DSIGPIO_6,1,func_PWM_2);
		}

		if (config & PIN_PWM_CFG_PWM1_1) {
			if (top_reg14.bit.UART9 == MUX_1) {
				pr_err("PWM1_1 conflict with UART9_1\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR3 == MUX_1 || top_reg9.bit.SENSOR3 == MUX_2 ||
				top_reg9.bit.SENSOR3 == MUX_4 || top_reg9.bit.SENSOR3 == MUX_5) {
				pr_err("PWM1_1 conflict with SENSOR3\r\n");
				return E_PAR;
			}

			top_reg6.bit.PWM1 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_1,1,func_PWM1_1);
		} else if (config & PIN_PWM_CFG_PWM1_2) {
			if (top_reg0.bit.DSI_PROT_EN == MUX_1) {
				pr_err("PWM1_2 conflict with DSI_PROT_EN\r\n");
				return E_PAR;
			}
			if ((top_reg13.bit.UART6 == MUX_2) &&
			    (top_reg14.bit.UART6_RTSCTS == MUX_1)) {
				pr_err("PWM1_2 conflict with UART6_2 CTS\r\n");
				return E_PAR;
			}

			top_reg6.bit.PWM1 = MUX_2;
			top_reg_dsigpio0.bit.DSIGPIO_7 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(DSIGPIO_7,1,func_PWM1_2);
		}

		if (config & PIN_PWM_CFG_PWM2_1) {
			if ((top_reg14.bit.UART9 == MUX_1) &&
			    (top_reg14.bit.UART9_RTSCTS == MUX_1 || top_reg14.bit.UART9_RTSCTS == MUX_2)) {
				pr_err("PWM2_1 conflict with UART9_1 RTS\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR3 == MUX_1 || top_reg9.bit.SENSOR3 == MUX_2 ||
				top_reg9.bit.SENSOR3 == MUX_4 || top_reg9.bit.SENSOR3 == MUX_5) {
				pr_err("PWM2_1 conflict with SENSOR3\r\n");
				return E_PAR;
			}

			top_reg6.bit.PWM2 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_2,1,func_PWM2_1);
		} else if (config & PIN_PWM_CFG_PWM2_2) {
			if (top_reg0.bit.DSI_PROT_EN == MUX_1) {
				pr_err("PWM2_2 conflict with DSI_PROT_EN\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF0 == MUX_3) {
				pr_err("PWM2_2 conflict with SIF0_3\r\n");
				return E_PAR;
			}

			top_reg6.bit.PWM2 = MUX_2;
			top_reg_dsigpio0.bit.DSIGPIO_8 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(DSIGPIO_8,1,func_PWM2_2);
		}

		if (config & PIN_PWM_CFG_PWM3_1) {
			if ((top_reg14.bit.UART9 == MUX_1) &&
			    (top_reg14.bit.UART9_RTSCTS == MUX_1)) {
				pr_err("PWM3_1 conflict with UART9_1 CTS\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR3 == MUX_1 || top_reg9.bit.SENSOR3 == MUX_2 ||
				top_reg9.bit.SENSOR3 == MUX_4 || top_reg9.bit.SENSOR3 == MUX_5) {
				pr_err("PWM3_1 conflict with SENSOR3\r\n");
				return E_PAR;
			}

			top_reg6.bit.PWM3 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_3,1,func_PWM3_1);
		} else if (config & PIN_PWM_CFG_PWM3_2) {
			if (top_reg0.bit.DSI_PROT_EN == MUX_1) {
				pr_err("PWM3_2 conflict with DSI_PROT_EN\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF0 == MUX_3) {
				pr_err("PWM3_2 conflict with SIF0_3\r\n");
				return E_PAR;
			}

			top_reg6.bit.PWM3 = MUX_2;
			top_reg_dsigpio0.bit.DSIGPIO_9 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(DSIGPIO_9,1,func_PWM3_2);
		}

		if (config & PIN_PWM_CFG_PWM4_1) {
			if (top_reg13.bit.UART8 == MUX_1) {
				pr_err("PWM4_1 conflict with UART8_1\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR3 == MUX_1 || top_reg9.bit.SENSOR3 == MUX_2 ||
				top_reg9.bit.SENSOR3 == MUX_4 || top_reg9.bit.SENSOR3 == MUX_5) {
				pr_err("PWM4_1 conflict with SENSOR3\r\n");
				return E_PAR;
			}

			top_reg6.bit.PWM4 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_4,1,func_PWM4_1);
		} else if (config & PIN_PWM_CFG_PWM4_2) {
			if (top_reg16.bit.SPI4 == MUX_2) {
				pr_err("PWM4_2 conflict with SPI4_2\r\n");
				return E_PAR;
			}

			top_reg6.bit.PWM4 = MUX_2;
			top_reg_dgpio0.bit.DGPIO_2 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(DGPIO_2,1,func_PWM4_2);
		}

		if (config & PIN_PWM_CFG_PWM5_1) {
			if (top_reg13.bit.UART8 == MUX_1) {
				pr_err("PWM5_1 conflict with UART8_1\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR3 == MUX_1 || top_reg9.bit.SENSOR3 == MUX_2 ||
				top_reg9.bit.SENSOR3 == MUX_4 || top_reg9.bit.SENSOR3 == MUX_5) {
				pr_err("PWM5_1 conflict with SENSOR3\r\n");
				return E_PAR;
			}

			top_reg6.bit.PWM5 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_5,1,func_PWM5_1);
		} else if (config & PIN_PWM_CFG_PWM5_2) {
			if (top_reg16.bit.SPI4 == MUX_2) {
				pr_err("PWM5_2 conflict with SPI4_2\r\n");
				return E_PAR;
			}

			top_reg6.bit.PWM5 = MUX_2;
			top_reg_dgpio0.bit.DGPIO_3 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(DGPIO_3,1,func_PWM5_2);
		}
	}

	return E_OK;
}

static int pinmux_config_pwmII(uint32_t config)
{
	if (config == PIN_PWMII_CFG_NONE) {
	} else {
		if (config & PIN_PWMII_CFG_PWM6_1) {
			if ((top_reg13.bit.UART8 == MUX_1) &&
			    (top_reg14.bit.UART8_RTSCTS == MUX_1 || top_reg14.bit.UART8_RTSCTS == MUX_2)) {
				pr_err("PWM6_1 conflict with UART8_1 RTS\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR3 == MUX_1 || top_reg9.bit.SENSOR3 == MUX_2 ||
				top_reg9.bit.SENSOR3 == MUX_4 || top_reg9.bit.SENSOR3 == MUX_5) {
				pr_err("PWM6_1 conflict with SENSOR3\r\n");
				return E_PAR;
			}

			top_reg6.bit.PWM6 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_6,1,func_PWM6_1);
		} else if (config & PIN_PWMII_CFG_PWM6_2) {
			if (top_reg19.bit.SATA_LED == MUX_1) {
				pr_err("PWM6_2 conflict with SATA_LED\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI4 == MUX_2) {
				pr_err("PWM6_2 conflict with SPI4_2\r\n");
				return E_PAR;
			}

			top_reg6.bit.PWM6 = MUX_2;
			top_reg_dgpio0.bit.DGPIO_4 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(DGPIO_4,1,func_PWM6_2);
		}

		if (config & PIN_PWMII_CFG_PWM7_1) {
			if ((top_reg13.bit.UART8 == MUX_1) &&
			    (top_reg14.bit.UART8_RTSCTS == MUX_1)) {
				pr_err("PWM7_1 conflict with UART8_1 CTS\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR3 == MUX_1 || top_reg9.bit.SENSOR3 == MUX_2 ||
				top_reg9.bit.SENSOR3 == MUX_4 || top_reg9.bit.SENSOR3 == MUX_5) {
				pr_err("PWM7_1 conflict with SENSOR3\r\n");
				return E_PAR;
			}

			top_reg6.bit.PWM7 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_7,1,func_PWM7_1);
		} else if (config & PIN_PWMII_CFG_PWM7_2) {
			if (top_reg14.bit.UART9 == MUX_2) {
				pr_err("PWM7_2 conflict with UART9_2\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI4 == MUX_2 && top_reg16.bit.SPI4_BUS_WIDTH == MUX_1) {
				pr_err("PWM7_2 conflict with SPI4_2 BUS_WIDTH\r\n");
				return E_PAR;
			}
			if (top_reg3.bit.ETH == MUX_1 || top_reg3.bit.ETH == MUX_2) {
				pr_err("PWM7_2 conflict with ETH\r\n");
				return E_PAR;
			}

			top_reg6.bit.PWM7 = MUX_2;
			top_reg_dgpio0.bit.DGPIO_5 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(DGPIO_5,1,func_PWM7_2);
		}

		if (config & PIN_PWMII_CFG_PWM8_1) {
			if (top_reg13.bit.UART7 == MUX_1) {
				pr_err("PWM8_1 conflict with UART7_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF5 == MUX_1) {
				pr_err("PWM8_1 conflict with SIF5_1\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR3 == MUX_1 || top_reg9.bit.SENSOR3 == MUX_2 ||
				top_reg9.bit.SENSOR3 == MUX_4 || top_reg9.bit.SENSOR3 == MUX_5) {
				pr_err("PWM8_1 conflict with SENSOR3\r\n");
				return E_PAR;
			}

			top_reg7.bit.PWM8 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_8,1,func_PWM8_1);
		} else if (config & PIN_PWMII_CFG_PWM8_2) {
			if (top_reg1.bit.SDIO3_BUS_WIDTH == MUX_1) {
				pr_err("PWM8_2 conflict with SDIO3_BUS_WIDTH\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C5 == MUX_3) {
				pr_err("PWM8_2 conflict with I2C5_3\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI == MUX_3) {
				pr_err("PWM8_2 conflict with SPI_3\r\n");
				return E_PAR;
			}
			if (top_reg13.bit.UART7 == MUX_2) {
				pr_err("PWM8_2 conflict with UART7_2\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF5 == MUX_2) {
				pr_err("PWM8_2 conflict with SIF5_2\r\n");
				return E_PAR;
			}

			top_reg7.bit.PWM8 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_4 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(CGPIO_4,1,func_PWM8_2);
		}

		if (config & PIN_PWMII_CFG_PWM9_1) {
			if (top_reg13.bit.UART7 == MUX_1) {
				pr_err("PWM9_1 conflict with UART7_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF5 == MUX_1) {
				pr_err("PWM9_1 conflict with SIF5_1\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SN3_CCIR_VSHS == MUX_1) {
				pr_err("PWM9_1 conflict with SN3_CCIR_VSHS\r\n");
				return E_PAR;
			}

			top_reg7.bit.PWM9 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_9,1,func_PWM9_1);
		} else if (config & PIN_PWMII_CFG_PWM9_2) {
			if (top_reg1.bit.SDIO3_BUS_WIDTH == MUX_1) {
				pr_err("PWM9_2 conflict with SDIO3_BUS_WIDTH\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C5 == MUX_3) {
				pr_err("PWM9_2 conflict with I2C5_3\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI == MUX_3) {
				pr_err("PWM9_2 conflict with SPI_3\r\n");
				return E_PAR;
			}
			if (top_reg13.bit.UART7 == MUX_2) {
				pr_err("PWM9_2 conflict with UART7_2\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF5 == MUX_2) {
				pr_err("PWM9_2 conflict with SIF5_2\r\n");
				return E_PAR;
			}

			top_reg7.bit.PWM9 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_5 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(CGPIO_5,1,func_PWM9_2);
		}

		if (config & PIN_PWMII_CFG_PWM10_1) {
			if ((top_reg13.bit.UART7 == MUX_1) &&
			    (top_reg14.bit.UART7_RTSCTS == MUX_1 || top_reg14.bit.UART7_RTSCTS == MUX_2)) {
				pr_err("PWM10_1 conflict with UART7_1 RTS\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF5 == MUX_1) {
				pr_err("PWM10_1 conflict with SIF5_1\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SN3_CCIR_VSHS == MUX_1) {
				pr_err("PWM10_1 conflict with SN3_CCIR_VSHS\r\n");
				return E_PAR;
			}

			top_reg7.bit.PWM10 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_10,1,func_PWM10_1);
		} else if (config & PIN_PWMII_CFG_PWM10_2) {
			if (top_reg1.bit.SDIO3_BUS_WIDTH == MUX_1) {
				pr_err("PWM10_2 conflict with SDIO3_BUS_WIDTH\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C6 == MUX_2) {
				pr_err("PWM10_2 conflict with I2C6_2\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI == MUX_3) {
				pr_err("PWM10_2 conflict with SPI_3\r\n");
				return E_PAR;
			}
			if ((top_reg13.bit.UART7 == MUX_2) &&
			    (top_reg14.bit.UART7_RTSCTS == MUX_1 || top_reg14.bit.UART7_RTSCTS == MUX_2)) {
				pr_err("PWM10_2 conflict with UART7_2 RTS\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF5 == MUX_2) {
				pr_err("PWM10_2 conflict with SIF5_2\r\n");
				return E_PAR;
			}

			top_reg7.bit.PWM10 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_6 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(CGPIO_6,1,func_PWM10_2);
		}

		if (config & PIN_PWMII_CFG_PWM11_1) {
			if ((top_reg13.bit.UART7 == MUX_1) &&
			    (top_reg14.bit.UART7_RTSCTS == MUX_1)) {
				pr_err("PWM11_1 conflict with UART7_1 CTS\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SN3_CCIR_VSHS == MUX_1) {
				pr_err("PWM11_1 conflict with SN3_CCIR_VSHS\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR3 == MUX_3 || top_reg9.bit.SENSOR3 == MUX_4) {
				pr_err("PWM11_1 conflict with SENSOR3\r\n");
				return E_PAR;
			}

			top_reg7.bit.PWM11 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_11,1,func_PWM11_1);
		} else if (config & PIN_PWMII_CFG_PWM11_2) {
			if (top_reg1.bit.SDIO3_BUS_WIDTH == MUX_1) {
				pr_err("PWM11_2 conflict with SDIO3_BUS_WIDTH\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C6 == MUX_3) {
				pr_err("PWM11_2 conflict with I2C6_3\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI == MUX_3 && top_reg16.bit.SPI_BUS_WIDTH == MUX_1) {
				pr_err("PWM11_2 conflict with SPI_3 BUS_WIDTH\r\n");
				return E_PAR;
			}
			if ((top_reg13.bit.UART7 == MUX_2) &&
			    (top_reg14.bit.UART7_RTSCTS == MUX_1)) {
				pr_err("PWM11_2 conflict with UART7_2 CTS\r\n");
				return E_PAR;
			}

			top_reg7.bit.PWM11 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_7 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(CGPIO_7,1,func_PWM11_2);
		}
	}

	return E_OK;
}

static int pinmux_config_ccnt(uint32_t config)
{
	if (config == PIN_CCNT_CFG_NONE) {
	} else {
		if (config & PIN_CCNT_CFG_CCNT_1) {
			if (top_reg13.bit.UART4 == MUX_1) {
				pr_err("PICNT conflict with UART4_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C7 == MUX_1) {
				pr_err("PICNT conflict with I2C7_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI3 == MUX_1) {
				pr_err("PICNT conflict with SPI3_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF2 == MUX_1) {
				pr_err("PICNT conflict with SIF2_1\r\n");
				return E_PAR;
			}

			top_reg7.bit.PICNT = MUX_1;
			top_reg_pgpio0.bit.PGPIO_21 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_21,1,func_CCNT);
		}

		if (config & PIN_CCNT_CFG_CCNT2_1) {
			if ((top_reg13.bit.UART4 == MUX_1) &&
			    (top_reg14.bit.UART4_RTSCTS == MUX_1 || top_reg14.bit.UART4_RTSCTS == MUX_2)) {
				pr_err("PICNT2 conflict with UART4_1 RTS\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C8 == MUX_1) {
				pr_err("PICNT2 conflict with I2C8_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI3 == MUX_1) {
				pr_err("PICNT2 conflict with SPI3_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF2 == MUX_1) {
				pr_err("PICNT2 conflict with SIF2_1\r\n");
				return E_PAR;
			}
			if (top_reg18.bit.RTC_DIV_OUT == MUX_1) {
				pr_err("PICNT2 conflict with RTC_DIV_OUT\r\n");
				return E_PAR;
			}

			top_reg7.bit.PICNT2 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_22 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_22,1,func_CCNT2);
		}

		if (config & PIN_CCNT_CFG_CCNT3_1) {
			if ((top_reg13.bit.UART4 == MUX_1) &&
			    (top_reg14.bit.UART4_RTSCTS == MUX_1)) {
				pr_err("PICNT3 conflict with UART4_1 CTS\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C8 == MUX_1) {
				pr_err("PICNT3 conflict with I2C8_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI3 == MUX_1 && top_reg16.bit.SPI3_BUS_WIDTH == MUX_1) {
				pr_err("PICNT3 conflict with SPI3_1 BUS_WIDTH\r\n");
				return E_PAR;
			}
			if (top_reg18.bit.RTC_EXT_CLK == MUX_1) {
				pr_err("PICNT3 conflict with RTC_EXT_CLK\r\n");
				return E_PAR;
			}

			top_reg7.bit.PICNT3 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_23 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_23,1,func_CCNT3);
		}
	}

	return E_OK;
}

static int pinmux_config_sensor(uint32_t config)
{
	if (config == PIN_SENSOR_CFG_NONE) {
	} else {
		if (config & PIN_SENSOR_CFG_12BITS) {
			if (top_reg9.bit.HSI2HSI3_SEL == MUX_0) {
				pr_err("(SENSOR = 0x0) conflict with HSI2HSI3_SEL as CSI/LVDS mode\r\n");
				return E_PAR;
			}

			top_reg9.bit.SENSOR = MUX_1;
			top_reg_hsigpio0.bit.HSIGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_5 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(HSIGPIO_0,12,func_SENSOR);
            gpio_func_keep(SGPIO_3,3,func_SENSOR);
		}

		if (config & PIN_SENSOR_CFG_SN3_MCLK_2) {
			if (top_reg9.bit.HSI2HSI3_SEL == MUX_0) {
				pr_err("SN3_MCLK_2 conflict with HSI2HSI3_SEL as CSI/LVDS mode\r\n");
				return E_PAR;
			}

			top_reg10.bit.SN3_MCLK = MUX_2;
			top_reg_hsigpio0.bit.HSIGPIO_15 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(HSIGPIO_15,1,func_SENSOR);
		}

		if (config & PIN_SENSOR_CFG_SN4_MCLK_2) {
			if (top_reg9.bit.HSI2HSI3_SEL == MUX_0) {
				pr_err("SN4_MCLK_2 conflict with HSI2HSI3_SEL as CSI/LVDS mode\r\n");
				return E_PAR;
			}

			top_reg10.bit.SN4_MCLK = MUX_2;
			top_reg_hsigpio0.bit.HSIGPIO_16 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(HSIGPIO_16,1,func_SENSOR);
		}
	}

	return E_OK;
}

static int pinmux_config_sensor2(uint32_t config)
{
	if (config == PIN_SENSOR2_CFG_NONE) {
	} else {
		if (config & PIN_SENSOR2_CFG_12BITS) {
			if (top_reg10.bit.SN2_XVSXHS == MUX_1) {
				pr_err("(SENSOR2 = 0x1) conflict with SN2_XVSXHS\r\n");
				return E_PAR;
			}
			if (top_reg10.bit.SN3_XVSXHS == MUX_1) {
				pr_err("(SENSOR2 = 0x1) conflict with SN3_XVSXHS\r\n");
				return E_PAR;
			}
			if (top_reg10.bit.SN4_XVSXHS == MUX_1) {
				pr_err("(SENSOR2 = 0x1) conflict with SN4_XVSXHS\r\n");
				return E_PAR;
			}
			if (top_reg10.bit.SN5_XVSXHS == MUX_1) {
				pr_err("(SENSOR2 = 0x1) conflict with SN5_XVSXHS\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C8 == MUX_2) {
				pr_err("(SENSOR2 = 0x1) conflict with I2C8_2\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C3 == MUX_2) {
				pr_err("(SENSOR2 = 0x1) conflict with I2C3_2\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C4 == MUX_2) {
				pr_err("(SENSOR2 = 0x1) conflict with I2C4_2\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C5 == MUX_2) {
				pr_err("(SENSOR2 = 0x1) conflict with I2C5_2\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI == MUX_2) {
				pr_err("(SENSOR2 = 0x1) conflict with SPI_2\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF2 == MUX_2) {
				pr_err("(SENSOR2 = 0x1) conflict with SIF2_2\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF3 == MUX_2) {
				pr_err("(SENSOR2 = 0x1) conflict with SIF3_2\r\n");
				return E_PAR;
			}

			top_reg9.bit.SENSOR2 = MUX_1;
			top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_8 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(SGPIO_6,15,func_SENSOR2);
		} else if (config & PIN_SENSOR2_CFG_CCIR8BITS_A) {
			if (top_reg10.bit.SN4_XVSXHS == MUX_1) {
				pr_err("(SENSOR2 = 0x2) conflict with SN4_XVSXHS_1\r\n");
				return E_PAR;
			}
			if (top_reg10.bit.SN5_XVSXHS == MUX_1) {
				pr_err("(SENSOR2 = 0x2) conflict with SN5_XVSXHS_1\r\n");
				return E_PAR;
			}
			if (top_reg10.bit.SN4_MCLK == MUX_1) {
				pr_err("(SENSOR2 = 0x2) conflict with SN4_MCLK_1\r\n");
				return E_PAR;
			}
			if (top_reg10.bit.SN5_MCLK == MUX_1) {
				pr_err("(SENSOR2 = 0x2) conflict with SN5_MCLK_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI == MUX_2) {
				pr_err("(SENSOR2 = 0x2) conflict with SPI_2\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C6 == MUX_2) {
				pr_err("(SENSOR2 = 0x2) conflict with I2C6_2\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF4 == MUX_2) {
				pr_err("(SENSOR2 = 0x2) conflict with SIF4_2\r\n");
				return E_PAR;
			}

			top_reg9.bit.SENSOR2 = MUX_2;
			top_reg_sgpio0.bit.SGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_24 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_6 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(SGPIO_6,1,func_SENSOR2);
            gpio_func_keep(SGPIO_17,8,func_SENSOR2);
		} else if (config & PIN_SENSOR2_CFG_CCIR8BITS_B) {
			if (top_reg10.bit.SN3_MCLK == MUX_1) {
				pr_err("(SENSOR2 = 0x3) conflict with SN3_MCLK_1\r\n");
				return E_PAR;
			}
			if (top_reg10.bit.SN3_XVSXHS == MUX_1) {
				pr_err("(SENSOR2 = 0x3) conflict with SN3_XVSXHS_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C3 == MUX_2) {
				pr_err("(SENSOR2 = 0x3) conflict with I2C3_2\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C4 == MUX_2) {
				pr_err("(SENSOR2 = 0x3) conflict with I2C4_2\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C5 == MUX_2) {
				pr_err("(SENSOR2 = 0x3) conflict with I2C5_2\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF2 == MUX_2) {
				pr_err("(SENSOR2 = 0x3) conflict with SIF2_2\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF3 == MUX_2) {
				pr_err("(SENSOR2 = 0x3) conflict with SIF3_2\r\n");
				return E_PAR;
			}

			top_reg9.bit.SENSOR2 = MUX_3;
			top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_2 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(SGPIO_2,1,func_SENSOR2);
            gpio_func_keep(SGPIO_9,8,func_SENSOR2);
		} else if (config & PIN_SENSOR2_CFG_CCIR8BITS_AB) {
			if (top_reg10.bit.SN3_MCLK == MUX_1) {
				pr_err("(SENSOR2 = 0x4) conflict with SN3_MCLK_1\r\n");
				return E_PAR;
			}
			if (top_reg10.bit.SN3_XVSXHS == MUX_1) {
				pr_err("(SENSOR2 = 0x4) conflict with SN3_XVSXHS_1\r\n");
				return E_PAR;
			}
			if (top_reg10.bit.SN4_XVSXHS == MUX_1) {
				pr_err("(SENSOR2 = 0x4) conflict with SN4_XVSXHS_1\r\n");
				return E_PAR;
			}
			if (top_reg10.bit.SN5_XVSXHS == MUX_1) {
				pr_err("(SENSOR2 = 0x4) conflict with SN5_XVSXHS_1\r\n");
				return E_PAR;
			}
			if (top_reg10.bit.SN4_MCLK == MUX_1) {
				pr_err("(SENSOR2 = 0x4) conflict with SN4_MCLK_1\r\n");
				return E_PAR;
			}
			if (top_reg10.bit.SN5_MCLK == MUX_1) {
				pr_err("(SENSOR2 = 0x4) conflict with SN5_MCLK_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C3 == MUX_2) {
				pr_err("(SENSOR2 = 0x4) conflict with I2C3_2\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C4 == MUX_2) {
				pr_err("(SENSOR2 = 0x4) conflict with I2C4_2\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C5 == MUX_2) {
				pr_err("(SENSOR2 = 0x4) conflict with I2C5_2\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI == MUX_2) {
				pr_err("(SENSOR2 = 0x4) conflict with SPI_2\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C6 == MUX_2) {
				pr_err("(SENSOR2 = 0x4) conflict with I2C6_2\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF2 == MUX_2) {
				pr_err("(SENSOR2 = 0x4) conflict with SIF2_2\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF3 == MUX_2) {
				pr_err("(SENSOR2 = 0x4) conflict with SIF3_2\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF4 == MUX_2) {
				pr_err("(SENSOR2 = 0x4) conflict with SIF4_2\r\n");
				return E_PAR;
			}

			top_reg9.bit.SENSOR2 = MUX_4;
			top_reg_sgpio0.bit.SGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_24 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_2 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(SGPIO_2,1,func_SENSOR2);
            gpio_func_keep(SGPIO_6,1,func_SENSOR2);
            gpio_func_keep(SGPIO_9,16,func_SENSOR2);
		} else if (config & PIN_SENSOR2_CFG_CCIR16BITS) {
			if (top_reg10.bit.SN3_XVSXHS == MUX_1) {
				pr_err("(SENSOR2 = 0x5) conflict with SN3_XVSXHS_1\r\n");
				return E_PAR;
			}
			if (top_reg10.bit.SN4_XVSXHS == MUX_1) {
				pr_err("(SENSOR2 = 0x5) conflict with SN4_XVSXHS_1\r\n");
				return E_PAR;
			}
			if (top_reg10.bit.SN5_XVSXHS == MUX_1) {
				pr_err("(SENSOR2 = 0x5) conflict with SN5_XVSXHS_1\r\n");
				return E_PAR;
			}
			if (top_reg10.bit.SN4_MCLK == MUX_1) {
				pr_err("(SENSOR2 = 0x5) conflict with SN4_MCLK_1\r\n");
				return E_PAR;
			}
			if (top_reg10.bit.SN5_MCLK == MUX_1) {
				pr_err("(SENSOR2 = 0x5) conflict with SN5_MCLK_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C3 == MUX_2) {
				pr_err("(SENSOR2 = 0x5) conflict with I2C3_2\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C4 == MUX_2) {
				pr_err("(SENSOR2 = 0x5) conflict with I2C4_2\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C5 == MUX_2) {
				pr_err("(SENSOR2 = 0x5) conflict with I2C5_2\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI == MUX_2) {
				pr_err("(SENSOR2 = 0x5) conflict with SPI_2\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C6 == MUX_2) {
				pr_err("(SENSOR2 = 0x5) conflict with I2C6_2\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF2 == MUX_2) {
				pr_err("(SENSOR2 = 0x5) conflict with SIF2_2\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF3 == MUX_2) {
				pr_err("(SENSOR2 = 0x5) conflict with SIF3_2\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF4 == MUX_2) {
				pr_err("(SENSOR2 = 0x5) conflict with SIF4_2\r\n");
				return E_PAR;
			}

			top_reg9.bit.SENSOR2 = MUX_5;
			top_reg_sgpio0.bit.SGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_24 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_6 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(SGPIO_6,1,func_SENSOR2);
            gpio_func_keep(SGPIO_9,16,func_SENSOR2);

		}

		if (config & PIN_SENSOR2_CFG_CCIR_VSHS) {
			if (top_reg10.bit.SN3_MCLK == MUX_1) {
				pr_err("SN2_CCIR_VSHS conflict with SN3_MCLK_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C8 == MUX_2) {
				pr_err("SN2_CCIR_VSHS conflict with I2C8_2\r\n");
				return E_PAR;
			}

			top_reg9.bit.SN2_CCIR_VSHS = MUX_1;
			top_reg_sgpio0.bit.SGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_2 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(SGPIO_7,2,func_SENSOR2);
            gpio_func_keep(SGPIO_2,1,func_SENSOR2);
		}

		if (config & PIN_SENSOR2_CFG_SN1_MCLK_1) {
			top_reg10.bit.SN_MCLK = MUX_1;
			top_reg_sgpio0.bit.SGPIO_0 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(SGPIO_0,1,func_SENSOR2);
		}

		if (config & PIN_SENSOR2_CFG_SN2_MCLK_1) {
			top_reg10.bit.SN2_MCLK = MUX_1;
			top_reg_sgpio0.bit.SGPIO_1 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(SGPIO_1,1,func_SENSOR2);
		}
	}

	return E_OK;
}

static int pinmux_config_sensor3(uint32_t config)
{
	if (config == PIN_SENSOR3_CFG_NONE) {
	} else {
		if (config & PIN_SENSOR3_CFG_12BITS) {
			if (top_reg14.bit.UART9 == MUX_1) {
				pr_err("(SENSOR3 = 0x1) conflict with UART9_1\r\n");
				return E_PAR;
			}
			if (top_reg13.bit.UART8 == MUX_1) {
				pr_err("(SENSOR3 = 0x1) conflict with UART8_1\r\n");
				return E_PAR;
			}
			if (top_reg13.bit.UART7 == MUX_1) {
				pr_err("(SENSOR3 = 0x1) conflict with UART7_1\r\n");
				return E_PAR;
			}
			if (top_reg13.bit.UART6 == MUX_1) {
				pr_err("(SENSOR3 = 0x1) conflict with UART6_1\r\n");
				return E_PAR;
			}
			if (top_reg6.bit.PWM0 == MUX_1 || top_reg6.bit.PWM1 == MUX_1 ||
				top_reg6.bit.PWM2 == MUX_1 || top_reg6.bit.PWM3 == MUX_1 ||
				top_reg6.bit.PWM4 == MUX_1 || top_reg6.bit.PWM5 == MUX_1 ||
				top_reg6.bit.PWM6 == MUX_1 || top_reg6.bit.PWM7 == MUX_1 ||
				top_reg7.bit.PWM8 == MUX_1 || top_reg7.bit.PWM9 == MUX_1 ||
				top_reg7.bit.PWM10 == MUX_1 || top_reg7.bit.PWM11 == MUX_1) {
				pr_err("(SENSOR3 = 0x1) conflict with PWM0~PWM11\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C3 == MUX_1) {
				pr_err("(SENSOR3 = 0x1) conflict with I2C3_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C4 == MUX_1) {
				pr_err("(SENSOR3 = 0x1) conflict with I2C4_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI == MUX_1) {
				pr_err("(SENSOR3 = 0x1) conflict with SPI_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF5 == MUX_1) {
				pr_err("(SENSOR3 = 0x1) conflict with SIF5_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF0 == MUX_1) {
				pr_err("(SENSOR3 = 0x1) conflict with SIF0_1\r\n");
				return E_PAR;
			}

			top_reg9.bit.SENSOR3 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_0,15,func_SENSOR3);
		} else if (config & PIN_SENSOR3_CFG_CCIR8BITS_A) {
			if (top_reg14.bit.UART9 == MUX_1) {
				pr_err("(SENSOR3 = 0x2) conflict with UART9_1\r\n");
				return E_PAR;
			}
			if (top_reg13.bit.UART8 == MUX_1) {
				pr_err("(SENSOR3 = 0x2) conflict with UART8_1\r\n");
				return E_PAR;
			}
			if (top_reg13.bit.UART7 == MUX_1) {
				pr_err("(SENSOR3 = 0x2) conflict with UART7_1\r\n");
				return E_PAR;
			}
			if (top_reg6.bit.PWM0 == MUX_1 || top_reg6.bit.PWM1 == MUX_1 ||
				top_reg6.bit.PWM2 == MUX_1 || top_reg6.bit.PWM3 == MUX_1 ||
				top_reg6.bit.PWM4 == MUX_1 || top_reg6.bit.PWM5 == MUX_1 ||
				top_reg6.bit.PWM6 == MUX_1 || top_reg6.bit.PWM7 == MUX_1 ||
				top_reg7.bit.PWM8 == MUX_1) {
				pr_err("(SENSOR3 = 0x2) conflict with PWM0~PWM8\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF5 == MUX_1) {
				pr_err("(SENSOR3 = 0x2) conflict with SIF5_1\r\n");
				return E_PAR;
			}

			top_reg9.bit.SENSOR3 = MUX_2;
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_0,9,func_SENSOR3);
		} else if (config & PIN_SENSOR3_CFG_CCIR8BITS_B) {
			if ((top_reg13.bit.UART7 == MUX_1) &&
			    (top_reg14.bit.UART7_RTSCTS == MUX_1)) {
				pr_err("(SENSOR3 = 0x3) conflict with UART7_1 CTS\r\n");
				return E_PAR;
			}
			if (top_reg13.bit.UART6 == MUX_1) {
				pr_err("(SENSOR3 = 0x3) conflict with UART6\r\n");
				return E_PAR;
			}
			if (top_reg13.bit.UART5 == MUX_1) {
				pr_err("(SENSOR3 = 0x3) conflict with UART5\r\n");
				return E_PAR;
			}
			if (top_reg7.bit.PWM11 == MUX_1) {
				pr_err("(SENSOR3 = 0x3) conflict with PWM11\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C3 == MUX_1) {
				pr_err("(SENSOR3 = 0x3) conflict with I2C3_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C4 == MUX_1) {
				pr_err("(SENSOR3 = 0x3) conflict with I2C4_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C5 == MUX_1) {
				pr_err("(SENSOR3 = 0x3) conflict with I2C5_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C6 == MUX_1) {
				pr_err("(SENSOR3 = 0x3) conflict with I2C6_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI == MUX_1) {
				pr_err("(SENSOR3 = 0x3) conflict with SPI_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI2 == MUX_1) {
				pr_err("(SENSOR3 = 0x3) conflict with SPI2_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF0 == MUX_1) {
				pr_err("(SENSOR3 = 0x3) conflict with SIF0_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF1 == MUX_1) {
				pr_err("(SENSOR3 = 0x3) conflict with SIF1_1\r\n");
				return E_PAR;
			}
			if (top_reg15.bit.SDP == MUX_1) {
				pr_err("(SENSOR3 = 0x3) conflict with SDP_1\r\n");
				return E_PAR;
			}

			top_reg9.bit.SENSOR3 = MUX_3;
			top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_11,9,func_SENSOR3);
		} else if (config & PIN_SENSOR3_CFG_CCIR8BITS_AB) {
			if (top_reg14.bit.UART9 == MUX_1) {
				pr_err("(SENSOR3 = 0x4) conflict with UART9_1\r\n");
				return E_PAR;
			}
			if (top_reg13.bit.UART8 == MUX_1) {
				pr_err("(SENSOR3 = 0x4) conflict with UART8_1\r\n");
				return E_PAR;
			}
			if (top_reg13.bit.UART7 == MUX_1) {
				pr_err("(SENSOR3 = 0x4) conflict with UART7_1\r\n");
				return E_PAR;
			}
			if (top_reg13.bit.UART6 == MUX_1) {
				pr_err("(SENSOR3 = 0x4) conflict with UART6_1\r\n");
				return E_PAR;
			}
			if (top_reg13.bit.UART5 == MUX_1) {
				pr_err("(SENSOR3 = 0x4) conflict with UART5_1\r\n");
				return E_PAR;
			}
			if (top_reg6.bit.PWM0 == MUX_1 || top_reg6.bit.PWM1 == MUX_1 ||
				top_reg6.bit.PWM2 == MUX_1 || top_reg6.bit.PWM3 == MUX_1 ||
				top_reg6.bit.PWM4 == MUX_1 || top_reg6.bit.PWM5 == MUX_1 ||
				top_reg6.bit.PWM6 == MUX_1 || top_reg6.bit.PWM7 == MUX_1 ||
				top_reg7.bit.PWM8 == MUX_1 || top_reg7.bit.PWM9 == MUX_1 ||
				top_reg7.bit.PWM10 == MUX_1 || top_reg7.bit.PWM11 == MUX_1) {
				pr_err("(SENSOR3 = 0x4) conflict with PWM0~PWM11\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C3 == MUX_1) {
				pr_err("(SENSOR3 = 0x4) conflict with I2C3_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C4 == MUX_1) {
				pr_err("(SENSOR3 = 0x4) conflict with I2C4_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C5 == MUX_1) {
				pr_err("(SENSOR3 = 0x4) conflict with I2C5_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C6 == MUX_1) {
				pr_err("(SENSOR3 = 0x4) conflict with I2C6_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI == MUX_1) {
				pr_err("(SENSOR3 = 0x4) conflict with SPI_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI2 == MUX_1) {
				pr_err("(SENSOR3 = 0x4) conflict with SPI2_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF5 == MUX_1) {
				pr_err("(SENSOR3 = 0x4) conflict with SIF5_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF0 == MUX_1) {
				pr_err("(SENSOR3 = 0x4) conflict with SIF0_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF1 == MUX_1) {
				pr_err("(SENSOR3 = 0x4) conflict with SIF1_1\r\n");
				return E_PAR;
			}
			if (top_reg15.bit.SDP == MUX_1) {
				pr_err("(SENSOR3 = 0x4) conflict with SDP_1\r\n");
				return E_PAR;
			}

			top_reg9.bit.SENSOR3 = MUX_4;
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_0,9,func_SENSOR3);
            gpio_func_keep(PGPIO_11,9,func_SENSOR3);
		} else if (config & PIN_SENSOR3_CFG_CCIR16BITS) {
			if (top_reg14.bit.UART9 == MUX_1) {
				pr_err("(SENSOR3 = 0x5) conflict with UART9_1\r\n");
				return E_PAR;
			}
			if (top_reg13.bit.UART8 == MUX_1) {
				pr_err("(SENSOR3 = 0x5) conflict with UART8_1\r\n");
				return E_PAR;
			}
			if (top_reg13.bit.UART7 == MUX_1) {
				pr_err("(SENSOR3 = 0x5) conflict with UART7_1\r\n");
				return E_PAR;
			}
			if (top_reg13.bit.UART6 == MUX_1) {
				pr_err("(SENSOR3 = 0x5) conflict with UART6_1\r\n");
				return E_PAR;
			}
			if (top_reg13.bit.UART5 == MUX_1) {
				pr_err("(SENSOR3 = 0x5) conflict with UART5_1\r\n");
				return E_PAR;
			}
			if (top_reg6.bit.PWM0 == MUX_1 || top_reg6.bit.PWM1 == MUX_1 ||
				top_reg6.bit.PWM2 == MUX_1 || top_reg6.bit.PWM3 == MUX_1 ||
				top_reg6.bit.PWM4 == MUX_1 || top_reg6.bit.PWM5 == MUX_1 ||
				top_reg6.bit.PWM6 == MUX_1 || top_reg6.bit.PWM7 == MUX_1 ||
				top_reg7.bit.PWM8 == MUX_1 || top_reg7.bit.PWM9 == MUX_1 ||
				top_reg7.bit.PWM10 == MUX_1 || top_reg7.bit.PWM11 == MUX_1) {
				pr_err("(SENSOR3 = 0x5) conflict with PWM0~PWM11\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C3 == MUX_1) {
				pr_err("(SENSOR3 = 0x5) conflict with I2C3_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C4 == MUX_1) {
				pr_err("(SENSOR3 = 0x5) conflict with I2C4_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C5 == MUX_1) {
				pr_err("(SENSOR3 = 0x5) conflict with I2C5_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C6 == MUX_1) {
				pr_err("(SENSOR3 = 0x5) conflict with I2C6_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI == MUX_1) {
				pr_err("(SENSOR3 = 0x5) conflict with SPI_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI2 == MUX_1) {
				pr_err("(SENSOR3 = 0x5) conflict with SPI2_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF5 == MUX_1) {
				pr_err("(SENSOR3 = 0x5) conflict with SIF5_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF0 == MUX_1) {
				pr_err("(SENSOR3 = 0x5) conflict with SIF0_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF1 == MUX_1) {
				pr_err("(SENSOR3 = 0x5) conflict with SIF1_1\r\n");
				return E_PAR;
			}
			if (top_reg15.bit.SDP == MUX_1) {
				pr_err("(SENSOR3 = 0x5) conflict with SDP_1\r\n");
				return E_PAR;
			}

			top_reg9.bit.SENSOR3 = MUX_5;
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_0,9,func_SENSOR3);
            gpio_func_keep(PGPIO_12,8,func_SENSOR3);
		}

		if (config & PIN_SENSOR3_CFG_CCIR_VSHS) {
			if (top_reg13.bit.UART7 == MUX_1) {
				pr_err("SN3_CCIR_VSHS conflict with UART7_1\r\n");
				return E_PAR;
			}
			if (top_reg7.bit.PWM9 == MUX_1 || top_reg7.bit.PWM10 == MUX_1 || top_reg7.bit.PWM11 == MUX_1) {
				pr_err("SN3_CCIR_VSHS conflict with PWM9~PWM11\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF5 == MUX_1) {
				pr_err("SN3_CCIR_VSHS conflict with SIF5_1\r\n");
				return E_PAR;
			}

			top_reg9.bit.SN3_CCIR_VSHS = MUX_1;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_9,3,func_SENSOR3);
		}

		if (config & PIN_SENSOR3_CFG_SN5_MCLK_2) {
			if (top_reg13.bit.UART4 == MUX_1) {
				pr_err("SN5_MCLK_2 conflict with UART4_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C7 == MUX_1) {
				pr_err("SN5_MCLK_2 conflict with I2C7_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI3 == MUX_1) {
				pr_err("SN5_MCLK_2 conflict with SPI3_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF2 == MUX_1) {
				pr_err("SN5_MCLK_2 conflict with SIF2_1\r\n");
				return E_PAR;
			}

			top_reg10.bit.SN5_MCLK = MUX_2;
			top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_20,1,func_SENSOR3);
		}
	}

	return E_OK;
}

static int pinmux_config_sensormisc(uint32_t config)
{
	if (config == PIN_SENSORMISC_CFG_NONE) {
	} else {
		/* MCLK */
		if (config & PIN_SENSORMISC_CFG_SN_MCLK_1) {
			top_reg10.bit.SN_MCLK = MUX_1;
			top_reg_sgpio0.bit.SGPIO_0 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(SGPIO_0,1,func_SN1_MCLK);
		}

		if (config & PIN_SENSORMISC_CFG_SN2_MCLK_1) {
			top_reg10.bit.SN2_MCLK = MUX_1;
			top_reg_sgpio0.bit.SGPIO_1 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(SGPIO_1,1,func_SN2_MCLK);
		}

		if (config & PIN_SENSORMISC_CFG_SN3_MCLK_1) {
			if (top_reg9.bit.SENSOR2 == MUX_3 || top_reg9.bit.SENSOR2 == MUX_4) {
				pr_err("SN3_MCLK_1 conflict with SENSOR2\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SN2_CCIR_VSHS == MUX_1) {
				pr_err("SN3_MCLK_1 conflict with SN2_CCIR_VSHS\r\n");
				return E_PAR;
			}

			top_reg10.bit.SN3_MCLK = MUX_1;
			top_reg_sgpio0.bit.SGPIO_2 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(SGPIO_2,1,func_SN3_MCLK);
		} else if (config & PIN_SENSORMISC_CFG_SN3_MCLK_2) {
			if (top_reg9.bit.HSI2HSI3_SEL == MUX_0) {
				pr_err("SN3_MCLK_2 conflict with HSI2HSI3_SEL as CSI/LVDS mode\r\n");
				return E_PAR;
			}

			top_reg10.bit.SN3_MCLK = MUX_2;
			top_reg_hsigpio0.bit.HSIGPIO_15 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(HSIGPIO_15,1,func_SN3_MCLK);
		}

		if (config & PIN_SENSORMISC_CFG_SN4_MCLK_1) {
			if (top_reg9.bit.SENSOR2 == MUX_2 || top_reg9.bit.SENSOR2 == MUX_4 || top_reg9.bit.SENSOR2 == MUX_5) {
				pr_err("SN4_MCLK_1 conflict with SENSOR2\r\n");
				return E_PAR;
			}

			top_reg10.bit.SN4_MCLK = MUX_1;
			top_reg_sgpio0.bit.SGPIO_21 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(SGPIO_21,1,func_SN4_MCLK);
		} else if (config & PIN_SENSORMISC_CFG_SN4_MCLK_2) {
			if (top_reg9.bit.HSI2HSI3_SEL == MUX_0) {
				pr_err("SN4_MCLK_2 conflict with HSI2HSI3_SEL as CSI/LVDS mode\r\n");
				return E_PAR;
			}

			top_reg10.bit.SN4_MCLK = MUX_2;
			top_reg_hsigpio0.bit.HSIGPIO_16 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(HSIGPIO_16,1,func_SN4_MCLK);
		}

		if (config & PIN_SENSORMISC_CFG_SN5_MCLK_1) {
			if (top_reg9.bit.SENSOR2 == MUX_2 || top_reg9.bit.SENSOR2 == MUX_4 || top_reg9.bit.SENSOR2 == MUX_5) {
				pr_err("SN5_MCLK_1 conflict with SENSOR2\r\n");
				return E_PAR;
			}

			top_reg10.bit.SN5_MCLK = MUX_1;
			top_reg_sgpio0.bit.SGPIO_22 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(SGPIO_22,1,func_SN5_MCLK);
		} else if (config & PIN_SENSORMISC_CFG_SN5_MCLK_2) {
			if (top_reg13.bit.UART4 == MUX_1) {
				pr_err("SN5_MCLK_2 conflict with UART4_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C7 == MUX_1) {
				pr_err("SN5_MCLK_2 conflict with I2C7_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI3 == MUX_1) {
				pr_err("SN5_MCLK_2 conflict with SPI3_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF2 == MUX_1) {
				pr_err("SN5_MCLK_2 conflict with SIF2_1\r\n");
				return E_PAR;
			}

			top_reg10.bit.SN5_MCLK = MUX_2;
			top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_20,1,func_SN5_MCLK);
		}

		/* XVSXHS */
		if (config & PIN_SENSORMISC_CFG_SN_XVSXHS_1) {
			top_reg10.bit.SN_XVSXHS = MUX_1;
			top_reg_sgpio0.bit.SGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_5 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(SGPIO_4,2,func_SN1_XVSXHS);
		}

		if (config & PIN_SENSORMISC_CFG_SN2_XVSXHS_1) {
			if (top_reg4.bit.I2C8 == MUX_2) {
				pr_err("SN2_XVSXHS_1 conflict with I2C8_2\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SN2_CCIR_VSHS == MUX_1) {
				pr_err("SN2_XVSXHS_1 conflict with SN2_CCIR_VSHS\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR2 == MUX_1) {
				pr_err("SN2_XVSXHS_1 conflict with SENSOR2\r\n");
				return E_PAR;
			}

			top_reg10.bit.SN2_XVSXHS = MUX_1;
			top_reg_sgpio0.bit.SGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_8 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(SGPIO_7,2,func_SN2_XVSXHS);
		}

		if (config & PIN_SENSORMISC_CFG_SN3_XVSXHS_1) {
			if (top_reg9.bit.SENSOR2 == MUX_1 || top_reg9.bit.SENSOR2 == MUX_3 ||
				top_reg9.bit.SENSOR2 == MUX_4 || top_reg9.bit.SENSOR2 == MUX_5) {
				pr_err("SN3_XVSXHS_1 conflict with SENSOR2\r\n");
				return E_PAR;
			}

			top_reg10.bit.SN3_XVSXHS = MUX_1;
			top_reg_sgpio0.bit.SGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_10 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(SGPIO_9,2,func_SN3_XVSXHS);
		}

		if (config & PIN_SENSORMISC_CFG_SN4_XVSXHS_1) {
			if (top_reg16.bit.SPI2 == MUX_2) {
				pr_err("SN4_XVSXHS_1 conflict with SPI2_2\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR2 == MUX_1 || top_reg9.bit.SENSOR2 == MUX_2 ||
				top_reg9.bit.SENSOR2 == MUX_4 || top_reg9.bit.SENSOR2 == MUX_5) {
				pr_err("SN4_XVSXHS_1 conflict with SENSOR2\r\n");
				return E_PAR;
			}

			top_reg10.bit.SN4_XVSXHS = MUX_1;
			top_reg_sgpio0.bit.SGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_18 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(SGPIO_17,2,func_SN4_XVSXHS);
		}

		if (config & PIN_SENSORMISC_CFG_SN5_XVSXHS_1) {
			if (top_reg16.bit.SPI2 == MUX_2) {
				pr_err("SN5_XVSXHS_1 conflict with SPI2_2\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR2 == MUX_1 || top_reg9.bit.SENSOR2 == MUX_2 ||
				top_reg9.bit.SENSOR2 == MUX_4 || top_reg9.bit.SENSOR2 == MUX_5) {
				pr_err("SN5_XVSXHS_1 conflict with SENSOR2\r\n");
				return E_PAR;
			}

			top_reg10.bit.SN5_XVSXHS = MUX_1;
			top_reg_sgpio0.bit.SGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_20 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(SGPIO_19,2,func_SN5_XVSXHS);
		}

		/* FLASH */
		if (config & PIN_SENSORMISC_CFG_FLASH_TRIG_IN_1) {
			if (top_reg4.bit.I2C7 == MUX_2) {
				pr_err("FLASH_TRIG_IN_1 conflict with I2C7_2\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF4 == MUX_2) {
				pr_err("FLASH_TRIG_IN_1 conflict with SIF4_2\r\n");
				return E_PAR;
			}

			top_reg9.bit.FLASH_TRIG_IN = MUX_1;
			top_reg_sgpio0.bit.SGPIO_25 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(SGPIO_25,1,func_SENSORMISC);
		} else if (config & PIN_SENSORMISC_CFG_FLASH_TRIG_IN_2) {
			if (top_reg9.bit.HSI2HSI3_SEL == MUX_0) {
				pr_err("FLASH_TRIG_IN_2 conflict with HSI2HSI3_SEL as CSI/LVDS mode\r\n");
				return E_PAR;
			}

			top_reg9.bit.FLASH_TRIG_IN = MUX_2;
			top_reg_hsigpio0.bit.HSIGPIO_17 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(HSIGPIO_17,1,func_SENSORMISC);
		}

		if (config & PIN_SENSORMISC_CFG_FLASH_TRIG_OUT_1) {
			if (top_reg4.bit.I2C7 == MUX_2) {
				pr_err("FLASH_TRIG_OUT_1 conflict with I2C7_2\r\n");
				return E_PAR;
			}

			top_reg9.bit.FLASH_TRIG_OUT = MUX_1;
			top_reg_sgpio0.bit.SGPIO_26 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(SGPIO_26,1,func_SENSORMISC);
		} else if (config & PIN_SENSORMISC_CFG_FLASH_TRIG_OUT_2) {
			if (top_reg9.bit.HSI2HSI3_SEL == MUX_0) {
				pr_err("FLASH_TRIG_OUT_2 conflict with HSI2HSI3_SEL as CSI/LVDS mode\r\n");
				return E_PAR;
			}

			top_reg9.bit.FLASH_TRIG_OUT = MUX_2;
			top_reg_hsigpio0.bit.HSIGPIO_18 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(HSIGPIO_18,1,func_SENSORMISC);
		}
	}

	return E_OK;
}

static int pinmux_config_sensorsync(uint32_t config)
{
	if (config == PIN_SENSORSYNC_CFG_NONE) {
	} else {
		/* MCLK source sync */
		if (config & PIN_SENSORSYNC_CFG_SN2_MCLKSRC_SN) {
			top_reg11.bit.SN2_MCLK_SRC = MUX_1;
		}

		if (config & PIN_SENSORSYNC_CFG_SN3_MCLKSRC_SN) {
			top_reg11.bit.SN3_MCLK_SRC = MUX_1;
		} else if (config & PIN_SENSORSYNC_CFG_SN3_MCLKSRC_SN2) {
			top_reg11.bit.SN3_MCLK_SRC = MUX_2;
		}

		if (config & PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN) {
			top_reg11.bit.SN4_MCLK_SRC = MUX_1;
		} else if (config & PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN2) {
			top_reg11.bit.SN4_MCLK_SRC = MUX_2;
		} else if (config & PIN_SENSORSYNC_CFG_SN4_MCLKSRC_SN3) {
			top_reg11.bit.SN4_MCLK_SRC = MUX_3;
		}

		if (config & PIN_SENSORSYNC_CFG_SN5_MCLKSRC_SN) {
			top_reg11.bit.SN5_MCLK_SRC = MUX_1;
		} else if (config & PIN_SENSORSYNC_CFG_SN5_MCLKSRC_SN2) {
			top_reg11.bit.SN5_MCLK_SRC = MUX_2;
		} else if (config & PIN_SENSORSYNC_CFG_SN5_MCLKSRC_SN3) {
			top_reg11.bit.SN5_MCLK_SRC = MUX_3;
		} else if (config & PIN_SENSORSYNC_CFG_SN5_MCLKSRC_SN4) {
			top_reg11.bit.SN5_MCLK_SRC = MUX_4;
		}

		/* XVSXHS source sync */
		if (config & PIN_SENSORSYNC_CFG_SN2_XVSXHSSRC_SN) {
			top_reg11.bit.SN2_XVSHS_SRC = MUX_1;
		}

		if (config & PIN_SENSORSYNC_CFG_SN3_XVSXHSSRC_SN) {
			top_reg11.bit.SN3_XVSHS_SRC = MUX_1;
		} else if (config & PIN_SENSORSYNC_CFG_SN3_XVSXHSSRC_SN2) {
			top_reg11.bit.SN3_XVSHS_SRC = MUX_2;
		}

		if (config & PIN_SENSORSYNC_CFG_SN4_XVSXHSSRC_SN) {
			top_reg11.bit.SN4_XVSHS_SRC = MUX_1;
		} else if (config & PIN_SENSORSYNC_CFG_SN4_XVSXHSSRC_SN2) {
			top_reg11.bit.SN4_XVSHS_SRC = MUX_2;
		} else if (config & PIN_SENSORSYNC_CFG_SN4_XVSXHSSRC_SN3) {
			top_reg11.bit.SN4_XVSHS_SRC = MUX_3;
		}

		if (config & PIN_SENSORSYNC_CFG_SN5_XVSXHSSRC_SN) {
			top_reg11.bit.SN5_XVSHS_SRC = MUX_1;
		} else if (config & PIN_SENSORSYNC_CFG_SN5_XVSXHSSRC_SN2) {
			top_reg11.bit.SN5_XVSHS_SRC = MUX_2;
		} else if (config & PIN_SENSORSYNC_CFG_SN5_XVSXHSSRC_SN3) {
			top_reg11.bit.SN5_XVSHS_SRC = MUX_3;
		} else if (config & PIN_SENSORSYNC_CFG_SN5_XVSXHSSRC_SN4) {
			top_reg11.bit.SN5_XVSHS_SRC = MUX_4;
		}
	}

	return E_OK;
}

static int pinmux_config_mipi_lvds(uint32_t config)
{
#if 0
	if (config == PIN_MIPI_LVDS_CFG_NONE) {
	} else {
		if (config & PIN_MIPI_LVDS_CFG_HSI2HSI3_TO_CSI) {
			if (top_reg9.bit.SENSOR == MUX_1) {
				pr_err("MIPI-RX/LVDS conflict with (SENSOR3 = 0x1)\r\n");
				return E_PAR;
			}
			if (top_reg10.bit.SN3_MCLK == MUX_2) {
				pr_err("MIPI-RX/LVDS conflict with SN3_MCLK_2\r\n");
				return E_PAR;
			}
			if (top_reg10.bit.SN4_MCLK == MUX_2) {
				pr_err("MIPI-RX/LVDS conflict with SN4_MCLK_2\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.FLASH_TRIG_IN == MUX_2) {
				pr_err("MIPI-RX/LVDS conflict with FLASH_TRIG_IN_2\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.FLASH_TRIG_OUT == MUX_2) {
				pr_err("MIPI-RX/LVDS conflict with FLASH_TRIG_OUT_2\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.FLASH_TRIG_OUT == MUX_2) {
				pr_err("MIPI-RX/LVDS conflict with FLASH_TRIG_OUT_2\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C4 == MUX_4) {
				pr_err("MIPI-RX/LVDS conflict with I2C4_4\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C3 == MUX_3) {
				pr_err("MIPI-RX/LVDS conflict with I2C3_3\r\n");
				return E_PAR;
			}

			top_reg9.bit.HSI2HSI3_SEL = MUX_0;
			top_reg_hsigpio0.bit.HSIGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_hsigpio0.bit.HSIGPIO_23 = GPIO_ID_EMUM_FUNC;
			pad_set_pull_updown(PAD_PIN_HSIGPIO0, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO1, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO2, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO3, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO4, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO5, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO6, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO7, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO8, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO9, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO10, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO11, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO12, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO13, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO14, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO15, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO16, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO17, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO18, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO19, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO20, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO21, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO22, PAD_NONE);
			pad_set_pull_updown(PAD_PIN_HSIGPIO23, PAD_NONE);
            gpio_func_keep(HSIGPIO_0,24,func_MIPI);
		}

		if (config & PIN_MIPI_LVDS_CFG_MIPI_SEL) {
			top_reg2.bit.MIPI_SEL = MUX_1;
		}
	}
#endif
	return E_OK;
}

static int pinmux_config_audio(uint32_t config)
{
	if (config & PIN_AUDIO_CFG_NONE) {
	} else {
		if (config & PIN_AUDIO_CFG_I2S_1) {
			if (top_reg13.bit.UART3 == MUX_1) {
				pr_err("I2S_1 conflict with UART3_1\r\n");
				return E_PAR;
			}
			if (top_reg5.bit.I2C9 == MUX_1) {
				pr_err("I2S_1 conflict with I2C9_1\r\n");
				return E_PAR;
			}
			if (top_reg5.bit.I2C10 == MUX_1) {
				pr_err("I2S_1 conflict with I2C10_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI4 == MUX_1) {
				pr_err("I2S_1 conflict with SPI4_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF3 == MUX_1) {
				pr_err("I2S_1 conflict with SIF3_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI3_RDY == MUX_1) {
				pr_err("I2S_1 conflict with SPI3_RDY_1\r\n");
				return E_PAR;
			}
			if (top_reg12.bit.DMIC == MUX_1) {
				pr_err("I2S_1 conflict with DMIC\r\n");
				return E_PAR;
			}
			if (top_reg3.bit.ETH2_PTP == MUX_1) {
				pr_err("I2S_1 conflict with ETH2_PTP\r\n");
				return E_PAR;
			}

			top_reg12.bit.I2S = MUX_1;
			top_reg_pgpio0.bit.PGPIO_24 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_25 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_26 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_27 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_24,4,func_I2S_1);
		} else if (config & PIN_AUDIO_CFG_I2S_2) {
			if (top_reg1.bit.SDIO2_EN == MUX_1) {
				pr_err("I2S_2 conflict with SDIO2\r\n");
				return E_PAR;
			}
			if (top_reg13.bit.UART8 == MUX_2) {
				pr_err("I2S_2 conflict with UART8_2\r\n");
				return E_PAR;
			}
			if (top_reg1.bit.EXTROM_EN == MUX_1) {
				pr_err("I2S_2 conflict with EXTROM\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI3 == MUX_2) {
				pr_err("I2S_2 conflict with SPI3_2\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI3_RDY == MUX_2) {
				pr_err("I2S_2 conflict with SPI3_RDY_2\r\n");
				return E_PAR;
			}
			if (top_reg8.bit.DSP_EJTAG_EN == MUX_1) {
				pr_err("I2S_2 conflict with DSP_EJTAG_EN\r\n");
				return E_PAR;
			}
			if (top_reg15.bit.SDP == MUX_2) {
				pr_err("I2S_2 conflict with SDP_2\r\n");
				return E_PAR;
			}

			top_reg12.bit.I2S = MUX_2;
			top_reg_cgpio0.bit.CGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_21 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(CGPIO_18,4,func_I2S_2);
		}

		if (config & PIN_AUDIO_CFG_I2S_MCLK_1) {
			if ((top_reg13.bit.UART2 == MUX_1) &&
			    (top_reg14.bit.UART2_RTSCTS == MUX_1 || top_reg14.bit.UART2_RTSCTS == MUX_2)) {
				pr_err("I2S_MCLK_1 conflict with UART2_1 RTS\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C == MUX_1) {
				pr_err("I2S_MCLK_1 conflict with I2C_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI5 == MUX_1) {
				pr_err("I2S_MCLK_1 conflict with SPI5_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF4 == MUX_1) {
				pr_err("I2S_MCLK_1 conflict with SIF4_1\r\n");
				return E_PAR;
			}
			if (top_reg3.bit.ETH_PTP == MUX_1) {
				pr_err("I2S_MCLK_1 conflict with ETH_PTP\r\n");
				return E_PAR;
			}

			top_reg12.bit.I2S_MCLK = MUX_1;
			top_reg_pgpio0.bit.PGPIO_28 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_28,1,func_I2S_1_MCLK);
		} else if (config & PIN_AUDIO_CFG_I2S_MCLK_2) {
			if (top_reg1.bit.SDIO2_EN == MUX_1) {
				pr_err("I2S_MCLK_2 conflict with SDIO2\r\n");
				return E_PAR;
			}
			if (top_reg13.bit.UART8 == MUX_2) {
				pr_err("I2S_MCLK_2 conflict with UART8_2\r\n");
				return E_PAR;
			}
			if (top_reg1.bit.EXTROM_EN == MUX_1) {
				pr_err("I2S_MCLK_2 conflict with EXTROM\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI3 == MUX_2) {
				pr_err("I2S_MCLK_2 conflict with SPI3_2\r\n");
				return E_PAR;
			}
			if (top_reg8.bit.DSP_EJTAG_EN == MUX_1) {
				pr_err("I2S_MCLK_2 conflict with DSP_EJTAG_EN\r\n");
				return E_PAR;
			}
			if (top_reg15.bit.SDP == MUX_2) {
				pr_err("I2S_MCLK_2 conflict with SDP_2\r\n");
				return E_PAR;
			}

			top_reg12.bit.I2S_MCLK = MUX_2;
			top_reg_cgpio0.bit.CGPIO_17 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(CGPIO_17,1,func_I2S_2_MCLK);
		}

		if (config & PIN_AUDIO_CFG_I2S2_1) {
			if (top_reg2.bit.LCD_TYPE == MUX_1 || top_reg2.bit.LCD_TYPE == MUX_2 ||
				top_reg2.bit.LCD_TYPE == MUX_4 || top_reg2.bit.LCD_TYPE == MUX_5 || top_reg2.bit.LCD_TYPE == MUX_6) {
				pr_err("I2S2_1 conflict with LCD_TYPE\r\n");
				return E_PAR;
			}
			if (top_reg2.bit.LCD2_TYPE == MUX_3 || top_reg2.bit.LCD2_TYPE == MUX_8 || top_reg2.bit.LCD2_TYPE == MUX_10) {
				pr_err("I2S2_1 conflict with LCD2_TYPE\r\n");
				return E_PAR;
			}
			if (top_reg2.bit.MEMIF_TYPE == MUX_2) {
				if ((top_reg2.bit.MEMIF_SEL == MUX_0) &&
					(top_reg2.bit.PMEMIF_DATA_WIDTH == MUX_2 || top_reg2.bit.PMEMIF_DATA_WIDTH == MUX_3)) {
					pr_err("I2S2_1 conflict with (MEMIF_TYPE = 2 && MEMIF_SEL = 0 && PMEMIF_DATA_WIDTH = 0x2 or 0x3)\r\n");
					return E_PAR;
				} else if (top_reg2.bit.MEMIF_SEL == MUX_1) {
					pr_err("I2S2_1 conflict with (MEMIF_TYPE = 2 && MEMIF_SEL = 1)\r\n");
					return E_PAR;
				}
			}
			if (top_reg16.bit.SPI5 == MUX_2) {
				pr_err("I2S2_1 conflict with SPI5_2\r\n");
				return E_PAR;
			}
			if (top_reg3.bit.ETH == MUX_1 || top_reg3.bit.ETH == MUX_2) {
				pr_err("I2S2_1 conflict with ETH\r\n");
				return E_PAR;
			}

			top_reg12.bit.I2S2 = MUX_1;
			top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_20,4,func_I2S2_1);
		}

		if (config & PIN_AUDIO_CFG_I2S2_MCLK_1) {
			top_reg12.bit.I2S2_MCLK = MUX_1;
			top_reg_lgpio0.bit.LGPIO_24 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_24,1,func_I2S2_1);
		}

		if (config & PIN_AUDIO_CFG_DMIC_1) {
			if (top_reg13.bit.UART3 == MUX_1) {
				pr_err("DMIC conflict with UART3_1\r\n");
				return E_PAR;
			}
			if (top_reg5.bit.I2C9 == MUX_1) {
				pr_err("DMIC conflict with I2C9_1\r\n");
				return E_PAR;
			}
			if (top_reg5.bit.I2C10 == MUX_1) {
				pr_err("DMIC conflict with I2C10_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI4 == MUX_1) {
				pr_err("DMIC conflict with SPI4_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF3 == MUX_1) {
				pr_err("DMIC conflict with SIF3_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI3_RDY == MUX_1) {
				pr_err("DMIC conflict with SPI3_RDY_1\r\n");
				return E_PAR;
			}
			if (top_reg12.bit.I2S == MUX_1) {
				pr_err("DMIC conflict with I2S_1\r\n");
				return E_PAR;
			}

			top_reg12.bit.DMIC = MUX_1;
			top_reg_pgpio0.bit.PGPIO_26 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_26,1,func_AUDIO_DMIC);

			if (config & PIN_AUDIO_CFG_DMIC_DATA0) {
				top_reg12.bit.DMIC_DATA0 = MUX_1;
				top_reg_pgpio0.bit.PGPIO_27 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(PGPIO_27,1,func_AUDIO_DMIC);
			}

			if (config & PIN_AUDIO_CFG_DMIC_DATA1) {
				top_reg12.bit.DMIC_DATA1 = MUX_1;
				top_reg_pgpio0.bit.PGPIO_25 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(PGPIO_25,1,func_AUDIO_DMIC);
			}
		}

		if (config & PIN_AUDIO_CFG_EXT_EAC_MCLK) {
			if (top_reg1.bit.SDIO_EN == MUX_1) {
				pr_err("EXT_EAC_MCLK conflict with SDIO\r\n");
				return E_PAR;
			}

			top_reg12.bit.EXT_EAC_MCLK = MUX_1;
			top_reg_cgpio0.bit.CGPIO_16 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(CGPIO_16,1,func_AUDIO_EXT_MCLK);
		}
	}

	return E_OK;
}

static int pinmux_config_uart(uint32_t config)
{
	if (config == PIN_UART_CFG_NONE) {
	} else {
		if (config & PIN_UART_CFG_UART_1) {
			if ((top_reg0.bit.EJTAG_SEL == MUX_1) && (top_reg0.bit.EJTAG_CH_SEL == MUX_1)) {
				pr_err("UART_1 conflict with EJTAG_2\r\n");
				return E_PAR;
			}

			top_reg13.bit.UART = MUX_1;
			top_reg_pgpio1.bit.PGPIO_32 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio1.bit.PGPIO_33 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_32,2,func_UART);
		}

		if (config & PIN_UART_CFG_UART2_1) {
			if (top_reg4.bit.I2C2 == MUX_1) {
				pr_err("UART2_1 conflict with I2C2_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI5 == MUX_1) {
				pr_err("UART2_1 conflict with SPI5_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF4 == MUX_1) {
				pr_err("UART2_1 conflict with SIF4_1\r\n");
				return E_PAR;
			}
			if (top_reg15.bit.REMOTE_EXT == MUX_1) {
				pr_err("UART2_1 conflict with REMOTE_EXT_1\r\n");
				return E_PAR;
			}
			if (top_reg15.bit.REMOTE == MUX_1) {
				pr_err("UART2_1 conflict with REMOTE_1\r\n");
				return E_PAR;
			}

			top_reg13.bit.UART2 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_30 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_31 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_30,2,func_UART2);


			if (config & PIN_UART_CFG_UART2_RTSCTS) {
				if (top_reg4.bit.I2C == MUX_1) {
					pr_err("UART2_1 RTSCTS conflict with I2C_1\r\n");
					return E_PAR;
				}
				if (top_reg12.bit.I2S_MCLK == MUX_1) {
					pr_err("UART2_1 RTSCTS conflict with I2S_MCLK_1\r\n");
					return E_PAR;
				}
				if (top_reg3.bit.ETH_PTP == MUX_1) {
					pr_err("UART2_1 RTSCTS conflict with ETH_PTP\r\n");
					return E_PAR;
				}
				if (top_reg18.bit.RTC_CLK == MUX_1) {
					pr_err("UART2_1 RTSCTS conflict with RTC_CLK\r\n");
					return E_PAR;
				}

				top_reg14.bit.UART2_RTSCTS = MUX_1;
				top_reg_pgpio0.bit.PGPIO_28 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_29 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(PGPIO_28,2,func_UART2_CTSRTS);
			} else if (config & PIN_UART_CFG_UART2_DIROE) {
				if (top_reg4.bit.I2C == MUX_1) {
					pr_err("UART2_1 DIROE conflict with I2C_1\r\n");
					return E_PAR;
				}
				if (top_reg12.bit.I2S_MCLK == MUX_1) {
					pr_err("UART2_1 DIROE conflict with I2S_MCLK_1\r\n");
					return E_PAR;
				}
				if (top_reg3.bit.ETH_PTP == MUX_1) {
					pr_err("UART2_1 DIROE conflict with ETH_PTP\r\n");
					return E_PAR;
				}

				top_reg14.bit.UART2_RTSCTS = MUX_2;
				top_reg_pgpio0.bit.PGPIO_28 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(PGPIO_28,1,func_UART2_DTROE);
			}
		}

		if (config & PIN_UART_CFG_UART3_1) {
			if (top_reg5.bit.I2C9 == MUX_1) {
				pr_err("UART3_1 conflict with I2C9_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI4 == MUX_1) {
				pr_err("UART3_1 conflict with SPI4_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF3 == MUX_1) {
				pr_err("UART3_1 conflict with SIF3_1\r\n");
				return E_PAR;
			}
			if (top_reg12.bit.I2S == MUX_1) {
				pr_err("UART3_1 conflict with I2S_1\r\n");
				return E_PAR;
			}
			if (top_reg3.bit.ETH2_PTP == MUX_1) {
				pr_err("UART3_1 conflict with ETH2_PTP\r\n");
				return E_PAR;
			}
			if (top_reg12.bit.DMIC_DATA1 == MUX_1) {
				pr_err("UART3_1 conflict with DMIC_DATA1\r\n");
				return E_PAR;
			}

			top_reg13.bit.UART3 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_24 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_25 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_24,2,func_UART3);

			if (config & PIN_UART_CFG_UART3_RTSCTS) {
				if (top_reg5.bit.I2C10 == MUX_1) {
					pr_err("UART3_1 RTSCTS conflict with I2C10_1\r\n");
					return E_PAR;
				}
				if (top_reg12.bit.DMIC == MUX_1) {
					pr_err("UART3_1 RTSCTS conflict with DMIC\r\n");
					return E_PAR;
				}
				if (top_reg16.bit.SPI3_RDY == MUX_1) {
					pr_err("UART3_1 RTSCTS conflict with SPI3_RDY_1\r\n");
					return E_PAR;
				}

				top_reg14.bit.UART3_RTSCTS = MUX_1;
				top_reg_pgpio0.bit.PGPIO_26 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_27 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(PGPIO_26,2,func_UART3_CTSRTS);
			} else if (config & PIN_UART_CFG_UART3_DIROE) {
				if (top_reg5.bit.I2C10 == MUX_1) {
					pr_err("UART3_1 DIROE conflict with I2C10_1\r\n");
					return E_PAR;
				}
				if (top_reg12.bit.DMIC == MUX_1) {
					pr_err("UART3_1 DIROE conflict with DMIC\r\n");
					return E_PAR;
				}

				top_reg14.bit.UART3_RTSCTS = MUX_2;
				top_reg_pgpio0.bit.PGPIO_26 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(PGPIO_26,1,func_UART3_DTROE);
			}
		}

		if (config & PIN_UART_CFG_UART4_1) {
			if (top_reg4.bit.I2C7 == MUX_1) {
				pr_err("UART4_1 conflict with I2C7_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI3 == MUX_1) {
				pr_err("UART4_1 conflict with SPI3_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF2 == MUX_1) {
				pr_err("UART4_1 conflict with SIF2_1\r\n");
				return E_PAR;
			}
			if (top_reg7.bit.PICNT == MUX_1) {
				pr_err("UART4_1 conflict with PICNT\r\n");
				return E_PAR;
			}
			if (top_reg10.bit.SN5_MCLK == MUX_2) {
				pr_err("UART4_1 conflict with SN5_MCLK_2\r\n");
				return E_PAR;
			}

			top_reg13.bit.UART4 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_21 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_20,2,func_UART4_1);

			if (config & PIN_UART_CFG_UART4_RTSCTS) {
				if (top_reg4.bit.I2C8 == MUX_1) {
					pr_err("UART4_1 RTSCTS conflict with I2C8_1\r\n");
					return E_PAR;
				}
				if (top_reg7.bit.PICNT2 == MUX_1) {
					pr_err("UART4_1 RTSCTS conflict with PICNT2\r\n");
					return E_PAR;
				}
				if (top_reg18.bit.RTC_DIV_OUT == MUX_1) {
					pr_err("UART4_1 RTSCTS conflict with RTC_DIV_OUT\r\n");
					return E_PAR;
				}
				if (top_reg7.bit.PICNT3 == MUX_1) {
					pr_err("UART4_1 RTSCTS conflict with PICNT3\r\n");
					return E_PAR;
				}
				if (top_reg18.bit.RTC_EXT_CLK == MUX_1) {
					pr_err("UART4_1 RTSCTS conflict with RTC_EXT_CLK\r\n");
					return E_PAR;
				}

				top_reg14.bit.UART4_RTSCTS = MUX_1;
				top_reg_pgpio0.bit.PGPIO_22 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_23 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(PGPIO_22,2,func_UART4_CTSRTS);
			} else if (config & PIN_UART_CFG_UART4_DIROE) {
				if (top_reg4.bit.I2C8 == MUX_1) {
					pr_err("UART4_1 DIROE conflict with I2C8_1\r\n");
					return E_PAR;
				}
				if (top_reg7.bit.PICNT2 == MUX_1) {
					pr_err("UART4_1 DIROE conflict with PICNT2\r\n");
					return E_PAR;
				}
				if (top_reg18.bit.RTC_DIV_OUT == MUX_1) {
					pr_err("UART4_1 DIROE conflict with RTC_DIV_OUT\r\n");
					return E_PAR;
				}

				top_reg14.bit.UART4_RTSCTS = MUX_2;
				top_reg_pgpio0.bit.PGPIO_22 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(PGPIO_22,1,func_UART4_DTROE);
			}
		} else if (config & PIN_UART_CFG_UART4_2) {
			top_reg13.bit.UART4 = MUX_2;
			top_reg_agpio0.bit.AGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_agpio0.bit.AGPIO_1 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(AGPIO_0,2,func_UART4_2);
			if (config & PIN_UART_CFG_UART4_RTSCTS) {
				top_reg14.bit.UART4_RTSCTS = MUX_1;
				top_reg_agpio0.bit.AGPIO_2 = GPIO_ID_EMUM_FUNC;
				top_reg_agpio0.bit.AGPIO_3 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(AGPIO_2,2,func_UART4_CTSRTS);
			} else if (config & PIN_UART_CFG_UART4_DIROE) {
				top_reg14.bit.UART4_RTSCTS = MUX_2;
				top_reg_agpio0.bit.AGPIO_2 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(AGPIO_2,1,func_UART4_DTROE);
			}
		}

		if (config & PIN_UART_CFG_UART5_1) {
			if (top_reg4.bit.I2C5 == MUX_1) {
				pr_err("UART5_1 conflict with I2C5_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI2 == MUX_1) {
				pr_err("UART5_1 conflict with SPI2_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF1 == MUX_1) {
				pr_err("UART5_1 conflict with SIF1_1\r\n");
				return E_PAR;
			}
			if (top_reg15.bit.SDP == MUX_1) {
				pr_err("UART5_1 conflict with SDP_1\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR3 == MUX_3 || top_reg9.bit.SENSOR3 == MUX_4 || top_reg9.bit.SENSOR3 == MUX_5) {
				pr_err("UART5_1 conflict with SENSOR3\r\n");
				return E_PAR;
			}

			top_reg13.bit.UART5 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_16,2,func_UART5_1);
			if (config & PIN_UART_CFG_UART5_RTSCTS) {
				if (top_reg4.bit.I2C6 == MUX_1) {
					pr_err("UART5_1 RTSCTS conflict with I2C6_1\r\n");
					return E_PAR;
				}

				top_reg14.bit.UART5_RTSCTS = MUX_1;
				top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(PGPIO_18,2,func_UART5_CTSRTS);
			} else if (config & PIN_UART_CFG_UART5_DIROE) {
				if (top_reg4.bit.I2C6 == MUX_1) {
					pr_err("UART5_1 DIROE conflict with I2C6_1\r\n");
					return E_PAR;
				}

				top_reg14.bit.UART5_RTSCTS = MUX_2;
				top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(PGPIO_18,1,func_UART5_DTROE);
			}
		} else if (config & PIN_UART_CFG_UART5_2) {
			if (top_reg0.bit.DSI_PROT_EN == MUX_1) {
				pr_err("UART5_2 conflict with DSI_PROT_EN\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI == MUX_2) {
				pr_err("UART5_2 conflict with SPI_2\r\n");
				return E_PAR;
			}

			top_reg13.bit.UART5 = MUX_2;
			top_reg_dsigpio0.bit.DSIGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_dsigpio0.bit.DSIGPIO_1 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(DSIGPIO_0,2,func_UART5_2);
			if (config & PIN_UART_CFG_UART5_RTSCTS) {
				top_reg14.bit.UART5_RTSCTS = MUX_1;
				top_reg_dsigpio0.bit.DSIGPIO_2 = GPIO_ID_EMUM_FUNC;
				top_reg_dsigpio0.bit.DSIGPIO_3 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(DSIGPIO_2,2,func_UART5_CTSRTS);
			} else if (config & PIN_UART_CFG_UART5_DIROE) {
				top_reg14.bit.UART5_RTSCTS = MUX_2;
				top_reg_dsigpio0.bit.DSIGPIO_2 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(DSIGPIO_2,1,func_UART5_DTROE);
			}
		}
	}

	return E_OK;
}

static int pinmux_config_uartII(uint32_t config)
{
	if (config == PIN_UARTII_CFG_NONE) {
	} else {
		if (config & PIN_UARTII_CFG_UART6_1) {
			if (top_reg4.bit.I2C3 == MUX_1) {
				pr_err("UART6_1 conflict with I2C3_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI == MUX_1) {
				pr_err("UART6_1 conflict with SPI_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF0 == MUX_1) {
				pr_err("UART6_1 conflict with SIF0_1\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR3 == MUX_1 || top_reg9.bit.SENSOR3 == MUX_3 ||
				top_reg9.bit.SENSOR3 == MUX_4 || top_reg9.bit.SENSOR3 == MUX_5) {
				pr_err("UART6_1 conflict with SENSOR3\r\n");
				return E_PAR;
			}

			top_reg13.bit.UART6 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_12,2,func_UART6_1);
			if (config & PIN_UARTII_CFG_UART6_RTSCTS) {
				if (top_reg4.bit.I2C4 == MUX_1) {
					pr_err("UART6_1 RTSCTS conflict with I2C4_1\r\n");
					return E_PAR;
				}
				if (top_reg15.bit.SDP == MUX_1) {
					pr_err("UART6_1 RTSCTS conflict with SDP_1\r\n");
					return E_PAR;
				}

				top_reg14.bit.UART6_RTSCTS = MUX_1;
				top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(PGPIO_14,2,func_UART6_CTSRTS);
			} else if (config & PIN_UARTII_CFG_UART6_DIROE) {
				if (top_reg4.bit.I2C4 == MUX_1) {
					pr_err("UART6_1 DIROE conflict with I2C4_1\r\n");
					return E_PAR;
				}

				top_reg14.bit.UART6_RTSCTS = MUX_2;
				top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(PGPIO_14,1,func_UART6_DTROE);
			}
		} else if (config & PIN_UARTII_CFG_UART6_2) {
			if (top_reg0.bit.DSI_PROT_EN == MUX_1) {
				pr_err("UART6_2 conflict with DSI_PROT_EN\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C4 == MUX_3) {
				pr_err("UART6_2 conflict with I2C4_3\r\n");
				return E_PAR;
			}

			top_reg13.bit.UART6 = MUX_2;
			top_reg_dsigpio0.bit.DSIGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_dsigpio0.bit.DSIGPIO_5 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(DSIGPIO_4,2,func_UART6_2);
			if (config & PIN_UARTII_CFG_UART6_RTSCTS) {
				if (top_reg6.bit.PWM0 == MUX_2) {
					pr_err("UART6_2 RTSCTS conflict with PWM0_2\r\n");
					return E_PAR;
				}
				if (top_reg6.bit.PWM1 == MUX_2) {
					pr_err("UART6_2 RTSCTS conflict with PWM1_2\r\n");
					return E_PAR;
				}

				top_reg14.bit.UART6_RTSCTS = MUX_1;
				top_reg_dsigpio0.bit.DSIGPIO_6 = GPIO_ID_EMUM_FUNC;
				top_reg_dsigpio0.bit.DSIGPIO_7 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(DSIGPIO_6,2,func_UART6_CTSRTS);
			} else if (config & PIN_UARTII_CFG_UART6_DIROE) {
				if (top_reg6.bit.PWM0 == MUX_2) {
					pr_err("UART6_2 DIROE conflict with PWM0_2\r\n");
					return E_PAR;
				}

				top_reg14.bit.UART6_RTSCTS = MUX_2;
				top_reg_dsigpio0.bit.DSIGPIO_6 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(DSIGPIO_6,1,func_UART6_DTROE);
			}
		}

		if (config & PIN_UARTII_CFG_UART7_1) {
			if (top_reg7.bit.PWM8 == MUX_1) {
				pr_err("UART7_1 conflict with PWM8_1\r\n");
				return E_PAR;
			}
			if (top_reg7.bit.PWM9 == MUX_1) {
				pr_err("UART7_1 conflict with PWM9_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF5 == MUX_1) {
				pr_err("UART7_1 conflict with SIF5_1\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR3) {
				pr_err("UART7_1 conflict with SENSOR3\r\n");
				return E_PAR;
			}

			top_reg13.bit.UART7 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_8,2,func_UART7_1);

			if (config & PIN_UARTII_CFG_UART7_RTSCTS) {
				if (top_reg7.bit.PWM10 == MUX_1) {
					pr_err("UART7_1 RTSCTS conflict with PWM10_1\r\n");
					return E_PAR;
				}
				if (top_reg7.bit.PWM11 == MUX_1) {
					pr_err("UART7_1 RTSCTS conflict with PWM11_1\r\n");
					return E_PAR;
				}

				top_reg14.bit.UART7_RTSCTS = MUX_1;
				top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_11 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(PGPIO_10,2,func_UART7_CTSRTS);
			} else if (config & PIN_UARTII_CFG_UART7_DIROE) {
				if (top_reg7.bit.PWM10 == MUX_1) {
					pr_err("UART7_1 DIROE conflict with PWM10_1\r\n");
					return E_PAR;
				}

				top_reg14.bit.UART7_RTSCTS = MUX_2;
				top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(PGPIO_10,1,func_UART7_DTROE);
			}
		} else if (config & PIN_UARTII_CFG_UART7_2) {
			if (top_reg1.bit.SDIO3_BUS_WIDTH == MUX_1) {
				pr_err("UART7_2 conflict with SDIO3_BUS_WIDTH\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C5 == MUX_3) {
				pr_err("UART7_2 conflict with I2C5_3\r\n");
				return E_PAR;
			}
			if (top_reg7.bit.PWM8 == MUX_1) {
				pr_err("UART7_2 conflict with PWM8_2\r\n");
				return E_PAR;
			}
			if (top_reg7.bit.PWM9 == MUX_1) {
				pr_err("UART7_2 conflict with PWM9_2\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI == MUX_3) {
				pr_err("UART7_2 conflict with SPI_3\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF5 == MUX_2) {
				pr_err("UART7_2 conflict with SIF5_2\r\n");
				return E_PAR;
			}

			top_reg13.bit.UART7 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_5 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(CGPIO_4,2,func_UART7_2);

			if (config & PIN_UARTII_CFG_UART7_RTSCTS) {
				if (top_reg4.bit.I2C6 == MUX_3) {
					pr_err("UART7_2 RTSCTS conflict with I2C6_3\r\n");
					return E_PAR;
				}
				if (top_reg7.bit.PWM10 == MUX_2) {
					pr_err("UART7_2 RTSCTS conflict with PWM10_2\r\n");
					return E_PAR;
				}
				if (top_reg7.bit.PWM11 == MUX_2) {
					pr_err("UART7_2 RTSCTS conflict with PWM11_2\r\n");
					return E_PAR;
				}

				top_reg14.bit.UART7_RTSCTS = MUX_1;
				top_reg_cgpio0.bit.CGPIO_6 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_7 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(CGPIO_6,2,func_UART7_CTSRTS);
			} else if (config & PIN_UARTII_CFG_UART7_DIROE) {
				if (top_reg4.bit.I2C6 == MUX_3) {
					pr_err("UART7_2 DIROE conflict with I2C6_3\r\n");
					return E_PAR;
				}
				if (top_reg7.bit.PWM10 == MUX_2) {
					pr_err("UART7_2 DIROE conflict with PWM10_2\r\n");
					return E_PAR;
				}

				top_reg14.bit.UART7_RTSCTS = MUX_2;
				top_reg_cgpio0.bit.CGPIO_6 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(CGPIO_6,1,func_UART7_DTROE);
			}
		}

		if (config & PIN_UARTII_CFG_UART8_1) {
			if (top_reg6.bit.PWM4 == MUX_1) {
				pr_err("UART8_1 conflict with PWM4_1\r\n");
				return E_PAR;
			}
			if (top_reg6.bit.PWM5 == MUX_1) {
				pr_err("UART8_1 conflict with PWM5_1\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR3 == MUX_1 || top_reg9.bit.SENSOR3 == MUX_2 ||
				top_reg9.bit.SENSOR3 == MUX_4 || top_reg9.bit.SENSOR3 == MUX_5) {
				pr_err("UART8_1 conflict with SENSOR3\r\n");
				return E_PAR;
			}

			top_reg13.bit.UART8 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_5 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_4,2,func_UART8_1);
			if (config & PIN_UARTII_CFG_UART8_RTSCTS) {
				if (top_reg6.bit.PWM6 == MUX_1) {
					pr_err("UART8_1 RTSCTS conflict with PWM6_1\r\n");
					return E_PAR;
				}
				if (top_reg6.bit.PWM7 == MUX_1) {
					pr_err("UART8_1 RTSCTS conflict with PWM7_1\r\n");
					return E_PAR;
				}

				top_reg14.bit.UART8_RTSCTS = MUX_1;
				top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_7 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(PGPIO_6,2,func_UART8_CTSRTS);
			} else if (config & PIN_UARTII_CFG_UART8_DIROE) {
				if (top_reg6.bit.PWM6 == MUX_1) {
					pr_err("UART8_1 DIROE conflict with PWM6_1\r\n");
					return E_PAR;
				}

				top_reg14.bit.UART8_RTSCTS = MUX_2;
				top_reg_pgpio0.bit.PGPIO_6 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(PGPIO_6,1,func_UART8_DTROE);
			}
		} else if (config & PIN_UARTII_CFG_UART8_2) {
			if (top_reg1.bit.SDIO2_EN == MUX_1) {
				pr_err("UART8_2 conflict with SDIO2\r\n");
				return E_PAR;
			}
			if (top_reg1.bit.EXTROM_EN == MUX_1) {
				pr_err("UART8_2 conflict with EXTROM\r\n");
				return E_PAR;
			}
			if (top_reg12.bit.I2S == MUX_2) {
				pr_err("UART8_2 conflict with I2S_2\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI3 == MUX_2) {
				pr_err("UART8_2 conflict with SPI3_2\r\n");
				return E_PAR;
			}
			if (top_reg8.bit.DSP_EJTAG_EN == MUX_1) {
				pr_err("UART8_2 conflict with DSP_EJTAG_EN\r\n");
				return E_PAR;
			}
			if (top_reg15.bit.SDP == MUX_2) {
				pr_err("UART8_2 conflict with SDP_2\r\n");
				return E_PAR;
			}

			top_reg13.bit.UART8 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_18 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(CGPIO_17,2,func_UART8_2);
			if (config & PIN_UARTII_CFG_UART8_RTSCTS) {
				top_reg14.bit.UART8_RTSCTS = MUX_1;
				top_reg_cgpio0.bit.CGPIO_19 = GPIO_ID_EMUM_FUNC;
				top_reg_cgpio0.bit.CGPIO_20 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(CGPIO_19,2,func_UART8_CTSRTS);
			} else if (config & PIN_UARTII_CFG_UART8_DIROE) {
				top_reg14.bit.UART8_RTSCTS = MUX_2;
				top_reg_cgpio0.bit.CGPIO_19 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(CGPIO_19,1,func_UART8_DTROE);
			}
		}

		if (config & PIN_UARTII_CFG_UART9_1) {
			if (top_reg6.bit.PWM0 == MUX_1) {
				pr_err("UART9_1 conflict with PWM0_1\r\n");
				return E_PAR;
			}
			if (top_reg6.bit.PWM1 == MUX_1) {
				pr_err("UART9_1 conflict with PWM1_1\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR3 == MUX_1 || top_reg9.bit.SENSOR3 == MUX_2 ||
				top_reg9.bit.SENSOR3 == MUX_4 || top_reg9.bit.SENSOR3 == MUX_5) {
				pr_err("UART9_1 conflict with SENSOR3\r\n");
				return E_PAR;
			}

			top_reg14.bit.UART9 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_1 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_0,2,func_UART9_1);
			if (config & PIN_UARTII_CFG_UART9_RTSCTS) {
				if (top_reg6.bit.PWM2 == MUX_1) {
					pr_err("UART9_1 RTSCTS conflict with PWM2_1\r\n");
					return E_PAR;
				}
				if (top_reg6.bit.PWM3 == MUX_1) {
					pr_err("UART9_1 RTSCTS conflict with PWM3_1\r\n");
					return E_PAR;
				}

				top_reg14.bit.UART9_RTSCTS = MUX_1;
				top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
				top_reg_pgpio0.bit.PGPIO_3 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(PGPIO_2,2,func_UART9_CTSRTS);
			} else if (config & PIN_UARTII_CFG_UART9_DIROE) {
				if (top_reg6.bit.PWM2 == MUX_1) {
					pr_err("UART9_1 DIROE conflict with PWM2_1\r\n");
					return E_PAR;
				}

				top_reg14.bit.UART9_RTSCTS = MUX_2;
				top_reg_pgpio0.bit.PGPIO_2 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(PGPIO_2,1,func_UART9_DTROE);
			}
		} else if (config & PIN_UARTII_CFG_UART9_2) {
			if (top_reg6.bit.PWM7 == MUX_2) {
				pr_err("UART9_2 conflict with PWM7_2\r\n");
				return E_PAR;
			}
			if (top_reg18.bit.SP_CLK == MUX_1) {
				pr_err("UART9_2 conflict with SP_CLK\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI4 == MUX_2 && top_reg16.bit.SPI4_BUS_WIDTH == MUX_1) {
				pr_err("UART9_2 conflict with SPI4_2 BUS_WIDTH\r\n");
				return E_PAR;
			}
			if (top_reg3.bit.ETH == MUX_1 || top_reg3.bit.ETH == MUX_2) {
				pr_err("UART9_2 conflict with ETH\r\n");
				return E_PAR;
			}

			top_reg14.bit.UART9 = MUX_2;
			top_reg_dgpio0.bit.DGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_6 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(DGPIO_5,2,func_UART9_2);
			if (config & PIN_UARTII_CFG_UART9_RTSCTS) {
				top_reg14.bit.UART9_RTSCTS = MUX_1;
				top_reg_dgpio0.bit.DGPIO_7 = GPIO_ID_EMUM_FUNC;
				top_reg_dgpio0.bit.DGPIO_8 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(DGPIO_7,2,func_UART9_CTSRTS);
			} else if (config & PIN_UARTII_CFG_UART9_DIROE) {
				top_reg14.bit.UART9_RTSCTS = MUX_2;
				top_reg_dgpio0.bit.DGPIO_7 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(DGPIO_7,1,func_UART9_DTROE);
			}
		}
	}

	return E_OK;
}

static int pinmux_config_remote(uint32_t config)
{
	if (config == PIN_REMOTE_CFG_NONE) {
	} else {
		if (config & PIN_REMOTE_CFG_REMOTE_1) {
			if (top_reg13.bit.UART2 == MUX_1) {
				pr_err("REMOTE_1 conflict with UART2_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C2 == MUX_1) {
				pr_err("REMOTE_1 conflict with I2C2_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI5 == MUX_1 && top_reg16.bit.SPI5_BUS_WIDTH == MUX_1) {
				pr_err("REMOTE_1 conflict with SPI5_1 BUS_WIDTH\r\n");
				return E_PAR;
			}

			top_reg15.bit.REMOTE = MUX_1;
			top_reg_pgpio0.bit.PGPIO_31 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_31,1,func_Remote);
		}

		if (config & PIN_REMOTE_CFG_REMOTE_EXT_1) {
			if (top_reg13.bit.UART2 == MUX_1) {
				pr_err("REMOTE_EXT_1 conflict with UART2_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C2 == MUX_1) {
				pr_err("REMOTE_EXT_1 conflict with I2C2_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI5 == MUX_1) {
				pr_err("REMOTE_EXT_1 conflict with SPI5_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF4 == MUX_1) {
				pr_err("REMOTE_EXT_1 conflict with SIF4_1\r\n");
				return E_PAR;
			}

			top_reg15.bit.REMOTE_EXT = MUX_1;
			top_reg_pgpio0.bit.PGPIO_30 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_30,1,func_Remote);
		}
	}

	return E_OK;
}

static int pinmux_config_sdp(uint32_t config)
{
	if (config == PIN_SDP_CFG_NONE) {
	} else {
		if (config & PIN_SDP_CFG_SDP_1) {
			if ((top_reg13.bit.UART6 == MUX_2) &&
			    (top_reg14.bit.UART6_RTSCTS == MUX_1)) {
				pr_err("SDP_1 conflict with UART6_1 CTS\r\n");
				return E_PAR;
			}
			if (top_reg13.bit.UART5 == MUX_1) {
				pr_err("SDP_1 conflict with UART5_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C5 == MUX_1) {
				pr_err("SDP_1 conflict with I2C5_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C6 == MUX_1) {
				pr_err("SDP_1 conflict with I2C6_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI == MUX_1 && top_reg16.bit.SPI_BUS_WIDTH == MUX_1) {
				pr_err("SDP_1 conflict with SPI_1 BUS_WIDTH\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI2 == MUX_1) {
				pr_err("SDP_1 conflict with SPI2_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF1 == MUX_1) {
				pr_err("SDP_1 conflict with SIF1_1\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR3 == MUX_1 || top_reg9.bit.SENSOR3 == MUX_3 ||
				top_reg9.bit.SENSOR3 == MUX_4 || top_reg9.bit.SENSOR3 == MUX_5) {
				pr_err("SDP_1 conflict with SENSOR3\r\n");
				return E_PAR;
			}

			top_reg15.bit.SDP = MUX_1;
			top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_15,5,func_SDP_1);
		} else if (config & PIN_SDP_CFG_SDP_2) {
			if (top_reg1.bit.SDIO2_EN == MUX_1) {
				pr_err("SDP_2 conflict with SDIO2\r\n");
				return E_PAR;
			}
			if (top_reg13.bit.UART8 == MUX_2) {
				pr_err("SDP_2 conflict with UART8_2\r\n");
				return E_PAR;
			}
			if (top_reg1.bit.EXTROM_EN == MUX_1) {
				pr_err("SDP_2 conflict with EXTROM\r\n");
				return E_PAR;
			}
			if (top_reg12.bit.I2S == MUX_2) {
				pr_err("SDP_2 conflict with I2S_2\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI3 == MUX_2) {
				pr_err("SDP_2 conflict with SPI3_2\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI3_RDY == MUX_2) {
				pr_err("SDP_2 conflict with SPI3_RDY_2\r\n");
				return E_PAR;
			}
			if (top_reg8.bit.DSP_EJTAG_EN == MUX_1) {
				pr_err("SDP_2 conflict with DSP_EJTAG_EN\r\n");
				return E_PAR;
			}

			top_reg15.bit.SDP = MUX_2;
			top_reg_cgpio0.bit.CGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_21 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(CGPIO_17,5,func_SDP_2);
		}
	}

	return E_OK;
}

static int pinmux_config_spi(uint32_t config)
{
	if (config == PIN_SPI_CFG_NONE) {
	} else {
		if (config & PIN_SPI_CFG_SPI_1) {
			if (top_reg13.bit.UART6 == MUX_1) {
				pr_err("SPI_1 conflict with UART6_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C3 == MUX_1) {
				pr_err("SPI_1 conflict with I2C3_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C4 == MUX_1) {
				pr_err("SPI_1 conflict with I2C4_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF0 == MUX_1) {
				pr_err("SPI_1 conflict with SIF0_1\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR3 == MUX_1 || top_reg9.bit.SENSOR3 == MUX_3 ||
				top_reg9.bit.SENSOR3 == MUX_4 || top_reg9.bit.SENSOR3 == MUX_5) {
				pr_err("SPI_1 conflict with SENSOR3\r\n");
				return E_PAR;
			}

			top_reg16.bit.SPI = MUX_1;
			top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_12,3,func_SPI_1);
			if (config & PIN_SPI_CFG_SPI_BUS_WIDTH) {
				top_reg16.bit.SPI_BUS_WIDTH = MUX_1;
				top_reg_pgpio0.bit.PGPIO_15 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(PGPIO_15,1,func_SPI_1);
			}
		} else if (config & PIN_SPI_CFG_SPI_2) {
			if (top_reg0.bit.DSI_PROT_EN == MUX_1) {
				pr_err("SPI_2 conflict with DSI_PROT_EN\r\n");
				return E_PAR;
			}
			if (top_reg13.bit.UART5 == MUX_2) {
				pr_err("SPI_2 conflict with UART5_2\r\n");
				return E_PAR;
			}

			top_reg16.bit.SPI = MUX_2;
			top_reg_dsigpio0.bit.DSIGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_dsigpio0.bit.DSIGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_dsigpio0.bit.DSIGPIO_2 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(DSIGPIO_0,3,func_SPI_2);
			if (config & PIN_SPI_CFG_SPI_BUS_WIDTH) {
				top_reg16.bit.SPI_BUS_WIDTH = MUX_1;
				top_reg_dsigpio0.bit.DSIGPIO_3 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(DSIGPIO_3,1,func_SPI_2);
			}
		} else if (config & PIN_SPI_CFG_SPI_3) {
			if (top_reg1.bit.SDIO3_BUS_WIDTH == MUX_1) {
				pr_err("SPI_3 conflict with SDIO3_BUS_WIDTH\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C5 == MUX_3) {
				pr_err("SPI_3 conflict with I2C5_3\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C6 == MUX_3) {
				pr_err("SPI_3 conflict with I2C6_3\r\n");
				return E_PAR;
			}
			if (top_reg7.bit.PWM8 == MUX_2) {
				pr_err("SPI_3 conflict with PWM8_2\r\n");
				return E_PAR;
			}
			if (top_reg7.bit.PWM9 == MUX_2) {
				pr_err("SPI_3 conflict with PWM9_2\r\n");
				return E_PAR;
			}
			if (top_reg7.bit.PWM10 == MUX_2) {
				pr_err("SPI_3 conflict with PWM10_2\r\n");
				return E_PAR;
			}
			if (top_reg13.bit.UART7 == MUX_2) {
				pr_err("SPI_3 conflict with UART7_2\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF5 == MUX_2) {
				pr_err("SPI_3 conflict with SIF5_2\r\n");
				return E_PAR;
			}

			top_reg16.bit.SPI = MUX_3;
			top_reg_cgpio0.bit.CGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_6 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(CGPIO_4,3,func_SPI_3);
			if (config & PIN_SPI_CFG_SPI_BUS_WIDTH) {
				if (top_reg7.bit.PWM11 == MUX_2) {
					pr_err("SPI_3 BUS_WIDTH conflict with PWM11_2\r\n");
					return E_PAR;
				}

				top_reg16.bit.SPI_BUS_WIDTH = MUX_1;
				top_reg_cgpio0.bit.CGPIO_7 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(CGPIO_7,1,func_SPI_3);
			}
		}

		if (config & PIN_SPI_CFG_SPI2_1) {
			if (top_reg13.bit.UART5 == MUX_1) {
				pr_err("SPI2_1 conflict with UART5_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C5 == MUX_1) {
				pr_err("SPI2_1 conflict with I2C5_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C6 == MUX_1) {
				pr_err("SPI2_1 conflict with I2C6_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF1 == MUX_1) {
				pr_err("SPI2_1 conflict with SIF1_1\r\n");
				return E_PAR;
			}
			if (top_reg15.bit.SDP == MUX_1) {
				pr_err("SPI2_1 conflict with SDP_1\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR3 == MUX_1 || top_reg9.bit.SENSOR3 == MUX_3 ||
				top_reg9.bit.SENSOR3 == MUX_4 || top_reg9.bit.SENSOR3 == MUX_5) {
				pr_err("SPI2_1 conflict with SENSOR3\r\n");
				return E_PAR;
			}

			top_reg16.bit.SPI2 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_16,3,func_SPI2_1);
			if (config & PIN_SPI_CFG_SPI2_BUS_WIDTH) {
				top_reg16.bit.SPI2_BUS_WIDTH = MUX_1;
				top_reg_pgpio0.bit.PGPIO_19 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(PGPIO_19,1,func_SPI2_1);
			}
		} else if (config & PIN_SPI_CFG_SPI2_2) {
			if (top_reg10.bit.SN4_XVSXHS == MUX_1) {
				pr_err("SPI2_2 conflict with SN4_XVSXHS_1\r\n");
				return E_PAR;
			}
			if (top_reg10.bit.SN5_XVSXHS == MUX_1) {
				pr_err("SPI2_2 conflict with SN5_XVSXHS_1\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR2 == MUX_1 || top_reg9.bit.SENSOR2 == MUX_2 ||
				top_reg9.bit.SENSOR2 == MUX_4 || top_reg9.bit.SENSOR2 == MUX_5) {
				pr_err("SPI2_2 conflict with SENSOR2\r\n");
				return E_PAR;
			}

			top_reg16.bit.SPI2 = MUX_2;
			top_reg_sgpio0.bit.SGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_19 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(SGPIO_17,3,func_SPI2_2);
			if (config & PIN_SPI_CFG_SPI2_BUS_WIDTH) {
				top_reg16.bit.SPI2_BUS_WIDTH = MUX_1;
				top_reg_sgpio0.bit.SGPIO_20 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(SGPIO_20,1,func_SPI2_2);
			}
		}

		if (config & PIN_SPI_CFG_SPI3_1) {
			if (top_reg13.bit.UART4 == MUX_1) {
				pr_err("SPI3_1 conflict with UART4_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C7 == MUX_1) {
				pr_err("SPI3_1 conflict with I2C7_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C8 == MUX_1) {
				pr_err("SPI3_1 conflict with I2C8_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF2 == MUX_1) {
				pr_err("SPI3_1 conflict with SIF2_1\r\n");
				return E_PAR;
			}
			if (top_reg7.bit.PICNT == MUX_1) {
				pr_err("SPI3_1 conflict with PICNT\r\n");
				return E_PAR;
			}
			if (top_reg7.bit.PICNT2 == MUX_1) {
				pr_err("SPI3_1 RTSCTS conflict with PICNT2\r\n");
				return E_PAR;
			}
			if (top_reg10.bit.SN5_MCLK == MUX_2) {
				pr_err("SPI3_1 conflict with SN5_MCLK_2\r\n");
				return E_PAR;
			}
			if (top_reg18.bit.RTC_DIV_OUT == MUX_1) {
				pr_err("SPI3_1 conflict with RTC_DIV_OUT\r\n");
				return E_PAR;
			}

			top_reg16.bit.SPI3 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_22 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_20,3,func_SPI3_1);
			if (config & PIN_SPI_CFG_SPI3_BUS_WIDTH) {
				if (top_reg7.bit.PICNT3 == MUX_1) {
					pr_err("SPI3_1 BUS_WIDTH conflict with PICNT3\r\n");
					return E_PAR;
				}
				if (top_reg18.bit.RTC_EXT_CLK == MUX_1) {
					pr_err("SPI3_1 BUS_WIDTH conflict with RTC_EXT_CLK\r\n");
					return E_PAR;
				}

				top_reg16.bit.SPI3_BUS_WIDTH = MUX_1;
				top_reg_pgpio0.bit.PGPIO_23 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(PGPIO_23,1,func_SPI3_1);
			}
		} else if (config & PIN_SPI_CFG_SPI3_2) {
			if (top_reg1.bit.SDIO2_EN == MUX_1) {
				pr_err("SPI3_2 conflict with SDIO2\r\n");
				return E_PAR;
			}
			if (top_reg13.bit.UART8 == MUX_2) {
				pr_err("SPI3_2 conflict with UART8_2\r\n");
				return E_PAR;
			}
			if (top_reg1.bit.EXTROM_EN == MUX_1) {
				pr_err("SPI3_2 conflict with EXTROM\r\n");
				return E_PAR;
			}
			if (top_reg12.bit.I2S == MUX_2) {
				pr_err("SPI3_2 conflict with I2S_2\r\n");
				return E_PAR;
			}
			if (top_reg8.bit.DSP_EJTAG_EN == MUX_1) {
				pr_err("SPI3_2 conflict with DSP_EJTAG_EN\r\n");
				return E_PAR;
			}
			if (top_reg15.bit.SDP == MUX_2) {
				pr_err("SPI3_2 conflict with SDP_2\r\n");
				return E_PAR;
			}

			top_reg16.bit.SPI3 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_19 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(CGPIO_17,3,func_SPI3_2);
			if (config & PIN_SPI_CFG_SPI3_BUS_WIDTH) {
				top_reg16.bit.SPI3_BUS_WIDTH = MUX_1;
				top_reg_cgpio0.bit.CGPIO_20 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(CGPIO_20,1,func_SPI3_2);
			}
		}

		if (config & PIN_SPI_CFG_SPI3_RDY_1) {
			if ((top_reg13.bit.UART3 == MUX_1) &&
				(top_reg14.bit.UART3_RTSCTS == MUX_1)) {
				pr_err("SPI3_RDY_1 conflict with UART3_1 CTS\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI4 == MUX_1 && top_reg16.bit.SPI4_BUS_WIDTH == MUX_1) {
				pr_err("SPI3_RDY_1 conflict with SPI4_1 BUS_WIDTH\r\n");
				return E_PAR;
			}
			if (top_reg5.bit.I2C10 == MUX_1) {
				pr_err("SPI3_RDY_1 conflict with I2C10_1\r\n");
				return E_PAR;
			}
			if (top_reg12.bit.I2S == MUX_1) {
				pr_err("SPI3_RDY_1 conflict with I2S_1\r\n");
				return E_PAR;
			}
			if (top_reg12.bit.DMIC_DATA0 == MUX_1) {
				pr_err("SPI3_RDY_1 conflict with DMIC_DATA0\r\n");
				return E_PAR;
			}

			top_reg16.bit.SPI3_RDY = MUX_1;
			top_reg_pgpio0.bit.PGPIO_27 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_27,1,func_SPI3_RDY);
		} else if (config & PIN_SPI_CFG_SPI3_RDY_2) {
			if (top_reg1.bit.SDIO2_EN == MUX_1) {
				pr_err("SPI3_RDY_2 conflict with SDIO2\r\n");
				return E_PAR;
			}
			if (top_reg12.bit.I2S == MUX_2) {
				pr_err("SPI3_RDY_2 conflict with I2S_2\r\n");
				return E_PAR;
			}
			if (top_reg8.bit.DSP_EJTAG_EN == MUX_1) {
				pr_err("SPI3_RDY_2 conflict with DSP_EJTAG_EN\r\n");
				return E_PAR;
			}
			if (top_reg15.bit.SDP == MUX_2) {
				pr_err("SPI3_RDY_2 conflict with SDP_2\r\n");
				return E_PAR;
			}

			top_reg16.bit.SPI3_RDY = MUX_2;
			top_reg_cgpio0.bit.CGPIO_21 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(CGPIO_21,1,func_SPI3_RDY2);
		}

		if (config & PIN_SPI_CFG_SPI4_1) {
			if (top_reg13.bit.UART3 == MUX_1) {
				pr_err("SPI4_1 conflict with UART3_1\r\n");
				return E_PAR;
			}
			if (top_reg5.bit.I2C9 == MUX_1) {
				pr_err("SPI4_1 conflict with I2C9_1\r\n");
				return E_PAR;
			}
			if (top_reg5.bit.I2C10 == MUX_1) {
				pr_err("SPI4_1 conflict with I2C10_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF3 == MUX_1) {
				pr_err("SPI4_1 conflict with SIF3_1\r\n");
				return E_PAR;
			}
			if (top_reg12.bit.I2S == MUX_1) {
				pr_err("SPI4_1 conflict with I2S_1\r\n");
				return E_PAR;
			}
			if (top_reg3.bit.ETH2_PTP == MUX_1) {
				pr_err("SPI4_1 conflict with ETH2_PTP\r\n");
				return E_PAR;
			}
			if (top_reg12.bit.DMIC == MUX_1) {
				pr_err("SPI4_1 conflict with DMIC\r\n");
				return E_PAR;
			}

			top_reg16.bit.SPI4 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_24 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_25 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_26 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_24,3,func_SPI4_1);
			if (config & PIN_SPI_CFG_SPI4_BUS_WIDTH) {
				if (top_reg16.bit.SPI3_RDY == MUX_1) {
					pr_err("SPI4_1 BUS_WIDTH conflict with SPI3_RDY_1\r\n");
					return E_PAR;
				}

				top_reg16.bit.SPI4_BUS_WIDTH = MUX_1;
				top_reg_pgpio0.bit.PGPIO_27 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(PGPIO_27,1,func_SPI4_1);
			}
		} else if (config & PIN_SPI_CFG_SPI4_2) {
			if (top_reg19.bit.SATA_LED == MUX_1) {
				pr_err("SPI4_2 conflict with SATA_LED\r\n");
				return E_PAR;
			}
			if (top_reg6.bit.PWM4 == MUX_2) {
				pr_err("SPI4_2 conflict with PWM4_2\r\n");
				return E_PAR;
			}
			if (top_reg6.bit.PWM5 == MUX_2) {
				pr_err("SPI4_2 conflict with PWM5_2\r\n");
				return E_PAR;
			}
			if (top_reg6.bit.PWM6 == MUX_2) {
				pr_err("SPI4_2 conflict with PWM6_2\r\n");
				return E_PAR;
			}

			top_reg16.bit.SPI4 = MUX_2;
			top_reg_dgpio0.bit.DGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_dgpio0.bit.DGPIO_4 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(DGPIO_2,3,func_SPI4_2);
			if (config & PIN_SPI_CFG_SPI4_BUS_WIDTH) {
				if (top_reg14.bit.UART9 == MUX_2) {
					pr_err("SPI4_2 conflict with UART9_2\r\n");
					return E_PAR;
				}
				if (top_reg6.bit.PWM7 == MUX_2) {
					pr_err("SPI4_2 conflict with PWM7_2\r\n");
					return E_PAR;
				}
				if (top_reg3.bit.ETH == MUX_1 || top_reg3.bit.ETH == MUX_2) {
					pr_err("SPI4_2 conflict with ETH\r\n");
					return E_PAR;
				}

				top_reg16.bit.SPI4_BUS_WIDTH = MUX_1;
				top_reg_dgpio0.bit.DGPIO_5 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(DGPIO_5,1,func_SPI4_2);
			}
		}

		if (config & PIN_SPI_CFG_SPI5_1) {
			if (top_reg13.bit.UART2 == MUX_1) {
				pr_err("SPI5_1 conflict with UART2_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C == MUX_1) {
				pr_err("SPI5_1 conflict with I2C_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C2 == MUX_1) {
				pr_err("SPI5_1 conflict with I2C2_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF4 == MUX_1) {
				pr_err("SPI5_1 conflict with SIF4_1\r\n");
				return E_PAR;
			}
			if (top_reg12.bit.I2S_MCLK == MUX_1) {
				pr_err("SPI5_1 conflict with I2S_MCLK_1\r\n");
				return E_PAR;
			}
			if (top_reg15.bit.REMOTE_EXT == MUX_1) {
				pr_err("SPI5_1 conflict with REMOTE_EXT_1\r\n");
				return E_PAR;
			}
			if (top_reg3.bit.ETH_PTP == MUX_1) {
				pr_err("SPI5_1 conflict with ETH_PTP\r\n");
				return E_PAR;
			}
			if (top_reg18.bit.RTC_CLK == MUX_1) {
				pr_err("SPI5_1 conflict with RTC_CLK\r\n");
				return E_PAR;
			}

			top_reg16.bit.SPI5 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_28 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_29 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_30 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_28,3,func_SPI5_1);
			if (config & PIN_SPI_CFG_SPI5_BUS_WIDTH) {
				if (top_reg15.bit.REMOTE == MUX_1) {
					pr_err("SPI5_1 BUS_WIDTH conflict with REMOTE_1\r\n");
					return E_PAR;
				}

				top_reg16.bit.SPI5_BUS_WIDTH = MUX_1;
				top_reg_pgpio0.bit.PGPIO_31 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(PGPIO_31,1,func_SPI5_1);
			}
		} else if (config & PIN_SPI_CFG_SPI5_2) {
			if (top_reg2.bit.LCD_TYPE == MUX_1 || top_reg2.bit.LCD_TYPE == MUX_2 ||
				top_reg2.bit.LCD_TYPE == MUX_4 || top_reg2.bit.LCD_TYPE == MUX_5 || top_reg2.bit.LCD_TYPE == MUX_6) {
				pr_err("SPI5_2 conflict with LCD_TYPE\r\n");
				return E_PAR;
			}
			if (top_reg12.bit.I2S2 == MUX_1) {
				pr_err("SPI5_2 conflict with I2S2_1\r\n");
				return E_PAR;
			}
			if (top_reg2.bit.LCD2_TYPE == MUX_3 || top_reg2.bit.LCD2_TYPE == MUX_8 || top_reg2.bit.LCD2_TYPE == MUX_10) {
				pr_err("SPI5_2 conflict with LCD2_TYPE\r\n");
				return E_PAR;
			}
			if (top_reg2.bit.MEMIF_TYPE == MUX_2) {
				if ((top_reg2.bit.MEMIF_SEL == MUX_0) &&
					(top_reg2.bit.PMEMIF_DATA_WIDTH == MUX_2 || top_reg2.bit.PMEMIF_DATA_WIDTH == MUX_3)) {
					pr_err("SPI5_2 conflict with (MEMIF_TYPE = 2 && MEMIF_SEL = 0 && PMEMIF_DATA_WIDTH = 0x2 or 0x3)\r\n");
					return E_PAR;
				} else if (top_reg2.bit.MEMIF_SEL == MUX_1) {
					pr_err("SPI5_2 conflict with (MEMIF_TYPE = 2 && MEMIF_SEL = 1)\r\n");
					return E_PAR;
				}
			}
			if (top_reg3.bit.ETH == MUX_1 || top_reg3.bit.ETH == MUX_2) {
				pr_err("SPI5_2 conflict with ETH\r\n");
				return E_PAR;
			}

			top_reg16.bit.SPI5 = MUX_2;
			top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_20,3,func_SPI5_2);
			if (config & PIN_SPI_CFG_SPI5_BUS_WIDTH) {
				top_reg16.bit.SPI5_BUS_WIDTH = MUX_1;
				top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(LGPIO_23,1,func_SPI5_2);
			}
		}
	}

	return E_OK;
}

static int pinmux_config_sif(uint32_t config)
{
	if (config == PIN_SIF_CFG_NONE) {
	} else {
		if (config & PIN_SIF_CFG_SIF0_1) {
			if (top_reg13.bit.UART6 == MUX_1) {
				pr_err("SIF0_1 conflict with UART6_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C3 == MUX_1) {
				pr_err("SIF0_1 conflict with I2C3_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C4 == MUX_1) {
				pr_err("SIF0_1 conflict with I2C4_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI == MUX_1) {
				pr_err("SIF0_1 conflict with SPI_1\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR3 == MUX_1 || top_reg9.bit.SENSOR3 == MUX_3 ||
				top_reg9.bit.SENSOR3 == MUX_4 || top_reg9.bit.SENSOR3 == MUX_5) {
				pr_err("SIF0_1 conflict with SENSOR3\r\n");
				return E_PAR;
			}

			top_reg17.bit.SIF0 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_14 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_12,3,func_SIF_1);
		} else if (config & PIN_SIF_CFG_SIF0_2) {
			if (top_reg2.bit.LCD2_TYPE == MUX_2) {
				pr_err("SIF0_2 conflict with LCD2_TYPE\r\n");
				return E_PAR;
			}
			if (top_reg2.bit.CCIR_DATA_WIDTH == MUX_1 && top_reg2.bit.CCIR_FIELD == MUX_1) {
				pr_err("SIF0_2 conflict with (CCIR_DATA_WIDTH = 1 && CCIR_FIELD = 1)\r\n");
				return E_PAR;
			}
			if (top_reg5.bit.I2C9 == MUX_2) {
				pr_err("SIF0_2 conflict with I2C9_2\r\n");
				return E_PAR;
			}
			if (top_reg2.bit.LCD_TYPE == MUX_3 || top_reg2.bit.LCD_TYPE == MUX_8 || top_reg2.bit.LCD_TYPE == MUX_10) {
				pr_err("SIF0_2 conflict with LCD_TYPE\r\n");
				return E_PAR;
			}
			if (top_reg2.bit.MEMIF_TYPE == MUX_1 && top_reg2.bit.MEMIF_SEL == MUX_1 && top_reg2.bit.MEMIF_TE_SEL == MUX_1) {
				pr_err("SIF0_2 conflict with (MEMIF_TYPE = 2 && MEMIF_SEL = 1 && MEMIF_TE_SEL = 1)\r\n");
				return E_PAR;
			}
			if (top_reg3.bit.ETH == MUX_1 || top_reg3.bit.ETH == MUX_2) {
				pr_err("SIF0_2 conflict with ETH\r\n");
				return E_PAR;
			}

			top_reg17.bit.SIF0 = MUX_2;
			top_reg_lgpio0.bit.LGPIO_25 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_26 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_27 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_25,3,func_SIF_2);
		} else if (config & PIN_SIF_CFG_SIF0_3) {
			if (top_reg0.bit.DSI_PROT_EN == MUX_1) {
				pr_err("SIF0_3 conflict with DSI_PROT_EN\r\n");
				return E_PAR;
			}
			if (top_reg6.bit.PWM2 == MUX_2) {
				pr_err("SIF0_3 conflict with PWM2_2\r\n");
				return E_PAR;
			}
			if (top_reg6.bit.PWM3 == MUX_2) {
				pr_err("SIF0_3 conflict with PWM3_2\r\n");
				return E_PAR;
			}
			if (top_reg18.bit.SP_CLK2 == MUX_1) {
				pr_err("SIF0_3 conflict with SP2_CLK\r\n");
				return E_PAR;
			}

			top_reg17.bit.SIF0 = MUX_3;
			top_reg_dsigpio0.bit.DSIGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_dsigpio0.bit.DSIGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_dsigpio0.bit.DSIGPIO_10 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(DSIGPIO_8,3,func_SIF_3);
		}

		if (config & PIN_SIF_CFG_SIF1_1) {
			if (top_reg13.bit.UART5 == MUX_1) {
				pr_err("SIF1_1 conflict with UART5_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C5 == MUX_1) {
				pr_err("SIF1_1 conflict with I2C5_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C6 == MUX_1) {
				pr_err("SIF1_1 conflict with I2C6_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI2 == MUX_1) {
				pr_err("SIF1_1 conflict with SPI2_1\r\n");
				return E_PAR;
			}
			if (top_reg15.bit.SDP == MUX_1) {
				pr_err("SIF1_1 conflict with SDP_1\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR3 == MUX_1 || top_reg9.bit.SENSOR3 == MUX_3 ||
				top_reg9.bit.SENSOR3 == MUX_4 || top_reg9.bit.SENSOR3 == MUX_5) {
				pr_err("SIF1_1 conflict with SENSOR3\r\n");
				return E_PAR;
			}

			top_reg17.bit.SIF1 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_18 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_16,3,func_SIF1_1);
		} else if (config & PIN_SIF_CFG_SIF1_2) {
			if (top_reg3.bit.ETH == MUX_1 || top_reg3.bit.ETH == MUX_2) {
				pr_err("SIF1_2 conflict with ETH\r\n");
				return E_PAR;
			}

			top_reg17.bit.SIF1 = MUX_2;
			top_reg_lgpio0.bit.LGPIO_28 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_29 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_30 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_28,3,func_SIF1_2);
		}

		if (config & PIN_SIF_CFG_SIF2_1) {
			if (top_reg13.bit.UART4 == MUX_1) {
				pr_err("SIF2_1 conflict with UART4_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C7 == MUX_1) {
				pr_err("SIF2_1 conflict with I2C7_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C8 == MUX_1) {
				pr_err("SIF2_1 conflict with I2C8_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI3 == MUX_1) {
				pr_err("SIF2_1 conflict with SPI3_1\r\n");
				return E_PAR;
			}
			if (top_reg7.bit.PICNT == MUX_1) {
				pr_err("SIF2_1 conflict with PICNT\r\n");
				return E_PAR;
			}
			if (top_reg7.bit.PICNT2 == MUX_1) {
				pr_err("SIF2_1 conflict with PICNT2\r\n");
				return E_PAR;
			}
			if (top_reg10.bit.SN5_MCLK == MUX_2) {
				pr_err("SIF2_1 conflict with SN5_MCLK_2\r\n");
				return E_PAR;
			}
			if (top_reg18.bit.RTC_DIV_OUT == MUX_1) {
				pr_err("SIF2_1 conflict with RTC_DIV_OUT\r\n");
				return E_PAR;
			}

			top_reg17.bit.SIF2 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_22 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_20,3,func_SIF2_1);
		} else if (config & PIN_SIF_CFG_SIF2_2) {
			if (top_reg4.bit.I2C3 == MUX_2) {
				pr_err("SIF2_2 conflict with I2C3_2\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C5 == MUX_2) {
				pr_err("SIF2_2 conflict with I2C5_2\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR2 == MUX_1 || top_reg9.bit.SENSOR2 == MUX_3 ||
				top_reg9.bit.SENSOR2 == MUX_4 || top_reg9.bit.SENSOR2 == MUX_5) {
				pr_err("SIF2_2 conflict with SENSOR2\r\n");
				return E_PAR;
			}

			top_reg17.bit.SIF2 = MUX_2;
			top_reg_sgpio0.bit.SGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_15 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(SGPIO_11,2,func_SIF2_2);
            gpio_func_keep(SGPIO_15,1,func_SIF2_2);
		}

		if (config & PIN_SIF_CFG_SIF3_1) {
			if (top_reg13.bit.UART3 == MUX_1) {
				pr_err("SIF3_1 conflict with UART3_1\r\n");
				return E_PAR;
			}
			if (top_reg5.bit.I2C9 == MUX_1) {
				pr_err("SIF3_1 conflict with I2C9_1\r\n");
				return E_PAR;
			}
			if (top_reg5.bit.I2C10 == MUX_1) {
				pr_err("SIF3_1 conflict with I2C10_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI4 == MUX_1) {
				pr_err("SIF3_1 conflict with SPI4_1\r\n");
				return E_PAR;
			}
			if (top_reg12.bit.I2S == MUX_1) {
				pr_err("SIF3_1 conflict with I2S_1\r\n");
				return E_PAR;
			}
			if (top_reg3.bit.ETH2_PTP == MUX_1) {
				pr_err("SIF3_1 conflict with ETH2_PTP\r\n");
				return E_PAR;
			}
			if (top_reg12.bit.DMIC == MUX_1) {
				pr_err("SIF3_1 conflict with DMIC\r\n");
				return E_PAR;
			}

			top_reg17.bit.SIF3 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_24 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_25 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_26 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_24,3,func_SIF3_1);
		} else if (config & PIN_SIF_CFG_SIF3_2) {
			if (top_reg4.bit.I2C4 == MUX_2) {
				pr_err("SIF3_2 conflict with I2C4_2\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C5 == MUX_2) {
				pr_err("SIF3_2 conflict with I2C5_2\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR2 == MUX_1 || top_reg9.bit.SENSOR2 == MUX_3 ||
				top_reg9.bit.SENSOR2 == MUX_4 || top_reg9.bit.SENSOR2 == MUX_5) {
				pr_err("SIF3_2 conflict with SENSOR2\r\n");
				return E_PAR;
			}

			top_reg17.bit.SIF3 = MUX_2;
			top_reg_sgpio0.bit.SGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_16 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(SGPIO_13,2,func_SIF3_2);
            gpio_func_keep(SGPIO_16,1,func_SIF3_2);
		}

		if (config & PIN_SIF_CFG_SIF4_1) {
			if (top_reg13.bit.UART2 == MUX_1) {
				pr_err("SIF4_1 conflict with UART2_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C == MUX_1) {
				pr_err("SIF4_1 conflict with I2C_1\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C2 == MUX_1) {
				pr_err("SIF4_1 conflict with I2C2_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI5 == MUX_1) {
				pr_err("SIF4_1 conflict with SPI5_1\r\n");
				return E_PAR;
			}
			if (top_reg12.bit.I2S_MCLK == MUX_1) {
				pr_err("SIF4_1 conflict with I2S_MCLK_1\r\n");
				return E_PAR;
			}
			if (top_reg15.bit.REMOTE_EXT == MUX_1) {
				pr_err("SIF4_1 conflict with REMOTE_EXT_1\r\n");
				return E_PAR;
			}
			if (top_reg3.bit.ETH_PTP == MUX_1) {
				pr_err("SIF4_1 conflict with ETH_PTP\r\n");
				return E_PAR;
			}
			if (top_reg18.bit.RTC_CLK == MUX_1) {
				pr_err("SIF4_1 conflict with RTC_CLK\r\n");
				return E_PAR;
			}

			top_reg17.bit.SIF4 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_28 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_29 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_30 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_28,3,func_SIF4_1);
		} else if (config & PIN_SIF_CFG_SIF4_2) {
			if (top_reg4.bit.I2C6 == MUX_2) {
				pr_err("SIF4_2 conflict with I2C6_2\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C7 == MUX_2) {
				pr_err("SIF4_2 conflict with I2C7_2\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR2 == MUX_1 || top_reg9.bit.SENSOR2 == MUX_2 ||
				top_reg9.bit.SENSOR2 == MUX_4 || top_reg9.bit.SENSOR2 == MUX_5) {
				pr_err("SIF4_2 conflict with SENSOR2\r\n");
				return E_PAR;
			}

			top_reg17.bit.SIF4 = MUX_2;
			top_reg_sgpio0.bit.SGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_24 = GPIO_ID_EMUM_FUNC;
			top_reg_sgpio0.bit.SGPIO_25 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(SGPIO_23,3,func_SIF4_2);
		}

		if (config & PIN_SIF_CFG_SIF5_1) {
			if (top_reg13.bit.UART7 == MUX_1) {
				pr_err("SIF5_1 conflict with UART7_1\r\n");
				return E_PAR;
			}
			if (top_reg7.bit.PWM8 == MUX_1) {
				pr_err("SIF5_1 conflict with PWM8_1\r\n");
				return E_PAR;
			}
			if (top_reg7.bit.PWM9 == MUX_1) {
				pr_err("SIF5_1 conflict with PWM9_1\r\n");
				return E_PAR;
			}
			if (top_reg7.bit.PWM10 == MUX_1) {
				pr_err("SIF5_1 conflict with PWM10_1\r\n");
				return E_PAR;
			}
			if (top_reg9.bit.SENSOR3) {
				pr_err("SIF5_1 conflict with SENSOR3\r\n");
				return E_PAR;
			}

			top_reg17.bit.SIF5 = MUX_1;
			top_reg_pgpio0.bit.PGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_pgpio0.bit.PGPIO_10 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_8,3,func_SIF5_1);
		} else if (config & PIN_SIF_CFG_SIF5_2) {
			if (top_reg1.bit.SDIO3_BUS_WIDTH == MUX_1) {
				pr_err("SIF5_2 conflict with SDIO3_BUS_WIDTH\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C5 == MUX_3) {
				pr_err("SIF5_2 conflict with I2C5_3\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C6 == MUX_3) {
				pr_err("SIF5_2 conflict with I2C6_3\r\n");
				return E_PAR;
			}
			if (top_reg7.bit.PWM8 == MUX_2) {
				pr_err("SIF5_2 conflict with PWM8_2\r\n");
				return E_PAR;
			}
			if (top_reg7.bit.PWM9 == MUX_2) {
				pr_err("SIF5_2 conflict with PWM9_2\r\n");
				return E_PAR;
			}
			if (top_reg7.bit.PWM10 == MUX_2) {
				pr_err("SIF5_2 conflict with PWM10_2\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI == MUX_3) {
				pr_err("SIF5_2 conflict with SPI_3\r\n");
				return E_PAR;
			}
			if (top_reg13.bit.UART7 == MUX_2) {
				pr_err("SIF5_2 conflict with UART7_2\r\n");
				return E_PAR;
			}

			top_reg17.bit.SIF5 = MUX_2;
			top_reg_cgpio0.bit.CGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_cgpio0.bit.CGPIO_6 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(CGPIO_4,3,func_SIF5_2);
		}
	}

	return E_OK;
}

static int pinmux_config_misc(uint32_t config)
{
	if (config == PIN_MISC_CFG_NONE) {
	} else {
		if (config & PIN_MISC_CFG_RTC_CLK_1) {
			if ((top_reg13.bit.UART2 == 1) &&
			    (top_reg14.bit.UART2_RTSCTS == MUX_1)) {
				pr_err("RTC_CLK conflict with UART2_1 CTS\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C == MUX_1) {
				pr_err("RTC_CLK conflict with I2C_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI5 == MUX_1) {
				pr_err("RTC_CLK conflict with SPI5_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF4 == MUX_1) {
				pr_err("RTC_CLK conflict with SIF4_1\r\n");
				return E_PAR;
			}

			top_reg18.bit.RTC_CLK = MUX_1;
			top_reg_pgpio0.bit.PGPIO_29 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_29,1,func_MISC);
		}

		if (config & PIN_MISC_CFG_RTC_EXT_CLK_1) {
			if ((top_reg13.bit.UART4 == MUX_1) &&
				(top_reg14.bit.UART4_RTSCTS == MUX_1)) {
				pr_err("RTC_EXT_CLK conflict with UART4_1 CTS\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C8 == MUX_1) {
				pr_err("RTC_EXT_CLK conflict with I2C8_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI3 == MUX_1 && top_reg16.bit.SPI3_BUS_WIDTH == MUX_1) {
				pr_err("RTC_EXT_CLK conflict with SPI3_1 BUS_WIDTH\r\n");
				return E_PAR;
			}
			if (top_reg7.bit.PICNT3 == MUX_1) {
				pr_err("RTC_EXT_CLK conflict with PICNT3\r\n");
				return E_PAR;
			}

			top_reg18.bit.RTC_EXT_CLK = MUX_1;
			top_reg_pgpio0.bit.PGPIO_23 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_23,1,func_MISC);
		}

		if (config & PIN_MISC_CFG_RTC_DIV_OUT_1) {
			if ((top_reg13.bit.UART4 == MUX_1) &&
			    (top_reg14.bit.UART4_RTSCTS == MUX_1 || top_reg14.bit.UART4_RTSCTS == MUX_2)) {
				pr_err("RTC_DIV_OUT conflict with UART4_1 RTS\r\n");
				return E_PAR;
			}
			if (top_reg4.bit.I2C8 == MUX_1) {
				pr_err("RTC_DIV_OUT conflict with I2C8_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI3 == MUX_1) {
				pr_err("RTC_DIV_OUT conflict with SPI3_1\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF2 == MUX_1) {
				pr_err("RTC_DIV_OUT conflict with SIF2_1\r\n");
				return E_PAR;
			}
			if (top_reg7.bit.PICNT2 == MUX_1) {
				pr_err("RTC_DIV_OUT conflict with PICNT2\r\n");
				return E_PAR;
			}

			top_reg18.bit.RTC_DIV_OUT = MUX_1;
			top_reg_pgpio0.bit.PGPIO_22 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(PGPIO_22,1,func_MISC);
		}

		if (config & PIN_MISC_CFG_SP_CLK_1) {
			if (top_reg14.bit.UART9 == MUX_2) {
				pr_err("SP_CLK conflict with UART9_2\r\n");
				return E_PAR;
			}
			if (top_reg3.bit.ETH == MUX_1 || top_reg3.bit.ETH == MUX_2) {
				pr_err("SP_CLK conflict with ETH\r\n");
				return E_PAR;
			}

			top_reg18.bit.SP_CLK = MUX_1;
			top_reg_dgpio0.bit.DGPIO_6 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(DGPIO_6,1,func_MISC);

		}

		if (config & PIN_MISC_CFG_SP2_CLK_1) {
			if (top_reg2.bit.LCD_TYPE == MUX_9) {
				pr_err("SP2_CLK conflict with (LCD_TYPE = 0x9)\r\n");
				return E_PAR;
			}
			if (top_reg2.bit.LCD2_TYPE == MUX_9) {
				pr_err("SP2_CLK conflict with (LCD2_TYPE = 0x9)\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF0 == MUX_3) {
				pr_err("SP2_CLK conflict with SIF0_3\r\n");
				return E_PAR;
			}

			top_reg18.bit.SP_CLK2 = MUX_1;
			top_reg_dsigpio0.bit.DSIGPIO_10 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(DSIGPIO_10,1,func_MISC);
		}

		if (config & PIN_MISC_CFG_SATA_LED_1) {
			if (top_reg6.bit.PWM6 == MUX_2) {
				pr_err("SATA_LED conflict with PWM6_2\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI4 == MUX_2) {
				pr_err("SATA_LED conflict with SPI4_2\r\n");
				return E_PAR;
			}

			top_reg19.bit.SATA_LED = MUX_1;
			top_reg_dgpio0.bit.DGPIO_4 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(DGPIO_4,1,func_MISC);
		}
	}

	return E_OK;
}

static int pinmux_config_lcd(uint32_t config)
{
	uint32_t tmp;

	tmp = config & PINMUX_DISPMUX_SEL_MASK;
	if (tmp == PINMUX_DISPMUX_SEL_MASK) {
		pr_err("invalid locate: 0x%x\r\n", config);
		return E_PAR;
	}

	disp_pinmux_config[PINMUX_FUNC_ID_LCD] = config;

	return E_OK;
}

static int pinmux_config_lcd2(uint32_t config)
{
	uint32_t tmp;

	tmp = config & PINMUX_DISPMUX_SEL_MASK;
	if (tmp == PINMUX_DISPMUX_SEL_MASK) {
		pr_err("invalid locate: 0x%x\r\n", config);
		return E_PAR;
	}

	disp_pinmux_config[PINMUX_FUNC_ID_LCD2] = config;

	return E_OK;
}

static int pinmux_config_tv(uint32_t config)
{
	uint32_t tmp;

	tmp = config & PINMUX_TV_HDMI_CFG_MASK;
	if ((tmp != PINMUX_TV_HDMI_CFG_NORMAL) && (tmp != PINMUX_TV_HDMI_CFG_PINMUX_ON)) {
		pr_err("invalid config: 0x%x\r\n", config);
		return E_PAR;
	}

	disp_pinmux_config[PINMUX_FUNC_ID_TV] = config;

	return E_OK;
}

static int pinmux_select_primary_lcd(uint32_t config)
{
	u32 pinmux_type;

	pinmux_type = config & ~(PINMUX_LCD_SEL_FEATURE_MSK);

	if (pinmux_type == PINMUX_LCD_SEL_GPIO) {
	} else if (pinmux_type <= PINMUX_LCD_SEL_MIPI) {  // lcd type
		if (pinmux_type == PINMUX_LCD_SEL_PARALLE_RGB888) {
			if (top_reg12.bit.I2S2 == MUX_1) {
				pr_err("(LCD_TYPE = 0xA) conflict with I2S2_1\r\n");
				return E_PAR;
			}
		}
		if (pinmux_type != PINMUX_LCD_SEL_MIPI) {
			if (top_reg3.bit.ETH2 == MUX_1 || top_reg3.bit.ETH2 == MUX_2) {
				pr_err("(LCD_TYPE = 0x1~0x8, 0xA) conflict with ETH2\r\n");
				return E_PAR;
			}
		}
		if (pinmux_type == PINMUX_LCD_SEL_PARALLE_RGB565 ||
			pinmux_type == PINMUX_LCD_SEL_RGB_16BITS ||
			pinmux_type == PINMUX_LCD_SEL_PARALLE_RGB666 ||
			pinmux_type == PINMUX_LCD_SEL_PARALLE_RGB888) {
			if (top_reg3.bit.ETH == MUX_1 || top_reg3.bit.ETH == MUX_2) {
				pr_err("(LCD_TYPE = 0x3, 0x7~0x8, 0xA) conflict with ETH\r\n");
				return E_PAR;
			}
		}

		if (pinmux_type == PINMUX_LCD_SEL_CCIR656) {
			top_reg2.bit.LCD_TYPE = MUX_1;
			top_reg2.bit.CCIR_DATA_WIDTH = MUX_0;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_0,9,func_LCD);
		} else if (pinmux_type == PINMUX_LCD_SEL_CCIR656_16BITS) {
			if (top_reg2.bit.LCD2_TYPE == MUX_1 || top_reg2.bit.LCD2_TYPE == MUX_2 ||
				top_reg2.bit.LCD2_TYPE == MUX_4 || top_reg2.bit.LCD2_TYPE == MUX_5 || top_reg2.bit.LCD2_TYPE == MUX_6) {
				pr_err("(LCD_TYPE = 0x1 && CCIR_DATA_WIDTH) conflict with LCD2_TYPE\r\n");
				return E_PAR;
			}
			if (top_reg2.bit.MEMIF_TYPE == MUX_1 && top_reg2.bit.MEMIF_SEL == MUX_1) {
				pr_err("(LCD_TYPE = 0x1 && CCIR_DATA_WIDTH) conflict with (MEMIF_TYPE = 1 && MEMIF_SEL = 1)\r\n");
				return E_PAR;
			}
			if (top_reg2.bit.MEMIF_TYPE == MUX_2 && top_reg2.bit.MEMIF_SEL == MUX_1) {
				pr_err("(LCD_TYPE = 0x1 && CCIR_DATA_WIDTH) conflict with (MEMIF_TYPE = 2 && MEMIF_SEL = 1)\r\n");
				return E_PAR;
			}
			if (top_reg3.bit.ETH == MUX_1 || top_reg3.bit.ETH == MUX_2) {
				pr_err("(LCD_TYPE = 0x1 && CCIR_DATA_WIDTH) conflict with ETH\r\n");
				return E_PAR;
			}

			top_reg2.bit.LCD_TYPE = MUX_1;
			top_reg2.bit.CCIR_DATA_WIDTH = MUX_1;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_0,9,func_LCD);
            gpio_func_keep(LGPIO_12,8,func_LCD);
		} else if (pinmux_type == PINMUX_LCD_SEL_CCIR601) {
			top_reg2.bit.LCD_TYPE = MUX_2;
			top_reg2.bit.CCIR_DATA_WIDTH = MUX_0;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_0,11,func_LCD);
			if (config & PINMUX_LCD_SEL_HVLD_VVLD) {
				if (top_reg2.bit.LCD2_TYPE == MUX_1 || top_reg2.bit.LCD2_TYPE == MUX_2 ||
					top_reg2.bit.LCD2_TYPE == MUX_4 || top_reg2.bit.LCD2_TYPE == MUX_5 || top_reg2.bit.LCD2_TYPE == MUX_6) {
					pr_err("(LCD_TYPE = 0x2 && CCIR_HVLD_VVLD) conflict with LCD2_TYPE\r\n");
					return E_PAR;
				}
				if (top_reg2.bit.MEMIF_TYPE == MUX_1 && top_reg2.bit.MEMIF_SEL == MUX_1) {
					pr_err("(LCD_TYPE = 0x2 && CCIR_HVLD_VVLD) conflict with (MEMIF_TYPE = 1 && MEMIF_SEL = 1)\r\n");
					return E_PAR;
				}
				if (top_reg2.bit.MEMIF_TYPE == MUX_2 && top_reg2.bit.MEMIF_SEL == MUX_1) {
					pr_err("(LCD_TYPE = 0x2 && CCIR_HVLD_VVLD) conflict with (MEMIF_TYPE = 2 && MEMIF_SEL = 1)\r\n");
					return E_PAR;
				}

				top_reg2.bit.CCIR_HVLD_VVLD = MUX_1;
				top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
				top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(LGPIO_12,2,func_LCD);
			}

			if (config & PINMUX_LCD_SEL_FIELD) {
				if (top_reg2.bit.LCD2_TYPE == MUX_1 || top_reg2.bit.LCD2_TYPE == MUX_2 ||
					top_reg2.bit.LCD2_TYPE == MUX_4 || top_reg2.bit.LCD2_TYPE == MUX_5 || top_reg2.bit.LCD2_TYPE == MUX_6) {
					pr_err("(LCD_TYPE = 0x2 && CCIR_FIELD) conflict with LCD2_TYPE\r\n");
					return E_PAR;
				}
				if (top_reg2.bit.MEMIF_TYPE == MUX_1 && top_reg2.bit.MEMIF_SEL == MUX_1) {
					pr_err("(LCD_TYPE = 0x2 && CCIR_FIELD) conflict with (MEMIF_TYPE = 1 && MEMIF_SEL = 1)\r\n");
					return E_PAR;
				}
				if (top_reg2.bit.MEMIF_TYPE == MUX_2 && top_reg2.bit.MEMIF_SEL == MUX_1) {
					pr_err("(LCD_TYPE = 0x2 && CCIR_FIELD) conflict with (MEMIF_TYPE = 2 && MEMIF_SEL = 1)\r\n");
					return E_PAR;
				}

				top_reg2.bit.CCIR_FIELD = MUX_1;
				top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(LGPIO_14,1,func_LCD);
			}
		} else if (pinmux_type == PINMUX_LCD_SEL_CCIR601_16BITS) {
			if (top_reg2.bit.LCD2_TYPE == MUX_1 || top_reg2.bit.LCD2_TYPE == MUX_2 ||
				top_reg2.bit.LCD2_TYPE == MUX_4 || top_reg2.bit.LCD2_TYPE == MUX_5 || top_reg2.bit.LCD2_TYPE == MUX_6) {
				pr_err("(LCD_TYPE = 0x2 && CCIR_DATA_WIDTH) conflict with LCD2_TYPE\r\n");
				return E_PAR;
			}
			if (top_reg2.bit.MEMIF_TYPE == MUX_1 && top_reg2.bit.MEMIF_SEL == MUX_1) {
				pr_err("(LCD_TYPE = 0x2 && CCIR_DATA_WIDTH) conflict with (MEMIF_TYPE = 1 && MEMIF_SEL = 1)\r\n");
				return E_PAR;
			}
			if (top_reg2.bit.MEMIF_TYPE == MUX_2 && top_reg2.bit.MEMIF_SEL == MUX_1) {
				pr_err("(LCD_TYPE = 0x2 && CCIR_DATA_WIDTH) conflict with (MEMIF_TYPE = 2 && MEMIF_SEL = 1)\r\n");
				return E_PAR;
			}
			if (top_reg3.bit.ETH == MUX_1 || top_reg3.bit.ETH == MUX_2) {
				pr_err("(LCD_TYPE = 0x2 && CCIR_DATA_WIDTH) conflict with ETH\r\n");
				return E_PAR;
			}

			top_reg2.bit.LCD_TYPE = MUX_2;
			top_reg2.bit.CCIR_DATA_WIDTH = MUX_1;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_0,11,func_LCD);
            gpio_func_keep(LGPIO_12,8,func_LCD);
			if (config & PINMUX_LCD_SEL_FIELD) {
				if (top_reg17.bit.SIF0 == MUX_2) {
					pr_err("(LCD_TYPE = 0x2 && CCIR_FIELD) conflict with SIF0_2\r\n");
					return E_PAR;
				}
				if (top_reg2.bit.MEMIF_TYPE == MUX_2 && top_reg2.bit.MEMIF_SEL == MUX_1 && top_reg2.bit.TE_SEL == MUX_1) {
					pr_err("(LCD_TYPE = 0x2 && CCIR_FIELD) conflict with (MEMIF_TYPE = 2 && MEMIF_SEL = 1 && MEMIF_TE_SEL = 1)\r\n");
					return E_PAR;
				}
				if (top_reg3.bit.ETH == MUX_1 || top_reg3.bit.ETH == MUX_2) {
					pr_err("(LCD_TYPE = 0x2 && CCIR_FIELD) conflict with ETH\r\n");
					return E_PAR;
				}

				top_reg2.bit.CCIR_FIELD = MUX_1;
				top_reg_lgpio0.bit.LGPIO_25 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(LGPIO_25,1,func_LCD);
			}
		} else if (pinmux_type == PINMUX_LCD_SEL_PARALLE_RGB565) {
			top_reg2.bit.LCD_TYPE = MUX_3;
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_0,11,func_LCD);
            gpio_func_keep(LGPIO_12,8,func_LCD);
		} else if (pinmux_type == PINMUX_LCD_SEL_SERIAL_RGB_8BITS) {
			top_reg2.bit.LCD_TYPE = MUX_4;
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_0,11,func_LCD);
		} else if (pinmux_type == PINMUX_LCD_SEL_SERIAL_RGB_6BITS) {
			top_reg2.bit.LCD_TYPE = MUX_5;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_2,9,func_LCD);
		} else if (pinmux_type == PINMUX_LCD_SEL_SERIAL_YCbCr_8BITS) {
			top_reg2.bit.LCD_TYPE = MUX_6;
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_0,11,func_LCD);
		} else if (pinmux_type == PINMUX_LCD_SEL_RGB_16BITS) {
			top_reg2.bit.LCD_TYPE = MUX_7;
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_0,11,func_LCD);
            gpio_func_keep(LGPIO_12,8,func_LCD);
		} else if (pinmux_type == PINMUX_LCD_SEL_PARALLE_RGB666) {
			top_reg2.bit.LCD_TYPE = MUX_8;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_24 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_25 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_26 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_27 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_2,9,func_LCD);
            gpio_func_keep(LGPIO_14,6,func_LCD);
            gpio_func_keep(LGPIO_22,6,func_LCD);
		} else if (pinmux_type == PINMUX_LCD_SEL_MIPI) {
			top_reg2.bit.LCD_TYPE = MUX_9;
		} else if (pinmux_type == PINMUX_LCD_SEL_PARALLE_RGB888) {
			top_reg2.bit.LCD_TYPE = MUX_10;
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_24 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_25 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_26 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_27 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_0,11,func_LCD);
            gpio_func_keep(LGPIO_12,16,func_LCD);
		}

		if (config & PINMUX_LCD_SEL_TE_ENABLE) {
			if (top_reg18.bit.SP_CLK2 == MUX_1) {
				pr_err("TE_SEL conflict with SP2_CLK\r\n");
				return E_PAR;
			}
			if (top_reg17.bit.SIF0 == MUX_3) {
				pr_err("TE_SEL conflict with SIF0_3\r\n");
				return E_PAR;
			}

			top_reg2.bit.TE_SEL = MUX_1;
			top_reg_dsigpio0.bit.DSIGPIO_10 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(DSIGPIO_10,1,func_LCD);
		}

		if (config & PINMUX_LCD_SEL_DE_ENABLE) {
			top_reg2.bit.PLCD_DE = MUX_1;
			top_reg_lgpio0.bit.LGPIO_11 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_11,1,func_LCD);
		}
	} else if (pinmux_type <= PINMUX_LCD_SEL_SERIAL_MI_SDI_SDO) {  // mi type
		if (top_reg3.bit.ETH2 == MUX_1 || top_reg3.bit.ETH2 == MUX_2) {
			pr_err("(MEMIF_SEL = 0) conflict with ETH2\r\n");
			return E_PAR;
		}

		if (pinmux_type == PINMUX_LCD_SEL_SERIAL_MI_SDIO) {
			top_reg2.bit.MEMIF_TYPE = MUX_1;
			top_reg2.bit.MEMIF_SEL = MUX_0;
			top_reg2.bit.SMEMIF_DATA_WIDTH = MUX_0;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_7,4,func_LCD);
			if (config & PINMUX_LCD_SEL_TE_ENABLE) {
				top_reg2.bit.MEMIF_TE_SEL = MUX_1;
				top_reg_lgpio0.bit.LGPIO_11 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(LGPIO_11,1,func_LCD);
			}
		} else if (pinmux_type == PINMUX_LCD_SEL_SERIAL_MI_SDI_SDO) {
			top_reg2.bit.MEMIF_TYPE = MUX_1;
			top_reg2.bit.MEMIF_SEL = MUX_0;
			top_reg2.bit.SMEMIF_DATA_WIDTH = MUX_1;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_5,2,func_LCD);
            gpio_func_keep(LGPIO_8,3,func_LCD);
			if (config & PINMUX_LCD_SEL_TE_ENABLE) {
				top_reg2.bit.MEMIF_TE_SEL = MUX_1;
				top_reg_lgpio0.bit.LGPIO_11 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(LGPIO_11,1,func_LCD);
			}
		} else if (pinmux_type == PINMUX_LCD_SEL_PARALLE_MI_8BITS) {
			top_reg2.bit.MEMIF_TYPE = MUX_2;
			top_reg2.bit.MEMIF_SEL = MUX_0;
			top_reg2.bit.PMEMIF_DATA_WIDTH = MUX_0;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_0,12,func_LCD);
			if (config & PINMUX_LCD_SEL_TE_ENABLE) {
				top_reg2.bit.MEMIF_TE_SEL = MUX_1;
				top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(LGPIO_22,1,func_LCD);
			}
		} else if (pinmux_type == PINMUX_LCD_SEL_PARALLE_MI_9BITS) {
			top_reg2.bit.MEMIF_TYPE = MUX_2;
			top_reg2.bit.MEMIF_SEL = MUX_0;
			top_reg2.bit.PMEMIF_DATA_WIDTH = MUX_1;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_0,13,func_LCD);
			if (config & PINMUX_LCD_SEL_TE_ENABLE) {
				top_reg2.bit.MEMIF_TE_SEL = MUX_1;
				top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(LGPIO_22,1,func_LCD);
			}
		} else if (pinmux_type == PINMUX_LCD_SEL_PARALLE_MI_16BITS) {
			if (top_reg2.bit.LCD2_TYPE == MUX_1 || top_reg2.bit.LCD2_TYPE == MUX_2 ||
				top_reg2.bit.LCD2_TYPE == MUX_4 || top_reg2.bit.LCD2_TYPE == MUX_5 || top_reg2.bit.LCD2_TYPE == MUX_6) {
				pr_err("(MEMIF_TYPE = 2 && MEMIF_SEL = 0 && PMEMIF_DATA_WIDTH = 2) conflict with LCD2_TYPE\r\n");
				return E_PAR;
			}
			if (top_reg12.bit.I2S2 == MUX_1) {
				pr_err("(MEMIF_TYPE = 2 && MEMIF_SEL = 0 && PMEMIF_DATA_WIDTH = 2) conflict with I2S2_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI5 == MUX_2) {
				pr_err("(MEMIF_TYPE = 2 && MEMIF_SEL = 0 && PMEMIF_DATA_WIDTH = 2) conflict with SPI5_2\r\n");
				return E_PAR;
			}

			top_reg2.bit.MEMIF_TYPE = MUX_2;
			top_reg2.bit.MEMIF_SEL = MUX_0;
			top_reg2.bit.PMEMIF_DATA_WIDTH = MUX_2;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_0,20,func_LCD);
			if (config & PINMUX_LCD_SEL_TE_ENABLE) {
				top_reg2.bit.MEMIF_TE_SEL = MUX_1;
				top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(LGPIO_22,1,func_LCD);
			}
		} else if (pinmux_type == PINMUX_LCD_SEL_PARALLE_MI_18BITS) {
			if (top_reg2.bit.LCD2_TYPE == MUX_1 || top_reg2.bit.LCD2_TYPE == MUX_2 ||
				top_reg2.bit.LCD2_TYPE == MUX_4 || top_reg2.bit.LCD2_TYPE == MUX_5 || top_reg2.bit.LCD2_TYPE == MUX_6) {
				pr_err("(MEMIF_TYPE = 2 && MEMIF_SEL = 0 && PMEMIF_DATA_WIDTH = 3) conflict with LCD2_TYPE\r\n");
				return E_PAR;
			}
			if (top_reg12.bit.I2S2 == MUX_1) {
				pr_err("(MEMIF_TYPE = 2 && MEMIF_SEL = 0 && PMEMIF_DATA_WIDTH = 3) conflict with I2S2_1\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI5 == MUX_2) {
				pr_err("(MEMIF_TYPE = 2 && MEMIF_SEL = 0 && PMEMIF_DATA_WIDTH = 3) conflict with SPI5_2\r\n");
				return E_PAR;
			}

			top_reg2.bit.MEMIF_TYPE = MUX_2;
			top_reg2.bit.MEMIF_SEL = MUX_0;
			top_reg2.bit.PMEMIF_DATA_WIDTH = MUX_3;
			top_reg_lgpio0.bit.LGPIO_8 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_9 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_10 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_11 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_0 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_1 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_2 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_3 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_4 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_5 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_6 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_7 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_0,22,func_LCD);
			if (config & PINMUX_LCD_SEL_TE_ENABLE) {
				top_reg2.bit.MEMIF_TE_SEL = MUX_1;
				top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(LGPIO_22,1,func_LCD);
			}
		}
	}

	return E_OK;
}

static int pinmux_select_secondary_lcd(uint32_t config)
{
	u32 pinmux_type;

	pinmux_type = config & ~(PINMUX_LCD_SEL_FEATURE_MSK);

	if (pinmux_type == PINMUX_LCD_SEL_GPIO) {
	} else if (pinmux_type <= PINMUX_LCD_SEL_MIPI) {  // lcd type
		if (pinmux_type != PINMUX_LCD_SEL_MIPI) {
			if (top_reg2.bit.LCD_TYPE == MUX_3 || top_reg2.bit.LCD_TYPE == MUX_7 ||
				top_reg2.bit.LCD_TYPE == MUX_8 || top_reg2.bit.LCD_TYPE == MUX_10) {
				pr_err("(LCD2_TYPE = 0x1~0x2, 0x4~0x6) conflict with LCD_TYPE\r\n");
				return E_PAR;
			}
			if (top_reg2.bit.CCIR_DATA_WIDTH == MUX_1) {
				pr_err("(LCD2_TYPE = 0x1~0x2, 0x4~0x6) conflict with CCIR_DATA_WIDTH\r\n");
				return E_PAR;
			}
			if (top_reg2.bit.CCIR_HVLD_VVLD == MUX_1) {
				pr_err("(LCD2_TYPE = 0x1~0x2, 0x4~0x6) conflict with CCIR_HVLD_VVLD\r\n");
				return E_PAR;
			}
			if (top_reg12.bit.I2S == MUX_1) {
				pr_err("(LCD2_TYPE = 0x1~0x2, 0x4~0x6) conflict with I2S_1\r\n");
				return E_PAR;
			}
			if (top_reg2.bit.MEMIF_TYPE == MUX_2 &&
				top_reg2.bit.MEMIF_SEL == MUX_0 &&
				(top_reg2.bit.PMEMIF_DATA_WIDTH == MUX_2 || top_reg2.bit.PMEMIF_DATA_WIDTH == MUX_3)) {
				pr_err("(LCD2_TYPE = 0x1~0x2, 0x4~0x6) conflict with (MEMIF_TYPE = 2 && MEMIF_SEL = 0 && PMEMIF_DATA_WIDTH = 0x2 or 0x3)\r\n");
				return E_PAR;
			}
			if (top_reg16.bit.SPI5 == MUX_2) {
				pr_err("(LCD2_TYPE = 0x1~0x2, 0x4~0x6) conflict with SPI5_2\r\n");
				return E_PAR;
			}
			if (top_reg3.bit.ETH == MUX_1 || top_reg3.bit.ETH == MUX_2) {
				pr_err("(LCD2_TYPE = 0x1~0x2, 0x4~0x6) conflict with ETH\r\n");
				return E_PAR;
			}
			if (top_reg3.bit.ETH2 == MUX_1 || top_reg3.bit.ETH2 == MUX_2) {
				pr_err("(LCD2_TYPE = 0x1~0x2, 0x4~0x6) conflict with ETH2\r\n");
				return E_PAR;
			}
		}

		if (pinmux_type == PINMUX_LCD_SEL_CCIR656) {
			top_reg2.bit.LCD2_TYPE = MUX_1;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_13,9,func_LCD2);
		} else if (pinmux_type == PINMUX_LCD_SEL_CCIR601) {
			if (top_reg17.bit.SIF0 == MUX_2) {
				pr_err("(LCD2_TYPE = 0x2) conflict with SIF0_2\r\n");
				return E_PAR;
			}

			top_reg2.bit.LCD2_TYPE = MUX_2;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_25 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_13,13,func_LCD2);
		} else if (pinmux_type == PINMUX_LCD_SEL_SERIAL_RGB_8BITS) {
			top_reg2.bit.LCD2_TYPE = MUX_4;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_13,11,func_LCD2);
		} else if (pinmux_type == PINMUX_LCD_SEL_SERIAL_RGB_6BITS) {
			top_reg2.bit.LCD2_TYPE = MUX_5;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_15,9,func_LCD2);
		} else if (pinmux_type == PINMUX_LCD_SEL_SERIAL_YCbCr_8BITS) {
			top_reg2.bit.LCD2_TYPE = MUX_6;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_13,11,func_LCD2);
		} else if (pinmux_type == PINMUX_LCD_SEL_MIPI) {
			top_reg2.bit.LCD_TYPE = MUX_9;
		}

		if (config & PINMUX_LCD_SEL_TE_ENABLE) {
			top_reg2.bit.TE_SEL = MUX_1;
			top_reg_dsigpio0.bit.DSIGPIO_10 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(DSIGPIO_10,1,func_LCD2);
		}

		if (config & PINMUX_LCD_SEL_DE_ENABLE) {
			top_reg2.bit.PLCD2_DE = MUX_1;
			top_reg_lgpio0.bit.LGPIO_24 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_24,1,func_LCD2);
		}
	} else if (pinmux_type <= PINMUX_LCD_SEL_SERIAL_MI_SDI_SDO) {  // mi type
		if (top_reg2.bit.LCD_TYPE == MUX_3 || top_reg2.bit.LCD_TYPE == MUX_7 ||
			top_reg2.bit.LCD_TYPE == MUX_8 || top_reg2.bit.LCD_TYPE == MUX_10) {
			pr_err("(MEMIF_SEL = 1) conflict with LCD_TYPE\r\n");
			return E_PAR;
		}
		if (top_reg2.bit.CCIR_DATA_WIDTH == MUX_1) {
			pr_err("(MEMIF_SEL = 1) conflict with CCIR_DATA_WIDTH\r\n");
			return E_PAR;
		}
		if (top_reg2.bit.CCIR_HVLD_VVLD == MUX_1) {
			pr_err("(MEMIF_SEL = 1) conflict with CCIR_HVLD_VVLD\r\n");
			return E_PAR;
		}
		if (top_reg2.bit.CCIR_FIELD == MUX_1) {
			pr_err("(MEMIF_SEL = 1) conflict with CCIR_FIELD\r\n");
			return E_PAR;
		}
		if (top_reg3.bit.ETH2 == MUX_1 || top_reg3.bit.ETH2 == MUX_2) {
			pr_err("(MEMIF_SEL = 1) conflict with ETH2\r\n");
			return E_PAR;
		}

		if (pinmux_type == PINMUX_LCD_SEL_SERIAL_MI_SDIO) {
			top_reg2.bit.MEMIF_TYPE = MUX_1;
			top_reg2.bit.MEMIF_SEL = MUX_1;
			top_reg2.bit.SMEMIF_DATA_WIDTH = MUX_0;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_14,4,func_LCD2);
			if (config & PINMUX_LCD_SEL_TE_ENABLE) {
				top_reg2.bit.MEMIF_TE_SEL = MUX_1;
				top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(LGPIO_18,1,func_LCD2);
			}
		} else if (pinmux_type == PINMUX_LCD_SEL_SERIAL_MI_SDI_SDO) {
			top_reg2.bit.MEMIF_TYPE = MUX_1;
			top_reg2.bit.MEMIF_SEL = MUX_1;
			top_reg2.bit.SMEMIF_DATA_WIDTH = MUX_1;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_12,2,func_LCD2);
            gpio_func_keep(LGPIO_15,3,func_LCD2);
			if (config & PINMUX_LCD_SEL_TE_ENABLE) {
				top_reg2.bit.MEMIF_TE_SEL = MUX_1;
				top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(LGPIO_18,1,func_LCD2);
			}
		} else if (pinmux_type == PINMUX_LCD_SEL_PARALLE_MI_8BITS) {
			if (top_reg12.bit.I2S2 == MUX_1) {
				pr_err("(MEMIF_TYPE = 2 && MEMIF_SEL = 1 && PMEMIF_DATA_WIDTH = 0) conflict with I2S2_1\r\n");
				return E_PAR;
			}
			if (top_reg3.bit.ETH == MUX_1 || top_reg3.bit.ETH == MUX_2) {
				pr_err("(MEMIF_TYPE = 2 && MEMIF_SEL = 1 && PMEMIF_DATA_WIDTH = 0) conflict with ETH\r\n");
				return E_PAR;
			}

			top_reg2.bit.MEMIF_TYPE = MUX_2;
			top_reg2.bit.MEMIF_SEL = MUX_1;
			top_reg2.bit.PMEMIF_DATA_WIDTH = MUX_0;
			top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_24 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_12,8,func_LCD2);
            gpio_func_keep(LGPIO_21,4,func_LCD2);
			if (config & PINMUX_LCD_SEL_TE_ENABLE) {
				top_reg2.bit.MEMIF_TE_SEL = MUX_1;
				top_reg_lgpio0.bit.LGPIO_25 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(LGPIO_25,1,func_LCD2);
			}
		} else if (pinmux_type == PINMUX_LCD_SEL_PARALLE_MI_9BITS) {
			if (top_reg12.bit.I2S2 == MUX_1) {
				pr_err("(MEMIF_TYPE = 2 && MEMIF_SEL = 1 && PMEMIF_DATA_WIDTH = 1) conflict with I2S2_1\r\n");
				return E_PAR;
			}
			if (top_reg3.bit.ETH == MUX_1 || top_reg3.bit.ETH == MUX_2) {
				pr_err("(MEMIF_TYPE = 2 && MEMIF_SEL = 1 && PMEMIF_DATA_WIDTH = 1) conflict with ETH\r\n");
				return E_PAR;
			}

			top_reg2.bit.MEMIF_TYPE = MUX_2;
			top_reg2.bit.MEMIF_SEL = MUX_1;
			top_reg2.bit.PMEMIF_DATA_WIDTH = MUX_1;
			top_reg_lgpio0.bit.LGPIO_21 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_22 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_23 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_24 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_12 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_13 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_14 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_15 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_16 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_17 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_18 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_19 = GPIO_ID_EMUM_FUNC;
			top_reg_lgpio0.bit.LGPIO_20 = GPIO_ID_EMUM_FUNC;
            gpio_func_keep(LGPIO_12,13,func_LCD2);
			if (config & PINMUX_LCD_SEL_TE_ENABLE) {
				top_reg2.bit.MEMIF_TE_SEL = MUX_1;
				top_reg_lgpio0.bit.LGPIO_25 = GPIO_ID_EMUM_FUNC;
                gpio_func_keep(LGPIO_25,1,func_LCD2);
			}
		}
	}

	return E_OK;
}

typedef int (*PINMUX_CONFIG_HDL)(uint32_t);
static PINMUX_CONFIG_HDL pinmux_config_hdl[] =
{
	pinmux_config_sdio,
	pinmux_config_nand,
	pinmux_config_eth,
	pinmux_config_i2c,
	pinmux_config_i2cII,
	pinmux_config_pwm,
	pinmux_config_pwmII,
	pinmux_config_ccnt,
	pinmux_config_sensor,
	pinmux_config_sensor2,
	pinmux_config_sensor3,
	pinmux_config_sensormisc,
	pinmux_config_sensorsync,
	pinmux_config_mipi_lvds,
	pinmux_config_audio,
	pinmux_config_uart,
	pinmux_config_uartII,
	pinmux_config_remote,
	pinmux_config_sdp,
	pinmux_config_spi,
	pinmux_config_sif,
	pinmux_config_misc,
	pinmux_config_lcd,
	pinmux_config_lcd2,
	pinmux_config_tv,
	pinmux_select_primary_lcd,
	pinmux_select_secondary_lcd,
};

/**
	Configure pinmux controller

	Configure pinmux controller by upper layer

	@param[in] info	nvt_pinctrl_info
	@return void
*/
ER pinmux_init(struct nvt_pinctrl_info *info)
{
	uint32_t i;
	int err;
	//unsigned long flags = 0;

	//pr_info("%s, %s\r\n", __func__, DRV_VERSION);

	/*Assume all PINMUX is GPIO*/
	top_reg1.reg = 0; // 0x04
	top_reg2.reg = 0; // 0x08
	top_reg3.reg = 0; // 0x0c
	top_reg4.reg = 0; // 0x10
	top_reg5.reg = 0; // 0x14
	top_reg6.reg = 0; // 0x18
	top_reg7.reg = 0x01; // 0x1c
	top_reg8.reg = 0x01; // 0x20
	top_reg9.reg = 0x100000; // 0x24
	top_reg10.reg = 0; // 0x28
	top_reg11.reg = 0; // 0x2c
	top_reg12.reg = 0; // 0x30
	top_reg13.reg = 1; // 0x34
	top_reg14.reg = 0;
	top_reg15.reg = 0;
	top_reg16.reg = 0;
	top_reg17.reg = 0;
	top_reg18.reg = 0;
	top_reg19.reg = 0;
	top_reg_cgpio0.reg = 0x00FFFFFF;
	top_reg_pgpio0.reg = 0xFFFFFFFF;
	top_reg_pgpio1.reg = 0x3C; // 0xac
	top_reg_dgpio0.reg = 0x3FFF;
	top_reg_lgpio0.reg = 0;
	top_reg_sgpio0.reg = 0x07FFFFFF;
	top_reg_hsigpio0.reg = 0x00FFFFFF;
	top_reg_dsigpio0.reg = 0x7FF;
	top_reg_agpio0.reg = 0xF;

	top_reg0.reg = TOP_GETREG(info, TOP_REG0_OFS);
	if (top_reg0.bit.EJTAG_SEL) {
		top_reg1.bit.EJTAG_EN = 1;
		top_reg_dgpio0.reg = 0x000001FF;
	} else {
		top_reg_dgpio0.reg = 0x00003FFF;
	}
    for(i=0;i<GPIO_total;i++)
    {
        dump_gpio_func[i]=0;
    }
	/* Enter critical section */
	//loc_cpu(flags);
	
	for (i = 0; i < PIN_FUNC_MAX; i++) {
		if (info->top_pinmux[i].pin_function != i) {
			pr_err("top_config[%d].pinFunction context error\n", i);
			/*Leave critical section*/
			//unl_cpu(flags);
			return E_CTX;
		}
		err = pinmux_config_hdl[i](info->top_pinmux[i].config);
		if (err != E_OK) {
			pr_err("top_config[%d].config config error\n", i);
			/*Leave critical section*/
			//unl_cpu(flags);
			return err;
		}
	}
	TOP_SETREG(info, TOP_REG1_OFS, top_reg1.reg);
	TOP_SETREG(info, TOP_REG2_OFS, top_reg2.reg);
	TOP_SETREG(info, TOP_REG3_OFS, top_reg3.reg);
	TOP_SETREG(info, TOP_REG4_OFS, top_reg4.reg);
	TOP_SETREG(info, TOP_REG5_OFS, top_reg5.reg);
	TOP_SETREG(info, TOP_REG6_OFS, top_reg6.reg);
	TOP_SETREG(info, TOP_REG7_OFS, top_reg7.reg);
	TOP_SETREG(info, TOP_REG8_OFS, top_reg8.reg);
	TOP_SETREG(info, TOP_REG9_OFS, top_reg9.reg);
	TOP_SETREG(info, TOP_REG10_OFS, top_reg10.reg);
	TOP_SETREG(info, TOP_REG11_OFS, top_reg11.reg);
	TOP_SETREG(info, TOP_REG12_OFS, top_reg12.reg);
	TOP_SETREG(info, TOP_REG13_OFS, top_reg13.reg);
	TOP_SETREG(info, TOP_REG14_OFS, top_reg14.reg);
	TOP_SETREG(info, TOP_REG15_OFS, top_reg15.reg);
	TOP_SETREG(info, TOP_REG16_OFS, top_reg16.reg);
	TOP_SETREG(info, TOP_REG17_OFS, top_reg17.reg);
	TOP_SETREG(info, TOP_REG18_OFS, top_reg18.reg);
	TOP_SETREG(info, TOP_REG19_OFS, top_reg19.reg);
	TOP_SETREG(info, TOP_REGCGPIO0_OFS, top_reg_cgpio0.reg);
	TOP_SETREG(info, TOP_REGPGPIO0_OFS, top_reg_pgpio0.reg);
	TOP_SETREG(info, TOP_REGPGPIO1_OFS, top_reg_pgpio1.reg);
	TOP_SETREG(info, TOP_REGDGPIO0_OFS, top_reg_dgpio0.reg);
	TOP_SETREG(info, TOP_REGLGPIO0_OFS, top_reg_lgpio0.reg);
	TOP_SETREG(info, TOP_REGSGPIO0_OFS, top_reg_sgpio0.reg);
	TOP_SETREG(info, TOP_REGHSIGPIO0_OFS, top_reg_hsigpio0.reg);
	TOP_SETREG(info, TOP_REGDSIGPIO0_OFS, top_reg_dsigpio0.reg);
	TOP_SETREG(info, TOP_REGAGPIO0_OFS, top_reg_agpio0.reg);
	/* Leave critical section */
	//unl_cpu(flags);

	return E_OK;
}

/*
void gpio_func_show(void)
{
   int i=0;

   int gpio_count=0;
   int func_num;
   //print C_GPIO
   for(i=CGPIO_0;i<C_GPIO_all;i++)
   {
        func_num=dump_gpio_func[i];
        if(func_num)
        {

            if(gpio_count<10)
            {
                printk("\033[0;32mC_GPIO%d---------------------%s\033[0m\n", gpio_count,dump_func[func_num-1]);
            }
            else
            {
                printk("\033[0;32mC_GPIO%d--------------------%s\033[0m\n", gpio_count,dump_func[func_num-1]);
            }
        }
        else
        {
            if(gpio_count<10)
            {
                printk("C_GPIO%d---------------------GPIO\n", gpio_count);
            }
            else
            {
                printk("C_GPIO%d--------------------GPIO\n", gpio_count);
            }
         }
        gpio_count++;
   }
   //HSI_GPIO
   gpio_count=0;
   for(i=HSIGPIO_0;i<HSI_GPIO_all;i++)
   {
        func_num=dump_gpio_func[i];
        if(func_num)
        {

            if(gpio_count<10)
            {
                printk("\033[0;32mHSI_GPIO%d-------------------%s\033[0m\n", gpio_count,dump_func[func_num-1]);
            }
            else
            {
                printk("\033[0;32mHSI_GPIO%d------------------%s\033[0m\n", gpio_count,dump_func[func_num-1]);
            }

        }
        else
        {
            if(gpio_count<10)
            {
                printk("HSI_GPIO%d-------------------GPIO\n", gpio_count);
            }
            else
            {
                printk("HSI_GPIO%d------------------GPIO\n", gpio_count);
            }
         }
        gpio_count++;
   }
   //S_GPIO
   gpio_count=0;
   for(i=SGPIO_0;i<S_GPIO_all;i++)
   {
        func_num=dump_gpio_func[i];
        if(func_num)
        {

            if(gpio_count<10)
            {
                printk("\033[0;32mS_GPIO%d---------------------%s\033[0m\n", gpio_count,dump_func[func_num-1]);
            }
            else
            {
                printk("\033[0;32mS_GPIO%d--------------------%s\033[0m\n", gpio_count,dump_func[func_num-1]);
            }
        }
        else
        {
           if(gpio_count<10)
            {
                printk("S_GPIO%d---------------------GPIO\n", gpio_count);
            }
            else
            {
                printk("S_GPIO%d--------------------GPIO\n", gpio_count);
            }
         }
        gpio_count++;
   }
   //L_GPIO
   gpio_count=0;
   for(i=LGPIO_0;i<L_GPIO_all;i++)
   {
        func_num=dump_gpio_func[i];
        if(func_num)
        {

            if(gpio_count<10)
            {
                printk("\033[0;32mL_GPIO%d---------------------%s\033[0m\n", gpio_count,dump_func[func_num-1]);
            }
            else
            {
                printk("\033[0;32mL_GPIO%d--------------------%s\033[0m\n", gpio_count,dump_func[func_num-1]);
            }
        }
        else
        {
            if(gpio_count<10)
            {
                printk("L_GPIO%d---------------------GPIO\n", gpio_count);
            }
            else
            {
                printk("L_GPIO%d--------------------GPIO\n", gpio_count);
            }
         }
        gpio_count++;
   }
   //DSI_GPIO
   gpio_count=0;
   for(i=DSIGPIO_0;i<DSI_GPIO_all;i++)
   {
        func_num=dump_gpio_func[i];
        if(func_num)
        {

            if(gpio_count<10)
            {
                printk("\033[0;32mDSI_GPIO%d-------------------%s\033[0m\n", gpio_count,dump_func[func_num-1]);
            }
            else
            {
                printk("\033[0;32mDSI_GPIO%d------------------%s\033[0m\n", gpio_count,dump_func[func_num-1]);
            }
        }
        else
        {
            if(gpio_count<10)
            {
                printk("DSI_GPIO%d-------------------GPIO\n", gpio_count);
            }
            else
            {
                printk("DSI_GPIO%d------------------GPIO\n", gpio_count);
            }
         }
        gpio_count++;
   }
   //A_GPIO
   gpio_count=0;
   for(i=AGPIO_0;i<A_GPIO_all;i++)
   {
        func_num=dump_gpio_func[i];
        if(func_num)
        {

           if(gpio_count<10)
            {
                printk("\033[0;32mA_GPIO%d---------------------%s\033[0m\n", gpio_count,dump_func[func_num-1]);
            }
            else
            {
                printk("\033[0;32mA_GPIO%d--------------------%s\033[0m\n", gpio_count,dump_func[func_num-1]);
            }
        }
        else
        {
            if(gpio_count<10)
            {
                printk("A_GPIO%d---------------------GPIO\n", gpio_count);
            }
            else
            {
                printk("A_GPIO%d--------------------GPIO\n", gpio_count);
            }
         }
        gpio_count++;
   }
   //P_GPIO
   gpio_count=0;
   for(i=PGPIO_0;i<P_GPIO_all;i++)
   {
        func_num=dump_gpio_func[i];
        if(func_num)
        {

            if(gpio_count<10)
            {
                printk("\033[0;32mP_GPIO%d---------------------%s\033[0m\n", gpio_count,dump_func[func_num-1]);
            }
            else
            {
                printk("\033[0;32mP_GPIO%d--------------------%s\033[0m\n", gpio_count,dump_func[func_num-1]);
            }
        }
        else
        {
            if(gpio_count<10)
            {
                printk("P_GPIO%d---------------------GPIO\n", gpio_count);
            }
            else
            {
                printk("P_GPIO%d--------------------GPIO\n", gpio_count);
            }
         }
        gpio_count++;
   }
   //D_GPIO
   gpio_count=0;
   for(i=DGPIO_0;i<D_GPIO_all;i++)
   {
        func_num=dump_gpio_func[i];
        if(func_num)
        {

            if(gpio_count<10)
            {
                printk("\033[0;32mD_GPIO%d---------------------%s\033[0m\n", gpio_count,dump_func[func_num-1]);
            }
            else
            {
                printk("\033[0;32mD_GPIO%d--------------------%s\033[0m\n", gpio_count,dump_func[func_num-1]);
            }
        }
        else
        {
            if(gpio_count<10)
            {
                printk("D_GPIO%d---------------------GPIO\n", gpio_count);
            }
            else
            {
                printk("D_GPIO%d--------------------GPIO\n", gpio_count);
            }
         }
        gpio_count++;
   }
}
EXPORT_SYMBOL(gpio_func_show);
*/
