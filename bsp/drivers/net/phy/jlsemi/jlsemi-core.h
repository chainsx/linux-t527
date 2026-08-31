/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2014-2025 JLSemi Limited
 * All Rights Reserved
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE of JLSemi Limited
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code.
 *
 * No part of this code may be reproduced, stored in a retrieval system,
 * or transmitted, in any form or by any means, electronic, mechanical,
 * photocopying, recording, or otherwise, without the prior written
 * permission of JLSemi Limited
 */

#ifndef _JLSEMI_CORE_H
#define _JLSEMI_CORE_H

#define JLSEMI_KERNEL_DEVICE_TREE_USE	0

/*
 * 0: log off
 * 1: error log on
 * 2: warn log on
 * 3: info log on
 * 4: debug log on
 */
#define JL_SUPPORT_PHY_DBG_LEVEL		3

#include <linux/phy.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/of.h>
#include "jlsemi-dt-phy.h"

/*JLSemi FE PHY ID*/
#define JL11X1AF_PHY_ID		0x937c4023
#define JL11X1AH_PHY_ID		0x937c4024
#define JL11X1B_PHY_ID		0x937c4025
#define JL11X1_PHY_ID		0x937c4020
/*JLSemi GE PHY ID*/
#define JL2XX1A_PHY_ID		0x937c4032
#define JL22X1B_PHY_ID		0x937c4033
#define JL21X1C_PHY_ID		0x937c4336
#define JL2XX1_PHY_ID		0x937c4030

//#define JLSEMI_PHY_ID_MASK	0xfffffff0
#define JL_PHY_ID_MATCH_EXACT	0xFFFFFFFF
#define JL_PHY_ID_MATCH_MODEL	0xFFFFFFF0
#define JL_PHY_ID_MATCH_VENDOR	0xFFFFFC00
#define JL_PHY_ID_MATCH_VD_MOD	0xFFFFFCF0

/*NE: Not Equal
 *EQ: Equal
 *LT: Little Than
 *LE: Little or Equal
 *GT: Greater Than
 *GE: Greater or Equal
 */
#define JL11X1A_NE(phy_id)		((phy_id) != JL11X1AF_PHY_ID && (phy_id) != JL11X1AH_PHY_ID)
#define JL11X1A_EQ(phy_id)		((phy_id) == JL11X1AF_PHY_ID || (phy_id) == JL11X1AH_PHY_ID)
#define JL11X1B_LT(phy_id)		(JL11X1A_EQ(phy_id))
#define JL11X1_EQ(phy_id)		((phy_id) == JL11X1_PHY_ID)

#define JL2XX1A_NE(phy_id)		((phy_id) != JL2XX1A_PHY_ID)
#define JL2XX1A_EQ(phy_id)		((phy_id) == JL2XX1A_PHY_ID)
#define JL22X1B_EQ(phy_id)		((phy_id) == JL22X1B_PHY_ID)
#define JL21X1C_LT(phy_id)		(JL2XX1A_EQ(phy_id) || JL22X1B_EQ(phy_id))
#define JL21X1C_NE(phy_id)		((phy_id) != JL21X1C_PHY_ID)
#define JL2XX1_EQ(phy_id)		((phy_id) == JL2XX1_PHY_ID)

//-----------------------------------------------------------------------//
//--------------------------- JLSemi Common -----------------------------//
//-----------------------------------------------------------------------//

#define JLSEMI_PAGE_31				31

//-----------------------------------------------------------------------//
//------------------------------- JL11X1 --------------------------------//
//-----------------------------------------------------------------------//

#define JL11X1_PAGE_0				0
#define JL11X1_PAGE_7				7
#define JL11X1_PAGE_24				24
#define JL11X1_PAGE_128				128
#define JL11X1_PAGE_129				129

#define JL11X1_DEVICE_31			31

// page 0 reg 28 Fiber Mode Control REG
#define JL11X1B_FIBER_MODE_REG		28
#define JL11X1B_FIBER_MODE_EN		BIT(5)

// page 0 reg 30 Interrupt Indicators REG
#define JL11X1_INTR_STATUS_REG		30

// page 7 reg 16 RMII Mode Setting REG
#define JL11X1_RMII_CTRL_REG		16
#define JL11X1_RMII_CLK_50M_INPUT	BIT(12)
#define JL11X1_RMII_MODE			BIT(3)
#define JL11X1_RMII_TX_SKEW_MASK	(0xf << 8)
#define JL11X1B_RMII_TX_SKEW_MASK	(1 << 8)
#define JL11X1_RMII_TX_SKEW(n)		((n << 8) & JL11X1_RMII_TX_SKEW_MASK)
#define JL11X1_RMII_RX_SKEW_MASK	(0xf << 4)
#define JL11X1B_RMII_RX_SKEW_MASK	(1 << 4)
#define JL11X1_RMII_RX_SKEW(n)		((n << 4) & JL11X1_RMII_RX_SKEW_MASK)
#define JL11X1_RMII_CRS_DV			BIT(2)

