// SPDX-License-Identifier: (GPL-2.0+ or MIT)
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Copyright (C) 2022 rengaomin@allwinnertech.com
 */

#ifndef _DT_BINDINGS_CLK_SUN65IW1_CPUPLL_H_
#define _DT_BINDINGS_CLK_SUN65IW1_CPUPLL_H_

#define CLK_PLL_CPU0		0
#define CLK_PLL_CPU0_DIV	1
#define CLK_CPU0		2
#define CLK_CPU0_APB_DIV	3
#define CLK_CPU0_AXI_DIV	4
#define CLK_PLL_CPU1		5
#define CLK_PLL_CPU1_DIV	6
#define CLK_CPU1		7
#define CLK_CPU1_APB_DIV	8
#define CLK_CPU1_AXI_DIV	9
#define CLK_PLL_CPU2		10
#define CLK_PLL_CPU2_DIV	11
#define CLK_CPU2		12
#define CLK_CPU2_APB_DIV	13
#define CLK_CPU2_AXI_DIV	14

#define CLK_CPUPLL_MAX_NO	(CLK_CPU2_AXI_DIV + 1)

#endif /* _DT_BINDINGS_CLK_SUN65IW1_H_ */
