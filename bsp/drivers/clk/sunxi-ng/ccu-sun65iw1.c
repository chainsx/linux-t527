// SPDX-License-Identifier: GPL-2.0-or-later
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Copyright (c) 2023 rengaomin@allwinnertech.com
 */

#include <dt-bindings/clock/sun65iw1-ccu.h>
#include <linux/clk-provider.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>

#include "ccu_common.h"
#include "ccu_reset.h"

#include "ccu_div.h"
#include "ccu_gate.h"
#include "ccu_mp.h"
#include "ccu_mult.h"
#include "ccu_nk.h"
#include "ccu_nkm.h"
#include "ccu_nkmp.h"
#include "ccu_nm.h"
#include "ccu_sdm.h"
#include "ccu-sun65iw1.h"

#define SUNXI_CCU_VERSION		"0.1.12"
#define UPD_KEY_VALUE 			0x8000000
#define AHB_MASTER_KEY_VALUE	0xE
#define MBUS_MASTER_KEY_VALUE	0x26
#define MBUS_CLOCK_GATE_KEY_VALUE	0x302

/*
 * The CPU PLL is actually NP clock, with P being /1, /2 or /4. However
 * P should only be used for output frequencies lower than 288 MHz.
 *
 * For now we can just model it as a multiplier clock, and force P to /1.
 *
 * The M factor is present in the register's description, but not in the
 * frequency formula, and it's documented as "M is only used for backdoor
 * testing", so it's not modelled and then force to 0.
 */

/* ccu_des_start */

#define SUN65IW1_PLL_PERI0_CTRL_REG   0x00A0
static struct ccu_nm pll_peri0_parent_clk = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.ldo_en			= BIT(30),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_MIN_MAX(8, 8, 53, 105),
	.min_rate		= 1272000000,
	.max_rate		= 2520000000,
	.sdm			= _SUNXI_CCU_SDM_INFO(BIT(24), 0x00A8),
	.common			= {
		.reg		= 0x00A0,
		.hw.init	= CLK_HW_INIT("pll-peri0-parent", "dcxo24M",
				&ccu_nm_ops,
				CLK_SET_RATE_UNGATE |
				CLK_IS_CRITICAL),
	},
};

static SUNXI_CCU_M(pll_peri0_2x_clk, "pll-peri0-2x",
			"pll-peri0-parent", 0x00A0,
			16, 2, 0);

static SUNXI_CCU_M(pll_peri0_800m_clk, "pll-peri0-800m",
			"pll-peri0-parent", 0x00A0,
			20, 3, 0);

static SUNXI_CCU_M(pll_peri0_480m_clk, "pll-peri0-480m",
			"pll-peri0-parent", 0x00A0,
			2, 3, 0);

static CLK_FIXED_FACTOR(pll_peri0_600m_clk, "pll-peri0-600m",
			"pll-peri0-2x", 2, 1, 0);

static CLK_FIXED_FACTOR(pll_peri0_400m_clk, "pll-peri0-400m",
			"pll-peri0-2x", 3, 1, 0);

static CLK_FIXED_FACTOR(pll_peri0_300m_clk, "pll-peri0-300m",
			"pll-peri0-600m", 2, 1, 0);

static CLK_FIXED_FACTOR(pll_peri0_200m_clk, "pll-peri0-200m",
			"pll-peri0-400m", 2, 1, 0);

static CLK_FIXED_FACTOR(pll_peri0_160m_clk, "pll-peri0-160m",
			"pll-peri0-480m", 3, 1, 0);

static CLK_FIXED_FACTOR(pll_peri0_150m_clk, "pll-peri0-150m",
			"pll-peri0-300m", 2, 1, 0);

static CLK_FIXED_FACTOR(pll_peri0_16m_clk, "pll-peri0-16m",
			"pll-peri0-160m", 10, 1, 0);

#define SUN65IW1_PLL_PERI1_CTRL_REG   0x00C0
static struct ccu_nm pll_peri1_parent_clk = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.ldo_en			= BIT(30),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_MIN_MAX(8, 8, 53, 105),
	.min_rate		= 1272000000,
	.max_rate		= 2520000000,
	.sdm			= _SUNXI_CCU_SDM_INFO(BIT(24), 0x00C8),
	.common			= {
		.reg		= 0x00C0,
		.hw.init	= CLK_HW_INIT("pll-peri1-parent", "dcxo24M",
				&ccu_nm_ops,
				CLK_SET_RATE_UNGATE |
				CLK_IGNORE_UNUSED),
	},
};

static SUNXI_CCU_M(pll_peri1_2x_clk, "pll-peri1-2x",
			"pll-peri1-parent", 0x00C0,
			16, 3, 0);

static SUNXI_CCU_M(pll_peri1_800m_clk, "pll-peri1-800m",
			"pll-peri1-parent", 0x00C0,
			20, 3, 0);

static SUNXI_CCU_M(pll_peri1_480m_clk, "pll-peri1-480m",
			"pll-peri1-parent", 0x00C0,
			2, 3, 0);

static CLK_FIXED_FACTOR(pll_peri1_600m_clk, "pll-peri1-600m",
			"pll-peri1-2x", 2, 1, 0);

static CLK_FIXED_FACTOR(pll_peri1_400m_clk, "pll-peri1-400m",
			"pll-peri1-2x", 3, 1, 0);

static CLK_FIXED_FACTOR(pll_peri1_300m_clk, "pll-peri1-300m",
			"pll-peri1-600m", 2, 1, 0);

static CLK_FIXED_FACTOR(pll_peri1_200m_clk, "pll-peri1-200m",
			"pll-peri1-400m", 2, 1, 0);

static CLK_FIXED_FACTOR(pll_peri1_160m_clk, "pll-peri1-160m",
			"pll-peri1-480m", 3, 1, 0);

static CLK_FIXED_FACTOR(pll_peri1_150m_clk, "pll-peri1-150m",
			"pll-peri1-300m", 2, 1, 0);

#define SUN65IW1_PLL_GPU_CTRL_REG   0x00E0
static struct ccu_nm pll_gpu_vco_clk = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.ldo_en			= BIT(30),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_MIN_MAX(8, 8, 53, 105),
	.min_rate		= 1272000000,
	.max_rate		= 2520000000,
	.sdm			= _SUNXI_CCU_SDM_INFO(BIT(24), 0x00E8),
	.common			= {
		.reg		= SUN65IW1_PLL_GPU_CTRL_REG,
		.hw.init	= CLK_HW_INIT("pll-gpu-vco", "dcxo24M",
				&ccu_nm_ops,
				CLK_SET_RATE_UNGATE |
				CLK_IGNORE_UNUSED),
	},
};

static SUNXI_CCU_M(pll_gpu_clk, "pll-gpu",
			"pll-gpu-vco", SUN65IW1_PLL_GPU_CTRL_REG,
			20, 3, CLK_SET_RATE_PARENT);

#define SUN65IW1_PLL_VIDEO0_CTRL_REG   0x0120
static struct ccu_nm pll_video0_parent_clk = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.ldo_en			= BIT(30),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_MIN_MAX(8, 8, 53, 105),
	.min_rate		= 1272000000,
	.max_rate		= 2520000000,
	.sdm			= _SUNXI_CCU_SDM_INFO(BIT(24), 0x0128),
	.common			= {
		.reg		= 0x0120,
		.hw.init	= CLK_HW_INIT("pll-video0-parent", "dcxo24M",
				&ccu_nm_ops,
				CLK_SET_RATE_UNGATE |
				CLK_IGNORE_UNUSED),
	},
};

static SUNXI_CCU_M(pll_video0_4x_clk, "pll-video0-4x",
			"pll-video0-parent", 0x0120,
			20, 3, CLK_SET_RATE_PARENT);

static SUNXI_CCU_M(pll_video0_3x_clk, "pll-video0-3x",
			"pll-video0-parent", 0x0120,
			16, 3, CLK_SET_RATE_PARENT);

#define SUN65IW1_PLL_VIDEO1_CTRL_REG   0x0140
static struct ccu_nm pll_video1_parent_clk = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.ldo_en			= BIT(30),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_MIN_MAX(8, 8, 53, 105),
	.min_rate		= 1272000000,
	.max_rate		= 2520000000,
	.sdm			= _SUNXI_CCU_SDM_INFO(BIT(24), 0x0148),
	.common			= {
		.reg		= 0x0140,
		.hw.init	= CLK_HW_INIT("pll-video1-parent", "dcxo24M",
				&ccu_nm_ops,
				CLK_SET_RATE_UNGATE |
				CLK_IGNORE_UNUSED),
	},
};

static SUNXI_CCU_M(pll_video1_4x_clk, "pll-video1-4x",
			"pll-video1-parent", 0x0140,
			20, 3, 0);

static SUNXI_CCU_M(pll_video1_3x_clk, "pll-video1-3x",
			"pll-video1-parent", 0x0140,
			16, 3, 0);

#define SUN65IW1_PLL_VIDEO2_CTRL_REG   0x0160
static struct ccu_nm pll_video2_parent_clk = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.ldo_en			= BIT(30),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_MIN_MAX(8, 8, 53, 105),
	.min_rate		= 1272000000,
	.max_rate		= 2520000000,
	.sdm			= _SUNXI_CCU_SDM_INFO(BIT(24), 0x0168),
	.common			= {
		.reg		= 0x0160,
		.hw.init	= CLK_HW_INIT("pll-video2-parent", "dcxo24M",
				&ccu_nm_ops,
				CLK_SET_RATE_UNGATE |
				CLK_IGNORE_UNUSED),
	},
};

static SUNXI_CCU_M(pll_video2_4x_clk, "pll-video2-4x",
			"pll-video2-parent", 0x0160,
			20, 3, CLK_SET_RATE_PARENT);

static SUNXI_CCU_M(pll_video2_3x_clk, "pll-video2-3x",
			"pll-video2-parent", 0x0160,
			16, 3, CLK_SET_RATE_PARENT);

#define SUN65IW1_PLL_VE_CTRL_REG   0x0220
static struct ccu_nm pll_ve_vco_clk = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.ldo_en			= BIT(30),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_MIN_MAX(8, 8, 53, 105),
	.min_rate		= 1272000000,
	.max_rate		= 2520000000,
	.sdm			= _SUNXI_CCU_SDM_INFO(BIT(24), 0x0228),
	.common			= {
		.reg		= SUN65IW1_PLL_VE_CTRL_REG,
		.hw.init	= CLK_HW_INIT("pll-ve-vco", "dcxo24M",
				&ccu_nm_ops,
				CLK_SET_RATE_UNGATE),
	},
};

static SUNXI_CCU_M(pll_ve_clk, "pll-ve",
			"pll-ve-vco", SUN65IW1_PLL_VE_CTRL_REG,
			20, 3, CLK_SET_RATE_PARENT);

#define SUN65IW1_PLL_AUDIO0_CTRL_REG   0x0260
static struct ccu_sdm_setting pll_audio0_sdm_table[] = {
	{ .rate = 90316800, .pattern = 0xC002872b, .m = 20, .n = 75 }, /* 22.5792 * 4 */
};

static struct ccu_nm pll_audio0_clk = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.ldo_en			= BIT(30),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_MIN_MAX(8, 8, 65, 130),
	.m			= _SUNXI_CCU_DIV(16, 6), /* M */
	.sdm			= _SUNXI_CCU_SDM(pll_audio0_sdm_table, BIT(24),
				0x268, BIT(31)),
	.common			= {
		.reg		= SUN65IW1_PLL_AUDIO0_CTRL_REG,
		.features	= CCU_FEATURE_SIGMA_DELTA_MOD,
		.hw.init	= CLK_HW_INIT("pll-audio0", "dcxo24M",
				&ccu_nm_ops,
				CLK_SET_RATE_UNGATE | CLK_IGNORE_UNUSED),
	},
};

