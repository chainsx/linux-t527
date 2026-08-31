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

#include "snd_sunxi_common.h"

#ifndef __SND_SUNXI_AHUB_MGMT_H
#define __SND_SUNXI_AHUB_MGMT_H

/* REG-GLB_CTL */
#define SUNXI_AHUB_SOFT_RST1			0x00
#define SUNXI_AHUB_SOFT_RST2			0x04
#define SUNXI_AHUB_CLK_GAT1			0x20
#define SUNXI_AHUB_CLK_GAT2			0x24
#define SUNXI_AHUB_CLK_GAT3			0x28
#define SUNXI_AHUB_IRQ_CTL1			0x40
#define SUNXI_AHUB_IRQ_CTL2			0x44
#define SUNXI_AHUB_IRQ_STS1			0x50
#define SUNXI_AHUB_IRQ_STS2			0x54
#define SUNXI_AHUB_APBIF_RX0_CONT_SEL1		0x60
#define SUNXI_AHUB_APBIF_RX0_CONT_SEL2		0x64
#define SUNXI_AHUB_APBIF_RX1_CONT_SEL1		0x68
#define SUNXI_AHUB_APBIF_RX1_CONT_SEL2		0x6C
#define SUNXI_AHUB_APBIF_RX2_CONT_SEL1		0x70
#define SUNXI_AHUB_APBIF_RX2_CONT_SEL2		0x74
#define SUNXI_AHUB_APBIF_RX3_CONT_SEL1		0x78
#define SUNXI_AHUB_APBIF_RX3_CONT_SEL2		0x7C
#define SUNXI_AHUB_APBIF_RX4_CONT_SEL1		0x80
#define SUNXI_AHUB_APBIF_RX4_CONT_SEL2		0x84
#define SUNXI_AHUB_APBIF_RX5_CONT_SEL1		0x88
#define SUNXI_AHUB_APBIF_RX5_CONT_SEL2		0x8C
#define SUNXI_AHUB_APBIF_RX6_CONT_SEL1		0x90
#define SUNXI_AHUB_APBIF_RX6_CONT_SEL2		0x94
#define SUNXI_AHUB_APBIF_RX7_CONT_SEL1		0x98
#define SUNXI_AHUB_APBIF_RX7_CONT_SEL2		0x9C
#define SUNXI_AHUB_ADDA_DAC_CONT_SEL1		0x148
#define SUNXI_AHUB_ADDA_DAC_CONT_SEL2		0x14C
#define SUNXI_AHUB_I2S0_TX_CONT_SEL1		0x180
#define SUNXI_AHUB_I2S0_TX_CONT_SEL2		0x184
#define SUNXI_AHUB_I2S1_TX_CONT_SEL1		0x188
#define SUNXI_AHUB_I2S1_TX_CONT_SEL2		0x18C
#define SUNXI_AHUB_I2S2_TX_CONT_SEL1		0x190
#define SUNXI_AHUB_I2S2_TX_CONT_SEL2		0x194
#define SUNXI_AHUB_I2S3_TX_CONT_SEL1		0x198
#define SUNXI_AHUB_I2S3_TX_CONT_SEL2		0x19C
#define SUNXI_AHUB_I2S4_TX_CONT_SEL1		0x1A0
#define SUNXI_AHUB_I2S4_TX_CONT_SEL2		0x1A4
#define SUNXI_AHUB_OWA_TX_CONT_SEL1		0x200
#define SUNXI_AHUB_OWA_TX_CONT_SEL2		0x204
#define SUNXI_AHUB_VER				0x2FF
/* REG-APBIF_TX(0-6) */
#define SUNXI_AHUB_APBIF_TX_CTL(n)		(0x00 + ((n) * 0x100) + 0x300)
#define SUNXI_AHUB_APBIF_TX_IRQ_CTL(n)		(0x10 + ((n) * 0x100) + 0x300)
#define SUNXI_AHUB_APBIF_TX_IRQ_STS(n)		(0x14 + ((n) * 0x100) + 0x300)
#define SUNXI_AHUB_APBIF_TX_FIFO_CTL(n)		(0x20 + ((n) * 0x100) + 0x300)
#define SUNXI_AHUB_APBIF_TX_FIFO_STS(n)		(0x28 + ((n) * 0x100) + 0x300)
#define SUNXI_AHUB_APBIF_TX_FIFO_DATA(n)	(0x30 + ((n) * 0x100) + 0x300)
#define SUNXI_AHUB_APBIF_TX_FIFO_CNT(n)		(0x34 + ((n) * 0x100) + 0x300)
#define SUNXI_AHUB_APBIF_TX_FIFO_CNT_INIT(n)	(0x38 + ((n) * 0x100) + 0x300)
#define SUNXI_AHUB_APBIF_TX_P2S_FIFO_STS(n)	(0x40 + ((n) * 0x100) + 0x300)
/* REG-APBIF_RX(0-7) */
#define SUNXI_AHUB_APBIF_RX_CTL(n)		(0x00 + ((n) * 0x100) + 0xB00)
#define SUNXI_AHUB_APBIF_RX_IRQ_CTL(n)		(0x10 + ((n) * 0x100) + 0xB00)
#define SUNXI_AHUB_APBIF_RX_IRQ_STS(n)		(0x14 + ((n) * 0x100) + 0xB00)
#define SUNXI_AHUB_APBIF_RX_FIFO_CTL(n)		(0x20 + ((n) * 0x100) + 0xB00)
#define SUNXI_AHUB_APBIF_RX_FIFO_STS(n)		(0x28 + ((n) * 0x100) + 0xB00)
#define SUNXI_AHUB_APBIF_RX_FIFO_DATA(n)	(0x40 + ((n) * 0x100) + 0xB00)
#define SUNXI_AHUB_APBIF_RX_FIFO_CNT(n)		(0x44 + ((n) * 0x100) + 0xB00)
#define SUNXI_AHUB_APBIF_RX_FIFO_CNT_INIT(n)	(0x48 + ((n) * 0x100) + 0xB00)
#define SUNXI_AHUB_APBIF_RX_S2P_FIFO_STS(n)	(0x50 + ((n) * 0x100) + 0xB00)
/* REG-DAM(0-1) */
#define SUNXI_AHUB_DAM_CTL(n)			(0x00 + ((n) * 0x100) + 0x1400)
#define SUNXI_AHUB_DAM_RX0_CONT_SEL1(n)		(0x10 + ((n) * 0x100) + 0x1400)
#define SUNXI_AHUB_DAM_RX0_CONT_SEL2(n)		(0x14 + ((n) * 0x100) + 0x1400)
#define SUNXI_AHUB_DAM_RX1_CONT_SEL1(n)		(0x18 + ((n) * 0x100) + 0x1400)
#define SUNXI_AHUB_DAM_RX1_CONT_SEL2(n)		(0x1C + ((n) * 0x100) + 0x1400)
#define SUNXI_AHUB_DAM_RX2_CONT_SEL1(n)		(0x20 + ((n) * 0x100) + 0x1400)
#define SUNXI_AHUB_DAM_RX2_CONT_SEL2(n)		(0x24 + ((n) * 0x100) + 0x1400)
#define SUNXI_AHUB_DAM_MIX_MAP0(n)		(0x40 + ((n) * 0x100) + 0x1400)
#define SUNXI_AHUB_DAM_MIX_MAP1(n)		(0x44 + ((n) * 0x100) + 0x1400)
#define SUNXI_AHUB_DAM_MIX_MAP2(n)		(0x48 + ((n) * 0x100) + 0x1400)
#define SUNXI_AHUB_DAM_MIX_MAP3(n)		(0x4C + ((n) * 0x100) + 0x1400)
#define SUNXI_AHUB_DAM_MIX_MAP4(n)		(0x50 + ((n) * 0x100) + 0x1400)
#define SUNXI_AHUB_DAM_MIX_MAP5(n)		(0x54 + ((n) * 0x100) + 0x1400)
#define SUNXI_AHUB_DAM_MIX_MAP6(n)		(0x58 + ((n) * 0x100) + 0x1400)
#define SUNXI_AHUB_DAM_MIX_MAP7(n)		(0x5C + ((n) * 0x100) + 0x1400)
#define SUNXI_AHUB_DAM_VOL_CTL0(n)		(0x80 + ((n) * 0x100) + 0x1400)
#define SUNXI_AHUB_DAM_VOL_CTL1(n)		(0x84 + ((n) * 0x100) + 0x1400)
#define SUNXI_AHUB_DAM_VOL_CTL2(n)		(0x88 + ((n) * 0x100) + 0x1400)
#define SUNXI_AHUB_DAM_VOL_CTL3(n)		(0x8C + ((n) * 0x100) + 0x1400)
#define SUNXI_AHUB_DAM_VOL_CTL4(n)		(0x90 + ((n) * 0x100) + 0x1400)
#define SUNXI_AHUB_DAM_VOL_CTL5(n)		(0x94 + ((n) * 0x100) + 0x1400)
#define SUNXI_AHUB_DAM_VOL_CTL6(n)		(0x98 + ((n) * 0x100) + 0x1400)
#define SUNXI_AHUB_DAM_VOL_CTL7(n)		(0x9C + ((n) * 0x100) + 0x1400)
#define SUNXI_AHUB_DAM_TX_CTL(n)		(0xA0 + ((n) * 0x100) + 0x1400)
#define SUNXI_AHUB_DAM_IRQ_CTL(n)		(0xC0 + ((n) * 0x100) + 0x1400)
#define SUNXI_AHUB_DAM_IRQ_STS(n)		(0xC4 + ((n) * 0x100) + 0x1400)
#define SUNXI_AHUB_DAM_RX0_FIFO_STS(n)		(0xD0 + ((n) * 0x100) + 0x1400)
#define SUNXI_AHUB_DAM_RX1_FIFO_STS(n)		(0xD4 + ((n) * 0x100) + 0x1400)
#define SUNXI_AHUB_DAM_RX2_FIFO_STS(n)		(0xD8 + ((n) * 0x100) + 0x1400)
#define SUNXI_AHUB_DAM_TX_FIFO_STS(n)		(0xDC + ((n) * 0x100) + 0x1400)

