/* SPDX-License-Identifier: GPL-2.0-or-later */
/* Copyright(c) 2020 - 2023 Allwinner Technology Co.,Ltd. All rights reserved. */
/*
 * The driver of SUNXI SecuritySystem controller.
 *
 * Copyright (C) 2014 Allwinner.
 *
 * Mintow <duanmintao@allwinnertech.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/vmalloc.h>
#include <linux/spinlock.h>
#include <linux/platform_device.h>
#include <crypto/internal/hash.h>
#include <crypto/internal/rng.h>
#include <crypto/internal/aead.h>
#include <crypto/hash.h>

#include <linux/dmaengine.h>
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>

#include "../sunxi_ce_cdev.h"
#include "sunxi_ce_reg.h"

#define NO_DMA_MAP		(0xE7)
#define SRC_DATA_DIR	(0)
#define DST_DATA_DIR	(0x1)

extern sunxi_ce_cdev_t	*ce_cdev;

irqreturn_t sunxi_ce_irq_handler(int irq, void *dev_id)
{
	int i;
	int pending = 0;
	sunxi_ce_cdev_t *p_cdev = (sunxi_ce_cdev_t *)dev_id;

	pending = ss_pending_get();
	SS_DBG("pending: %#x\n", pending);
	for (i = 0; i < SS_FLOW_NUM; i++) {
		if (pending & (CE_CHAN_PENDING << (2 * i))) {
			SS_DBG("Chan %d completed. pending: %#x\n", i, pending);
			ss_pending_clear(i);
			complete(&p_cdev->flows[i].done);
		}
	}

	return IRQ_HANDLED;
}

void ce_task_desc_print(ce_task_desc_t *task)
{
	int i;
	u64 phy_addr;

#ifndef SUNXI_CE_DEBUG
	return;
#endif

	SS_DBG("---------------------task_info--------------------\n");
	SS_DBG("[channel_id] 0x%x\n", task->chan_id);
	SS_DBG("task->comm_ctl = 0x%x\n", task->comm_ctl);
	SS_DBG("task->sym_ctl = 0x%x\n", task->sym_ctl);
	SS_DBG("task->asym_ctl = 0x%x\n", task->asym_ctl);
	SS_DBG("task->key_addr = 0x%x\n", task->key_addr);
	SS_DBG("task->iv_addr = 0x%x\n", task->iv_addr);
	SS_DBG("task->ctr_addr = 0x%x\n", task->ctr_addr);
	SS_DBG("task->data_len = 0x%x\n", task->data_len);

	for (i = 0; i < 8; i++) {
		phy_addr = (u64)ce_task_addr_get((u8 *)&(task->src[i].addr));
		if (phy_addr) {
			SS_DBG("task->src[%d].addr = 0x%llx\n", i, phy_addr);
			SS_DBG("task->src[%d].len = 0x%x\n", i, task->src[i].len);
		}
	}

	for (i = 0; i < 8; i++) {
		phy_addr = (u64)ce_task_addr_get((u8 *)&(task->dst[i].addr));
		if (phy_addr) {
			SS_DBG("task->dst[%d].addr = 0x%llx\n", i, phy_addr);
			SS_DBG("task->dst[%d].len = 0x%x\n", i, task->dst[i].len);
		}
	}
	SS_DBG("task->task_phy_addr = 0x%llx\n", (u64)task->task_phy_addr);
}

static int check_aes_ctx_vaild(crypto_aes_req_ctx_t *req)
{
	if (!req->src_buffer || !req->dst_buffer || !req->key_buffer) {
		SS_ERR("Invalid para: src = 0x%px, dst = 0x%px key = 0x%p\n",
				req->src_buffer, req->dst_buffer, req->key_buffer);
		return -EINVAL;
	}

	if (req->iv_length) {
		if (!req->iv_buf) {
			SS_ERR("Invalid para: iv_buf = 0x%px\n", req->iv_buf);
			return -EINVAL;
		}
	}

	SS_DBG("key_length = %d\n", req->key_length);
	if (req->key_length > AES_MAX_KEY_SIZE) {
		SS_ERR("Invalid para: key_length = %d\n", req->key_length);
		return -EINVAL;
	} else if (req->key_length < AES_MIN_KEY_SIZE) {
		SS_ERR("Invalid para: key_length = %d\n", req->key_length);
		return -EINVAL;
	}

	return 0;
}

static void ce_aes_config(crypto_aes_req_ctx_t *req, ce_task_desc_t *task)
{
	u8 ctr_addr[16] = {0};

	task->chan_id = req->channel_id;
	ss_method_set(req->dir, SS_METHOD_AES, task);
	ss_aes_mode_set(req->aes_mode, task);

	if (req->aes_mode == SS_AES_MODE_CFB)
		ss_cfb_bitwidth_set(req->bit_width, task);
	else if (req->aes_mode == SS_AES_MODE_CTR)
		ss_ctr_bitwidth_set(ctr_addr, req->bit_width, task);
	else
		SS_DBG("the current mode does not need to set bit_width\n");
}

static void task_iv_init(crypto_aes_req_ctx_t *req, ce_task_desc_t *task, int flag)
{
	if (req->iv_length) {
		if (flag == DMA_TO_DEVICE) {
			ss_iv_set(req->iv_buf, req->iv_length, task);
			req->iv_phy = dma_map_single(ce_cdev->pdevice, req->iv_buf,
									req->iv_length, DMA_TO_DEVICE);
			SS_DBG("iv = %px, iv_phy_addr = 0x%lx\n", req->iv_buf, req->iv_phy);
		} else if (flag == DMA_FROM_DEVICE) {
			dma_unmap_single(ce_cdev->pdevice,
				req->iv_phy, req->iv_length, DMA_FROM_DEVICE);
		} else if (flag == NO_DMA_MAP) {
			task->iv_addr = (req->iv_phy >> WORD_ALGIN);
			SS_DBG("iv_phy_addr = 0x%lx\n", req->iv_phy);
		}
	}
	return;
}

static ce_task_desc_t *ce_task_alloc(void)
{
	dma_addr_t task_phy_addr;
	ce_task_desc_t *task;

	task = dma_pool_zalloc(ce_cdev->task_pool, GFP_KERNEL, &task_phy_addr);
	if (task == NULL) {
		SS_ERR("Failed to alloc for task\n");
		return NULL;
	} else {
		task->next_virt = NULL;
		task->task_phy_addr = task_phy_addr;
		SS_DBG("task = 0x%px task_phy = 0x%px\n", task, (void *)task_phy_addr);
	}

	return task;
}

static void ce_task_destroy(ce_task_desc_t *task)
{
	ce_task_desc_t *prev;

	while (task != NULL) {
		prev = task;
		task = task->next_virt;
		SS_DBG("prev = 0x%px, prev_phy = 0x%px\n", prev, (void *)prev->task_phy_addr);
		dma_pool_free(ce_cdev->task_pool, prev, prev->task_phy_addr);
	}
	return;
}

static int ce_task_data_init(crypto_aes_req_ctx_t *req, phys_addr_t src_phy,
						phys_addr_t dst_phy, phys_addr_t pad_phy, u32 length, ce_task_desc_t *task)
{
	u32 block_size = 127 * 1024;
	u32 block_size_word = (block_size >> 2);
	u32 block_num, alloc_flag = 0;
	u32 last_data_len, last_size;
	u32 data_len_offset = 0;
	u32 i = 0, n;
	dma_addr_t ptask_phy;
	dma_addr_t next_iv_phy;
	ce_task_desc_t *ptask = task, *prev;

	block_num = length / block_size;
	last_size = length % block_size;
	ptask->data_len = 0;
	SS_DBG("total_len = 0x%x block_num =%d last_size =%d\n", length, block_num, last_size);
	while (length) {

		if (alloc_flag) {
			ptask = dma_pool_zalloc(ce_cdev->task_pool, GFP_KERNEL, &ptask_phy);
			if (ptask == NULL) {
				SS_ERR("Failed to alloc for ptask\n");
				return -ENOMEM;
			}
			ptask->chan_id  = prev->chan_id;
			ptask->comm_ctl = prev->comm_ctl;
			ptask->sym_ctl  = prev->sym_ctl;
			ptask->asym_ctl = prev->asym_ctl;
			ptask->key_addr = (prev->key_addr >> WORD_ALGIN);
			ptask->iv_addr = (prev->iv_addr >> WORD_ALGIN);
			ptask->data_len = 0;
			prev->next_task_addr = (ptask_phy >> WORD_ALGIN);
			prev->next_virt = ptask;
			ptask->task_phy_addr = ptask_phy;

			SS_DBG("ptask = 0x%px, ptask_phy = 0x%px\n", ptask, (void *)ptask_phy);

			if (SS_AES_MODE_CBC == req->aes_mode) {
				req->iv_phy = next_iv_phy;
				task_iv_init(req, ptask, NO_DMA_MAP);
			}
			i = 0;
		}

		if (block_num) {
			n = (block_num > 8) ? CE_SCATTERS_PER_TASK : block_num;
			for (i = 0; i < n; i++) {
				ptask->src[i].addr = ((src_phy + data_len_offset) >> WORD_ALGIN);
				ptask->src[i].len = block_size_word;
				ptask->dst[i].addr = ((dst_phy + data_len_offset) >> WORD_ALGIN);
				ptask->dst[i].len = block_size_word;
				ptask->data_len += block_size;
				data_len_offset += block_size;
			}
			block_num = block_num - n;
		}

		SS_DBG("block_num =%d i =%d\n", block_num, i);

		/* the last no engure block size */
		if ((block_num == 0) && (last_size == 0)) {	/* block size aglin */
			ptask->comm_ctl |= CE_COMM_CTL_TASK_INT_MASK;
			alloc_flag = 0;
			ptask->next_task_addr = 0;
			break;
		} else if ((block_num == 0) && (last_size != 0)) {
			SS_DBG("last_size =%d data_len_offset= %d\n", last_size, data_len_offset);
			/* not block size aglin */
			if ((i < CE_SCATTERS_PER_TASK) && (data_len_offset < length)) {
				last_data_len = length - data_len_offset;

				ptask->src[i].addr = ((src_phy + data_len_offset) >> WORD_ALGIN);
				ptask->src[i].len = ((last_data_len + req->padding_len) >> 2);	/* len in word */
				ptask->dst[i].addr = ((dst_phy + data_len_offset) >> WORD_ALGIN);
				ptask->dst[i].len = ((last_data_len + req->padding_len) >> 2);	/* len in word */
				ptask->data_len += last_data_len;
				ptask->comm_ctl |= CE_COMM_CTL_TASK_INT_MASK;
				ptask->next_task_addr = 0;
				break;
			}
		}

		if (req->dir == SS_DIR_ENCRYPT) {
			next_iv_phy = ptask->dst[7].addr + (ptask->dst[7].len << 2) - 16;
		} else {
			next_iv_phy = ptask->src[7].addr + (ptask->src[7].len << 2) - 16;
		}
		alloc_flag = 1;
		prev = ptask;
	}

	if (!length) {
		pad_phy = dma_map_single(ce_cdev->pdevice, req->padding, req->padding_len, DMA_TO_DEVICE);
		ptask->src[0].addr = pad_phy >> WORD_ALGIN;
		ptask->src[0].len = req->padding_len >> 2;  /* len in word */
		ptask->dst[0].addr = (dst_phy >> WORD_ALGIN);
		ptask->dst[0].len = (req->padding_len >> 2);  /* len in word */
		ptask->data_len += (req->padding_len >> 2);  /* len in word */
		ptask->comm_ctl |= CE_COMM_CTL_TASK_INT_MASK;
		ptask->next_task_addr = 0;

		if (req->dir == SS_DIR_ENCRYPT) {
			next_iv_phy = ptask->dst[7].addr + (ptask->dst[7].len << 2) - 16;
		} else {
			next_iv_phy = ptask->src[7].addr + (ptask->src[7].len << 2) - 16;
		}
	}
	return 0;
}

