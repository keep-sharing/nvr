/*
	@file       nvt_flash_spi.c
	@ingroup    mIStgNAND

	@brief      NAND low layer driver which will access to NAND controller

	@note       Nothing

	Copyright   Novatek Microelectronics Corp. 2011.  All rights reserved.

*/
#include "nvt_flash_spi_int.h"
#include "nvt_flash_spi_reg.h"

/*
	Configure nand host transfer register

	This function configure controller transfer related register

	@param[in]  transParam      Setup transfer configuration
	@param[in]  pageCount       Transfer page count
	@param[in]  length          Transfer length
	@param[in]  multiPageSelect multi page / multi spare / single page

	@return void
*/
void nand_host_setup_transfer(struct drv_nand_dev_info *info, \
				struct smc_setup_trans *transParam, \
				uint32_t pageCount, \
				uint32_t length, \
				uint32_t multiPageSelect)
{
	const struct nvt_nand_flash *f = info->flash_info;
	uint32_t value3 = 0;
	uint32_t value = 0;

	uint32_t value1 = 0;

	value3 = NAND_GETREG(info, NAND_INTMASK_REG_OFS);
	value3 &= ~NAND_COMP_INTREN;
	NAND_SETREG(info, NAND_INTMASK_REG_OFS, value3);

	if (pageCount > 0) {
		NAND_SETREG(info, NAND_PAGENUM_REG_OFS, pageCount);
		value = multiPageSelect;
	}

	//Even if not use FIFO, FIFO dir & PIO/DMA mode need to set
	NAND_SETREG(info, NAND_FIFO_CTRL_REG_OFS, (transParam->fifoDir | transParam->transMode));

	value1 = NAND_GETREG(info, NAND_CTRL0_REG_OFS);

	NAND_SETREG(info, NAND_CTRL0_REG_OFS, value);


	// Set column address
	NAND_SETREG(info, NAND_COLADDR_REG_OFS, transParam->colAddr);

	// Set row address
	NAND_SETREG(info, NAND_ROWADDR_REG_OFS, transParam->rowAddr);

	// Set data length
	NAND_SETREG(info, NAND_DATALEN_REG_OFS, length);

	if((value1 & _PROTECT1_EN) == _PROTECT1_EN)
		value |= _PROTECT1_EN;

	if((value1 & _PROTECT2_EN) == _PROTECT2_EN)
		value |= _PROTECT2_EN;
#if (defined(CONFIG_TARGET_NA51090) || defined(CONFIG_TARGET_NA51090_A64))
	value |= (transParam->type);
#else
	//NT9852x/56x cs @ 0x20(NAND_CTRL0_REG_OFS) bit[8]
	value |= (transParam->type | (transParam->uiCS << 8));
#endif
	if(transParam->fifoDir == _FIFO_READ)
		value |= _NAND_WP_EN;

	NAND_SETREG(info, NAND_CTRL0_REG_OFS, value);

#if (defined(CONFIG_TARGET_NA51090) || defined(CONFIG_TARGET_NA51090_A64) || defined(CONFIG_TARGET_NA51103) || defined(CONFIG_TARGET_NA51103_A64))
	//config CS
	value3 = NAND_GETREG(info, NAND_SPI_CFG_REG_OFS);
	value3 &= ~(1<<3);
	value3 |= (transParam->uiCS<<3);
	NAND_SETREG(info, NAND_SPI_CFG_REG_OFS, value3);
#endif

	value3 = NAND_GETREG(info, NAND_CTRL_STS_REG_OFS);
	value3 |= NAND_COMP_STS;
	NAND_SETREG(info, NAND_CTRL_STS_REG_OFS, value3);

	// Set control0 register
	if ((transParam->type == _DATA_RD3) && (transParam->transMode == _PIO_MODE)) {
		NAND_DEBUG_MSG(" op cmd(0x16), read spare area only, do not use fifo_en\r\n");
	} else {
		if (length && ((length >= GET_NAND_SPARE_SIZE(f)*(1+info->flash_info->spare_size)) || \
						(length == 8) || (length == 4) || (transParam->type == _SPI_READ_N_BYTES) || \
						multiPageSelect == _MULTI_SPARE)) {
			NAND_SETREG(info, NAND_FIFO_CTRL_REG_OFS, \
				(transParam->fifoDir | transParam->transMode));

			NAND_SETREG(info, NAND_FIFO_CTRL_REG_OFS, (_FIFO_EN | \
								transParam->fifoDir | \
								transParam->transMode));
		}
	}
	NAND_DEBUG_MSG("  NAND_FIFO_CTRL_REG: 0x%x\r\n", NAND_GETREG(info, NAND_FIFO_CTRL_REG_OFS));

	value3 = NAND_GETREG(info, NAND_INTMASK_REG_OFS);
	value3 |= NAND_COMP_INTREN;
	NAND_SETREG(info, NAND_INTMASK_REG_OFS, value3);

	//config NAND nor type
	nand_hostSetNandType(info, info->flash_info->config_nand_type);
}