// page 7 reg 17 LED Mode REG
#define JL11X1B_LED_MODE_REG		17
#define JL11X1B_SUPP_LED_MODE	(JL1XXX_LED0_EEE | \
								 JL1XXX_LED0_100_ACTIVITY | \
								 JL1XXX_LED0_10_ACTIVITY | \
								 JL1XXX_LED0_100_LINK | \
								 JL1XXX_LED0_10_LINK | \
								 JL1XXX_LED1_EEE | \
								 JL1XXX_LED1_100_ACTIVITY | \
								 JL1XXX_LED1_10_ACTIVITY | \
								 JL1XXX_LED1_100_LINK | \
								 JL1XXX_LED1_10_LINK)

// page 7 reg 19 Interrupt & LED Enable REG
#define JL11X1_INTR_LED_EN_REG		19
#define JL11X1_INTR_LINK			BIT(13)
#define JL11X1_INTR_AN_ERR			BIT(11)
#define JL11X1_LED_EN				BIT(3)

#define JL11X1B_INTR_AN_COMPLETE	BIT(15)

// page 24 reg 24 TX setting REG
#define JL11X1_TX_REG				24
#define JL11X1_MDI_TX_BM_MASK		0x1c00
#define JL11X1_MDI_TX_BM(n)			(n << 10)
#define JL11X1_MDI_TX_SRN			BIT(0)

// page 24 reg 25 LED Blink REG
#define JL11X1_LED_BLINK_REG		25
#define JL11X1_LED_PERIOD_MASK		0xff00
#define JL11X1_LEDPERIOD(n)			((n << 8) & JL11X1_LED_PERIOD_MASK)
#define JL11X1_LED_ON_MASK			0x00ff
#define JL11X1_LEDON(n)				((n << 0) & JL11X1_LED_ON_MASK)

// page 128 reg 29 LED Mode REG
#define JL11X1_LED_GPIO_REG			29
#define JL11X1_SUPP_GPIO		(JL1XXX_GPIO_LED0_EN | \
								 JL1XXX_GPIO_LED0_OUT | \
								 JL1XXX_GPIO_LED1_EN | \
								 JL1XXX_GPIO_LED1_OUT)

// page 129 reg 24 LED Mode REG
#define JL11X1_LED_MODE_REG			24
#define JL11X1_SUPP_LED_MODE	(JL1XXX_LED0_EEE | \
								 JL1XXX_LED0_100_ACTIVITY | \
								 JL1XXX_LED0_10_ACTIVITY | \
								 JL1XXX_LED0_100_LINK | \
								 JL1XXX_LED0_10_LINK | \
								 JL1XXX_LED1_EEE | \
								 JL1XXX_LED1_100_ACTIVITY | \
								 JL1XXX_LED1_10_ACTIVITY | \
								 JL1XXX_LED1_100_LINK | \
								 JL1XXX_LED1_10_LINK)

// page 129 reg 25~27 WOL MAC ADDR0~2 REG
#define JL11X1_MAC_ADDR0_REG		25
#define JL11X1_MAC_ADDR1_REG		26
#define JL11X1_MAC_ADDR2_REG		27

// page 129 reg 28 WOL Control REG
#define JL11X1_WOL_CTRL_REG			28
#define JL11X1_WOL_DIS				BIT(15)
#define JL11X1_WOL_CLEAR			BIT(1)
#define JL11X1_WOL_RECEIVE			BIT(0)
#define ADDR8_HIGH_TO_LOW(n)		((n >> 4) | (n << 4))

// Dev 0x1f reg 20 LED Blink REG
#define JL11X1B_LED_BLINK_REG		20
#define JL11X1B_LED_ON_MASK			0xff00
#define JL11X1B_LEDON(n)			((n << 8) & JL11X1B_LED_ON_MASK)
#define JL11X1B_LED_PERIOD_MASK		0x00ff
#define JL11X1B_LEDPERIOD(n)		((n << 0) & JL11X1B_LED_PERIOD_MASK)

// Dev 0x1f Reg 0x9
#define JL11X1B_REG_24				0x24

// Dev 0x1f Reg 0xb
#define JL11X1B_REG_2C				0x2C

// Dev 0x1f Reg 0x38
#define JL11X1B_REG_E0				0xE0

//-----------------------------------------------------------------------//
//------------------------------- JL2XX1 --------------------------------//
//-----------------------------------------------------------------------//

