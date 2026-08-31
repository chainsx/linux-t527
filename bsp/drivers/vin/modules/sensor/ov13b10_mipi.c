/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * A V4L2 driver for ov13b10 Raw cameras.
 *
 * Copyright (c) 2017 by Allwinnertech Co., Ltd.  http:
 *
 * Authors:  Liu Chensheng <liuchensheng@allwinnertech.com>
 *     Liang WeiJie <liangweijie@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include "../../utility/vin_log.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/delay.h>
#include <linux/videodev2.h>
#include <linux/clk.h>
#include <media/v4l2-device.h>
#include <media/v4l2-mediabus.h>
#include <linux/io.h>
#include "camera.h"
#include "sensor_helper.h"

MODULE_AUTHOR("lwj");
MODULE_DESCRIPTION("A low-level driver for ov13b10 R2A sensors");
MODULE_LICENSE("GPL");

#define MCLK              (24*1000*1000)
int ov13b10_sensor_vts;

/*
 * Our nominal (default) frame rate.
 */

#define SENSOR_FRAME_RATE 30

/*
 * The ov13b10 sits on i2c with ID 0x6c/0x20
 */
#define I2C_ADDR 0x6c
#define SENSOR_NAME "ov13b10_mipi"

/*
 * The default register settings
 */

static struct regval_list sensor_default_regs[] = {
	{0x0103, 0x01},	// PLL
	{0x0303, 0x01},
	{0x0305, 0x46},
	{0x0321, 0x00},
	{0x0323, 0x04},
	{0x0324, 0x01},
	{0x0325, 0x50},
	{0x0326, 0x81},
	{0x0327, 0x04},
	{0x3012, 0x07},	// system control
	{0x3013, 0x32},
	{0x3107, 0x23},	// SCCB control
	{0x3501, 0x0c},	// AEC
	{0x3502, 0x10},
	{0x3504, 0x08},
	{0x3508, 0x07},
	{0x3509, 0xc0},
	{0x3600, 0x16},	// Analog control
	{0x3601, 0x54},
	{0x3612, 0x4e},
	{0x3620, 0x00},
	{0x3621, 0x68},
	{0x3622, 0x66},
	{0x3623, 0x03},
	{0x3662, 0x92},
	{0x3666, 0xbb},
	{0x3667, 0x44},
	{0x366e, 0xff},
	{0x366f, 0xf3},
	{0x3675, 0x44},
	{0x3676, 0x00},
	{0x367f, 0xe9},
	{0x3681, 0x32},
	{0x3682, 0x1f},
	{0x3683, 0x0b},
	{0x3684, 0x0b},
	{0x3704, 0x0f},	// Sensor top control
	{0x3706, 0x40},
	{0x3708, 0x3b},
	{0x3709, 0x72},
	{0x370b, 0xa2},
	{0x3714, 0x24},
	{0x371a, 0x3e},
	{0x3725, 0x42},
	{0x3739, 0x12},
	{0x3767, 0x00},
	{0x377a, 0x0d},
	{0x3789, 0x18},
	{0x3790, 0x40},
	{0x3791, 0xa2},
	{0x37c2, 0x04},
	{0x37c3, 0xf1},
	{0x37d9, 0x0c},
	{0x37da, 0x02},
	{0x37dc, 0x02},
	{0x37e1, 0x04},
	{0x37e2, 0x0a},
	{0x3800, 0x00},	// X start
	{0x3801, 0x00},
	{0x3802, 0x00},	// Y start
	{0x3803, 0x08},
	{0x3804, 0x10},	// X end 0x108f 4239
	{0x3805, 0x8f},
	{0x3806, 0x0c},	// Y end 0xc47  3143
	{0x3807, 0x47},
	{0x3808, 0x10},	// X output size 4208
	{0x3809, 0x70},
	{0x380a, 0x0c},	// Y output size 3120
	{0x380b, 0x30},
	{0x380c, 0x04},	// HTS 0x0498 1176
	{0x380d, 0x98},
	{0x380e, 0x0c},	// VTS 0x0c7c 3139
	{0x380f, 0x7c},
	{0x3811, 0x0f},	// ISP X win
	{0x3813, 0x08},	// ISP Y win
	{0x3814, 0x01},	// X Inc odd
	{0x3815, 0x01},	// X Inc even
	{0x3816, 0x01},	// Y Inc odd
	{0x3817, 0x01},	// Y Inc even
	{0x381f, 0x08},	// VTS EXP diff
	{0x3820, 0x88},	// 0x88  fromat
	{0x3821, 0x00},
	{0x3822, 0x14},	// VTS auto dis
	{0x382e, 0xe6},	// Expo offset
	{0x3c80, 0x00},
	{0x3c87, 0x01},
	{0x3c8c, 0x19},
	{0x3c8d, 0x1c},
	{0x3ca0, 0x00},
	{0x3ca1, 0x00},
	{0x3ca2, 0x00},
	{0x3ca3, 0x00},
	{0x3ca4, 0x50},
	{0x3ca5, 0x11},
	{0x3ca6, 0x01},
	{0x3ca7, 0x00},
	{0x3ca8, 0x00},
	{0x4008, 0x02},	// BLC control
	{0x4009, 0x0f},
	{0x400a, 0x01},
	{0x400b, 0x19},
	{0x4011, 0x21},
	{0x4017, 0x08},
	{0x4019, 0x04},
	{0x401a, 0x58},
	{0x4032, 0x1e},
	{0x4050, 0x02},
	{0x4051, 0x09},
	{0x405e, 0x00},
	{0x4066, 0x02},
	{0x4501, 0x00},	// colum ADC sync and SYNC_FIFO
	{0x4502, 0x10},
	{0x4505, 0x00},
	{0x4800, 0x64},	// MIPI
	{0x481b, 0x3e},
	{0x481f, 0x30},
	{0x4825, 0x34},
	{0x4837, 0x0e},
	{0x484b, 0x01},
	{0x4883, 0x02},
	{0x5000, 0xfd},	// DSP
	{0x5001, 0x0d},
	{0x5045, 0x20},
	{0x5046, 0x20},
	{0x5047, 0xa4},
	{0x5048, 0x20},
	{0x5049, 0xa4},
};

