/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2025 - 2028 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Allwinner's ALSA SoC Audio driver
 *
 * Copyright (c) 2025, zhouxijing <zhouxijing@allwinnertech.com>
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
#include "snd_sunxi_ac203c.h"

struct sunxi_ac203c_clk {
	/* parent */
	struct clk *clk_pll_audio0;
	struct clk *clk_pll_audio1_5x;
	/* module */
	struct clk *clk_adda_dac;
	/* bus & reset */
	struct clk *clk_bus;
	struct reset_control *clk_rst;
	/* record current clk */
	struct clk *clk_pll_play;
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
		SND_LOG_ERR_STD(E_AUDIOCODEC_SWDEP_CLK_INIT, "clk rst get failed\n");
		ret =  PTR_ERR(clk->clk_rst);
		goto err_get_clk_rst;
	}

	/* get bus clk */
	clk->clk_bus = of_clk_get_by_name(clk_np, "clk_bus_adda");
	if (IS_ERR_OR_NULL(clk->clk_bus)) {
		SND_LOG_ERR_STD(E_AUDIOCODEC_SWDEP_CLK_INIT, "clk bus get failed\n");
		ret = PTR_ERR(clk->clk_bus);
		goto err_get_clk_bus;
	}

	/* get parent clk */
	clk->clk_pll_audio0 = of_clk_get_by_name(clk_np, "clk_pll_audio0");
	if (IS_ERR_OR_NULL(clk->clk_pll_audio0)) {
		SND_LOG_ERR_STD(E_AUDIOCODEC_SWDEP_CLK_INIT, "clk_pll_audio0 get failed\n");
		ret = PTR_ERR(clk->clk_pll_audio0);
		goto err_get_clk_pll_audio0;
	}

	clk->clk_pll_audio1_5x = of_clk_get_by_name(clk_np, "clk_pll_audio1_5x");
	if (IS_ERR_OR_NULL(clk->clk_pll_audio1_5x)) {
		SND_LOG_ERR_STD(E_AUDIOCODEC_SWDEP_CLK_INIT, "clk_pll_audio1_5x get failed\n");
		ret = PTR_ERR(clk->clk_pll_audio1_5x);
		goto err_get_clk_pll_audio1_5x;
	}

	/* get module clk */
	clk->clk_adda_dac = of_clk_get_by_name(clk_np, "clk_adda_dac");
	if (IS_ERR_OR_NULL(clk->clk_adda_dac)) {
		SND_LOG_ERR_STD(E_AUDIOCODEC_SWDEP_CLK_INIT, "clk_adda_dac get failed\n");
		ret = PTR_ERR(clk->clk_adda_dac);
		goto err_get_clk_adda_dac;
	}

	return clk;

err_get_clk_adda_dac:
	clk_put(clk->clk_pll_audio1_5x);
err_get_clk_pll_audio1_5x:
	clk_put(clk->clk_pll_audio0);
err_get_clk_pll_audio0:
	clk_put(clk->clk_bus);
err_get_clk_bus:
err_get_clk_rst:
	return NULL;
}

void snd_ac203c_clk_exit(void *clk_orig)
{
	struct sunxi_ac203c_clk *clk = (struct sunxi_ac203c_clk *)clk_orig;

	SND_LOG_DEBUG("\n");

	clk_put(clk->clk_adda_dac);
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

	/* to avoid register modification before module load */
	reset_control_assert(clk->clk_rst);
	if (reset_control_deassert(clk->clk_rst)) {
		SND_LOG_ERR_STD(E_AUDIOCODEC_SWDEP_CLK_EN, "clk_rst deassert failed\n");
		ret = -EINVAL;
		goto err_deassert_rst;
	}

	if (clk_prepare_enable(clk->clk_bus)) {
		SND_LOG_ERR_STD(E_AUDIOCODEC_SWDEP_CLK_EN, "clk_bus enable failed\n");
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
	struct sunxi_ac203c_clk *clk = (struct sunxi_ac203c_clk *)clk_orig;

	SND_LOG_DEBUG("\n");

	if (clk_prepare_enable(clk->clk_pll_play)) {
		SND_LOG_ERR_STD(E_AUDIOCODEC_SWDEP_CLK_EN, "clk_pll_play enable failed\n");
		return -EINVAL;
	}

	if (clk_prepare_enable(clk->clk_adda_dac)) {
		SND_LOG_ERR_STD(E_AUDIOCODEC_SWDEP_CLK_EN, "clk_adda_dac enable failed\n");
		clk_disable_unprepare(clk->clk_pll_play);
		return -EINVAL;
	}

	return 0;
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

	clk_disable_unprepare(clk->clk_adda_dac);
	clk_disable_unprepare(clk->clk_pll_play);
}

int snd_ac203c_clk_rate(void *clk_orig, unsigned int freq_in, unsigned int freq_out)
{
	struct sunxi_ac203c_clk *clk = (struct sunxi_ac203c_clk *)clk_orig;

	SND_LOG_DEBUG("\n");

	if (freq_in % 24576000 == 0) {
		if (clk_set_parent(clk->clk_adda_dac, clk->clk_pll_audio1_5x)) {
			SND_LOG_ERR_STD(E_AUDIOCODEC_SWDEP_CLK_SET,
					"set dac parent clk failed\n");
			return -EINVAL;
		}
		clk->clk_pll_play = clk->clk_pll_audio1_5x;
	} else {
		if (clk_set_parent(clk->clk_adda_dac, clk->clk_pll_audio0)) {
			SND_LOG_ERR_STD(E_AUDIOCODEC_SWDEP_CLK_SET,
					"set dac parent clk failed\n");
			return -EINVAL;
		}
		clk->clk_pll_play = clk->clk_pll_audio0;
	}
	if (clk_set_rate(clk->clk_adda_dac, freq_out * 4)) {
		SND_LOG_ERR_STD(E_AUDIOCODEC_SWDEP_CLK_SET,
				"set clk_adda_dac rate failed, rate: %u\n", freq_out);
		return -EINVAL;
	}

	return 0;
}