/*
	Configure SM transfer command

	Command send to NAND

	@param[in]  command     nand command
	@param[in]  bTmOutEN    time out enable / disable

	@return void
*/
void nand_host_send_command(struct drv_nand_dev_info *info, uint32_t command,
							uint32_t bTmOutEN)
{
	union T_NAND_CTRL0_REG RegCtrl0;

#if defined(CONFIG_TARGET_NA51102) || defined(CONFIG_TARGET_NA51102_A64)
	// Enable AXI ch
	nand_host_axi_ch_disable(info, 0);
#endif

	// Set command
	NAND_SETREG(info, NAND_COMMAND_REG_OFS, command);

	RegCtrl0.Reg = NAND_GETREG(info, NAND_CTRL0_REG_OFS);

	RegCtrl0.Bit.OPER_EN = 1;

	RegCtrl0.Bit.TIMEOUT_EN = bTmOutEN;

	NAND_SETREG(info, NAND_CTRL0_REG_OFS, RegCtrl0.Reg);
}


/*
	NAND controller host receive data routing

	After read operation related command issued, called this function to reveive data

	@param[in]  	buffer      buffer receive to
			length      receive buffer length
			transMode   PIO/DMA mode usage

	@return
		- @b E_OK           receive data success
		- @b E_SYS          error status
*/
int nand_host_receive_data(struct drv_nand_dev_info *info, uint8_t *buffer,
					uint32_t length, uint32_t transMode)
{
	uint32_t index = 0, fifo_size = 0, tempBuffer = 0;
	if (nvt_get_chip_id() == CHIP_NA51090 || nvt_get_chip_id() == CHIP_NA51102 || nvt_get_chip_id() == CHIP_NA51103) {
		fifo_size = 128;
	} else {
		fifo_size = 64;
	}

	if(transMode == _PIO_MODE) {
		while(length) {
			if(length >= fifo_size) {
				if(NAND_GETREG(info, NAND_FIFO_STS_REG_OFS) & _FIFO_FULL) {
					for (index = fifo_size; index > 0; index -= 4) {
						*((uint32_t *)buffer) = NAND_GETREG(info, NAND_DATAPORT_REG_OFS);
						length -= 4;
						buffer +=4;
					}
				}
			} else {
				if((NAND_GETREG(info, NAND_FIFO_STS_REG_OFS) & _FIFO_CNT_MASK) == ((length+3)/4)) {
					tempBuffer = NAND_GETREG(info, NAND_DATAPORT_REG_OFS);
					if(length < 4) {
						for(index = length; index > 0; index--) {
							*((uint8_t *)buffer) = tempBuffer & (0xFF) ;
							tempBuffer >>= 8;
							length -= 1;
							buffer += 1;
						}
					} else {
						*((uint32_t *)buffer) = tempBuffer;
						length -= 4;
						buffer += 4;
					}
				}
			}
		}
	}

	return nand_cmd_wait_complete(info);
}


/*
	NAND controller host transmit data routing

	After write operation related command issued, called this function to transmit data

	@param[in]  	buffer      buffer transmit from
			length      transmit buffer length
			transMode   PIO/DMA mode usage

	@return
		- @b E_OK           transmit data success
		- @b E_SYS          error status
*/
int nand_host_transmit_data(struct drv_nand_dev_info *info, uint8_t *buffer, uint32_t length, uint32_t transMode)
{
	uint32_t index = 0, fifo_size, tempBuffer = 0;
	uint8_t tempData = 0;
	BOOL not_aligned = 0;
	if (nvt_get_chip_id() == CHIP_NA51090 || nvt_get_chip_id() == CHIP_NA51102 || nvt_get_chip_id() == CHIP_NA51103) {
		fifo_size = 128;
	} else {
		fifo_size = 64;
	}

	if(transMode == _PIO_MODE) {
		NAND_SETREG(info, NAND_FIFO_CTRL_REG_OFS, (_FIFO_EN | NAND_GETREG(info, NAND_FIFO_CTRL_REG_OFS)));
		
		while(length) {
			if(length >= fifo_size) {
				if(NAND_GETREG(info, NAND_FIFO_STS_REG_OFS) & _FIFO_EMPTY) {
					for (index = fifo_size; index > 0; index -= 4) {
						NAND_SETREG(info, NAND_DATAPORT_REG_OFS, *((uint32_t *)buffer));
						length -= 4;
						buffer += 4;
					}
				}
			} else {
				if((NAND_GETREG(info, NAND_FIFO_STS_REG_OFS) & _FIFO_EMPTY)) {
					while(length) {
						if(length < 4) {
							for(index = 0; index < length; index++) {
								tempData = (*((uint8_t *)buffer));
								tempBuffer |= (tempData << (8 * index));
								buffer += 1;
							}
							NAND_SETREG(info, NAND_DATAPORT_REG_OFS, tempBuffer);
							length = 0;
							not_aligned = 1;
						} else {
							NAND_SETREG(info, NAND_DATAPORT_REG_OFS, *((uint32_t *)buffer));
							length -= 4;
							buffer += 4;
						}
					}
				}
			}
		}

		if(not_aligned) {
			mdelay(1);
			tempBuffer = NAND_GETREG(info, NAND_FIFO_CTRL_REG_OFS);
        	tempBuffer &= ~(0x1);
			NAND_SETREG(info, NAND_FIFO_CTRL_REG_OFS, tempBuffer);
		}
	}

	return nand_cmd_wait_complete(info);
}


