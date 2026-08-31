// SPDX-License-Identifier: GPL-2.0-or-later

#include <linux/clk-provider.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <dt-bindings/clock/sun65iw1-ddrpll-ccu.h>

#include "ccu_common.h"
#include "ccu-sun65iw1.h"
#include "ccu_frac.h"
#include "ccu_sdm.h"
#include "ccu_mux.h"
#include "ccu_div.h"

#define SUNXI_CCU_VERSION		"0.0.5"

#define _SUNXI_CCU_MULT_PLL(_reg, _offset, _shift, _width, _max, _min) \
	{								\
		.reg    = _reg,						\
		.offset	= _offset,					\
		.shift	= _shift,						\
		.width	= _width,					\
		.max	= _max,					\
		.min	= _min,					\
	}

#define _SUNXI_CCU_MULT_PLL_NOT_OFFSET(_reg, _shift, _width, _max, _min)	\
			_SUNXI_CCU_MULT_PLL(_reg, 0, _shift, _width, _max, _min)

struct ccu_mult_pll {
	u32 		reg;
	u8			offset;
	u8			shift;
	u8			width;
	u16			max;
	u16			min;
};

struct ccu_pll {
	u32			output;
	u32			lock;
	u32			lock_enable;
	u32			ldo_en;
	u32			enable;
	struct ccu_mult_pll n;
	struct ccu_frac_internal frac;
	struct ccu_sdm_internal	 sdm;
	unsigned int		fixed_post_div;
	unsigned int		fixed_pre_div;
	unsigned int		min_rate;
	unsigned int		max_rate;

	struct ccu_common	common;
};

static inline struct ccu_pll *hw_to_ccu_pll(struct clk_hw *hw)
{
	struct ccu_pll *ret;

	struct ccu_common *common = hw_to_ccu_common(hw);

	ret = container_of(common, struct ccu_pll, common);

	return ret;
}

static inline struct ccu_div *hw_to_ccu_sun65iw1_dram(struct clk_hw *hw)
{
	struct ccu_div *ret;

	struct ccu_common *common = hw_to_ccu_common(hw);

	ret = container_of(common, struct ccu_div, common);

	return ret;
}

static unsigned long ccu_pll_recalc_rate(struct clk_hw *hw,
					unsigned long parent_rate)
{
	struct ccu_pll *pll = hw_to_ccu_pll(hw);
	struct ccu_common *common = &(pll->common);
	u64 rate;
	u32 n;
	u32 reg;

	reg = readl(common->base + pll->n.reg);
	n = reg >> pll->n.shift;
	n &= ((1 << pll->n.width) - 1);
	n += pll->n.offset;

	rate = parent_rate * n;

	return rate;
}

static u8 ccu_sun65iw1_dram_get_parent(struct clk_hw *hw)
{
	struct ccu_div *cd = hw_to_ccu_div(hw);

	return ccu_mux_helper_get_parent(&cd->common, &cd->mux);
}

static unsigned long ccu_sun65iw1_dram_recalc_rate(struct clk_hw *hw,
					unsigned long parent_rate)
{
	struct ccu_div *dram_clk = hw_to_ccu_sun65iw1_dram(hw);
	struct ccu_common *common = &(dram_clk->common);
	unsigned long rate = 0;
	u32 mux, m;
	u32 reg;
	struct clk_hw *parent_hw;

	reg = readl(common->base + common->reg);
	m = reg >> dram_clk->div.shift;
	m &= ((1 << dram_clk->div.width) - 1);

	mux = reg >> dram_clk->mux.shift;
	mux &= ((1 << dram_clk->mux.width) - 1);

	parent_hw = clk_hw_get_parent_by_index(hw, mux);

	parent_rate = clk_hw_get_rate(parent_hw);

	rate = parent_rate / (m + 1);

	return rate;
}

const struct clk_ops ccu_pll_ops = {
	.recalc_rate = ccu_pll_recalc_rate,
};

const struct clk_ops ccu_sun65iw1_dram_ops = {
	.recalc_rate = ccu_sun65iw1_dram_recalc_rate,
	.get_parent = ccu_sun65iw1_dram_get_parent,
};

static struct ccu_pll pll_ddr0 = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_PLL_NOT_OFFSET(0x4, 4, 8, 117, 150),
	.min_rate		= 2808000000,
	.max_rate		= 3600000000,
	.common			= {
		.reg		= 0x0000,
		.hw.init	= CLK_HW_INIT("pll-ddr0", "dcxo24M",
				&ccu_pll_ops,
				CLK_SET_RATE_UNGATE |
				CLK_IGNORE_UNUSED),
	},
};

