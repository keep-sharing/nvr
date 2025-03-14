/**

    @file       na51068_regs.h
    @ingroup
    @note
    Copyright   Novatek Microelectronics Corp. 2019.  All rights reserved.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as
    published by the Free Software Foundation.
*/

#ifndef __ASM_ARCH_NA51068_REGS_H_
#define __ASM_ARCH_NA51068_REGS_H_

/*
 * HAL IO macros.
 */
#define HAL_READ_UINT8( _register_, _value_ ) \
        ((_value_) = *((volatile unsigned char *)(_register_)))

#define HAL_WRITE_UINT8( _register_, _value_ ) \
        (*((volatile unsigned char *)(_register_)) = (_value_))

#define HAL_READ_UINT16( _register_, _value_ ) \
        ((_value_) = *((volatile unsigned short *)(_register_)))

#define HAL_WRITE_UINT16( _register_, _value_ ) \
        (*((volatile unsigned short *)(_register_)) = (_value_))

#define HAL_READ_UINT32( _register_, _value_ ) \
        ((_value_) = *((volatile unsigned int *)(_register_)))

#define HAL_WRITE_UINT32( _register_, _value_ ) \
        (*((volatile unsigned int *)(_register_)) = (_value_))

#define HAL_EDID_REG_CTRL	0xcc040214UL
/*********************************************************/
/*
 * DMA control register
 */
#define HAL_DMA_REG_BASE	0xcc0a0080UL

#define HAL_DMA_CHA_SRC_ADDR	(HAL_DMA_REG_BASE + 0x00)
#define HAL_DMA_CHA_DEST_ADDR	(HAL_DMA_REG_BASE + 0x04)
#define HAL_DMA_CHA_COUNT	(HAL_DMA_REG_BASE + 0x08)
#define HAL_DMA_CHA_CMD		(HAL_DMA_REG_BASE + 0x0c)

#define HAL_DMA_CHB_SRC_ADDR	(HAL_DMA_REG_BASE + 0x10)
#define HAL_DMA_CHB_DEST_ADDR	(HAL_DMA_REG_BASE + 0x14)
#define HAL_DMA_CHB_COUNT	(HAL_DMA_REG_BASE + 0x18)
#define HAL_DMA_CHB_CMD		(HAL_DMA_REG_BASE + 0x1c)

#define HAL_DMA_CHC_SRC_ADDR	(HAL_DMA_REG_BASE + 0x20)
#define HAL_DMA_CHC_DEST_ADDR	(HAL_DMA_REG_BASE + 0x24)
#define HAL_DMA_CHC_COUNT	(HAL_DMA_REG_BASE + 0x28)
#define HAL_DMA_CHC_CMD		(HAL_DMA_REG_BASE + 0x2c)

#define HAL_DMA_CHD_SRC_ADDR	(HAL_DMA_REG_BASE + 0x30)
#define HAL_DMA_CHD_DEST_ADDR	(HAL_DMA_REG_BASE + 0x34)
#define HAL_DMA_CHD_COUNT	(HAL_DMA_REG_BASE + 0x38)
#define HAL_DMA_CHD_CMD		(HAL_DMA_REG_BASE + 0x3c)

#define HAL_DMA_ENABLE			0x00000001UL
#define HAL_DMA_FINISH_STATUS		0x00000002UL
#define HAL_DMA_FINISH_INTR_ENABLE	0x00000004UL
#define HAL_DMA_BURST_MODE		0x00000008UL
#define HAL_DMA_ERR_RSP_STATUS		0x00000010UL
#define HAL_DMA_ERR_RSP_INTR_ENABLE	0x00000020UL
#define HAL_DMA_SRC_ADDR_BUS		0x00000040UL
#define HAL_DMA_DEST_ADDR_BUS		0x00000080UL
#define HAL_DMA_SRC_ADDR_INCR		0x00000700UL
#define HAL_DMA_DEST_ADDR_INCR		0x00007000UL
#define HAL_DMA_DATA_WIDTH		0x00300000UL
#define HAL_DMA_REQ_SIGNAL_SEL		0x0f000000UL

#define DMA_ENABLE			0x00000001UL
#define DMA_DISABLE			0x00000000UL

#define DMA_SRC_BURST_NO_INC		0x00000000UL
#define DMA_SRC_BURST_INC_1		0x00000100UL
#define DMA_SRC_BURST_INC_2		0x00000200UL
#define DMA_SRC_BURST_INC_4		0x00000300UL
#define DMA_SRC_BURST_DEC_1		0x00000500UL
#define DMA_SRC_BURST_DEC_2		0x00000600UL
#define DMA_SRC_BURST_DEC_4		0x00000700UL

