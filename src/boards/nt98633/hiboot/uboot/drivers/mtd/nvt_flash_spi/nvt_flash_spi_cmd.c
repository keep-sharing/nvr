#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <malloc.h>
#include "nvt_flash_spi_reg.h"
#include "nvt_flash_spi_int.h"


u32 nvt_flash_ecc_err_pages[100] = {0};

#if defined(CONFIG_NVT_FW_UPDATE_LED) && !defined(CONFIG_NVT_PWM)
#define OP_ERASE 0
#define OP_PROGRAM 1
int RESTORE_GPIO_DIR = 0;

/*      LED function for FW update*/
static void led_set_gpio_high(int operation)
{
	u32 gpio_reg;
	int ofs = NVT_LED_PIN / 32;
	int shift = NVT_LED_PIN & 0x1F;

	ofs = ofs * 0x4;

	/*Set gpio as high*/
	gpio_reg = INW(IOADDR_GPIO_REG_BASE + 0x20 + ofs);

	if (gpio_reg & (1 << shift)) {
		RESTORE_GPIO_DIR = 1;
	} else {
		gpio_reg |= (1 << shift);
		OUTW(IOADDR_GPIO_REG_BASE + 0x20 + ofs, gpio_reg);
	}

	OUTW(IOADDR_GPIO_REG_BASE + 0x40 + ofs, (1 << shift));

	/*Config duration*/
	if (operation) {
		mdelay(NVT_LED_PROGRAM_DURATION);
	} else {
		mdelay(NVT_LED_ERASE_DURATION);
	}
}

static void led_set_gpio_low(void)
{
	u32 gpio_reg;
	int ofs = NVT_LED_PIN / 32;
	int shift = NVT_LED_PIN & 0x1F;

	ofs = ofs * 0x4;
	/*Set gpio as low*/
	OUTW(IOADDR_GPIO_REG_BASE + 0x60 + ofs, (1 << shift));

	/*Force gpio direction as original config*/
	if (!(RESTORE_GPIO_DIR)) {
		gpio_reg = INW(IOADDR_GPIO_REG_BASE + 0x20 + ofs);
		gpio_reg &= ~(1 << shift);
		OUTW(IOADDR_GPIO_REG_BASE + 0x20 + ofs, gpio_reg);
		RESTORE_GPIO_DIR = 0;
	}
}
#endif

/*
    Unlock SPI NAND block protect
*/
static ER spi_nand_unlock_BP(struct drv_nand_dev_info *info)
{
	struct smc_setup_trans transParam;

	ER ret = E_OK;

	if (info->flash_info->spi_nand_status.bBlockUnlocked == FALSE) {

		/*
		___|cmd(0x1F)|___|SR(0x01)(row byte0)|___|SR(0x0)(row byte1)|____
		*/
		transParam.colAddr      = 0;
		transParam.rowAddr      = (SPINAND_FEATURE_BLOCK_LOCK << 8) \
								  | 0x00;
		transParam.fifoDir      = _FIFO_READ;
		transParam.transMode    = _PIO_MODE;
		transParam.type         = _CMD_ADDR;
		transParam.uiCS         = info->flash_info->chip_sel;

		nand_host_setup_transfer(info, &transParam, 0, 0, _SINGLE_PAGE);

		nand_hostSetupAddressCycle(info, NAND_2_ADDRESS_CYCLE_CNT, \
								   NAND_NOT_CONFIGED_ADDRESS_CYCLE_CNT);

		nand_host_send_command(info, SPINAND_CMD_SET_FEATURE, FALSE);

		ret = nand_cmd_wait_complete(info);

		if (ret == E_OK) {
			info->flash_info->spi_nand_status.bBlockUnlocked = TRUE;
		}
	}

	return ret;
}

static ER spinand_wait_status(struct drv_nand_dev_info *info, uint32_t feature, uint32_t sts_reg, uint8_t *status)
{
	struct smc_setup_trans transParam;
	union T_NAND_COMMAND_REG uiCmdCycle = {0};
	ER ret = E_OK;

	do {
		transParam.colAddr      = 0;
		transParam.rowAddr      = 0;
		transParam.fifoDir      = _FIFO_READ;
		transParam.transMode    = _PIO_MODE;
		transParam.type         = _AUTO_STATUS;
		transParam.uiCS         = info->flash_info->chip_sel;

		nand_host_setup_transfer(info, &transParam, 0, 0, _SINGLE_PAGE);
		uiCmdCycle.Bit.CMD_CYC_1ST = feature;
		uiCmdCycle.Bit.CMD_CYC_2ND = sts_reg;

		nand_hostSetupStatusCheckBit(info, NAND_STS_BIT_0, NAND_STS_BIT_0_EQUAL_0);

		nand_host_send_command(info, uiCmdCycle.Reg, FALSE);

		ret = nand_cmd_wait_complete(info);
	} while (0);

	*status = (UINT8)NAND_GETREG(info, NAND_CTRL_STS_REG_OFS);

	return ret;
}

/*
    SPI NAND read page(SPI Single page read)

    @return
        - @b E_OK: success
        - @b E_OACV: already closed
        - @b E_NOMEM: read range exceed flash page
        - @b Else: fail
*/
static ER spiNand_readPage(struct drv_nand_dev_info *info, uint32_t uiRowAddr,
						   uint32_t uiColAddr, uint8_t *pBuf, uint32_t uiBufSize)
{
	struct smc_setup_trans transParam;
	UINT8 ucStatus;
	uint32_t uiPageCnt;
	uint32_t *pulBuf_spare;
	unsigned long uiSpareAreaAddr;
	uint32_t j;

	uint32_t *pulBuf = (uint32_t *)pBuf;
	uintptr_t startAddr = (uintptr_t)pBuf;

	union T_NAND_COMMAND_REG  uiCmdCycle = {0};
	union T_NAND_COMMAND2_REG uiCmd2Cycle = {0};

	uint32_t uiNandSectionCountPerPage = (info->flash_info->page_size / 0x200);

	ER ret = E_OK;
	static int ecc_page_count = 0;

	NAND_DEBUG_MSG("readPage: row: 0x%x, col: 0x%x, size: 0x%x \n", uiRowAddr, uiColAddr, uiBufSize);

	do {
		uiPageCnt = uiBufSize / info->flash_info->page_size;

		if (uiPageCnt == 1 && ((uiColAddr + uiBufSize) > (info->flash_info->page_size + info->flash_info->oob_size))) {
			printf("col addr 0x%08x, buf size 0x%08x exceed 0x%08x + 0x%08x\r\n",
				   (int32_t)uiColAddr, (int32_t)uiBufSize, info->flash_info->page_size, info->flash_info->oob_size);
			ret = E_NOMEM;
			break;
		} else {
			if (uiBufSize % info->flash_info->page_size != 0 && uiColAddr != info->flash_info->page_size) {
				printf("uiBufSize[0x%08x] not multiple of page size[0x%08x]\r\n",
					   uiBufSize, info->flash_info->page_size);
				ret = E_SYS;
				break;
			}
		}

		// 2-plane flash
		if (info->flash_info->plane_flags && ((uiRowAddr & (0x1 << 6)) || (uiColAddr == info->flash_info->page_size))) {
			uiCmdCycle.Bit.CMD_CYC_1ST = SPINAND_CMD_PAGE_READ;
			transParam.colAddr = 0;
			transParam.rowAddr = uiRowAddr;
			transParam.fifoDir = _FIFO_READ;
			transParam.transMode = _PIO_MODE;
			transParam.type = _CMD_ADDR;
			transParam.uiCS = info->flash_info->chip_sel;

			NAND_DEBUG_MSG("2-plane selet \n");

			nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_NORMAL_DUPLEX_MODE, NAND_SPI_ORDER_MODE_1);
			nand_host_setup_transfer(info, &transParam, 0, 0, _SINGLE_PAGE);
			nand_hostSetupAddressCycle(info, NAND_ROW_ADDRESS_3_CYCLE_CNT, NAND_COL_ADDRESS_1_CYCLE_CNT);
			nand_host_send_command(info, uiCmdCycle.Reg, FALSE);

			ret = nand_cmd_wait_complete(info);
			if (ret != E_OK) {
				return ret;
			}

			ret = spinand_wait_status(info, SPINAND_CMD_GET_FEATURE, SPINAND_FEATURE_STATUS, &ucStatus);
			if (ret != E_OK) {
				return ret;
			}
#ifndef CONFIG_FLASH_ONLY_DUAL
#if (defined(CONFIG_TARGET_NA51103) || defined(CONFIG_TARGET_NA51103_A64))
			if (info->flash_info->spi_nand_status.bQuadProgram) {
				uiCmdCycle.Bit.CMD_CYC_1ST = SPINAND_CMD_CACHE_READX4;
				nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_QUAD_MODE, NAND_SPI_ORDER_MODE_1);
			} else {
				uiCmdCycle.Bit.CMD_CYC_1ST = SPINAND_CMD_CACHE_READX2;
				nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_DUAL_MODE, NAND_SPI_ORDER_MODE_1);
			}
#else
			uiCmdCycle.Bit.CMD_CYC_1ST = SPINAND_CMD_CACHE_READX4;

			nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_QUAD_MODE, NAND_SPI_ORDER_MODE_1);
#endif
#else       //define as CONFIG_FLASH_ONLY_DUAL
			uiCmdCycle.Bit.CMD_CYC_1ST = SPINAND_CMD_CACHE_READX2;

			nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_DUAL_MODE, NAND_SPI_ORDER_MODE_1);
#endif

#if defined(CONFIG_TARGET_NA51055) || defined(CONFIG_TARGET_NA51089)
			/*Disable clk_auto_gating before set cs_manual*/
			*(u32 *) 0xF00200B0 &= ~0x80000;
#endif

			nand_hostSetCSActive(info, NAND_SPI_CS_HIGH);

			nand_hostConfigChipSelOperationMode(info, NAND_SPI_CS_MANUAL_MODE);

			nand_hostSetCSActive(info, NAND_SPI_CS_LOW);

			transParam.colAddr      = 0;

			if (uiRowAddr & (0x1 << 6)) {
				if (info->flash_info->device_size == 512) {
					transParam.rowAddr      = (uiColAddr | (1 << 13)) << 8;
				} else {
					transParam.rowAddr = (uiColAddr | (1 << 12)) << 8;
				}
			} else {
				transParam.rowAddr = uiColAddr<<8;
			}

			transParam.fifoDir      = _FIFO_READ;
			transParam.transMode    = _PIO_MODE;
			transParam.type         = _CMD_ADDR;
			transParam.uiCS         = info->flash_info->chip_sel;

			// Wrap[1..0] read option
			if (uiColAddr == info->flash_info->page_size) {
				transParam.rowAddr |= (2 << 14) << 8;
			}

			nand_host_setup_transfer(info, &transParam, 0, 0, _SINGLE_PAGE);
			nand_hostSetupAddressCycle(info, NAND_ROW_ADDRESS_3_CYCLE_CNT, NAND_COL_ADDRESS_1_CYCLE_CNT);
			nand_host_send_command(info, uiCmdCycle.Reg, FALSE);

			ret = nand_cmd_wait_complete(info);
			if (ret != E_OK) {
				nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_NORMAL_DUPLEX_MODE, NAND_SPI_ORDER_MODE_1);
				return ret;
			}

			nand_hostSetNandType(info, NANDCTRL_SPI_NOR_TYPE);

			transParam.colAddr      = 0;
			transParam.rowAddr      = 0;
			transParam.fifoDir      = _FIFO_READ;
			transParam.transMode    = _DMA_MODE;
			transParam.type         = _SPI_READ_N_BYTES;
			transParam.uiCS         = info->flash_info->chip_sel;

			NAND_SETREG(info, NAND_DMASTART_REG_OFS, startAddr);

			if (uiColAddr == info->flash_info->page_size) {
				nand_host_setup_transfer(info, &transParam, 0, info->flash_info->oob_size, _SINGLE_PAGE);
			} else {
				nand_host_setup_transfer(info, &transParam, 1, info->flash_info->page_size + info->flash_info->oob_size, _SINGLE_PAGE);
			}

			if (nvt_get_chip_id() == CHIP_NA51103) {
				if (uiColAddr == info->flash_info->page_size) {
					flush_dcache_range((uintptr_t)pulBuf, (uintptr_t)(pulBuf + (info->flash_info->oob_size) / 4));
				} else {
					flush_dcache_range((uintptr_t)pulBuf, (uintptr_t)(pulBuf + (info->flash_info->page_size + info->flash_info->oob_size) / 4));
				}
			}
			nand_host_send_command(info, uiCmdCycle.Reg, TRUE);

			ret = nand_cmd_wait_complete(info);

			if (uiColAddr == info->flash_info->page_size) {
				invalidate_dcache_range((uintptr_t)pulBuf, (uintptr_t)(pulBuf + info->flash_info->oob_size / 4));
			} else {
				invalidate_dcache_range((uintptr_t)pulBuf, (uintptr_t)(pulBuf + (info->flash_info->page_size + info->flash_info->oob_size) / 4));
			}
			nand_hostSetCSActive(info, NAND_SPI_CS_HIGH);

			nand_hostConfigChipSelOperationMode(info, NAND_SPI_CS_AUTO_MODE);

#if defined(CONFIG_TARGET_NA51055) || defined(CONFIG_TARGET_NA51089)
			/*Enable clk_auto_gating after set cs_manual*/
			*(u32 *) 0xF00200B0 |= 0x80000;
