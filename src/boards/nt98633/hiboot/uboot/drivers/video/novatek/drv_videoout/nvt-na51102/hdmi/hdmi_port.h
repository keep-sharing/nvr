#ifndef __HDMI_PORT__
#define __HDMI_PORT__

#include <common.h>
#include <asm/io.h>
#include <asm/nvt-common/nvt_types.h>



/**
	PLL ID
*/
typedef enum {
	PLL_ID_0		= 0,		///< PLL0 (for AXI0)
	PLL_ID_1		= 1,		///< PLL1 (for internal 480 MHz)
	PLL_ID_2		= 2,		///< PLL2 (for AXI1)
	PLL_ID_3		= 3,		///< PLL3 (for DRAM)
	PLL_ID_4		= 4,		///< PLL4 (for AHB)
	PLL_ID_5		= 5,		///< PLL5 (for SIE_MCLK2)
	PLL_ID_6		= 6,		///< PLL6 (for SDC)
	PLL_ID_7		= 7,		///< PLL7 (for ETH)
	PLL_ID_8		= 8,		///< PLL8 (for CNN)
	PLL_ID_9		= 9,		///< PLL9 (for DISP0)
	PLL_ID_10		= 10,		///< PLL10 (for DISP1)
	PLL_ID_11		= 11,		///< PLL11 (for SIE_MCLK)
	PLL_ID_12		= 12,		///< PLL12 (for AUDIO)
	PLL_ID_13		= 13,		///< PLL13 (for AXI2)
	PLL_ID_14		= 14,		///< PLL14 (for VENC)
	PLL_ID_15		= 15,		///< PLL15 (for IPP CODEC BAKUP0)
	PLL_ID_16		= 16,		///< PLL16 (for CPU)
	PLL_ID_17		= 17,		///< PLL17 (for DSI)
	PLL_ID_18		= 18,		///< PLL18 (for IPP CODEC BAKUP1)
	PLL_ID_19		= 19,		///< PLL19 (for SIE SENSOR)
	PLL_ID_20		= 20,		///< PLL20 (for USB3 PHY)
	PLL_ID_21		= 21,		///< PLL21 (for DSP)
	PLL_ID_22		= 22,		///< PLL22 (for SATA PHY)
	PLL_ID_23		= 23,		///< PLL23 (for VIE)
	PLL_ID_24		= 24,		///< PLL24 (for AE)

	PLL_ID_FIXED320 = 25,		///< Fixed 320MHz PLL

	PLL_ID_MAX,
	ENUM_DUMMY4WORD(PLL_ID)
} PLL_ID;






#define PLL_CR0_OFS                      0x20
#define PLL_CR1_OFS                      0x24
#define PLL_CR2_OFS                      0x28



#define PLL530_PLL00_BASE_REG_OFS        0x4400
#define PLL530_PLL02_BASE_REG_OFS        0x4480
#define PLL530_PLL03_BASE_REG_OFS        0x44C0
#define PLL530_PLL04_BASE_REG_OFS        0x4500
#define PLL530_PLL05_BASE_REG_OFS        0x4540
#define PLL530_PLL06_BASE_REG_OFS        0x4580
#define PLL530_PLL07_BASE_REG_OFS        0x45C0
#define PLL530_PLL08_BASE_REG_OFS        0x4600
#define PLL530_PLL09_BASE_REG_OFS        0x4640
#define PLL530_PLL10_BASE_REG_OFS        0x4680
#define PLL530_PLL11_BASE_REG_OFS        0x46C0
#define PLL530_PLL12_BASE_REG_OFS        0x4700
#define PLL530_PLL13_BASE_REG_OFS        0x4740
#define PLL530_PLL14_BASE_REG_OFS        0x4780
#define PLL530_PLL15_BASE_REG_OFS        0x47C0
#define PLL530_PLL16_BASE_REG_OFS        0x4800
#define PLL530_PLL17_BASE_REG_OFS        0x4840
#define PLL530_PLL18_BASE_REG_OFS        0x4880
#define PLL530_PLL19_BASE_REG_OFS        0x48C0
#define PLL530_PLL20_BASE_REG_OFS        0x4900
#define PLL530_PLL21_BASE_REG_OFS        0x4940
#define PLL530_PLL22_BASE_REG_OFS        0x4980
#define PLL530_PLL23_BASE_REG_OFS        0x49C0
#define PLL530_PLL24_BASE_REG_OFS        0x4A00