#define SUNXI_AHUB_MAX_REG			SUNXI_AHUB_DAM_TX_FIFO_STS(1)

/* BIT-GLB_CTL */
/* SUNXI_AHUB_SOFT_RST1 */
#define TX6_SOFT_RST		22
#define TX5_SOFT_RST		21
#define TX4_SOFT_RST		20
#define TX3_SOFT_RST		19
#define TX2_SOFT_RST		18
#define TX1_SOFT_RST		17
#define TX0_SOFT_RST		16
#define RX7_SOFT_RST		7
#define RX6_SOFT_RST		6
#define RX5_SOFT_RST		5
#define RX4_SOFT_RST		4
#define RX3_SOFT_RST		3
#define RX2_SOFT_RST		2
#define RX1_SOFT_RST		1
#define RX0_SOFT_RST		0
/* SUNXI_AHUB_SOFT_RST2 */
#define DAM1_SOFT_RST		1
#define DAM0_SOFT_RST		0
/* SUNXI_AHUB_CLK_GAT1 */
#define TX6_GAT			22
#define TX5_GAT			21
#define TX4_GAT			20
#define TX3_GAT			19
#define TX2_GAT			18
#define TX1_GAT			17
#define TX0_GAT			16
#define RX7_GAT			7
#define RX6_GAT			6
#define RX5_GAT			5
#define RX4_GAT			4
#define RX3_GAT			3
#define RX2_GAT			2
#define RX1_GAT			1
#define RX0_GAT			0
/* SUNXI_AHUB_CLK_GAT2 */
#define DAM1_GAT0		21
#define DAM0_GAT0		20
#define OWA_GAT0		16
#define I2S4_GAT0		12
#define I2S3_GAT0		11
#define I2S2_GAT0		10
#define I2S1_GAT0		9
#define I2S0_GAT0		8
#define DMIC_GAT0		4
#define AUDIOCODEC_GAT0		1
/* SUNXI_AHUB_CLK_GAT3 */
#define OWA_GAT1		16
#define I2S4_GAT1		12
#define I2S3_GAT1		11
#define I2S2_GAT1		10
#define I2S1_GAT1		9
#define I2S0_GAT1		8
#define AUDIOCODEC_GAT1		1
/* SUNXI_AHUB_IRQ_CTL1 */
#define TX6_INT_EN		22
#define TX5_INT_EN		21
#define TX4_INT_EN		20
#define TX3_INT_EN		19
#define TX2_INT_EN		18
#define TX1_INT_EN		17
#define TX0_INT_EN		16
#define RX7_INT_EN		7
#define RX6_INT_EN		6
#define RX5_INT_EN		5
#define RX4_INT_EN		4
#define RX3_INT_EN		3
#define RX2_INT_EN		2
#define RX1_INT_EN		1
#define RX0_INT_EN		0
/* SUNXI_AHUB_IRQ_CTL2 */
#define DAM1_INT_EN		21
#define DAM0_INT_EN		20
#define OWA_INT_EN		16
#define I2S4_INT_EN		12
#define I2S3_INT_EN		11
#define I2S2_INT_EN		10
#define I2S1_INT_EN		9
#define I2S0_INT_EN		8
#define DMIC_INT_EN		4
#define AUDIOCODEC_INT_EN	1
/* SUNXI_AHUB_IRQ_STS1 */
#define TX6_INT			22
#define TX5_INT			21
#define TX4_INT			20
#define TX3_INT			19
#define TX2_INT			18
#define TX1_INT			17
#define TX0_INT			16
#define RX7_INT			7
#define RX6_INT			6
#define RX5_INT			5
#define RX4_INT			4
#define RX3_INT			3
#define RX2_INT			2
#define RX1_INT			1
#define RX0_INT			0
/* SUNXI_AHUB_IRQ_STS2 */
#define DAM1_INT		21
#define DAM0_INT		20
#define OWA_INT			16
#define I2S4_INT		12
#define I2S3_INT		11
#define I2S2_INT		10
#define I2S1_INT		9
#define I2S0_INT		8
#define DMIC_INT		4
#define AUDIOCODEC_INT		1
/* SUNXI_AHUB_APBIF_RX0_CONT_SEL1 */
#define NONE_TX			31
#define DAM1_TX			29
#define DAM0_TX			28
#define APBIF6_TX		6
#define APBIF5_TX		5
#define APBIF4_TX		4
#define APBIF3_TX		3
#define APBIF2_TX		2
#define APBIF1_TX		1
#define APBIF0_TX		0
/* SUNXI_AHUB_APBIF_RX0_CONT_SEL2 */
#define I2S4_TX			20
#define I2S3_TX			19
#define I2S2_TX			18
#define I2S1_TX			17
#define I2S0_TX			16
#define CODEC_TX		13
#define OWA_TX			8
#define DMIC_TX			0
/* SUNXI_AHUB_APBIF_RX1_CONT_SEL1 */
#define RX1_CONT_SEL1		0
/* SUNXI_AHUB_APBIF_RX1_CONT_SEL2 */
#define RX1_CONT_SEL2		0
/* SUNXI_AHUB_APBIF_RX2_CONT_SEL1 */
#define RX2_CONT_SEL1		0
/* SUNXI_AHUB_APBIF_RX2_CONT_SEL2 */
#define RX2_CONT_SEL2		0
/* SUNXI_AHUB_APBIF_RX3_CONT_SEL1 */
#define RX3_CONT_SEL1		0
/* SUNXI_AHUB_APBIF_RX3_CONT_SEL2 */
#define RX3_CONT_SEL2		0
/* SUNXI_AHUB_APBIF_RX4_CONT_SEL1 */
#define RX4_CONT_SEL1		0
/* SUNXI_AHUB_APBIF_RX4_CONT_SEL2 */
#define RX4_CONT_SEL2		0
/* SUNXI_AHUB_APBIF_RX5_CONT_SEL1 */
#define RX5_CONT_SEL1		0
/* SUNXI_AHUB_APBIF_RX5_CONT_SEL2 */
#define RX5_CONT_SEL2		0
/* SUNXI_AHUB_APBIF_RX6_CONT_SEL1 */
#define RX6_CONT_SEL1		0
/* SUNXI_AHUB_APBIF_RX6_CONT_SEL2 */
#define RX6_CONT_SEL2		0
/* SUNXI_AHUB_APBIF_RX7_CONT_SEL1 */
#define RX7_CONT_SEL1		0
/* SUNXI_AHUB_APBIF_RX7_CONT_SEL2 */
#define RX7_CONT_SEL2		0
/* SUNXI_AHUB_ADDA_DAC_CONT_SEL1 */
#define ADDA_DAC_CONT_SEL1	0
/* SUNXI_AHUB_ADDA_DAC_CONT_SEL2 */
#define ADDA_DAC_CONT_SEL2	0
/* SUNXI_AHUB_I2S0_TX_CONT_SEL1 */
#define I2S0_TX_CONT_SEL1	0
/* SUNXI_AHUB_I2S0_TX_CONT_SEL2 */
#define I2S0_TX_CONT_SEL2	0
/* SUNXI_AHUB_I2S1_TX_CONT_SEL1 */
#define I2S1_TX_CONT_SEL1	0
/* SUNXI_AHUB_I2S1_TX_CONT_SEL2 */
#define I2S1_TX_CONT_SEL2	0
/* SUNXI_AHUB_I2S2_TX_CONT_SEL1 */
#define I2S2_TX_CONT_SEL1	0
/* SUNXI_AHUB_I2S2_TX_CONT_SEL2 */
#define I2S2_TX_CONT_SEL2	0
/* SUNXI_AHUB_I2S3_TX_CONT_SEL1 */
#define I2S3_TX_CONT_SEL1	0
/* SUNXI_AHUB_I2S3_TX_CONT_SEL2 */
#define I2S3_TX_CONT_SEL2	0
/* SUNXI_AHUB_I2S4_TX_CONT_SEL1 */
#define I2S4_TX_CONT_SEL1	0
/* SUNXI_AHUB_I2S4_TX_CONT_SEL2 */
#define I2S4_TX_CONT_SEL2	0
/* SUNXI_AHUB_OWA_TX_CONT_SEL1 */
#define OWA_TX_CONT_SEL1	0
/* SUNXI_AHUB_OWA_TX_CONT_SEL2 */
#define OWA_TX_CONT_SEL2	0
/* SUNXI_AHUB_VER */
#define VER_V			16
#define VER_R			8
#define VER_M			0