static struct regval_list sensor_4160x3120_18fps_regs[] = {
	{0x0305, 0x46},
	{0x3501, 0x0c},
	{0x3502, 0x10},
	{0x3662, 0x92},
	{0x3714, 0x24},
	{0x371a, 0x3e},
	{0x3739, 0x12},
	{0x37c2, 0x04},
	{0x37d9, 0x0c},
	{0x3800, 0x00},
	{0x3801, 0x00},
	{0x3802, 0x00},
	{0x3803, 0x08},
	{0x3804, 0x10},
	{0x3805, 0x8f},
	{0x3806, 0x0c},
	{0x3807, 0x47},
	{0x3808, 0x10},
	{0x3809, 0x70},
	{0x380a, 0x0c},
	{0x380b, 0x30},
	{0x380c, 0x04},
	{0x380d, 0x98},
	{0x380e, 0x14},	// VTS 0x14ab 5291
	{0x380f, 0xab},
	{0x3811, 0x0f},
	{0x3813, 0x08},
	{0x3814, 0x01},
	{0x3815, 0x01},
	{0x3816, 0x01},
	{0x3817, 0x01},
	{0x3820, 0x88},
	{0x3821, 0x00},
	{0x3c38, 0x19},
	{0x4008, 0x02},
	{0x4009, 0x0f},
	{0x4050, 0x02},
	{0x4051, 0x09},
	{0x4501, 0x00},
	{0x4505, 0x00},
	{0x4837, 0x0e},
	{0x5000, 0xfd},
	{0x5001, 0x0d},
};

static struct regval_list sensor_4160x3120_30fps_regs[] = {
	{0x0305, 0x46},
	{0x3501, 0x0c},
	{0x3502, 0x10},
	{0x3662, 0x92},
	{0x3714, 0x24},
	{0x371a, 0x3e},
	{0x3739, 0x12},
	{0x37c2, 0x04},
	{0x37d9, 0x0c},
	{0x3800, 0x00},
	{0x3801, 0x00},
	{0x3802, 0x00},
	{0x3803, 0x08},
	{0x3804, 0x10},
	{0x3805, 0x8f},
	{0x3806, 0x0c},
	{0x3807, 0x47},
	{0x3808, 0x10},
	{0x3809, 0x70},
	{0x380a, 0x0c},
	{0x380b, 0x30},
	{0x380c, 0x04},
	{0x380d, 0x98},
	{0x380e, 0x0c},	// VTS 0x0c7c 3139
	{0x380f, 0x7c},
	{0x3811, 0x0f},
	{0x3813, 0x08},
	{0x3814, 0x01},
	{0x3815, 0x01},
	{0x3816, 0x01},
	{0x3817, 0x01},
	{0x3820, 0x88},
	{0x3821, 0x00},
	{0x3c38, 0x19},
	{0x4008, 0x02},
	{0x4009, 0x0f},
	{0x4050, 0x02},
	{0x4051, 0x09},
	{0x4501, 0x00},
	{0x4505, 0x00},
	{0x4837, 0x0e},
	{0x5000, 0xfd},
	{0x5001, 0x0d},
};

static struct regval_list sensor_2104x1560_30fps_regs[] = {
	{0x0305, 0x23},
	{0x3501, 0x06},
	{0x3502, 0x10},
	{0x3662, 0x88},
	{0x3714, 0x28},
	{0x371a, 0x3e},
	{0x3739, 0x10},
	{0x37c2, 0x14},
	{0x37d9, 0x06},
	{0x3800, 0x00},
	{0x3801, 0x00},
	{0x3802, 0x00},
	{0x3803, 0x08},
	{0x3804, 0x10},
	{0x3805, 0x8f},
	{0x3806, 0x0c},
	{0x3807, 0x47},
	{0x3808, 0x08},
	{0x3809, 0x38},
	{0x380a, 0x06},
	{0x380b, 0x18},
	{0x380c, 0x04},
	{0x380d, 0x98},
	{0x380e, 0x0c},	// VTS 0x0c7c 3196
	{0x380f, 0x7c},
	{0x3811, 0x07},
	{0x3813, 0x04},
	{0x3814, 0x03},
	{0x3815, 0x01},
	{0x3816, 0x03},
	{0x3817, 0x01},
	{0x3820, 0x88},
	{0x3821, 0x00},
	{0x3c38, 0x18},
	{0x4008, 0x00},
	{0x4009, 0x05},
	{0x4050, 0x00},
	{0x4051, 0x05},
	{0x4501, 0x08},
	{0x4505, 0x04},
	{0x4837, 0x1d},
	{0x5000, 0xfd},
	{0x5001, 0x0d},
};

