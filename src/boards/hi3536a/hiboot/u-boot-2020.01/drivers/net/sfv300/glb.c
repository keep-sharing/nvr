// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Shenshu Technologies CO., LIMITED.
 *
 */


#define ETH_SFV300

#include <config.h>
#include "bspeth.h"
#include "mac.h"
#include "ctrl.h"
#include "glb.h"

int bspeth_glb_preinit_dummy(struct bspeth_netdev_local const *ld)
{
	if (ld == NULL)
		return -1;

	local_lock_init(ld);

	/*
	 * bspeth_glb_preinit_dummy
	 * HW MAX DEFAULT RX-PKT-LEN [42,1518]
	 * HW MAC FILTER TABLE DISABLE
	 */

	/* soft reset */
	bspeth_writel_bits(ld, 1, GLB_SOFT_RESET, BITS_ETH_SOFT_RESET);
	mdelay(1);
	bspeth_writel_bits(ld, 0, GLB_SOFT_RESET, BITS_ETH_SOFT_RESET);

	bspeth_set_endian_mode(ld, ETH_LITTLE_ENDIAN);

	bspeth_set_linkstat(ld, 0);

	bspeth_set_negmode(ld, ETH_NEGMODE_CPUSET);

	/* RMII mode */
	bspeth_set_mii_mode(ld, ud_bit_name(ETH_MII_RMII_MODE));

	bspeth_writel_bits(ld, ~0, GLB_RW_IRQ_ENA, ud_bit_name(BITS_IRQS));
	bspeth_writel_bits(ld, ~0, GLB_RW_IRQ_ENA, ud_bit_name(BITS_IRQS_ENA));
	bspeth_writel_bits(ld, ~0, GLB_RW_IRQ_ENA, BITS_IRQS_ENA_ALLPORT);
	bspeth_irq_disable(ld, ~0);

	/* init */
	bspeth_writel(ld, 0, GLB_FWCTRL);
	bspeth_writel(ld, 0, GLB_MACTCTRL);

	/* disable vlan func */
	bspeth_writel_bits(ld, 0, GLB_FWCTRL, BITS_VLAN_ENABLE);

	/* enable UpEther<->CPU */
	bspeth_writel_bits(ld, 1, GLB_FWCTRL, BITS_FW2CPU_ENA_UP);
	bspeth_writel_bits(ld, 0, GLB_FWCTRL, BITS_FWALL2CPU_UP);
	bspeth_writel_bits(ld, 0, GLB_MACTCTRL, BITS_BROAD2CPU_UP);
	bspeth_writel_bits(ld, 1, GLB_MACTCTRL, BITS_MACT_ENA_UP);

	/* enable DownEther<->CPU and UpEther<->CPU */
	bspeth_writel_bits(ld, 1, GLB_FWCTRL, BITS_FW2CPU_ENA_DOWN);
	bspeth_writel_bits(ld, 0, GLB_FWCTRL, BITS_FWALL2CPU_DOWN);
	bspeth_writel_bits(ld, 0, GLB_MACTCTRL, BITS_BROAD2CPU_DOWN);
	bspeth_writel_bits(ld, 1, GLB_MACTCTRL, BITS_MACT_ENA_DOWN);

	bspeth_set_mac_leadcode_cnt_limit(ld, 0);

	return 0;
}