/* BIT-APBIF_TX */
/* SUNXI_AHUB_APBIF_TX_CTL */
#define TX_SW			16
#define TX_CHAN_NUM		8
#define TX_EN			4
/* SUNXI_AHUB_APBIF_TX_IRQ_CTL */
#define TX_P2S_FIFO_URUN_INT_EN	9
#define TX_P2S_FIFO_ORUN_INT_EN	8
#define TX_FIFO_URUN_INT_EN	2
#define TX_FIFO_ORUN_INT_EN	1
#define TX_FIFO_DRQ_INT_EN	0
/* SUNXI_AHUB_APBIF_TX_IRQ_STS */
#define TX_P2S_FIFO_URUN_INT	9
#define TX_P2S_FIFO_ORUN_INT	8
#define TX_FIFO_URUN_INT	2
#define TX_FIFO_ORUN_INT	1
#define TX_FIFO_DRQ_INT		0
/* SUNXI_AHUB_APBIF_TX_FIFO_CTL */
#define TX_FIFO_DRQ_EN		31
#define TX_FLUSH_FIFO		12
#define TX_FIFO_TL		4
#define TX_FIFO_IM		0
/* SUNXI_AHUB_APBIF_TX_FIFO_STS */
#define TX_FIFO_FULL		9
#define TX_FIFO_EMPTY		8
#define TX_FIFO_WL		0
/* SUNXI_AHUB_APBIF_TX_FIFO_DATA */
#define TX_DATA			0
/* SUNXI_AHUB_APBIF_TX_FIFO_CNT */
#define TX_CNT			0
/* SUNXI_AHUB_APBIF_TX_FIFO_CNT_INIT */
#define TX_CNT_INIT		0
/* SUNXI_AHUB_APBIF_TX_P2S_FIFO_STS */
#define TX_P2S_FIFO_FULL	31
#define TX_P2S_FIFO_EMPTY	30
#define TX_P2S_FIFO_WCLK_WL	16
#define TX_P2S_FIFO_RCLK_WL	0

