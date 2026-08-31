// SPDX-License-Identifier: GPL-2.0
/*
 * PCIe host controller driver for Allwinner SoCs.
 *
 * Copyright (C) 2026 Allwinner Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/of_device.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/resource.h>
#include <linux/types.h>
#include <linux/reset.h>
#include <linux/phy/phy.h>
#include <linux/pm_runtime.h>

#include <sunxi-nsi.h>

#include "../../pci.h"
#include "pcie-designware.h"
#include "pcie-dw-sunxi.h"

#define SUNXI_PCIE_MODULE_VERSION	"0.1.1"

struct sunxi_pcie_of_data {
	u64 cpu_addr_fix;
	bool bandwith_limited;
};

struct sunxi_pcie {
	struct dw_pcie			*pci;
	void __iomem			*app_base;
	const struct sunxi_pcie_of_data	*hw_data;
	struct phy			*phy;
	struct regulator_bulk_data	*regulators;
	unsigned int			regulator_cnt;
	struct clk_bulk_data		*clks;
	unsigned int			clk_cnt;
	struct reset_control_bulk_data	*rsts;
	unsigned int			rst_cnt;
	struct gpio_desc		*rst_gpio;
	int				sii_irq;
	struct irq_domain		*intx_domain;
};

static const char *sunxi_pcie_clks[] = {
	"aux",
	"axi_slv",
	"its",
};

static const char *sunxi_pcie_rsts[] = {
	"rst",
	"pwrup_rst",
	"its",
};

static const char *sunxi_pcie_regulators[] = {
	"pcie0v9",
	"pcie1v8",
	"pcie3v3",
	"pcie12v",
};

static inline u32 sunxi_pcie_app_readl(struct sunxi_pcie *sunxi_pcie, u32 reg)
{
	return readl(sunxi_pcie->app_base + reg);
}

static inline void sunxi_pcie_app_writel(u32 val, struct sunxi_pcie *sunxi_pcie, u32 reg)
{
	writel(val, sunxi_pcie->app_base + reg);
}

static void sunxi_pcie_intx_irq_mask(struct irq_data *data)
{
	struct sunxi_pcie *sunxi_pcie = irq_data_get_irq_chip_data(data);
	struct dw_pcie *pci = sunxi_pcie->pci;
	struct dw_pcie_rp *pp = &pci->pp;
	irq_hw_number_t hwirq = irqd_to_hwirq(data);
	unsigned long flags;
	u32 mask, stas;

	raw_spin_lock_irqsave(&pp->lock, flags);
	mask = sunxi_pcie_app_readl(sunxi_pcie, SII_INT_MASK0);
	mask &= ~INTX_RX_ASSERT(hwirq);
	sunxi_pcie_app_writel(mask, sunxi_pcie, SII_INT_MASK0);
	stas = sunxi_pcie_app_readl(sunxi_pcie, SII_INT_STAS0);
	stas |= INTX_RX_ASSERT(hwirq);
	sunxi_pcie_app_writel(stas, sunxi_pcie, SII_INT_STAS0);
	raw_spin_unlock_irqrestore(&pp->lock, flags);
}

static void sunxi_pcie_intx_irq_unmask(struct irq_data *data)
{
	struct sunxi_pcie *sunxi_pcie = irq_data_get_irq_chip_data(data);
	struct dw_pcie *pci = sunxi_pcie->pci;
	struct dw_pcie_rp *pp = &pci->pp;
	irq_hw_number_t hwirq = irqd_to_hwirq(data);
	unsigned long flags;
	u32 mask, stas;

	raw_spin_lock_irqsave(&pp->lock, flags);
	stas = sunxi_pcie_app_readl(sunxi_pcie, SII_INT_STAS0);
	stas |= INTX_RX_ASSERT(hwirq);
	sunxi_pcie_app_writel(stas, sunxi_pcie, SII_INT_STAS0);
	mask = sunxi_pcie_app_readl(sunxi_pcie, SII_INT_MASK0);
	mask |= INTX_RX_ASSERT(hwirq);
	sunxi_pcie_app_writel(mask, sunxi_pcie, SII_INT_MASK0);
	raw_spin_unlock_irqrestore(&pp->lock, flags);
}

static struct irq_chip sunxi_pcie_sii_intx_chip = {
	.name		= "PCI-INTx",
	.irq_mask	= sunxi_pcie_intx_irq_mask,
	.irq_unmask	= sunxi_pcie_intx_irq_unmask,
	.flags		= IRQCHIP_SKIP_SET_WAKE | IRQCHIP_MASK_ON_SUSPEND,
};

static int sunxi_pcie_intx_map(struct irq_domain *domain, unsigned int irq,
				 irq_hw_number_t hwirq)
{
	irq_set_chip_and_handler(irq, &sunxi_pcie_sii_intx_chip, handle_simple_irq);
	irq_set_chip_data(irq, domain->host_data);
	return 0;
}

static const struct irq_domain_ops intx_domain_ops = {
	.map = sunxi_pcie_intx_map,
};

static int sunxi_pcie_intx_irq_domain_alloc(struct sunxi_pcie *sunxi_pcie)
{
	struct dw_pcie *pci = sunxi_pcie->pci;
	struct device_node *intc_node;

	intc_node = of_get_child_by_name(pci->dev->of_node, "legacy-interrupt-controller");
	if (!intc_node) {
		dev_warn(pci->dev, "missing child interrupt-controller node\n");
		return -ENODEV;
	}

	sunxi_pcie->intx_domain = irq_domain_add_linear(intc_node, PCI_NUM_INTX,
							&intx_domain_ops, sunxi_pcie);
	of_node_put(intc_node);
	if (!sunxi_pcie->intx_domain) {
		dev_err(pci->dev, "failed to get a INTx IRQ domain\n");
		return -EINVAL;
	}

	return 0;
}

static void sunxi_pcie_intx_irq_domain_free(struct sunxi_pcie *sunxi_pcie)
{
	if (sunxi_pcie->intx_domain)
		irq_domain_remove(sunxi_pcie->intx_domain);
}

static void sunxi_pcie_legacy_int_handler(struct irq_desc *desc)
{
	struct irq_chip *chip = irq_desc_get_chip(desc);
	struct sunxi_pcie *sunxi_pcie = irq_desc_get_handler_data(desc);
	unsigned long intx;
	u32 mask, stas;
	u32 shift = __ffs(INTX_RX_ASSERT_MASK);
	u32 hwirq = shift;

	chained_irq_enter(chip, desc);

	mask = sunxi_pcie_app_readl(sunxi_pcie, SII_INT_MASK0);
	stas = sunxi_pcie_app_readl(sunxi_pcie, SII_INT_STAS0);
	intx = mask & stas & INTX_RX_ASSERT_MASK;

	for_each_set_bit_from(hwirq, &intx, PCI_NUM_INTX + shift) {
		/* Clear INTx status */
		sunxi_pcie_app_writel(BIT(hwirq), sunxi_pcie, SII_INT_STAS0);
		generic_handle_domain_irq(sunxi_pcie->intx_domain, hwirq - shift);
	}

	chained_irq_exit(chip, desc);
}

