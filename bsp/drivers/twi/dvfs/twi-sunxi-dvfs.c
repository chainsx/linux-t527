/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * SUNXI TWI Controller Dvfs Driver
 *
 */

#define SUNXI_MODNAME "twi"
#include <sunxi-log.h>
#include "twi-sunxi-dvfs.h"

static void sunxi_twi_dvfs_set_packet_interval(void __iomem *base_addr, u16 val)
{
	u32 reg_val = readl(base_addr + TWI_DVFS_CTL);
	u32 reg_old = reg_val;

	reg_val &= ~PACKET_INTERVAL;
	reg_val |= FIELD_PREP(TWI_DVFS_PKT_INTERVAL, val);

	if (reg_val != reg_old)
		writel(reg_val, base_addr + TWI_DVFS_CTL);
}

static void sunxi_twi_dvfs_readback_enable(void __iomem *base_addr, int ch)
{
	u32 reg_val = readl(base_addr + TWI_DVFS_CTL);
	u32 reg_old = reg_val;

	reg_val |= TWI_DVFS_RB_EN(ch);

	if (reg_val != reg_old)
		writel(reg_val, base_addr + TWI_DVFS_CTL);
}

static void sunxi_twi_dvfs_readback_disable(void __iomem *base_addr, int ch)
{
	u32 reg_val = readl(base_addr + TWI_DVFS_CTL);
	u32 reg_old = reg_val;

	reg_val &= ~TWI_DVFS_RB_EN(ch);

	if (reg_val != reg_old)
		writel(reg_val, base_addr + TWI_DVFS_CTL);
}

static void sunxi_twi_dvfs_channel_enable(void __iomem *base_addr, int ch)
{
	u32 reg_val = readl(base_addr + TWI_DVFS_CTL);
	u32 reg_old = reg_val;

	reg_val |= TWI_DVFS_CH_EN(ch);

	if (reg_val != reg_old)
		writel(reg_val, base_addr + TWI_DVFS_CTL);
}

static void sunxi_twi_dvfs_channel_disable(void __iomem *base_addr, int ch)
{
	u32 reg_val = readl(base_addr + TWI_DVFS_CTL);
	u32 reg_old = reg_val;

	reg_val &= ~TWI_DVFS_CH_EN(ch);

	if (reg_val != reg_old)
		writel(reg_val, base_addr + TWI_DVFS_CTL);
}

static void sunxi_twi_dvfs_rb_irq_enable(void __iomem *base_addr, int ch)
{
	u32 reg_val = readl(base_addr + TWI_DVFS_INT_CTRL);
	u32 reg_old = reg_val;

	reg_val |= TWI_DVFS_IRQ_RB_ERR(ch);

	if (reg_val != reg_old)
		writel(reg_val, base_addr + TWI_DVFS_INT_CTRL);
}

static void sunxi_twi_dvfs_rb_irq_disable(void __iomem *base_addr, int ch)
{
	u32 reg_val = readl(base_addr + TWI_DVFS_INT_CTRL);
	u32 reg_old = reg_val;

	reg_val &= ~TWI_DVFS_IRQ_RB_ERR(ch);

	if (reg_val != reg_old)
		writel(reg_val, base_addr + TWI_DVFS_INT_CTRL);
}

static void sunxi_twi_dvfs_tx_irq_enable(void __iomem *base_addr, int ch)
{
	u32 reg_val = readl(base_addr + TWI_DVFS_INT_CTRL);
	u32 reg_old = reg_val;

	reg_val |= TWI_DVFS_IRQ_TRAN_ERR(ch);

	if (reg_val != reg_old)
		writel(reg_val, base_addr + TWI_DVFS_INT_CTRL);
}

static void sunxi_twi_dvfs_tx_irq_disable(void __iomem *base_addr, u8 ch)
{
	u32 reg_val = readl(base_addr + TWI_DVFS_INT_CTRL);
	u32 reg_old = reg_val;

	reg_val &= ~(TWI_DVFS_IRQ_TRAN_ERR(ch));

	if (reg_val != reg_old)
		writel(reg_val, base_addr + TWI_DVFS_INT_CTRL);
}

