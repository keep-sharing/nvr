// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */

#ifndef __GMAC_CTRL_H__
#define __GMAC_CTRL_H__

#include "gmac.h"
#include "util.h"

#define STATION_ADDR_LOW			0x0000
#define STATION_ADDR_HIGH			0x0004
#define MAC_DUPLEX_HALF_CTRL		0x0008

#define MAX_FRM_SIZE				0x003c
#define BITS_MAX_FRM_SIZE			mk_bits(0, 14)

#define PORT_MODE					0x0040
#define BITS_PORT_MODE				mk_bits(0, 3)

#define PORT_EN						0x0044
#define BITS_TX_EN					mk_bits(2, 1)
#define BITS_RX_EN					mk_bits(1, 1)

#define REC_FILT_CONTROL			0x0064
#define BIT_CRC_ERR_PASS			mk_bits(5, 1)
#define BIT_PAUSE_FRM_PASS			mk_bits(4, 1)
#define BIT_VLAN_DROP_EN			mk_bits(3, 1)
#define BIT_BC_DROP_EN				mk_bits(2, 1)
#define BIT_MC_MATCH_EN				mk_bits(1, 1)
#define BIT_UC_MATCH_EN				mk_bits(0, 1)

#define CF_CRC_STRIP				0x01B0
#define BIT_CF_CRC_STRIP			mk_bits(0, 1)

#define MODE_CHANGE_EN				0x01b4
#define BIT_MODE_CHANGE_EN			mk_bits(0, 1)

#define RECV_CONTROL				0x01e0
#define BIT_STRIP_PAD_EN			mk_bits(3, 1)
#define BIT_RUNT_PKT_EN				mk_bits(4, 1)

#define CRF_MIN_PACKET				0x0210
#define BIT_TSO_VERSION				mk_bits(20, 12)

#define RX_FQ_START_ADDR			0x0500
#define RX_FQ_DEPTH					0x0504
#define BITS_RX_FQ_DEPTH			mk_bits(0, 19)
#define BITS_RX_FQ_ADDR_HI8			mk_bits(24, 8)
#define RX_FQ_WR_ADDR				0x0508
#define BITS_RX_FQ_WR_ADDR			mk_bits(0, 21)
#define RX_FQ_RD_ADDR				0x050c
#define BITS_RX_FQ_RD_ADDR			mk_bits(0, 21)
#define RX_FQ_VLDDESC_CNT			0x0510
#define BITS_RX_FQ_VLDDESC_CNT		mk_bits(0, 16)
#define RX_FQ_ALEMPTY_TH			0x0514
#define BITS_RX_FQ_ALEMPTY_TH		mk_bits(0, 16)
#define RX_FQ_REG_EN				0x0518
#define BITS_RX_FQ_START_ADDR_EN	mk_bits(2, 1)
#define BITS_RX_FQ_DEPTH_EN			mk_bits(1, 1)
#define BITS_RX_FQ_RD_ADDR_EN		mk_bits(0, 1)
#define RX_FQ_ALFULL_TH				0x051c
#define BITS_RX_FQ_ALFULL_TH		mk_bits(0, 16)

#define RX_BQ_START_ADDR			0x0520
#define RX_BQ_DEPTH					0x0524
#define BITS_RX_BQ_DEPTH			mk_bits(0, 19)
#define BITS_RX_BQ_ADDR_HI8			mk_bits(24, 8)
#define RX_BQ_WR_ADDR				0x0528
#define BITS_RX_BQ_WR_ADDR			mk_bits(0, 21)
#define RX_BQ_RD_ADDR				0x052c
#define BITS_RX_BQ_RD_ADDR			mk_bits(0, 21)
#define RX_BQ_FREE_DESC_CNT			0x0530
#define BITS_RX_BQ_FREE_DESC_CNT	mk_bits(0, 16)
#define RX_BQ_ALEMPTY_TH			0x0534
#define BITS_RX_BQ_ALEMPTY_TH		mk_bits(0, 16)
#define RX_BQ_REG_EN				0x0538
#define BITS_RX_BQ_START_ADDR_EN	mk_bits(2, 1)
#define BITS_RX_BQ_DEPTH_EN			mk_bits(1, 1)
#define BITS_RX_BQ_WR_ADDR_EN		mk_bits(0, 1)
#define RX_BQ_ALFULL_TH				0x053c
#define BITS_RX_BQ_ALFULL_TH		mk_bits(0, 16)

