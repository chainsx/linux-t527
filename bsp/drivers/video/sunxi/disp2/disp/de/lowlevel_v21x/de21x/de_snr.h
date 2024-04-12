/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * de_snr.h
 *
 * Copyright (c) 2007-2018 Allwinnertech Co., Ltd.
 * Author: zhengxiaobin <zhengxiaobin@allwinnertech.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef _DE_SNR_H
#define _DE_SNR_H

#include "de_top.h"

s32 de_snr_init(u32 disp, u8 __iomem *de_reg_base);
s32 de_snr_exit(u32 disp);
s32 de_snr_get_reg_blocks(u32 disp,
	struct de_reg_block **blks, u32 *blk_num);
s32 de_snr_set_para(u32 disp, u32 chn,
		    struct disp_layer_config_data **const pdata, u32 layer_num);

#endif /*End of file*/
