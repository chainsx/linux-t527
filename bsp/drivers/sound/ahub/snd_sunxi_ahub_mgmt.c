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

#define SUNXI_MODNAME		"sound-ahub-mgmt"
#include <linux/module.h>
#include <sound/soc.h>
#include <linux/of.h>
#include <linux/device.h>
#include <linux/ioport.h>
#include <linux/regmap.h>
#include <linux/of_address.h>
#include <sound/soc.h>

#include "snd_sunxi_ahub_mgmt.h"
#include "snd_sunxi_log.h"

#define DRV_NAME	"sunxi-snd-ahub-mgmt"

struct sunxi_ahub g_ahub;

/* Provide DMA-related interfaces for the port */
void snd_sunxi_set_apbif_ch(enum SUNXI_AHUB_PORT port, unsigned int ch)
{
	struct regmap *regmap = g_ahub.mem.regmap;

	SND_LOG_DEBUG("\n");

	if (port <= 6)
		regmap_update_bits(regmap, SUNXI_AHUB_APBIF_TX_CTL(port),
				   0xF << TX_CHAN_NUM, (ch - 1) << TX_CHAN_NUM);
	else
		regmap_update_bits(regmap, SUNXI_AHUB_APBIF_RX_CTL(port - 7),
				   0xF << RX_CHAN_NUM, (ch - 1) << RX_CHAN_NUM);
}
EXPORT_SYMBOL_GPL(snd_sunxi_set_apbif_ch);

void snd_sunxi_set_apbif_sw(enum SUNXI_AHUB_PORT port, unsigned int sw)
{
	struct regmap *regmap = g_ahub.mem.regmap;

	SND_LOG_DEBUG("\n");

	if (port <= 6)
		regmap_update_bits(regmap, SUNXI_AHUB_APBIF_TX_CTL(port),
				   0x7 << TX_SW, ((sw >> 2) - 1) << TX_SW);
	else
		regmap_update_bits(regmap, SUNXI_AHUB_APBIF_RX_CTL(port - 7),
				   0x7 << RX_SW, ((sw >> 2) - 1) << RX_SW);
}
EXPORT_SYMBOL_GPL(snd_sunxi_set_apbif_sw);

void snd_sunxi_set_apbif_fifo_mode(enum SUNXI_AHUB_PORT port, unsigned int mode)
{
	struct regmap *regmap = g_ahub.mem.regmap;

	SND_LOG_DEBUG("\n");

	if (port <= 6)
		regmap_update_bits(regmap, SUNXI_AHUB_APBIF_TX_FIFO_CTL(port),
				   0x1 << TX_FIFO_IM, mode << TX_FIFO_IM);
	else
		regmap_update_bits(regmap, SUNXI_AHUB_APBIF_RX_FIFO_CTL(port - 7),
				   0x1 << RX_FIFO_OM, mode << RX_FIFO_OM);
}
EXPORT_SYMBOL_GPL(snd_sunxi_set_apbif_fifo_mode);

void snd_sunxi_apbif_ctl(enum SUNXI_AHUB_PORT port, bool en)
{
	struct regmap *regmap = g_ahub.mem.regmap;

	SND_LOG_DEBUG("\n");

	if (port <= 6)
		regmap_update_bits(regmap, SUNXI_AHUB_APBIF_TX_CTL(port),
				   0x1 << TX_EN, en << TX_EN);
	else
		regmap_update_bits(regmap, SUNXI_AHUB_APBIF_RX_CTL(port - 7),
				   0x1 << RX_EN, en << RX_EN);
}
EXPORT_SYMBOL_GPL(snd_sunxi_apbif_ctl);

void snd_sunxi_flush_fifo(enum SUNXI_AHUB_PORT port)
{
	struct regmap *regmap = g_ahub.mem.regmap;

	SND_LOG_DEBUG("\n");

	if (port <= 6) {
		/* set cnt = 0 */
		regmap_write(regmap, SUNXI_AHUB_APBIF_TX_FIFO_CNT_INIT(port), 0);
		regmap_update_bits(regmap, SUNXI_AHUB_APBIF_TX_FIFO_CTL(port),
				   0x1 << TX_FLUSH_FIFO, 0x1 << TX_FLUSH_FIFO);
	} else {
		regmap_write(regmap, SUNXI_AHUB_APBIF_RX_FIFO_CNT_INIT(port - 7), 0);
		regmap_update_bits(regmap, SUNXI_AHUB_APBIF_RX_FIFO_CTL(port - 7),
				   0x1 << RX_FLUSH_FIFO, 0x1 << RX_FLUSH_FIFO);
	}
}
EXPORT_SYMBOL_GPL(snd_sunxi_flush_fifo);

void snd_sunxi_drq_ctl(enum SUNXI_AHUB_PORT port, bool en)
{
	struct regmap *regmap = g_ahub.mem.regmap;

	SND_LOG_DEBUG("\n");

	if (port <= 6)
		regmap_update_bits(regmap, SUNXI_AHUB_APBIF_TX_FIFO_CTL(port),
				   0x1 << TX_FIFO_DRQ_EN, en << TX_FIFO_DRQ_EN);
	else
		regmap_update_bits(regmap, SUNXI_AHUB_APBIF_RX_FIFO_CTL(port - 7),
				   0x1 << RX_FIFO_DRQ_EN, en << RX_FIFO_DRQ_EN);
}
EXPORT_SYMBOL_GPL(snd_sunxi_drq_ctl);

unsigned int snd_sunxi_get_dev_fifo(enum SUNXI_AHUB_PORT port)
{
	SND_LOG_DEBUG("\n");

	if (port <= 6)
		return g_ahub.mem.res.start + SUNXI_AHUB_APBIF_TX_FIFO_DATA(port);
	else
		return g_ahub.mem.res.start + SUNXI_AHUB_APBIF_RX_FIFO_DATA(port - 7);
}
EXPORT_SYMBOL_GPL(snd_sunxi_get_dev_fifo);

/* Provide CLK-related interfaces for the port */
void snd_sunxi_soft_reset(enum SUNXI_AHUB_PORT port)
{
	struct regmap *regmap = g_ahub.mem.regmap;

	SND_LOG_DEBUG("\n");

	switch (port) {
	case SUNXI_AHUB_PORT_CODEC_RX:
	case SUNXI_AHUB_PORT_I2S0_RX:
	case SUNXI_AHUB_PORT_I2S1_RX:
	case SUNXI_AHUB_PORT_I2S2_RX:
	case SUNXI_AHUB_PORT_I2S3_RX:
	case SUNXI_AHUB_PORT_I2S4_RX:
	case SUNXI_AHUB_PORT_OWA_RX:
		regmap_update_bits(regmap, SUNXI_AHUB_SOFT_RST1,
				   0x1 << (TX0_SOFT_RST + port),
				   0x1 << (TX0_SOFT_RST + port));
		break;
	case SUNXI_AHUB_PORT_CODEC_TX:
	case SUNXI_AHUB_PORT_I2S0_TX:
	case SUNXI_AHUB_PORT_I2S1_TX:
	case SUNXI_AHUB_PORT_I2S2_TX:
	case SUNXI_AHUB_PORT_I2S3_TX:
	case SUNXI_AHUB_PORT_I2S4_TX:
	case SUNXI_AHUB_PORT_OWA_TX:
	case SUNXI_AHUB_PORT_DMIC_TX:
		regmap_update_bits(regmap, SUNXI_AHUB_SOFT_RST1,
				   0x1 << (RX0_SOFT_RST + port - 7),
				   0x1 << (RX0_SOFT_RST + port - 7));
		break;
	default:
		SND_LOG_ERR("port invalid:%d\n", port);
	}
}
EXPORT_SYMBOL_GPL(snd_sunxi_soft_reset);

/* just for snd_sunxi_clk_gating_ctl */
struct sunxi_gating_reg_bit {
	unsigned int reg;
	unsigned int bit;
};
static const struct sunxi_gating_reg_bit gating_reg_bit[] = {
	{ SUNXI_AHUB_CLK_GAT3, AUDIOCODEC_GAT1 },
	{ SUNXI_AHUB_CLK_GAT3, I2S0_GAT1 },
	{ SUNXI_AHUB_CLK_GAT3, I2S1_GAT1 },
	{ SUNXI_AHUB_CLK_GAT3, I2S2_GAT1 },
	{ SUNXI_AHUB_CLK_GAT3, I2S3_GAT1 },
	{ SUNXI_AHUB_CLK_GAT3, I2S4_GAT1 },
	{ SUNXI_AHUB_CLK_GAT3, OWA_GAT1 },
	{ SUNXI_AHUB_CLK_GAT2, AUDIOCODEC_GAT0 },
	{ SUNXI_AHUB_CLK_GAT2, I2S0_GAT0 },
	{ SUNXI_AHUB_CLK_GAT2, I2S1_GAT0 },
	{ SUNXI_AHUB_CLK_GAT2, I2S2_GAT0 },
	{ SUNXI_AHUB_CLK_GAT2, I2S3_GAT0 },
	{ SUNXI_AHUB_CLK_GAT2, I2S4_GAT0 },
	{ SUNXI_AHUB_CLK_GAT2, OWA_GAT0 },
	{ SUNXI_AHUB_CLK_GAT2, DMIC_GAT0 },
};

void snd_sunxi_clk_gating_ctl(enum SUNXI_AHUB_PORT port, bool en)
{
	struct regmap *regmap = g_ahub.mem.regmap;

	SND_LOG_DEBUG("\n");

	regmap_update_bits(regmap, gating_reg_bit[port].reg,
			   0x1 << gating_reg_bit[port].bit, en << gating_reg_bit[port].bit);

	if (port <= 6)
		regmap_update_bits(regmap, SUNXI_AHUB_CLK_GAT1,
				   0x1 << (TX0_GAT + port), en << (TX0_GAT + port));
	else
		regmap_update_bits(regmap, SUNXI_AHUB_CLK_GAT1,
				   0x1 << (RX0_GAT + port - 7), en << (RX0_GAT + port - 7));
}
EXPORT_SYMBOL_GPL(snd_sunxi_clk_gating_ctl);

/* just for snd_sunxi_ahub_ctl */
struct sunxi_ahub_port_table1 {
	enum SUNXI_AHUB_PORT port1;
	enum SUNXI_AHUB_PORT *port2;
};
/* port -> src port */
static struct sunxi_ahub_port_table1 port_src_table[] = {
	{ SUNXI_AHUB_PORT_APBIF0_RX,	&g_ahub.ahub_src[0] },
	{ SUNXI_AHUB_PORT_APBIF1_RX,	&g_ahub.ahub_src[1] },
	{ SUNXI_AHUB_PORT_APBIF2_RX,	&g_ahub.ahub_src[2] },
	{ SUNXI_AHUB_PORT_APBIF3_RX,	&g_ahub.ahub_src[3] },
	{ SUNXI_AHUB_PORT_APBIF4_RX,	&g_ahub.ahub_src[4] },
	{ SUNXI_AHUB_PORT_APBIF5_RX,	&g_ahub.ahub_src[5] },
	{ SUNXI_AHUB_PORT_APBIF6_RX,	&g_ahub.ahub_src[6] },
	{ SUNXI_AHUB_PORT_APBIF6_RX,	&g_ahub.ahub_src[7] },
	{ SUNXI_AHUB_PORT_CODEC_RX,	&g_ahub.ahub_src[8] },
	{ SUNXI_AHUB_PORT_I2S0_RX,	&g_ahub.ahub_src[9] },
	{ SUNXI_AHUB_PORT_I2S1_RX,	&g_ahub.ahub_src[10] },
	{ SUNXI_AHUB_PORT_I2S2_RX,	&g_ahub.ahub_src[11] },
	{ SUNXI_AHUB_PORT_I2S3_RX,	&g_ahub.ahub_src[12] },
	{ SUNXI_AHUB_PORT_I2S4_RX,	&g_ahub.ahub_src[13] },
	{ SUNXI_AHUB_PORT_OWA_RX,	&g_ahub.ahub_src[14] },
	{ SUNXI_AHUB_PORT_DAM0_RX0,	&g_ahub.ahub_src[15] },
	{ SUNXI_AHUB_PORT_DAM0_RX1,	&g_ahub.ahub_src[16] },
	{ SUNXI_AHUB_PORT_DAM0_RX2,	&g_ahub.ahub_src[17] },
	{ SUNXI_AHUB_PORT_DAM1_RX0,	&g_ahub.ahub_src[18] },
	{ SUNXI_AHUB_PORT_DAM1_RX1,	&g_ahub.ahub_src[19] },
	{ SUNXI_AHUB_PORT_DAM1_RX2,	&g_ahub.ahub_src[20] },
};

