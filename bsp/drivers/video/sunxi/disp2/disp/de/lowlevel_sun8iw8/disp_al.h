/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Allwinner SoCs display driver.
 *
 * Copyright (C) 2016 Allwinner.
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */
#ifndef _DISP_AL_H_
#define _DISP_AL_H_

#include "../include.h"
#include "de_feat.h"
#include "de_hal.h"
#include "de_enhance.h"
#include "de_wb.h"
#include "de_smbl.h"
#include "de_csc.h"
#include "de_lcd.h"
#include "de_clock.h"
#include "de_rtmx.h"

/* avoid compilation errors */
enum {
	DISP_AL_IRQ_TPYE_RCQ         = 0,
	DISP_AL_IRQ_TPYE_RTMX_GLABAL = 0,
	DISP_AL_IRQ_TPYE_UNDEFINE    = 0,
};

enum {
	DISP_AL_IRQ_FLAG_FRAME_END  = DE_IRQ_FLAG_FRAME_END,
	DISP_AL_IRQ_FLAG_RCQ_FINISH = DE_IRQ_FLAG_RCQ_FINISH,
	DISP_AL_IRQ_FLAG_RCQ_ACCEPT = DE_IRQ_FLAG_RCQ_ACCEPT,
	DISP_AL_IRQ_FLAG_MASK       = DE_IRQ_FLAG_MASK,
};

enum  {
	DISP_AL_IRQ_STATE_FRAME_END  = DE_IRQ_STATE_FRAME_END,
	DISP_AL_IRQ_STATE_RCQ_FINISH = DE_IRQ_STATE_RCQ_FINISH,
	DISP_AL_IRQ_STATE_RCQ_ACCEPT = DE_IRQ_STATE_RCQ_ACCEPT,
	DISP_AL_IRQ_STATE_MASK = DE_IRQ_STATE_MASK,
};

enum {
	DISP_AL_CAPTURE_IRQ_FLAG_FRAME_END = WB_IRQ_FLAG_INTR,
	DISP_AL_CAPTURE_IRQ_FLAG_RCQ_ACCEPT = DE_WB_IRQ_FLAG_RCQ_ACCEPT,
	DISP_AL_CAPTURE_IRQ_FLAG_RCQ_FINISH = DE_WB_IRQ_FLAG_RCQ_FINISH,
	DISP_AL_CAPTURE_IRQ_FLAG_MASK =
		DISP_AL_CAPTURE_IRQ_FLAG_FRAME_END
		| DISP_AL_CAPTURE_IRQ_FLAG_RCQ_ACCEPT
		| DISP_AL_CAPTURE_IRQ_FLAG_RCQ_FINISH,
};

enum {
	DISP_AL_CAPTURE_IRQ_STATE_FRAME_END = WB_IRQ_STATE_PROC_END,
	DISP_AL_CAPTURE_IRQ_STATE_RCQ_ACCEPT = DE_WB_IRQ_STATE_RCQ_ACCEPT,
	DISP_AL_CAPTURE_IRQ_STATE_RCQ_FINISH = DE_WB_IRQ_STATE_RCQ_FINISH,
	DISP_AL_CAPTURE_IRQ_STATE_MASK =
		DISP_AL_CAPTURE_IRQ_STATE_FRAME_END
		| DISP_AL_CAPTURE_IRQ_STATE_RCQ_ACCEPT
		| DISP_AL_CAPTURE_IRQ_STATE_RCQ_FINISH,
};

struct lcd_clk_info {
	enum disp_lcd_if lcd_if;
	int tcon_div;
	int lcd_div;
	int dsi_div;
	int dsi_rate;
};

int disp_al_manager_init(unsigned int disp);
int disp_al_manager_exit(unsigned int disp);
extern int disp_al_manager_apply(unsigned int disp,
				 struct disp_manager_data *data);
extern int disp_al_layer_apply(unsigned int disp,
			       struct disp_layer_config_data *data,
			       unsigned int layer_num);
extern int disp_init_al(struct disp_bsp_init_para *para);
extern int disp_al_manager_sync(unsigned int disp);
extern int disp_al_manager_update_regs(unsigned int disp);
int disp_al_manager_query_irq(unsigned int disp);

int disp_al_enhance_apply(unsigned int disp,
			  struct disp_enhance_config *config);
int disp_al_enhance_update_regs(unsigned int disp);
int disp_al_enhance_sync(unsigned int disp);
int disp_al_enhance_tasklet(unsigned int disp);

