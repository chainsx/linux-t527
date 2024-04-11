/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * include/boot_param.h
 *
 * Copyright (c) 2021-2025 Allwinnertech Co., Ltd.
 * Author: lujianliang <lujianliang@allwinnertech.com>
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
#ifndef __BOOT_PARAM_H
#define __BOOT_PARAM_H

#include <linux/mtd/spi-nor.h>

#define BOOT_PARAM_MAGIC		"bootpara"
#define BOOT_PARAM_SIZE			4096
#define CHECK_SUM			0x5F0A6C39
#define CONFIG_SPINOR_UBOOT_OFFSET	128 /* sectors */

struct sunxi_boot_parameter_header {
	u8 magic[8]; //bootpara
	u32 version; // describe the region version
	u32 check_sum;
	u32 length;
	u8 reserved[12];
};

//---total len is 4K
struct sunxi_boot_param_region {
	struct sunxi_boot_parameter_header header;//32
	char sdmmc_info[256];
	char nand_info[256];
	char spiflash_info[256];
	char ddr_info[512];
	u8 reserved[2784];// = 4096 - sdmmc_size - nand_size - spi_size - ddr_size - 32
};

#define SPINOR_BOOT_PARAM_MAGIC		"NORPARAM"
typedef struct {
	u8			magic[8];
	__s32			readcmd;
	__s32			read_mode;
	__s32			write_mode;
	__s32			flash_size;
	__s32			addr4b_opcodes;
	__s32			erase_size;
	__s32			delay_cycle;/*When the frequency is greater than 60MHZ configured as 1;less than 24MHZ configured as 2;greater 24MHZ and less 60HZ as 3*/
	__s32			lock_flag;
	__s32			frequency;
	unsigned int		sample_delay;
	unsigned int		sample_mode;
	enum spi_nor_protocol	read_proto;
	enum spi_nor_protocol	write_proto;
	u8			read_dummy;
} boot_spinor_info_t;

/******************************************************************************/
/*                              head of Boot0                                 */
/******************************************************************************/
#define BOOT0_MAGIC                     "eGON.BT0"
typedef struct _normal_gpio_cfg {
	unsigned char port;
	unsigned char port_num;
	char mul_sel;
	char pull;
	char drv_level;
	char data;
	unsigned char reserved[2];
} normal_gpio_cfg;

typedef struct _boot0_private_head_t {
	unsigned int prvt_head_size;
	char prvt_head_vsn[4];        /* the version of boot0_private_head_t */
	unsigned int dram_para[32];   /* Original values is arbitrary */
	int uart_port;
	normal_gpio_cfg uart_ctrl[2];
	int enable_jtag;              /* 1 : enable,  0 : disable */
	normal_gpio_cfg jtag_gpio[5];
	normal_gpio_cfg storage_gpio[32];
	char storage_data[512 - sizeof(normal_gpio_cfg) * 32];
}
boot0_private_head_t;

typedef struct standard_Boot_file_head {
	unsigned int jump_instruction;  /* one intruction jumping to real code */
	unsigned char magic[8];         /* ="eGON.BT0" or "eGON.BT1",  not C-style string */
	unsigned int check_sum;         /* generated by PC */
	unsigned int length;            /* generated by PC */
	unsigned int pub_head_size;     /* size of boot_file_head_t */
	unsigned char pub_head_vsn[4];  /* version of boot_file_head_t */
	unsigned char file_head_vsn[4]; /* version of boot0_file_head_t or boot1_file_head_t */
	unsigned char Boot_vsn[4];      /* Boot version */
	unsigned char eGON_vsn[4];      /* eGON version */
	unsigned char platform[8];      /* platform information */
} standard_boot_file_head_t;

typedef struct _boot0_file_head_t {
	standard_boot_file_head_t boot_head;
	boot0_private_head_t prvt_head;
} boot0_file_head_t;

#endif