struct sunxi_ahub_port_table2 {
	enum SUNXI_AHUB_PORT port1;
	enum SUNXI_AHUB_PORT port2;
};
/* apbif port <-> module port */
static const struct sunxi_ahub_port_table2 apbif_module_table[] = {
	{ SUNXI_AHUB_PORT_APBIF0_TX,	SUNXI_AHUB_PORT_CODEC_RX },
	{ SUNXI_AHUB_PORT_APBIF1_TX,	SUNXI_AHUB_PORT_I2S0_RX },
	{ SUNXI_AHUB_PORT_APBIF2_TX,	SUNXI_AHUB_PORT_I2S1_RX },
	{ SUNXI_AHUB_PORT_APBIF3_TX,	SUNXI_AHUB_PORT_I2S2_RX },
	{ SUNXI_AHUB_PORT_APBIF4_TX,	SUNXI_AHUB_PORT_I2S3_RX },
	{ SUNXI_AHUB_PORT_APBIF5_TX,	SUNXI_AHUB_PORT_I2S4_RX },
	{ SUNXI_AHUB_PORT_APBIF6_TX,	SUNXI_AHUB_PORT_OWA_RX },

	{ SUNXI_AHUB_PORT_APBIF0_RX,	SUNXI_AHUB_PORT_CODEC_TX },
	{ SUNXI_AHUB_PORT_APBIF1_RX,	SUNXI_AHUB_PORT_I2S0_TX },
	{ SUNXI_AHUB_PORT_APBIF2_RX,	SUNXI_AHUB_PORT_I2S1_TX },
	{ SUNXI_AHUB_PORT_APBIF3_RX,	SUNXI_AHUB_PORT_I2S2_TX },
	{ SUNXI_AHUB_PORT_APBIF4_RX,	SUNXI_AHUB_PORT_I2S3_TX },
	{ SUNXI_AHUB_PORT_APBIF5_RX,	SUNXI_AHUB_PORT_I2S4_TX },
	{ SUNXI_AHUB_PORT_APBIF6_RX,	SUNXI_AHUB_PORT_OWA_TX },
	{ SUNXI_AHUB_PORT_APBIF7_RX,	SUNXI_AHUB_PORT_DMIC_TX },
};

static void sunxi_ahub_set_dam_volume(enum SUNXI_AHUB_PORT port, unsigned int tx_cnt)
{
	struct regmap *regmap = g_ahub.mem.regmap;

	if (port == SUNXI_AHUB_PORT_DAM0_TX) {
		switch (tx_cnt) {
		case 0:
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL0(0), 0x0);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL1(0), 0x0);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL2(0), 0x0);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL3(0), 0x0);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL4(0), 0x0);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL5(0), 0x0);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL6(0), 0x0);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL7(0), 0x0);
		break;
		case 1:
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL0(0), 0x1110111);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL1(0), 0x1110111);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL2(0), 0x1110111);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL3(0), 0x1110111);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL4(0), 0x1110111);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL5(0), 0x1110111);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL6(0), 0x1110111);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL7(0), 0x1110111);
		break;
		case 2:
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL0(0), 0x2220222);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL1(0), 0x2220222);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL2(0), 0x2220222);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL3(0), 0x2220222);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL4(0), 0x2220222);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL5(0), 0x2220222);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL6(0), 0x2220222);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL7(0), 0x2220222);
		break;
		case 3:
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL0(0), 0x4440444);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL1(0), 0x4440444);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL2(0), 0x4440444);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL3(0), 0x4440444);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL4(0), 0x4440444);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL5(0), 0x4440444);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL6(0), 0x4440444);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL7(0), 0x4440444);
		break;
		}
	} else {
		switch (tx_cnt) {
		case 0:
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL0(1), 0x0);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL1(1), 0x0);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL2(1), 0x0);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL3(1), 0x0);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL4(1), 0x0);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL5(1), 0x0);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL6(1), 0x0);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL7(1), 0x0);
		break;
		case 1:
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL0(1), 0x1110111);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL1(1), 0x1110111);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL2(1), 0x1110111);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL3(1), 0x1110111);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL4(1), 0x1110111);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL5(1), 0x1110111);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL6(1), 0x1110111);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL7(1), 0x1110111);
		break;
		case 2:
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL0(1), 0x2220222);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL1(1), 0x2220222);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL2(1), 0x2220222);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL3(1), 0x2220222);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL4(1), 0x2220222);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL5(1), 0x2220222);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL6(1), 0x2220222);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL7(1), 0x2220222);
		break;
		case 3:
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL0(1), 0x4440444);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL1(1), 0x4440444);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL2(1), 0x4440444);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL3(1), 0x4440444);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL4(1), 0x4440444);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL5(1), 0x4440444);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL6(1), 0x4440444);
			regmap_write(regmap, SUNXI_AHUB_DAM_VOL_CTL7(1), 0x4440444);
		break;
		}
	}
}

unsigned int snd_sunxi_ahub_ctl(enum SUNXI_AHUB_PORT port, unsigned int ch, bool en)
{
	struct regmap *regmap = g_ahub.mem.regmap;
	enum SUNXI_AHUB_PORT src;
	enum SUNXI_AHUB_PORT apbif;
	enum SUNXI_AHUB_PORT first_dam_tx = SUNXI_AHUB_PORT_NONE;
	static unsigned int dam0_tx_en_cnt, dam1_tx_en_cnt;
	unsigned int real_ch;
	unsigned int i, j;
	bool is_ch_equal = true;

	SND_LOG_DEBUG("\n");

	mutex_lock(&g_ahub.ahub_lock);

	/* Find the apbif corresponding to the module */
	for (i = 0; i < ARRAY_SIZE(apbif_module_table); i++) {
		if (apbif_module_table[i].port2 == port) {
			apbif = apbif_module_table[i].port1;
			break;
		}
	}

	/* Play Stream
	 * 1. Check whether there is a dam-rx whose src is apbif-tx and enable the dam-rx.
	 * 2. Determine whether the src of module-rx is dam-tx and enable the dam-tx.
	 * 3. If dam-tx needs to be enabled, check whether there is any usage of the secondary dam
	 *    and enable it.
	 * 4. Return the number of channels of dam-tx when the number of channels of dam-tx does
	 *    not match that of apbif-tx.
	 */
	if (port <= 6) {
		/* Enable dam rx */
		for (i = 15; i < 21; i++) {
			if (i <= 17 && g_ahub.ahub_src[i] == apbif) {
				first_dam_tx = SUNXI_AHUB_PORT_DAM0_TX;
				regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(0),
						   0x1 << (DAM_RX0_EN + i - 15),
						   en << (DAM_RX0_EN + i - 15));
			} else if (g_ahub.ahub_src[i] == apbif) {
				first_dam_tx = SUNXI_AHUB_PORT_DAM1_TX;
				regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(1),
						   0x1 << (DAM_RX0_EN + i - 18),
						   en << (DAM_RX0_EN + i - 18));
			}
		}

		/* Find src corresponding to port */
		for (i = 0; i < ARRAY_SIZE(port_src_table); i++) {
			if (port_src_table[i].port1 == port) {
				src = *(port_src_table[i].port2);
				break;
			}
		}

		/* Enable dam tx and return channels */
		if (src == SUNXI_AHUB_PORT_DAM0_TX) {
			/* Set dam volume */
			for (i = 0, j = 0; i < 3; i++) {
				if (g_ahub.ahub_src[15 + i] != SUNXI_AHUB_PORT_NONE)
					j++;
			}
			sunxi_ahub_set_dam_volume(SUNXI_AHUB_PORT_DAM0_TX, j);

			if (en) {
				if (dam0_tx_en_cnt++ == 0)
					regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(0),
							   0x1 << DAM_TX_EN, 0x1 << DAM_TX_EN);
				/* Check whether there is any usage of the secondary dam */
				if (first_dam_tx == SUNXI_AHUB_PORT_DAM1_TX) {
					if (dam1_tx_en_cnt++ == 0)
						regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(1),
								   0x1 << DAM_TX_EN,
								   0x1 << DAM_TX_EN);
					/* The situation where dam-tx is used multiple times as
					   another group of dma-rx is not supported */
					for (i = 0; i < 3; i++)
						if (g_ahub.ahub_src[15 + i] == SUNXI_AHUB_PORT_DAM1_TX)
							break;
					regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(0),
							   0x1 << (DAM_RX0_EN + i),
							   0x1 << (DAM_RX0_EN + i));
				}
			} else {
				if (dam0_tx_en_cnt-- == 1)
					regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(0),
							   0x1 << DAM_TX_EN, 0x0 << DAM_TX_EN);
				if (first_dam_tx == SUNXI_AHUB_PORT_DAM1_TX) {
					if (dam1_tx_en_cnt-- == 1)
						regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(1),
								   0x1 << DAM_TX_EN,
								   0x0 << DAM_TX_EN);
					for (i = 0; i < 3; i++)
						if (g_ahub.ahub_src[15 + i] == SUNXI_AHUB_PORT_DAM1_TX)
							break;
					regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(0),
							   0x1 << (DAM_RX0_EN + i),
							   0x0 << (DAM_RX0_EN + i));
				}
			}

			if (g_ahub.dam_ch[0] != ch) {
				mutex_unlock(&g_ahub.ahub_lock);
				return g_ahub.dam_ch[0];
			}

		} else if (src == SUNXI_AHUB_PORT_DAM1_TX) {
			/* Set dam volume */
			for (i = 0, j = 0; i < 3; i++) {
				if (g_ahub.ahub_src[18 + i] != SUNXI_AHUB_PORT_NONE)
					j++;
			}
			sunxi_ahub_set_dam_volume(SUNXI_AHUB_PORT_DAM1_TX, j);

			if (en) {
				if (dam1_tx_en_cnt++ == 0)
					regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(1),
							   0x1 << DAM_TX_EN, 0x1 << DAM_TX_EN);
				if (first_dam_tx == SUNXI_AHUB_PORT_DAM0_TX) {
					if (dam0_tx_en_cnt++ == 0)
						regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(0),
								   0x1 << DAM_TX_EN,
								   0x1 << DAM_TX_EN);
					for (i = 0; i < 3; i++)
						if (g_ahub.ahub_src[18 + i] == SUNXI_AHUB_PORT_DAM0_TX)
							break;
					regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(1),
							   0x1 << (DAM_RX0_EN + i),
							   0x1 << (DAM_RX0_EN + i));
				}
			} else {
				if (dam1_tx_en_cnt-- == 1)
					regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(1),
							   0x1 << DAM_TX_EN, 0x0 << DAM_TX_EN);
				if (first_dam_tx == SUNXI_AHUB_PORT_DAM0_TX) {
					if (dam0_tx_en_cnt-- == 1)
						regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(0),
								   0x1 << DAM_TX_EN,
								   0x0 << DAM_TX_EN);
					for (i = 0; i < 3; i++)
						if (g_ahub.ahub_src[18 + i] == SUNXI_AHUB_PORT_DAM0_TX)
							break;
					regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(1),
							   0x1 << (DAM_RX0_EN + i),
							   0x0 << (DAM_RX0_EN + i));
				}
			}
			if (g_ahub.dam_ch[4] != ch) {
				mutex_unlock(&g_ahub.ahub_lock);
				return g_ahub.dam_ch[4];
			}
		}
	/* Cap Stream
	 * 1. Check whether there is a dam-rx whose src is module-tx and enable the dam-rx.
	 * 2. Determine whether the src of apbif-rx is dam-tx and enable the dam-tx.
	 * 3. If dam-tx needs to be enabled, check whether there is any usage of the secondary dam
	 *    and enable it.
	 * 4. Return the number of channels of dam-rx when the number of channels of dam-rx does
	 *    not match that of apbif-rx.
	 */
	} else {
		/* Enable dam rx and return channels */
		for (i = 15; i < 21; i++) {
			if (g_ahub.ahub_src[i] == port && i <= 17) {
				first_dam_tx = SUNXI_AHUB_PORT_DAM0_TX;
				regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(0),
						   0x1 << (DAM_RX0_EN + i - 15),
						   en << (DAM_RX0_EN + i - 15));
				if (g_ahub.dam_ch[i - 14] != ch) {
					is_ch_equal = false;
					real_ch = g_ahub.dam_ch[i - 14];
				}
			} else if (g_ahub.ahub_src[i] == port) {
				first_dam_tx = SUNXI_AHUB_PORT_DAM1_TX;
				regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(1),
						   0x1 << (DAM_RX0_EN + i - 15),
						   en << (DAM_RX0_EN + i - 15));
				if (g_ahub.dam_ch[i - 13] != ch) {
					is_ch_equal = false;
					real_ch = g_ahub.dam_ch[i - 13];
				}
			}
		}

		/* Find src corresponding to port */
		for (i = 0; i < ARRAY_SIZE(port_src_table); i++) {
			if (port_src_table[i].port1 == apbif) {
				src = *(port_src_table[i].port2);
				break;
			}
		}

		/* Enable dam tx */
		if (src == SUNXI_AHUB_PORT_DAM0_TX) {
			/* Set dam volume */
			for (i = 0, j = 0; i < 3; i++) {
				if (g_ahub.ahub_src[15 + i] != SUNXI_AHUB_PORT_NONE)
					j++;
			}
			sunxi_ahub_set_dam_volume(SUNXI_AHUB_PORT_DAM0_TX, j);

			if (en) {
				if (dam0_tx_en_cnt++ == 0)
					regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(0),
							   0x1 << DAM_TX_EN, 0x1 << DAM_TX_EN);
				if (first_dam_tx == SUNXI_AHUB_PORT_DAM1_TX) {
					if (dam1_tx_en_cnt++ == 0)
						regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(1),
								   0x1 << DAM_TX_EN,
								   0x1 << DAM_TX_EN);
					for (i = 0; i < 3; i++)
						if (g_ahub.ahub_src[15 + i] == SUNXI_AHUB_PORT_DAM1_TX)
							break;
					regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(0),
							   0x1 << (DAM_RX0_EN + i),
							   0x1 << (DAM_RX0_EN + i));
				}
			} else {
				if (dam0_tx_en_cnt-- == 1)
					regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(0),
							   0x1 << DAM_TX_EN, 0x0 << DAM_TX_EN);
				if (first_dam_tx == SUNXI_AHUB_PORT_DAM1_TX) {
					if (dam1_tx_en_cnt-- == 1)
						regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(1),
								   0x1 << DAM_TX_EN,
								   0x0 << DAM_TX_EN);
					for (i = 0; i < 3; i++)
						if (g_ahub.ahub_src[15 + i] == SUNXI_AHUB_PORT_DAM1_TX)
							break;
					regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(0),
							   0x1 << (DAM_RX0_EN + i),
							   0x0 << (DAM_RX0_EN + i));
				}
			}
		} else if (src == SUNXI_AHUB_PORT_DAM1_TX) {
			/* Set dam volume */
			for (i = 0, j = 0; i < 3; i++) {
				if (g_ahub.ahub_src[18 + i] != SUNXI_AHUB_PORT_NONE)
					j++;
			}
			sunxi_ahub_set_dam_volume(SUNXI_AHUB_PORT_DAM1_TX, j);

			if (en) {
				if (dam1_tx_en_cnt++ == 0)
					regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(1),
							   0x1 << DAM_TX_EN, 0x1 << DAM_TX_EN);
				if (first_dam_tx == SUNXI_AHUB_PORT_DAM0_TX) {
					if (dam0_tx_en_cnt++ == 0)
						regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(0),
								   0x1 << DAM_TX_EN,
								   0x1 << DAM_TX_EN);
					for (i = 0; i < 3; i++)
						if (g_ahub.ahub_src[18 + i] == SUNXI_AHUB_PORT_DAM0_TX)
							break;
					regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(1),
							   0x1 << (DAM_RX0_EN + i),
							   0x1 << (DAM_RX0_EN + i));
				}
			} else {
				if (dam1_tx_en_cnt-- == 1)
					regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(1),
							   0x1 << DAM_TX_EN, 0x0 << DAM_TX_EN);
				if (first_dam_tx == SUNXI_AHUB_PORT_DAM0_TX) {
					if (dam0_tx_en_cnt-- == 1)
						regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(0),
								   0x1 << DAM_TX_EN,
								   0x0 << DAM_TX_EN);
					for (i = 0; i < 3; i++)
						if (g_ahub.ahub_src[18 + i] == SUNXI_AHUB_PORT_DAM0_TX)
							break;
					regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(1),
							   0x1 << (DAM_RX0_EN + i),
							   0x0 << (DAM_RX0_EN + i));
				}
			}
		}

		if (!is_ch_equal) {
			mutex_unlock(&g_ahub.ahub_lock);
			return real_ch;
		}
	}

	mutex_unlock(&g_ahub.ahub_lock);
	return ch;
}

