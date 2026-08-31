/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
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

#ifndef __SND_SUN55IW7_CODEC_H
#define __SND_SUN55IW7_CODEC_H

/* REG-Digital */
#define SUNXI_DAC_DPC		0x00
#define SUNXI_DAC_VOL_CTL	0x04
#define SUNXI_DAC_FIFO_CTL	0x10
#define SUNXI_DAC_FIFO_STA	0x14
#define SUNXI_DAC_TXDATA	0X20
#define SUNXI_DAC_CNT		0x24
#define SUNXI_DAC_DEBUG		0x28

#define	SUNXI_ADC_FIFO_CTL	0x30
#define SUNXI_ADC_VOL_CTL1	0x34
#define SUNXI_ADC_FIFO_STA	0x38
#define SUNXI_ADC_RXDATA	0x40
#define SUNXI_ADC_CNT		0x44
#define SUNXI_ADC_DEBUG		0x4C
#define SUNXI_ADC_DIG_CTL	0x50

#define SUNXI_VAR1SPEEDUP_DOWN_CTL	0x54

#define SUNXI_AN_DEBUG1		0x58
#define SUNXI_AN_DEBUG2		0x5C

#define SUNXI_DAC_DAP_CTL	0xF0
#define SUNXI_ADC_DAP_CTL	0xF8
#define SUNXI_DAC_DRC_CTL	0x108
#define SUNXI_ADC_DRC_CTL	0x208

#define SUNXI_VERSION		0x2C0

/* REG-Analog */
#define SUNXI_ADCL_AN_CTL	0x300
#define SUNXI_ADCR_AN_CTL	0x304

#define SUNXI_DAC_AN_CTL	0x310

#define SUNXI_RAMP_CTL		0x31C
#define SUNXI_BIAS_REG		0x320
#define SUNXI_HP2_REG		0x340
#define SUNXI_POWER_AN_CTL	0x348
#define SUNXI_ADC_CUR_REG	0x34C
#define SUNXI_AUTOGATE_CTRL	0x350
#define SUNXI_DAC_LOOPBACK_DBG	0x354
#define SUNXI_AUDIO_MAX_REG	SUNXI_DAC_LOOPBACK_DBG

/* BITS */
/* SUNXI_DAC_DPC:0x00 */
#define DAC_DIG_EN		31
#define MODQU			25
#define DWA_EN			24
#define HPF_EN			18
#define DVOL			12
#define DITHER_SGM		8
#define DITHER_SFT		4
#define DITHER_EN		1
#define HUB_EN			0

/* SUNXI_DAC_VOL_CTL:0x04 */
#define DAC_VOL_SEL		16
#define DAC_VOL_L		8
#define DAC_VOL_R		0

/* SUNXI_DAC_FIFO_CTL:0x10 */
#define DAC_FS			29
#define FIR_VER			28
#define SEND_LASAT		26
#define DAC_FIFO_MODE		24
#define DAC_DRQ_CLR_CNT		21
#define TX_TRIG_LEVEL		8
#define DAC_MONO_EN		6
#define TX_SAMPLE_BITS		5
#define DAC_DRQ_EN		4
#define DAC_IRQ_EN		3
#define DAC_FIFO_UNDERRUN_IRQ_EN	2
#define DAC_FIFO_OVERRUN_IRQ_EN		1
#define DAC_FIFO_FLUSH		0

/* SUNXI_DAC_FIFO_STA:0x14 */
#define	DAC_TX_EMPTY		23
#define	DAC_TXE_CNT		8
#define	DAC_TXE_INT		3
#define	DAC_TXU_INT		2
#define	DAC_TXO_INT		1

/* SUNXI_DAC_DEBUG:0x28 */
#define DAC_DBG_EN		31
#define DAC_LR_SEL		30
#define	DA2AD_LOOP_MODE		12
#define	DAC_MODU_SEL		11
#define	DAC_PATTERN_SEL		9
#define	CODEC_CLK_SEL		8
#define	DAC_SWP			6
#define	ADDA_LOOP_MOD_SEL	0

/* SUNXI_ADC_FIFO_CTL:0x30 */
#define ADC_FS			29
#define ADC_DIG_EN		28
#define ADCFDT			26
#define ADCDFEN			25
#define RX_FIFO_MODE		24
#define RX_SYNC_EN_START	21
#define RX_SYNC_EN		20
#define RX_SAMPLE_BITS		16
#define RX_FIFO_TRG_LEVEL	4
#define ADC_DRQ_EN		3
#define ADC_IRQ_EN		2
#define ADC_OVERRUN_IRQ_EN	1
#define ADC_FIFO_FLUSH		0