static u32 sunxi_twi_dvfs_get_irq_status(void __iomem *base_addr)
{
	return readl(base_addr + TWI_DVFS_INT_STAT);
}

static void sunxi_twi_dvfs_clr_irq_status(void __iomem *base_addr, u32 bitmap)
{
	writel(bitmap, base_addr + TWI_DVFS_INT_STAT);
}

static void sunxi_twi_dvfs_set_dev_reg(void __iomem *base_addr, int ch, u8 reg_addr)
{
	u32 reg_val = readl(base_addr + TWI_DVFS_DEV_CFG(ch));
	u32 reg_old = reg_val;

	reg_val &= ~TWI_DVFS_DEV_CFG_REG_ADDR;
	reg_val |= FIELD_PREP(TWI_DVFS_DEV_CFG_REG_ADDR, reg_addr);

	if (reg_val != reg_old)
		writel(reg_val, base_addr + TWI_DVFS_DEV_CFG(ch));
}

static void sunxi_twi_dvfs_set_dev_mask(void __iomem *base_addr, int ch, u8 mask)
{
	u32 reg_val = readl(base_addr + TWI_DVFS_DEV_CFG(ch));
	u32 reg_old = reg_val;

	reg_val |= TWI_DVFS_DEV_CFG_MASK_EN;
	reg_val &= ~TWI_DVFS_DEV_CFG_MASK;
	reg_val |= FIELD_PREP(TWI_DVFS_DEV_CFG_MASK, mask);

	if (reg_val != reg_old)
		writel(reg_val, base_addr + TWI_DVFS_DEV_CFG(ch));
}

static u32 sunxi_twi_dvfs_get_dev_status(void __iomem *base_addr, int ch)
{
	return readl(base_addr + TWI_DVFS_DEV_STAT(ch));
}

static void sunxi_twi_dvfs_set_rr8_control(void __iomem *base_addr, u8 u0, u8 u1, u8 u2)
{
	u8 bitmap = (u2 << 2) | (u1 << 1) | (u0);

	writel(bitmap, base_addr + TWI_DVFS_RR8_CTL);
}

static void sunxi_twi_dvfs_set_dev_arbiter(void __iomem *base_addr, int ch, u8 priority)
{
	u32 reg_val = readl(base_addr + TWI_DVFS_ARBITER);
	u32 reg_old = reg_val;

	reg_val &= ~TWI_DVFS_DEV_PROI_MASK(ch);
	reg_val |= (priority << TWI_DVFS_DEV_PROI_LOW(ch));

	if (reg_val != reg_old)
		writel(reg_val, base_addr + TWI_DVFS_ARBITER);
}

int sunxi_twi_dvfs_enable(struct i2c_adapter *adap, int ch)
{
	struct sunxi_twi *twi = (struct sunxi_twi *)adap->algo_data;
	if (!twi->twi_dvfs_enable) {
		sunxi_err(twi->dev, "dvfs function must be used in drv and no_suspend mode\n");
		return -EPERM;
	}
	if (ch >= twi->data->dvfs_valid_chan_nums) {
		sunxi_err(twi->dev, "target chan %d exceed valid chan nums %d\n", ch, twi->data->dvfs_valid_chan_nums);
		return -ENOMEM;
	}

	sunxi_twi_dvfs_tx_irq_enable(twi->base_addr, ch);

	if (twi->dvfs_devices->dvfs_channel[ch].readback_en) {
		sunxi_twi_dvfs_readback_enable(twi->base_addr, ch);
		sunxi_twi_dvfs_rb_irq_enable(twi->base_addr, ch);
	}

	sunxi_twi_dvfs_channel_enable(twi->base_addr, ch);
	twi->dvfs_devices->dvfs_channel[ch].channel_en = true;

	sunxi_twi_dvfs_set_rr8_control(twi->base_addr, 1, 0, 0);
	return 0;
}
EXPORT_SYMBOL_GPL(sunxi_twi_dvfs_enable);

