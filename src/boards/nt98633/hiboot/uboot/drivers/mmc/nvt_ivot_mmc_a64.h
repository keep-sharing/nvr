/*
 *  driver/mmc/nvt_ivot_mmc_a64.h
 *
 *  Author:	Howard Chang
 *  Created:	Nov 15, 2021
 *  Copyright:	Novatek Inc.
 *
 */

#ifndef __NVT_IVOT_MMC_H__
#define __NVT_IVOT_MMC_H__

#include "nvt_ivot_mmcreg.h"
#include "nvt_ivot_mmcplat.h"
#include <linux/bitops.h>

/************************
 *        Common        *
 ************************/
#define DDR_MASK              (0xF00000000)
#define ADDR_MAU2             (0x1)
#define ADDR_PCIE_MAU1        (0x4)
#define ADDR_PCIE_MAU2        (0x5)

#define SDIO_HOST_ID_1        0
#define SDIO_HOST_ID_2        1
#define SDIO_HOST_ID_3        2

int nvt_mmc_init(int id);

#define SDIO_HOST_WRITE_DATA                (FALSE)
#define SDIO_HOST_READ_DATA                 (TRUE)

enum SDIO_MODE_DRIVING {
	SDIO_DS_MODE_CLK = 0,
	SDIO_DS_MODE_CMD,
	SDIO_DS_MODE_DATA,
	SDIO_HS_MODE_CLK,
	SDIO_HS_MODE_CMD,
	SDIO_HS_MODE_DATA,
	SDIO_SDR50_MODE_CLK,
	SDIO_SDR50_MODE_CMD,
	SDIO_SDR50_MODE_DATA,
	SDIO_SDR104_MODE_CLK,
	SDIO_SDR104_MODE_CMD,
	SDIO_SDR104_MODE_DATA,
	SDIO_MAX_MODE_DRIVING,
};

struct mmc_nvt_host {
	struct mmc_cmd *cmd;
	struct mmc_data *data;
	u32 mmc_input_clk;
	u32 mmc_default_clk;
	void __iomem *base;
	struct resource *mem_res;
	int id;
	unsigned char bus_mode;
	unsigned char data_dir;
	unsigned char suspended;

	/* buffer is used during PIO of one scatterlist segment, and
	 * is updated along with buffer_bytes_left.  bytes_left applies
	 * to all N blocks of the PIO transfer.
	 */
	u8 *buffer;
	u32 buffer_bytes_left;
	u32 bytes_left;
	bool do_dma;
	/*early data*/
	bool data_early;

	u32 pad_driving[SDIO_MAX_MODE_DRIVING];
	u32 pinmux_value;
	int enable_8bits;
	u32 ext_caps;

	/* Version of the MMC/SD controller */
	u8 version;
	/* for ns in one cycle calculation */
	unsigned ns_in_one_cycle;
	int indly_sel;
	int neg_sample_edge;
};

/*
    SDIO send command execution result.

    Encoding of sdiohost_sendcmd() result.
*/
#define SDIO_HOST_CMD_OK                    (0)     /* command execution OK*/
#define SDIO_HOST_RSP_TIMEOUT               (-1)    /* response timeout*/
#define SDIO_HOST_RSP_CRCFAIL               (-2)    /* response CRC fail*/
#define SDIO_HOST_CMD_FAIL                  (-3)    /* command fail*/

/*
    SDIO data transfer result.

    Encoding of sdiohost_waitdataend() result.
*/
#define SDIO_HOST_DATA_OK                   (0)     /* data transfer OK*/
#define SDIO_HOST_DATA_TIMEOUT              (-1)    /* data block timeout*/
#define SDIO_HOST_DATA_CRCFAIL              (-2)    /* data block CRC fail*/
#define SDIO_HOST_DATA_FAIL                 (-3)    /* data transfer fail*/

#define SDIO_HOST_BOOT_ACK_OK               (0)
#define SDIO_HOST_BOOT_ACK_TIMEOUT          (-1)
#define SDIO_HOST_BOOT_ACK_ERROR            (-2)
#define SDIO_HOST_BOOT_END                  (0)
#define SDIO_HOST_BOOT_END_ERROR            (-1)

/*
    SDIO response type

    @note for sdiohost_sendcmd()
*/
typedef enum {
	SDIO_HOST_RSP_NONE,         /* No response*/
	SDIO_HOST_RSP_SHORT,        /* Short response*/
	SDIO_HOST_RSP_LONG,         /* Long response*/
	SDIO_HOST_RSP_SHORT_TYPE2,  /* Short response timeout is 5 bus clock*/
	SDIO_HOST_RSP_LONG_TYPE2,   /* Long response timeout is 5 bus clock*/
	SDIO_HOST_RSP_VOLT_DETECT,  /* voltage detect response*/
	ENUM_DUMMY4WORD(SDIO_HOST_RESPONSE)
} SDIO_HOST_RESPONSE;

#define SDIO_HOST_MAX_VOLT_TIMER  (0xFFF) /* max value of voltage switch timer*/

#define SDIO_DES_TABLE_NUM        (128)
#define SDIO_DES_WORD_SIZE        (3)   /*descriptor 3 word*/
#define SDIO_HOST_MAX_DATA_LENGTH (64*1024*1024)
#if (defined(CONFIG_TARGET_NA51000) || defined(CONFIG_TARGET_NA51000_A64) || defined(CONFIG_TARGET_NA51055) || defined(CONFIG_TARGET_NA51089))
#define SDIO_HOST_DATA_FIFO_DEPTH (16)
#else
#define SDIO_HOST_DATA_FIFO_DEPTH (32)
#endif

