// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Copyright (c) 2022 liujuan1@allwinnertech.com
 */

#include <linux/clk-provider.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/iopoll.h>
#include <linux/syscore_ops.h>
#include <linux/slab.h>
#include <dt-bindings/clock/sun65iw1-cpupll-ccu.h>

#include "ccu_common.h"
#include "ccu_reset.h"
#include "ccu_nm.h"
#include "ccu_nkmp.h"

#define SUNXI_CPUPLL_CCU_VERSION		"0.0.1"

#define SUN65IW1_PLL_CPU0_REG			(0x0000)
#define SUN65IW1_PLL_CPU1_REG			(0x0100)
#define SUN65IW1_PLL_CPU2_REG			(0x0200)

#define SUN65IW1_PLL_CPU0_PAT0_REG		(0x0004)
#define SUN65IW1_PLL_CPU1_PAT0_REG		(0x0104)
#define SUN65IW1_PLL_CPU2_PAT0_REG		(0x0204)

#define SUN65IW1_PLL_CPU0_SSC_REG		(0x0014)
#define SUN65IW1_PLL_CPU1_SSC_REG		(0x0114)
#define SUN65IW1_PLL_CPU2_SSC_REG		(0x0214)

#define SUN65IW1_PLL_CPU0_CLK_REG		(0x0018)
#define SUN65IW1_PLL_CPU1_CLK_REG		(0x0118)
#define SUN65IW1_PLL_CPU2_CLK_REG		(0x0218)

#define SUN65IW1_PLL_CPU0_CLK_DIV_REG		(0x0020)
#define SUN65IW1_PLL_CPU1_CLK_DIV_REG		(0x0120)
#define SUN65IW1_PLL_CPU2_CLK_DIV_REG		(0x0220)

#define SUN65IW1_CPU_PLL_MAX_RATE		(2016000000)

static struct ccu_nkmp pll_cpu0_clk = {
	.output		= BIT(27),
	.lock		= BIT(28),
	.lock_enable	= BIT(29),
	.ldo_en		= BIT(30),
	.enable		= BIT(31),
	.n		= _SUNXI_CCU_MULT_OFFSET_MIN_MAX(8, 8, 0, 20, 84),
	.m		= _SUNXI_CCU_DIV(0, 4),
	.p		= _SUNXI_CCU_DIV(16, 2), /* P in cpu_clk reg */
	.p_reg		= SUN65IW1_PLL_CPU0_CLK_DIV_REG,
	.max_rate	= SUN65IW1_CPU_PLL_MAX_RATE,
	.common		= {
		.reg		= SUN65IW1_PLL_CPU0_REG,
		.ssc_reg	= SUN65IW1_PLL_CPU0_SSC_REG,
		.clear		= BIT(26),
		.features	= CCU_FEATURE_CLEAR_MOD | CCU_FEATURE_CLAC_CACHED | CCU_FEATURE_TYPE_NKMP,
		.hw.init	= CLK_HW_INIT("pll-cpu0", "dcxo24M",
				&ccu_nkmp_ops,
				CLK_GET_RATE_NOCACHE | CLK_IS_CRITICAL \
				| CLK_SET_RATE_UNGATE),
	},
};

static const char * const cpu0_parents[] = { "dcxo24M", "osc32k", "iosc", "pll-cpu0", "pll-peri0-2x", "pll-peri0-600m", "pll-peri1-2x" };
static SUNXI_CCU_MUX(cpu0_clk, "cpu0", cpu0_parents,
		SUN65IW1_PLL_CPU0_CLK_REG, 24, 3, CLK_SET_RATE_PARENT | CLK_IS_CRITICAL);

/* M or N only support 1 or 3 */
static SUNXI_CCU_M(pll_cpu0_apb_div_clk, "pll-cpu0-apb-div",
		"cpu0", SUN65IW1_PLL_CPU0_CLK_DIV_REG, 2, 2, 0);
static SUNXI_CCU_M(pll_cpu0_axi_div_clk, "pll-cpu0-axi-div",
		"cpu0", SUN65IW1_PLL_CPU0_CLK_DIV_REG, 0, 2, 0);

