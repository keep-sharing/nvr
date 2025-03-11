#ifndef __HDMI_PORT__
#define __HDMI_PORT__

#include <common.h>
#include <asm/io.h>

#define LINUX
#define	DRV_VER_96680			0x20170000
#define P_GPIO_PIN				46	//TBD
#define HDMI_RSTN				1	//any value
#define	P_GPIO_28				28	//any value
#define	P_GPIO_29				29	//any value

#define FLG_ID_HDMI				0x0
#define FLGPTN_HDMI				0x1
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

//#define HDMI_DEV1   1
//#ifndef CONFIG_FPGA
//#define CONFIG_FPGA 1
//#endif

//#ifdef CONFIG_FPGA
/*TC670*/
//#define PHY_TC18034   0
//#else
/* TC18034: UMC */
#define PHY_TC18034   1
//#endif

#if PHY_TC18034
#define HDMI_I2C_ADDR   0xB0
#define PHY_PAGE        0x02
#else
#define HDMI_I2C_ADDR   0xC0
#define PHY_PAGE        0x09
#endif

//#ifdef CONFIG_FPGA
//#define HDMI_I2C_BUS	3
//#else
#define HDMI_I2C_BUS	3
//#endif

enum {
	I2C_CONFIG_ID_RR_EN,		///< Enable/Disable the HDMI SCDC Read Request function.
	I2C_CONFIG_ID_RR_CALLBACK,	///< HDMI SCDC Read Request Mode select. Please use I2C_RR_MODE as input parameter.
	I2C_CONFIG_ID_BUSCLOCK,
	I2C_CONFIG_ID_HANDLE_NACK,
	I2C_CONFIG_ID_TSR,

};

enum {
	IDE1_CLK = 1,
	HDMI_CLK,
	CEC_CLK,
	IDECLK_FREQ,
};

enum {
	PLL_ID_2 = 1,
	PLL_CLKSEL_IDE_CLKSRC,
	PLL_CLKSEL_IDE_CLKSRC_PLL2,
};

typedef unsigned int       		UINT;
typedef unsigned int       		UINT32;     ///< Unsigned 32 bits data type
typedef unsigned short      	UINT16;     ///< Unsigned 16 bits data type
typedef signed int         		INT32;      ///< Signed 32 bits data type
typedef unsigned char       	UINT8;      ///< Unsigned 8 bits data type
typedef unsigned int			BOOL;
typedef void (*DRV_CB)(UINT32 uiEvent);

extern unsigned int DRV_INT_HDMI;
extern unsigned int IOADDR_HDMI_REG_BASE;
#if 0
extern INT32 efuse_read_param_ops(UINT32 option, UINT16 *trim_val);
#endif

#define E_OK  			0
#define E_SYS			-5			///< System error
#define E_FAIL 			E_SYS
#define E_NOSPT         (-17)   	///< Feature not supported
#define E_PAR           (-33)		///< Parameter error
#define	ER				int
#define TRUE            1
#define FALSE           0
#define DISABLE         0           ///< Feature is disabled
#define ENABLE          1           ///< Feature is enabled
#define ENUM_DUMMY4WORD(name)   E_##name = 0x10000000

#ifdef CONFIG_FPGA
#define _FPGA_EMULATION_	ENABLE
#define _EMULATION_			ENABLE
#else
#define _FPGA_EMULATION_	DISABLE
#define _EMULATION_			DISABLE
#endif

#define DRV_SUPPORT_IST     DISABLE
#define EFUSE_HDMI_TRIM_DATA	1
#define OUTW(addr, val)		writel(val, (void *)(addr))
#define INW(addr)			readl((void *)(addr))

#define pll_setPLLEn(x, y)
/* gpio */
#define gpio_set_dir(x,y)	do { } while(0)
#define gpio_clear_pin(x)	do { } while(0)
#define DELAY_MS(x)			msleep(x)
#define pinmux_set_pinmux(x, y)	do { } while(0)
#define pll_set_clock_rate(x, y)	do { } while(0)

#define REGVALUE         	UINT32
typedef unsigned int        UBITFIELD;  ///< Unsigned bit field
typedef signed int          BITFIELD;   ///< Signed bit field

#define REGDEF_BEGIN(name)      \
	typedef union                   \
	{                               \
		REGVALUE    reg;            \
		struct                      \
		{

// Register Cache Word bit defintion
#define REGDEF_BIT(field, bits) \
			UBITFIELD   field : bits;

// Register Cache Word type defintion trailer
#define REGDEF_END(name)        \
		} bit;                      \
	} T_##name;                     \

// Macro to define register offset
#define REGDEF_OFFSET(name, ofs)        static const UINT32 name##_OFS=(ofs);

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

#define I2C_CONFIG_ID	int

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
void i2c_set_config(I2C_SESSION Session, I2C_CONFIG_ID ConfigID, UINT32 uiConfig);
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

#endif /* __HDMI_PORT__ */