static int sunxi_pcie_host_init(struct dw_pcie_rp *pp)
{
	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);
	struct sunxi_pcie *sunxi_pcie = to_sunxi_pcie(pci);
	int ret;

	ret = sunxi_pcie_intx_irq_domain_alloc(sunxi_pcie);
	if (ret < 0)
		dev_warn(pci->dev, "failed to init irq domain\n");

	irq_set_chained_handler_and_data(sunxi_pcie->sii_irq, sunxi_pcie_legacy_int_handler, sunxi_pcie);

	return 0;
}

static void sunxi_pcie_host_deinit(struct dw_pcie_rp *pp)
{
	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);
	struct sunxi_pcie *sunxi_pcie = to_sunxi_pcie(pci);

	sunxi_pcie_intx_irq_domain_free(sunxi_pcie);
}

static const struct dw_pcie_host_ops sunxi_pcie_host_ops = {
	.host_init	= sunxi_pcie_host_init,
	.host_deinit	= sunxi_pcie_host_deinit,
};

static void sunxi_pcie_bandwith_limited(struct sunxi_pcie *sunxi_pcie)
{
#if IS_ENABLED(CONFIG_AW_NSI)
	struct dw_pcie *pci = sunxi_pcie->pci;
	u32 offset, val;
	u32 gen, lane;
	u32 bwl, port;

	offset = dw_pcie_find_capability(pci, PCI_CAP_ID_EXP);
	val = dw_pcie_readw_dbi(pci, offset + PCI_EXP_LNKSTA);
	gen = FIELD_GET(PCI_EXP_LNKSTA_CLS, val);
	lane = FIELD_GET(PCI_EXP_LNKSTA_NLW, val);

	switch (pcie_link_speed[gen]) {
	case PCIE_SPEED_2_5GT:
		/* Gen1 250MB/s per lane */
		bwl = 250;
		break;
	case PCIE_SPEED_5_0GT:
		/* Gen2 500MB/s per lane */
		bwl = 500;
		break;
	case PCIE_SPEED_8_0GT:
		/* Gen3 984.6MB/s per lane */
		bwl = 985;
		break;
	case PCIE_SPEED_16_0GT:
		/* Gen4 1.969GB/s per lane */
		bwl = 1969;
		break;
	default:
		dev_err(pci->dev, "bandwith limited unknow gen %d\n", gen);
		return ;
	}

	bwl *= lane;

	for_each_ports(port) {
		if (strstr(get_name(port), "pcie")) {
			nsi_port_set_abs_bwlen(port, false);
			nsi_port_set_abs_bwl(port, bwl);
			nsi_port_set_abs_bwlen(port, true);
			dev_info_once(pci->dev, "bandwith limited %d MB/s\n", bwl);
		}
	}
#endif
}

