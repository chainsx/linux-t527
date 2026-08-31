// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014-2025 JLSemi Limited
 * All Rights Reserved
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE of JLSemi Limited
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code.
 *
 * No part of this code may be reproduced, stored in a retrieval system,
 * or transmitted, in any form or by any means, electronic, mechanical,
 * photocopying, recording, or otherwise, without the prior written
 * permission of JLSemi Limited
 */

#include "jlsemi-core.h"
#include <linux/phy.h>
#include <linux/module.h>
#include <linux/netdevice.h>

#define JLSEMI_DRIVER_VERSION		"v1.3.2"


#if (JLSEMI_DEBUG_INFO)
static int jlsemi_phy_reg_print(struct phy_device *phydev)
{
	int ret;
	int i;

	for (i = 0; i < ARRAY_SIZE(jl_phy); i++) {
		if (jl_phy[i].enable) {
			ret = jlsemi_read_paged(phydev, jl_phy[i].page,
						jl_phy[i].reg);
			JL_PHY_DBG_MSG(_DBG_INFO, "%s: 0x%x\n", jl_phy[i].string, ret);
			if (ret < 0)
				return ret;
		}
	}

	return 0;
}
#endif


static int jlsemi_probe(struct phy_device *phydev)
{
	struct device *dev = jlsemi_mdio_device_get(phydev);
	struct jlsemi_priv *priv = NULL;
	int err;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	phydev->priv = priv;

#if (JLSEMI_KERNEL_DEVICE_TREE_USE)
	if (!dev->of_node)
		JL_PHY_DBG_MSG(_DBG_INFO, "Find device node failed\n");
#endif
	err = jlsemi_operation_args_get(phydev);
	if (err < 0)
		return err;

	JL_PHY_DBG_MSG(_DBG_INFO, "phy_id 0x%x, drv->phy_id 0x%x, phy_addr 0x%x!\n",
					phydev->phy_id, phydev->drv->phy_id, phydev->mdio.addr);

	if (phydev->drv->phy_id == JL2XX1_PHY_ID) {
		priv->nstats = ARRAY_SIZE(jl2xxx_hw_stats);
		priv->hw_stats = jl2xxx_hw_stats;
		priv->stats = kcalloc(priv->nstats, sizeof(u64), GFP_KERNEL);
		if (!priv->stats)
			return -ENOMEM;
	}

	return 0;
}


#if (JLSEMI_PHY_ANEG_DONE)
static inline int jlsemi_aneg_done(struct phy_device *phydev)
{
	u16 phy_mode;
	int value = phy_read(phydev, MII_BMSR);

	if (phydev->drv->phy_id == JL2XX1_PHY_ID) {
		phy_mode = jl2xx1_interface_mode_get(phydev);

		// fiber not an complite
		if (((phy_mode == JL2XXX_FIBER_RGMII_MODE) ||
			(phy_mode == JL2XXX_UTP_FIBER_RGMII_MODE)))
			return BMSR_ANEGCOMPLETE;
	}

	return (value < 0) ? value : (value & BMSR_ANEGCOMPLETE);
}
#endif

static int jlsemi_suspend(struct phy_device *phydev)
{
	struct jlsemi_priv *priv = phydev->priv;

	if (phydev->drv->phy_id == JL2XX1_PHY_ID) {
		/* clear wol event */
		if (priv->wol.enable & JL2XXX_WOL_STATIC_OP_EN) {
			jlsemi_set_bits(phydev, JL2XX1_PAGE_4608,
					JL2XX1_WOL_STAS_REG,
					JL2XX1_WOL_EVENT);
			jlsemi_clear_bits(phydev, JL2XX1_PAGE_4608,
					JL2XX1_WOL_STAS_REG,
					JL2XX1_WOL_EVENT);
		}
	}
	return genphy_suspend(phydev);
}

static int jlsemi_resume(struct phy_device *phydev)
{
	return genphy_resume(phydev);
}

static void jlsemi_remove(struct phy_device *phydev)
{
	struct device *dev = jlsemi_mdio_device_get(phydev);
	struct jlsemi_priv *priv = phydev->priv;

	if (priv && priv->stats)
		kfree(priv->stats);
	if (priv)
		devm_kfree(dev, priv);
}

