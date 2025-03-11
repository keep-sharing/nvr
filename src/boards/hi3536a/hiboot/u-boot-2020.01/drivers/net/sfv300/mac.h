// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */


#ifndef __ETH_MAC_H__
#define __ETH_MAC_H__

#include "bspeth.h"

#ifdef ETH_SFV300

#define U_MAC_PORTSEL     0x0200
#define D_MAC_PORTSEL     0x2200
#define U_MAC_RO_STAT     0x0204
#define D_MAC_RO_STAT     0x2204
#define U_MAC_PORTSET     0x0208
#define D_MAC_PORTSET     0x2208
#define U_MAC_STAT_CHANGE 0x020C
#define D_MAC_STAT_CHANGE 0x220C
#define U_MAC_SET         0x0210
#define D_MAC_SET         0x2210
#define U_MAC_RX_IPGCTRL  0x0214
#define D_MAC_RX_IPGCTRL  0x2214
#define U_MAC_TX_IPGCTRL  0x0218
#define D_MAC_TX_IPGCTRL  0x2218

/* bits of UD_MAC_PORTSET and UD_MAC_RO_STAT */
#define BITS_MACSTAT mk_bits(0, 3)

/* bits of U_MAC_PORTSEL and D_MAC_PORTSEL */
#define BITS_NEGMODE  mk_bits(0, 1)
#define BITS_MII_MODE mk_bits(1, 1)

/* bits of U_MAC_TX_IPGCTRL and D_MAC_TX_IPGCTRL */
#define BITS_PRE_CNT_LIMIT mk_bits(23, 3)
#define BITS_IPG           mk_bits(16, 7)
#define BITS_FC_INTER      mk_bits(0, 16)

#endif

#define ETH_SPD_100M bit(2)
#define ETH_LINKED   bit(1)
#define ETH_DUP_FULL bit(0)

void bspeth_set_mac_leadcode_cnt_limit(struct bspeth_netdev_local const *ld, u32 cnt);
void bspeth_set_mac_trans_interval_bits(struct bspeth_netdev_local const *ld, u32 nbits);
void bspeth_set_mac_fc_interval(struct bspeth_netdev_local const *ld, u32 para);

void bspeth_set_linkstat(struct bspeth_netdev_local const *ld, u32 mode);
u32 bspeth_get_linkstat(struct bspeth_netdev_local const *ld);

#define ETH_NEGMODE_CPUSET 1
#define ETH_NEGMODE_AUTO   0

void bspeth_set_negmode(struct bspeth_netdev_local const *ld, u32 mode);
u32 bspeth_get_negmode(struct bspeth_netdev_local const *ld);

#define ETH_MII_MODE  0
#define ETH_RMII_MODE 1

void bspeth_set_mii_mode(struct bspeth_netdev_local const *ld, u32 mode);

#endif
