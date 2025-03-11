/*
    HDMI TX module CEC Driver

    HDMI TX module CEC Driver.

    @file       hdmitx_cec.c
    @ingroup    mIDrvDisp_HDMITx
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2012.  All rights reserved.
*/
#include "hdmi_port.h"
#include "hdmitx_int.h"
#include "hdmitx_reg.h"
#ifndef __KERNEL__
#include "ist.h"
#endif

#ifdef __KERNEL__
#define loc_cpu(x)	if (x) {}
#define unl_cpu(x)	if (x) {}
#endif /* LINUX */
static BOOL                 bhdmi_cec_open    = FALSE;
static UINT32               ui_physical_addr  = 0x1000;


#ifndef __KERNEL__
#if (DRV_SUPPORT_IST == ENABLE)
#else
extern DRV_CB               p_cec_cb_func;
#endif
#endif

#if 1

/**
    Open Consumer Electronics Control (CEC) controller Driver

    The user must open HDMI controller by the api hdmitx_open() first.
    This api would open the CEC controller needed operating source clock and intialize
    the interrupt for cec usage.

    @return
     -@b E_OK:  Done and Success.
     -@b E_SYS: HDMI is not opened yet! CEC can be opened after HDMI is opened only.
*/
ER hdmicec_open(void)
{
	T_CEC_CTRL_REG  reg_ctrl;
	unsigned long      flags;

	if (bhdmi_cec_open == TRUE) {
		return E_OK;
	}

#if _FPGA_EMULATION_
	if (!hdmitx_is_opened()) {
		hdmitx_open();
		DBG_DUMP("Open hdmi together during FPGA Test!\r\n");
	}
#else
	// The CEC Controller can only be opened after hdmi driver opened in real application.
	// Besides, the CEC used interrupt is configured in hdmi_open() too.
	if (!hdmitx_is_opened()) {
		DBG_ERR("Pls open hdmi first\r\n");
		return E_SYS;
	}
#endif

	// Enable CEC Clock Source: set to 100Khz
#if _FPGA_EMULATION_
	// In FPGA CEC Clock is connected to 40MHz. Not 3MHz.
	// Set 0x2C for Src27MHz; Set 0x27 for Src24MHz
	pll_set_clock_rate(PLL_CLKSEL_CEC_CLKDIV, PLL_CEC_CLKDIV(0x27));
#else
	pll_set_clock_rate(PLL_CLKSEL_CEC_CLKDIV, PLL_CEC_CLKDIV(0x1D));
#endif

	//pll_enable_clock(CEC_CLK);

	// Enable CEC related Interrupts.
	hdmitx_set_int_en(CEC_INTALL, ENABLE);

#if 0
	{
		UINT32 cnt;

		cnt = 0;
		while (1) {
			DELAY_US(500);
			if (gpio_get_pin(P_GPIO_27)) {
				cnt++;
			} else {
				cnt = 0;
			}

			if (cnt >= 24) {
				//12ms = 2.4ms x 5
				break;
			}
		}
	}
#endif

	// Enable CEC module
	loc_cpu(flags);
	reg_ctrl.reg = CEC_GETREG(CEC_CTRL_REG_OFS);
	reg_ctrl.bit.EN = ENABLE;
	CEC_SETREG(CEC_CTRL_REG_OFS,    reg_ctrl.reg);
	unl_cpu(flags);

	bhdmi_cec_open = TRUE;
	return E_OK;
}


/**
    Close CEC module driver

    It is better to close the CEC driver before the HDMI controller to avoid un-predictable events.

    @return
     -@b E_OK: Done and Success.
*/
ER hdmicec_close(void)
{
	T_CEC_CTRL_REG  reg_ctrl;
	unsigned long      flags;

	if (bhdmi_cec_open == FALSE) {
		return E_OK;
	}

	// Disable CEC module
	loc_cpu(flags);
	reg_ctrl.reg = CEC_GETREG(CEC_CTRL_REG_OFS);
	reg_ctrl.bit.EN = DISABLE;
	CEC_SETREG(CEC_CTRL_REG_OFS,    reg_ctrl.reg);
	unl_cpu(flags);

	// Set CEC Interrupt Disable
	hdmitx_set_int_en(CEC_INTALL, DISABLE);

	// Disable CEC Clock Source
	//pll_disable_clock(CEC_CLK);

#ifndef __KERNEL__
#if (DRV_SUPPORT_IST == ENABLE)
	pf_ist_cb_hdmi    = NULL;
#else
	p_cec_cb_func      = NULL;
#endif
#endif

	bhdmi_cec_open    = FALSE;
	return E_OK;
}