#define JL2XX1_PAGE_0				0
#define JL2XX1_PAGE_18				18
#define JL2XX1_PAGE_19				19
#define JL2XX1_PAGE_22				22
#define JL2XX1_PAGE_41				41
#define JL2XX1_PAGE_45				45
#define JL2XX1_PAGE_46				46
#define JL2XX1_PAGE_128				128
#define JL2XX1_PAGE_158				158
#define JL2XX1_PAGE_160				160
#define JL2XX1_PAGE_173				173
#define JL2XX1_PAGE_174				174
#define JL2XX1_PAGE_179				179
#define JL2XX1_PAGE_181				181
#define JL2XX1_PAGE_190				190
#define JL2XX1_PAGE_257				257
#define JL2XX1_PAGE_259				259
#define JL2XX1_PAGE_282				282		//0x11A
#define JL2XX1_PAGE_287				287		//0x11F
#define JL2XX1_PAGE_289				289		//0x121
#define JL2XX1_PAGE_2052			2052	//0x804
#define JL2XX1_PAGE_2055			2055	//0x807
#define JL2XX1_PAGE_2626			2626	//0xA42
#define JL2XX1_PAGE_2627			2627	//0xA43
#define JL2XX1_PAGE_3332			3332	//0xD04
#define JL2XX1_PAGE_3336			3336	//0xD08
#define JL2XX1_PAGE_4096			4096	//0x1000
#define JL2XX1_PAGE_4608			4608	//0x1200

// page 0 reg 1 Basic Mode Status REG
#define JL2XX1_BMCR_REG				0x0000
#define JL2XX1_BMCR_SOFT_RESET		BIT(15)
#define JL2XX1_BMCR_SPEED_LSB		BIT(13)
#define JL2XX1_BMCR_DUPLEX			BIT(8)
#define JL2XX1_BMCR_SPEED_MSB		BIT(6)

// page 0 reg 17 PHY Downshift REG
#define JL2XX1_DSFT_CTRL_REG		17
#define JL2XX1_DSFT_AN_ERR_EN		BIT(15)
#define JL2XX1_DSFT_TWO_WIRE_EN		BIT(14)
#define JL2XX1_DSFT_SMART_EN		BIT(13)
#define JL2XX1_DSFT_EN				BIT(12)
#define JL2XX1_DSFT_STL_MASK		0x03e0
#define JL2XX1_DSFT_STL_CNT(n)		(((n << 5) & JL2XX1_DSFT_STL_MASK))
#define JL2XX1_DSFT_AN_MASK			0x001f
#define JL2XX1_DSFT_CNT_MAX			32

// page 0 reg 20 PHY External/PMD Local Loopback Mode REG
#define JL2XX1_LPBK_MODE_REG		20
#define JL2XX1_LPBK_MODE_MASK		0x6
#define JL2XX1_LPBK_PMD_MODE		BIT(2)
#define JL2XX1_LPBK_EXT_MODE		BIT(1)

// page 0 reg 29 PHY INFO REG
#define JL2XX1_PHY_INFO_REG			29
#define JL2XX1_SW_MASK				0xffff

// page 0 reg 30 PHY Status 6 REG
#define JL2XX1_PHY_MODE_REG				30
#define JL2XX1_FIBER_1000				BIT(12)
#define JL2XX1_FIBER_100				BIT(11)
#define JL2XX1_SERDES_1000				BIT(10)
#define JL2XX1_SERDES_100				BIT(9)
#define JL2XX1_SERDES_10				BIT(8)
#define JL2XX1_PHY_FIBER_MODE_MASK		0x1800
#define JL2XX1_PHY_SERDES_MODE_MASK		0x700

// page 18 reg 21 PHY Work Mode REG
#define JL2XX1_USER_CONFIG_REG			21
#define JL2XX1_USER_CONFIG_RXC_OUT		BIT(14)
#define JL2XX1_USER_CONFIG_NO_AN_1G		(BIT(11) | BIT(10))
#define JL2XX1_USER_CONFIG_WOL_EN		BIT(6)
#define JL2XX1_USER_CONFIG_MODE_MASK	0x7

// page 19 reg 16 PHY LED Driver Ctrl0 REG
#define JL21X1C_LED_DRV_CTRL_0_REG		16
#define JL21X1C_LED_POLARITY_0			BIT(6)

// page 19 reg 17 PHY LED Driver Ctrl1 REG
#define JL21X1C_LED_DRV_CTRL_1_REG		17
#define JL21X1C_LED_POLARITY_1			BIT(6)

// page 19 reg 18 PHY LED Driver Ctrl2 REG
#define JL21X1C_LED_DRV_CTRL_2_REG		18
#define JL21X1C_LED_POLARITY_2			BIT(6)

