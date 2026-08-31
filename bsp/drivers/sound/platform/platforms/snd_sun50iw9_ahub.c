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

sunxi_ahub_clk_t snd_ahub_clk_init(struct platform_device *pdev)
{
	(void)pdev;

	SND_LOG_DEBUG("\n");

	return (void *)1;
}

void snd_sunxi_ahub_clk_exit(sunxi_ahub_clk_t clk_orig)
{
	(void)clk_orig;

	SND_LOG_DEBUG("\n");
}

int snd_sunxi_ahub_clk_enable(sunxi_ahub_clk_t clk_orig)
{
	(void)clk_orig;

	SND_LOG_DEBUG("\n");

	return 0;
}

void snd_sunxi_ahub_clk_disable(sunxi_ahub_clk_t clk_orig)
{
	(void)clk_orig;

	SND_LOG_DEBUG("\n");
}

int snd_sunxi_ahub_clk_rate(sunxi_ahub_clk_t clk_orig, unsigned int freq_in, unsigned int freq_out)
{
	(void)clk_orig;
	(void)freq_in;
	(void)freq_out;

	SND_LOG_DEBUG("\n");

	return 0;
}
