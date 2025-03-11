/*
    HDMI TX module driver

    HDMI TX module driver.

    @file       hdmitx.c
    @ingroup    mIDrvDisp_HDMITx
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2014.  All rights reserved.
*/
#include "hdmitx_int.h"
#include "hdmitx_reg.h"
#include "hdmi_port.h"

int eye_enhance = 0;


int clock_rate = 0;

static BOOL         b_hdmi_tx_open     = FALSE;
static UINT32       gui_hdmi_int_sts;

#ifdef __KERNEL__
#define loc_cpu(x)	if (x) {}
#define unl_cpu(x)	if (x) {}
#endif /* LINUX */

#if (DRV_SUPPORT_IST == ENABLE)

#else
DRV_CB              p_cec_cb_func      = NULL;
#endif

#if HDMITX_DEBUG_FORCE_DVI
// FORCE_RGB / FORCE_UNDERSCAN / VERIFY_VID
UINT32              hdmi_ctrl_option[HDMICFG_OPTION_ALL] = {TRUE,  TRUE, FALSE};
#else
// FORCE_RGB / FORCE_UNDERSCAN / VERIFY_VID
UINT32              hdmi_ctrl_option[HDMICFG_OPTION_ALL] = {TRUE, TRUE, FALSE};
#endif

BOOL				b_hdmi_edid_msg			= DISABLE;
UINT32              gui_trim_rsel			= HDMITX_RTRIM_DEFAULT;
UINT32              gui_manufacturer_id;
BOOL                b_hdmi_i2c_opened      = FALSE;
I2C_SESSION         hdmi_i2c_ses;
BOOL                hdmi_i2c_select       = TRUE;

HDMI_I2C_OBJ        hdmi_i2c_obj[2] = {
	{
#if 1
        i2c5_open,
		i2c5_close,
		i2c5_lock,
		i2c5_unlock,
		i2c5_set_config,
		i2c5_transmit,
		i2c5_receive,
#else
		hdmiddc_open,
		hdmiddc_close,
		hdmiddc_lock,
		hdmiddc_unlock,
		hdmiddc_set_config,
		hdmiddc_transmit,
		hdmiddc_receive,
#endif
	},

	{
		i2c5_open,
		i2c5_close,
		i2c5_lock,
		i2c5_unlock,
		i2c5_set_config,
		i2c5_transmit,
		i2c5_receive,
	}
};

extern UINT32       hdmi_sw_ddc_scl, hdmi_sw_ddc_sda;

ER hdmitx_write_phy(UINT32 ui_offset, UINT32 ui_value);
ER hdmitx_read_phy(UINT32 ui_offset, UINT32 *pui_value);

/**
    @addtogroup mIDrvDisp_HDMITx
*/
//@{

void hdmitx_set_tmds_oe(BOOL b_en);
//void hdmi_isr(void);

#if HDMITX_SCDC_RRTEST
static SWTIMER_ID   g_hdmi_sw_timer_id;
static void hdmitx_read_request_sw_timer_cb(UINT32 ui_event)
{
	hdmitx_scdc_write(0xC0, 0x00);
	hdmitx_scdc_write(0x10, 0x07);
	hdmitx_scdc_write(0xC0, 0xFF);
	hdmi_i2c_obj[hdmi_i2c_select].set_config(hdmi_i2c_ses, I2C_CONFIG_ID_RR_EN,        ENABLE);
	DBG_DUMP("KLINSKLINSKLINS ");
}

#endif

/*
    hdmi isr

    hdmi isr

    @return void
*/
void hdmi_isr(void)
{
	T_HDMI_INTSTS_REG    reg_int_sts;
	T_HDMI_SYSCTRL_REG   reg_sys_ctrl;

	reg_int_sts.reg = HDMI_GETREG(HDMI_INTSTS_REG_OFS);
	reg_int_sts.reg &= HDMI_GETREG(HDMI_INTEN_REG_OFS);

	if (reg_int_sts.reg) {
		// Clear Interrupt Status
		HDMI_SETREG(HDMI_INTSTS_REG_OFS, reg_int_sts.reg);

		gui_hdmi_int_sts |= reg_int_sts.reg;

		if (reg_int_sts.bit.RSEN_STS) {
			//hdmitx_debug(("HDMI RSEN!\r\n"));
			reg_sys_ctrl.reg = HDMI_GETREG(HDMI_SYSCTRL_REG_OFS);
			if (reg_sys_ctrl.bit.RSEN) {
				hdmitx_set_tmds_oe(TRUE);
			} else {
				hdmitx_set_tmds_oe(FALSE);
			}
		}

#if (DRV_SUPPORT_IST == ENABLE)
		if ((reg_int_sts.reg & CEC_INTALL) && (pf_ist_cb_hdmi != NULL)) {
			drv_set_ist_event(DRV_IST_MODULE_HDMI, (reg_int_sts.reg & CEC_INTALL));
		}
#else
		if ((reg_int_sts.reg & CEC_INTALL) && (p_cec_cb_func)) {
			p_cec_cb_func((reg_int_sts.reg & CEC_INTALL));
		}
#endif
		/* set_flg(ID flgid, FLGPTN setptn) */

		iset_flg(FLG_ID_HDMI, (FLGPTN) FLGPTN_HDMI);


#if HDMITX_DEBUG
		if (reg_int_sts.bit.OVRUN_STS) {
			hdmitx_debug(("HDMI AUD OverRun!\r\n"));
		}
		if (reg_int_sts.bit.RSEN_STS) {
			hdmitx_debug(("HDMI RSEN!\r\n"));
		}
		if (reg_int_sts.bit.HPD_CHG_STS) {
			reg_sys_ctrl.reg = HDMI_GETREG(HDMI_SYSCTRL_REG_OFS);
			if (reg_sys_ctrl.bit.HPD) {
				hdmitx_debug(("HDMI HPD plug!\r\n"));
			} else {
				hdmitx_debug(("HDMI HPD unplug!\r\n"));
			}
		}
		if (reg_int_sts.bit.VSYNC_STS) {
			hdmitx_debug(("HDMI VSYNC!\r\n"));
		}
		if (reg_int_sts.bit.PIXLK_STBL_STS) {
			hdmitx_debug(("HDMI PCLK STBL!\r\n"));
		}
		if (reg_int_sts.bit.ACR_OVR_STS) {
			hdmitx_debug(("HDMI ACR!\r\n"));
		}
		if (reg_int_sts.bit.CTS_CHG_STS) {
			hdmitx_debug(("HDMI CTS-CHG!\r\n"));
		}
		if (reg_int_sts.bit.GCP_CHG_MUTE_STS) {
			hdmitx_debug(("HDMI GCP_CHG_MUTE_STS!\r\n"));
		}
#endif

#if HDMICEC_DEBUG
		if (reg_int_sts.bit.CEC_RX_FRM_STS) {
			hdmicec_debug(("HDMI CEC_RX_FRM_STS!\r\n"));
		}
		if (reg_int_sts.bit.CEC_RXPOLLING_STS) {
			hdmicec_debug(("HDMI CEC_RXPOLLING_STS!\r\n"));
		}
		if (reg_int_sts.bit.CEC_TX_FRM_STS) {
			hdmicec_debug(("HDMI CEC_TX_FRM_STS!\r\n"));
		}
		if (reg_int_sts.bit.CEC_TX_RETRY_STS) {
			hdmicec_debug(("HDMI CEC_TX_RETRY_STS!\r\n"));
		}
		if (reg_int_sts.bit.CEC_TX_AL_STS) {
			hdmicec_debug(("HDMI CEC_TX_AL_STS!\r\n"));
		}
		if (reg_int_sts.bit.CEC_RX_TIMEOUT_STS) {
			hdmicec_debug(("HDMI CEC_RX_TIMEOUT_STS!\r\n"));
		}
		if (reg_int_sts.bit.CEC_ERROR_STS) {
			hdmicec_debug(("HDMI CEC_ERROR_STS!\r\n"));
		}
#endif
	}
}

#if 1

/*
    For TC72670 HDMI PHY Control at FPGA
*/
ER hdmitx_write_phy(UINT32 ui_offset, UINT32 ui_value)
{
#if _FPGA_EMULATION_
	I2C_DATA    i2c_data;
	I2C_BYTE    i2c_byte[4];
	I2C_STS     i2c_sts;
	UINT32      addr = HDMI_I2C_ADDR;

	//DBG_DUMP("SET TC670 REG[0x%02X] = 0x%02X\r\n",uiOffset,uiValue);

	hdmiddc_lock(0);

	i2c_data.VersionInfo     = DRV_VER_96680;
	i2c_data.pByte           = i2c_byte;
	i2c_data.ByteCount       = I2C_BYTE_CNT_3;

	i2c_byte[0].uiValue      = addr;
	i2c_byte[0].Param        = I2C_BYTE_PARAM_START;

	i2c_byte[1].uiValue      = ui_offset;
	i2c_byte[1].Param        = I2C_BYTE_PARAM_NONE;

	i2c_byte[2].uiValue      = ui_value;
	i2c_byte[2].Param        = I2C_BYTE_PARAM_STOP;

	i2c_sts = hdmiddc_transmit(&i2c_data);
	if (i2c_sts != I2C_STS_OK) {
		DBG_ERR("Tx Err. %d\r\n", i2c_sts);
	}

	hdmiddc_unlock(0);

#else

#if _EMULATION_
	DBG_DUMP("HDMIW[0x%02X]=0x%02X\r\n", ui_offset, ui_value);
#endif
	if (ui_offset >= 0x80) {
		HDMI_SETREG(HDMI_PHYBASE_REG_OFS + 0x200 + (ui_offset << 2), ui_value);
	}
	else {
		HDMI_SETREG(HDMI_PHYBASE_REG_OFS + (ui_offset << 2), ui_value);
	}
#endif /* _FPGA_EMULATION_ */
	return E_OK;
}

/*
    For TC72670 HDMI PHY Control at FPGA
*/
ER hdmitx_read_phy(UINT32 ui_offset, UINT32 *pui_value)
{
#if _FPGA_EMULATION_

	I2C_DATA    i2c_data;
	I2C_BYTE    i2c_byte[4];
	I2C_STS     i2c_sts;
	UINT32      addr = HDMI_I2C_ADDR;

	hdmiddc_lock(0);

#if 1
	i2c_data.VersionInfo     = DRV_VER_96680;
	i2c_data.pByte           = i2c_byte;
	i2c_data.ByteCount       = I2C_BYTE_CNT_3;

	i2c_byte[0].uiValue      = addr;
	i2c_byte[0].Param        = I2C_BYTE_PARAM_START;

	i2c_byte[1].uiValue      = ui_offset;
	i2c_byte[1].Param        = I2C_BYTE_PARAM_NONE;

	i2c_data.ByteCount   = I2C_BYTE_CNT_1;
	i2c_byte[2].Param    = I2C_BYTE_PARAM_NACK | I2C_BYTE_PARAM_STOP;
	if ((i2c_sts = hdmiddc_receive(&i2c_data)) != I2C_STS_OK) {
		DBG_ERR("RX Err. %d\r\n", i2c_sts);
	}

#else
	i2c_data.VersionInfo     = DRV_VER_96680;
	i2c_data.pByte           = i2c_byte;
	i2c_data.ByteCount       = I2C_BYTE_CNT_3;

	i2c_byte[0].uiValue      = addr;
	i2c_byte[0].Param        = I2C_BYTE_PARAM_START;

	i2c_byte[1].uiValue      = ui_offset;
	i2c_byte[1].Param        = I2C_BYTE_PARAM_NONE;

	i2c_byte[2].uiValue      = addr | 0x1;
	i2c_byte[2].Param        = I2C_BYTE_PARAM_START;

	i2c_sts = hdmiddc_transmit(&i2c_data);
	if (i2c_sts != I2C_STS_OK) {
		DBG_ERR("Tx Err1. %d\r\n", i2c_sts);
	}

	i2c_data.ByteCount   = I2C_BYTE_CNT_1;
	i2c_byte[0].Param    = I2C_BYTE_PARAM_NACK | I2C_BYTE_PARAM_STOP;
	//if (hdmiddc_receive(&i2c_data) != I2C_STS_OK) {
	//	DBG_ERR("RX Err. %d\r\n", i2c_sts);
	//}
	if (hdmi_i2c_obj[1].receive(&i2c_data) != I2C_STS_OK) {
		DBG_ERR("RX Err. %d\r\n", i2c_sts);
	}
#endif
	hdmiddc_unlock(0);

	*pui_value = i2c_byte[0].uiValue;

#else
	if (ui_offset >= 0x80) {
		*pui_value = HDMI_GETREG(HDMI_PHYBASE_REG_OFS  + 0x200 + (ui_offset << 2));
	}
	else {
		*pui_value = HDMI_GETREG(HDMI_PHYBASE_REG_OFS + (ui_offset << 2));
	}
#endif /* _FPGA_EMULATION_ */

	return E_OK;
}