#define PLL530_PLL0_CR0_REG_OFS         (PLL530_PLL00_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL530_PLL2_CR0_REG_OFS         (PLL530_PLL02_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL530_PLL3_CR0_REG_OFS         (PLL530_PLL03_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL530_PLL4_CR0_REG_OFS         (PLL530_PLL04_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL530_PLL5_CR0_REG_OFS         (PLL530_PLL05_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL530_PLL6_CR0_REG_OFS         (PLL530_PLL06_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL530_PLL7_CR0_REG_OFS         (PLL530_PLL07_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL530_PLL8_CR0_REG_OFS         (PLL530_PLL08_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL530_PLL9_CR0_REG_OFS         (PLL530_PLL09_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL530_PLL10_CR0_REG_OFS        (PLL530_PLL10_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL530_PLL11_CR0_REG_OFS        (PLL530_PLL11_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL530_PLL12_CR0_REG_OFS        (PLL530_PLL12_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL530_PLL13_CR0_REG_OFS        (PLL530_PLL13_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL530_PLL14_CR0_REG_OFS        (PLL530_PLL14_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL530_PLL15_CR0_REG_OFS        (PLL530_PLL15_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL530_PLL16_CR0_REG_OFS        (PLL530_PLL16_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL530_PLL17_CR0_REG_OFS        (PLL530_PLL17_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL530_PLL18_CR0_REG_OFS        (PLL530_PLL18_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL530_PLL19_CR0_REG_OFS        (PLL530_PLL19_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL530_PLL20_CR0_REG_OFS        (PLL530_PLL20_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL530_PLL21_CR0_REG_OFS        (PLL530_PLL21_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL530_PLL22_CR0_REG_OFS        (PLL530_PLL22_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL530_PLL23_CR0_REG_OFS        (PLL530_PLL23_BASE_REG_OFS+PLL_CR0_OFS)
#define PLL530_PLL24_CR0_REG_OFS        (PLL530_PLL24_BASE_REG_OFS+PLL_CR0_OFS)



#define LINUX
#define	DRV_VER_96680			0x20170000
#define P_GPIO_PIN				46	//TBD
#define HDMI_RSTN				1	//any value
#define	P_GPIO_28				28	//any value
#define	P_GPIO_29				29	//any value


#define FLGPTN_HDMI_REMOVE		0x2		/* driver is going to remove */
#define HDMITX_HWDDC_SPEED		80000	//can't exceed 100K
#define TWF_ORW					0x1
#define TWF_CLR					0x4
#define FLGPTN					unsigned int
#define PFLGPTN					unsigned int *
#define ID						unsigned int
#define	DBG_DUMP				printf
#define DBG_WRN					printf
#define DBG_ERR					printf
#define DBG_IND					printf

#define iset_flg				set_flg

#define i2c5_open				i2c_open
#define i2c5_close				i2c_close
#define i2c5_lock				i2c_lock
#define i2c5_unlock				i2c_unlock
#define i2c5_set_config			i2c_set_config
#define i2c5_transmit			i2c_transmit
#define i2c5_receive			i2c_receive

#define hdmiddc_open            i2c_open
#define hdmiddc_close           i2c_close
#define hdmiddc_lock            i2c_lock
#define hdmiddc_unlock          i2c_unlock
#define hdmiddc_setConfig       i2c_set_config
#define hdmiddc_transmit        i2c_transmit
#define hdmiddc_receive         i2c_receive

#define PHY_TC18034   0