void ce_print_task_desc(ce_task_desc_t *task)
{
	int i;
	u64 phy_addr;

#ifndef SUNXI_CE_DEBUG
	return;
#endif

	pr_info("---------------------task_info--------------------\n");
	pr_info("[channel_id] 0x%x\n", task->chan_id);
	pr_info("task->comm_ctl = 0x%x\n", task->comm_ctl);
	pr_info("task->sym_ctl = 0x%x\n", task->sym_ctl);
	pr_info("task->asym_ctl = 0x%x\n", task->asym_ctl);
	pr_info("task->key_addr = 0x%x\n", task->key_addr);
	pr_info("task->iv_addr = 0x%x\n", task->iv_addr);
	pr_info("task->ctr_addr = 0x%x\n", task->ctr_addr);
	pr_info("task->data_len = 0x%x\n", task->data_len);

	for (i = 0; i < 8; i++) {
		phy_addr = (u64)ce_task_addr_get((u8 *)&(task->src[i].addr));
		if (phy_addr) {
			pr_info("task->src[%d].addr = 0x%llx\n", i, phy_addr);
			pr_info("task->src[%d].len = 0x%x\n", i, task->src[i].len);
		}
	}

	for (i = 0; i < 8; i++) {
		phy_addr = (u64)ce_task_addr_get((u8 *)&(task->dst[i].addr));
		if (phy_addr) {
			pr_info("task->dst[%d].addr = 0x%llx\n", i, phy_addr);
			pr_info("task->dst[%d].len = 0x%x\n", i, task->dst[i].len);
		}
	}
	pr_info("task->task_phy_addr = 0x%llx\n", (u64)task->task_phy_addr);
}