static u64 sunxi_pcie_cpu_addr_fixup(struct dw_pcie *pci, u64 cpu_addr)
{
	struct sunxi_pcie *sunxi_pcie = to_sunxi_pcie(pci);

	return cpu_addr - sunxi_pcie->hw_data->cpu_addr_fix;
}

static int sunxi_pcie_link_up(struct dw_pcie *pci)
{
	struct sunxi_pcie *sunxi_pcie = to_sunxi_pcie(pci);
	u32 val = sunxi_pcie_app_readl(sunxi_pcie, SII_INT_STAS1);

	if ((val & RDLH_LINK_UP) && (val & SMLH_LINK_UP)) {
		if (sunxi_pcie->hw_data->bandwith_limited)
			sunxi_pcie_bandwith_limited(sunxi_pcie);
		return true;
	}

	return false;
}

static void sunxi_pcie_ltssm_enable(struct sunxi_pcie *sunxi_pcie)
{
	u32 val;

	val = sunxi_pcie_app_readl(sunxi_pcie, PCIE_LTSSM_ENABLE);
	val |= APP_LTSSM_ENABLE;
	sunxi_pcie_app_writel(val, sunxi_pcie, PCIE_LTSSM_ENABLE);
}

static void sunxi_pcie_ltssm_disable(struct sunxi_pcie *sunxi_pcie)
{
	u32 val;

	val = sunxi_pcie_app_readl(sunxi_pcie, PCIE_LTSSM_ENABLE);
	val &= ~APP_LTSSM_ENABLE;
	sunxi_pcie_app_writel(val, sunxi_pcie, PCIE_LTSSM_ENABLE);
}

static int sunxi_pcie_start_link(struct dw_pcie *pci)
{
	struct sunxi_pcie *sunxi_pcie = to_sunxi_pcie(pci);

	gpiod_set_value_cansleep(sunxi_pcie->rst_gpio, 1);
	msleep(100);
	gpiod_set_value_cansleep(sunxi_pcie->rst_gpio, 0);

	sunxi_pcie_ltssm_enable(sunxi_pcie);

	return 0;
}

