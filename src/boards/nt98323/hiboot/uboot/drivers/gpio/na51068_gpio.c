/*
 *  gpio/na51068_gpio.c
 *
 *  Author:	Howard Chang
 *  Created:	April, 10 2020
 *  Copyright:	Novatek Inc.
 *
 */
#include <common.h>
#ifdef CONFIG_DM_GPIO
#include <dm.h>
#endif
#include <asm/io.h>
#include <asm/gpio.h>

#include <asm/nvt-common/rcw_macro.h>
#include <asm/arch/IOAddress.h>

#define CG_REG_ADDR(ofs)       (IOADDR_CG_REG_BASE+(ofs))
#define CG_GETREG(ofs)         INW(CG_REG_ADDR(ofs))
#define CG_SETREG(ofs,value)   OUTW(CG_REG_ADDR(ofs), (value))

#define NVT_GPIO_STG_DATA_0             (0x04)
#define NVT_GPIO_STG_DIR_0              (0x08)
#define NVT_GPIO_STG_SET_0              (0x10)
#define NVT_GPIO_STG_CLR_0              (0x14)

#ifdef CONFIG_DM_GPIO
struct nvt_gpio_bank {
	phys_addr_t base;
};
#endif

#ifndef CONFIG_DM_GPIO
static void gpio_init(void)
{
	static int token = 0;

	if (!token) {
		u32 reg_value;

		reg_value = CG_GETREG(0x58);
		reg_value |= ((0x1 << 27) | (0x1 << 28) | (0x1 << 29) | \
				(0x1 << 31));
		CG_SETREG(0x58, reg_value);

		reg_value = CG_GETREG(0x74);
		reg_value &= ~((0x1 << 3) | (0x1 << 4) | (0x1 << 5) | \
				(0x1 << 24));
		CG_SETREG(0x74, reg_value);

		token++;
	}
}
#endif

static void gpio_setreg(unsigned pin, u32 offset, u32 value)
{
#ifndef CONFIG_DM_GPIO
	gpio_init();
#endif
	if (pin < GPIO_1(0))
		OUTW(IOADDR_GPIO_REG_BASE + (offset), (value));
	else if ((pin >= GPIO_1(0)) && (pin < GPIO_2(0)))
		OUTW(IOADDR_GPIO1_REG_BASE + (offset), (value));
	else if ((pin >= GPIO_2(0)) && (pin < GPIO_3(0)))
		OUTW(IOADDR_GPIO2_REG_BASE + (offset), (value));
	else
		OUTW(IOADDR_GPIO3_REG_BASE + (offset), (value));
}

static u32 gpio_getreg(unsigned pin, u32 offset)
{
#ifndef CONFIG_DM_GPIO
	gpio_init();
#endif
	if (pin < GPIO_1(0))
		return INW(IOADDR_GPIO_REG_BASE+(offset));
	else if ((pin >= GPIO_1(0)) && (pin < GPIO_2(0)))
		return INW(IOADDR_GPIO1_REG_BASE+(offset));
	else if ((pin >= GPIO_2(0)) && (pin < GPIO_3(0)))
		return INW(IOADDR_GPIO2_REG_BASE+(offset));
	else
		return INW(IOADDR_GPIO3_REG_BASE+(offset));
}

static int gpio_validation(unsigned pin)
{
	if (pin > GPIO_3(31)) {
		printf("The gpio number is out of range\n");
		return -E_NOSPT;
	} else
		return 0;
}

#ifndef CONFIG_DM_GPIO
int gpio_request(unsigned offset, const char *label)
{
	return gpio_validation(offset);
}

int gpio_free(unsigned offset)
{
	return 0;
}

int gpio_direction_input(unsigned offset)
#else
int nvt_gpio_direction_input(struct udevice *dev, unsigned offset)
#endif
{
	u32 reg_data, shift;

	if (gpio_validation(offset) < 0)
		return -E_NOSPT;

	shift = offset & 0x1F;

	reg_data = gpio_getreg(offset, NVT_GPIO_STG_DIR_0);

	reg_data &= ~(1<<shift);   /*input*/

	gpio_setreg(offset, NVT_GPIO_STG_DIR_0, reg_data);

	return 0;
}