/* about kcontrol */
static const char *ahub_route_mux_text[] = {
	"NONE",
	"DAM1_TX",
	"DAM0_TX",
	"APBIF6_TX",
	"APBIF5_TX",
	"APBIF4_TX",
	"APBIF3_TX",
	"APBIF2_TX",
	"APBIF1_TX",
	"APBIF0_TX",
	"I2S4_TX",
	"I2S3_TX",
	"I2S2_TX",
	"I2S1_TX",
	"I2S0_TX",
	"CODEC_TX",
	"OWA_TX",
	"DMIC_TX",
};
static const char *ahub_dam_ch_mux_text[] = {
	"1ch",
	"2ch",
	"3ch",
	"4ch",
	"5ch",
	"6ch",
	"7ch",
	"8ch",
	"9ch",
	"10ch",
	"11ch",
	"12ch",
	"13ch",
	"14ch",
	"15ch",
	"16ch",
};
static const char *ahub_dam_map_mux_text[] = {
	"ch0",
	"ch1",
	"ch2",
	"ch3",
	"ch4",
	"ch5",
	"ch6",
	"ch7",
	"ch8",
	"ch9",
	"ch10",
	"ch11",
	"ch12",
	"ch13",
	"ch14",
	"ch15",
};

static SOC_ENUM_SINGLE_DECL(ahub_route_mux_0, SND_SOC_NOPM, 0, ahub_route_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_route_mux_1, SND_SOC_NOPM, 1, ahub_route_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_route_mux_2, SND_SOC_NOPM, 2, ahub_route_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_route_mux_3, SND_SOC_NOPM, 3, ahub_route_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_route_mux_4, SND_SOC_NOPM, 4, ahub_route_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_route_mux_5, SND_SOC_NOPM, 5, ahub_route_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_route_mux_6, SND_SOC_NOPM, 6, ahub_route_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_route_mux_7, SND_SOC_NOPM, 7, ahub_route_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_route_mux_8, SND_SOC_NOPM, 8, ahub_route_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_route_mux_9, SND_SOC_NOPM, 9, ahub_route_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_route_mux_10, SND_SOC_NOPM, 10, ahub_route_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_route_mux_11, SND_SOC_NOPM, 11, ahub_route_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_route_mux_12, SND_SOC_NOPM, 12, ahub_route_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_route_mux_13, SND_SOC_NOPM, 13, ahub_route_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_route_mux_14, SND_SOC_NOPM, 14, ahub_route_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_route_mux_15, SND_SOC_NOPM, 15, ahub_route_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_route_mux_16, SND_SOC_NOPM, 16, ahub_route_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_route_mux_17, SND_SOC_NOPM, 17, ahub_route_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_route_mux_18, SND_SOC_NOPM, 18, ahub_route_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_route_mux_19, SND_SOC_NOPM, 19, ahub_route_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_route_mux_20, SND_SOC_NOPM, 20, ahub_route_mux_text);

static SOC_ENUM_SINGLE_DECL(ahub_dam_ch_mux_0, SND_SOC_NOPM, 0, ahub_dam_ch_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_ch_mux_1, SND_SOC_NOPM, 1, ahub_dam_ch_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_ch_mux_2, SND_SOC_NOPM, 2, ahub_dam_ch_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_ch_mux_3, SND_SOC_NOPM, 3, ahub_dam_ch_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_ch_mux_4, SND_SOC_NOPM, 4, ahub_dam_ch_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_ch_mux_5, SND_SOC_NOPM, 5, ahub_dam_ch_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_ch_mux_6, SND_SOC_NOPM, 6, ahub_dam_ch_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_ch_mux_7, SND_SOC_NOPM, 7, ahub_dam_ch_mux_text);

static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_0, SND_SOC_NOPM, 0, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_1, SND_SOC_NOPM, 1, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_2, SND_SOC_NOPM, 2, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_3, SND_SOC_NOPM, 3, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_4, SND_SOC_NOPM, 4, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_5, SND_SOC_NOPM, 5, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_6, SND_SOC_NOPM, 6, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_7, SND_SOC_NOPM, 7, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_8, SND_SOC_NOPM, 8, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_9, SND_SOC_NOPM, 9, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_10, SND_SOC_NOPM, 10, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_11, SND_SOC_NOPM, 11, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_12, SND_SOC_NOPM, 12, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_13, SND_SOC_NOPM, 13, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_14, SND_SOC_NOPM, 14, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_15, SND_SOC_NOPM, 15, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_16, SND_SOC_NOPM, 16, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_17, SND_SOC_NOPM, 17, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_18, SND_SOC_NOPM, 18, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_19, SND_SOC_NOPM, 19, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_20, SND_SOC_NOPM, 20, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_21, SND_SOC_NOPM, 21, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_22, SND_SOC_NOPM, 22, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_23, SND_SOC_NOPM, 23, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_24, SND_SOC_NOPM, 24, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_25, SND_SOC_NOPM, 25, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_26, SND_SOC_NOPM, 26, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_27, SND_SOC_NOPM, 27, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_28, SND_SOC_NOPM, 28, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_29, SND_SOC_NOPM, 29, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_30, SND_SOC_NOPM, 30, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_31, SND_SOC_NOPM, 31, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_32, SND_SOC_NOPM, 32, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_33, SND_SOC_NOPM, 33, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_34, SND_SOC_NOPM, 34, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_35, SND_SOC_NOPM, 35, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_36, SND_SOC_NOPM, 36, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_37, SND_SOC_NOPM, 37, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_38, SND_SOC_NOPM, 38, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_39, SND_SOC_NOPM, 39, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_40, SND_SOC_NOPM, 40, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_41, SND_SOC_NOPM, 41, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_42, SND_SOC_NOPM, 42, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_43, SND_SOC_NOPM, 43, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_44, SND_SOC_NOPM, 44, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_45, SND_SOC_NOPM, 45, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_46, SND_SOC_NOPM, 46, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_47, SND_SOC_NOPM, 47, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_48, SND_SOC_NOPM, 48, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_49, SND_SOC_NOPM, 49, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_50, SND_SOC_NOPM, 50, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_51, SND_SOC_NOPM, 51, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_52, SND_SOC_NOPM, 52, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_53, SND_SOC_NOPM, 53, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_54, SND_SOC_NOPM, 54, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_55, SND_SOC_NOPM, 55, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_56, SND_SOC_NOPM, 56, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_57, SND_SOC_NOPM, 57, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_58, SND_SOC_NOPM, 58, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_59, SND_SOC_NOPM, 59, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_60, SND_SOC_NOPM, 60, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_61, SND_SOC_NOPM, 61, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_62, SND_SOC_NOPM, 62, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_63, SND_SOC_NOPM, 63, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_64, SND_SOC_NOPM, 64, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_65, SND_SOC_NOPM, 65, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_66, SND_SOC_NOPM, 66, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_67, SND_SOC_NOPM, 67, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_68, SND_SOC_NOPM, 68, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_69, SND_SOC_NOPM, 69, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_70, SND_SOC_NOPM, 70, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_71, SND_SOC_NOPM, 71, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_72, SND_SOC_NOPM, 72, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_73, SND_SOC_NOPM, 73, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_74, SND_SOC_NOPM, 74, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_75, SND_SOC_NOPM, 75, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_76, SND_SOC_NOPM, 76, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_77, SND_SOC_NOPM, 77, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_78, SND_SOC_NOPM, 78, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_79, SND_SOC_NOPM, 79, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_80, SND_SOC_NOPM, 80, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_81, SND_SOC_NOPM, 81, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_82, SND_SOC_NOPM, 82, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_83, SND_SOC_NOPM, 83, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_84, SND_SOC_NOPM, 84, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_85, SND_SOC_NOPM, 85, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_86, SND_SOC_NOPM, 86, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_87, SND_SOC_NOPM, 87, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_88, SND_SOC_NOPM, 88, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_89, SND_SOC_NOPM, 89, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_90, SND_SOC_NOPM, 90, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_91, SND_SOC_NOPM, 91, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_92, SND_SOC_NOPM, 92, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_93, SND_SOC_NOPM, 93, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_94, SND_SOC_NOPM, 94, ahub_dam_map_mux_text);
static SOC_ENUM_SINGLE_DECL(ahub_dam_map_mux_95, SND_SOC_NOPM, 95, ahub_dam_map_mux_text);

static const unsigned int ahub_route_mux_reg[][2] = {
	{ SUNXI_AHUB_APBIF_RX0_CONT_SEL1,	SUNXI_AHUB_APBIF_RX0_CONT_SEL2 },
	{ SUNXI_AHUB_APBIF_RX1_CONT_SEL1,	SUNXI_AHUB_APBIF_RX1_CONT_SEL2 },
	{ SUNXI_AHUB_APBIF_RX2_CONT_SEL1,	SUNXI_AHUB_APBIF_RX2_CONT_SEL2 },
	{ SUNXI_AHUB_APBIF_RX3_CONT_SEL1,	SUNXI_AHUB_APBIF_RX3_CONT_SEL2 },
	{ SUNXI_AHUB_APBIF_RX4_CONT_SEL1,	SUNXI_AHUB_APBIF_RX4_CONT_SEL2 },
	{ SUNXI_AHUB_APBIF_RX5_CONT_SEL1,	SUNXI_AHUB_APBIF_RX5_CONT_SEL2 },
	{ SUNXI_AHUB_APBIF_RX6_CONT_SEL1,	SUNXI_AHUB_APBIF_RX6_CONT_SEL2 },
	{ SUNXI_AHUB_APBIF_RX7_CONT_SEL1,	SUNXI_AHUB_APBIF_RX7_CONT_SEL2 },
	{ SUNXI_AHUB_ADDA_DAC_CONT_SEL1,	SUNXI_AHUB_ADDA_DAC_CONT_SEL2 },
	{ SUNXI_AHUB_I2S0_TX_CONT_SEL1,		SUNXI_AHUB_I2S0_TX_CONT_SEL2 },
	{ SUNXI_AHUB_I2S1_TX_CONT_SEL1,		SUNXI_AHUB_I2S1_TX_CONT_SEL2 },
	{ SUNXI_AHUB_I2S2_TX_CONT_SEL1,		SUNXI_AHUB_I2S2_TX_CONT_SEL2 },
	{ SUNXI_AHUB_I2S3_TX_CONT_SEL1,		SUNXI_AHUB_I2S3_TX_CONT_SEL2 },
	{ SUNXI_AHUB_I2S4_TX_CONT_SEL1,		SUNXI_AHUB_I2S4_TX_CONT_SEL2 },
	{ SUNXI_AHUB_OWA_TX_CONT_SEL1,		SUNXI_AHUB_OWA_TX_CONT_SEL2 },
	{ SUNXI_AHUB_DAM_RX0_CONT_SEL1(0),	SUNXI_AHUB_DAM_RX0_CONT_SEL2(0) },
	{ SUNXI_AHUB_DAM_RX1_CONT_SEL1(0),	SUNXI_AHUB_DAM_RX1_CONT_SEL2(0) },
	{ SUNXI_AHUB_DAM_RX2_CONT_SEL1(0),	SUNXI_AHUB_DAM_RX2_CONT_SEL2(0) },
	{ SUNXI_AHUB_DAM_RX0_CONT_SEL1(1),	SUNXI_AHUB_DAM_RX0_CONT_SEL2(1) },
	{ SUNXI_AHUB_DAM_RX1_CONT_SEL1(1),	SUNXI_AHUB_DAM_RX1_CONT_SEL2(1) },
	{ SUNXI_AHUB_DAM_RX2_CONT_SEL1(1),	SUNXI_AHUB_DAM_RX2_CONT_SEL2(1) },
};

