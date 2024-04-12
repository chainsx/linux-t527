/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * isp_default_tbl.h for all v4l2 subdev manage
 *
 * Copyright (c) 2017 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 *
 * Authors:  Zhao Wei <zhaowei@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _ISP_DEFAULT_TBL_H_
#define _ISP_DEFAULT_TBL_H_

#include "../platform/platform_cfg.h"

#if IS_ENABLED(CONFIG_ARCH_SUN8IW16P1)
#include "isp520/isp520_reg_cfg.h"
#elif defined CONFIG_ARCH_SUN8IW19P1
#include "isp521/isp521_reg_cfg.h"
#elif defined CONFIG_ARCH_SUN50IW10
#include "isp522/isp522_reg_cfg.h"
#elif defined CONFIG_ARCH_SUN55IW3 || defined CONFIG_ARCH_SUN55IW6 || defined CONFIG_ARCH_SUN60IW1 || defined CONFIG_ARCH_SUN60IW2
#include "isp600/isp600_reg_cfg.h"
#else
#include "isp500/isp500_reg_cfg.h"
#endif

unsigned int isp_default_reg[ISP_LOAD_REG_SIZE>>2] = {
#if !defined ISP_600
#if !defined CONFIG_ARCH_SUN8IW16P1 && !defined CONFIG_ARCH_SUN8IW19P1 && !defined CONFIG_ARCH_SUN50IW10
	0x00000101, 0x00000001, 0x00004111, 0x00000087,
	0x03c00010, 0x00000000, 0x28000000, 0x04000000,
	0x0dc11000, 0x0dc11400, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x80000000, 0x00000004, 0x00000000, 0x0136003c,
	0x00000106, 0x00005040, 0x00000000, 0x00000000,
	0x00000000, 0x000f0013, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x01e00280, 0x01e00280,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x0f000200, 0x01390010,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x04000804, 0x00000000, 0x00000000, 0x00000000,
	0x00021010, 0x00000000, 0x00000000, 0x00000000,
	0x00400010, 0x01000100, 0x00200020, 0x00000100,
	0x00200020, 0x00200020, 0x04000400, 0x04000400,
	0x00200020, 0x00200020, 0x00ff00ff, 0x000000ff,
	0x000f0013, 0x00000000, 0x00000000, 0x00000000,
	0x00080008, 0x00000000, 0x00000000, 0x00000000,
	0x40070f01, 0xfcff0080, 0x1f173c2d, 0x001845c8,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x01000100, 0x01000100, 0x00000fff, 0x00000010,
	0x30000000, 0x00000080, 0x0003875c, 0x00400010,
	0x02000200, 0x04000400, 0x00000000, 0x00000484,
	0x00000808, 0x00420077, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x02040107, 0x07680064, 0x01c206d6, 0x068701c2,
	0x000007b7, 0x02010010, 0x00000008, 0x00000000,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00000000, 0x00000000, 0x00000000,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x02108421, 0x02108421,
	0x02108421, 0x02108421, 0x02108421, 0x02108421,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00000000, 0x00000000,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
#else
	0x00000173, 0x00530000, 0xfff00000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x1f01ba90, 0x1f01ba90, 0x1f018000, 0x00000000,
	0x00001451, 0x00000000, 0x8000010a, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x001f0010, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00004224, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x01ce01cd, 0x0000ffff, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000cc0,
	0x00000000, 0x00200020, 0x00200020, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x000000a0, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x04380780, 0x04380780, 0x00000000, 0x00200020,
	0x00200020, 0x00000000, 0x00000000, 0x00000000,
	0x0f000200, 0x01390010, 0x003c0c00, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x04000804, 0x00000000, 0x00000000, 0x00000000,
	0x0136003c, 0x00000106, 0x00005040, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00b4c8dc, 0x1c181410, 0x00203040, 0x8360410a,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x1f1f1fff, 0x00020000, 0x00ff00ff, 0x001900ff,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00200020, 0x00200020, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x04000400, 0x04000400, 0x00000000, 0x00000000,
	0x01000100, 0x01000100, 0x00000fff, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x30000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x40070f01, 0xfcff0080, 0x1f173c2d, 0x001845c8,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000080, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00800080, 0x00800080, 0x041f0404, 0x00000000,
	0x00000140, 0x1f1f0018, 0x00400040, 0x00400040,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000010, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000484, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x208100a0, 0x42cc00ab, 0x008700f0, 0x01e50111,
	0xffdffbff, 0x00000100, 0x00000100, 0x00000100,
	0x00005100, 0xf2d33b98, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x02040107, 0x07680064, 0x01c206d6, 0x068701c2,
	0x000007b7, 0x02000040, 0x00000200, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x000f0013, 0x00000000, 0x00000000, 0x00000000,
	0x01080000, 0x008600f0, 0x00000000, 0x39ccf1ac,
	0x00029391, 0x107ca5bd, 0x37e52759, 0x002d0035,
	0x00c5027b, 0x004d009d, 0x00710088, 0x2fb00511,
	0x2cc00414, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x000000f0,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00ff00ff, 0x000000ff, 0x000f0013, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00080008, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000008, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00000008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00000008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00000008, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x01000100,
	0x01000100, 0x01000100, 0x01000100, 0x01000100,
	0x01000100, 0x01000100, 0x01000100, 0x01000100,
	0x01000100, 0x01000100, 0x01000100, 0x01000100,
	0x01000100, 0x01000100, 0x01000100, 0x01000100,
	0x01000100, 0x01000100, 0x01000100, 0x01000100,
	0x01000100, 0x01000100, 0x01000100, 0x01000100,
	0x01000100, 0x01000100, 0x01000100, 0x01000100,
	0x01000100, 0x01000100, 0x01000100, 0x01000100,
	0x01000100, 0x01000100, 0x01000100, 0x01000100,
	0x01000100, 0x01000100, 0x01000100, 0x01000100,
	0x01000100, 0x01000100, 0x01000100, 0x01000100,
	0x01000100, 0x01000100, 0x01000100, 0x01000100,
	0x01000100, 0x01000100, 0x01000100, 0x01000100,
	0x01000100, 0x01000100, 0x01000100, 0x01000100,
	0x01000100, 0x01000100, 0x01000100, 0x01000100,
	0x01000100, 0x01000100, 0x01000100, 0x01000100,
	0x01000100, 0x01000100, 0x01000100, 0x01000100,
	0x01000100, 0x01000100, 0x01000100, 0x01000100,
	0x01000100, 0x80808080, 0x80808080, 0x80808080,
	0x80808080, 0x80808080, 0x80808080, 0x80808080,
	0x80808080, 0x80808080, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00000008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00000008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00000008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00080008, 0x00080008, 0x00080008,
	0x00080008, 0x00000008, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
#endif
#else /* else isp600 */
	0x00000005, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000001, 0x140162f0, 0x005f0f0c, 0x00000013,
	0x00000000, 0x00000080, 0x00000000, 0x14010000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000080, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000080, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000080, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x000c00c7, 0x00000053, 0x0fa0002a, 0x00000000,
	0x00000000, 0x14011a80, 0x00000000, 0x00000000,
	0x04380780, 0x04380780, 0x00000000, 0x00000000,
	0x00000000, 0x00000080, 0x00000030, 0x00100110,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
#if IS_ENABLED(CONFIG_ARCH_SUN60IW2)
	0x1e0c1e1e, 0x00240130, 0x241e0c1e, 0x000c0130,
#elif IS_ENABLED(CONFIG_ARCH_SUN60IW1)
	0x1a0c1a1a, 0x00240130, 0x241a0c1a, 0x000c0130,
#elif IS_ENABLED(CONFIG_ARCH_SUN55IW6)
	0x1e0c1e1e, 0x00240100, 0x241e0c1e, 0x000c0100,
#else
	0x120c1212, 0x002400c0, 0x24120c12, 0x000c00c0,
#endif
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x000180c0, 0x80008000, 0x00000000, 0x00000000,
	0x1fc01fc0, 0x1fc01fc0, 0x000c00c0, 0x00000000,
	0x04000400, 0x04000400, 0x00000000, 0x00000000,
	0x1fc01fc0, 0x1fc01fc0, 0x000c00c0, 0x00000000,
	0x04000400, 0x04000400, 0x00000000, 0x00000000,
	0x1fc01fc0, 0x1fc01fc0, 0x000c00c0, 0x00000000,
	0x04000400, 0x04000400, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x07033371, 0x0fffff0f, 0x00000000, 0x00000000,
	0xffff0013, 0x0f000200, 0x00400015, 0x0f000333,
	0x04000040, 0x08000010, 0x08000002, 0x04000004,
	0xffff0013, 0x0f000200, 0x00400015, 0x0f000333,
	0x04000040, 0x08000010, 0x08000002, 0x04000004,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0xa4a0c040, 0x08004000, 0x0aaaee0c, 0x00000000,
	0x0136003c, 0x00000106, 0x00005040, 0x00000000,
	0x03bf021b, 0x0023fb0e, 0x0008ad60, 0x0000000a,
	0x00000000, 0x57310000, 0x002f0053, 0x00000000,
	0x80008000, 0x0f000200, 0x0f000200, 0x00130013,
	0x00800080, 0x00000000, 0x01040609, 0x202000c3,
	0x00100010, 0x01000100, 0x00100010, 0x01000100,
	0xc627a010, 0x00000000, 0x00000000, 0x00000000,
	0x4000401c, 0x40040100, 0x40003024, 0x40040100,
	0x40002018, 0x40040100, 0x4000181c, 0x40040100,
	0x0400008a, 0x40000100, 0x0020400f, 0x00000000,
	0x00d50078, 0x0a8c3c2d, 0x08000044, 0x07d2005b,
	0x50185050, 0x20401010, 0x00000001, 0x00000000,
	0x02002020, 0x02002020, 0x00800080, 0x00000000,
	0x010100c0, 0x011001e0, 0x011801e8, 0x012001f0,
	0x012c01f8, 0x013801fa, 0x013c01fc, 0x013f01ff,
	0x3c244058, 0x00005678, 0x20408010, 0x20408005,
	0x2a002300, 0x00000160, 0x1fc01fc0, 0x1fc01fc0,
	0x010001c4, 0x01a20100, 0x00000fff, 0x00000000,
	0x04000400, 0x04000400, 0x00000000, 0x00000000,
	0x304383c0, 0x304383c0, 0x304383c0, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x800f8000, 0x000003ff, 0x0a8c2d3c, 0x08000044,
	0x07d2005b, 0x00000000, 0x00000000, 0x00000000,
	0x00800080, 0x08010200, 0xff008020, 0x00000000,
	0x00800080, 0x08010200, 0xff008020, 0x00000000,
	0x00800080, 0x08010200, 0xff008020, 0x00000000,
	0x00800080, 0x08010200, 0xff008020, 0x00000000,
	0x00800000, 0x0024f000, 0x02000010, 0x0084c000,
	0xffffff74, 0x00001010, 0x0fa00fff, 0x01010101,
	0x03000200, 0x00600140, 0x01189200, 0x0008fb20,
	0x00000000, 0x00801000, 0x16010040, 0x40100210,
	0x00000000, 0x00000000, 0x00100100, 0x00000000,
	0x00000000, 0x00000000, 0x00100100, 0x00000000,
	0x00000000, 0x00200020, 0x00018600, 0x00000000,
	0x00000000, 0x00200020, 0x00018600, 0x00000000,
	0x00000000, 0x00200020, 0x00018600, 0x00000000,
	0x00000100, 0x00000000, 0x00000100, 0x00000000,
	0x00000100, 0x00000000, 0x00000000, 0x00000000,
	0x00100010, 0x00100010, 0x00000000, 0x00000000,
	0x00000008, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x02040107, 0x07680064, 0x01c206d6, 0x068701c2,
	0x004007b7, 0x02000200, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x003c0050, 0x00040400, 0x000012c0,
	0x00000000, 0x00000000, 0x01000100, 0x01000100,
	0x01080000, 0x008600f0, 0x00000000, 0x39ccf1ac,
	0x00029391, 0x00000000, 0x00000000, 0x00000035,
	0x0000027b, 0x0000009d, 0x00000088, 0x2fb00511,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x000000f0,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x003c0050, 0x0fff0004, 0x0fff0fff,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000008, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x04380780, 0x00000004, 0x00000000,
	0x00000000, 0x04380780, 0x00000004, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00040200, 0x00100580, 0x00400880, 0x02001000,
	0x00000000, 0x01800480, 0x08000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00310057, 0x00310057, 0x00310057, 0x00310057,
	0x00310057, 0x00310057, 0x00310057, 0x00310057,
	0x00310057, 0x00310057, 0x00310057, 0x00000000,
	0x0053002f, 0x0053002f, 0x0053002f, 0x0053002f,
	0x0053002f, 0x0053002f, 0x0053002f, 0x0053002f,
	0x0053002f, 0x0053002f, 0x0053002f, 0x0053002f,
	0x00310057, 0x00310057, 0x00310057, 0x00310057,
	0x00310057, 0x00310057, 0x00310057, 0x00310057,
	0x00310057, 0x00310057, 0x00310057, 0x00000000,
	0x0053002f, 0x0053002f, 0x0053002f, 0x0053002f,
	0x0053002f, 0x0053002f, 0x0053002f, 0x0053002f,
	0x0053002f, 0x0053002f, 0x0053002f, 0x0053002f,
	0x00100010, 0x00100010, 0x00100010, 0x00100010,
	0x00100010, 0x00100010, 0x00100010, 0x00100010,
	0x00100010, 0x00100010, 0x00100010, 0x00100010,
	0x00100010, 0x00100010, 0x00100010, 0x00100010,
	0x00100010, 0x00000000, 0x00000000, 0x00000000,
	0x00100010, 0x00100010, 0x00100010, 0x00100010,
	0x00100010, 0x00100010, 0x00100010, 0x00100010,
	0x00100010, 0x00100010, 0x00100010, 0x00100010,
	0x00100010, 0x00100010, 0x00100010, 0x00100010,
	0x00100010, 0x00000000, 0x00000000, 0x00000000,
	0x00100010, 0x00100010, 0x00100010, 0x00100010,
	0x00100010, 0x00100010, 0x00100010, 0x00100010,
	0x00100010, 0x00100010, 0x00100010, 0x00100010,
	0x00100010, 0x00100010, 0x00100010, 0x00100010,
	0x00100010, 0x00000000, 0x00000000, 0x00000000,
	0x00100010, 0x00100010, 0x00100010, 0x00100010,
	0x00100010, 0x00100010, 0x00100010, 0x00100010,
	0x00100010, 0x00100010, 0x00100010, 0x00100010,
	0x00100010, 0x00100010, 0x00100010, 0x00100010,
	0x00100010, 0x00000000, 0x00000000, 0x00000000,
	0x00100010, 0x00100010, 0x00100010, 0x00100010,
	0x00100010, 0x00100010, 0x00100010, 0x00100010,
	0x00100010, 0x00100010, 0x00100010, 0x00100010,
	0x00100010, 0x00100010, 0x00100010, 0x00100010,
	0x00100010, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x01010101, 0x01010101, 0x01010101, 0x01010101,
	0x01010101, 0x01010101, 0x01010101, 0x01010101,
	0x01010101, 0x00000000, 0x00000000, 0x00000000,
	0x01010101, 0x01010101, 0x01010101, 0x01010101,
	0x01010101, 0x01010101, 0x01010101, 0x01010101,
	0x01010101, 0x00000000, 0x00000000, 0x00000000,
	0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00000000, 0x00000000, 0x00000000,
	0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00000000, 0x00000000, 0x00000000,
	0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00000000, 0x00000000, 0x00000000,
	0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
	0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0x00000000, 0x0f0f0f0f, 0x0f0f0f0f,
	0x0f0f0f0f, 0x0f0f0f0f, 0x0f0f0f0f, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x0f0f0f0f, 0x0f0f0f0f, 0x0f0f0f0f, 0x0f0f0f0f,
	0x0f0f0f0f, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0xffffffff, 0xffffffff,
	0xffffffff, 0xffffffff, 0xffffffff, 0x00000000,
	0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
	0xffffffff, 0x00000000, 0x0ff3fcff, 0x0ff3fcff,
	0x0ff3fcff, 0x0ff3fcff, 0x0ff3fcff, 0x0ff3fcff,
	0x0ff3fcff, 0x0ff3fcff, 0x0ff3fcff, 0x0ff3fcff,
	0x0ff3fcff, 0x0ff3fcff, 0x000f000f, 0x000f000f,
	0x000f000f, 0x000f000f, 0x000f000f, 0x000f000f,
	0x000f000f, 0x000f000f, 0x000f000f, 0x000f000f,
	0x000f000f, 0x000f000f, 0x000f000f, 0x000f000f,
	0x000f000f, 0x000f000f, 0x000f000f, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
	0x00000000, 0x00000000, 0x00000000, 0x00000000,
#endif
};

#endif /* _ISP_DEFAULT_TBL_H_ */