/**
    Hook CEC ISR callback

    Hook CEC ISR callback handler.

    @param[in] isr_cb    ISR callback function for CEC.

    @return void
*/
void hdmicec_hook_callback(DRV_CB isr_cb)
{
	if (bhdmi_cec_open == FALSE) {
		return;
	}

#ifndef __KERNEL__
#if (DRV_SUPPORT_IST == ENABLE)
	pf_ist_cb_hdmi    = isr_cb;
#else
	p_cec_cb_func      = isr_cb;
#endif
#endif
}

#endif


/**
    Set HDMI CEC functional configurations

    This api is used to configure the function configuration of the CEC controller, including CEC bus transmit/receive timing,
    bus free count 1/2/3, retry count, ...etc.

    @param[in] config_id     Control function selection ID.
    @param[in] ui_config     Please reference to the "CEC_CONFIG_ID" and the application note for the control function parameter details.

    @return void
*/
void hdmicec_set_config(CEC_CONFIG_ID config_id, UINT32 ui_config)
{
	unsigned long      flags;

	loc_cpu(flags);
	switch (config_id) {
	case CEC_CONFIG_ID_FREECNT1: {
			T_CEC_CTRL_REG  reg_ctrl;

			if (ui_config > 15) {
				ui_config = 15;
			}

			// The free-count-1 is previous command sent successfully and want to send another command,
			// the initiator must wait this duration. Default value is 0x7.
			reg_ctrl.reg = CEC_GETREG(CEC_CTRL_REG_OFS);
			reg_ctrl.bit.FREECNT_1 = ui_config;
			CEC_SETREG(CEC_CTRL_REG_OFS,    reg_ctrl.reg);
		}
		break;

	case CEC_CONFIG_ID_FREECNT2: {
			T_CEC_CTRL_REG  reg_ctrl;

			if (ui_config > 15) {
				ui_config = 15;
			}

			// The free-count-2 is the new initiator must wait the bus idle time duration,
			// and then be allowed to send command. Default value is 0x5.
			reg_ctrl.reg = CEC_GETREG(CEC_CTRL_REG_OFS);
			reg_ctrl.bit.FREECNT_2 = ui_config;
			CEC_SETREG(CEC_CTRL_REG_OFS,    reg_ctrl.reg);
		}
		break;

	case CEC_CONFIG_ID_FREECNT3: {
			T_CEC_CTRL_REG  reg_ctrl;

			if (ui_config > 15) {
				ui_config = 15;
			}

			// The free-count-3 is when previous attemp to send failed, the initiator
			// can wait this duration for the next bus idle time. Default value is 0x3.
			reg_ctrl.reg = CEC_GETREG(CEC_CTRL_REG_OFS);
			reg_ctrl.bit.FREECNT_3 = ui_config;
			CEC_SETREG(CEC_CTRL_REG_OFS,    reg_ctrl.reg);
		}
		break;

	case CEC_CONFIG_ID_RETRY_CNT: {
			T_CEC_CTRL_REG  reg_ctrl;

			if (ui_config > 5) {
				ui_config = 5;
			}

			if (ui_config == 0) {
				ui_config = 1;
			}

			// When command send failed (NACKed by follower), the controller would automatically
			// re-transmit command for RETRY_CNT times.
			reg_ctrl.reg = CEC_GETREG(CEC_CTRL_REG_OFS);
			reg_ctrl.bit.TX_RETRY = ui_config;
			CEC_SETREG(CEC_CTRL_REG_OFS,    reg_ctrl.reg);
		}
		break;

	case CEC_CONFIG_ID_TX_TRIG: {
			T_CEC_TXTRIG_REG    reg_tx_trig;

			// Enable Command Transmit
			reg_tx_trig.reg = CEC_GETREG(CEC_TXTRIG_REG_OFS);
			reg_tx_trig.bit.TX_TRIG = 1;

			if (ui_config > 0) {
				CEC_SETREG(CEC_TXTRIG_REG_OFS, reg_tx_trig.reg);
			}
		}
		break;

	case CEC_CONFIG_ID_LOCAL_ADDR: {
			T_CEC_CTRL_REG  reg_ctrl;

			// The Local address on the CEC bus. For playback device, only 4/8/12 is allowed.
			// This address is got at logical address allocation process after cec opened.
			// The controller can only ACKed the bus command with with address.
			reg_ctrl.reg = CEC_GETREG(CEC_CTRL_REG_OFS);
			reg_ctrl.bit.LOCAL_ADDR   = ui_config & 0xF;
			CEC_SETREG(CEC_CTRL_REG_OFS,    reg_ctrl.reg);
		}
		break;

	case CEC_CONFIG_ID_SRC_ADDR: {
			T_CEC_TXHDR_REG reg_tx_hdr;

			// The Source address on the Tx command, this is used to notify the Rx where the command from.
			reg_tx_hdr.reg = CEC_GETREG(CEC_TXHDR_REG_OFS);
			reg_tx_hdr.bit.TX_SRC_ADDR  = ui_config & 0xF;
			CEC_SETREG(CEC_TXHDR_REG_OFS,   reg_tx_hdr.reg);
		}
		break;

	case CEC_CONFIG_ID_DEST_ADDR: {
			T_CEC_TXHDR_REG reg_tx_hdr;

			// The Tx command destination address. The follower with this address would receive this tx command.
			reg_tx_hdr.reg = CEC_GETREG(CEC_TXHDR_REG_OFS);
			reg_tx_hdr.bit.TX_DEST_ADDR  = ui_config & 0xF;
			CEC_SETREG(CEC_TXHDR_REG_OFS,   reg_tx_hdr.reg);
		}
		break;

	case CEC_CONFIG_ID_TXRISE_OFS: {
			T_CEC_TXTIME0_REG reg_time0;

			if (ui_config > 255) {
				ui_config = 255;
			}

			// Tx Command Rise Time Config
			reg_time0.reg = CEC_GETREG(CEC_TXTIME0_REG_OFS);
			reg_time0.bit.TX_RISE_OFS = ui_config;
			CEC_SETREG(CEC_TXTIME0_REG_OFS,   reg_time0.reg);
		}
		break;

	case CEC_CONFIG_ID_TXFALL_OFS: {
			T_CEC_TXTIME0_REG reg_time0;

			if (ui_config > 255) {
				ui_config = 255;
			}

			// Tx Command Fall Time Config
			reg_time0.reg = CEC_GETREG(CEC_TXTIME0_REG_OFS);
			reg_time0.bit.TX_FALL_OFS = ui_config;
			CEC_SETREG(CEC_TXTIME0_REG_OFS,   reg_time0.reg);
		}
		break;

	case CEC_CONFIG_ID_RXRISE_OFS: {
			T_CEC_RXTIME0_REG reg_time0;

			if (ui_config > 255) {
				ui_config = 255;
			}

			// Rx Command Rise Time Offset
			reg_time0.reg = CEC_GETREG(CEC_RXTIME0_REG_OFS);
			reg_time0.bit.RX_DATARISE_OFS = ui_config;
			CEC_SETREG(CEC_RXTIME0_REG_OFS,   reg_time0.reg);
		}
		break;

	case CEC_CONFIG_ID_RXFALL_OFS: {
			T_CEC_RXTIME1_REG reg_time1;

			if (ui_config > 255) {
				ui_config = 255;
			}

			// Rx Command Fall Time Offset
			reg_time1.reg = CEC_GETREG(CEC_RXTIME1_REG_OFS);
			reg_time1.bit.RX_DATAFALL_OFS = ui_config;
			CEC_SETREG(CEC_RXTIME1_REG_OFS,   reg_time1.reg);
		}
		break;

	case CEC_CONFIG_ID_RXSTART_OFS: {
			T_CEC_RXTIME0_REG reg_time0;

			if (ui_config > 255) {
				ui_config = 255;
			}

			// Rx Command Start bit Offset
			reg_time0.reg = CEC_GETREG(CEC_RXTIME0_REG_OFS);
			reg_time0.bit.RX_START_OFS = ui_config;
			CEC_SETREG(CEC_RXTIME0_REG_OFS,   reg_time0.reg);
		}
		break;

	case CEC_CONFIG_ID_RXSMPL_OFS: {
			T_CEC_RXTIME0_REG reg_time0;

			if (ui_config > 255) {
				ui_config = 255;
			}

			// Rx Command Sample Time Offset
			reg_time0.reg = CEC_GETREG(CEC_RXTIME0_REG_OFS);
			reg_time0.bit.RX_SMP_OFS = ui_config;
			CEC_SETREG(CEC_RXTIME0_REG_OFS,   reg_time0.reg);
		}
		break;

	case CEC_CONFIG_ID_DEGLITCH: {
			T_CEC_RXTIME0_REG reg_time0;

			if (ui_config > 15) {
				ui_config = 15;
			}

			// Rx Command De-Glitch time
			reg_time0.reg = CEC_GETREG(CEC_RXTIME0_REG_OFS);
			reg_time0.bit.DE_GLITCH = ui_config;
			CEC_SETREG(CEC_RXTIME0_REG_OFS,   reg_time0.reg);
		}
		break;

	case CEC_CONFIG_ID_ERROR_LOW: {
			T_CEC_RXTIME1_REG reg_time1;

			reg_time1.reg = CEC_GETREG(CEC_RXTIME1_REG_OFS);
			reg_time1.bit.RX_ERR_LOW = ui_config;
			CEC_SETREG(CEC_RXTIME1_REG_OFS,   reg_time1.reg);
		}
		break;

	case CEC_CONFIG_ID_NACK_CTRL: {
			T_CEC_FRMSIZE_REG   reg_frm;

			reg_frm.reg = CEC_GETREG(CEC_FRMSIZE_REG_OFS);
			reg_frm.bit.NACK_CTRL = (ui_config > 0);
			CEC_SETREG(CEC_FRMSIZE_REG_OFS, reg_frm.reg);
		}
		break;

	case CEC_CONFIG_ID_ARBI_RANGE: {
			T_CEC_TXTRIG_REG    reg_tx_trig;

			reg_tx_trig.reg = CEC_GETREG(CEC_TXTRIG_REG_OFS);
			reg_tx_trig.bit.ARBI_RANGE = ui_config;
			CEC_SETREG(CEC_TXTRIG_REG_OFS, reg_tx_trig.reg);
		}
		break;

	case CEC_CONFIG_ID_PHY_ADDR: {
			ui_physical_addr = ui_config & 0xFFFF;
			hdmitx_debug(("SET CEC PHYSICAL ADDRESS to 0x%04X\r\n", ui_physical_addr));
		}
		break;


	default:
		break;
	}
	unl_cpu(flags);


}