/*
    For TC72670 HDMI PHY Control at FPGA
*/
static void hdmitx_init_phy(void)
{
#if !_FPGA_EMULATION_
#if 1
	UINT32	val_R;

	hdmitx_write_phy(0x30, 0x00);
	hdmitx_write_phy(0x09, 0x43);
	hdmitx_write_phy(0x05, 0x05);
	
	hdmitx_write_phy(0x01, 0x01);
	hdmitx_write_phy(0x07, 0x3C);
	hdmitx_write_phy(0x08, 0x00);
	DELAY_US(1000);
	hdmitx_write_phy(0x07, 0x34);

	if (clock_rate > 80000) {
		hdmitx_write_phy(0x05, 0x05);
		hdmitx_write_phy(0x06, 0x00);
		hdmitx_read_phy(0x00, &val_R);
		val_R &= 0xEF;
		hdmitx_write_phy(0x00, val_R);
		hdmitx_read_phy(0x05, &val_R);
		val_R &= 0x33;
		val_R |= 0x04;
		hdmitx_write_phy(0x05, val_R);		
	}
	else {
		hdmitx_write_phy(0x05, 0x05);
		hdmitx_write_phy(0x06, 0x00);
		hdmitx_read_phy(0x00, &val_R);
		val_R &= 0xEF;
		hdmitx_write_phy(0x00, val_R);
		hdmitx_read_phy(0x05, &val_R);
		val_R &= 0x33;
		val_R |= 0xC4;
		hdmitx_write_phy(0x05, val_R);		
	}
	
	hdmitx_write_phy(0x08, 0x08);           //PLL, power on
	hdmitx_read_phy(0x08, &val_R);
	DELAY_US(10000);
	hdmitx_write_phy(0x01, 0x00);
#if 0
	result= efuse_readParamOps(EFUSE_HDMI_TRIM_DATA, &data);		
	if ((result == 0) && (data < 0x20)) {
		//printk("HDMI: efuse 0x%x\n", (uint)data);
		hdmitx_write_phy(0x0C, data);
	}
	else {
		hdmitx_write_phy(0x0C, 0x0D);
	}
#else
	hdmitx_write_phy(0x0C, 0x0D);
#endif
	if (clock_rate <= 110000) {
		hdmitx_write_phy(0x7B, 0x00);
		hdmitx_write_phy(0x9D, 0x00);
		hdmitx_write_phy(0x9E, 0x00);
		hdmitx_write_phy(0x9F, 0x00);
		hdmitx_write_phy(0x08, 0x08);
		hdmitx_write_phy(0x98, 0x00);
		hdmitx_write_phy(0x99, 0x00);
		hdmitx_write_phy(0x0E, 0x00);
		hdmitx_write_phy(0x9B, 0x00);
		hdmitx_write_phy(0x7C, 0x0F);
		hdmitx_write_phy(0x9A, 0x00);
	} else if ( clock_rate > 220000) {
		hdmitx_write_phy(0x7B, 0x04);
		hdmitx_write_phy(0x9D, 0x04);
		hdmitx_write_phy(0x9E, 0x04);
		hdmitx_write_phy(0x9F, 0x00);
		hdmitx_write_phy(0x08, 0x48);
		hdmitx_write_phy(0x98, 0x44);
		hdmitx_write_phy(0x99, 0x01);
		hdmitx_write_phy(0x0E, 0x12);
		hdmitx_write_phy(0x9B, 0x22);
		hdmitx_write_phy(0x7C, 0x0F);
		hdmitx_write_phy(0x9A, 0x00);
	} else {
		hdmitx_write_phy(0x7B, 0x00);
		hdmitx_write_phy(0x9D, 0x00);
		hdmitx_write_phy(0x9E, 0x00);
		hdmitx_write_phy(0x9F, 0x00);
		hdmitx_write_phy(0x08, 0x18);
		hdmitx_write_phy(0x98, 0x11);
		hdmitx_write_phy(0x99, 0x00);
		hdmitx_write_phy(0x0E, 0x01);
		hdmitx_write_phy(0x9B, 0x11);
		hdmitx_write_phy(0x7C, 0x0F);
		hdmitx_write_phy(0x9A, 0x00);
	}	
	printf("%s, clock_rate:%dKHz\n", __func__, clock_rate);

	DELAY_US(10000);
	hdmitx_write_phy(0x7C, 0x0F);
	hdmitx_write_phy(0x0D, 0xF8); 
#else
	int	val_0xE;

	hdmitx_write_phy(0x01, 0x01);    	//HDMITX DPHY reset
	hdmitx_write_phy(0x30, 0x00);
	hdmitx_write_phy(0x09, 0x43);    	//LDO power on
	hdmitx_write_phy(0x05, 0x05);   		//PLL, P2S_DRV, Bias_trim power on
	//hdmitx_write_phy(0x0C, guiTrimRsel);   		// 需帶入efuse trim
	udelay(100);
	hdmitx_write_phy(0x05, 0x04);       	//PLL, P2S_DRV, Bias_trim power off
	udelay(1);
	hdmitx_write_phy(0x05, 0x05);       	//PLL, P2S_DRV, Bias_trim power on

	if (clock_rate <= 110000) { /* 頻率 ≦ 110 Mhz */
		val_0xE = 0x1;
	} else if (clock_rate <= 220000) { /* 110Mhz < 頻率 ≦ 220 Mhz */
		val_0xE = eye_enhance ? 0x21 : 0x11;
	} else if (clock_rate <= 297000) { /* 220Mhz < 頻率 ≦ 297 Mhz */
		val_0xE = eye_enhance ? 0x75 : 0x65;
	} else {
		val_0xE = 0x1;
	}
	printf("%s, clock_rate:%dKHz, ofs 0xE:0x%x \n", __func__, clock_rate, val_0xE);
	hdmitx_write_phy(0x0E, val_0xE);	//DRV enable

	udelay(1000);
	hdmitx_write_phy(0x01, 0x00);       	//HDMITX DPHY reset release
#endif

#elif PHY_TC18034//0:TC670(TSMC), 1:tc18034(UMC)
	I2C_SESSION Sess = I2C_TOTAL_SESSION;

	DBG_DUMP("Init TC18034 PHY Start\r\n");

	hdmiddc_open(&Sess);
	hdmiddc_setConfig(Sess, I2C_CONFIG_ID_BUSCLOCK,      80000);
	hdmiddc_setConfig(Sess, I2C_CONFIG_ID_HANDLE_NACK,   TRUE);

#if 1//0:pattern gen, 1:normal flow
    hdmitx_write_phy(0xFF, 0x00);
    hdmitx_write_phy(0x02, 0x03);
    hdmitx_write_phy(0x03, 0x81);
    hdmitx_write_phy(0xFF, 0x02);
    hdmitx_write_phy(0x30, 0x10);

    hdmitx_write_phy(0xFF, 0x00);
    hdmitx_write_phy(0xE2, 0x03);
    hdmitx_write_phy(0xE2, 0x02);

    hdmitx_write_phy(0xFF, 0x02);
    hdmitx_write_phy(0x09, 0x43);
    hdmitx_write_phy(0x05, 0x05);
    hdmitx_write_phy(0x0B, 0x1D);
    hdmitx_write_phy(0x0C, 0x10);
    hdmitx_write_phy(0x0E, 0x65);

    hdmitx_write_phy(0xFF, 0x00);
    hdmitx_write_phy(0xE3, 0x01);
    hdmitx_write_phy(0xFF, 0x01);
    hdmitx_write_phy(0x88, 0x00);
    hdmitx_write_phy(0x89, 0x60);
    hdmitx_write_phy(0x8A, 0x0C);

#else
    hdmitx_write_phy(0xFF, 0x00);
    hdmitx_write_phy(0xE2, 0x03);
    hdmitx_write_phy(0xE2, 0x02);

    hdmitx_write_phy(0xFF, 0x02);
    hdmitx_write_phy(0x09, 0x43);
    hdmitx_write_phy(0x05, 0x05);
    hdmitx_write_phy(0x0B, 0x1D);
    hdmitx_write_phy(0x0C, 0x10);
    hdmitx_write_phy(0x0E, 0x65);

    hdmitx_write_phy(0xFF, 0x00);
    hdmitx_write_phy(0xE3, 0x01);
    hdmitx_write_phy(0xFF, 0x01);
    hdmitx_write_phy(0x88, 0x00);
    hdmitx_write_phy(0x89, 0x60);
    hdmitx_write_phy(0x8A, 0x0C);

    // TG
    hdmitx_write_phy(0xff, 0x02);// p // Page 9
    hdmitx_write_phy(0x15, 0x72);// p
    hdmitx_write_phy(0x16, 0x06);// p
    hdmitx_write_phy(0x17, 0x28);// p
    hdmitx_write_phy(0x18, 0x00);// p
    hdmitx_write_phy(0x19, 0x04);// p
    hdmitx_write_phy(0x1a, 0x01);// p
    hdmitx_write_phy(0x1b, 0x00);// p
    hdmitx_write_phy(0x1c, 0x05);// p
    hdmitx_write_phy(0x1d, 0xee);// p
    hdmitx_write_phy(0x1e, 0x02);// p
    hdmitx_write_phy(0x1f, 0x05);// p
    hdmitx_write_phy(0x20, 0x00);// p
    hdmitx_write_phy(0x21, 0x19);// p
    hdmitx_write_phy(0x22, 0x00);// p
    hdmitx_write_phy(0x23, 0xd0);// p
    hdmitx_write_phy(0x24, 0x02);// p
    hdmitx_write_phy(0x11, 0x1a);// p
    hdmitx_write_phy(0x10, 0x09);// p
#endif

#else
	I2C_SESSION sess = I2C_TOTAL_SESSION;

	DBG_DUMP("Init TC670 PHY Start\r\n");

	//hdmitx_setConfig(HDMI_CONFIG_ID_SWDDC_SCL, P_GPIO_24);
	//hdmitx_setConfig(HDMI_CONFIG_ID_SWDDC_SDA, P_GPIO_25);

	hdmiddc_open(&Sess);
	hdmiddc_set_config(sess, I2C_CONFIG_ID_BUSCLOCK,      80000);
	hdmiddc_set_config(sess, I2C_CONFIG_ID_HANDLE_NACK,   TRUE);

	//// DDR Setting (Main)
	hdmitx_write_phy(0xff, 0x05); // Page 0x05

	// 以下為DDR Initial (好像跟Default一樣, 但我們還是再設一次)
	hdmitx_write_phy(0x8d, 0x02);
	hdmitx_write_phy(0x8e, 0x07);
	hdmitx_write_phy(0xd7, 0x03);
	hdmitx_write_phy(0xd8, 0x00);
	hdmitx_write_phy(0xd9, 0x0e);
	hdmitx_write_phy(0x8d, 0x02);
	hdmitx_write_phy(0x8e, 0x07);
	hdmitx_write_phy(0x93, 0x90);
	hdmitx_write_phy(0x93, 0xd0);
	hdmitx_write_phy(0x95, 0x30);
	hdmitx_write_phy(0x09, 0xf0);
	hdmitx_write_phy(0xc2, 0x00);
	hdmitx_write_phy(0xad, 0x1f);
	hdmitx_write_phy(0xb0, 0x5b);
	hdmitx_write_phy(0xb7, 0x1b);
	hdmitx_write_phy(0xb1, 0xbf);
	hdmitx_write_phy(0xb2, 0x2d);
	hdmitx_write_phy(0xb3, 0x15);
	hdmitx_write_phy(0xae, 0x2d);
	hdmitx_write_phy(0xaf, 0x15);
	hdmitx_write_phy(0xb4, 0x2d);
	hdmitx_write_phy(0xb5, 0x15);
	hdmitx_write_phy(0xbc, 0x2d);
	hdmitx_write_phy(0xbd, 0x15);
	hdmitx_write_phy(0xb8, 0x99);
	hdmitx_write_phy(0xb9, 0x99);
	hdmitx_write_phy(0xa9, 0xe0);
	hdmitx_write_phy(0x9e, 0x03);
	hdmitx_write_phy(0x24, 0xfc);
	hdmitx_write_phy(0x20, 0x02);
	hdmitx_write_phy(0x21, 0x02);
	hdmitx_write_phy(0x22, 0x02);
	hdmitx_write_phy(0x23, 0x02);
	hdmitx_write_phy(0x07, 0x00);
	hdmitx_write_phy(0x08, 0x00);
	hdmitx_write_phy(0x2a, 0x00);
	hdmitx_write_phy(0x2b, 0x00);
	hdmitx_write_phy(0x2c, 0x00);
	hdmitx_write_phy(0x2d, 0x00);
	hdmitx_write_phy(0x14, 0x00);
	hdmitx_write_phy(0x16, 0x00);
	hdmitx_write_phy(0x03, 0x01);
	hdmitx_write_phy(0x19, 0x00);
	hdmitx_write_phy(0x51, 0x09);

	hdmitx_write_phy(0x01, 0x80);
	hdmitx_write_phy(0x15, 0x40);
	hdmitx_write_phy(0x19, 0x40);


	// DDR Setting (TF)
	hdmitx_write_phy(0xff, 0x07); // Page 0x07

	// 以下為DDR Initial (好像跟Default一樣, 但我們還是再設一次)
	hdmitx_write_phy(0x8d, 0x02);
	hdmitx_write_phy(0x8e, 0x07);
	hdmitx_write_phy(0xd7, 0x03);
	hdmitx_write_phy(0xd8, 0x00);
	hdmitx_write_phy(0xd9, 0x0e);
	hdmitx_write_phy(0x8d, 0x02);
	hdmitx_write_phy(0x8e, 0x07);
	hdmitx_write_phy(0x93, 0x90);
	hdmitx_write_phy(0x93, 0xd0);
	hdmitx_write_phy(0x95, 0x30);
	hdmitx_write_phy(0x09, 0xf0);
	hdmitx_write_phy(0xc2, 0x00);
	hdmitx_write_phy(0xad, 0x1f);
	hdmitx_write_phy(0xb0, 0x5b);
	hdmitx_write_phy(0xb7, 0x1b);
	hdmitx_write_phy(0xb1, 0xbf);
	hdmitx_write_phy(0xb2, 0x2d);
	hdmitx_write_phy(0xb3, 0x15);
	hdmitx_write_phy(0xae, 0x2d);
	hdmitx_write_phy(0xaf, 0x15);
	hdmitx_write_phy(0xb4, 0x2d);
	hdmitx_write_phy(0xb5, 0x15);
	hdmitx_write_phy(0xbc, 0x2d);
	hdmitx_write_phy(0xbd, 0x15);
	hdmitx_write_phy(0xb8, 0x99);
	hdmitx_write_phy(0xb9, 0x99);
	hdmitx_write_phy(0xa9, 0xe0);
	hdmitx_write_phy(0x9e, 0x03);
	hdmitx_write_phy(0x24, 0xfc);
	hdmitx_write_phy(0x20, 0x02);
	hdmitx_write_phy(0x21, 0x02);
	hdmitx_write_phy(0x22, 0x02);
	hdmitx_write_phy(0x23, 0x02);
	hdmitx_write_phy(0x07, 0x00);
	hdmitx_write_phy(0x08, 0x00);
	hdmitx_write_phy(0x2a, 0x00);
	hdmitx_write_phy(0x2b, 0x00);
	hdmitx_write_phy(0x2c, 0x00);
	hdmitx_write_phy(0x2d, 0x00);
	hdmitx_write_phy(0x14, 0x00);
	hdmitx_write_phy(0x16, 0x00);
	hdmitx_write_phy(0x03, 0x01);
	hdmitx_write_phy(0x19, 0x00);
	hdmitx_write_phy(0x51, 0x09);

	hdmitx_write_phy(0x01, 0x00);
	hdmitx_write_phy(0x15, 0x43);
	hdmitx_write_phy(0x51, 0x80);
	hdmitx_write_phy(0x19, 0x70);

	// 有Clock Path沒設定到
	hdmitx_write_phy(0x15, 0x63);
	hdmitx_write_phy(0xd2, 0x40);


	hdmitx_write_phy(0x38, 0x18);
	hdmitx_write_phy(0x39, 0x18);
	hdmitx_write_phy(0x40, 0x1A);
	hdmitx_write_phy(0x41, 0x1A);
	hdmitx_write_phy(0x42, 0x1A);
	hdmitx_write_phy(0x43, 0x1A);
	hdmitx_write_phy(0x44, 0x1A);
	hdmitx_write_phy(0x45, 0x1A);
	hdmitx_write_phy(0x46, 0x1C);
	hdmitx_write_phy(0x47, 0x1C);
	hdmitx_write_phy(0x48, 0x1A);
	hdmitx_write_phy(0x53, 0x06);
	hdmitx_write_phy(0x54, 0x1A);
	hdmitx_write_phy(0x55, 0x09);
	hdmitx_write_phy(0x56, 0x88);
	hdmitx_write_phy(0x57, 0x1A);
	hdmitx_write_phy(0x58, 0x06);
	hdmitx_write_phy(0x59, 0x1A);
	hdmitx_write_phy(0x5A, 0x1A);
	hdmitx_write_phy(0x5B, 0x09);
	hdmitx_write_phy(0x64, 0x71);
	hdmitx_write_phy(0x65, 0x04);
	hdmitx_write_phy(0x66, 0x00);
	hdmitx_write_phy(0x67, 0x70);
	hdmitx_write_phy(0x68, 0x71);
	hdmitx_write_phy(0x80, 0x1A);
	hdmitx_write_phy(0x81, 0x1A);
	hdmitx_write_phy(0x82, 0x1A);
	hdmitx_write_phy(0x83, 0x1C);
	hdmitx_write_phy(0x83, 0x1C);
	hdmitx_write_phy(0x85, 0x99);
	hdmitx_write_phy(0x86, 0x99);
	hdmitx_write_phy(0x87, 0x99);
	hdmitx_write_phy(0x88, 0x99);
	hdmitx_write_phy(0x89, 0x00);
	hdmitx_write_phy(0x8A, 0x00);
	hdmitx_write_phy(0x8B, 0x00);
	hdmitx_write_phy(0x8C, 0x00);


	// USB Path要再選, 但不影響HDMI TX, 故列上
	hdmitx_write_phy(0xff, 0x00); // Page 0x00
	hdmitx_write_phy(0x6f, 0x03);
	hdmitx_write_phy(0xff, 0x04); // Page 0x04
	hdmitx_write_phy(0x30, 0x50); // CHIP_SCAN_MODE = 1
	hdmitx_write_phy(0xff, 0x06); // Page 0x06
	hdmitx_write_phy(0x30, 0x50); // CHIP_SCAN_MODE = 1

	// PINMUX to HDMI TX
	hdmitx_write_phy(0xfe, 0x07);

#if 1//0:pattern gen, 1:normal flow
	hdmitx_write_phy(0xff, 0x00);//page 0

	// when using internal clock bit[1] shall be cleared.
	hdmitx_write_phy(0x5f, 0x03);

	hdmitx_write_phy(0xff, 0x09);//page 9

	// important notice: Bit[5] is TCLK sample edge inverse. Bit[1] is external clock
	//hdmitx_writePhy(0x00, 0x22);
	hdmitx_write_phy(0x00, 0x02);

	hdmitx_write_phy(0x05, 0x05);//LVCK
	hdmitx_write_phy(0x06, 0x20);//LPF_CAP2_EN, Default
	hdmitx_write_phy(0x09, 0x43);//ALL Power
	hdmitx_write_phy(0x0e, 0x01);//OE
	hdmitx_write_phy(0x0c, 0x10);//VL
	hdmitx_write_phy(0x0a, 0x00);//reg_rg, Default

	//sramble off  hdmi 1.4: Default off. use hdmi set_config driver to enable.
	hdmitx_write_phy(0x30, 0x00);
	//sramble on  but divide by 4 off
	//hdmitx_write_phy(0x30, 0x04);
	//sramble on  but divide by 4 on
	//hdmitx_write_phy(0x30, 0x06);

	hdmitx_write_phy(0x05, 0x05);//deep color

#elif 1
	//
	// HDMI PHY internal pattern gen 74.25MHz
	// no need to provide anything including clock is using phy internal pll
	//

	// HDMI TX
	hdmitx_write_phy(0xff, 0x00);// p // Page 0
	// when using external clock bit[1] shall be set.
	hdmitx_write_phy(0x5f, 0x01);// p // HDMI TX HI_RSTN
	hdmitx_write_phy(0xff, PHY_PAGE);// p // Page 9
	hdmitx_write_phy(0x05, 0x05);// p // LVCK
	hdmitx_write_phy(0x09, 0x43);// p // ALL Power
	hdmitx_write_phy(0x0e, 0x01);// p // OE
	hdmitx_write_phy(0x0c, 0x00);// p // VL
	hdmitx_write_phy(0x0a, 0x00);// p // reg_rg
	hdmitx_write_phy(0x30, 0x00);// p // HDMI 1.4
	//s c0 w 30 06 p // HDMI 2.0
	hdmitx_write_phy(0x05, 0x05);// p // Deep Color

	hdmitx_write_phy(0x0e, 0x01);// p // 27M

	// MPLL Check
	hdmitx_write_phy(0xff, 0x00);// p // Page 0
	hdmitx_write_phy(0x05, 0x01);// p // MPLL Page B
	hdmitx_write_phy(0xff, 0x01);// p // Page 1


#if 1
	// 74.25M
	hdmitx_write_phy(0x76, 0x60);// p //
	hdmitx_write_phy(0x75, 0x00);// p //
	hdmitx_write_phy(0x77, 0x0C);// p //

	// TG
	hdmitx_write_phy(0xff, PHY_PAGE);// p // Page 9
	hdmitx_write_phy(0x15, 0x72);// p
	hdmitx_write_phy(0x16, 0x06);// p
	hdmitx_write_phy(0x17, 0x28);// p
	hdmitx_write_phy(0x18, 0x00);// p
	hdmitx_write_phy(0x19, 0x04);// p
	hdmitx_write_phy(0x1a, 0x01);// p
	hdmitx_write_phy(0x1b, 0x00);// p
	hdmitx_write_phy(0x1c, 0x05);// p
	hdmitx_write_phy(0x1d, 0xee);// p
	hdmitx_write_phy(0x1e, 0x02);// p
	hdmitx_write_phy(0x1f, 0x05);// p
	hdmitx_write_phy(0x20, 0x00);// p
	hdmitx_write_phy(0x21, 0x19);// p
	hdmitx_write_phy(0x22, 0x00);// p
	hdmitx_write_phy(0x23, 0xd0);// p
	hdmitx_write_phy(0x24, 0x02);// p
	hdmitx_write_phy(0x11, 0x1a);// p
	hdmitx_write_phy(0x10, 0x09);// p
#else
	// 4KP30

	// 74.25M
	//hdmitx_write_phy(0x76 ,0x60);// p //
	//hdmitx_write_phy(0x75 ,0x00);// p //
	//hdmitx_write_phy(0x77 ,0x0C);// p //

	// 27M
	hdmitx_write_phy(0x76, 0x80);// p //
	hdmitx_write_phy(0x75, 0x00);// p //
	hdmitx_write_phy(0x77, 0x04);// p //

	hdmitx_write_phy(0xff, PHY_PAGE);// p // Page 9
	hdmitx_write_phy(0x15, 0x7C);// p
	hdmitx_write_phy(0x16, 0x15);// p
	hdmitx_write_phy(0x17, 0x58);// p
	hdmitx_write_phy(0x18, 0x00);// p
	hdmitx_write_phy(0x19, 0x80);// p
	hdmitx_write_phy(0x1a, 0x01);// p
	hdmitx_write_phy(0x1b, 0xFE);// p
	hdmitx_write_phy(0x1c, 0x0F);// 0xFFE = 4094
	hdmitx_write_phy(0x1d, 0xca);// p
	hdmitx_write_phy(0x1e, 0x08);// p
	hdmitx_write_phy(0x1f, 0x0a);// p
	hdmitx_write_phy(0x20, 0x00);// p
	hdmitx_write_phy(0x21, 0x52);// p
	hdmitx_write_phy(0x22, 0x00);// p
	hdmitx_write_phy(0x23, 0x70);// p
	hdmitx_write_phy(0x24, 0x08);// p
	hdmitx_write_phy(0x11, 0x10);// p
	hdmitx_write_phy(0x10, 0x09);// p
#endif

	hdmitx_write_phy(0xff, PHY_PAGE);// p // Page 9
	hdmitx_write_phy(0x0c, 0x10);// TRIM to 0x10


	/*
	    important Notice: Below is using HDMI TC72670 PHY embedded patgen with embedded PLL 74.25MHz.
	    Below is the option of using embedded patgen but using external clock.
	    When FPGA stage, we can use this to verify our pixel clock quality.
	*/
#if 0
	//use external clock
	hdmitx_write_phy(0xff, 0x00);
	hdmitx_write_phy(0x5f, 0x03);// p // HDMI TX HI_RSTN
	hdmitx_write_phy(0xff, PHY_PAGE);
	hdmitx_write_phy(0x00, 0x02);// external clock
#endif

#elif 1
	// HDMI TX
	hdmitx_write_phy(0xff, 0x00);// p // Page 0
	hdmitx_write_phy(0x5f, 0x01);// p // HDMI TX HI_RSTN
	hdmitx_write_phy(0xff, PHY_PAGE);// p // Page 9
	hdmitx_write_phy(0x05, 0x05);// p // LVCK
	hdmitx_write_phy(0x06, 0x20);
	hdmitx_write_phy(0x09, 0x43);// p // ALL Power
	hdmitx_write_phy(0x0e, 0x01);// p // OE
	hdmitx_write_phy(0x0c, 0x00);// p // VL
	hdmitx_write_phy(0x0a, 0x00);// p // reg_rg

	// HDMI 1.4 / 2.0 select
	hdmitx_write_phy(0x30, 0x00);// p // HDMI 1.4
	//hdmitx_write_phy(0x30 ,0x04);// p //sramble on   hdmi 2.0  bit[1]: Divide by 4.  bit[2]: Scramble EN


	hdmitx_write_phy(0x05, 0x05);// p // Deep Color
	//s c0 w 0e 01 p // 148.5M
	hdmitx_write_phy(0x0e, 0x15);// p // 297M

	// MPLL Check
	hdmitx_write_phy(0xff, 0x00);// p // Page 0
	hdmitx_write_phy(0x05, 0x01);// p // MPLL Page B
	hdmitx_write_phy(0xff, 0x01);// p // Page 1

	// 594M
	//hdmitx_write_phy(0x0e ,0x3d);// p // 297M
	//hdmitx_write_phy(0x76 ,0x00);// p //
	//hdmitx_write_phy(0x75 ,0x00);// p //
	//hdmitx_write_phy(0x77 ,0x63);// p //

	// 297M
	hdmitx_write_phy(0x76, 0x80);// p //
	hdmitx_write_phy(0x75, 0x00);// p //
	hdmitx_write_phy(0x77, 0x31);// p //

	// 74.25M
	//hdmitx_write_phy(0x76 ,0x60);// p //
	//hdmitx_write_phy(0x75 ,0x00);// p //
	//hdmitx_write_phy(0x77 ,0x0C);// p //

	// 148.5M
	//hdmitx_write_phy(0x76 ,0xc0);// p //
	//hdmitx_write_phy(0x75 ,0x00);// p //
	//hdmitx_write_phy(0x77 ,0x18);// p //

	// HDMI TX 297MHz/594MHz
	hdmitx_write_phy(0xff, PHY_PAGE);// p // Page 9
	hdmitx_write_phy(0x15, 0x7C);// p
	hdmitx_write_phy(0x16, 0x15);// p
	hdmitx_write_phy(0x17, 0x58);// p
	hdmitx_write_phy(0x18, 0x00);// p
	hdmitx_write_phy(0x19, 0x80);// p
	hdmitx_write_phy(0x1a, 0x01);// p
	hdmitx_write_phy(0x1b, 0x00);// p
	hdmitx_write_phy(0x1c, 0x10);// 0x1000 = 4096
	hdmitx_write_phy(0x1d, 0xca);// p
	hdmitx_write_phy(0x1e, 0x08);// p
	hdmitx_write_phy(0x1f, 0x0a);// p
	hdmitx_write_phy(0x20, 0x00);// p
	hdmitx_write_phy(0x21, 0x52);// p
	hdmitx_write_phy(0x22, 0x00);// p
	hdmitx_write_phy(0x23, 0x70);// p
	hdmitx_write_phy(0x24, 0x08);// p
	hdmitx_write_phy(0x11, 0x1A);// p
	hdmitx_write_phy(0x10, 0x09);// p

	hdmitx_write_phy(0xff, PHY_PAGE);// p // Page 9
	hdmitx_write_phy(0x0c, 0x10);// TRIM to 0x10
#elif 1

	// HDMI TX
	hdmitx_write_phy(0xff, 0x00);// p // Page 0
	hdmitx_write_phy(0x5f, 0x01);// p // HDMI TX HI_RSTN

	hdmitx_write_phy(0xff, PHY_PAGE);// p // Page 9
	hdmitx_write_phy(0x05, 0x05);// p // LVCK
	hdmitx_write_phy(0x06, 0x22);
	hdmitx_write_phy(0x09, 0x43);// p // ALL Power
	hdmitx_write_phy(0x0e, 0x01);// p // OE
	hdmitx_write_phy(0x0c, 0x00);// p // VL
	hdmitx_write_phy(0x0a, 0x02);// p // reg_rg
	//s c0 w 30 00 p // HDMI 1.4
	hdmitx_write_phy(0x30, 0x00);// p // HDMI 1.4
	//hdmitx_write_phy(0x30 ,0x06);// p // HDMI 2.0
	hdmitx_write_phy(0x05, 0x05);// p // Deep Color

	//s c0 w 0e 01 p // 148.5M
	//s c0 w 0e 15 p // 297M
	hdmitx_write_phy(0x0e, 0x1d);// p // 594M

	// MPLL Check
	hdmitx_write_phy(0xff, 0x00);// p // Page 0
	hdmitx_write_phy(0x05, 0x01);// p // MPLL Page B
	hdmitx_write_phy(0xff, 0x01);// p // Page 1

	// 594M
	hdmitx_write_phy(0x76, 0x00);// p //
	hdmitx_write_phy(0x75, 0x00);// p //
	hdmitx_write_phy(0x77, 0x63);// p //

	// HDMI TX 297MHz/594MHz
	hdmitx_write_phy(0xff, PHY_PAGE);// p // Page 9
	hdmitx_write_phy(0x15, 0x30);// p
	hdmitx_write_phy(0x16, 0x11);// p
	hdmitx_write_phy(0x17, 0x58);// p
	hdmitx_write_phy(0x18, 0x00);// p
	hdmitx_write_phy(0x19, 0x80);// p
	hdmitx_write_phy(0x1a, 0x01);// p
	hdmitx_write_phy(0x1b, 0x00);// p
	hdmitx_write_phy(0x1c, 0x0f);// p
	hdmitx_write_phy(0x1d, 0xca);// p
	hdmitx_write_phy(0x1e, 0x08);// p
	hdmitx_write_phy(0x1f, 0x0a);// p
	hdmitx_write_phy(0x20, 0x00);// p
	hdmitx_write_phy(0x21, 0x52);// p
	hdmitx_write_phy(0x22, 0x00);// p
	hdmitx_write_phy(0x23, 0x70);// p
	hdmitx_write_phy(0x24, 0x08);// p
	hdmitx_write_phy(0x11, 0x18);// p
	hdmitx_write_phy(0x10, 0x09);// p

	hdmitx_write_phy(0xff, PHY_PAGE);// p // Page 9
	hdmitx_write_phy(0x0c, 0x10);// TRIM to 0x10

#endif
#endif
	DBG_DUMP("Init PHY END\r\n");
}

