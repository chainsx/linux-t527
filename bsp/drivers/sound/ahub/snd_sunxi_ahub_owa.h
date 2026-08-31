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

#ifndef __SND_SUNXI_AHUB_OWA_H
#define __SND_SUNXI_AHUB_OWA_H

#include "snd_sunxi_pcm.h"
#include "snd_sunxi_rxsync.h"
#include "snd_sunxi_common.h"
#include "snd_sunxi_ahub_mgmt.h"

#define	SUNXI_OWA_CTL			0x00
#define	SUNXI_OWA_TXCFG			0x04
#define	SUNXI_OWA_RXCFG			0x08
#define SUNXI_OWA_INT_STA		0x0C
#define	SUNXI_OWA_FIFO_CTL		0x14
#define	SUNXI_OWA_FIFO_STA		0x18
#define	SUNXI_OWA_INT			0x1C
#define SUNXI_OWA_CHAN_CFG		0x20
#define	SUNXI_OWA_TXCNT			0x24
#define	SUNXI_OWA_RXCNT			0x28
#define	SUNXI_OWA_TXCH_STA0		0x2C
#define	SUNXI_OWA_TXCH_STA1		0x30
#define	SUNXI_OWA_RXCH_STA0		0x34
#define	SUNXI_OWA_RXCH_STA1		0x38
#define	SUNXI_OWA_INS_REDET_CTL		0x3C
#define	SUNXI_OWA_EXP_CTL		0x40
#define	SUNXI_OWA_EXP_ISTA		0x44
#define	SUNXI_OWA_EXP_INFO0		0x48
#define	SUNXI_OWA_EXP_INFO1		0x4C
#define	SUNXI_OWA_EXP_DBG0		0x50
#define	SUNXI_OWA_EXP_DBG1		0x54
#define	SUNXI_OWA_EXP_VER		0x58
#define	SUNXI_OWA_EXP_CDR_DBG		0x5C
#define SUNXI_OWA_REG_MAX		SUNXI_OWA_EXP_CDR_DBG

