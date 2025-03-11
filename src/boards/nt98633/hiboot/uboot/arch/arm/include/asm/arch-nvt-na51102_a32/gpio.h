/*
 * Copyright Novatek Inc
 *
 * SPDX-License-Identifier:      GPL-2.0+
 */

#ifndef __ARCH_GPIO_H
#define __ARCH_GPIO_H

#define C_GPIO(pin)                     (pin)
#define L_GPIO(pin)                     (pin + 0x20)
#define P_GPIO(pin)                     (pin + 0x40)
#define HSI_GPIO(pin)                   (pin + 0x80)
#define D_GPIO(pin)                     (pin + 0xA0)
#define S_GPIO(pin)                     (pin + 0xC0)
#define DSI_GPIO(pin)                   (pin + 0xE0)
#define A_GPIO(pin)                     (pin + 0x100)

/*
 * Empty file - cmd_gpio.c requires this. The implementation
 * is in drivers/gpio/na51xxx_gpio.c instead of inlined here.
 */

#endif /* __ARCH_GPIO_H */
