/*
 * This header provides constants for binding novatek,na*-gpio.
 *
 * The first cell in Novetek's GPIO specifier is the GPIO ID. The macros below
 * provide names for this.
 *
 * The second cell contains standard flag values specified in gpio.h.
 */

#ifndef _DT_BINDINGS_GPIO_NVT_NA51103_GPIO_H
#define _DT_BINDINGS_GPIO_NVT_NA51103_GPIO_H

#include <dt-bindings/gpio/gpio.h>

/* GPIO pin number translation */
#define C_GPIO(pin)                  (pin)
#define J_GPIO(pin)                  (pin + 0x20)
#define P_GPIO(pin)                  (pin + 0x40)
#define E_GPIO(pin)                  (pin + 0x60)
#define D_GPIO(pin)                  (pin + 0x80)
#define S_GPIO(pin)                  (pin + 0xA0)

#define GPIO_RISING_EDGE             0x1
#define GPIO_FALLING_EDGE            0x0

#define GPIO_INTERRUPT               0x1
#define GPIO_POLLING                 0x0

#define GPIO_HIGH                    0x1
#define GPIO_LOW                     0x0

/* PAD pull pin number translation */
#define PAD_REG_TO_BASE(reg)         (((reg)/(4))*(32))
#define PAD_CGPIO(pin)               (2 * pin + PAD_REG_TO_BASE(0x00))
#define PAD_JGPIO(pin)               (2 * pin + PAD_REG_TO_BASE(0x04))
#define PAD_PGPIO(pin)               (2 * pin + PAD_REG_TO_BASE(0x08))
#define PAD_DGPIO(pin)               (2 * pin + PAD_REG_TO_BASE(0x18))
#define PAD_EGPIO(pin)               (2 * pin + PAD_REG_TO_BASE(0x20))
#define PAD_SGPIO(pin)               (2 * pin + PAD_REG_TO_BASE(0x28))

#define PAD_NONE                     0x00
#define PAD_PULLDOWN                 0x01
#define PAD_PULLUP                   0x02
#define PAD_KEEPER                   0x03

/* PAD driving pin number translation */
#define PAD_DS_CGPIO(pin)            (4 * pin + PAD_REG_TO_BASE(0x50))
#define PAD_DS_JGPIO(pin)            (4 * pin + PAD_REG_TO_BASE(0x58))
#define PAD_DS_PGPIO(pin)            (4 * pin + PAD_REG_TO_BASE(0x60))
#define PAD_DS_DGPIO(pin)            (4 * pin + PAD_REG_TO_BASE(0x80))
#define PAD_DS_EGPIO(pin)            (4 * pin + PAD_REG_TO_BASE(0x90))
#define PAD_DS_SGPIO(pin)            (4 * pin + PAD_REG_TO_BASE(0xA0))

#define PAD_DS_L0                    0x00
#define PAD_DS_L1                    0x01
#define PAD_DS_L2                    0x02
#define PAD_DS_L3                    0x03
#define PAD_DS_L4                    0x04
#define PAD_DS_L5                    0x05
#define PAD_DS_L6                    0x06
#define PAD_DS_L7                    0x07

#endif