/* just for sunxi_ahub_route_get/set */
static const unsigned int ahub_route_mux_val1[] = {
	0x1 << NONE_TX,
	0x1 << DAM1_TX,
	0x1 << DAM0_TX,
	0x1 << APBIF6_TX,
	0x1 << APBIF5_TX,
	0x1 << APBIF4_TX,
	0x1 << APBIF3_TX,
	0x1 << APBIF2_TX,
	0x1 << APBIF1_TX,
	0x1 << APBIF0_TX,
};
static const unsigned int ahub_route_mux_val2[] = {
	0x1 << I2S4_TX,
	0x1 << I2S3_TX,
	0x1 << I2S2_TX,
	0x1 << I2S1_TX,
	0x1 << I2S0_TX,
	0x1 << CODEC_TX,
	0x1 << OWA_TX,
	0x1 << DMIC_TX,
};

/* just for sunxi_ahub_route_set, text_port[ucontrol->value.integer.value[0]] = port */
static enum SUNXI_AHUB_PORT text_port[] = {
	SUNXI_AHUB_PORT_NONE,
	SUNXI_AHUB_PORT_DAM1_TX,
	SUNXI_AHUB_PORT_DAM0_TX,
	SUNXI_AHUB_PORT_APBIF6_TX,
	SUNXI_AHUB_PORT_APBIF5_TX,
	SUNXI_AHUB_PORT_APBIF4_TX,
	SUNXI_AHUB_PORT_APBIF3_TX,
	SUNXI_AHUB_PORT_APBIF2_TX,
	SUNXI_AHUB_PORT_APBIF1_TX,
	SUNXI_AHUB_PORT_APBIF0_TX,
	SUNXI_AHUB_PORT_I2S4_TX,
	SUNXI_AHUB_PORT_I2S3_TX,
	SUNXI_AHUB_PORT_I2S2_TX,
	SUNXI_AHUB_PORT_I2S1_TX,
	SUNXI_AHUB_PORT_I2S0_TX,
	SUNXI_AHUB_PORT_CODEC_TX,
	SUNXI_AHUB_PORT_OWA_TX,
	SUNXI_AHUB_PORT_DMIC_TX,
};

static int sunxi_ahub_route_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
	unsigned int shift = e->shift_l;
	struct regmap *regmap = g_ahub.mem.regmap;
	unsigned int src_reg1, src_reg2;
	unsigned int reg_val1, reg_val2;
	int i;

	SND_LOG_DEBUG("\n");

	src_reg1 = ahub_route_mux_reg[shift][0];
	src_reg2 = ahub_route_mux_reg[shift][1];
	regmap_read(regmap, src_reg1, &reg_val1);
	regmap_read(regmap, src_reg2, &reg_val2);

	reg_val1 &= 0xB000007F;
	reg_val2 &= 0x1F2101;

	for (i = 0; i < ARRAY_SIZE(ahub_route_mux_val1); i++) {
		if (reg_val1 & ahub_route_mux_val1[i]) {
			ucontrol->value.integer.value[0] = i;
			return 0;
		}
	}
	for (i = 0; i < ARRAY_SIZE(ahub_route_mux_val2); i++) {
		if (reg_val2 & ahub_route_mux_val2[i]) {
			ucontrol->value.integer.value[0] = ARRAY_SIZE(ahub_route_mux_val1) + i;
			return 0;
		}
	}

	return -EINVAL;
}

static int sunxi_ahub_route_set(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
	unsigned int shift = e->shift_l;
	struct regmap *regmap = g_ahub.mem.regmap;
	unsigned int src_reg1, src_reg2;
	unsigned int src_bit;

	SND_LOG_DEBUG("\n");

	src_reg1 = ahub_route_mux_reg[shift][0];
	src_reg2 = ahub_route_mux_reg[shift][1];

	regmap_write(regmap, src_reg1, 0x0);
	regmap_write(regmap, src_reg2, 0x0);

	if (ucontrol->value.integer.value[0] < ARRAY_SIZE(ahub_route_mux_val1)) {
		src_bit = ahub_route_mux_val1[ucontrol->value.integer.value[0]];
		regmap_update_bits(regmap, src_reg1, src_bit, src_bit);
	} else {
		src_bit = ahub_route_mux_val2[ucontrol->value.integer.value[0] -
					      ARRAY_SIZE(ahub_route_mux_val1)];
		regmap_update_bits(regmap, src_reg2, src_bit, src_bit);
	}

	g_ahub.ahub_src[shift] = text_port[ucontrol->value.integer.value[0]];

	return 0;
}

static int sunxi_ahub_dam_ch_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
	unsigned int shift = e->shift_l;
	struct regmap *regmap = g_ahub.mem.regmap;
	unsigned int reg_val;
	unsigned int offset;

	SND_LOG_DEBUG("\n");

	if (shift <= 3)
		regmap_read(regmap, SUNXI_AHUB_DAM_CTL(0), &reg_val);
	else
		regmap_read(regmap, SUNXI_AHUB_DAM_CTL(1), &reg_val);

	if (shift == 0 || shift == 4)
		offset = DAM_TX_CHAN_NUM;
	else if (shift <= 3)
		offset = DAM_RX0_CHAN_NUM + (shift - 1) * 4;
	else
		offset = DAM_RX0_CHAN_NUM + (shift - 5) * 4;

	ucontrol->value.integer.value[0] = 0xF & (reg_val >> offset);

	return 0;
}

static int sunxi_ahub_dam_ch_set(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
	unsigned int shift = e->shift_l;
	struct regmap *regmap = g_ahub.mem.regmap;

	SND_LOG_DEBUG("\n");

	if (shift == 0)
		regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(0), 0xF << DAM_TX_CHAN_NUM,
				   ucontrol->value.integer.value[0] << DAM_TX_CHAN_NUM);
	else if (shift == 4)
		regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(1), 0xF << DAM_TX_CHAN_NUM,
				   ucontrol->value.integer.value[0] << DAM_TX_CHAN_NUM);
	else if (shift <= 3)
		regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(0),
				   0xF << (DAM_RX0_CHAN_NUM + (shift - 1) * 4),
				   ucontrol->value.integer.value[0] <<
				   (DAM_RX0_CHAN_NUM + (shift - 1) * 4));
	else
		regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(1),
				   0xF << (DAM_RX0_CHAN_NUM + (shift - 4) * 4),
				   ucontrol->value.integer.value[0] <<
				   (DAM_RX0_CHAN_NUM + (shift - 4) * 4));

	g_ahub.dam_ch[shift] = ucontrol->value.integer.value[0] + 1;

	return 0;
}

#define CREATE_DAM_TX_MAP_REG_BIT(n) \
	static const unsigned int ahub_dam_tx_map_reg_bit##n[][2] = { \
		{ SUNXI_AHUB_DAM_MIX_MAP0(n), DAM_TX_CH0_SRC0_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP0(n), DAM_TX_CH0_SRC1_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP0(n), DAM_TX_CH0_SRC2_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP0(n), DAM_TX_CH1_SRC0_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP0(n), DAM_TX_CH1_SRC1_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP0(n), DAM_TX_CH1_SRC2_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP1(n), DAM_TX_CH2_SRC0_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP1(n), DAM_TX_CH2_SRC1_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP1(n), DAM_TX_CH2_SRC2_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP1(n), DAM_TX_CH3_SRC0_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP1(n), DAM_TX_CH3_SRC1_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP1(n), DAM_TX_CH3_SRC2_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP2(n), DAM_TX_CH4_SRC0_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP2(n), DAM_TX_CH4_SRC1_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP2(n), DAM_TX_CH4_SRC2_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP2(n), DAM_TX_CH5_SRC0_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP2(n), DAM_TX_CH5_SRC1_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP2(n), DAM_TX_CH5_SRC2_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP3(n), DAM_TX_CH6_SRC0_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP3(n), DAM_TX_CH6_SRC1_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP3(n), DAM_TX_CH6_SRC2_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP3(n), DAM_TX_CH7_SRC0_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP3(n), DAM_TX_CH7_SRC1_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP3(n), DAM_TX_CH7_SRC2_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP4(n), DAM_TX_CH8_SRC0_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP4(n), DAM_TX_CH8_SRC1_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP4(n), DAM_TX_CH8_SRC2_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP4(n), DAM_TX_CH9_SRC0_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP4(n), DAM_TX_CH9_SRC1_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP4(n), DAM_TX_CH9_SRC2_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP5(n), DAM_TX_CH10_SRC0_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP5(n), DAM_TX_CH10_SRC1_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP5(n), DAM_TX_CH10_SRC2_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP5(n), DAM_TX_CH11_SRC0_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP5(n), DAM_TX_CH11_SRC1_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP5(n), DAM_TX_CH11_SRC2_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP6(n), DAM_TX_CH12_SRC0_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP6(n), DAM_TX_CH12_SRC1_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP6(n), DAM_TX_CH12_SRC2_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP6(n), DAM_TX_CH13_SRC0_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP6(n), DAM_TX_CH13_SRC1_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP6(n), DAM_TX_CH13_SRC2_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP7(n), DAM_TX_CH14_SRC0_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP7(n), DAM_TX_CH14_SRC1_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP7(n), DAM_TX_CH14_SRC2_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP7(n), DAM_TX_CH15_SRC0_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP7(n), DAM_TX_CH15_SRC1_MAP }, \
		{ SUNXI_AHUB_DAM_MIX_MAP7(n), DAM_TX_CH15_SRC2_MAP }, \
	}

CREATE_DAM_TX_MAP_REG_BIT(0);
CREATE_DAM_TX_MAP_REG_BIT(1);

static int sunxi_ahub_dam_tx_map_get(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
	unsigned int shift = e->shift_l;
	struct regmap *regmap = g_ahub.mem.regmap;
	unsigned int reg_val;

	SND_LOG_DEBUG("\n");

	if (shift <= 47) {
		regmap_read(regmap, ahub_dam_tx_map_reg_bit0[shift][0], &reg_val);
		reg_val = (reg_val >> ahub_dam_tx_map_reg_bit0[shift][1]) & 0xF;
	} else {
		regmap_read(regmap, ahub_dam_tx_map_reg_bit1[shift - 48][0], &reg_val);
		reg_val = (reg_val >> ahub_dam_tx_map_reg_bit1[shift - 48][1]) & 0xF;
	}

	ucontrol->value.integer.value[0] = reg_val;

	return 0;
}

static int sunxi_ahub_dam_tx_map_set(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct soc_enum *e = (struct soc_enum *)kcontrol->private_value;
	unsigned int shift = e->shift_l;
	struct regmap *regmap = g_ahub.mem.regmap;

	SND_LOG_DEBUG("\n");

	if (shift <= 47) {
		regmap_update_bits(regmap, ahub_dam_tx_map_reg_bit0[shift][0],
				   0xF << ahub_dam_tx_map_reg_bit0[shift][1],
				   ucontrol->value.integer.value[0] <<
				   ahub_dam_tx_map_reg_bit0[shift][1]);
	} else {
		regmap_update_bits(regmap, ahub_dam_tx_map_reg_bit1[shift - 48][0],
				   0xF << ahub_dam_tx_map_reg_bit1[shift - 48][1],
				   ucontrol->value.integer.value[0] <<
				   ahub_dam_tx_map_reg_bit1[shift - 48][1]);
	}

	return 0;
}