static void sunxi_pcie_stop_link(struct dw_pcie *pci)
{
	struct sunxi_pcie *sunxi_pcie = to_sunxi_pcie(pci);

	sunxi_pcie_ltssm_disable(sunxi_pcie);
	gpiod_set_value_cansleep(sunxi_pcie->rst_gpio, 1);
}

static const struct dw_pcie_ops sunxi_pcie_ops = {
	.cpu_addr_fixup	= sunxi_pcie_cpu_addr_fixup,
	.link_up	= sunxi_pcie_link_up,
	.start_link	= sunxi_pcie_start_link,
	.stop_link	= sunxi_pcie_stop_link,
};

static int sunxi_pcie_hw_init(struct sunxi_pcie *sunxi_pcie)
{
	struct dw_pcie *pci = sunxi_pcie->pci;
	int ret;

	/* regulators enable */
	ret = regulator_bulk_enable(sunxi_pcie->regulator_cnt, sunxi_pcie->regulators);
	if (ret) {
		dev_err(pci->dev, "failed to enable regulator bulk %d\n", ret);
		goto err_regulator;
	}

	/* resets deassert */
	ret = reset_control_bulk_deassert(sunxi_pcie->rst_cnt, sunxi_pcie->rsts);
	if (ret) {
		dev_err(pci->dev, "failed to reset control bulk %d\n", ret);
		goto err_reset;
	}

	/* clocks enable */
	ret = clk_bulk_prepare_enable(sunxi_pcie->clk_cnt, sunxi_pcie->clks);
	if (ret) {
		dev_err(pci->dev, "failed to enable clk bulk %d\n", ret);
		goto err_clk;
	}

	/* phy enable */
	ret = phy_init(sunxi_pcie->phy);
	if (ret) {
		dev_err(pci->dev, "fail to init phy %d\n", ret);
		goto err_phy;
	}

	return 0;
err_phy:
	clk_bulk_disable_unprepare(sunxi_pcie->clk_cnt, sunxi_pcie->clks);
err_clk:
	reset_control_bulk_assert(sunxi_pcie->rst_cnt, sunxi_pcie->rsts);
err_reset:
	regulator_bulk_disable(sunxi_pcie->regulator_cnt, sunxi_pcie->regulators);
err_regulator:
	return ret;
}

static void sunxi_pcie_hw_deinit(struct sunxi_pcie *sunxi_pcie)
{
	phy_exit(sunxi_pcie->phy);
	clk_bulk_disable_unprepare(sunxi_pcie->clk_cnt, sunxi_pcie->clks);
	reset_control_bulk_assert(sunxi_pcie->rst_cnt, sunxi_pcie->rsts);
	regulator_bulk_disable(sunxi_pcie->regulator_cnt, sunxi_pcie->regulators);
}

