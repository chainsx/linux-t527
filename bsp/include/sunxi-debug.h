/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2020 - 2025 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Allwinner's debug functions
 * Martin Wu <wuyan@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 */
#ifndef __SUNXI_DEBUG_H__
#define __SUNXI_DEBUG_H__

#include <linux/atomic.h>
#include <linux/timekeeping.h>
#include <sunxi-log.h>

#define sunxi_assert_non_atomic()     BUG_ON(in_atomic())

#endif
