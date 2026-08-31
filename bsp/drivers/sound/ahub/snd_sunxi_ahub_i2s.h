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

#ifndef __SND_SUNXI_AHUB_I2S_H
#define __SND_SUNXI_AHUB_I2S_H

#include "snd_sunxi_pcm.h"
#include "snd_sunxi_rxsync.h"
#include "snd_sunxi_common.h"
#include "snd_sunxi_ahub_mgmt.h"

/* REG-I2S */
#define SUNXI_I2S_CTL			0x00
#define SUNXI_I2S_FMT0			0x04
#define SUNXI_I2S_FMT1			0x08
#define SUNXI_I2S_INT_STS		0x0C
#define SUNXI_I2S_FIFO_CTL		0x14
#define SUNXI_I2S_FIFO_STS		0x18
#define SUNXI_I2S_INTCTL		0x1C
#define SUNXI_I2S_CLKDIV		0x24
#define SUNXI_I2S_TXCNT			0x28
#define SUNXI_I2S_RXCNT			0x2C
#define SUNXI_I2S_CHCFG			0x30
#define SUNXI_I2S_TX0CHSEL		0x34
#define SUNXI_I2S_TX1CHSEL		0x38
#define SUNXI_I2S_TX2CHSEL		0x3C
#define SUNXI_I2S_TX3CHSEL		0x40
#define SUNXI_I2S_TX0CHMAP0		0x44
#define SUNXI_I2S_TX0CHMAP1		0x48
#define SUNXI_I2S_TX1CHMAP0		0x4C
#define SUNXI_I2S_TX1CHMAP1		0x50
#define SUNXI_I2S_TX2CHMAP0		0x54
#define SUNXI_I2S_TX2CHMAP1		0x58
#define SUNXI_I2S_TX3CHMAP0		0x5C
#define SUNXI_I2S_TX3CHMAP1		0x60
#define SUNXI_I2S_RXCHSEL		0x64
#define SUNXI_I2S_RXCHMAP0		0x68
#define SUNXI_I2S_RXCHMAP1		0x6C
#define SUNXI_I2S_RXCHMAP2		0x70
#define SUNXI_I2S_RXCHMAP3		0x74
#define SUNXI_I2S_REV			0x7C
#define SUNXI_I2S_MAX_REG		SUNXI_I2S_REV

/* BIT-I2S */
/* SUNXI_I2S_CTL */
#define RX_SYNC_EN_START		21
#define RX_SYNC_EN			20
#define BCLK_OUT			18
#define LRCK_OUT			17
#define LRCKR_CTL			16
#define SDO3_EN				11
#define SDO2_EN				10
#define SDO1_EN				9
#define SDO0_EN				8
#define MUTE_CTL			6
#define MODE_SEL			4
#define LOOP_EN				3
#define CTL_TXEN			2
#define CTL_RXEN			1
#define GLOBAL_EN			0
/* SUNXI_I2S_FMT0 */
#define LRCK_WIDTH			30
#define LRCK_POLARITY			19
#define LRCK_PERIOD			8
#define BCLK_POLARITY			7
#define SAMPLE_RESOLUTION		4
#define EDGE_TRANSFER			3
#define SLOT_WIDTH			0
/* SUNXI_I2S_FMT1 */
#define RX_MLS				7
#define TX_MLS				6
#define SEXT				4
#define RX_PDM				2
#define TX_PDM				0
/* SUNXI_I2S_INT_STS */
#define TXU_INT				6
#define TXO_INT				5
#define RXU_INT				2
#define RXO_INT				1
/* SUNXI_I2S_FIFO_CTL */
#define TX_DLY_PRE_REN_WL		4
#define TX_DLY_PRE_REN_EN		3
#define TXIM				2
#define RXOM				0
/* SUNXI_I2S_FIFO_STS */
#define FIFO_TXF			25
#define FIFO_TXE			24
#define FIFO_TX_WL			16
#define FIFO_RXF			9
#define FIFO_RXE			8
#define FIFO_RX_WL			0
/* SUNXI_I2S_INTCTL */
#define TXUI_EN				6
#define TXOI_EN				5
#define RXUIEN				2
#define RXOIEN				1
/* SUNXI_I2S_CLKDIV */
#define MCLKOUT_EN			8
#define BCLK_DIV			4
#define MCLK_DIV			0
/* SUNXI_I2S_TXCNT */
#define TX_CNT				0
/* SUNXI_I2S_RXCNT */
#define RX_CNT				0
/* SUNXI_I2S_CHCFG */
#define TX_SLOT_HIZ			9
#define TX_STATE			8
#define RX_SLOT_NUM			4
#define TX_SLOT_NUM			0
/* SUNXI_I2S_TXnCHSEL(n:0-3) */
#define TX_OFFSET			20
#define TX_CHSEL			16
#define TX_CHEN				0
/* SUNXI_I2S_RXCHSEL */
#define RX_OFFSET			20
#define RX_CHSEL			16
/* SUNXI_I2S_REV */
#define VER				0

typedef void sunxi_i2s_clk_t;

struct sunxi_i2s_mem {
	struct resource res;
	void __iomem *membase;
	struct resource *memregion;
	struct regmap *regmap;
};

struct sunxi_i2s_pinctl {
	struct pinctrl *pinctrl;
	struct pinctrl_state *pinstate;
	struct pinctrl_state *pinstate_sleep;

	bool pinctrl_used;
};

struct sunxi_i2s_dts {
	/* value must be (2^n)Kbyte */
	size_t playback_cma;
	size_t playback_fifo_size;
	size_t capture_cma;
	size_t capture_fifo_size;

	unsigned int dai_type;
	unsigned int tdm_num;

