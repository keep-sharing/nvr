// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#include <config.h>
#include <common.h>
#include <asm/io.h>
#include <asm/arch/platform.h>
#include <spi_flash.h>
#include <nand.h>
#include <netdev.h>
#include <mmc.h>
#include <sdhci.h>
#include <asm/armv8/mmu.h>
#include <command.h>
#include <cpu_common.h>
#include <cpu_func.h>
#include <asm/mach-types.h>
#include <serial.h>
#include <linux/mtd/mtd.h>
#ifdef CONFIG_MS_NVR
#include "hi3536a_gpio.h"
#endif


static struct mm_region ss626v100_mem_map[] = {
	{
		.virt = 0x0UL,
		.phys = 0x0UL,
		.size = 0x05000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			PTE_BLOCK_NON_SHARE
	},
	{
		.virt = 0x05000000UL,
		.phys = 0x05000000UL,
		.size = 0x40000000UL - 0x05000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{
		.virt = 0x40000000UL,
		.phys = 0x40000000UL,
		.size = 0x200000000UL, /* PHYS_SDRAM_1_SIZE */
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			PTE_BLOCK_INNER_SHARE
	},
	{
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = ss626v100_mem_map;
static int boot_media = BOOT_MEDIA_UNKNOWN;

#if defined(CONFIG_SHOW_BOOT_PROGRESS)
void show_boot_progress(int progress)
{
	printf("Boot reached stage %d\n", progress);
}
#endif

#define COMP_MODE_ENABLE ((unsigned int)0x0000EAEF)

static inline void delay(unsigned long loops)
{
	__asm__ volatile ("1:\n"
			"subs %0, %1, #1\n"
			"bne 1b" : "=r" (loops) : "0" (loops));
}

int is_tee_enable_otp(void)
{
	if ((NUM_FF & readl(OTP_USER_LOCKABLE0)) == OTP_SOC_TEE_DISABLE_FLAG)
		return OTP_TEE_DISABLE;  /* tee is disable */
	else
		return OTP_TEE_ENABLE;  /* tee is enable */
}

int check_otp_cmd_mode(void)
{
	unsigned int el;
	if (is_tee_enable_otp() == OTP_TEE_ENABLE) {   /* tee is enable */
		asm volatile("mrs %0, CurrentEL" : "=r" (el) : : "cc");
		el >>= 2; /* Move Left 2 bit */
		if (el == EXCEPTION_LEVEL1)
			return OTP_REE_CMD_MODE;
		else if (el == EXCEPTION_LEVEL3)
			return OTP_TEE_CMD_MODE;
	}

	return OTP_TEE_CMD_MODE;   /* TEE mode */
}

/* get uboot start media. */
int get_boot_media(void)
{
	return boot_media;
}

static void boot_flag_init(void)
{
	unsigned int regval, boot_mode;

	/* get boot mode */
	regval = __raw_readl(SYS_CTRL_REG_BASE + REG_SYSSTAT);
	boot_mode = get_sys_boot_mode(regval);

	switch (boot_mode) {
		/* [3:2] 00b - boot from Spi Nor device */
	case BOOT_FROM_SPI:
		boot_media = BOOT_MEDIA_SPIFLASH;
		break;
	/* [3:2] 01b - boot from Spi Nand device */
	case BOOT_FROM_SPI_NAND:
		boot_media = BOOT_MEDIA_NAND;
		break;
	/* [3:2] 10b - boot from Nand device */
	case BOOT_FROM_NAND:
		boot_media = BOOT_MEDIA_NAND;
		break;
	/* [3:2] 11b - boot from emmc */
	case BOOT_FROM_EMMC:
		boot_media = BOOT_MEDIA_EMMC;
		break;
	default:
			boot_media = BOOT_MEDIA_UNKNOWN;
			break;
	}
}

int board_early_init_f(void)
{
	return 0;
}

#define UBOOT_DATA_ADDR     0x41000000UL
#define UBOOT_DATA_SIZE     0x80000UL
#if (CONFIG_AUTO_UPDATE == 1)
#if defined(CONFIG_FMC)
static int data_to_spiflash(void)
{
	static struct spi_flash *flash = NULL;
	void *buf = NULL;
	int spi_flash_erase_ret;
	ssize_t val;

	/* 0:bus  0:cs  1000000:max_hz  0x3:spi_mode */
	flash = spi_flash_probe(0, 0, 1000000, 0x3);
	if (!flash) {
		printf("Failed to initialize SPI flash\n");
		return -1;
	}

	/* erase the address range. */
	printf("Spi flash erase...\n");
	spi_flash_erase_ret = spi_flash_erase(flash, NUM_0, UBOOT_DATA_SIZE);
	if (spi_flash_erase_ret) {
		printf("SPI flash sector erase failed\n");
		return 1;
	}

	buf = map_physmem((unsigned long)UBOOT_DATA_ADDR,
			UBOOT_DATA_SIZE, MAP_WRBACK);
	if (!buf) {
		puts("Failed to map physical memory\n");
		return 1;
	}

	/* copy the data from RAM to FLASH */
	printf("Spi flash write...\n");
	val = flash->write(flash, NUM_0, UBOOT_DATA_SIZE, buf);
	if (val) {
		printf("SPI flash write failed, return %zd\n", val);
		unmap_physmem(buf, UBOOT_DATA_SIZE);
		return 1;
	}

	unmap_physmem(buf, UBOOT_DATA_SIZE);
	return 0; /* 0:success */
}

static int data_to_nandflash(void)
{
	struct mtd_info *nand_flash = NULL;
	void *buf = NULL;
	size_t length = UBOOT_DATA_SIZE;
	int val;

	nand_flash = nand_info[0];

	printf("Nand flash erase...\n");
	val = nand_erase(nand_flash, 0, UBOOT_DATA_SIZE);
	if (val) {
		printf("Nand flash erase failed\n");
		return 1;
	}

	buf = map_physmem((unsigned long)UBOOT_DATA_ADDR,
			UBOOT_DATA_SIZE, MAP_WRBACK);
	if (!buf) {
		puts("Failed to map physical memory\n");
		return 1;
	}

	printf("Nand flash write...\n");
	val = nand_write(nand_flash, 0, &length, buf);
	if (val) {
		printf("Nand flash write failed, return %d\n", val);
		unmap_physmem(buf, UBOOT_DATA_SIZE);
		return 1;
	}

	unmap_physmem(buf, UBOOT_DATA_SIZE);
	return 0;
}
#endif /* CONFIG_FMC */

#if defined(CONFIG_MMC)
static int data_to_emmc(void)
{
	struct mmc *mmc = find_mmc_device(0);
	void *buf = NULL;

	if (!mmc)
		return 1;

	(void)mmc_init(mmc);

	buf = map_physmem((unsigned long)UBOOT_DATA_ADDR,
			UBOOT_DATA_SIZE, MAP_WRBACK);
	if (!buf) {
		puts("Failed to map physical memory\n");
		return 1;
	}

	printf("MMC write...\n");
	blk_dwrite(mmc_get_blk_desc(mmc), 0, (UBOOT_DATA_SIZE >> NUM_9), buf);
	unmap_physmem(buf, UBOOT_DATA_SIZE);
	return 0;
}
#endif /* CONFIG_MMC */

static int save_bootdata_to_flash(void)
{
	unsigned int sd_update_flag;
	int ret;
	sd_update_flag = readl(REG_BASE_SCTL + REG_SC_GEN4);
	if (sd_update_flag == START_MAGIC) {
#if defined(CONFIG_FMC)
		if (boot_media == BOOT_MEDIA_SPIFLASH) {
			ret = data_to_spiflash();
			if (ret != 0)
				return ret;
		}
		if (boot_media == BOOT_MEDIA_NAND) {
			ret = data_to_nandflash();
			if (ret != 0)
				return ret;
		}
#endif
#if defined(CONFIG_MMC)
		if (boot_media == BOOT_MEDIA_EMMC) {
			ret = data_to_emmc();
			if (ret != 0)
				return ret;
		}
#endif

		printf("update success!\n");
	}

	return 0;
}
#endif

int auto_update_flag = 0;
int bare_chip_program = 0;

#define REG_BASE_GPIO0          0x11090000
#define GPIO0_0_DATA_OFST       0x4
#define GPIO_DIR_OFST		0x400
#define PROC_TIME_OUT           100
#define PROC_LOOP               5
#define DOWNLOAD_FLAG           0x1f
#define ACK     		0xAA

#if (CONFIG_AUTO_UPDATE == 1)
static int is_bare_program(void)
{
	return 1;
}
#endif

static bool is_get_ack(void)
{
	unsigned char cr;

	if (serial_tstc()) {
		cr = (unsigned char)serial_getc();
		if (cr == ACK)
			return true;
	}
	return false;
}

/* Connect to TOOLS */
static int uart_self_boot_check(void)
{
	uint32_t count, i;
	uint32_t timer_count;

	for (count = 0; count < PROC_LOOP; ++count) {
		for (i = 0; i < PROC_LOOP; i++)
			serial_putc((unsigned char) DOWNLOAD_FLAG);

		timer_count = 0;
		while (timer_count < PROC_TIME_OUT) {
			if (is_get_ack())
				return 1;
			timer_count++;
			udelay(100); /* delay 100 us */
		}
	}
	serial_putc((unsigned char) '\n');

	return 0;
}

static void set_bootloader_download_process_flag(void)
{
	uint32_t channel_type;

	if (readl(REG_START_FLAG) == START_MAGIC)
		return;

	channel_type = readl(REG_BASE_SCTL + REG_DATA_CHANNEL_TYPE);
	switch (channel_type) {
	case BOOT_SEL_PCIE:
		return;
	default:
		break;
	}

	/* set download flag */
	if (uart_self_boot_check()) {
		writel(START_MAGIC, REG_START_FLAG);
		return;
	}
}

#if (CONFIG_AUTO_UPDATE == 1)
static int is_auto_update(void)
{
#if (CONFIG_AUTO_SD_UPDATE == 1) || (CONFIG_AUTO_USB_UPDATE == 1)
	/* to add some judgement if neccessary */
	unsigned int  val[NUM_3];

	writel(0, REG_BASE_GPIO0 + GPIO_DIR_OFST);

	val[NUM_0] = readl(REG_BASE_GPIO0 + GPIO0_0_DATA_OFST);
	if (val[NUM_0])
		return 0;

	udelay(10000); /* delay 10000 us */
	val[NUM_1] = readl(REG_BASE_GPIO0 + GPIO0_0_DATA_OFST);
	udelay(10000); /* delay 10000 us */
	val[NUM_2] = readl(REG_BASE_GPIO0 + GPIO0_0_DATA_OFST);
	udelay(10000); /* delay 10000 us */

	if (val[NUM_0] == val[NUM_1] && val[NUM_1] == val[NUM_2] && val[NUM_0] == NUM_0)
		return 1;    /* update enable */
	else
		return 0;

#else
	return 0;
#endif
}
#endif

void set_pcie_para_ss626v100(void)
{
	unsigned int val;
	val = readl(SYS_CTRL_REG_BASE + SYS_SATA);
	if ((val & (0x3 << PCIE_MODE)) == 0) {
		/* X2 release phy reset */
		val = readl(CRG_REG_BASE + PERI_CRG98);
		val &= ((~(0x1 << PHY1_SRS_REQ)) & (~(0x1 << PHY0_SRS_REQ)));
		writel(val, CRG_REG_BASE + PERI_CRG98);

		/* X2 select phy reset from crg */
		val = readl(CRG_REG_BASE + PERI_CRG98);
		val |= (0x1 << PHY1_SRS_REQ_SEL) | (0x1 << PHY0_SRS_REQ_SEL);
		writel(val, CRG_REG_BASE + PERI_CRG98);
		mdelay(10); /* delay 10 ms */

		/*
		 *  X2 seperate_rate=1
		 */
		writel(0x90f, MISC_REG_BASE + MISC_CTRL5);
		writel(0x94f, MISC_REG_BASE + MISC_CTRL5);
		writel(0x90f, MISC_REG_BASE + MISC_CTRL5);
		writel(0x0, MISC_REG_BASE + MISC_CTRL5);
		writel(0x92f, MISC_REG_BASE + MISC_CTRL5);
		writel(0x96f, MISC_REG_BASE + MISC_CTRL5);
		writel(0x92f, MISC_REG_BASE + MISC_CTRL5);
		writel(0x0, MISC_REG_BASE + MISC_CTRL5);
		mdelay(10); /* delay 10 ms */

		/*
		 * X2 split_cp_dis
		 */
		writel(0xd11, MISC_REG_BASE + MISC_CTRL5);
		writel(0xd51, MISC_REG_BASE + MISC_CTRL5);
		writel(0xd11, MISC_REG_BASE + MISC_CTRL5);
		writel(0x0, MISC_REG_BASE + MISC_CTRL5);
		writel(0xd31, MISC_REG_BASE + MISC_CTRL5);
		writel(0xd71, MISC_REG_BASE + MISC_CTRL5);
		writel(0xd31, MISC_REG_BASE + MISC_CTRL5);
		writel(0x0, MISC_REG_BASE + MISC_CTRL5);
		mdelay(10); /* delay 10 ms */
	} else {
		/* X1 release phy reset */
		val = readl(CRG_REG_BASE + PERI_CRG98);
		val &= ~(0x1 << PHY0_SRS_REQ);
		writel(val, CRG_REG_BASE + PERI_CRG98);

		/* X1 select phy reset from crg */
		val = readl(CRG_REG_BASE + PERI_CRG98);
		val |= (0x1 << PHY0_SRS_REQ_SEL);
		writel(val, CRG_REG_BASE + PERI_CRG98);
		mdelay(10); /* delay 10 ms */

		/*
		 * X1 seperate_rate=1
		 */
		writel(0x90f, MISC_REG_BASE + MISC_CTRL5);
		writel(0x94f, MISC_REG_BASE + MISC_CTRL5);
		writel(0x90f, MISC_REG_BASE + MISC_CTRL5);
		writel(0x0, MISC_REG_BASE + MISC_CTRL5);
		mdelay(10); /* delay 10 ms */

		/*
		 * X1 split_cp_dis
		 */
		writel(0xd11, MISC_REG_BASE + MISC_CTRL5);
		writel(0xd51, MISC_REG_BASE + MISC_CTRL5);
		writel(0xd11, MISC_REG_BASE + MISC_CTRL5);
		writel(0x0, MISC_REG_BASE + MISC_CTRL5);
		mdelay(10); /* delay 10 ms */
	};
}

int misc_init_r(void)
{
#ifdef CONFIG_RANDOM_ETHADDR
	random_init_r();
#endif
	env_set("verify", "n");

#if (CONFIG_AUTO_UPDATE == 1)
	/* auto update flag */
	if (is_auto_update())
		auto_update_flag = 1;
	else
		auto_update_flag = 0;

	/* bare chip program flag */
	if (is_bare_program())
		bare_chip_program = 1;
	else
		bare_chip_program = 0;

#ifdef CFG_MMU_HANDLEOK
	dcache_stop();
#endif

#ifdef CFG_MMU_HANDLEOK
	dcache_start();
#endif

#endif /* CONFIG_AUTO_UPDATE */

#if (CONFIG_AUTO_UPDATE == 1)
	if (auto_update_flag)
		do_auto_update();
	if (bare_chip_program && !auto_update_flag)
		save_bootdata_to_flash();
#endif
	set_bootloader_download_process_flag();
	return 0;
}

#ifdef CONFIG_MS_NVR
#define VER_MSB       MS_GPIO(11,1)
#define VER_LSB       MS_GPIO(13,6)
MS_HW_VER ms_hw_ver = MS_HW_VER_NONE;

int ms_hw_version(void)
{
    unsigned int ver = 0;

    if (ms_hw_ver != MS_HW_VER_NONE) {
        return ms_hw_ver;
    }

    gpio_init();
    gpio_mode(VER_MSB, GIO_MODE_INPUT);
    gpio_mode(VER_LSB, GIO_MODE_INPUT);
    ver = gpio_read(VER_LSB);
    ver |= (gpio_read(VER_MSB) << 1);

    switch (ver) {
        case 0x03:
            ms_hw_ver = MS_HW_VER_8000;
            printf("board hardware : 8000\n");
            break;
        case 0x2:
            ms_hw_ver = MS_HW_VER_7000;
            printf("board hardware : 7000\n");
            break;
        default:
            ms_hw_ver = MS_HW_VER_NONE;
            printf("board hardware : unknown(0x%x)\n", ver);
            break;
    }

    return ms_hw_ver;
}

#endif /* CONFIG_MS_NVR */

int board_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->bd->bi_arch_number = MACH_TYPE_SS626V100;
	gd->bd->bi_boot_params = CFG_BOOT_PARAMS;

	boot_flag_init();
#ifdef CONFIG_MS_NVR
        ms_hw_version();
#endif /* CONFIG_MS_NVR */

	return 0;
}

int dram_init(void)
{
	DECLARE_GLOBAL_DATA_PTR;

	gd->ram_size = PHYS_SDRAM_1_SIZE;
	return 0;
}

void reset_cpu(ulong addr)
{
	writel(0x12345678, REG_BASE_SCTL + REG_SC_SYSRES);
	while (1);
}

int timer_init(void)
{
	/*
	 *  *Under uboot, 0xffffffff is set to load register,
	 *   * timer_clk = BUSCLK/2/256.
	 *    * e.g. BUSCLK = 50M, it will roll back after 0xffffffff/timer_clk
	 *     * = 43980s = 12hours
	 *      */
	__raw_writel(0, CFG_TIMERBASE + REG_TIMER_CONTROL);
	__raw_writel(~0, CFG_TIMERBASE + REG_TIMER_RELOAD);

	/* 32 bit, periodic */
	__raw_writel(CFG_TIMER_CTRL, CFG_TIMERBASE + REG_TIMER_CONTROL);

	return 0;
}

int board_eth_init(bd_t *bis)
{
	int rc = 0;

#ifdef CONFIG_GMACV300_ETH
	rc = gmac_initialize(bis);
#endif
	return rc;
}

#ifdef CONFIG_GENERIC_MMC
int board_mmc_init(bd_t *bis)
{
	int ret = 0;
	int dev_num = 0;

#ifdef CONFIG_MMC_SDHCI
	ret = sdhci_add_port(0, EMMC_BASE_REG, MMC_TYPE_MMC);
	if (ret == 0) {
		ret = bsp_mmc_init(dev_num);
		if (ret)
			printf("No EMMC device found !\n");
	}
	dev_num++;
#endif

#ifdef CONFIG_AUTO_SD_UPDATE
	if (is_auto_update()) {
		ret = sdhci_add_port(1, SDIO0_BASE_REG, MMC_TYPE_SD);
		if (ret)
			return ret;

		ret = bsp_mmc_init(dev_num);
		if (ret)
			printf("No SD device found !\n");
	}
#endif

	return ret;
}
#endif

static unsigned int npu_get_stable_power_cpm(void)
{
	unsigned int power_cpm = 0;
	unsigned int val;
	unsigned int mask = 1;

	do {
		power_cpm += mask;

		if (power_cpm > NPU_CPM_POWER_MAX_VAL)
			return 0;

		if (power_cpm == (mask << NPU_CPM_POWER_STEP) - 1)
			mask <<= NPU_CPM_POWER_STEP;

		writel(power_cpm, (uintptr_t)(SOC_SYS_BASE_ADDR + SYS_NPU_POWER_CPM_OFFSET));
		udelay(1); /* delay 1 us */

		val = (readl((uintptr_t)(SOC_SYS_BASE_ADDR + SYS_NPU_CPM_MA_VAL_OFFSET)) & NPU_CPM_MA_MASK);
	} while (val < NPU_CPM_MA_MIN_VAL || val > NPU_CPM_MA_MIN_MAX);

	return power_cpm;
}

int config_pi_defense_registers(void)
{
	unsigned int index;
	unsigned int times = 0;
	unsigned int val = 0;
	unsigned int reg_val = 0;

	reg_val = readl((uintptr_t)(SOC_SYS_BASE_ADDR + SYS_NPU_VOL_OFFSET));
	reg_val += NPU_VOL_OFFSET_VAL;
	writel(reg_val, (uintptr_t)(SOC_SYS_BASE_ADDR + SYS_NPU_VOL_OFFSET));
	udelay(NPU_DELAY_TIME_US);

	/* 1. FFS clk */
	writel(NPU_FFS_RESET_VAL, (uintptr_t)(SOC_CRG_BASE_ADDR + CRG_NPU_FFS_CLK_OFFSET));	/* FFS reset */
	writel(NPU_CLK_CTRL_VAL, (uintptr_t)(SOC_CRG_BASE_ADDR + CRG_NPU_CLK_CTRL_OFFSET));
	writel(NPU_DROP_FLAG_VAL, (uintptr_t)(SOC_NPU_TOP_BASE_ADDR + NPU_TOP_DROP_FLAG_OFFSET));

	/* 2. static config */
	writel(NPU_FFS_DEF_VAL, (uintptr_t)(SOC_CRG_BASE_ADDR + CRG_NPU_FFS_CONFIG_OFFSET));	/* FFS */
	writel(NPU_CPM_RESET_VAL, (uintptr_t)(SOC_CRG_BASE_ADDR + CRG_NPU_CPM_CLK_OFFSET));	/* CPM reset */
	writel(NPU_CPM_DEF_VAL, (uintptr_t)(SOC_SYS_BASE_ADDR + SYS_NPU_CPM_CONFIG_OFFSET));

	/* 3. detect ma */
	writel(NPU_CPM_UNRESET_VAL, (uintptr_t)(SOC_CRG_BASE_ADDR + CRG_NPU_CPM_CLK_OFFSET));	/* CPM unreset */
	reg_val = npu_get_stable_power_cpm();
	if (reg_val == 0)
		return 0;
	writel(reg_val, (uintptr_t)(SOC_SYS_BASE_ADDR + SYS_NPU_POWER_CPM_OFFSET));

	/* 4. Threshold */
	for (index = 0; index < NPU_GET_MA_TIMES; index++) {
		reg_val = (readl((uintptr_t)(SOC_SYS_BASE_ADDR + SYS_NPU_CPM_MA_VAL_OFFSET)) & NPU_CPM_MA_MASK);
		if (reg_val < NPU_CPM_MA_MIN_VAL || reg_val > NPU_CPM_MA_MIN_MAX)
			continue;
		val += reg_val;
		times++;
	}

	if (times == 0)
		return 0;

	val /= times;
	val = (val - NPU_CPM_THRESHOLD_DIFF) << NPU_CPM_THRESHOLD_BIT;
	val += readl((uintptr_t)(SOC_SYS_BASE_ADDR + SYS_NPU_CPM_CONFIG_OFFSET));
	writel(val, (uintptr_t)(SOC_SYS_BASE_ADDR + SYS_NPU_CPM_CONFIG_OFFSET));

	/* 5. freq enable */
	writel(NPU_FFS_UNRESET_VAL, (uintptr_t)(SOC_CRG_BASE_ADDR + CRG_NPU_FFS_CLK_OFFSET));	/* FFS unreset */
	udelay(NPU_CRG_DELAY_TIME);

	val = readl((uintptr_t)(SOC_CRG_BASE_ADDR + CRG_NPU_FFS_STATE_OFFSET));
	if ((val & NPU_FFS_STATE_MASK_VAL) == 0)
		writel(NPU_FFS_RESET_VAL, (uintptr_t)(SOC_CRG_BASE_ADDR + CRG_NPU_FFS_CLK_OFFSET));	/* FFS reset */

	writel(NPU_FFS_CLK_VAL, (uintptr_t)(SOC_CRG_BASE_ADDR + CRG_NPU_FFS_CONFIG_OFFSET));

	/* 6. npu clk disable */
	writel(NPU_CLK_DISABLE_VAL, (uintptr_t)(SOC_CRG_BASE_ADDR + CRG_NPU_CLK_CTRL_OFFSET));

	reg_val = readl((uintptr_t)(SOC_SYS_BASE_ADDR + SYS_NPU_VOL_OFFSET));
	reg_val -= NPU_VOL_OFFSET_VAL;
	writel(reg_val, (uintptr_t)(SOC_SYS_BASE_ADDR + SYS_NPU_VOL_OFFSET));

	return 0;
}

static int start_riscv(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long addr;

	if (argc < 2)
		return CMD_RET_USAGE;
	addr = simple_strtoul(argv[1], NULL, 16); /* 16 Hexadecimal */

	/* start up riscv */
	{
		writel(VMCU_CRG_VAL, REG_VMCU_CRG);

		printf ("## Starting RISCV UP at 0x%016lX ...\n", addr);
		__asm_flush_dcache_all();
		writel(addr, 0x13012000); // set start addr

		writel(0x1, 0x11022000); // jtag to mcu
		writel(0x1, 0x1301202c); // core wait
		writel(0x1, 0x1301203c); // rst

		writel(0x0, 0x1301202c); // unwait
		writel(0x0, 0x1301203c); // unrst
	}
	return 0;
}

static int do_watch_dog_reset(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int reg_val = 0;

	printf("watch dog reset ...\n");
	mdelay(10); /* delay 10 ms */
	reg_val = readl((uintptr_t)REG_MISC_CTRL3);
	reg_val |= WATCH_DOG_MODE;
	writel(reg_val, REG_MISC_CTRL3);
	writel(WATCH_DOG_LOAD_VAL, REG_BASE_WATCH_DOG);
	reg_val = readl((uintptr_t)WATCH_DOG_CONTROL);
	reg_val |= WATCH_DOG_ENABLE;
	writel(reg_val, WATCH_DOG_CONTROL);

	return 0;
}

U_BOOT_CMD(
	go_riscv, CONFIG_SYS_MAXARGS, 1,  start_riscv,
	"start riscv at address 'addr'",
	"addr [arg ...]\n    - start riscv application at address 'addr'\n"
	"      passing 'arg' as arguments"
);

U_BOOT_CMD(
	dog_reset, CONFIG_SYS_MAXARGS, 0, do_watch_dog_reset,
	"watchdog reset system",
	"watchdog reset \n"
);
