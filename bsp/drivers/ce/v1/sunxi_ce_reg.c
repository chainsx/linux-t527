/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * The interface function of controlling the SS register.
 *
 * Copyright (C) 2013 Allwinner.
 *
 * Mintow <duanmintao@allwinnertech.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/types.h>
#include <linux/delay.h>
#include <asm/io.h>

#include "../sunxi_ce_cdev.h"
#include "sunxi_ce_reg.h"

inline u32 ss_readl(u32 offset)
{
	return readl(ss_membase() + offset);
}

inline void ss_writel(u32 offset, u32 val)
{
	writel(val, ss_membase() + offset);
}

u32 ss_reg_rd(u32 offset)
{
	return ss_readl(offset);
}

void ss_reg_wr(u32 offset, u32 val)
{
	ss_writel(offset, val);
}

void ss_sm_end_set(void)
{
	int val = ss_readl(SS_REG_CTL);

	val |= SS_REG_CTL_SM_END;
	ss_writel(SS_REG_CTL, val);
}

void ss_keyselect_set(int select)
{
	int val = ss_readl(SS_REG_CTL);

	val &= ~SS_REG_CTL_KEY_SELECT_MASK;
	val |= select << SS_REG_CTL_KEY_SELECT_SHIFT;
	ss_writel(SS_REG_CTL, val);
}

void ss_keysize_set(int size)
{
	int val = ss_readl(SS_REG_CTL);
	int type = SS_AES_KEY_SIZE_128;

	switch (size) {
	case AES_KEYSIZE_192:
		type = SS_AES_KEY_SIZE_192;
		break;
	case AES_KEYSIZE_256:
		type = SS_AES_KEY_SIZE_256;
		break;
	default:
		break;
	}

	val &= ~(SS_REG_CTL_KEY_SIZE_MASK);
	val |= (type << SS_REG_CTL_KEY_SIZE_SHIFT);
	ss_writel(SS_REG_CTL, val);
}

void ss_fifo_init(void)
{
	int val = ss_readl(SS_REG_FCSR);

	val &= ~(SS_REG_FCSR_RXFIFO_TRIG_LEVEL_MASK
			|SS_REG_FCSR_TXFIFO_TRIG_LEVEL_MASK);
	ss_writel(SS_REG_FCSR, val);
}

int ss_pending_get(void)
{
	int val = ss_readl(SS_REG_ICSR);

	/* val&=SS_REG_ICSR_RXFIFO_EMP_PENDING|SS_REG_ICSR_TXFIFO_AVA_PENDING;*/
	return val;
}

void ss_pending_clear(int flow)
{
	int val = ss_readl(SS_REG_ICSR);

	val |= SS_REG_ICSR_RXFIFO_EMP_PENDING|SS_REG_ICSR_TXFIFO_AVA_PENDING;
	ss_writel(SS_REG_ICSR, val);
}

void ss_irq_enable(int flow)
{
	int val = ss_readl(SS_REG_ICSR);

	val |= SS_REG_ICSR_RXFIFO_EMP_INT_ENABLE | SS_REG_ICSR_TXFIFO_AVA_INT_ENABLE;
	ss_writel(SS_REG_ICSR, val);
}

void ss_irq_disable(int flow)
{
	int val = ss_readl(SS_REG_ICSR);

	val &= ~(SS_REG_ICSR_RXFIFO_EMP_INT_ENABLE | SS_REG_ICSR_TXFIFO_AVA_INT_ENABLE);
	ss_writel(SS_REG_ICSR, val);
}

void ss_dma_enable(int flow)
{
	int val = ss_readl(SS_REG_ICSR);

	val |= SS_REG_ICSR_DRA_ENABLE;
	ss_writel(SS_REG_ICSR, val);
}

void ss_dma_disable(int flow)
{
	int val = ss_readl(SS_REG_ICSR);

	val &= ~SS_REG_ICSR_DRA_ENABLE;
	ss_writel(SS_REG_ICSR, val);
}

