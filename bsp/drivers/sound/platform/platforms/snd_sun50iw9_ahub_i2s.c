/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2025 Allwinner Technology Co.,Ltd. All rights reserved. */
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

struct sunxi_ahub_i2s_clk {
	struct clk *clk_pll;
	struct clk *clk_pllx4;
	struct clk *clk_module;
	struct clk *clk_bus;
	struct reset_control *clk_rst;
};

void *snd_ahub_i2s_clk_init(struct platform_device *pdev)
{
	int ret = 0;
	struct device_node *np = pdev->dev.of_node;
	struct sunxi_ahub_i2s_clk *clk = NULL;

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
	clk->clk_bus = of_clk_get_by_name(np, "clk_bus_audio_hub");
	if (IS_ERR_OR_NULL(clk->clk_bus)) {
		SND_LOG_ERR("clk bus get failed\n");
		ret = -EBUSY;
		goto err_get_bus_clk;
	}

	/* get parent clk */
	clk->clk_pll = of_clk_get_by_name(np, "clk_pll_audio");
	if (IS_ERR_OR_NULL(clk->clk_pll)) {
		SND_LOG_ERR("clk pll get failed\n");
		ret = -EBUSY;
		goto err_get_pll_clk;
	}
	clk->clk_pllx4 = of_clk_get_by_name(np, "clk_pll_audio_4x");
	if (IS_ERR_OR_NULL(clk->clk_pllx4)) {
		SND_LOG_ERR("clk pllx4 get failed\n");
		ret = -EBUSY;
		goto err_get_pllx4_clk;
	}

	/* get module clk */
	clk->clk_module = of_clk_get_by_name(np, "clk_audio_hub");
	if (IS_ERR_OR_NULL(clk->clk_module)) {
		SND_LOG_ERR("clk module get failed\n");
		ret = -EBUSY;
		goto err_get_module_clk;
	}

	/* set ahub clk parent */
	if (clk_set_parent(clk->clk_module, clk->clk_pllx4)) {
		SND_LOG_ERR("set parent of clk_module to pllx4 failed\n");
		ret = -EINVAL;
		goto err_set_parent;
	}

	/* enable clk of ahub */
	ret = snd_ahub_i2s_clk_enable(clk);
	if (ret) {
		SND_LOG_ERR("clk enable failed\n");
		ret = -EINVAL;
		goto err_clk_enable;
	}

	return clk;

err_clk_enable:
err_set_parent:
	clk_put(clk->clk_module);
err_get_module_clk:
	clk_put(clk->clk_pllx4);
err_get_pllx4_clk:
	clk_put(clk->clk_pll);
err_get_pll_clk:
	clk_put(clk->clk_bus);
err_get_bus_clk:
err_get_rst_clk:
	kfree(clk);
	return NULL;
}

void snd_ahub_i2s_clk_exit(void *clk_orig)
{
	struct sunxi_ahub_i2s_clk *clk = (void *)clk_orig;

	SND_LOG_DEBUG("\n");

	clk_put(clk->clk_module);
	clk_put(clk->clk_pll);
	clk_put(clk->clk_pllx4);
	clk_put(clk->clk_bus);
	kfree(clk);
}

int snd_ahub_i2s_clk_enable(void *clk_orig)
{
	int ret = 0;
	struct sunxi_ahub_i2s_clk *clk = (struct sunxi_ahub_i2s_clk *)clk_orig;

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

	if (clk_prepare_enable(clk->clk_pll)) {
		SND_LOG_ERR("clk_pll enable failed\n");
		ret = -EBUSY;
		goto err_enable_pll_clk;
	}
	if (clk_prepare_enable(clk->clk_pllx4)) {
		SND_LOG_ERR("clk_pllx4 enable failed\n");
		ret = -EBUSY;
		goto err_enable_pllx4_clk;
	}
	if (clk_prepare_enable(clk->clk_module)) {
		SND_LOG_ERR("clk_module enable failed\n");
		ret = -EBUSY;
		goto err_enable_module_clk;
	}

	return 0;

err_enable_module_clk:
	clk_disable_unprepare(clk->clk_pllx4);
err_enable_pllx4_clk:
	clk_disable_unprepare(clk->clk_pll);
err_enable_pll_clk:
	clk_disable_unprepare(clk->clk_bus);
err_enable_clk_bus:
	reset_control_assert(clk->clk_rst);
err_deassert_rst:
	return ret;
}

void snd_ahub_i2s_clk_disable(void *clk_orig)
{
	struct sunxi_ahub_i2s_clk *clk = (struct sunxi_ahub_i2s_clk *)clk_orig;

	SND_LOG_DEBUG("\n");

	clk_disable_unprepare(clk->clk_module);
	clk_disable_unprepare(clk->clk_pllx4);
	clk_disable_unprepare(clk->clk_pll);
}

int snd_ahub_i2s_clk_rate(void *clk_orig, unsigned int freq_in, unsigned int freq_out)
{
	struct sunxi_ahub_i2s_clk *clk = (struct sunxi_ahub_i2s_clk *)clk_orig;

	SND_LOG_DEBUG("\n");

	if (clk_set_parent(clk->clk_module, clk->clk_pllx4)) {
		SND_LOG_ERR_STD(E_I2S_SWDEP_CLK_SET, "set clk_module parent failed\n");
		return -EINVAL;
	}
	if (clk_set_rate(clk->clk_pllx4, freq_in)) {
		SND_LOG_ERR("freq : %u pllx4 clk unsupport\n", freq_in);
		return -EINVAL;
	}

	if (clk_set_rate(clk->clk_module, freq_out)) {
		SND_LOG_ERR_STD(E_I2S_SWDEP_CLK_SET, "freq : %u module clk unsupport\n", freq_out);
		return -EINVAL;
	}

	return 0;
}
