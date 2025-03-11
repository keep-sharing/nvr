#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <asm/gpio.h>

#include <asm/nvt-common/rcw_macro.h>
#include <asm/arch/IOAddress.h>
#include <nt98633_gpio.h>

#define GPIO_GETREG(ofs)	INW(IOADDR_GPIO_REG_BASE+(ofs))
#define GPIO_SETREG(ofs, value)	OUTW(IOADDR_GPIO_REG_BASE +(ofs),(value))

#define NVT_GPIO_STG_DATA_0		(0)
#define NVT_GPIO_STG_DIR_0		(0x30)
#define NVT_GPIO_STG_SET_0		(0x60)
#define NVT_GPIO_STG_CLR_0		(0x90)

void m_delay(int n)
{
    while (n --) {
        udelay(1000);
    }

}

static int gpio_validation(unsigned pin)
{
	if ((pin < C_GPIO(0)) || (pin > C_GPIO(13) && pin < J_GPIO(0)) || \
	(pin > J_GPIO(5) && pin < P_GPIO(0)) || \
	(pin > P_GPIO(44) && pin < E_GPIO(0)) || \
	(pin > E_GPIO(31) && pin < D_GPIO(0)) || \
	(pin > D_GPIO(11) && pin < S_GPIO(0)) || \
	(pin > S_GPIO(83) && pin < B_GPIO(0)) || (pin > B_GPIO(16))) {
		debug("The gpio number is out of range\n");
		return E_NOSPT;
	} else
		return 0;
}

static int nvt_gpio_direction_input(unsigned offset)
{
	unsigned long reg_data;
	unsigned long ofs = (offset >> 5) << 2;

	if (gpio_validation(offset) < 0)
		return E_NOSPT;

	offset &= (32-1);

	reg_data = GPIO_GETREG(NVT_GPIO_STG_DIR_0 + ofs);

	reg_data &= ~(1<<offset);   /*input*/

	GPIO_SETREG(NVT_GPIO_STG_DIR_0 + ofs, reg_data);

	return 0;
}

static int nvt_gpio_direction_output(unsigned offset, int value)
{
	unsigned long reg_data, tmp;
	unsigned long ofs = (offset >> 5) << 2;

	if (gpio_validation(offset) < 0)
		return E_NOSPT;

	offset &= (32-1);

	tmp = (1<<offset);

	reg_data = GPIO_GETREG(NVT_GPIO_STG_DIR_0 + ofs);

	reg_data |= (1<<offset);    /*output*/

	GPIO_SETREG(NVT_GPIO_STG_DIR_0 + ofs, reg_data);

	if (value)
		GPIO_SETREG(NVT_GPIO_STG_SET_0 + ofs, tmp);
	else
		GPIO_SETREG(NVT_GPIO_STG_CLR_0 + ofs, tmp);

	return 0;
}

static int nvt_gpio_get_value(unsigned offset)
{
	unsigned long tmp;
	unsigned long ofs = (offset >> 5) << 2;

	if (gpio_validation(offset) < 0)
		return E_NOSPT;

	offset &= (32-1);

	tmp = (1<<offset);

	return (GPIO_GETREG(NVT_GPIO_STG_DATA_0 + ofs) & tmp) != 0;
}

static int nvt_gpio_set_value(unsigned offset, int value)
{
	unsigned long tmp;
	unsigned long ofs = (offset >> 5) << 2;

	if (gpio_validation(offset) < 0)
		return E_NOSPT;

	offset &= (32-1);

	tmp = (1<<offset);

	if (value)
		GPIO_SETREG(NVT_GPIO_STG_SET_0 + ofs, tmp);
	else
		GPIO_SETREG(NVT_GPIO_STG_CLR_0 + ofs, tmp);

	return 0;
}

int nt98633_gpio_read(unsigned pin)
{
    nvt_gpio_direction_input(pin);
    return nvt_gpio_get_value(pin);
}

void nt98633_gpio_write(unsigned pin, int lv)
{
    nvt_gpio_direction_output(pin, lv);
    nvt_gpio_set_value(pin, lv);
}

void buzz(int timeout)
{
    nt98633_gpio_write(BUZZ_CTL, BUZZ_ON);
    while (timeout--) {
        m_delay(100);
    }
    nt98633_gpio_write(BUZZ_CTL, BUZZ_OFF);
}

void gpio_setting(void)
{
    buzz(2);
    nt98633_gpio_write(STAT_GREEN_LED, LED_ON);
}

void restore_setting(void)
{
    run_command("mmc erase.part cfg", 0);
    printf("restore factory done ... \n");
    buzz(10);
}

int switch_setting(void)
{
    char *bootargs;
    char *bootcmd;
    char *p;

    bootargs = env_get("bootargs");
    if (!bootargs) {
        printf("Get bootargs fail!\n");
        return 0;
    }

    bootcmd = env_get("bootcmd");
    if (!bootcmd) {
        printf("Get bootcmd fail!\n");
        return 0;
    }

    if ((p = strstr(bootargs, "mmcblk1p6")) != NULL) {
        *(p + 8) = '7';
    } else if ((p = strstr(bootargs, "mmcblk1p7")) != NULL) {
        *(p + 8) = '6';
    } else {
        printf("bootargs is error\n");
        return 0;
    }


    if ((p = strstr(bootcmd, "ker1")) != NULL) {
        *(p + 3) = '2';
    } else if ((p = strstr(bootcmd, "ker2")) != NULL) {
        *(p + 3) = '1';
    } else {
        printf("bootcmd is error\n");
        return 0;
    }
    run_command("saveenv", 0);
    printf("switch fs_ker done ... \n");
    buzz(20);
    return 0;
}

int factory_reset(void)
{
    unsigned char i = 0;
    if (nt98633_gpio_read(RESET_KEY) != RESET_KEY_PRESSED) {
        return 0;
    } else {
        printf("The reset key is pressed !\n");
        for (i = 0; i < 30; i ++) {
            if (nt98633_gpio_read(RESET_KEY) == RESET_KEY_PRESSED) {
                if (i == 7) {
                    restore_setting();
                } else if (i == 26) {
                    switch_setting();
                    break;
                } else {
                    m_delay(500);
                    continue;
                }
            } else {
                break;
            }
        }
    }
    return 0;
}