static int jlsemi_config_aneg(struct phy_device *phydev)
{
	u16 phy_mode;

	if (phydev->drv->phy_id == JL2XX1_PHY_ID) {
		phy_mode = jl2xx1_interface_mode_get(phydev);

		if (((phy_mode == JL2XXX_FIBER_RGMII_MODE) ||
			(phy_mode == JL2XXX_UTP_FIBER_RGMII_MODE)))
			return jl2xx1_config_aneg_fiber(phydev);
	}

	return genphy_config_aneg(phydev);
}

static int jlsemi_config_init(struct phy_device *phydev)
{
	struct jlsemi_priv *priv = phydev->priv;
	int ret;

	if (!priv->static_inited) {
#if (JLSEMI_DEBUG_INFO)
		JL_PHY_DBG_MSG(_DBG_INFO, "jlsemi_config_init_before:\n");
		ret = jlsemi_phy_reg_print(phydev);
		if (ret < 0)
			return ret;
#endif
		ret = jlsemi_static_op_init(phydev);
		if (ret < 0)
			return ret;
#if (JLSEMI_DEBUG_INFO)
		JL_PHY_DBG_MSG(_DBG_INFO, "jlsemi_config_init_after:\n");
		ret = jlsemi_phy_reg_print(phydev);
		if (ret < 0)
			return ret;
#endif
		priv->static_inited = JLSEMI_PHY_NOT_REENTRANT;
	}

	JL_PHY_DBG_MSG(_DBG_DEBUG, "dev_addr %0X:%0X:%0X:%0X:%0X:%0X\n",
		phydev->attached_dev->dev_addr[0], phydev->attached_dev->dev_addr[1],
		phydev->attached_dev->dev_addr[2], phydev->attached_dev->dev_addr[3],
		phydev->attached_dev->dev_addr[4], phydev->attached_dev->dev_addr[5]);

	return 0;
}

#if (JLSEMI_PHY_WOL)
static void jlsemi_get_wol(struct phy_device *phydev,
				struct ethtool_wolinfo *wol)
{
	struct jlsemi_priv *priv = phydev->priv;
	int wol_en;

	if (priv->wol.ethtool) {
		wol->supported = WAKE_MAGIC;
		wol->wolopts = 0;

		wol_en = jlsemi_wol_dynamic_op_get(phydev);

		if (wol_en)
			wol->wolopts |= WAKE_MAGIC;
	}
}

static int jlsemi_set_wol(struct phy_device *phydev,
				struct ethtool_wolinfo *wol)
{
	struct jlsemi_priv *priv = phydev->priv;
	int err;

	if (priv->wol.ethtool) {
		if (wol->wolopts & WAKE_MAGIC) {
			err = jlsemi_wol_dynamic_op_set(phydev);
			if (err < 0)
				return err;
		}
	}

	return 0;
}
#endif

static int jlsemi_config_intr(struct phy_device *phydev)
{
	struct jlsemi_priv *priv = phydev->priv;
	int err;

	if (priv->intr.enable & JLSEMI_INTR_STATIC_OP_EN) {
		err = jlsemi_intr_ack_event(phydev);
		if (err < 0)
			return err;

		err = jlsemi_intr_static_op_set(phydev);
		if (err < 0)
			return err;
	}

	return 0;
}