static struct regval_list sensor_2104x1560_60fps_regs[] = {
// 59.6fps, line time 10.5us, 560Mbps/lane
	{0x0305, 0x23},
	{0x3501, 0x06},
	{0x3502, 0x10},
	{0x3662, 0x88},
	{0x3714, 0x28},
	{0x371a, 0x3e},
	{0x3739, 0x10},
	{0x37c2, 0x14},
	{0x37d9, 0x06},
	{0x3800, 0x00},
	{0x3801, 0x00},
	{0x3802, 0x00},
	{0x3803, 0x08},
	{0x3804, 0x10},
	{0x3805, 0x8f},
	{0x3806, 0x0c},
	{0x3807, 0x47},
	{0x3808, 0x08},
	{0x3809, 0x38},
	{0x380a, 0x06},
	{0x380b, 0x18},
	{0x380c, 0x04},
	{0x380d, 0x98},
	{0x380e, 0x06},	// VTS 0x063e 1598
	{0x380f, 0x3e},
	{0x3811, 0x07},
	{0x3813, 0x04},
	{0x3814, 0x03},
	{0x3815, 0x01},
	{0x3816, 0x03},
	{0x3817, 0x01},
	{0x3820, 0x88},
	{0x3821, 0x00},
	{0x3c38, 0x18},
	{0x4008, 0x00},
	{0x4009, 0x05},
	{0x4050, 0x00},
	{0x4051, 0x05},
	{0x4501, 0x08},
	{0x4505, 0x04},
	{0x4837, 0x1d},
	{0x5000, 0xfd},
	{0x5001, 0x0d},
};

#if VIN_FALSE
static struct regval_list sensor_1280x720_30fps_regs[] = {
	{0x0305, 0x23},
	{0x3501, 0x03},
	{0x3502, 0x00},
	{0x3662, 0x88},
	{0x3714, 0x28},
	{0x371a, 0x3e},
	{0x3739, 0x10},
	{0x37c2, 0x14},
	{0x37d9, 0x06},
	{0x3800, 0x03},
	{0x3801, 0x30},
	{0x3802, 0x03},
	{0x3803, 0x48},
	{0x3804, 0x0d},
	{0x3805, 0x5f},
	{0x3806, 0x09},
	{0x3807, 0x07},
	{0x3808, 0x05},
	{0x3809, 0x00},
	{0x380a, 0x02},
	{0x380b, 0xd0},
	{0x380c, 0x04},
	{0x380d, 0x98},
	{0x380e, 0x0c},	// VTS 0x0c78 3192
	{0x380f, 0x78},
	{0x3811, 0x0b},
	{0x3813, 0x08},
	{0x3814, 0x03},
	{0x3815, 0x01},
	{0x3816, 0x03},
	{0x3817, 0x01},
	{0x3820, 0x88},
	{0x3821, 0x00},
	{0x3c38, 0x18},
	{0x4008, 0x00},
	{0x4009, 0x05},
	{0x4050, 0x00},
	{0x4051, 0x05},
	{0x4501, 0x08},
	{0x4505, 0x04},
	{0x4837, 0x1d},
	{0x5000, 0xfd},
	{0x5001, 0x0d},
};
#endif

static struct regval_list sensor_1280x720_60fps_regs[] = {
	{0x0305, 0x23},
	{0x3501, 0x03},
	{0x3502, 0x00},
	{0x3662, 0x88},
	{0x3714, 0x28},
	{0x371a, 0x3e},
	{0x3739, 0x10},
	{0x37c2, 0x14},
	{0x37d9, 0x06},
	{0x3800, 0x03},
	{0x3801, 0x30},
	{0x3802, 0x03},
	{0x3803, 0x48},
	{0x3804, 0x0d},
	{0x3805, 0x5f},
	{0x3806, 0x09},
	{0x3807, 0x07},
	{0x3808, 0x05},
	{0x3809, 0x00},
	{0x380a, 0x02},
	{0x380b, 0xd0},
	{0x380c, 0x04},
	{0x380d, 0x98},
	{0x380e, 0x06},	// VTS 0x063c 1596
	{0x380f, 0x3c},
	{0x3811, 0x0b},
	{0x3813, 0x08},
	{0x3814, 0x03},
	{0x3815, 0x01},
	{0x3816, 0x03},
	{0x3817, 0x01},
	{0x3820, 0x88},
	{0x3821, 0x00},
	{0x3c38, 0x18},
	{0x4008, 0x00},
	{0x4009, 0x05},
	{0x4050, 0x00},
	{0x4051, 0x05},
	{0x4501, 0x08},
	{0x4505, 0x04},
	{0x4837, 0x1d},
	{0x5000, 0xfd},
	{0x5001, 0x0d},
};

