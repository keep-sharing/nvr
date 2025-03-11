/*
    HDMI TX module GPIO Version DDC

    HDMI TX module GPIO Version DDC.

    @file       hdmitx_ddc.c
    @ingroup    mIDrvDisp_HDMITx
    @note       Nothing.

    Copyright   Novatek Microelectronics Corp. 2013.  All rights reserved.
*/
#include "hdmitx_int.h"
#ifndef __KERNEL__
#include "Delay.h"
#include "top.h"
#endif

UINT32 hdmi_sw_ddc_scl, hdmi_sw_ddc_sda;

#if 0
static BOOL hdmi_ddc_start    = FALSE;
static BOOL hdmi_sw_ddc_opened = FALSE;
static BOOL hdmi_sw_ddc_locked = FALSE;

/*
    Open GPIO Version DDC Driver

    Note: Because the HDMI DDC is 1-by-by I2C protocol.
    This GPIO Version DDC access does not implement Arbitration Judgement feature.
*/
ER hdmiddc_open(PI2C_SESSION p_session)
{
	if (hdmi_sw_ddc_opened) {
		return E_OK;
	}

	DBG_WRN("HDMI DDC Channel is usiong GPIO Version.\r\n");

	hdmi_sw_ddc_opened = TRUE;
	*p_session       = I2C_SES0;

	gpio_set_dir(hdmi_sw_ddc_scl, GPIO_DIR_INPUT);
	gpio_set_dir(hdmi_sw_ddc_sda, GPIO_DIR_INPUT);

	return E_OK;
}

/*
    Close GPIO Version DDC Driver
*/
ER hdmiddc_close(I2C_SESSION session)
{
	if (!hdmi_sw_ddc_opened) {
		return E_OK;
	}

	hdmi_sw_ddc_opened = FALSE;
	return E_OK;
}

/*
    Lock GPIO Version DDC Usage.

    This API would also turn OFF the  I2C CH2 pinmux.
*/
ER hdmiddc_lock(I2C_SESSION session)
{
	while (hdmi_sw_ddc_locked) {
		DELAY_US(10000);
	}

	// Because we make sure that the DDC is using GPIO version.
	// So the I2C CH2 pinmux can be closed here which become acceptable for NT96650.
	if ((hdmi_sw_ddc_scl == P_GPIO_28) || (hdmi_sw_ddc_scl == P_GPIO_29) || (hdmi_sw_ddc_sda == P_GPIO_28) || (hdmi_sw_ddc_sda == P_GPIO_29)) {
		pinmux_set_pinmux(PINMUX_FUNC_ID_I2C2_1ST, PINMUX_I2C_SEL_INACTIVE);
	}

	hdmi_sw_ddc_locked = TRUE;
	return E_OK;
}

/*
    UnLock GPIO Version DDC Usage
*/
ER hdmiddc_unlock(I2C_SESSION session)
{
	hdmi_sw_ddc_locked = FALSE;
	return E_OK;
}

/*
    Transmit One to Four Bytes

    Note: Because the HDMI DDC is 1-by-by I2C protocol.
    This GPIO Version DDC access does not implement Arbitration Judgement feature.
*/
I2C_STS hdmiddc_transmit(PI2C_DATA p_data)
{
	return I2C_STS_OK;
}

/*
    Receive One to Four Bytes

    Note: Because the HDMI DDC is 1-by-by I2C protocol.
    This GPIO Version DDC access does not implement Arbitration Judgement feature.
*/
I2C_STS hdmiddc_receive(PI2C_DATA p_data)
{
	return I2C_STS_OK;
}

/*
    Set Configuration
*/
void hdmiddc_set_config(I2C_SESSION session, I2C_CONFIG_ID config_id, UINT32 ui_config)
{

}
#endif
