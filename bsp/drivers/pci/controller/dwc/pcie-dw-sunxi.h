// SPDX-License-Identifier: GPL-2.0
/*
 * PCIe controller driver header for Allwinner SoCs.
 *
 * Copyright (C) 2026 Allwinner Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef _PCIE_DW_SUNXI_H_
#define _PCIE_DW_SUNXI_H_

#define to_sunxi_pcie(x) dev_get_drvdata((x)->dev)

/* User Define APP Register */
#define PCIE_LTSSM_ENABLE	0x0c00
	#define APP_LTSSM_ENABLE	BIT(0)
#define SII_INT_MASK0		0x0e00
#define SII_INT_STAS0		0x0e08
	#define INTX_TX_DEASSERT_MASK	GENMASK(28, 25)
	#define INTX_TX_ASSERT_MASK	GENMASK(24, 21)
	#define INTX_RX_DEASSERT_MASK	GENMASK(12, 9)
	#define INTX_RX_ASSERT_MASK	GENMASK(8, 5)
	#define INTX_TX_DEASSERT(x)	BIT((x) + __ffs(INTX_TX_DEASSERT_MASK))
	#define INTX_TX_ASSERT(x)	BIT((x) + __ffs(INTX_TX_ASSERT_MASK))
	#define INTX_RX_DEASSERT(x)	BIT((x) + __ffs(INTX_RX_DEASSERT_MASK))
	#define INTX_RX_ASSERT(x)	BIT((x) + __ffs(INTX_RX_ASSERT_MASK))
#define SII_INT_MASK1		0x0e04
#define SII_INT_STAS1		0x0e0c
	#define RDLH_LINK_UP		BIT(1)
	#define SMLH_LINK_UP		BIT(0)
#define SII_INT_MASK2		0x0e10
#define SII_INT_STAS2		0x0e18
#define SII_INT_MASK3		0x0e14
#define SII_INT_STAS3		0x0e1c

#define PCIE_CTRL_VER		0x7ffd0
#define PCIE_PHY_VER		0x7ffe0
#define PCIE_TOP_VER		0x7fffc

#endif /* _PCIE_DW_SUNXI_H_ */