#endif

/*
    Wait HDMI interrupt event flag

    Wait HDMI interrupt event flag

    @param[in] waiting_int   OR of waited interrupt flag ID

    @return OR of waited interrupt flag ID
*/
HDMI_INT_TYPE hdmitx_wait_flag(HDMI_INT_TYPE waiting_int)
{
	FLGPTN  ui_flag;
	UINT32  int_sts;

	while (1) {
		wai_flg(&ui_flag, FLG_ID_HDMI, FLGPTN_HDMI, TWF_ORW | TWF_CLR);
		int_sts = gui_hdmi_int_sts;
		if (int_sts & waiting_int) {
			gui_hdmi_int_sts &= ~waiting_int;
			return (int_sts & waiting_int);
		}
	}
}

/*
    Reset hdmi Tx module

    Reset hdmi Tx module's analog & digital logic to default state

    @param[in] b_en     Reset hdmi Tx module
       -@b TRUE: reset
       -@b FALSE: unreset

    @return void
*/
static void hdmitx_reset(BOOL b_en)
{
	T_HDMI_SYSCTRL_REG reg_sys_ctrl;

	reg_sys_ctrl.reg = HDMI_GETREG(HDMI_SYSCTRL_REG_OFS);

	if (b_en) {
		reg_sys_ctrl.bit.TXD_RST      = 1;
		reg_sys_ctrl.bit.TXPHY_RST    = 1;
	} else {
		reg_sys_ctrl.bit.TXD_RST      = 0;
		reg_sys_ctrl.bit.TXPHY_RST    = 0;
	}

	HDMI_SETREG(HDMI_SYSCTRL_REG_OFS, reg_sys_ctrl.reg);

}