/* BIT-APBIF_RX */
/* SUNXI_AHUB_APBIF_RX_CTL(n) */
#define RX_SW			16
#define RX_CHAN_NUM		8
#define RX_EN			4
/* SUNXI_AHUB_APBIF_RX_IRQ_CTL */
#define RX_P2S_FIFO_URUN_INT_EN	9
#define RX_P2S_FIFO_ORUN_INT_EN	8
#define RX_FIFO_URUN_INT_EN	2
#define RX_FIFO_ORUN_INT_EN	1
#define RX_FIFO_DRQ_INT_EN	0
/* SUNXI_AHUB_APBIF_RX_IRQ_STS */
#define RX_P2S_FIFO_URUN_INT	9
#define RX_P2S_FIFO_ORUN_INT	8
#define RX_FIFO_URUN_INT	2
#define RX_FIFO_ORUN_INT	1
#define RX_FIFO_DRQ_INT		0
/* SUNXI_AHUB_APBIF_RX_FIFO_CTL */
#define RX_FIFO_DRQ_EN		31
#define RX_FLUSH_FIFO		12
#define RX_FIFO_TL		4
#define RX_FIFO_OM		0
/* SUNXI_AHUB_APBIF_RX_FIFO_STS */
#define RX_FIFO_FULL		9
#define RX_FIFO_EMPTY		8
#define RX_FIFO_WL		0
/* SUNXI_AHUB_APBIF_RX_FIFO_DATA */
#define RX_DATA			0
/* SUNXI_AHUB_APBIF_RX_FIFO_CNT */
#define RX_CNT			0
/* SUNXI_AHUB_APBIF_RX_FIFO_CNT_INIT */
#define RX_CNT_INIT		0
/* SUNXI_AHUB_APBIF_RX_P2S_FIFO_STS */
#define RX_P2S_FIFO_FULL	31
#define RX_P2S_FIFO_EMPTY	30
#define RX_P2S_FIFO_WCLK_WL	16
#define RX_P2S_FIFO_RCLK_WL	0