int disp_al_smbl_apply(unsigned int disp, struct disp_smbl_info *info);
int disp_al_smbl_update_regs(unsigned int disp);
int disp_al_smbl_sync(unsigned int disp);
int disp_al_smbl_get_status(unsigned int disp);
int disp_al_smbl_tasklet(unsigned int disp);

int disp_al_capture_init(unsigned int disp);
int disp_al_capture_exit(unsigned int disp);
int disp_al_capture_sync(u32 disp);
int disp_al_capture_apply(unsigned int disp, struct disp_capture_config *cfg);
int disp_al_capture_get_status(unsigned int disp);

int disp_al_lcd_cfg(u32 screen_id, struct disp_panel_para *panel,
		    struct panel_extend_para *extend_panel);
int disp_al_lcd_cfg_ext(u32 screen_id, struct panel_extend_para *extend_panel);
int disp_al_lcd_enable(u32 screen_id, struct disp_panel_para *panel);
int disp_al_lcd_disable(u32 screen_id, struct disp_panel_para *panel);
int disp_al_lcd_query_irq(u32 screen_id, __lcd_irq_id_t irq_id,
			  struct disp_panel_para *panel);
int disp_al_lcd_tri_busy(u32 screen_id, struct disp_panel_para *panel);
int disp_al_lcd_tri_start(u32 screen_id, struct disp_panel_para *panel);
int disp_al_lcd_io_cfg(u32 screen_id, u32 enable,
		       struct disp_panel_para *panel);
int disp_al_lcd_get_cur_line(u32 screen_id, struct disp_panel_para *panel);
int disp_al_lcd_get_start_delay(u32 screen_id, struct disp_panel_para *panel);
int disp_al_lcd_get_clk_info(u32 screen_id, struct lcd_clk_info *info,
			     struct disp_panel_para *panel);
int disp_al_lcd_enable_irq(u32 screen_id, __lcd_irq_id_t irq_id,
			   struct disp_panel_para *panel);
int disp_al_lcd_disable_irq(u32 screen_id, __lcd_irq_id_t irq_id,
			    struct disp_panel_para *panel);

int disp_al_hdmi_enable(u32 screen_id);
int disp_al_hdmi_disable(u32 screen_id);
int disp_al_hdmi_cfg(u32 screen_id, struct disp_video_timings *video_info);

int disp_al_vdevice_cfg(u32 screen_id, struct disp_video_timings *video_info,
			struct disp_vdevice_interface_para *para,
			u8 config_tcon_only);
int disp_al_vdevice_enable(u32 screen_id);
int disp_al_vdevice_disable(u32 screen_id);

int disp_al_device_get_cur_line(u32 screen_id);
int disp_al_device_get_start_delay(u32 screen_id);
int disp_al_device_query_irq(u32 screen_id);
int disp_al_device_enable_irq(u32 screen_id);
int disp_al_device_disable_irq(u32 screen_id);
int disp_al_device_get_status(u32 screen_id);

int disp_al_get_fb_info(unsigned int sel, struct disp_layer_info *info);
int disp_al_get_display_size(unsigned int sel, unsigned int *width,
			     unsigned int *height);

int disp_exit_al(void);
bool disp_al_get_direct_show_state(unsigned int disp);
void disp_al_show_builtin_patten(u32 hwdev_index, u32 patten);
int disp_al_lcd_get_status(u32 screen_id, struct disp_panel_para *panel);

static inline s32 disp_al_capture_set_rcq_update(u32 disp, u32 en) { return 0; }

static inline u32 disp_al_capture_query_irq_state(u32 disp, u32 irq_state) { return 0; }

static inline s32 disp_al_capture_set_all_rcq_head_dirty(u32 disp, u32 dirty) { return 0; }

static inline s32 disp_al_capture_set_irq_enable(u32 disp, u32 irq_flag, u32 en) { return 0; }

static inline s32 disp_al_manager_set_rcq_update(u32 disp, u32 en) { return 0; }

static inline s32 disp_al_manager_set_all_rcq_head_dirty(u32 disp, u32 dirty) { return 0; }

static inline s32 disp_al_manager_set_irq_enable(u32 disp, u32 irq_flag, u32 irq_type, u32 en) { return 0; }

static inline u32 disp_al_manager_query_irq_state(u32 disp, u32 irq_state, u32 irq_type) { return 0; }

static inline int disp_al_device_set_de_id(u32 screen_id, u32 de_id) { return 0; }

static inline int disp_al_device_set_de_use_rcq(u32 screen_id, u32 use_rcq) { return 0; }

static inline int disp_al_device_set_output_type(u32 screen_id, u32 output_type) { return 0; }

static inline void disp_al_flush_layer_address(u32 disp, u32 chn, u32 layer_id) {}

#endif