#endif

			nand_hostSetNandType(info, NANDCTRL_SPI_NAND_TYPE);

			nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_NORMAL_DUPLEX_MODE, NAND_SPI_ORDER_MODE_1);

			if (uiColAddr != info->flash_info->page_size) {
				ucStatus = (UINT8)NAND_GETREG(info, NAND_CTRL_STS_REG_OFS);

				if (info->use_ecc == NANDCTRL_SPIFLASH_USE_ONDIE_ECC) {
					if (info->ecc_status_type == 1) {   // ecc_sts b[7..4] for XT26G01C
						if (ucStatus & SPINAND_ECC_STATUS_TYPE1_MASK) {
							// ECC uncorrectable
							if ((ucStatus & SPINAND_ECC_STATUS_TYPE1_MASK) == SPINAND_ECC_STATUS_TYPE1_UNCORR_ERR) {
								ret = E_CTX;
								printf("uncorrect ecc error sts 0x%08x, row Addr 0x%08x, col Addr 0x%08x\r\n", ucStatus, (uint32_t)uiRowAddr, (uint32_t)uiColAddr);
								break;
							} else {
								ret = ECC_CORRECTED;
								debug("%d-Bit errors were detected and corrected\n", (int)(ucStatus & SPINAND_ECC_STATUS_TYPE1_MASK) >> 4);
								debug("II sts 0x%08x, row Addr 0x%08x, col Addr 0x%08x\r\n", ucStatus, (uint32_t)uiRowAddr, (uint32_t)uiColAddr);
							}
						}
					} else {    // normal ecc_sts type, b[5..4]
						if (ucStatus & SPINAND_FEATURE_STATUS_ECC_MASK) {
							// ECC uncorrectable
							if ((ucStatus & SPINAND_FEATURE_STATUS_ECC_MASK) == SPINAND_FEATURE_STATUS_ECC_UNCORR_ERR) {
								ret = E_CTX;
								printf("uncorrect ecc error sts 0x%08x, row Addr 0x%08x, col Addr 0x%08x\r\n", ucStatus, (uint32_t)uiRowAddr, (uint32_t)uiColAddr);
								break;
							} else {
								ret = ECC_CORRECTED;
								debug("Bit errors were detected and corrected\n");
								debug("II sts 0x%08x, row Addr 0x%08x, col Addr 0x%08x\r\n", ucStatus, (uint32_t)uiRowAddr, (uint32_t)uiColAddr);
							}
						}
					}
				} else if (info->use_ecc == NANDCTRL_SPIFLASH_USE_INTERNAL_BCH_ECC) {
					if (ucStatus & NAND_PRI_ECC_STS) {
						NAND_DEBUG_MSG("Primary BCH ECC procedure\n");
						NAND_SETREG(info, NAND_CTRL_STS_REG_OFS, NAND_PRI_ECC_STS);

						nand_host_set_fifo_enable(info, FALSE);

						if (nand_host_correct_bch_ecc(info, (u8 *)pulBuf, info->flash_info->page_size / 0x200) != E_OK) {
							printf("bch%d uncorrect ecc error sts 0x%08x, row Addr 0x%08x, col Addr 0x%08x\r\n", info->flash_info->spare_size, (uint32_t)SPINAND_FEATURE_STATUS_ECC_UNCORR_ERR, uiRowAddr, uiColAddr);
							ret = E_CTX;
						} else {
							debug("bch%d Bit errors were detected and corrected\n", info->flash_info->spare_size);
							debug("  row Addr 0x%08x, col Addr 0x%08x\n", uiRowAddr, uiColAddr);
							invalidate_dcache_range((uintptr_t)pulBuf, (uintptr_t)(pulBuf + info->flash_info->page_size + info->flash_info->oob_size));
							ret = ECC_CORRECTED;
						}
					}

				} else {
					if (ucStatus & NAND_PRI_ECC_STS) {
						printf("Primary RS ECC procedure\n");
						NAND_SETREG(info, NAND_CTRL_STS_REG_OFS, NAND_PRI_ECC_STS);

						nand_host_set_fifo_enable(info, FALSE);

						if (nand_host_correct_reedsolomon_ecc(info, (u8 *)pulBuf, info->flash_info->page_size / 0x200) != E_OK) {
							printf("uncorrect ecc error sts 0x%08x, row Addr 0x%08x, col Addr 0x%08x\r\n", (uint32_t)SPINAND_FEATURE_STATUS_ECC_UNCORR_ERR, (uint32_t)uiRowAddr, (uint32_t)uiColAddr);
							ret = E_CTX;
						} else {
							invalidate_dcache_range((uintptr_t)pulBuf, (uintptr_t)(pulBuf + info->flash_info->page_size + info->flash_info->oob_size));
							ret = ECC_CORRECTED;
						}
					}
				}
			}
		} else {
			transParam.colAddr = uiColAddr;
			transParam.rowAddr = uiRowAddr;
			transParam.fifoDir = _FIFO_READ;
			transParam.transMode = _DMA_MODE;
			transParam.type = _DATA_RD3;
			transParam.uiCS = info->flash_info->chip_sel;

			//Read spare data
			if (uiColAddr == info->flash_info->page_size) {
				u32 module0_reg;
				if (GET_NAND_SPARE_SIZE(info->flash_info) != uiBufSize) {
					printf("Spare size [%2d]!= Fill buffer size[%2d]\r\n", GET_NAND_SPARE_SIZE(info->flash_info), uiBufSize);
					break;
				}
				NAND_DEBUG_MSG("Read spare data\r\n");

				if (info->use_ecc == NANDCTRL_SPIFLASH_USE_INTERNAL_RS_ECC) {
					module0_reg = NAND_GETREG(info, NAND_MODULE0_REG_OFS);
					module0_reg &= ~0x300;
					NAND_SETREG(info, NAND_MODULE0_REG_OFS, module0_reg | NAND_PRI_ECC_SPI_ON_DIE);
				}

				// Wrap[1..0] read option
				if (info->flash_info->plane_flags) {
					transParam.colAddr = uiColAddr | (0x2 << 14);
				}

				transParam.transMode    = _PIO_MODE;

				if (info->flash_info->spare_size) {
					nand_host_setup_transfer(info, &transParam, 0, GET_NAND_SPARE_SIZE(info->flash_info) * 2, _SINGLE_PAGE);
				} else {
					nand_host_setup_transfer(info, &transParam, 0, GET_NAND_SPARE_SIZE(info->flash_info), _SINGLE_PAGE);
				}

				uiCmdCycle.Bit.CMD_CYC_1ST = SPINAND_CMD_PAGE_READ;

				uiCmdCycle.Bit.CMD_CYC_2ND = SPINAND_CMD_GET_FEATURE;

				uiCmdCycle.Bit.CMD_CYC_3RD = SPINAND_FEATURE_STATUS;

#ifndef CONFIG_FLASH_ONLY_DUAL
#if (defined(CONFIG_TARGET_NA51103) || defined(CONFIG_TARGET_NA51103_A64))
				if (info->flash_info->spi_nand_status.bQuadProgram) {
					uiCmdCycle.Bit.CMD_CYC_4TH = SPINAND_CMD_CACHE_READX4;
					nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_QUAD_MODE, NAND_SPI_ORDER_MODE_1);
				} else {
					uiCmdCycle.Bit.CMD_CYC_4TH = SPINAND_CMD_CACHE_READX2;
					nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_DUAL_MODE, NAND_SPI_ORDER_MODE_1);
				}
#else
				uiCmdCycle.Bit.CMD_CYC_4TH = SPINAND_CMD_CACHE_READX4;

				nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_QUAD_MODE, NAND_SPI_ORDER_MODE_1);
#endif
#else
				uiCmdCycle.Bit.CMD_CYC_4TH = SPINAND_CMD_CACHE_READX2;

				nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_DUAL_MODE, NAND_SPI_ORDER_MODE_1);
#endif

				uiCmd2Cycle.Bit.CMD_CYC_5TH = SPINAND_CMD_GET_FEATURE;

				uiCmd2Cycle.Bit.CMD_CYC_6TH = SPINAND_FEATURE_STATUS;

				nand_hostSetupAddressCycle(info, NAND_ROW_ADDRESS_3_CYCLE_CNT, NAND_COL_ADDRESS_2_CYCLE_CNT);

				nand_hostSetupStatusCheckBit(info, NAND_STS_BIT_0, NAND_STS_BIT_0_EQUAL_0);

				NAND_SETREG(info, NAND_COMMAND2_REG_OFS, uiCmd2Cycle.Reg);

				nand_host_send_command(info, uiCmdCycle.Reg, TRUE);

				ret = nand_cmd_wait_complete(info);

				nand_host_set_cpu_access_spare(info, TRUE);

				uiSpareAreaAddr = (unsigned long)(IOADDR_NAND_REG_BASE + NAND_SPARE00_REG_OFS);

#if DUMP_NAND_DEBUG_MSG
				NAND_DEBUG_MSG("dbg dump spare sram:\r\n");
				for (j = 0; j < uiNandSectionCountPerPage * (1 + (info->flash_info->spare_size)); j++) {
					NAND_DEBUG_MSG("    0x%02x: 0x%08x 0x%08x 0x%08x 0x%08x \r\n", j * 0x10,  \
								   * (uint32_t *)(uiSpareAreaAddr + 0 + j * 0x10), \
								   * (uint32_t *)(uiSpareAreaAddr + 4 + j * 0x10), \
								   * (uint32_t *)(uiSpareAreaAddr + 8 + j * 0x10), \
								   * (uint32_t *)(uiSpareAreaAddr + 12 + j * 0x10));
				}
#endif

#if defined(CONFIG_TARGET_NA51102) || defined(CONFIG_TARGET_NA51102_A64)
				if ((info->flash_info->spare_size) && (info->flash_info->spare_align)) {
					if (uiNandSectionCountPerPage == 4) {
						NAND_DEBUG_MSG("read 2k page, swap oob:\r\n");

						nvt_writel(nvt_readl(uiSpareAreaAddr + 0x0), pulBuf + 0); // B0-11
						nvt_writel(nvt_readl(uiSpareAreaAddr + 0x4), pulBuf + 1);
						nvt_writel(nvt_readl(uiSpareAreaAddr + 0x8), pulBuf + 2);
						for (j = 0; j < 4; j++) {   // B12-59
							nvt_writel(nvt_readl(uiSpareAreaAddr + 0x40 + j * 4), pulBuf + j + 0x3);
							nvt_writel(nvt_readl(uiSpareAreaAddr + 0x50 + j * 4), pulBuf + j + 0x7);
							nvt_writel(nvt_readl(uiSpareAreaAddr + 0x60 + j * 4), pulBuf + j + 0xB);
						}
						nvt_writel(nvt_readl(uiSpareAreaAddr + 0x70), pulBuf + 0xF); //B60-63
					} else {
						NAND_DEBUG_MSG("read 4k page, swap oob:\r\n");
						for (j = 0; j < 6; j++) {   // B0-23
							nvt_writel(nvt_readl(uiSpareAreaAddr + 0x4 * j), pulBuf + j);
						}
						for (j = 0; j < 26; j++) {  // B24-127
							nvt_writel(nvt_readl(uiSpareAreaAddr + 0x80 + 0x4 * j), pulBuf + 6 + j);
						}
					}
#else
				if (0) {    // oob: fixed
#endif
				} else {
					if (uiNandSectionCountPerPage > 1) {
						for (j = 0; j < 4; j++) {
							nvt_writel(nvt_readl(uiSpareAreaAddr + 0 + j * 4), pulBuf + j +  0);
							nvt_writel(nvt_readl(uiSpareAreaAddr + 16 + j * 4), pulBuf + j +  4);
							nvt_writel(nvt_readl(uiSpareAreaAddr + 32 + j * 4), pulBuf + j +  8);
							nvt_writel(nvt_readl(uiSpareAreaAddr + 48 + j * 4), pulBuf + j + 12);
						}
						if (uiNandSectionCountPerPage > 4) {
							for (j = 0; j < 4; j++) {
								nvt_writel(nvt_readl(uiSpareAreaAddr + 0x40 + j * 4), pulBuf + j + 16);
								nvt_writel(nvt_readl(uiSpareAreaAddr + 0x50 + j * 4), pulBuf + j + 20);
								nvt_writel(nvt_readl(uiSpareAreaAddr + 0x60 + j * 4), pulBuf + j + 24);
								nvt_writel(nvt_readl(uiSpareAreaAddr + 0x70 + j * 4), pulBuf + j + 28);
							}
						}
					} else {
						nvt_writel(nvt_readl(uiSpareAreaAddr + 0x0), pulBuf + 0);
						nvt_writel(nvt_readl(uiSpareAreaAddr + 0x4), pulBuf + 1);
						nvt_writel(nvt_readl(uiSpareAreaAddr + 0x8), pulBuf + 2);
						nvt_writel(nvt_readl(uiSpareAreaAddr + 0xC), pulBuf + 3);

					}
				}

#if DUMP_NAND_DEBUG_MSG
				NAND_DEBUG_MSG("dbg dump spare buf:\r\n");
				for (j = 0; j < uiNandSectionCountPerPage; j++) {
					NAND_DEBUG_MSG("    0x%02x: 0x%08x 0x%08x 0x%08x 0x%08x \r\n", j * 0x10, \
								   nvt_readl(pulBuf + j * 0x4 + 0),
								   nvt_readl(pulBuf + j * 0x4 + 1),
								   nvt_readl(pulBuf + j * 0x4 + 2),
								   nvt_readl(pulBuf + j * 0x4 + 3));
				}
#endif

				nand_host_set_cpu_access_spare(info, FALSE);

				if (info->use_ecc == NANDCTRL_SPIFLASH_USE_INTERNAL_RS_ECC) {
					module0_reg = NAND_GETREG(info, NAND_MODULE0_REG_OFS);
					module0_reg &= ~0x300;
					NAND_SETREG(info, NAND_MODULE0_REG_OFS, module0_reg | NAND_PRI_ECC_RS);
				}
			} else {
				NAND_SETREG(info, NAND_DMASTART_REG_OFS, startAddr);

				nand_host_setup_transfer(info, &transParam, 1, info->flash_info->page_size, _SINGLE_PAGE);

				if (nvt_get_chip_id() == CHIP_NA51090 || nvt_get_chip_id() == CHIP_NA51102) {
					invalidate_dcache_range((uintptr_t)pulBuf, (uintptr_t)(pulBuf + (info->flash_info->page_size + info->flash_info->oob_size) / 4));
				}
				if (nvt_get_chip_id() == CHIP_NA51103) {
					flush_dcache_range((uintptr_t)pulBuf, (uintptr_t)(pulBuf + (info->flash_info->page_size + info->flash_info->oob_size) / 4));
				}

				uiCmdCycle.Bit.CMD_CYC_1ST = SPINAND_CMD_PAGE_READ;

				uiCmdCycle.Bit.CMD_CYC_2ND = SPINAND_CMD_GET_FEATURE;

				uiCmdCycle.Bit.CMD_CYC_3RD = SPINAND_FEATURE_STATUS;

#ifndef CONFIG_FLASH_ONLY_DUAL
#if (defined(CONFIG_TARGET_NA51103) || defined(CONFIG_TARGET_NA51103_A64))
				if (info->flash_info->spi_nand_status.bQuadProgram) {
					uiCmdCycle.Bit.CMD_CYC_4TH = SPINAND_CMD_CACHE_READX4;
					nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_QUAD_MODE, NAND_SPI_ORDER_MODE_1);
				} else {
					uiCmdCycle.Bit.CMD_CYC_4TH = SPINAND_CMD_CACHE_READX2;
					nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_DUAL_MODE, NAND_SPI_ORDER_MODE_1);
				}