static uint32_t ce_aes_sw_padding(crypto_aes_req_ctx_t *ctx)
{
	uint32_t blk_num = 0;
	uint32_t padding_size = 0;
	uint32_t last_blk_size = 0;

	blk_num = ctx->src_length / AES_BLOCK_SIZE;
	last_blk_size = ctx->src_length % AES_BLOCK_SIZE;

	if (last_blk_size) {
		padding_size = AES_BLOCK_SIZE - last_blk_size;
		memcpy(ctx->padding, ctx->src_buffer + blk_num * AES_BLOCK_SIZE, last_blk_size);
		memset(ctx->padding + last_blk_size, padding_size, padding_size);
		ctx->padding_len = AES_BLOCK_SIZE;
	} else {
		ctx->padding_len = 0;
	}

	return blk_num * AES_BLOCK_SIZE;
}

static int aes_crypto_start(crypto_aes_req_ctx_t *req, u8 *src_buffer,
							u32 src_length, u8 *dst_buffer)
{
	int ret = 0;
	int channel_id = req->channel_id;
	u32 padding_flag = ce_cdev->flows[req->channel_id].buf_pendding;
	phys_addr_t key_phy = 0;
	phys_addr_t src_phy = 0;
	phys_addr_t dst_phy = 0;
	phys_addr_t pad_phy = 0;
	ce_task_desc_t *task = NULL;
	uint32_t src_word_len = 0;

	task = ce_task_alloc();
	if (!task) {
		return -ENOMEM;
	}

	src_length = ce_aes_sw_padding(req);
	src_word_len = src_length >> 2;

	/* task_mode_set */
	ce_aes_config(req, task);

	/* task_key_set */
	if (req->key_length) {
		SS_DBG("key = 0x%px, key_phy_addr = 0x%px\n", req->key_buffer, (void *)key_phy);
		ss_key_set(req->key_buffer, req->key_length, task);
		key_phy = dma_map_single(ce_cdev->pdevice,
					req->key_buffer, req->key_length, DMA_TO_DEVICE);
	}

	SS_DBG("ion_flag = %d padding_flag =%d", req->ion_flag, padding_flag);
	/* task_iv_set */
	if (req->ion_flag && padding_flag) {
		task_iv_init(req, task, NO_DMA_MAP);
	} else {
		task_iv_init(req, task, DMA_TO_DEVICE);
	}

	/* task_data_set */
	/* only the last src_buf is malloc */
	if (req->ion_flag && (!padding_flag)) {
		src_phy = req->src_phy;
	} else {
		src_phy = dma_map_single(ce_cdev->pdevice, src_buffer, src_length, DMA_TO_DEVICE);
	}
	SS_DBG("src = 0x%px, src_phy_addr = 0x%px\n", src_buffer, (void *)src_phy);

	/* the dst_buf is from user */
	if (req->ion_flag) {
		dst_phy = req->dst_phy;
	} else {
		dst_phy = dma_map_single(ce_cdev->pdevice, dst_buffer, src_length + req->padding_len, DMA_FROM_DEVICE);
	}
	SS_DBG("dst = 0x%px, dst_phy_addr = 0x%px\n", dst_buffer, (void *)dst_phy);

	ce_task_data_init(req, src_phy, dst_phy, pad_phy, src_length, task);
	if (src_length && req->padding_len) {
		pad_phy = dma_map_single(ce_cdev->pdevice, req->padding, req->padding_len, DMA_TO_DEVICE);
		task->src[1].addr = pad_phy >> WORD_ALGIN;;
		task->src[1].len = req->padding_len >> 2; /*len in word*/
	}
	ce_print_task_desc(task);

	/* start ce */
	ss_pending_clear(channel_id);
	ss_irq_enable(channel_id);

	init_completion(&ce_cdev->flows[channel_id].done);
	ss_ctrl_start(task);
	/* ce_reg_print(); */


	ret = wait_for_completion_timeout(&ce_cdev->flows[channel_id].done,
		msecs_to_jiffies(SS_WAIT_TIME));
	if (ret == 0) {
		SS_ERR("Timed out\n");
		ce_reg_print();
		ce_task_destroy(task);
		ce_reset();
		ret = -ETIMEDOUT;
		goto out;
	}

	SS_DBG("After CE, TSR: 0x%08x, ERR: 0x%08x\n",
		ss_reg_rd(CE_REG_TSR), ss_reg_rd(CE_REG_ERR));

	if (ss_flow_err(channel_id)) {
		ce_print_task_desc(task);
		ce_reg_print();
		SS_ERR("CE return error: %d\n", ss_flow_err(channel_id));
		ret = -EINVAL;
		goto out;
	}

	ret = 0;
out:
	ss_irq_disable(channel_id);
	ce_task_destroy(task);

	/* key */
	if (req->key_length)
		dma_unmap_single(ce_cdev->pdevice, key_phy, req->key_length, DMA_TO_DEVICE);

	/* iv */
	task_iv_init(req, task, DMA_TO_DEVICE);

	/* data */
	dma_unmap_single(ce_cdev->pdevice, src_phy, req->src_length, DMA_TO_DEVICE);

	if (req->padding)
		dma_unmap_single(ce_cdev->pdevice, pad_phy, AES_BLOCK_SIZE, DMA_TO_DEVICE);

	dma_unmap_single(ce_cdev->pdevice, dst_phy, req->dst_length, DMA_FROM_DEVICE);

	return ret;
}

