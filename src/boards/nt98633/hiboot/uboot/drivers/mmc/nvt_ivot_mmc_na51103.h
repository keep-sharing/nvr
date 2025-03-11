/*
 *  driver/mmc/nvt_ivot_mmc_na51103.h
 *
 *  Copyright:	Novatek Inc.
 *
 */

#ifndef __NVT_IVOT_MMC_NA51103_H__
#define __NVT_IVOT_MMC_NA51103_H__

#include <asm/arch/na51103_regs.h>
#include <linux/bitops.h>

/**************************
 *        Platform        *
 **************************/
/* SDIO controller number */
#define SDIO_HOST_ID_COUNT              (1)

/* SRCCLK */
#ifdef CONFIG_NVT_FPGA_EMULATION
#define SRCCLK_192MHZ                   (12000000)
#define SRCCLK_480MHZ                   (48000000)
#else
#define SRCCLK_192MHZ                   (192000000)
#define SRCCLK_480MHZ                   (480000000)
#endif

/* CLKSEL */
#define CG_SDIO_CLKSEL_REG              (IOADDR_CG_REG_BASE + 0x20)
#define CG_SDIO_CLKSEL_MASK             GENMASK(1, 0)
#define CG_SDIO_CLKSEL_SHIFT            (0)

#define _CLKSEL_192MHZ                  (0x0)
#define _CLKSEL_480MHZ                  (0x1)

/* CLKDIV */
#define CG_SDIO_CLKDIV_REG              (IOADDR_CG_REG_BASE + 0x40)
#define CG_SDIO_CLKDIV_MASK             GENMASK(10, 0)
#define CG_SDIO_CLKDIV_SHIFT            (0)

/* CLKEN */
#define CG_SDIO_CLKEN_REG               (IOADDR_CG_REG_BASE + 0x74)
#define CG_SDIO_CLKEN_MASK              BIT(11)

/* RSTN */
#define CG_SDIO_RSTN_REG                (IOADDR_CG_REG_BASE + 0x98)
#define CG_SDIO_RSTN_MASK               BIT(2)

/* PINMUX */
#define PIN_SDIO_CFG_SDIO_1             (0x1)
#define PIN_SDIO_CFG_SDIO_2             (0x2)

#define PIN_SDIO_CFG_SDIO2_1            (0x10)
#define PIN_SDIO_CFG_SDIO2_BUS_WIDTH    (0x20)
#define PIN_SDIO_CFG_SDIO2_DS           (0x40)

#define PIN_SDIO_CFG_SDIO3_1            (0x100)
#define PIN_SDIO_CFG_SDIO3_BUS_WIDTH    (0x200)
#define PIN_SDIO_CFG_SDIO3_DS           (0x400)

/* TOP */
#define TOP_SDIO_EN_REG                 (IOADDR_TOP_REG_BASE + 0x04)
#define TOP_SDIO_EN_MASK                BIT(4)
#define TOP_SDIO_EN_GPIO_REG            (IOADDR_TOP_REG_BASE + 0xA8)
#define TOP_SDIO_EN_GPIO_MASK           BIT(20) | BIT(21)
#define TOP_SDIO_EN_GPIO_2_REG          (IOADDR_TOP_REG_BASE + 0xB4)
#define TOP_SDIO_EN_GPIO_2_MASK         BIT(6) | BIT(7) | BIT(8) | BIT(9)

/* PAD */
#define PAD_PUPD0_REG_OFS               (0x00)

#define PAD_REG_TO_BASE(reg)            (((reg)/(4))*(32))
#define	PAD_DS_PGPIO_BASE               PAD_REG_TO_BASE(0x60)       // 0x60~0x74
#define	PAD_DS_DGPIO_BASE               PAD_REG_TO_BASE(0x80)       // 0x80~0x84

