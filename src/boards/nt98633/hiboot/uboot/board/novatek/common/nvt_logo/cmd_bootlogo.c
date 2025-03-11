
#include <common.h>
#include <command.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <part.h>
#include <asm/hardware.h>
#include <asm/nvt-common/nvt_types.h>
#include <asm/nvt-common/nvt_common.h>
#include <asm/nvt-common/shm_info.h>
#include <stdlib.h>
#include <linux/arm-smccc.h>
#include "cmd_bootlogo.h"
#include <asm/arch/display.h>
#include <asm/arch/top.h>
#include <linux/libfdt.h>

#include "logo.dat"   //jpg bitstream binary
#include "logo2.dat"

extern void jpeg_setfmt(unsigned int fmt);
extern void jpeg_decode(unsigned char *inbuf, unsigned char *outbuf);
extern void jpeg_getdim(unsigned int *width, unsigned int *height);

UINT32 dma_getDramBaseAddr(DMA_ID id);
UINT32 dma_getDramCapacity(DMA_ID id);

#define HEAVY_LOAD_CTRL_OFS(ch)         (DMA_CHANNEL0_HEAVY_LOAD_CTRL_OFS + ((ch) * 0x10))
#define HEAVY_LOAD_ADDR_OFS(ch)         (DMA_CHANNEL0_HEAVY_LOAD_START_ADDR_OFS + ((ch) * 0x10))
#define HEAVY_LOAD_SIZE_OFS(ch)         (DMA_CHANNEL0_HEAVY_LOAD_DMA_SIZE_OFS + ((ch) * 0x10))
#define HEAVY_LOAD_WAIT_CYCLE_OFS(ch)   (DMA_CHANNEL0_HEAVY_LOAD_WAIT_CYCLE_OFS + ((ch) * 0x10))

#define PROTECT_START_ADDR_OFS(ch)      (DMA_PROTECT_STARTADDR0_REG0_OFS+(ch)*8)
#define PROTECT_END_ADDR_OFS(ch)        (DMA_PROTECT_STOPADDR0_REG0_OFS+(ch)*8)
#define PROTECT_CH_MSK0_OFS(ch)         (DMA_PROTECT_RANGE0_MSK0_REG_OFS+(ch)*32)
#define PROTECT_CH_MSK1_OFS(ch)         (DMA_PROTECT_RANGE0_MSK1_REG_OFS+(ch)*32)
#define PROTECT_CH_MSK2_OFS(ch)         (DMA_PROTECT_RANGE0_MSK2_REG_OFS+(ch)*32)
#define PROTECT_CH_MSK3_OFS(ch)         (DMA_PROTECT_RANGE0_MSK3_REG_OFS+(ch)*32)
#define PROTECT_CH_MSK4_OFS(ch)         (DMA_PROTECT_RANGE0_MSK4_REG_OFS+(ch)*32)
#define PROTECT_CH_MSK5_OFS(ch)         (DMA_PROTECT_RANGE0_MSK5_REG_OFS+(ch)*32)

static UINT32 chip_id = 0x0;

#define INREG32(x)          			(*((volatile UINT32*)(x)))
#define OUTREG32(x, y)      			(*((volatile UINT32*)(x)) = (y))    ///< Write 32bits IO register
#define SETREG32(x, y)      			OUTREG32((x), INREG32(x) | (y))     ///< Set 32bits IO register
#define CLRREG32(x, y)      			OUTREG32((x), INREG32(x) & ~(y))    ///< Clear 32bits IO register

#define LOGO_DBG_MSG 0
#if LOGO_DBG_MSG
#define _Y_LOG(fmt, args...)         	printf(DBG_COLOR_YELLOW fmt DBG_COLOR_END, ##args)
#define _R_LOG(fmt, args...)         	printf(DBG_COLOR_RED fmt DBG_COLOR_END, ##args)
#define _M_LOG(fmt, args...)         	printf(DBG_COLOR_MAGENTA fmt DBG_COLOR_END, ##args)
#define _G_LOG(fmt, args...)         	printf(DBG_COLOR_GREEN fmt DBG_COLOR_END, ##args)
#define _W_LOG(fmt, args...)         	printf(DBG_COLOR_WHITE fmt DBG_COLOR_END, ##args)
#define _X_LOG(fmt, args...)         	printf(DBG_COLOR_HI_GRAY fmt DBG_COLOR_END, ##args)
#else
#define _Y_LOG(fmt, args...)
#define _W_LOG(fmt, args...)
#endif
#define GPIO_LCD_SIF_SEN            	L_GPIO_22//SIF CH1
#define GPIO_LCD_SIF_SCK            	L_GPIO_23
#define GPIO_LCD_SIF_SDA            	L_GPIO_24
extern uint8_t *nvt_fdt_buffer;

#if CONFIG_TARGET_NA51055
#define PLL_CLKSEL_IDE_CLKSRC_480   (0x00)    //< Select IDE clock source as 480 MHz
#define PLL_CLKSEL_IDE_CLKSRC_PLL6  (0x01)    //< Select IDE clock source as PLL6 (for IDE/ETH)
#define PLL_CLKSEL_IDE_CLKSRC_PLL4  (0x02)    //< Select IDE clock source as PLL4 (for SSPLL)
#define PLL_CLKSEL_IDE_CLKSRC_PLL9  (0x03)    //< Select IDE clock source as PLL9 (for IDE/ETH backup)

#define PLL4_OFFSET 0xF0021318
#define PLL6_OFFSET 0xF0021288
#define PLL9_OFFSET 0xF002134c

#define PLL4_OFFSET_528 0xF00244E0
#define PLL6_OFFSET_528 0xF0024520
#define PLL9_OFFSET_528 0xF0024560
#define DSI_RSTN_OFFSET 0xF0020088

#elif CONFIG_TARGET_NA51089
#define PLL_CLKSEL_IDE_CLKSRC_480   (0x00)    //< Select IDE clock source as 480 MHz
#define PLL_CLKSEL_IDE_CLKSRC_PLL6  (0x01)    //< Select IDE clock source as PLL6 (for IDE/ETH)
#define PLL_CLKSEL_IDE_CLKSRC_PLL9  (0x03)    //< Select IDE clock source as PLL9 (for IDE/ETH backup)

