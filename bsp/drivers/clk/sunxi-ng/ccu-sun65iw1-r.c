// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Copyright (c) 2023 rengaomin@allwinnertech.com
 */

#include <linux/clk-provider.h>
#include <linux/module.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>

#include "ccu_common.h"
#include "ccu_reset.h"

#include "ccu_div.h"
#include "ccu_gate.h"
#include "ccu_mp.h"
#include "ccu_nm.h"

#include "ccu-sun65iw1-r.h"

#define SUNXI_R_CCU_VERSION	"0.0.2"

/* ccu_des_start */

static const char * const ahb_parents[] = { "dcxo24M", "rtc32k", "rc-16m", "pll-peri0-200m", "pll-peri0-300m" };

static SUNXI_CCU_M_WITH_MUX(r_ahb_clk, "r-ahb",
			ahb_parents, 0x0000,
			0, 5,
			24, 3,
			CLK_IGNORE_UNUSED);

static const char * const apbs_parents[] = { "dcxo24M", "rtc32k", "rc-16m", "pll-peri0-200m" };

static SUNXI_CCU_M_WITH_MUX(r_apb0_clk, "r-apb0",
			apbs_parents, 0x000C,
			0, 5,
			24, 3,
			CLK_IGNORE_UNUSED);

static SUNXI_CCU_M_WITH_MUX(r_apb1_clk, "r-apb1",
			apbs_parents, 0x0010,
			0, 5,
			24, 3,
			CLK_IGNORE_UNUSED);

static const char * const timer_parents[] = { "dcxo24M", "rtc32k", "rc-16m", "pll-peri0-200m" };

static SUNXI_CCU_M_WITH_MUX_GATE(r_timer0_clk, "r-timer0",
			timer_parents, 0x0100,
			1, 3,	/* M */
			4, 3,	/* mux */
			BIT(0),	/* gate */
			CLK_IGNORE_UNUSED);

static SUNXI_CCU_M_WITH_MUX_GATE(r_timer1_clk, "r-timer1",
			timer_parents, 0x0104,
			1, 3,	/* M */
			4, 3,	/* mux */
			BIT(0),	/* gate */
			CLK_IGNORE_UNUSED);

static SUNXI_CCU_M_WITH_MUX_GATE(r_timer2_clk, "r-timer2",
			timer_parents, 0x0108,
			1, 3,	/* M */
			4, 3,	/* mux */
			BIT(0),	/* gate */
			CLK_IGNORE_UNUSED);

static SUNXI_CCU_M_WITH_MUX_GATE(r_timer3_clk, "r-timer3",
			timer_parents, 0x010c,
			1, 3,	/* M */
			4, 3,	/* mux */
			BIT(0),	/* gate */
			CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_timer_bus_clk, "r-timer-bus",
			"r-ahb",
			0x011C, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_twd_bus_clk, "r-twd-bus",
			"r-apb0",
			0x012C, BIT(0), CLK_IGNORE_UNUSED);

static const char * const pwm_parents[] = { "dcxo24M", "rtc32k", "rc-16m" };

static SUNXI_CCU_MUX_WITH_GATE(r_pwm_clk, "r-pwm",
			pwm_parents, 0x0130,
			24, 2,	/* mux */
			BIT(31), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_pwm_bus_clk, "r-pwm-bus",
			"r-apb0",
			0x013C, BIT(0), CLK_IGNORE_UNUSED);

static const char * const spi_parents[] = { "dcxo24M", "pll-peri0-200m", "pll-peri0-300m", "pll-peri1-300m" };

static SUNXI_CCU_MP_WITH_MUX_GATE_NO_INDEX(r_spi_clk, "r-spi",
			spi_parents, 0x0150,
			0, 5,	/* M */
			8, 5,	/* N */
			24, 3,	/* mux */
			BIT(31), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_spi_bus_clk, "r-spi-bus",
			"r-ahb",
			0x015C, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_uart1_bus_clk, "r-uart1-bus",
			"r-apb1",
			0x018C, BIT(1), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_uart0_bus_clk, "r-uart0-bus",
			"r-apb1",
			0x018C, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_twi2_bus_clk, "r-twi2-bus",
			"r-apb1",
			0x019C, BIT(2), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_twi1_bus_clk, "r-twi1-bus",
			"r-apb1",
			0x019C, BIT(1), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_twi0_bus_clk, "r-twi0-bus",
			"r-apb1",
			0x019C, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_ppu_vo0_bus_clk, "r-ppu-vo0-bus",
			"r-apb0",
			0x01AC, BIT(3), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_ppu_hsi_bus_clk, "r-ppu-hsi-bus",
			"r-apb0",
			0x01AC, BIT(2), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_ppu_ve_bus_clk, "r-ppu-ve-bus",
			"r-apb0",
			0x01AC, BIT(1), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_ppu_vi_bus_clk, "r-ppu-vi-bus",
			"r-apb0",
			0x01AC, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_tzma_bus_clk, "r-tzma-bus",
			"r-apb0",
			0x01B0, BIT(0), CLK_IGNORE_UNUSED);