static int sunxi_pcie_resource_get(struct platform_device *pdev, struct sunxi_pcie *sunxi_pcie)
{
	struct dw_pcie *pci = sunxi_pcie->pci;
	int i, ret;

	/* regulators resource get */
	sunxi_pcie->regulator_cnt = ARRAY_SIZE(sunxi_pcie_regulators);
	sunxi_pcie->regulators = devm_kcalloc(pci->dev, sunxi_pcie->regulator_cnt, sizeof(*sunxi_pcie->regulators), GFP_KERNEL);
	for (i = 0; i < sunxi_pcie->regulator_cnt; i++)
		sunxi_pcie->regulators[i].supply = sunxi_pcie_regulators[i];
	ret = devm_regulator_bulk_get(pci->dev, sunxi_pcie->regulator_cnt, sunxi_pcie->regulators);
	if (ret)
		return dev_err_probe(pci->dev, ret, "failed to get regulator bulk\n");

	/* clocks resource get */
	sunxi_pcie->clk_cnt = ARRAY_SIZE(sunxi_pcie_clks);
	sunxi_pcie->clks = devm_kcalloc(pci->dev, sunxi_pcie->clk_cnt, sizeof(*sunxi_pcie->clks), GFP_KERNEL);
	for (i = 0; i < sunxi_pcie->clk_cnt; i++)
		sunxi_pcie->clks[i].id = sunxi_pcie_clks[i];
	ret = devm_clk_bulk_get_optional(pci->dev, sunxi_pcie->clk_cnt, sunxi_pcie->clks);
	if (ret < 0)
		return dev_err_probe(pci->dev, ret, "failed to get clk bulk\n");

	/* resets resource get */
	sunxi_pcie->rst_cnt = ARRAY_SIZE(sunxi_pcie_rsts);
	sunxi_pcie->rsts = devm_kcalloc(pci->dev, sunxi_pcie->rst_cnt, sizeof(*sunxi_pcie->rsts), GFP_KERNEL);
	for (i = 0; i < sunxi_pcie->rst_cnt; i++)
		sunxi_pcie->rsts[i].id = sunxi_pcie_rsts[i];
	ret = devm_reset_control_bulk_get_optional_shared(pci->dev, sunxi_pcie->rst_cnt, sunxi_pcie->rsts);
	if (ret)
		return dev_err_probe(pci->dev, ret, "failed to get reset control bulk\n");

	/* gpios resource get */
	sunxi_pcie->rst_gpio = devm_gpiod_get_optional(pci->dev, "reset", GPIOD_OUT_LOW);
	if (IS_ERR(sunxi_pcie->rst_gpio))
		return dev_err_probe(pci->dev, PTR_ERR(sunxi_pcie->rst_gpio), "failed get reset gpiod\n");

	/* irq resource get */
	ret = platform_get_irq_byname(pdev, "sii");
	if (ret < 0)
		return dev_err_probe(pci->dev, ret, "failed get sii irq\n");
	sunxi_pcie->sii_irq = ret;

	/* iomem resource get */
	sunxi_pcie->app_base = devm_platform_ioremap_resource_byname(pdev, "app");
	if (IS_ERR(sunxi_pcie->app_base))
		return dev_err_probe(pci->dev, PTR_ERR(sunxi_pcie->app_base), "failed ioremap app resource\n");

	/* phy resource get */
	sunxi_pcie->phy = devm_phy_get(pci->dev, "pcie-phy");
	if (IS_ERR(sunxi_pcie->phy))
		return dev_err_probe(pci->dev, PTR_ERR(sunxi_pcie->phy), "failed get phy\n");

	return 0;
}

static int sunxi_pcie_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct sunxi_pcie *sunxi_pcie;
	struct dw_pcie *pci;
	struct dw_pcie_rp *pp;
	int ret;

	sunxi_pcie = devm_kzalloc(dev, sizeof(*sunxi_pcie), GFP_KERNEL);
	if (!sunxi_pcie)
		return -ENOMEM;

	sunxi_pcie->hw_data = of_device_get_match_data(dev);
	if (!sunxi_pcie->hw_data)
		return -EINVAL;

	pci = devm_kzalloc(dev, sizeof(*pci), GFP_KERNEL);
	if (!pci)
		return -ENOMEM;

	pci->dev = dev;
	pci->ops = &sunxi_pcie_ops;

	pp = &pci->pp;
	pp->ops = &sunxi_pcie_host_ops;

	sunxi_pcie->pci = pci;
	platform_set_drvdata(pdev, sunxi_pcie);

	ret = sunxi_pcie_resource_get(pdev, sunxi_pcie);
	if (ret) {
		dev_err(dev, "failed to get resource %d\n", ret);
		return ret;
	}

	pm_runtime_enable(dev);
	ret = pm_runtime_get_sync(dev);
	if (ret < 0) {
		dev_err(dev, "failed to get runtime sync %d\n", ret);
		return ret;
	}

	ret = sunxi_pcie_hw_init(sunxi_pcie);
	if (ret) {
		dev_err(dev, "failed to initialize hw %d\n", ret);
		goto err_hw_init;
	}

	ret = dw_pcie_host_init(pp);
	if (ret) {
		dev_err(dev, "failed to initialize host %d\n", ret);
		goto err_host_init;
	}

	dev_info(dev, "probe success (version: %s)\n", SUNXI_PCIE_MODULE_VERSION);

	return 0;