int sunxi_twi_dvfs_disable(struct i2c_adapter *adap, int ch)
{
	struct sunxi_twi *twi = (struct sunxi_twi *)adap->algo_data;
	if (!twi->twi_dvfs_enable) {
		sunxi_err(twi->dev, "dvfs function must be used in drv and no_suspend mode\n");
		return -EPERM;
	}
	if (ch >= twi->data->dvfs_valid_chan_nums) {
		sunxi_err(twi->dev, "target chan %d exceed valid chan nums %d\n", ch, twi->data->dvfs_valid_chan_nums);
		return -ENOMEM;
	}

	sunxi_twi_dvfs_tx_irq_disable(twi->base_addr, ch);

	if (twi->dvfs_devices->dvfs_channel[ch].readback_en) {
		sunxi_twi_dvfs_readback_disable(twi->base_addr, ch);
		sunxi_twi_dvfs_rb_irq_disable(twi->base_addr, ch);
	}

	sunxi_twi_dvfs_channel_disable(twi->base_addr, ch);
	twi->dvfs_devices->dvfs_channel[ch].channel_en = false;

	sunxi_twi_dvfs_set_rr8_control(twi->base_addr, 0, 0, 0);
	return 0;
}
EXPORT_SYMBOL_GPL(sunxi_twi_dvfs_disable);

int sunxi_twi_dvfs_enable_all(struct i2c_adapter *adap)
{
	unsigned int i = 0;
	struct sunxi_twi *twi = (struct sunxi_twi *)adap->algo_data;
	if (!twi->twi_dvfs_enable) {
		sunxi_err(twi->dev, "dvfs function must be used in drv and no_suspend mode\n");
		return -EPERM;
	}

	for (i = 0; i < twi->data->dvfs_valid_chan_nums; i++)
		sunxi_twi_dvfs_enable(adap, i);

	return 0;
}
EXPORT_SYMBOL_GPL(sunxi_twi_dvfs_enable_all);

int sunxi_twi_dvfs_disable_all(struct i2c_adapter *adap)
{
	unsigned int i = 0;
	struct sunxi_twi *twi = (struct sunxi_twi *)adap->algo_data;
	if (!twi->twi_dvfs_enable) {
		sunxi_err(twi->dev, "dvfs function must be used in drv and no_suspend mode\n");
		return -EPERM;
	}

	for (i = 0; i < twi->data->dvfs_valid_chan_nums; i++)
		sunxi_twi_dvfs_disable(adap, i);
	return 0;
}
EXPORT_SYMBOL_GPL(sunxi_twi_dvfs_disable_all);

int sunxi_twi_dvfs_set_slave_addr(struct i2c_adapter *adap, u8 addr)
{
	unsigned int i = 0;
	struct sunxi_twi *twi = (struct sunxi_twi *)adap->algo_data;
	struct i2c_msg msg = {
		.addr = addr,
	};

	if (!twi->twi_dvfs_enable) {
		sunxi_err(twi->dev, "dvfs function must be used in drv and no_suspend mode\n");
		return -EPERM;
	}

	for (i = 0; i < twi->data->dvfs_valid_chan_nums; i++) {
		if (twi->dvfs_devices->dvfs_channel[i].channel_en) {
			sunxi_err(twi->dev, "slave addr can not be set because channel %d is enabled\n", i);
			return -EPERM;
		}
	}

	sunxi_twi_drv_set_slave_addr(twi, &msg);
	twi->dvfs_devices->addr = addr;
	return 0;
}
EXPORT_SYMBOL_GPL(sunxi_twi_dvfs_set_slave_addr);

int sunxi_twi_dvfs_set_interval(struct i2c_adapter *adap, u16 interval)
{
	unsigned i = 0;
	struct sunxi_twi *twi = (struct sunxi_twi *)adap->algo_data;
	if (!twi->twi_dvfs_enable) {
		sunxi_err(twi->dev, "dvfs function must be used in drv and no_suspend mode\n");
		return -EPERM;
	}

	for (i = 0; i < twi->data->dvfs_valid_chan_nums; i++) {
		if (twi->dvfs_devices->dvfs_channel[i].channel_en) {
			sunxi_err(twi->dev, "dvfs packet interval can not be set because channel %d is enabled\n", i);
			return -EPERM;
		}
	}

	sunxi_twi_dvfs_set_packet_interval(twi->base_addr, interval);
	twi->dvfs_devices->interval = interval;
	return 0;
}
EXPORT_SYMBOL_GPL(sunxi_twi_dvfs_set_interval);