static struct ccu_nkmp pll_cpu1_clk = {
	.output		= BIT(27),
	.lock		= BIT(28),
	.lock_enable	= BIT(29),
	.ldo_en		= BIT(30),
	.enable		= BIT(31),
	.n		= _SUNXI_CCU_MULT_OFFSET_MIN_MAX(8, 8, 0, 20, 84),
	.m		= _SUNXI_CCU_DIV(0, 4),
	.p		= _SUNXI_CCU_DIV(16, 2), /* P in cpu_clk reg */
	.p_reg		= SUN65IW1_PLL_CPU1_CLK_DIV_REG,
	.max_rate	= SUN65IW1_CPU_PLL_MAX_RATE,
	.common		= {
		.reg		= SUN65IW1_PLL_CPU1_REG,
		.ssc_reg	= SUN65IW1_PLL_CPU1_SSC_REG,
		.clear		= BIT(26),
		.features	= CCU_FEATURE_CLEAR_MOD | CCU_FEATURE_CLAC_CACHED | CCU_FEATURE_TYPE_NKMP,
		.hw.init	= CLK_HW_INIT("pll-cpu1", "dcxo24M",
				&ccu_nkmp_ops,
				CLK_GET_RATE_NOCACHE | CLK_IS_CRITICAL \
				| CLK_SET_RATE_UNGATE),
	},
};

static const char * const cpu1_parents[] = { "dcxo24M", "osc32k", "iosc", "pll-cpu1", "pll-peri0-2x", "pll-peri0-600m", "pll-peri1-2x" };
static SUNXI_CCU_MUX(cpu1_clk, "cpu1", cpu1_parents,
		SUN65IW1_PLL_CPU1_CLK_REG, 24, 3, CLK_SET_RATE_PARENT | CLK_IS_CRITICAL);

/* M or N only support 1 or 3 */
static SUNXI_CCU_M(pll_cpu1_apb_div_clk, "pll-cpu1-apb-div",
		"cpu1", SUN65IW1_PLL_CPU1_CLK_DIV_REG, 2, 2, 0);
static SUNXI_CCU_M(pll_cpu1_axi_div_clk, "pll-cpu1-axi-div",
		"cpu1", SUN65IW1_PLL_CPU1_CLK_DIV_REG, 0, 2, 0);

static struct ccu_nkmp pll_cpu2_clk = {
	.output		= BIT(27),
	.lock		= BIT(28),
	.lock_enable	= BIT(29),
	.ldo_en		= BIT(30),
	.enable		= BIT(31),
	.n		= _SUNXI_CCU_MULT_OFFSET_MIN_MAX(8, 8, 0, 20, 84),
	.m		= _SUNXI_CCU_DIV(0, 4),
	.p		= _SUNXI_CCU_DIV(16, 2), /* P in cpu_clk reg */
	.p_reg		= SUN65IW1_PLL_CPU2_CLK_DIV_REG,
	.max_rate	= SUN65IW1_CPU_PLL_MAX_RATE,
	.common		= {
		.reg		= SUN65IW1_PLL_CPU2_REG,
		.ssc_reg	= SUN65IW1_PLL_CPU2_SSC_REG,
		.clear		= BIT(26),
		.features	= CCU_FEATURE_CLEAR_MOD | CCU_FEATURE_CLAC_CACHED | CCU_FEATURE_TYPE_NKMP,
		.hw.init	= CLK_HW_INIT("pll-cpu2", "dcxo24M",
				&ccu_nkmp_ops,
				CLK_GET_RATE_NOCACHE | CLK_IS_CRITICAL \
				| CLK_SET_RATE_UNGATE),
	},
};

static const char * const cpu2_parents[] = { "dcxo24M", "osc32k", "iosc", "pll-cpu2", "pll-peri0-2x", "pll-peri0-600m", "pll-peri1-2x" };
static SUNXI_CCU_MUX(cpu2_clk, "cpu2", cpu2_parents,
		SUN65IW1_PLL_CPU2_CLK_REG, 24, 3, CLK_SET_RATE_PARENT | CLK_IS_CRITICAL);

/* M or N only support 1 or 3 */
static SUNXI_CCU_M(pll_cpu2_apb_div_clk, "pll-cpu2-apb-div",
		"cpu2", SUN65IW1_PLL_CPU2_CLK_DIV_REG, 2, 2, 0);
static SUNXI_CCU_M(pll_cpu2_axi_div_clk, "pll-cpu2-axi-div",
		"cpu2", SUN65IW1_PLL_CPU2_CLK_DIV_REG, 0, 2, 0);

static struct ccu_common *sunxi_pll_cpu_clks[] = {
	&pll_cpu0_clk.common,
	&cpu0_clk.common,
	&pll_cpu0_apb_div_clk.common,
	&pll_cpu0_axi_div_clk.common,
	&pll_cpu1_clk.common,
	&cpu1_clk.common,
	&pll_cpu1_apb_div_clk.common,
	&pll_cpu1_axi_div_clk.common,
	&pll_cpu2_clk.common,
	&cpu2_clk.common,
	&pll_cpu2_apb_div_clk.common,
	&pll_cpu2_axi_div_clk.common,

};

