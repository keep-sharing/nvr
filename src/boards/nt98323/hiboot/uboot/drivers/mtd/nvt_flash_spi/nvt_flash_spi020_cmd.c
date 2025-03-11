#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <malloc.h>
#include "nvt_flash_spi020_reg.h"
#include "nvt_flash_spi020_int.h"


u32 nvt_flash_ecc_err_pages[100] = {0};
u32 gSPINOR_DTR_TYPE = NANDCTRL_SDR_TYPE;

typedef enum {
	FLASH_ADDR_SIZE_3BYTES,
	FLASH_ADDR_SIZE_4BYTES,

	FLASH_ADDR_SIZE_COUNT,

	ENUM_DUMMY4WORD(FLASH_ADDR_SIZE_ENUM)

} FLASH_ADDR_SIZE_ENUM;

typedef enum {
	FLASH_WIDTH_1BIT,
	FLASH_WIDTH_2BITS,
	FLASH_WIDTH_4BITS,

	FLASH_WIDTH_COUNT,

	ENUM_DUMMY4WORD(FLASH_WIDTH_ENUM)

} FLASH_WIDTH_ENUM;

#if defined(CONFIG_NVT_FW_UPDATE_LED) && !defined(CONFIG_NVT_PWM)
#define OP_ERASE 0
#define OP_PROGRAM 1
int RESTORE_GPIO_DIR = 0;

/*      LED function for FW update*/
static void led_set_gpio_high(int operation)
{
	u32 gpio_reg;
	int ofs = NVT_LED_PIN/32;
	int shift = NVT_LED_PIN & 0x1F;

	ofs = ofs*0x4;

	/*Set gpio as high*/
	gpio_reg = INW(IOADDR_GPIO_REG_BASE + 0x20 + ofs);

	if (gpio_reg & (1 << shift))
		RESTORE_GPIO_DIR = 1;
	else {
		gpio_reg |= (1 << shift);
		OUTW(IOADDR_GPIO_REG_BASE + 0x20 + ofs, gpio_reg);
	}

	OUTW(IOADDR_GPIO_REG_BASE + 0x40 + ofs, (1 << shift));

	/*Config duration*/
	if (operation)
		mdelay(NVT_LED_PROGRAM_DURATION);
	else
		mdelay(NVT_LED_ERASE_DURATION);
}

static void led_set_gpio_low(void)
{
	u32 gpio_reg;
	int ofs = NVT_LED_PIN/32;
	int shift = NVT_LED_PIN & 0x1F;

	ofs = ofs*0x4;
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
        FTSPI_TRANS_T trans = {0};
        UINT8 buffer[1];

	ER ret = E_OK;

	if (info->flash_info->spi_nand_status.bBlockUnlocked == FALSE) {
		/*
		___|cmd(0x1F)|___|SR(0x01)(row byte0)|___|SR(0x0)(row byte1)|____
		*/
		trans.cs_sel = info->flash_info->chip_sel;
		trans.ctrl_mode = FCTRL_OP_NORMAL;
		trans.io_mode = FCTRL_IO_SERIAL;
		trans.command = SPINAND_CMD_SET_FEATURE;
		trans.flash_addr = SPINAND_FEATURE_BLOCK_LOCK;
		trans.flash_addr_count = 1;
		buffer[0] = 0x00;       // write 0 to flash register
		trans.data_buf = (UINT32)buffer;
		trans.data_dir = FIFO_DIR_WRITE;
		trans.data_len = 1;
		ret = nand_host_issue_transfer(info, &trans);

		if (ret == E_OK)
			info->flash_info->spi_nand_status.bBlockUnlocked = TRUE;
	}

	return ret;
}