// page 19 reg 19 PHY LED Blink REG, refer to page 4096 reg 20
#define JL21X1C_LED_BLINK_REG			19

// page 19 reg 26 PHY RGMII Delay Config 0 REG
#define JL21X1C_RGMII_DLY_CFG_0_REG					26
#define JL21X1C_CFG0_RGMII_RX_CLK_INV_LATCH_EN_BIT	12
#define JL21X1C_CFG0_RGMII_RX_CLK_INV_LATCH_EN_MASK	BIT(12)
#define JL21X1C_CFG0_TX_AD_DAT_SEL_BIT				4
#define JL21X1C_CFG0_TX_AD_DAT_SEL_MASK				BIT(4)
#define JL21X1C_CFG0_RX_AD_DAT_SEL_BIT				3
#define JL21X1C_CFG0_RX_AD_DAT_SEL_MASK				BIT(3)

// page 22 reg 23 PHY RGMII Delay Config 1 REG
#define JL21X1C_RGMII_DLY_CFG_1_REG					23
#define JL21X1C_CFG1_ANA_DLL_RX_SEL_BIT			12
#define JL21X1C_CFG1_ANA_DLL_RX_SEL_MASK		BIT(12)
#define JL21X1C_CFG1_ANA_DLL_TX_SEL_BIT			11
#define JL21X1C_CFG1_ANA_DLL_TX_SEL_MASK		BIT(11)
#define JL21X1C_CFG1_ANA_DLL_TX_INV_SEL_BIT		10
#define JL21X1C_CFG1_ANA_DLL_TX_INV_SEL_MASK	BIT(10)

// page 41 reg 16 PHY TOP Reserved 0 REG
#define JL21X1C_TOP_RSVD_0_REG						16

// page 41 reg 17 PHY TOP Reserved 1 REG
#define JL21X1C_TOP_RSVD_1_REG						17

// page 41 reg 18 PHY TOP Reserved 2 REG
#define JL21X1C_TOP_RSVD_2_REG						18

// page 41 reg 22 PHY TOP Reserved 6 REG
#define JL21X1C_TOP_RSVD_6_REG						22

// page 45 reg 22 PHY TOP GEPHY RSTN REG
#define JL21X1C_TOP_GEPHY_RSTN_REG					22

// page 46 reg 17 PHY RGMII Delay Config 2 REG
#define JL21X1C_RGMII_DLY_CFG_2_REG				17
#define JL21X1C_CFG2_RGMII_RX_DCR_SEL_BIT		8
#define JL21X1C_CFG2_RGMII_RX_DCR_SEL_MASK		BIT(8)
#define JL21X1C_CFG2_RGMII_DLY_SEL_TX_BIT		4
#define JL21X1C_CFG2_RGMII_DLY_SEL_TX_MASK		BIT(4)
#define JL21X1C_CFG2_RGMII_DLY_SEL_RX_BIT		0
#define JL21X1C_CFG2_RGMII_DLY_SEL_RX_MASK		BIT(0)

// page 128 reg 28 PHY 1000BASE-T Fast Link Down REG
#define JL2XX1_FLD_CTRL_REG			28
#define JL2XX1_FLD_EN_BIT			13
#define JL2XX1_FLD_EN_MASK			BIT(13)
#define JL2XX1_FLD_DELAY_MASK_BIT	11
#define JL2XX1_FLD_DELAY_MASK		(3 << 11)
#define JL2XX1_FLD_DELAY_00MS		0
#define JL2XX1_FLD_DELAY_10MS		1
#define JL2XX1_FLD_DELAY_20MS		2
#define JL2XX1_FLD_DELAY_40MS		3

// page 158 reg 16 PHY INTR PIN REG
#define JL2XX1_INTR_PIN_REG			16
#define JL2XX1_INTR_PIN_EN			BIT(14)

// page 160 reg 17 PHY Global Configure 1 REG
#define JL2XX1_GLOBAL_CONFIG1_REG	17

// page 160 reg 21 PHY PIN EN REG
#define JL2XX1_PIN_EN_REG			21
#define JL2XX1_PIN_OUTPUT			BIT(11)

// page 160 reg 25 PHY Global Configure 9 REG
#define JL2XX1_GLOBAL_CONFIG9_REG	25
#define JL2XX1_GLOBAL_CPU_RESET		BIT(3)

// page 173 reg 16 PHY LOAD GO REG
#define JL2XX1_LOAD_GO_REG			16
#define JL2XX1_LOAD_GO				0

// page 173 reg 17 PHY LOAD DATA0 REG
#define JL2XX1_LOAD_DATA0_REG		17
#define JL2XX1_LOAD_DATA0			0x3a01