int do_aes_crypto(crypto_aes_req_ctx_t *req_ctx)
{
	u8 data_block[AES_BLOCK_SIZE];
	int channel_id = req_ctx->channel_id;
	int ret;

	ret = check_aes_ctx_vaild(req_ctx);
	if (ret) {
		return -EINVAL;
	}

	memset(data_block, 0x0, AES_BLOCK_SIZE);
	ce_cdev->flows[channel_id].buf_pendding = 0;

	ret = aes_crypto_start(req_ctx, req_ctx->src_buffer, req_ctx->src_length, req_ctx->dst_buffer);
	if (ret) {
		SS_ERR("aes %s fail\n", (req_ctx->dir == SS_DIR_DECRYPT) ? "decrypt" : "encrypt");
		return ret;
	}

	if (req_ctx->dir == SS_DIR_DECRYPT)
		req_ctx->dst_length = req_ctx->src_length;

	SS_ERR("do_aes_crypto sucess\n");
	return 0;
}

static int check_rsa_ctx_vaild(crypto_rsa_req_ctx_t *req)
{
	if (!req->sign_buffer || !req->pkey_buffer || !req->dst_buffer) {
		SS_ERR("Invalid para: src = 0x%px, dst = 0x%px key = 0x%p\n",
		       req->sign_buffer, req->pkey_buffer, req->dst_buffer);
		return -EINVAL;
	}

	if ((!req->sign_length) || (!req->pkey_length)) {
		SS_ERR("Invalid len: sign = 0x%x, pkey = 0x%x\n",
		       req->sign_length, req->pkey_length);
		return -EINVAL;
	}
	return 0;
}

