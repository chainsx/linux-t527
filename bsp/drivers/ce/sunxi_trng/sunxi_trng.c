/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * (C) Copyright 2007-2013
 * Reuuimlla Technology Co., Ltd. <www.reuuimllatech.com>
 * Aaron.Maoye <leafy.myeh@reuuimllatech.com>
 *
 * Driver of Allwinner TRNG controller.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 */

#include <sunxi-log.h>
#include <sunxi-common.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/clk.h>
#include <linux/reset.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <crypto/internal/rng.h>
#include <sunxi-trng.h>

#define SUNXI_TRNG_VERSION	"1.0.0"

/* define trng reg addr */
#define TRNG_CTRL_CFG			(0x00)
#define TRNG_JITTER_CFG			(0x04)
#define TRNG_JITTER_CNT_TIMING		(0x08)
#define TRNG_MONITOR_RCT		(0x0C)
#define TRNG_MONITOR_APT		(0x10)
#define TRNG_EXSTRACT_CFG		(0x14)
#define TRNG_RAND_BIT_URN0		(0x18)
#define TRNG_RAND_BIT_URN1		(0x1C)
#define TRNG_RAND_BIT_URN2		(0x20)
#define TRNG_RAND_BIT_URN3		(0x24)
#define TRNG_JITTER_CNT_RESULT0		(0x28)
#define TRNG_JITTER_CNT_RESULT1		(0x2C)
#define TRNG_JITTER_CNT_RESULT2		(0x30)
#define TRNG_JITTER_CNT_RESULT3		(0x34)
#define TRNG_JITTER_CNT_RESULT4		(0x38)
#define TRNG_JITTER_CNT_RESULT5		(0x3C)
#define TRNG_JITTER_CNT_RESULT6		(0x40)
#define TRNG_JITTER_CNT_RESULT7		(0x44)
#define TRNG_JITTER_COUNTER_READY	(0x48)
#define TRNG_REG_ACCESS_CTR1		(0x4C)
#define TRNG_REG_ACCESS_CTR2		(0x50)

/* TRNG_CTRL_CFG */
#define TRNG_READY_OFFSET		(12)
#define TRNG_READY_MASK			(0x7 << TRNG_READY_OFFSET)
#define TRNG_RO_CTRL			(0xff << 4)
#define TRNG_EN				(0x1)

/* TRNG_JITTER_CFG */
#define TRNG_JITTER_WORK_EN		(0x1)

/* TRNG_MONITOR_RCT */
#define TRNG_MONITOR_EN			(0x1 << 11)
#define TRNG_RCT_C_VAL			(0x408)
#define TRNG_RCT_C_OFFSET		(0)
#define TRNG_RCT_C			(TRNG_RCT_C_VAL << TRNG_RCT_C_OFFSET)

/* TRNG_MONITOR_APT */
#define TRNG_APT_W_VAL			(0x400)
#define TRNG_APT_W_OFFSET		(12)
#define TRNG_APT_W			(TRNG_APT_W_VAL << TRNG_APT_W_OFFSET)
#define TRNG_APT_C			(0x3f1)

/* TRNG_EXSTRACT_CFG */
#define TRNG_RESILIENT_RATIO_VAL	(0xfff)
#define TRNG_RESILIENT_RATIO_OFFSET	(12)
#define TRNG_RESILIENT_RATIO		(TRNG_RESILIENT_RATIO_VAL << TRNG_RESILIENT_RATIO_OFFSET)
#define TRNG_EXTRACT_START		(0x1 << 9)
#define TRNG_RESILIENT_TYPE		(0x1 << 8)
#define TRNG_RO_SAMPLING_RATION1	(0xf << 4)
#define TRNG_RO_SAMPLING_RATION0	(0xf)

/* The number of random values obtained each time is fixed at 4 */
#define EACH_TIME_GET_RANDOM_VAL_NUM	4
#define SUNXI_TRNG_MAX_LEN		32

#define SUNXI_TRNG_DEV_NAME		"sunxi-trng"

static void __iomem *sunxi_trng_base_addr;

static void sunxi_trng_enable(void)
{
	u32 reg_val;

	reg_val = readl(sunxi_trng_base_addr + TRNG_CTRL_CFG);
	reg_val |= TRNG_EN | TRNG_RO_CTRL;
	writel(reg_val, sunxi_trng_base_addr + TRNG_CTRL_CFG);

	reg_val = readl(sunxi_trng_base_addr + TRNG_MONITOR_RCT);
	reg_val |= TRNG_MONITOR_EN;
	writel(reg_val, sunxi_trng_base_addr + TRNG_MONITOR_RCT);
}