#define TX_BQ_START_ADDR			0x0580
#define TX_BQ_DEPTH					0x0584
#define BITS_TX_BQ_DEPTH			mk_bits(0, 19)
#define BITS_TX_BQ_ADDR_HI8			mk_bits(24, 8)
#define TX_BQ_WR_ADDR				0x0588
#define BITS_TX_BQ_WR_ADDR			mk_bits(0, 21)
#define TX_BQ_RD_ADDR				0x058c
#define BITS_TX_BQ_RD_ADDR			mk_bits(0, 21)
#define TX_BQ_VLDDESC_CNT			0x0590
#define BITS_TX_BQ_VLDDESC_CNT		mk_bits(0, 16)
#define TX_BQ_ALEMPTY_TH			0x0594
#define BITS_TX_BQ_ALEMPTY_TH		mk_bits(0, 16)
#define TX_BQ_REG_EN				0x0598
#define BITS_TX_BQ_START_ADDR_EN	mk_bits(2, 1)
#define BITS_TX_BQ_DEPTH_EN			mk_bits(1, 1)
#define BITS_TX_BQ_RD_ADDR_EN		mk_bits(0, 1)
#define TX_BQ_ALFULL_TH				0x059c
#define BITS_TX_BQ_ALFULL_TH		mk_bits(0, 16)

#define TX_RQ_START_ADDR			0x05a0
#define TX_RQ_DEPTH					0x05a4
#define BITS_TX_RQ_DEPTH			mk_bits(0, 19)
#define BITS_TX_RQ_ADDR_HI8			mk_bits(24, 8)
#define TX_RQ_WR_ADDR				0x05a8
#define BITS_TX_RQ_WR_ADDR			mk_bits(0, 21)
#define TX_RQ_RD_ADDR				0x05ac
#define BITS_TX_RQ_RD_ADDR			mk_bits(0, 21)
#define TX_RQ_FREE_DESC_CNT			0x05b0
#define BITS_TX_RQ_FREE_DESC_CNT	mk_bits(0, 16)
#define TX_RQ_ALEMPTY_TH			0x05b4
#define BITS_TX_RQ_ALEMPTY_TH		mk_bits(0, 16)
#define TX_RQ_REG_EN				0x05b8
#define BITS_TX_RQ_START_ADDR_EN	mk_bits(2, 1)
#define BITS_TX_RQ_DEPTH_EN			mk_bits(1, 1)
#define BITS_TX_RQ_WR_ADDR_EN		mk_bits(0, 1)
#define TX_RQ_ALFULL_TH				0x05bc
#define BITS_TX_RQ_ALFULL_TH		mk_bits(0, 16)

#define RAW_PMU_INT					0x05c0