static struct regval_list sensor_1280x720_120fps_regs[] = {
// 119.35fps,line time 10.5us, 560Mbps/lane
	{0x0305, 0x23},
	{0x3501, 0x03},
	{0x3502, 0x00},
	{0x3662, 0x88},
	{0x3714, 0x28},
	{0x371a, 0x3e},
	{0x3739, 0x10},
	{0x37c2, 0x14},
	{0x37d9, 0x06},
	{0x3800, 0x03},
	{0x3801, 0x30},
	{0x3802, 0x03},
	{0x3803, 0x48},
	{0x3804, 0x0d},
	{0x3805, 0x5f},
	{0x3806, 0x09},
	{0x3807, 0x07},
	{0x3808, 0x05},
	{0x3809, 0x00},
	{0x380a, 0x02},
	{0x380b, 0xd0},
	{0x380c, 0x04},
	{0x380d, 0x98},
	{0x380e, 0x03},	// VTS 0x031e 798
	{0x380f, 0x1e},
	{0x3811, 0x0b},
	{0x3813, 0x08},
	{0x3814, 0x03},
	{0x3815, 0x01},
	{0x3816, 0x03},
	{0x3817, 0x01},
	{0x3820, 0x88},
	{0x3821, 0x00},
	{0x3c38, 0x18},
	{0x4008, 0x00},
	{0x4009, 0x05},
	{0x4050, 0x00},
	{0x4051, 0x05},
	{0x4501, 0x08},
	{0x4505, 0x04},
	{0x4837, 0x1d},
	{0x5000, 0xfd},
	{0x5001, 0x0d},
};

/*
 * Here we'll try to encapsulate the changes for just the output
 * video format.
 *
 */

static struct regval_list sensor_fmt_raw[] = {

};

static int sensor_g_exp(struct v4l2_subdev *sd, __s32 *value)
{
	struct sensor_info *info = to_state(sd);

	*value = info->exp;
	sensor_dbg("sensor_get_exposure = %d\n", info->exp);
	return 0;
}

static int sensor_s_exp(struct v4l2_subdev *sd, unsigned int exp_val)
{
	unsigned char explow = 0, expmid = 0, exphigh = 0;

	exp_val >>= 4;

	if (exp_val < 4)
		exp_val = 4;
	if (exp_val > 0xffffff)
		exp_val = 0xffffff;

	exphigh = (unsigned char)((exp_val >> 16) & 0xff);
	expmid = (unsigned char)((exp_val >> 8) & 0xff);
	explow = (unsigned char)(exp_val & 0xff);

	sensor_write(sd, 0x3500, exphigh);
	sensor_write(sd, 0x3501, expmid);
	sensor_write(sd, 0x3502, explow);

	return 0;
}

static int sensor_g_gain(struct v4l2_subdev *sd, __s32 *value)
{
	struct sensor_info *info = to_state(sd);

	*value = info->gain;
	sensor_dbg("sensor_get_gain = %d\n", info->gain);
	return 0;
}

/* The OV13B10 support up to 15.5x(15x in this case) analog gain and 4x digital gain
 * analog gain: set 256 as 1x
 * digital gain: set 1024 as 1x
*/
static int sensor_s_gain(struct v4l2_subdev *sd, int gain_val)
{
	unsigned int Again = 0, Dgain = 0;
	unsigned char Againhigh = 0, Againlow = 0;
	unsigned char Dgainhigh = 0, Dgainmid = 0, Dgainlow = 0;

	if (gain_val < 16)
		gain_val = 16;
	if (gain_val > 16 * 15 * 4)
		gain_val = 16 * 15 * 4;

	if (gain_val <= 16 * 15) {
		/* Tgain = Again*Dgain Dgain = 1 */
		Again = gain_val << 4; // Again: '16 as 1x' convert to '256 as 1x'
		Againhigh = (unsigned char)((Again & 0x0f00) >> 8);
		Againlow = (unsigned char)(Again & 0x00fe);
		Dgainhigh = 0x01;
		Dgainmid = 0x00;
		Dgainlow = 0x00;
	} else {
		/* Tgain = Again*Dgain  Again = 15 */
		Againhigh = 0x0f;
		Againlow = 0x00;
		Dgain = (gain_val << 6) / 15; // Dgain: '16 as 1x' convert to '1024 as 1x'
		Dgainhigh = (Dgain & 0xc00) >> 10;
		Dgainmid = (Dgain & 0x3fc) >> 2;
		Dgainlow = (Dgain & 0x003) << 6;
	}

	sensor_write(sd, 0x3508, Againhigh);
	sensor_write(sd, 0x3509, Againlow);
	sensor_write(sd, 0x350a, Dgainhigh);
	sensor_write(sd, 0x350b, Dgainmid);
	sensor_write(sd, 0x350c, Dgainlow);

	sensor_dbg("sensor_set_gain = %d\n", gain_val);

	return 0;
}

static int frame_length = 3808;
static int sensor_s_exp_gain(struct v4l2_subdev *sd,
			     struct sensor_exp_gain *exp_gain)
{
	int exp_val, gain_val, shutter;
	struct sensor_info *info = to_state(sd);

	exp_val = exp_gain->exp_val;
	gain_val = exp_gain->gain_val;

	shutter = exp_val >> 4;
	if (shutter > ov13b10_sensor_vts - 8)
		frame_length = shutter + 8;
	else
		frame_length = ov13b10_sensor_vts;