#else
				uiCmdCycle.Bit.CMD_CYC_4TH = SPINAND_CMD_CACHE_READX4;
				nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_QUAD_MODE, NAND_SPI_ORDER_MODE_1);
#endif
#else
				uiCmdCycle.Bit.CMD_CYC_4TH = SPINAND_CMD_CACHE_READX2;

				nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_DUAL_MODE, NAND_SPI_ORDER_MODE_1);
#endif

				uiCmd2Cycle.Bit.CMD_CYC_5TH = SPINAND_CMD_GET_FEATURE;

				uiCmd2Cycle.Bit.CMD_CYC_6TH = SPINAND_FEATURE_STATUS;

				nand_hostSetupAddressCycle(info, NAND_ROW_ADDRESS_3_CYCLE_CNT, NAND_COL_ADDRESS_2_CYCLE_CNT);

				nand_hostSetupStatusCheckBit(info, NAND_STS_BIT_0, NAND_STS_BIT_0_EQUAL_0);

				NAND_SETREG(info, NAND_COMMAND2_REG_OFS, uiCmd2Cycle.Reg);

				nand_host_send_command(info, uiCmdCycle.Reg, TRUE);

				ret = nand_cmd_wait_complete(info);

				invalidate_dcache_range((uintptr_t)pulBuf, (uintptr_t)(pulBuf + (info->flash_info->page_size + info->flash_info->oob_size) / 4));

				nand_host_set_cpu_access_spare(info, TRUE);

				uiSpareAreaAddr = (unsigned long)(IOADDR_NAND_REG_BASE + NAND_SPARE00_REG_OFS);

				pulBuf_spare = (uint32_t *)(pulBuf + info->flash_info->page_size / 4);

#if DUMP_NAND_DEBUG_MSG
				NAND_DEBUG_MSG("dbg dump spare:\r\n");
				for (j = 0; j < uiNandSectionCountPerPage * (1 + (info->flash_info->spare_size)); j++) {
					NAND_DEBUG_MSG("    0x%02x: 0x%08x 0x%08x 0x%08x 0x%08x \r\n", j * 0x10,  \
								   * (uint32_t *)(uiSpareAreaAddr + 0 + j * 0x10), \
								   * (uint32_t *)(uiSpareAreaAddr + 4 + j * 0x10), \
								   * (uint32_t *)(uiSpareAreaAddr + 8 + j * 0x10), \
								   * (uint32_t *)(uiSpareAreaAddr + 12 + j * 0x10));
				}
#endif

#if defined(CONFIG_TARGET_NA51102) || defined(CONFIG_TARGET_NA51102_A64)
				if ((info->flash_info->spare_size) && (info->flash_info->spare_align)) {
					if (uiNandSectionCountPerPage == 4) {
						NAND_DEBUG_MSG("read 2k page, swap oob:\r\n");

						nvt_writel(nvt_readl(uiSpareAreaAddr + 0x0), pulBuf_spare + 0);
						nvt_writel(nvt_readl(uiSpareAreaAddr + 0x4), pulBuf_spare + 1);
						nvt_writel(nvt_readl(uiSpareAreaAddr + 0x8), pulBuf_spare + 2);

						for (j = 0; j < 4; j++) {
							nvt_writel(nvt_readl(uiSpareAreaAddr + 0x40 + j * 4), pulBuf_spare + j + 0x3);
							nvt_writel(nvt_readl(uiSpareAreaAddr + 0x50 + j * 4), pulBuf_spare + j + 0x7);
							nvt_writel(nvt_readl(uiSpareAreaAddr + 0x60 + j * 4), pulBuf_spare + j + 0xB);
						}
						nvt_writel(nvt_readl(uiSpareAreaAddr + 0x70), pulBuf_spare + 0xf);
					} else {
						NAND_DEBUG_MSG("read 4k page, swap oob:\r\n");
						for (j = 0; j < 6; j++) {   // B0-23
							nvt_writel(nvt_readl(uiSpareAreaAddr + 0x4 * j), pulBuf_spare + j);
						}
						for (j = 0; j < 26; j++) {  // B24-127
							nvt_writel(nvt_readl(uiSpareAreaAddr + 0x80 + 0x4 * j), pulBuf_spare + 6 + j);
						}
					}
#else
				if (0) {    // oob: fixed
#endif
				} else {
					if (uiNandSectionCountPerPage > 1) {
						for (j = 0; j < 4; j++) {
							nvt_writel(nvt_readl(uiSpareAreaAddr + 0 + j * 4), pulBuf_spare + j + 0);
							nvt_writel(nvt_readl(uiSpareAreaAddr + 16 + j * 4), pulBuf_spare + j + 4);
							nvt_writel(nvt_readl(uiSpareAreaAddr + 32 + j * 4), pulBuf_spare + j + 8);
							nvt_writel(nvt_readl(uiSpareAreaAddr + 48 + j * 4), pulBuf_spare + j + 12);
						}
						if (uiNandSectionCountPerPage > 4) {
							for (j = 0; j < 4; j++) {
								nvt_writel(nvt_readl(uiSpareAreaAddr + 0x40 + j * 4), pulBuf_spare + j + 16);
								nvt_writel(nvt_readl(uiSpareAreaAddr + 0x50 + j * 4), pulBuf_spare + j + 20);
								nvt_writel(nvt_readl(uiSpareAreaAddr + 0x60 + j * 4), pulBuf_spare + j + 24);
								nvt_writel(nvt_readl(uiSpareAreaAddr + 0x70 + j * 4), pulBuf_spare + j + 28);
							}
						}
					} else {
						nvt_writel(nvt_readl(uiSpareAreaAddr + 0x0), pulBuf_spare + 0);
						nvt_writel(nvt_readl(uiSpareAreaAddr + 0x4), pulBuf_spare + 1);
						nvt_writel(nvt_readl(uiSpareAreaAddr + 0x8), pulBuf_spare + 2);
						nvt_writel(nvt_readl(uiSpareAreaAddr + 0xC), pulBuf_spare + 3);
					}
				}

#if DUMP_NAND_DEBUG_MSG
				NAND_DEBUG_MSG("dbg dump spare buf:\r\n");
				for (j = 0; j < uiNandSectionCountPerPage; j++) {
					NAND_DEBUG_MSG("    0x%02x: 0x%08x 0x%08x 0x%08x 0x%08x \r\n", j * 0x10, \
								   nvt_readl(pulBuf_spare + j * 0x4 + 0),
								   nvt_readl(pulBuf_spare + j * 0x4 + 1),
								   nvt_readl(pulBuf_spare + j * 0x4 + 2),
								   nvt_readl(pulBuf_spare + j * 0x4 + 3));
				}
#endif

				nand_host_set_cpu_access_spare(info, FALSE);

				ucStatus = (UINT8)NAND_GETREG(info, NAND_CTRL_STS_REG_OFS);
				NAND_DEBUG_MSG("NAND get status: 0x%x\n", ucStatus);

				if (info->use_ecc == NANDCTRL_SPIFLASH_USE_ONDIE_ECC) {
					if (info->ecc_status_type == 1) {   // ecc_sts b[7..4] for XT26G01C
						if (ucStatus & SPINAND_ECC_STATUS_TYPE1_MASK) {
							// ECC uncorrectable
							if ((ucStatus & SPINAND_ECC_STATUS_TYPE1_MASK) == SPINAND_ECC_STATUS_TYPE1_UNCORR_ERR) {
								ret = E_CTX;
								printf("uncorrect ecc error sts 0x%08x, row Addr 0x%08x, col Addr 0x%08x\r\n", ucStatus, (uint32_t)uiRowAddr, (uint32_t)uiColAddr);
								printf("uncorrect ecc error block 0x%08x, page 0x%x\n", (uint32_t)(uiRowAddr & 0xFFFFFF40) / 64, (uint32_t)(uiRowAddr & 0xFFFFFF40) * 0x800);
								nvt_flash_ecc_err_pages[ecc_page_count] = uiRowAddr * 0x800;
								ecc_page_count++;
								break;
							} else {
								ret = ECC_CORRECTED;
								debug("%d-Bit errors were detected and corrected\n", (int)(ucStatus & SPINAND_ECC_STATUS_TYPE1_MASK) >> 4);
								debug("II sts 0x%08x, row Addr 0x%08x, col Addr 0x%08x\r\n", ucStatus, (uint32_t)uiRowAddr, (uint32_t)uiColAddr);
							}
						}
					} else {    // normal ecc_sts type, b[5..4]
						if (ucStatus & SPINAND_FEATURE_STATUS_ECC_MASK) {
							// ECC uncorrectable
							if ((ucStatus & SPINAND_FEATURE_STATUS_ECC_MASK) == SPINAND_FEATURE_STATUS_ECC_UNCORR_ERR) {
								ret = E_CTX;
								printf("uncorrect ecc error sts 0x%08x, row Addr 0x%08x, col Addr 0x%08x\r\n", ucStatus, (uint32_t)uiRowAddr, (uint32_t)uiColAddr);
								printf("uncorrect ecc error block 0x%08x, page 0x%x\n", (uint32_t)(uiRowAddr & 0xFFFFFF40) / 64, (uint32_t)(uiRowAddr & 0xFFFFFF40) * 0x800);
								nvt_flash_ecc_err_pages[ecc_page_count] = uiRowAddr * 0x800;
								ecc_page_count++;
								break;
							} else {
								ret = ECC_CORRECTED;
								debug("Bit errors were detected and corrected\n");
								debug("II sts 0x%08x, row Addr 0x%08x, col Addr 0x%08x\r\n", ucStatus, (uint32_t)uiRowAddr, (uint32_t)uiColAddr);
							}
						}
					}
				} else if (info->use_ecc == NANDCTRL_SPIFLASH_USE_INTERNAL_BCH_ECC) {
					if (NAND_GETREG(info, NAND_CTRL_STS_REG_OFS) & NAND_PRI_ECC_STS) {
						NAND_DEBUG_MSG("Primary BCH ECC procedure\n");
						NAND_SETREG(info, NAND_CTRL_STS_REG_OFS, NAND_PRI_ECC_STS);

						nand_host_set_fifo_enable(info, FALSE);

						if (nand_host_correct_bch_ecc(info, (u8 *)pulBuf, info->flash_info->page_size / 0x200) != E_OK) {
							printf("bch%d uncorrect ecc error sts 0x%08x, row Addr 0x%08x, col Addr 0x%08x\r\n", info->flash_info->spare_size, (uint32_t)SPINAND_FEATURE_STATUS_ECC_UNCORR_ERR, uiRowAddr, uiColAddr);
							ret = E_CTX;
						} else {
							debug("bch%d Bit errors were detected and corrected\n", info->flash_info->spare_size);
							debug("  row Addr 0x%08x, col Addr 0x%08x\n", uiRowAddr, uiColAddr);
							invalidate_dcache_range((uintptr_t)pulBuf, (uintptr_t)(pulBuf + (info->flash_info->page_size + info->flash_info->oob_size) / 4));
							ret = ECC_CORRECTED;
						}
					}
				} else {
					if (ucStatus & NAND_PRI_ECC_STS) {
						printf("Primary RS ECC procedure\n");
						NAND_SETREG(info, NAND_CTRL_STS_REG_OFS, NAND_PRI_ECC_STS);

						nand_host_set_fifo_enable(info, FALSE);

						if (nand_host_correct_reedsolomon_ecc(info, (u8 *)pulBuf, info->flash_info->page_size / 0x200) != E_OK) {
							printf("uncorrect ecc error sts 0x%08x, row Addr 0x%08x, col Addr 0x%08x\r\n", (uint32_t)SPINAND_FEATURE_STATUS_ECC_UNCORR_ERR, (uint32_t)uiRowAddr, (uint32_t)uiColAddr);
							ret = E_CTX;
						} else {
							invalidate_dcache_range((uintptr_t)pulBuf, (uintptr_t)(pulBuf + (info->flash_info->page_size + info->flash_info->oob_size) / 4));
							ret = ECC_CORRECTED;
						}
					}
				}
			}
		}
	}
	while (0);
	return ret;
}

