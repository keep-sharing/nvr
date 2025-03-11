static inline void emmc_sys_init(void)
{
	int index = 0;
	unsigned int tmp_reg;

	for (index = MMC_IOMUX_START_ADDR;
			index <= MMC_IOMUX_END_ADDR; index += 4) {
		tmp_reg = himci_readl(IO_CONFIG_REG_BASE + index);
		tmp_reg &= ~MMC_IOMUX_CTRL_MASK;
		tmp_reg |= MMC_IOMUX_CTRL;
		himci_writel(tmp_reg, IO_CONFIG_REG_BASE + index);
	}

	/* SDIO clock phase */
	tmp_reg = himci_readl(CRG_REG_BASE + REG_CRG49);
	tmp_reg &= ~SDIO0_CLK_SEL_MASK;
	tmp_reg |= SDIO0_CLK_SEL_49_5M; /* phasic move to xls table. */
	himci_writel(tmp_reg, CRG_REG_BASE + REG_CRG49);

	/* SDIO soft reset */
	tmp_reg |= SDIO0_SRST_REQ;
	himci_writel(tmp_reg, CRG_REG_BASE + REG_CRG49);
	delay(1000 * DELAY_US);
	tmp_reg &= ~SDIO0_SRST_REQ;
	tmp_reg |= SDIO0_CKEN;
	himci_writel(tmp_reg, CRG_REG_BASE + REG_CRG49);
}