static const struct snd_kcontrol_new sunxi_ahub_route_controls[] = {
	SOC_ENUM_EXT("APBIF0 RX SRC SEL", ahub_route_mux_0,
		     sunxi_ahub_route_get, sunxi_ahub_route_set),
	SOC_ENUM_EXT("APBIF1 RX SRC SEL", ahub_route_mux_1,
		     sunxi_ahub_route_get, sunxi_ahub_route_set),
	SOC_ENUM_EXT("APBIF2 RX SRC SEL", ahub_route_mux_2,
		     sunxi_ahub_route_get, sunxi_ahub_route_set),
	SOC_ENUM_EXT("APBIF3 RX SRC SEL", ahub_route_mux_3,
		     sunxi_ahub_route_get, sunxi_ahub_route_set),
	SOC_ENUM_EXT("APBIF4 RX SRC SEL", ahub_route_mux_4,
		     sunxi_ahub_route_get, sunxi_ahub_route_set),
	SOC_ENUM_EXT("APBIF5 RX SRC SEL", ahub_route_mux_5,
		     sunxi_ahub_route_get, sunxi_ahub_route_set),
	SOC_ENUM_EXT("APBIF6 RX SRC SEL", ahub_route_mux_6,
		     sunxi_ahub_route_get, sunxi_ahub_route_set),
	SOC_ENUM_EXT("APBIF7 RX SRC SEL", ahub_route_mux_7,
		     sunxi_ahub_route_get, sunxi_ahub_route_set),

	SOC_ENUM_EXT("CODEC RX SRC SEL", ahub_route_mux_8,
		     sunxi_ahub_route_get, sunxi_ahub_route_set),
	SOC_ENUM_EXT("I2S0 RX SRC SEL", ahub_route_mux_9,
		     sunxi_ahub_route_get, sunxi_ahub_route_set),
	SOC_ENUM_EXT("I2S1 RX SRC SEL", ahub_route_mux_10,
		     sunxi_ahub_route_get, sunxi_ahub_route_set),
	SOC_ENUM_EXT("I2S2 RX SRC SEL", ahub_route_mux_11,
		     sunxi_ahub_route_get, sunxi_ahub_route_set),
	SOC_ENUM_EXT("I2S3 RX SRC SEL", ahub_route_mux_12,
		     sunxi_ahub_route_get, sunxi_ahub_route_set),
	SOC_ENUM_EXT("I2S4 RX SRC SEL", ahub_route_mux_13,
		     sunxi_ahub_route_get, sunxi_ahub_route_set),
	SOC_ENUM_EXT("OWA RX SRC SEL", ahub_route_mux_14,
		     sunxi_ahub_route_get, sunxi_ahub_route_set),

	SOC_ENUM_EXT("DAM0 RX SRC0 SEL", ahub_route_mux_15,
		     sunxi_ahub_route_get, sunxi_ahub_route_set),
	SOC_ENUM_EXT("DAM0 RX SRC1 SEL", ahub_route_mux_16,
		     sunxi_ahub_route_get, sunxi_ahub_route_set),
	SOC_ENUM_EXT("DAM0 RX SRC2 SEL", ahub_route_mux_17,
		     sunxi_ahub_route_get, sunxi_ahub_route_set),
	SOC_ENUM_EXT("DAM1 RX SRC0 SEL", ahub_route_mux_18,
		     sunxi_ahub_route_get, sunxi_ahub_route_set),
	SOC_ENUM_EXT("DAM1 RX SRC1 SEL", ahub_route_mux_19,
		     sunxi_ahub_route_get, sunxi_ahub_route_set),
	SOC_ENUM_EXT("DAM1 RX SRC2 SEL", ahub_route_mux_20,
		     sunxi_ahub_route_get, sunxi_ahub_route_set),
};

static const struct snd_kcontrol_new sunxi_ahub_dam_ch_controls[] = {
	SOC_ENUM_EXT("DAM0 TX CH SEL", ahub_dam_ch_mux_0,
		     sunxi_ahub_dam_ch_get, sunxi_ahub_dam_ch_set),
	SOC_ENUM_EXT("DAM0 RX0 CH SEL", ahub_dam_ch_mux_1,
		     sunxi_ahub_dam_ch_get, sunxi_ahub_dam_ch_set),
	SOC_ENUM_EXT("DAM0 RX1 CH SEL", ahub_dam_ch_mux_2,
		     sunxi_ahub_dam_ch_get, sunxi_ahub_dam_ch_set),
	SOC_ENUM_EXT("DAM0 RX2 CH SEL", ahub_dam_ch_mux_3,
		     sunxi_ahub_dam_ch_get, sunxi_ahub_dam_ch_set),
	SOC_ENUM_EXT("DAM1 TX CH SEL", ahub_dam_ch_mux_4,
		     sunxi_ahub_dam_ch_get, sunxi_ahub_dam_ch_set),
	SOC_ENUM_EXT("DAM1 RX0 CH SEL", ahub_dam_ch_mux_5,
		     sunxi_ahub_dam_ch_get, sunxi_ahub_dam_ch_set),
	SOC_ENUM_EXT("DAM1 RX1 CH SEL", ahub_dam_ch_mux_6,
		     sunxi_ahub_dam_ch_get, sunxi_ahub_dam_ch_set),
	SOC_ENUM_EXT("DAM1 RX2 CH SEL", ahub_dam_ch_mux_7,
		     sunxi_ahub_dam_ch_get, sunxi_ahub_dam_ch_set),
};

static const struct snd_kcontrol_new sunxi_ahub_dam_tx_map_controls[] = {
	SOC_ENUM_EXT("DAM0TXCH0 SRC0 Select", ahub_dam_map_mux_0, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH0 SRC1 Select", ahub_dam_map_mux_1, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH0 SRC2 Select", ahub_dam_map_mux_2, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH1 SRC0 Select", ahub_dam_map_mux_3, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH1 SRC1 Select", ahub_dam_map_mux_4, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH1 SRC2 Select", ahub_dam_map_mux_5, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH2 SRC0 Select", ahub_dam_map_mux_6, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH2 SRC1 Select", ahub_dam_map_mux_7, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH2 SRC2 Select", ahub_dam_map_mux_8, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH3 SRC0 Select", ahub_dam_map_mux_9, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH3 SRC1 Select", ahub_dam_map_mux_10, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH3 SRC2 Select", ahub_dam_map_mux_11, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH4 SRC0 Select", ahub_dam_map_mux_12, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH4 SRC1 Select", ahub_dam_map_mux_13, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH4 SRC2 Select", ahub_dam_map_mux_14, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH5 SRC0 Select", ahub_dam_map_mux_15, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH5 SRC1 Select", ahub_dam_map_mux_16, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH5 SRC2 Select", ahub_dam_map_mux_17, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH6 SRC0 Select", ahub_dam_map_mux_18, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH6 SRC1 Select", ahub_dam_map_mux_19, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH6 SRC2 Select", ahub_dam_map_mux_20, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH7 SRC0 Select", ahub_dam_map_mux_21, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH7 SRC1 Select", ahub_dam_map_mux_22, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH7 SRC2 Select", ahub_dam_map_mux_23, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH8 SRC0 Select", ahub_dam_map_mux_24, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH8 SRC1 Select", ahub_dam_map_mux_25, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH8 SRC2 Select", ahub_dam_map_mux_26, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH9 SRC0 Select", ahub_dam_map_mux_27, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH9 SRC1 Select", ahub_dam_map_mux_28, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH9 SRC2 Select", ahub_dam_map_mux_29, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH10 SRC0 Select", ahub_dam_map_mux_30, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH10 SRC1 Select", ahub_dam_map_mux_31, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH10 SRC2 Select", ahub_dam_map_mux_32, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH11 SRC0 Select", ahub_dam_map_mux_33, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH11 SRC1 Select", ahub_dam_map_mux_34, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH11 SRC2 Select", ahub_dam_map_mux_35, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH12 SRC0 Select", ahub_dam_map_mux_36, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH12 SRC1 Select", ahub_dam_map_mux_37, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH12 SRC2 Select", ahub_dam_map_mux_38, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH13 SRC0 Select", ahub_dam_map_mux_39, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH13 SRC1 Select", ahub_dam_map_mux_40, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH13 SRC2 Select", ahub_dam_map_mux_41, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH14 SRC0 Select", ahub_dam_map_mux_42, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH14 SRC1 Select", ahub_dam_map_mux_43, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH14 SRC2 Select", ahub_dam_map_mux_44, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH15 SRC0 Select", ahub_dam_map_mux_45, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH15 SRC1 Select", ahub_dam_map_mux_46, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM0TXCH15 SRC2 Select", ahub_dam_map_mux_47, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),

	SOC_ENUM_EXT("DAM1TXCH0 SRC0 Select", ahub_dam_map_mux_48, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH0 SRC1 Select", ahub_dam_map_mux_49, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH0 SRC2 Select", ahub_dam_map_mux_50, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH1 SRC0 Select", ahub_dam_map_mux_51, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH1 SRC1 Select", ahub_dam_map_mux_52, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH1 SRC2 Select", ahub_dam_map_mux_53, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH2 SRC0 Select", ahub_dam_map_mux_54, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH2 SRC1 Select", ahub_dam_map_mux_55, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH2 SRC2 Select", ahub_dam_map_mux_56, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH3 SRC0 Select", ahub_dam_map_mux_57, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH3 SRC1 Select", ahub_dam_map_mux_58, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH3 SRC2 Select", ahub_dam_map_mux_59, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH4 SRC0 Select", ahub_dam_map_mux_60, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH4 SRC1 Select", ahub_dam_map_mux_61, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH4 SRC2 Select", ahub_dam_map_mux_62, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH5 SRC0 Select", ahub_dam_map_mux_63, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH5 SRC1 Select", ahub_dam_map_mux_64, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH5 SRC2 Select", ahub_dam_map_mux_65, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH6 SRC0 Select", ahub_dam_map_mux_66, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH6 SRC1 Select", ahub_dam_map_mux_67, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH6 SRC2 Select", ahub_dam_map_mux_68, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH7 SRC0 Select", ahub_dam_map_mux_69, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH7 SRC1 Select", ahub_dam_map_mux_70, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH7 SRC2 Select", ahub_dam_map_mux_71, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH8 SRC0 Select", ahub_dam_map_mux_72, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH8 SRC1 Select", ahub_dam_map_mux_73, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH8 SRC2 Select", ahub_dam_map_mux_74, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH9 SRC0 Select", ahub_dam_map_mux_75, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH9 SRC1 Select", ahub_dam_map_mux_76, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH9 SRC2 Select", ahub_dam_map_mux_77, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH10 SRC0 Select", ahub_dam_map_mux_78, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH10 SRC1 Select", ahub_dam_map_mux_79, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH10 SRC2 Select", ahub_dam_map_mux_80, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH11 SRC0 Select", ahub_dam_map_mux_81, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH11 SRC1 Select", ahub_dam_map_mux_82, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH11 SRC2 Select", ahub_dam_map_mux_83, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH12 SRC0 Select", ahub_dam_map_mux_84, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH12 SRC1 Select", ahub_dam_map_mux_85, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH12 SRC2 Select", ahub_dam_map_mux_86, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH13 SRC0 Select", ahub_dam_map_mux_87, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH13 SRC1 Select", ahub_dam_map_mux_88, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH13 SRC2 Select", ahub_dam_map_mux_89, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH14 SRC0 Select", ahub_dam_map_mux_90, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH14 SRC1 Select", ahub_dam_map_mux_91, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH14 SRC2 Select", ahub_dam_map_mux_92, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH15 SRC0 Select", ahub_dam_map_mux_93, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH15 SRC1 Select", ahub_dam_map_mux_94, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
	SOC_ENUM_EXT("DAM1TXCH15 SRC2 Select", ahub_dam_map_mux_95, sunxi_ahub_dam_tx_map_get, sunxi_ahub_dam_tx_map_set),
};

/* just for sunxi_ahub_mgmt_config_init */
struct src_bit_port_table {
	unsigned int bit;
	enum SUNXI_AHUB_PORT port;
};
static struct src_bit_port_table src_bit_port_table[] = {
	{ 0x1 << NONE_TX,	SUNXI_AHUB_PORT_NONE },
	{ 0x1 << DAM1_TX,	SUNXI_AHUB_PORT_DAM1_TX },
	{ 0x1 << DAM0_TX,	SUNXI_AHUB_PORT_DAM0_TX },
	{ 0x1 << APBIF6_TX,	SUNXI_AHUB_PORT_APBIF6_TX },
	{ 0x1 << APBIF5_TX,	SUNXI_AHUB_PORT_APBIF5_TX },
	{ 0x1 << APBIF4_TX,	SUNXI_AHUB_PORT_APBIF4_TX },
	{ 0x1 << APBIF3_TX,	SUNXI_AHUB_PORT_APBIF3_TX },
	{ 0x1 << APBIF2_TX,	SUNXI_AHUB_PORT_APBIF2_TX },
	{ 0x1 << APBIF1_TX,	SUNXI_AHUB_PORT_APBIF1_TX },
	{ 0x1 << APBIF0_TX,	SUNXI_AHUB_PORT_APBIF0_TX },
	{ 0x1 << I2S4_TX,	SUNXI_AHUB_PORT_I2S4_TX },
	{ 0x1 << I2S3_TX,	SUNXI_AHUB_PORT_I2S3_TX },
	{ 0x1 << I2S2_TX,	SUNXI_AHUB_PORT_I2S2_TX },
	{ 0x1 << I2S1_TX,	SUNXI_AHUB_PORT_I2S1_TX },
	{ 0x1 << I2S0_TX,	SUNXI_AHUB_PORT_I2S0_TX },
	{ 0x1 << CODEC_TX,	SUNXI_AHUB_PORT_CODEC_TX },
	{ 0x1 << OWA_TX,	SUNXI_AHUB_PORT_OWA_TX },
	{ 0x1 << DMIC_TX,	SUNXI_AHUB_PORT_DMIC_TX },
};

static int sunxi_ahub_mgmt_config_init(void)
{
	struct regmap *regmap = g_ahub.mem.regmap;
	unsigned int src_reg1, src_reg2;
	unsigned int src_bit = 0;
	unsigned int i, j;

	SND_LOG_DEBUG("\n");

	/* init rx-port src */
	for (i = 0; i < ARRAY_SIZE(g_ahub.ahub_src); i++) {
		src_reg1 = ahub_route_mux_reg[i][0];
		src_reg2 = ahub_route_mux_reg[i][1];

		for (j = 0; j <= 17; j++) {
			if (g_ahub.ahub_src[i] == src_bit_port_table[j].port) {
				src_bit = src_bit_port_table[j].bit;
				if (g_ahub.ahub_src[i] > 14)
					regmap_update_bits(regmap, src_reg1, src_bit, src_bit);
				else
					regmap_update_bits(regmap, src_reg2, src_bit, src_bit);
				break;
			}
		}
	}

	/* enable dam reset/gate clk */
	regmap_update_bits(regmap, SUNXI_AHUB_SOFT_RST2,
			   0x1 << DAM0_SOFT_RST, 0x1 << DAM0_SOFT_RST);
	regmap_update_bits(regmap, SUNXI_AHUB_SOFT_RST2,
			   0x1 << DAM1_SOFT_RST, 0x1 << DAM1_SOFT_RST);
	regmap_update_bits(regmap, SUNXI_AHUB_CLK_GAT2, 0x1 << DAM0_GAT0, 0x1 << DAM0_GAT0);
	regmap_update_bits(regmap, SUNXI_AHUB_CLK_GAT2, 0x1 << DAM1_GAT0, 0x1 << DAM1_GAT0);

	/* init dam source */
	regmap_write(regmap, SUNXI_AHUB_DAM_RX0_CONT_SEL1(0), 0x80000000);
	regmap_write(regmap, SUNXI_AHUB_DAM_RX1_CONT_SEL1(0), 0x80000000);
	regmap_write(regmap, SUNXI_AHUB_DAM_RX2_CONT_SEL1(0), 0x80000000);
	regmap_write(regmap, SUNXI_AHUB_DAM_RX0_CONT_SEL1(1), 0x80000000);
	regmap_write(regmap, SUNXI_AHUB_DAM_RX1_CONT_SEL1(1), 0x80000000);
	regmap_write(regmap, SUNXI_AHUB_DAM_RX2_CONT_SEL1(1), 0x80000000);

	/* init dam tx/rx channel */
	regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(0), 0xF << DAM_TX_CHAN_NUM,
			   (g_ahub.dam_ch[0] - 1) << DAM_TX_CHAN_NUM);
	regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(0), 0xF << DAM_RX0_CHAN_NUM,
			   (g_ahub.dam_ch[1] - 1) << DAM_RX0_CHAN_NUM);
	regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(0), 0xF << DAM_RX1_CHAN_NUM,
			   (g_ahub.dam_ch[2] - 1) << DAM_RX1_CHAN_NUM);
	regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(0), 0xF << DAM_RX2_CHAN_NUM,
			   (g_ahub.dam_ch[3] - 1) << DAM_RX2_CHAN_NUM);
	regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(1), 0xF << DAM_TX_CHAN_NUM,
			   (g_ahub.dam_ch[4] - 1) << DAM_TX_CHAN_NUM);
	regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(1), 0xF << DAM_RX0_CHAN_NUM,
			   (g_ahub.dam_ch[5] - 1) << DAM_RX0_CHAN_NUM);
	regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(1), 0xF << DAM_RX1_CHAN_NUM,
			   (g_ahub.dam_ch[6] - 1) << DAM_RX1_CHAN_NUM);
	regmap_update_bits(regmap, SUNXI_AHUB_DAM_CTL(1), 0xF << DAM_RX2_CHAN_NUM,
			   (g_ahub.dam_ch[7] - 1) << DAM_RX2_CHAN_NUM);

	/* init dam tx ch map */
	for (i = 0; i < 16; i++) {
		for (j = 0; j < 3; j++) {
			regmap_update_bits(regmap, ahub_dam_tx_map_reg_bit0[i * 3 + j][0],
				   0xF << ahub_dam_tx_map_reg_bit0[i * 3 + j][1],
				   g_ahub.ch_map[i * 3 + j] <<
				   ahub_dam_tx_map_reg_bit0[i * 3 + j][1]);
			regmap_update_bits(regmap, ahub_dam_tx_map_reg_bit1[i * 3 + j][0],
				   0xF << ahub_dam_tx_map_reg_bit1[i * 3 + j][1],
				   g_ahub.ch_map[i * 3 + j + 48] <<
				   ahub_dam_tx_map_reg_bit1[i * 3 + j][1]);
		}
	}

	return 0;
}