static struct clk_hw_onecell_data sunxi_cpupll_hw_clks = {
	.hws	= {
		[CLK_PLL_CPU0]		= &pll_cpu0_clk.common.hw,
		[CLK_CPU0]		= &cpu0_clk.common.hw,
		[CLK_CPU0_APB_DIV]	= &pll_cpu0_apb_div_clk.common.hw,
		[CLK_CPU0_AXI_DIV]	= &pll_cpu0_axi_div_clk.common.hw,
		[CLK_PLL_CPU1]		= &pll_cpu1_clk.common.hw,
		[CLK_CPU1]		= &cpu1_clk.common.hw,
		[CLK_CPU1_APB_DIV]	= &pll_cpu1_apb_div_clk.common.hw,
		[CLK_CPU1_AXI_DIV]	= &pll_cpu1_axi_div_clk.common.hw,
		[CLK_PLL_CPU2]		= &pll_cpu2_clk.common.hw,
		[CLK_CPU2]		= &cpu2_clk.common.hw,
		[CLK_CPU2_APB_DIV]	= &pll_cpu2_apb_div_clk.common.hw,
		[CLK_CPU2_AXI_DIV]	= &pll_cpu2_axi_div_clk.common.hw,
	},
	.num = CLK_CPUPLL_MAX_NO,
};

static const struct sunxi_ccu_desc cpupll_desc = {
	.ccu_clks	= sunxi_pll_cpu_clks,
	.num_ccu_clks	= ARRAY_SIZE(sunxi_pll_cpu_clks),
	.hw_clks	= &sunxi_cpupll_hw_clks,
	.resets		= NULL,
	.num_resets	= 0,
};

static const u32 sun65iw1_pll_cpu_regs[] = {
	SUN65IW1_PLL_CPU0_REG,
	SUN65IW1_PLL_CPU1_REG,
	SUN65IW1_PLL_CPU2_REG,
};

static const u32 sun65iw1_pll_cpu_pat0_regs[] = {
	SUN65IW1_PLL_CPU0_PAT0_REG,
	SUN65IW1_PLL_CPU1_PAT0_REG,
	SUN65IW1_PLL_CPU2_PAT0_REG,
};

static const u32 sun65iw1_pll_cpu_ssc_regs[] = {
	SUN65IW1_PLL_CPU0_SSC_REG,
	SUN65IW1_PLL_CPU1_SSC_REG,
	SUN65IW1_PLL_CPU2_SSC_REG,
};

#if (defined CONFIG_AW_FPGA_S4) || (defined CONFIG_AW_FPGA_V7)
static void ccupll_helper_wait_for_lock(void __iomem *addr, u32 lock)
{
}

static void cpupll_helper_wait_for_clear(void __iomem *addr, u32 clear)
{
}
#else
static void ccupll_helper_wait_for_lock(void __iomem *addr, u32 lock)
{
	u32 reg;

	WARN_ON(readl_relaxed_poll_timeout(addr, reg, reg & lock, 100, 70000));
}

static void cpupll_helper_wait_for_clear(void __iomem *addr, u32 clear)
{
	u32 reg;

	reg = readl(addr);
	writel(reg | clear, addr);

	WARN_ON(readl_relaxed_poll_timeout_atomic(addr, reg, !(reg & clear), 100, 10000));
}
#endif

static int cpupll_notifier_cb(struct notifier_block *nb,
				unsigned long event, void *data)
{
	struct ccu_pll_nb *pll = to_ccu_pll_nb(nb);
	int ret = 0;

	if (event == PRE_RATE_CHANGE) {
		/* Enable ssc function */
		set_reg(pll->common->base + pll->common->ssc_reg, 1, 1, pll->enable);
	} else if (event == POST_RATE_CHANGE) {
		/* Disable ssc function */
		set_reg(pll->common->base + pll->common->ssc_reg, 0, 1, pll->enable);
	}

	ccu_helper_wait_for_clear(pll->common, pll->common->clear);

	return notifier_from_errno(ret);
}

static struct ccu_pll_nb cpupll0_nb = {
	.common = &pll_cpu0_clk.common,
	.enable = 31, /* switch ssc mode */
	.clk_nb = {
		.notifier_call = cpupll_notifier_cb,
	},
};

static struct ccu_pll_nb cpupll1_nb = {
	.common = &pll_cpu1_clk.common,
	.enable = 31, /* switch ssc mode */
	.clk_nb = {
		.notifier_call = cpupll_notifier_cb,
	},
};