/*
    Set HDMI TMDS Output Enable

    Set HDMI TMDS Output Enable. Disable TMDS output can save power during HDMI power down.

    @param[in] b_en
     - @b TRUE:  Enable.
     - @b FALSE: Disable

    @return void
*/
void hdmitx_set_tmds_oe(BOOL b_en)
{

}

/*
    Set HDMI controller's Power ON/OFF

    Enable HDMI Power Down would also Disable TMDS Output Enable in this API.

    @param[in] b_pd
     - @b TRUE:   Set HDMI Power OFF
     - @b FALSE:  Set HDMI Power ON

    @return void
*/
static void hdmitx_set_power_down(BOOL b_pd)
{
#if _FPGA_EMULATION_
	hdmitx_write_phy(0xff, PHY_PAGE);// page 9
#endif

#if !_EMULATION_
	if (b_pd) {
		//hdmitx_write_phy(0x0E, 0x0);	 //DRV disable
		//hdmitx_write_phy(0x5, 0x4);	 //PLL, P2S_DRV, Bias_trim power off
		hdmitx_write_phy(0x09, 0x42);
	} else {
		hdmitx_write_phy(0x09, 0x43);
	}
#endif
}

/*
    Set HDMI Interrupt Enable/Disable

    Set HDMI Interrupt Enable/Disable

    @param[in] int_type  Interrupt ID to be ENABLE/DISABLE
    @param[in] enable
     - @b TRUE:  ENABLE interrupt
     - @b FALSE: DISABLE interrupt

    @return void
*/
void hdmitx_set_int_en(HDMI_INT_TYPE int_type, BOOL enable)
{
	T_HDMI_INTEN_REG    reg_int_en;
	unsigned long flags;

	loc_cpu(flags);
	reg_int_en.reg = HDMI_GETREG(HDMI_INTEN_REG_OFS);

	if (enable == TRUE) {
		reg_int_en.reg |= int_type;
	} else {
		reg_int_en.reg &= ~int_type;
	}
	HDMI_SETREG(HDMI_INTEN_REG_OFS, reg_int_en.reg);
	unl_cpu(flags);

	return;
}

/*
    HDMI Driver Attach

    This api would open DDC's pinmux and then enable HDMI's source clock.
*/
static void hdmitx_attach(void)
{
#if 0
	pll_enable_clock(HDMI_CLK);
#endif
}

/*
    HDMI Driver Detach

    This api would close DDC's pinmux and disable HDMI's source clock.
*/
static void hdmitx_detach(void)
{
#if 0
	pll_disable_clock(HDMI_CLK);
#endif
}

/*
    Enable HDMI InfoFrames Transmission.
*/
void hdmitx_enable_infofrm_tx(HDMIINFO_EN  info_en, HDMIINFO_RPT info_rpt_en)
{
	T_HDMI_PKTCTRL_REG reg_pkt_ctrl;

	reg_pkt_ctrl.reg = HDMI_GETREG(HDMI_PKTCTRL_REG_OFS);
	reg_pkt_ctrl.reg |= (info_en | info_rpt_en);
	HDMI_SETREG(HDMI_PKTCTRL_REG_OFS, reg_pkt_ctrl.reg);
}

/*
    Disable HDMI InfoFrames Transmission.
*/
void hdmitx_disable_infofrm_tx(HDMIINFO_EN  info_dis, HDMIINFO_RPT info_rpt_dis)
{
	T_HDMI_PKTCTRL_REG reg_pkt_ctrl;

	reg_pkt_ctrl.reg = HDMI_GETREG(HDMI_PKTCTRL_REG_OFS);
	reg_pkt_ctrl.reg &= ~(info_dis | info_rpt_dis);
	HDMI_SETREG(HDMI_PKTCTRL_REG_OFS, reg_pkt_ctrl.reg);
}

/*
    Setup HDMI AUDIO's configurations
*/
static UINT32 hdmitx_config_audio(HDMI_AUDIOFMT ADOFMT)
{
	T_HDMI_AUDCTRL_REG  reg_aud_ctrl;
	T_HDMI_PCMHDR0_REG  reg_phdr0;
	T_HDMI_PCMHDR1_REG  reg_phdr1;
	T_HDMI_INFO0_REG    reg_info0;
	T_HDMI_INFO1_REG    reg_info1;
	T_HDMI_INFO2_REG    reg_info2;
	T_HDMI_INFO3_REG    reg_info3;

	reg_aud_ctrl.reg = HDMI_GETREG(HDMI_AUDCTRL_REG_OFS);
	reg_aud_ctrl.bit.CTS_SEL = 0;// Use HW-CTS Value

	reg_phdr0.reg    = HDMI_GETREG(HDMI_PCMHDR0_REG_OFS);

	switch (ADOFMT) {
	case HDMI_AUDIO48KHZ: {
			//N = 6144 = 0x1800
			reg_aud_ctrl.bit.N_SVAL = HDMI_NVAL6144;
			reg_phdr0.bit.FS = HDMI_PCM_FS48KHZ;
			break;
		}
	case HDMI_AUDIO44_1KHZ: {
			//N = 6272 = 0x1880
			reg_aud_ctrl.bit.N_SVAL = HDMI_NVAL6272;
			reg_phdr0.bit.FS = HDMI_PCM_FS44_1KHZ;
			break;
		}
	case HDMI_AUDIO32KHZ: {
			//N = 4096 = 0x1000
			reg_aud_ctrl.bit.N_SVAL = HDMI_NVAL4096;
			reg_phdr0.bit.FS = HDMI_PCM_FS32KHZ;
			break;
		}
	default: {
			DBG_WRN("Not HDMI support Audio mode\r\n");
			return 1;
		}

	}
	HDMI_SETREG(HDMI_AUDCTRL_REG_OFS, reg_aud_ctrl.reg);
	HDMI_SETREG(HDMI_PCMHDR0_REG_OFS, reg_phdr0.reg);


	// Set PCM Header: Audio 16bits per sample
	reg_phdr1.reg = HDMI_GETREG(HDMI_PCMHDR1_REG_OFS);
	reg_phdr1.bit.MAXLEN  = 0;//MaxLen = 20bits
	reg_phdr1.bit.WORDLEN = 1;//16bits
	HDMI_SETREG(HDMI_PCMHDR1_REG_OFS, reg_phdr1.reg);


	//
	//  Audio infoframe Configuration
	//

	/* Header Word */
	reg_info0.reg = 0;//HDMI_GETREG(HDMI_AUDINFO_REG_OFS+HDMI_INFO0_REG_OFS);
	reg_info0.bit.TYPE = HDMIINFO_AUD_TYPE;
	reg_info0.bit.VERS = HDMIINFO_AUD_VER;
	reg_info0.bit.LEN  = HDMIINFO_AUD_LEN;

	reg_info0.bit.CHSUM  = (HDMIINFO_AUD_TYPE + HDMIINFO_AUD_VER + HDMIINFO_AUD_LEN + HDMIINFO_AUD_D1_CHCNT_2CH);
	reg_info0.bit.CHSUM  = 0x100 - reg_info0.bit.CHSUM;
	HDMI_SETREG(HDMI_AUDINFO_REG_OFS + HDMI_INFO0_REG_OFS, reg_info0.reg);

	/* Data Word 1*/
	reg_info1.reg = 0;
	reg_info1.bit.BYTE1 = HDMIINFO_AUD_D1_CHCNT_2CH;// Channel Cnt = 2ch;  CodingType = Refer to Streaming Header
	HDMI_SETREG(HDMI_AUDINFO_REG_OFS + HDMI_INFO1_REG_OFS, reg_info1.reg);

	/* Data Word 2*/
	reg_info2.reg = 0;
	HDMI_SETREG(HDMI_AUDINFO_REG_OFS + HDMI_INFO2_REG_OFS, reg_info2.reg);

	/* Data Word 3*/
	reg_info3.reg = 0;
	HDMI_SETREG(HDMI_AUDINFO_REG_OFS + HDMI_INFO3_REG_OFS, reg_info3.reg);

	return 0;
}

static UINT32 hdmitx_get_picture_aspect(HDMI_VIDEOID vid_code)
{
	UINT32 pic_aspect_ratio = HDMIINFO_AVI_D1_SAMEASPIC_ASPECTRATIO;

	switch (vid_code) {
	case HDMI_1280X720P60:      /* VID4     1280X720P   @59.94/60Hz */
	case HDMI_1920X1080P60:     /* VID16    1920X1080P  @59.94/60Hz */
	case HDMI_1920X1080P30:     /* VID34    1920X1080P  @29.97/30Hz */
	case HDMI_2560X1440P60:
	case HDMI_1920X1200P60:
		pic_aspect_ratio = HDMIINFO_AVI_16_9_SAMEASPIC_ASPECTRATIO;
		break;
	default:
		break;
	}
	return pic_aspect_ratio;
}