#define SUN65IW1_PLL_AUDIO1_CTRL_REG   0x0280
static struct ccu_sdm_setting pll_audio1_sdm_table[] = {
	{ .rate = 2359296000, .pattern = 0xC0009BA6, .m = 1, .n = 98 }, /* 24.576 * 96 */
};

static struct ccu_nm pll_audio1_parent_clk = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.ldo_en			= BIT(30),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_MIN_MAX(8, 8, 75, 129),
	.min_rate		= 1800000000,
	.max_rate		= 3096000000,
	.sdm			= _SUNXI_CCU_SDM(pll_audio1_sdm_table, BIT(24),
				0x288, BIT(31)),
	.common			= {
		.reg		= 0x0280,
		.features	= CCU_FEATURE_SIGMA_DELTA_MOD,
		.hw.init	= CLK_HW_INIT("pll-audio1-parent", "dcxo24M",
				&ccu_nm_ops,
				CLK_SET_RATE_UNGATE |
				CLK_IGNORE_UNUSED),
	},
};

static SUNXI_CCU_M(pll_audio1_2x_clk, "pll-audio1-2x",
			"pll-audio1-parent", 0x0280,
			20, 3, CLK_SET_RATE_PARENT);

static SUNXI_CCU_M(pll_audio1_5x_clk, "pll-audio1-5x",
			"pll-audio1-parent", 0x0280,
			16, 3, CLK_SET_RATE_PARENT);

static const char * const ahb_parents[] = { "dcxo24M", "rtc32k", "rc-16m", "pll-peri0-600m" };

static SUNXI_CCU_M_WITH_MUX(ahb_clk, "ahb", ahb_parents,
			0x0500, 0, 5, 24, 2, 0);

static SUNXI_CCU_M_WITH_MUX(apb0_clk, "apb0", ahb_parents,
			0x0510, 0, 5, 24, 2, 0);

static SUNXI_CCU_M_WITH_MUX(apb1_clk, "apb1", ahb_parents,
			0x0518, 0, 5, 24, 2, 0);

static const char * const apb_uart_parents[] = { "dcxo24M", "rtc32k", "rc-16m", "pll-peri0-600m", "pll-peri0-480m" };

static SUNXI_CCU_M_WITH_MUX(apb_uart_clk, "apb-uart", apb_uart_parents,
			0x0538, 0, 5, 24, 3, 0);

static const char * const cpu_sys_dp_parents[] = { "dcxo24M", "pll-peri0-2x", "pll-video0-4x", "pll-peri0-800m", "pll-video0-3x", "dfi-clk" };

static SUNXI_CCU_M_WITH_MUX_GATE(cpu_sys_dp_clk, "cpu-sys-dp",
			cpu_sys_dp_parents, 0x0548,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			CLK_SET_RATE_NO_REPARENT);

static const char * const cpux_gic_parents[] = { "dcxo24M", "rtc32k", "pll-peri0-600m", "pll-peri0-480m", "pll-peri0-400m" };

static SUNXI_CCU_M_WITH_MUX_GATE(cpux_gic_clk, "cpux-gic",
			cpux_gic_parents, 0x0560,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			CLK_IGNORE_UNUSED);

static const char * const nsi_parents[] = { "dcxo24M", "pll-video0-3x", "pll-peri0-600m", "pll-peri0-480m", "pll-peri0-400m", "dfi-clk" };

static SUNXI_CCU_M_WITH_MUX_GATE_KEY(nsi_clk, "nsi",
			nsi_parents, 0x0580,
			0, 5,	/* M */
			24, 3,	/* mux */
			UPD_KEY_VALUE,
			BIT(31),	/* gate */
			CLK_IGNORE_UNUSED | CLK_SET_RATE_NO_REPARENT);

static SUNXI_CCU_GATE(nsi_cfg_bus_clk, "nsi-cfg",
			"dcxo24M",
			0x0584, BIT(0), 0);

static const char * const mbus_parents[] = { "dcxo24M", "pll-peri0-600m", "pll-peri0-480m", "pll-peri0-400m", "dfi-clk" };

static SUNXI_CCU_M_WITH_MUX_GATE_KEY(mbus_clk, "mbus",
			mbus_parents, 0x0588,
			0, 5,	/* M */
			24, 3,	/* mux */
			UPD_KEY_VALUE,
			BIT(31),	/* gate */
			CLK_IGNORE_UNUSED | CLK_SET_RATE_NO_REPARENT);