#define ENA_PMU_INT						0x05c4
#define ENA_BITS_MAC_FIFO_ERR_INT		mk_bits(30, 1)
#define ENA_BITS_TX_RQ_IN_TIMEOUT_INT	mk_bits(29, 1)
#define ENA_BITS_RX_BQ_IN_TIMEOUT_INT	mk_bits(28, 1)
#define ENA_BITS_TXOUTCFF_FULL_INT		mk_bits(27, 1)
#define ENA_BITS_TXOUTCFF_EMPTY_INT		mk_bits(26, 1)
#define ENA_BITS_TXCFF_FULL_INT			mk_bits(25, 1)
#define ENA_BITS_TXCFF_EMPTY_INT		mk_bits(24, 1)
#define ENA_BITS_RXOUTCFF_FULL_INT		mk_bits(23, 1)
#define ENA_BITS_RXOUTCFF_EMPTY_INT		mk_bits(22, 1)
#define ENA_BITS_RXCFF_FULL_INT			mk_bits(21, 1)
#define ENA_BITS_RXCFF_EMPTY_INT		mk_bits(20, 1)
#define ENA_BITS_TX_RQ_IN_INT			mk_bits(19, 1)
#define ENA_BITS_TX_BQ_OUT_INT			mk_bits(18, 1)
#define ENA_BITS_RX_BQ_IN_INT			mk_bits(17, 1)
#define ENA_BITS_RX_FQ_OUT_INT			mk_bits(16, 1)
#define ENA_BITS_TX_RQ_EMPTY_INT		mk_bits(15, 1)
#define ENA_BITS_TX_RQ_FULL_INT			mk_bits(14, 1)
#define ENA_BITS_TX_RQ_ALEMPTY_INT		mk_bits(13, 1)
#define ENA_BITS_TX_RQ_ALFULL_INT		mk_bits(12, 1)
#define ENA_BITS_TX_BQ_EMPTY_INT		mk_bits(11, 1)
#define ENA_BITS_TX_BQ_FULL_INT			mk_bits(10, 1)
#define ENA_BITS_TX_BQ_ALEMPTY_INT		mk_bits(9, 1)
#define ENA_BITS_TX_BQ_ALFULL_INT		mk_bits(8, 1)
#define ENA_BITS_RX_BQ_EMPTY_INT		mk_bits(7, 1)
#define ENA_BITS_RX_BQ_FULL_INT			mk_bits(6, 1)
#define ENA_BITS_RX_BQ_ALEMPTY_INT		mk_bits(5, 1)
#define ENA_BITS_RX_BQ_ALFULL_INT		mk_bits(4, 1)
#define ENA_BITS_RX_FQ_EMPTY_INT		mk_bits(3, 1)
#define ENA_BITS_RX_FQ_FULL_INT			mk_bits(2, 1)
#define ENA_BITS_RX_FQ_ALEMPTY_INT		mk_bits(1, 1)
#define ENA_BITS_RX_FQ_ALFULL_INT		mk_bits(0, 1)

#define STATUS_PMU_INT						0x05c8
#define STATUS_BITS_MAC_FIFO_ERR_INT		mk_bits(30, 1)
#define STATUS_BITS_TX_RQ_IN_TIMEOUT_INT	mk_bits(29, 1)
#define STATUS_BITS_RX_BQ_IN_TIMEOUT_INT	mk_bits(28, 1)
#define STATUS_BITS_TXOUTCFF_FULL_INT		mk_bits(27, 1)
#define STATUS_BITS_TXOUTCFF_EMPTY_INT		mk_bits(26, 1)
#define STATUS_BITS_TXCFF_FULL_INT			mk_bits(25, 1)
#define STATUS_BITS_TXCFF_EMPTY_INT			mk_bits(24, 1)
#define STATUS_BITS_RXOUTCFF_FULL_INT		mk_bits(23, 1)
#define STATUS_BITS_RXOUTCFF_EMPTY_INT		mk_bits(22, 1)
#define STATUS_BITS_RXCFF_FULL_INT			mk_bits(21, 1)
#define STATUS_BITS_RXCFF_EMPTY_INT			mk_bits(20, 1)
#define STATUS_BITS_TX_RQ_IN_INT			mk_bits(19, 1)
#define STATUS_BITS_TX_BQ_OUT_INT			mk_bits(18, 1)
#define STATUS_BITS_RX_BQ_IN_INT			mk_bits(17, 1)
#define STATUS_BITS_RX_FQ_OUT_INT			mk_bits(16, 1)
#define STATUS_BITS_TX_RQ_EMPTY_INT			mk_bits(15, 1)
#define STATUS_BITS_TX_RQ_FULL_INT			mk_bits(14, 1)
#define STATUS_BITS_TX_RQ_ALEMPTY_INT		mk_bits(13, 1)
#define STATUS_BITS_TX_RQ_ALFULL_INT		mk_bits(12, 1)
#define STATUS_BITS_TX_BQ_EMPTY_INT			mk_bits(11, 1)
#define STATUS_BITS_TX_BQ_FULL_INT			mk_bits(10, 1)
#define STATUS_BITS_TX_BQ_ALEMPTY_INT		mk_bits(9, 1)
#define STATUS_BITS_TX_BQ_ALFULL_INT		mk_bits(8, 1)
#define STATUS_BITS_RX_BQ_EMPTY_INT			mk_bits(7, 1)
#define STATUS_BITS_RX_BQ_FULL_INT			mk_bits(6, 1)
#define STATUS_BITS_RX_BQ_ALEMPTY_INT		mk_bits(5, 1)
#define STATUS_BITS_RX_BQ_ALFULL_INT		mk_bits(4, 1)
#define STATUS_BITS_RX_FQ_EMPTY_INT			mk_bits(3, 1)
#define STATUS_BITS_RX_FQ_FULL_INT			mk_bits(2, 1)
#define STATUS_BITS_RX_FQ_ALEMPTY_INT		mk_bits(1, 1)
#define STATUS_BITS_RX_FQ_ALFULL_INT		mk_bits(0, 1)