static void sunxi_trng_disable(void)
{
	u32 reg_val;

	reg_val = readl(sunxi_trng_base_addr + TRNG_JITTER_CFG);
	reg_val &= ~TRNG_JITTER_WORK_EN;
	writel(reg_val, sunxi_trng_base_addr + TRNG_JITTER_CFG);

	writel(0, sunxi_trng_base_addr + TRNG_EXSTRACT_CFG);

	reg_val = readl(sunxi_trng_base_addr + TRNG_MONITOR_RCT);
	reg_val &= ~TRNG_MONITOR_EN;
	writel(reg_val, sunxi_trng_base_addr + TRNG_MONITOR_RCT);

	writel(0, sunxi_trng_base_addr + TRNG_CTRL_CFG);
}

static void sunxi_trng_config(void)
{
	u32 reg_val;

	reg_val = readl(sunxi_trng_base_addr + TRNG_EXSTRACT_CFG);
	reg_val |= TRNG_RESILIENT_RATIO;
	writel(reg_val, sunxi_trng_base_addr + TRNG_EXSTRACT_CFG);
}

static void sunxi_trng_monitor_cfg_init(void)
{
	u32 reg_val;

	reg_val = readl(sunxi_trng_base_addr + TRNG_MONITOR_RCT);
	reg_val |= TRNG_RCT_C;
	writel(reg_val, sunxi_trng_base_addr + TRNG_MONITOR_RCT);

	reg_val = readl(sunxi_trng_base_addr + TRNG_MONITOR_APT);
	reg_val |= TRNG_APT_C | TRNG_APT_W;
	writel(reg_val, sunxi_trng_base_addr + TRNG_MONITOR_APT);
}

static int sunxi_trng_start(void)
{
	u32 reg_val;

	reg_val = readl(sunxi_trng_base_addr + TRNG_EXSTRACT_CFG);
	reg_val |= TRNG_EXTRACT_START;
	writel(reg_val, sunxi_trng_base_addr + TRNG_EXSTRACT_CFG);

	return wait_field_equ(sunxi_trng_base_addr + TRNG_CTRL_CFG, TRNG_READY_MASK, 0, 1000);
}

static void sunxi_trng_get_value(u8 *trng_buf)
{
	trng_buf[0] = readl(sunxi_trng_base_addr + TRNG_RAND_BIT_URN0);
	trng_buf[1] = readl(sunxi_trng_base_addr + TRNG_RAND_BIT_URN1);
	trng_buf[2] = readl(sunxi_trng_base_addr + TRNG_RAND_BIT_URN2);
	trng_buf[3] = readl(sunxi_trng_base_addr + TRNG_RAND_BIT_URN3);
}

static int sunxi_trng_clk_init(struct sunxi_trng *trng)
{
	int err;

	if (reset_control_reset(trng->reset)) {
		dev_err(trng->dev, "reset control deassert failed!\n");
		return -EINVAL;
	}

	if (clk_prepare_enable(trng->clk)) {
		dev_err(trng->dev, "enable trng clock failed!\n");
		err = -EINVAL;
		goto err0;
	}

	return 0;
err0:
	reset_control_assert(trng->reset);
	return err;
}

static int sunxi_trng_resource_get(struct sunxi_trng *trng)
{
	trng->res = platform_get_resource(trng->pdev, IORESOURCE_MEM, 0);
	if (!trng->res) {
		dev_err(trng->dev, "failed to get MEM res\n");
		return -ENXIO;
	}
	sunxi_trng_base_addr = devm_ioremap_resource(trng->dev, trng->res);
	if (IS_ERR(sunxi_trng_base_addr)) {
		dev_err(trng->dev, "unable to ioremap\n");
		return PTR_ERR(sunxi_trng_base_addr);
	}

	trng->clk = devm_clk_get(trng->dev, "trng_clk");
	if (IS_ERR(trng->clk)) {
		dev_err(trng->dev, "request clock failed\n");
		return -EINVAL;
	}

	trng->reset = devm_reset_control_get(trng->dev, NULL);
	if (IS_ERR_OR_NULL(trng->reset)) {
		dev_err(trng->dev, "request reset failed\n");
		return -EINVAL;
	}

	return 0;
}

int sunxi_trng_exstract_random(u8 *trng_buf, u32 trng_len)
{
	int ret, addr_offset, need_get_len = trng_len;
	u8 get_trng_val_buf[EACH_TIME_GET_RANDOM_VAL_NUM] = {0};

	if (!trng_buf) {
		pr_err("input pointer is NULL\n");
		return -1;
	}

	for (addr_offset = 0; addr_offset < trng_len; addr_offset += EACH_TIME_GET_RANDOM_VAL_NUM) {
		ret = sunxi_trng_start();
		if (ret)
			return ret;

		sunxi_trng_get_value(get_trng_val_buf);
		pr_debug("get data is 0x%x, 0x%x, 0x%x, 0x%x\n",
			get_trng_val_buf[0], get_trng_val_buf[1], get_trng_val_buf[2], get_trng_val_buf[3]);

		if (need_get_len > EACH_TIME_GET_RANDOM_VAL_NUM) {
			memcpy(trng_buf + addr_offset, get_trng_val_buf, EACH_TIME_GET_RANDOM_VAL_NUM);
			need_get_len -= EACH_TIME_GET_RANDOM_VAL_NUM;
		} else {
			memcpy(trng_buf + addr_offset, get_trng_val_buf, need_get_len);
			break;
		}
	}

	return 0;
}
EXPORT_SYMBOL(sunxi_trng_exstract_random);