/**
    Get HDMI CEC functional configurations

    Get HDMI CEC functional configurations

    @param[in] config_id     Control function selection ID.

    @return  Please reference to the "CEC_CONFIG_ID" for the control function parameter details.
*/
UINT32 hdmicec_get_config(CEC_CONFIG_ID config_id)
{
	UINT32 ret = 0;

	switch (config_id) {
	case CEC_CONFIG_ID_FREECNT1: {
			T_CEC_CTRL_REG  reg_ctrl;

			reg_ctrl.reg = CEC_GETREG(CEC_CTRL_REG_OFS);
			ret = reg_ctrl.bit.FREECNT_1;
		}
		break;

	case CEC_CONFIG_ID_FREECNT2: {
			T_CEC_CTRL_REG  reg_ctrl;

			reg_ctrl.reg = CEC_GETREG(CEC_CTRL_REG_OFS);
			ret = reg_ctrl.bit.FREECNT_2;
		}
		break;

	case CEC_CONFIG_ID_FREECNT3: {
			T_CEC_CTRL_REG  reg_ctrl;

			reg_ctrl.reg = CEC_GETREG(CEC_CTRL_REG_OFS);
			ret = reg_ctrl.bit.FREECNT_3;
		}
		break;

	case CEC_CONFIG_ID_RETRY_CNT: {
			T_CEC_CTRL_REG  reg_ctrl;

			reg_ctrl.reg = CEC_GETREG(CEC_CTRL_REG_OFS);
			ret = reg_ctrl.bit.TX_RETRY;
		}
		break;

	case CEC_CONFIG_ID_LOCAL_ADDR: {
			T_CEC_CTRL_REG  reg_ctrl;

			reg_ctrl.reg = CEC_GETREG(CEC_CTRL_REG_OFS);
			ret = reg_ctrl.bit.LOCAL_ADDR;
		}
		break;

	case CEC_CONFIG_ID_SRC_ADDR: {
			T_CEC_TXHDR_REG reg_tx_hdr;

			reg_tx_hdr.reg = CEC_GETREG(CEC_TXHDR_REG_OFS);
			ret = reg_tx_hdr.bit.TX_SRC_ADDR;
		}
		break;

	case CEC_CONFIG_ID_DEST_ADDR: {
			T_CEC_TXHDR_REG reg_tx_hdr;

			reg_tx_hdr.reg = CEC_GETREG(CEC_TXHDR_REG_OFS);
			ret = reg_tx_hdr.bit.TX_DEST_ADDR;
		}
		break;

	case CEC_CONFIG_ID_TXRISE_OFS: {
			T_CEC_TXTIME0_REG reg_time0;

			reg_time0.reg = CEC_GETREG(CEC_TXTIME0_REG_OFS);
			ret = reg_time0.bit.TX_RISE_OFS;
		}
		break;

	case CEC_CONFIG_ID_TXFALL_OFS: {
			T_CEC_TXTIME0_REG reg_time0;

			reg_time0.reg = CEC_GETREG(CEC_TXTIME0_REG_OFS);
			ret = reg_time0.bit.TX_FALL_OFS;
		}
		break;

	case CEC_CONFIG_ID_RXRISE_OFS: {
			T_CEC_RXTIME0_REG reg_time0;

			reg_time0.reg = CEC_GETREG(CEC_RXTIME0_REG_OFS);
			ret = reg_time0.bit.RX_DATARISE_OFS;
		}
		break;

	case CEC_CONFIG_ID_RXFALL_OFS: {
			T_CEC_RXTIME1_REG reg_time1;

			reg_time1.reg = CEC_GETREG(CEC_RXTIME1_REG_OFS);
			ret = reg_time1.bit.RX_DATAFALL_OFS;
		}
		break;

	case CEC_CONFIG_ID_RXSTART_OFS: {
			T_CEC_RXTIME0_REG reg_time0;

			reg_time0.reg = CEC_GETREG(CEC_RXTIME0_REG_OFS);
			ret = reg_time0.bit.RX_START_OFS;
		}
		break;

	case CEC_CONFIG_ID_RXSMPL_OFS: {
			T_CEC_RXTIME0_REG reg_time0;

			reg_time0.reg = CEC_GETREG(CEC_RXTIME0_REG_OFS);
			ret = reg_time0.bit.RX_SMP_OFS;
		}
		break;

	case CEC_CONFIG_ID_DEGLITCH: {
			T_CEC_RXTIME0_REG reg_time0;

			reg_time0.reg = CEC_GETREG(CEC_RXTIME0_REG_OFS);
			ret = reg_time0.bit.DE_GLITCH;
		}
		break;

	case CEC_CONFIG_ID_ERROR_LOW: {
			T_CEC_RXTIME1_REG reg_time1;

			reg_time1.reg = CEC_GETREG(CEC_RXTIME1_REG_OFS);
			ret = reg_time1.bit.RX_ERR_LOW;
		}
		break;

	case CEC_CONFIG_ID_NACK_CTRL: {
			T_CEC_FRMSIZE_REG   reg_frm;

			reg_frm.reg = CEC_GETREG(CEC_FRMSIZE_REG_OFS);
			ret = reg_frm.bit.NACK_CTRL;
		}
		break;


	case CEC_CONFIG_ID_PHY_ADDR: {
			ret = ui_physical_addr;
		}
		break;

	default:
		DBG_WRN("Get Cfg ID err\r\n");
		break;
	}


	return ret;
}