#define PLL6_OFFSET 0xF00244E0
#define PLL9_OFFSET 0xF0024520
#define DSI_RSTN_OFFSET 0xF0020088

#elif defined(CONFIG_TARGET_NA51102_A64)
#define PLL_CLKSEL_IDE_CLKSRC_480   (0x00)    //< Select IDE clock source as 480 MHz
#define PLL_CLKSEL_IDE_CLKSRC_PLL9  (0x01)    //< Select IDE clock source as PLL9 (for IDE)
#define PLL_CLKSEL_IDE_CLKSRC_PLL10 (0x03)    //< Select IDE clock source as PLL10 (for IDE backup)

#define PLL9_OFFSET 0x2F0020660
#define PLL10_OFFSET 0x2F00206A0
#define DSI_RSTN_OFFSET 0x2F0020088

#endif


/**
    Set module clock rate

    Set module clock rate, one module at a time.

    @param[in] id      Module ID(PLL_CLKSEL_*), one module at a time.
                          Please refer to pll.h
    @param[in] value    Moudle clock rate(PLL_CLKSEL_*_*), please refer to pll.h

    @return void
*/
void pll_set_ideclock_rate(int id, u32 value)
{
	REGVALUE reg_data;
#if CONFIG_TARGET_NA51102_A64
	UINT64 ui_reg_offset;
#else
	UINT32 ui_reg_offset;
#endif
	if (id == PLL_CLKSEL_IDE_CLKSRC_480) {
		return;
#if CONFIG_TARGET_NA51055 || CONFIG_TARGET_NA51089
	} else if (id == PLL_CLKSEL_IDE_CLKSRC_PLL6) {
#if CONFIG_TARGET_NA51055
		if(nvt_get_chip_id() == CHIP_NA51084){
			ui_reg_offset = PLL6_OFFSET_528;
		}else
#endif
		{
			ui_reg_offset = PLL6_OFFSET;
		}
#endif
#if CONFIG_TARGET_NA51055
	} else if (id == PLL_CLKSEL_IDE_CLKSRC_PLL4) {
		if(nvt_get_chip_id() == CHIP_NA51084){
			ui_reg_offset = PLL4_OFFSET_528;
		}else{
			ui_reg_offset = PLL4_OFFSET;
		}
#endif
	} else if (id == PLL_CLKSEL_IDE_CLKSRC_PLL9) {
#if CONFIG_TARGET_NA51055
		if(nvt_get_chip_id() == CHIP_NA51084){
			ui_reg_offset = PLL9_OFFSET_528;
		}else
#endif
		{
			ui_reg_offset = PLL9_OFFSET;
		}
#if CONFIG_TARGET_NA51102 || CONFIG_TARGET_NA51102_A64
	}  else if (id == PLL_CLKSEL_IDE_CLKSRC_PLL10) {
		ui_reg_offset = PLL10_OFFSET;
#endif
	}else {
		printf("no support soruce 0x%x.\r\n", id);
	}

	reg_data = ((value / 12000000) * 131072);

	OUTW(ui_reg_offset, reg_data & 0xFF);
	OUTW(ui_reg_offset + 0x4, (reg_data >> 8) & 0xFF);
	OUTW(ui_reg_offset + 0x8, (reg_data >> 16) & 0xFF);
}


static int nvt_getfdt_logo_addr_size(ulong addr, ulong *fdt_addr, ulong *fdt_len)
{
	int len;
	int nodeoffset; /* next node offset from libfdt */
	const u32 *nodep; /* property node pointer */

	*fdt_addr = 0;
	*fdt_len = 0;

	nodeoffset = fdt_path_offset((const void *)addr, "/logo");
	if (nodeoffset < 0) {
		return -1;
	} else {
		nodep = fdt_getprop((const void *)addr, nodeoffset, "enable", &len);
		if ((nodep > 0) && (be32_to_cpu(nodep[0]) == 1)) {
			nodep = fdt_getprop((const void *)addr, nodeoffset, "lcd_type", &len);
			if (nodep <= 0) {
				printf("no lcd_type\n");
			//	return -5;
			}
		} else {
			printf("no ebable\n");
			return -2;
		}
	}

	nodeoffset = fdt_path_offset((const void *)addr, "/nvt_memory_cfg/logo-fb");
	if (nodeoffset < 0) {
		printf("no logo-fb\n");
		return -3;
	}

	nodep = fdt_getprop((const void *)addr, nodeoffset, "reg", &len);
	if (len == 0) {
		printf("no reg\n");
		return -4;
	}

	*fdt_addr = be32_to_cpu(nodep[0]);
	*fdt_len = be32_to_cpu(nodep[1]);
	return 0;
}

#if CONFIG_TARGET_NA51102 || CONFIG_TARGET_NA51102_A64
static int nvt_getfdt_logo2_addr_size(ulong addr, ulong *fdt_addr, ulong *fdt_len)
{
	int len;
	int nodeoffset; /* next node offset from libfdt */
	const u32 *nodep; /* property node pointer */

	*fdt_addr = 0;
	*fdt_len = 0;

	nodeoffset = fdt_path_offset((const void *)addr, "/logo2");
	if (nodeoffset < 0) {
		return -1;
	} else {
		nodep = fdt_getprop((const void *)addr, nodeoffset, "enable", &len);
		if ((nodep > 0) && (be32_to_cpu(nodep[0]) == 1)) {
			nodep = fdt_getprop((const void *)addr, nodeoffset, "lcd_type", &len);
			if (nodep <= 0) {
				printf("no lcd_type\n");
				return -5;
			}
		} else {
			printf("no ebable\n");
			return -2;
		}
	}

	nodeoffset = fdt_path_offset((const void *)addr, "/nvt_memory_cfg/logo2-fb");
	if (nodeoffset < 0) {
		printf("no logo2-fb\n");
		return -3;
	}

	nodep = fdt_getprop((const void *)addr, nodeoffset, "reg", &len);
	if (len == 0) {
		printf("no reg\n");
		return -4;
	}

	*fdt_addr = be32_to_cpu(nodep[0]);
	*fdt_len = be32_to_cpu(nodep[1]);
	return 0;
}
#endif
 