/*
	Check NAND read ready bit and make sure status register is ready

	@return E_OK : ready
		E_TMOUT: busy
*/
int nand_host_check_ready(struct drv_nand_dev_info *info)
{
#if (CONFIG_NVT_NAND_TYPE == 0)
	uint32_t i;

	i = 0;
	do {
		i++;
		//For slow card
		if (i > 0x48000) {
			printf("Status always busy\n\r");
			return -1;
		}
	} while ((NAND_GETREG(info, NAND_CTRL_STS_REG_OFS) & NAND_SM_BUSY) == 0);
#endif
	return E_OK;
}


/*
	Enable / Disable FIFO

	Enable and Disable FIFO of NAND controller

	@param[in]  uiEN     enable / disable

	@return void
*/
void nand_host_set_fifo_enable(struct drv_nand_dev_info *info, uint32_t uiEN)
{
	union T_NAND_FIFO_CTRL_REG RegFIFOCtrl;

	RegFIFOCtrl.Reg = NAND_GETREG(info, NAND_FIFO_CTRL_REG_OFS);

	RegFIFOCtrl.Bit.FIFO_EN = uiEN;

	NAND_SETREG(info, NAND_FIFO_CTRL_REG_OFS, RegFIFOCtrl.Reg);

	if(uiEN == FALSE)
		while((NAND_GETREG(info, NAND_FIFO_CTRL_REG_OFS) & _FIFO_EN) != 0);
}


/*
	Set NAND controller error correction SRAM access right(for CPU or NAND controller)

	Set register to switch error information SRAM access right to CPU or NAND controller

	@param[in]  bcpu_Acc
		- @b TRUE       switch error information sram area access right to CPU
		- @b FALSE      Switch error information sram area access right to NAND controller access

	@return
		- void
*/
void nand_host_set_cpu_access_err(struct drv_nand_dev_info *info, uint32_t bcpu_Acc)
{
	union T_NAND_SRAM_ACCESS_REG  RegSramAcc = {0x00000000};
	uint8_t i;

	RegSramAcc.Reg = NAND_GETREG(info, NAND_SRAM_ACCESS_REG_OFS);

	RegSramAcc.Bit.ERROR_ACC = bcpu_Acc;

	NAND_SETREG(info, NAND_SRAM_ACCESS_REG_OFS,RegSramAcc.Reg);

	//dummy read to delay 200ns for SRAM ready
	if(bcpu_Acc == TRUE) {
		for(i = 0; i < SRAMRDY_DELAY; i++)
			RegSramAcc.Reg = NAND_GETREG(info, NAND_SRAM_ACCESS_REG_OFS);
	}
}


/*
	Set NAND controller spare SRAM area access right(for CPU or NAND controller)

	Set register to switch spare area SRAM access right to CPU or NAND controller

	@param[in]  bcpu_Acc
		- @b TRUE       switch spare sram area access right to CPU
		- @b FALSE      switch spare sram area access right to NAND controller

	@return
		- void
*/
void nand_host_set_cpu_access_spare(struct drv_nand_dev_info *info,
							uint32_t bcpu_Acc)
{
	union T_NAND_SRAM_ACCESS_REG  RegSramAcc = {0x00000000};
	uint8_t i;

	RegSramAcc.Reg = NAND_GETREG(info, NAND_SRAM_ACCESS_REG_OFS);

	RegSramAcc.Bit.SPARE_ACC = bcpu_Acc;

	NAND_SETREG(info, NAND_SRAM_ACCESS_REG_OFS,RegSramAcc.Reg);

	//dummy read to delay 200ns for SRAM ready
	if(bcpu_Acc == TRUE) {
		for(i = 0; i < SRAMRDY_DELAY; i++)
			RegSramAcc.Reg = NAND_GETREG(info, NAND_SRAM_ACCESS_REG_OFS);
	}
}