#if PHY_TC18034
#define HDMI_I2C_ADDR   0xB0
#define PHY_PAGE        0x02
#else
#define HDMI_I2C_ADDR   0xC0
#define PHY_PAGE        0x09
#endif


#define HDMI_I2C_BUS	3




enum {
	IDE1_CLK = 1,
	HDMI_CLK,
	CEC_CLK,
	IDECLK_FREQ,
};



//typedef unsigned int       		UINT;
//typedef unsigned int       		UINT32;     ///< Unsigned 32 bits data type
//typedef unsigned short      	UINT16;     ///< Unsigned 16 bits data type
//typedef signed int         		INT32;      ///< Signed 32 bits data type
//typedef unsigned char       	UINT8;      ///< Unsigned 8 bits data type
//typedef unsigned int			BOOL;
typedef void (*DRV_CB)(UINT32 uiEvent);

extern unsigned int DRV_INT_HDMI;
extern uintptr_t PIOADDR_HDMI_REG_BASE;


#if 0
#define E_OK  			0
#define E_SYS			-5			///< System error
#define E_FAIL 			E_SYS
#define E_NOSPT         (-17)   	///< Feature not supported
#define E_PAR           (-33)		///< Parameter error
#ifndef ER
#define	ER				int
#endif
#define TRUE            1
#define FALSE           0
#define DISABLE         0           ///< Feature is disabled
#define ENABLE          1           ///< Feature is enabled
#define ENUM_DUMMY4WORD(name)   E_##name = 0x10000000
#endif

#ifdef CONFIG_FPGA
#define _FPGA_EMULATION_	ENABLE
#define _EMULATION_			ENABLE
#else
#define _FPGA_EMULATION_	DISABLE
#define _EMULATION_			DISABLE
#endif

#define DRV_SUPPORT_IST     DISABLE
#define EFUSE_HDMI_TRIM_DATA	1


#define pll_setPLLEn(x, y)
/* gpio */
#define gpio_set_dir(x,y)	do { } while(0)
#define gpio_clear_pin(x)	do { } while(0)
#define pinmux_set_pinmux(x, y)	do { } while(0)


/**
    I2C Status

    I2C status for transmit and receive. Only ONE of these base status could be returned.
*/
typedef enum {
	I2C_STS_OK,                     ///< (Base status) Status OK
	I2C_STS_INVALID_PARAM,          ///< (Base status) Invalid parameter
	I2C_STS_NACK,                   ///< (Base status) Receive NACK
	I2C_STS_BUS_NOT_AVAILABLE,      ///< (Base status) Bus is not avaliable
	I2C_STS_AL,                     ///< (Base status) Arbitration lost
	I2C_STS_SAM,                    ///< (Base status) Slave address matched, for i2c_waitSARMatch()
	I2C_STS_GC,                     ///< (Base status) Receive general call, for i2c_waitSARMatch()

	I2C_STS_STOP = 0x80000000,      ///< (Extra status) Detect STOP at slave mode
	///< You might get (I2C_STS_NACK | I2C_STS_STOP) or (I2C_STS_NACK | I2C_STS_STOP)

	ENUM_DUMMY4WORD(I2C_STS)
} I2C_STS, *PI2C_STS;



/**
    I2C session ID

    When I2C operates at master mode, I2C master can be transmit to multiple i2c-devices on the same I2C
    physical bus. This session id means one i2c-device on the bus. When the application user writes the
    driver for one i2c-device, the user should use i2c_open() to get the session id for your device.
    And then use this session id for the furthur i2c-device configurations/control.
    when the i2c_lock(I2C_SESSION) by the dedicated i2c driver session, the i2c configurations for that
    session would taking effect, such as I2C-BUS-Speed/Pinmux/... ,etc. So the user must notice that the
    i2c_setConfig(I2C_SESSION) configurations must be completed before i2c_lock(I2C_SESSION).
*/
typedef enum {
	I2C_SES0 = 0,                       ///< I2C session 0
	I2C_SES1,                           ///< I2C session 1
	I2C_SES2,                           ///< I2C session 2
	I2C_SES3,                           ///< I2C session 3
	I2C_SES4,                           ///< I2C session 4
	I2C_SES5,                           ///< I2C session 5

	I2C_TOTAL_SESSION,                  ///< I2C total session number
	ENUM_DUMMY4WORD(I2C_SESSION)
} I2C_SESSION, *PI2C_SESSION;