static void ce_rsa_config(crypto_rsa_req_ctx_t *req, ce_task_desc_t *task)
{
	task->chan_id = req->channel_id;
	ss_method_set(req->dir, SS_METHOD_RSA, task);
	ss_rsa_width_set(req->rsa_width, task);
	task->comm_ctl |= 1 << CE_COMM_CTL_TASK_INT_SHIFT;
}

static void ce_task_rsa_init(crypto_rsa_req_ctx_t *req, phys_addr_t pkey_phy,
			     phys_addr_t sign_phy, phys_addr_t dst_phy,
			     ce_task_desc_t *ptask)
{
	ulong data_len = 0;

	ce_task_addr_set(0, pkey_phy, (u8 *)&(ptask->key_addr));
	data_len = (req->rsa_width) * CE_RSA_SRC_NUM;

	ce_task_addr_set(0, pkey_phy, (u8 *)&(ptask->src[0].addr));
	ptask->src[0].len = (req->pkey_length) >> WORD_ALGIN;	/* length in word */
	ce_task_addr_set(0, sign_phy, (u8 *)&(ptask->src[1].addr));
	ptask->src[1].len = (req->sign_length) >> WORD_ALGIN;	/* length in word */

	ce_task_addr_set(0, dst_phy, (u8 *)&(ptask->dst[0].addr));
	ptask->dst[0].len = (req->dst_length) >> WORD_ALGIN;	/* length in word */
	ss_data_len_set(data_len, ptask);
}

static void ce_rsa_ctx_print(crypto_rsa_req_ctx_t *rsa_ctx)
{
	crypto_rsa_req_ctx_t *rsa_req_ctx = rsa_ctx;
	SS_ERR("the rsa task");

	SS_ERR("[sign address] 0x%px\n", rsa_req_ctx->sign_buffer);
	SS_ERR("[sign length] %d\n", rsa_req_ctx->sign_length);

	SS_ERR("[pkey address] 0x%px\n", rsa_req_ctx->pkey_buffer);
	SS_ERR("[pkey length] %d\n", rsa_req_ctx->pkey_length);

	SS_ERR("[dst address] 0x%px\n", rsa_req_ctx->dst_buffer);
	SS_ERR("[dst length] %d\n", rsa_req_ctx->dst_length);

	SS_ERR("[dir] %d\n", rsa_req_ctx->dir);
	SS_ERR("[rsa width] %d\n", rsa_req_ctx->rsa_width);
	SS_ERR("[flag] %d\n", rsa_req_ctx->flag);
	SS_ERR("[channel_id] %d\n", rsa_req_ctx->channel_id);
}