static SUNXI_CCU_GATE(iommu_apb_bus_clk, "iommu-apb-bus",
			"dcxo24M",
			0x058C, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE_WITH_KEY(pll_stby_sys_peri0_bus_clk, "pll-stby-sys-peri0-bus",
			"dcxo24M",
			0x05C0, AHB_MASTER_KEY_VALUE, BIT(28), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE_WITH_KEY(smhc2_ahb_sw_bus_clk, "smhc2-ahb-sw-bus",
			"dcxo24M",
			0x05C0, AHB_MASTER_KEY_VALUE, BIT(15), 0);

static SUNXI_CCU_GATE_WITH_KEY(smhc1_ahb_sw_bus_clk, "smhc1-ahb-sw-bus",
			"dcxo24M",
			0x05C0, AHB_MASTER_KEY_VALUE, BIT(14), 0);

static SUNXI_CCU_GATE_WITH_KEY(smhc0_ahb_sw_bus_clk, "smhc0-ahb-sw-bus",
			"dcxo24M",
			0x05C0, AHB_MASTER_KEY_VALUE, BIT(13), 0);

static SUNXI_CCU_GATE_WITH_KEY(hsi_ahb_sw_bus_clk, "hsi-ahb-sw-bus",
			"dcxo24M",
			0x05C0, AHB_MASTER_KEY_VALUE, BIT(11), 0);

static SUNXI_CCU_GATE_WITH_KEY(secure_sys_ahb_bus_clk, "secure-sys-ahb-bus",
			"dcxo24M",
			0x05C0, AHB_MASTER_KEY_VALUE, BIT(8), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE_WITH_KEY(gpu_ahb_sw_bus_clk, "gpu-ahb-sw-bus",
			"dcxo24M",
			0x05C0, AHB_MASTER_KEY_VALUE, BIT(7), 0);

static SUNXI_CCU_GATE_WITH_KEY(video_out0_ahb_sw_bus_clk, "video-out0-ahb-sw-bus",
			"dcxo24M",
			0x05C0, AHB_MASTER_KEY_VALUE, BIT(3), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE_WITH_KEY(video_in_ahb_sw_bus_clk, "video-in-ahb-sw-bus",
			"dcxo24M",
			0x05C0, AHB_MASTER_KEY_VALUE, BIT(2), 0);

static SUNXI_CCU_GATE_WITH_KEY(ve0_ahb_sw_bus_clk, "ve0-ahb-sw-bus",
			"dcxo24M",
			0x05C0, AHB_MASTER_KEY_VALUE, BIT(1), 0);

static SUNXI_CCU_GATE_WITH_KEY(dma0_mbus_clk, "dma0-mbus",
			"dcxo24M",
			0x05E0, MBUS_MASTER_KEY_VALUE, BIT(28), 0);

static SUNXI_CCU_GATE_WITH_KEY(ce_sys_axi_bus_clk, "ce-sys-axi-bus",
			"dcxo24M",
			0x05E0, MBUS_MASTER_KEY_VALUE, BIT(8), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE_WITH_KEY(gpu_axi_bus_clk, "gpu-axi-bus",
			"dcxo24M",
			0x05E0, MBUS_MASTER_KEY_VALUE, BIT(7), 0);

static SUNXI_CCU_GATE_WITH_KEY(de_sys_mbus_clk, "de-sys-mbus",
			"dcxo24M",
			0x05E0, MBUS_MASTER_KEY_VALUE, BIT(5), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE_WITH_KEY(video_in_mbus_clk, "video-in-mbus",
			"dcxo24M",
			0x05E0, MBUS_MASTER_KEY_VALUE, BIT(2), 0);

static SUNXI_CCU_GATE_WITH_KEY(ve0_mbus_clk, "ve0-mbus",
			"dcxo24M",
			0x05E0, MBUS_MASTER_KEY_VALUE, BIT(1), 0);

static SUNXI_CCU_GATE_WITH_KEY(gmac0_axi_clk, "gmac0-axi",
			"dcxo24M",
			0x05E4, MBUS_CLOCK_GATE_KEY_VALUE, BIT(11), 0);

static SUNXI_CCU_GATE_WITH_KEY(isp_mbus_clk, "isp-mbus",
			"dcxo24M",
			0x05E4, MBUS_CLOCK_GATE_KEY_VALUE, BIT(9), 0);

static SUNXI_CCU_GATE_WITH_KEY(csi_mbus_clk, "csi-mbus",
			"dcxo24M",
			0x05E4, MBUS_CLOCK_GATE_KEY_VALUE, BIT(8), 0);

static SUNXI_CCU_GATE_WITH_KEY(ce_sys_axi_clk, "ce-sys_axi",
			"dcxo24M",
			0x05E4, MBUS_CLOCK_GATE_KEY_VALUE, BIT(2), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE_WITH_KEY(ve0_mbus_gate_clk, "ve0-mbus-gate",
			"dcxo24M",
			0x05E4, MBUS_CLOCK_GATE_KEY_VALUE, BIT(1), 0);

static SUNXI_CCU_GATE_WITH_KEY(dma0_mbus_gate_clk, "dma0-mbus-gate",
			"dcxo24M",
			0x05E4, MBUS_CLOCK_GATE_KEY_VALUE, BIT(0), 0);

static SUNXI_CCU_GATE(dma0_ahb_bus_clk, "dma0-ahb-bus",
			"dcxo24M",
			0x0704, BIT(0), 0);

static SUNXI_CCU_GATE(spinlock_ahb_bus_clk, "spinlock-ahb-bus",
			"dcxo24M",
			0x0724, BIT(0), 0);

static SUNXI_CCU_GATE(msgbox_cpux_ahb_bus_clk, "msgbox-cpux-ahb-bus",
			"dcxo24M",
			0x0744, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(msgbox_cpus_ahb_bus_clk, "msgbox-cpus-ahb-bus",
			"dcxo24M",
			0x074C, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(pwm0_apb_bus_clk, "pwm0-apb-bus",
			"dcxo24M",
			0x0784, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(dcu_bus_clk, "dcu-bus",
			"dcxo24M",
			0x07A4, BIT(0), 0);

static SUNXI_CCU_GATE(dap_ahb_bus_clk, "dap-ahb-bus",
			"dcxo24M",
			0x07AC, BIT(0), 0);

static const char * const timer0_parents[] = { "dcxo24M", "rc-16m", "rtc32k", "pll-peri0-200m" };

static struct ccu_div timer0_0_clk = {
	.enable			= BIT(31),
	.div			= _SUNXI_CCU_DIV_FLAGS(0, 3, CLK_DIVIDER_POWER_OF_TWO),
	.mux			= _SUNXI_CCU_MUX(24, 3), /* mux */
	.common			= {
		.reg		= 0x0800,
		.hw.init	= CLK_HW_INIT_PARENTS("timer0-0",
				timer0_parents,
				&ccu_div_ops, 0),
	},
};

static struct ccu_div timer0_1_clk = {
	.enable			= BIT(31),
	.div			= _SUNXI_CCU_DIV_FLAGS(0, 3, CLK_DIVIDER_POWER_OF_TWO),
	.mux			= _SUNXI_CCU_MUX(24, 3), /* mux */
	.common			= {
		.reg		= 0x0804,
		.hw.init	= CLK_HW_INIT_PARENTS("timer0-1",
				timer0_parents,
				&ccu_div_ops, 0),
	},
};

static struct ccu_div timer0_2_clk = {
	.enable			= BIT(31),
	.div			= _SUNXI_CCU_DIV_FLAGS(0, 3, CLK_DIVIDER_POWER_OF_TWO),
	.mux			= _SUNXI_CCU_MUX(24, 3), /* mux */
	.common			= {
		.reg		= 0x0808,
		.hw.init	= CLK_HW_INIT_PARENTS("timer0-2",
				timer0_parents,
				&ccu_div_ops, 0),
	},
};

static struct ccu_div timer0_3_clk = {
	.enable			= BIT(31),
	.div			= _SUNXI_CCU_DIV_FLAGS(0, 3, CLK_DIVIDER_POWER_OF_TWO),
	.mux			= _SUNXI_CCU_MUX(24, 3), /* mux */
	.common			= {
		.reg		= 0x080C,
		.hw.init	= CLK_HW_INIT_PARENTS("timer0-3",
				timer0_parents,
				&ccu_div_ops, 0),
	},
};

static SUNXI_CCU_GATE(timer0_ahb_bus_clk, "timer0-ahb-bus",
			"dcxo24M",
			0x0850, BIT(0), 0);

static const char * const de_parents[] = { "pll-peri0-600m", "pll-peri0-480m", "pll-peri0-400m", "pll-ve" };

static SUNXI_CCU_M_WITH_MUX_GATE(de_clk, "de",
			de_parents, 0x0A00,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			CLK_IGNORE_UNUSED | CLK_SET_RATE_NO_REPARENT);

static SUNXI_CCU_GATE(de0_ahb_bus_clk, "de0-ahb-bus",
			"dcxo24M",
			0x0A04, BIT(0), CLK_IGNORE_UNUSED);

static const char * const g2d_parents[] = { "pll-peri0-600m", "pll-peri0-480m", "pll-peri0-400m" };

static SUNXI_CCU_M_WITH_MUX_GATE(g2d_clk, "g2d",
			g2d_parents, 0x0A40,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			CLK_SET_RATE_NO_REPARENT | CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(g2d_ahb_bus_clk, "g2d-ahb-bus",
			"dcxo24M",
			0x0A44, BIT(0), CLK_IGNORE_UNUSED);

static const char * const eink_panel_parents[] = { "pll-peri0-300m", "pll-video0-4x", "pll-video0-3x", "pll-video1-4x", "pll-video1-3x" };

static SUNXI_CCU_M_WITH_MUX_GATE(eink_panel_clk, "eink-panel",
			eink_panel_parents, 0x0A64,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			CLK_SET_RATE_NO_REPARENT | CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(eink_ahb_bus_clk, "eink-ahb-bus",
			"dcxo24M",
			0x0A6C, BIT(0), CLK_IGNORE_UNUSED);

static const char * const ve0_parents[] = { "pll-ve", "pll-peri0-800m", "pll-peri0-600m", "pll-peri0-480m" };

static SUNXI_CCU_M_WITH_MUX_GATE(ve0_clk, "ve0",
			ve0_parents, 0x0A80,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			CLK_SET_RATE_NO_REPARENT | CLK_SET_RATE_PARENT);

static SUNXI_CCU_GATE(ve0_ahb_bus_clk, "ve0-ahb-bus",
			"dcxo24M",
			0x0A8C, BIT(0), 0);

static const char * const ce_sys_parents[] = { "dcxo24M", "pll-peri0-400m", "pll-peri0-480m", "pll-peri0-600m" };

static SUNXI_CCU_M_WITH_MUX_GATE(ce_sys_clk, "ce-sys",
			ce_sys_parents, 0x0AC0,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(ce_sys_ip_ahb_bus_clk, "ce-sys-ip-ahb-bus",
			"dcxo24M",
			0x0AC4, BIT(0), CLK_IGNORE_UNUSED);

static const char * const gpu0_parents[] = { "pll-gpu", "pll-peri0-800m", "pll-peri0-600m", "pll-peri0-400m", "pll-peri0-300m", "pll-peri0-200m" };

static struct clk_div_table gpu_div_table[] = {
    { .val = 0, .div = 1 },
    { .val = 8, .div = 2 },
    { .val = 12, .div = 4 },
    { /* Sentinel */ },
};

static struct ccu_div gpu_clk = {
	.enable			= BIT(31),
	.div			= _SUNXI_CCU_DIV_TABLE(0, 4, gpu_div_table),
	.mux			= _SUNXI_CCU_MUX(24, 3),
	.common			= {
		.reg		= 0x0B20,
		.hw.init	= CLK_HW_INIT_PARENTS("gpu", gpu0_parents,
				&ccu_div_ops,
				CLK_OPS_PARENT_ENABLE | CLK_SET_RATE_NO_REPARENT),
	},
};

static SUNXI_CCU_GATE(gpu_ahb_bus_clk, "gpu-ahb-bus",
			"dcxo24M",
			0x0B24, BIT(0), 0);

static SUNXI_CCU_GATE(dramc_ahb_bus_clk, "dramc-ahb-bus",
			"dcxo24M",
			0x0C0C, BIT(0), CLK_IGNORE_UNUSED);

static const char * const smhc_parents[] = { "dcxo24M", "pll-peri0-400m", "pll-peri0-300m", "pll-peri1-400m", "pll-peri1-300m" };

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(smhc0_clk, "smhc0",
			smhc_parents, 0x0D00,
			0, 5,	/* M */
			8, 5,	/* P */
			24, 3,	/* mux */
			BIT(31), CLK_SET_RATE_NO_REPARENT);

static SUNXI_CCU_GATE(smhc0_ahb_bus_clk, "smhc0-ahb-bus",
			"dcxo24M",
			0x0D0C, BIT(0), 0);

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(smhc1_clk, "smhc1",
			smhc_parents, 0x0D10,
			0, 5,	/* M */
			8, 5,	/* N */
			24, 3,	/* mux */
			BIT(31), CLK_SET_RATE_NO_REPARENT);

static SUNXI_CCU_GATE(smhc1_ahb_bus_clk, "smhc1-ahb-bus",
			"dcxo24M",
			0x0D1C, BIT(0), 0);

static const char * const smhc2_parents[] = { "dcxo24M", "pll-peri0-800m", "pll-peri0-600m", "pll-peri1-800m", "pll-peri1-600m" };

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(smhc2_clk, "smhc2",
			smhc2_parents, 0x0D20,
			0, 5,	/* M */
			8, 5,	/* N */
			24, 3,	/* mux */
			BIT(31), CLK_SET_RATE_NO_REPARENT);

static SUNXI_CCU_GATE(smhc2_ahb_bus_clk, "smhc2-ahb-bus",
			"dcxo24M",
			0x0D2C, BIT(0), 0);

static SUNXI_CCU_GATE(uart0_apb_bus_clk, "uart0-apb-bus",
			"apb-uart",
			0x0E00, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(uart1_apb_bus_clk, "uart1-apb-bus",
			"apb-uart",
			0x0E04, BIT(0), 0);

static SUNXI_CCU_GATE(uart2_apb_bus_clk, "uart2-apb-bus",
			"apb-uart",
			0x0E08, BIT(0), 0);

static SUNXI_CCU_GATE(uart3_apb_bus_clk, "uart3-apb-bus",
			"apb-uart",
			0x0E0C, BIT(0), 0);

static SUNXI_CCU_GATE(uart4_apb_bus_clk, "uart4-apb-bus",
			"apb-uart",
			0x0E10, BIT(0), 0);

static SUNXI_CCU_GATE(uart5_apb_bus_clk, "uart5-apb-bus",
			"apb-uart",
			0x0E14, BIT(0), 0);

static SUNXI_CCU_GATE(uart6_apb_bus_clk, "uart6-apb-bus",
			"apb-uart",
			0x0E18, BIT(0), 0);

static SUNXI_CCU_GATE(uart7_apb_bus_clk, "uart7-apb-bus",
			"apb-uart",
			0x0E1C, BIT(0), 0);

static SUNXI_CCU_GATE(twi0_apb_bus_clk, "twi0-apb-bus",
			"apb1",
			0x0E80, BIT(0), 0);

static SUNXI_CCU_GATE(twi1_apb_bus_clk, "twi1-apb-bus",
			"apb1",
			0x0E84, BIT(0), 0);

static SUNXI_CCU_GATE(twi2_apb_bus_clk, "twi2-apb-bus",
			"apb1",
			0x0E88, BIT(0), 0);

static SUNXI_CCU_GATE(twi3_apb_bus_clk, "twi3-apb-bus",
			"apb1",
			0x0E8C, BIT(0), 0);

static SUNXI_CCU_GATE(twi4_apb_bus_clk, "twi4-apb-bus",
			"apb1",
			0x0E90, BIT(0), 0);

static SUNXI_CCU_GATE(twi5_apb_bus_clk, "twi5-apb-bus",
			"apb1",
			0x0E94, BIT(0), 0);

static const char * const spi_parents[] = { "dcxo24M", "pll-peri0-480m", "pll-peri0-300m", "pll-peri0-200m", "pll-peri1-480m", "pll-peri1-300m", "pll-peri1-200m" };

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(spi0_clk, "spi0",
			spi_parents, 0x0F00,
			0, 5,	/* M */
			8, 5,	/* N */
			24, 3,	/* mux */
			BIT(31), 0);

static SUNXI_CCU_GATE(spi0_ahb_bus_clk, "spi0-ahb",
			"dcxo24M",
			0x0F04, BIT(0), 0);

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(spi1_clk, "spi1",
			spi_parents, 0x0F08,
			0, 5,	/* M */
			8, 5,	/* N */
			24, 3,	/* mux */
			BIT(31), 0);

static SUNXI_CCU_GATE(spi1_ahb_bus_clk, "spi1-ahb-bus",
			"dcxo24M",
			0x0F0C, BIT(0), 0);

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(spi2_clk, "spi2",
			spi_parents, 0x0F10,
			0, 5,	/* M */
			8, 5,	/* N */
			24, 3,	/* mux */
			BIT(31), 0);

static SUNXI_CCU_GATE(spi2_ahb_bus_clk, "spi2-ahb-bus",
			"dcxo24M",
			0x0F14, BIT(0), 0);

static const char * const gpadc0_parents[] = { "dcxo24M", "clk48m", "pll-peri0-480m" };

static SUNXI_CCU_M_WITH_MUX_GATE(gpadc0_clk, "gpadc0",
			gpadc0_parents, 0x0FC0,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(gpadc0_apb_bus_clk, "gpadc0-apb-bus",
			"dcxo24M",
			0x0FC4, BIT(0), 0);

static SUNXI_CCU_GATE(tsensor_apb_bus_clk, "tsensor-apb-bus",
			"dcxo24M",
			0x0FE4, BIT(0), 0);

static const char * const ir_rx0_parents[] = { "rtc32k", "dcxo24M" };

static SUNXI_CCU_M_WITH_MUX_GATE(ir_rx0_clk, "ir-rx0",
			ir_rx0_parents, 0x1000,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(ir_rx0_apb_bus_clk, "ir-rx0-apb-bus",
			"dcxo24M",
			0x1004, BIT(0), 0);

static const char * const ir_tx_parents[] = { "dcxo24M", "pll-peri1-600m" };

static SUNXI_CCU_M_WITH_MUX_GATE(ir_tx_clk, "ir-tx",
			ir_tx_parents, 0x1008,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(ir_tx_apb_bus_clk, "ir-tx-apb-bus",
			"dcxo24M",
			0x100C, BIT(0), 0);

static const char * const i2s_parents[] = { "pll-audio0", "pll-audio1-2x", "pll-audio1-5x", "pll-peri0-200m" };

static SUNXI_CCU_M_WITH_MUX_GATE(i2s0_clk, "i2s0",
			i2s_parents, 0x1200,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			CLK_SET_RATE_NO_REPARENT | CLK_SET_RATE_PARENT);

static SUNXI_CCU_GATE(i2s0_apb_bus_clk, "i2s0-apb-bus",
			"dcxo24M",
			0x120C, BIT(0), 0);

static SUNXI_CCU_M_WITH_MUX_GATE(i2s1_clk, "i2s1",
			i2s_parents, 0x1210,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			CLK_SET_RATE_NO_REPARENT | CLK_SET_RATE_PARENT);

static SUNXI_CCU_GATE(i2s1_apb_bus_clk, "i2s1-apb-bus",
			"dcxo24M",
			0x121C, BIT(0), 0);

static SUNXI_CCU_M_WITH_MUX_GATE(i2s2_clk, "i2s2",
			i2s_parents, 0x1220,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			CLK_SET_RATE_NO_REPARENT | CLK_SET_RATE_PARENT);

static SUNXI_CCU_GATE(i2s2_apb_bus_clk, "i2s2-apb-bus",
			"dcxo24M",
			0x122C, BIT(0), 0);

static SUNXI_CCU_M_WITH_MUX_GATE(i2s3_clk, "i2s3",
			i2s_parents, 0x1230,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			CLK_SET_RATE_NO_REPARENT | CLK_SET_RATE_PARENT);

static SUNXI_CCU_GATE(i2s3_apb_bus_clk, "i2s3-apb-bus",
			"dcxo24M",
			0x123C, BIT(0), 0);

static const char * const owa0_tx_parents[] = { "pll-audio0", "pll-audio1-2x", "pll-audio1-5x" };

static SUNXI_CCU_M_WITH_MUX_GATE(owa0_tx_clk, "owa0-tx",
			owa0_tx_parents, 0x1280,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			CLK_SET_RATE_NO_REPARENT | CLK_SET_RATE_PARENT);

static const char * const owa0_rx_parents[] = { "pll-peri0-400m", "pll-peri0-300m", "pll-audio0", "pll-audio1-2x", "pll-audio1-5x" };

static SUNXI_CCU_M_WITH_MUX_GATE(owa0_rx_clk, "owa0-rx",
			owa0_rx_parents, 0x1284,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			CLK_SET_RATE_NO_REPARENT | CLK_SET_RATE_PARENT);

static SUNXI_CCU_GATE(owa0_apb_bus_clk, "owa0-apb-bus",
			"dcxo24M",
			0x128C, BIT(0), 0);

static const char * const dmic_parents[] = { "pll-audio0", "pll-audio1-2x", "pll-audio1-5x" };

static SUNXI_CCU_M_WITH_MUX_GATE(dmic_clk, "dmic",
			dmic_parents, 0x12C0,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			CLK_SET_RATE_NO_REPARENT | CLK_SET_RATE_PARENT);

static SUNXI_CCU_GATE(dmic_apb_bus_clk, "dmic-apb-bus",
			"dcxo24M",
			0x12CC, BIT(0), 0);

static const char * const audiocodec0_dac_parents[] = { "pll-audio0", "pll-audio1-2x", "pll-audio1-5x" };

static SUNXI_CCU_M_WITH_MUX_GATE(audiocodec0_dac_clk, "audiocodec0-dac",
			audiocodec0_dac_parents, 0x12E0,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			CLK_SET_RATE_NO_REPARENT | CLK_SET_RATE_PARENT);

static const char * const audiocodec0_adc_parents[] = { "pll-audio0", "pll-audio1-2x", "pll-audio1-5x" };

static SUNXI_CCU_M_WITH_MUX_GATE(audiocodec0_adc_clk, "audiocodec0-adc",
			audiocodec0_adc_parents, 0x12E8,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			CLK_SET_RATE_NO_REPARENT | CLK_SET_RATE_PARENT);

static SUNXI_CCU_GATE(audiocodec0_apb_bus_clk, "audiocodec0-apb-bus",
			"dcxo24M",
			0x12EC, BIT(0), 0);

static const char * const audiocodec1_dac_parents[] = { "pll-audio0", "pll-audio1-2x", "pll-audio1-5x" };

static SUNXI_CCU_M_WITH_MUX_GATE(audiocodec1_dac_clk, "audiocodec1-dac",
			audiocodec1_dac_parents, 0x12F0,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			CLK_SET_RATE_NO_REPARENT | CLK_SET_RATE_PARENT);

static SUNXI_CCU_GATE(audiocodec1_apb_bus_clk, "audiocodec1-apb-bus",
			"dcxo24M",
			0x12FC, BIT(0), 0);

static SUNXI_CCU_GATE(usb0_bus_clk, "usb0-bus",
			"dcxo24M",
			0x1300, BIT(31), 0);

static SUNXI_CCU_GATE(usb0_dev_ahb_bus_clk, "usb0-dev-ahb-bus",
			"dcxo24M",
			0x1304, BIT(8), 0);

static SUNXI_CCU_GATE(usb0_ehci_ahb_bus_clk, "usb0-ehci-ahb-bus",
			"dcxo24M",
			0x1304, BIT(4), 0);

static SUNXI_CCU_GATE(usb0_ohci_ahb_bus_clk, "usb0-ohci-ahb-bus",
			"dcxo24M",
			0x1304, BIT(0), 0);

static SUNXI_CCU_GATE(usb1_bus_clk, "usb1-bus",
			"dcxo24M",
			0x1308, BIT(31), 0);

static SUNXI_CCU_GATE(usb1_ehci_ahb_bus_clk, "usb1-ehci-ahb-bus",
			"dcxo24M",
			0x130C, BIT(4), 0);

static SUNXI_CCU_GATE(usb1_ohci_ahb_bus_clk, "usb1-ohci-ahb-bus",
			"dcxo24M",
			0x130C, BIT(0), 0);

static SUNXI_CCU_GATE(usb2p0_sys_phy_ref_bus_clk, "usb2p0-sys-phy-ref-bus",
			"dcxo24M",
			0x1340, BIT(31), 0);

static SUNXI_CCU_GATE(usb2p0_sys_ahb_bus_clk, "usb2p0-sys-ahb-bus",
			"dcxo24M",
			0x1344, BIT(0), 0);

static SUNXI_CCU_GATE(usb2_u2_phy_ref_bus_clk, "usb2-u2-phy-ref-bus",
			"dcxo24M",
			0x1348, BIT(31), 0);

static const char * const usb2_suspend_parents[] = { "rtc32k", "dcxo24M" };

static SUNXI_CCU_M_WITH_MUX_GATE(usb2_suspend_clk, "usb2-suspend",
			usb2_suspend_parents, 0x1350,
			0, 5,	/* M */
			24, 1,	/* mux */
			BIT(31),	/* gate */
			0);

static const char * const usb2_ref_parents[] = { "dcxo24M", "pll-peri0-300m" };

static SUNXI_CCU_M_WITH_MUX_GATE(usb2_ref_clk, "usb2-ref",
			usb2_ref_parents, 0x1354,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			0);

static const char * const usb2_u3_only_utmi_parents[] = { "dcxo24M", "pll-peri0-300m" };

static SUNXI_CCU_M_WITH_MUX_GATE(usb2_u3_only_utmi_clk, "usb2-u3-only-utmi",
			usb2_u3_only_utmi_parents, 0x1360,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			0);

static const char * const usb2_u2_only_pipe_parents[] = { "dcxo24M", "pll-peri0-480m" };

static SUNXI_CCU_M_WITH_MUX_GATE(usb2_u2_only_pipe_clk, "usb2-u2-only-pipe",
			usb2_u2_only_pipe_parents, 0x1364,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			0);

static const char * const pcie0_aux_parents[] = { "dcxo24M", "rtc32k" };

static SUNXI_CCU_M_WITH_MUX_GATE(pcie0_aux_clk, "pcie0-aux",
			pcie0_aux_parents, 0x1380,
			0, 5,	/* M */
			24, 1,	/* mux */
			BIT(31),	/* gate */
			0);

static const char * const pcie0_axi_s_parents[] = { "pll-peri0-300m", "pll-peri0-200m" };

static SUNXI_CCU_M_WITH_MUX_GATE(pcie0_axi_s_clk, "pcie0-axi-s",
			pcie0_axi_s_parents, 0x1384,
			0, 5,	/* M */
			24, 1,	/* mux */
			BIT(31),	/* gate */
			0);

static const char * const hsi_comb0_phy_cfg_parents[] = { "pll-peri0-600m", "pll-peri0-400m" };

static SUNXI_CCU_M_WITH_MUX_GATE(hsi_comb0_phy_cfg_clk, "hsi-comb0-phy-cfg",
			hsi_comb0_phy_cfg_parents, 0x13C0,
			0, 5,	/* M */
			24, 1,	/* mux */
			BIT(31),	/* gate */
			0);

static const char * const hsi_comb0_phy_ref_parents[] = { "dcxo24M", "pll-peri0-200m" };

static SUNXI_CCU_M_WITH_MUX_GATE(hsi_comb0_phy_ref_clk, "hsi-comb0-phy-ref",
			hsi_comb0_phy_ref_parents, 0x13C4,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(hsi_axi_bus_clk, "hsi-axi-bus",
			"dcxo24M",
			0x13CC, BIT(1), 0);

static SUNXI_CCU_GATE(hsi_ahb_bus_clk, "hsi-ahb-bus",
			"dcxo24M",
			0x13CC, BIT(0), 0);

static const char * const hsi_axi_parents[] = { "dcxo24M", "pll-peri0-600m", "pll-peri0-480m", "pll-peri0-400m" };

static SUNXI_CCU_M_WITH_MUX_GATE(hsi_axi_clk, "hsi-axi",
			hsi_axi_parents, 0x13E0,
			0, 5,	/* M */
			24, 2,	/* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_M_WITH_GATE(gmac0_phy_clk, "gmac0-phy",
			"pll-peri0-150m", 0x1400,
			0, 5, BIT(31), 0);

static SUNXI_CCU_GATE(gmac0_ahb_bus_clk, "gmac0-ahb-bus",
			"dcxo24M",
			0x140C, BIT(0), 0);

static const char * const tcon_lcd0_parents[] = { "pll-video0-4x", "pll-video1-4x", "pll-video2-4x", "pll-peri0-2x", "pll-video0-3x", "pll-video1-3x", "pll-video2-3x" };

static SUNXI_CCU_M_WITH_MUX_GATE(tcon_lcd0_clk, "tcon-lcd0",
			tcon_lcd0_parents, 0x1500,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			CLK_IGNORE_UNUSED | CLK_SET_RATE_NO_REPARENT);

static SUNXI_CCU_GATE(tcon_lcd0_ahb_bus_clk, "tcon-lcd0-ahb-bus",
			"dcxo24M",
			0x1504, BIT(0), CLK_IGNORE_UNUSED);

static const char * const mipi_dsi0_parents[] = { "dcxo24M", "pll-peri0-200m", "pll-peri0-150m" };

static SUNXI_CCU_M_WITH_MUX_GATE(mipi_dsi0_clk, "mipi-dsi0",
			mipi_dsi0_parents, 0x1580,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(mipi_dsi0_ahb_bus_clk, "mipi-dsi0-ahb-bus",
			"dcxo24M",
			0x1584, BIT(0), CLK_IGNORE_UNUSED);

static const char * const combophy0_parents[] = { "pll-video0-4x", "pll-video1-4x", "pll-video2-4x", "pll-peri0-2x", "pll-video0-3x", "pll-video1-3x", "pll-video2-3x" };

static SUNXI_CCU_M_WITH_MUX_GATE(combophy0_clk, "combophy0",
			combophy0_parents, 0x15C0,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			CLK_IGNORE_UNUSED | CLK_SET_RATE_NO_REPARENT);

static const char * const tcon_tv0_edp_parents[] = { "pll-video0-4x", "pll-video1-4x", "pll-video2-4x", "pll-peri0-300m", "pll-video0-3x", "pll-video1-3x", "pll-video2-3x" };

static SUNXI_CCU_M_WITH_MUX_GATE(tcon_tv0_edp_clk, "tcon-tv0-edp",
			tcon_tv0_edp_parents, 0x1600,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			CLK_IGNORE_UNUSED | CLK_SET_RATE_NO_REPARENT);

static SUNXI_CCU_GATE(tcon_tv0_ahb_bus_clk, "tcon-tv0-ahb-bus",
			"dcxo24M",
			0x1604, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(edp_ahb_bus_clk, "edp-ahb-bus",
			"dcxo24M",
			0x164C, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(vo0_ahb_bus_clk, "vo0-ahb-bus",
			"dcxo24M",
			0x16C4, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(vo1_ahb_bus_clk, "vo1-ahb-bus",
			"dcxo24M",
			0x16CC, BIT(0), CLK_IGNORE_UNUSED);

static const char * const ledc_parents[] = { "dcxo24M", "pll-peri0-600m" };

static SUNXI_CCU_M_WITH_MUX_GATE(ledc_clk, "ledc",
			ledc_parents, 0x1700,
			0, 5,	/* M */
			24, 1,	/* mux */
			BIT(31),	/* gate */
			0);

static SUNXI_CCU_GATE(ledc_apb_bus_clk, "ledc-apb-bus",
			"dcxo24M",
			0x1704, BIT(0), 0);

static const char * const csi_master_parents[] = { "dcxo24M", "pll-video1-4x", "pll-video1-3x", "pll-video2-4x", "pll-video2-3x", "pll-video0-4x", "pll-video0-3x" };

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(csi_master0_clk, "csi-master0",
			csi_master_parents, 0x1800,
			0, 5,	/* M */
			8, 5,	/* N */
			24, 3,	/* mux */
			BIT(31), CLK_SET_RATE_NO_REPARENT);

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(csi_master1_clk, "csi-master1",
			csi_master_parents, 0x1804,
			0, 5,	/* M */
			8, 5,	/* N */
			24, 3,	/* mux */
			BIT(31), CLK_SET_RATE_NO_REPARENT);

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(csi_master2_clk, "csi-master2",
			csi_master_parents, 0x1808,
			0, 5,	/* M */
			8, 5,	/* N */
			24, 3,	/* mux */
			BIT(31), CLK_SET_RATE_NO_REPARENT);

static const char * const csi_parents[] = { "pll-video2-4x", "pll-video2-3x", "pll-peri0-600m", "pll-peri0-480m", "pll-peri0-400m", "pll-video0-4x", "pll-video1-4x", "pll-ve" };

static SUNXI_CCU_M_WITH_MUX_GATE(csi_clk, "csi",
			csi_parents, 0x1840,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			CLK_SET_RATE_NO_REPARENT | CLK_SET_RATE_PARENT);

static const char * const isp_parents[] = { "pll-video2-4x", "pll-video2-3x", "pll-peri0-600m", "pll-peri0-480m", "pll-peri0-400m", "pll-video0-4x", "pll-video1-4x", "pll-ve" };

static SUNXI_CCU_M_WITH_MUX_GATE(isp_clk, "isp",
			isp_parents, 0x1860,
			0, 5,	/* M */
			24, 3,	/* mux */
			BIT(31),	/* gate */
			CLK_SET_RATE_NO_REPARENT | CLK_SET_RATE_PARENT);

static SUNXI_CCU_GATE(video_in_ahb_bus_clk, "video-in-ahb-bus",
			"dcxo24M",
			0x1884, BIT(0), 0);

#define SUN65IW1_PLL_PERI0_EN_REG 	0x1908
#define SUN65IW1_PLL_PERI1_EN_REG 	0x190C
#define SUN65IW1_PLL_VIDEO_EN_REG 	0x1910
#define SUN65IW1_PLL_GPU_EN_REG		0x1914
#define SUN65IW1_PLL_VE_EN_REG		0x1918
#define SUN65IW1_PLL_AUDIO_EN_REG	0x191C

static SUNXI_CCU_GATE(pll_output_bypass_bus_clk, "pll-output-bypass-bus",
			"dcxo24M",
			0x1A20, BIT(0), 0);

static SUNXI_CCU_GATE(gmac_aximon_bus_clk, "gmac-aximon-bus",
			"dcxo24M",
			0x1C00, BIT(3), 0);

static SUNXI_CCU_GATE(hsi_aximon_bus_clk, "hsi-aximon-bus",
			"dcxo24M",
			0x1C00, BIT(2), 0);

static SUNXI_CCU_GATE(ce_sys_aximon_bus_clk, "ce-sys-aximon-bus",
			"dcxo24M",
			0x1C00, BIT(1), 0);

static SUNXI_CCU_GATE(gpu_aximon_bus_clk, "gpu-aximon-bus",
			"dcxo24M",
			0x1C00, BIT(0), 0);

static SUNXI_CCU_GATE(dcu_aximon_bus_clk, "dcu-aximon-bus",
			"dcxo24M",
			0x1C04, BIT(1), 0);

static SUNXI_CCU_GATE(cpu_sys_aximon_bus_clk, "cpu-sys-aximon-bus",
			"dcxo24M",
			0x1C04, BIT(0), 0);

static SUNXI_CCU_GATE(clk50m_bus_clk, "clk50m",
			"dcxo24M",
			0x1F30, BIT(4), 0);

static SUNXI_CCU_GATE(clk25m_bus_clk, "clk25m",
			"dcxo24M",
			0x1F30, BIT(3), 0);

static SUNXI_CCU_GATE(clk16m_bus_clk, "clk16m",
			"dcxo24M",
			0x1F30, BIT(2), 0);

static SUNXI_CCU_GATE(clk12m_bus_clk, "clk12m",
			"dcxo24M",
			0x1F30, BIT(1), 0);

static SUNXI_CCU_GATE(clk24m_bus_clk, "clk24m",
			"dcxo24M",
			0x1F30, BIT(0), 0);

static const char * const clk27m_parents[] = { "pll-video0-4x", "pll-video1-4x", "pll-video2-4x"};

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(clk27m_clk, "clk27m",
			clk27m_parents, 0x1F34,
			0, 5,	/* M */
			8, 5,	/* P */
			24, 2,	/* mux */
			BIT(31), CLK_SET_RATE_NO_REPARENT);

static const char * const pclk_parents[] = { "apb0"};

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(pclk_clk, "pclk",
			pclk_parents, 0x1F38,
			5, 0,
			5, 5,
			24, 0,	/* mux */
			BIT(31), 0);

static const char * const fanout_rc_16m_parent[] = { "pll-peri0-16m", "rc-16m" };

static SUNXI_CCU_MUX(fanout_rc_16m_clk, "fanout-rc-16m", fanout_rc_16m_parent,
			0x1F3C, 31, 1, 0);

static const char * const fanout_parent[] = { "rtc32k", "clk12m", "clk16m", "clk24m", "clk25m", "clk27m", "pclk", "clk50m"};

static SUNXI_CCU_MUX_WITH_GATE(fanout2_clk, "fanout2",
			fanout_parent, 0x1F3C,
			6, 3,
			BIT(23), 0);

static SUNXI_CCU_MUX_WITH_GATE(fanout1_clk, "fanout1",
			fanout_parent, 0x1F3C,
			3, 3,
			BIT(22), 0);

static SUNXI_CCU_MUX_WITH_GATE(fanout0_clk, "fanout0",
			fanout_parent, 0x1F3C,
			0, 3,
			BIT(21), 0);

static SUNXI_CCU_GATE(ahb_montior_gate_clk, "ahb-montior-gate",
			"dcxo24M",
			0x05C0, BIT(31), CLK_IS_CRITICAL);

static SUNXI_CCU_GATE(sd_montior_gate_clk, "sd-montior-gate",
			"dcxo24M",
			0x05C0, BIT(29), CLK_IS_CRITICAL);

/* for pm resume */
#define SUN65IW1_PLL_PERIPH1_PATTERN0_REG	0x0C8

#ifdef CONFIG_PM_SLEEP
static struct ccu_nm pll_audio0_sdm_pat0_clk = {
	.common		= {
		.reg		= 0x268,
	},
};

static struct ccu_nm pll_audio1_sdm_pat0_clk = {
	.common		= {
		.reg		= 0x288,
	},
};

static struct ccu_nm pll_peri1_sdm_pat0_clk = {
	.common		= {
		.reg		= 0x0C8,
	},
};
#endif
/* ccu_des_end */

/* rst_def_start */
static struct ccu_reset_map sun65iw1_ccu_resets[] = {
	[RST_BUS_NSI_CFG]		= { 0x0584, BIT(17) },
	[RST_BUS_NSI]			= { 0x0584, BIT(16) },
	[RST_BUS_DMA0]			= { 0x0704, BIT(16) },
	[RST_BUS_SPINLOCK]		= { 0x0724, BIT(16) },
	[RST_BUS_MSGBOX_CPUX]		= { 0x0744, BIT(16) },
	[RST_BUS_MSGBOX_CPU]		= { 0x074c, BIT(16) },
	[RST_BUS_PWM0]			= { 0x0784, BIT(16) },
	[RST_BUS_DCU]			= { 0x07a4, BIT(16) },
	[RST_BUS_DAP]			= { 0x07ac, BIT(16) },
	[RST_BUS_TIMER0]		= { 0x0850, BIT(16) },
	[RST_BUS_DE0]			= { 0x0a04, BIT(16) },
	[RST_BUS_G2D]			= { 0x0a44, BIT(16) },
	[RST_BUS_EINK]			= { 0x0a6c, BIT(16) },
	[RST_BUS_VE0]			= { 0x0a8c, BIT(16) },
	[RST_BUS_CE_SY]			= { 0x0ac4, BIT(16) },
	[RST_BUS_GPU]			= { 0x0b24, BIT(16) },
	[RST_BUS_DRAMC]			= { 0x0c0c, BIT(16) },
	[RST_BUS_SMHC0]			= { 0x0d0c, BIT(16) },
	[RST_BUS_SMHC1]			= { 0x0d1c, BIT(16) },
	[RST_BUS_SMHC2]			= { 0x0d2c, BIT(16) },
	[RST_BUS_UART0]			= { 0x0e00, BIT(16) },
	[RST_BUS_UART1]			= { 0x0e04, BIT(16) },
	[RST_BUS_UART2]			= { 0x0e08, BIT(16) },
	[RST_BUS_UART3]			= { 0x0e0c, BIT(16) },
	[RST_BUS_UART4]			= { 0x0e10, BIT(16) },
	[RST_BUS_UART5]			= { 0x0e14, BIT(16) },
	[RST_BUS_UART6]			= { 0x0e18, BIT(16) },
	[RST_BUS_UART7]			= { 0x0e1c, BIT(16) },
	[RST_BUS_TWI0]			= { 0x0e80, BIT(16) },
	[RST_BUS_TWI1]			= { 0x0e84, BIT(16) },
	[RST_BUS_TWI2]			= { 0x0e88, BIT(16) },
	[RST_BUS_TWI3]			= { 0x0e8c, BIT(16) },
	[RST_BUS_TWI4]			= { 0x0e90, BIT(16) },
	[RST_BUS_TWI5]			= { 0x0e94, BIT(16) },
	[RST_BUS_SPI0]			= { 0x0f04, BIT(16) },
	[RST_BUS_SPI1]			= { 0x0f0c, BIT(16) },
	[RST_BUS_SPI2]			= { 0x0f14, BIT(16) },
	[RST_BUS_GPADC0]		= { 0x0fc4, BIT(16) },
	[RST_BUS_TSENSO]		= { 0x0fe4, BIT(16) },
	[RST_BUS_IR_RX0]		= { 0x1004, BIT(16) },
	[RST_BUS_IR_TX]			= { 0x100c, BIT(16) },
	[RST_BUS_I2S0]			= { 0x120c, BIT(16) },
	[RST_BUS_I2S1]			= { 0x121c, BIT(16) },
	[RST_BUS_I2S2]			= { 0x122c, BIT(16) },
	[RST_BUS_I2S3]			= { 0x123c, BIT(16) },
	[RST_BUS_OWA0]			= { 0x128c, BIT(16) },
	[RST_BUS_DMIC]			= { 0x12cc, BIT(16) },
	[RST_BUS_AUDIOCODEC0]		= { 0x12ec, BIT(16) },
	[RST_BUS_AUDIOCODEC1]		= { 0x12fc, BIT(16) },
	[RST_USB0_DEV]			= { 0x1304, BIT(24) },
	[RST_USB0_EHCI]			= { 0x1304, BIT(20) },
	[RST_USB0_OHCI]			= { 0x1304, BIT(16) },
	[RST_USB1_EHCI]			= { 0x130c, BIT(20) },
	[RST_USB1_OHCI]			= { 0x130c, BIT(16) },
	[RST_USB2P0_SY]			= { 0x1344, BIT(16) },
	[RST_USB2]			= { 0x135c, BIT(16) },
	[RST_BUS_PCIE0]			= { 0x138c, BIT(17) },
	[RST_BUS_PCIE0_PWR_UP]		= { 0x138c, BIT(16) },
	[RST_BUS_HSI_SY]		= { 0x13cc, BIT(16) },
	[RST_BUS_GMAC0_AXI]		= { 0x140c, BIT(17) },
	[RST_BUS_GMAC0_AHB]		= { 0x140c, BIT(16) },
	[RST_BUS_TCON_LCD0]		= { 0x1504, BIT(16) },
	[RST_BUS_LVDS0]			= { 0x1544, BIT(16) },
	[RST_BUS_MIPI_DSI0]		= { 0x1584, BIT(16) },
	[RST_BUS_TCON_TV0]		= { 0x1604, BIT(16) },
	[RST_BUS_EDP]			= { 0x164c, BIT(16) },
	[RST_BUS_VO0]			= { 0x16c4, BIT(16) },
	[RST_BUS_VO1]			= { 0x16cc, BIT(16) },
	[RST_BUS_VIDEO_OUT0]		= { 0x16e4, BIT(16) },
	[RST_BUS_LEDC]			= { 0x1704, BIT(16) },
	[RST_BUS_VIDEO_IN]		= { 0x1884, BIT(16) },
	[RST_BUS_GMAC0_AXIMON]		= { 0x1c00, BIT(19) },
	[RST_BUS_HSI_AXIMON]		= { 0x1c00, BIT(18) },
	[RST_BUS_CE_SYS_AXIMON]		= { 0x1c00, BIT(17) },
	[RST_BUS_GPU_AXIMON]		= { 0x1c00, BIT(16) },
	[RST_BUS_DCU_AHBMON]		= { 0x1c04, BIT(17) },
	[RST_BUS_CPU_SYS_AHBMON]	= { 0x1c04, BIT(16) },
};
/* rst_def_end */

/* ccu_def_start */
static struct clk_hw_onecell_data sun65iw1_hw_clks = {
	.hws    = {
		[CLK_PLL_PERI0_PARENT]		= &pll_peri0_parent_clk.common.hw,
		[CLK_PLL_PERI0_2X]		= &pll_peri0_2x_clk.common.hw,
		[CLK_PLL_PERI0_800M]		= &pll_peri0_800m_clk.common.hw,
		[CLK_PLL_PERI0_480M]		= &pll_peri0_480m_clk.common.hw,
		[CLK_PLL_PERI0_600M]		= &pll_peri0_600m_clk.hw,
		[CLK_PLL_PERI0_400M]		= &pll_peri0_400m_clk.hw,
		[CLK_PLL_PERI0_300M]            = &pll_peri0_300m_clk.hw,
		[CLK_PLL_PERI0_200M]		= &pll_peri0_200m_clk.hw,
		[CLK_PLL_PERI0_160M]		= &pll_peri0_160m_clk.hw,
		[CLK_PLL_PERI0_150M]		= &pll_peri0_150m_clk.hw,
		[CLK_PLL_PERI0_16M]		= &pll_peri0_16m_clk.hw,
		[CLK_PLL_PERI1_PARENT]		= &pll_peri1_parent_clk.common.hw,
		[CLK_PLL_PERI1_2X]		= &pll_peri1_2x_clk.common.hw,
		[CLK_PLL_PERI1_800M]		= &pll_peri1_800m_clk.common.hw,
		[CLK_PLL_PERI1_480M]		= &pll_peri1_480m_clk.common.hw,
		[CLK_PLL_PERI1_600M]		= &pll_peri1_600m_clk.hw,
		[CLK_PLL_PERI1_400M]		= &pll_peri1_400m_clk.hw,
		[CLK_PLL_PERI1_300M]            = &pll_peri1_300m_clk.hw,
		[CLK_PLL_PERI1_200M]		= &pll_peri1_200m_clk.hw,
		[CLK_PLL_PERI1_160M]		= &pll_peri1_160m_clk.hw,
		[CLK_PLL_PERI1_150M]		= &pll_peri1_150m_clk.hw,
		[CLK_PLL_GPU_VCO]		= &pll_gpu_vco_clk.common.hw,
		[CLK_PLL_GPU]			= &pll_gpu_clk.common.hw,
		[CLK_PLL_VIDEO0_PARENT]		= &pll_video0_parent_clk.common.hw,
		[CLK_PLL_VIDEO0_4X]		= &pll_video0_4x_clk.common.hw,
		[CLK_PLL_VIDEO0_3X]		= &pll_video0_3x_clk.common.hw,
		[CLK_PLL_VIDEO1_PARENT]		= &pll_video1_parent_clk.common.hw,
		[CLK_PLL_VIDEO1_4X]		= &pll_video1_4x_clk.common.hw,
		[CLK_PLL_VIDEO1_3X]		= &pll_video1_3x_clk.common.hw,
		[CLK_PLL_VIDEO2_PARENT]		= &pll_video2_parent_clk.common.hw,
		[CLK_PLL_VIDEO2_4X]		= &pll_video2_4x_clk.common.hw,
		[CLK_PLL_VIDEO2_3X]		= &pll_video2_3x_clk.common.hw,
		[CLK_PLL_VE_VCO]		= &pll_ve_vco_clk.common.hw,
		[CLK_PLL_VE]			= &pll_ve_clk.common.hw,
		[CLK_PLL_AUDIO0]		= &pll_audio0_clk.common.hw,
		[CLK_PLL_AUDIO1_PARENT]		= &pll_audio1_parent_clk.common.hw,
		[CLK_PLL_AUDIO1_2X]		= &pll_audio1_2x_clk.common.hw,
		[CLK_PLL_AUDIO1_5X]		= &pll_audio1_5x_clk.common.hw,
		[CLK_AHB]			= &ahb_clk.common.hw,
		[CLK_APB0]			= &apb0_clk.common.hw,
		[CLK_APB1]			= &apb1_clk.common.hw,
		[CLK_APB_UART]			= &apb_uart_clk.common.hw,
		[CLK_CPU_SYS_DP]		= &cpu_sys_dp_clk.common.hw,
		[CLK_CPUX_GIC]			= &cpux_gic_clk.common.hw,
		[CLK_NSI]			= &nsi_clk.common.hw,
		[CLK_BUS_NSI_CFG]		= &nsi_cfg_bus_clk.common.hw,
		[CLK_MBUS]			= &mbus_clk.common.hw,
		[CLK_BUS_IOMMU_APB]		= &iommu_apb_bus_clk.common.hw,
		[CLK_BUS_PLL_STBY_SYS_PERI0]	= &pll_stby_sys_peri0_bus_clk.common.hw,
		[CLK_BUS_SMHC2_AHB_SW]		= &smhc2_ahb_sw_bus_clk.common.hw,
		[CLK_BUS_SMHC1_AHB_SW]		= &smhc1_ahb_sw_bus_clk.common.hw,
		[CLK_BUS_SMHC0_AHB_SW]		= &smhc0_ahb_sw_bus_clk.common.hw,
		[CLK_BUS_HSI_AHB_SW]		= &hsi_ahb_sw_bus_clk.common.hw,
		[CLK_BUS_SECURE_SYS_AHB]	= &secure_sys_ahb_bus_clk.common.hw,
		[CLK_BUS_GPU_AHB_SW]		= &gpu_ahb_sw_bus_clk.common.hw,
		[CLK_BUS_VIDEO_OUT0_AHB_SW]	= &video_out0_ahb_sw_bus_clk.common.hw,
		[CLK_BUS_VIDEO_IN_AHB_SW]	= &video_in_ahb_sw_bus_clk.common.hw,
		[CLK_BUS_VE0_AHB_SW]		= &ve0_ahb_sw_bus_clk.common.hw,
		[CLK_MBUS_DMA0]			= &dma0_mbus_clk.common.hw,
		[CLK_BUS_CE_SYS_AXI]		= &ce_sys_axi_bus_clk.common.hw,
		[CLK_BUS_GPU_AXI]		= &gpu_axi_bus_clk.common.hw,
		[CLK_MBUS_DE_SYS]		= &de_sys_mbus_clk.common.hw,
		[CLK_MBUS_VIDEO_IN]		= &video_in_mbus_clk.common.hw,
		[CLK_MBUS_VE0]			= &ve0_mbus_clk.common.hw,
		[CLK_GMAC0_AXI]			= &gmac0_axi_clk.common.hw,
		[CLK_ISP_MBUS]			= &isp_mbus_clk.common.hw,
		[CLK_CIS_MBUS]			= &csi_mbus_clk.common.hw,
		[CLK_CE_SYS_AXI]		= &ce_sys_axi_clk.common.hw,
		[CLK_VE0_MBUS_GATE]		= &ve0_mbus_gate_clk.common.hw,
		[CLK_DMA0_MBUS_GATE]	= &dma0_mbus_gate_clk.common.hw,
		[CLK_BUS_DMA0_AHB]		= &dma0_ahb_bus_clk.common.hw,
		[CLK_BUS_SPINLOCK_AHB]		= &spinlock_ahb_bus_clk.common.hw,
		[CLK_BUS_MSGBOX_CPUX_AHB]	= &msgbox_cpux_ahb_bus_clk.common.hw,
		[CLK_BUS_MSGBOX_CPUS_AHB]	= &msgbox_cpus_ahb_bus_clk.common.hw,
		[CLK_BUS_PWM0_APB]		= &pwm0_apb_bus_clk.common.hw,
		[CLK_BUS_DCU]			= &dcu_bus_clk.common.hw,
		[CLK_BUS_DAP_AHB]		= &dap_ahb_bus_clk.common.hw,
		[CLK_TIMER0_0]			= &timer0_0_clk.common.hw,
		[CLK_TIMER0_1]			= &timer0_1_clk.common.hw,
		[CLK_TIMER0_2]			= &timer0_2_clk.common.hw,
		[CLK_TIMER0_3]			= &timer0_3_clk.common.hw,
		[CLK_BUS_TIMER0_AHB]		= &timer0_ahb_bus_clk.common.hw,
		[CLK_DE]			= &de_clk.common.hw,
		[CLK_BUS_DE0_AHB]		= &de0_ahb_bus_clk.common.hw,
		[CLK_G2D]			= &g2d_clk.common.hw,
		[CLK_BUS_G2D_AHB]		= &g2d_ahb_bus_clk.common.hw,
		[CLK_EINK_PANEL]		= &eink_panel_clk.common.hw,
		[CLK_BUS_EINK_AHB]		= &eink_ahb_bus_clk.common.hw,
		[CLK_VE0]			= &ve0_clk.common.hw,
		[CLK_BUS_VE0_AHB]		= &ve0_ahb_bus_clk.common.hw,
		[CLK_CE_SYS]			= &ce_sys_clk.common.hw,
		[CLK_BUS_CE_SYS_IP_AHB]		= &ce_sys_ip_ahb_bus_clk.common.hw,
		[CLK_GPU]				= &gpu_clk.common.hw,
		[CLK_BUS_GPU_AHB]		= &gpu_ahb_bus_clk.common.hw,
		[CLK_BUS_DRAMC_AHB]		= &dramc_ahb_bus_clk.common.hw,
		[CLK_SMHC0]			= &smhc0_clk.common.hw,
		[CLK_BUS_SMHC0_AHB]		= &smhc0_ahb_bus_clk.common.hw,
		[CLK_SMHC1]			= &smhc1_clk.common.hw,
		[CLK_BUS_SMHC1_AHB]		= &smhc1_ahb_bus_clk.common.hw,
		[CLK_SMHC2]			= &smhc2_clk.common.hw,
		[CLK_BUS_SMHC2_AHB]		= &smhc2_ahb_bus_clk.common.hw,
		[CLK_BUS_UART0_APB]		= &uart0_apb_bus_clk.common.hw,
		[CLK_BUS_UART1_APB]		= &uart1_apb_bus_clk.common.hw,
		[CLK_BUS_UART2_APB]		= &uart2_apb_bus_clk.common.hw,
		[CLK_BUS_UART3_APB]		= &uart3_apb_bus_clk.common.hw,
		[CLK_BUS_UART4_APB]		= &uart4_apb_bus_clk.common.hw,
		[CLK_BUS_UART5_APB]		= &uart5_apb_bus_clk.common.hw,
		[CLK_BUS_UART6_APB]		= &uart6_apb_bus_clk.common.hw,
		[CLK_BUS_UART7_APB]		= &uart7_apb_bus_clk.common.hw,
		[CLK_BUS_TWI0_APB]		= &twi0_apb_bus_clk.common.hw,
		[CLK_BUS_TWI1_APB]		= &twi1_apb_bus_clk.common.hw,
		[CLK_BUS_TWI2_APB]		= &twi2_apb_bus_clk.common.hw,
		[CLK_BUS_TWI3_APB]		= &twi3_apb_bus_clk.common.hw,
		[CLK_BUS_TWI4_APB]		= &twi4_apb_bus_clk.common.hw,
		[CLK_BUS_TWI5_APB]		= &twi5_apb_bus_clk.common.hw,
		[CLK_SPI0]			= &spi0_clk.common.hw,
		[CLK_BUS_SPI0_AHB]		= &spi0_ahb_bus_clk.common.hw,
		[CLK_SPI1]			= &spi1_clk.common.hw,
		[CLK_BUS_SPI1_AHB]		= &spi1_ahb_bus_clk.common.hw,
		[CLK_SPI2]			= &spi2_clk.common.hw,
		[CLK_BUS_SPI2_AHB]		= &spi2_ahb_bus_clk.common.hw,
		[CLK_GPADC0]			= &gpadc0_clk.common.hw,
		[CLK_BUS_GPADC0_APB]		= &gpadc0_apb_bus_clk.common.hw,
		[CLK_BUS_TSENSOR_APB]		= &tsensor_apb_bus_clk.common.hw,
		[CLK_IR_RX0]			= &ir_rx0_clk.common.hw,
		[CLK_BUS_IR_RX0_APB]		= &ir_rx0_apb_bus_clk.common.hw,
		[CLK_IR_TX]			= &ir_tx_clk.common.hw,
		[CLK_BUS_IR_TX_APB]		= &ir_tx_apb_bus_clk.common.hw,
		[CLK_I2S0]			= &i2s0_clk.common.hw,
		[CLK_BUS_I2S0_APB]		= &i2s0_apb_bus_clk.common.hw,
		[CLK_I2S1]			= &i2s1_clk.common.hw,
		[CLK_BUS_I2S1_APB]		= &i2s1_apb_bus_clk.common.hw,
		[CLK_I2S2]			= &i2s2_clk.common.hw,
		[CLK_BUS_I2S2_APB]		= &i2s2_apb_bus_clk.common.hw,
		[CLK_I2S3]			= &i2s3_clk.common.hw,
		[CLK_BUS_I2S3_APB]		= &i2s3_apb_bus_clk.common.hw,
		[CLK_OWA0_TX]			= &owa0_tx_clk.common.hw,
		[CLK_OWA0_RX]			= &owa0_rx_clk.common.hw,
		[CLK_BUS_OWA0_APB]		= &owa0_apb_bus_clk.common.hw,
		[CLK_DMIC]			= &dmic_clk.common.hw,
		[CLK_BUS_DMIC_APB]		= &dmic_apb_bus_clk.common.hw,
		[CLK_AUDIOCODEC0_DAC]		= &audiocodec0_dac_clk.common.hw,
		[CLK_AUDIOCODEC0_ADC]		= &audiocodec0_adc_clk.common.hw,
		[CLK_BUS_AUDIOCODEC0_APB]	= &audiocodec0_apb_bus_clk.common.hw,
		[CLK_AUDIOCODEC1_DAC]		= &audiocodec1_dac_clk.common.hw,
		[CLK_BUS_AUDIOCODEC1_APB]	= &audiocodec1_apb_bus_clk.common.hw,
		[CLK_BUS_USB0]			= &usb0_bus_clk.common.hw,
		[CLK_BUS_USB0_DEV_AHB]		= &usb0_dev_ahb_bus_clk.common.hw,
		[CLK_BUS_USB0_EHCI_AHB]		= &usb0_ehci_ahb_bus_clk.common.hw,
		[CLK_BUS_USB0_OHCI_AHB]		= &usb0_ohci_ahb_bus_clk.common.hw,
		[CLK_BUS_USB1]			= &usb1_bus_clk.common.hw,
		[CLK_BUS_USB1_EHCI_AHB]		= &usb1_ehci_ahb_bus_clk.common.hw,
		[CLK_BUS_USB1_OHCI_AHB]		= &usb1_ohci_ahb_bus_clk.common.hw,
		[CLK_BUS_USB2P0_SYS_PHY_REF]	= &usb2p0_sys_phy_ref_bus_clk.common.hw,
		[CLK_BUS_USB2P0_SYS_AHB]	= &usb2p0_sys_ahb_bus_clk.common.hw,
		[CLK_BUS_USB2_U2_PHY_REF]	= &usb2_u2_phy_ref_bus_clk.common.hw,
		[CLK_USB2_SUSPEND]		= &usb2_suspend_clk.common.hw,
		[CLK_USB2_REF]			= &usb2_ref_clk.common.hw,
		[CLK_USB2_U3_ONLY_UTMI]		= &usb2_u3_only_utmi_clk.common.hw,
		[CLK_USB2_U2_ONLY_PIPE]		= &usb2_u2_only_pipe_clk.common.hw,
		[CLK_PCIE0_AUX]			= &pcie0_aux_clk.common.hw,
		[CLK_PCIE0_AXI_S]		= &pcie0_axi_s_clk.common.hw,
		[CLK_HSI_COMB0_PHY_CFG]		= &hsi_comb0_phy_cfg_clk.common.hw,
		[CLK_HSI_COMB0_PHY_REF]		= &hsi_comb0_phy_ref_clk.common.hw,
		[CLK_BUS_HSI_AXI]		= &hsi_axi_bus_clk.common.hw,
		[CLK_BUS_HSI_AHB]		= &hsi_ahb_bus_clk.common.hw,
		[CLK_HSI_AXI]			= &hsi_axi_clk.common.hw,
		[CLK_GMAC0_PHY]			= &gmac0_phy_clk.common.hw,
		[CLK_BUS_GMAC0_AHB]		= &gmac0_ahb_bus_clk.common.hw,
		[CLK_TCON_LCD0]			= &tcon_lcd0_clk.common.hw,
		[CLK_BUS_TCON_LCD0_AHB]		= &tcon_lcd0_ahb_bus_clk.common.hw,
		[CLK_MIPI_DSI0]			= &mipi_dsi0_clk.common.hw,
		[CLK_BUS_MIPI_DSI0_AHB]		= &mipi_dsi0_ahb_bus_clk.common.hw,
		[CLK_COMBOPHY0]			= &combophy0_clk.common.hw,
		[CLK_TCON_TV0_EDP]		= &tcon_tv0_edp_clk.common.hw,
		[CLK_BUS_TCON_TV0_AHB]		= &tcon_tv0_ahb_bus_clk.common.hw,
		[CLK_BUS_EDP_AHB]		= &edp_ahb_bus_clk.common.hw,
		[CLK_BUS_VO0_AHB]		= &vo0_ahb_bus_clk.common.hw,
		[CLK_BUS_VO1_AHB]		= &vo1_ahb_bus_clk.common.hw,
		[CLK_LEDC]			= &ledc_clk.common.hw,
		[CLK_BUS_LEDC_APB]		= &ledc_apb_bus_clk.common.hw,
		[CLK_CSI_MASTER0]		= &csi_master0_clk.common.hw,
		[CLK_CSI_MASTER1]		= &csi_master1_clk.common.hw,
		[CLK_CSI_MASTER2]		= &csi_master2_clk.common.hw,
		[CLK_CSI]			= &csi_clk.common.hw,
		[CLK_ISP]			= &isp_clk.common.hw,
		[CLK_BUS_VIDEO_IN_AHB]		= &video_in_ahb_bus_clk.common.hw,
		[CLK_BUS_PLL_OUTPUT_BYPASS]	= &pll_output_bypass_bus_clk.common.hw,
		[CLK_BUS_GMAC_AXIMON]		= &gmac_aximon_bus_clk.common.hw,
		[CLK_BUS_HSI_AXIMON]		= &hsi_aximon_bus_clk.common.hw,
		[CLK_BUS_CE_SYS_AXIMON]		= &ce_sys_aximon_bus_clk.common.hw,
		[CLK_BUS_GPU_AXIMON]		= &gpu_aximon_bus_clk.common.hw,
		[CLK_BUS_DCU_AXIMON]		= &dcu_aximon_bus_clk.common.hw,
		[CLK_BUS_CPU_SYS_AXIMON]	= &cpu_sys_aximon_bus_clk.common.hw,
		[CLK_BUS_CLK50M]		= &clk50m_bus_clk.common.hw,
		[CLK_BUS_CLK25M]		= &clk25m_bus_clk.common.hw,
		[CLK_BUS_CLK16M]		= &clk16m_bus_clk.common.hw,
		[CLK_BUS_CLK12M]		= &clk12m_bus_clk.common.hw,
		[CLK_BUS_CLK24M]		= &clk24m_bus_clk.common.hw,
		[CLK_CLK27M]			= &clk27m_clk.common.hw,
		[CLK_PCLK]			= &pclk_clk.common.hw,
		[CLK_FANOUT_RC_16M]		= &fanout_rc_16m_clk.common.hw,
		[CLK_FANOUT2]			= &fanout2_clk.common.hw,
		[CLK_FANOUT1]			= &fanout1_clk.common.hw,
		[CLK_FANOUT0]			= &fanout0_clk.common.hw,
		[CLK_AHB_MONITOR_GATE]		= &ahb_montior_gate_clk.common.hw,
		[CLK_SD_MONITOR_GATE]		= &sd_montior_gate_clk.common.hw,
	},
	.num = CLK_NUMBER,
};
/* ccu_def_end */

static struct ccu_common *sun65iw1_ccu_clks[] = {
	&pll_peri0_parent_clk.common,
	&pll_peri0_2x_clk.common,
	&pll_peri0_800m_clk.common,
	&pll_peri0_480m_clk.common,
	&pll_peri1_parent_clk.common,
	&pll_peri1_2x_clk.common,
	&pll_peri1_800m_clk.common,
	&pll_peri1_480m_clk.common,
	&pll_gpu_vco_clk.common,
	&pll_gpu_clk.common,
	&pll_video0_parent_clk.common,
	&pll_video0_4x_clk.common,
	&pll_video0_3x_clk.common,
	&pll_video1_parent_clk.common,
	&pll_video1_4x_clk.common,
	&pll_video1_3x_clk.common,
	&pll_video2_parent_clk.common,
	&pll_video2_4x_clk.common,
	&pll_video2_3x_clk.common,
	&pll_ve_vco_clk.common,
	&pll_ve_clk.common,
	&pll_audio0_clk.common,
	&pll_audio1_parent_clk.common,
	&pll_audio1_2x_clk.common,
	&pll_audio1_5x_clk.common,
	&ahb_clk.common,
	&apb0_clk.common,
	&apb1_clk.common,
	&apb_uart_clk.common,
	&cpu_sys_dp_clk.common,
	&cpux_gic_clk.common,
	&nsi_clk.common,
	&nsi_cfg_bus_clk.common,
	&mbus_clk.common,
	&iommu_apb_bus_clk.common,
	&pll_stby_sys_peri0_bus_clk.common,
	&smhc2_ahb_sw_bus_clk.common,
	&smhc1_ahb_sw_bus_clk.common,
	&smhc0_ahb_sw_bus_clk.common,
	&hsi_ahb_sw_bus_clk.common,
	&secure_sys_ahb_bus_clk.common,
	&gpu_ahb_sw_bus_clk.common,
	&video_out0_ahb_sw_bus_clk.common,
	&video_in_ahb_sw_bus_clk.common,
	&ve0_ahb_sw_bus_clk.common,
	&dma0_mbus_clk.common,
	&ce_sys_axi_bus_clk.common,
	&gpu_axi_bus_clk.common,
	&de_sys_mbus_clk.common,
	&video_in_mbus_clk.common,
	&ve0_mbus_clk.common,
	&gmac0_axi_clk.common,
	&isp_mbus_clk.common,
	&csi_mbus_clk.common,
	&ce_sys_axi_clk.common,
	&ve0_mbus_gate_clk.common,
	&dma0_mbus_gate_clk.common,
	&dma0_ahb_bus_clk.common,
	&spinlock_ahb_bus_clk.common,
	&msgbox_cpux_ahb_bus_clk.common,
	&msgbox_cpus_ahb_bus_clk.common,
	&pwm0_apb_bus_clk.common,
	&dcu_bus_clk.common,
	&dap_ahb_bus_clk.common,
	&timer0_0_clk.common,
	&timer0_1_clk.common,
	&timer0_2_clk.common,
	&timer0_3_clk.common,
	&timer0_ahb_bus_clk.common,
	&de_clk.common,
	&de0_ahb_bus_clk.common,
	&g2d_clk.common,
	&g2d_ahb_bus_clk.common,
	&eink_panel_clk.common,
	&eink_ahb_bus_clk.common,
	&ve0_clk.common,
	&ve0_ahb_bus_clk.common,
	&ce_sys_clk.common,
	&ce_sys_ip_ahb_bus_clk.common,
	&gpu_clk.common,
	&gpu_ahb_bus_clk.common,
	&dramc_ahb_bus_clk.common,
	&smhc0_clk.common,
	&smhc0_ahb_bus_clk.common,
	&smhc1_clk.common,
	&smhc1_ahb_bus_clk.common,
	&smhc2_clk.common,
	&smhc2_ahb_bus_clk.common,
	&uart0_apb_bus_clk.common,
	&uart1_apb_bus_clk.common,
	&uart2_apb_bus_clk.common,
	&uart3_apb_bus_clk.common,
	&uart4_apb_bus_clk.common,
	&uart5_apb_bus_clk.common,
	&uart6_apb_bus_clk.common,
	&uart7_apb_bus_clk.common,
	&twi0_apb_bus_clk.common,
	&twi1_apb_bus_clk.common,
	&twi2_apb_bus_clk.common,
	&twi3_apb_bus_clk.common,
	&twi4_apb_bus_clk.common,
	&twi5_apb_bus_clk.common,
	&spi0_clk.common,
	&spi0_ahb_bus_clk.common,
	&spi1_clk.common,
	&spi1_ahb_bus_clk.common,
	&spi2_clk.common,
	&spi2_ahb_bus_clk.common,
	&gpadc0_clk.common,
	&gpadc0_apb_bus_clk.common,
	&tsensor_apb_bus_clk.common,
	&ir_rx0_clk.common,
	&ir_rx0_apb_bus_clk.common,
	&ir_tx_clk.common,
	&ir_tx_apb_bus_clk.common,
	&i2s0_clk.common,
	&i2s0_apb_bus_clk.common,
	&i2s1_clk.common,
	&i2s1_apb_bus_clk.common,
	&i2s2_clk.common,
	&i2s2_apb_bus_clk.common,
	&i2s3_clk.common,
	&i2s3_apb_bus_clk.common,
	&owa0_tx_clk.common,
	&owa0_rx_clk.common,
	&owa0_apb_bus_clk.common,
	&dmic_clk.common,
	&dmic_apb_bus_clk.common,
	&audiocodec0_dac_clk.common,
	&audiocodec0_adc_clk.common,
	&audiocodec0_apb_bus_clk.common,
	&audiocodec1_dac_clk.common,
	&audiocodec1_apb_bus_clk.common,
	&usb0_bus_clk.common,
	&usb0_dev_ahb_bus_clk.common,
	&usb0_ehci_ahb_bus_clk.common,
	&usb0_ohci_ahb_bus_clk.common,
	&usb1_bus_clk.common,
	&usb1_ehci_ahb_bus_clk.common,
	&usb1_ohci_ahb_bus_clk.common,
	&usb2p0_sys_phy_ref_bus_clk.common,
	&usb2p0_sys_ahb_bus_clk.common,
	&usb2_u2_phy_ref_bus_clk.common,
	&usb2_suspend_clk.common,
	&usb2_ref_clk.common,
	&usb2_u3_only_utmi_clk.common,
	&usb2_u2_only_pipe_clk.common,
	&pcie0_aux_clk.common,
	&pcie0_axi_s_clk.common,
	&hsi_comb0_phy_cfg_clk.common,
	&hsi_comb0_phy_ref_clk.common,
	&hsi_axi_bus_clk.common,
	&hsi_ahb_bus_clk.common,
	&hsi_axi_clk.common,
	&gmac0_phy_clk.common,
	&gmac0_ahb_bus_clk.common,
	&tcon_lcd0_clk.common,
	&tcon_lcd0_ahb_bus_clk.common,
	&mipi_dsi0_clk.common,
	&mipi_dsi0_ahb_bus_clk.common,
	&combophy0_clk.common,
	&tcon_tv0_edp_clk.common,
	&tcon_tv0_ahb_bus_clk.common,
	&edp_ahb_bus_clk.common,
	&vo0_ahb_bus_clk.common,
	&vo1_ahb_bus_clk.common,
	&ledc_clk.common,
	&ledc_apb_bus_clk.common,
	&csi_master0_clk.common,
	&csi_master1_clk.common,
	&csi_master2_clk.common,
	&csi_clk.common,
	&isp_clk.common,
	&video_in_ahb_bus_clk.common,
	&pll_output_bypass_bus_clk.common,
	&gmac_aximon_bus_clk.common,
	&hsi_aximon_bus_clk.common,
	&ce_sys_aximon_bus_clk.common,
	&gpu_aximon_bus_clk.common,
	&dcu_aximon_bus_clk.common,
	&cpu_sys_aximon_bus_clk.common,
	&clk50m_bus_clk.common,
	&clk25m_bus_clk.common,
	&clk16m_bus_clk.common,
	&clk12m_bus_clk.common,
	&clk24m_bus_clk.common,
	&clk27m_clk.common,
	&pclk_clk.common,
	&fanout_rc_16m_clk.common,
	&fanout2_clk.common,
	&fanout1_clk.common,
	&fanout0_clk.common,
	&ahb_montior_gate_clk.common,
	&sd_montior_gate_clk.common,
#ifdef CONFIG_PM_SLEEP
	&pll_audio0_sdm_pat0_clk.common,
	&pll_audio1_sdm_pat0_clk.common,
	&pll_peri1_sdm_pat0_clk.common,
#endif
};

static struct ccu_reg_dump sun65iw1_distbus_restore_clks[] = {
	{ 0x05C0, AHB_MASTER_KEY_VALUE },
	{ 0x05E0, MBUS_MASTER_KEY_VALUE },
	{ 0x05E4, MBUS_CLOCK_GATE_KEY_VALUE },
	{ 0x0580, UPD_KEY_VALUE},
	{ 0x0588, UPD_KEY_VALUE},
};

static const struct sunxi_ccu_desc sun65iw1_ccu_desc = {
	.ccu_clks	= sun65iw1_ccu_clks,
	.num_ccu_clks	= ARRAY_SIZE(sun65iw1_ccu_clks),

	.hw_clks	= &sun65iw1_hw_clks,

	.resets		= sun65iw1_ccu_resets,
	.num_resets	= ARRAY_SIZE(sun65iw1_ccu_resets),
};

static int sun65iw1_ccu_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct device *dev = &pdev->dev;
	void __iomem *reg;
	int ret;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(dev, "Fail to get IORESOURCE_MEM\n");
		return -EINVAL;
	}

	reg = devm_ioremap(dev, res->start, resource_size(res));
	if (IS_ERR(reg)) {
		dev_err(dev, "Fail to map IO resource\n");
		return PTR_ERR(reg);
	}

	/* Enable RES_DCAP_24M_GATE to calibrating the system clock frequency.*/
	set_reg(reg + RES_DCAP_24M_GATE, 0x1, 1, 0);

	/*
	 * In order to pass the EMI certification, the SDM function of
	 * the peripheral 1 bus is enabled, and the frequency is still
	 * calculated using the previous division factor.
	 */
	set_reg(reg + SUN65IW1_PLL_PERIPH1_PATTERN0_REG, 0xd1303333, 32, 0);
	set_reg(reg + SUN65IW1_PLL_PERI1_CTRL_REG, 1, 1, 24);

	/*
	 * 1. Enable pll_xxx clk gate
	 * 2. Set pll_xxx auto gate to auto
	 */
	set_reg(reg + SUN65IW1_PLL_PERI0_EN_REG, 0x8fff0000, 32, 0);
	set_reg(reg + SUN65IW1_PLL_PERI1_EN_REG, 0x9fff0000, 32, 0);
	set_reg(reg + SUN65IW1_PLL_VIDEO_EN_REG, 0x770000, 32, 0);
	set_reg(reg + SUN65IW1_PLL_GPU_EN_REG, 0x10000, 32, 0);
	set_reg(reg + SUN65IW1_PLL_VE_EN_REG, 0x10000, 32, 0);
	set_reg(reg + SUN65IW1_PLL_AUDIO_EN_REG, 0x70000, 32, 0);

	ret = sunxi_parse_sdm_info(pdev->dev.of_node);
	if (ret)
		sunxi_debug(NULL, "%s: sdm_info not enabled", __func__);

	ret = sunxi_ccu_probe(pdev->dev.of_node, reg, &sun65iw1_ccu_desc);
	if (ret)
		return ret;

	sunxi_ccu_sleep_init(reg, sun65iw1_ccu_clks,
			ARRAY_SIZE(sun65iw1_ccu_clks),
			sun65iw1_distbus_restore_clks,
			ARRAY_SIZE(sun65iw1_distbus_restore_clks));

	sunxi_info(NULL, "sunxi ccu driver version: %s\n", SUNXI_CCU_VERSION);
	return 0;
}

static const struct of_device_id sun65iw1_ccu_ids[] = {
	{ .compatible = "allwinner,sun65iw1-ccu" },
	{ }
};

static struct platform_driver sun65iw1_ccu_driver = {
	.probe	= sun65iw1_ccu_probe,
	.driver	= {
		.name	= "sun65iw1-ccu",
		.of_match_table	= sun65iw1_ccu_ids,
	},
};

static int __init sun65iw1_ccu_init(void)
{
	int err;

	err = platform_driver_register(&sun65iw1_ccu_driver);
	if (err)
		pr_err("register ccu sun65iw1 failed\n");

	return err;
}

core_initcall(sun65iw1_ccu_init);

static void __exit sun65iw1_ccu_exit(void)
{
	platform_driver_unregister(&sun65iw1_ccu_driver);
}
module_exit(sun65iw1_ccu_exit);

MODULE_DESCRIPTION("Allwinner sun65iw1 clk driver");
MODULE_AUTHOR("rengaomin<rengaomin@allwinnertech.com>");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(SUNXI_CCU_VERSION);