/**
    Set CEC Transmit Command Data

    Set CEC Transmit Command Data, including the Data Size and the data buffer address.
    The first ui_tx_data_size bytes of the data buffer address would be assign to the transmit register of the CEC controller.
    After using this api, the user should call hdmicec_set_config(CEC_CONFIG_ID_TX_TRIG, TRUE) to trigger the transmit operation.
    After the transmission is done, the interrupt status CEC_TXFRM would be issued and the user can identify it at the callback handler.
    If setting the ui_tx_data_size to 0x0 means none of the Data Bytes would transmit, this is still the valid packet on the CEC bus because
    only the header byte would be sent and this is called the polling or ping command in the CEC standard.

    @param[in] ui_tx_data_size     Number of the CEC Tx command bytes. Valid range from 0 to 15.
    @param[in] ui_data_buf_addr    CEC command data buffer address. The first ui_tx_data_size bytes of the data buffer address would be assign to the transmit register of the CEC controller.

    @return void
*/
void hdmicec_set_tx_data(UINT32 ui_tx_data_size, UINT32 ui_data_buf_addr)
{
#if 0

	UINT32 i, *p_buf;
	T_CEC_FRMSIZE_REG   reg_frm;

	if (ui_tx_data_size > 15) {
		ui_tx_data_size = 15;
	}

	reg_frm.reg = CEC_GETREG(CEC_FRMSIZE_REG_OFS);
	reg_frm.bit.TX_LENGTH = ui_tx_data_size;
	CEC_SETREG(CEC_FRMSIZE_REG_OFS, reg_frm.reg);


	p_buf = (UINT32 *)ui_data_buf_addr;
	ui_tx_data_size = (ui_tx_data_size + 3) >> 2;

	for (i = 0; i < ui_tx_data_size; i++) {
		CEC_SETREG((CEC_TXDATA0_REG_OFS + (i << 2)), p_buf[i]);
	}
#endif
}