err_host_init:
	sunxi_pcie_hw_deinit(sunxi_pcie);
err_hw_init:
	pm_runtime_put_sync(dev);
	pm_runtime_disable(dev);
	return ret;
}

static int sunxi_pcie_remove(struct platform_device *pdev)
{
	struct sunxi_pcie *sunxi_pcie = platform_get_drvdata(pdev);
	struct dw_pcie *pci = sunxi_pcie->pci;
	struct dw_pcie_rp *pp = &pci->pp;

	dw_pcie_host_deinit(pp);
	sunxi_pcie_hw_deinit(sunxi_pcie);
	pm_runtime_put_sync(pci->dev);
	pm_runtime_disable(pci->dev);

	return 0;
}

#if IS_ENABLED(CONFIG_PM)
static int sunxi_pcie_suspend_noirq(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct sunxi_pcie *sunxi_pcie = platform_get_drvdata(pdev);
	struct dw_pcie *pci = sunxi_pcie->pci;

	dw_pcie_stop_link(pci);
	udelay(200);
	sunxi_pcie_hw_deinit(sunxi_pcie);

	dev_info(pci->dev, "suspend noirq finished\n");

	return 0;
}

static int sunxi_pcie_resume_noirq(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct sunxi_pcie *sunxi_pcie = platform_get_drvdata(pdev);
	struct dw_pcie *pci = sunxi_pcie->pci;
	int ret;

	ret = sunxi_pcie_hw_init(sunxi_pcie);
	if (ret) {
		dev_err(dev, "failed to initialize hw %d\n", ret);
		goto err_hw_init;
	}

	udelay(200);

	dw_pcie_setup_rc(&pci->pp);

	ret = dw_pcie_start_link(pci);
	if (ret) {
		dev_err(dev, "failed to start link %d\n", ret);
		return ret;
	}

	ret = dw_pcie_wait_for_link(pci);
	if (ret) {
		dev_err(dev, "wait for link timeout %d\n", ret);
		return ret;
	}

	dev_info(pci->dev, "resume noirq finished\n");

	return 0;
err_hw_init:
	return ret;
}

static struct dev_pm_ops sunxi_pcie_pm_ops = {
	SET_NOIRQ_SYSTEM_SLEEP_PM_OPS(sunxi_pcie_suspend_noirq, sunxi_pcie_resume_noirq)
};
#else
static struct dev_pm_ops sunxi_pcie_pm_ops;
#endif /* CONFIG_PM */

static const struct sunxi_pcie_of_data cfg_0_0_0 = {
	.cpu_addr_fix = 0x20000000,
	.bandwith_limited = true,
};

static const struct sunxi_pcie_of_data cfg_0_1_0 = {
	.bandwith_limited = true,
};

static const struct of_device_id sunxi_pcie_of_match[] = {
	{ .compatible = "allwinner,sun55iw3-pcie", .data = &cfg_0_0_0, },
	{ .compatible = "allwinner,sun60iw2-pcie", .data = &cfg_0_1_0, },
	{ .compatible = "allwinner,sun65iw1-pcie", .data = &cfg_0_1_0, },
	{},
};

static struct platform_driver sunxi_pcie_driver = {
	.driver = {
		.name	= "sunxi-dw-pcie",
		.of_match_table = sunxi_pcie_of_match,
		.suppress_bind_attrs = true,
		.pm = &sunxi_pcie_pm_ops,
	},
	.probe = sunxi_pcie_probe,
	.remove = sunxi_pcie_remove,
};
module_platform_driver(sunxi_pcie_driver);

MODULE_AUTHOR("jingyanliang <jingyanliang@allwinnertech.com>");
MODULE_DESCRIPTION("Allwinner DesignWare PCIe Controller Host Drivers");
MODULE_VERSION(SUNXI_PCIE_MODULE_VERSION);
MODULE_LICENSE("GPL v2");