/*
    SPI NAND program page

    @param[in] uiRowAddr        row address
    @param[in] uiColAddr        column address
    @param[in] pBuf             buffer to be programmed
    @param[in] uiBufSize        buffer size
    @param[in] pSpareBuf        buffer to be programmed on spare
    @param[in] uiSpareSize      buffer size of pSpareBuf

    @return
        - @b E_OK: success
        - @b E_OACV: already closed
        - @b E_NOMEM: program range exceed flash page
        - @b Else: fail
*/
static ER spiNand_programPage(struct drv_nand_dev_info *info, uint32_t uiRowAddr, uint32_t uiColAddr, UINT8 *pBuf, uint32_t uiBufSize, UINT8 *pSpareBuf, UINT32 uiSpareSize)
{
	struct smc_setup_trans  transParam;
	UINT8 ucStatus;

	uintptr_t *pulBuf = (uintptr_t *)(pBuf + uiColAddr);

	union T_NAND_COMMAND_REG  uiCmdCycle = {0};
	union T_NAND_COMMAND2_REG uiCmd2Cycle = {0};

	ER ret = E_OK;

	NAND_DEBUG_MSG("progPage: row: 0x%x, col: 0x%x, size: 0x%x \n", uiRowAddr, uiColAddr, uiBufSize);

#if defined(CONFIG_NVT_FW_UPDATE_LED) && !defined(CONFIG_NVT_PWM)
	led_set_gpio_high(OP_PROGRAM);
#endif

	if (info->flash_info->plane_flags && (uiRowAddr & (0x1 << 6))) {
		if (info->flash_info->device_size == 512) {
			uiColAddr |= (0x1 << 13);
		} else {
			uiColAddr |= (0x1 << 12);
		}
	}

	do {
		ret = spi_nand_unlock_BP(info);
		if (ret != E_OK) {
			break;
		}

		if ((uiColAddr & info->flash_info->page_size) && \
			!(uiColAddr & (info->flash_info->page_size-1))) {
			transParam.colAddr = 0;
			transParam.rowAddr = 0;
			transParam.fifoDir = _FIFO_READ;
			transParam.transMode = _PIO_MODE;
			transParam.type = _CMD;
			transParam.uiCS = info->flash_info->chip_sel;

			NAND_DEBUG_MSG("writeSpare \n");

			nand_host_setup_transfer(info, &transParam, 0, 0, _SINGLE_PAGE);

			nand_host_send_command(info, SPINAND_CMD_WEN, FALSE);

			ret = nand_cmd_wait_complete(info);
			if (ret != E_OK) {
				return ret;
			}

			nand_hostSetNandType(info, NANDCTRL_SPI_NOR_TYPE);
			info->flash_info->config_nand_type = NANDCTRL_SPI_NOR_TYPE;

#if defined(CONFIG_TARGET_NA51055) || defined(CONFIG_TARGET_NA51089)
			/*Disable clk_auto_gating before set cs_manual*/
			*(u32 *) 0xF00200B0 &= ~0x80000;
#endif

			nand_hostSetCSActive(info, NAND_SPI_CS_HIGH);

			nand_hostConfigChipSelOperationMode(info, NAND_SPI_CS_MANUAL_MODE);

			nand_hostSetCSActive(info, NAND_SPI_CS_LOW);

			uiCmdCycle.Bit.CMD_CYC_1ST = SPINAND_CMD_PROGRAM_LOAD;

			transParam.colAddr = 0;
			transParam.rowAddr = uiColAddr;
			transParam.fifoDir = _FIFO_READ;
			transParam.transMode = _PIO_MODE;
			transParam.type = _CMD_ADDR;
			transParam.uiCS = info->flash_info->chip_sel;

			nand_host_setup_transfer(info, &transParam, 0, 0, _SINGLE_PAGE);

			nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_NORMAL_DUPLEX_MODE, NAND_SPI_ORDER_MODE_1);

			nand_hostSetupAddressCycle(info, NAND_ROW_ADDRESS_2_CYCLE_CNT, NAND_COL_ADDRESS_1_CYCLE_CNT);

			nand_host_send_command(info, uiCmdCycle.Reg, FALSE);

			ret = nand_cmd_wait_complete(info);

			if (ret != E_OK) {
				nand_hostSetCSActive(info, NAND_SPI_CS_HIGH);
				nand_hostConfigChipSelOperationMode(info, NAND_SPI_CS_AUTO_MODE);
				return ret;
			}

			nand_hostSetupAddressCycle(info, NAND_ROW_ADDRESS_3_CYCLE_CNT, NAND_COL_ADDRESS_1_CYCLE_CNT);

			transParam.colAddr = 0;
			transParam.rowAddr = 0;
			transParam.fifoDir = _FIFO_WRITE;
			transParam.transMode = _PIO_MODE;
			transParam.type = _SPI_WRITE_N_BYTES;

			if (info->use_ecc == NANDCTRL_SPIFLASH_USE_INTERNAL_BCH_ECC) {
				pulBuf = (uintptr_t *)(info->mmio_base + NAND_SPARE00_REG_OFS);
				nand_host_set_cpu_access_spare(info, TRUE);
				if (info->flash_info->spare_size) {
					uiBufSize *= 2;
				}
			} else {
				u8 chip_id = info->flash_info->chip_id & 0xFF;

				if (chip_id == SPI_NAND_GD) {
					nvt_writel(nvt_readl(pulBuf) & ~0xF000, pulBuf);
				}
			}

			nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_NORMAL_DUPLEX_MODE, NAND_SPI_ORDER_MODE_1);

			nand_host_setup_transfer(info, &transParam, 0, uiBufSize, _SINGLE_PAGE);

			nand_host_send_command(info, uiCmdCycle.Reg, FALSE);

			ret = nand_host_transmit_data(info, (u8 *)pulBuf, uiBufSize, _PIO_MODE);

			if (ret != E_OK) {
				nand_hostSetCSActive(info, NAND_SPI_CS_HIGH);
				nand_hostConfigChipSelOperationMode(info, NAND_SPI_CS_AUTO_MODE);
				nand_host_set_cpu_access_spare(info, FALSE);
				return ret;
			}

			nand_hostSetCSActive(info, NAND_SPI_CS_HIGH);

			nand_hostConfigChipSelOperationMode(info, NAND_SPI_CS_AUTO_MODE);
			nand_host_set_cpu_access_spare(info, FALSE);

#if defined(CONFIG_TARGET_NA51055) || defined(CONFIG_TARGET_NA51089)
			/*Enable clk_auto_gating after set cs_manual*/
			*(u32 *) 0xF00200B0 |= 0x80000;
#endif

			nand_hostSetNandType(info, NANDCTRL_SPI_NAND_TYPE);
			info->flash_info->config_nand_type = NANDCTRL_SPI_NAND_TYPE;

			uiCmdCycle.Bit.CMD_CYC_1ST = SPINAND_CMD_PROGRAM_EXE;

			transParam.colAddr = 0;
			transParam.rowAddr = uiRowAddr;
			transParam.fifoDir = _FIFO_READ;
			transParam.transMode = _PIO_MODE;
			transParam.type = _CMD_ADDR;
			transParam.uiCS = info->flash_info->chip_sel;

			nand_host_setup_transfer(info, &transParam, 0, 0, _SINGLE_PAGE);

			nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_NORMAL_DUPLEX_MODE, NAND_SPI_ORDER_MODE_1);

			nand_hostSetupAddressCycle(info, NAND_ROW_ADDRESS_3_CYCLE_CNT, NAND_NOT_CONFIGED_ADDRESS_CYCLE_CNT);

			nand_host_send_command(info, uiCmdCycle.Reg, FALSE);

			ret = nand_cmd_wait_complete(info);

			if (ret != E_OK) {
				break;
			}

			transParam.colAddr = 0;
			transParam.rowAddr = 0;
			transParam.type = _AUTO_STATUS;

			nand_host_setup_transfer(info, &transParam, 0, 1, _SINGLE_PAGE);

			nand_hostSetupStatusCheckBit(info, NAND_STS_BIT_0, NAND_STS_BIT_0_EQUAL_0);

			nand_host_send_command(info, SPINAND_CMD_GET_FEATURE | (SPINAND_FEATURE_STATUS << 8), 0);

			ret = nand_cmd_wait_complete(info);
		} else {
			// write enable
			transParam.colAddr      = uiColAddr;
			transParam.rowAddr      = uiRowAddr;
			transParam.fifoDir      = _FIFO_WRITE;
			transParam.transMode    = _DMA_MODE;
			transParam.type         = _SPI_PROG_CMD;
			transParam.uiCS         = info->flash_info->chip_sel;

			NAND_SETREG(info, NAND_DMASTART_REG_OFS, (uintptr_t)pulBuf);

			if (uiBufSize > info->flash_info->page_size) {
				flush_dcache_range((uintptr_t)pulBuf, (uintptr_t)(pulBuf + info->flash_info->page_size / 4));
				nand_host_setup_transfer(info, &transParam, 1, info->flash_info->page_size, _SINGLE_PAGE);
			} else {
				flush_dcache_range((uintptr_t)pulBuf, (uintptr_t)(pulBuf + uiBufSize / 4));
				nand_host_setup_transfer(info, &transParam, 1, uiBufSize, _SINGLE_PAGE);
			}

			uiCmdCycle.Bit.CMD_CYC_1ST = SPINAND_CMD_WEN;

			if (info->flash_info->spi_nand_status.bQuadProgram) {
				uiCmdCycle.Bit.CMD_CYC_2ND = SPINAND_CMD_PROGRAM_LOADX4;

				nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_QUAD_MODE, NAND_SPI_ORDER_MODE_1);
			} else {
				uiCmdCycle.Bit.CMD_CYC_2ND = SPINAND_CMD_PROGRAM_LOAD;

				nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_NORMAL_DUPLEX_MODE, NAND_SPI_ORDER_MODE_1);
			}

			uiCmdCycle.Bit.CMD_CYC_3RD = SPINAND_CMD_PROGRAM_EXE;

			uiCmdCycle.Bit.CMD_CYC_4TH = SPINAND_CMD_GET_FEATURE;

			uiCmd2Cycle.Bit.CMD_CYC_5TH = SPINAND_FEATURE_STATUS;

			nand_hostSetupAddressCycle(info, NAND_ROW_ADDRESS_3_CYCLE_CNT, NAND_COL_ADDRESS_2_CYCLE_CNT);

			nand_hostSetupStatusCheckBit(info, NAND_STS_BIT_0, NAND_STS_BIT_0_EQUAL_0);

			NAND_SETREG(info, NAND_COMMAND2_REG_OFS, uiCmd2Cycle.Reg);

			nand_host_send_command(info, uiCmdCycle.Reg, TRUE);

			ret = nand_cmd_wait_complete(info);

			if (ret != E_OK) {
				break;
			}
		}

		ucStatus = (UINT8)NAND_GETREG(info, NAND_CTRL_STS_REG_OFS);

		if (ucStatus & SPINAND_FEATURE_STATUS_PROG_FAIL) {
			ret = E_SYS;
		}

	} while (0);

#if defined(CONFIG_NVT_FW_UPDATE_LED) && !defined(CONFIG_NVT_PWM)
	led_set_gpio_low();
#endif
	return ret;
}
/*
    Reset NAND flash

    Send reset command to NAND flash

    @return
        - @b E_SYS      Status fail
        - @b E_TMOUT    Controller timeout
        - @b E_OK       Operation success

    @note for nand_CmdComplete()
*/
int nand_cmd_reset(struct drv_nand_dev_info *info)
{
	struct smc_setup_trans transParam;

	transParam.colAddr      = 0;
	transParam.rowAddr      = 0;
	transParam.fifoDir      = _FIFO_READ;
	transParam.transMode    = _PIO_MODE;
	transParam.type         = _CMD;
	transParam.uiCS         = info->flash_info->chip_sel;

	nand_host_setup_transfer(info, &transParam, 0, 0, _SINGLE_PAGE);

	nand_host_send_command(info, SPINAND_CMD_RESET, 0);

	return nand_cmd_wait_complete(info);
}


/*
    Read status command

    Send read status command to NAND flash

    @return
        - @b E_SYS      Status fail
        - @b E_TMOUT    Controller timeout
        - @b E_OK       Operation success

    @note for nand_CmdComplete()
*/
int nand_cmd_read_status(struct drv_nand_dev_info *info, uint32_t set)
{
	int ret;
	uint32_t status = 0;
	struct smc_setup_trans transParam;
	uint32_t spi_cmd = 0;

	transParam.colAddr = 0;
	transParam.rowAddr = 0;
	transParam.fifoDir = _FIFO_READ;
	transParam.transMode = _PIO_MODE;
	transParam.type = _CMD_STS;
	transParam.uiCS = info->flash_info->chip_sel;

	nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_NORMAL_DUPLEX_MODE, NAND_SPI_ORDER_MODE_1);

	nand_host_setup_transfer(info, &transParam, 0, 1, _SINGLE_PAGE);

	spi_cmd = SPINAND_CMD_GET_FEATURE;

	if (set == NAND_SPI_STS_FEATURE_1) {
		spi_cmd |= (SPINAND_FEATURE_BLOCK_LOCK << 8);
	} else if (set == NAND_SPI_STS_FEATURE_2) {
		spi_cmd |= (SPINAND_FEATURE_OPT << 8);
	} else if (set == NAND_SPI_STS_FEATURE_4) {
		spi_cmd |= (SPINAND_FEATURE_STATUS_ECC_UNCORR_ERR << 8);
	} else {
		spi_cmd |= (SPINAND_FEATURE_STATUS << 8);
	}

	nand_host_send_command(info, spi_cmd, 0);

	ret = nand_cmd_wait_complete(info);

	if (ret == E_OK) {
		status = NAND_GETREG(info, NAND_CTRL_STS_REG_OFS) & 0xFF;
		memcpy(info->data_buff + info->buf_start, &status, 1);
	}
	return status;
}


/*
    Write status command

    Send write status command to NAND flash

    @return
        - @b E_SYS      Status fail
        - @b E_TMOUT    Controller timeout
        - @b E_OK       Operation success

    @note for nand_CmdComplete()
*/
int nand_cmd_write_status(struct drv_nand_dev_info *info, u32 set, u32 status)
{
	int ret;
	struct smc_setup_trans transParam;
	uint32_t spi_cmd = 0;
	union T_NAND_COMMAND_REG  uiCmdCycle = {0};

	uiCmdCycle.Bit.CMD_CYC_1ST = SPINAND_CMD_SET_FEATURE;
	if (set == NAND_SPI_STS_FEATURE_1) {
		spi_cmd = SPINAND_FEATURE_BLOCK_LOCK << 8;
	} else if (set == NAND_SPI_STS_FEATURE_2) {
		spi_cmd = SPINAND_FEATURE_OPT << 8;
	} else {
		spi_cmd = SPINAND_FEATURE_STATUS << 8;
	}

	spi_cmd |= status;

	transParam.colAddr = 0;
	transParam.rowAddr = spi_cmd;
	transParam.fifoDir = _FIFO_READ;
	transParam.transMode = _PIO_MODE;
	transParam.type = _CMD_ADDR;
	transParam.uiCS = info->flash_info->chip_sel;

	nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_NORMAL_DUPLEX_MODE, NAND_SPI_ORDER_MODE_1);

	nand_host_setup_transfer(info, &transParam, 0, 0, _SINGLE_PAGE);

	nand_hostSetupAddressCycle(info, NAND_ROW_ADDRESS_2_CYCLE_CNT, NAND_COL_ADDRESS_2_CYCLE_CNT);

	nand_host_send_command(info, uiCmdCycle.Reg, 0);

	ret = nand_cmd_wait_complete(info);

	return ret;
}