/* SUNXI_ADC_VOL_CTL1:0x34 */
#define ADC2_VOL		8
#define ADC1_VOL		0

/* SUNXI_ADC_FIFO_STA:0x38 */
#define	ADC_RXA			23
#define	ADC_RXA_CNT		8
#define	ADC_RXA_INT		3
#define	ADC_RXO_INT		1

/* SUNXI_ADC_DEBUG:0x4C */
#define	ADC_SWP		24
#define AD_DBG_EN	10

/* SUNXI_ADC_DIG_CTL:0x50 */
#define ADC1_2_VOL_EN		16
#define ADC_CHANNEL_EN		0

/* SUNXI_VAR1SPEEDUP_DOWN_CTL:0x54 */
#define VRA1SPEEDUP_DOWN_STATE		4
#define VRA1SPEEDUP_DOWN_CTL		1
#define VRA1SPEEDUP_DOWN_RST_CTL	0

/* SUNXI_AN_DEBUG1:0x58 */
#define DAC_MUTE_EN		18

/* SUNXI_AN_DEBUG2:0x5C */
#define DACR_DATA		23
#define DACR_DBG_DATA		16
#define DACR_DATA_CTL		15
#define DACL_DATA		8
#define DACL_DBG_DATA		1
#define DACL_DATA_CTL		0

/* SUNXI_DAC_DAP_CTL:0xF0 */
#define DDAP_EN			31
#define DDAP_DRC_EN		29
#define DDAP_HPF_EN		28

/* SUNXI_ADC_DAP_CTL:0xF8 */
#define ADAP_EN			31
#define ADAP_DRC_EN		29
#define ADAP_HPF_EN		28

/* SUNXI_ADCL_AN_CTL:0x300 */
#define ADC1_EN			31
#define MIC1_EN		30
#define ADC1_DITHER_CTL		29
#define MIC1SINGLE_EN		28
#define LINEINL2_EN		27
#define LINEINL1_EN		26
#define DSM1_DITHER_LVL		24
#define PGA1_OPAAF2_BIAS_CURRENT	20
#define LINEINL_GAIN_CTL	18
#define ADC1_PGA_GAIN_CTL	8
#define ADC1_IOPAAF		6
#define ADC1_IOPSDM1		4
#define ADC1_IOPSDM2		2
#define PGA1_OP1_CHOPPER_EN	1
#define PGA1_OP2_CHOPPER_EN	0

/* SUNXI_ADCR_AN_CTL:0x304 */
#define ADC2_EN			31
#define MIC2_EN			30
#define ADC2_DITHER_CTL		29
#define MIC2SINGLE_EN		28
#define LINEINR2_EN		27
#define LINEINR1_EN		26
#define DSM2_DITHER_LVL		24
#define PGA2_OPAAF2_BIAS_CURRENT	20
#define LINEINR_GAIN_CTL	18
#define LINEINL_SEL		14
#define ADC2_PGA_GAIN_CTL	8
#define ADC2_IOPAAF		6
#define ADC2_IOPSDM1		4
#define ADC2_IOPSDM2		2
#define PGA2_OP1_CHOPPER_EN	1
#define PGA2_OP2_CHOPPER_EN	0

/* SUNXI_DAC_AN_CTL:0x310 */
#define DAC_CH_EN		31
#define DAC_CH_NOL_EN		30
#define DAC_CH_CK_SET		28
#define CKDAC_DLY_SET		26
#define DAC_CH_NOL_SET		24
#define LOUT_CH_EN		23
#define LOUT_CH_NOL_EN		22
#define LOUT_CH_CK_SET		20
#define LOUT_CH_DLY_SET		18
#define LOUT_CH_NOLDLY_SET	16
#define DACL_EN			15
#define DACR_EN			14
#define LINEOUTL_EN		13
#define LMUTE			12
#define LINEOUTR_EN		11
#define RMUTE			10
#define LINEOUT_GAIN		0

/* SUNXI_RAMP_CTL:0x31C */
#define RAMP_RISE_INT_EN	31
#define RAMP_RISE_INT		30
#define RAMP_FALL_INT_EN	29
#define RAMP_FALL_INT		28
#define RAMP_SOFT_RESET		24
#define RAMP_CLK_DIV_M		16
#define HP_PULL_OUT_EN		15
#define RAMP_HOLD_STEP		12
#define GAP_STEP		8
#define RAMP_STEP		4
#define RAMP_DOWN_EN		3
#define RAMP_UP_EN		2
#define RAMP_CTL_EN		1
#define RAMP_DIG_EN		0

