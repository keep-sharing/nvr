#ifndef __NT98633_GPIO_H__
#define __NT98633_GPIO_H__

#define RESET_KEY_PRESSED (0)
#define BUZZ_ON           (0)
#define BUZZ_OFF          (1)
#define LED_OFF			  (1)
#define LED_ON			  (0)

#define STAT_GREEN_LED  P_GPIO(43)
#define BUZZ_CTL       	D_GPIO(0)
#define RESET_KEY       D_GPIO(1)
#define PYH_SELECT      S_GPIO(34)
#define PYH_SELECT_2    S_GPIO(35) 

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

int nt98633_gpio_read(unsigned pin);
void nt98633_gpio_write(unsigned pin, int lv);
void gpio_setting(void);
int factory_reset(void);

#endif



