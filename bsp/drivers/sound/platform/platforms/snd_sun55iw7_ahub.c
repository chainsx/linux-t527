/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2025 - 2028 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Allwinner's ALSA SoC Audio driver
 *
 * Copyright (c) 2025, wuhao <wuhao@allwinnertech.com>
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

#include "snd_sunxi_ahub_i2s.h"

struct sunxi_ahub_clk {
	struct clk *clk_pll_audio0;
	struct clk *clk_pll_peri1_600m;
	struct clk *clk_module;
	struct clk *clk_bus;
	struct reset_control *clk_rst;
};

sunxi_ahub_clk_t snd_ahub_clk_init(struct platform_device *pdev)
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
		ret = -EBUSY;
		goto err_get_rst_clk;
	}

	/* get bus clk */
	clk->clk_bus = of_clk_get_by_name(np, "clk_bus_ahub_apb");
	if (IS_ERR_OR_NULL(clk->clk_bus)) {
		SND_LOG_ERR("clk bus get failed\n");
		ret = -EBUSY;
		goto err_get_bus_clk;
	}

	clk->clk_pll_audio0 = of_clk_get_by_name(np, "clk_pll_audio0");
	if (IS_ERR_OR_NULL(clk->clk_pll_audio0)) {
		SND_LOG_ERR("clk_pll_audio0 get failed\n");
		ret = -EBUSY;
		goto err_get_clk_pll_audio0;
	}

	clk->clk_pll_peri1_600m = of_clk_get_by_name(np, "clk_pll_peri1_600m");
	if (IS_ERR_OR_NULL(clk->clk_pll_peri1_600m)) {
		SND_LOG_ERR("clk_pll_peri1_600m get failed\n");
		ret = -EBUSY;
		goto err_get_clk_pll_peri1_600m;
	}

	/* get module clk */
	clk->clk_module = of_clk_get_by_name(np, "clk_ahub");
	if (IS_ERR_OR_NULL(clk->clk_module)) {
		SND_LOG_ERR("clk module get failed\n");
		ret = -EBUSY;
		goto err_get_module_clk;
	}


	return (sunxi_ahub_clk_t)clk;

err_get_module_clk:
	clk_put(clk->clk_pll_peri1_600m);
err_get_clk_pll_peri1_600m:
	clk_put(clk->clk_pll_audio0);
err_get_clk_pll_audio0:
	clk_put(clk->clk_bus);
err_get_bus_clk:
err_get_rst_clk:
	kfree(clk);
	return NULL;
}

int snd_ahub_clk_bus_enable(sunxi_ahub_clk_t clk_orig)
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

void snd_ahub_clk_bus_disable(sunxi_ahub_clk_t clk_orig)
{
	struct sunxi_ahub_clk *clk = (struct sunxi_ahub_clk *)clk_orig;

	SND_LOG_DEBUG("\n");

	clk_disable_unprepare(clk->clk_bus);
	reset_control_assert(clk->clk_rst);
}

void snd_sunxi_ahub_clk_exit(sunxi_ahub_clk_t clk_orig)
{
	struct sunxi_ahub_clk *clk = (struct sunxi_ahub_clk *)clk_orig;

	SND_LOG_DEBUG("\n");

	snd_sunxi_ahub_clk_disable(clk_orig);

	clk_put(clk->clk_module);
	clk_put(clk->clk_pll_audio0);
	clk_put(clk->clk_pll_peri1_600m);
	clk_put(clk->clk_bus);
	kfree(clk);
}

int snd_sunxi_ahub_clk_enable(sunxi_ahub_clk_t clk_orig)
{
	int ret = 0;
	struct sunxi_ahub_clk *clk = (struct sunxi_ahub_clk *)clk_orig;

	SND_LOG_DEBUG("\n");

	if (reset_control_deassert(clk->clk_rst)) {
		SND_LOG_ERR("deassert reset clk failed\n");
		ret = -EBUSY;
		goto err_deassert_rst;
	}

	if (clk_prepare_enable(clk->clk_bus)) {
		SND_LOG_ERR("ahub clk bus enable failed\n");
		ret = -EBUSY;
		goto err_enable_clk_bus;
	}

	if (clk_prepare_enable(clk->clk_pll_audio0)) {
		SND_LOG_ERR("clk_pll_audio0 enable failed\n");
		ret = -EBUSY;
		goto err_enable_clk_pll_audio0;
	}


	if (clk_prepare_enable(clk->clk_pll_peri1_600m)) {
		SND_LOG_ERR("clk_pll_peri1_600m enable failed\n");
		ret = -EBUSY;
		goto err_enable_pll_peri1_600m;
	}

	if (clk_prepare_enable(clk->clk_module)) {
		SND_LOG_ERR("clk_module enable failed\n");
		ret = -EBUSY;
		goto err_enable_module_clk;
	}

	return 0;

err_enable_module_clk:
	clk_disable_unprepare(clk->clk_pll_peri1_600m);
err_enable_pll_peri1_600m:
	clk_disable_unprepare(clk->clk_pll_audio0);
err_enable_clk_pll_audio0:
	clk_disable_unprepare(clk->clk_bus);
err_enable_clk_bus:
	reset_control_assert(clk->clk_rst);
err_deassert_rst:
	return ret;
}

void snd_sunxi_ahub_clk_disable(sunxi_ahub_clk_t clk_orig)
{
	struct sunxi_ahub_clk *clk = (struct sunxi_ahub_clk *)clk_orig;

	SND_LOG_DEBUG("\n");

	clk_disable_unprepare(clk->clk_module);
	clk_disable_unprepare(clk->clk_pll_audio0);
	clk_disable_unprepare(clk->clk_pll_peri1_600m);
	clk_disable_unprepare(clk->clk_bus);
	reset_control_assert(clk->clk_rst);
}

int snd_sunxi_ahub_clk_rate(sunxi_ahub_clk_t clk_orig, unsigned int freq_in, unsigned int freq_out)
{
	struct sunxi_ahub_clk *clk = (struct sunxi_ahub_clk *)clk_orig;

	SND_LOG_DEBUG("freq_in ->%u, freq_out ->%u\n", freq_in, freq_out);

	if (!clk) {
		SND_LOG_ERR("clk is NULL\n");
		return -EINVAL;
	}

	if (clk_set_parent(clk->clk_module, clk->clk_pll_peri1_600m)) {
		SND_LOG_ERR("set parent of clk_module to clk_pll_peri1_600m failed\n");
		return -EINVAL;
	}

	if (clk_set_rate(clk->clk_module, freq_out)) {
		SND_LOG_ERR_STD(E_I2S_SWDEP_CLK_SET, "freq : 100m module clk unsupport\n");
		return -EINVAL;
	}

	return 0;
}
