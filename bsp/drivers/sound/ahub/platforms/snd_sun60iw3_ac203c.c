// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright(c) 2025 - 2028 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Allwinner's ALSA SoC Audio driver
 *
 * Copyright (c) 2025, huhaoxin <huhaoxin@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/clk.h>
#include <linux/reset.h>
#include <linux/device.h>
#include <linux/regmap.h>

#include "snd_sunxi_log.h"
#include "snd_sunxi_ahub_ac203c.h"

struct sunxi_ac203c_clk {
	struct reset_control *clk_rst;
	struct clk *clk_bus;

	struct clk *clk_pll_audio0;
	struct clk *clk_pll_audio1_5x;
	struct clk *clk_pll_cur;

	struct clk *clk_ac203c;
};

sunxi_ac203c_clk_t *snd_ac203c_clk_init(struct platform_device *pdev)
{
	int ret = 0;
	struct device_node *np = pdev->dev.of_node;
	struct device_node *clk_np = of_parse_phandle(np, "clocks-node", 0);
	struct platform_device *clk_pdev = of_find_device_by_node(clk_np);
	struct sunxi_ac203c_clk *clk = NULL;

	SND_LOG_DEBUG("\n");

	clk = kzalloc(sizeof(*clk), GFP_KERNEL);
	if (!clk) {
		SND_LOG_ERR("can't allocate sunxi_ac203c_clk memory\n");
		return NULL;
	}

	/* get rst clk */
	clk->clk_rst = devm_reset_control_get(&clk_pdev->dev, NULL);
	if (IS_ERR_OR_NULL(clk->clk_rst)) {
		SND_LOG_ERR("clk rst get failed\n");
		ret =  PTR_ERR(clk->clk_rst);
		goto err_get_clk_rst;
	}

	/* get bus clk */
	clk->clk_bus = of_clk_get_by_name(clk_np, "clk_bus_ac203c");
	if (IS_ERR_OR_NULL(clk->clk_bus)) {
		SND_LOG_ERR("clk bus get failed\n");
		ret = PTR_ERR(clk->clk_bus);
		goto err_get_clk_bus;
	}

	/* get parent clk */
	clk->clk_pll_audio0 = of_clk_get_by_name(clk_np, "clk_pll_audio0");
	if (IS_ERR_OR_NULL(clk->clk_pll_audio0)) {
		SND_LOG_ERR("clk_pll_audio0 get failed\n");
		ret = PTR_ERR(clk->clk_pll_audio0);
		goto err_get_clk_pll_audio0;
	}
	clk->clk_pll_audio1_5x = of_clk_get_by_name(clk_np, "clk_pll_audio1_5x");
	if (IS_ERR_OR_NULL(clk->clk_pll_audio1_5x)) {
		SND_LOG_ERR("clk_pll_audio1_5x get failed\n");
		ret = PTR_ERR(clk->clk_pll_audio1_5x);
		goto err_get_clk_pll_audio1_5x;
	}

	/* get ac203c clk */
	clk->clk_ac203c = of_clk_get_by_name(clk_np, "clk_ac203c");
	if (IS_ERR_OR_NULL(clk->clk_ac203c)) {
		SND_LOG_ERR("clk ac203c get failed\n");
		ret = PTR_ERR(clk->clk_ac203c);
		goto err_get_clk_ac203c;
	}

	return clk;

err_get_clk_ac203c:
	clk_put(clk->clk_pll_audio1_5x);
err_get_clk_pll_audio1_5x:
	clk_put(clk->clk_pll_audio0);
err_get_clk_pll_audio0:
	clk_put(clk->clk_bus);
err_get_clk_bus:
err_get_clk_rst:
	kfree(clk);
	return NULL;
}

void snd_ac203c_clk_exit(void *clk_orig)
{
	struct sunxi_ac203c_clk *clk = (struct sunxi_ac203c_clk *)clk_orig;

	SND_LOG_DEBUG("\n");

	clk_put(clk->clk_ac203c);
	clk_put(clk->clk_pll_audio1_5x);
	clk_put(clk->clk_pll_audio0);
	clk_put(clk->clk_bus);

	kfree(clk);
}

int snd_ac203c_clk_bus_enable(void *clk_orig)
{
	int ret = 0;
	struct sunxi_ac203c_clk *clk = (struct sunxi_ac203c_clk *)clk_orig;

	SND_LOG_DEBUG("\n");

	if (reset_control_deassert(clk->clk_rst)) {
		SND_LOG_ERR("clk_rst deassert failed\n");
		ret = -EINVAL;
		goto err_deassert_rst;
	}

	if (clk_prepare_enable(clk->clk_bus)) {
		SND_LOG_ERR("clk_bus enable failed\n");
		ret = -EINVAL;
		goto err_enable_clk_bus;
	}

	return 0;

err_enable_clk_bus:
	reset_control_assert(clk->clk_rst);
err_deassert_rst:
	return ret;
}

int snd_ac203c_clk_enable(void *clk_orig)
{
	int ret = 0;
	struct sunxi_ac203c_clk *clk = (struct sunxi_ac203c_clk *)clk_orig;

	SND_LOG_DEBUG("\n");

	if (clk_prepare_enable(clk->clk_pll_cur)) {
		SND_LOG_ERR("clk_pll_cur enable failed\n");
		ret = -EINVAL;
		goto err_enable_clk_pll_cur;
	}

	if (clk_prepare_enable(clk->clk_ac203c)) {
		SND_LOG_ERR("clk_ac203c enable failed\n");
		ret = -EINVAL;
		goto err_enable_clk_ac203c;
	}

	return 0;

err_enable_clk_ac203c:
	clk_disable_unprepare(clk->clk_pll_cur);
err_enable_clk_pll_cur:
	return ret;
}

void snd_ac203c_clk_bus_disable(void *clk_orig)
{
	struct sunxi_ac203c_clk *clk = (struct sunxi_ac203c_clk *)clk_orig;

	SND_LOG_DEBUG("\n");

	clk_disable_unprepare(clk->clk_bus);
	reset_control_assert(clk->clk_rst);
}

void snd_ac203c_clk_disable(void *clk_orig)
{
	struct sunxi_ac203c_clk *clk = (struct sunxi_ac203c_clk *)clk_orig;

	SND_LOG_DEBUG("\n");

	clk_disable_unprepare(clk->clk_ac203c);
	clk_disable_unprepare(clk->clk_pll_cur);
}

int snd_ac203c_clk_rate(void *clk_orig, unsigned int freq_in, unsigned int freq_out)
{
	struct sunxi_ac203c_clk *clk = (struct sunxi_ac203c_clk *)clk_orig;

	SND_LOG_DEBUG("\n");

	if (freq_in % 24576000 == 0) {
		if (clk_set_parent(clk->clk_ac203c, clk->clk_pll_audio1_5x)) {
			SND_LOG_ERR("set ac203c parent clk failed\n");
			return -EINVAL;
		}
		clk->clk_pll_cur = clk->clk_pll_audio1_5x;
	} else {
		if (clk_set_parent(clk->clk_ac203c, clk->clk_pll_audio0)) {
			SND_LOG_ERR("set ac203c parent clk failed\n");
			return -EINVAL;
		}
		clk->clk_pll_cur = clk->clk_pll_audio0;
	}

	if (clk_set_rate(clk->clk_ac203c, freq_out)) {
		SND_LOG_ERR("freq : %u module clk unsupport\n", freq_out);
		return -EINVAL;
	}

	return 0;
}
