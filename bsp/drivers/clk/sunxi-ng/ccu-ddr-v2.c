/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Allwinner DDR Clock driver.
 *
 * Copyright (C) 2020 Allwinner Technology, Inc.
 *	fanqinghua <fanqinghua@allwinnertech.com>
 *
 * Implementation of ddr clock source driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <sunxi-log.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/clk-provider.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/regmap.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <sunxi-sip.h>

#define MDFS_MC_DFS_CONFIG				(0x000)
#define MDFS_MC_DFS_MASTER_MASK			(0x004)
#define MDFS_MC_DFS_CMD(x)				(0x800 + (x) * 0x10)
#define MDFS_MC_DFS_WDATA_RDATA(x)		(0x804 + (x) * 0x10)
#define MDFS_MC_DFS_DATA_MASK(x)		(0x808 + (x) * 0x10)
#define MDFS_MC_DFS_STATE_RB			(0x1c)
#define MDFS_MC_DFS_CMD_DLY(x)			(0x80c + x*0x10)
#define MDFS_MX_DFIMISC					(0x0a0201b0)
#define MDFS_MP_CLK_CTRL				(0x0a03001c)
#define ACC_PHY							(0x1 <<18)
#define CMD_WR							(0x1 <<16)

#define DRIVER_NAME	"DDR-Clock-Driver"

struct sunxi_ddrclk_plat_data {
	unsigned int dram_clk_ctrl;
	/* the freq factor about ddrc and memory die, 1:2 or 1:4*/
	unsigned int factor;
};

/*
 * PLL_DDR / (DIV + 1) = DDR_CLK;
 * DDR_PHY_CLK = DDR_CLK * 2;
 * DDR_PHY_IO_CLK = DDR_CLK * 4;
 * So (DIV + 1) = PLL_DDR / (DDR_PHY_CLK / 2);
 * So (DIV + 1) = (PLL_DDR / DDR_PHY_CLK) * 2;
 * The freq in opp table is DDR_PHY_CLK.
 */

struct sunxi_ddrclk {
	struct device *dev;
	void __iomem	*dramphy_base;
	void __iomem	*dfs_base;
	unsigned int	dram_clk;
	unsigned int	dram_div;
	unsigned long	freq[4];
	struct clk_hw	hw;
	struct mutex  ddrfreq_lock;
	struct clk *ddrpll0_clk;
	struct clk *ddrpll1_clk;
	struct clk *ddrpll2_clk;
	unsigned long ddrpll[4];
	const struct sunxi_ddrclk_plat_data *plat_data;
	spinlock_t      lock;
};

#define to_sunxi_ddrclk_hw(_hw) container_of(_hw, struct sunxi_ddrclk, hw)

static int dbg_enable;
module_param_named(dbg_level, dbg_enable, int, 0644);

#define DBG(args...) \
	do { \
		if (dbg_enable) { \
			sunxi_info(NULL, args); \
		} \
	} while (0)

static inline void mdfs_conf(
				struct sunxi_ddrclk *ddrclk,
				unsigned int reg,
				unsigned int flag,
				unsigned int data,
				unsigned int dmask,
				unsigned int id,
				unsigned int delay)
{
	writel((reg&0xFFFF) | flag, ddrclk->dfs_base + MDFS_MC_DFS_CMD(id));
	writel(data, ddrclk->dfs_base + MDFS_MC_DFS_WDATA_RDATA(id));
	writel(dmask, ddrclk->dfs_base + MDFS_MC_DFS_DATA_MASK(id));
	writel(delay, ddrclk->dfs_base + MDFS_MC_DFS_CMD_DLY(id));
}

static int set_ddrfreq(struct sunxi_ddrclk *ddrclk, unsigned int freq_id)
{
	unsigned int dram_div = ddrclk->dram_div;
	unsigned int reg_val, pll = 0, id0 = 0;
	unsigned int master_id = dram_div >> 29;

	pll = ((dram_div >> (8 * freq_id + 3)) & 0x3);
	id0 = ((dram_div >> 8 * freq_id) & 0x7);

	mdfs_conf(ddrclk, MDFS_MX_DFIMISC, CMD_WR, freq_id << 8 | 0x0, 0xFFFFE0FE, 0xe, 0);
	mdfs_conf(ddrclk, MDFS_MP_CLK_CTRL, ACC_PHY | CMD_WR, (id0 << 0), 0xFFFFFFF8, 0x15, 0);
	mdfs_conf(ddrclk, MDFS_MP_CLK_CTRL, ACC_PHY | CMD_WR, (pll << 8), 0xFFFFFCFF, 0x16, 0);

	if (master_id) {
		reg_val = readl(ddrclk->dfs_base + MDFS_MC_DFS_MASTER_MASK);
		reg_val &= ~(0x1 << master_id);
		writel(reg_val, ddrclk->dfs_base + MDFS_MC_DFS_MASTER_MASK);
	}

	reg_val = readl(ddrclk->dfs_base + MDFS_MC_DFS_CONFIG);
	reg_val |= (0x1 << 8);
	reg_val &= ~(0x1 << 2);
	writel(reg_val, ddrclk->dfs_base + MDFS_MC_DFS_CONFIG);

	while (readl(ddrclk->dfs_base + MDFS_MC_DFS_STATE_RB) >> 31 != 0x1)
		;
	while ((readl(ddrclk->dfs_base + MDFS_MC_DFS_STATE_RB) & 0xf) != 0)
		;

	reg_val = readl(ddrclk->dfs_base + MDFS_MC_DFS_CONFIG);
	reg_val |= (0x1 << 0);
	writel(reg_val, ddrclk->dfs_base + MDFS_MC_DFS_CONFIG);

	while (readl(ddrclk->dfs_base + MDFS_MC_DFS_STATE_RB) >> 31 != 0x1)
		;
	while ((readl(ddrclk->dfs_base + MDFS_MC_DFS_STATE_RB) & 0xf) != 0)
		;
	DBG("DFS finish freq_id is %d \n", freq_id);

	return 0;
}