static struct ccu_pll pll_ddr1 = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_PLL_NOT_OFFSET(0x204, 4, 8, 84, 117),
	.min_rate		= 2016000000,
	.max_rate		= 2808000000,
	.common			= {
		.reg		= 0x0200,
		.hw.init	= CLK_HW_INIT("pll-ddr1", "dcxo24M",
				&ccu_pll_ops,
				CLK_SET_RATE_UNGATE |
				CLK_IGNORE_UNUSED),
	},
};

static struct ccu_pll pll_ddr2 = {
	.output			= BIT(27),
	.lock			= BIT(28),
	.lock_enable		= BIT(29),
	.enable			= BIT(31),
	.n			= _SUNXI_CCU_MULT_PLL_NOT_OFFSET(0x404, 4, 8, 75, 100),
	.min_rate		= 1800000000,
	.max_rate		= 2400000000,
	.common			= {
		.reg		= 0x0400,
		.hw.init	= CLK_HW_INIT("pll-ddr2", "dcxo24M",
				&ccu_pll_ops,
				CLK_SET_RATE_UNGATE |
				CLK_IGNORE_UNUSED),
	},
};

static const char * const dram_mux_parent[] = { "pll-ddr0", "pll-ddr1", "pll-ddr2"};

struct ccu_div dram_mux_clk = {
	.enable = 0,
	.div	= _SUNXI_CCU_DIV(0, 3),
	.mux	= _SUNXI_CCU_MUX_TABLE(8, 2, NULL),
	.common = {
		.reg		= 0x001c,
		.hw.init	= CLK_HW_INIT_PARENTS("dram-mux-clk",
							  dram_mux_parent,
							  &ccu_sun65iw1_dram_ops,
							  CLK_GET_RATE_NOCACHE),
	},
};

static CLK_FIXED_FACTOR(dfi_clk, "dfi-clk",
			"dram-mux-clk", 4, 1, 0);

static struct clk_hw_onecell_data sunxi_pll_ddr_hw_clks = {
	.hws	= {
		[CLK_PLL_DDR0]		= &pll_ddr0.common.hw,
		[CLK_PLL_DDR1]		= &pll_ddr1.common.hw,
		[CLK_PLL_DDR2]		= &pll_ddr2.common.hw,
		[CLK_DRAM_MUX]		= &dram_mux_clk.common.hw,
		[CLK_DFI]			= &dfi_clk.hw,
	},
	.num = CLK_PLL_DRAM_MAX_NO,
};

static struct ccu_common *sunxi_pll_ddr_clks[] = {
	&pll_ddr0.common,
	&pll_ddr1.common,
	&pll_ddr2.common,
	&dram_mux_clk.common,
};

static const struct sunxi_ccu_desc pll_dram_desc = {
	.ccu_clks	= sunxi_pll_ddr_clks,
	.num_ccu_clks	= ARRAY_SIZE(sunxi_pll_ddr_clks),
	.hw_clks	= &sunxi_pll_ddr_hw_clks,
	.resets		= NULL,
	.num_resets	= 0,
};

static int sun65iw1_pll_ddr_probe(struct platform_device *pdev)
{
	int ret = 0;
	void __iomem *reg;

	reg = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(reg))
		return PTR_ERR(reg);

	ret = sunxi_ccu_probe(pdev->dev.of_node, reg, &pll_dram_desc);
	if (!ret) {
		dev_info(&pdev->dev, "sun65iw1 pll clk init OK\n");
	}

	return 0;
}

static const struct of_device_id sun65iw1_pll_ddr_ids[] = {
	{ .compatible = "allwinner,sun65iw1-pll-dram" },
	{ }
};

static struct platform_driver sun65iw1_pll_ddr_driver = {
	.probe	= sun65iw1_pll_ddr_probe,
	.driver	= {
		.name	= "sun65iw1-pll-dram",
		.of_match_table	= sun65iw1_pll_ddr_ids,
	},
};

static int __init sunxi_ccu_pll_ddr_init(void)
{
	int ret;

	ret = platform_driver_register(&sun65iw1_pll_ddr_driver);
	if (ret)
		pr_err("register ccu sun65iw1 pll dram failed\n");

	return ret;
}
core_initcall(sunxi_ccu_pll_ddr_init);

static void __exit sunxi_ccu_pll_ddr_exit(void)
{
	return platform_driver_unregister(&sun65iw1_pll_ddr_driver);
}
module_exit(sunxi_ccu_pll_ddr_exit);

MODULE_DESCRIPTION("Allwinner sun65iw1 pll ddr clk driver");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(SUNXI_CCU_VERSION);
MODULE_AUTHOR("weizhouxiang<weizhouxiang@allwinnertech.com>");