static int sunxi_ahub_mgmt_probe(struct snd_soc_component *component)
{
	int ret;

	SND_LOG_DEBUG("\n");

	ret = snd_soc_add_component_controls(component, sunxi_ahub_route_controls,
					     ARRAY_SIZE(sunxi_ahub_route_controls));
	if (ret)
		SND_LOG_ERR("add ahub route kcontrols failed\n");

	ret = snd_soc_add_component_controls(component, sunxi_ahub_dam_ch_controls,
					     ARRAY_SIZE(sunxi_ahub_dam_ch_controls));
	if (ret)
		SND_LOG_ERR("add ahub dam ch kcontrols failed\n");

	ret = snd_soc_add_component_controls(component, sunxi_ahub_dam_tx_map_controls,
					     ARRAY_SIZE(sunxi_ahub_dam_tx_map_controls));
	if (ret)
		SND_LOG_ERR("add ahub dam tx map kcontrols failed\n");

	sunxi_ahub_mgmt_config_init();

	return 0;
}

static void sunxi_ahub_mgmt_remove(struct snd_soc_component *component)
{
	(void)component;
	SND_LOG_DEBUG("\n");
}

static int sunxi_ahub_mgmt_suspend(struct snd_soc_component *component)
{
	(void)component;

	SND_LOG_DEBUG("\n");

	snd_ahub_clk_disable(g_ahub.clk);
	snd_ahub_clk_bus_disable(g_ahub.clk);

	return 0;
}

static int sunxi_ahub_mgmt_resume(struct snd_soc_component *component)
{
	(void)component;
	int ret;

	SND_LOG_DEBUG("\n");

	ret = snd_ahub_clk_bus_enable(g_ahub.clk);
	if (ret) {
		SND_LOG_ERR("clk_bus and clk_rst enable failed\n");
		return ret;
	}

	ret = snd_ahub_clk_enable(g_ahub.clk);
	if (ret) {
		SND_LOG_ERR("clk_pll and clk_ahub enable failed\n");
		snd_ahub_clk_bus_disable(g_ahub.clk);
		return ret;
	}

	ret = snd_ahub_clk_rate(g_ahub.clk, 200000000, 200000000);
	if (ret) {
		SND_LOG_ERR("set rate failed\n");
		snd_ahub_clk_disable(g_ahub.clk);
		snd_ahub_clk_bus_disable(g_ahub.clk);
		return ret;
	}

	return 0;
}

static struct snd_soc_component_driver sunxi_ahub_mgmt_dev = {
	.name		= DRV_NAME,
	.probe		= sunxi_ahub_mgmt_probe,
	.remove		= sunxi_ahub_mgmt_remove,
	.suspend	= sunxi_ahub_mgmt_suspend,
	.resume		= sunxi_ahub_mgmt_resume,
};

static struct snd_soc_dai_driver sunxi_ahub_mgmt_dai = {
	.name			= DRV_NAME,
	/* In order to deal with the alsa framework layer inspection */
	.playback.channels_min	= 1,
	.capture.channels_min	= 1,
};

/* debug */
static struct audio_reg_label sunxi_global_reg_labels[] = {
	REG_LABEL(SUNXI_AHUB_SOFT_RST1),
	REG_LABEL(SUNXI_AHUB_SOFT_RST2),
	REG_LABEL(SUNXI_AHUB_CLK_GAT1),
	REG_LABEL(SUNXI_AHUB_CLK_GAT2),
	REG_LABEL(SUNXI_AHUB_CLK_GAT3),
	REG_LABEL(SUNXI_AHUB_IRQ_CTL1),
	REG_LABEL(SUNXI_AHUB_IRQ_CTL2),
	REG_LABEL(SUNXI_AHUB_IRQ_STS1),
	REG_LABEL(SUNXI_AHUB_IRQ_STS2),
	REG_LABEL(SUNXI_AHUB_APBIF_RX0_CONT_SEL1),
	REG_LABEL(SUNXI_AHUB_APBIF_RX0_CONT_SEL2),
	REG_LABEL(SUNXI_AHUB_APBIF_RX1_CONT_SEL1),
	REG_LABEL(SUNXI_AHUB_APBIF_RX1_CONT_SEL2),
	REG_LABEL(SUNXI_AHUB_APBIF_RX2_CONT_SEL1),
	REG_LABEL(SUNXI_AHUB_APBIF_RX2_CONT_SEL2),
	REG_LABEL(SUNXI_AHUB_APBIF_RX3_CONT_SEL1),
	REG_LABEL(SUNXI_AHUB_APBIF_RX3_CONT_SEL2),
	REG_LABEL(SUNXI_AHUB_APBIF_RX4_CONT_SEL1),
	REG_LABEL(SUNXI_AHUB_APBIF_RX4_CONT_SEL2),
	REG_LABEL(SUNXI_AHUB_APBIF_RX5_CONT_SEL1),
	REG_LABEL(SUNXI_AHUB_APBIF_RX5_CONT_SEL2),
	REG_LABEL(SUNXI_AHUB_APBIF_RX6_CONT_SEL1),
	REG_LABEL(SUNXI_AHUB_APBIF_RX6_CONT_SEL2),
	REG_LABEL(SUNXI_AHUB_APBIF_RX7_CONT_SEL1),
	REG_LABEL(SUNXI_AHUB_APBIF_RX7_CONT_SEL2),
	REG_LABEL(SUNXI_AHUB_ADDA_DAC_CONT_SEL1),
	REG_LABEL(SUNXI_AHUB_ADDA_DAC_CONT_SEL2),
	REG_LABEL(SUNXI_AHUB_I2S0_TX_CONT_SEL1),
	REG_LABEL(SUNXI_AHUB_I2S0_TX_CONT_SEL2),
	REG_LABEL(SUNXI_AHUB_I2S1_TX_CONT_SEL1),
	REG_LABEL(SUNXI_AHUB_I2S1_TX_CONT_SEL2),
	REG_LABEL(SUNXI_AHUB_I2S2_TX_CONT_SEL1),
	REG_LABEL(SUNXI_AHUB_I2S2_TX_CONT_SEL2),
	REG_LABEL(SUNXI_AHUB_I2S3_TX_CONT_SEL1),
	REG_LABEL(SUNXI_AHUB_I2S3_TX_CONT_SEL2),
	REG_LABEL(SUNXI_AHUB_I2S4_TX_CONT_SEL1),
	REG_LABEL(SUNXI_AHUB_I2S4_TX_CONT_SEL2),
	REG_LABEL(SUNXI_AHUB_OWA_TX_CONT_SEL1),
	REG_LABEL(SUNXI_AHUB_OWA_TX_CONT_SEL2),
	REG_LABEL(SUNXI_AHUB_VER),
};

#define APBIF_TX_REG(n) \
	REG_LABEL(SUNXI_AHUB_APBIF_TX_CTL(n)), \
	REG_LABEL(SUNXI_AHUB_APBIF_TX_IRQ_CTL(n)), \
	REG_LABEL(SUNXI_AHUB_APBIF_TX_IRQ_STS(n)), \
	REG_LABEL(SUNXI_AHUB_APBIF_TX_FIFO_CTL(n)), \
	REG_LABEL(SUNXI_AHUB_APBIF_TX_FIFO_STS(n)), \
	REG_LABEL(SUNXI_AHUB_APBIF_TX_FIFO_DATA(n)), \
	REG_LABEL(SUNXI_AHUB_APBIF_TX_FIFO_CNT(n)), \
	REG_LABEL(SUNXI_AHUB_APBIF_TX_FIFO_CNT_INIT(n)), \
	REG_LABEL(SUNXI_AHUB_APBIF_TX_P2S_FIFO_STS(n))

static struct audio_reg_label sunxi_apbif_tx_reg_labels[] = {
	APBIF_TX_REG(0),
	APBIF_TX_REG(1),
	APBIF_TX_REG(2),
	APBIF_TX_REG(3),
	APBIF_TX_REG(4),
	APBIF_TX_REG(5),
	APBIF_TX_REG(6),
};

#define APBIF_RX_REG(n) \
	REG_LABEL(SUNXI_AHUB_APBIF_RX_CTL(n)), \
	REG_LABEL(SUNXI_AHUB_APBIF_RX_IRQ_CTL(n)), \
	REG_LABEL(SUNXI_AHUB_APBIF_RX_IRQ_STS(n)), \
	REG_LABEL(SUNXI_AHUB_APBIF_RX_FIFO_CTL(n)), \
	REG_LABEL(SUNXI_AHUB_APBIF_RX_FIFO_STS(n)), \
	REG_LABEL(SUNXI_AHUB_APBIF_RX_FIFO_DATA(n)), \
	REG_LABEL(SUNXI_AHUB_APBIF_RX_FIFO_CNT(n)), \
	REG_LABEL(SUNXI_AHUB_APBIF_RX_FIFO_CNT_INIT(n)), \
	REG_LABEL(SUNXI_AHUB_APBIF_RX_S2P_FIFO_STS(n))

