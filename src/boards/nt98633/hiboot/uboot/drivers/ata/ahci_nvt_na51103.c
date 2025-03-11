/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) Novatek Inc.
 */

#include "ahci_nvt.h"
#include <asm/arch/IOAddress.h>
#include <asm/arch/efuse_protected.h>

#define msleep(a) udelay(a * 1000)

static int nvt_sata_tune_en = 0;
static int rxclkinv;
static int nvt_sata_dma_opt = 0;
//static int nvt_ahci_hflag = AHCI_HFLAG_YES_NCQ;// | AHCI_HFLAG_YES_FBS;// // 0x4000
//static int nvt_sata_ddrabt[4] = {[0 ...(3)] = 0x0};

static void nvt_sata_phy_write(u32 val, void __iomem *phy_reg)
{
	if (nvt_sata_tune_en != 0) {
		printk("write phyreg:0x%lx val:0x%08x\r\n", (uintptr_t)(void __iomem *)phy_reg, val);
	}
	writel(val, phy_reg);
}

static u32 nvt_sata_phy_read(void __iomem *phy_reg)
{
	u32 val = 0;

	val = readl(phy_reg);
	if (nvt_sata_tune_en != 0) {
		printk("read phyreg:0x%lx val:0x%08x\r\n", (uintptr_t)(void __iomem *) phy_reg, val);
	}
	return val;
}

static inline void nvt_sata100_phy_parameter_setting(struct udevice *dev)
{
	struct ahci_nvt_plat_data *plat_data = dev_get_platdata(dev);
	u32 val = 0;

	if (rxclkinv) {
		//nvt_sata_phy_write(0x11, (plat_data->top_va_base + 0x30));
		val =  nvt_sata_phy_read((plat_data->top_va_base + 0x0));
		val |= (0x1 << 6);
		nvt_sata_phy_write(val, (plat_data->top_va_base + 0x0));
	}
	nvt_sata_phy_read((plat_data->top_va_base + 0x30));

	/* axi ch dis*/
	val = nvt_sata_phy_read((plat_data->top_va_base + 0x40));
	val &= ~(0x1 << 16);
	nvt_sata_phy_write(val, (plat_data->top_va_base + 0x40));

	nvt_sata_phy_write(0x40000000, (plat_data->top_va_base + 0x1C));

	nvt_sata_phy_write(0x0, (plat_data->top_va_base + 0x18));			/*mode0 */
	//nvt_sata_phy_write(0x80004C26, (plat_data->top_va_base + 0x18));	/*mode1 */
	//nvt_sata_phy_write(0x40004C26,  (plat_data->top_va_base + 0x18));	/*mode2 */

	if (nvt_sata_dma_opt) {
		nvt_sata_phy_write(0x100, (plat_data->top_va_base + 0x00));
	} else {
		nvt_sata_phy_write(0x0, (plat_data->top_va_base + 0x00));
	}

	nvt_sata_phy_write(nvt_sata_phy_read((plat_data->top_va_base + 0x20)) & ~(0xF << 20), (plat_data->top_va_base + 0x20));

	nvt_sata_phy_write(nvt_sata_phy_read((plat_data->top_va_base + 0x04)) & ~0x43, (plat_data->top_va_base + 0x04));
	msleep(5);

	// Release PHY_I2C_APB_RESETN
	nvt_sata_phy_write(nvt_sata_phy_read((plat_data->top_va_base + 0x04)) | 0x2, (plat_data->top_va_base + 0x04));
	msleep(5);
#if 0
	scontrol = readl(base + 0x12C);

	if ((scontrol & 0xF0) != 0x00) {
		nvt_sata_phy_write(0x04040000, (plat_data->phy_va_base + 0x850));
	}

	if ((scontrol & 0xF0) == 0x10) {
		// RATE_SET_1P5G
		nvt_sata_phy_write(0x0006c7d0, (plat_data->phy_va_base + 0x0C0));
		nvt_sata_phy_write(0x01430000, (plat_data->phy_va_base + 0x81C));
		nvt_sata_phy_write(0x04040008, (plat_data->phy_va_base + 0x850));
	}

	if ((scontrol & 0xF0) == 0x20) {
		// RATE_SET_3G
		nvt_sata_phy_write(0x0006c7d0, (plat_data->phy_va_base + 0x0C0));
		nvt_sata_phy_write(0x01420000, (plat_data->phy_va_base + 0x81C));
		nvt_sata_phy_write(0x04040004, (plat_data->phy_va_base + 0x850));
	}

	if ((scontrol & 0xF0) == 0x30) {
		// RATE_SET_6G
		nvt_sata_phy_write(0x0006c7d0, (plat_data->phy_va_base + 0x0C0));
		nvt_sata_phy_write(0x01410000, (plat_data->phy_va_base + 0x81C));
		nvt_sata_phy_write(0x04040000, (plat_data->phy_va_base + 0x850));
	}
	msleep(1);

	if (ssc) {
		//printk("TURN ON SSC\r\n");
		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x406C));
		val &= ~(0xFF << 0);
		val |= (0x79 << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x406C));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x40AC));
		val &= ~(0xFF << 0);
		val |= (0xC8 << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x40AC));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x40A8));
		val &= ~(0xFF << 0);
		val |= (0x0D << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x40A8));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x4098));
		val &= ~(0xFF << 0);
		val |= (0xC7 << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x4098));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x4098));
		val &= ~(0xFF << 0);
		val |= (0xCF << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x4098));

		val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x4098));
		val &= ~(0xFF << 0);
		val |= (0xC7 << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x4098));
	}

	nvt_sata_phy_write((0x06400000 + ((phy_txffepost1 & 0xF) << 0) +
			    ((phy_txffepost2 & 0x3) << 4) + ((phy_txffepre & 0x7) << 6) +
			    ((phy_txampboost & 0x3) << 9) + ((phy_txampredu & 0x1F) << 11) +
			    ((phy_txslewadj & 0x7) << 16)), (plat_data->phy_va_base + 0x820));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x834));
	val &= ~(0xF << 12);
	val |= (phy_losrefgendecoderbinin << 12);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x834));