/*
    Setup HDMI VIDEO's configurations
*/
static UINT32 hdmitx_config_video(HDMI_VIDEOID vid_code)
{
	T_HDMI_TMDSCTRL0_REG    reg_tmds_ctrl0;
	T_HDMI_INFO0_REG        reg_info0;
	T_HDMI_INFO1_REG        reg_info1;
	T_HDMI_INFO2_REG        reg_info2;
	T_HDMI_INFO3_REG        reg_info3;
	T_HDMI_INFO4_REG        reg_info4;
	UINT8                   under_scan, pr = 0x0;
	HDMI_INTF_MODE          mode_fmt;
	UINT32					pic_aspect_ratio = HDMIINFO_AVI_16_9_SAMEASPIC_ASPECTRATIO;

	//if ((VID_Code >= HDMI_3840X2160P24) && (VID_Code <= HDMI_4096X2160P30)) {
	//	hdmitx_write_phy(0x0E, 0x65);
	//} else {
	//	hdmitx_write_phy(0x0E, 0x01);
	//}


	//
	// Handle the pixel replication
	//
	reg_tmds_ctrl0.reg = HDMI_GETREG(HDMI_TMDSCTRL0_REG_OFS);
	switch (vid_code) {
#if 1//!_FPGA_EMULATION_
	/* Pixel Repeat Once */
	case HDMI_720X480I60:      /* VID6     720X480i   @59.94/60Hz */
	case HDMI_720X480I60_16X9: /* VID7     720X480i   @59.94/60Hz */
	case HDMI_720X240P60:      /* VID8     720X240p   @59.94/60Hz */
	case HDMI_720X240P60_16X9: /* VID9     720X240p   @59.94/60Hz */
	case HDMI_1440X480I60:     /* VID10    1440X480I  @59.94/60Hz */
	case HDMI_1440X480I60_16X9:/* VID11    1440X480I  @59.94/60Hz */
	case HDMI_1440X240P60:     /* VID12    1440X240P  @59.94/60Hz */
	case HDMI_1440X240P60_16X9:/* VID13    1440X240P  @59.94/60Hz */
	case HDMI_720X576I50:      /* VID21    720X576I   @50Hz       */
	case HDMI_720X576I50_16X9: /* VID22    720X576I   @50Hz       */
	case HDMI_720X288P50:      /* VID23    720X288P   @50Hz       */
	case HDMI_720X288P50_16X9: /* VID24    720X288P   @50Hz       */
	case HDMI_1440X576I50:     /* VID25    1440X576I  @50Hz       */
	case HDMI_1440X576I50_16X9:/* VID26    1440X576I  @50Hz       */
	case HDMI_1440X288P50:     /* VID27    1440X288P  @50Hz       */
	case HDMI_1440X288P50_16X9:/* VID28    1440X288P  @50Hz       */
	case HDMI_720X576I100:     /* VID44    720X576I   @100Hz      */
	case HDMI_720X576I100_16X9:/* VID45    720X576I   @100Hz      */
	case HDMI_720X480I120:     /* VID50    720X480I   @120Hz      */
	case HDMI_720X480I120_16X9:/* VID51    720X480I   @120Hz      */
	case HDMI_720X576I200:     /* VID54    720X576I   @200Hz      */
	case HDMI_720X576I200_16X9:/* VID55    720X576I   @200Hz      */
	case HDMI_720X480I240:     /* VID58    720X480I   @240Hz      */
	case HDMI_720X480I240_16X9: { /* VID59    720X480I   @240Hz      */
#if _FPGA_EMULATION_
			hdmitx_write_phy(0xff, PHY_PAGE);// page 9
#endif
#if !_EMULATION_
			//hdmitx_write_phy(0x05, 0x09);// tclk x2
#endif
			pr = 0x1;
			break;
		}
#endif

	/* Pixel Repeat None */
	default: {
#if _FPGA_EMULATION_
			hdmitx_write_phy(0xff, PHY_PAGE);// page 9
#endif
#if !_EMULATION_
			//hdmitx_write_phy(0x05, 0x05);// tclk x1
#endif
			pr = 0x0;
			break;
		}

	}
	//reg_tmds_ctrl0.bit.STERM       = 1;
	HDMI_SETREG(HDMI_TMDSCTRL0_REG_OFS, reg_tmds_ctrl0.reg);

	hdmitx_disable_infofrm_tx(HDMIINFO_EN_GEN1, HDMIINFO_RPT_GEN1);
	pic_aspect_ratio = hdmitx_get_picture_aspect(vid_code);

	switch (vid_code) {
	case HDMI14B_3840X2160P24:
	case HDMI14B_3840X2160P25:
	case HDMI14B_3840X2160P30:
	//coverity[unterminated_case]
	case HDMI14B_4096X2160P24:
	{
		T_HDMI_INFO0_REG reg_info0;
		T_HDMI_INFO1_REG reg_info1;
		T_HDMI_INFO2_REG reg_info2;


		/* Header Word */
		reg_info0.reg = 0;
		reg_info0.bit.TYPE = HDMIINFO_VSI_TYPE;
		reg_info0.bit.VERS = HDMIINFO_VSI_VER;
		reg_info0.bit.LEN  = 0x05;

		/* Data Word 1*/
		reg_info1.reg = 0;
		reg_info1.bit.BYTE1 = HDMIINFO_VSI_DB1;
		reg_info1.bit.BYTE2 = HDMIINFO_VSI_DB2;
		reg_info1.bit.BYTE3 = HDMIINFO_VSI_DB3;
		reg_info1.bit.BYTE4 = 0x20;
		HDMI_SETREG(HDMI_GENINFO_REG_OFS + HDMI_INFO1_REG_OFS, reg_info1.reg);

		/* Data Word 2*/
		reg_info2.reg = 0;

		if (vid_code == HDMI14B_3840X2160P30)
			reg_info2.bit.BYTE5 = 1;
		else if (vid_code == HDMI14B_3840X2160P25)
			reg_info2.bit.BYTE5 = 2;
		else if (vid_code == HDMI14B_3840X2160P24)
			reg_info2.bit.BYTE5 = 3;
		else if (vid_code == HDMI14B_4096X2160P24)
			reg_info2.bit.BYTE5 = 4;

		reg_info2.bit.BYTE6 = 0x00;
		reg_info2.bit.BYTE7 = 0x00;
		reg_info2.bit.BYTE8 = 0x00;
		HDMI_SETREG(HDMI_GENINFO_REG_OFS + HDMI_INFO2_REG_OFS, reg_info2.reg);


		reg_info0.bit.CHSUM  = (UINT8)(HDMIINFO_VSI_TYPE + HDMIINFO_VSI_VER + reg_info0.bit.LEN + HDMIINFO_VSI_DB1 + HDMIINFO_VSI_DB2 + HDMIINFO_VSI_DB3 + 0x20 + reg_info2.bit.BYTE5 + reg_info2.bit.BYTE6);
		reg_info0.bit.CHSUM  = 0x100 - reg_info0.bit.CHSUM;
		HDMI_SETREG(HDMI_GENINFO_REG_OFS + HDMI_INFO0_REG_OFS, reg_info0.reg);

		hdmitx_enable_infofrm_tx(HDMIINFO_EN_GEN1, HDMIINFO_RPT_GEN1);

		vid_code = 0;
	}// No BREAK needed

	case HDMI_640X480P60:       /* VID1     640X480P    @59.94/60Hz */
	case HDMI_720X480P60_16X9:  /* VID2     720X480P    @59.94/60Hz */
	case HDMI_720X480P60:       /* VID3     720X480P    @59.94/60Hz */
	case HDMI_1280X720P60:      /* VID4     1280X720P   @59.94/60Hz */
	case HDMI_1920X1080I60:     /* VID5     1920X1080I  @59.94/60Hz */
	case HDMI_720X480I60:       /* VID6     720X480i    @59.94/60Hz */
	case HDMI_720X480I60_16X9:  /* VID7     720X480i    @59.94/60Hz */
	case HDMI_720X240P60:       /* VID8     720X240p    @59.94/60Hz */
	case HDMI_720X240P60_16X9:  /* VID9     720X240p    @59.94/60Hz */
	case HDMI_1440X480I60:      /* VID10    1440X480I   @59.94/60Hz */
	case HDMI_1440X480I60_16X9: /* VID11    1440X480I   @59.94/60Hz */
	case HDMI_1440X240P60:      /* VID12    1440X240P   @59.94/60Hz */
	case HDMI_1440X240P60_16X9: /* VID13    1440X240P   @59.94/60Hz */
	case HDMI_1440X480P60:      /* VID14    1440X480P   @59.94/60Hz */
	case HDMI_1440X480P60_16X9: /* VID15    1440X480P   @59.94/60Hz */
	case HDMI_1920X1080P60:     /* VID16    1920X1080P  @59.94/60Hz */
	case HDMI_720X576P50:       /* VID17    720X576P    @50Hz       */
	case HDMI_720X576P50_16X9:  /* VID18    720X576P    @50Hz       */
	case HDMI_1280X720P50:      /* VID19    1280X720P   @50Hz       */
	case HDMI_1920X1080I50:     /* VID20    1920X1080i  @50Hz       */
	case HDMI_720X576I50:       /* VID21    720X576I    @50Hz       */
	case HDMI_720X576I50_16X9:  /* VID22    720X576I    @50Hz       */
	case HDMI_720X288P50:       /* VID23    720X288P    @50Hz       */
	case HDMI_720X288P50_16X9:  /* VID24    720X288P    @50Hz       */
	case HDMI_1440X576I50:      /* VID25    1440X576I   @50Hz       */
	case HDMI_1440X576I50_16X9: /* VID26    1440X576I   @50Hz       */
	case HDMI_1440X288P50:      /* VID27    1440X288P   @50Hz       */
	case HDMI_1440X288P50_16X9: /* VID28    1440X288P   @50Hz       */
	case HDMI_1440X576P50:      /* VID29    1440X576P   @50Hz       */
	case HDMI_1440X576P50_16X9: /* VID30    1440X576P   @50Hz       */
	case HDMI_1920X1080P50:     /* VID31    1920X1080P  @50Hz       */
	case HDMI_1920X1080P24:     /* VID32    1920X1080P  @23.98/24Hz */
	case HDMI_1920X1080P25:     /* VID33    1920X1080P  @25Hz       */
	case HDMI_1920X1080P30:     /* VID34    1920X1080P  @29.97/30Hz */
	case HDMI_1920X1080I50_VT1250:/*VID39   1920X1080I  @50Hz       */
	case HDMI_1920X1080I100:    /* VID40    1920X1080I  @100Hz      */
	case HDMI_1280X720P100:     /* VID41    1280X720P   @100Hz      */
	case HDMI_720X576P100:      /* VID42    720X576P    @100Hz      */
	case HDMI_720X576P100_16X9: /* VID43    720X576P    @100Hz      */
	case HDMI_720X576I100:      /* VID44    720X576I    @100Hz      */
	case HDMI_720X576I100_16X9: /* VID45    720X576I    @100Hz      */
	case HDMI_1920X1080I120:    /* VID46    1920X1080I  @119.8/120Hz*/
	case HDMI_1280X720P120:     /* VID47    1280X720P   @119.8/120Hz*/
	case HDMI_720X480P120:      /* VID48    720X480P    @119.8/120Hz*/
	case HDMI_720X480P120_16X9: /* VID49    720X480P    @119.8/120Hz*/
	case HDMI_720X480I120:      /* VID50    720X480I    @119.8/120Hz*/
	case HDMI_720X480I120_16X9: /* VID51    720X480I    @119.8/120Hz*/
	case HDMI_720X576P200:      /* VID52    720X576P    @200Hz      */
	case HDMI_720X576P200_16X9: /* VID53    720X576P    @200Hz      */
	case HDMI_720X576I200:      /* VID54    720X576I    @200Hz      */
	case HDMI_720X576I200_16X9: /* VID55    720X576I    @200Hz      */
	case HDMI_720X480P240:      /* VID56    720X480P    @239.7/240Hz*/
	case HDMI_720X480P240_16X9: /* VID57    720X480P    @239.7/240Hz*/
	case HDMI_720X480I240:      /* VID58    720X480I    @239.7/240Hz*/
	case HDMI_720X480I240_16X9: /* VID59    720X480I    @239.7/240Hz*/

	case HDMI_1024X768P60:
	case HDMI_1280X1024P60:
	case HDMI_1600X1200P60:
	case HDMI_2560X1440P60:
	case HDMI_1440X900P60:
	case HDMI_1680X1050P60:
	case HDMI_1920X1200P60:

	case HDMI_3840X2160P24:     /* VID93    3840x2160P  @24Hz*/
	case HDMI_3840X2160P25:         /* VID94 */
	case HDMI_3840X2160P30:         /* VID95 */
	case HDMI_4096X2160P24:         /* VID98 */
	case HDMI_4096X2160P25:         /* VID99 */
	case HDMI_4096X2160P30: {       /* VID100*/

			if ((hdmitx_get_edid_info() & HDMITX_TV_UNDERSCAN) || (hdmi_ctrl_option[HDMICFG_OPTION_FORCE_UNDERSCAN])) {
				// Support under_scan
				under_scan = 0x2;
			} else if (hdmitx_get_edid_info()) {
				// Support OverScan
				under_scan = 0x1;
			} else {
				// EDID Parsing Fail, Use NoData
				under_scan = 0x0;
			}


			// Output Format RGB444 or YUV444 depends on EDID
			if ((hdmitx_get_edid_info()&HDMITX_TV_YCBCR444) && (hdmi_ctrl_option[HDMICFG_OPTION_FORCE_RGB] == 0)) {
				mode_fmt = (HDMI_INTF_YCBCR444 << 5);
			} else if ((hdmitx_get_edid_info()&HDMITX_TV_YCBCR422) && (hdmi_ctrl_option[HDMICFG_OPTION_FORCE_RGB] == 0)) {
				mode_fmt = (HDMI_INTF_YCBCR422 << 5);
			} else {
				mode_fmt = (HDMI_INTF_RGB444  << 5);
			}


			//
			//  AVI InfoFrame Configuration
			//

			/* Header Word */
			reg_info0.reg = 0;
			reg_info0.bit.TYPE = HDMIINFO_AVI_TYPE;
			reg_info0.bit.VERS = HDMIINFO_AVI_VER;
			reg_info0.bit.LEN  = HDMIINFO_AVI_LEN;

#ifdef __KERNEL__
			if (vid_code > 100) {
				vid_code = 0;
			}
#endif
			//reg_info0.bit.CHSUM  = (HDMIINFO_AVI_TYPE + HDMIINFO_AVI_VER + HDMIINFO_AVI_LEN + under_scan + mode_fmt + HDMIINFO_AVI_D1_SAMEASPIC_ASPECTRATIO + vid_code + pr);
			reg_info0.bit.CHSUM  = (HDMIINFO_AVI_TYPE + HDMIINFO_AVI_VER + HDMIINFO_AVI_LEN + under_scan + mode_fmt + pic_aspect_ratio + vid_code + pr);
			reg_info0.bit.CHSUM  = 0x100 - reg_info0.bit.CHSUM;
			HDMI_SETREG(HDMI_AVIINFO_REG_OFS + HDMI_INFO0_REG_OFS, reg_info0.reg);

			/* Data Word 1*/
			reg_info1.reg = 0;
			reg_info1.bit.BYTE1 = under_scan + mode_fmt;
			reg_info1.bit.BYTE2 = pic_aspect_ratio;	//HDMIINFO_AVI_D1_SAMEASPIC_ASPECTRATIO;
			reg_info1.bit.BYTE3 = 0x00;
			reg_info1.bit.BYTE4 = vid_code;
			HDMI_SETREG(HDMI_AVIINFO_REG_OFS + HDMI_INFO1_REG_OFS, reg_info1.reg);

			/* Data Word 2*/
			reg_info2.reg = 0;
			reg_info2.bit.BYTE5 = pr;
			HDMI_SETREG(HDMI_AVIINFO_REG_OFS + HDMI_INFO2_REG_OFS, reg_info2.reg);

			/* Data Word 3*/
			reg_info3.reg = 0;
			HDMI_SETREG(HDMI_AVIINFO_REG_OFS + HDMI_INFO3_REG_OFS, reg_info3.reg);

			/* Data Word 4*/
			reg_info4.reg = 0;
			HDMI_SETREG(HDMI_AVIINFO_REG_OFS + HDMI_INFO4_REG_OFS, reg_info4.reg);

			break;
		}
	default:
		DBG_ERR("No Support VID number!\r\n");
		return 1;
	}

	return 0;

}