static const char * const ir_rx_parents[] = { "rtc32k", "dcxo24M" };

static SUNXI_CCU_M_WITH_MUX_GATE(r_ir_rx_clk, "r-ir-rx",
			ir_rx_parents, 0x01C0,
			0, 5,	/* M */
			24, 2,	/* mux */
			BIT(31),	/* gate */
			CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_ir_rx_bus_clk, "r-ir-rx-bus",
			"r-apb0",
			0x01CC, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_rtc_bus_clk, "r-rtc-bus",
			"r-ahb",
			0x020C, BIT(0), CLK_IGNORE_UNUSED);

static const char * const cpus_24m_parents[] = { "dcxo24M", "rtc32k", "16m-rc" };

static SUNXI_CCU_MUX_WITH_GATE(r_cpus_24m_clk, "r-cpus-24m",
			cpus_24m_parents, 0x0210,
			24, 2,	/* mux */
			BIT(31), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_cpus_cfg_bus_clk, "r-cpus-cfg-bus",
			"r-apb0",
			0x021C, BIT(1), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_cpus_core_bus_clk, "r-cpus-core-bus",
			"dcxo24M",
			0x021C, BIT(0), CLK_IGNORE_UNUSED);

static SUNXI_CCU_GATE(r_cpuidle_bus_clk, "r-cpuidle-bus",
			"apb0",
			0x022C, BIT(0), CLK_IGNORE_UNUSED);
/* ccu_des_end */

/* rst_def_start */
static struct ccu_reset_map sun65iw1_r_ccu_resets[] = {
	[RST_BUS_R_TIMER0]		= { 0x011c, BIT(16) },
	[RST_BUS_R_PWM]			= { 0x013c, BIT(16) },
	[RST_BUS_R_SPI]			= { 0x015c, BIT(16) },
	[RST_BUS_R_UART1]		= { 0x018c, BIT(17) },
	[RST_BUS_R_UART0]		= { 0x018c, BIT(16) },
	[RST_BUS_R_TWI2]		= { 0x019c, BIT(18) },
	[RST_BUS_R_TWI1]		= { 0x019c, BIT(17) },
	[RST_BUS_R_TWI0]		= { 0x019c, BIT(16) },
	[RST_BUS_R_IR_RX]		= { 0x01cc, BIT(16) },
	[RST_BUS_RTC]			= { 0x020c, BIT(16) },
	[RST_BUS_CPUS_CFG]		= { 0x021c, BIT(16) },
	[RST_BUS_CPUIDLE]		= { 0x022c, BIT(16) },
};
/* rst_def_end */

static struct clk_hw_onecell_data sun65iw1_r_hw_clks = {
	.hws    = {
		[CLK_R_AHB]			= &r_ahb_clk.common.hw,
		[CLK_R_APB0]			= &r_apb0_clk.common.hw,
		[CLK_R_APB1]			= &r_apb1_clk.common.hw,
		[CLK_R_TIMER0]			= &r_timer0_clk.common.hw,
		[CLK_R_TIMER1]			= &r_timer1_clk.common.hw,
		[CLK_R_TIMER2]			= &r_timer2_clk.common.hw,
		[CLK_R_TIMER3]			= &r_timer3_clk.common.hw,
		[CLK_BUS_R_TIMER]		= &r_timer_bus_clk.common.hw,
		[CLK_BUS_R_TWD]			= &r_twd_bus_clk.common.hw,
		[CLK_R_PWM]			= &r_pwm_clk.common.hw,
		[CLK_BUS_R_PWM]			= &r_pwm_bus_clk.common.hw,
		[CLK_R_SPI]			= &r_spi_clk.common.hw,
		[CLK_BUS_R_SPI]			= &r_spi_bus_clk.common.hw,
		[CLK_BUS_R_UART1]		= &r_uart1_bus_clk.common.hw,
		[CLK_BUS_R_UART0]		= &r_uart0_bus_clk.common.hw,
		[CLK_BUS_R_TWI2]		= &r_twi2_bus_clk.common.hw,
		[CLK_BUS_R_TWI1]		= &r_twi1_bus_clk.common.hw,
		[CLK_BUS_R_TWI0]		= &r_twi0_bus_clk.common.hw,
		[CLK_BUS_R_PPU_VO0]		= &r_ppu_vo0_bus_clk.common.hw,
		[CLK_BUS_R_PPU_HSI]		= &r_ppu_hsi_bus_clk.common.hw,
		[CLK_BUS_R_PPU_VE]		= &r_ppu_ve_bus_clk.common.hw,
		[CLK_BUS_R_PPU_VI]		= &r_ppu_vi_bus_clk.common.hw,
		[CLK_BUS_R_TZMA]		= &r_tzma_bus_clk.common.hw,
		[CLK_R_IR_RX]			= &r_ir_rx_clk.common.hw,
		[CLK_BUS_R_IR_RX]		= &r_ir_rx_bus_clk.common.hw,
		[CLK_BUS_R_RTC]			= &r_rtc_bus_clk.common.hw,
		[CLK_R_CPUS_24M]		= &r_cpus_24m_clk.common.hw,
		[CLK_BUS_R_CPUS_CFG]		= &r_cpus_cfg_bus_clk.common.hw,
		[CLK_BUS_R_CPUS_CORE]		= &r_cpus_core_bus_clk.common.hw,
		[CLK_BUS_R_CPUIDLE]		= &r_cpuidle_bus_clk.common.hw,
	},
	.num = CLK_R_NUMBER,
};
/* ccu_def_end */