static int nvt_getfdt_logo_gpio(ulong addr, ulong *gpio_cs, ulong *gpio_clk, ulong *gpio_data)
{
    int len;
    int nodeoffset; /* next node offset from libfdt */
    const u32 *nodep; /* property node pointer */
	
    *gpio_cs = 0;
    *gpio_clk = 0;
	*gpio_data = 0;

    nodeoffset = fdt_path_offset((const void *)addr, "/display");
    if (nodeoffset < 0) {
        printf("no display node, please check nvt-display.dtsi\r\n");
        return -1;
    } else {
        nodep = fdt_getprop((const void *)addr, nodeoffset, "lcd_ctrl", &len);
        if ((nodep > 0) && (be32_to_cpu(nodep[0]) == 1)) {
            nodep = fdt_getprop((const void *)addr, nodeoffset, "gpio_cs", &len);
            if (nodep <= 0) {
                printf("wrong gpio_cs value\n");
                return -1;
            }
			else
				*gpio_cs = be32_to_cpu(nodep[0]);
		
			nodep = fdt_getprop((const void *)addr, nodeoffset, "gpio_clk", &len);
            if (nodep <= 0) {
                printf("wrong gpio_clk value\n");
                return -1;
            }
            else
                *gpio_clk = be32_to_cpu(nodep[0]);

			nodep = fdt_getprop((const void *)addr, nodeoffset, "gpio_data", &len);
            if (nodep <= 0) {
                printf("wrong gpio_data value\n");
                return -1;
            }
            else
                *gpio_data = be32_to_cpu(nodep[0]);
        } else {
			printf("uboot logo support gpio only\r\n");
			return -1;
        }
    }
	return 0;
}

#if CONFIG_TARGET_NA51102 || CONFIG_TARGET_NA51102_A64
static int nvt_getfdt_logo2_gpio(ulong addr, ulong *gpio_cs, ulong *gpio_clk, ulong *gpio_data)
{
    int len;
    int nodeoffset; /* next node offset from libfdt */
    const u32 *nodep; /* property node pointer */
	
    *gpio_cs = 0;
    *gpio_clk = 0;
	*gpio_data = 0;

    nodeoffset = fdt_path_offset((const void *)addr, "/display2");
    if (nodeoffset < 0) {
        printf("no display node, please check nvt-display.dtsi\r\n");
        return -1;
    } else {
        nodep = fdt_getprop((const void *)addr, nodeoffset, "lcd_ctrl", &len);
        if ((nodep > 0) && (be32_to_cpu(nodep[0]) == 1)) {
            nodep = fdt_getprop((const void *)addr, nodeoffset, "gpio_cs", &len);
            if (nodep <= 0) {
                printf("wrong gpio_cs value\n");
                return -1;
            }
			else
				*gpio_cs = be32_to_cpu(nodep[0]);
		
			nodep = fdt_getprop((const void *)addr, nodeoffset, "gpio_clk", &len);
            if (nodep <= 0) {
                printf("wrong gpio_clk value\n");
                return -1;
            }
            else
                *gpio_clk = be32_to_cpu(nodep[0]);

			nodep = fdt_getprop((const void *)addr, nodeoffset, "gpio_data", &len);
            if (nodep <= 0) {
                printf("wrong gpio_data value\n");
                return -1;
            }
            else
                *gpio_data = be32_to_cpu(nodep[0]);
        } else {
			printf("uboot logo support gpio only\r\n");
			return -1;
        }
    }
	return 0;
}
#endif 

