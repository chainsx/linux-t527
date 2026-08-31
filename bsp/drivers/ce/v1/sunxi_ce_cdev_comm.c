/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * The driver of SUNXI SecuritySystem controller.
 *
 * Copyright (C) 2013 Allwinner.
 *
 * Mintow <duanmintao@allwinnertech.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <crypto/internal/hash.h>
#include <crypto/internal/rng.h>
#include <linux/delay.h>
#include <linux/random.h>

#include "../sunxi_ce_cdev.h"
#include "sunxi_ce_reg.h"

/* Reverse the bytes in a 64-bit value */
#define SS_SWAP64(val) \
	((u64)((((u64)(val) & 0x00000000000000ffULL) << 56) | \
			(((u64)(val) & 0x000000000000ff00ULL) << 40) | \
			(((u64)(val) & 0x0000000000ff0000ULL) << 24) | \
			(((u64)(val) & 0x00000000ff000000ULL) <<  8) | \
			(((u64)(val) & 0x000000ff00000000ULL) >>  8) | \
			(((u64)(val) & 0x0000ff0000000000ULL) >> 24) | \
			(((u64)(val) & 0x00ff000000000000ULL) >> 40) | \
			(((u64)(val) & 0xff00000000000000ULL) >> 56)))

static void ss_ce_reg_clear(void)
{
	int i;

	for (i = 0; i <= SS_REG_ICSR; i += 4) {
		ss_reg_wr(i, 0x0);
	}
}

static int ss_aes_enc_dec(uint8_t *input, uint8_t *output, uint32_t size, uint8_t block_size)
{
	uint32_t i, j;

	ss_ctrl_start();
	for (i = 0; i < size; i += block_size) {
		for (j = 0; j < block_size; j += 4) {
			while (!(ss_reg_rd(SS_REG_FCSR) & SS_REG_FCSR_RXFIFO_STATUS))
				;
			ss_reg_wr(SS_REG_RXFIFO, *((uint32_t *)&input[i + j]));
		}
		for (j = 0; j < block_size; j += 4) {
			while (!(ss_reg_rd(SS_REG_FCSR) & SS_REG_FCSR_TXFIFO_STATUS))
				;
			*((uint32_t *)&output[i + j]) = ss_reg_rd(SS_REG_TXFIFO);
		}
	}
	return 0;
}

int do_aes_crypto(crypto_aes_req_ctx_t *req_ctx)
{
	uint32_t align_len = req_ctx->src_length & (~(AES_BLOCK_SIZE - 1));
	uint32_t remain_len = req_ctx->src_length & (AES_BLOCK_SIZE - 1);
	uint8_t padding[16] = {0};

	ss_pending_clear(0);
	ss_fifo_init();

	ss_method_set(req_ctx->dir, req_ctx->method);
	ss_aes_mode_set(req_ctx->aes_mode);

	SS_DBG("Flow: %d, Dir: %d, Method: %d, Mode: %d, len: %d\n",
		0, req_ctx->dir, req_ctx->method, req_ctx->aes_mode, req_ctx->src_length);

	ss_key_set(req_ctx->key_buffer, req_ctx->key_length);

	if ((req_ctx->aes_mode == SS_AES_MODE_CBC || req_ctx->aes_mode == SS_AES_MODE_CTS)
						&& (req_ctx->iv_buf != NULL)) {
		ss_iv_set(req_ctx->iv_buf, req_ctx->iv_length);
	}

	if (req_ctx->aes_mode == SS_AES_MODE_CTR)
		ss_cnt_set(req_ctx->iv_buf, req_ctx->bit_width);

	SS_DBG("align_len = %d, remain_len = %d, len = %d\n",
			align_len, remain_len, req_ctx->src_length);
	if (align_len) {
		if (req_ctx->aes_mode == SS_AES_MODE_CTS)
			ss_reg_wr(SS_REG_CTS_LEN, align_len + 32);
		ss_aes_enc_dec((uint8_t *)req_ctx->src_buffer, (uint8_t *)req_ctx->dst_buffer, align_len, AES_BLOCK_SIZE);
	}
	if (remain_len) {
		if (req_ctx->aes_mode == SS_AES_MODE_CTS)
			ss_reg_wr(SS_REG_CTS_LEN, AES_BLOCK_SIZE);
		memcpy(padding, req_ctx->src_buffer + align_len, remain_len);
		ss_aes_enc_dec(padding, (uint8_t *)req_ctx->dst_buffer + align_len, AES_BLOCK_SIZE, AES_BLOCK_SIZE);
	}

	ss_pending_clear(0);

	ss_ctrl_stop();

	ss_ce_reg_clear();

	SS_DBG("aes crypto sucess\n");

	return 0;
}