#endif

	/* efuse */
	u32	trim;
	int code, code2;
	BOOL is_found;
	u32 BIAS_VLDOSEL_PI, BIAS_LDOSEL_PLL, BIAS_VREF_SEL, BIAS_RX_VLDO_SEL, TX_ZTX_CTL;

	code = otp_key_manager(OTP_SATA_TRIM_FIELD);
	code2 = otp_key_manager(OTP_SATA_REMAIN_TRIM_FIELD);

	if (code == -33 || code2 == -33) {
		/* Apply default */
		pr_err("%s: [%d][%d]: Read SATA trim error\r\n", __func__, OTP_SATA_TRIM_FIELD, OTP_SATA_REMAIN_TRIM_FIELD);

		//0x1B8[3:0] BIAS_VLDOSEL_PI  = 0xA
		//0x1B8[7:4] BIAS_LDOSEL_PLL  = 0xA
		val = nvt_sata_phy_read((plat_data->phy_va_base + 0x1B8));
		val &= ~(0xFF << 0);
		val |= (0xAA << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1B8));
		//0x1BC[3:0] BIAS_VREF_SEL    = 0xA
		val = nvt_sata_phy_read((plat_data->phy_va_base + 0x1BC));
		val &= ~(0xF << 0);
		val |= (0xA << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1BC));
		//0x174[3:0] BIAS_RX_VLDO_SEL = 0x4
		val = nvt_sata_phy_read((plat_data->phy_va_base + 0x174));
		val &= ~(0xF << 0);
		val |= (0x4 << 0);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x174));
		//TX_ZTX_CTL
		//SM: 0x00C0[7:4] = 4'b1111
		//SW: 0x0040[7:4] = 0x8
		val = nvt_sata_phy_read((plat_data->phy_va_base + 0xC0));
		val &= ~(0xF << 4);
		val |= (0xF << 4);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0xC0));
		val = nvt_sata_phy_read((plat_data->phy_va_base + 0x40));
		val &= ~(0xF << 4);
		val |= (0x8 << 4);
		nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x40));
	} else {
		is_found = extract_trim_valid(code, (u32 *)&trim);
		if (is_found) {
			/* Apply efuse trim data */
			BIAS_VLDOSEL_PI    = (trim) & 0xF;
			BIAS_RX_VLDO_SEL   = (trim  >> 4) & 0xF;
			BIAS_LDOSEL_PLL    = (trim  >> 8) & 0xF;
			BIAS_VREF_SEL      = (code2 >> 8) & 0xF;
			TX_ZTX_CTL         = (code2 >> 12) & 0xF;
			pr_err("%s: [%d][%d]: Apply SATA trim (is_found=%d, trim=0x%08x, code=0x%08x, code2=0x%08x)\r\n", __func__, OTP_SATA_TRIM_FIELD, OTP_SATA_REMAIN_TRIM_FIELD, is_found, (int)trim, (int)code, (int)code2);
			pr_warn("BIAS_VLDOSEL_PI  = 0x%04x, apply to 0x1B8[3:0]\r\n", (int)BIAS_VLDOSEL_PI);
			pr_warn("BIAS_RX_VLDO_SEL = 0x%04x, apply to 0x174[3:0]\r\n", (int)BIAS_RX_VLDO_SEL);
			pr_warn("BIAS_LDOSEL_PLL  = 0x%04x, apply to 0x1B8[7:4]\r\n", (int)BIAS_LDOSEL_PLL);
			pr_warn("BIAS_VREF_SEL    = 0x%04x, apply to 0x1BC[3:0]\r\n", (int)BIAS_VREF_SEL);
			pr_warn("TX_ZTX_CTL       = 0x%04x, apply to 0x40[7:4]\r\n", (int)TX_ZTX_CTL);

			//0x1B8[3:0] BIAS_VLDOSEL_PI  = sata_trim_data[0]=>[3:0]
			//0x1B8[7:4] BIAS_LDOSEL_PLL  = sata_trim_data[0]=>[11:8]
			val = nvt_sata_phy_read((plat_data->phy_va_base + 0x1B8));
			val &= ~(0xFF << 0);
			val |= (BIAS_VLDOSEL_PI << 0);
			val |= (BIAS_LDOSEL_PLL << 4);
			nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1B8));
			//0x1BC[3:0] BIAS_VREF_SEL    = sata_trim_data[0]=>[15:12]
			val = nvt_sata_phy_read((plat_data->phy_va_base + 0x1BC));
			val &= ~(0xF << 0);
			val |= (BIAS_VREF_SEL << 0);
			nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1BC));
			//0x174[3:0] BIAS_RX_VLDO_SEL = sata_trim_data[0]=>[7:4]
			val = nvt_sata_phy_read((plat_data->phy_va_base + 0x174));
			val &= ~(0xF << 0);
			val |= (BIAS_RX_VLDO_SEL << 0);
			nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x174));
			//TX_ZTX_CTL
			//SM: 0x00C0[7:4] = 4'b1111
			//SW: 0x0040[7:4] = sata_trim_data[1]=>[3:0]
			val = nvt_sata_phy_read((plat_data->phy_va_base + 0xC0));
			val &= ~(0xF << 4);
			val |= (0xF << 4);
			nvt_sata_phy_write(val, (plat_data->phy_va_base + 0xC0));
			val = nvt_sata_phy_read((plat_data->phy_va_base + 0x40));
			val &= ~(0xF << 4);
			val |= (TX_ZTX_CTL << 4);
			nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x40));
		} else {
			/* Apply default */
			pr_err("%s: [%d][%d]: Extract SATA trim error (is_found=%d, trim=0x%08x, code=0x%08x, code2=0x%08x)\r\n\r\n", __func__, OTP_SATA_TRIM_FIELD, OTP_SATA_REMAIN_TRIM_FIELD, is_found, (int)trim, (int)code, (int)code2);

			//0x1B8[3:0] BIAS_VLDOSEL_PI  = 0xA
			//0x1B8[7:4] BIAS_LDOSEL_PLL  = 0xA
			val = nvt_sata_phy_read((plat_data->phy_va_base + 0x1B8));
			val &= ~(0xFF << 0);
			val |= (0xAA << 0);
			nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1B8));
			//0x1BC[3:0] BIAS_VREF_SEL    = 0xA
			val = nvt_sata_phy_read((plat_data->phy_va_base + 0x1BC));
			val &= ~(0xF << 0);
			val |= (0xA << 0);
			nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1BC));
			//0x174[3:0] BIAS_RX_VLDO_SEL = 0x4
			val = nvt_sata_phy_read((plat_data->phy_va_base + 0x174));
			val &= ~(0xF << 0);
			val |= (0x4 << 0);
			nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x174));
			//TX_ZTX_CTL
			//SM: 0x00C0[7:4] = 4'b1111
			//SW: 0x0040[7:4] = 0x8
			val = nvt_sata_phy_read((plat_data->phy_va_base + 0xC0));
			val &= ~(0xF << 4);
			val |= (0xF << 4);
			nvt_sata_phy_write(val, (plat_data->phy_va_base + 0xC0));
			val = nvt_sata_phy_read((plat_data->phy_va_base + 0x40));
			val &= ~(0xF << 4);
			val |= (0x8 << 4);
			nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x40));
		}
	}

	// BEST_SETTING
	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2004));
	val &= ~(0xFF << 0);
	val |= (0x30 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2004));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x2014));
	val &= ~(0xFF << 0);
	val |= (0x1E << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x2014));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x1E8));
	val &= ~(0xFF << 0);
	val |= (0x02 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1E8));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x9F0));
	val &= ~(0x1 << 0);
	val |= (0x1 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x9F0));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x94C));
	val &= ~(0xFF << 0);
	val |= (0x88 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x94C));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x1CC));
	val &= ~(0xFF << 0);
	val |= (0x3F << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1CC));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x504));
	val &= ~(0xFF  << 0);
	val |= (0x08 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x504));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x1000));
	val &= ~(0xFF  << 0);
	val |= (0x07 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1000));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x18C));
	val &= ~(0xFF  << 0);
	val |= (0x08 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x18C));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x94));
	val &= ~(0xFF  << 0);
	val |= (0x07 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x94));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x8));
	val &= ~(0xFF  << 0);
	val |= (0x44 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x8));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x1BC));
	val &= ~(0x1  << 5);
	val |= (0x1 << 5);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1BC));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x3010));
	val &= ~(0x7  << 12);
	val |= (0x1 << 12);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x3010));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x5E4));
	val &= ~(0xFF  << 0);
	val |= (0x3F << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x5E4));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x5E8));
	val &= ~(0xFF  << 0);
	val |= (0x00 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x5E8));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x3010));
	val &= ~(0xFF  << 0);
	val |= (0x22 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x3010));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x5C));
	val &= ~(0xF  << 4);
	val |= (0x2 << 4);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x5C));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x3010));
	val &= ~(0xF  << 24);
	val |= (0x0 << 24);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x3010));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x64));
	val &= ~(0x3  << 5);
	val |= (0x0 << 5);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x64));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x3010));
	val &= ~(0xF  << 28);
	val |= (0x5 << 28);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x3010));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x68));
	val &= ~(0x3  << 5);
	val |= (0x1 << 5);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x68));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x10D8));
	val &= ~(0xFF  << 0);
	val |= (0x58 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x10D8));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x10DC));
	val &= ~(0x3  << 0);
	val |= (0x2 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x10DC));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x10D0));
	val &= ~(0xFF  << 0);
	val |= (0x2c << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x10D0));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x10D4));
	val &= ~(0x3  << 0);
	val |= (0x1 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x10D4));


	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x24C));
	val &= ~(0x1  << 6);
	val |= (0x1 << 6);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x24C));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x208));
	val &= ~(0x3  << 2);
	val |= (0x0 << 2);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x208));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x240));
	val &= ~(0x3  << 0);
	val |= (0x0 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x240));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x244));
	val &= ~(0x3  << 0);
	val |= (0x0 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x244));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x248));
	val &= ~(0x3  << 0);
	val |= (0x0 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x248));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x250));
	val &= ~(0xF  << 4);
	val |= (0x0 << 4);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x250));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x240));
	val &= ~(0xF  << 4);
	val |= (0x0 << 4);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x240));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x244));
	val &= ~(0xF  << 4);
	val |= (0x0 << 4);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x244));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x248));
	val &= ~(0xF  << 4);
	val |= (0x0 << 4);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x248));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x4C));
	val &= ~(0x1  << 2);
	val |= (0x0 << 2);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x4C));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x24C));
	val &= ~(0x1  << 7);
	val |= (0x1 << 7);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x24C));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x240));
	val &= ~(0x1  << 3);
	val |= (0x1 << 3);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x240));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x244));
	val &= ~(0x1  << 3);
	val |= (0x1 << 3);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x244));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x248));
	val &= ~(0x1  << 3);
	val |= (0x1 << 3);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x248));