static int do_bootlogo(cmd_tbl_t *cmdtp, int flag, int argc, char *const argv[])
{
	PDISP_OBJ       p_emu_disp_obj;
	DISPCTRL_PARAM  emu_disp_ctrl;
	DISPDEV_PARAM   emu_disp_dev;
	DISPLAYER_PARAM emu_disp_lyr;
#if CONFIG_TARGET_NA51102 || CONFIG_TARGET_NA51102_A64
	PDISP_OBJ       p_emu_disp_obj2;
	DISPCTRL_PARAM  emu_disp_ctrl2;
	DISPDEV_PARAM   emu_disp_dev2;
	DISPLAYER_PARAM emu_disp_lyr2;
#endif
	ulong logo_addr, logo_size;
	ulong logo_addr2, logo_size2;
	ulong gpio_cs,gpio_clk,gpio_data;
	unsigned int img_width, img_height;
	unsigned int img_width2, img_height2;
	int ret = 0;
	int nodeoffset; /* next node offset from libfdt */
	const u32 *nodep; /* property node pointer */
	int len;
	int clock_source = 1, clock_source2=1;
	int clock_freq = 300000000, clock_freq2= 150000000;
	REGVALUE reg_data;
	UINT32 ui_reg_offset;
	int tohdmi = 0;  //1:ide1, 2:ide2
	int panel = 1;
	int panel2 = 0;
	int layer = 0; //0:vdo0,1:osd
#if ( CONFIG_DISP_VIDEO_HDMITX_IDE)
	DISP_HDMI_CFG	 config;
	PDISPDEV_OBJ     phdmi_disp_obj;
#endif


#if CONFIG_TARGET_NA51102 || CONFIG_TARGET_NA51102_A64	
    if(argc >= 2)
	{
		char *p = argv[1];
		char *p2 = argv[2];
		char *p3 = argv[3];
		if (!strcmp("hdmi", p))
		{
#if ( CONFIG_DISP_VIDEO_HDMITX_IDE)
			tohdmi = 1;
#else
			tohdmi = 0;
#endif
			if(!strcmp("osd", p2))
                layer = 1;
		}else if (!strcmp("0", p)){
#if ( CONFIG_DISP_VIDEO_HDMITX_IDE)
			if(!strcmp("hdmi", p2))	
				tohdmi = 1;
			if(!strcmp("osd", p3))
				layer = 1;
#else 
			if(!strcmp("osd", p2))
				layer = 1;
#endif
		}else if (!strcmp("1", p)){
			panel = 0;
			panel2 = 1;
#if ( CONFIG_DISP_VIDEO_HDMITX_IDE)
			if(!strcmp("hdmi", p2)) 
				tohdmi = 2;
			if(!strcmp("osd", p3))
                layer = 1;
#else
			if(!strcmp("osd", p2))
				layer = 1;
#endif
		}else if (!strcmp("2", p)){
			panel = 1;
			panel2 = 1;
#if ( CONFIG_DISP_VIDEO_HDMITX_IDE)
			if(!strcmp("hdmi", p2)) 
				tohdmi = 1;
			if(!strcmp("osd", p3))
                layer = 1;
#else
			if(!strcmp("osd", p2))
                layer = 1;
#endif

		} 
		else 
		{
			printf("Usage: bootlogo {index}{hdmi}, index=> 0:LCD1, 1:LCD2, 2:both\n");
			return -1;
		}
	}
#endif
	_W_LOG("do_bootlogo\r\n");
	nvt_pinmux_probe();

	reg_data = INREG32(DSI_RSTN_OFFSET);
	reg_data &= 0xFFFFFFF7;
	OUTW(DSI_RSTN_OFFSET, reg_data);
	udelay(1000);
	reg_data |= 0x8;
	OUTW(DSI_RSTN_OFFSET, reg_data);

	if(panel){
		p_emu_disp_obj = disp_get_display_object(DISP_1);
		p_emu_disp_obj->open();

#if CONFIG_TARGET_NA51102 || CONFIG_TARGET_NA51102_A64
		nodeoffset = fdt_path_offset((const void *)nvt_fdt_buffer, "/nvt-clk/clk_ide_src");
		if (nodeoffset < 0) {
			printf("no find node clk_ide_src\n");
		} else {
			nodep = fdt_getprop((const void *)nvt_fdt_buffer, nodeoffset, "parent_idx", &len);
			if (len > 0) {
				clock_source = be32_to_cpu(nodep[0]);
			} else {
				printf("no ide clock_source info, used default PLL\n");
			}

			nodep = fdt_getprop((const void *)nvt_fdt_buffer, nodeoffset, "current_rate", &len);
			if (len > 0) {
				clock_freq	= be32_to_cpu(nodep[0]);
			} else {
				printf("no ide source-frequency info, used default \n");
			}

			//printf("lcd clk src:%d freq:%d : 0x%x : %d\n", clock_source, clock_freq, *nodep, len);
		}

#else
		nodeoffset = fdt_path_offset((const void *)nvt_fdt_buffer, "/ide@f0800000");
		if (nodeoffset < 0) {
			printf("no find node ide@f0800000\n");
		} else {
			nodep = fdt_getprop((const void *)nvt_fdt_buffer, nodeoffset, "clock-source", &len);
			if (len > 0) {
				clock_source = be32_to_cpu(nodep[0]);
			} else {
				printf("no ide clock_source info, used default PLL\n");
			}

			nodep = fdt_getprop((const void *)nvt_fdt_buffer, nodeoffset, "source-frequency", &len);
			if (len > 0) {
				clock_freq  = be32_to_cpu(nodep[0]);
			} else {
				printf("no ide source-frequency info, used default \n");
			}

			//printf("lcd clk src:%d freq:%d : 0x%x : %d\n", clock_source, clock_freq, *nodep, len);
		}
#endif
#if ( CONFIG_DISP_VIDEO_HDMITX_IDE)
		if(tohdmi == 1){
			emu_disp_dev.SEL.HOOK_DEVICE_OBJECT.dev_id         = DISPDEV_ID_TVHDMI;
			emu_disp_dev.SEL.HOOK_DEVICE_OBJECT.p_disp_dev_obj   = dispdev_get_hdmi_dev_obj();
			p_emu_disp_obj->dev_ctrl(DISPDEV_HOOK_DEVICE_OBJECT, &emu_disp_dev);
			printf("hook hdmi dev done \n");
		} else 
#endif
		{
			emu_disp_dev.SEL.HOOK_DEVICE_OBJECT.dev_id         = DISPDEV_ID_PANEL;
			emu_disp_dev.SEL.HOOK_DEVICE_OBJECT.p_disp_dev_obj   = dispdev_get_lcd1_dev_obj();
			p_emu_disp_obj->dev_ctrl(DISPDEV_HOOK_DEVICE_OBJECT, &emu_disp_dev);
			printf("hook panel dev done \n");
		}
		
		pll_set_ideclock_rate(clock_source, clock_freq);

		emu_disp_ctrl.SEL.SET_SRCCLK.src_clk = (DISPCTRL_SRCCLK)clock_source;//DISPCTRL_SRCCLK_PLL6;
		p_emu_disp_obj->disp_ctrl(DISPCTRL_SET_SRCCLK, &emu_disp_ctrl);

		////////////////////////////DISP-1//////////////////////////////////////////////////
		emu_disp_dev.SEL.SET_REG_IF.lcd_ctrl     = DISPDEV_LCDCTRL_GPIO;
		emu_disp_dev.SEL.SET_REG_IF.ui_sif_ch     = SIF_CH2;
	if(nvt_getfdt_logo_gpio((ulong)nvt_fdt_buffer,&gpio_cs,&gpio_clk,&gpio_data) == 0)
	{
		printf("gpio_cs = %d, gpio_clk=%d, gpio_data=%d\r\n",gpio_cs,gpio_clk,gpio_data);
		emu_disp_dev.SEL.SET_REG_IF.ui_gpio_sen   = gpio_cs;
	    emu_disp_dev.SEL.SET_REG_IF.ui_gpio_clk   = gpio_clk;
    	emu_disp_dev.SEL.SET_REG_IF.ui_gpio_data  = gpio_data;
	}
	else
	{
#if CONFIG_TARGET_NA51055
		emu_disp_dev.SEL.SET_REG_IF.ui_gpio_sen   = L_GPIO(22);
		emu_disp_dev.SEL.SET_REG_IF.ui_gpio_clk   = L_GPIO(23);
		emu_disp_dev.SEL.SET_REG_IF.ui_gpio_data  = L_GPIO(24);
#elif CONFIG_TARGET_NA51089
		emu_disp_dev.SEL.SET_REG_IF.ui_gpio_sen   = S_GPIO(6);
		emu_disp_dev.SEL.SET_REG_IF.ui_gpio_clk   = S_GPIO(7);
		emu_disp_dev.SEL.SET_REG_IF.ui_gpio_data  = S_GPIO(8);
#elif CONFIG_TARGET_NA51102_A64
		emu_disp_dev.SEL.SET_REG_IF.ui_gpio_sen   = L_GPIO(22);
		emu_disp_dev.SEL.SET_REG_IF.ui_gpio_clk   = L_GPIO(23);
		emu_disp_dev.SEL.SET_REG_IF.ui_gpio_data  = L_GPIO(24);

#endif
	}
		p_emu_disp_obj->dev_ctrl(DISPDEV_SET_REG_IF, &emu_disp_dev);
		p_emu_disp_obj->dev_ctrl(DISPDEV_CLOSE_DEVICE, NULL);

#if ( CONFIG_DISP_VIDEO_HDMITX_IDE)
		if(tohdmi == 1){
			emu_disp_dev.SEL.GET_PREDISPSIZE.dev_id = DISPDEV_ID_TVHDMI;
		} 
		else 
#endif
		{
			emu_disp_dev.SEL.GET_PREDISPSIZE.dev_id = DISPDEV_ID_PANEL;
		}
		p_emu_disp_obj->dev_ctrl(DISPDEV_GET_PREDISPSIZE, &emu_disp_dev);
		_Y_LOG("Pre Get Size =%d, %d\r\n", (int)(emu_disp_dev.SEL.GET_PREDISPSIZE.ui_buf_width), (int)(emu_disp_dev.SEL.GET_PREDISPSIZE.ui_buf_height));

		p_emu_disp_obj->dev_ctrl(DISPDEV_GET_LCDMODE, &emu_disp_dev);
		_Y_LOG("LCD mode =%d\r\n", (int)(emu_disp_dev.SEL.GET_LCDMODE.mode));
#if ( CONFIG_DISP_VIDEO_HDMITX_IDE)
		if(tohdmi == 1){
			emu_disp_dev.SEL.OPEN_DEVICE.dev_id = DISPDEV_ID_TVHDMI;
			config.audio_fmt = HDMI_AUDIO48KHZ;
			config.output_mode = 0;//2; //HDMI_MODE_RGB444 = 0, //	 HDMI_MODE_YCBCR422,  //	HDMI_MODE_YCBCR444,  //
			config.vid	= HDMI_1920X1080P30;//HDMI_720X480P60;
			config.source = HDMI_IDE_0;

			phdmi_disp_obj= dispdev_get_hdmi_dev_obj();
			phdmi_disp_obj->presetconfig((PDISP_HDMI_CFG)&config);
			emu_disp_dev.SEL.SET_HDMIMODE.video_id = HDMI_1920X1080P30;
			emu_disp_dev.SEL.SET_HDMIMODE.audio_id = HDMI_AUDIO48KHZ;
			p_emu_disp_obj->dev_ctrl(DISPDEV_SET_HDMIMODE, &emu_disp_dev);
			emu_disp_dev.SEL.OPEN_DEVICE.dev_id = DISPDEV_ID_TVHDMI;	
		} 
		else 
#endif
		{
			emu_disp_dev.SEL.OPEN_DEVICE.dev_id = DISPDEV_ID_PANEL;
		}	
		p_emu_disp_obj->dev_ctrl(DISPDEV_OPEN_DEVICE, &emu_disp_dev);

		p_emu_disp_obj->dev_ctrl(DISPDEV_GET_DISPSIZE, &emu_disp_dev);

		emu_disp_lyr.SEL.SET_WINSIZE.ui_win_width     = emu_disp_dev.SEL.GET_DISPSIZE.ui_buf_width;
		emu_disp_lyr.SEL.SET_WINSIZE.ui_win_height   	= emu_disp_dev.SEL.GET_DISPSIZE.ui_buf_height;
		emu_disp_lyr.SEL.SET_WINSIZE.i_win_ofs_x      = 0;
		emu_disp_lyr.SEL.SET_WINSIZE.i_win_ofs_y      = 0;
		if(layer == 0)
			p_emu_disp_obj->disp_lyr_ctrl(DISPLAYER_VDO1, DISPLAYER_OP_SET_WINSIZE, &emu_disp_lyr);
		else
			p_emu_disp_obj->disp_lyr_ctrl(DISPLAYER_OSD1, DISPLAYER_OP_SET_WINSIZE, &emu_disp_lyr);
		


		_W_LOG("DISPLAYER_OP_SET_WINSIZE %d %d\r\n", emu_disp_lyr.SEL.SET_WINSIZE.ui_win_width, emu_disp_lyr.SEL.SET_WINSIZE.ui_win_height);

		ret = nvt_getfdt_logo_addr_size((ulong)nvt_fdt_buffer, &logo_addr, &logo_size);
		if (ret != 0) {
			printf("err:%d\r\n", ret);
			return -1;
		}
		_Y_LOG("logo_addr %x logo_size %x\r\n", logo_addr, logo_size);

		_Y_LOG("start JPEG decode size: %x\n", sizeof(inbuf));
		if(layer == 0)
			jpeg_setfmt(1); // YUV
		else
			jpeg_setfmt(3); //ARGB4444
		jpeg_decode(inbuf, (unsigned char *)logo_addr);
		jpeg_getdim(&img_width, &img_height);
		_Y_LOG("image size: %d x %d\n", img_width, img_height);
		if (logo_size < img_width * img_height * 2) {
			printf("(%d,%d) size small 0x%x\r\n", img_width, img_height, logo_size);
			return -1;
		}
		flush_dcache_range((unsigned long)logo_addr, (unsigned long)(logo_addr + logo_size));


		{
			UINT32 buf_w = img_width;
			UINT32 buf_h = img_height;
			UINT32 uiVDO_YAddr = logo_addr;
			UINT32 VDO_BUF_SIZE = buf_w * buf_h;
			UINT32	uiVDO_UVAddr = uiVDO_YAddr + VDO_BUF_SIZE;
			_W_LOG("show logo %x %x\r\n", uiVDO_YAddr, uiVDO_UVAddr);
			
			if(layer == 0)
				emu_disp_lyr.SEL.SET_MODE.buf_format   = DISPBUFFORMAT_YUV422PACK;
			else
				emu_disp_lyr.SEL.SET_MODE.buf_format   = DISPBUFFORMAT_ARGB4444;
			emu_disp_lyr.SEL.SET_MODE.buf_mode     = DISPBUFMODE_BUFFER_REPEAT;
			emu_disp_lyr.SEL.SET_MODE.buf_number   = DISPBUFNUM_3;
			if(layer == 0)
				p_emu_disp_obj->disp_lyr_ctrl(DISPLAYER_VDO1, DISPLAYER_OP_SET_MODE, &emu_disp_lyr);
			else
				p_emu_disp_obj->disp_lyr_ctrl(DISPLAYER_OSD1, DISPLAYER_OP_SET_MODE, &emu_disp_lyr);
			_W_LOG("DISPLAYER_OP_SET_MODE\r\n");

			emu_disp_lyr.SEL.SET_BUFSIZE.ui_buf_width   =  buf_w;
			emu_disp_lyr.SEL.SET_BUFSIZE.ui_buf_height  =  buf_h;
			if(layer == 0)
			{
				emu_disp_lyr.SEL.SET_BUFSIZE.ui_buf_line_ofs = buf_w;
				p_emu_disp_obj->disp_lyr_ctrl(DISPLAYER_VDO1, DISPLAYER_OP_SET_BUFSIZE, &emu_disp_lyr);
			}
			else
			{
				emu_disp_lyr.SEL.SET_BUFSIZE.ui_buf_line_ofs = buf_w*2;
			 	p_emu_disp_obj->disp_lyr_ctrl(DISPLAYER_OSD1, DISPLAYER_OP_SET_BUFSIZE, &emu_disp_lyr);
			}
			_W_LOG("DISPLAYER_OP_SET_BUFSIZE %d %d\r\n", emu_disp_lyr.SEL.SET_BUFSIZE.ui_buf_line_ofs, buf_h);

#if 0 //fill YUV color for test
			memset((void *)uiVDO_YAddr, 0x4C, VDO_BUF_SIZE / 2);
			memset((void *)uiVDO_YAddr + VDO_BUF_SIZE / 2, 0x67, VDO_BUF_SIZE / 2);
			memset((void *)uiVDO_UVAddr, 0x4C, VDO_BUF_SIZE / 2);
			memset((void *)uiVDO_UVAddr + VDO_BUF_SIZE / 2, 0xAD, VDO_BUF_SIZE / 2);
#endif
			memset((void *)&emu_disp_lyr, 0, sizeof(DISPLAYER_PARAM));
			if(layer == 0)
			{
            	emu_disp_lyr.SEL.SET_VDOBUFADDR.buf_sel = DISPBUFADR_0;
	            emu_disp_lyr.SEL.SET_VDOBUFADDR.ui_addr_y0 = uiVDO_YAddr;
	            emu_disp_lyr.SEL.SET_VDOBUFADDR.ui_addr_cb0 = uiVDO_UVAddr;
 	            emu_disp_lyr.SEL.SET_VDOBUFADDR.ui_addr_cr0 = uiVDO_UVAddr;
   		   		p_emu_disp_obj->disp_lyr_ctrl(DISPLAYER_VDO1, DISPLAYER_OP_SET_VDOBUFADDR, &emu_disp_lyr);
			}
			else
			{
				emu_disp_lyr.SEL.SET_OSDBUFADDR.buf_sel = DISPBUFADR_0;
				emu_disp_lyr.SEL.SET_OSDBUFADDR.ui_addr_buf0 = uiVDO_YAddr;
				emu_disp_lyr.SEL.SET_OSDBUFADDR.ui_addr_buf1 = uiVDO_UVAddr;
				emu_disp_lyr.SEL.SET_OSDBUFADDR.ui_addr_buf2 = uiVDO_UVAddr;
				p_emu_disp_obj->disp_lyr_ctrl(DISPLAYER_OSD1, DISPLAYER_OP_SET_OSDBUFADDR, &emu_disp_lyr);
			}

			_W_LOG("DISPLAYER_OP_SET_VDOBUFADDR\r\n");
			emu_disp_ctrl.SEL.SET_ALL_LYR_EN.b_en      = TRUE;
			if(layer == 0)
				emu_disp_ctrl.SEL.SET_ALL_LYR_EN.disp_lyr  = DISPLAYER_VDO1;
			else
				emu_disp_ctrl.SEL.SET_ALL_LYR_EN.disp_lyr  = DISPLAYER_OSD1;
			p_emu_disp_obj->disp_ctrl(DISPCTRL_SET_ALL_LYR_EN, &emu_disp_ctrl);
			p_emu_disp_obj->load(TRUE);
			p_emu_disp_obj->wait_frm_end(TRUE);

			_W_LOG("DISPCTRL_SET_ALL_LYR_EN\r\n");

		}
	}
#if CONFIG_TARGET_NA51102 || CONFIG_TARGET_NA51102_A64	
	if(panel2){
		p_emu_disp_obj2 = disp_get_display_object(DISP_2);
		p_emu_disp_obj2->open();
		nodeoffset = fdt_path_offset((const void *)nvt_fdt_buffer, "/nvt-clk/clk_ide2_src");
		if (nodeoffset < 0) {
			printf("no find node clk_ide2_src\n");
		} else {
			nodep = fdt_getprop((const void *)nvt_fdt_buffer, nodeoffset, "parent_idx", &len);
			if (len > 0) {
				clock_source2 = be32_to_cpu(nodep[0]);
			} else {
				printf("no ide2 clock_source info, used default PLL\n");
			}

			nodep = fdt_getprop((const void *)nvt_fdt_buffer, nodeoffset, "current_rate", &len);
			if (len > 0) {
				clock_freq2 = be32_to_cpu(nodep[0]);
			} else {
				printf("no ide2 source-frequency info, used default \n");
			}

			//printf("lcd2 clk src:%d freq:%d \n", clock_source2, clock_freq2);
		}
#if ( CONFIG_DISP_VIDEO_HDMITX_IDE)
		if(tohdmi == 2){
			emu_disp_dev2.SEL.HOOK_DEVICE_OBJECT.dev_id         = DISPDEV_ID_TVHDMI;
			emu_disp_dev2.SEL.HOOK_DEVICE_OBJECT.p_disp_dev_obj   = dispdev_get_hdmi_dev_obj();	
			p_emu_disp_obj2->dev_ctrl(DISPDEV_HOOK_DEVICE_OBJECT, &emu_disp_dev2);
			printf("hook hdmi dev done \n");
		} 
		else 
#endif
		{
			emu_disp_dev2.SEL.HOOK_DEVICE_OBJECT.dev_id         = DISPDEV_ID_PANEL;
			emu_disp_dev2.SEL.HOOK_DEVICE_OBJECT.p_disp_dev_obj   = dispdev_get_lcd2_dev_obj();
			p_emu_disp_obj2->dev_ctrl(DISPDEV_HOOK_DEVICE_OBJECT, &emu_disp_dev2);
			printf("hook panel2 dev done \n");
		}
		pll_set_ideclock_rate(clock_source2, clock_freq2);
		emu_disp_ctrl2.SEL.SET_SRCCLK.src_clk = (DISPCTRL_SRCCLK)clock_source2;
		p_emu_disp_obj2->disp_ctrl(DISPCTRL_SET_SRCCLK, &emu_disp_ctrl2);
		emu_disp_dev2.SEL.SET_REG_IF.lcd_ctrl	  = DISPDEV_LCDCTRL_GPIO;
		emu_disp_dev2.SEL.SET_REG_IF.ui_sif_ch	   = SIF_CH2;
		if(nvt_getfdt_logo2_gpio((ulong)nvt_fdt_buffer,&gpio_cs,&gpio_clk,&gpio_data) == 0)
		{
			printf("gpio_cs = %d, gpio_clk=%d, gpio_data=%d\r\n",gpio_cs,gpio_clk,gpio_data);
			emu_disp_dev2.SEL.SET_REG_IF.ui_gpio_sen   = gpio_cs;
		    emu_disp_dev2.SEL.SET_REG_IF.ui_gpio_clk   = gpio_clk;
    		emu_disp_dev2.SEL.SET_REG_IF.ui_gpio_data  = gpio_data;
		}
		else
		{
			emu_disp_dev2.SEL.SET_REG_IF.ui_gpio_sen   = L_GPIO(30);
			emu_disp_dev2.SEL.SET_REG_IF.ui_gpio_clk   = L_GPIO(28);
			emu_disp_dev2.SEL.SET_REG_IF.ui_gpio_data  = L_GPIO(29);
		}
		p_emu_disp_obj2->dev_ctrl(DISPDEV_SET_REG_IF, &emu_disp_dev2);
		p_emu_disp_obj2->dev_ctrl(DISPDEV_CLOSE_DEVICE, NULL);
		if(tohdmi == 2){
			emu_disp_dev2.SEL.GET_PREDISPSIZE.dev_id = DISPDEV_ID_TVHDMI;
		} else {
			emu_disp_dev2.SEL.GET_PREDISPSIZE.dev_id = DISPDEV_ID_PANEL;
		}
		p_emu_disp_obj2->dev_ctrl(DISPDEV_GET_PREDISPSIZE, &emu_disp_dev2);
		 _Y_LOG("Pre Get Size =%d, %d\r\n", (int)(emu_disp_dev2.SEL.GET_PREDISPSIZE.ui_buf_width), (int)(emu_disp_dev2.SEL.GET_PREDISPSIZE.ui_buf_height));
		p_emu_disp_obj2->dev_ctrl(DISPDEV_GET_LCDMODE, &emu_disp_dev2);
		_Y_LOG("LCD mode =%d\r\n", (int)(emu_disp_dev2.SEL.GET_LCDMODE.mode));
#if ( CONFIG_DISP_VIDEO_HDMITX_IDE)
		if(tohdmi == 2){
			emu_disp_dev2.SEL.OPEN_DEVICE.dev_id = DISPDEV_ID_TVHDMI;
			config.audio_fmt = HDMI_AUDIO48KHZ;
			config.output_mode = 0;//2; //HDMI_MODE_RGB444 = 0, //	 HDMI_MODE_YCBCR422,  //	HDMI_MODE_YCBCR444,  //
			config.vid	= HDMI_1920X1080P30;//HDMI_720X480P60;
			config.source = HDMI_IDE_1;
			phdmi_disp_obj= dispdev_get_hdmi_dev_obj();
			phdmi_disp_obj->presetconfig((PDISP_HDMI_CFG)&config);
			emu_disp_dev2.SEL.SET_HDMIMODE.video_id = HDMI_1920X1080P30;
			emu_disp_dev2.SEL.SET_HDMIMODE.audio_id = HDMI_AUDIO48KHZ;
			p_emu_disp_obj2->dev_ctrl(DISPDEV_SET_HDMIMODE, &emu_disp_dev2);
			emu_disp_dev2.SEL.OPEN_DEVICE.dev_id = DISPDEV_ID_TVHDMI;
		} else 
#endif
		{
			emu_disp_dev2.SEL.OPEN_DEVICE.dev_id = DISPDEV_ID_PANEL;
		}

		p_emu_disp_obj2->dev_ctrl(DISPDEV_OPEN_DEVICE, &emu_disp_dev2);

		p_emu_disp_obj2->dev_ctrl(DISPDEV_GET_DISPSIZE, &emu_disp_dev2);
		emu_disp_lyr2.SEL.SET_WINSIZE.ui_win_width	   = emu_disp_dev2.SEL.GET_DISPSIZE.ui_buf_width;;
		emu_disp_lyr2.SEL.SET_WINSIZE.ui_win_height 	= emu_disp_dev2.SEL.GET_DISPSIZE.ui_buf_height;
		emu_disp_lyr2.SEL.SET_WINSIZE.i_win_ofs_x	   = 0;
		emu_disp_lyr2.SEL.SET_WINSIZE.i_win_ofs_y	   = 0;
		if(layer == 0)
			p_emu_disp_obj2->disp_lyr_ctrl(DISPLAYER_VDO1, DISPLAYER_OP_SET_WINSIZE, &emu_disp_lyr2);
		else
			p_emu_disp_obj2->disp_lyr_ctrl(DISPLAYER_OSD1, DISPLAYER_OP_SET_WINSIZE, &emu_disp_lyr2);

		ret = nvt_getfdt_logo2_addr_size((ulong)nvt_fdt_buffer, &logo_addr2, &logo_size2);
		if (ret != 0) {
			printf("err:%d\r\n", ret);
			return -1;
		}
		_Y_LOG("logo2_addr %x logo2_size %x\r\n", logo_addr2, logo_size2);

		_Y_LOG("start JPEG decode size: %x\n", sizeof(inbuf2));
		if(layer == 0)
			jpeg_setfmt(1);
		else 
			jpeg_setfmt(3);
		jpeg_decode(inbuf2, (unsigned char *)logo_addr2);
		jpeg_getdim(&img_width2, &img_height2);
		_Y_LOG("image size: %d x %d\n", img_width2, img_height2);
		if (logo_size2 < img_width2 * img_height2 * 2) {
			printf("(%d,%d) size small 0x%x\r\n", img_width2, img_height2, logo_size2);
			return -1;
		}
		flush_dcache_range((unsigned long)logo_addr2, (unsigned long)(logo_addr2 + logo_size2));
		{
		UINT32 buf_w = img_width2;
		UINT32 buf_h = img_height2;
		UINT32 uiVDO_YAddr = logo_addr2;
		UINT32 VDO_BUF_SIZE = buf_w * buf_h;
		UINT32	uiVDO_UVAddr = uiVDO_YAddr + VDO_BUF_SIZE;
		_W_LOG("show logo %x %x\r\n", uiVDO_YAddr, uiVDO_UVAddr);
	
		if(layer == 0)
		{
			emu_disp_lyr2.SEL.SET_MODE.buf_format	= DISPBUFFORMAT_YUV422PACK;
			emu_disp_lyr2.SEL.SET_MODE.buf_mode    = DISPBUFMODE_BUFFER_REPEAT;
			emu_disp_lyr2.SEL.SET_MODE.buf_number	= DISPBUFNUM_3;
			p_emu_disp_obj2->disp_lyr_ctrl(DISPLAYER_VDO1, DISPLAYER_OP_SET_MODE, &emu_disp_lyr2);
		}
		else
		{
            emu_disp_lyr2.SEL.SET_MODE.buf_format   = DISPBUFFORMAT_ARGB4444;
            emu_disp_lyr2.SEL.SET_MODE.buf_mode    = DISPBUFMODE_BUFFER_REPEAT;
            emu_disp_lyr2.SEL.SET_MODE.buf_number   = DISPBUFNUM_3;
            p_emu_disp_obj2->disp_lyr_ctrl(DISPLAYER_OSD1, DISPLAYER_OP_SET_MODE, &emu_disp_lyr2);
		}
		_W_LOG("DISPLAYER_OP_SET_MODE\r\n");

		emu_disp_lyr2.SEL.SET_BUFSIZE.ui_buf_width	=  buf_w;
		emu_disp_lyr2.SEL.SET_BUFSIZE.ui_buf_height =  buf_h;
		if(layer == 0)
		{
			emu_disp_lyr2.SEL.SET_BUFSIZE.ui_buf_line_ofs = buf_w;
			p_emu_disp_obj2->disp_lyr_ctrl(DISPLAYER_VDO1, DISPLAYER_OP_SET_BUFSIZE, &emu_disp_lyr2);
		}
		else
		{
        	emu_disp_lyr2.SEL.SET_BUFSIZE.ui_buf_line_ofs = buf_w*2;
            p_emu_disp_obj2->disp_lyr_ctrl(DISPLAYER_OSD1, DISPLAYER_OP_SET_BUFSIZE, &emu_disp_lyr);
		}
		_W_LOG("DISPLAYER_OP_SET_BUFSIZE %d %d\r\n", buf_w, buf_h);

#if 0 //fill YUV color for test
		memset((void *)uiVDO_YAddr, 0x4C, VDO_BUF_SIZE / 2);
		memset((void *)uiVDO_YAddr + VDO_BUF_SIZE / 2, 0x67, VDO_BUF_SIZE / 2);
		memset((void *)uiVDO_UVAddr, 0x4C, VDO_BUF_SIZE / 2);
		memset((void *)uiVDO_UVAddr + VDO_BUF_SIZE / 2, 0xAD, VDO_BUF_SIZE / 2);
#endif
		
		memset((void *)&emu_disp_lyr2, 0, sizeof(DISPLAYER_PARAM));
		if(layer == 0)
		{
			emu_disp_lyr2.SEL.SET_VDOBUFADDR.buf_sel = DISPBUFADR_0;
			emu_disp_lyr2.SEL.SET_VDOBUFADDR.ui_addr_y0 = uiVDO_YAddr;
			emu_disp_lyr2.SEL.SET_VDOBUFADDR.ui_addr_cb0 = uiVDO_UVAddr;
			emu_disp_lyr2.SEL.SET_VDOBUFADDR.ui_addr_cr0 = uiVDO_UVAddr;
			p_emu_disp_obj2->disp_lyr_ctrl(DISPLAYER_VDO1, DISPLAYER_OP_SET_VDOBUFADDR, &emu_disp_lyr2);
		}
		else
		{
        	emu_disp_lyr2.SEL.SET_OSDBUFADDR.buf_sel = DISPBUFADR_0;
        	emu_disp_lyr2.SEL.SET_OSDBUFADDR.ui_addr_buf0 = uiVDO_YAddr;
      		emu_disp_lyr2.SEL.SET_OSDBUFADDR.ui_addr_buf1 = uiVDO_UVAddr;
       		emu_disp_lyr2.SEL.SET_OSDBUFADDR.ui_addr_buf2 = uiVDO_UVAddr;
           	p_emu_disp_obj2->disp_lyr_ctrl(DISPLAYER_OSD1, DISPLAYER_OP_SET_OSDBUFADDR, &emu_disp_lyr2);
		}
	
		_W_LOG("DISPLAYER_OP_SET_VDOBUFADDR\r\n");

		emu_disp_ctrl2.SEL.SET_ALL_LYR_EN.b_en	   = TRUE;
		if(layer == 0)
			emu_disp_ctrl2.SEL.SET_ALL_LYR_EN.disp_lyr	= DISPLAYER_VDO1;
		else
			emu_disp_ctrl2.SEL.SET_ALL_LYR_EN.disp_lyr = DISPLAYER_OSD1;
		p_emu_disp_obj2->disp_ctrl(DISPCTRL_SET_ALL_LYR_EN, &emu_disp_ctrl2);
		p_emu_disp_obj2->load(TRUE);
		p_emu_disp_obj2->wait_frm_end(TRUE);

		_W_LOG("DISPCTRL_SET_ALL_LYR_EN\r\n");
		}
	}
#endif 
	return 0;
}



U_BOOT_CMD(
	bootlogo,	3,	1,	do_bootlogo,
	"show lcd bootlogo",
	"no argument means LCD output only, \n"
);