int sunxi_twi_dvfs_set_chan_reg(struct i2c_adapter *adap, int ch, u8 reg)
{
	struct sunxi_twi *twi = (struct sunxi_twi *)adap->algo_data;
	if (!twi->twi_dvfs_enable) {
		sunxi_err(twi->dev, "dvfs function must be used in drv and no_suspend mode\n");
		return -EPERM;
	}
	if (ch >= twi->data->dvfs_valid_chan_nums) {
		sunxi_err(twi->dev, "target chan %d exceed valid chan nums %d\n", ch, twi->data->dvfs_valid_chan_nums);
		return -ENOMEM;
	}
	if (twi->dvfs_devices->dvfs_channel[ch].channel_en) {
		sunxi_err(twi->dev, "dvfs can not set device reg addr because %d channel is enabled\n", ch);
		return -EBUSY;
	}

	sunxi_twi_dvfs_set_dev_reg(twi->base_addr, ch, reg);
	twi->dvfs_devices->dvfs_channel[ch].reg_addr = reg;
	return 0;
}
EXPORT_SYMBOL_GPL(sunxi_twi_dvfs_set_chan_reg);

int sunxi_twi_dvfs_set_chan_mask(struct i2c_adapter *adap, int ch, u8 mask)
{
	struct sunxi_twi *twi = (struct sunxi_twi *)adap->algo_data;
	if (!twi->twi_dvfs_enable) {
		sunxi_err(twi->dev, "dvfs function must be used in drv and no_suspend mode\n");
		return -EPERM;
	}
	if (ch >= twi->data->dvfs_valid_chan_nums) {
		sunxi_err(twi->dev, "target chan %d exceed valid chan nums %d\n", ch, twi->data->dvfs_valid_chan_nums);
		return -ENOMEM;
	}
	if (twi->dvfs_devices->dvfs_channel[ch].channel_en) {
		sunxi_err(twi->dev, "dvfs can not set mask reg addr because %d channel is enabled\n", ch);
		return -EBUSY;
	}

	sunxi_twi_dvfs_set_dev_mask(twi->base_addr, ch, mask);
	twi->dvfs_devices->dvfs_channel[ch].mask = mask;
	return 0;
}
EXPORT_SYMBOL_GPL(sunxi_twi_dvfs_set_chan_mask);

int sunxi_twi_dvfs_set_chan_rb(struct i2c_adapter *adap, int ch, bool rb)
{
	struct sunxi_twi *twi = (struct sunxi_twi *)adap->algo_data;
	if (!twi->twi_dvfs_enable) {
		sunxi_err(twi->dev, "dvfs function must be used in drv and no_suspend mode\n");
		return -EPERM;
	}
	if (ch >= twi->data->dvfs_valid_chan_nums) {
		sunxi_err(twi->dev, "target chan %d exceed valid chan nums %d\n", ch, twi->data->dvfs_valid_chan_nums);
		return -ENOMEM;
	}
	if (twi->dvfs_devices->dvfs_channel[ch].channel_en) {
		sunxi_err(twi->dev, "dvfs can not set readback because %d channel is enabled\n", ch);
		return -EBUSY;
	}

	twi->dvfs_devices->dvfs_channel[ch].readback_en = rb;
	return 0;
}
EXPORT_SYMBOL_GPL(sunxi_twi_dvfs_set_chan_rb);

int sunxi_twi_dvfs_set_chan_prio(struct i2c_adapter *adap, int ch, u8 priority)
{
	struct sunxi_twi *twi = (struct sunxi_twi *)adap->algo_data;
	if (!twi->twi_dvfs_enable) {
		sunxi_err(twi->dev, "dvfs function must be used in drv and no_suspend mode\n");
		return -EPERM;
	}
	if (ch >= twi->data->dvfs_valid_chan_nums) {
		sunxi_err(twi->dev, "target chan %d exceed valid chan nums %d\n", ch, twi->data->dvfs_valid_chan_nums);
		return -ENOMEM;
	}
	if (twi->dvfs_devices->dvfs_channel[ch].channel_en) {
		sunxi_err(twi->dev, "dvfs can not set priority because %d channel is enabled\n", ch);
		return -EBUSY;
	}

	sunxi_twi_dvfs_set_dev_arbiter(twi->base_addr, ch, priority);
	twi->dvfs_devices->dvfs_channel[ch].priority = priority;
	return 0;
}
EXPORT_SYMBOL_GPL(sunxi_twi_dvfs_set_chan_prio);