static unsigned long sunxi_ddr_clk_recalc_rate(struct clk_hw *hw,
					       unsigned long parent_rate)
{
	struct sunxi_ddrclk *ddrclk = to_sunxi_ddrclk_hw(hw);
	const struct sunxi_ddrclk_plat_data *plat_data = ddrclk->plat_data;
	unsigned long rate;
	unsigned int reg_val;

	reg_val = readl(ddrclk->dramphy_base + plat_data->dram_clk_ctrl);
	rate = (ddrclk->ddrpll[(reg_val >> 8) & 0x3] >> 1) / ((reg_val & 0x7) + 1);

	return rate;
}

static long sunxi_ddr_clk_round_rate(struct clk_hw *hw,
				     unsigned long target_rate,
				     unsigned long *prate)
{
	struct sunxi_ddrclk *ddrclk = to_sunxi_ddrclk_hw(hw);

	if (target_rate <= ddrclk->freq[0])
		return ddrclk->freq[0];
	else if (target_rate <= ddrclk->freq[1])
		return ddrclk->freq[1];
	else if (target_rate <= ddrclk->freq[2])
		return ddrclk->freq[2];
	else
		return ddrclk->freq[3];
}

static int sunxi_ddr_clk_set_rate(struct clk_hw *hw, unsigned long drate,
				  unsigned long prate)
{
	struct sunxi_ddrclk *ddrclk = to_sunxi_ddrclk_hw(hw);
	unsigned int freq_id;

	if (drate <= ddrclk->freq[0])
		freq_id = 3;
	else if (drate <= ddrclk->freq[1])
		freq_id = 2;
	else if (drate <= ddrclk->freq[2])
		freq_id = 1;
	else
		freq_id = 0;

	DBG("drate:%ldM\n", drate / 1000000);
	mutex_lock(&ddrclk->ddrfreq_lock);
	set_ddrfreq(ddrclk, freq_id);
	ddrclk->dram_clk = drate;
	mutex_unlock(&ddrclk->ddrfreq_lock);

	return 0;
}

const struct clk_ops sunxi_ddrclk_ops = {
	.recalc_rate = sunxi_ddr_clk_recalc_rate,
	.round_rate = sunxi_ddr_clk_round_rate,
	.set_rate = sunxi_ddr_clk_set_rate,
};

static const struct sunxi_ddrclk_plat_data ddrclk_sun65iw1_data = {
	.dram_clk_ctrl = 0x1c,
	.factor = 1,
};

static const struct of_device_id clk_ddr_of_match[] = {
	{ .compatible = "allwinner,sun65iw1_clock_ddr", .data = &ddrclk_sun65iw1_data},
	{ },
};
MODULE_DEVICE_TABLE(of, clk_ddr_of_match);