// page 174 reg 16 PHY Patch REG
#define JL2XX1_PATCH_REG			16
#define JL2XX1_PATCH_MASK			0xffff

// page 179 reg 16 PHY Patch Regval REG
#define JL2XX1_REG_16				16

// page 181 reg 18 PHY BASE1000X Status REG
#define JL2XX1_BASE1000X_STATUS_REG	18
#define JL2XX1_BASE1000X_LINK		BIT(2)

// page 190 reg 23 PHY Special Reserved REG
#define JL2XX1_PHY_RSVD6_REG		23

// page 257 reg 29 PHY ANA PM0 REG
#define JL2XX1_ANA_PM0_REG			29
#define JL2XX1_BUCK_SEL_VOUT_CFG	BIT(6)

// page 259 reg 29 PHY CPU EMEM Control REG
#define JL2XX1_CPU_EMEM_CTRL_REG	29

// page 259 reg 30 PHY CPU EMEM Write Data REG
#define JL2XX1_CPU_EMEM_WDATA_REG	30

// page 282 reg 17 PHY Downshift REG, refer to page 0 reg 17
#define JL21X1C_DSFT_CTRL_REG		17

// page 282 reg 20 PHY External/PMD Local Loopback Mode REG
#define JL21X1C_LPBK_MODE_REG		20
#define JL21X1C_LPBK_SELECT_BIT		1
#define JL21X1C_LPBK_SELECT_MASK	BIT(1)
#define JL21X1C_LPBK_NO				0
#define JL21X1C_LPBK_EXT			1
#define JL21X1C_LPBK_PMD			2

// page 287 reg 21 PHY GEPHY Reserved 13 REG
#define JL21X1C_GEPHY_RSVD_13_REG		21

// page 289 reg 30 PHY ADC DAC Force Control REG
#define JL21X1C_ADC_DAC_FORCE_CTRL_REG	30

// page 2052 reg 16 PHY WOL Control 0 REG
#define JL21X1C_WOL_CTRL0_REG			16
#define JL21X1C_WOL_POLARITY		BIT(11)
#define JL21X1C_WOL_EN				BIT(10)
#define JL21X1C_WOL_DA_CK_EN		BIT(7)
#define JL21X1C_WOL_SPEED_BIT		(5)
#define JL21X1C_WOL_SPEED_MASK		(0x3 << 5)
#define JL21X1C_WOL_SPEED_10M		(0 << 5)
#define JL21X1C_WOL_SPEED_100M		(1 << 5)
#define JL21X1C_WOL_SPEED_1000M		(2 << 5)
#define JL21X1C_WOL_P2UP_EN			BIT(4)
#define JL21X1C_WOL_LE_EN			BIT(3)
#define JL21X1C_WOL_WUF_EN			BIT(2)
#define JL21X1C_WOL_MP_CK_EN		BIT(1)
#define JL21X1C_WOL_MP_EN			BIT(0)
#define JL21X1C_WOL_FUNC_EN_MASK	((JL21X1C_WOL_MP_EN) | \
									 (JL21X1C_WOL_WUF_EN) | \
									 (JL21X1C_WOL_LE_EN) | \
									 (JL21X1C_WOL_P2UP_EN))

// page 2052 reg 19 PHY WOL RF 0 REG, DST_ADDR_HIGH[47:32]
#define JL21X1C_WOL_RF0_REG			19

// page 2052 reg 20 PHY WOL RF 1 REG, DST_ADDR_MID[31:16]
#define JL21X1C_WOL_RF1_REG			20

// page 2052 reg 21 PHY WOL RF 2 REG, DST_ADDR_LOW[15:0]
#define JL21X1C_WOL_RF2_REG			21

// page 2055 reg 17 PHY 1000BASE-T Fast Link Down REG
#define JL21X1C_FLD_CTRL_REG		17
#define JL21X1C_FLD_DELAY_MASK_BIT	1
#define JL21X1C_FLD_DELAY_MASK		(3 << 1)
#define JL21X1C_FLD_DELAY_00MS			0
#define JL21X1C_FLD_DELAY_10MS			1
#define JL21X1C_FLD_DELAY_20MS			2
#define JL21X1C_FLD_DELAY_40MS			3
#define JL21X1C_FLD_EN_BIT			0
#define JL21X1C_FLD_EN_MASK			BIT(0)

// page 2626 reg 18 PHY INTR Control REG
#define JL2XX1_INTR_CTRL_REG		18
#define JL2XX1_INTR_LINK_CHANGE		BIT(4)
#define JL2XX1_INTR_AN_COMPLETE		BIT(3)
#define JL2XX1_INTR_AN_PAGE			BIT(2)
#define JL2XX1_INTR_AN_ERR			BIT(0)