static struct audio_reg_label sunxi_apbif_rx_reg_labels[] = {
	APBIF_RX_REG(0),
	APBIF_RX_REG(1),
	APBIF_RX_REG(2),
	APBIF_RX_REG(3),
	APBIF_RX_REG(4),
	APBIF_RX_REG(5),
	APBIF_RX_REG(6),
	APBIF_RX_REG(7),
};

#define DAM_REG(n) \
	REG_LABEL(SUNXI_AHUB_DAM_CTL(n)), \
	REG_LABEL(SUNXI_AHUB_DAM_RX0_CONT_SEL1(n)), \
	REG_LABEL(SUNXI_AHUB_DAM_RX0_CONT_SEL2(n)), \
	REG_LABEL(SUNXI_AHUB_DAM_RX1_CONT_SEL1(n)), \
	REG_LABEL(SUNXI_AHUB_DAM_RX1_CONT_SEL2(n)), \
	REG_LABEL(SUNXI_AHUB_DAM_RX2_CONT_SEL1(n)), \
	REG_LABEL(SUNXI_AHUB_DAM_RX2_CONT_SEL2(n)), \
	REG_LABEL(SUNXI_AHUB_DAM_MIX_MAP0(n)), \
	REG_LABEL(SUNXI_AHUB_DAM_MIX_MAP1(n)), \
	REG_LABEL(SUNXI_AHUB_DAM_MIX_MAP2(n)), \
	REG_LABEL(SUNXI_AHUB_DAM_MIX_MAP3(n)), \
	REG_LABEL(SUNXI_AHUB_DAM_MIX_MAP4(n)), \
	REG_LABEL(SUNXI_AHUB_DAM_MIX_MAP5(n)), \
	REG_LABEL(SUNXI_AHUB_DAM_MIX_MAP6(n)), \
	REG_LABEL(SUNXI_AHUB_DAM_MIX_MAP7(n)), \
	REG_LABEL(SUNXI_AHUB_DAM_VOL_CTL0(n)), \
	REG_LABEL(SUNXI_AHUB_DAM_VOL_CTL1(n)), \
	REG_LABEL(SUNXI_AHUB_DAM_VOL_CTL2(n)), \
	REG_LABEL(SUNXI_AHUB_DAM_VOL_CTL3(n)), \
	REG_LABEL(SUNXI_AHUB_DAM_VOL_CTL4(n)), \
	REG_LABEL(SUNXI_AHUB_DAM_VOL_CTL5(n)), \
	REG_LABEL(SUNXI_AHUB_DAM_VOL_CTL6(n)), \
	REG_LABEL(SUNXI_AHUB_DAM_VOL_CTL7(n)), \
	REG_LABEL(SUNXI_AHUB_DAM_TX_CTL(n)), \
	REG_LABEL(SUNXI_AHUB_DAM_IRQ_CTL(n)), \
	REG_LABEL(SUNXI_AHUB_DAM_IRQ_STS(n)), \
	REG_LABEL(SUNXI_AHUB_DAM_RX0_FIFO_STS(n)), \
	REG_LABEL(SUNXI_AHUB_DAM_RX1_FIFO_STS(n)), \
	REG_LABEL(SUNXI_AHUB_DAM_RX2_FIFO_STS(n)), \
	REG_LABEL(SUNXI_AHUB_DAM_TX_FIFO_STS(n))

static struct audio_reg_label sunxi_dam_reg_labels[] = {
	DAM_REG(0),
	DAM_REG(1),
};

static struct audio_reg_group sunxi_reg_groups[] = {
	REG_GROUP(sunxi_global_reg_labels),
	REG_GROUP(sunxi_apbif_tx_reg_labels),
	REG_GROUP(sunxi_apbif_rx_reg_labels),
	REG_GROUP(sunxi_dam_reg_labels),
};

#if IS_ENABLED(CONFIG_SND_SOC_SUNXI_DEBUG)
/* sysfs debug */
static void snd_sunxi_dump_version(void *priv, char *buf, size_t *count)
{
	struct platform_device *pdev = g_ahub.pdev;
	size_t count_tmp = 0;

	if (pdev)
		if (pdev->dev.driver)
			if (pdev->dev.driver->owner)
				goto module_version;
	return;

module_version:
	count_tmp += sprintf(buf + count_tmp, "%s\n", pdev->dev.driver->owner->version);
	*count = count_tmp;
}

static void snd_sunxi_dump_help(void *priv, char *buf, size_t *count)
{
	size_t count_tmp = 0;

	count_tmp += sprintf(buf + count_tmp, "1. reg read : echo {num} > dump && cat dump\n");
	count_tmp += sprintf(buf + count_tmp, "num:\n");
	count_tmp += sprintf(buf + count_tmp, "0(all) 1(global) 2(apbif-tx) 3(apbif-rx) 4(dam)\n");
	count_tmp += sprintf(buf + count_tmp, "eg. echo 0x1 > dump\n");
	count_tmp += sprintf(buf + count_tmp, "2. reg write: echo {reg} {value} > dump\n");
	count_tmp += sprintf(buf + count_tmp, "eg. echo 0x00 0xaa > dump\n");

	*count = count_tmp;
}

static int snd_sunxi_dump_show(void *priv, char *buf, size_t *count)
{
	struct regmap *regmap = g_ahub.mem.regmap;
	int reg_domain = g_ahub.show_reg_num;
	unsigned int output_reg_val;
	size_t count_tmp = 0;
	unsigned int i, j;

	if (reg_domain < 0 || reg_domain > 4)
		return 0;
	else
		g_ahub.show_reg_num = -1;

	/* show specify domain reg */
	if (reg_domain != 0) {
		for (i = 0; i < sunxi_reg_groups[reg_domain - 1].size; ++i) {
			regmap_read(regmap, sunxi_reg_groups[reg_domain - 1].label[i].address,
				    &output_reg_val);
			count_tmp += sprintf(buf + count_tmp, "[0x%03x]: 0x%8x\n",
					     sunxi_reg_groups[reg_domain - 1].label[i].address,
					     output_reg_val);
		}
		goto end;
	}

	/* show all domain reg */
	for (i = 0; i < ARRAY_SIZE(sunxi_reg_groups); ++i) {
		for (j = 0; j < sunxi_reg_groups[i].size; ++j) {
			regmap_read(regmap, sunxi_reg_groups[i].label[j].address, &output_reg_val);
			count_tmp += sprintf(buf + count_tmp, "[0x%03x]: 0x%8x\n",
				     sunxi_reg_groups[i].label[j].address, output_reg_val);
		}
		count_tmp += sprintf(buf + count_tmp, "\n");
	}

end:
	*count = count_tmp;
	return 0;
}

static int snd_sunxi_dump_store(void *priv, const char *buf, size_t count)
{
	struct regmap *regmap = g_ahub.mem.regmap;
	unsigned int scanf_cnt;
	unsigned int val1, val2, val3;

	if (count <= 1)	/* null or only "\n" */
		return 0;

	scanf_cnt = sscanf(buf, "0x%x 0x%x", &val1, &val2);
	if (scanf_cnt == 1 && val1 <= 4 && val1 >= 0) {
		g_ahub.show_reg_num = val1;
	} else if (scanf_cnt == 2) {
		regmap_read(regmap, val1, &val3);
		SND_LOG_INFO("reg[0x%03x]: 0x%x (old)\n", val1, val3);
		regmap_write(regmap, val1, val2);
		regmap_read(regmap, val1, &val3);
		SND_LOG_INFO("reg[0x%03x]: 0x%x (new)\n", val1, val3);
	} else {
		SND_LOG_ERR("wrong format: %s\n", buf);
		return -1;
	}

	return 0;
}
#endif

static struct regmap_config sunxi_regmap_config = {
	.reg_bits	= 32,
	.reg_stride	= 4,
	.val_bits	= 32,
	.max_register	= SUNXI_AHUB_MAX_REG,
	.cache_type	= REGCACHE_NONE,
};

static int snd_sunxi_mem_init(struct platform_device *pdev, struct sunxi_ahub_mgmt_mem *mem)
{
	int ret = 0;
	struct device_node *np = pdev->dev.of_node;

	SND_LOG_DEBUG("\n");

	ret = of_address_to_resource(np, 0, &mem->res);
	if (ret) {
		SND_LOG_ERR("parse device node resource failed\n");
		ret = -EINVAL;
		goto err_of_addr_to_resource;
	}

	mem->memregion = devm_request_mem_region(&pdev->dev, mem->res.start,
						 resource_size(&mem->res),
						 DRV_NAME);
	if (IS_ERR_OR_NULL(mem->memregion)) {
		SND_LOG_ERR("memory region already claimed\n");
		ret = -EBUSY;
		goto err_devm_request_region;
	}

	mem->membase = devm_ioremap(&pdev->dev, mem->memregion->start,
				    resource_size(mem->memregion));
	if (IS_ERR_OR_NULL(mem->membase)) {
		SND_LOG_ERR("ioremap failed\n");
		ret = -EBUSY;
		goto err_devm_ioremap;
	}

	mem->regmap = devm_regmap_init_mmio(&pdev->dev, mem->membase, &sunxi_regmap_config);
	if (IS_ERR_OR_NULL(mem->regmap)) {
		SND_LOG_ERR("regmap init failed\n");
		ret = -EINVAL;
		goto err_devm_regmap_init;
	}

	return 0;

err_devm_regmap_init:
	devm_iounmap(&pdev->dev, mem->membase);
err_devm_ioremap:
	devm_release_mem_region(&pdev->dev, mem->memregion->start, resource_size(mem->memregion));
err_devm_request_region:
err_of_addr_to_resource:
	return ret;
};

static void snd_sunxi_mem_exit(struct platform_device *pdev, struct sunxi_ahub_mgmt_mem *mem)
{
	SND_LOG_DEBUG("\n");

	devm_iounmap(&pdev->dev, mem->membase);
	devm_release_mem_region(&pdev->dev, mem->memregion->start, resource_size(mem->memregion));
}

/* just for snd_sunxi_dts_parse */
struct src_str_table {
	char *src_name;
	enum SUNXI_AHUB_PORT port;
};
struct src_str_table src_str_table[] = {
	{ "none",	SUNXI_AHUB_PORT_NONE },

	{ "codec-tx",	SUNXI_AHUB_PORT_CODEC_TX },
	{ "i2s0-tx",	SUNXI_AHUB_PORT_I2S0_TX },
	{ "i2s1-tx",	SUNXI_AHUB_PORT_I2S1_TX },
	{ "i2s2-tx",	SUNXI_AHUB_PORT_I2S2_TX },
	{ "i2s3-tx",	SUNXI_AHUB_PORT_I2S3_TX },
	{ "i2s4-tx",	SUNXI_AHUB_PORT_I2S4_TX },
	{ "owa-tx",	SUNXI_AHUB_PORT_OWA_TX },
	{ "dmic-tx",	SUNXI_AHUB_PORT_DMIC_TX },

	{ "apbif0-tx",	SUNXI_AHUB_PORT_APBIF0_TX },
	{ "apbif1-tx",	SUNXI_AHUB_PORT_APBIF1_TX },
	{ "apbif2-tx",	SUNXI_AHUB_PORT_APBIF2_TX },
	{ "apbif3-tx",	SUNXI_AHUB_PORT_APBIF3_TX },
	{ "apbif4-tx",	SUNXI_AHUB_PORT_APBIF4_TX },
	{ "apbif5-tx",	SUNXI_AHUB_PORT_APBIF5_TX },
	{ "apbif6-tx",	SUNXI_AHUB_PORT_APBIF6_TX },
};

static void sunxi_rx_src_parse(struct device_node *np, char *attr_str,
			       enum SUNXI_AHUB_PORT *port, enum SUNXI_AHUB_PORT default_val)
{
	const char *str;
	unsigned int i;
	int ret;

	ret = of_property_read_string(np, attr_str, &str);
	if (ret == 0) {
		for (i = 0; i < ARRAY_SIZE(src_str_table); i++) {
			if (strcmp(str, src_str_table[i].src_name) == 0) {
				*port = src_str_table[i].port;
				return;
			}
		}
	}

	*port = default_val;
	SND_LOG_DEBUG("%s val missing or invalid, use default val\n", attr_str);
}

static void sunxi_dam_ch_parse(struct device_node *np, char *attr_str,
			       unsigned int *ch, unsigned int default_val)
{
	unsigned int temp_val;
	int ret;

	ret = of_property_read_u32(np, attr_str, &temp_val);
	if (ret == 0 && temp_val <= 16) {
		*ch = temp_val;
		return;
	}

	*ch = default_val;
	SND_LOG_DEBUG("%s val missing or invalid, use default val\n", attr_str);
}

static void sunxi_dam_ch_map_parse(struct device_node *np, char *attr_str,
				   unsigned int *ch_map, unsigned int default_val)
{
	unsigned int temp_val;
	int ret;

	ret = of_property_read_u32(np, attr_str, &temp_val);
	if (ret == 0 && temp_val <= 15) {
		*ch_map = temp_val;
		return;
	}

	*ch_map = default_val;
	SND_LOG_DEBUG("%s val missing or invalid, use default val\n", attr_str);
}