/**
    I2C transmit/receive byte counter for PIO mode

    I2C transmit/receive byte counter for PIO mode.
*/
typedef enum {
	I2C_BYTE_CNT_1 = 1,
	I2C_BYTE_CNT_2,
	I2C_BYTE_CNT_3,
	I2C_BYTE_CNT_4,
	I2C_BYTE_CNT_5,
	I2C_BYTE_CNT_6,
	I2C_BYTE_CNT_7,
	I2C_BYTE_CNT_8,

	ENUM_DUMMY4WORD(I2C_BYTE_CNT)
} I2C_BYTE_CNT, *PI2C_BYTE_CNT;

/**
    I2C transmit/receive byte parameter for PIO mode

    I2C transmit/receive byte parameter for PIO mode.
*/
typedef enum {
	I2C_BYTE_PARAM_NONE     = 0x00000000,   ///< Dummy, for slave transmit or master transmit without START or STOP
	I2C_BYTE_PARAM_ACK      = 0x00000000,   ///< Receive mode only, response ACK after data is received
	I2C_BYTE_PARAM_NACK     = 0x00000001,   ///< Receive mode only, response NACK after data is received
	I2C_BYTE_PARAM_START    = 0x00000004,   ///< Master mode only, generate START condition before transmit data
	I2C_BYTE_PARAM_STOP     = 0x00000008,   ///< Master mode only, generate STOP condition after data is transmitted or received
	///< @note  Can't generate STOP and START at the same byte

	ENUM_DUMMY4WORD(I2C_BYTE_PARAM)
} I2C_BYTE_PARAM, *PI2C_BYTE_PARAM;

/**
    I2C transmit/receive byte data and parameter for PIO mode

    I2C transmit/receive byte data and parameter for PIO mode.
*/
typedef struct {
	UINT32          uiValue;                ///< 8-bits data to transmit or receive
	I2C_BYTE_PARAM  Param;                  ///< Parameter to transmit or receive data
} I2C_BYTE, *PI2C_BYTE;

/**
    I2C transmit / receive parameter for PIO mode

    I2C transmit / receive parameter for PIO mode.
*/
typedef struct {
	int    			VersionInfo;            ///< Driver version
	I2C_BYTE_CNT    ByteCount;              ///< How many bytes will be transmitted / received
	I2C_BYTE       *pByte;                  ///< Byte data and parameter
} I2C_DATA, *PI2C_DATA;

ER i2c_open(PI2C_SESSION pSession);
ER i2c_close(I2C_SESSION Session);
ER i2c_lock(I2C_SESSION Session);
ER i2c_unlock(I2C_SESSION Session);

I2C_STS i2c_transmit(PI2C_DATA pData);
I2C_STS i2c_receive(PI2C_DATA pData);
void drv_enable_int(int IntNum);
void drv_disable_int(int IntNum);

ER hdmitx_open(void);
void hdmi_isr(void);
/* porting */
//void pll_enable_clock(int clkid);
//void pll_disable_clock(int clkid);
ER pll_set_clock_freq(int ClkID, UINT32 uiFreq);
ER pll_set_pLL(int id, UINT32 uiSetting);
//void pll_disable_system_reset(int id);
ER set_flg(ID flgid, FLGPTN setptn);
ER wai_flg(PFLGPTN p_flgptn, ID flgid, FLGPTN waiptn, UINT wfmode);
UINT32 gpio_getPin(UINT32 pin);
void DELAY_US(UINT32 uiUS);

ER pll_set_driver_pll_hdmi(PLL_ID id, UINT32 ui_setting);
UINT32 pll_get_pll_freq_hdmi(PLL_ID id);



#endif /* __HDMI_PORT__ */