/* BIT-DAM */
/* SUNXI_AHUB_DAM_CTL */
#define DAM_RX2_CHAN_NUM	24
#define DAM_RX1_CHAN_NUM	20
#define DAM_RX0_CHAN_NUM	16
#define DAM_TX_CHAN_NUM		8
#define DAM_RX2_EN		6
#define DAM_RX1_EN		5
#define DAM_RX0_EN		4
#define DAM_TX_EN		0
/* SUNXI_AHUB_DAM_RX0_CONT_SEL1 */
#define DAM_RX0_CONT_SEL1	0
/* SUNXI_AHUB_DAM_RX0_CONT_SEL2 */
#define DAM_RX0_CONT_SEL2	0
/* SUNXI_AHUB_DAM_RX1_CONT_SEL1 */
#define DAM_RX1_CONT_SEL1	0
/* SUNXI_AHUB_DAM_RX1_CONT_SEL2 */
#define DAM_RX1_CONT_SEL2	0
/* SUNXI_AHUB_DAM_RX2_CONT_SEL1 */
#define DAM_RX2_CONT_SEL1	0
/* SUNXI_AHUB_DAM_RX2_CONT_SEL2 */
#define DAM_RX2_CONT_SEL2	0
/* SUNXI_AHUB_DAM_MIX_MAP0 */
#define DAM_TX_CH1_SRC2_MAP	24
#define DAM_TX_CH1_SRC1_MAP	20
#define DAM_TX_CH1_SRC0_MAP	16
#define DAM_TX_CH0_SRC2_MAP	8
#define DAM_TX_CH0_SRC1_MAP	4
#define DAM_TX_CH0_SRC0_MAP	0
/* SUNXI_AHUB_DAM_MIX_MAP1 */
#define DAM_TX_CH3_SRC2_MAP	24
#define DAM_TX_CH3_SRC1_MAP	20
#define DAM_TX_CH3_SRC0_MAP	16
#define DAM_TX_CH2_SRC2_MAP	8
#define DAM_TX_CH2_SRC1_MAP	4
#define DAM_TX_CH2_SRC0_MAP	0
/* SUNXI_AHUB_DAM_MIX_MAP2 */
#define DAM_TX_CH5_SRC2_MAP	24
#define DAM_TX_CH5_SRC1_MAP	20
#define DAM_TX_CH5_SRC0_MAP	16
#define DAM_TX_CH4_SRC2_MAP	8
#define DAM_TX_CH4_SRC1_MAP	4
#define DAM_TX_CH4_SRC0_MAP	0
/* SUNXI_AHUB_DAM_MIX_MAP3 */
#define DAM_TX_CH7_SRC2_MAP	24
#define DAM_TX_CH7_SRC1_MAP	20
#define DAM_TX_CH7_SRC0_MAP	16
#define DAM_TX_CH6_SRC2_MAP	8
#define DAM_TX_CH6_SRC1_MAP	4
#define DAM_TX_CH6_SRC0_MAP	0
/* SUNXI_AHUB_DAM_MIX_MAP4 */
#define DAM_TX_CH9_SRC2_MAP	24
#define DAM_TX_CH9_SRC1_MAP	20
#define DAM_TX_CH9_SRC0_MAP	16
#define DAM_TX_CH8_SRC2_MAP	8
#define DAM_TX_CH8_SRC1_MAP	4
#define DAM_TX_CH8_SRC0_MAP	0
/* SUNXI_AHUB_DAM_MIX_MAP5 */
#define DAM_TX_CH11_SRC2_MAP	24
#define DAM_TX_CH11_SRC1_MAP	20
#define DAM_TX_CH11_SRC0_MAP	16
#define DAM_TX_CH10_SRC2_MAP	8
#define DAM_TX_CH10_SRC1_MAP	4
#define DAM_TX_CH10_SRC0_MAP	0
/* SUNXI_AHUB_DAM_MIX_MAP6 */
#define DAM_TX_CH13_SRC2_MAP	24
#define DAM_TX_CH13_SRC1_MAP	20
#define DAM_TX_CH13_SRC0_MAP	16
#define DAM_TX_CH12_SRC2_MAP	8
#define DAM_TX_CH12_SRC1_MAP	4
#define DAM_TX_CH12_SRC0_MAP	0
/* SUNXI_AHUB_DAM_MIX_MAP7 */
#define DAM_TX_CH15_SRC2_MAP	24
#define DAM_TX_CH15_SRC1_MAP	20
#define DAM_TX_CH15_SRC0_MAP	16
#define DAM_TX_CH14_SRC2_MAP	8
#define DAM_TX_CH14_SRC1_MAP	4
#define DAM_TX_CH14_SRC0_MAP	0
/* SUNXI_AHUB_DAM_VOL_CTL0 */
#define DAM_TX_CH1_SRC2_GAIN	24
#define DAM_TX_CH1_SRC1_GAIN	20
#define DAM_TX_CH1_SRC0_GAIN	16
#define DAM_TX_CH0_SRC2_GAIN	8
#define DAM_TX_CH0_SRC1_GAIN	4
#define DAM_TX_CH0_SRC0_GAIN	0
/* SUNXI_AHUB_DAM_VOL_CTL1 */
#define DAM_TX_CH3_SRC2_GAIN	24
#define DAM_TX_CH3_SRC1_GAIN	20
#define DAM_TX_CH3_SRC0_GAIN	16
#define DAM_TX_CH2_SRC2_GAIN	8
#define DAM_TX_CH2_SRC1_GAIN	4
#define DAM_TX_CH2_SRC0_GAIN	0
/* SUNXI_AHUB_DAM_VOL_CTL2 */
#define DAM_TX_CH5_SRC2_GAIN	24
#define DAM_TX_CH5_SRC1_GAIN	20
#define DAM_TX_CH5_SRC0_GAIN	16
#define DAM_TX_CH4_SRC2_GAIN	8
#define DAM_TX_CH4_SRC1_GAIN	4
#define DAM_TX_CH4_SRC0_GAIN	0
/* SUNXI_AHUB_DAM_VOL_CTL3 */
#define DAM_TX_CH7_SRC2_GAIN	24
#define DAM_TX_CH7_SRC1_GAIN	20
#define DAM_TX_CH7_SRC0_GAIN	16
#define DAM_TX_CH6_SRC2_GAIN	8
#define DAM_TX_CH6_SRC1_GAIN	4
#define DAM_TX_CH6_SRC0_GAIN	0
/* SUNXI_AHUB_DAM_VOL_CTL4 */
#define DAM_TX_CH9_SRC2_GAIN	24
#define DAM_TX_CH9_SRC1_GAIN	20
#define DAM_TX_CH9_SRC0_GAIN	16
#define DAM_TX_CH8_SRC2_GAIN	8
#define DAM_TX_CH8_SRC1_GAIN	4
#define DAM_TX_CH8_SRC0_GAIN	0
/* SUNXI_AHUB_DAM_VOL_CTL5 */
#define DAM_TX_CH11_SRC2_GAIN	24
#define DAM_TX_CH11_SRC1_GAIN	20
#define DAM_TX_CH11_SRC0_GAIN	16
#define DAM_TX_CH10_SRC2_GAIN	8
#define DAM_TX_CH10_SRC1_GAIN	4
#define DAM_TX_CH10_SRC0_GAIN	0
/* SUNXI_AHUB_DAM_VOL_CTL6 */
#define DAM_TX_CH13_SRC2_GAIN	24
#define DAM_TX_CH13_SRC1_GAIN	20
#define DAM_TX_CH13_SRC0_GAIN	16
#define DAM_TX_CH12_SRC2_GAIN	8
#define DAM_TX_CH12_SRC1_GAIN	4
#define DAM_TX_CH12_SRC0_GAIN	0
/* SUNXI_AHUB_DAM_VOL_CTL7 */
#define DAM_TX_CH15_SRC2_GAIN	24
#define DAM_TX_CH15_SRC1_GAIN	20
#define DAM_TX_CH15_SRC0_GAIN	16
#define DAM_TX_CH14_SRC2_GAIN	8
#define DAM_TX_CH14_SRC1_GAIN	4
#define DAM_TX_CH14_SRC0_GAIN	0
/* SUNXI_AHUB_DAM_TX_CTL */
#define DAM_TX_DLY_DTS		4
#define DAM_TX_DLY_EN 		0
/* SUNXI_AHUB_DAM_IRQ_CTL */
#define DAM_TX_URUN_INT_EN	17
#define DAM_TX_ORUN_INT_EN 	16
#define DAM_RX2_URUN_INT_EN 	9
#define DAM_RX2_ORUN_INT_EN 	8
#define DAM_RX1_URUN_INT_EN 	5
#define DAM_RX1_ORUN_INT_EN 	4
#define DAM_RX0_URUN_INT_EN 	1
#define DAM_RX0_ORUN_INT_EN 	0
/* SUNXI_AHUB_DAM_IRQ_STS */
#define DAM_TX_URUN_INT		17
#define DAM_TX_ORUN_INT 	16
#define DAM_RX2_URUN_INT 	9
#define DAM_RX2_ORUN_INT 	8
#define DAM_RX1_URUN_INT 	5
#define DAM_RX1_ORUN_INT 	4
#define DAM_RX0_URUN_INT 	1
#define DAM_RX0_ORUN_INT 	0
/* SUNXI_AHUB_DAM_RX0_FIFO_STS */
#define DAM_RX0_FULL 		31
#define DAM_RX0_EMPTY 		30
#define DAM_RX0_WL 		0
/* SUNXI_AHUB_DAM_RX1_FIFO_STS */
#define DAM_RX1_FULL 		31
#define DAM_RX1_EMPTY 		30
#define DAM_RX1_WL 		0
/* SUNXI_AHUB_DAM_RX2_FIFO_STS */
#define DAM_RX2_FULL 		31
#define DAM_RX2_EMPTY 		30
#define DAM_RX2_WL 		0
/* SUNXI_AHUB_DAM_TX_FIFO_STS */
#define DAM_TX_FULL 		31
#define DAM_TX_EMPTY 		30
#define DAM_TX_WL 		0