/*
	NAND Reed Solomon ECC correction routing

	ECC correction flow by using reed solomon method,
	including correct error in spare area first 6 bytes

	@param[in]  Buf         Buffer to correct by RS correct routing
	@param[in]  section     section to be corrected

	@return
		- @b E_OK       success
		- @b E_CTX      uncorrect error
*/
int nand_host_correct_reedsolomon_ecc(struct drv_nand_dev_info *info,
					uint8_t * Buf, uint32_t uiSection)
{
	int32_t ret = 0;
	uint32_t i, j, shifts;
	uint32_t hrdata, rs_sts, rs_err, err_reg, err_addr, err_bit, mask_bit;

	nand_host_set_cpu_access_err(info, TRUE);

	rs_sts = NAND_GETREG(info, NAND_RSERR_STS0_REG_OFS);

	for(i = 0; i < uiSection; i++) {
		shifts = 4 * i;
		rs_err = (rs_sts >> shifts) & 0x7;
		if(rs_err == 0)
			continue;
		else if(rs_err == 5) {
			ret = E_CTX;
			continue;
		}

		for(j = 0; j < rs_err; j++) {

			err_reg = NAND_SEC0_EADDR0_REG_OFS + (16 * i) + (4 * j);

			hrdata = NAND_GETREG(info, (uintptr_t)err_reg);
			err_addr = hrdata & 0xFFFF;

			if(err_addr >= _512BYTE) {
				nand_host_set_cpu_access_spare(info, TRUE);
				err_addr -= _512BYTE;
				err_addr += (uint32_t)((CONFIG_SYS_NAND_BASE + NAND_SPARE00_REG_OFS) + (i * 16));
			}

			err_bit = (hrdata >> 16) & 0xFF;

			mask_bit = (err_addr & 0x03) * 8;

			if((hrdata & 0xFFFF) >= _512BYTE) {
				* (volatile uint32_t *)(uintptr_t) (err_addr & 0xFFFFFFFC) ^= (err_bit << mask_bit);
				nand_host_set_cpu_access_spare(info, FALSE);
			} else
				Buf[err_addr+ i*_512BYTE] ^= (err_bit);   // correct bit
		}
	}

	nand_host_set_cpu_access_err(info, FALSE);

	return ret;
}

/*
	NAND secondary ECC correction routing

	ECC correction flow by using secondary ECC method,

	@param[in]  Buf         Buffer to correct by RS correct routing
	@param[in]  section     section to be corrected

	@return
		- @b E_OK       success
		- @b E_CTX      uncorrect error
*/
int nand_host_correct_secondary_ecc(struct drv_nand_dev_info *info,
					uint32_t uiSection)
{
	int32_t ret = E_OK;
	uint32_t i, shifts;
	uint32_t hrdata, ham_sts, ham_err, err_reg, err_addr, err_bit;
	uint32_t mask_bit;

	nand_host_set_cpu_access_err(info, TRUE);

	ham_sts = NAND_GETREG(info, NAND_SECONDARY_ECC_STS_REG_OFS);

	for(i = 0; i < uiSection; i++) {   // 8 sections
		shifts = 2 * i;
		ham_err = (ham_sts >> shifts) & 0x3;
		if(ham_err == 0)
			continue;
		else if(ham_err == 2) {
			ret = E_CTX;
			continue;
		} else if(ham_err == 3) {
			ret = E_CTX;
			continue;
		}
		err_reg = NAND_SEC0_EADDR0_REG_OFS + (16 * i) + 4;

		hrdata = NAND_GETREG(info, (uintptr_t)err_reg);

		err_addr = hrdata & _ECC_SEC0_SECONDARY_ERRBYTE;
		err_bit = (hrdata & _ECC_SEC0_SECONDARY_ERRBIT)>>_ECC_SEC0_SECONDARY_BITSFT;

		if(err_addr >= _512BYTE) {
			nand_host_set_cpu_access_spare(info, TRUE);
			err_addr -= _512BYTE;
			err_addr += (uint32_t)(CONFIG_SYS_NAND_BASE + NAND_SPARE00_REG_OFS + i * 16);
			mask_bit = (err_addr & 0x03) * 8;

			* (volatile uint32_t *)(uintptr_t) (err_addr & 0xFFFFFFFC) ^= ((1<<err_bit) << mask_bit);

			nand_host_set_cpu_access_spare(info, FALSE);
		} else {
			printf("Secondary ECC should occurred > section size[0x%08x]\r\n", _512BYTE);
			ret = E_SYS;
			break;
		}
	}

	nand_host_set_cpu_access_err(info, FALSE);

	return ret;
}