static int rsa_crypto_start(crypto_rsa_req_ctx_t *req)
{
	int err;
	int channel_id = req->channel_id;
	phys_addr_t pkey_phy;
	phys_addr_t sign_phy;
	phys_addr_t dst_phy;
	ce_task_desc_t *task;

	task = ce_task_alloc();
	if (!task)
		return -ENOMEM;

	/* task_set */
	ce_rsa_config(req, task);
	pkey_phy = dma_map_single(ce_cdev->pdevice, req->pkey_buffer, req->pkey_length, DMA_TO_DEVICE);
	SS_DBG("pkey = 0x%px, pkey_phy_addr = 0x%px\n", req->pkey_buffer, (void *)pkey_phy);

	sign_phy = dma_map_single(ce_cdev->pdevice, req->sign_buffer, req->sign_length, DMA_TO_DEVICE);
	SS_DBG("sign = 0x%px, sign_phy_addr = 0x%px\n", req->sign_buffer, (void *)sign_phy);

	dst_phy = dma_map_single(ce_cdev->pdevice, req->dst_buffer, req->dst_length, DMA_FROM_DEVICE);
	SS_DBG("dst = 0x%px, dst_phy_addr = 0x%px\n", req->dst_buffer, (void *)dst_phy);

	ce_task_rsa_init(req, pkey_phy, sign_phy, dst_phy, task);
	/* start ce */
	ss_pending_clear(channel_id);
	ss_irq_enable(channel_id);
	ce_task_desc_print(task);

	init_completion(&ce_cdev->flows[channel_id].done);
	ss_ctrl_start(task);

	err = wait_for_completion_timeout(&ce_cdev->flows[channel_id].done, msecs_to_jiffies(SS_WAIT_TIME));
	if (!err) {
		SS_ERR("Timed out\n");
		ce_task_desc_print(task);
		ce_rsa_ctx_print(req);
		ce_reg_print();
		ce_reset();
		err = -ETIMEDOUT;
		goto out;
	}

	ss_irq_disable(channel_id);

	SS_DBG("After CE, TSR: 0x%08x, ERR: 0x%08x\n", ss_reg_rd(CE_REG_TSR), ss_reg_rd(CE_REG_ERR));

	err = ss_flow_err(channel_id);
	if (err) {
		ce_rsa_ctx_print(req);
		SS_ERR("CE return error: %d\n", err);
		err = -EINVAL;
		goto out;
	}

	err = 0;
out:
	ce_task_destroy(task);

	/* pkey */
	dma_unmap_single(ce_cdev->pdevice, pkey_phy, req->pkey_length, DMA_TO_DEVICE);
	/* data */
	dma_unmap_single(ce_cdev->pdevice, sign_phy, req->sign_length, DMA_TO_DEVICE);

	dma_unmap_single(ce_cdev->pdevice, dst_phy, req->dst_length, DMA_FROM_DEVICE);

	return err;
}

int do_rsa_crypto(crypto_rsa_req_ctx_t *req_ctx)
{
	int err;

	err = check_rsa_ctx_vaild(req_ctx);
	if (err)
		return err;

	if (req_ctx->dir == SS_DIR_ENCRYPT) {
		SS_ERR("rsa encrypt fail\n");
		return -EINVAL;
	} else {
		err = rsa_crypto_start(req_ctx);
		if (err) {
			SS_ERR("rsa decrypt fail\n");
			return -EINVAL;
		}
	}
	SS_ERR("do_rsa_crypto sucess\n");
	return err;
}

static void ce_ecc_config(crypto_ecc_req_ctx_t *req, ce_task_desc_t *task)
{
	task->chan_id = req->channel_id;
	ss_method_set(req->dir, SS_METHOD_ECC, task);
	ss_ecc_width_set(req->width / 8, task);
	ss_ecc_op_mode_set(req->mode, task);
	task->comm_ctl |= 1 << CE_COMM_CTL_TASK_INT_SHIFT;
}

static int ce_ecc_data_init(crypto_ecc_req_ctx_t *req, ce_task_desc_t *task, phys_addr_t key_phy,
	phys_addr_t iv_phy, phys_addr_t src_phy, phys_addr_t dst_phy)
{
	u32 src_data_len, dst_data_len, ecc_word_size;

	ecc_word_size = req->width / BITS_PER_WORD;	/* in word */
	if (req->width == CE_ECC_WIDTH_521)
		ecc_word_size = CE_ECC_521_WORD_NUM;

	switch (req->mode) {
	case CE_ECC_OP_ENC:
		src_data_len = ecc_word_size * CE_ECC_ENC_SRC_NUM;
		dst_data_len = ecc_word_size * CE_ECC_ENC_DST_NUM;
		/* config task src */
		ce_task_addr_set(0, key_phy, (u8 *)&(task->src[0].addr));
		task->src[0].len = ecc_word_size;

		ce_task_addr_set(0, iv_phy, (u8 *)&(task->src[1].addr));
		task->src[1].len = ecc_word_size;

		ce_task_addr_set(0, src_phy, (u8 *)&(task->src[2].addr));
		task->src[2].len = src_data_len - 2 * ecc_word_size;

		task->data_len = src_data_len * BYTES_PER_WORD;	/* length in byte */

		/* config task dst */
		ce_task_addr_set(0, dst_phy, (u8 *)&(task->dst[0].addr));
		task->dst[0].len = dst_data_len;
		break;
	case CE_ECC_OP_VERIFY:
		src_data_len = ecc_word_size * CE_ECC_VERIFY_SRC_NUM;
		dst_data_len = ecc_word_size;

		/* config task src */
		ce_task_addr_set(0, src_phy, (u8 *)&(task->src[0].addr));
		task->src[0].len = src_data_len;
		task->data_len = src_data_len * BYTES_PER_WORD;	/* length in byte */

		/* config task dst */
		ce_task_addr_set(0, dst_phy, (u8 *)&(task->dst[0].addr));
		task->dst[0].len = dst_data_len;
		task->next_task_addr = 0;
		break;
	default:
		SS_ERR("ecc mode not support\n");
		return -EINVAL;
	}

	return 0;
}