// page 2627 reg 25 PHY CLK Control REG
#define JL2XX1_CLK_CTRL_REG			25
#define JL2XX1_CLK_SRC				BIT(12)
#define JL2XX1_CLK_125M_OUT			BIT(11)
#define JL2XX1_CLK_SSC_EN			BIT(3)
#define JL2XX1_CLK_OUT_PIN			BIT(0)

// page 2627 reg 29 PHY INTR Status REG
#define JL2XX1_INTR_STATUS_REG		29

// page 3332 reg 16 PHY LED Control REG
#define JL2XX1_LED_CTRL_REG			16
#define JL2XX1_SUPP_LED_MODE	(JL2XXX_LED0_LINK10 | \
				 JL2XXX_LED0_LINK100 | \
				 JL2XXX_LED0_LINK1000 | \
				 JL2XXX_LED0_ACTIVITY | \
				 JL2XXX_LED1_LINK10 | \
				 JL2XXX_LED1_LINK100 | \
				 JL2XXX_LED1_LINK1000 | \
				 JL2XXX_LED1_ACTIVITY | \
				 JL2XXX_LED2_LINK10 | \
				 JL2XXX_LED2_LINK100 | \
				 JL2XXX_LED2_LINK1000 | \
				 JL2XXX_LED2_ACTIVITY)

// page 3336 reg 17 PHY RGMII Control REG
#define JL2XX1_RGMII_CTRL_REG		17
#define JL2XX1_RGMII_RX_DLY_2NS_BIT	BIT(9)
#define JL2XX1_RGMII_TX_DLY_2NS_BIT	BIT(8)

// page 4096 reg 19 PHY LED Polarity REG
#define JL2XX1_LED_POLARITY_REG		19

// page 4096 reg 20 PHY LED Blink REG
#define JL2XX1_LED_BLINK_REG		20
#define JL2XX1_LED_PERIOD_MASK		0xff00
#define JL2XX1_LEDPERIOD(n)			((n << 8) & JL2XX1_LED_PERIOD_MASK)
#define JL2XX1_LED_ON_MASK			0x00ff
#define JL2XX1_LEDON(n)				((n << 0) & JL2XX1_LED_ON_MASK)

// page 4608 reg 16 PHY WOL Control REG
#define JL2XX1_WOL_STAS_REG			16
#define JL2XX1_WOL_EN				BIT(15)
#define JL2XX1_WOL_POLARITY			BIT(14)
#define JL2XX1_WOL_EVENT			BIT(1)

// page 4608 reg 17~19 PHY MAC ADDR2~0 REG
#define JL2XX1_MAC_ADDR2_REG		17
#define JL2XX1_MAC_ADDR1_REG		18
#define JL2XX1_MAC_ADDR0_REG		19

/*************************************************************************/

struct jl_patch {
	const u32 *data;
	u16 data_len;
	const u16 version;
	struct {
		const u16 *info;
		u16 info_len;
	} phy;
	bool (*check)(struct phy_device *phydev, struct jl_patch *patch);
	int (*load)(struct phy_device *phydev, struct jl_patch *patch);
	int (*verify)(struct phy_device *phydev, struct jl_patch *patch);
};

struct jl_hw_stat {
	const char *string;
	u8 reg;
	u16 page;
	u16 mask;
	u16 enable;
};

static const struct jl_hw_stat jl_phy[] = {
	{
		.string = "page0,reg0",
		.enable = true,
		.page = 0,
		.reg = 0,
	}, {
		.string = "page0,reg1",
		.enable = false,
		.page = 0,
		.reg = 1,
	},
};

static const struct jl_hw_stat jl2xxx_hw_stats[] = {
	{
		.string	= "phy_patch_version",
		.reg	= JL2XX1_PATCH_REG,
		.page	= JL2XX1_PAGE_174,
		.mask	= JL2XX1_PATCH_MASK,
	}, {
		.string	= "phy_software_version",
		.reg	= JL2XX1_PHY_INFO_REG,
		.page	= JL2XX1_PAGE_0,
		.mask	= JL2XX1_SW_MASK,
	},
};

enum {
	JL_SPEED_10M	= 0,
	JL_SPEED_100M	= 1,
	JL_SPEED_1000M	= 2,
};

struct jl_led_ctrl {
	u32 enable;			/* LED control enable */
	u32 mode;			/* LED work mode */
	u32 global_period;	/* LED global twinkle period */
	u32 global_on;		/* LED global twinkle hold on time */
	u32 gpio_output;	/* LED is used as gpio output */
	u32 polarity;		/* LED polarity */
	bool ethtool;		/* Whether the ethtool is supported */
};