static ER spiNand_wait_status(struct drv_nand_dev_info *info, UINT32 feature, UINT32 sts_reg, UINT8 *p_status)
{
	ER ret = E_OK;
	do {
		FTSPI_TRANS_T trans = {0};
		UINT8 buffer[1];

		trans.cs_sel = info->flash_info->chip_sel;
		trans.ctrl_mode = FCTRL_OP_POLLING_STATUS;
		trans.io_mode = FCTRL_IO_SERIAL;
		trans.command = feature;
		trans.flash_addr = sts_reg;
		trans.flash_addr_count = 1;
		buffer[0] = 0x00;       // write 0 to flash register
		trans.data_buf = (UINT32)buffer;
		trans.data_dir = FIFO_DIR_READ;
		trans.data_len = 0;     // must be 0 when read status
		trans.p_status = p_status;
		ret = nand_host_issue_transfer(info, &trans);
	} while (0);

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
			uint32_t uiColAddr, uint8_t* pBuf, uint32_t uiBufSize)
{
	FTSPI_TRANS_T trans = {0};
	UINT8 buffer[1];
	UINT8 ucStatus;
	UINT32 uiPageCnt;
	UINT32 *pulBuf = (UINT32 *)pBuf;
	ER ret = E_OK;

	do {
		uiPageCnt = uiBufSize / info->flash_info->page_size;

		if (uiPageCnt == 1 && ((uiColAddr+uiBufSize) > (info->flash_info->page_size + info->flash_info->oob_size))) {
			printf("col addr 0x%08x, buf size 0x%08x exceed 0x%08x + 0x%08x\r\n",
				(int32_t)uiColAddr, (int32_t)uiBufSize, info->flash_info->page_size, info->flash_info->oob_size);
			ret = E_NOMEM;
			break;
		} else {
			if(uiBufSize % info->flash_info->page_size != 0 && uiColAddr != info->flash_info->page_size) {
				printf("uiBufSize[0x%08x] not multiple of page size[0x%08x]\r\n",
				uiBufSize, info->flash_info->page_size);
				ret = E_SYS;
				break;
			}
		}

		// Issue page read command
		memset(&trans, 0, sizeof(trans));
		trans.cs_sel = info->flash_info->chip_sel;
		trans.ctrl_mode = FCTRL_OP_NORMAL;
		trans.io_mode = FCTRL_IO_SERIAL;
		trans.command = SPINAND_CMD_PAGE_READ;
		trans.flash_addr  = uiRowAddr;
		trans.flash_addr_count = 3;
		buffer[0] = 0x00;       // write 0 to flash register
		trans.data_len = 0;
		trans.data_dir = FIFO_DIR_WRITE;
		trans.data_buf = (UINT32)buffer;
		ret = nand_host_issue_transfer(info, &trans);
		if (ret != E_OK) {
			break;
		}

		ret = spiNand_wait_status(info, SPINAND_CMD_GET_FEATURE, SPINAND_FEATURE_STATUS, &ucStatus);

		if (ret != E_OK) {
			printf("%s: wait status fail\r\n", __func__);
			return ret;
		}

		// Issue read from cache command
		memset(&trans, 0, sizeof(trans));
		trans.cs_sel = info->flash_info->chip_sel;
		trans.ctrl_mode = FCTRL_OP_NORMAL;
#ifndef CONFIG_FLASH_ONLY_DUAL
		trans.io_mode = FCTRL_IO_QUAD;
		trans.command = SPINAND_CMD_CACHE_READX4;
#else
		trans.io_mode = FCTRL_IO_DUAL;
		trans.command = SPINAND_CMD_CACHE_READX2;
#endif
		trans.dummy_cyles = 8;
		if(info->flash_info->plane_flags) {
			if((uiRowAddr & 0x40) == 0x40) {
				trans.flash_addr  = (uiColAddr | (1<<12));
			} else {
				trans.flash_addr  = uiColAddr;
			}
		} else {
			trans.flash_addr  = uiColAddr;
		}
		trans.flash_addr_count = 2;
		trans.data_len = uiBufSize;
		trans.data_dir = FIFO_DIR_READ;
		trans.data_buf = (UINT32)pulBuf;
		ret = nand_host_issue_transfer(info, &trans);
		if (ret != E_OK) {
			break;
		}
	} while (0);

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
static ER spiNand_programPage(struct drv_nand_dev_info *info, UINT32 uiRowAddr, UINT32 uiColAddr, UINT8* pBuf, UINT32 uiBufSize, UINT8* pSpareBuf, UINT32 uiSpareSize)
{
	FTSPI_TRANS_T trans = {0};
	UINT8 buffer[1];
	UINT8 ucStatus;

	UINT32 *pulBuf = (UINT32 *)(pBuf + uiColAddr);
	ER ret = E_OK;

#if defined(CONFIG_NVT_FW_UPDATE_LED) && !defined(CONFIG_NVT_PWM)
	led_set_gpio_high(OP_PROGRAM);
#endif
	do {
		ret = spi_nand_unlock_BP(info);
		if (ret != E_OK)
			break;

		// write enable
		// ___|cmd|___ = 0x07 command cycle only
		memset(&trans, 0, sizeof(trans));
		trans.cs_sel = info->flash_info->chip_sel;
		trans.ctrl_mode = FCTRL_OP_NORMAL;
		trans.io_mode = FCTRL_IO_SERIAL;
		trans.command = SPINAND_CMD_WEN;
		buffer[0] = 0x00;       // write 0 to flash register
		trans.data_buf = (UINT32)buffer;
		trans.data_dir = FIFO_DIR_WRITE;
		ret = nand_host_issue_transfer(info, &trans);
		if (ret != E_OK) {
			break;
		}

		// Issue program load command
		memset(&trans, 0, sizeof(trans));
		trans.cs_sel = info->flash_info->chip_sel;
		trans.ctrl_mode = FCTRL_OP_NORMAL;
#ifndef CONFIG_FLASH_ONLY_DUAL
		if (info->flash_info->spi_nand_status.bQuadProgram) {
			trans.io_mode = FCTRL_IO_QUAD;
			trans.command = SPINAND_CMD_PROGRAM_LOADX4;
		} else {
			trans.io_mode = FCTRL_IO_SERIAL;
			trans.command = SPINAND_CMD_PROGRAM_LOAD;
		}
#else
		trans.io_mode = FCTRL_IO_SERIAL;
		trans.command = SPINAND_CMD_PROGRAM_LOAD;
#endif
		if(info->flash_info->plane_flags) {
			if((uiRowAddr & 0x40) == 0x40) {
				trans.flash_addr  = (uiColAddr | (1<<12));
			} else {
				trans.flash_addr  = uiColAddr;
			}
		} else {
			trans.flash_addr  = uiColAddr;
		}
		trans.flash_addr_count = 2;
		trans.data_len = uiBufSize;
		trans.data_dir = FIFO_DIR_WRITE;
		trans.data_buf = (UINT32)pulBuf;
		ret = nand_host_issue_transfer(info, &trans);
		if (ret != E_OK) {
			break;
		}

		// Issue program execute command
		memset(&trans, 0, sizeof(trans));
		trans.cs_sel = info->flash_info->chip_sel;
		trans.ctrl_mode = FCTRL_OP_NORMAL;
		trans.io_mode = FCTRL_IO_SERIAL;
		trans.command = SPINAND_CMD_PROGRAM_EXE;
		trans.flash_addr = uiRowAddr;
		trans.flash_addr_count = 3;
		ret = nand_host_issue_transfer(info, &trans);
		if (ret != E_OK) {
			break;
		}

		spiNand_wait_status(info, SPINAND_CMD_GET_FEATURE, SPINAND_FEATURE_STATUS, &ucStatus);

		if (ucStatus & SPINAND_FEATURE_STATUS_ERASE_FAIL) {
			debug("erase/program block fail 0x%lx\r\n", uiRowAddr);
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
	FTSPI_TRANS_T trans = {0};

	trans.cs_sel = info->flash_info->chip_sel;
	trans.ctrl_mode = FCTRL_OP_NORMAL;
	trans.io_mode = FCTRL_IO_SERIAL;
	trans.command = SPINAND_CMD_RESET;
	trans.data_len = 0;
	trans.data_dir = FIFO_DIR_WRITE;

	return nand_host_issue_transfer(info, &trans);
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
	uint32_t spi_cmd = 0;

	if (set == NAND_SPI_STS_FEATURE_1)
		spi_cmd = SPINAND_FEATURE_BLOCK_LOCK;
	else if (set == NAND_SPI_STS_FEATURE_2)
		spi_cmd = SPINAND_FEATURE_OPT;
	else if (set == NAND_SPI_STS_FEATURE_4)
		spi_cmd = SPINAND_FEATURE_STATUS_ECC_UNCORR_ERR;
	else
		spi_cmd = SPINAND_FEATURE_STATUS;

	do {
		FTSPI_TRANS_T trans = {0};
		UINT8 buffer[1];
		trans.cs_sel = info->flash_info->chip_sel;
		trans.ctrl_mode = FCTRL_OP_READ_STATUS;
		trans.io_mode = FCTRL_IO_SERIAL;
		trans.command = SPINAND_CMD_GET_FEATURE;
		trans.flash_addr = spi_cmd;
		trans.flash_addr_count = 1;
		buffer[0] = 0x00;       // write 0 to flash register
		trans.data_buf = (UINT32)buffer;
		trans.data_dir = FIFO_DIR_READ;
		trans.data_len = 0;     // must be 0 when read status
		trans.p_status = (void*)&status;
		ret = nand_host_issue_transfer(info, &trans);
	} while (0);

	if(ret == E_OK)
		memcpy(info->data_buff + info->buf_start, &status, 1);

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
	uint32_t spi_cmd = 0;

	if (set == NAND_SPI_STS_FEATURE_1)
		spi_cmd = SPINAND_FEATURE_BLOCK_LOCK;
	else if (set == NAND_SPI_STS_FEATURE_2)
		spi_cmd = SPINAND_FEATURE_OPT;
	else
		spi_cmd = SPINAND_FEATURE_STATUS;

	do {
		FTSPI_TRANS_T trans = {0};
		UINT8 buffer[1];

		trans.cs_sel = info->flash_info->chip_sel;
		trans.ctrl_mode = FCTRL_OP_NORMAL;
		trans.io_mode = FCTRL_IO_SERIAL;
		trans.command = SPINAND_CMD_SET_FEATURE;
		// combine 1 byte address + 1 byte status to pesudo 2 byte address
		trans.flash_addr = (spi_cmd<<8) | status;
		trans.flash_addr_count = 2;
		buffer[0] = 0x00;       // write 0 to flash register
		trans.data_buf = (UINT32)buffer;
		trans.data_dir = FIFO_DIR_WRITE;
		trans.data_len = 0;     // must be 0 when read status
		ret = nand_host_issue_transfer(info, &trans);
	} while (0);

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
	FTSPI_TRANS_T trans = {0};
	UINT8 buffer[1];
	UINT8 ucStatus;
	ER ret = E_OK;

#if defined(CONFIG_NVT_FW_UPDATE_LED) && !defined(CONFIG_NVT_PWM)
	led_set_gpio_high(OP_ERASE);
#endif

	do {
		ret = spi_nand_unlock_BP(info);
		if (ret != E_OK)
			break;

		// write enable
		// ___|cmd|___ = 0x07 command cycle only
		memset(&trans, 0, sizeof(trans));
		trans.cs_sel = info->flash_info->chip_sel;
		trans.ctrl_mode = FCTRL_OP_NORMAL;
		trans.io_mode = FCTRL_IO_SERIAL;
		trans.command = SPINAND_CMD_WEN;
		buffer[0] = 0x00;       // write 0 to flash register
		trans.data_buf = (UINT32)buffer;
		trans.data_dir = FIFO_DIR_WRITE;
		ret = nand_host_issue_transfer(info, &trans);
		if (ret != E_OK) {
			break;
		}

		// Issue erase command
		memset(&trans, 0, sizeof(trans));
		trans.cs_sel = info->flash_info->chip_sel;
		trans.ctrl_mode = FCTRL_OP_NORMAL;
		trans.io_mode = FCTRL_IO_SERIAL;
		trans.command = SPINAND_CMD_BLK_ERASE;
		trans.flash_addr = block_address;
		trans.flash_addr_count = 3;
		trans.data_dir = FIFO_DIR_WRITE;
		ret = nand_host_issue_transfer(info, &trans);
		if (ret != E_OK) {
			break;
		}

		spiNand_wait_status(info, SPINAND_CMD_GET_FEATURE, SPINAND_FEATURE_STATUS, &ucStatus);

		if (ucStatus & SPINAND_FEATURE_STATUS_ERASE_FAIL) {
			debug("Erase block fail 0x%08x\r\n", block_address);
			ret = E_SYS;
		}

		if(ucStatus & SPINAND_FEATURE_STATUS_ERASE_FAIL) {
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
int nand_cmd_read_id(uint8_t * card_id, struct drv_nand_dev_info *info)
{
	FTSPI_TRANS_T trans = {0};

	trans.cs_sel = info->flash_info->chip_sel;
	trans.ctrl_mode = FCTRL_OP_NORMAL;
	trans.io_mode = FCTRL_IO_SERIAL;
	trans.command = SPINAND_CMD_JEDECID;
	if (info->flash_type == NANDCTRL_SPI_NAND_TYPE) {
		trans.flash_addr = 0;
		trans.flash_addr_count = 1;
	}
	trans.data_buf = (UINT32)card_id;
	trans.data_dir = FIFO_DIR_READ;
	trans.data_len = 4;

	return nand_host_issue_transfer(info, &trans);
}

/*
    Read ECC corrected bits

    @return correct bits
*/

int nand_cmd_read_flash_ecc_corrected(struct drv_nand_dev_info *info)
{
#if 0
    	struct smc_setup_trans transParam;
	uint8_t  ecc_bits[4];

	transParam.colAddr = 0;
	transParam.rowAddr = 0;
	transParam.fifoDir = _FIFO_READ;
	transParam.transMode = _PIO_MODE;
	transParam.type = _READ_ID;
	transParam.uiCS	= 0;

	nand_host_setup_transfer(info, &transParam, 0, 4, _SINGLE_PAGE);

	nand_host_send_command(info, SPINAND_CMD_MXIC_READ_ECC, FALSE);

	nand_host_receive_data(info, &ecc_bits[0], 4, _PIO_MODE);

	return ecc_bits[0] & 0xF;
#else
	return 0;
#endif
}

/*
	Wait SPI flash ready

	Wait SPI flash returned to ready state

	@param[in] uiWaitMs     Timeout setting. (Unit: ms)

	@return
		- @b E_OK: success
		- @b Else: fail. Maybe timeout.
*/
static ER spiflash_waitReady(struct drv_nand_dev_info *info, u8 * ucStatus)
{
	ER              ret = E_OK;

	do {
		FTSPI_TRANS_T trans = {0};
		UINT8 buffer[1];

		trans.cs_sel = info->flash_info->chip_sel;
		trans.ctrl_mode = FCTRL_OP_POLLING_STATUS;
		trans.io_mode = FCTRL_IO_SERIAL;
		trans.command = FLASH_CMD_RDSR;
		buffer[0] = 0x00;       // write 0 to flash register
		trans.data_buf = (UINT32)buffer;
		trans.data_dir = FIFO_DIR_READ;
		trans.data_len = 0;     // must be 0 when read status
		trans.p_status = ucStatus;
		ret = nand_host_issue_transfer(info, &trans);
	} while (0);

	return ret;
}

static ER spi_nor_send_cmd(struct drv_nand_dev_info *info, UINT8 cmd)
{
	FTSPI_TRANS_T trans = {0};

	trans.cs_sel = info->flash_info->chip_sel;
	trans.ctrl_mode = FCTRL_OP_NORMAL;
	trans.io_mode = FCTRL_IO_SERIAL;
	trans.command = cmd;
	trans.data_len = 0;
	trans.data_dir = FIFO_DIR_WRITE;

	return nand_host_issue_transfer(info, &trans);
}

static ER spiNor_getStatus(struct drv_nand_dev_info *info, UINT32 uiSet, UINT8 *p_status)
{
	UINT32          spi_cmd = 0;
	ER              ret = E_OK;

	if (uiSet == NAND_SPI_NOR_STS_RDSR_1) {
		spi_cmd = FLASH_CMD_RDSR;
	} else if (uiSet == NAND_SPI_NOR_STS_RDSR_2) {
		spi_cmd = FLASH_CMD_RDSR2;
	} else if (uiSet == NAND_SPI_NOR_STS_RDSR_3) {
		spi_cmd = FLASH_CMD_RDSR3;
	} else {
		printf("%s: unknow cmd set %d\r\n", __func__, (int)(uiSet));
		return E_SYS;
	}

	do {
		FTSPI_TRANS_T trans = {0};
		UINT8 buffer[1];

		trans.cs_sel = info->flash_info->chip_sel;
		trans.ctrl_mode = FCTRL_OP_READ_STATUS;
		trans.io_mode = FCTRL_IO_SERIAL;
		trans.command = spi_cmd;
		buffer[0] = 0x00;       // write 0 to flash register
		trans.data_buf = (UINT32)buffer;
		trans.data_dir = FIFO_DIR_READ;
		trans.data_len = 0;     // must be 0 when read status
		trans.p_status = p_status;
		ret = nand_host_issue_transfer(info, &trans);
	} while (0);

	return ret;
}

static ER spiNor_setStatus(struct drv_nand_dev_info *info, UINT32 uiSet, UINT8 ucStatus)
{
	UINT32              spi_cmd = 0;
	ER                  ret = E_OK;

	if (uiSet == NAND_SPI_NOR_STS_WRSR_1) {
		spi_cmd = FLASH_CMD_WRSR;
	} else {
		spi_cmd = FLASH_CMD_WRSR2;
	}

	do {
		FTSPI_TRANS_T trans = {0};
		UINT8 buffer[1];

		trans.cs_sel = info->flash_info->chip_sel;
		trans.ctrl_mode = FCTRL_OP_NORMAL;
		trans.io_mode = FCTRL_IO_SERIAL;
		trans.command = spi_cmd;
		trans.flash_addr = ucStatus;
		trans.flash_addr_count = 1;
		buffer[0] = 0x00;       // write 0 to flash register
		trans.data_buf = (UINT32)buffer;
		trans.data_dir = FIFO_DIR_WRITE;
		trans.data_len = 0;     // must be 0 when read status
		ret = nand_host_issue_transfer(info, &trans);
	} while (0);

	return ret;
}


static ER spiflash_disableWriteLatch(struct drv_nand_dev_info *info)
{
	return spi_nor_send_cmd(info, FLASH_CMD_WRDI);
}

static ER spiflash_enableWriteLatch(struct drv_nand_dev_info *info)
{
	UINT8 uiWord = 0;
	ER  ret;

	do {
		spiNor_getStatus(info, NAND_SPI_NOR_STS_RDSR_1, &uiWord);

		uiWord &= ~(FLASH_STATUS_WP_BITS | FLASH_STATUS_WEL_BITS);

		ret = spi_nor_send_cmd(info, FLASH_CMD_EWSR);
		if (ret != E_OK) {
			printf("%s: EWSR fail\r\n", __func__);
			return ret;
		}

		ret = spiNor_setStatus(info, NAND_SPI_NOR_STS_WRSR_1, uiWord);

		if (ret != E_OK) {
			break;
		}

		spiNor_getStatus(info, NAND_SPI_NOR_STS_RDSR_1, &uiWord);

		ret = spi_nor_send_cmd(info, FLASH_CMD_WREN);
		if (ret != E_OK) {
			printf("%s: WREN fail\r\n", __func__);
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
int nand_cmd_read_operation(struct drv_nand_dev_info *info, int8_t * buffer, uint32_t pageAddress, uint32_t numPage)
{
	const struct nvt_nand_flash *f = info->flash_info;

	return spiNand_readPage(info, pageAddress / f->page_size, 0, (uint8_t *)buffer, numPage*f->page_size);
}


/*
	Write pages.(single page operation)

	@param buffer      Buffer address
	@param pageAddress The address of the page. Only (n * g_pNandInfo->uiBytesPerPageData) will be valid.
		Beware that the max accessible size is 4GB.  One should use write_readOperation2() instead.
	@param numPage     How many pages
	@return E_OK or E_SYS
*/
int nand_cmd_write_operation_single(struct drv_nand_dev_info *info, int8_t * buffer, uint32_t pageAddress, uint32_t column)
{
	const struct nvt_nand_flash *f = info->flash_info;
	uint32_t buf_len = info->buf_count - column;

	return spiNand_programPage(info, pageAddress / f->page_size, column, (UINT8 *)buffer, buf_len, 0, 0);
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
#if 0
	union T_NAND_RSERR_STS0_REG reg_correct = {0};
	u8 correct_count = 0;

	reg_correct.Reg = NAND_GETREG(info, NAND_RSERR_STS0_REG_OFS);

	/*Read the largest number of correct bits*/
	correct_count = reg_correct.Bit.SEC0_ERR_STS;
	if (reg_correct.Bit.SEC1_ERR_STS > correct_count)
		correct_count = reg_correct.Bit.SEC1_ERR_STS;

	if (reg_correct.Bit.SEC2_ERR_STS > correct_count)
		correct_count = reg_correct.Bit.SEC2_ERR_STS;

	if (reg_correct.Bit.SEC3_ERR_STS > correct_count)
		correct_count = reg_correct.Bit.SEC3_ERR_STS;

	return correct_count;
#else
	return 0;
#endif
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

int spinor_erase_sector(struct drv_nand_dev_info *info, struct spi_flash *flash, u32 byte_addr)
{
	FTSPI_TRANS_T trans = {0};
	UINT8 ucSts;
	ER  ret;

	ret = spiflash_waitReady(info, &ucSts);
	if (ret != E_OK) {
		printf("%s: wait ready before erase fail\r\n", __func__);
		return ret;
	}

	ret = spiflash_enableWriteLatch(info);
	if (ret != E_OK) {
		printf("%s: enable write latch fail\r\n", __func__);
		return ret;
	}

	trans.cs_sel = info->flash_info->chip_sel;
	trans.ctrl_mode = FCTRL_OP_NORMAL;
	trans.io_mode = FCTRL_IO_SERIAL;
	if (info->flash_info->device_size > SPI_FLASH_16MB_BOUN) {
		trans.flash_addr_count = 4;
	} else {
		trans.flash_addr_count = 3;
	}

	trans.command = flash->erase_cmd;
	trans.flash_addr = byte_addr;
	trans.data_dir = FIFO_DIR_WRITE;

	ret = nand_host_issue_transfer(info, &trans);

	spiflash_waitReady(info, &ucSts);

	spiflash_disableWriteLatch(info);

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
int spinor_pio_operation(struct drv_nand_dev_info *info, u32 byte_addr, u32 byte_size, u8 *pbuf)
{
	FTSPI_TRANS_T trans = {0};
	FLASH_ADDR_SIZE_ENUM idx_addr_bytes = 0;
	FLASH_WIDTH_ENUM idx_bus_width = 0;
	DTR_TYPE_SEL idx_xdr = 0;
	ER ret = E_OK;
	//    const UINT32 FLASH_ADDR_3BYTES = 0;
	//    const UINT32 FLASH_ADDR_4BYTES = 1;
	//    const UINT32 FLASH_WIDTH_1BIT = 0;
	//    const UINT32 FLASH_WIDTH_2BITS = 1;
	//    const UINT32 FLASH_WIDTH_4BITS = 2;
	const UINT32 v_flash_read_cmd[DTR_TYPE_NUM][FLASH_WIDTH_COUNT][FLASH_ADDR_SIZE_COUNT] = {
		// SDR
		{
			// 1 bit
			{   FLASH_CMD_READ,             FLASH_CMD_READ_4BYTE},
			// 2 bit (dual)
			{   FLASH_CMD_DUAL_READ,        FLASH_CMD_DUAL_READ_4BYTE},
			// 4 bit (quad)
			{   FLASH_CMD_QUAD_READ_NORMAL, FLASH_CMD_QUAD_READ_NORMAL_4BYTE},
		},
		// DTR
		{
			// 1 bit
			{   FLASH_CMD_DTR_FASTREAD,     FLASH_CMD_DTR_FASTREAD},
			// 2 bit (dual)
			{   FLASH_CMD_DTR_DUAL_READ,    FLASH_CMD_DTR_DUAL_READ},
			// 4 bit (quad)
			{   FLASH_CMD_DTR_QUAD_READ,    FLASH_CMD_DTR_QUAD_READ},
		},
	};
	const FCTRL_IO_MODE v_io_mode[DTR_TYPE_NUM][FLASH_WIDTH_COUNT] = {
		// SDR
		{
			FCTRL_IO_SERIAL,
			FCTRL_IO_DUAL,
			FCTRL_IO_QUAD
		},
		// DTR
		{
			FCTRL_IO_SERIAL,
			FCTRL_IO_DUAL_IO,
			FCTRL_IO_QUAD_IO
		}
	};
	const UINT32 v_dummy_cycles[DTR_TYPE_NUM][FLASH_WIDTH_COUNT] = {
		// SDR
		{
			0,
			8,
			8
		},
		// DTR
		{
			6,
			4,
			7
		},
	};
	const INT32 v_conti_mode[DTR_TYPE_NUM][FLASH_WIDTH_COUNT] = {
		// SDR
		{
			-1,     // -1 stands for no continous mode
			-1,
			-1
		},
		// DTR
		{
			-1,
			0xFF,
			0xFF
		},
	};

	if (gSPINOR_DTR_TYPE == NANDCTRL_SDR_TYPE) {
		idx_xdr = NANDCTRL_SDR_TYPE;
	} else {
		idx_xdr = NANDCTRL_DTR_TYPE;
	}


	//
	// Generate host/flash bus width capabilities
	//
	if (info->flash_info->nor_read_mode == SPI_NOR_QUAD_READ) {
		idx_bus_width = FLASH_WIDTH_4BITS;
	} else if (info->flash_info->nor_read_mode == SPI_NOR_DUAL_READ) {
		idx_bus_width = FLASH_WIDTH_2BITS;
	} else {
		idx_bus_width = FLASH_WIDTH_1BIT;
	}

	if (info->flash_info->device_size > SPI_FLASH_16MB_BOUN) {
		idx_addr_bytes = FLASH_ADDR_SIZE_4BYTES;
	} else {
		idx_addr_bytes = FLASH_ADDR_SIZE_3BYTES;
	}

	// Issue read from cache command
	memset(&trans, 0, sizeof(trans));
	trans.cs_sel = info->flash_info->chip_sel;
	trans.ctrl_mode = FCTRL_OP_NORMAL;
	trans.io_mode = v_io_mode[idx_xdr][idx_bus_width];
	trans.command = v_flash_read_cmd[idx_xdr][idx_bus_width][idx_addr_bytes];
	if (gSPINOR_DTR_TYPE == NANDCTRL_DTR_TYPE) {
		trans.flash_addr_count = 3;	// still not implement, force 3 byte addr
	} else {
		trans.flash_addr_count = (idx_addr_bytes==FLASH_ADDR_SIZE_4BYTES)?4:3;
	}
	trans.dummy_cyles = v_dummy_cycles[idx_xdr][idx_bus_width];
	trans.flash_addr  = byte_addr;
	trans.data_len = byte_size;
	trans.data_dir = FIFO_DIR_READ;
	trans.data_buf = (UINT32)pbuf;
	if (v_conti_mode[idx_xdr][idx_bus_width] != -1) {
		trans.is_conti_mode = TRUE;
		trans.conti_mode = (UINT32)v_conti_mode[idx_xdr][idx_bus_width];
	}

	if (gSPINOR_DTR_TYPE == NANDCTRL_DTR_TYPE) {
		trans.xdr_sel = FCTRL_DDR;
	}
	ret = nand_host_issue_transfer(info, &trans);

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
int spiflash_page_program(struct drv_nand_dev_info *info, u32 byte_addr, u32 sector_size, u8* pbuf)
{
	FTSPI_TRANS_T trans = {0};
	UINT8   ucSts;
	FLASH_ADDR_SIZE_ENUM idx_addr_bytes = 0;
	FLASH_WIDTH_ENUM idx_bus_width = 0;
	ER ret = E_OK;
	//    const UINT32 FLASH_ADDR_3BYTES = 0;
	//    const UINT32 FLASH_ADDR_4BYTES = 1;
	//    const UINT32 FLASH_WIDTH_1BIT = 0;
	//    const UINT32 FLASH_WIDTH_2BITS = 1;
	//    const UINT32 FLASH_WIDTH_4BITS = 2;
	const UINT32 v_flash_write_cmd[FLASH_WIDTH_COUNT][FLASH_ADDR_SIZE_COUNT] = {
		// 1 bit
		{   FLASH_CMD_PAGEPROG,         FLASH_CMD_PAGEPROG_4BYTE},
		// 2 bit (dual) (most flash not support 2 bit program, use 1 bit here)
		{   FLASH_CMD_PAGEPROG,         FLASH_CMD_PAGEPROG_4BYTE},
		// 4 bit (quad)
		{   FLASH_CMD_PAGEPROG_4X,      FLASH_CMD_PAGEPROG_4X_4BYTE},
	};
	const FCTRL_IO_MODE v_io_mode[FLASH_WIDTH_4BITS+1] = {
		FCTRL_IO_SERIAL,
		FCTRL_IO_DUAL,
		FCTRL_IO_QUAD
	};

	//
	// Check bus width/flash address size
	//
	if (info->flash_info->nor_quad_support) {
		idx_bus_width = FLASH_WIDTH_4BITS;
	} else {
		idx_bus_width = FLASH_WIDTH_1BIT;
	}
	if (info->flash_info->device_size > SPI_FLASH_16MB_BOUN) {
		idx_addr_bytes = FLASH_ADDR_SIZE_4BYTES;
	} else {
		idx_addr_bytes = FLASH_ADDR_SIZE_3BYTES;
	}

	spiflash_enableWriteLatch(info);

	// Issue program command
	memset(&trans, 0, sizeof(trans));
	trans.cs_sel = info->flash_info->chip_sel;
	trans.ctrl_mode = FCTRL_OP_NORMAL;
	trans.io_mode = v_io_mode[idx_bus_width];
	trans.command = v_flash_write_cmd[idx_bus_width][idx_addr_bytes];
	trans.flash_addr_count = (idx_addr_bytes==FLASH_ADDR_SIZE_4BYTES)?4:3;
	trans.flash_addr  = byte_addr;
	//    trans.flash_addr_count = 2;
	trans.data_len = sector_size;
	trans.data_dir = FIFO_DIR_WRITE;
	trans.data_buf = (UINT32)pbuf;
	ret = nand_host_issue_transfer(info, &trans);
	if (ret != E_OK) {
		return ret;
	}

	ret = spiflash_waitReady(info, &ucSts);

	spiflash_disableWriteLatch(info);

	return ret;
}

static int spiflash_read_data(struct drv_nand_dev_info *info, u32 addr, u32 byte_size, u8 *pbuf)
{
	return spinor_pio_operation(info, addr , byte_size, pbuf);
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
	u32 remain_byte, read_cycle, addr_index, cycle_index;
	u32 align_length, remain_length;
	int ret = E_SYS;

	/*Handle if dram starting address is not cacheline alignment*/
	if ((u32)pbuf & (CONFIG_SYS_CACHELINE_SIZE-1)) {
		align_length = (u32)pbuf & (CONFIG_SYS_CACHELINE_SIZE-1);
		align_length = CONFIG_SYS_CACHELINE_SIZE - align_length;

		ret = spinor_pio_operation(info, byte_addr , align_length, pbuf);
		if (ret != E_OK) {
			printf("%s: fail at flash address 0x%x length 0x%x\r\n", __func__, byte_addr, align_length);
			return ret;
		}

		if (byte_size <= align_length)
			return E_OK;
		else
			byte_size -= align_length;

		byte_addr += align_length;

		pbuf += align_length;
	}

	align_length = (byte_size / CONFIG_SYS_CACHELINE_SIZE) * \
			CONFIG_SYS_CACHELINE_SIZE;

	remain_length = ((byte_size - align_length + 3) / 4) * 4;

	read_cycle = align_length / SPIFLASH_MAX_READ_BYTE_AT_ONCE;

	addr_index = byte_addr;

	remain_byte = align_length % SPIFLASH_MAX_READ_BYTE_AT_ONCE;

	if (align_length) {
		for(cycle_index = 0; cycle_index < read_cycle; cycle_index++, addr_index += SPIFLASH_MAX_READ_BYTE_AT_ONCE) {
			ret = spiflash_read_data(info, addr_index, SPIFLASH_MAX_READ_BYTE_AT_ONCE, (pbuf + (cycle_index * SPIFLASH_MAX_READ_BYTE_AT_ONCE)));
			if (ret != E_OK) {
				printf("%s: fail at flash address 0x%x\r\n", __func__, addr_index);
				break;
			} else
				debug("R");
		}

		if(remain_byte) {
			ret = spiflash_read_data(info, addr_index, remain_byte, (pbuf + (cycle_index * SPIFLASH_MAX_READ_BYTE_AT_ONCE)));
			if (ret != E_OK)
				printf("%s: fail at flash address 0x%x\r\n", __func__, addr_index);
			else
				debug("r");
		}
#if 0
		invalidate_dcache_range((u32)pbuf, (u32)(pbuf + align_length));
#endif
	}

	/*Handle if length is not cacheline alignment*/
	if (remain_length) {
		ret = spinor_pio_operation(info, byte_addr + align_length , remain_length, pbuf + align_length);
		if (ret != E_OK)
			printf("%s: fail at flash address 0x%x length 0x%x\r\n", __func__, byte_addr + align_length, remain_length);
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
int spinor_program_operation(struct drv_nand_dev_info *info, u32 byte_addr, u32 sector_size, u8* pbuf)
{
	u32 program_cycle;
	u32 addr_index;
	u32 cycle_index;
	int ret = E_SYS;
	u32 p_flush_buf = (u32)pbuf;
	u32 cacheline_mask = CONFIG_SYS_CACHELINE_SIZE-1;
	size_t align_size = roundup(sector_size, ARCH_DMA_MINALIGN);

	program_cycle = sector_size / SPIFLASH_MAX_PROGRAM_BYTE_AT_ONCE;

	addr_index = byte_addr;

	if (p_flush_buf & (CONFIG_SYS_CACHELINE_SIZE-1)) {
		p_flush_buf &= ~cacheline_mask;
		flush_dcache_range(p_flush_buf, p_flush_buf + align_size + CONFIG_SYS_CACHELINE_SIZE);
	} else
		flush_dcache_range(p_flush_buf, p_flush_buf + align_size);

	for(cycle_index = 0; cycle_index < program_cycle; cycle_index++, addr_index += SPIFLASH_MAX_PROGRAM_BYTE_AT_ONCE) {
		ret = spiflash_page_program(info, addr_index, SPIFLASH_MAX_PROGRAM_BYTE_AT_ONCE, (pbuf + (cycle_index * SPIFLASH_MAX_PROGRAM_BYTE_AT_ONCE)));

		if (ret != E_OK) {
			printf("%s: fail at flash address 0x%x\r\n", __func__, addr_index);
			break;
		} else
			debug("W");

		sector_size -= SPIFLASH_MAX_PROGRAM_BYTE_AT_ONCE;
	}

	if (sector_size) {

		/*Minimal transmitting size should be word align*/
		if (sector_size % WORD_ALIGN_OFFSET)
			sector_size += WORD_ALIGN_OFFSET - (sector_size % WORD_ALIGN_OFFSET);

		ret = spiflash_page_program(info, addr_index, sector_size, (pbuf + (cycle_index * SPIFLASH_MAX_PROGRAM_BYTE_AT_ONCE)));
	}

	return ret;
}


int spinor_read_status(struct drv_nand_dev_info *info, u8 status_set, u8* status)
{
	UINT32          spi_cmd = 0;
	ER              ret = E_OK;

	if (status_set == NAND_SPI_NOR_STS_RDSR_1) {
		spi_cmd = FLASH_CMD_RDSR;
	} else if (status_set == NAND_SPI_NOR_STS_RDSR_2) {
		spi_cmd = FLASH_CMD_RDSR2;
	} else if (status_set == NAND_SPI_NOR_STS_RDSR_3) {
		spi_cmd = FLASH_CMD_RDSR3;
	} else {
		printf("%s: unknow cmd set %d\r\n", __func__, (int)(status_set));
		return E_SYS;
	}

	do {
		FTSPI_TRANS_T trans = {0};
		UINT8 buffer[1];

		trans.cs_sel = info->flash_info->chip_sel;
		trans.ctrl_mode = FCTRL_OP_READ_STATUS;
		trans.io_mode = FCTRL_IO_SERIAL;
		trans.command = spi_cmd;
		buffer[0] = 0x00;       // write 0 to flash register
		trans.data_buf = (UINT32)buffer;
		trans.data_dir = FIFO_DIR_READ;
		trans.data_len = 0;     // must be 0 when read status
		trans.p_status = status;
		ret = nand_host_issue_transfer(info, &trans);
	} while (0);

	return ret;
}

int spinor_write_status(struct drv_nand_dev_info *info, u8 status_set, u8 status)
{
	UINT32              spi_cmd = 0;
	ER                  ret = E_OK;

	if (status_set == NAND_SPI_NOR_STS_WRSR_1) {
		spi_cmd = FLASH_CMD_WRSR;
	} else {
		spi_cmd = FLASH_CMD_WRSR2;
	}

	spiflash_enableWriteLatch(info);

	do {
		FTSPI_TRANS_T trans = {0};
		UINT8 buffer[1];

		trans.cs_sel = info->flash_info->chip_sel;
		trans.ctrl_mode = FCTRL_OP_NORMAL;
		trans.io_mode = FCTRL_IO_SERIAL;
		trans.command = spi_cmd;
		trans.flash_addr = status;
		trans.flash_addr_count = 1;
		buffer[0] = 0x00;       // write 0 to flash register
		trans.data_buf = (UINT32)buffer;
		trans.data_dir = FIFO_DIR_WRITE;
		trans.data_len = 0;     // must be 0 when read status
		ret = nand_host_issue_transfer(info, &trans);
	} while (0);

	return ret;
}

int spinor_write_status_array(struct drv_nand_dev_info *info, u8* v_status, int len)
{
	ER                  ret = E_OK;

	if (len > 2) {
		printf("%s: len %d forced to 2\r\n", __func__, len);
		len = 2;
	}

	spiflash_enableWriteLatch(info);

	do {
		FTSPI_TRANS_T trans = {0};

		trans.cs_sel = info->flash_info->chip_sel;
		trans.ctrl_mode = FCTRL_OP_NORMAL;
		trans.io_mode = FCTRL_IO_SERIAL;
		trans.command = FLASH_CMD_WRSR;
		trans.flash_addr_count = 0;
		trans.data_buf = (UINT32)v_status;
		trans.data_dir = FIFO_DIR_WRITE;
		trans.data_len = len;
		ret = nand_host_issue_transfer(info, &trans);
	} while (0);

	return ret;
}
