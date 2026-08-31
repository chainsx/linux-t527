/*
 * Copyright (c) 2007-2018 Allwinnertech Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _VE_RPMSG_H_
#define _VE_RPMSG_H_

typedef int amp_ctrl(void);

struct ve_amp_ctrl {
	amp_ctrl *rv_start;
	amp_ctrl *rv_stop;
	bool rv_irq_state;
	bool iommu_need;
	struct platform_device *pdev;
};

void amp_ve_init(void *amp_ops);
void amp_ve_exit(void);

#endif