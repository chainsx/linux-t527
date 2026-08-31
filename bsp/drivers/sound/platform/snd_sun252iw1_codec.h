/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Allwinner's ALSA SoC Audio driver
 *
 * Copyright (c) 2022, wuhao <wuhao@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 */

#ifndef _SUN252IW1_CODEC_H
#define _SUN252IW1_CODEC_H

/* REG-Digital */
#define SUNXI_DAC_DPC			0x00
#define SUNXI_DAC_VOL_CTRL		0x04
#define SUNXI_DAC_FIFOC			0x10
#define SUNXI_DAC_FIFOS			0x14

#define SUNXI_DAC_TXDATA		0X20
#define SUNXI_DAC_CNT			0x24
#define SUNXI_DAC_DG			0x28

#define	SUNXI_ADC_FIFOC			0x30
#define	SUNXI_ADC_VOL_CTRL		0x34
#define SUNXI_ADC_FIFOS			0x38
#define SUNXI_ADC_RXDATA		0x40
#define SUNXI_ADC_CNT			0x44
#define SUNXI_ADC_DEBUG			0x4C
#define SUNXI_ADC_DIG_CTRL		0x50

#define SUNXI_VRA1SPEEDUP_DOWN_CTRL	0x54

#define SUNXI_DAC_DAP_CTL		0xF0
#define SUNXI_ADC_DAP_CTL		0xF8
#define SUNXI_DAC_DRC_CTRL		0x108
#define SUNXI_ADC_DRC_CTRL		0x208

#define SUNXI_AC_VERSION		0x2C0

/* Analog register */
#define SUNXI_ADC1_REG			0x300
#define SUNXI_ADC2_REG			0x304
#define SUNXI_DAC_REG			0x310
#define SUNXI_DAC_DHP_REG		0x314
#define SUNXI_BIAS_REG			0x320
#define SUNXI_POWER_REG			0x348
#define SUNXI_AUDIO_MAX_REG		SUNXI_POWER_REG

/* BITS */
/* SUNXI_DAC_DPC:0x00 */
#define EN_DAC			31
#define MODQU			25
#define DWA_EN			24
#define HPF_EN			18
#define DVOL			12
#define DITHER_SGM		8
#define DITHER_SFT		4
#define DITHER_EN		1
#define DAC_HUB_EN		0

/* SUNXI_DAC_VOL_CTRL:0x04 */
#define DAC_VOL_SEL		16
#define DAC_VOL_L		8

/* SUNXI_DAC_FIFOC:0x10 */
#define DAC_FS				29
#define FIR_VER				28
#define SEND_LASAT			26
#define FIFO_MODE			24
#define DAC_DRQ_CLR_CNT			21
#define TX_TRIG_LEVEL			8
#define DAC_MONO_EN			6
#define TX_SAMPLE_BITS			5
#define DAC_DRQ_EN			4
#define DAC_IRQ_EN			3
#define FIFO_UNDERRUN_IRQ_EN		2
#define FIFO_OVERRUN_IRQ_EN		1
#define FIFO_FLUSH			0

/* SUNXI_DAC_FIFOS:0x14 */
#define TX_EMPTY			23
#define DAC_TXE_CNT			8
#define DAC_TXE_INT			3
#define DAC_TXU_INT			2
#define DAC_TXO_INT			1

/* SUNXI_DAC_DG:0x28 */
#define	DAC_MODU_SEL		11
#define	DAC_PATTERN_SEL		9
#define	DAC_CODEC_CLK_SEL	8
#define	DAC_SWP			6
#define	ADDA_LOOP_MODE		0

/* SUNXI_ADC_FIFOC:0x30 */
#define ADC_FS			29
#define EN_AD			28
#define ADCFDT			26
#define ADCDFEN			25
#define RX_FIFO_MODE		24
#define RX_SYNC_EN_STA		21
#define RX_SYNC_EN		20
#define RX_SAMPLE_BITS		16
#define RX_FIFO_TRG_LEVEL	4
#define ADC_DRQ_EN		3
#define ADC_IRQ_EN		2
#define ADC_OVERRUN_IRQ_EN	1
#define ADC_FIFO_FLUSH		0

/* SUNXI_ADC_VOL_CTRL:0x34 */
#define ADC2_VOL		8
#define ADC1_VOL		0

/* SUNXI_ADC_FIFOS:0x38 */
#define RXA				23
#define ADC_RXA_CNT			8
#define ADC_RXA_INT			3
#define ADC_RXO_INT			1

/* SUNXI_ADC_DEBUG:0x4C */
#define	ADC_SWP1		24

/* SUNXI_ADC_DIG_CTRL:0x50 */
#define ADC1_2_VOL_EN			16
#define ADC2_CHANNEL_EN			1
#define ADC1_CHANNEL_EN			0

/* SUNXI_VRA1SPEEDUP_DOWN_CTRL:0x54 */
#define VRA1SPEEDUP_DOWN_STATE		4
#define VRA1SPEEDUP_DOWN_CTRL		1
#define VRA1SPEEDUP_DOWN_RST_CTRL	0