/*
	NAND BCH ECC correction routing

	ECC correction flow by using BCH method
	including correct error in spare area first 12 bytes

	@param[in]  Buf         Buffer to correct by RS correct routing
	@param[in]  section     section to be corrected

	@return
		- @b E_OK       success
		- @b E_CTX      uncorrect error
*/
UINT32 gNANDSpareAlignSel = 1;
int nand_host_correct_bch_ecc(struct drv_nand_dev_info *info,
					uint8_t * Buf, uint32_t uiSection)
{
	int32_t ret = 0;
	uint32_t i, j, shifts;
	uint32_t hrdata, bch_sts, bch_err, err_reg, err_addr, err_bit, mask_bit;

	nand_host_set_cpu_access_err(info, TRUE);

	bch_sts = NAND_GETREG(info, NAND_BCHERR_STS0_REG_OFS);
	NAND_DEBUG_MSG("    BCH sts: 0x%08x \r\n", bch_sts);

	for(i = 0; i < uiSection; i++) {
		shifts = 4 * i;
		bch_err = (bch_sts >> shifts) & 0xf;
		if (bch_err == 0) {
			NAND_DEBUG_MSG("    no error\r\n");
			continue;
		} else if(bch_err == 9) {
			NAND_DEBUG_MSG("    uncorrect ecc error\r\n");
			ret = E_CTX;
			continue;
		}
		NAND_DEBUG_MSG("    %d-bit errors were detected \r\n", (int)bch_err);

		for(j = 0; j < bch_err; j++) {

			err_reg = NAND_SEC0_EADDR0_REG_OFS + (32 * i) + (4 * j);

			hrdata = NAND_GETREG(info, (uintptr_t)err_reg);

			err_addr = hrdata & 0xFFFF;
			err_bit = (hrdata >> 16) & 0xFF;
			err_addr -= _512BYTE*i;
			mask_bit = (err_addr & 0x03) * 8;	// u8 to u32

			if (err_addr >= _512BYTE) {
				// error bit in the spare area
				nand_host_set_cpu_access_spare(info, TRUE);

				err_addr += ((CONFIG_SYS_NAND_BASE + NAND_SPARE00_REG_OFS) - (0x200*(uiSection-i)));
				if (!gNANDSpareAlignSel && ((err_addr - i*0x10)&0xFF) > 0x2) {
					// no align, user area [0~2], ecc area[3~15]
					NAND_DEBUG_MSG("      sec[%d]BCH error bit[%03d][b%02x] (SpareEcc), skip\r\n", i, (err_addr&0x3FF), err_bit);
					nand_host_set_cpu_access_spare(info, FALSE);
					continue;
				} else if (gNANDSpareAlignSel){
					if (uiSection == 4) {
						// 2K page + align, Byte 0~11
						if ((err_addr&0xFF) > 0xB) {
							NAND_DEBUG_MSG("      sec[%d]BCH error bit[%03d][b%02x] (SpareEcc, B12~63), skip\r\n", i, err_addr, err_bit);
							nand_host_set_cpu_access_spare(info, FALSE);
							continue;
						}
					} else {
						// 4K page + align, Byte 0~23
						if ((err_addr&0xFF) > 0x17) {
							NAND_DEBUG_MSG("      sec[%d]BCH error bit[%03d][b%02x] (SpareEcc, B24~127), skip\r\n", i, err_addr, err_bit);
							nand_host_set_cpu_access_spare(info, FALSE);
							continue;
						}
					}
				}
				NAND_DEBUG_MSG("      sec[%d]BCH error bit[%03d][b%02x] (SpareUser)\r\n", i, err_addr, err_bit);

			} else {
				NAND_DEBUG_MSG("      sec[%d]BCH error bit[%03d][b%02x]\r\n", i, err_addr, err_bit);
			}

			if (err_addr >= _512BYTE) {
				* (volatile uint32_t *)(uintptr_t)(err_addr & 0xFFFFFFFC) ^= (err_bit << mask_bit);
				NAND_DEBUG_MSG("        verify spare[0x%08x]:0x%x\r\n", (err_addr & 0xFFFFFFFC), *(volatile uint32_t *)(uintptr_t)(err_addr & 0xFFFFFFFC));
				nand_host_set_cpu_access_spare(info, FALSE);
			} else {
				Buf[err_addr + i * _512BYTE] ^= (err_bit); // correct bit
			}
		}
	}

	nand_host_set_cpu_access_err(info, FALSE);

	return ret;
}

/*
	Reset NAND dll delay module

	Issue SW reset of NAND dll delay module

	@return void
*/
void nand_dll_reset(void)
{
	union T_NAND_DLL_PHASE_DLY_REG1 phy_dly;

	phy_dly.Reg = NAND_GETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS);
	phy_dly.Bit.PHY_SW_RESET = 1;
	NAND_SETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS, phy_dly.Reg);

	do {
		phy_dly.Reg = NAND_GETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS);
	} while(phy_dly.Bit.PHY_SW_RESET == 1);
}