static void snd_sunxi_dts_parse(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	char *src_name[21];
	char str[32];
	unsigned int i, j, k;

	SND_LOG_DEBUG("\n");

	/* parse rx src */
	sunxi_rx_src_parse(np, "apbif0-rx-src", &g_ahub.ahub_src[0], SUNXI_AHUB_PORT_CODEC_TX);
	sunxi_rx_src_parse(np, "apbif1-rx-src", &g_ahub.ahub_src[1], SUNXI_AHUB_PORT_I2S0_TX);
	sunxi_rx_src_parse(np, "apbif2-rx-src", &g_ahub.ahub_src[2], SUNXI_AHUB_PORT_I2S1_TX);
	sunxi_rx_src_parse(np, "apbif3-rx-src", &g_ahub.ahub_src[3], SUNXI_AHUB_PORT_I2S2_TX);
	sunxi_rx_src_parse(np, "apbif4-rx-src", &g_ahub.ahub_src[4], SUNXI_AHUB_PORT_I2S3_TX);
	sunxi_rx_src_parse(np, "apbif5-rx-src", &g_ahub.ahub_src[5], SUNXI_AHUB_PORT_I2S4_TX);
	sunxi_rx_src_parse(np, "apbif6-rx-src", &g_ahub.ahub_src[6], SUNXI_AHUB_PORT_OWA_TX);
	sunxi_rx_src_parse(np, "apbif7-rx-src", &g_ahub.ahub_src[7], SUNXI_AHUB_PORT_DMIC_TX);

	sunxi_rx_src_parse(np, "codec-rx-src", &g_ahub.ahub_src[8], SUNXI_AHUB_PORT_APBIF0_TX);
	sunxi_rx_src_parse(np, "i2s0-rx-src", &g_ahub.ahub_src[9], SUNXI_AHUB_PORT_APBIF1_TX);
	sunxi_rx_src_parse(np, "i2s1-rx-src", &g_ahub.ahub_src[10], SUNXI_AHUB_PORT_APBIF2_TX);
	sunxi_rx_src_parse(np, "i2s2-rx-src", &g_ahub.ahub_src[11], SUNXI_AHUB_PORT_APBIF3_TX);
	sunxi_rx_src_parse(np, "i2s3-rx-src", &g_ahub.ahub_src[12], SUNXI_AHUB_PORT_APBIF4_TX);
	sunxi_rx_src_parse(np, "i2s4-rx-src", &g_ahub.ahub_src[13], SUNXI_AHUB_PORT_APBIF5_TX);
	sunxi_rx_src_parse(np, "owa-rx-src", &g_ahub.ahub_src[14], SUNXI_AHUB_PORT_APBIF6_TX);

	sunxi_rx_src_parse(np, "dam0-rx0-src", &g_ahub.ahub_src[15], SUNXI_AHUB_PORT_NONE);
	sunxi_rx_src_parse(np, "dam0-rx1-src", &g_ahub.ahub_src[16], SUNXI_AHUB_PORT_NONE);
	sunxi_rx_src_parse(np, "dam0-rx2-src", &g_ahub.ahub_src[17], SUNXI_AHUB_PORT_NONE);
	sunxi_rx_src_parse(np, "dam1-rx0-src", &g_ahub.ahub_src[18], SUNXI_AHUB_PORT_NONE);
	sunxi_rx_src_parse(np, "dam1-rx1-src", &g_ahub.ahub_src[19], SUNXI_AHUB_PORT_NONE);
	sunxi_rx_src_parse(np, "dam1-rx2-src", &g_ahub.ahub_src[20], SUNXI_AHUB_PORT_NONE);

	/* parse tx/rx channels */
	sunxi_dam_ch_parse(np, "dam0-tx-ch", &g_ahub.dam_ch[0], 2);
	sunxi_dam_ch_parse(np, "dam0-rx0-ch", &g_ahub.dam_ch[1], 2);
	sunxi_dam_ch_parse(np, "dam0-rx1-ch", &g_ahub.dam_ch[2], 2);
	sunxi_dam_ch_parse(np, "dam0-rx2-ch", &g_ahub.dam_ch[3], 2);
	sunxi_dam_ch_parse(np, "dam1-tx-ch", &g_ahub.dam_ch[4], 2);
	sunxi_dam_ch_parse(np, "dam1-rx0-ch", &g_ahub.dam_ch[5], 2);
	sunxi_dam_ch_parse(np, "dam1-rx1-ch", &g_ahub.dam_ch[6], 2);
	sunxi_dam_ch_parse(np, "dam1-rx2-ch", &g_ahub.dam_ch[7], 2);

	/* parse tx channel map */
	for (i = 0; i < 16; i++) {
		for (j = 0; j < 3; j++) {
			sprintf(str, "dam0-tx-ch%u-src%u-map", i, j);
			sunxi_dam_ch_map_parse(np, str, &g_ahub.ch_map[i * 3 + j], i);

			sprintf(str, "dam1-tx-ch%u-src%u-map", i, j);
			sunxi_dam_ch_map_parse(np, str, &g_ahub.ch_map[i * 3 + j + 48], i);
		}
	}

	/* param print */
	for (i = 0; i < ARRAY_SIZE(g_ahub.ahub_src); i++) {
		for (j = 0; j < ARRAY_SIZE(src_str_table); j++) {
			if (g_ahub.ahub_src[i] == src_str_table[j].port) {
				src_name[i] = src_str_table[j].src_name;
				break;
			}
		}
	}

	SND_LOG_DEBUG("apbif0-rx-src:%s\n", src_name[0]);
	SND_LOG_DEBUG("apbif1-rx-src:%s\n", src_name[1]);
	SND_LOG_DEBUG("apbif2-rx-src:%s\n", src_name[2]);
	SND_LOG_DEBUG("apbif3-rx-src:%s\n", src_name[3]);
	SND_LOG_DEBUG("apbif4-rx-src:%s\n", src_name[4]);
	SND_LOG_DEBUG("apbif5-rx-src:%s\n", src_name[5]);
	SND_LOG_DEBUG("apbif6-rx-src:%s\n", src_name[6]);
	SND_LOG_DEBUG("apbif7-rx-src:%s\n", src_name[7]);
	SND_LOG_DEBUG("codec-rx-src:%s\n", src_name[8]);
	SND_LOG_DEBUG("i2s0-rx-src:%s\n", src_name[9]);
	SND_LOG_DEBUG("i2s1-rx-src:%s\n", src_name[10]);
	SND_LOG_DEBUG("i2s2-rx-src:%s\n", src_name[11]);
	SND_LOG_DEBUG("i2s3-rx-src:%s\n", src_name[12]);
	SND_LOG_DEBUG("i2s4-rx-src:%s\n", src_name[13]);
	SND_LOG_DEBUG("owa-rx-src:%s\n", src_name[14]);
	SND_LOG_DEBUG("dam0-rx0-src:%s\n", src_name[15]);
	SND_LOG_DEBUG("dam0-rx1-src:%s\n", src_name[16]);
	SND_LOG_DEBUG("dam0-rx2-src:%s\n", src_name[17]);
	SND_LOG_DEBUG("dam1-rx0-src:%s\n", src_name[18]);
	SND_LOG_DEBUG("dam1-rx1-src:%s\n", src_name[19]);
	SND_LOG_DEBUG("dam1-rx2-src:%s\n", src_name[20]);

	SND_LOG_DEBUG("dam0-tx-ch:%u\n", g_ahub.dam_ch[0]);
	SND_LOG_DEBUG("dam0-rx0-ch:%u\n", g_ahub.dam_ch[1]);
	SND_LOG_DEBUG("dam0-rx1-ch:%u\n", g_ahub.dam_ch[2]);
	SND_LOG_DEBUG("dam0-rx2-ch:%u\n", g_ahub.dam_ch[3]);
	SND_LOG_DEBUG("dam1-tx-ch:%u\n", g_ahub.dam_ch[4]);
	SND_LOG_DEBUG("dam1-rx0-ch:%u\n", g_ahub.dam_ch[5]);
	SND_LOG_DEBUG("dam1-rx1-ch:%u\n", g_ahub.dam_ch[6]);
	SND_LOG_DEBUG("dam1-rx2-ch:%u\n", g_ahub.dam_ch[7]);

	for (i = 0; i < 2; i++) {
		for (j = 0; j < 16; j++) {
			for (k = 0; k < 3; k++) {
				sprintf(str, "dam%u-tx-ch%u-src%u-map", i, j, k);
				SND_LOG_DEBUG("%s:%u\n", str, g_ahub.ch_map[i * 16 * 3 + j * 3 + k]);
			}
		}
	}
}

static int sunxi_ahub_mgmt_dev_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct sunxi_ahub_mgmt_mem *mem = &g_ahub.mem;
	int ret;

	SND_LOG_DEBUG("\n");

	/* mem init */
	ret = snd_sunxi_mem_init(pdev, mem);
	if (ret) {
		SND_LOG_ERR("remap init failed\n");
		ret = -EINVAL;
		goto err_snd_sunxi_mem_init;
	}

	/* dts parse */
	snd_sunxi_dts_parse(pdev);

	/* clk init */
	g_ahub.clk = snd_ahub_clk_init(pdev);
	if (!g_ahub.clk) {
		SND_LOG_ERR("clk init failed\n");
		ret = -EINVAL;
		goto err_snd_ahub_clk_init;
	}
	ret = snd_ahub_clk_bus_enable(g_ahub.clk);
	if (ret) {
		SND_LOG_ERR("clk_bus and clk_rst enable failed\n");
		ret = -EINVAL;
		goto err_snd_ahub_clk_bus_enable;
	}
	ret = snd_ahub_clk_enable(g_ahub.clk);
	if (ret) {
		SND_LOG_ERR("clk_pll and clk_ahub enable failed\n");
		ret = -EINVAL;
		goto err_snd_ahub_clk_enable;
	}

	ret = snd_ahub_clk_rate(g_ahub.clk, 200000000, 200000000);
	if (ret) {
		SND_LOG_ERR("set rate failed\n");
		ret = -EINVAL;
		goto err_snd_ahub_clk_rate;
	}

	ret = snd_soc_register_component(&pdev->dev, &sunxi_ahub_mgmt_dev, &sunxi_ahub_mgmt_dai, 1);
	if (ret) {
		SND_LOG_ERR("component register failed\n");
		ret = -ENOMEM;
		goto err_snd_soc_register_component;
	}

	mutex_init(&g_ahub.ahub_lock);

#if IS_ENABLED(CONFIG_SND_SOC_SUNXI_DEBUG)
	snprintf(g_ahub.module_name, 32, "%s", "AudioHub");
	g_ahub.dump.name = g_ahub.module_name;
	g_ahub.dump.priv = &g_ahub;
	g_ahub.dump.dump_version = snd_sunxi_dump_version;
	g_ahub.dump.dump_help = snd_sunxi_dump_help;
	g_ahub.dump.dump_show = snd_sunxi_dump_show;
	g_ahub.dump.dump_store = snd_sunxi_dump_store;
	ret = snd_sunxi_dump_register(&g_ahub.dump);
	if (ret)
		SND_LOG_WARN("snd_sunxi_dump_register failed\n");
#endif

	SND_LOG_INFO("register ahub_mgmt platform success\n");

	return 0;

err_snd_soc_register_component:
err_snd_ahub_clk_rate:
	snd_ahub_clk_disable(g_ahub.clk);
err_snd_ahub_clk_enable:
	snd_ahub_clk_bus_disable(g_ahub.clk);
err_snd_ahub_clk_bus_enable:
	snd_ahub_clk_exit(g_ahub.clk);
err_snd_ahub_clk_init:
	snd_sunxi_mem_exit(pdev, mem);
err_snd_sunxi_mem_init:
	of_node_put(np);
	return ret;
}

static int sunxi_ahub_mgmt_dev_remove(struct platform_device *pdev)
{
	struct sunxi_ahub_mgmt_mem *mem = &g_ahub.mem;

#if IS_ENABLED(CONFIG_SND_SOC_SUNXI_DEBUG)
	snd_sunxi_dump_unregister(&g_ahub.dump);
#endif

	mutex_destroy(&g_ahub.ahub_lock);

	snd_sunxi_mem_exit(pdev, mem);
	snd_ahub_clk_disable(g_ahub.clk);
	snd_ahub_clk_bus_disable(g_ahub.clk);
	snd_ahub_clk_exit(g_ahub.clk);

	return 0;
}

static const struct of_device_id snd_ahub_mgmt_of_match[] = {
	{ .compatible = "allwinner," DRV_NAME, },
	{},
};
MODULE_DEVICE_TABLE(of, snd_ahub_mgmt_of_match);

static struct platform_driver sunxi_ahub_mgmt_driver = {
	.driver	= {
		.name		= DRV_NAME,
		.owner		= THIS_MODULE,
		.of_match_table	= snd_ahub_mgmt_of_match,
	},
	.probe	= sunxi_ahub_mgmt_dev_probe,
	.remove	= sunxi_ahub_mgmt_dev_remove,
};

int __init sunxi_ahub_mgmt_dev_init(void)
{
	int ret;

	ret = platform_driver_register(&sunxi_ahub_mgmt_driver);
	if (ret != 0) {
		SND_LOG_ERR("platform driver register failed\n");
		return -EINVAL;
	}

	return ret;
}

void __exit sunxi_ahub_mgmt_dev_exit(void)
{
	platform_driver_unregister(&sunxi_ahub_mgmt_driver);
}

late_initcall(sunxi_ahub_mgmt_dev_init);
module_exit(sunxi_ahub_mgmt_dev_exit);

MODULE_AUTHOR("huhaoxin@allwinnertech.com");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0.0");
MODULE_DESCRIPTION("sunxi soundcard of audio ahub mgmt");
