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
#include <linux/clk.h>
#include <linux/reset.h>
#include <linux/device.h>
#include <linux/regmap.h>

#include "snd_sunxi_log.h"
#include "snd_sunxi_ahub_mgmt.h"

struct sunxi_ahub_clk {
	struct reset_control *clk_rst;
	struct clk *clk_bus;

	struct clk *clk_ahub;
};

sunxi_ahub_clk_t *snd_ahub_clk_init(struct platform_device *pdev)
{
	int ret = 0;
	struct device_node *np = pdev->dev.of_node;
	struct sunxi_ahub_clk *clk = NULL;

	SND_LOG_DEBUG("\n");

	clk = kzalloc(sizeof(*clk), GFP_KERNEL);
	if (!clk) {
		SND_LOG_ERR("can't allocate sunxi_ahub_clk memory\n");
		return NULL;
	}

	/* get rst clk */
	clk->clk_rst = devm_reset_control_get(&pdev->dev, NULL);
	if (IS_ERR_OR_NULL(clk->clk_rst)) {
		SND_LOG_ERR("clk rst get failed\n");
		ret =  PTR_ERR(clk->clk_rst);
		goto err_get_clk_rst;
	}

	/* get bus clk */
	clk->clk_bus = of_clk_get_by_name(np, "clk_bus_ahub");
	if (IS_ERR_OR_NULL(clk->clk_bus)) {
		SND_LOG_ERR("clk bus get failed\n");
		ret = PTR_ERR(clk->clk_bus);
		goto err_get_clk_bus;
	}

	/* get ahub clk */
	clk->clk_ahub = of_clk_get_by_name(np, "clk_ahub");
	if (IS_ERR_OR_NULL(clk->clk_ahub)) {
		SND_LOG_ERR("clk ahub get failed\n");
		ret = PTR_ERR(clk->clk_ahub);
		goto err_get_clk_ahub;
	}

	return clk;

err_get_clk_ahub:
	clk_put(clk->clk_bus);
err_get_clk_bus:
err_get_clk_rst:
	kfree(clk);
	return NULL;
}

void snd_ahub_clk_exit(void *clk_orig)
{
	struct sunxi_ahub_clk *clk = (struct sunxi_ahub_clk *)clk_orig;

	SND_LOG_DEBUG("\n");

	clk_put(clk->clk_ahub);
	clk_put(clk->clk_bus);

	kfree(clk);
}

int snd_ahub_clk_bus_enable(void *clk_orig)
{
	int ret = 0;
	struct sunxi_ahub_clk *clk = (struct sunxi_ahub_clk *)clk_orig;

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

int snd_ahub_clk_enable(void *clk_orig)
{
	int ret = 0;
	struct sunxi_ahub_clk *clk = (struct sunxi_ahub_clk *)clk_orig;

	SND_LOG_DEBUG("\n");

	if (clk_prepare_enable(clk->clk_ahub)) {
		SND_LOG_ERR("clk_ahub enable failed\n");
		ret = -EINVAL;
	}

	return ret;
}

void snd_ahub_clk_bus_disable(void *clk_orig)
{
	struct sunxi_ahub_clk *clk = (struct sunxi_ahub_clk *)clk_orig;

	SND_LOG_DEBUG("\n");

	clk_disable_unprepare(clk->clk_bus);
	reset_control_assert(clk->clk_rst);
}

void snd_ahub_clk_disable(void *clk_orig)
{
	struct sunxi_ahub_clk *clk = (struct sunxi_ahub_clk *)clk_orig;

	SND_LOG_DEBUG("\n");

	clk_disable_unprepare(clk->clk_ahub);
}

int snd_ahub_clk_rate(void *clk_orig, unsigned int freq_in, unsigned int freq_out)
{
	(void)freq_in;
	struct sunxi_ahub_clk *clk = (struct sunxi_ahub_clk *)clk_orig;

	SND_LOG_DEBUG("\n");

	if (clk_set_rate(clk->clk_ahub, freq_out)) {
		SND_LOG_ERR("set clk_ahub rate failed, rate: 100MHz\n");
		return -EINVAL;
	}

	return 0;
}