struct jl_fld_ctrl {
	u32 enable;			/* Fast link down control enable */
	u32 delay;			/* Fast link down time */
	bool ethtool;		/* Whether the ethtool is supported */
};

struct jl_wol_ctrl {
	u32 enable;			/* Wake On LAN control enable */
	bool ethtool;		/* Whether the ethtool is supported */
};

struct jl_intr_ctrl {
	u32 enable;			/* Interrupt control enable */
	bool ethtool;		/* Whether the ethtool is supported */
};

struct jl_downshift_ctrl {
	u32 enable;			/* Downshift control enable */
	u32 count;			/* Downshift control count */
	bool ethtool;		/* Whether the ethtool is supported */
};

struct jl_rgmii_ctrl {
	u32 enable;			/* Rgmii control enable */
	u32 rx_delay;		/* Rgmii control rx delay */
	u32 tx_delay;		/* Rgmii control tx delay */
	u32 rx_drive;		/* Rgmii control rx drive */
	bool ethtool;		/* Whether the ethtool is supported */
};

struct jl_clk_ctrl {
	u32 enable;			/* Clock 125M control enable */
	bool ethtool;		/* Whether the ethtool is supported */
};

struct jl_interface_mode_ctrl {
	u32 enable;			/* Interface mode control enable */
	u32 mode;			/* Interface mode select mode */
	bool ethtool;		/* Whether the ethtool is supported */
};

struct jl_loopback_ctrl {
	u32 enable;			/* Loopback control enable */
	u32 mode;			/* Loopback select mode */
	bool ethtool;		/* Whether the ethtool is supported */
};

struct jl_mdi_ctrl {
	u32 enable;			/* Mdi control enable */
	u32 rate;			/* Mdi select Rate */
	u32 amplitude;		/* Mdi select amplitude */
	bool ethtool;		/* Whether the ethtool is supported */
};

struct jl_rmii_ctrl {
	u32 enable;			/* Rmii control enable */
	u32 tx_timing;		/* Rmii modify tx timing */
	u32 rx_timing;		/* Rmii modify rx timing */
	bool ethtool;		/* Whether the ethtool is supported */
};

struct jl_rxc_out_ctrl {
	u32 enable;			/* Rx clock out control enable */
	bool inited;		/* Only execute once flag */
	bool ethtool;		/* Whether the ethtool is supported */
};

struct jlsemi_priv {
	struct jl_led_ctrl led;
	struct jl_fld_ctrl fld;
	struct jl_wol_ctrl wol;
	struct jl_intr_ctrl intr;
	struct jl_downshift_ctrl downshift;
	struct jl_mdi_ctrl mdi;
	struct jl_rmii_ctrl rmii;
	struct jl_rgmii_ctrl rgmii;
	struct jl_clk_ctrl clk;
	const struct jl_hw_stat *hw_stats;
	bool static_inited;	/* Initialization flag */
	int nstats;			/* Record for dynamic operation */
	u64 *stats;			/* Pointer for dynamic operation */
	struct jl_interface_mode_ctrl itf_mode;
	struct jl_loopback_ctrl lpbk;
	struct jl_rxc_out_ctrl rxc_out;
};

/* macros to simplify debug checking */
enum {
	_DBG_OFF	= 0,
	_DBG_ERROR	= 1,
	_DBG_WARN	= 2,
	_DBG_INFO	= 3,
	_DBG_DEBUG	= 4,
};

static const char * const _jl_phy_dbg_token[] = {
	"Null",
	"Err",
	"Warn",
	"Info",
	"Debug"
};

#define JL_PHY_DBG_TOKEN(r)          \
	_jl_phy_dbg_token[(((int32_t)r) > 0 && ((int)r) <= _DBG_DEBUG) ? (r) : _DBG_OFF]