/*
    read config command

    Send read config command to NAND flash

    @return
        - @b E_SYS      Status fail
        - @b E_TMOUT    Controller timeout
        - @b E_OK       Operation success

    @note for nand_CmdComplete()
*/
int nand_cmd_read_config(struct drv_nand_dev_info *info, u32 config)
{
	int ret;
	struct smc_setup_trans transParam;

	transParam.colAddr = 0;
	transParam.rowAddr = 0;
	transParam.fifoDir = _FIFO_READ;
	transParam.transMode = _PIO_MODE;
	transParam.type = _CMD_STS;
	transParam.uiCS = info->flash_info->chip_sel;

	nand_host_setup_transfer(info, &transParam, 0, 0, _SINGLE_PAGE);

	nand_host_send_command(info, config, 0);

	ret = nand_cmd_wait_complete(info);

	*(info->data_buff) = (UINT8)NAND_GETREG(info, NAND_CTRL_STS_REG_OFS);

	return ret;
}

/*
    Erase block command.

    Issue erase block command of NAND controller
    @note block_num is the physical block number instead of the address.

    @param[in] block_num   which physical block number want to erase
    @return success or fail
        - @b E_OK:     Erase operation success
        - @b E_TMOUT   Time out
        - @b E_SYS:    Erase operation fail
*/
int nand_cmd_erase_block(struct drv_nand_dev_info *info, uint32_t block_address)
{
	struct smc_setup_trans transParam;
	UINT8 ucStatus;
	ER ret = E_OK;

	NAND_DEBUG_MSG("eraseBlock: addr: 0x%x \n", block_address);


#if defined(CONFIG_NVT_FW_UPDATE_LED) && !defined(CONFIG_NVT_PWM)
	led_set_gpio_high(OP_ERASE);
#endif

	do {
		ret = spi_nand_unlock_BP(info);
		if (ret != E_OK) {
			break;
		}

		// write enable
		// ___|cmd|___ = 0x07 command cycle only

		transParam.colAddr = 0;
		transParam.rowAddr = 0;
		transParam.fifoDir = _FIFO_READ;
		transParam.transMode = _PIO_MODE;
		transParam.type = _CMD;
		transParam.uiCS = info->flash_info->chip_sel;

		nand_host_setup_transfer(info, &transParam, 0, 0, _SINGLE_PAGE);

		nand_host_send_command(info, SPINAND_CMD_WEN, FALSE);

		ret = nand_cmd_wait_complete(info);

		if (ret != E_OK) {
			break;
		}

		transParam.colAddr = 0;
		transParam.rowAddr = block_address;
		transParam.fifoDir = _FIFO_WRITE;
		transParam.transMode = _PIO_MODE;
		transParam.type = _BLK_ERSE;
		transParam.uiCS = info->flash_info->chip_sel;

		nand_host_setup_transfer(info, &transParam, 0, 0, _SINGLE_PAGE);

		nand_hostSetupAddressCycle(info, NAND_ROW_ADDRESS_3_CYCLE_CNT, NAND_NOT_CONFIGED_COL_ADDRESS_CYCLE_CNT);

		nand_hostSetupStatusCheckBit(info, NAND_STS_BIT_0, NAND_STS_BIT_0_EQUAL_0);

		nand_host_send_command(info, SPINAND_CMD_BLK_ERASE | (SPINAND_CMD_GET_FEATURE << 8) | (SPINAND_FEATURE_STATUS << 16), TRUE);

		ret = nand_cmd_wait_complete(info);

		if (ret != E_OK) {
			break;
		}

		ucStatus = (UINT8)NAND_GETREG(info, NAND_CTRL_STS_REG_OFS);

		if (ucStatus & SPINAND_FEATURE_STATUS_ERASE_FAIL) {
			ret = E_SYS;
			break;
		}

	} while (0);

#if defined(CONFIG_NVT_FW_UPDATE_LED) && !defined(CONFIG_NVT_PWM)
	led_set_gpio_low();
#endif

	return ret;
}
/*
    Read device ID.

    @return Device ID
*/
int nand_cmd_read_id(uint8_t *card_id, struct drv_nand_dev_info *info)
{
	struct smc_setup_trans transParam;

	transParam.colAddr = 0;
	transParam.rowAddr = 0;
	transParam.fifoDir = _FIFO_READ;
	transParam.transMode = _PIO_MODE;
	transParam.type = _READ_ID;
	transParam.uiCS = info->flash_info->chip_sel;

	nand_host_setup_transfer(info, &transParam, 0, 4, _SINGLE_PAGE);

	nand_hostSetupAddressCycle(info, NAND_ROW_ADDRESS_1_CYCLE_CNT, NAND_NOT_CONFIGED_COL_ADDRESS_CYCLE_CNT);

	nand_host_send_command(info, SPINAND_CMD_JEDECID, FALSE);

	return nand_host_receive_data(info, card_id, 4, _PIO_MODE);
}

/*
    Read ECC corrected bits

    @return correct bits
*/

int nand_cmd_read_flash_ecc_corrected(struct drv_nand_dev_info *info)
{
	struct smc_setup_trans transParam;
	uint8_t  ecc_bits[4];

	transParam.colAddr = 0;
	transParam.rowAddr = 0;
	transParam.fifoDir = _FIFO_READ;
	transParam.transMode = _PIO_MODE;
	transParam.type = _READ_ID;
	transParam.uiCS = info->flash_info->chip_sel;

	nand_host_setup_transfer(info, &transParam, 0, 4, _SINGLE_PAGE);

	nand_host_send_command(info, SPINAND_CMD_MXIC_READ_ECC, FALSE);

	nand_host_receive_data(info, &ecc_bits[0], 4, _PIO_MODE);

	return ecc_bits[0] & 0xF;
}


/*
    nand_cmd_wait_complete

    Check NAND operation command complete.

    @return
        - @b E_SYS      Status fail
        - @b E_OK       Operation success
*/
int nand_cmd_wait_complete(struct drv_nand_dev_info *info)
{
	unsigned int time_start, time_now;

	time_start = get_timer(0);

	while (1) {
		if (NAND_GETREG(info, NAND_CTRL_STS_REG_OFS) & NAND_COMP_STS) {
			break;
		}

		time_now = get_timer(0);
		if ((time_now - time_start) > 4500) {
			printf("_COMP_INTR not set\n\r");
			return -EIO;
		}
	}

	NAND_SETREG(info, NAND_CTRL_STS_REG_OFS, NAND_COMP_STS);

#if defined(CONFIG_TARGET_NA51102) || defined(CONFIG_TARGET_NA51102_A64)
	// Disable AXI ch
	nand_host_axi_ch_disable(info, 1);
#endif

	if (info->ecc_status_type == 1) {
		// special ecc_sts type for XT26G01C
		if ((NAND_GETREG(info, NAND_CTRL_STS_REG_OFS) & NAND_STS_FAIL_STS)) {
			NAND_SETREG(info, NAND_CTRL_STS_REG_OFS, NAND_STS_FAIL_STS);
			printf("NAND status fail 0x%x\r\n", NAND_GETREG(info, NAND_CTRL_STS_REG_OFS));
			return -EIO;
		}
	} else {
		// normal ecc_sts type
		if ((NAND_GETREG(info, NAND_CTRL_STS_REG_OFS) & NAND_STS_FAIL_STS) && \
			!(NAND_GETREG(info, NAND_CTRL_STS_REG_OFS) & SPINAND_FEATURE_STATUS_ECC_UNCORR_ERR)) {
			NAND_SETREG(info, NAND_CTRL_STS_REG_OFS, NAND_STS_FAIL_STS);
			printf("NAND status fail 0x%x\r\n", NAND_GETREG(info, NAND_CTRL_STS_REG_OFS));
			return -EIO;
		}
	}

	if (NAND_GETREG(info, NAND_CTRL_STS_REG_OFS) & NAND_TOUT_STS) {
		NAND_SETREG(info, NAND_CTRL_STS_REG_OFS, NAND_TOUT_STS);
		printf("NAND cmd timeout\r\n");
		return -EIO;
	}
	return nand_host_check_ready(info);
}

/*
    Wait SPI flash ready

    Wait SPI flash returned to ready state

    @param[in] uiWaitMs     Timeout setting. (Unit: ms)

    @return
        - @b E_OK: success
        - @b Else: fail. Maybe timeout.
*/
static ER spiflash_wait_ready(struct drv_nand_dev_info *info, u8 *ucStatus)
{
	union T_NAND_COMMAND_REG uiCmdCycle = {0};
	struct smc_setup_trans transParam;
	ER ret = E_OK;

	do {
		transParam.colAddr = 0;
		transParam.rowAddr = 0;
		transParam.fifoDir = _FIFO_READ;
		transParam.transMode = _PIO_MODE;
		transParam.type = _AUTO_STATUS;
		transParam.uiCS = info->flash_info->chip_sel;

		nand_host_setup_transfer(info, &transParam, 0, 0, _SINGLE_PAGE);

		uiCmdCycle.Bit.CMD_CYC_1ST = FLASH_CMD_RDSR;

		nand_hostSetupStatusCheckBit(info, NAND_STS_BIT_0, NAND_STS_BIT_0_EQUAL_0);

		nand_host_send_command(info, uiCmdCycle.Reg, TRUE);

		ret = nand_cmd_wait_complete(info);

	} while (0);

	if (ucStatus) {
		* ucStatus = (UINT8)NAND_GETREG(info, NAND_CTRL_STS_REG_OFS);
	}

	return ret;
}

static ER spiflash_disable_write_latch(struct drv_nand_dev_info *info)
{
	struct smc_setup_trans transParam;
	int  ret;

	transParam.colAddr = 0;
	transParam.rowAddr = 0;
	transParam.fifoDir = _FIFO_READ;
	transParam.transMode = _PIO_MODE;
	transParam.type = _CMD;
	transParam.uiCS = info->flash_info->chip_sel;

	nand_host_setup_transfer(info, &transParam, 0, 0, _SINGLE_PAGE);

	nand_host_send_command(info, FLASH_CMD_WRDI, FALSE);

	ret = nand_cmd_wait_complete(info);

	return ret;
}

static ER spiflash_enable_write_latch(struct drv_nand_dev_info *info)
{
	struct smc_setup_trans transParam;
	int ret;

	do {
		transParam.colAddr = 0;
		transParam.rowAddr = 0;
		transParam.fifoDir = _FIFO_READ;
		transParam.transMode = _PIO_MODE;
		transParam.type = _CMD;
		transParam.uiCS = info->flash_info->chip_sel;

		nand_host_setup_transfer(info, &transParam, 0, 0, _SINGLE_PAGE);

		nand_host_send_command(info, FLASH_CMD_WREN, FALSE);

		ret = nand_cmd_wait_complete(info);

		if (ret != E_OK) {
			break;
		}
	} while (0);

	return ret;
}

/*
    Read pages.

    @param[out] buffer Buffer address
    @param pageAddress The address of the page. Only (n * g_pNandInfo->uiBytesPerPageData) will be valid.
            Beware that the max accessible size is 4GB.  One should use nand_readOperation2() instead.
    @param numPage How many pages
    @return
        - @b E_OK       read operation success
        - @b E_PAR      parameter error
        - @b E_SYS
*/
int nand_cmd_read_operation(struct drv_nand_dev_info *info, int8_t *buffer, uint32_t pageAddress, uint32_t numPage)
{
	const struct nvt_nand_flash *f = info->flash_info;
	return spiNand_readPage(info, pageAddress / f->page_size, 0, (uint8_t *)buffer, numPage * f->page_size);
}


/*
    Write pages.(single page operation)

    @param buffer      Buffer address
    @param pageAddress The address of the page. Only (n * g_pNandInfo->uiBytesPerPageData) will be valid.
        Beware that the max accessible size is 4GB.  One should use write_readOperation2() instead.
    @param numPage     How many pages
    @return E_OK or E_SYS
*/
int nand_cmd_write_operation_single(struct drv_nand_dev_info *info, int8_t *buffer, uint32_t pageAddress, uint32_t column)
{
	const struct nvt_nand_flash *f = info->flash_info;
	uint32_t buf_len = info->buf_count - column;

	return spiNand_programPage(info, pageAddress / f->page_size, column, (UINT8 *)buffer, buf_len, 0, 0);
}

void nand_store_cache(void *pbuf, u32 len)
{
	return;
}