/*
	Set NAND PHY SRC Clock

	Set phy clock source

	@return void
*/
void nand_phy_config(struct drv_nand_dev_info *info)
{
	union T_NAND_DLL_PHASE_DLY_REG1 phy_dly;
	union T_NAND_DLL_PHASE_DLY_REG2 phy_dly2;
	union T_NAND_MODULE0_REG RegMdule0 = {0};
	u32 chip_id = nvt_get_chip_id();

	phy_dly.Reg = NAND_GETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS);
	phy_dly2.Reg = NAND_GETREG(NULL, NAND_DLL_PHASE_DLY_REG2_OFS);
	RegMdule0.Reg = NAND_GETREG(NULL, NAND_MODULE0_REG_OFS);

	phy_dly.Bit.PHY_SAMPCLK_INV = 1;

	if (chip_id == CHIP_NA51055) {
		if (info->ops_freq > 96000000) {
			if (RegMdule0.Bit.SPI_FLASH_TYPE) {
				phy_dly.Bit.PHY_SRC_CLK_SEL = 1;
				phy_dly2.Bit.INDLY_SEL = 0x3F;
			} else {
				phy_dly.Bit.PHY_SRC_CLK_SEL = 0;
				phy_dly2.Bit.INDLY_SEL = 0x20;
			}
		} else {
			phy_dly.Bit.PHY_SRC_CLK_SEL = 0;
			phy_dly2.Bit.INDLY_SEL = 0x0;
		}
	} else if (chip_id == CHIP_NA51089) {
		phy_dly.Bit.PHY_SRC_CLK_SEL = 0;
		phy_dly.Bit.PHY_PAD_CLK_SEL = 1;
		phy_dly2.Bit.INDLY_SEL = 0x5;
	} else if (chip_id == CHIP_NA51103) {
		phy_dly.Bit.PHY_SRC_CLK_SEL = 0;
		phy_dly.Bit.PHY_PAD_CLK_SEL = 1;
		phy_dly2.Bit.INDLY_SEL = 0x0;
	} else if (chip_id == CHIP_NA51102) {
		phy_dly.Bit.PHY_SRC_CLK_SEL = 0;
		phy_dly.Bit.PHY_PAD_CLK_SEL = 1;
		if (info->ops_freq < 24000000) {
			phy_dly.Bit.PHY_SAMPCLK_INV = 0;
			phy_dly2.Bit.INDLY_SEL = 0x0;
		} else {
			phy_dly2.Bit.INDLY_SEL = 0x5;
		}
	} else {
		phy_dly.Bit.PHY_SRC_CLK_SEL = 0;
		phy_dly.Bit.PHY_PAD_CLK_SEL = 1;
		phy_dly2.Bit.INDLY_SEL = 0x8;
	}

	NAND_SETREG(NULL, NAND_DLL_PHASE_DLY_REG2_OFS, phy_dly2.Reg);
	NAND_SETREG(NULL, NAND_DLL_PHASE_DLY_REG1_OFS, phy_dly.Reg);
}

/*
	Configure nand host page size

	This function configure controller of SPI NAND page size

	@param[OUT]
	@return
		- @b_MULTI_PAGE
		- @b_SINGLE_PAGE

*/
UINT32 nand_hostGetMultipageSelect(void)
{
	union T_NAND_CTRL0_REG  RegCtrl = {0};

	RegCtrl.Reg = NAND_GETREG(info, NAND_CTRL0_REG_OFS);
	return (RegCtrl.Reg & 0x00030000);
}
/*
	set nand type

	set nand type

	@return
		- @b E_NOSPT    Not support
		- @b E_OK
*/
int nand_hostSetNandType(struct drv_nand_dev_info *info, NAND_TYPE_SEL uiNandType)
{
	union T_NAND_MODULE0_REG RegMdule0 = {0};
	int uRet = E_OK;

	RegMdule0.Reg = NAND_GETREG(info, NAND_MODULE0_REG_OFS);

	switch(uiNandType) {
	case NANDCTRL_ONFI_NAND_TYPE:
		RegMdule0.Bit.NAND_TYPE = 0;
		RegMdule0.Bit.SPI_FLASH_TYPE = 0;
	break;

	case NANDCTRL_SPI_NAND_TYPE:
		RegMdule0.Bit.NAND_TYPE = 1;
		RegMdule0.Bit.SPI_FLASH_TYPE = 0;
		//RegMdule0.Bit.COL_ADDR = NAND_1COL_ADDR;
		//RegMdule0.Bit.ROW_ADD = NAND_1ROW_ADDR;
	break;

	case NANDCTRL_SPI_NOR_TYPE:
		RegMdule0.Bit.NAND_TYPE = 1;
		RegMdule0.Bit.SPI_FLASH_TYPE = 1;
		//RegMdule0.Bit.COL_ADDR = NAND_1COL_ADDR;
		//RegMdule0.Bit.ROW_ADD = NAND_1ROW_ADDR;
	break;

	default:
		return E_NOSPT;
	break;
	}

	info->flash_info->config_nand_type = uiNandType;
	NAND_SETREG(info, NAND_MODULE0_REG_OFS, RegMdule0.Reg);

	return uRet;
}


/*
	Configure nand host row and column address cycle

	This function configure controller row & column cycle

	@param[in]  uiCS        chip select
	@param[in]  uiRow       row     cycle
	@param[in]  uiColumn    column  cycle

	@return
		- @b E_SYS      Status fail
		- @b E_OK       Operation success

*/
int nand_hostSetupAddressCycle(struct drv_nand_dev_info *info, NAND_ADDRESS_CYCLE_CNT uiRow, NAND_ADDRESS_CYCLE_CNT uiCol)
{
	union T_NAND_MODULE0_REG  RegMdule0 = {0};

	RegMdule0.Reg = NAND_GETREG(info, NAND_MODULE0_REG_OFS);

	if(uiCol > NAND_3_ADDRESS_CYCLE_CNT && uiCol != NAND_NOT_CONFIGED_ADDRESS_CYCLE_CNT)
		return E_SYS;

	if(uiCol != NAND_NOT_CONFIGED_ADDRESS_CYCLE_CNT)
		RegMdule0.Bit.COL_ADDR = uiCol;

	if(uiRow != NAND_NOT_CONFIGED_ADDRESS_CYCLE_CNT)
		RegMdule0.Bit.ROW_ADD = uiRow;

	NAND_SETREG(info, NAND_MODULE0_REG_OFS, RegMdule0.Reg);

	return E_OK;
}