static struct ccu_pll_nb cpupll2_nb = {
	.common = &pll_cpu2_clk.common,
	.enable = 31,
	.clk_nb = {
		.notifier_call = cpupll_notifier_cb,
	},
};

static int sun65iw1_cpupll_probe(struct platform_device *pdev)
{
	void __iomem *reg;
	u32 val;
	int i;
	unsigned int step = 0, ssc = 0;
	struct device_node *np = pdev->dev.of_node;

	reg = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(reg))
		return PTR_ERR(reg);

	if (of_property_read_u32(np, "pll_step", &step))
		step = 0x9;

	if (of_property_read_u32(np, "pll_ssc", &ssc))
		ssc = 0x3300;

	/* TODO: assume boot use the cpupll */
	for (i = 0; i < ARRAY_SIZE(sun65iw1_pll_cpu_ssc_regs); i++) {
		/*
		 * 1. Config n,m1,m0,p: default:480M
		 * 2. Enable pll_en pll_ldo_en lock_en pll_output
		 * 3. wait for update and lock
		 */
		val = readl(reg + sun65iw1_pll_cpu_regs[i]);
		val |= BIT(27) | BIT(29) | BIT(30) | BIT(31);
		writel(val, reg + sun65iw1_pll_cpu_regs[i]);

		cpupll_helper_wait_for_clear(reg + sun65iw1_pll_cpu_regs[i], BIT(26));
		ccupll_helper_wait_for_lock(reg + sun65iw1_pll_cpu_regs[i], BIT(28));

		/*
		 * set pat0_ctrl_reg
		 */
		val = readl(reg + sun65iw1_pll_cpu_pat0_regs[i]);
		val |= GENMASK(30, 29);  /* ues Triangular(3bit) */
		writel(val, reg + sun65iw1_pll_cpu_pat0_regs[i]);

		/*
		 * set ssc/step in ssc reg
		 */
		val = readl(reg + sun65iw1_pll_cpu_ssc_regs[i]);
		val &= ~GENMASK(28, 12);
		val &= ~GENMASK(3, 0);
		val |= (ssc << 12 | step << 0);
		writel(val, reg + sun65iw1_pll_cpu_ssc_regs[i]);

		/*
		 * enable ssc mode
		 */
		val = readl(reg + sun65iw1_pll_cpu_ssc_regs[i]);
		val |= BIT(31);
		writel(val, reg + sun65iw1_pll_cpu_ssc_regs[i]);

		cpupll_helper_wait_for_clear(reg + sun65iw1_pll_cpu_regs[i], BIT(26));

		/*
		 * disable ssc mode
		 */
		val = readl(reg + sun65iw1_pll_cpu_ssc_regs[i]);
		val &= ~BIT(31);
		writel(val, reg + sun65iw1_pll_cpu_ssc_regs[i]);

		cpupll_helper_wait_for_clear(reg + sun65iw1_pll_cpu_regs[i], BIT(26));
	}

	sunxi_ccu_probe(pdev->dev.of_node, reg, &cpupll_desc);

	ccu_pll_notifier_register(&cpupll0_nb);
	ccu_pll_notifier_register(&cpupll1_nb);
	ccu_pll_notifier_register(&cpupll2_nb);

	sunxi_info(NULL, "sunxi pll_cpu driver version: %s\n", SUNXI_CPUPLL_CCU_VERSION);

	return 0;
}

static const struct of_device_id sun65iw1_cpupll_ids[] = {
	{ .compatible = "allwinner,sun65iw1-cpupll" },
	{ }
};

static struct platform_driver sun65iw1_cpupll_driver = {
	.probe	= sun65iw1_cpupll_probe,
	.driver	= {
		.name	= "sun65iw1-cpupll",
		.of_match_table	= sun65iw1_cpupll_ids,
	},
};

static int __init sunxi_ccu_cpupll_init(void)
{
	int ret;

	ret = platform_driver_register(&sun65iw1_cpupll_driver);
	if (ret)
		pr_err("register ccu sun65iw1 cpupll failed\n");

	return ret;
}
core_initcall(sunxi_ccu_cpupll_init);

static void __exit sunxi_ccu_cpupll_exit(void)
{
	return platform_driver_unregister(&sun65iw1_cpupll_driver);
}
module_exit(sunxi_ccu_cpupll_exit);

MODULE_DESCRIPTION("Allwinner sun65iw1 cpupll clk driver");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("0.0.5");
MODULE_AUTHOR("rengaomin<rengaomin@allwinnertech.com>");