#define DESC_WR_RD_ENA				0x05CC

#define IN_QUEUE_TH					0x05d8
#define BITS_TX_RQ_IN_TH			mk_bits(16, 8)
#define BITS_RX_BQ_IN_TH			mk_bits(0, 8)

#define OUT_QUEUE_TH				0x05dc
#define BITS_TX_BQ_OUT_TH			mk_bits(16, 8)
#define BITS_RX_FQ_OUT_TH			mk_bits(0, 8)

#define RX_BQ_IN_TIMEOUT_TH			0x05E0
#define BITS_RX_BQ_IN_TIMEOUT_TH	mk_bits(0, 24)

#define TX_RQ_IN_TIMEOUT_TH			0x05e4
#define BITS_TX_RQ_IN_TIMEOUT_TH	mk_bits(0, 24)

#define STOP_CMD					0x05e8
#define BITS_TX_STOP_EN				mk_bits(1, 1)
#define BITS_RX_STOP_EN				mk_bits(0, 1)

#define FLUSH_CMD					0x05EC
#define BITS_TX_FLUSH_CMD			mk_bits(3, 1)
#define BITS_RX_FLUSH_CMD			mk_bits(2, 1)
#define BITS_TX_FLUSH_FLAG			mk_bits(1, 1)
#define BITS_RX_FLUSH_FLAG			mk_bits(0, 1)

#define  RAW_INT_ALL_MASK 0xffffffff

void gmac_set_macif(struct gmac_netdev_local const *ld,
		  int mode, unsigned int speed);
int gmac_hw_set_macaddress(struct gmac_netdev_local const *ld,
		  const unsigned char *mac, int len);
int gmac_hw_get_macaddress(struct gmac_netdev_local const *ld,
		  unsigned char *mac, int len);
int gmac_glb_preinit_dummy(struct gmac_netdev_local const *ld);
void gmac_sys_init(void);
void gmac_sys_exit(void);
void gmac_sys_allstop(void);
int gmac_set_hwq_depth(struct gmac_netdev_local const *ld);
int gmac_set_rx_fq_hwq_addr(struct gmac_netdev_local const *ld,
		  phys_addr_t phy_addr);
int gmac_set_rx_bq_hwq_addr(struct gmac_netdev_local const *ld,
		  phys_addr_t phy_addr);
int gmac_set_tx_bq_hwq_addr(struct gmac_netdev_local const *ld,
		  phys_addr_t phy_addr);
int gmac_set_tx_rq_hwq_addr(struct gmac_netdev_local const *ld,
		  phys_addr_t phy_addr);
int gmac_clear_irqstatus(struct gmac_netdev_local const *ld, int irqs);
int gmac_read_irqstatus(struct gmac_netdev_local *ld);
int gmac_irq_enable(struct gmac_netdev_local *ld, int irqs);
int gmac_irq_disable(struct gmac_netdev_local *ld, int irqs);
void gmac_desc_enable(struct gmac_netdev_local const *ld, u32 desc_ena);
void gmac_desc_disable(struct gmac_netdev_local const *ld, u32 desc_dis);
void gmac_desc_flush(struct gmac_netdev_local const *ld);

u32 gmac_readl(struct gmac_netdev_local const *ld, u32 ofs);
void gmac_writel(struct gmac_netdev_local const *ld, u32 v, u32 ofs);
u32 gmac_readl_bits(struct gmac_netdev_local const *ld, u32 ofs, u32 bits_desc);
void gmac_writel_bits(struct gmac_netdev_local const *ld, u32 v, u32 ofs, u32 bits_desc);
void gmac_trace(int level, const char *fmt, ...);
void gmac_error(const char *fmt, ...);
void gmac_assert(bool cond);
#endif