#define DMA_DST_BURST_NO_INC		0x00000000UL
#define DMA_DST_BURST_INC_1		0x00001000UL
#define DMA_DST_BURST_INC_2		0x00002000UL
#define DMA_DST_BURST_INC_4		0x00003000UL
#define DMA_DST_BURST_DEC_1		0x00005000UL
#define DMA_DST_BURST_DEC_2		0x00006000UL
#define DMA_DST_BURST_DEC_4		0x00007000UL

#define DMA_SRC_ADDR_APB_BUS		0x00000000UL
#define DMA_SRC_ADDR_AHB_BUS		0x00000040UL
#define DMA_DEST_ADDR_APB_BUS		0x00000000UL
#define DMA_DEST_ADDR_AHB_BUS		0x00000080UL

#define DMA_DATA_WIDTH_WORD		0x00000000UL
#define DMA_DATA_WIDTH_HWORD		0x00100000UL
#define DMA_DATA_WIDTH_BYTE		0x00200000UL

#define DMA_REQ_SIGNAL_NO		0x00000000UL
#define DMA_REQ_SIGNAL_PB		0x01000000UL
#define DMA_REQ_SIGNAL_SDMMC		0x02000000UL
#define DMA_REQ_SIGNAL_CF		0x03000000UL
#define DMA_REQ_SIGNAL_SMC		0x04000000UL
#define DMA_REQ_SIGNAL_USB		0x05000000UL
#define DMA_REQ_SIGNAL_SDIO		0x05000000UL
#define DMA_REQ_SIGNAL_AUDIO		0x06000000UL
#define DMA_REQ_SIGNAL_SPDIF		0x06000000UL
#define DMA_REQ_SIGNAL_MSPRO		0x07000000UL
#define DMA_REQ_SIGNAL_SPI		0x09000000UL

/*
 * uart register
 */
#define HAL_UART_REG_BASE		0xFE200000UL
#define HAL_UART2_REG_BASE		0xFE220000UL
#define HAL_UART3_REG_BASE		0xFE240000UL
#define HAL_UART4_REG_BASE		0xFE260000UL

#define HAL_UARTX_CLKMHZ(_which_)	((_which_==0)?HAL_UART0_CLKMHZ:HAL_UART1_CLKMHZ)// (HAL_UART0_CLKMHZ + (_which_)*24000000)
#define HAL_UARTX_BASE(_which_)		((_which_<2)?(HAL_UART_REG_BASE + (_which_)*0x70000):(HAL_UART3_REG_BASE + (_which_ - 2)*0x70000))

#define HAL_UARTx_THR(_which_)		(HAL_UARTX_BASE(_which_) + 0x00)
#define HAL_UARTx_RBR(_which_)		(HAL_UARTX_BASE(_which_) + 0x00)
#define HAL_UARTx_DLABLO(_which_)	(HAL_UARTX_BASE(_which_) + 0x00)
#define HAL_UARTx_DLABHI(_which_)	(HAL_UARTX_BASE(_which_) + 0x04)
#define HAL_UARTx_IER(_which_)		(HAL_UARTX_BASE(_which_) + 0x04)
#define HAL_UARTx_IIR(_which_)		(HAL_UARTX_BASE(_which_) + 0x08)
#define HAL_UARTx_IFCR(_which_)		(HAL_UARTX_BASE(_which_) + 0x08)
#define HAL_UARTx_LCR(_which_)		(HAL_UARTX_BASE(_which_) + 0x0c)
#define HAL_UARTx_LSR(_which_)		(HAL_UARTX_BASE(_which_) + 0x14)
#define HAL_UARTx_SCRATCH(_which_)	(HAL_UARTX_BASE(_which_) + 0x1c)

#define HAL_UART0_CLKMHZ		48000000
#define HAL_UART0_BASE			HAL_UARTX_BASE(0)

#define HAL_UART0_THR			HAL_UARTx_THR(0)
#define HAL_UART0_RBR			HAL_UARTx_RBR(0)
#define HAL_UART0_DLABLO		HAL_UARTx_DLABLO(0)
#define HAL_UART0_DLABHI		HAL_UARTx_DLABHI(0)
#define HAL_UART0_IER			HAL_UARTx_IER(0)
#define HAL_UART0_IIR			HAL_UARTx_IIR(0)
#define HAL_UART0_IFCR			HAL_UARTx_IFCR(0)
#define HAL_UART0_LCR			HAL_UARTx_LCR(0)
#define HAL_UART0_LSR			HAL_UARTx_LSR(0)
#define HAL_UART0_SCRATCH		HAL_UARTx_SCRATCH(0)

#define HAL_UART1_CLKMHZ		48000000
#define HAL_UART1_BASE			HAL_UARTX_BASE(1)