typedef void sunxi_ahub_clk_t;

/* for module */
enum SUNXI_AHUB_PORT {
	/* none */
	SUNXI_AHUB_PORT_NONE = -1,
	/* module->play */
	SUNXI_AHUB_PORT_CODEC_RX = 0,
	SUNXI_AHUB_PORT_I2S0_RX,
	SUNXI_AHUB_PORT_I2S1_RX,
	SUNXI_AHUB_PORT_I2S2_RX,
	SUNXI_AHUB_PORT_I2S3_RX,
	SUNXI_AHUB_PORT_I2S4_RX,
	SUNXI_AHUB_PORT_OWA_RX,
	/* module->cap */
	SUNXI_AHUB_PORT_CODEC_TX,
	SUNXI_AHUB_PORT_I2S0_TX,
	SUNXI_AHUB_PORT_I2S1_TX,
	SUNXI_AHUB_PORT_I2S2_TX,
	SUNXI_AHUB_PORT_I2S3_TX,
	SUNXI_AHUB_PORT_I2S4_TX,
	SUNXI_AHUB_PORT_OWA_TX,
	SUNXI_AHUB_PORT_DMIC_TX,
	/* apbif->play */
	SUNXI_AHUB_PORT_APBIF0_TX,
	SUNXI_AHUB_PORT_APBIF1_TX,
	SUNXI_AHUB_PORT_APBIF2_TX,
	SUNXI_AHUB_PORT_APBIF3_TX,
	SUNXI_AHUB_PORT_APBIF4_TX,
	SUNXI_AHUB_PORT_APBIF5_TX,
	SUNXI_AHUB_PORT_APBIF6_TX,
	/* apbif->cap */
	SUNXI_AHUB_PORT_APBIF0_RX,
	SUNXI_AHUB_PORT_APBIF1_RX,
	SUNXI_AHUB_PORT_APBIF2_RX,
	SUNXI_AHUB_PORT_APBIF3_RX,
	SUNXI_AHUB_PORT_APBIF4_RX,
	SUNXI_AHUB_PORT_APBIF5_RX,
	SUNXI_AHUB_PORT_APBIF6_RX,
	SUNXI_AHUB_PORT_APBIF7_RX,
	/* dam */
	SUNXI_AHUB_PORT_DAM0_TX,
	SUNXI_AHUB_PORT_DAM0_RX0,
	SUNXI_AHUB_PORT_DAM0_RX1,
	SUNXI_AHUB_PORT_DAM0_RX2,
	SUNXI_AHUB_PORT_DAM1_TX,
	SUNXI_AHUB_PORT_DAM1_RX0,
	SUNXI_AHUB_PORT_DAM1_RX1,
	SUNXI_AHUB_PORT_DAM1_RX2,
};