/*
	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0xCC));
	val &= ~(0x1  << 5);
	val |= (0x1 << 5);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0xCC));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x4C));
	val &= ~(0x1  << 5);
	val |= (0x1 << 5);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x4C));
*/
	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x208));
	val &= ~(0x7  << 4);
	val |= (0x0 << 4);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x208));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x160));
	val &= ~(0x1  << 0);
	val |= (0x1 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x160));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x250));
	val &= ~(0xF  << 0);
	val |= (0xF << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x250));

	// update sata pll bandwidth setting
	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x1130));
	val &= ~(0xFF  << 0);
	val |= (0x35 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1130));

	val =  nvt_sata_phy_read((plat_data->phy_va_base + 0x1134));
	val &= ~(0xFF  << 0);
	val |= (0x06 << 0);
	nvt_sata_phy_write(val, (plat_data->phy_va_base + 0x1134));

	// Release PORn
	nvt_sata_phy_write((nvt_sata_phy_read(plat_data->top_va_base + 0x04) | 0x41), (plat_data->top_va_base + 0x04));
	msleep(10);

	return;
}

int nvtsata_platform_init(struct udevice *dev)
{
	struct ahci_nvt_plat_data *plat_data = dev_get_platdata(dev);

	if (!plat_data->base) {
		pr_err("Error base addr\r\n");
		return -1;
	}

	if (plat_data->base == (void __iomem *) IOADDR_SATA0_REG_BASE) {
		writel(readl(IOADDR_CG_REG_BASE + 0x00) | (1 << 5),		IOADDR_CG_REG_BASE + 0x00);					//Enable SATA phy(pll5) clk
		writel(readl(IOADDR_CG_REG_BASE + 0x98) & ~(1 << 10),		IOADDR_CG_REG_BASE + 0x98);					//Reset SATA0 clk
		writel(readl(IOADDR_CG_REG_BASE + 0x98) | (1 << 10),		IOADDR_CG_REG_BASE + 0x98);					//Release Reset SATA0 phy clk
		writel(readl(IOADDR_CG_REG_BASE + 0x74) | (1 << 19),		IOADDR_CG_REG_BASE + 0x74);					//Release SATA0 clk
		writel(readl(IOADDR_CG_REG_BASE + 0xE4) | (1 << 4),		IOADDR_CG_REG_BASE + 0xE4);					//Release SATA0 program clk
	} else if (plat_data->base == (void __iomem *) IOADDR_SATA1_REG_BASE) {
		writel(readl(IOADDR_CG_REG_BASE + 0x00) | (1 << 5),		IOADDR_CG_REG_BASE + 0x00);					//Enable SATA phy(pll5) clk
		writel(readl(IOADDR_CG_REG_BASE + 0x98) & ~(1 << 11),		IOADDR_CG_REG_BASE + 0x98);					//Reset SATA1 clk
		writel(readl(IOADDR_CG_REG_BASE + 0x98) | (1 << 11),		IOADDR_CG_REG_BASE + 0x98);					//Release Reset SATA1 phy clk
		writel(readl(IOADDR_CG_REG_BASE + 0x74) | (1 << 20),		IOADDR_CG_REG_BASE + 0x74);					//Release SATA1 clk
		writel(readl(IOADDR_CG_REG_BASE + 0xE4) | (1 << 5),		IOADDR_CG_REG_BASE + 0xE4);					//Release SATA1 program clk
	} else {
		pr_err("Wrong base addr\r\n");
		return -1;
	}

	nvt_sata100_phy_parameter_setting(dev);

	return 0;
}