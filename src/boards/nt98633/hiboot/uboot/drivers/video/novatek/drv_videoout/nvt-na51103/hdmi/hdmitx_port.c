/* This file is porting file for HDMI driver
 */

#include <common.h>
#include <command.h>
#include <config.h>
#include <asm/io.h>
//#include <asm/arch/IOAddress.h>
#include "hdmi_port.h"
#include "hdmitx.h"
#include "hdmitx_int.h"
#include "dispdev_hdmi.h"
#include "dispdev_hdmi_if.h"
#include "hdmitx_reg.h"
#include "asm/arch/hdmi_if.h"
#include "nvt_hdmi.h"


extern int clock_rate;
//extern UINT32  guiTrimRsel;
extern I2C_STS	err_code;
int	phy_trim_val = 0;

#define DRV_VER			"1.02.00"

#define I2C_BUFF_SZ		I2C_BYTE_CNT_8

#define DISPDEV_OBJ		hdmi_info.dispdev_obj

/* Linux wait queue */
unsigned int DRV_INT_HDMI = 52;
#if defined(CONFIG_ARM64)
uintptr_t  PIOADDR_HDMI_REG_BASE = 0x2F0880000;
static uintptr_t  IOADDR_CG_REG_BASE = 0x2F0020000;
#else
uintptr_t  PIOADDR_HDMI_REG_BASE = 0xF0880000;
static uintptr_t  IOADDR_CG_REG_BASE = 0xF0020000;
#endif


/* main structure
 */
static struct hdmi_info_s {
	DISP_HDMI_CFG config;
	PDISPDEV_OBJ  dispdev_obj;
} hdmi_info;

/* TBD */
INT32 efuse_readParamOps(UINT32 option, UINT16 *trim_val)
{
	INT32 ret = 0;

	switch (option) {
	  case EFUSE_HDMI_TRIM_DATA:
	  	if (phy_trim_val)
	  		*trim_val = phy_trim_val;
	  	else
	  		*trim_val = (UINT16)HDMITX_RTRIM_DEFAULT;
	  	break;
	  default:
	  	ret = 0;
	  	break;
	}

	return ret;
}

ER set_flg(ID flgid, FLGPTN setptn)
{
	return E_OK;
}

ER wai_flg(PFLGPTN p_flgptn, ID flgid, FLGPTN waiptn, UINT wfmode)
{
	return E_OK;
}

/**
    Get GPIO pin call from hdmitx_checkHotplug()

    Get signal status of specified GPIO pin

    @param[in] pin GPIO pin number.

    @return
     - @b TRUE:  Hotplug attached.
     - @b FALSE: Hotplug detached.
*/
UINT32 gpio_getPin(UINT32 pin)
{
	return 1;
}

void pll_enableClock(int id)
{
}

void pll_disableClock(int id)
{
}

ER pll_setClockFreq(int ClkID, UINT32 uiFreq)
{
	return E_OK; /* do nothing */
}

void pll_disableSystemReset(int id)
{
	if (id) {}

}

void DELAY_US(UINT32 uiUS)
{
	udelay(uiUS);
}

ER pll_setPLL(int id, UINT32 uiSetting)
{
	return E_OK;
}

ER i2c_open(PI2C_SESSION pSession)
{
    return E_FAIL;
}

ER i2c_close(I2C_SESSION Session)
{
	return E_OK;
}

ER i2c_lock(I2C_SESSION Session)
{
	return E_OK;
}

ER i2c_unlock(I2C_SESSION Session)
{
	return E_OK;
}

void i2c_set_config(I2C_SESSION Session, I2C_CONFIG_ID ConfigID, UINT32 uiConfig)
{

}

/* porting from hdmitx_edid.c. I2c consists of two parts, one is write and the other is read. According to uitron's
 * characteristic, we put i2c_transfer in receive function
 */
I2C_STS i2c_transmit(PI2C_DATA pData)
{
	return I2C_STS_OK;
}

/* porting from hdmitx_edid.c. I2c consists of two parts, one is write and the other is read. According to uitron's
 * characteristic, we put i2c_transfer in receive function
 */
I2C_STS i2c_receive(PI2C_DATA pData)
{
	return I2C_STS_OK;
}

void drv_enable_int(int IntNum)
{
}

void drv_disable_int(int IntNum)
{
}

void hdmi_callback(UINT32 data)
{
}

int hdmi_if_init(hdmi_video_t video)
{
	unsigned int val;

	val = readl((void *)(IOADDR_CG_REG_BASE + 0x74));
	val |= (0x1 << 14);
	writel(val, (void *)(IOADDR_CG_REG_BASE + 0x74));

	memset(&hdmi_info, 0, sizeof(struct hdmi_info_s));

	hdmi_info.config.audio_fmt = HDMI_AUDIO48KHZ;
	hdmi_info.config.output_mode = HDMI_MODE_YCBCR444;
	hdmi_info.config.source = NVT_HDMI_LCD310_0;

	DISPDEV_OBJ = dispdev_get_hdmi_dev_obj();
	/* register callback */
	DISPDEV_OBJ->setDevIOCtrl(hdmi_callback);

	switch (video) {
	case HDMI_VIDEO_720P:
		clock_rate = 74250;
		hdmi_info.config.vid = HDMI_1280X720P60;
		break;
	case HDMI_VIDEO_1080P:
		clock_rate = 148500;
		hdmi_info.config.vid = HDMI_1920X1080P60;
		break;
	case HDMI_VIDEO_1024x768:
		clock_rate = 65000;
		hdmi_info.config.vid = HDMI_1024X768P60;
		break;
	default:
		printf("%s: Error Value: %d \n", __func__, video);
		return -1;
		break;
	}

	if (DISPDEV_OBJ->preSetConfig(&hdmi_info.config) != E_OK) {
		printf("%s, error! set hdmi fail! \n", __func__);
	} else {
		printf("%s, apply new vid:%d setting. \n", __func__, hdmi_info.config.vid);
	}

	DISPDEV_OBJ->open();

	return 0;
}