static int jl2xx1_config_carrier_extend(struct phy_device *phydev)
{
	struct jlsemi_priv *priv = phydev->priv;
	static u16 fw_version = 0xffff, last_status;
	u16 value, cur_status;
	u16 phy_mode;

	if ((priv->itf_mode.enable & JL2XXX_ITF_MODE_STATIC_OP_EN) &&
		(priv->itf_mode.mode != JL2XXX_PHY_SGMII_RGMII_MODE) &&
		(priv->itf_mode.mode != JL2XXX_MAC_SGMII_RGMII_MODE)) {
		return 0;
	}

	phy_mode = jl2xx1_interface_mode_get(phydev);
	if ((phy_mode != JL2XXX_PHY_SGMII_RGMII_MODE) &&
		(phy_mode != JL2XXX_MAC_SGMII_RGMII_MODE)) {
		return 0;
	}

	/* read page 0, reg 29 */
	if (fw_version == 0xffff)
		fw_version = jlsemi_read_paged(phydev, JL2XX1_PAGE_0,
						JL2XX1_PHY_INFO_REG);
	if (JL2XX1A_NE(phydev->phy_id) || (fw_version != 0x2208))
		return 0;

	/* get page 181, reg 18, bit 2 */
	value = jlsemi_read_paged(phydev, JL2XX1_PAGE_181,
				JL2XX1_BASE1000X_STATUS_REG);
	cur_status = (value >> 2) & 1;

	if (last_status == 0 && cur_status == 1) {
		/* set page 160, reg 17, bit 2, val 1 */
		jlsemi_set_bits(phydev, JL2XX1_PAGE_160,
				JL2XX1_GLOBAL_CONFIG1_REG,
				(1 << 2));
		/* set page 190, reg 23, bit 15, val 1 */
		jlsemi_set_bits(phydev, JL2XX1_PAGE_190,
				JL2XX1_PHY_RSVD6_REG,
				(1 << 15));
	}
	last_status = cur_status;

	return 0;
}

static int jlsemi_read_status(struct phy_device *phydev)
{
	struct jlsemi_priv *priv = phydev->priv;
	bool fiber_serdes;
	int err;

	if (phydev->drv->phy_id == JL2XX1_PHY_ID) {
		if ((!priv->rxc_out.inited) &&
			(priv->rxc_out.enable & JL2XXX_RXC_OUT_STATIC_OP_EN) &&
			(priv->itf_mode.mode == JL2XXX_MAC_SGMII_RGMII_MODE)) {
			err = jl2xx1_interface_mode_set(phydev, priv->itf_mode.mode);
			if (err < 0)
				return err;

			err = jlsemi_soft_reset(phydev);
			if (err < 0)
				return err;
			priv->rxc_out.inited = true;
		}
	}

	if (priv->intr.enable & JLSEMI_INTR_STATIC_OP_EN) {
		err = jlsemi_intr_ack_event(phydev);
		if (err < 0)
			return err;
	}

	err = genphy_read_status(phydev);
	if (err < 0)
		return err;

	if (phydev->drv->phy_id == JL2XX1_PHY_ID) {
		err = jl2xx1_config_carrier_extend(phydev);
		if (err < 0)
			return err;

		fiber_serdes = jl2xx1_read_fiber_serdes_status(phydev);
		if (fiber_serdes)
			return 0;
	}

	return 0;
}

#if (JLSEMI_PHY_TUNABLE)
static int jlsemi_get_tunable(struct phy_device *phydev,
				struct ethtool_tunable *tuna, void *data)
{
	struct jlsemi_priv *priv = phydev->priv;

	if (phydev->drv->phy_id == JL11X1_PHY_ID)
		return 0;

	switch (tuna->id) {
	case ETHTOOL_PHY_FAST_LINK_DOWN:
		if (priv->fld.ethtool)
			return jl2xx1_fld_dynamic_op_get(phydev, data);
		break;
	case ETHTOOL_PHY_DOWNSHIFT:
		if (priv->downshift.ethtool)
			return jl2xx1_downshift_dynamic_op_get(phydev, data);
		break;
	default:
		return -EOPNOTSUPP;
	}

	return 0;
}

static int jlsemi_set_tunable(struct phy_device *phydev,
				struct ethtool_tunable *tuna, const void *data)
{
	struct jlsemi_priv *priv = phydev->priv;

	if (phydev->drv->phy_id == JL11X1_PHY_ID)
		return 0;

	switch (tuna->id) {
	case ETHTOOL_PHY_FAST_LINK_DOWN:
		if (priv->fld.ethtool)
			return jl2xx1_fld_dynamic_op_set(phydev, data);
		break;
	case ETHTOOL_PHY_DOWNSHIFT:
		if (priv->downshift.ethtool)
			return jl2xx1_downshift_dynamic_op_set(phydev, *(const u8 *)data);
		break;
	default:
		return -EOPNOTSUPP;
	}

	return 0;
}
#endif