void nand_cmd_write_spare_sram(struct drv_nand_dev_info *info)
{
	unsigned long spare_area_addr;
	uint32_t *pul_buf = (uint32_t *)(info->data_buff + info->flash_info->page_size);
	uint32_t section_count_per_page = (info->flash_info->page_size / 0x200);
	int j = 0;
#if defined(CONFIG_TARGET_NA51102) || defined(CONFIG_TARGET_NA51102_A64)
	u8 chip_id = info->flash_info->chip_id & 0xFF;
#endif

	NAND_DEBUG_MSG("writeSpare sram\n");

	flush_dcache_range((uintptr_t)pul_buf, (uintptr_t)(pul_buf + info->flash_info->oob_size / 4));

	nand_host_set_cpu_access_spare(info, TRUE);
	spare_area_addr = (unsigned long) info->mmio_base + NAND_SPARE00_REG_OFS;

#if defined(CONFIG_TARGET_NA51102) || defined(CONFIG_TARGET_NA51102_A64)
	if ((info->flash_info->spare_size) && (info->flash_info->spare_align)) {
		// clear spare sram
		for (j = 0; j < 64; j++) {
			nvt_writel(0xFFFFFFFF, spare_area_addr + 0x4 * j);
		}

		if (section_count_per_page == 4) {
			NAND_DEBUG_MSG("2k page, swap oob:\r\n");
			nvt_writel(nvt_readl(pul_buf + 0), spare_area_addr + 0x0);
			nvt_writel(nvt_readl(pul_buf + 1), spare_area_addr + 0x4);
			nvt_writel(nvt_readl(pul_buf + 2), spare_area_addr + 0x8);
			for (j = 0; j < 4; j++) {
				nvt_writel(nvt_readl(pul_buf + j + 0x3), spare_area_addr + 0x40 + j * 4);
				nvt_writel(nvt_readl(pul_buf + j + 0x7), spare_area_addr + 0x50 + j * 4);
				nvt_writel(nvt_readl(pul_buf + j + 0xb), spare_area_addr + 0x60 + j * 4);
			}
			nvt_writel(nvt_readl(pul_buf + 0xf), spare_area_addr + 0x70);

			if (chip_id == SPI_NAND_GD) {
				NAND_SETREG(info, NAND_SPARE00_REG_OFS + 0x7C, 0x0FFFFFFF);
			}

		} else {
			NAND_DEBUG_MSG("4k page, swap oob:\r\n");
			for (j = 0; j < 6; j++) {   // B0-23
				nvt_writel(nvt_readl(pul_buf + j), spare_area_addr + 0x4 * j);
			}
			for (j = 0; j < 26; j++) {  // B24-127
				nvt_writel(nvt_readl(pul_buf + 6 + j), spare_area_addr + 0x80 + 0x4 * j);
			}

			if (chip_id == SPI_NAND_GD) {
				NAND_SETREG(info, NAND_SPARE00_REG_OFS + 0xFC, 0x0FFFFFFF);
			}
		}

#else
	if (0) {    // oob: fixed
#endif
	} else {
		if (section_count_per_page > 1) {
			// clear spare sram
			for (j = 0; j < (4 * section_count_per_page); j++) {
				NAND_SETREG(info, NAND_SPARE00_REG_OFS + 0x4 * j, 0xFFFFFFFF);
			}

			if (info->use_ecc == NANDCTRL_SPIFLASH_USE_INTERNAL_BCH_ECC) {
				if (section_count_per_page == 4) {
					for (j = 0; j < 3; j++) {
						nvt_writel(nvt_readl(pul_buf + j), spare_area_addr + j * 4);
					}
				} else {
					for (j = 0; j < 6; j++) {
						nvt_writel(nvt_readl(pul_buf + j), spare_area_addr + j * 4);
					}
				}

#if 0   // workaround for GD flash
				if (chip_id == SPI_NAND_GD) {
					NAND_SETREG(info, NAND_SPARE00_REG_OFS + 0x14, 0x7FFFFFFF);
					NAND_SETREG(info, NAND_SPARE00_REG_OFS + 0x20, 0x7FFFFFFF);
					NAND_SETREG(info, NAND_SPARE00_REG_OFS + 0x2C, 0x7FFFFFFF);
				}
#endif
			} else {

				for (j = 0; j < section_count_per_page; j++) {
					nvt_writel(nvt_readl(pul_buf + j +  0), spare_area_addr +  0 + j * 4);
					nvt_writel(nvt_readl(pul_buf + j +  4), spare_area_addr + 16 + j * 4);
					nvt_writel(nvt_readl(pul_buf + j +  8), spare_area_addr + 32 + j * 4);
					nvt_writel(nvt_readl(pul_buf + j + 12), spare_area_addr + 48 + j * 4);
				}
			}

		} else {
			nvt_writel(nvt_readl(pul_buf + 0), spare_area_addr + 0x0);
			nvt_writel(nvt_readl(pul_buf + 1), spare_area_addr + 0x4);
			nvt_writel(nvt_readl(pul_buf + 2), spare_area_addr + 0x8);
			nvt_writel(nvt_readl(pul_buf + 3), spare_area_addr + 0xC);
		}
	}

	nand_host_set_cpu_access_spare(info, FALSE);
}

/*
    Read the spare data from a page for debug.

    @param[out] spare0  The value of Spare Area Read Data Register 0
    @param[out] spare1  The value of Spare Area Read Data Register 1
    @param[out] spare2  The value of Spare Area Read Data Register 2
    @param[out] spare3  The value of Spare Area Read Data Register 3
    @param pageAddress The address of the page
    @return
        - @b E_OK       read spare success
        - @b E_SYS      read spare operation fail(status fail)
        - @b E_CTX      read spare encounter ecc uncorrect error(Only if Reed solomon ecc usage)
*/
int nand_cmd_read_page_spare_data(struct drv_nand_dev_info *info, int8_t *buffer, uint32_t page_address)
{
	const struct nvt_nand_flash *f = info->flash_info;

	return spiNand_readPage(info, page_address / f->page_size, f->page_size, (uint8_t *)buffer, f->oob_size);
}

/*
    Read the largest number of ecc corrected bits
*/
int nand_cmd_read_ecc_corrected(struct drv_nand_dev_info *info)
{
	u8 correct_count = 0;

	if (info->use_ecc == NANDCTRL_SPIFLASH_USE_INTERNAL_RS_ECC) {
		union T_NAND_RSERR_STS0_REG reg_correct = {0};

		reg_correct.Reg = NAND_GETREG(info, NAND_RSERR_STS0_REG_OFS);

		/*Read the largest number of correct bits*/
		correct_count = reg_correct.Bit.SEC0_ERR_STS;
		if (reg_correct.Bit.SEC1_ERR_STS > correct_count) {
			correct_count = reg_correct.Bit.SEC1_ERR_STS;
		}

		if (reg_correct.Bit.SEC2_ERR_STS > correct_count) {
			correct_count = reg_correct.Bit.SEC2_ERR_STS;
		}

		if (reg_correct.Bit.SEC3_ERR_STS > correct_count) {
			correct_count = reg_correct.Bit.SEC3_ERR_STS;
		}
	} else { //if (info->use_ecc == NANDCTRL_SPIFLASH_USE_INTERNAL_BCH_ECC) {
		union T_NAND_BCHERR_STS0_REG reg_correct = {0};
		u8 i;

		reg_correct.Reg = NAND_GETREG(info, NAND_BCHERR_STS0_REG_OFS);

		for (i = 0; i < 8; i++) {
			if ((reg_correct.Reg & 0xF) > correct_count) {
				correct_count = reg_correct.Reg & 0xF;
			}
			reg_correct.Reg = reg_correct.Reg >> 4;
		}
	}

	return correct_count;
}

/*
    Dummy read command

    Dummy read command

    @return
        - @b E_SYS      Status fail
        - @b E_TMOUT    Controller timeout
        - @b E_OK       Operation success

    @note for nand_cmd_wait_complete()
*/
int nand_cmd_dummy_read(struct drv_nand_dev_info *info)
{
	return 0;
}

/*
    Read SPI flash

    @param[in] addr Byte address of SPI flash
    @param[in] byte_size    Byte count of read data. Should be word alignment (i.e. multiple of 4).
    @param[out] pbuf    Pointer point to store read data

    @return void
*/
static int spiflash_read_data(struct drv_nand_dev_info *info, u32 addr, u32 byte_size, u8 *pbuf)
{
	union T_NAND_COMMAND_REG uiCmdCycle = {0};
	struct smc_setup_trans transParam;
	int ret = E_OK;
#if 0
	u32 dummy_read = 0;
#endif

#if defined(CONFIG_TARGET_NA51055) || defined(CONFIG_TARGET_NA51089)
	/*Disable clk_auto_gating before set cs_manual*/
	*(u32 *) 0xF00200B0 &= ~0x80000;
#endif

	nand_hostSetCSActive(info, NAND_SPI_CS_HIGH);

	nand_hostConfigChipSelOperationMode(info, NAND_SPI_CS_MANUAL_MODE);

	nand_hostSetCSActive(info, NAND_SPI_CS_LOW);

	if (info->flash_info->nor_read_mode == SPI_NOR_QUAD_READ) {
		if (info->flash_info->device_size > SPI_FLASH_16MB_BOUN) {
			uiCmdCycle.Bit.CMD_CYC_1ST = FLASH_CMD_QUAD_READ_NORMAL_4BYTE;
		} else {
			uiCmdCycle.Bit.CMD_CYC_1ST = FLASH_CMD_QUAD_READ_NORMAL;
		}
	} else if (info->flash_info->nor_read_mode == SPI_NOR_DUAL_READ) {
		if (info->flash_info->device_size > SPI_FLASH_16MB_BOUN) {
			uiCmdCycle.Bit.CMD_CYC_1ST = FLASH_CMD_DUAL_READ_4BYTE;
		} else {
			uiCmdCycle.Bit.CMD_CYC_1ST = FLASH_CMD_DUAL_READ;
		}
	} else {
		if (info->flash_info->device_size > SPI_FLASH_16MB_BOUN) {
			uiCmdCycle.Bit.CMD_CYC_1ST = FLASH_CMD_READ_4BYTE;
		} else {
			uiCmdCycle.Bit.CMD_CYC_1ST = FLASH_CMD_READ;
		}
	}



	transParam.colAddr      = 0;
	transParam.rowAddr      = addr;
	transParam.fifoDir      = _FIFO_READ;
	transParam.transMode    = _PIO_MODE;
	transParam.type         = _CMD_ADDR;
	transParam.uiCS         = info->flash_info->chip_sel;

	nand_host_setup_transfer(info, &transParam, 0, 0, _SINGLE_PAGE);

	if (info->flash_info->device_size > SPI_FLASH_16MB_BOUN) {
		nand_hostSetupAddressCycle(0, NAND_ROW_ADDRESS_4_CYCLE_CNT, NAND_COL_ADDRESS_1_CYCLE_CNT);
	} else {
		nand_hostSetupAddressCycle(0, NAND_ROW_ADDRESS_3_CYCLE_CNT, NAND_COL_ADDRESS_1_CYCLE_CNT);
	}

	nand_host_send_command(info, uiCmdCycle.Reg, FALSE);

	ret = nand_cmd_wait_complete(info);

	if (ret != E_OK) {
		nand_hostSetCSActive(info, NAND_SPI_CS_HIGH);
		nand_hostConfigChipSelOperationMode(info, NAND_SPI_CS_AUTO_MODE);
		return ret;
	}

	transParam.colAddr      = 0;
	transParam.rowAddr      = 0;
	transParam.fifoDir      = _FIFO_READ;
	transParam.type         = _SPI_READ_N_BYTES;
	transParam.uiCS         = info->flash_info->chip_sel;

	if (info->flash_info->nor_read_mode != SPI_NOR_NORMAL_READ) {
		transParam.transMode = _PIO_MODE;
#if 0
		/*Dummy 8 T*/
		nand_host_setup_transfer(info, &transParam, 0, 1, _SINGLE_PAGE);

		nand_host_send_command(info, uiCmdCycle.Reg, 0);

		ret = nand_host_receive_data(info, &dummy_read, 1, _PIO_MODE);
#else
		transParam.type = _DUMMY_CLOCK;
		nand_host_setup_transfer(info, &transParam, 0, 0, _SINGLE_PAGE);
		NAND_SETREG(info, NAND_DUMMY_CLOCK_NUM_OFS, 0x7);
		nand_host_send_command(info, uiCmdCycle.Reg, FALSE);
		ret = nand_cmd_wait_complete(info);
#endif
		if (ret != E_OK) {
			nand_hostSetCSActive(info, NAND_SPI_CS_HIGH);
			nand_hostConfigChipSelOperationMode(info, NAND_SPI_CS_AUTO_MODE);
			return ret;
		}

		/*Dummy 2 T*/
		if (((info->rdcr_status & 0xC0) == 0xC0) && (info->ops_freq > 96000000) && \
			(info->hspeed_dummy_cycle == 10)) {
			debug("SF: dummy 10T mode with RDCR 0x%08x\n", info->rdcr_status);
			transParam.type = _DUMMY_CLOCK;
			nand_host_setup_transfer(info, &transParam, 0, 0, _SINGLE_PAGE);
			NAND_SETREG(info, NAND_DUMMY_CLOCK_NUM_OFS, 0x1);
			nand_host_send_command(info, uiCmdCycle.Reg, FALSE);
			ret = nand_cmd_wait_complete(info);
			if (ret != E_OK) {
				nand_hostSetCSActive(info, NAND_SPI_CS_HIGH);
				nand_hostConfigChipSelOperationMode(info, NAND_SPI_CS_AUTO_MODE);
				return ret;
			}
		}
	}

	transParam.type         = _SPI_READ_N_BYTES;
	transParam.transMode    = _DMA_MODE;

	NAND_SETREG(info, NAND_DMASTART_REG_OFS, (uintptr_t)pbuf);

	if (info->flash_info->nor_read_mode == SPI_NOR_QUAD_READ) {
		nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_QUAD_MODE, NAND_SPI_ORDER_MODE_1);
	} else if (info->flash_info->nor_read_mode == SPI_NOR_DUAL_READ) {
		nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_DUAL_MODE, NAND_SPI_ORDER_MODE_1);
	} else {
		nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_NORMAL_DUPLEX_MODE, NAND_SPI_ORDER_MODE_1);
	}

	nand_host_setup_transfer(info, &transParam, 0, byte_size, _SINGLE_PAGE);

	nand_host_send_command(info, uiCmdCycle.Reg, FALSE);

	ret = nand_cmd_wait_complete(info);

	nand_hostSetCSActive(info, NAND_SPI_CS_HIGH);

	nand_hostConfigChipSelOperationMode(info, NAND_SPI_CS_AUTO_MODE);

	if (info->flash_info->nor_read_mode != SPI_NOR_NORMAL_READ) {
		nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_NORMAL_DUPLEX_MODE, NAND_SPI_ORDER_MODE_1);
	}

#if defined(CONFIG_TARGET_NA51055) || defined(CONFIG_TARGET_NA51089)
	/*Enable clk_auto_gating after set cs_manual*/
	*(u32 *) 0xF00200B0 |= 0x80000;
#endif

	return ret;
}