/* dts parse */
enum SUNXI_DTS_PARSE {
	SUNXI_DTS_PARSE_STR = 0,
	SUNXI_DTS_PARSE_UINT,
};

/* mgmt init */
struct sunxi_ahub_mgmt_mem {
	struct resource res;
	void __iomem *membase;
	struct resource *memregion;
	struct regmap *regmap;
};

struct sunxi_ahub {
	const char *module_version;
	struct platform_device *pdev;

	struct sunxi_ahub_mgmt_mem mem;
	struct sunxi_ahub_mgmt_clk *clk;
	struct mutex ahub_lock;

	/* 0-7  :apbif_rx[0-7]
	 * 8    :codec_rx
	 * 9-13 :i2s[0-4]_rx
	 * 14   :owa_rx
	 * 15-17:dam0_rx[0-2]
	 * 18-20:dam1_rx[0-2]
	 */
	enum SUNXI_AHUB_PORT ahub_src[21];
	/* 0  :dam0_tx_ch
	 * 1-3:dam0_rx[0-2]_ch
	 * 4  :dam1_tx_ch
	 * 5-7:dam1_rx[0-2]_ch
	 */
	unsigned int dam_ch[8];
	/* 0-2:dam0_tx_ch0_src[0-2]_map
	 * 3-5:dam0_tx_ch1_src[0-2]_map
	 * ...
	 * 48-50:dam1_tx_ch0_src[0-2]_map
	 * 51-53:dam1_tx_ch1_src[0-2]_map
	 * ...
	 */
	unsigned int ch_map[96];