/* BIT-OWA */
/* SUNXI_OWA_CTL */
#define	CTL_RST_RX			12
#define CTL_MCLKDIV			5
#define CTL_MCLKEN			3
#define	CTL_LOOP_EN			2
#define	CTL_GEN_EN			1
#define	CTL_RST_TX			0
/* SUNXI_OWA_TXCFG */
#define	TXCFG_SINGLE_MOD		31
#define	TXCFG_NO_WAIT_EN		18
#define	TXCFG_ASS			17
#define	TXCFG_DATA_TYPE			16
#define	TXCFG_CLK_DIV_RATIO		4
#define	TXCFG_SAMPLE_BIT		2
#define	TXCFG_CHAN_STA_EN		1
#define	TXCFG_TXEN			0
/* SUNXI_OWA_RXCFG */
#define	RXCFG_PD_NUM_MOD_EN		16
#define	RXCFG_PD_NUM_MOD		8
#define	RXCFG_CDR_MOD			5
#define	RXCFG_LOCK_FLAG			4
#define	RXCFG_CHST_SRC			3
#define	RXCFG_CDR_PREAMBLE		2
#define	RXCFG_CHST_CP			1
#define	RXCFG_RXEN			0
/* SUNXI_OWA_INT_STA */
#define	INT_STA_RXLOCK			18
#define	INT_STA_RXUNLOCK		17
#define	INT_STA_RXPAR			16
#define	INT_STA_TXU			6
#define	INT_STA_TXO			5
#define	INT_STA_RXO			1
#define	INT_STA_RXU			0
/* SUNXI_OWA_FIFO_CTL */
#define	OWA_RX_SYNC_EN_START		21
#define	OWA_RX_SYNC_EN			20
#define	FIFO_CTL_TXIM			2
#define	FIFO_CTL_RXOM			0
/* SUNXI_OWA_FIFO_STA */
#define	FIFO_STA_TXF			31
#define	FIFO_STA_TXE			30
#define	FIFO_STA_TX_WL			16
#define	FIFO_STA_RXF			15
#define	FIFO_STA_RXE			14
#define	FIFO_STA_RX_WL			0
/* SUNXI_OWA_INT */
#define	INT_RXLOCKEN			18
#define	INT_RXUNLOCKEN			17
#define	INT_RXPAREN			16
#define	INT_TXUIEN			6
#define	INT_TXOIEN			5
#define	INT_RXOIEN			1
#define	INT_RXUIEN			0
/* SUNXI_OWA_CHAN_CFG */
#define	CHANCFG_TX_CHAN_NUM		4
#define	CHANCFG_RX_CHAN_NUM		0
/* SUNXI_OWA_TXCNT */
#define	TX_CNT				0
/* SUNXI_OWA_RXCNT */
#define	RX_CNT				0
/* SUNXI_OWA_TXCH_STA0 */
#define	TXCHSTA0_CLK			28
#define	TXCHSTA0_SAMFREQ		24
#define	TXCHSTA0_CHNUM			20
#define	TXCHSTA0_SRCNUM			16
#define	TXCHSTA0_CATECODE		8
#define	TXCHSTA0_MODE			6
#define	TXCHSTA0_EMPHASIS		3
#define	TXCHSTA0_CP			2
#define	TXCHSTA0_DATA_TYPE		1
#define	TXCHSTA0_PRO			0
/* SUNXI_OWA_TXCH_STA1 */
#define	TXCHSTA1_CGMSA			8
#define	TXCHSTA1_ORISAMFREQ		4
#define	TXCHSTA1_SAMWORDLEN		1
#define	TXCHSTA1_MAXWORDLEN		0
/* SUNXI_OWA_RXCH_STA0 */
#define	RXCHSTA0_CLK			28
#define	RXCHSTA0_SAMFREQ		24
#define	RXCHSTA0_CHNUM			20
#define	RXCHSTA0_SRCNUM			16
#define	RXCHSTA0_CATECODE		8
#define	RXCHSTA0_MODE			6
#define	RXCHSTA0_EMPHASIS		3
#define	RXCHSTA0_CP			2
#define	RXCHSTA0_DATA_TYPE		1
#define	RXCHSTA0_PRO			0
/* SUNXI_OWA_RXCH_STA1 */
#define	RXCHSTA1_CGMSA			8
#define	RXCHSTA1_ORISAMFREQ		4
#define	RXCHSTA1_SAMWORDLEN		1
#define	RXCHSTA1_MAXWORDLEN		0
/* SUNXI_OWA_INS_REDET_CTL */
#define	INSREDET_MONITOR_EN		8
#define	INSREDET_MONITOR_LEN		0
/* SUNXI_OWA_EXP_CTL */
#define BURST_DATA_OUT_SEL		30
#define RPOTBF_NUM			16
#define UNIT_SEL			15
#define RX_MODE_MAN			14
#define RX_MODE				13
#define AUDIO_DATA_BIT_EN		12
#define DATA_LEG_BIT_EN			11
#define DATA_TYPE_BIT_EN		10
#define SYNCW_BIT_EN			9
#define INSET_DET_EN			8
#define INSET_DET_NUM			0
/* SUNXI_OWA_EXP_ISTA */
#define PD_CHAN_INT_EN			24
#define PC_PAUSE_STOP_INT_EN		23
#define PC_BIT_CH_INT_EN		22
#define PC_ERR_FLAG_INT_EN		21
#define PC_DTYOE_CH_INT_EN		20
#define RPDB_ERR_INT_EN			19
#define PCPD_CAP_INT_EN			18
#define PAPB_CAP_INT_EN			17
#define INSET_INT_EN			16
#define PD_CHAN_INT			8
#define PC_PAUSE_STOP_INT		7
#define PC_BIT_CH_INT			6
#define PC_ERR_FLAG_INT			5
#define PC_DTYOE_CH_INT			4
#define RPDB_ERR_INT			3
#define PCPD_CAP_INT			2
#define PAPB_CAP_INT			1
#define INSET_INT			0
/* SUNXI_OWA_EXP_INFO0 */
#define PC_DATA_INFO			16
#define PD_DATA_INFO			0
/* SUNXI_OWA_EXP_INFO1 */
#define RPOTBF_VAL			16
#define SAMPLE_RATE_VAL			0
/* SUNXI_OWA_EXP_DBG0 */
#define DATA_CAP_STA_MACHE		16
#define RE_DATA_COUNT_VAL		0
/* SUNXI_OWA_EXP_DBG1 */
#define RPOTBF_COUNT			16
#define SAMPLE_RATE_COUNT		0
/* SUNXI_OWA_EXP_VER */
#define VER				0
/* SUNXI_OWA_EXP_CDR_DBG */
#define CDR_STATE			28
#define PERIOD_MAX			16
#define PERIOD_MIN			0