	sensor_write(sd, 0x3208, 0x00);/* enter group write */
	sensor_write(sd, 0x380f, frame_length & 0xff);
	sensor_write(sd, 0x380e, frame_length >> 8);
	//if (shutter > ov13b10_sensor_vts - 16)
	//	exp_val = ov13b10_sensor_vts - 16;
	sensor_s_exp(sd, exp_val);
	sensor_s_gain(sd, gain_val);
	sensor_write(sd, 0x3208, 0x10);/* end group write */
	sensor_write(sd, 0x3208, 0xa0);/* init group write */
	info->exp = exp_val;
	info->gain = gain_val;
	return 0;
}

static int sensor_s_sw_stby(struct v4l2_subdev *sd, int on_off)
{
	int ret;
	data_type rdval;

	ret = sensor_read(sd, 0x0100, &rdval);
	if (ret != 0)
		return ret;

	if (on_off == CSI_GPIO_LOW)
		ret = sensor_write(sd, 0x0100, rdval & 0xfe);
	else
		ret = sensor_write(sd, 0x0100, rdval | 0x01);

	return ret;
}

/*
 * Stuff that knows about the sensor.
 */

static int sensor_power(struct v4l2_subdev *sd, int on)
{
	int ret = 0;

	switch (on) {
	case STBY_ON:
		sensor_dbg("STBY_ON!\n");
		ret = sensor_s_sw_stby(sd, CSI_GPIO_LOW);
		if (ret < 0)
			sensor_err("soft stby falied!\n");
		usleep_range(10000, 12000);
		cci_lock(sd);
		vin_gpio_write(sd, PWDN, CSI_GPIO_LOW);
		cci_unlock(sd);
		vin_set_mclk(sd, OFF);
		break;
	case STBY_OFF:
		sensor_dbg("STBY_OFF!\n");
		cci_lock(sd);
		vin_set_mclk_freq(sd, MCLK);
		vin_set_mclk(sd, ON);
		usleep_range(10000, 12000);
		vin_gpio_write(sd, PWDN, CSI_GPIO_HIGH);
		usleep_range(10000, 12000);
		ret = sensor_s_sw_stby(sd, CSI_GPIO_HIGH);
		if (ret < 0)
			sensor_err("soft stby off falied!\n");
		cci_unlock(sd);
		break;
	case PWR_ON:
		sensor_dbg("PWR_ON!\n");
		cci_lock(sd);
		vin_gpio_set_status(sd, PWDN, 1);
		vin_gpio_set_status(sd, RESET, 1);
		vin_gpio_write(sd, PWDN, CSI_GPIO_LOW);
		vin_gpio_write(sd, RESET, CSI_GPIO_LOW);
		vin_set_pmu_channel(sd, AFVDD, ON);
		vin_set_pmu_channel(sd, IOVDD, ON);
		//usleep_range(5000, 6000);
		//vin_gpio_write(sd, PWDN, CSI_GPIO_HIGH);
		vin_set_pmu_channel(sd, CAMERAVDD, ON);
		vin_set_pmu_channel(sd, AVDD, ON);
		//usleep_range(5000, 6000);
		vin_set_pmu_channel(sd, DVDD, ON);
		usleep_range(5000, 6000);
		vin_gpio_write(sd, RESET, CSI_GPIO_HIGH);
		vin_gpio_write(sd, PWDN, CSI_GPIO_HIGH);
		//usleep_range(5000, 6000);
		vin_set_mclk_freq(sd, MCLK);
		vin_set_mclk(sd, ON);
		usleep_range(5000, 6000);
		cci_unlock(sd);
		break;
	case PWR_OFF:
		sensor_dbg("PWR_OFF!\n");
		cci_lock(sd);
		vin_set_mclk(sd, OFF);
		usleep_range(500, 600);
		vin_gpio_write(sd, PWDN, CSI_GPIO_LOW);
		vin_gpio_write(sd, RESET, CSI_GPIO_LOW);
		usleep_range(5000, 6000);
		vin_set_pmu_channel(sd, DVDD, OFF);
		//usleep_range(5000, 6000);
		vin_set_pmu_channel(sd, AVDD, OFF);
		vin_set_pmu_channel(sd, CAMERAVDD, OFF);
		//usleep_range(5000, 6000);
		vin_set_pmu_channel(sd, IOVDD, OFF);
		vin_set_pmu_channel(sd, AFVDD, OFF);
		vin_gpio_write(sd, PWDN, CSI_GPIO_LOW);
		//usleep_range(5000, 6000);
		//vin_gpio_set_status(sd, RESET, 0);
		//vin_gpio_set_status(sd, PWDN, 0);
		cci_unlock(sd);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int sensor_reset(struct v4l2_subdev *sd, u32 val)
{
	switch (val) {
	case 0:
		vin_gpio_write(sd, RESET, CSI_GPIO_HIGH);
		usleep_range(100, 120);
		break;
	case 1:
		vin_gpio_write(sd, RESET, CSI_GPIO_LOW);
		usleep_range(100, 120);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int sensor_detect(struct v4l2_subdev *sd)
{
	data_type rdval;

	sensor_read(sd, 0x300b, &rdval);
	if (rdval != 0x0d) {
		sensor_err("read 0x300b return value 0x%x\n", rdval);
		return -ENODEV;
	}

	sensor_read(sd, 0x300c, &rdval);
	if (rdval != 0x42) {
		sensor_err("read 0x300c return value 0x%x\n", rdval);
		return -ENODEV;
	}

	return 0;
}

static int sensor_init(struct v4l2_subdev *sd, u32 val)
{
	int ret;
	struct sensor_info *info = to_state(sd);

	sensor_dbg("sensor_init\n");

	/*Make sure it is a target sensor */
	ret = sensor_detect(sd);
	if (ret) {
		sensor_err("chip found is not an target chip.\n");
		return ret;
	}

	info->focus_status = 0;
	info->low_speed = 0;
	info->width = 4224;
	info->height = 3136;
	info->hflip = 0;
	info->vflip = 0;

	info->tpf.numerator = 1;
	info->tpf.denominator = 18;	/* 18fps */

	info->preview_first_flag = 1;

	return 0;
}

static long sensor_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	int ret = 0;
	struct sensor_info *info = to_state(sd);

	switch (cmd) {
	case GET_CURRENT_WIN_CFG:
		if (info->current_wins != NULL) {
			memcpy(arg, info->current_wins,
			       sizeof(struct sensor_win_size));
			ret = 0;
		} else {
			sensor_err("empty wins!\n");
			ret = -1;
		}
		break;
	case SET_FPS:
		ret = 0;
		break;
	case VIDIOC_VIN_SENSOR_EXP_GAIN:
		ret = sensor_s_exp_gain(sd, (struct sensor_exp_gain *)arg);
		break;
	case VIDIOC_VIN_SENSOR_CFG_REQ:
		sensor_cfg_req(sd, (struct sensor_config *)arg);
		break;
	case VIDIOC_VIN_ACT_INIT:
		ret = actuator_init(sd, (struct actuator_para *)arg);
		break;
	case VIDIOC_VIN_ACT_SET_CODE:
		ret = actuator_set_code(sd, (struct actuator_ctrl *)arg);
		break;
	case VIDIOC_VIN_FLASH_EN:
		ret = flash_en(sd, (struct flash_para *)arg);
		break;
	default:
		return -EINVAL;
	}
	return ret;
}

/*
 * Store information about the video data format.
 */
static struct sensor_format_struct sensor_formats[] = {
	{
		.desc = "Raw RGB Bayer",
		.mbus_code = MEDIA_BUS_FMT_SBGGR10_1X10,
		.regs = sensor_fmt_raw,
		.regs_size = ARRAY_SIZE(sensor_fmt_raw),
		.bpp = 1
	},
};
#define N_FMTS ARRAY_SIZE(sensor_formats)

/*
 * Then there is the issue of window sizes.  Try to capture the info here.
 */

static struct sensor_win_size sensor_win_sizes[] = {
{
	 .width = 4160,
	 .height = 3120,
	 .hoffset = 0,
	 .voffset = 0,
	 .hts = 1176,
	 .vts = 5291,
	 .pclk = 112 * 1000 * 1000,	// 240 * 1000 * 1000,
	 .mipi_bps = 1120 * 1000 * 1000,	// 1800
	 .fps_fixed = 18,
	 .bin_factor = 1,
	 .intg_min = 1 << 4,
	 .intg_max = (3328 - 4) << 4,
	 .gain_min = 1 << 4,
	 .gain_max = 15 << 4,
	 .regs = sensor_4160x3120_18fps_regs,
	 .regs_size = ARRAY_SIZE(sensor_4160x3120_18fps_regs),
	 .set_size = NULL,
	},

	{
	 .width = 4160,
	 .height = 3120,
	 .hoffset = 0,
	 .voffset = 0,
	 .hts = 1176,
	 .vts = 3196,
	 .pclk = 112 * 1000 * 1000,	// 240 * 1000 * 1000,
	 .mipi_bps = 1120 * 1000 * 1000,	// 1800
	 .fps_fixed = 30,
	 .bin_factor = 1,
	 .intg_min = 1 << 4,
	 .intg_max = (3328 - 4) << 4,
	 .gain_min = 1 << 4,
	 .gain_max = 15 << 4,
	 .regs = sensor_4160x3120_30fps_regs,
	 .regs_size = ARRAY_SIZE(sensor_4160x3120_30fps_regs),
	 .set_size = NULL,
	},

	{
	 .width = 2096,
	 .height = 1552,
	 .hoffset = 0,
	 .voffset = 0,
	 .hts = 1176,
	 .vts = 3196,
	 .pclk = 112 * 1000 * 1000,
	 .mipi_bps = 560 * 1000 * 1000,
	 .fps_fixed = 30,
	 .bin_factor = 1,
	 .intg_min = 1 << 4,
	 .intg_max = (2588 - 4) << 4,
	 .gain_min = 1 << 4,
	 .gain_max = 15 << 4,
	 .regs = sensor_2104x1560_30fps_regs,
	 .regs_size = ARRAY_SIZE(sensor_2104x1560_30fps_regs),
	 .set_size = NULL,
	 .top_clk = 360000000,
	 .isp_clk = 300000000,
	},

	{
	 .width = 2096,
	 .height = 1552,
	 .hoffset = 0,
	 .voffset = 0,
	 .hts = 1176,
	 .vts = 1598,
	 .pclk = 112 * 1000 * 1000,
	 .mipi_bps = 560 * 1000 * 1000,
	 .fps_fixed = 60,
	 .bin_factor = 1,
	 .intg_min = 1 << 4,
	 .intg_max = (2588 - 4) << 4,
	 .gain_min = 1 << 4,
	 .gain_max = 15 << 4,
	 .regs = sensor_2104x1560_60fps_regs,
	 .regs_size = ARRAY_SIZE(sensor_2104x1560_60fps_regs),
	 .set_size = NULL,
	},

#if VIN_FALSE
	{
	 .width = 1280,
	 .height = 720,
	 .hoffset = 0,
	 .voffset = 0,
	 .hts = 1176,
	 .vts = 3192,
	 .pclk = 112 * 1000 * 1000,
	 .mipi_bps = 560 * 1000 * 1000,
	 .fps_fixed = 30,
	 .bin_factor = 1,
	 .intg_min = 1 << 4,
	 .intg_max = (2588 - 4) << 4,
	 .gain_min = 1 << 4,
	 .gain_max = 15 << 4,
	 .regs = sensor_1280x720_30fps_regs,
	 .regs_size = ARRAY_SIZE(sensor_1280x720_30fps_regs),
	 .set_size = NULL,
	},
#endif

	{
	 .width = 1280,
	 .height = 720,
	 .hoffset = 0,
	 .voffset = 0,
	 .hts = 1176,
	 .vts = 1596,
	 .pclk = 112 * 1000 * 1000,
	 .mipi_bps = 560 * 1000 * 1000,
	 .fps_fixed = 60,
	 .bin_factor = 1,
	 .intg_min = 1 << 4,
	 .intg_max = (2588 - 4) << 4,
	 .gain_min = 1 << 4,
	 .gain_max = 15 << 4,
	 .regs = sensor_1280x720_60fps_regs,
	 .regs_size = ARRAY_SIZE(sensor_1280x720_60fps_regs),
	 .set_size = NULL,
	},

	{
	 .width = 1280,
	 .height = 720,
	 .hoffset = 0,
	 .voffset = 0,
	 .hts = 1176,
	 .vts = 798,
	 .pclk = 112 * 1000 * 1000,
	 .mipi_bps = 560 * 1000 * 1000,
	 .fps_fixed = 120,
	 .bin_factor = 1,
	 .intg_min = 1 << 4,
	 .intg_max = (2588 - 4) << 4,
	 .gain_min = 1 << 4,
	 .gain_max = 15 << 4,
	 .regs = sensor_1280x720_120fps_regs,
	 .regs_size = ARRAY_SIZE(sensor_1280x720_120fps_regs),
	 .set_size = NULL,
	},
};

#define N_WIN_SIZES (ARRAY_SIZE(sensor_win_sizes))

static int sensor_g_mbus_config(struct v4l2_subdev *sd, unsigned int pad,
				struct v4l2_mbus_config *cfg)
{
	cfg->type = V4L2_MBUS_CSI2_DPHY;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0)
	cfg->bus.mipi_csi2.num_data_lanes = 0 | V4L2_MBUS_CSI2_4_LANE | V4L2_MBUS_CSI2_CHANNEL_0;
#else
	cfg->flags = 0 | V4L2_MBUS_CSI2_4_LANE | V4L2_MBUS_CSI2_CHANNEL_0;
#endif

	return 0;
}

static int sensor_g_ctrl(struct v4l2_ctrl *ctrl)
{
	struct sensor_info *info =
			container_of(ctrl->handler,
			struct sensor_info, handler);
	struct v4l2_subdev *sd = &info->sd;

	switch (ctrl->id) {
	case V4L2_CID_GAIN:
		return sensor_g_gain(sd, &ctrl->val);
	case V4L2_CID_EXPOSURE:
		return sensor_g_exp(sd, &ctrl->val);
	}
	return -EINVAL;
}

static int sensor_s_ctrl(struct v4l2_ctrl *ctrl)
{
	struct sensor_info *info =
			container_of(ctrl->handler,
			struct sensor_info, handler);
	struct v4l2_subdev *sd = &info->sd;

	switch (ctrl->id) {
	case V4L2_CID_GAIN:
		return sensor_s_gain(sd, ctrl->val);
	case V4L2_CID_EXPOSURE:
		return sensor_s_exp(sd, ctrl->val);
	}
	return -EINVAL;
}

static int sensor_reg_init(struct sensor_info *info)
{

	int ret = 0;
	struct v4l2_subdev *sd = &info->sd;
	struct sensor_format_struct *sensor_fmt = info->fmt;
	struct sensor_win_size *wsize = info->current_wins;
	struct sensor_exp_gain exp_gain;

	ret = sensor_write_array(sd, sensor_default_regs,
			       ARRAY_SIZE(sensor_default_regs));
	if (ret < 0) {
		sensor_err("write sensor_default_regs error\n");
		return ret;
	}

	sensor_write_array(sd, sensor_fmt->regs, sensor_fmt->regs_size);

	if (wsize->regs)
		sensor_write_array(sd, wsize->regs, wsize->regs_size);

	if (wsize->set_size)
		wsize->set_size(sd);

	info->width = wsize->width;
	info->height = wsize->height;
	ov13b10_sensor_vts = wsize->vts;

	exp_gain.exp_val = info->exp;
	exp_gain.gain_val = info->gain;
	if (0 == exp_gain.exp_val || 0 == exp_gain.gain_val) {
		exp_gain.exp_val = 20000;
		exp_gain.gain_val = 512;
	}
	sensor_s_exp_gain(sd, &exp_gain);

	sensor_write(sd, 0x0100, 0x01);

	return 0;
}

static int sensor_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct sensor_info *info = to_state(sd);

	sensor_print("%s on = %d, %d*%d fps: %d code: %x\n", __func__, enable,
			info->current_wins->width, info->current_wins->height,
			info->current_wins->fps_fixed, info->fmt->mbus_code);

	if (!enable)
		return 0;

	return sensor_reg_init(info);
}

/* ----------------------------------------------------------------------- */

static const struct v4l2_ctrl_ops sensor_ctrl_ops = {
	.g_volatile_ctrl = sensor_g_ctrl,
	.s_ctrl = sensor_s_ctrl,
};

static const struct v4l2_subdev_core_ops sensor_core_ops = {
	.reset = sensor_reset,
	.init = sensor_init,
	.s_power = sensor_power,
	.ioctl = sensor_ioctl,
#if IS_ENABLED(CONFIG_COMPAT)
	.compat_ioctl32 = sensor_compat_ioctl32,
#endif
};

static const struct v4l2_subdev_video_ops sensor_video_ops = {
	.s_stream = sensor_s_stream,
};

static const struct v4l2_subdev_pad_ops sensor_pad_ops = {
	.enum_mbus_code = sensor_enum_mbus_code,
	.enum_frame_size = sensor_enum_frame_size,
	.enum_frame_interval = sensor_enum_frame_interval,
	.get_fmt = sensor_get_fmt,
	.set_fmt = sensor_set_fmt,
	.get_mbus_config = sensor_g_mbus_config,
};

static const struct v4l2_subdev_ops sensor_ops = {
	.core = &sensor_core_ops,
	.video = &sensor_video_ops,
	.pad = &sensor_pad_ops,
};

/* ----------------------------------------------------------------------- */
static struct cci_driver cci_drv = {
	.name = SENSOR_NAME,
	.addr_width = CCI_BITS_16,
	.data_width = CCI_BITS_8,
};

static int sensor_init_controls(struct v4l2_subdev *sd,
			const struct v4l2_ctrl_ops *ops)
{
	struct sensor_info *info = to_state(sd);
	struct v4l2_ctrl_handler *handler = &info->handler;
	struct v4l2_ctrl *ctrl;
	int ret = 0;

	v4l2_ctrl_handler_init(handler, 2);

	v4l2_ctrl_new_std(handler, ops, V4L2_CID_GAIN, 1 * 1600,
				256 * 1600, 1, 1 * 1600);
	ctrl = v4l2_ctrl_new_std(handler, ops, V4L2_CID_EXPOSURE, 0,
				65536 * 16, 1, 0);
	if (ctrl != NULL)
		ctrl->flags |= V4L2_CTRL_FLAG_VOLATILE;

	if (handler->error) {
		ret = handler->error;
		v4l2_ctrl_handler_free(handler);
	}

	sd->ctrl_handler = handler;

	return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 6, 0)
static int sensor_probe(struct i2c_client *client)
#else
static int sensor_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
#endif
{
	struct v4l2_subdev *sd;
	struct sensor_info *info;

	info = kzalloc(sizeof(struct sensor_info), GFP_KERNEL);
	if (info == NULL)
		return -ENOMEM;
	mutex_init(&info->lock);
	sd = &info->sd;
	cci_dev_probe_helper(sd, client, &sensor_ops, &cci_drv);
	sensor_init_controls(sd, &sensor_ctrl_ops);

	info->fmt = &sensor_formats[0];
	info->fmt_pt = &sensor_formats[0];
	info->win_pt = &sensor_win_sizes[0];
	info->fmt_num = N_FMTS;
	info->win_size_num = N_WIN_SIZES;
	info->sensor_field = V4L2_FIELD_NONE;
	info->af_first_flag = 1;
	info->time_hs = 0x20;
	info->deskew = 0x02;
	info->exp = 0;
	info->gain = 0;

	return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0)
static int sensor_remove(struct i2c_client *client)
#else
static void sensor_remove(struct i2c_client *client)
#endif
{
	struct v4l2_subdev *sd;

	sd = cci_dev_remove_helper(client, &cci_drv);
	kfree(to_state(sd));
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0)
	return 0;
#endif
}

static const struct i2c_device_id sensor_id[] = {
	{SENSOR_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, sensor_id);

static struct i2c_driver sensor_driver = {
	.driver = {
		   .owner = THIS_MODULE,
		   .name = SENSOR_NAME,
		   },
	.probe = sensor_probe,
	.remove = sensor_remove,
	.id_table = sensor_id,
};
static __init int init_sensor(void)
{
	return cci_dev_init_helper(&sensor_driver);
}

static __exit void exit_sensor(void)
{
	cci_dev_exit_helper(&sensor_driver);
}

VIN_INIT_DRIVERS(init_sensor);
module_exit(exit_sensor);