static void hdmitx_validate_rtrim(void)
{
	#if !_EMULATION_
	//hdmitx_write_phy(0x0C, gui_trim_rsel);
	#endif
}

static void hdmitx_scdc_dump(void)
{
	UINT32 i, data = 0;

	DBG_DUMP("      0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0A  0x0B  0x0C  0x0D  0x0E  0x0F");
	for (i = 0; i < 256; i++) {
		if ((i & 0xF) == 0x0) {
			DBG_DUMP("\r\n0x%02X: ", i);
		}

		hdmitx_scdc_read(i, &data);
		DBG_DUMP("0x%02X  ", data);
	}
	DBG_DUMP("\r\n");
}

#if HDMITX_SCDC_READREQUEST
static void hdmitx_scdc_rr_callback(UINT32 ui_event)
{
#if 1
	{
		UINT8 UPDATE_0 = 0, UPDATE_1 = 0;

		i2c5_get_data(&UPDATE_0, &UPDATE_1);

		DBG_WRN("0x%08X UPDATE0=0x%02X  UPDATE1=0x%02X\r\n", ui_event, UPDATE_0, UPDATE_1);
	}
#else
	// STOP CONDITION TEST
	{
		UINT32 data1, data2;

		while (hdmitx_scdc_read(0x10, &data1) != E_OK) {
			DBG_DUMP("Q");
		}

		if (data1) {
			while (hdmitx_scdc_write(0x10, data1) != E_OK) {
				DBG_DUMP("W");
			}
		}

		while (hdmitx_scdc_read(0x10, &data2) != E_OK) {
			DBG_DUMP("E");
		}

		DBG_WRN("0x%08X R10=0x%02X  0x%02X\r\n", ui_event, data1, data2);

	}
#endif

#if !HDMITX_SCDC_RRTEST
	hdmi_i2c_obj[hdmi_i2c_select].set_config(hdmi_i2c_ses, I2C_CONFIG_ID_RR_EN,        ENABLE);
#endif

}
#endif

#if 1

/**
    Open HDMI Transmitter Driver

   Open HDMI Transmitter and then intialize the source clock / DDC / interrupt for usage.
   Besides, this would also read the TV/Display's EDID information from DDC channel and parsing the EDID.
   The user can get the EDID information after hdmitx_open() by the api hdmitx_get_edid_info().
   The driver is opened or not can be checked by the hdmitx_is_opened.
   The behavior of the re-open of the HDMI driver in under save protection in the driver design.\n
   In the system design, the DDC channel is using the I2C2 controller in normal operation.
   The user must remember opening the I2C2 pinmux in the systemInit phase.

    @return
     - @b E_OK: Done and success
*/
ER hdmitx_open(void)
{
	//UINT16                  hdmi_trim_data;
#if _FPGA_EMULATION_
	T_HDMI_TMDSCTRL1_REG    reg_tmds_ctrl1;
#endif
	T_HDMI_SYSCTRL_REG      reg_sys_ctrl;

	if (b_hdmi_tx_open == TRUE) {
		return E_OK;
	}

	//pll_disable_system_reset(HDMI_RSTN);


	hdmitx_attach();

#if _FPGA_EMULATION_
	hdmitx_reset_sil9002();
#endif

#if !_EMULATION_
	hdmitx_reset(TRUE);
#endif
	hdmitx_reset(FALSE);


	hdmitx_init_phy();

	// Enable Digital Logic to Validate interrupt status
	hdmitx_set_power_down(FALSE);
#if 0
	// Get HDMI eFuse resist-trimming value
	if (efuse_read_param_ops(EFUSE_HDMI_TRIM_DATA, &hdmi_trim_data) >= 0) {
		gui_trim_rsel = hdmi_trim_data & HDMITX_RTRIM_EFUSE_MASK;
	} else {
		gui_trim_rsel = HDMITX_RTRIM_DEFAULT;
	}
#endif
	// Set Calibrated resistor value
	hdmitx_validate_rtrim();

#if 1//!_FPGA_EMULATION_
	// TC72670B HDMI PHY Bug. The RSEN would couple clock signal and keep toggling.
	// The root cause the connect to wrong power source.
	// So we disable RSEN interrupt when using TC72670B
	hdmitx_set_int_en(RSEN | ACR_OVR,  ENABLE);
#endif

	hdmitx_enable_infofrm_tx(0, HDMIINFO_RPT_AVI | HDMIINFO_RPT_AUD | HDMIINFO_RPT_GCP | HDMIINFO_RPT_GEN1 | HDMIINFO_RPT_GEN2);


	//
	//  Enable HDMI interrupt after Tx starts
	//
	gui_hdmi_int_sts = 0;
	drv_enable_int(DRV_INT_HDMI);

#if !(HDMITX_DEBUG && HDMITX_DEBUG_SKIP_EDID)

	if (b_hdmi_i2c_opened == FALSE) {
		if (hdmi_i2c_obj[hdmi_i2c_select].open(&hdmi_i2c_ses) == E_OK) {
			b_hdmi_i2c_opened = TRUE;
		} else {
			b_hdmi_i2c_opened = FALSE;
		}
	}
#endif

	hdmitx_init_ddc(b_hdmi_i2c_opened);

	hdmitx_set_config(HDMI_CONFIG_ID_3D_FMT, HDMI3D_NONE);
	hdmitx_set_config(HDMI_CONFIG_ID_AV_MUTE, FALSE);

#if _FPGA_EMULATION_
	// PHY Default Config Values
	reg_tmds_ctrl1.reg = HDMI_GETREG(HDMI_TMDSCTRL1_REG_OFS);
	reg_tmds_ctrl1.bit.SAFE_START = 200;//default value is 0x1D0
	HDMI_SETREG(HDMI_TMDSCTRL1_REG_OFS, reg_tmds_ctrl1.reg);
#endif

	// Default as HDMI 2.0
	reg_sys_ctrl.reg = HDMI_GETREG(HDMI_SYSCTRL_REG_OFS);
	reg_sys_ctrl.bit.HDMI2      = 1;
	HDMI_SETREG(HDMI_SYSCTRL_REG_OFS, reg_sys_ctrl.reg);

	b_hdmi_tx_open = TRUE;
	return E_OK;
}

/**
    Check the HDMI-Tx driver is opened or not

    Check the HDMI-Tx driver is opened or not

    @return
     - @b TRUE:  Already opened.
     - @b FALSE: Have not opened yet.
*/
BOOL hdmitx_is_opened(void)
{
	return b_hdmi_tx_open;
}

/**
    Close the HDMI-Tx module

    Close the HDMI-Tx module, this api would power down the hdmi-tx module and also close the I2C2(DDC) usage.

    @return void
     - @b E_OK: Done and success
*/
ER  hdmitx_close(void)
{
	if (b_hdmi_tx_open == FALSE) {
		return E_OK;
	}

	// Enter critical section
	//loc_cpu();

	drv_disable_int(DRV_INT_HDMI);

	// Close HDMI Digital Clock & DDC-Pinmux
	hdmitx_detach();

	// Power Down HDMI PHY Logic
	hdmitx_set_power_down(TRUE);

	hdmitx_set_int_en(HDMI_INTALL, DISABLE);

	// Disable HDMI Audio IN/OUT Stream
	hdmitx_set_config(HDMI_CONFIG_ID_AUDIO_STREAM_EN, FALSE);
	hdmitx_set_tmds_oe(DISABLE);
	hdmitx_set_config(HDMI_CONFIG_ID_3D_FMT, HDMI3D_NONE);



	if (b_hdmi_i2c_opened) {
		if (hdmi_i2c_obj[hdmi_i2c_select].close(hdmi_i2c_ses) == E_OK) {
			b_hdmi_i2c_opened = FALSE;
		} else {
			DBG_ERR("DDC close fail\r\n");
		}
	}

	b_hdmi_tx_open = FALSE;
	return E_OK;
}


/**
    Check hotplug or not

    This is used to check if the HDMI cable is plugged or not.

    @return
     - @b TRUE:  Hotplug attached.
     - @b FALSE: Hotplug detached.
*/
BOOL hdmitx_check_hotplug(void)
{
	T_HDMI_SYSCTRL_REG  reg_sys_ctrl;
	
	reg_sys_ctrl.reg = HDMI_GETREG(HDMI_SYSCTRL_REG_OFS);
	
	return reg_sys_ctrl.bit.HPD;
}

/**
    Check if HDMI's video Clock is stable or not

    This api is used in the HDMI display device object to check if the hdmi input video clock is stable or not after
    opening the IDE Pixel clock. If the none of the video clock or the clock is not stable, this api would output the
    warning message "HDMI Video Clock Not Stable! Timeout!!" on console.

    @return void
*/
void hdmitx_check_video_stable(void)
{
	T_HDMI_SYSCTRL_REG  reg_sys_ctrl;
	UINT32              count;

	count = HDMITX_CHECK_TIMEOUT;
	do {
		count--;
		reg_sys_ctrl.reg = HDMI_GETREG(HDMI_SYSCTRL_REG_OFS);
	} while ((reg_sys_ctrl.bit.PIXCLK_STABLE == 0x0) && count);

	if (count == 0) {
		DBG_WRN("HDMI Clk No Stable!Timeout!\r\n");
	}
}
#endif