static int sunxi_trng_read_random(struct hwrng *rng, void *buf, size_t max, bool wait)
{
	int ret, addr_offset = 0, need_get_len = max;
	u8 trng_buf[SUNXI_TRNG_MAX_LEN];

	if (!buf) {
		pr_err("read random input pointer is NULL\n");
		return 0;
	}

	for (addr_offset = 0; addr_offset < max; addr_offset += SUNXI_TRNG_MAX_LEN) {
		ret = sunxi_trng_exstract_random(trng_buf, SUNXI_TRNG_MAX_LEN);
		if (ret) {
			pr_err("sunxi_trng_exstract_random: %d!\n", ret);
			return ret;
		}

		if (need_get_len > SUNXI_TRNG_MAX_LEN) {
			memcpy((buf + addr_offset), trng_buf, SUNXI_TRNG_MAX_LEN);
			need_get_len -= SUNXI_TRNG_MAX_LEN;
		} else {
			memcpy((buf + addr_offset), trng_buf, need_get_len);
			break;
		}
	}

	return max;
}

static int sunxi_trng_register_hwrng(struct sunxi_trng *trng)
{
	int ret;

	trng->hwrng = devm_kzalloc(trng->dev, sizeof(struct hwrng), GFP_KERNEL);
	if (!trng->hwrng) {
		dev_err(trng->dev, "failed to devm_kzalloc: %d!\n", ret);
		return -ENOMEM;
	}

	trng->hwrng->name = SUNXI_TRNG_DEV_NAME;
	trng->hwrng->read = sunxi_trng_read_random;
	trng->hwrng->quality = 512;
	trng->hwrng->priv = (unsigned long)trng->dev;

	ret = devm_hwrng_register(trng->dev, trng->hwrng);
	if (ret) {
		dev_err(trng->dev, "failed to register hwrng: %d!\n", ret);
		return ret;
	}

	return 0;
}

static int sunxi_trng_probe(struct platform_device *pdev)
{
	struct sunxi_trng *trng;
	int err;

	trng = devm_kzalloc(&pdev->dev, sizeof(*trng), GFP_KERNEL);
	if (IS_ERR_OR_NULL(trng))
		return -ENOMEM;

	trng->pdev = pdev;
	trng->dev = &pdev->dev;

	err = sunxi_trng_resource_get(trng);
	if (err)
		return err;

	err = sunxi_trng_clk_init(trng);
	if (err)
		return err;

	sunxi_trng_monitor_cfg_init();

	err = sunxi_trng_register_hwrng(trng);
	if (err)
		return err;

	sunxi_trng_enable();
	sunxi_trng_config();

	dev_set_drvdata(trng->dev, trng);

	dev_info(trng->dev, "v%s probe success\n", SUNXI_TRNG_VERSION);

	return 0;
}

static int sunxi_trng_remove(struct platform_device *pdev)
{
	struct sunxi_trng *trng = platform_get_drvdata(pdev);

	sunxi_trng_disable();
	clk_disable_unprepare(trng->clk);
	reset_control_assert(trng->reset);

	return 0;
}

static const struct of_device_id sunxi_trng_match[] = {
	{ .compatible = "allwinner,sunxi-trng", },
	{},
};
MODULE_DEVICE_TABLE(of, sunxi_trng_match);


static struct platform_driver sunxi_trng_driver = {
	.probe  = sunxi_trng_probe,
	.remove = sunxi_trng_remove,
	.driver = {
		.name  = "sunxi-trng",
		.of_match_table = sunxi_trng_match,
	},
};

static int __init sunxi_trng_init(void)
{
	return platform_driver_register(&sunxi_trng_driver);
}

static void __exit sunxi_trng_exit(void)
{
	platform_driver_unregister(&sunxi_trng_driver);
}

module_init(sunxi_trng_init);
module_exit(sunxi_trng_exit);

MODULE_AUTHOR("chenmingxi<chenmingxi@allwinnertech.com>");
MODULE_DESCRIPTION("Driver for Allwinner TRNG controller");
MODULE_LICENSE("GPL");
MODULE_VERSION(SUNXI_TRNG_VERSION);
