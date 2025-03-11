/*
 *  driver/mmc/nvt_ivot_mmc_na51090.h
 *
 *  Copyright:	Novatek Inc.
 *
 */

#ifndef __NVT_IVOT_MMC_NA51090_H__
#define __NVT_IVOT_MMC_NA51090_H__

#include <asm/arch/na51090_regs.h>
#include <linux/bitops.h>

/**************************
 *        Platform        *
 **************************/
/* SDIO controller number */
#define SDIO_HOST_ID_COUNT              (2)

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

#define CG_SDIO2_CLKSEL_REG             (IOADDR_CG_REG_BASE + 0x20)
#define CG_SDIO2_CLKSEL_MASK            GENMASK(3, 2)
#define CG_SDIO2_CLKSEL_SHIFT           (2)

#define _CLKSEL_192MHZ                  (0x0)
#define _CLKSEL_480MHZ                  (0x1)

/* CLKDIV */
#define CG_SDIO_CLKDIV_REG              (IOADDR_CG_REG_BASE + 0x40)
#define CG_SDIO_CLKDIV_MASK             GENMASK(10, 0)
#define CG_SDIO_CLKDIV_SHIFT            (0)

#define CG_SDIO2_CLKDIV_REG             (IOADDR_CG_REG_BASE + 0x40)
#define CG_SDIO2_CLKDIV_MASK            GENMASK(26, 16)
#define CG_SDIO2_CLKDIV_SHIFT           (16)

/* CLKEN */
#define CG_SDIO_CLKEN_REG               (IOADDR_CG_REG_BASE + 0x74)
#define CG_SDIO_CLKEN_MASK              BIT(11)

#define CG_SDIO2_CLKEN_REG              (IOADDR_CG_REG_BASE + 0x74)
#define CG_SDIO2_CLKEN_MASK             BIT(12)

/* RSTN */
#define CG_SDIO_RSTN_REG                (IOADDR_CG_REG_BASE + 0x98)
#define CG_SDIO_RSTN_MASK               BIT(2)

#define CG_SDIO2_RSTN_REG               (IOADDR_CG_REG_BASE + 0x98)
#define CG_SDIO2_RSTN_MASK              BIT(3)

/* PINMUX */
#define PIN_SDIO_CFG_SDIO_1             (0x1)

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
#define TOP_SDIO_EN_GPIO_MASK           BIT(12) | BIT(13) | BIT(15) | BIT(16) | BIT(17) | BIT(18)

#define TOP_SDIO2_EN_REG                (IOADDR_TOP_REG_BASE + 0x04)
#define TOP_SDIO2_EN_MASK               BIT(8)
#define TOP_SDIO2_EN_GPIO_REG           (IOADDR_TOP_REG_BASE + 0xA0)
#define TOP_SDIO2_EN_GPIO_MASK          BIT(8) | BIT(9) | BIT(10) | BIT(11) | BIT(12) | BIT(13)

#define TOP_SDIO2_WIDTH_REG             (IOADDR_TOP_REG_BASE + 0x04)
#define TOP_SDIO2_WIDTH_MASK            BIT(10)
#define TOP_SDIO2_WIDTH_GPIO_REG        (IOADDR_TOP_REG_BASE + 0xA0)
#define TOP_SDIO2_WIDTH_GPIO_MASK       BIT(2) | BIT(3) | BIT(4) | BIT(5)

/* PAD */
#define PAD_PUPD0_REG_OFS               (0x00)

#define PAD_REG_TO_BASE(reg)            (((reg)/(4))*(32))
#define	PAD_DS_CGPIO_BASE               PAD_REG_TO_BASE(0x50)       // 0x50~0x54
#define	PAD_DS_PGPIO_BASE               PAD_REG_TO_BASE(0x60)       // 0x60~0x74

#define PAD_DS_CGPIO2                   (PAD_DS_CGPIO_BASE + 8)
#define PAD_DS_CGPIO3                   (PAD_DS_CGPIO_BASE + 12)
#define PAD_DS_CGPIO4                   (PAD_DS_CGPIO_BASE + 16)
#define PAD_DS_CGPIO5                   (PAD_DS_CGPIO_BASE + 20)
#define PAD_DS_CGPIO8                   (PAD_DS_CGPIO_BASE + 32)
#define PAD_DS_CGPIO9                   (PAD_DS_CGPIO_BASE + 36)
#define PAD_DS_CGPIO10                  (PAD_DS_CGPIO_BASE + 40)
#define PAD_DS_CGPIO11                  (PAD_DS_CGPIO_BASE + 44)
#define PAD_DS_CGPIO12                  (PAD_DS_CGPIO_BASE + 48)
#define PAD_DS_CGPIO13                  (PAD_DS_CGPIO_BASE + 52)
#define PAD_DS_PGPIO12                  (PAD_DS_PGPIO_BASE + 48)
#define PAD_DS_PGPIO13                  (PAD_DS_PGPIO_BASE + 52)
#define PAD_DS_PGPIO15                  (PAD_DS_PGPIO_BASE + 60)
#define PAD_DS_PGPIO16                  (PAD_DS_PGPIO_BASE + 64)
#define PAD_DS_PGPIO17                  (PAD_DS_PGPIO_BASE + 68)
#define PAD_DS_PGPIO18                  (PAD_DS_PGPIO_BASE + 72)