/**
    Set HDMI functionality configuration

    This is used to configure the main functions of the HDMI controller, including the Video/Audio formats, output-enable,
    video-source, 3D-format, ...etc. Please reference to the "HDMI_CONFIG_ID" and the application note for the control function details.

    @param[in] config_id     Control function selection ID.
    @param[in] ui_config     Please reference to the "HDMI_CONFIG_ID" and the application note for the control function parameter details.

    @return void
*/
void hdmitx_set_config(HDMI_CONFIG_ID config_id, UINT32 ui_config)
{

	/* Would Not Blocking by the b_hdmi_tx_open */
	switch (config_id) {
	case HDMI_CONFIG_ID_EDIDMSG: {
			b_hdmi_edid_msg = ui_config;
		}
		return;

	case HDMI_CONFIG_ID_AUDIO_MUTE: {
			T_HDMI_SYSCTRL_REG reg_sys_ctrl;

			reg_sys_ctrl.reg = HDMI_GETREG(HDMI_SYSCTRL_REG_OFS);
			reg_sys_ctrl.bit.AUD_MUTE = (ui_config > 0);
			HDMI_SETREG(HDMI_SYSCTRL_REG_OFS, reg_sys_ctrl.reg);
		}
		return;

	case HDMI_CONFIG_ID_SOURCE: {
			T_HDMI_TOP_REG reg_ctrl;

			reg_ctrl.reg = HDMI_GETREG(HDMI_TOP_REG_OFS);
			reg_ctrl.bit.LCD_SRC_SEL = ui_config;
			HDMI_SETREG(HDMI_TOP_REG_OFS, reg_ctrl.reg);
		}
		return;


	/* Control Options */
	case HDMI_CONFIG_ID_FORCE_RGB: {
			hdmi_ctrl_option[HDMICFG_OPTION_FORCE_RGB]        = (ui_config > 0);
		}
		return;

	case HDMI_CONFIG_ID_FORCE_UNDERSCAN: {
			hdmi_ctrl_option[HDMICFG_OPTION_FORCE_UNDERSCAN]  = (ui_config > 0);
		}
		return;

	case HDMI_CONFIG_ID_VERIFY_VIDEOID: {
			hdmi_ctrl_option[HDMICFG_OPTION_VERIFY_VID]       = (ui_config > 0);
		}
		return;

	case HDMI_CONFIG_ID_HWDDC: {
			hdmi_i2c_select = ui_config;
		}
		return;

	case HDMI_CONFIG_ID_SWDDC_SCL: {
			hdmi_sw_ddc_scl  = ui_config;
		}
		return;

	case HDMI_CONFIG_ID_SWDDC_SDA: {
			hdmi_sw_ddc_sda  = ui_config;
		}
		return;

	case HDMI_CONFIG_ID_TRIM_RSEL: {
			// This config is only valid for emulation.
			// In normal mode, guiTrimRsel should comes from eFuse.
#if _EMULATION_
			gui_trim_rsel = ui_config;

			if (b_hdmi_tx_open) {
				// Set Calibrated resistor value
				hdmitx_validate_rtrim();
			}
#endif
		}
		return;

	default: {
			break;
		}

	}



	if (b_hdmi_tx_open == FALSE) {
		return;
	}

	switch (config_id) {
	case HDMI_CONFIG_ID_OE: {
			T_HDMI_CTRL_REG         reg_ctrl;

			if (ui_config) {
				// If EDID Read fail, default act as HDMI device.
				reg_ctrl.reg = HDMI_GETREG(HDMI_CTRL_REG_OFS);
#if !HDMITX_DEBUG_FORCE_DVI
				if ((hdmitx_get_edid_info()&HDMITX_TV_HDMI) || (hdmitx_get_edid_info() == 0x0)) {
					reg_ctrl.bit.HDMI_MODE = 1;//HDMI Mode
				} else
#endif
				{
					reg_ctrl.bit.HDMI_MODE = 0;//DVI Mode
				}

				HDMI_SETREG(HDMI_CTRL_REG_OFS, reg_ctrl.reg);
				hdmitx_enable_infofrm_tx(HDMIINFO_EN_AVI | HDMIINFO_EN_AUD | HDMIINFO_EN_GCP, HDMIINFO_RPT_AVI | HDMIINFO_RPT_AUD | HDMIINFO_RPT_GCP);
			}

			hdmitx_set_tmds_oe((ui_config > 0));
		}
		break;

	case HDMI_CONFIG_ID_SCDC_ENABLE: {
#if HDMITX_SCDC_SUPPORT
			if (ui_config > 0) {
				// SCDC Cap
				if (hdmitx_get_hdmi_2_vsdb_cap() & HDMI2VSDB_SCDC_PRESENT) {
					UINT32 data;

					if (hdmitx_scdc_read(0x01, &data) != E_OK) {
						DBG_ERR("Read SCDC Sink Version failed\r\n");
						return;
					}

					if (data != HDMI2_SCDC_VERSION) {
						DBG_WRN("Sink's SCDC version not matched.(%d)\r\n", data);
					} else {
						hdmitx_debug(("SCDC Enable!\r\n"));
					}

					if (hdmitx_scdc_write(0x02, HDMI2_SCDC_VERSION) != E_OK) {
						DBG_ERR("Write SCDC Src Version failed\r\n");
						return;
					}

#if HDMITX_SCDC_READREQUEST
					// RR Cap
					if (hdmitx_get_hdmi_2_vsdb_cap() & HDMI2VSDB_RR_CAP) {
						// Set SCDC RR Enable
						hdmitx_scdc_write(0x30, 0x01);

						hdmi_i2c_obj[hdmi_i2c_select].set_config(hdmi_i2c_ses, I2C_CONFIG_ID_RR_SAR,       HDMI2_SCDC_SLAVEADDR >> 1);
						hdmi_i2c_obj[hdmi_i2c_select].set_config(hdmi_i2c_ses, I2C_CONFIG_ID_RR_MODE,      I2C_RR_MODE_READ_UPDATE_FLAG);
						hdmi_i2c_obj[hdmi_i2c_select].set_config(hdmi_i2c_ses, I2C_CONFIG_ID_RR_CALLBACK, (UINT32)hdmitx_scdc_rr_callback);

#if HDMITX_SCDC_RRTEST
						if (SwTimer_Open(&g_hdmi_sw_timer_id, hdmitx_read_request_sw_timer_cb) == E_OK) {
							SwTimer_Cfg(g_hdmi_sw_timer_id, 2000, SWTIMER_MODE_FREE_RUN);
							SwTimer_Start(g_hdmi_sw_timer_id);
						}
						DBG_DUMP("RR TEST RR\r\n");
#else
						hdmi_i2c_obj[hdmi_i2c_select].set_config(hdmi_i2c_ses, I2C_CONFIG_ID_RR_EN,        ENABLE);
#endif
					}
#endif
				}

			} else {
				hdmi_i2c_obj[hdmi_i2c_select].set_config(hdmi_i2c_ses, I2C_CONFIG_ID_RR_EN,                DISABLE);
				hdmi_i2c_obj[hdmi_i2c_select].set_config(hdmi_i2c_ses, I2C_CONFIG_ID_RR_CALLBACK,          0);
			}
#endif
		}
		break;

	case HDMI_CONFIG_ID_VIDEO: {
			hdmitx_config_video(ui_config);
		}
		break;

	case HDMI_CONFIG_ID_AUDIO: {
			hdmitx_config_audio(ui_config);
		}
		break;

	case HDMI_CONFIG_ID_AV_MUTE: {
			T_HDMI_GENCTRL_REG  reg_gcp_ctrl;
			T_HDMI_CTRL_REG     reg_ctrl;
			UINT32              count;

			reg_gcp_ctrl.reg = 0;
			if (ui_config > 0) {
				reg_gcp_ctrl.bit.SETAVM = 1;
			} else {
				reg_gcp_ctrl.bit.CLRAVM = 1;
			}
			HDMI_SETREG(HDMI_GENCTRL_REG_OFS, reg_gcp_ctrl.reg);


			count = HDMITX_CHECK_TIMEOUT;
			do {
				count--;
				reg_ctrl.reg = HDMI_GETREG(HDMI_CTRL_REG_OFS);
			} while ((reg_ctrl.bit.GCP_MUTE_STS == reg_gcp_ctrl.bit.CLRAVM) && count);

			if (count == 0) {
				DBG_WRN("HDMI MUTE STS!Timeout!\r\n");
			}
		}
		break;

	case HDMI_CONFIG_ID_AUDIO_STREAM_EN: {
			T_HDMI_AUDCTRL_REG  reg_aud_ctrl;

			reg_aud_ctrl.reg = HDMI_GETREG(HDMI_AUDCTRL_REG_OFS);
			reg_aud_ctrl.bit.AUD_EN = (ui_config > 0);
			HDMI_SETREG(HDMI_AUDCTRL_REG_OFS, reg_aud_ctrl.reg);
		}
		break;

	case HDMI_CONFIG_ID_3D_FMT: {
			if (ui_config > (UINT32)HDMI3D_TOPANDBOTTOM) {
				break;
			}

			if (ui_config) {
				T_HDMI_INFO0_REG reg_info0;
				T_HDMI_INFO1_REG reg_info1;
				T_HDMI_INFO2_REG reg_info2;

				/* Header Word */
				reg_info0.reg = 0;
				reg_info0.bit.TYPE = HDMIINFO_VSI_TYPE;
				reg_info0.bit.VERS = HDMIINFO_VSI_VER;

				/* Data Word 1*/
				reg_info1.reg = 0;
				reg_info1.bit.BYTE1 = HDMIINFO_VSI_DB1;
				reg_info1.bit.BYTE2 = HDMIINFO_VSI_DB2;
				reg_info1.bit.BYTE3 = HDMIINFO_VSI_DB3;
				reg_info1.bit.BYTE4 = HDMIINFO_VSI_3D_PRESENT;
				HDMI_SETREG(HDMI_GENINFO_REG_OFS + HDMI_INFO1_REG_OFS, reg_info1.reg);

				/* Data Word 2*/
				reg_info2.reg = 0;
				if (ui_config == (UINT32) HDMI3D_SIDEBYSIDE_HALF) {
					reg_info0.bit.LEN   = 0x06;
					reg_info2.bit.BYTE5 = HDMIINFO_VSI_SIDEBYSIDE_HALF;
					reg_info2.bit.BYTE6 = HDMIINFO_VSI_HORI_SUBSAMPLE;
				} else if (ui_config == (UINT32) HDMI3D_TOPANDBOTTOM) {
					reg_info0.bit.LEN   = 0x05;
					reg_info2.bit.BYTE5 = HDMIINFO_VSI_TOP_N_BOTTOM;
					reg_info2.bit.BYTE6 = 0x00;
				}
				reg_info2.bit.BYTE7 = 0x00;
				reg_info2.bit.BYTE8 = 0x00;
				HDMI_SETREG(HDMI_GENINFO_REG_OFS + HDMI_INFO2_REG_OFS, reg_info2.reg);

				reg_info0.bit.CHSUM  = (UINT8)(HDMIINFO_VSI_TYPE + HDMIINFO_VSI_VER + reg_info0.bit.LEN + HDMIINFO_VSI_DB1 + HDMIINFO_VSI_DB2 + HDMIINFO_VSI_DB3 + HDMIINFO_VSI_3D_PRESENT + reg_info2.bit.BYTE5 + reg_info2.bit.BYTE6);
				reg_info0.bit.CHSUM  = 0x100 - reg_info0.bit.CHSUM;
				HDMI_SETREG(HDMI_GENINFO_REG_OFS + HDMI_INFO0_REG_OFS, reg_info0.reg);

				hdmitx_enable_infofrm_tx(HDMIINFO_EN_GEN1, HDMIINFO_RPT_GEN1);
			} else {
				hdmitx_disable_infofrm_tx(HDMIINFO_EN_GEN1, HDMIINFO_RPT_GEN1);
			}
		}
		break;

	case HDMI_CONFIG_ID_SWING_CTRL:
	case HDMI_CONFIG_ID_BGR_CTRL:
		break;

	case HDMI_CONFIG_ID_VSYNC_INVERT: {
			T_HDMI_SYSCTRL_REG reg_sys_ctrl;

			if (ui_config > 0) {
				// SSCP counting from VSYNC Rising Edge
				hdmitx_write_phy(0x35, 0x00);
			} else {
				// SSCP counting from VSYNC Falling Edge
				hdmitx_write_phy(0x35, 0x01);
			}

			reg_sys_ctrl.reg = HDMI_GETREG(HDMI_SYSCTRL_REG_OFS);
			reg_sys_ctrl.bit.VD_INV = (ui_config > 0);
			HDMI_SETREG(HDMI_SYSCTRL_REG_OFS, reg_sys_ctrl.reg);
		}
		break;
	case HDMI_CONFIG_ID_HSYNC_INVERT: {
			T_HDMI_SYSCTRL_REG reg_sys_ctrl;

			reg_sys_ctrl.reg = HDMI_GETREG(HDMI_SYSCTRL_REG_OFS);
			reg_sys_ctrl.bit.HD_INV = (ui_config > 0);
			HDMI_SETREG(HDMI_SYSCTRL_REG_OFS, reg_sys_ctrl.reg);
		}
		break;
	case HDMI_CONFIG_ID_COMPONENT0: {
			T_HDMI_SYSCTRL_REG reg_sys_ctrl;

			reg_sys_ctrl.reg = HDMI_GETREG(HDMI_SYSCTRL_REG_OFS);
			reg_sys_ctrl.bit.COMP0_SEL = ui_config;
			HDMI_SETREG(HDMI_SYSCTRL_REG_OFS, reg_sys_ctrl.reg);
		}
		break;
	case HDMI_CONFIG_ID_COMPONENT1: {
			T_HDMI_SYSCTRL_REG reg_sys_ctrl;

			reg_sys_ctrl.reg = HDMI_GETREG(HDMI_SYSCTRL_REG_OFS);
			reg_sys_ctrl.bit.COMP1_SEL = ui_config;
			HDMI_SETREG(HDMI_SYSCTRL_REG_OFS, reg_sys_ctrl.reg);
		}
		break;
	case HDMI_CONFIG_ID_COMPONENT2: {
			T_HDMI_SYSCTRL_REG reg_sys_ctrl;

			reg_sys_ctrl.reg = HDMI_GETREG(HDMI_SYSCTRL_REG_OFS);
			reg_sys_ctrl.bit.COMP2_SEL = ui_config;
			HDMI_SETREG(HDMI_SYSCTRL_REG_OFS, reg_sys_ctrl.reg);
		}
		break;

	case HDMI_CONFIG_ID_SCRAMBLE_EN: {
			UINT32 value = 0, value2 = 0;

			if (!(hdmitx_get_hdmi_2_vsdb_cap() & HDMI2VSDB_SCDC_PRESENT)) {
				DBG_ERR("SCDC not support. SCRAMBLE_EN failed\r\n");
				return;
			}

			hdmitx_scdc_read(0x20,  &value2);

#if _FPGA_EMULATION_
			hdmitx_write_phy(0xFF, PHY_PAGE);
#endif
			hdmitx_read_phy(0x30,  &value);

			if (ui_config) {
#if _FPGA_EMULATION_
				//TC72670B Bug: SSCP Delay count to 255
				hdmitx_write_phy(0x32, 0xFF);
#else
				hdmitx_write_phy(0x34, 0x02);//MSb
				hdmitx_write_phy(0x33, 0x20);//LSb
#endif

				// Enable PHY Scramble
				hdmitx_write_phy(0x30,  value | 0x04);

				// Enable SCDC Scramble
				hdmitx_scdc_write(0x20, value2 | 0x01);
			} else {
				// Disable PHY Scramble
				hdmitx_write_phy(0x30, (value  & (~0x04)));

				// Disable SCDC Scramble
				hdmitx_scdc_write(0x20, (value2 & (~0x01)));
			}
		}
		break;
	case HDMI_CONFIG_ID_PIXCLK_DIVIDE4: {
			UINT32 value = 0;

#if _FPGA_EMULATION_
			hdmitx_write_phy(0xFF, PHY_PAGE);//page9
#endif
			hdmitx_read_phy(0x30,  &value);

			if (ui_config) {
				// Enable PHY HDMI2.0 and Divide by 4
				hdmitx_write_phy(0x30,  value | 0x06);

				// Enable SCDC Scramble and Divide by 4
				hdmitx_scdc_write(0x20, 0x03);
			} else {
				// Disable PHY HDMI2.0 and Divide by 4
				hdmitx_write_phy(0x30, (value  & (~0x06)));

				// Disable SCDC Scramble and Divide by 4
				hdmitx_scdc_write(0x20, 0x00);
			}
		}
		break;

	case HDMI_CONFIG_ID_SCDC_DUMP: {
			if (hdmitx_get_hdmi_2_vsdb_cap() & HDMI2VSDB_SCDC_PRESENT) {
				hdmitx_scdc_dump();
			}
		}
		break;
	case HDMI_CONFIG_ID_SCDC_WRITE: {
			DBG_DUMP("SCDC WRITE R[0x%02X]=0x%02X\r\n", (ui_config >> 8) & 0xFF, ui_config & 0xFF);
			hdmitx_scdc_write((ui_config >> 8) & 0xFF, ui_config & 0xFF);
		}
		break;

	default:
		DBG_WRN("Set cfg ID err\r\n");
		break;

	}

}