/*
    Page program

    Program a page of SPI flash

    @param[in] addr        Byte address of SPI flash
    @param[in] byte_size       Byte count of written data. Should be <= 256 and word alignment (i.e. multiple of 4).
    @param[out] pbuf            Pointer point to store read data

    @return
        - @b E_OK: program success
        - @b FALSE: program fail. Maybe timeout.
*/
static int spiflash_page_program(struct drv_nand_dev_info *info, u32 addr, u32 byte_size, u8 *pbuf)
{
	union T_NAND_COMMAND_REG uiCmdCycle = {0};
	struct smc_setup_trans transParam;
	int ret;
	u8 ucSts;

	if (info->enter_4byte_addr_mode == 1) {
		enter_4byte_address(info);
	}

	spiflash_enable_write_latch(info);

#if defined(CONFIG_TARGET_NA51055) || defined(CONFIG_TARGET_NA51089)
	/*Disable clk_auto_gating before set cs_manual*/
	*(u32 *) 0xF00200B0 &= ~0x80000;
#endif

	nand_hostSetCSActive(info, NAND_SPI_CS_HIGH);
	nand_hostConfigChipSelOperationMode(info, NAND_SPI_CS_MANUAL_MODE);

	nand_hostSetCSActive(info, NAND_SPI_CS_LOW);

	if (info->flash_info->nor_quad_support) {
		if (info->flash_info->device_size > SPI_FLASH_16MB_BOUN)
			if (info->enter_4byte_addr_mode == 1) {
				uiCmdCycle.Bit.CMD_CYC_1ST = FLASH_CMD_PAGEPROG_4X;
			} else {
				uiCmdCycle.Bit.CMD_CYC_1ST = FLASH_CMD_PAGEPROG_4X_4BYTE;
			}
		else {
			uiCmdCycle.Bit.CMD_CYC_1ST = FLASH_CMD_PAGEPROG_4X;
		}
	} else {
		if (info->flash_info->device_size > SPI_FLASH_16MB_BOUN)
			if (info->enter_4byte_addr_mode == 1) {
				uiCmdCycle.Bit.CMD_CYC_1ST = FLASH_CMD_PAGEPROG;
			} else {
				uiCmdCycle.Bit.CMD_CYC_1ST = FLASH_CMD_PAGEPROG_4BYTE;
			}
		else {
			uiCmdCycle.Bit.CMD_CYC_1ST = FLASH_CMD_PAGEPROG;
		}
	}

	transParam.colAddr = 0;
	transParam.rowAddr = addr;
	transParam.fifoDir = _FIFO_READ;
	transParam.transMode = _PIO_MODE;
	transParam.type = _CMD_ADDR;
	transParam.uiCS = info->flash_info->chip_sel;

	nand_host_setup_transfer(info, &transParam, 0, 0, _SINGLE_PAGE);

	if (info->flash_info->device_size > SPI_FLASH_16MB_BOUN) {
		nand_hostSetupAddressCycle(0, NAND_ROW_ADDRESS_4_CYCLE_CNT, NAND_NOT_CONFIGED_ADDRESS_CYCLE_CNT);
	} else {
		nand_hostSetupAddressCycle(0, NAND_ROW_ADDRESS_3_CYCLE_CNT, NAND_NOT_CONFIGED_ADDRESS_CYCLE_CNT);
	}

	nand_host_send_command(info, uiCmdCycle.Reg, FALSE);

	ret = nand_cmd_wait_complete(info);

	if (ret != E_OK) {
		nand_hostSetCSActive(info, NAND_SPI_CS_HIGH);
		nand_hostConfigChipSelOperationMode(info, NAND_SPI_CS_AUTO_MODE);
		if (info->enter_4byte_addr_mode == 1) {
			leave_4byte_address(info);
		}

		return ret;
	}

	transParam.fifoDir = _FIFO_WRITE;
	if (byte_size < SPIFLASH_MAX_PROGRAM_BYTE_AT_ONCE) {
		transParam.transMode = _PIO_MODE;
	} else {
		transParam.transMode = _DMA_MODE;
		NAND_SETREG(info, NAND_DMASTART_REG_OFS, (uintptr_t)pbuf);
	}

	transParam.type = _SPI_WRITE_N_BYTES;

	if (info->flash_info->nor_quad_support) {
		nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_QUAD_MODE, NAND_SPI_ORDER_MODE_1);
	} else {
		nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_NORMAL_DUPLEX_MODE, NAND_SPI_ORDER_MODE_1);
	}

	nand_host_setup_transfer(info, &transParam, 0, byte_size, _SINGLE_PAGE);

	nand_host_send_command(info, uiCmdCycle.Reg, FALSE);

	if (byte_size < SPIFLASH_MAX_PROGRAM_BYTE_AT_ONCE) {
		ret = nand_host_transmit_data(info, (u8 *)pbuf, byte_size, _PIO_MODE);
	} else {
		ret = nand_cmd_wait_complete(info);
	}

	nand_hostSetCSActive(info, NAND_SPI_CS_HIGH);
	nand_hostConfigChipSelOperationMode(info, NAND_SPI_CS_AUTO_MODE);

#if defined(CONFIG_TARGET_NA51055) || defined(CONFIG_TARGET_NA51089)
	/*Enable clk_auto_gating after set cs_manual*/
	*(u32 *) 0xF00200B0 |= 0x80000;
#endif

	nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_NORMAL_DUPLEX_MODE, NAND_SPI_ORDER_MODE_1);

	if (ret != E_OK) {
		if (info->enter_4byte_addr_mode == 1) {
			leave_4byte_address(info);
		}
		return ret;
	}

	ret = spiflash_wait_ready(info, &ucSts);

	spiflash_disable_write_latch(info);

	if (info->enter_4byte_addr_mode == 1) {
		leave_4byte_address(info);
	}

	return ret;
}

int spinor_erase_sector(struct drv_nand_dev_info *info, struct spi_flash *flash, u32 byte_addr)
{
	union T_NAND_COMMAND_REG cmd_cycle = {0};
	struct smc_setup_trans transParam;
	u8 status;
	int ret;

	spiflash_wait_ready(info, &status);

	if (info->enter_4byte_addr_mode == 1) {
		enter_4byte_address(info);
	}

	spiflash_enable_write_latch(info);

	//row addr = 0x44332211 --> 0x44 --> 0x33 --> 0x22 --> 0x11
	//                                            ___     ___________
	//use command + 1 cycle row address cycle ___|cmd|___|Row1stcycle|___

	//use block erase cycle
	//   ____    ______    ______    ______
	//___|cmd|___|addr1|___|addr2|___|addr3|___| ... |___|cmd2|___ ...
	//                                                            ____
	//                                                            |sts|__ ...

	cmd_cycle.Bit.CMD_CYC_1ST = flash->erase_opcode;
	cmd_cycle.Bit.CMD_CYC_2ND = FLASH_CMD_RDSR;
	transParam.colAddr = 0;
	transParam.rowAddr = byte_addr;
	transParam.fifoDir = _FIFO_WRITE;
	transParam.transMode = _PIO_MODE;
	transParam.type = _BLK_ERSE;
	transParam.uiCS = info->flash_info->chip_sel;

	if (info->flash_info->device_size > SPI_FLASH_16MB_BOUN) {
		nand_hostSetupAddressCycle(0, NAND_ROW_ADDRESS_4_CYCLE_CNT, NAND_NOT_CONFIGED_COL_ADDRESS_CYCLE_CNT);
	} else {
		nand_hostSetupAddressCycle(0, NAND_ROW_ADDRESS_3_CYCLE_CNT, NAND_NOT_CONFIGED_COL_ADDRESS_CYCLE_CNT);
	}

	nand_hostSetupStatusCheckBit(info, NAND_STS_BIT_0, NAND_STS_BIT_0_EQUAL_0);

	nand_host_setup_transfer(info, &transParam, 0, 0, _SINGLE_PAGE);

	nand_host_send_command(info, cmd_cycle.Reg, TRUE);

	ret = nand_cmd_wait_complete(info);

	spiflash_disable_write_latch(info);

	if (info->enter_4byte_addr_mode == 1) {
		leave_4byte_address(info);
	}

	return ret;
}

int spinor_erase_chip(struct drv_nand_dev_info *info)
{
	struct smc_setup_trans transParam;
	u8 status;
	int ret;

	spiflash_wait_ready(info, &status);

	spiflash_enable_write_latch(info);

	do {
		transParam.colAddr = 0;
		transParam.rowAddr = 0;
		transParam.fifoDir = _FIFO_READ;
		transParam.transMode = _PIO_MODE;
		transParam.type = _CMD;
		transParam.uiCS = info->flash_info->chip_sel;

		nand_host_setup_transfer(info, &transParam, 0, 0, _SINGLE_PAGE);

		nand_host_send_command(info, CMD_ERASE_CHIP, FALSE);

		ret = nand_cmd_wait_complete(info);

		if (ret != E_OK) {
			break;
		}
	} while (0);

	spiflash_disable_write_latch(info);

	return ret;
}


static int spinor_pio_operation(struct drv_nand_dev_info *info, u32 byte_addr, u32 byte_size, u8 *pbuf)
{
	struct smc_setup_trans transParam;
	union T_NAND_COMMAND_REG uiCmdCycle = {0};
	int ret;

#if defined(CONFIG_TARGET_NA51055) || defined(CONFIG_TARGET_NA51089)
	/*Disable clk_auto_gating before set cs_manual*/
	*(u32 *) 0xF00200B0 &= ~0x80000;
#endif

	nand_hostSetCSActive(info, NAND_SPI_CS_HIGH);

	nand_hostConfigChipSelOperationMode(info, NAND_SPI_CS_MANUAL_MODE);

	nand_hostSetCSActive(info, NAND_SPI_CS_LOW);

	if (info->flash_info->nor_read_mode == SPI_NOR_QUAD_READ) {
		if (info->flash_info->device_size > SPI_FLASH_16MB_BOUN) {
			uiCmdCycle.Bit.CMD_CYC_1ST = FLASH_CMD_QUAD_READ_NORMAL_4BYTE;
		} else {
			uiCmdCycle.Bit.CMD_CYC_1ST = FLASH_CMD_QUAD_READ_NORMAL;
		}
	} else if (info->flash_info->nor_read_mode == SPI_NOR_DUAL_READ) {
		if (info->flash_info->device_size > SPI_FLASH_16MB_BOUN) {
			uiCmdCycle.Bit.CMD_CYC_1ST = FLASH_CMD_DUAL_READ_4BYTE;
		} else {
			uiCmdCycle.Bit.CMD_CYC_1ST = FLASH_CMD_DUAL_READ;
		}
	} else {
		if (info->flash_info->device_size > SPI_FLASH_16MB_BOUN) {
			uiCmdCycle.Bit.CMD_CYC_1ST = FLASH_CMD_READ_4BYTE;
		} else {
			uiCmdCycle.Bit.CMD_CYC_1ST = FLASH_CMD_READ;
		}
	}

	transParam.colAddr      = 0;
	transParam.rowAddr      = byte_addr;
	transParam.fifoDir      = _FIFO_READ;
	transParam.transMode    = _PIO_MODE;
	transParam.type         = _CMD_ADDR;
	transParam.uiCS         = info->flash_info->chip_sel;

	nand_host_setup_transfer(info, &transParam, 0, 0, _SINGLE_PAGE);

	if (info->flash_info->device_size > SPI_FLASH_16MB_BOUN) {
		nand_hostSetupAddressCycle(0, NAND_ROW_ADDRESS_4_CYCLE_CNT, NAND_COL_ADDRESS_1_CYCLE_CNT);
	} else {
		nand_hostSetupAddressCycle(0, NAND_ROW_ADDRESS_3_CYCLE_CNT, NAND_COL_ADDRESS_1_CYCLE_CNT);
	}

	nand_host_send_command(info, uiCmdCycle.Reg, FALSE);

	ret = nand_cmd_wait_complete(info);


	if (ret != E_OK) {
		nand_hostSetCSActive(info, NAND_SPI_CS_HIGH);
		nand_hostConfigChipSelOperationMode(info, NAND_SPI_CS_AUTO_MODE);
		return ret;
	}

	if (info->flash_info->nor_read_mode != SPI_NOR_NORMAL_READ) {
		transParam.colAddr    = 0;
		transParam.rowAddr    = 0;
		transParam.transMode  = _PIO_MODE;
		transParam.type       = _DUMMY_CLOCK;

		nand_host_setup_transfer(info, &transParam, 0, 0, _SINGLE_PAGE);

		NAND_SETREG(info, NAND_DUMMY_CLOCK_NUM_OFS, 0x7);

		nand_host_send_command(info, uiCmdCycle.Reg, FALSE);

		ret = nand_cmd_wait_complete(info);

		if (ret != E_OK) {
			nand_hostSetCSActive(info, NAND_SPI_CS_HIGH);
			nand_hostConfigChipSelOperationMode(info, NAND_SPI_CS_AUTO_MODE);
			return ret;
		}

		//Dummy 2 T
		if (((info->rdcr_status & 0xC0) == 0xC0) && (info->ops_freq > 96000000) && \
			(info->hspeed_dummy_cycle == 10)) {
			debug("SF: dummy 10T mode with RDCR 0x%08x\n", info->rdcr_status);

			transParam.type = _DUMMY_CLOCK;

			nand_host_setup_transfer(info, &transParam, 0, 0, _SINGLE_PAGE);

			NAND_SETREG(info, NAND_DUMMY_CLOCK_NUM_OFS, 0x1);

			nand_host_send_command(info, uiCmdCycle.Reg, FALSE);

			ret = nand_cmd_wait_complete(info);

			if (ret != E_OK) {
				nand_hostSetCSActive(info, NAND_SPI_CS_HIGH);
				nand_hostConfigChipSelOperationMode(info, NAND_SPI_CS_AUTO_MODE);
				return ret;
			}
		}
	}

	transParam.colAddr = 0;
	transParam.rowAddr = 0;
	transParam.fifoDir = _FIFO_READ;
	transParam.type = _SPI_READ_N_BYTES;

	if (info->flash_info->nor_read_mode == SPI_NOR_QUAD_READ) {
		nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_QUAD_MODE, NAND_SPI_ORDER_MODE_1);
	} else if (info->flash_info->nor_read_mode == SPI_NOR_DUAL_READ) {
		nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_DUAL_MODE, NAND_SPI_ORDER_MODE_1);
	} else {
		nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_NORMAL_DUPLEX_MODE, NAND_SPI_ORDER_MODE_1);
	}

	nand_host_setup_transfer(info, &transParam, 0, byte_size, _SINGLE_PAGE);

	nand_host_send_command(info, uiCmdCycle.Reg, FALSE);

	ret = nand_host_receive_data(info, pbuf, byte_size, _PIO_MODE);

	nand_hostSetCSActive(info, NAND_SPI_CS_HIGH);

	nand_hostConfigChipSelOperationMode(info, NAND_SPI_CS_AUTO_MODE);

	if (info->flash_info->nor_read_mode != SPI_NOR_NORMAL_READ) {
		nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_NORMAL_DUPLEX_MODE, NAND_SPI_ORDER_MODE_1);
	}

