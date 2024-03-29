/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * vin_core.h for video manage
 *
 * Copyright (c) 2017 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 *
 * Authors:  Zhao Wei <zhaowei@allwinnertech.com>
 *	Yang Feng <yangfeng@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _VIN_CORE_H_
#define _VIN_CORE_H_

#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/workqueue.h>
#include <linux/pm_runtime.h>

#include <media/sunxi_camera_v2.h>
#include <media/videobuf2-core.h>
#include <media/v4l2-device.h>
#include <media/v4l2-mediabus.h>

#include "../platform/platform_cfg.h"
#include "../modules/flash/flash.h"

#include "../modules/actuator/actuator.h"
#include "../vin-mipi/bsp_mipi_csi.h"
#include "../utility/vin_supply.h"
#include "vin_video.h"
#include "../vin.h"

#define MAX_FRAME_MEM   (150*1024*1024)
#define MIN_WIDTH       (32)
#define MIN_HEIGHT      (32)
#define MAX_WIDTH       (4800)
#define MAX_HEIGHT      (4800)

struct vin_status_info {
	unsigned int width;
	unsigned int height;
	unsigned int h_off;
	unsigned int v_off;
	unsigned int err_cnt;
	unsigned int buf_cnt;
	unsigned int buf_size;
	unsigned int buf_rest;
	unsigned int frame_cnt;
	unsigned int lost_cnt;
	unsigned int frame_internal;
	unsigned int max_internal;
	unsigned int min_internal;
	struct prs_input_para prs_in;
};

struct vin_coor {
	unsigned int x1;
	unsigned int y1;
	unsigned int x2;
	unsigned int y2;
};

struct vin_ptn_cfg {
	__u32 ptn_en;
	__u32 ptn_w;
	__u32 ptn_h;
	__u32 ptn_mode;
	__u32 ptn_dw;
	__u32 ptn_type;
	struct vin_mm ptn_buf;
};

enum vin_sdram_status {
	SDRAM_NOT_USED,
	SDRAM_CAN_ENABLE,
	SDRAM_ALREADY_ENABLE,
};

struct vin_sdram_dfs {
	struct csic_chfreq_obs_value csic_chfreq_obs_value;
	struct csic_chfreq_obs_value last_csic_chfreq_obs_value;
	unsigned int stable_frame_cnt;
	unsigned int stable_frame_cnt_tmp;
	unsigned int sensor_status;
	unsigned int vinc_sdram_status;
};

struct vinc_tvin {
	bool flag;
	__u32 input_fmt;
	__u16 work_mode;
};

struct vin_core {
	unsigned char is_empty;
	unsigned char id;
	unsigned char support_raw;
	unsigned char total_rx_ch;
	unsigned char hflip;
	unsigned char vflip;
	unsigned char sensor_hflip;
	unsigned char sensor_vflip;
	unsigned char stream_idx;
	unsigned char fps_ds;
	unsigned char use_timer;
	unsigned char large_image;/* 2:get merge yuv, 1: get pattern raw (save in kernel), 0: normal */
	unsigned int rear_sensor;
	unsigned int front_sensor;
	unsigned int sensor_sel;
	unsigned int csi_sel;
	unsigned int mipi_sel;
	unsigned int isp_sel;
	unsigned int tdm_rx_sel;
	unsigned int vipp_sel;
	unsigned int isp_tx_ch;
	unsigned int vin_clk;
	unsigned char dma_merge_mode;
	unsigned int delay_init;
	unsigned int bandwidth;
	int irq;
	void __iomem *base;
	struct platform_device *pdev;
	struct isp_debug_mode isp_dbg;
	struct sensor_exp_gain exp_gain;
	struct v4l2_device *v4l2_dev;
	const struct vin_pipeline_ops *pipeline_ops;
	struct vin_status_info vin_status;
	struct timer_list timer_for_reset;
	struct vin_ptn_cfg ptn_cfg;
	struct vin_vid_cap vid_cap;
	unsigned int work_mode;
	unsigned int stream_count;
	unsigned char vir_prosess_ch;
	unsigned char is_irq_empty;
	unsigned char noneed_register;
	unsigned char logic_top_stream_count;
	unsigned char ve_ol_ch;
	struct csi_ve_online_cfg ve_online_cfg;
#ifdef CSIC_SDRAM_DFS
	struct vin_sdram_dfs vin_dfs;
#endif
	struct vinc_tvin tvin;
#if IS_ENABLED(CONFIG_ISP_SERVER_MELIS)
	struct work_struct ldci_buf_send_task;
#endif
#if IS_ENABLED(CONFIG_RV_RUN_CAR_REVERSE)
	struct rpmsg_vinc rpmsg;
#endif

};

static inline struct sensor_instance *get_valid_sensor(struct vin_core *vinc)
{
	struct vin_md *vind = dev_get_drvdata(vinc->v4l2_dev->dev);
	int valid_idx = NO_VALID_SENSOR;

	valid_idx = vind->modules[vinc->sensor_sel].sensors.valid_idx;

	if (valid_idx == NO_VALID_SENSOR)
		return NULL;

	return &vind->modules[vinc->sensor_sel].sensors.inst[valid_idx];
}
int sunxi_vin_debug_register_driver(void);
void sunxi_vin_debug_unregister_driver(void);
int sunxi_vin_core_register_driver(void);
void sunxi_vin_core_unregister_driver(void);
struct vin_core *sunxi_vin_core_get_dev(int index);
struct vin_fmt *vin_find_format(const u32 *pixelformat, const u32 *mbus_code,
				  unsigned int mask, int index, bool have_code);

#endif /* _VIN_CORE_H_ */