void ss_key_set(char *key, int size)
{
	int i = 0;
	int *val = (int *)key;
	int key_sel = CE_KEY_SELECT_INPUT;
	struct {
		int type;
		char desc[AES_MIN_KEY_SIZE];
	} keys[] = {
		{CE_KEY_SELECT_SSK,		   CE_KS_SSK},
		{CE_KEY_SELECT_HUK,		   CE_KS_HUK},

		{CE_KEY_SELECT_INTERNAL_0, CE_KS_INTERNAL_0},
		{CE_KEY_SELECT_INTERNAL_1, CE_KS_INTERNAL_1},
		{CE_KEY_SELECT_INTERNAL_2, CE_KS_INTERNAL_2},
		{CE_KEY_SELECT_INTERNAL_3, CE_KS_INTERNAL_3},
		{CE_KEY_SELECT_INTERNAL_4, CE_KS_INTERNAL_4},
		{CE_KEY_SELECT_INTERNAL_5, CE_KS_INTERNAL_5},
		{CE_KEY_SELECT_INTERNAL_6, CE_KS_INTERNAL_6},
		{CE_KEY_SELECT_INTERNAL_7, CE_KS_INTERNAL_7},
		{CE_KEY_SELECT_INPUT, ""} };

	while (keys[i].type != CE_KEY_SELECT_INPUT) {
		if (strncasecmp(key, keys[i].desc, AES_MIN_KEY_SIZE) == 0) {
			key_sel = keys[i].type;
			memset(key, 0, size);
			break;
		}
		i++;
	}
	SS_DBG("The key select: %d\n", key_sel);

	ss_keyselect_set(key_sel);
	ss_keysize_set(size);
	for (i = 0; i < size/4; i++, val++) {
		ss_writel(SS_REG_KEY(i), *val);
		/* SS_DBG("*val = 0x%08x\n", *val); */
	}
}

void ss_iv_set(char *iv, int size)
{
	int i;
	int *val = (int *)iv;

	for (i = 0; i < size/4; i++, val++) {
		ss_writel(SS_REG_IV(i), *val);
		/* SS_DBG("%d: *val = 0x%08x\n", i, *val); */
	}
}

void ss_cntsize_set(int size)
{
	int val = ss_readl(SS_REG_CTL);
	int cnt_width;

	switch (size) {
	case SS_CTR_SIZE_16:
		cnt_width = SS_CNT_WIDTH_16;
		break;
	case SS_CTR_SIZE_32:
		cnt_width = SS_CNT_WIDTH_32;
		break;
	case SS_CTR_SIZE_64:
		cnt_width = SS_CNT_WIDTH_64;
		break;
	case SS_CTR_SIZE_128:
		cnt_width = SS_CNT_WIDTH_128;
		break;
	default:
		break;
	}

	val &= ~SS_REG_CTL_CTR_SIZE_MASK;
	val |= cnt_width << SS_REG_CTL_CTR_SIZE_SHIFT;
	ss_writel(SS_REG_CTL, val);
}

void ss_cnt_set(char *cnt, int size)
{
	int i;
	int *val = (int *)cnt;

	ss_cntsize_set(size);
	for (i = 0; i < size / 4; i++, val++) {
		ss_writel(SS_REG_CNT(i), *val);
		SS_DBG("*val = 0x%08x\n", *val);
	}
}

void ss_cnt_get(int flow, char *cnt, int size)
{
	int i;
	int *val = (int *)cnt;
	int base = SS_REG_CNT_BASE;

	for (i = 0; i < size / 4; i++, val++)
		*val = ss_readl(base + i*4);
}

void ss_md_get(char *dst, char *src, int size)
{
	int i;
	int *val = (int *)dst;

	for (i = 0; i < size/4; i++, val++) {
		*val = ss_readl(SS_REG_MD(i));
		/* SS_DBG("%d: *val = 0x%08x\n", i, *val); */
	}
}

void ss_iv_mode_set(int mode)
{
	int val = ss_readl(SS_REG_CTL);

	val &= ~SS_REG_CTL_IV_MODE_MASK;
	val |= mode << SS_REG_CTL_IV_MODE_SHIFT;
	ss_writel(SS_REG_CTL, val);
}

void ss_method_set(int dir, int type)
{
	int val = ss_readl(SS_REG_CTL);

	val &= ~(SS_REG_CTL_OP_DIR_MASK|SS_REG_CTL_METHOD_MASK);
	val |= dir << SS_REG_CTL_OP_DIR_SHIFT;
	val |= type << SS_REG_CTL_METHOD_SHIFT;
	ss_writel(SS_REG_CTL, val);
}

void ss_aes_mode_set(int mode)
{
	int val = ss_readl(SS_REG_CTL);

	val &= ~SS_REG_CTL_OP_MODE_MASK;
	val |= mode << SS_REG_CTL_OP_MODE_SHIFT;
	ss_writel(SS_REG_CTL, val);
}

void ss_rng_mode_set(int mode)
{
	int val = ss_readl(SS_REG_CTL);

	val &= ~SS_REG_CTL_PRNG_MODE_MASK;
	val |= mode << SS_REG_CTL_PRNG_MODE_SHIFT;
	ss_writel(SS_REG_CTL, val);
}