#define JL_PHY_DBG_MSG(level, fmt, ...) \
	do { \
		if (level <= JL_SUPPORT_PHY_DBG_LEVEL) { \
			printk(KERN_ALERT"[%s][%s:%d] "fmt, JL_PHY_DBG_TOKEN(level), __func__, __LINE__, ##__VA_ARGS__); \
		} \
	} while (0)


/************************* JLSemi iteration code *************************/
struct device *jlsemi_mdio_device_get(struct phy_device *phydev);

u16 jl2xx1_interface_mode_get(struct phy_device *phydev);

int jl2xx1_interface_mode_set(struct phy_device *phydev, u16 mode);

int jl2xx1_fld_dynamic_op_get(struct phy_device *phydev, u8 *msecs);

int jl2xx1_fld_dynamic_op_set(struct phy_device *phydev, const u8 *msecs);

int jl2xx1_downshift_dynamic_op_get(struct phy_device *phydev, u8 *data);

int jl2xx1_downshift_dynamic_op_set(struct phy_device *phydev, u8 cnt);

int jlsemi_intr_ack_event(struct phy_device *phydev);

int jlsemi_intr_static_op_set(struct phy_device *phydev);

int jlsemi_wol_dynamic_op_get(struct phy_device *phydev);

int jlsemi_wol_dynamic_op_set(struct phy_device *phydev);

int jlsemi_operation_args_get(struct phy_device *phydev);

int jlsemi_static_op_init(struct phy_device *phydev);

int jlsemi_soft_reset(struct phy_device *phydev);

bool jl2xx1_read_fiber_serdes_status(struct phy_device *phydev);

int jl2xx1_config_aneg_fiber(struct phy_device *phydev);

/********************** Convenience function for phy **********************/

/* Notice: You should change page 0 when you When you call it after */
int jlsemi_write_page(struct phy_device *phydev, int page);

int jlsemi_read_page(struct phy_device *phydev);

int jlsemi_modify_paged_reg(struct phy_device *phydev,
				int page, u32 regnum,
				u16 mask, u16 set);

int jlsemi_set_bits(struct phy_device *phydev,
			int page, u32 regnum, u16 val);

int jlsemi_clear_bits(struct phy_device *phydev,
			int page, u32 regnum, u16 val);

int jlsemi_fetch_bit(struct phy_device *phydev,
			int page, u32 regnum, u16 val);

int jlsemi_read_paged(struct phy_device *phydev,
			int page, u32 regnum);

int jlsemi_write_paged(struct phy_device *phydev,
			int page, u32 regnum, u16 value);

int jlsemi_drivers_register(struct phy_driver *phydrvs, int size);

void jlsemi_drivers_unregister(struct phy_driver *phydrvs, int size);

/**
 * module_jlsemi_driver() - Helper macro for registering PHY drivers
 * @__phy_drivers: array of PHY drivers to register
 *
 * Helper macro for PHY drivers which do not do anything special in module
 * init/exit. Each module may only use this macro once, and calling it
 * replaces module_init() and module_exit().
 */
#if (JLSEMI_PHY_DRV_REGISTER)

#define jlsemi_module_driver(__phy_drivers, __count)			\
static int __init phy_module_init(void)					\
{									\
	JL_PHY_DBG_MSG(_DBG_INFO, "JLSemi PHY driver (%s) module is loaded: %s.\n",	\
	JLSEMI_DRIVER_VERSION, (JLSEMI_KERNEL_DEVICE_TREE_USE ? "DTS" : "MACRO"));	\
	return jlsemi_drivers_register(__phy_drivers, __count);		\
}									\
module_init(phy_module_init);						\
static void __exit phy_module_exit(void)				\
{									\
	JL_PHY_DBG_MSG(_DBG_INFO, "JLSemi PHY driver (%s) module is removed: %s.\n",	\
	JLSEMI_DRIVER_VERSION, (JLSEMI_KERNEL_DEVICE_TREE_USE ? "DTS" : "MACRO"));	\
	jlsemi_drivers_unregister(__phy_drivers, __count);		\
}									\
module_exit(phy_module_exit)

#define module_jlsemi_driver(__phy_drivers)				\
	jlsemi_module_driver(__phy_drivers, ARRAY_SIZE(__phy_drivers))

#else

#define jlsemi_module_driver(__phy_drivers, __count)			\
static int __init jlsemi_phy_module_init(void)					\
{									\
	JL_PHY_DBG_MSG(_DBG_INFO, "JLSemi PHY driver (%s) module is loaded: %s.\n",	\
	JLSEMI_DRIVER_VERSION, (JLSEMI_KERNEL_DEVICE_TREE_USE ? "DTS" : "MACRO"));	\
	return phy_drivers_register(__phy_drivers, __count, THIS_MODULE); \
}									\
module_init(jlsemi_phy_module_init);						\
static void __exit jlsemi_phy_module_exit(void)				\
{									\
	JL_PHY_DBG_MSG(_DBG_INFO, "JLSemi PHY driver (%s) module is removed: %s.\n",	\
	JLSEMI_DRIVER_VERSION, (JLSEMI_KERNEL_DEVICE_TREE_USE ? "DTS" : "MACRO"));	\
	phy_drivers_unregister(__phy_drivers, __count);			\
}									\
module_exit(jlsemi_phy_module_exit)

#define module_jlsemi_driver(__phy_drivers)				\
	jlsemi_module_driver(__phy_drivers, ARRAY_SIZE(__phy_drivers))

#endif

#endif /* _JLSEMI_CORE_H */