/* SUNXI_BIAS_REG:0x320 */
#define ADC_BIAS_CURRENT_SEL	10
#define BIAS_CURRENT_TRIM	0

/* SUNXI_HP2_REG:0x340 */
#define HPFB_BUF_EN		31
#define HEADPHONE_GAIN	28
#define HPFB_RES		26
#define OPDRV_CUR		24
#define HP_OFFSET_ADJ	22
#define HP_DRV_EN		21
#define HP_VO_OFFSET_EN	20
#define RSWITCH			19
#define RAMPEN			18
#define HPFB_IN_EN		17
#define	RAMP_FINAL_CTRL	16
#define	RAMP_OUT_EN		15
#define RESERVED		13
#define	RAMP_FILTER_CTRL	8
#define HP_CHOPPER_EN	7
#define HP_CH_NOL_EN	6
#define HP_CHOPPER_CKS	4
#define HP_CH_DLYSET	2
#define HP_CH_NOLDLY_SET	0

/* SUNXI_POWER_AN_CTL:0x348 */
#define VAR1SPEEDUP_DOWN_FURTHER_CTL	29
#define AVCCPOR_MONITOR		16

/* SUNXI_ADC_CUR_REG:0x34C */
#define ADC2_PGA_CH_DLYSET	14
#define ADC2_PGA_CH_EN		13
#define ADC2_PGA_CHOPPER_NOL_EN	12
#define ADC2_PGA_CH_NOLDLYSET	10
#define ADC2_PGA_CHOPPERCKS	8
#define ADC1_PGA_CH_DLYSET	6
#define ADC1_PGA_CH_EN		5
#define ADC1_PGA_CHOPPER_NOL_EN	4
#define ADC1_PGA_CH_NOLDLYSET	2
#define ADC1_PGA_CHOPPERCKS	0

/* SUNXI_AUTOGATE_CTRL:0x350 */
#define AUTOGATE_EN		0

/* SUNXI_DAC_LOOPBACK_DBG:0x354 */
#define DAC_LOOPBACK_CS		0

#define DACDRC_SHIFT		1
#define DACHPF_SHIFT		2
#define ADCDRC_SHIFT		3
#define ADCHPF_SHIFT		4

struct sunxi_codec_mem {
	struct resource res;
	void __iomem *membase;
	struct resource *memregion;
	struct regmap *regmap;
};

struct sunxi_codec_clk {
	/* parent */
	struct clk *clk_pll_audio0;	/* 22.5792MHz */
	struct clk *clk_pll_peri1_600m;	/* 24.576MHz */
	/* module */
	struct clk *clk_audio_dac;
	struct clk *clk_audio_adc;
	/* bus & reset */
	struct clk *clk_bus;
	struct reset_control *clk_rst;
	/* record current clk */
	struct clk *clk_pll_play;
	struct clk *clk_pll_cap;
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
	u32 dacr_vol;
	u32 adc1_vol;
	u32 adc2_vol;
	u32 lineout_gain;
	u32 lineinl_gain;
	u32 lineinr_gain;
	u32 hpout_gain;
	u32 adc1_gain;
	u32 adc2_gain;

	/* ramp */
	unsigned int ramp_time_up;
	unsigned int ramp_time_down;
};

struct sunxi_audio_status {
	struct mutex apf_mutex; /* audio playback function mutex lock */
	bool spk;
	bool lineoutl;
	bool lineoutr;

	struct mutex acf_mutex; /* audio capture function mutex lock */
	bool mic1;
	bool mic2;
};

struct sunxi_codec {
	const char *module_version;
	struct platform_device *pdev;

	struct sunxi_codec_mem mem;
	struct sunxi_codec_clk clk;
	struct sunxi_codec_dts dts;
	struct snd_sunxi_rglt *rglt;
	struct sunxi_audio_status audio_sta;
	enum SND_SUNXI_CLK_STATUS clk_play_sta;
	enum SND_SUNXI_CLK_STATUS clk_cap_sta;

	unsigned int pa_pin_max;
	struct snd_sunxi_pacfg *pa_cfg;

	/* debug */
	char module_name[32];
	struct snd_sunxi_dump dump;
	bool show_reg_all;
};

#endif /* __SND_SUN55IW7_CODEC_H */