	/* "tx-pin"
	 * 1. tx pin enable setting.
	 * 2. we can set it like tx_pin = <3 0> - enable tx_pin0 and tx_pin3.
	 *
	 * "rx-pin"
	 * 1. rx pin enable setting
	 * 2. we can set it like rx_pin = <1 2> - enable rx_pin1 and rx_pin2.
	 *
	 * "tx-pin-chmap"
	 * 1. tx pin channel map setting.
	 * 2. the subscript of value corresponds to the channel number.
	 * 3. the value corresponds to slot number.
	 * 4. we can use it like tx-pin1-chmap = <1 2 ... 15> to set pin1 channel map.
	 * 5. if enabled the pin, but not set "tx-pin-chmap", default set <0 1 2 ...>.
	 * 6. if the count of value is less than 16, then missing member will be set 0.
	 *
	 * rxfifo map
	 * 1. "rxfifo-pinmap" - set the corresponding pin.
	 * 2. "rxfifo-chmap" - set the corresponding slot.
	 * 3. we can use it like rxfifo-pinmap = <0 1 2>; rxfifo-chmap = <1 2 14>; to set rxfifo map.
	 * 4. the count of value of "rxfifo-pinmap" can != the count of value of "rxfifo-pinmap",
	 *    then missing member would be set 0.
	 */
	bool tx_pin[4];
	uint32_t tx_pin_chmap[4][16];
	uint32_t rxfifo_pinmap[16];
	uint32_t rxfifo_chmap[16];

	/* components func -> rx_sync */
	bool rx_sync_ctl;
	int rx_sync_id;
	rx_sync_domain_t rx_sync_domain;

	unsigned int clk_en_post_delay;

	/*
	 * "clk-mode": means support clk as needed.
	 * clk-mode = <0>: not need pre-support clk
	 * clk-mode = <1>: need pre-support clk during probe
	 *
	 * "clk-mode-mclk-freq": means mclk freq needed.
	 * eg. clk-mode-mclk-freq = <24576000>
	 *
	 * "clk-mode-lrck-freq": means lrck freq needed.
	 * eg. clk-mode-lrck-freq = <48000>
	 *
	 * "clk-mode-slots" and "clk-mode-slots": used to cal bclk freq
	 *
	 * "daifmt": means I2S params, set params depend on codec need.
	 * I2S params include:
	 * eg.
	 * 	clk-mode-format = "i2s";
	 * 	clk-mode-frame-inversion;
	 * 	clk-mode-bitclock-inversion;
	 * 	clk-mode-frame-master;
	 * 	clk-mode-bitclock-master;
	 */
	unsigned int clk_mode;				/* read from dts */
	unsigned int clk_mode_mclk_freq;		/* read from dts */
	unsigned int clk_mode_lrck_freq;		/* read from dts */
	unsigned int clk_mode_bclk_freq;
	unsigned int clk_mode_daifmt;
	unsigned int clk_mode_slots;			/* read from dts */
	unsigned int clk_mode_slot_width;		/* read from dts */
};

struct sunxi_audio_status {
	struct mutex apf_mutex;
	bool spk;
};

enum SUNXI_I2S_DAI_FMT_SEL {
	SUNXI_I2S_DAI_PLL = 0,
	SUNXI_I2S_DAI_MCLK,
	SUNXI_I2S_DAI_FMT,
	SUNXI_I2S_DAI_MASTER,
	SUNXI_I2S_DAI_INVERT,
	SUNXI_I2S_DAI_SLOT_NUM,
	SUNXI_I2S_DAI_SLOT_WIDTH,
};

struct sunxi_i2s_dai_fmt {
	unsigned int pllclk_freq;
	unsigned int moduleclk_freq;
	unsigned int fmt;
	unsigned int slots;
	unsigned int slot_width;
	u32 data_late;
	bool tx_lsb_first;
	bool rx_lsb_first;
};

struct sunxi_i2s_clk_sta {
	struct mutex clk_mutex;
	unsigned int old_rate;
	u32 p_work;
	u32 c_work;
};

struct sunxi_i2s {
	unsigned int i2s_num;
	enum SUNXI_AHUB_PORT i2s_ahub_tx;
	enum SUNXI_AHUB_PORT i2s_ahub_rx;

	const char *module_version;
	struct platform_device *pdev;

	struct audio_reg_group reg_group;

	struct sunxi_i2s_mem mem;
	sunxi_i2s_clk_t *clk;
	struct snd_sunxi_rglt *rglt;
	struct sunxi_i2s_pinctl pin;
	struct sunxi_i2s_dts dts;
	struct sunxi_audio_status audio_sta;
	struct sunxi_i2s_clk_sta i2s_clk_sta;

	struct sunxi_dma_params playback_dma_param;
	struct sunxi_dma_params capture_dma_param;

	struct sunxi_i2s_dai_fmt i2s_dai_fmt;

	bool tx_trigger_bypass;

	/* debug */
	char module_name[32];
	struct snd_sunxi_dump dump;
	bool show_reg_all;

	/* pa config */
	unsigned int pa_pin_max;
	struct snd_sunxi_pacfg *pa_cfg;
};

sunxi_i2s_clk_t *snd_i2s_clk_init(struct platform_device *pdev);
void snd_i2s_clk_exit(void *clk_orig);
int snd_i2s_clk_bus_enable(void *clk_orig);
int snd_i2s_clk_enable(void *clk_orig);
void snd_i2s_clk_bus_disable(void *clk_orig);
void snd_i2s_clk_disable(void *clk_orig);
int snd_i2s_clk_rate(void *clk_orig, unsigned int freq_in, unsigned int freq_out);

#endif /* __SND_SUNXI_AHUB_I2S_H */