static int ss_hash_calc(u8 *msge, u32 msglen, u32 md_size)
{
	u8 padding[128];
	u32 remain = msglen & 0x3f; // len % 64
	u32 paddings = remain < 56 ? (64 - remain - 8) : (120 - remain - 8);
	u64 bit_size = ((u64)msglen) << 3;
	int i;

	/* init padding data */
	memset(padding, 0, 128);
	/* copy tail data */
	memcpy(padding, (void *)&msge[msglen & (~0x3f)], remain);

	/* fill last padding data */
	padding[remain] = 0x80;

	/* fill data bit size */
	if (md_size == 20 || md_size == 32)
		bit_size = SS_SWAP64(bit_size);

	memcpy(&padding[remain + paddings], &bit_size, 8);

	ss_ctrl_start();

	for (i = 0; i < (msglen & (~0x3f)); i += 4) {
		while (!(ss_reg_rd(SS_REG_FCSR) & SS_REG_FCSR_RXFIFO_STATUS))
			;
		ss_reg_wr(SS_REG_RXFIFO, *((u32 *)&msge[i]));
		SS_DBG("text is 0x%x\n", *((u32 *)&msge[i]));
	}
	for (i = 0; i < (remain + paddings + 8); i += 4) {
		while (!(ss_reg_rd(SS_REG_FCSR) & SS_REG_FCSR_RXFIFO_STATUS))
			;
		ss_reg_wr(SS_REG_RXFIFO, *((u32 *)&padding[i]));
		SS_DBG("remain text %d is 0x%8x\n", i, *((u32 *)&padding[i]));
	}

	ss_sha_final();
	ss_check_sha_end();

	return 0;
}

int do_hash_crypto(crypto_hash_req_ctx_t *req)
{
	u32 hash_type[][2] = {
		/* v5 hash mode, v1 hash mode */
		{0, 4},
		{1, 3},
		{2, 6},
		{3, 6},
	};

	SS_DBG("Flow: %d, Mode: %d, type: %d, text len is %d, degist len: %d\n",
		0, req->hash_mode, hash_type[req->hash_mode][1], req->text_length, req->dst_length);
	if ((req->hash_mode == 2) || (req->hash_mode > 3)) {
		pr_err("hash mode is not supported. Ensure that your hash mode is md5, sha1, or sha256\n");
		return -1;
	}
	ss_pending_clear(0);
	ss_fifo_init();

	ss_method_set(SS_DIR_ENCRYPT, hash_type[req->hash_mode][1]);
	ss_iv_mode_set(SS_IV_MODE_CONSTANT);

	ss_hash_calc(req->text_buffer, req->text_length, req->dst_length);

	ss_md_get(req->dst_buffer, NULL, req->dst_length);

	ss_pending_clear(0);

	ss_ctrl_stop();
	ss_ce_reg_clear();

	return 0;
}

