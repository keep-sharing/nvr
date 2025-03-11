/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef _MCI_V200_H_
#define _MCI_V200_H_

#define POWER_ON        0x1
#define POWER_OFF       0x0

#define CARD_UNPLUGED   0x1
#define CARD_PLUGED     0x0

#define ENABLE          0x1
#define DISABLE         0x0

#define MCI_DETECT_TIMEOUT	(HZ / 2)

#define MCI_REQUEST_TIMEOUT	(5 * HZ)

#define MAX_RETRY_COUNT	100000

#define MMC_CLK		100000000
#define MMC_CCLK_MIN	400000

#define MCI_DEBUG	DISABLE
/* Open it as you need #define MCI_DEBUG	ENABLE */

#if MCI_DEBUG
extern int debug_type;
#define MCI_DEBUG_TYPE	(MCI_DEBUG_TYPE_REG | \
				MCI_DEBUG_TYPE_FUN | \
				MCI_DEBUG_TYPE_CMD | \
				MCI_DEBUG_TYPE_INFO | \
				MCI_DEBUG_TYPE_ERR)

#define MCI_DEBUG_TYPE_REG	(0x01 << 0)
#define MCI_DEBUG_TYPE_FUN	(0x01 << 1)
#define MCI_DEBUG_TYPE_CMD	(0x01 << 2)
#define MCI_DEBUG_TYPE_INFO	(0x01 << 3)
#define MCI_DEBUG_TYPE_ERR	(0x01 << 4)

#define MCI_DEBUG_FMT "MCI_DEBUG "

extern char *get_debug_type_string(int type);
#define mci_debug(type, msg...) do { \
	if (debug_type & type) { \
		printf(MCI_DEBUG_FMT "(%s) %s:%d: ", \
		get_debug_type_string(type), \
		__func__, __LINE__); \
		printf(msg); \
		printf("\n"); \
	} \
} while (0)

#define mci_debug_reg(msg...)	mci_debug(MCI_DEBUG_TYPE_REG, msg)
#define mci_debug_fun(msg...)	mci_debug(MCI_DEBUG_TYPE_FUN, msg)
#define mci_debug_cmd(msg...)	mci_debug(MCI_DEBUG_TYPE_CMD, msg)
#define mci_debug_info(msg...) mci_debug(MCI_DEBUG_TYPE_INFO, msg)
#define mci_debug_err(msg...)	mci_debug(MCI_DEBUG_TYPE_ERR, msg)

#define MCI_ASSERT_FMT "MCI_ASSERT "

#define mci_assert(cond) do { \
	if (!(cond)) { \
		printf(MCI_ASSERT_FMT "%s:%d\n", __func__, __LINE__); \
		BUG(); \
	} \
} while (0)
#else
#define mci_debug(type, msg...)
#define mci_debug_reg(msg...)
#define mci_debug_fun(msg...)
#define mci_debug_cmd(msg...)
#define mci_debug_info(msg...)
#define mci_debug_err(msg...)
#define mci_assert(cond)
#endif

#define MCI_ERROR_FMT "MCI_ERROR "

#define mci_error(s...) do { \
	printf(MCI_ERROR_FMT "%s:%d: ", __func__, __LINE__); \
	printf(s); \
	printf("\n"); \
} while (0)

#define mci_readl(addr) ((unsigned int)(readl((uintptr_t)(addr))))

#define mci_writel(v, addr) do {writel(v, (uintptr_t)(addr)); \
	mci_debug_reg("writel(0x%04X) = 0x%08X", (uintptr_t)(addr), (unsigned int)(v)); \
} while (0)

struct mci_dma_des {
	unsigned long idmac_des_ctrl;
	unsigned long idmac_des_buf_size;
	unsigned long idmac_des_buf_addr;
	unsigned long idmac_des_next_addr;
};

struct mci_host {
	const char *name;
	struct mmc *mmc;
	unsigned long	base;
	unsigned int	card_status;
	unsigned int	dev_id;
	unsigned int	port;
	struct mmc_cmd *cmd;
	struct mci_dma_des *dma_des;
	struct mmc_config cfg;
	unsigned int            is_tuning;
};

typedef union {
	unsigned int cmd_arg;
	struct cmd_bits_arg {
		unsigned int cmd_index : 6;
		unsigned int response_expect : 1;
		unsigned int response_length : 1;
		unsigned int check_response_crc : 1;
		unsigned int data_transfer_expected : 1;
		unsigned int read_write : 1;
		unsigned int transfer_mode : 1;
		unsigned int send_auto_stop : 1;
		unsigned int wait_prvdata_complete : 1;
		unsigned int stop_abort_cmd : 1;
		unsigned int send_initialization : 1;
		unsigned int card_number : 5;
		unsigned int update_clk_reg_only : 1; /* bit 21 */
		unsigned int reserved : 9;
		unsigned int start_cmd : 1; /* HSB */
	} bits;
} cmd_arg_s;

#endif