#if defined(CONFIG_TARGET_NA51055) || defined(CONFIG_TARGET_NA51089)
	/*Enable clk_auto_gating after set cs_manual*/
	*(u32 *) 0xF00200B0 |= 0x80000;
#endif

	return ret;
}


/*
    Read SPI flash data

    Read data from SPI flash.

    @param[in] byte_addr   Byte address of flash.
    @param[in] byte_size   Byte size of read data.
    @param[out] pbuf        Pointer point to store read data

    @return
        - @b E_OK: erase success
        - @b E_CTX: driver not opened
        - @b Else: read fail
*/
int spinor_read_operation(struct drv_nand_dev_info *info, u32 byte_addr, u32 byte_size, u8 *pbuf)
{
	u32 addr_index;
	u32 align_length, remain_length;
	int ret = E_SYS;

	align_length = (byte_size / CONFIG_SYS_CACHELINE_SIZE) * CONFIG_SYS_CACHELINE_SIZE;
	remain_length = byte_size - align_length;
	addr_index = byte_addr;

	if (align_length) {
		ret = spiflash_read_data(info, addr_index, align_length, pbuf);
		if (ret != E_OK) {
			printf("%s: fail at flash address 0x%x\r\n", __func__, addr_index);
		} else {
			debug("r");
		}
		invalidate_dcache_range((uintptr_t)pbuf, (uintptr_t)(pbuf + align_length));
	}

	/*Handle if length is not cacheline alignment*/
	if (remain_length) {
		ret = spinor_pio_operation(info, addr_index + align_length, remain_length, pbuf + align_length);
		if (ret != E_OK) {
			printf("%s: fail at flash address 0x%x length 0x%x\r\n", __func__, addr_index + align_length, remain_length);
		}
	}

	return ret;
}

/*
    Program a sector

    Program a sector of SPI flash.

    @param[in] byte_addr   Byte address of programed sector. Should be sector size alignment.
    @param[in] sector_size Byte size of one secotr. Should be sector size.
    @param[in] pbuf         pointer point to written data

    @return
        - @b E_OK: erase success
        - @b E_CTX: driver not opened
        - @b E_MACV: byte_addr is set into s/w write protect detect region
        - @b E_TMOUT: flash ready timeout
        - @b E_PAR: byte_addr is not sector size alignment
        - @b Else: program fail
*/
int spinor_program_operation(struct drv_nand_dev_info *info, u32 byte_addr, u32 sector_size, u8 *pbuf)
{
	u32 program_cycle;
	u32 addr_index;
	u32 cycle_index;
	int ret = E_SYS;
	u32 p_flush_buf = (uintptr_t)pbuf;
	u32 cacheline_mask = CONFIG_SYS_CACHELINE_SIZE - 1;
	size_t align_size = roundup(sector_size, ARCH_DMA_MINALIGN);

	program_cycle = sector_size / SPIFLASH_MAX_PROGRAM_BYTE_AT_ONCE;
	addr_index = byte_addr;

	if (p_flush_buf & (CONFIG_SYS_CACHELINE_SIZE - 1)) {
		p_flush_buf &= ~cacheline_mask;
		flush_dcache_range(p_flush_buf, p_flush_buf + align_size + CONFIG_SYS_CACHELINE_SIZE);
	} else {
		flush_dcache_range(p_flush_buf, p_flush_buf + align_size);
	}

	for (cycle_index = 0; cycle_index < program_cycle; cycle_index++, addr_index += SPIFLASH_MAX_PROGRAM_BYTE_AT_ONCE) {
		ret = spiflash_page_program(info, addr_index, SPIFLASH_MAX_PROGRAM_BYTE_AT_ONCE, (pbuf + (cycle_index * SPIFLASH_MAX_PROGRAM_BYTE_AT_ONCE)));

		if (ret != E_OK) {
			printf("%s: fail at flash address 0x%x\r\n", __func__, addr_index);
			break;
		} else {
			debug("W");
		}

		sector_size -= SPIFLASH_MAX_PROGRAM_BYTE_AT_ONCE;
	}

	if (sector_size) {
		ret = spiflash_page_program(info, addr_index, sector_size, (pbuf + (cycle_index * SPIFLASH_MAX_PROGRAM_BYTE_AT_ONCE)));
	}

	return ret;
}

int spinor_read_status(struct drv_nand_dev_info *info, u8 status_set, u8 *status)
{
	struct smc_setup_trans trans_param;
	u32 spi_cmd = 0;
	int ret = E_OK;

	do {
		trans_param.colAddr = 0;
		trans_param.rowAddr = 0;
		trans_param.fifoDir = _FIFO_READ;
		trans_param.transMode = _PIO_MODE;
		trans_param.type = _CMD_STS;
		trans_param.uiCS = 0;

		nand_host_setup_transfer(info, &trans_param, 0, 0, _SINGLE_PAGE);

		if (status_set == NAND_SPI_NOR_STS_RDSR_1) {
			spi_cmd = FLASH_CMD_RDSR;
		} else if (status_set == NAND_SPI_NOR_STS_RDCR) {
			spi_cmd = FLASH_CMD_RDCR;
		} else {
			spi_cmd = FLASH_CMD_RDSR2;
		}

		nand_host_send_command(info, spi_cmd, FALSE);

		ret = nand_cmd_wait_complete(info);

	} while (0);

	*status = NAND_GETREG(info, NAND_CTRL_STS_REG_OFS);

	return ret;
}

int spinor_write_status(struct drv_nand_dev_info *info, u8 status_set, u8 status)
{
	union T_NAND_COMMAND_REG cmd_cycle = {0};
	struct smc_setup_trans trans_param;
	u32 spi_cmd = 0;
	int ret = E_OK;
	u8 wait_status = 0;

	spiflash_enable_write_latch(info);

	do {
		//row addr = 0x44332211 --> 0x44 --> 0x33 --> 0x22 --> 0x11
		//                                            ___     ___________
		//use command + 1 cycle row address cycle ___|cmd|___|Row1stcycle|___

		if (status_set == NAND_SPI_NOR_STS_WRSR_1) {
			cmd_cycle.Bit.CMD_CYC_1ST = FLASH_CMD_WRSR;
		} else {
			cmd_cycle.Bit.CMD_CYC_1ST = FLASH_CMD_WRSR2;
		}

		spi_cmd = status;

		trans_param.colAddr = 0;
		trans_param.rowAddr = spi_cmd;
		trans_param.fifoDir = _FIFO_READ;
		trans_param.transMode = _PIO_MODE;
		trans_param.type = _CMD_ADDR;
		trans_param.uiCS = 0;

		nand_host_setup_transfer(info, &trans_param, 0, 0, _SINGLE_PAGE);

		if (info->flash_info->qe_flags & SPI_FLASH_BUSWIDTH_QUAD_TYPE4) {
			nand_hostSetupAddressCycle(0, NAND_ROW_ADDRESS_2_CYCLE_CNT, NAND_COL_ADDRESS_1_CYCLE_CNT);
		} else {
			nand_hostSetupAddressCycle(0, NAND_ROW_ADDRESS_1_CYCLE_CNT, NAND_COL_ADDRESS_1_CYCLE_CNT);
		}

		nand_host_send_command(info, cmd_cycle.Reg, FALSE);

		ret = nand_cmd_wait_complete(info);

		if (ret) {
			return ret;
		}

	} while (0);

	ret = spiflash_wait_ready(info, &wait_status);
	if (ret) {
		printf("wait ready timeout with status 0x%x\n", wait_status);
	}

	return ret;
}

int spi_nor_set_status_half_word(struct drv_nand_dev_info *info, u16 hstatus)
{
	union T_NAND_COMMAND_REG cmd_cycle = {0};
	struct smc_setup_trans trans_param;
	u32 spi_cmd = 0;
	int ret = E_OK;
	u8 wait_status = 0;

	spiflash_enable_write_latch(info);

	do {
		cmd_cycle.Bit.CMD_CYC_1ST = FLASH_CMD_WRSR;

		spi_cmd = hstatus;

		trans_param.colAddr = 0;
		trans_param.rowAddr = spi_cmd;
		trans_param.fifoDir = _FIFO_READ;
		trans_param.transMode = _PIO_MODE;
		trans_param.type = _CMD_ADDR;
		trans_param.uiCS = 0;

		nand_host_setup_transfer(info, &trans_param, 0, 0, _SINGLE_PAGE);

		/*Note CMD address is msb first*/
		/*Ex 0x00000740 will send __|07|__|40|__*/
		nand_hostSetupAddressCycle(0, NAND_ROW_ADDRESS_2_CYCLE_CNT, NAND_COL_ADDRESS_1_CYCLE_CNT);

		nand_host_send_command(info, cmd_cycle.Reg, FALSE);

		ret = nand_cmd_wait_complete(info);

		if (ret) {
			return ret;
		}
	} while (0);

	ret = spiflash_wait_ready(info, &wait_status);
	if (ret) {
		printf("wait ready timeout with status 0x%x\n", wait_status);
	}

	return ret;
}

int spinor_read_sfdp(struct drv_nand_dev_info *info, u32 addr, u32 byte_size, u8 *pbuf)
{
	struct smc_setup_trans transParam;
	union T_NAND_COMMAND_REG uiCmdCycle = {0};
	int ret;

#if defined(CONFIG_TARGET_NA51055) || defined(CONFIG_TARGET_NA51089)
	/*Disable clk_auto_gating before set cs_manual*/
	*(u32 *) 0xF00200B0 &= ~0x80000;
#endif

	nand_hostSetCSActive(info, NAND_SPI_CS_HIGH);

	nand_hostConfigChipSelOperationMode(info, NAND_SPI_CS_MANUAL_MODE);

	nand_hostSetCSActive(info, NAND_SPI_CS_LOW);

	uiCmdCycle.Bit.CMD_CYC_1ST = FLASH_CMD_RD_SFDP;

	transParam.colAddr      = 0;
	transParam.rowAddr      = addr;
	transParam.fifoDir      = _FIFO_READ;
	transParam.transMode    = _PIO_MODE;
	transParam.type         = _CMD_ADDR;
	transParam.uiCS         = info->flash_info->chip_sel;

	nand_host_setup_transfer(info, &transParam, 0, 0, _SINGLE_PAGE);

	nand_hostSetupAddressCycle(0, NAND_ROW_ADDRESS_3_CYCLE_CNT, NAND_COL_ADDRESS_1_CYCLE_CNT);

	nand_host_send_command(info, uiCmdCycle.Reg, FALSE);

	ret = nand_cmd_wait_complete(info);

	if (ret != E_OK) {
		nand_hostSetCSActive(info, NAND_SPI_CS_HIGH);
		nand_hostConfigChipSelOperationMode(info, NAND_SPI_CS_AUTO_MODE);
		return ret;
	}

	transParam.colAddr    = 0;
	transParam.rowAddr    = 0;
	transParam.transMode  = _PIO_MODE;
	transParam.type       = _DUMMY_CLOCK;

	nand_host_setup_transfer(info, &transParam, 0, 0, _SINGLE_PAGE);

	NAND_SETREG(info, NAND_DUMMY_CLOCK_NUM_OFS, 0x7);

	nand_host_send_command(info, uiCmdCycle.Reg, FALSE);

	ret = nand_cmd_wait_complete(info);

	if (ret != E_OK) {
		nand_hostSetCSActive(info, NAND_SPI_CS_HIGH);
		nand_hostConfigChipSelOperationMode(info, NAND_SPI_CS_AUTO_MODE);
		return ret;
	}

	transParam.colAddr = 0;
	transParam.rowAddr = 0;
	transParam.fifoDir = _FIFO_READ;
	transParam.type = _SPI_READ_N_BYTES;

	nand_hostSetSPIIORegister(info, NAND_SPI_CS_ACTIVE_LOW, NAND_SPI_NORMAL_DUPLEX_MODE, NAND_SPI_ORDER_MODE_1);

	nand_host_setup_transfer(info, &transParam, 0, byte_size, _SINGLE_PAGE);

	nand_host_send_command(info, uiCmdCycle.Reg, FALSE);

	ret = nand_host_receive_data(info, pbuf, byte_size, _PIO_MODE);

	nand_hostSetCSActive(info, NAND_SPI_CS_HIGH);

	nand_hostConfigChipSelOperationMode(info, NAND_SPI_CS_AUTO_MODE);

#if defined(CONFIG_TARGET_NA51055) || defined(CONFIG_TARGET_NA51089)
	/*Enable clk_auto_gating after set cs_manual*/
	*(u32 *) 0xF00200B0 |= 0x80000;
#endif

	return ret;
}

/*
    Enter 4BYTE Address mode

*/
int enter_4byte_address(struct drv_nand_dev_info *info)
{
	struct smc_setup_trans transParam;
	int ret;

	do {
		transParam.colAddr      = 0;
		transParam.rowAddr      = 0;
		transParam.fifoDir      = _FIFO_READ;
		transParam.transMode    = _PIO_MODE;
		transParam.type         = _CMD;
		transParam.uiCS         = info->flash_info->chip_sel;

		nand_host_setup_transfer(info, &transParam, 0, 0, _SINGLE_PAGE);
		nand_host_send_command(info, FLASH_CMD_ENTER_4BYTE_ADDR, FALSE);
		ret = nand_cmd_wait_complete(info);
	} while (0);

	return ret;
}

/*
    Leave 4BYTE Address mode

*/
int leave_4byte_address(struct drv_nand_dev_info *info)
{
	struct smc_setup_trans transParam;
	int ret;

	do {
		transParam.colAddr      = 0;
		transParam.rowAddr      = 0;
		transParam.fifoDir      = _FIFO_READ;
		transParam.transMode    = _PIO_MODE;
		transParam.type         = _CMD;
		transParam.uiCS         = info->flash_info->chip_sel;

		nand_host_setup_transfer(info, &transParam, 0, 0, _SINGLE_PAGE);
		nand_host_send_command(info, FLASH_CMD_LEAVE_4BYTE_ADDR, FALSE);
		ret = nand_cmd_wait_complete(info);
	} while (0);

	return ret;
}