#define PAD_DS_SDIO_CLK                 PAD_DS_PGPIO12
#define PAD_DS_SDIO_CMD                 PAD_DS_PGPIO13
#define PAD_DS_SDIO_D0                  PAD_DS_PGPIO15
#define PAD_DS_SDIO_D1                  PAD_DS_PGPIO16
#define PAD_DS_SDIO_D2                  PAD_DS_PGPIO17
#define PAD_DS_SDIO_D3                  PAD_DS_PGPIO18

#define PAD_DS_SDIO2_CLK                PAD_DS_CGPIO8
#define PAD_DS_SDIO2_CMD                PAD_DS_CGPIO9
#define PAD_DS_SDIO2_D0                 PAD_DS_CGPIO10
#define PAD_DS_SDIO2_D1                 PAD_DS_CGPIO11
#define PAD_DS_SDIO2_D2                 PAD_DS_CGPIO12
#define PAD_DS_SDIO2_D3                 PAD_DS_CGPIO13
#define PAD_DS_SDIO2_D4                 PAD_DS_CGPIO2
#define PAD_DS_SDIO2_D5                 PAD_DS_CGPIO3
#define PAD_DS_SDIO2_D6                 PAD_DS_CGPIO4
#define PAD_DS_SDIO2_D7                 PAD_DS_CGPIO5

/* Controlled as GPIO */
#define GPIO_SDIO_CLK                   P_GPIO(12)
#define GPIO_SDIO_CMD                   P_GPIO(13)
#define GPIO_SDIO_D0                    P_GPIO(15)
#define GPIO_SDIO_D1                    P_GPIO(16)
#define GPIO_SDIO_D2                    P_GPIO(17)
#define GPIO_SDIO_D3                    P_GPIO(18)

/* PAD pull */
#define PAD_PULLDOWN                    (0x1)

#define PAD_PUPD_SDIO_CLK_REG           (IOADDR_PAD_REG_BASE + 0x08)
#define PAD_PUPD_SDIO_CLK_MASK          GENMASK(25, 24)
#define PAD_PUPD_SDIO_CLK_SHIFT         (24)

#define PAD_PUPD_SDIO_CMD_REG           (IOADDR_PAD_REG_BASE + 0x08)
#define PAD_PUPD_SDIO_CMD_MASK          GENMASK(27, 26)
#define PAD_PUPD_SDIO_CMD_SHIFT         (26)

#define PAD_PUPD_SDIO_D0_REG            (IOADDR_PAD_REG_BASE + 0x08)
#define PAD_PUPD_SDIO_D0_MASK           GENMASK(31, 30)
#define PAD_PUPD_SDIO_D0_SHIFT          (30)

#define PAD_PUPD_SDIO_D1_REG            (IOADDR_PAD_REG_BASE + 0x0C)
#define PAD_PUPD_SDIO_D1_MASK           GENMASK(1, 0)
#define PAD_PUPD_SDIO_D1_SHIFT          (0)
#define PAD_PUPD_SDIO_D2_REG            (IOADDR_PAD_REG_BASE + 0x0C)
#define PAD_PUPD_SDIO_D2_MASK           GENMASK(3, 2)
#define PAD_PUPD_SDIO_D2_SHIFT          (2)

#define PAD_PUPD_SDIO_D3_REG            (IOADDR_PAD_REG_BASE + 0x0C)
#define PAD_PUPD_SDIO_D3_MASK           GENMASK(5, 4)
#define PAD_PUPD_SDIO_D3_SHIFT          (4)

static inline uint32_t nvt_mmc_get_indly(uint8_t host_id, uint32_t freq)
{
    if (freq >= 192000000) {
        return 0x13;
    } else if (freq >= 160000000) {
        return 0x7;
    } else if (freq >= 120000000) {
        return 0x0;
    } else if (freq >= 96000000) {
        return 0x23;
    } else {
        return 0x0;
    }
}

static inline uint32_t nvt_mmc_get_outdly(uint8_t host_id, uint32_t freq)
{
    return 0;
}

#endif
