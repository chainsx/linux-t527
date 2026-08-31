/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * SUNXI TWI Controller Dvfs Definition
 *
 */

#ifndef _SUNXI_TWI_DVFS_H_
#define _SUNXI_TWI_DVFS_H_

#include <linux/bitfield.h>
#include <linux/twi/sunxi-twi.h>
#include "../twi-sunxi.h"

/* I2C DVFS Register Offset */
#define DVFS_CHAN_MAX		(8)
#define TWI_DVFS_CTL			(0x314)
#define TWI_DVFS_INT_CTRL		(0x318)
#define TWI_DVFS_INT_STAT		(0x31C)
#define TWI_DVFS_ARBITER		(0x320)
#define TWI_DVFS_DEV_CFG(x)		(0x330 + (x) * 0x4)
#define TWI_DVFS_DEV_STAT(x)	(0x360 + (x) * 0x4)
#define TWI_DVFS_RR8_CTL		(0x380)

#define TWI_DVFS_PKT_INTERVAL		GENMASK(31, 16)
#define TWI_DVFS_RB_EN(x)			BIT(x + 8)
#define TWI_DVFS_CH_EN(x)			BIT(x + 0)
#define TWI_DVFS_CH_ALL_ENABLE		GENMASK(7, 0)
#define TWI_DVFS_IRQ_RB_ERR(x)		BIT(x + 16)
#define TWI_DVFS_IRQ_TRAN_OK(x)		BIT(x + 8)
#define TWI_DVFS_IRQ_TRAN_ERR(x)	BIT(x + 0)

#define TWI_DVFS_DEV_CFG_MASK_EN	BIT(31)
#define TWI_DVFS_DEV_CFG_MASK		GENMASK(15, 8)
#define TWI_DVFS_DEV_CFG_REG_ADDR	GENMASK(7, 0)

#define TWI_DVFS_DEV_STAT_RB_DATA	GENMASK(31, 24)
#define TWI_DVFS_DEV_STAT_SID_PMU	GENMASK(23, 16)
#define TWI_DVFS_DEV_STAT_REG_ADDR	GENMASK(15, 8)
#define TWI_DVFS_DEV_STAT_SEND_DATA	GENMASK(7, 0)

#define TWI_DVFS_DEV_PROI_LOW(x)	((x) * (0x04))
#define TWI_DVFS_DEV_PROI_MASK(x)	GENMASK((TWI_DVFS_DEV_PROI_LOW(x) + 3), TWI_DVFS_DEV_PROI_LOW(x))

struct sunxi_twi_dvfs_channel {
	u8 priority;
	bool channel_en;
	bool readback_en;
	u8 mask;
	u8 reg_addr;
};

struct sunxi_twi_dvfs_device {
	struct sunxi_twi_dvfs_channel *dvfs_channel;
	u8 addr; /*dvfs device addr*/
	u16 interval; /*packet interval*/
};


int sunxi_twi_dvfs_core_process(struct sunxi_twi *twi);

#endif /* _SUNXI_TWI_DVFS_H_ */