/**
    Get the CEC received Data Command

    Get CEC Received Command Data, including the Command Data length.
    The pui_rx_data_size contains the number of received command byte count.
    The ui_data_buf_addr is the buffer address and the first byte of the buffer contans the CEC header byte which indicating
    the source and destination device address. The user can reference to the CEC standard for the details.

    @param[out] pui_rx_data_size    Number of the CEC received command bytes. Valid range from 1 to 16.
    @param[out] ui_data_buf_addr    The returned CEC command data buffer address.

    @return void
*/
void hdmicec_get_rx_data(UINT32 *pui_rx_data_size, UINT32 ui_data_buf_addr)
{
#if 0
	T_CEC_FRMSIZE_REG   reg_frm;
	UINT32              i;
	UINT8               *p_buf;

	p_buf = (UINT8 *)ui_data_buf_addr;

	reg_frm.reg = CEC_GETREG(CEC_FRMSIZE_REG_OFS);
	*pui_rx_data_size  = reg_frm.bit.RX_LENGTH + 1;

	p_buf[0] = CEC_GETREG(CEC_RXHDR_REG_OFS) & 0xFF;
	for (i = 0; i < (reg_frm.bit.RX_LENGTH); i++) {
		p_buf[i + 1] = (UINT8)((CEC_GETREG(CEC_RXDATA0_REG_OFS + (i & 0xC)) >> ((i & 0x3) << 3)) & 0xFF);
	}
#endif
}