/* SUNXI_DAC_DAP_CTL:0xF0 */
#define DDAP_EN			31
#define DDAP_DRC_EN		29
#define DDAP_HPF_EN		28

/* SUNXI_ADC_DAP_CTL:0xF8 */
#define ADAP0_EN		31
#define ADAP0_DRC_EN		29
#define ADAP0_HPF_EN		28

/* SUNXI_ADC1_REG : 0x300 */
#define ADC1_EN				28
#define MIC1_PGA_EN			27
#define ADC1_MIC_START_CTRL		26
#define ADC1_PGA_GAIN_CTRL		22
#define PGA1_OP1			21
#define PGA1_OP2			20
#define ADC1_DSM_RST			19
#define ADC1_DITHER_CTRL		18
#define ADC1_DSM_DITHER_LVL		16
#define ADC1_IOPSDM1			10
#define ADC1_IOPSDM2			8

/* SUNXI_ADC2_REG : 0x304 */
#define ADC2_EN				28
#define MIC2_PGA_EN			27
#define ADC2_PGA_GAIN_CTRL		22
#define PGA2_OP1			21
#define PGA2_OP2			20
#define ADC2_DSM_RST			19
#define ADC2_DITHER_CTRL		18
#define ADC2_DSM_DITHER_LVL		16
#define ADC2_IOPSDM1			10
#define ADC2_IOPSDM2			8

/* SUNXI_DAC_REG : 0x310 */
#define CKDAC_DLYSET                    22
#define LINEOUT_VOL			19
#define DACLEN				18
#define LINEOUTLEN			17
#define DACLMUTE			16
#define LOUT_CH_EN                      15
#define LOUT_CH_NOL_EN                  14
#define DAC_CH_EN                       7
#define DAC_CH_NOL_EN			6
#define DAC_CH_CKSET                    4
#define DAC_CH_DLYSET                   2
#define LOUT_CH_NOLDLY_SET              0

/* SUNXI_DAC_DHP_REG : 0x314 */
#define DAC_DHP_GAIN_BYPASS				8
#define DAC_PLAYBACK_MUTE_DET_IN_TIME	5
#define ATT_STEP						3
#define LOUT_AUTO_ATT					2
#define	LOUT_AUTO_MUTE					1
#define DHP_GAIN_ZC_DEN					0

/* SUNXI_BIAS_REG : 0x320 */
#define IOPADC				14
#define IOPVRS				12
#define IOPDACS				10
#define BIA_CUR_TRIM			0

/* SUNXI_POWER_REG      :0x348 */
#define VAR1SPEEDUP_DOWN_FURTHER_CTRL	29
#define AVCCPOR				16

/* for DRC/HPF Switch */
#define DACDRC_SHIFT			1
#define DACHPF_SHIFT			2
#define ADCDRC0_SHIFT			3
#define ADCHPF0_SHIFT			4

/* for input mux Switch */
#define INPUT1_SHIFT			0
#define INPUT2_SHIFT			1

/* for input gain Switch */
#define FMINL_GAIN_SHIFT			0
#define FMINR_GAIN_SHIFT			1
#define LINEINL_GAIN_SHIFT			2
#define LINEINR_GAIN_SHIFT			3

struct sunxi_codec_mem {
	struct resource res;
	void __iomem *membase;
	struct resource *memregion;
	struct regmap *regmap;
};

struct sunxi_codec_clk {
	/* parent */
	struct clk *clk_pll_audio_1x;

	/* module */
	struct clk *clk_audio_dac;
	struct clk *clk_audio_adc;
	/* bus & reset */
	struct clk *clk_bus;
	struct reset_control *clk_rst;
};

struct sunxi_audio_status {
	struct mutex apf_mutex; /* audio playback function mutex lock */
	struct mutex pa_pin_mutex;
	bool spk;

	struct mutex acf_mutex; /* audio capture function mutex lock */
	bool adc1;
	bool adc2;
};

struct sunxi_codec_dts {
	/* tx_hub */
	bool tx_hub_en;

	/* rx_sync */
	bool rx_sync_en;
	bool rx_sync_ctl;
	int rx_sync_id;
	rx_sync_domain_t rx_sync_domain;

	/* volume & gain */
	u32 dac_vol;
	u32 dacl_vol;
	u32 lineout_vol;
	u32 adc1_vol;
	u32 adc2_vol;
	u32 adc1_gain;
	u32 adc2_gain;
};

struct sunxi_codec {
	const char *module_version;
	struct platform_device *pdev;

	struct sunxi_codec_mem mem;
	struct sunxi_codec_clk clk;
	struct snd_sunxi_rglt *rglt;
	struct sunxi_codec_dts dts;
	struct sunxi_audio_status audio_sta;
	enum SND_SUNXI_CLK_STATUS clk_sta;

	unsigned int pa_pin_max;
	struct snd_sunxi_pacfg *pa_cfg;

	/* debug */
	char module_name[32];
	struct snd_sunxi_dump dump;
	bool show_reg_all;
};

#endif /* __SND_SUN252IW1_CODEC_H */
