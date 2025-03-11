#ifndef __ASM_ARCH_HI3536_GPIO_H
#define __ASM_ARCH_HI3536_GPIO_H

/*  GPIO0 -> GPIO3: 0x1215 0000 -> 0x1218 0000
  *  GPIO5 -> GPIO13: 0x121a 0000 -> 0x1222 0000
  * GPIO15: 0x20260000
  */

#define HI_GPIO_BASE			(0x12150000)
#define HI_GPIO_OFFSET			(0x10000)
#define HI_GPIO_GROUP_NUM	    (14) //group4 is null;
#define HI_GPIO_BIT_NUM			(8)

// pin defined
#define MS_GPIO(g,n)	(g*8+n)
#define MS_GROUP(gpio)	(gpio/8)
#define MS_BIT(gpio)	(gpio%8)
#define MS_PADDR(gpio)	(1 << (MS_BIT(gpio)+2))
#define MS_ENABLE		(1)
//soft i2c gpio 
#define I2C2_SDA_IO		MS_GPIO(12,6)
#define I2C2_SCL_IO		MS_GPIO(12,7)

#define RESET_KEY_PRESSED (0)
#define BUZZ_ON           (0)
#define BUZZ_OFF          (1)

//gpio functions
//#define RELAY_OUT1
//#define RELAY_OUT2
//#define RELAY_OUT3
//#define RELAY_OUT4

#define POWER_GP0       MS_GPIO(9,0)
#define STAT_GREEN_LED  MS_GPIO(0,1)
#define BUZZ_CTL       MS_GPIO(0,0)
#define RESET_KEY       MS_GPIO(0,2)
#define PYH_SELECT      MS_GPIO(0,6) 
#define PYH_SELECT_2    MS_GPIO(9,2) 

// Muxctrl configure
#define HI_MUX_BASE		(0x120f0000)
#define HI_MUX_NUM		(100)

#define I2C2_SDA_MUX		(56)		//muxreg28 bit[0] data 0x0
#define I2C2_SDA_MUX_DATA	(0x0)

#define I2C2_SCL_MUX		(57)		// muxreg29 bit[0] data 0x0
#define I2C2_SCL_MUX_DATA	(0x0)


/*
  *	DATA[7:0]: control by PADDR bit[9:2]
  *
  */
typedef unsigned long		Ulong;
typedef int            		Int32;
typedef short          		Int16;
typedef char           		Int8;
typedef unsigned int   		Uint32;
typedef unsigned short 		Uint16;
typedef unsigned char  		Uint8;
typedef volatile int		Vint32;
typedef volatile short		Vint16;
typedef volatile char		Vint8;
typedef volatile Uint32 	VU32;
typedef volatile Uint16		VU16;
typedef volatile Uint8		VU8; 

typedef struct {
	VU8		DATA[0x400];	// GPIO data control
	VU32	DIR;	// GPIO directory control register
	VU32	IS;		// GPIO interrupt trigger register
	VU32	IBE;	// GPIO dual edge-triggered interrupt register
	VU32	IEV;	// GPIO interrupt condition register
	VU32	IE;		// GPIO interrupt mask register
	VU32	RIS;	// GPIO interrupt Raw Status Register
	VU32	MIS;	// GPIO Masked interrupt status register
	VU32	IC;		// GPIO interrupt status clear register
	
}GPIO_REG_T;

#define GPIO_REG_SIZE	sizeof(GPIO_REG_T)

typedef enum
{
	GIO_MODE_INPUT = 0,
	GIO_MODE_OUTPUT,
}GIO_MODE_E;

typedef struct {
	VU32	mux[HI_MUX_NUM];
}MUX_REG_T;

#define MUX_REG_SIZE	sizeof(MUX_REG_T)

// use for debug information
//#define __DEBUG
#ifdef __DEBUG
#define __D(x...) \
	do { \
		printf("%s->%d: ", __FUNCTION__, __LINE__); \
		printf(x); \
		printf("\n"); \
	} while (0)
#else

#define __D(args...) do { } while (0)

#endif

#define __E(x...) \
	do { \
		printf("%s(%d): ", __FUNCTION__, __LINE__); \
		printf(x); \
		printf("\n"); \
	} while (0)


int gpio_init(void);
int gpio_deinit(void);
int gpio_mode(int gpio, GIO_MODE_E mode);
int gpio_read(int gpio);
int gpio_write(int gpio, int lv);
int gpio_mux(int mux,int lv);

int factory_reset(void);


#ifdef CONFIG_SOFT_I2C
#define I2C_TRISTATE    gpio_mode(I2C2_SDA_IO, GIO_MODE_INPUT)
#define I2C_ACTIVE  gpio_mode(I2C2_SDA_IO, GIO_MODE_OUTPUT)
#define I2C_INIT    gpio_init()
#define I2C_DELAY   udelay(3)
#define I2C_READ    gpio_read(I2C2_SDA_IO)
#define I2C_SCL(bit)  gpio_write(I2C2_SCL_IO, !!(bit))  
#define I2C_SDA(bit)  gpio_write(I2C2_SDA_IO, !!(bit))  
#define IRCUT_WRITE(bit)  gpio_write(IRCUT_IO, bit)  
#define IRCUT_READ  gpio_read(IRCUT_IO)  
#endif 


#endif