int do_ecc_crypto(crypto_ecc_req_ctx_t *req)
{
	int err;
	int channel_id = req->channel_id;
	phys_addr_t key_phy;
	phys_addr_t iv_phy;
	phys_addr_t dst_phy;
	phys_addr_t src_phy;
	ce_task_desc_t *task;

	task = ce_task_alloc();
	if (!task)
		return -ENOMEM;

	ce_ecc_config(req, task);;

	key_phy = dma_map_single(ce_cdev->pdevice, req->key_buffer, req->key_length, DMA_TO_DEVICE);
	SS_DBG("pkey = 0x%px, key_phy_addr = 0x%px\n", req->key_buffer, (void *)key_phy);

	iv_phy = dma_map_single(ce_cdev->pdevice, req->iv_buffer, req->iv_length, DMA_TO_DEVICE);
	SS_DBG("sign = 0x%px, iv_phy_addr = 0x%px\n", req->iv_buffer, (void *)iv_phy);

	src_phy = dma_map_single(ce_cdev->pdevice, req->src_buffer, req->src_length, DMA_TO_DEVICE);
	SS_DBG("src = 0x%px, src_phy_addr = 0x%px\n", req->src_buffer, (void *)src_phy);

	dst_phy = dma_map_single(ce_cdev->pdevice, req->dst_buffer, req->dst_length, DMA_FROM_DEVICE);
	SS_DBG("dst = 0x%px, dst_phy_addr = 0x%px\n", req->dst_buffer, (void *)dst_phy);

	err = ce_ecc_data_init(req, task, key_phy, iv_phy, src_phy, dst_phy);
	if (err)
		goto out;

	/* start ce */
	ss_pending_clear(channel_id);
	ss_irq_enable(channel_id);
	ce_task_desc_print(task);

	init_completion(&ce_cdev->flows[channel_id].done);
	ss_ctrl_start(task);

	err = wait_for_completion_timeout(&ce_cdev->flows[channel_id].done, msecs_to_jiffies(SS_WAIT_TIME));
	if (!err) {
		SS_ERR("Timed out\n");
		ce_task_desc_print(task);
		ce_reg_print();
		ce_reset();
		err = -ETIMEDOUT;
		goto out;
	}

	ss_irq_disable(channel_id);

	SS_DBG("After CE, TSR: 0x%08x, ERR: 0x%08x\n", ss_reg_rd(CE_REG_TSR), ss_reg_rd(CE_REG_ERR));

	err = 0;
out:
	ce_task_destroy(task);

	/* key */
	dma_unmap_single(ce_cdev->pdevice, key_phy, req->key_length, DMA_TO_DEVICE);

	/* iv */
	dma_unmap_single(ce_cdev->pdevice, iv_phy, req->iv_length, DMA_TO_DEVICE);

	/* src */
	dma_unmap_single(ce_cdev->pdevice, src_phy, req->src_length, DMA_TO_DEVICE);

	/* dst */
	dma_unmap_single(ce_cdev->pdevice, dst_phy, req->dst_length, DMA_FROM_DEVICE);

	if (err)
		return err;

	err = ss_flow_err(channel_id);
	if (err) {
		SS_ERR("CE return error: %d\n", err);
		return -EINVAL;
	}
	/*
	 * Verify is to substitute the hash into the ECC curve for verification.
	 * If the verification is successful, output 1; otherwise, output 0.
	 */
	if (req->mode == CE_ECC_OP_VERIFY) {
		SS_DBG("the verify is %d\n", req->dst_buffer[0]);
		if (req->dst_buffer[0] != 1) {
			SS_ERR("ecc verify failed\n");
			return -2;
		}
	}

	return 0;
}

static int check_hash_ctx_vaild(crypto_hash_req_ctx_t *req)
{
	if (!req->text_buffer || !req->dst_buffer) {
		SS_ERR("Invalid para: text = 0x%px, dst = 0x%px\n",
		       req->text_buffer, req->dst_buffer);
		return -EINVAL;
	}

	if (!req->text_length) {
		SS_ERR("Invalid len: text = 0x%x\n", req->text_length);
		return -EINVAL;
	}

	if (req->key_length) {
		if (!req->key_buffer) {
			SS_ERR("Invalid para: key = 0x%px\n", req->key_buffer);
			return -EINVAL;
		}
	}

	if (req->iv_length) {
		if (!req->iv_buffer) {
			SS_ERR("Invalid para: key = 0x%px\n", req->iv_buffer);
			return -EINVAL;
		}
	}

	return 0;
}

static void ce_hash_config(crypto_hash_req_ctx_t *req, ce_task_desc_t *task)
{
	task->chan_id = req->channel_id;
	ss_hash_method_set(&req->hash_mode, task);
	ss_hash_cmd_set(task);
}