/**
    Get HDMI functionality configuration

    Use this api to get the current HDMI function configuration.

    @param[in] config_id     Get which Control function configuration selection.

    @return Please reference to the "HDMI_CONFIG_ID" for the control function parameter details.
*/
UINT32 hdmitx_get_config(HDMI_CONFIG_ID config_id)
{
	UINT32 ret = 0;

	switch (config_id) {
	case HDMI_CONFIG_ID_OE: {
			UINT32 value = 0;

			if (b_hdmi_tx_open) {
				hdmitx_read_phy(0x09, &value);
			}
			ret = value & 0x1;
		}
		break;

	case HDMI_CONFIG_ID_VIDEO: {
			T_HDMI_INFO1_REG        reg_info1;

			reg_info1.reg = HDMI_GETREG(HDMI_AVIINFO_REG_OFS + HDMI_INFO1_REG_OFS);
			ret = reg_info1.bit.BYTE4;
		}
		break;

	case HDMI_CONFIG_ID_AUDIO: {
			T_HDMI_PCMHDR0_REG  reg_phdr0;

			reg_phdr0.reg = HDMI_GETREG(HDMI_PCMHDR0_REG_OFS);

			if (reg_phdr0.bit.FS == HDMI_PCM_FS48KHZ) {
				ret = HDMI_AUDIO48KHZ;
			} else if (reg_phdr0.bit.FS == HDMI_PCM_FS32KHZ) {
				ret = HDMI_AUDIO32KHZ;
			} else {
				ret = HDMI_AUDIO44_1KHZ;
			}
		}
		break;

	case HDMI_CONFIG_ID_AV_MUTE: {
			T_HDMI_CTRL_REG     reg_ctrl;

			reg_ctrl.reg = HDMI_GETREG(HDMI_CTRL_REG_OFS);
			ret = reg_ctrl.bit.GCP_MUTE_STS;
		}
		break;

	case HDMI_CONFIG_ID_SOURCE: {
			T_HDMI_TOP_REG reg_ctrl;

			reg_ctrl.reg = HDMI_GETREG(HDMI_TOP_REG_OFS);
			ret = reg_ctrl.bit.LCD_SRC_SEL;
		}
		break;


	/* Control Options */

	case HDMI_CONFIG_ID_FORCE_RGB: {
			ret = hdmi_ctrl_option[HDMICFG_OPTION_FORCE_RGB];
		}
		break;

	case HDMI_CONFIG_ID_FORCE_UNDERSCAN: {
			ret = hdmi_ctrl_option[HDMICFG_OPTION_FORCE_UNDERSCAN];
		}
		break;

	case HDMI_CONFIG_ID_VERIFY_VIDEOID: {
			ret = hdmi_ctrl_option[HDMICFG_OPTION_VERIFY_VID];
		}
		break;

	case HDMI_CONFIG_ID_MANUFACTURER_ID: {
			ret = gui_manufacturer_id;
		}
		break;

	case HDMI_CONFIG_ID_SWING_CTRL: {




		}
		break;

	case HDMI_CONFIG_ID_SEL_BGR: {




		}
		break;

	case HDMI_CONFIG_ID_REG_BGR_EN: {
			//Ret = HDMI_GETREG(HDMI_TTXCFG0D_REG_OFS) & 0x1;
		}
		break;

	case HDMI_CONFIG_ID_BGR_CTRL: {




		}
		break;

	case HDMI_CONFIG_ID_BGR_TEST: {




		}
		break;

	case HDMI_CONFIG_ID_CLKDETECT: {




		}
		break;

	case HDMI_CONFIG_ID_PHY_PDB: {




		}
		break;

	case HDMI_CONFIG_ID_TERM_EN: {




		}
		break;

	case HDMI_CONFIG_ID_BW_CTRL: {




		}
		break;

	case HDMI_CONFIG_ID_CKSW_CTRL: {



		}
		break;

	case HDMI_CONFIG_ID_PHYREG_PD: {
			//T_HDMI_TTXCFG0A_REG      RegTxCfgA;
			//RegTxCfgA.reg    = HDMI_GETREG(HDMI_TTXCFG0A_REG_OFS);
			//ret = RegTxCfgA.bit.REG_PD;
		}
		break;

	case HDMI_CONFIG_ID_RXSENSE: {
			T_HDMI_SYSCTRL_REG   reg_sys_ctrl;

			reg_sys_ctrl.reg = HDMI_GETREG(HDMI_SYSCTRL_REG_OFS);
			ret = reg_sys_ctrl.bit.RSEN;
		}
		break;

	default:
		break;

	}


	return ret;
}

#if 0

void hdmitx_start_trimming(void)
{
	//T_HDMI_TTXCFG06_REG RegPhy06;
	//T_HDMI_TTXCFG09_REG RegPhy09;
	//T_HDMI_TTXCFG34_REG RegPhy34;

	//pmc_turnonPower(PMC_MODULE_HDMI);

	// Disable IDE Clock
	pll_disable_clock(IDE1_CLK);

	// Enable HDMI CLK
	pll_enable_clock(HDMI_CLK);

	// Set IDE Clock Source as PLL2 and PLL as 216MHz
	pll_set_pll_en(PLL_ID_2, DISABLE);
	pll_set_pll(PLL_ID_2,   0x240000); // 216MHz
	pll_set_clock_rate(PLL_CLKSEL_IDE_CLKSRC, PLL_CLKSEL_IDE_CLKSRC_PLL2);
	pll_set_pll_en(PLL_ID_2, ENABLE);

	// Set IDE Pixel Clock as 27MHz
	pll_set_clock_freq(IDECLK_FREQ, 27000000 * 2);
	pll_enable_clock(IDE1_CLK);

	// Delay 10us after pixel clock enabled
	DELAY_US(10);



	// Step0: Clear manual
	//HDMI_SETREG(HDMI_TTXCFG38_REG_OFS, 0x00);

	// Step1: Set reg_pdb_mode/reg_oe_mode as register control 0x09[0][2]
	//RegPhy09.reg = HDMI_GETREG(HDMI_TTXCFG09_REG_OFS);
	//RegPhy09.bit.REG_PDB_M = 1;
	//RegPhy09.bit.REG_OE_M  = 1;
	//HDMI_SETREG(HDMI_TTXCFG09_REG_OFS, RegPhy09.reg);

	// Step2: Set 0x34[6:3] to 0xF
	//RegPhy34.reg = 0xC0; // default
	//HDMI_SETREG(HDMI_TTXCFG34_REG_OFS, RegPhy34.reg);
	//RegPhy34.bit.REG_FT_RTRIM_EN        = 1;
	//RegPhy34.bit.REG_FT_RTRIM_EXT_EN    = 1;
	//RegPhy34.bit.REG_FT_RTRIM_OP_EN     = 1;
	//RegPhy34.bit.REG_RESTRIM_EN         = 1;
	//HDMI_SETREG(HDMI_TTXCFG34_REG_OFS, RegPhy34.reg);

	// Step3: reg_oe = 0    0x06[2]
	//RegPhy06.reg = HDMI_GETREG(HDMI_TTXCFG06_REG_OFS);
	//RegPhy06.bit.REG_OE = 0;
	//HDMI_SETREG(HDMI_TTXCFG06_REG_OFS, RegPhy06.reg);


	// Bollow the variable gui_manufacturer_id for timing control
	gui_manufacturer_id = PERF_GET_CURRENT();//Perf_GetCurrent

}

UINT32 hdmitx_get_trimming_data(void)
{
	UINT32 ret = 0xFFFFFFFF;
	//T_HDMI_TTXCFG34_REG RegPhy34;
	//T_HDMI_TTXCFG35_REG RegPhy35;

	// Delay at least 10ms
	if ((PERF_GET_CURRENT() - gui_manufacturer_id) < 10000) {
		DBG_DUMP("hdmitrim use delay %d\r\n", (10000 - (PERF_GET_CURRENT() - gui_manufacturer_id)));

		while ((PERF_GET_CURRENT() - gui_manufacturer_id) < 10000) {
			DELAY_US(50);
		}
	}

	// Step4: Set trim_trig 0x34[0]
	//RegPhy34.reg = HDMI_GETREG(HDMI_TTXCFG34_REG_OFS);
	//RegPhy34.bit.TRIM_TRIG = 1;
	//HDMI_SETREG(HDMI_TTXCFG34_REG_OFS, RegPhy34.reg);

	// Delay at least 32us to get result
	DELAY_US(50);

	//RegPhy34.reg = HDMI_GETREG(HDMI_TTXCFG34_REG_OFS);
	//if(RegPhy34.bit.TRIM_ERR_FLAG)
	//{
	//    DBG_ERR("Trimmimg Error.\r\n");
	//}
	//else if (RegPhy34.bit.TRIM_DONE_FLAG)
	//{
	//    RegPhy35.reg = HDMI_GETREG(HDMI_TTXCFG35_REG_OFS);
	//    ret = RegPhy35.bit.TRIM_RSEL;
	//}
	//else
	//{
	//    DBG_ERR("No Result?\r\n");
	//}

	pll_disable_clock(HDMI_CLK);
	pll_disable_clock(IDE1_CLK);

	return ret;
}
#endif

#if _FPGA_EMULATION_
/*
    HDMI Pattern Gen Enable
*/
void hdmitx_set_pg_enable(BOOL b_en)
{
	T_HDMI_PG0_REG  reg_pg0;

	reg_pg0.reg = HDMI_GETREG(HDMI_PG0_REG_OFS);
	reg_pg0.bit.EN       = b_en;
	reg_pg0.bit.UPDATE   = 1;
	HDMI_SETREG(HDMI_PG0_REG_OFS, reg_pg0.reg);
}

/*
    HDMI Pattern Gen Test Pattern Selection
*/
void hdmitx_set_pg_pattern(UINT32 ui_pat_id)
{
	T_HDMI_PG0_REG  reg_pg0;

	reg_pg0.reg = HDMI_GETREG(HDMI_PG0_REG_OFS);
	reg_pg0.bit.PG_TYPE = ui_pat_id;
	HDMI_SETREG(HDMI_PG0_REG_OFS, reg_pg0.reg);
}

/*
    Set HDMI Pattern Gen Register 1
*/
void hdmitx_set_pg_1(UINT32 gray, UINT32 HTOT)
{
	T_HDMI_PG1_REG  reg_pg1;

	reg_pg1.reg          = 0;
	reg_pg1.bit.GRAY     = gray;
	reg_pg1.bit.SG_HTOT  = HTOT;
	HDMI_SETREG(HDMI_PG1_REG_OFS, reg_pg1.reg);
}

/*
    Set HDMI Pattern Gen Register 2
*/
void hdmitx_set_pg_2(UINT32 HSW, UINT32 HAB)
{
	T_HDMI_PG2_REG  reg_pg2;

	reg_pg2.reg          = 0;
	reg_pg2.bit.SG_HSW   = HSW;
	reg_pg2.bit.SG_HAB   = HAB;
	HDMI_SETREG(HDMI_PG2_REG_OFS, reg_pg2.reg);
}

/*
    Set HDMI Pattern Gen Register 3
*/
void hdmitx_set_pg_3(UINT32 HAW, UINT32 VTOT)
{
	T_HDMI_PG3_REG  reg_pg3;

	reg_pg3.reg          = 0;
	reg_pg3.bit.SG_HAW   = HAW;
	reg_pg3.bit.SG_VTOT   = VTOT;
	HDMI_SETREG(HDMI_PG3_REG_OFS, reg_pg3.reg);
}

/*
    Set HDMI Pattern Gen Register 4
*/
void hdmitx_set_pg_4(UINT32 VSW, UINT32 VAB)
{
	T_HDMI_PG4_REG  reg_pg4;

	reg_pg4.reg          = 0;
	reg_pg4.bit.SG_VSW   = VSW;
	reg_pg4.bit.SG_VAB   = VAB;
	HDMI_SETREG(HDMI_PG4_REG_OFS, reg_pg4.reg);
}

/*
    Set HDMI Pattern Gen Register 5
*/
void hdmitx_set_pg_5(UINT32 VAW)
{
	T_HDMI_PG5_REG  reg_pg5;

	reg_pg5.reg          = 0;
	reg_pg5.bit.SG_VAW   = VAW;
	HDMI_SETREG(HDMI_PG5_REG_OFS, reg_pg5.reg);
}

/*
    Reset FPGA external PHY SIL9002
*/
void hdmitx_reset_sil9002(void)
{
	T_HDMI_DEBUG_REG    reg_debug;

	reg_debug.reg = HDMI_GETREG(HDMI_DEBUG_REG_OFS);
	reg_debug.bit.SIL9002_PD       = 0;
	reg_debug.bit.SIL9002_ISEL     = 0;
	HDMI_SETREG(HDMI_DEBUG_REG_OFS, reg_debug.reg);
	DELAY_US(10000);
	reg_debug.bit.SIL9002_PD       = 1;
	HDMI_SETREG(HDMI_DEBUG_REG_OFS, reg_debug.reg);
}
#endif

//@}