static void ss_crc_calc(uint32_t *input, uint32_t *output, u32 length)
{
	u32 i, last;

	ss_reg_wr(SS_REG_CTS_LEN, length);
	ss_ctrl_start();

	for (i = 0; i < (length & (~3)); i += 4) {
		while (!(ss_reg_rd(SS_REG_FCSR) & SS_REG_FCSR_RXFIFO_STATUS))
			;
		ss_reg_wr(SS_REG_RXFIFO, *((uint32_t *)&input[i]));
	}

	if (length & 3) {
		memcpy(&last, input + i, length & 3);
		ss_reg_wr(SS_REG_RXFIFO, last);
	}

	ss_reg_wr(SS_REG_ICSR, 0x1 << 3);
	ss_sm_end_set();

	udelay(10);

	*output = ss_reg_rd(SS_REG_CRC_RESULT);
	ss_pending_clear(0);
	ss_ctrl_stop();
}

int do_crc_crypto(crypto_crc_req_ctx_t *req_ctx)
{
	SS_DBG("src = 0x%x, src-len = %d, dst-len = %d\n", *req_ctx->src_buffer, req_ctx->src_length, req_ctx->dst_length);
	ss_pending_clear(0);
	ss_fifo_init();

	ss_method_set(req_ctx->dir, SS_METHOD_CRC);
	ss_reg_wr(SS_REG_CRC_POLY, req_ctx->poly);
	ss_crc_cfg_set(req_ctx->xorout, req_ctx->refout, req_ctx->refin,
					req_ctx->init, req_ctx->width);

	ss_crc_calc((uint32_t *)req_ctx->src_buffer, (uint32_t *)req_ctx->dst_buffer, req_ctx->src_length);

	ss_ce_reg_clear();

	return 0;
}

static void ss_prng_start(void)
{
	int val = ss_reg_rd(SS_REG_CTL);

	val |= SS_PRNG_START;
	ss_reg_wr(SS_REG_CTL, val);

	while (ss_reg_rd(SS_REG_CTL) & SS_PRNG_START)
		;

	SS_DBG("gen prng success\n");
}

static void ss_prng_seed(void)
{
	int i;
	u32 seed[SS_PRNG_SEED_NUM];

	get_random_bytes((u8 *)seed, SS_PRNG_SEED_NUM);
	for (i = 0; i < SS_PRNG_SEED_NUM / sizeof(u32); i++) {
		ss_reg_wr(SS_REG_KEY(i), seed[i]);
	}
}

int do_rng_crypto(crypto_rng_req_ctx_t *req)
{
	int addr_offset;
	u32 random[SS_PRNG_MAX_RAND_LEN / sizeof(u32)] = {0};
	int need_get_len = req->dst_length;

	if (!req->dst_buffer) {
		pr_err("input pointer is NULL\n");
		return -1;
	}

	ss_method_set(SS_DIR_ENCRYPT, SS_METHOD_PRNG);
	ss_rng_mode_set(SS_RNG_MODE_ONESHOT);

	for (addr_offset = 0; addr_offset < req->dst_length; addr_offset += SS_PRNG_MAX_RAND_LEN) {
		ss_prng_seed();
		ss_ctrl_start();
		ss_prng_start();

		ss_md_get((char *)random, NULL, SS_PRNG_MAX_RAND_LEN);
		ss_ctrl_stop();
		SS_DBG("prng get data is 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n", random[0], random[1], random[2], random[3], random[4]);

		if (need_get_len > SS_PRNG_MAX_RAND_LEN) {
			memcpy(req->dst_buffer + addr_offset, (u8 *)random, SS_PRNG_MAX_RAND_LEN * sizeof(u8));
			need_get_len -= SS_PRNG_MAX_RAND_LEN;
		} else {
			memcpy(req->dst_buffer + addr_offset, (u8 *)random, need_get_len * sizeof(u8));
			break;
		}
	}

	ss_ce_reg_clear();

	return 0;
}

irqreturn_t sunxi_ce_irq_handler(int irq, void *dev_id)
{
	/* sunxi_ce_cdev_t *sss = (sunxi_ce_cdev_t *)dev_id; */
	int pending = 0;

	pending = ss_pending_get();
	ss_pending_clear(0);
	/* SS_DBG("SS pending %#x\n", pending); */

	return IRQ_HANDLED;
}
