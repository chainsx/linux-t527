/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * Copyright (c) 2023 rengaomin@allwinnertech.com
 */

#ifndef _DT_BINDINGS_CLK_SUN65IW1_R_CCU_H_
#define _DT_BINDINGS_CLK_SUN65IW1_R_CCU_H_

#define CLK_R_AHB		0
#define CLK_R_APB0		1
#define CLK_R_APB1		2
#define CLK_R_TIMER0		3
#define CLK_R_TIMER1		4
#define CLK_R_TIMER2		5
#define CLK_R_TIMER3		6
#define CLK_BUS_R_TIMER		7
#define CLK_BUS_R_TWD		8
#define CLK_R_PWM		9
#define CLK_BUS_R_PWM		10
#define CLK_R_SPI		11
#define CLK_BUS_R_SPI		12
#define CLK_BUS_R_UART1		13
#define CLK_BUS_R_UART0		14
#define CLK_BUS_R_TWI2		15
#define CLK_BUS_R_TWI1		16
#define CLK_BUS_R_TWI0		17
#define CLK_BUS_R_PPU_VO0	18
#define CLK_BUS_R_PPU_HSI	19
#define CLK_BUS_R_PPU_VE	20
#define CLK_BUS_R_PPU_VI	21
#define CLK_BUS_R_TZMA		22
#define CLK_R_IR_RX		23
#define CLK_BUS_R_IR_RX		24
#define CLK_BUS_R_RTC		25
#define CLK_R_CPUS_24M		26
#define CLK_BUS_R_CPUS_CFG	27
#define CLK_BUS_R_CPUS_CORE	28
#define CLK_BUS_R_CPUIDLE	29

#define CLK_R_NUMBER		(CLK_BUS_R_CPUIDLE + 1)

#endif /* _DT_BINDINGS_CLK_SUN65IW1_R_CCU_H_ */