#define HAL_UART1_THR			HAL_UARTx_THR(1)
#define HAL_UART1_RBR			HAL_UARTx_RBR(1)
#define HAL_UART1_DLABLO		HAL_UARTx_DLABLO(1)
#define HAL_UART1_DLABHI		HAL_UARTx_DLABHI(1)
#define HAL_UART1_IER			HAL_UARTx_IER(1)
#define HAL_UART1_IIR			HAL_UARTx_IIR(1)
#define HAL_UART1_IFCR			HAL_UARTx_IFCR(1)
#define HAL_UART1_LCR			HAL_UARTx_LCR(1)
#define HAL_UART1_LSR			HAL_UARTx_LSR(1)
#define HAL_UART1_SCRATCH		HAL_UARTx_SCRATCH(1)

/* bit mapping of IER register */
#define HAL_UART_IER_RXDATA		0x00000001UL
#define HAL_UART_IER_THREMPTY		0x00000002UL
#define HAL_UART_IER_LINESTATUS		0x00000004UL
#define HAL_UART_IER_INTMASK		0x00000007UL

/* bit mapping of IIR register */
#define HAL_UART_IIR_INT_PENDING	0x00000001UL
#define HAL_UART_IIR_INT_THREMPTY	0x00000002UL
#define HAL_UART_IIR_INT_RXDATA		0x00000004UL
#define HAL_UART_IIR_INT_LINESTATUS	0x00000006UL
#define HAL_UART_IIR_INT_TIMEOUT	0x0000000cUL
#define HAL_UART_IIR_INT_ID		0x0000000fUL

/* bit mapping of FCR register */
#define HAL_UART_FCR_FIFO_ENABLE	0x00000001UL
#define HAL_UART_FCR_RXFIFO_RST		0x00000002UL
#define HAL_UART_FCR_TXFIFO_RST		0x00000004UL
#define HAL_UART_FCR_MODE_DMA		0x00000008UL
#define HAL_UART_FCR_RXTRIGGER_LV	0x000000c0UL

#define FCR_RX_SHIFBIT			6
#define FCR_RXTRIGGER_LV1		(0x00 << FCR_RX_SHIFBIT)
#define FCR_RXTRIGGER_LV4		(0x01 << FCR_RX_SHIFBIT)
#define FCR_RXTRIGGER_LV8		(0x02 << FCR_RX_SHIFBIT)
#define FCR_RXTRIGGER_LV14		(0x03 << FCR_RX_SHIFBIT)

/* bit mapping of LCR register */
#define HAL_UART_LCR_CHARLENGTH		0x00000003UL
#define HAL_UART_LCR_STOPBIT		0x00000004UL
#define HAL_UART_LCR_PARITY_ENABLE	0x00000008UL
#define HAL_UART_LCR_PARITY_EVEN	0x00000010UL
#define HAL_UART_LCR_PARITY_STICK	0x00000020UL
#define HAL_UART_LCR_MODE_LOOPBACK	0x00000040UL
#define HAL_UART_LCR_DLAB		0x00000080UL

#define LCR_CHAR_LEN5			0x00000000UL
#define LCR_CHAR_LEN6			0x00000001UL
#define LCR_CHAR_LEN7			0x00000002UL
#define LCR_CHAR_LEN8			0x00000003UL

#define LCR_STOP1			0x00000000UL
#define LCR_STOP2			0x00000004UL

#define LCR_PARITY_DISABLE		0x00000000UL
#define LCR_PARITY_ENABLE		0x00000008UL

#define LCR_PARITY_STICK_DISABLE	0x00000000UL
#define LCR_PARITY_STICK_ENABLE		0x00000004UL

#define LCR_EVEN_PARITY			0x00000010
#define LCR_STICK_PARITY		0x00000020

/* bit mapping of LSR register */
#define HAL_UART_LSR_DATAREADY		0x00000001UL
#define HAL_UART_LSR_ERR_OVERRUN	0x00000002UL
#define HAL_UART_LSR_ERR_PARITY		0x00000004UL
#define HAL_UART_LSR_ERR_FRAMING	0x00000008UL
#define HAL_UART_LSR_THREMPTY		0x00000020UL
#define HAL_UART_LSR_TXEMPTY		0x00000040UL
#define HAL_UART_LSR_ERR_RXFIFO		0x00000080UL

#define CPU_PLL_T			0x62
#define AXI_PLL_T			0x6a
#define DDR_PLL_T			0x82

/* DDR config register */
#define REG_DDR_SETTING			*((volatile unsigned int*)0xBD010008)
#define DDR_SETTING_16BIT		(1 << 28)

static inline unsigned long read_periphbase(void)
{
	unsigned long val;
	__asm__ __volatile__(
				"mrc p15, 4, %0, c15, c0, 0\n\t"
				: "=r"(val)
				);
	return val;
}

#define SCU_BASE_ADDR                           0xF9C00000
#define GLOBAL_TIMER_BASE_ADDR                  (SCU_BASE_ADDR + 0x0200)
#define PRIVATE_TIMER_BASE_ADDR                 (SCU_BASE_ADDR + 0x0600)
#endif /* __ASM_ARCH_NA51068_REGS_H_ */
