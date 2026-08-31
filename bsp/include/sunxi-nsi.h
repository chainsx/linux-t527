/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 *
 * Copyright (C) 2015 AllWinnertech Ltd.
 *
 * Author: ouyangkun <ouyangkun@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef __LINUX_SUNXI_NSI_H
#define __LINUX_SUNXI_NSI_H

#include "../drivers/bus/sunxi-nsi.h"

#if IS_ENABLED(CONFIG_AW_NSI_DISTRIBUTE)
void sunxi_nsi_master_ready(struct device *dev);
#else
static inline void sunxi_nsi_master_ready(struct device *dev) { ; }
#endif
int notrace nsi_port_setmode(enum nsi_pmu port, unsigned int mode);
int notrace nsi_port_set_abs_bwlen(enum nsi_pmu port, bool en);
int notrace nsi_set_cpu_rw_bwl(unsigned int cpu_port, unsigned int bwl);
int notrace nsi_port_set_abs_bwl(enum nsi_pmu port, unsigned int bwl);
int notrace nsi_port_setio(enum nsi_pmu port, bool io);
int notrace nsi_port_setqos(enum nsi_pmu port, unsigned int qos);
int notrace nsi_port_setpri(enum nsi_pmu port, unsigned int pri);

#endif