static struct ccu_common *sun65iw1_r_ccu_clks[] = {
	&r_ahb_clk.common,
	&r_apb0_clk.common,
	&r_apb1_clk.common,
	&r_timer0_clk.common,
	&r_timer1_clk.common,
	&r_timer2_clk.common,
	&r_timer3_clk.common,
	&r_timer_bus_clk.common,
	&r_twd_bus_clk.common,
	&r_pwm_clk.common,
	&r_pwm_bus_clk.common,
	&r_spi_clk.common,
	&r_spi_bus_clk.common,
	&r_uart1_bus_clk.common,
	&r_uart0_bus_clk.common,
	&r_twi2_bus_clk.common,
	&r_twi1_bus_clk.common,
	&r_twi0_bus_clk.common,
	&r_ppu_vo0_bus_clk.common,
	&r_ppu_hsi_bus_clk.common,
	&r_ppu_ve_bus_clk.common,
	&r_ppu_vi_bus_clk.common,
	&r_tzma_bus_clk.common,
	&r_ir_rx_clk.common,
	&r_ir_rx_bus_clk.common,
	&r_rtc_bus_clk.common,
	&r_cpus_24m_clk.common,
	&r_cpus_cfg_bus_clk.common,
	&r_cpus_core_bus_clk.common,
	&r_cpuidle_bus_clk.common,
};

static const struct sunxi_ccu_desc sun65iw1_r_ccu_desc = {
	.ccu_clks	= sun65iw1_r_ccu_clks,
	.num_ccu_clks	= ARRAY_SIZE(sun65iw1_r_ccu_clks),

	.hw_clks	= &sun65iw1_r_hw_clks,

	.resets		= sun65iw1_r_ccu_resets,
	.num_resets	= ARRAY_SIZE(sun65iw1_r_ccu_resets),
};

static int sun65iw1_r_ccu_probe(struct platform_device *pdev)
{
	void __iomem *reg;
	int ret;

	reg = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(reg))
		return PTR_ERR(reg);

	ret = sunxi_ccu_probe(pdev->dev.of_node, reg, &sun65iw1_r_ccu_desc);
	if (ret)
		return ret;

	sunxi_ccu_sleep_init(reg, sun65iw1_r_ccu_clks,
			ARRAY_SIZE(sun65iw1_r_ccu_clks),
			NULL, 0);

	sunxi_info(NULL, "sunxi r_ccu driver version: %s\n", SUNXI_R_CCU_VERSION);
	return 0;
}

static const struct of_device_id sun65iw1_r_ccu_ids[] = {
	{ .compatible = "allwinner,sun65iw1-r-ccu" },
	{ }
};

static struct platform_driver sun65iw1_r_ccu_driver = {
	.probe	= sun65iw1_r_ccu_probe,
	.driver	= {
		.name	= "sun65iw1-r-ccu",
		.of_match_table	= sun65iw1_r_ccu_ids,
	},
};

static int __init sunxi_ccu_sun65iw1_r_init(void)
{
	int ret;

	ret = platform_driver_register(&sun65iw1_r_ccu_driver);
	if (ret)
		pr_err("register ccu sun65iw1-r failed\n");

	return ret;
}
core_initcall(sunxi_ccu_sun65iw1_r_init);

static void __exit sunxi_ccu_sun65iw1_r_exit(void)
{
	return platform_driver_unregister(&sun65iw1_r_ccu_driver);
}
module_exit(sunxi_ccu_sun65iw1_r_exit);

MODULE_DESCRIPTION("Allwinner sun65iw1-r clk driver");
MODULE_AUTHOR("rengaomin<rengaomin@allwinnertech.com>");
MODULE_LICENSE("GPL v2");
MODULE_VERSION(SUNXI_R_CCU_VERSION);