/* Mask and default val */
#define	CTL_TXTL_MASK			0xFF
#define	CTL_TXTL_DEFAULT		0x40
#define	CTL_RXTL_MASK			0x7F
#define	CTL_RXTL_DEFAULT		0x20
/* Debug shift */
#define RX_CHANNEL_SHIFT		0
#define RX_WORD_LEN_SHIFT		1
#define RX_RATE_SHIFT			2
#define RX_ORIG_RATE_SHIFT		3
#define RX_DATA_TYPE_SHIFT		4

typedef void sunxi_owa_clk_t;

struct sunxi_owa_mem {
	struct resource res;
	void __iomem *membase;
	struct resource *memregion;
	struct regmap *regmap;
};

struct sunxi_owa_pinctl {
	struct pinctrl *pinctrl;
	struct pinctrl_state *pinstate;
	struct pinctrl_state *pinstate_sleep;

	bool pinctrl_used;
};

struct sunxi_owa_dts {
	/* value must be (2^n)Kbyte */
	size_t playback_cma;
	size_t playback_fifo_size;
	size_t capture_cma;
	size_t capture_fifo_size;

	/* components func -> rx_sync */
	bool rx_sync_ctl;
	int rx_sync_id;
	rx_sync_domain_t rx_sync_domain;

	/* clk fs */
	unsigned int pll_fs;
};

enum RX_LOCK_FLAG_STA {
	RX_LOCK_DEFAULT = 0,
	RX_LOCK = 0,
	RX_UNLOCK,
};

struct sunxi_owa_irq {
	unsigned int id;

	struct snd_pcm_substream *substream;

	/* add for false trigger interrupt */
	bool running;
	enum RX_LOCK_FLAG_STA flag_sta;
	enum RX_LOCK_FLAG_STA flag_sta_old;

	/* To solve channel exchange after plug and unplug */
	struct delayed_work lock_confirm_work;
};

struct sunxi_owa {
	const char *module_version;
	struct platform_device *pdev;

	struct sunxi_owa_mem mem;
	sunxi_owa_clk_t *clk;
	struct snd_sunxi_rglt *rglt;
	struct sunxi_owa_pinctl pin;
	struct sunxi_owa_dts dts;
	struct sunxi_owa_irq owa_irq;

	enum SND_SUNXI_CLK_STATUS clk_sta;

	struct sunxi_dma_params playback_dma_param;
	struct sunxi_dma_params capture_dma_param;

	/* debug */
	char module_name[32];
	struct snd_sunxi_dump dump;
	bool show_reg_all;
};

sunxi_owa_clk_t *snd_owa_clk_init(struct platform_device *pdev);
void snd_owa_clk_exit(void *clk_orig);
int snd_owa_clk_bus_enable(void *clk_orig);
int snd_owa_clk_enable(void *clk_orig);
void snd_owa_clk_bus_disable(void *clk_orig);
void snd_owa_clk_disable(void *clk_orig);
int snd_owa_clk_rate(void *clk_orig, unsigned int freq_in, unsigned int freq_out);

#endif /* __SND_SUNXI_AHUB_OWA_H */