/*
	Configure nand host page size

	This function configure controller of SPI NAND page size

	@param[in]  uiCS        chip select
	@param[in]  uiRow       row     cycle
	@param[in]  uiColumn    column  cycle

	@return
		- @b E_SYS      Status fail
		- @b E_OK       Operation success

*/
int nand_hostSetupPageSize(struct drv_nand_dev_info *info, NAND_PAGE_SIZE uiPageSize)
{
	union T_NAND_MODULE0_REG RegMdule0 = {0};

	RegMdule0.Reg = NAND_GETREG(info, NAND_MODULE0_REG_OFS);

	if(uiPageSize == NAND_PAGE_SIZE_512 || uiPageSize == NAND_PAGE_SIZE_2048 || uiPageSize == NAND_PAGE_SIZE_4096) {
		RegMdule0.Bit._PAGE_SIZE = uiPageSize;
		NAND_SETREG(info, NAND_MODULE0_REG_OFS, RegMdule0.Reg);
		return E_OK;
	} else
		return E_SYS;
}



/*
	NAND controller enable polling bit match operation

	Enable bit value compare function.
	After invoke spi_enBitMatch(), it will send uiCmd to SPI device
	and continously read data from SPI device.
	Once bit position specified by uiBitPosition of read data becomes bWaitValue,
	SPI module will stop checking and raise a interrupt.
	Caller of spi_enBitMatch() can use spi_waitBitMatch() to wait this checking complete.

	@param[in] uiStatusValue    The check value
	@param[in] uiStatusMask     Indicates which bit of status value you want to check. 1 for comparison and 0 for not comparison

	@return
		- @b E_OK: success
		- @b Else: fail
*/
int nand_hostSetupStatusCheckBit(struct drv_nand_dev_info *info, u8 uiStatusMask, u8 uiStatusValue)
{
	union T_NAND_STATUS_CHECK_REG RegStsChk = {0};

	RegStsChk.Bit.STATUS_VALUE = uiStatusValue;
	RegStsChk.Bit.STATUS_MASK = uiStatusMask;

	NAND_SETREG(info, NAND_STATUS_CHECK_REG_OFS, RegStsChk.Reg);
	return E_OK;
}

/*
	Configure nand host page size

	This function configure controller of SPI NAND page size

	@param[OUT]
	@return
		- @b_MULTI_PAGE
		- @b_SINGLE_PAGE

*/
ER nand_hostSetSPIIORegister(struct drv_nand_dev_info *info, NAND_SPI_CS_POLARITY uiCSPol, NAND_SPI_BUS_WIDTH uiBusWidth, NAND_SPI_IO_ORDER uiIOOrder)
{
	union T_NAND_SPI_CFG_REG  RegIO = {0};

	RegIO.Reg = NAND_GETREG(info, NAND_SPI_CFG_REG_OFS);

	RegIO.Bit.SPI_CS_POL    = uiCSPol;
	RegIO.Bit.SPI_BS_WIDTH  = uiBusWidth;
	RegIO.Bit.SPI_IO_ORDER  = uiIOOrder;
	RegIO.Bit.SPI_PULL_WPHLD  = 0x1;

	NAND_SETREG(info, NAND_SPI_CFG_REG_OFS, RegIO.Reg);
	return E_OK;
}

/*
	Configure nand host chip select manual mode

	Configure nand host chip select manual mode

	@param[in] bModeSel    manual mode or auto mode
		- NAND_SPI_CS_AUTO_MODE     : manual mode(CS configure by user)
		- NAND_SPI_CS_MANUAL_MODE   : auto mode(CS configure by controller)

*/
ER nand_hostConfigChipSelOperationMode(struct drv_nand_dev_info *info, NAND_SPI_CS_OP_MODE bModeSel)
{
	union T_NAND_SPI_CFG_REG  cfgReg = {0};

	cfgReg.Reg = NAND_GETREG(info, NAND_SPI_CFG_REG_OFS);
	cfgReg.Bit.SPI_OPERATION_MODE   = bModeSel;

	NAND_SETREG(info, NAND_SPI_CFG_REG_OFS, cfgReg.Reg);
	return E_OK;
}

/*
	Configure nand host chip select manual mode

	Configure nand host chip select manual mode

	@param[in] bCSLevel    CS (chip select) level
		- NAND_SPI_CS_LOW    : CS set low
		- NAND_SPI_CS_HIGH   : CS set high

*/
ER nand_hostSetCSActive(struct drv_nand_dev_info *info, NAND_SPI_CS_LEVEL bCSLevel)
{
	union T_NAND_SPI_CFG_REG  cfgReg = {0};

	cfgReg.Reg = NAND_GETREG(info, NAND_SPI_CFG_REG_OFS);
	cfgReg.Bit.SPI_NAND_CS   = bCSLevel;

	NAND_SETREG(info, NAND_SPI_CFG_REG_OFS, cfgReg.Reg);
	return E_OK;
}