#ifndef CONFIG_DM_GPIO
int gpio_direction_output(unsigned offset, int value)
#else
int nvt_gpio_direction_output(struct udevice *dev, unsigned offset, int value)
#endif
{
	u32 reg_data, shift, tmp;

	if (gpio_validation(offset) < 0)
		return -E_NOSPT;

	shift = offset & 0x1F;

	tmp = (1<<shift);

	reg_data = gpio_getreg(offset, NVT_GPIO_STG_DIR_0);

	reg_data |= tmp;    /*output*/

	gpio_setreg(offset, NVT_GPIO_STG_DIR_0, reg_data);


	if (value)
		gpio_setreg(offset, NVT_GPIO_STG_SET_0, tmp);
	else
		gpio_setreg(offset, NVT_GPIO_STG_CLR_0, tmp);

	return 0;
}

#ifndef CONFIG_DM_GPIO
int gpio_get_value(unsigned offset)
#else
int nvt_gpio_get_value(struct udevice *dev, unsigned offset)
#endif
{
	u32 tmp, shift;

	if (gpio_validation(offset) < 0)
		return -E_NOSPT;

	shift = offset & 0x1F;

	tmp = (1<<shift);

	return (gpio_getreg(offset, NVT_GPIO_STG_DATA_0) & tmp) != 0;
}

#ifndef CONFIG_DM_GPIO
int gpio_set_value(unsigned offset, int value)
#else
int nvt_gpio_set_value(struct udevice *dev, unsigned offset, int value)
#endif
{
	u32 tmp, shift;

	printf("%s %d\n", __FUNCTION__, __LINE__);

	if (gpio_validation(offset) < 0)
		return -E_NOSPT;

	shift = offset & 0x1F;

	tmp = (1<<shift);

	if (value)
		gpio_setreg(offset, NVT_GPIO_STG_SET_0, tmp);
	else
		gpio_setreg(offset, NVT_GPIO_STG_CLR_0, tmp);

	return 0;
}

#ifdef CONFIG_DM_GPIO
static int nvt_gpio_get_function(struct udevice *dev, unsigned offset)
{
	u32 reg_data;

	if (gpio_validation(offset) < 0)
		return E_NOSPT;

	reg_data = gpio_getreg(offset, NVT_GPIO_STG_DIR_0);

	if (reg_data & (1<<offset))
		return GPIOF_OUTPUT;

	return GPIOF_INPUT;
}

static const struct dm_gpio_ops gpio_nvt_ops = {
	.direction_input	= nvt_gpio_direction_input,
	.direction_output	= nvt_gpio_direction_output,
	.get_value		= nvt_gpio_get_value,
	.set_value		= nvt_gpio_set_value,
	.get_function		= nvt_gpio_get_function,
};

static int nvt_gpio_probe(struct udevice *dev)
{
	static int token = 0;

	printf("%s\n", __FUNCTION__);

	if (!token) {
		u32 reg_value;

		reg_value = CG_GETREG(0x58);
		reg_value |= ((0x1 << 27) | (0x1 << 28) | (0x1 << 29) | \
				(0x1 << 31));
		CG_SETREG(0x58, reg_value);

		reg_value = CG_GETREG(0x74);
		reg_value &= ~((0x1 << 3) | (0x1 << 4) | (0x1 << 5) | \
				(0x1 << 24));
		CG_SETREG(0x74, reg_value);

		token++;
	}

	return 0;
}

static int nvt_gpio_ofdata_to_platdata(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	uc_priv->gpio_count = 128;
	uc_priv->bank_name = "nvt-gpio";

	return 0;
}


static const struct udevice_id nvt_gpio_ids[] = {
	{ .compatible = "nvt,nvt_gpio" },
	{ }
};

U_BOOT_DRIVER(gpio_msm) = {
	.name	= "gpio_nvt",
	.id	= UCLASS_GPIO,
	.of_match = nvt_gpio_ids,
	.ofdata_to_platdata = nvt_gpio_ofdata_to_platdata,
	.probe	= nvt_gpio_probe,
	.ops	= &gpio_nvt_ops,
	.priv_auto_alloc_size = sizeof(struct nvt_gpio_bank),
};
#endif