/* sdio_protocol.h */
#define SDIO_HOST_WRITE_DATA                (FALSE)
#define SDIO_HOST_READ_DATA                 (TRUE)

/* Command Register Bit */
#define SDIO_CMD_REG_INDEX                  0x0000003F  /* bit 5..0*/
#define SDIO_CMD_REG_NEED_RSP               0x00000040  /* bit 6*/
#define SDIO_CMD_REG_LONG_RSP               0x000000C0  /* bit 7*/
#define SDIO_CMD_REG_RSP_TYPE2              0x00000100  /* bit 8*/
#define SDIO_CMD_REG_APP_CMD                0x00000000  /* bit x*/
#define SDIO_CMD_REG_ABORT                  0x00000800  /* bit 11*/
#define SDIO_CMD_REG_VOLTAGE_SWITCH_DETECT  0x00001000  /* bit 12*/

/* Status/Interrupt Mask Register Bit */
#define SDIO_STATUS_REG_RSP_CRC_FAIL        0x00000001  /* bit 0*/
#define SDIO_STATUS_REG_DATA_CRC_FAIL       0x00000002  /* bit 1*/
#define SDIO_STATUS_REG_RSP_TIMEOUT         0x00000004  /* bit 2*/
#define SDIO_STATUS_REG_DATA_TIMEOUT        0x00000008  /* bit 3*/
#define SDIO_STATUS_REG_RSP_CRC_OK          0x00000010  /* bit 4*/
#define SDIO_STATUS_REG_DATA_CRC_OK         0x00000020  /* bit 5*/
#define SDIO_STATUS_REG_CMD_SEND            0x00000040  /* bit 6*/
#define SDIO_STATUS_REG_DATA_END            0x00000080  /* bit 7*/
#define SDIO_STATUS_REG_INT                 0x00000100  /* bit 8*/
#define SDIO_STATUS_REG_READWAIT            0x00000200  /* bit 9*/
#define SDIO_STATUS_REG_EMMC_BOOTACKREV     0x00008000  /* bit 15*/
#define SDIO_STATUS_REG_EMMC_BOOTACKTOUT    0x00010000  /* bit 16*/
#define SDIO_STATUS_REG_EMMC_BOOTEND        0x00020000  /* bit 17*/
#define SDIO_STATUS_REG_EMMC_BOOTACKERR     0x00040000  /* bit 18*/
#define SDIO_STATUS_REG_DMA_ERR             0x00080000  /* bit 19*/
#define SDIO_INTMASK_ALL                    0x000003FF  /* bit 9..0*/

/* Bus Width Register bit definition */
#define SDIO_BUS_WIDTH1                     0x0  /* bit 1..0*/
#define SDIO_BUS_WIDTH4                     0x1  /* bit 1..0*/
#define SDIO_BUS_WIDTH8                     0x2  /* bit 1..0*/

#define MMCST_RSP_CRC_FAIL                  BIT(0)
#define MMCST_DATA_CRC_FAIL                 BIT(1)
#define MMCST_RSP_TIMEOUT                   BIT(2)
#define MMCST_DATA_TIMEOUT                  BIT(3)
#define MMCST_RSP_CRC_OK                    BIT(4)
#define MMCST_DATA_CRC_OK                   BIT(5)
#define MMCST_CMD_SENT                      BIT(6)
#define MMCST_DATA_END                      BIT(7)
#define MMCST_SDIO_INT                      BIT(8)
#define MMCST_READ_WAIT                     BIT(9)
#define MMCST_CARD_BUSY2READY               BIT(10)
#define MMCST_VOL_SWITCH_END                BIT(11)
#define MMCST_VOL_SWITCH_TIMEOUT            BIT(12)
#define MMCST_RSP_VOL_SWITCH_FAIL           BIT(13)
#define MMCST_VOL_SWITCH_GLITCH             BIT(14)
#define MMCST_EMMC_BOOT_ACK_RECEIVE         BIT(15)
#define MMCST_EMMC_BOOT_ACK_TIMEOUT         BIT(16)
#define MMCST_EMMC_BOOT_END                 BIT(17)
#define MMCST_EMMC_BOOT_ACK_ERROR           BIT(18)
#define MMCST_DMA_ERROR                     BIT(19)

#define SDIO_DELAY_PHASE_SEL_MAX        16
#define SDIO_DELAY_PHASE_UNIT_MAX       8
#define SDIO_AUTO_TUNE_REPEAT           16
#define SDIO_EDGE_RESULT_THD            5       // phase_sel must locate zero bit string and (zero_len >= 5)

typedef enum {
	SDIO_MODE_DS = 25000000,
	SDIO_MODE_HS = 50000000,
	SDIO_MODE_SDR50 = 100000000,
	SDIO_MODE_SDR104 = 208000000,
	ENUM_DUMMY4WORD(SDIO_SPEED_MODE)
} SDIO_SPEED_MODE;

typedef enum {
	SDIO_HOST_SAMPLE_CLK_EDGE_POS,              // sample positive edge
	SDIO_HOST_SAMPLE_CLK_EDGE_NEG,              // sample negative edge
	SDIO_HOST_SAMPLE_CLK_EDGE_BOTH,             // sample positive/negative edge both
} SDIO_HOST_SAMPLE_CLK_EDGE;

typedef enum {
	SDIO_HOST_DET_MODE_AUTO,
	SDIO_HOST_DET_MODE_FIRST_EDGE,              // detect first change edge only
} SDIO_HOST_DET_MODE;

#endif