static int ddr_clock_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = pdev->dev.of_node;
	struct device_node *dram_np;
	struct sunxi_ddrclk *ddrclk;
	struct clk_init_data init;
	struct clk *clk, *parent_clk;
	const char *parent_name;
	unsigned int sdiv;
	int ret = 0;

	if (!np) {
		sunxi_err(&pdev->dev, "failed to match ddr clock\n");
		return -ENODEV;
	}

	ddrclk = devm_kzalloc(&pdev->dev, sizeof(*ddrclk), GFP_KERNEL);
	if (!ddrclk)
		return -ENOMEM;

	ddrclk->dev = &pdev->dev;
	platform_set_drvdata(pdev, ddrclk);

	mutex_init(&ddrclk->ddrfreq_lock);
	dram_np = of_find_node_by_path("/dram");
	if (!dram_np) {
		sunxi_err(&pdev->dev, "failed to find dram node\n");
		return -ENODEV;
	}

	ret = of_property_read_u32(dram_np, "dram_para[00]", &ddrclk->dram_clk);
	if (ret) {
		ret = of_property_read_u32(dram_np, "dram_para00", &ddrclk->dram_clk);
		if (ret) {
			sunxi_err(&pdev->dev, "failed to find dram_clk\n");
			return -ENODEV;
		}
	}

	sunxi_err(NULL, "dram_clk:%d\n", ddrclk->dram_clk);

	ret = of_property_read_u32(dram_np, "dram_para[24]", &ddrclk->dram_div);
	if (ret) {
		ret = of_property_read_u32(dram_np, "dram_para24", &ddrclk->dram_div);
		if (ret) {
			sunxi_err(&pdev->dev, "failed to find dram_div\n");
			return -ENODEV;
		}
	}
	sunxi_err(NULL, "dram_div:0x%x\n", ddrclk->dram_div);

	ddrclk->dramphy_base = of_iomap(np, 0);
	if (!ddrclk->dramphy_base) {
		sunxi_err(&pdev->dev, "map dramphy failed\n");
		return -ENODEV;
	}

	ddrclk->dfs_base = of_iomap(np, 1);
	if (!ddrclk->dfs_base) {
		sunxi_err(&pdev->dev, "map dfs failed\n");
		return -ENODEV;
	}

	ddrclk->ddrpll0_clk = devm_clk_get(dev, "ddrpll0");
	if (IS_ERR(ddrclk->ddrpll0_clk)) {
		sunxi_err(&pdev->dev, "ddrpll0 get error!\n");
		return PTR_ERR(ddrclk->ddrpll0_clk);
	}
	ddrclk->ddrpll[0] = clk_get_rate(ddrclk->ddrpll0_clk);

	ddrclk->ddrpll1_clk = devm_clk_get(dev, "ddrpll1");
	if (IS_ERR(ddrclk->ddrpll1_clk)) {
		sunxi_err(&pdev->dev, "ddrpll1 get error!\n");
		return PTR_ERR(ddrclk->ddrpll1_clk);
	}
	ddrclk->ddrpll[1] = clk_get_rate(ddrclk->ddrpll1_clk);

	ddrclk->ddrpll2_clk = devm_clk_get(dev, "ddrpll2");
	if (IS_ERR(ddrclk->ddrpll2_clk)) {
		sunxi_err(&pdev->dev, "ddrpll2 get error!\n");
		return PTR_ERR(ddrclk->ddrpll2_clk);
	}
	ddrclk->ddrpll[2] = clk_get_rate(ddrclk->ddrpll2_clk);
	ddrclk->ddrpll[3] = ddrclk->ddrpll[2];

	sdiv = ddrclk->dram_div >> 24;
	ddrclk->freq[0] = (ddrclk->ddrpll[(sdiv >> 3) & 0x3] >> 1) / ((sdiv & 0x7) + 1);
	sdiv = ddrclk->dram_div >> 16;
	ddrclk->freq[1] = (ddrclk->ddrpll[(sdiv >> 3) & 0x3] >> 1) / ((sdiv & 0x7) + 1);
	sdiv = ddrclk->dram_div >> 8;
	ddrclk->freq[2] = (ddrclk->ddrpll[(sdiv >> 3) & 0x3] >> 1) / ((sdiv & 0x7) + 1);
	sdiv = ddrclk->dram_div >> 0;
	ddrclk->freq[3] = (ddrclk->ddrpll[(sdiv >> 3) & 0x3] >> 1) / ((sdiv & 0x7) + 1);

	parent_clk = devm_clk_get(&pdev->dev, "dfi_clk");
	if (IS_ERR(parent_clk)) {
		sunxi_err(&pdev->dev, "clk_get dfi_clk failed\n");
		ret = -ENODEV;
		goto out;
	}

	parent_name = __clk_get_name(parent_clk);
	if (!parent_name) {
		sunxi_err(&pdev->dev, "get clk name failed\n");
		ret = -ENODEV;
		goto out;
	}

	ddrclk->hw.init = &init;
	init.name = "sdram";
	init.ops = &sunxi_ddrclk_ops;
	init.parent_names = &parent_name;
	init.num_parents = 1;
	init.flags = CLK_SET_RATE_NO_REPARENT | CLK_GET_RATE_NOCACHE;

	ddrclk->plat_data = of_device_get_match_data(dev);
	clk = devm_clk_register(&pdev->dev, &ddrclk->hw);
	if (IS_ERR(clk)) {
		sunxi_err(&pdev->dev, "clk_register failed\n");
		ret = -ENODEV;
		goto out;
	}

	of_clk_add_provider(np, of_clk_src_simple_get, clk);
	return 0;

out:
	return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 11, 0)
static void ddr_clock_remove(struct platform_device *pdev)
#else
static int ddr_clock_remove(struct platform_device *pdev)
#endif
{
	struct device_node *np = pdev->dev.of_node;

	of_clk_del_provider(np);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 11, 0)
#else
	return 0;
#endif
}

static struct platform_driver ddr_clock_driver = {
	.probe   = ddr_clock_probe,
	.remove  = ddr_clock_remove,
	.driver  = {
		.name  = "sunxi-ddrclock",
		.of_match_table = clk_ddr_of_match,
	},
};

module_platform_driver(ddr_clock_driver);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Allwinner DDR Clock driver");
MODULE_ALIAS("platform:" DRIVER_NAME);
MODULE_AUTHOR("fanqinghua <fanqinghua@allwinnertech.com>");
MODULE_VERSION("1.0.4");
