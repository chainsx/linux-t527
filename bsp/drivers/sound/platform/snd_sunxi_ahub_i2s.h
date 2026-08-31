/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Allwinner's ALSA SoC Audio driver
 *
 * Copyright (c) 2022, Dby <dby@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#ifndef __SND_SUNXI_AHUB_I2S_H
#define __SND_SUNXI_AHUB_I2S_H

#include "snd_sunxi_pcm.h"
#include "snd_sunxi_ahub_dam.h"

typedef void sunxi_ahub_i2s_clk_t;

struct sunxi_ahub_pinctl {
	struct pinctrl *pinctrl;
	struct pinctrl_state *pinstate;
	struct pinctrl_state *pinstate_sleep;

	bool pinctrl_used;
};

struct sunxi_ahub_dts {
	unsigned int dai_type;
	unsigned int apb_num;
	unsigned int tdm_num;
	unsigned int tx_pin;
	unsigned int rx_pin;

	/* value must be (2^n)Kbyte */
	size_t playback_cma;
	size_t playback_fifo_size;
	size_t capture_cma;
	size_t capture_fifo_size;
};

struct sunxi_ahub_rglt {
	struct regulator *ahub_rglt;
	const char *rglt_name;
};

struct sunxi_ahub_i2s_clk_sta {
	struct mutex ahub_clk_mutex;
	u32 p_work;
	u32 c_work;
};

struct sunxi_ahub {
	struct device *dev;

	struct sunxi_ahub_mem mem;
	sunxi_ahub_clk_t ahub_clk;
	sunxi_ahub_i2s_clk_t *i2s_clk;
	struct sunxi_ahub_i2s_clk_sta ahub_i2s_clk_sta;
	struct sunxi_ahub_pinctl pin;
	struct sunxi_ahub_dts dts;
	struct sunxi_ahub_rglt rglt;

	struct sunxi_dma_params playback_dma_param;
	struct sunxi_dma_params capture_dma_param;

	/* for Hardware param setting */
	unsigned int fmt;
	unsigned int pllclk_freq;
	unsigned int moduleclk_freq;
	unsigned int mclk_freq;
	unsigned int lrck_freq;
	unsigned int bclk_freq;
	unsigned int data_late;
	bool tx_lsb_first;
	bool rx_lsb_first;
};

sunxi_ahub_i2s_clk_t *snd_ahub_i2s_clk_init(struct platform_device *pdev);
void snd_ahub_i2s_clk_exit(void *clk_orig);
int snd_ahub_i2s_clk_enable(void *clk_orig);
void snd_ahub_i2s_clk_disable(void *clk_orig);
int snd_ahub_i2s_clk_rate(void *clk_orig, unsigned int freq_in, unsigned int freq_out);

#endif /* __SND_SUNXI_AHUB_I2S_H */