#if (JLSEMI_PHY_GET_STATS)
static void jlsemi_get_stats(struct phy_device *phydev,
				struct ethtool_stats *stats, u64 *data)
{
	struct jlsemi_priv *priv = phydev->priv;
	int val, i;

	if (phydev->drv->phy_id == JL11X1_PHY_ID || !priv)
		return;

	for (i = 0; i < priv->nstats; i++) {
		val = jlsemi_read_paged(phydev, priv->hw_stats[i].page,
					priv->hw_stats[i].reg);
		if (val < 0) {
			data[i] = U64_MAX;
		} else {
			val = val & priv->hw_stats[i].mask;
			priv->stats[i] += val;
			data[i] = priv->stats[i];
		}
	}
}

static void jlsemi_get_strings(struct phy_device *phydev, u8 *data)
{
	struct jlsemi_priv *priv = phydev->priv;
	int i;

	if (phydev->drv->phy_id == JL11X1_PHY_ID || !priv)
		return;

	for (i = 0; i < priv->nstats; i++)
		strlcpy(data + i * ETH_GSTRING_LEN,
			priv->hw_stats[i].string, ETH_GSTRING_LEN);
}
#endif

#if (JLSEMI_PHY_ANEG_DONE)
#define DEFINE_JLPHY_ANEG_DONE .aneg_done = jlsemi_aneg_done,
#else
#define DEFINE_JLPHY_ANEG_DONE
#endif

#if (JLSEMI_PHY_WOL)
#define DEFINE_JLPHY_WOL .get_wol = jlsemi_get_wol, .set_wol = jlsemi_set_wol,
#else
#define DEFINE_JLPHY_WOL
#endif

#if (JLSEMI_PHY_TUNABLE)
#define DEFINE_JLPHY_TUNABLE .get_tunable = jlsemi_get_tunable, .set_tunable = jlsemi_set_tunable,
#else
#define DEFINE_JLPHY_TUNABLE
#endif

#if (JLSEMI_PHY_GET_STATS)
#define DEFINE_JLPHY_GET_STATS .get_stats = jlsemi_get_stats, .get_strings = jlsemi_get_strings,
#else
#define DEFINE_JLPHY_GET_STATS
#endif

#define DEFINE_JLPHY_DRIVER(id, mask, driver_name) \
		.phy_id			= id, \
		.phy_id_mask	= mask, \
		.name			= driver_name, \
		.probe			= jlsemi_probe, \
		.read_status	= jlsemi_read_status, \
		.config_init	= jlsemi_config_init, \
		.config_intr	= jlsemi_config_intr, \
		.config_aneg	= jlsemi_config_aneg, \
		.suspend		= jlsemi_suspend, \
		.resume			= jlsemi_resume, \
		.remove			= jlsemi_remove,


static struct phy_driver jlsemi_drivers[] = {
	{
		DEFINE_JLPHY_DRIVER(JL11X1_PHY_ID, JL_PHY_ID_MATCH_MODEL, "JL1xxx Fast Ethernet")
		DEFINE_JLPHY_ANEG_DONE
		DEFINE_JLPHY_WOL
		.features	= PHY_BASIC_FEATURES,
	},
	{
		DEFINE_JLPHY_DRIVER(JL2XX1_PHY_ID, JL_PHY_ID_MATCH_VD_MOD, "JL2xxx Gigabit Ethernet")
		DEFINE_JLPHY_ANEG_DONE
		DEFINE_JLPHY_WOL
		DEFINE_JLPHY_TUNABLE
		DEFINE_JLPHY_GET_STATS
		.features	= PHY_GBIT_FEATURES,
	},
};

module_jlsemi_driver(jlsemi_drivers);

static struct mdio_device_id __maybe_unused jlsemi_tbl[] = {
	{JL11X1_PHY_ID, JL_PHY_ID_MATCH_MODEL},
	{JL2XX1_PHY_ID, JL_PHY_ID_MATCH_VD_MOD},
	{ }
};

MODULE_DEVICE_TABLE(mdio, jlsemi_tbl);

MODULE_DESCRIPTION("JLSemi PHY driver");
MODULE_AUTHOR("Gangqiao Kuang");
MODULE_LICENSE("GPL");
