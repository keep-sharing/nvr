/******************************************************************************
 *	Flash Memory Controller Device Driver
 *	Copyright (c) 2014 - 2015 by Hisilicon
 *	All rights reserved.
 * ***
 *	Create by hisilicon
 *
 *****************************************************************************/

/*****************************************************************************/
#include <common.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <hifmc_common.h>

/*****************************************************************************/
unsigned int hifmc_ip_ver;
unsigned char hifmc_def_dev_type = FLASH_TYPE_DEFAULT;
unsigned char hifmc_ip_user;
unsigned char hifmc_cs_user[CONFIG_HIFMC_MAX_CS_NUM];

/*****************************************************************************/
int hifmc_ip_ver_check(void)
{
	if (hifmc_ip_ver == HIFMC_VER_100)
		return 0;

	printf("Check Flash Memory Controller v100 ... ");
	hifmc_ip_ver = readl(CONFIG_HIFMC_REG_BASE + FMC_VERSION);
	if (hifmc_ip_ver != HIFMC_VER_100) {
		printf("\n");
		return -EFAULT;
	}
	printf("Found\n");

	return 0;
}

/*****************************************************************************/
void hifmc_dev_type_check(unsigned char type)
{
	unsigned int reg, spi_device_type, flash_type;

	FMC_PR(BT_DBG, "\t|*-Start Check current device type\n");

	reg = readl((void *)(REG_BASE_SCTL + REG_SYSSTAT));
	FMC_PR(BT_DBG, "\t||-Get SYS STATUS[%#x]%#x\n", REG_SYSSTAT, reg);
	spi_device_type = GET_SPI_DEVICE_TYPE(reg);
	if (hifmc_def_dev_type == FLASH_TYPE_DEFAULT)
		hifmc_def_dev_type = spi_device_type;

	if (type == FLASH_TYPE_DEFAULT)
		spi_device_type = hifmc_def_dev_type;
	else if (spi_device_type != type) {
		FMC_PR(BT_DBG, "\t||-Boot media isn't SPI %s.\n",
				type ? "Nand" : "Nor");
		spi_device_type = type;
	}

	reg = readl((void *)(CONFIG_HIFMC_REG_BASE + FMC_CFG));
	FMC_PR(BT_DBG, "\t||-Get CFG[%#x]%#x\n", FMC_CFG, reg);
	flash_type = (reg & FLASH_SEL_MASK) >> FLASH_SEL_SHIFT;
	if (spi_device_type != flash_type) {
		reg &= ~FLASH_SEL_MASK;
		reg |= FMC_CFG_FLASH_SEL(spi_device_type);
	}

	writel(reg, (void *)(CONFIG_HIFMC_REG_BASE + FMC_CFG));
	FMC_PR(BT_DBG, "\t||-Set CFG[%#x]%#x\n", FMC_CFG, reg);

	FMC_PR(BT_DBG, "\t|*-End Check current device type\n");
}

/*****************************************************************************/
char *ulltostr(unsigned long long size)
{
	int ix;
	static char buffer[20];
	char *fmt[] = {"%u", "%uK", "%uM", "%uG", "%uT"};

	for (ix = 0; (ix < 5) && !(size & 0x3FF) && size; ix++)
		size = (size >> 10);

	sprintf(buffer, fmt[ix], size);
	return buffer;
}

/*****************************************************************************/
void debug_register_dump(void)
{
	int ix;
	unsigned int base = CONFIG_HIFMC_REG_BASE;

	printf("Register dump:");
	for (ix = 0; ix <= 0x98; ix += 0x04) {
		if (!(ix & 0x0F))
			printf("\n0x%08X: ", (base + ix));
		printf("%08X ", readl((void *)(base + ix)));
	}
	printf("\n");
}