int sunxi_twi_dvfs_chan_init(struct i2c_adapter *adap, int ch, u8 priority, u8 reg, u8 mask, bool rb)
{
	struct sunxi_twi *twi = (struct sunxi_twi *)adap->algo_data;
	if (!twi->twi_dvfs_enable) {
		sunxi_err(twi->dev, "dvfs function must be used in drv and no_suspend mode\n");
		return -EPERM;
	}
	if (ch >= twi->data->dvfs_valid_chan_nums) {
		sunxi_err(twi->dev, "target chan %d exceed valid chan nums %d\n", ch, twi->data->dvfs_valid_chan_nums);
		return -ENOMEM;
	}
	if (twi->dvfs_devices->dvfs_channel[ch].channel_en) {
		sunxi_err(twi->dev, "dvfs chan cannot init because %d channel is enabled\n", ch);
		return -EBUSY;
	}

	sunxi_twi_dvfs_set_chan_reg(adap, ch, reg);
	sunxi_twi_dvfs_set_chan_rb(adap, ch, rb);
	sunxi_twi_dvfs_set_chan_mask(adap, ch, mask);
	sunxi_twi_dvfs_set_chan_prio(adap, ch, priority);
	return 0;
}
EXPORT_SYMBOL_GPL(sunxi_twi_dvfs_chan_init);

int sunxi_twi_dvfs_core_process(struct sunxi_twi *twi)
{
	u32 irq_status = sunxi_twi_dvfs_get_irq_status(twi->base_addr);
	u32 dev_status;
	int i;

	sunxi_debug(twi->dev, "dvfs-mode: [slave address:(0x%x),irq state:(0x%x)]\n", twi->dvfs_devices->addr, irq_status);
	sunxi_twi_dvfs_clr_irq_status(twi->base_addr, irq_status);

	for (i = 0; i < twi->data->dvfs_valid_chan_nums; i++) {
		if (!twi->dvfs_devices->dvfs_channel[i].channel_en)
			continue;

		if (irq_status & TWI_DVFS_IRQ_TRAN_OK(i)) {
			/* Packet transmit complete */
			sunxi_debug(twi->dev, "dvfs channel %d xfer completed\n", i);
		} else if (twi->dvfs_devices->dvfs_channel[i].readback_en && (irq_status & TWI_DVFS_IRQ_RB_ERR(i))) {
			/* Packet receive failed */
			sunxi_err(twi->dev, "dvfs channel %d readback packet failed\n", i);
			goto err;
		} else if (irq_status & TWI_DVFS_IRQ_TRAN_ERR(i)) {
			/* Packet transmit failed */
			sunxi_err(twi->dev, "dvfs channel %d xfer packet failed\n", i);
			goto err;
		} else {
			sunxi_err(twi->dev, "dvfs channel %d unknown irq status 0x%x\n", i, irq_status);
			goto err;
		}
	}

	return 0;

err:
	dev_status = sunxi_twi_dvfs_get_dev_status(twi->base_addr, i);
	sunxi_err(twi->dev, "dvfs device %d status : "
		"rb(0x%02lx) sid(0x%02lx) reg(0x%02lx) data(0x%02lx)\n", i,
		FIELD_GET(TWI_DVFS_DEV_STAT_RB_DATA, dev_status),
		FIELD_GET(TWI_DVFS_DEV_STAT_SID_PMU, dev_status),
		FIELD_GET(TWI_DVFS_DEV_STAT_REG_ADDR, dev_status),
		FIELD_GET(TWI_DVFS_DEV_STAT_SEND_DATA, dev_status)
		);
	sunxi_twi_soft_reset(twi);
	return -1;
}