void ss_crc_cfg_set(int xorout, int refout, int refin, int init, int width)
{
	int val = ss_readl(SS_REG_CTL);

	val &= ~(SS_REG_CTL_CRCCFG_MASK | SS_REG_CTL_WIDTH_SHIFT);
	val |= xorout << SS_REG_CTL_XOROUT_SHIFT;
	val |= refout << SS_REG_CTL_REFOUT_SHIFT;
	val |= refin << SS_REG_CTL_REFIN_SHIFT;
	val |= init << SS_REG_CTL_INIT_SHIFT;
	val |= width << SS_REG_CTL_WIDTH_SHIFT;
	ss_writel(SS_REG_CTL, val);
}

static int ss_txfifo_data_cnt(void)
{
	int val = ss_readl(SS_REG_FCSR);

	val &= SS_REG_FCSR_TXFIFO_AVA_CNT_MASK;
	return val >> SS_REG_FCSR_TXFIFO_AVA_CNT_SHIFT;
}

void ss_sha_final(void)
{
	int val = ss_readl(SS_REG_CTL);

	val |= SS_REG_CTL_SHA_END_MASK;
	ss_writel(SS_REG_CTL, val);
}

void ss_check_sha_end(void)
{
	while (ss_readl(SS_REG_CTL) & SS_REG_CTL_SHA_END_MASK) {
		/* SS_DBG("Need wait for the hardware.\n"); */
		msleep(10);
	}
}

void ss_ctrl_start(void)
{
	int val = ss_readl(SS_REG_CTL);

	val |= SS_CTL_START;
	ss_writel(SS_REG_CTL, val);
}

void ss_ctrl_stop(void)
{
	int val = ss_readl(SS_REG_CTL);

	val &= ~SS_CTL_START;
	ss_writel(SS_REG_CTL, val);
}

int ss_random_rd(char *data, int len)
{
	int i;
	int val = 0;
	int tail = len%4;
	int *random = (int *)data;
	int retlen = 0;

	for (i = 0; i < (len/4); i++) {
		while (ss_txfifo_data_cnt() == 0)
			msleep(1);

		random[i] = ss_readl(SS_REG_TXFIFO);
		retlen += 4;
	}

	if (tail != 0) {
		while (ss_txfifo_data_cnt() == 0)
			msleep(1);

		val = ss_readl(SS_REG_TXFIFO);
		memcpy(&data[retlen], &val, tail);
		retlen += tail;
	}
	return retlen;
}

void ss_wait_idle(void)
{
}

int ss_reg_print(char *buf, int len)
{
	return snprintf(buf, len,
"The SS control register:\n"
"[CTL] 0x%02x    = 0x%08x\n"
"[KEY0-7] 0x%02x = 0x%08x, 0x%08x, 0x%08x, 0x%08x\n"
"                0x%08x, 0x%08x, 0x%08x, 0x%08x\n"
"[IV0-3] 0x%02x  = 0x%08x, 0x%08x, 0x%08x, 0x%08x\n"
"[CNT0-3] 0x%02x = 0x%08x, 0x%08x, 0x%08x, 0x%08x\n"
"[FCSR] 0x%02x   = 0x%08x, [ICSR] 0x%02x = 0x%08x\n"
"[MD0-4] 0x%02x  = 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x\n"
"[RXFIFO] 0x%02x = 0x%08x, [TXFIFO] 0x%02x = 0x%08x\n",
		SS_REG_CTL, ss_readl(SS_REG_CTL),
		SS_REG_KEY(0),
		ss_readl(SS_REG_KEY(0)), ss_readl(SS_REG_KEY(1)),
		ss_readl(SS_REG_KEY(2)), ss_readl(SS_REG_KEY(3)),
		ss_readl(SS_REG_KEY(4)), ss_readl(SS_REG_KEY(5)),
		ss_readl(SS_REG_KEY(6)), ss_readl(SS_REG_KEY(7)),
		SS_REG_IV(0),
		ss_readl(SS_REG_IV(0)), ss_readl(SS_REG_IV(1)),
		ss_readl(SS_REG_IV(2)), ss_readl(SS_REG_IV(3)),
		SS_REG_CNT(0),
		ss_readl(SS_REG_CNT(0)), ss_readl(SS_REG_CNT(1)),
		ss_readl(SS_REG_CNT(2)), ss_readl(SS_REG_CNT(3)),
		SS_REG_FCSR, ss_readl(SS_REG_FCSR),
		SS_REG_ICSR, ss_readl(SS_REG_ICSR),
		SS_REG_MD(0),
		ss_readl(SS_REG_MD(0)), ss_readl(SS_REG_MD(1)),
		ss_readl(SS_REG_MD(2)), ss_readl(SS_REG_MD(3)),
		ss_readl(SS_REG_MD(4)),
		SS_REG_RXFIFO, ss_readl(SS_REG_RXFIFO),
		SS_REG_TXFIFO, ss_readl(SS_REG_TXFIFO));
}