/*
	Configure nand host timing 2 configuration

	Configure nand host timing 2 configuration

	@param[in] bCSLevel    CS (chip select) level
		- NAND_SPI_CS_LOW    : CS set low
		- NAND_SPI_CS_HIGH   : CS set high

*/
void nand_host_settiming2(struct drv_nand_dev_info *info, u32 time)
{
	union T_NAND_TIME2_REG  time_reg = {0};

	time_reg.Reg = time;

	if ((nvt_get_chip_id() == CHIP_NA51103) && time_reg.Bit.TSLCH < 3) {
		printf("331 SPI NAND TSLCH need >= 0x3, force config as 0x3\n");
		time_reg.Bit.TSLCH = 0x3;
	}

	if (time_reg.Bit.TSHCH < 0x4) {
		printf("SPI NAND TSHCH need >= 0x4, force config as 0x4\n");
		time_reg.Bit.TSHCH = 0x4;
	}

	NAND_SETREG(info, NAND_TIME2_REG_OFS, time_reg.Reg);
}

/*
    Configure nand host spare size

    @param[in]  bSpareSizeSel
    			- @b NAND_SPI_SPARE_16_BYTE : section = 512 + 16 byte
    			- @b NAND_SPI_SPARE_32_BYTE : section = 512 + 32 byte

*/
void nand_host_set_spare_size_sel(struct drv_nand_dev_info *info, NAND_SPI_SPARE_SIZE_SEL bSpareSizeSel)
{
	union T_NAND_MODULE0_REG  cfgReg = {0};

	cfgReg.Reg = NAND_GETREG(info, NAND_MODULE0_REG_OFS);

	//gNANDSpareSizeSel = bSpareSizeSel;

	cfgReg.Bit.SPARE_32_SEL = bSpareSizeSel;
	NAND_SETREG(info, NAND_MODULE0_REG_OFS, cfgReg.Reg);
}

/*
    Configure nand host Spare Align

    @param[in]  bSpareAlignSel
				- @b NAND_SPI_SPARE_ALIGN_SECTION: 3-byte user data and  13-byte ecc code per section
					|      section-0      |      section-1      | ...
					|  user  |  ecc code  |  user  |  ecc code  | ...
					| 3-byte |  13-byte   | 3-byte |  13-byte   | ...

				- @b NAND_SPI_SPARE_ALIGN_PAGE: 12-byte user data and  52-byte ecc code per page
					|             in 2K page spare              |
					|      user      |         ecc code         |
					|    12-byte     |          52-byte         |

					|             in 4K page spare              |
					|      user      |         ecc code         |
					|    24-byte     |         108-byte         |

*/
void nand_host_set_spare_align_sel(struct drv_nand_dev_info *info, NAND_SPI_SPARE_ALIGN_SEL bSpareAlignSel)
{
	union T_NAND_MODULE0_REG  cfgReg = {0};

	cfgReg.Reg = NAND_GETREG(info, NAND_MODULE0_REG_OFS);

	//gNANDSpareAlignSel = SpareAlignSel;

	cfgReg.Bit.SPARE_ALIGN_SEL = bSpareAlignSel;
	NAND_SETREG(info, NAND_MODULE0_REG_OFS, cfgReg.Reg);
}

/*
    Configure nand host BCH ecc protect spare user area or not

    @param[in]  bSpareEccFree
    			- @b NAND_SPI_SPARE_ECC_FREE_DISABLE : spare user area is protected
    			- @b NAND_SPI_SPARE_ECC_FREE_ENABLE  : spare user area is not protected

*/
void nand_host_set_spare_eccfree(struct drv_nand_dev_info *info, NAND_SPI_SPARE_ECC_FREE bSpareEccFree)
{
	union T_NAND_ECC_FREE_REG  cfgReg = {0};

	cfgReg.Reg = NAND_GETREG(info, NAND_ECC_FREE_REG_OFS);

	//gNANDSpareSizeSel = bSpareSizeSel;

	cfgReg.Bit.ECC_FREE = bSpareEccFree;
	NAND_SETREG(info, NAND_ECC_FREE_REG_OFS, cfgReg.Reg);
}

/*
    Enable / Disable AXI CH

    Enable and Disable AXI channel of NAND controller

    @param[in]  uiEN     0: enable, 1: disable

    @return void
*/
void nand_host_axi_ch_disable(struct drv_nand_dev_info *info, BOOL uiEN)
{
	union T_NAND_AXI_REG0 axiReg = {0};

	axiReg.Reg = NAND_GETREG(info, NAND_AXI_REG0_OFS);

	axiReg.Bit.AXI_CH_DISABLE = uiEN;

	NAND_SETREG(info, NAND_AXI_REG0_OFS, axiReg.Reg);

	if (uiEN == FALSE) {
		while ((NAND_GETREG(info, NAND_AXI_REG0_OFS) & _FIFO_EN) != 0);
	}
}
