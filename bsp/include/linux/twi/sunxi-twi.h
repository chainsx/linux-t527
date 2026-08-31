/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * SUNXI TWI Header Definition
 *
 */

#ifndef __LINUX_SUNXI_TWI_H
#define __LINUX_SUNXI_TWI_H

#include <linux/ctype.h>
#include <linux/i2c.h>

#if IS_ENABLED(CONFIG_AW_TWI_DVFS)
extern int sunxi_twi_dvfs_enable_all(struct i2c_adapter *adap);
extern int sunxi_twi_dvfs_disable_all(struct i2c_adapter *adap);
extern int sunxi_twi_dvfs_enable(struct i2c_adapter *adap, int ch);
extern int sunxi_twi_dvfs_disable(struct i2c_adapter *adap, int ch);
extern int sunxi_twi_dvfs_set_slave_addr(struct i2c_adapter *adap, u8 addr);
extern int sunxi_twi_dvfs_set_interval(struct i2c_adapter *adap, u16 interval);
extern int sunxi_twi_dvfs_chan_init(struct i2c_adapter *adap, int ch, u8 priority, u8 reg, u8 mask, bool rb);
extern int sunxi_twi_dvfs_set_chan_reg(struct i2c_adapter *adap, int ch, u8 reg);
extern int sunxi_twi_dvfs_set_chan_mask(struct i2c_adapter *adap, int ch, u8 mask);
extern int sunxi_twi_dvfs_set_chan_rb(struct i2c_adapter *adap, int ch, bool rb);
extern int sunxi_twi_dvfs_set_chan_prio(struct i2c_adapter *adap, int ch, u8 priority);
#else
static inline int sunxi_twi_dvfs_enable_all(struct i2c_adapter *adap)
{
	return -EINVAL;
}
static inline int sunxi_twi_dvfs_disable_all(struct i2c_adapter *adap)
{
	return -EINVAL;
}
static inline int sunxi_twi_dvfs_enable(struct i2c_adapter *adap, int ch)
{
	return -EINVAL;
}
static inline int sunxi_twi_dvfs_disable(struct i2c_adapter *adap, int ch)
{
	return -EINVAL;
}
static inline int sunxi_twi_dvfs_set_slave_addr(struct i2c_adapter *adap, u8 addr)
{
	return -EINVAL;
}
static inline int sunxi_twi_dvfs_set_interval(struct i2c_adapter *adap, u16 interval)
{
	return -EINVAL;
}
static inline int sunxi_twi_dvfs_chan_init(struct i2c_adapter *adap, int ch, u8 priority, u8 reg, u8 mask, bool rb)
{
	return -EINVAL;
}
static inline int sunxi_twi_dvfs_set_chan_reg(struct i2c_adapter *adap, int ch, u8 reg)
{
	return -EINVAL;
}
static inline int sunxi_twi_dvfs_set_chan_mask(struct i2c_adapter *adap, int ch, u8 mask)
{
	return -EINVAL;
}
static inline int sunxi_twi_dvfs_set_chan_rb(struct i2c_adapter *adap, int ch, bool rb)
{
	return -EINVAL;
}
static inline int sunxi_twi_dvfs_set_chan_prio(struct i2c_adapter *adap, int ch, u8 priority)
{
	return -EINVAL;
}
#endif


#endif /* __LINUX_SUNXI_TWI_H */