// 1st pinmux
#define PAD_DS_PGPIO14                  (PAD_DS_PGPIO_BASE + 56)
#define PAD_DS_PGPIO15                  (PAD_DS_PGPIO_BASE + 60)
#define PAD_DS_PGPIO17                  (PAD_DS_PGPIO_BASE + 68)
#define PAD_DS_PGPIO18                  (PAD_DS_PGPIO_BASE + 72)
#define PAD_DS_PGPIO19                  (PAD_DS_PGPIO_BASE + 76)
#define PAD_DS_PGPIO23                  (PAD_DS_PGPIO_BASE + 92)

// 2nd pinmux
#define PAD_DS_PGPIO20                  (PAD_DS_PGPIO_BASE + 80)
#define PAD_DS_PGPIO21                  (PAD_DS_PGPIO_BASE + 84)
#define PAD_DS_DGPIO6                   (PAD_DS_DGPIO_BASE + 24)
#define PAD_DS_DGPIO7                   (PAD_DS_DGPIO_BASE + 28)
#define PAD_DS_DGPIO8                   (PAD_DS_DGPIO_BASE + 32)
#define PAD_DS_DGPIO9                   (PAD_DS_DGPIO_BASE + 36)

#define PAD_DS_SDIO_CLK                 PAD_DS_DGPIO7
#define PAD_DS_SDIO_CMD                 PAD_DS_DGPIO6
#define PAD_DS_SDIO_D0                  PAD_DS_DGPIO8
#define PAD_DS_SDIO_D1                  PAD_DS_DGPIO9
#define PAD_DS_SDIO_D2                  PAD_DS_PGPIO20
#define PAD_DS_SDIO_D3                  PAD_DS_PGPIO21

/* Controlled as GPIO */
#define GPIO_SDIO_CLK                   D_GPIO(7)
#define GPIO_SDIO_CMD                   D_GPIO(6)
#define GPIO_SDIO_D0                    D_GPIO(8)
#define GPIO_SDIO_D1                    D_GPIO(9)
#define GPIO_SDIO_D2                    P_GPIO(20)
#define GPIO_SDIO_D3                    P_GPIO(21)

/* PAD pull */
#define PAD_PULLDOWN                    (0x1)

#define PAD_PUPD_SDIO_CLK_REG           (IOADDR_PAD_REG_BASE + 0x18)
#define PAD_PUPD_SDIO_CLK_MASK          GENMASK(15, 14)
#define PAD_PUPD_SDIO_CLK_SHIFT         (14)

#define PAD_PUPD_SDIO_CMD_REG           (IOADDR_PAD_REG_BASE + 0x18)
#define PAD_PUPD_SDIO_CMD_MASK          GENMASK(13, 12)
#define PAD_PUPD_SDIO_CMD_SHIFT         (12)

#define PAD_PUPD_SDIO_D0_REG            (IOADDR_PAD_REG_BASE + 0x18)
#define PAD_PUPD_SDIO_D0_MASK           GENMASK(17, 16)
#define PAD_PUPD_SDIO_D0_SHIFT          (16)

#define PAD_PUPD_SDIO_D1_REG            (IOADDR_PAD_REG_BASE + 0x18)
#define PAD_PUPD_SDIO_D1_MASK           GENMASK(19, 18)
#define PAD_PUPD_SDIO_D1_SHIFT          (18)

#define PAD_PUPD_SDIO_D2_REG            (IOADDR_PAD_REG_BASE + 0x0C)
#define PAD_PUPD_SDIO_D2_MASK           GENMASK(9, 8)
#define PAD_PUPD_SDIO_D2_SHIFT          (8)

#define PAD_PUPD_SDIO_D3_REG            (IOADDR_PAD_REG_BASE + 0x0C)
#define PAD_PUPD_SDIO_D3_MASK           GENMASK(11, 10)
#define PAD_PUPD_SDIO_D3_SHIFT          (10)


static inline uint32_t nvt_mmc_get_indly(uint8_t host_id, uint32_t freq)
{
    return 0;
}

static inline uint32_t nvt_mmc_get_outdly(uint8_t host_id, uint32_t freq)
{
    return 0;
}

#endif