static void ce_task_hash_init(crypto_hash_req_ctx_t *req, phys_addr_t key_phy,
			      phys_addr_t text_phy, phys_addr_t dst_phy, phys_addr_t len_buf_phy, ce_task_desc_t *ptask)
{
	u64 total_bit_len = req->text_length * BITS_PER_BYTE;

	ce_task_addr_set(0, text_phy, (u8 *)&(ptask->src[0].addr));
	ptask->src[0].len = DIV_ROUND_UP(req->text_length, 4);

	ce_task_addr_set(0, dst_phy, (u8 *)&(ptask->dst[0].addr));
	ptask->dst[0].len = (req->dst_length) / BYTES_PER_WORD;

	if (req->hash_mode == SS_METHOD_SHA224)
		ptask->dst[0].len = SHA256_DIGEST_SIZE / BYTES_PER_WORD;

	if (req->hash_mode == SS_METHOD_SHA384)
		ptask->dst[0].len = SHA512_DIGEST_SIZE / BYTES_PER_WORD;

	ptask->data_len = total_bit_len;
	ce_task_addr_set(0, len_buf_phy, (u8 *)&(ptask->key_addr));

	if (req->key_length) {
		ce_task_addr_set(0, key_phy, (u8 *)&(ptask->ctr_addr));
	}
}

static int hash_crypto_start(crypto_hash_req_ctx_t *req)
{
	int err;
	u64 *total_bit_len_buf;
	phys_addr_t len_buf_phy;
	phys_addr_t key_phy = 0;
	phys_addr_t text_phy = 0;
	phys_addr_t dst_phy = 0;
	ce_task_desc_t *task;
	int channel_id = req->channel_id;

	task = ce_task_alloc();
	if (!task)
		return -ENOMEM;

	ce_hash_config(req, task);

	total_bit_len_buf = kzalloc(8, GFP_KERNEL | GFP_DMA);
	if (!total_bit_len_buf) {
		err = -ENOMEM;
		goto err0;
	}

	*total_bit_len_buf = (u64)req->text_length * BITS_PER_BYTE;
	len_buf_phy = dma_map_single(ce_cdev->pdevice, total_bit_len_buf, 8, DMA_TO_DEVICE);
	SS_DBG("total_bit_len_buf = 0x%px, len_buf_phy = 0x%px\n", total_bit_len_buf, (void *)len_buf_phy);

	/* task_key_set */
	if (req->key_length) {
		key_phy = dma_map_single(ce_cdev->pdevice, req->key_buffer, req->key_length, DMA_TO_DEVICE);
		SS_DBG("key = 0x%px, key_phy_addr = 0x%px\n", req->key_buffer, (void *)key_phy);
	}

	/* task_data_set */
	text_phy = dma_map_single(ce_cdev->pdevice, req->text_buffer, req->text_length, DMA_TO_DEVICE);
	SS_DBG("src = 0x%px, src_phy_addr = 0x%px\n", req->text_buffer, (void *)text_phy);

	/* the dst_buf is from user */
	dst_phy = dma_map_single(ce_cdev->pdevice, req->dst_buffer, req->dst_length, DMA_FROM_DEVICE);
	SS_DBG("dst = 0x%px, dst_phy_addr = 0x%px\n", req->dst_buffer, (void *)dst_phy);

	ce_task_hash_init(req, key_phy, text_phy, dst_phy, len_buf_phy, task);

	/* start ce */
	ss_pending_clear(channel_id);
	ss_irq_enable(channel_id);

	init_completion(&ce_cdev->flows[channel_id].done);
	ss_ctrl_start(task);
	ce_reg_print();
	ce_task_desc_print(task);

	err = wait_for_completion_timeout(&ce_cdev->flows[channel_id].done,
					  msecs_to_jiffies(SS_WAIT_TIME));
	if (err == 0) {
		SS_ERR("Timed out\n");
		ce_reg_print();
		ce_task_desc_print(task);
		ce_reset();
		err = -ETIMEDOUT;
		goto err1;
	}

	ss_irq_disable(channel_id);

	if (ss_flow_err(channel_id)) {
		SS_ERR("CE return error: %d\n", ss_flow_err(channel_id));
		err = -EINVAL;
		goto err1;
	}
	err = 0;

err0:
	ce_task_destroy(task);

err1:
	kfree(total_bit_len_buf);

	/* key */
	if (req->key_length)
		dma_unmap_single(ce_cdev->pdevice, key_phy, req->key_length, DMA_TO_DEVICE);

	dma_unmap_single(ce_cdev->pdevice, len_buf_phy, 8, DMA_TO_DEVICE);

	dma_unmap_single(ce_cdev->pdevice, text_phy, req->text_length, DMA_TO_DEVICE);

	dma_unmap_single(ce_cdev->pdevice, dst_phy, req->dst_length, DMA_FROM_DEVICE);

	return err;
}

int do_hash_crypto(crypto_hash_req_ctx_t *req)
{
	int err;

	err = check_hash_ctx_vaild(req);
	if (err)
		return err;

	err = hash_crypto_start(req);
	if (err) {
		SS_ERR("hash encrypt fail\n");
		return err;
	}

	return 0;
}

int do_rng_crypto(crypto_rng_req_ctx_t *req)
{
	// TODO
	return 0;
}