	/* debug */
	char module_name[32];
	struct snd_sunxi_dump dump;
	int show_reg_num;
};

/* for ahub mgmt */
sunxi_ahub_clk_t *snd_ahub_clk_init(struct platform_device *pdev);
void snd_ahub_clk_exit(void *clk_orig);
int snd_ahub_clk_bus_enable(void *clk_orig);
int snd_ahub_clk_enable(void *clk_orig);
void snd_ahub_clk_bus_disable(void *clk_orig);
void snd_ahub_clk_disable(void *clk_orig);
int snd_ahub_clk_rate(void *clk_orig, unsigned int freq_in, unsigned int freq_out);

/* provide interfaces for other modules */
void snd_sunxi_set_apbif_ch(enum SUNXI_AHUB_PORT port, unsigned int ch);
void snd_sunxi_set_apbif_sw(enum SUNXI_AHUB_PORT port, unsigned int sw);
void snd_sunxi_set_apbif_fifo_mode(enum SUNXI_AHUB_PORT port, unsigned int mode);
void snd_sunxi_apbif_ctl(enum SUNXI_AHUB_PORT port, bool en);
void snd_sunxi_flush_fifo(enum SUNXI_AHUB_PORT port);
void snd_sunxi_drq_ctl(enum SUNXI_AHUB_PORT port, bool en);
unsigned int snd_sunxi_get_dev_fifo(enum SUNXI_AHUB_PORT port);
void snd_sunxi_soft_reset(enum SUNXI_AHUB_PORT port);
void snd_sunxi_clk_gating_ctl(enum SUNXI_AHUB_PORT port, bool en);
unsigned int snd_sunxi_ahub_ctl(enum SUNXI_AHUB_PORT port, unsigned int ch, bool en);

#endif /* __SND_SUNXI_AHUB_MGMT_H */
