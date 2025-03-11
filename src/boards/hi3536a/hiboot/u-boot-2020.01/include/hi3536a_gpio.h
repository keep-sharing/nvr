#ifndef __HI3536A_GPIO_H__
#define __HI3536A_GPIO_H__

#define HI_GPIO_BASE        (0x11090000)	// gpio寄存器的起始基地址
#define HI_GPIO_OFFSET      (0x1000)		// gpio每一组寄存器的偏移量
#define HI_GPIO_GROUP_NUM   (15)			// gpio共有15组
#define HI_GPIO_BIT_NUM     (8)				// 每组最大引脚数为8个

// pin defined
#define MS_GPIO(g,n)	(g*8+n)
#define MS_GROUP(gpio)	(gpio/8)
#define MS_BIT(gpio)	(gpio%8)
#define MS_PADDR(gpio)	(1 << (MS_BIT(gpio)+2))
#define MS_ENABLE		(1)

#define RESET_KEY_PRESSED (0)
#define BUZZ_ON           (1)
#define BUZZ_OFF          (0)
#define LED_OFF			  (0)
#define LED_ON			  (1)

#define STAT_GREEN_LED  MS_GPIO(13,7)
#define BUZZ_CTL       	MS_GPIO(2,0)
#define RESET_KEY       MS_GPIO(1,0)
#define PYH_SELECT      MS_GPIO(11,1)
#define PYH_SELECT_2    MS_GPIO(13,6) 

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
void gpio_setting(void);
int factory_reset(void);

#endif



